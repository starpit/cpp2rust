// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include "cpp2rust_lib.h"

#include <clang/Tooling/ArgumentsAdjusters.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/MemoryBuffer.h>

#include <csetjmp>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <set>

#include "compat/platform_flags.h"
#include "converter/converter.h"
#include "converter/models/converter_refcount.h"
#include "converter/survey.h"
#include "frontend_action.h"
#include "tu_guard.h"

namespace cpp2rust {

namespace tu_guard {
namespace {
// Per-TU recovery point, live only while `armed` is true.
sigjmp_buf g_recover;
bool g_armed = false;
int g_dropped = 0;
std::string g_summary;
const char *g_current_file = "";

// SIGABRT/SIGSEGV/SIGBUS/SIGILL/SIGFPE land here. siglongjmp out of a signal
// handler is the only way back: this pod has no gdb and cannot link ASAN, and
// the alternative is losing the whole run's output to one bad TU.
void SignalTrampoline(int sig) {
  if (!g_armed) {
    // Not our TU: restore the default disposition and let it kill us, so a
    // crash outside a guarded region is still a crash.
    std::signal(sig, SIG_DFL);
    std::raise(sig);
    return;
  }
  g_armed = false;
  const char *name = sig == SIGABRT   ? "SIGABRT (assert)"
                     : sig == SIGSEGV ? "SIGSEGV"
                     : sig == SIGBUS  ? "SIGBUS"
                     : sig == SIGILL  ? "SIGILL"
                                      : "fatal signal";
  llvm::errs() << "cpp2rust: CONTAINED " << name << " in TU "
               << g_current_file << '\n';
  siglongjmp(g_recover, sig);
}

// exit() from inside a TU (converter.cpp's VisitRecoveryExpr, mapper.cpp's
// duplicate-rule bail, converter_refcount.cpp's unsupported-cast bails) is
// routed through BailOut, which longjmps instead when armed.
void InstallHandlers() {
  for (int sig : {SIGABRT, SIGSEGV, SIGBUS, SIGILL, SIGFPE}) {
    std::signal(sig, SignalTrampoline);
  }
  // `LLVM ERROR:` (llvm::report_fatal_error, used by mapper.cpp for an
  // unmapped type) calls exit(1) directly -- neither a signal nor a route
  // through BailOut -- so without this handler ONE such TU kills the whole
  // --dir run and the remaining TUs are never even attempted.
  llvm::install_fatal_error_handler(
      [](void *, const char *reason, bool) {
        llvm::errs() << "LLVM ERROR: " << reason << '\n';
        BailOut("llvm::report_fatal_error");
        std::exit(1); // unreachable when armed
      },
      nullptr);
}
} // namespace

bool Armed() { return g_armed; }

void BailOut(const char *site) {
  if (!g_armed) {
    // --file path: identical behaviour to the bare exit(1) this replaced.
    std::exit(1);
  }
  g_armed = false;
  llvm::errs() << "cpp2rust: CONTAINED abort at " << site << " in TU "
               << g_current_file << '\n';
  siglongjmp(g_recover, 1);
}

int DroppedCount() { return g_dropped; }

const std::string &DroppedSummary() { return g_summary; }

} // namespace tu_guard

namespace {
// The ONE way this process drives clang, shared by --file and --dir.
//
// --dir used to use `clang::tooling::ClangTool`, and every single `#include`
// in that path failed with `err_cannot_open_file` on search-path entry 1:
// header search claimed to find a FileEntryRef and then createFileID could not
// open it. A two-line `#include <cstdint>` hello-world reproduced it, with the
// DB's exact argv parsing cleanly under raw clang++, so the breakage was in
// ClangTool's own FileManager/OverlayFileSystem, not in the flags. Going
// through runToolOnCodeWithArgs -- which builds a fresh FileManager over an
// InMemoryFileSystem-on-RealFileSystem overlay per call -- resolves includes
// correctly, so --dir now reads each TU's bytes and comes through here too.
//
// `factory` is the run-wide FrontendActionFactory: its `first_` flag gates the
// file preamble, so it must be passed in rather than created per TU.
//
// Returns false if the compilation emitted an error or fatal diagnostic; the
// caller MUST treat that as a dropped TU, never as a silent success.
bool RunOneTU(clang::tooling::FrontendActionFactory &factory,
              std::string_view cc_code, std::vector<std::string> tool_args,
              std::string_view filename) {
  // The in-memory TU must keep the real path of the file it was read from:
  // a quoted #include is resolved relative to the *including file's*
  // directory, so passing a bare basename makes clang believe the TU lives in
  // the process CWD and every sibling header becomes invisible.
  // Redefine __FILE__ to just the basename so the generated code still does
  // not contain system-specific absolute paths.
  auto basename = std::filesystem::path(filename).filename().string();
  tool_args.push_back("-Wno-builtin-macro-redefined");
  tool_args.push_back("-D__FILE__=\"" + basename + "\"");

  return clang::tooling::runToolOnCodeWithArgs(
      factory.create(), cc_code, tool_args, std::string(filename),
      filename.ends_with(".c") ? CLANG_C_COMPILER : CLANG_CXX_COMPILER);
}

// Turn one compile-database entry's argv into flags for RunOneTU: drop
// argv[0], `-c`, `-o <x>`, the diagnostic noise and the source path itself,
// and make relative include directories absolute against the entry's
// `directory` field (the process CWD is not that directory). Mirrors the
// filter in verif/dxpflags.py, which is the known-working reference.
void AppendDbFlags(const clang::tooling::CompileCommand &cmd,
                   std::vector<std::string> &out) {
  static const std::set<std::string> kTakesArg = {
      "-I",       "-isystem", "-iquote",   "-idirafter", "-include",
      "-D",       "-U",       "-std",      "-Xclang",    "-isysroot",
      "--sysroot"};
  static const std::set<std::string> kPathArg = {"-I", "-isystem", "-iquote",
                                                 "-idirafter"};
  const std::string &dir = cmd.Directory;
  auto absolutize = [&dir](const std::string &p) {
    if (p.empty() || p[0] == '/') {
      return p;
    }
    return (std::filesystem::path(dir) / p).lexically_normal().string();
  };

  const auto &toks = cmd.CommandLine;
  for (size_t i = 1; i < toks.size(); ++i) { // i = 1: skip argv[0]
    const std::string &t = toks[i];
    if (t.empty() || t[0] != '-') {
      continue; // the source path, and anything else positional
    }
    if (t == "-c") {
      continue;
    }
    if (t == "-o") {
      ++i;
      continue;
    }
    // Diagnostic and codegen options: irrelevant to translation, and -Werror
    // would turn a warning into a dropped TU.
    if (t.rfind("-W", 0) == 0 || t.rfind("-O", 0) == 0 ||
        t.rfind("-M", 0) == 0 || t == "-g" || t == "-pedantic" ||
        t == "-pipe") {
      continue;
    }
    if (kTakesArg.count(t) && i + 1 < toks.size()) { // separated: -I <dir>
      out.push_back(t);
      out.push_back(kPathArg.count(t) ? absolutize(toks[i + 1]) : toks[i + 1]);
      ++i;
      continue;
    }
    if (t.size() > 2 && t.rfind("-I", 0) == 0) { // joined: -I<dir>
      out.push_back("-I");
      out.push_back(absolutize(t.substr(2)));
      continue;
    }
    out.push_back(t);
  }
}
} // namespace

std::string TranspileSrc(std::string_view cc_code, Model model,
                         const std::vector<std::string_view> &cxx_flags,
                         const std::string &rules_dir,
                         std::string_view filename) {
  auto tool_args = getPlatformClangBeginFlags();
  tool_args.push_back("-fparse-all-comments");
  tool_args.insert(tool_args.end(), cxx_flags.begin(), cxx_flags.end());
  auto end_flags = getPlatformClangEndFlags();
  tool_args.insert(tool_args.end(), end_flags.begin(), end_flags.end());

  std::string rs_code;
  FrontendActionFactory factory(rs_code, model, rules_dir);
  // MEASUREMENT INTEGRITY -- the --file= twin of the check in TranspileDir.
  // A TU whose compilation emitted an error or fatal diagnostic (an #include
  // that never resolved, say) has NOT been translated: what the converter saw
  // was a truncated AST, and whatever Rust it emitted from it is a fabrication.
  // Before this, --file= discarded RunOneTU's bool and returned that Rust, so
  // the caller wrote it out, ran rustfmt over it and exited 0 -- which made
  // every translate-rate number taken through --file= (i.e. the whole census)
  // untrustworthy.  --file= translates exactly one TU, so "drop the TU" and
  // "produce nothing" are the same thing: throw the partial Rust away and
  // return empty, which cpp2rust.cpp turns into EXIT_FAILURE.  The stderr line
  // is what makes it loud: an empty return on its own would read as a clean
  // 0-line translation.
  //
  // NOT ARMING tu_guard HERE IS DELIBERATE, not an omission: the guard needs a
  // live sigsetjmp recovery point (tu_guard::g_recover, set only in
  // TranspileDir's loop) and InstallHandlers() is likewise only called there.
  // Arming it on this path would make BailOut siglongjmp into a jmp_buf that
  // was never set. tu_guard.h states the contract: "The guard is ARMED ONLY on
  // the --dir path. On the --file path Armed() is false and BailOut behaves
  // exactly as the bare exit(1) it replaced."
  if (!RunOneTU(factory, cc_code, std::move(tool_args), filename)) {
    llvm::errs() << "cpp2rust: FATAL DIAGNOSTIC in TU " << filename
                 << " -- not translated\n";
    // MEASUREMENT INTEGRITY, --survey half (row g3003). The empty return above
    // is the whole signal on the translate path: cpp2rust.cpp turns it into
    // `ERROR: empty output file` + EXIT_FAILURE. SURVEY MODE NEVER REACHES
    // THAT CHECK -- it returns EXIT_SUCCESS before it, by design, because a
    // survey legitimately produces no Rust. So the two things survey mode does
    // produce, the record and the exit code, BOTH have to be told.
    //
    // MEASURED, not inferred: before this, `--survey` on a TU with an
    // unresolvable #include exited 0 and wrote a record byte-identical past the
    // `#tu` line to a clean TU's, i.e. "0 distinct gaps". THAT is the defect;
    // the exit code is the lesser half.
    //
    // 1. The RECORD, because a number nobody reads is not a signal. A missing
    //    row is invisible to every survey consumer; a `harness-fault` row is
    //    not. Record() itself rewrites the TSV, so this survives even if the
    //    process is killed before cpp2rust.cpp's own Write().
    if (survey::Enabled()) {
      survey::Record(survey::GapKind::kHarnessFault,
                     "clang emitted an error/fatal diagnostic: the AST was "
                     "TRUNCATED and this TU was NOT surveyed -- its zero gaps "
                     "mean nothing; see stderr for the diagnostic",
                     std::string(filename));
    }
    // 2. The EXIT CODE, through the signal --dir already uses rather than a
    //    third convention: cpp2rust.cpp reads tu_guard::DroppedCount() and has
    //    code 3 for "output written, but TU(s) dropped". On the translate path
    //    this changes nothing -- the empty return hits `rs_code.empty()` and
    //    returns EXIT_FAILURE long before the DroppedCount() check -- so the
    //    --file= behaviour proven at d6151ccb is untouched.
    ++tu_guard::g_dropped;
    tu_guard::g_summary += "  " + std::string(filename) + '\n';
    return {};
  }
  Converter::EmitOpaqueRecords(rs_code);
  Converter::EmitVirtualMethods(rs_code);
  if (model == Model::kRefCount) {
    ConverterRefCount::EmitMethodsOnPtr(rs_code);
  }
  Converter::EmitGlobalInits(model, rs_code);
  return rs_code;
}

std::string TranspileDir(std::string_view build_dir, Model model,
                         const std::vector<std::string_view> &cxx_flags,
                         const std::string &rules_dir) {
  std::string error_message;
  auto compile_dbase = clang::tooling::CompilationDatabase::loadFromDirectory(
      build_dir, error_message);
  if (!compile_dbase) {
    return {};
  }

  auto commands = compile_dbase->getAllCompileCommands();

  std::string rs_code;
  // ONE factory for the whole run: its `first_` flag gates the prelude, so it
  // must NOT be recreated per TU. The per-TU state that makes the merge/dedup
  // work lives in Converter:: process-static storage (decl_ids_, record_decls_,
  // virtual_methods_, globals_, global_inits_), not in the ClangTool, so
  // running one ClangTool per file preserves it.
  FrontendActionFactory factory(rs_code, model, rules_dir);

  tu_guard::InstallHandlers();

  for (size_t i = 0; i < commands.size(); ++i) {
    const std::string &file = commands[i].Filename;
    llvm::errs() << '[' << (i + 1) << '/' << commands.size() << "] Processing "
                 << file << '\n';

    // Snapshot the emitted code so a failing TU can be rolled back whole.
    const size_t pre_tu_size = rs_code.size();
    tu_guard::g_current_file = file.c_str();

    // Roll the emitted code back to the pre-TU boundary and record the drop
    // LOUDLY: on stderr, and as a comment in the emitted file.
    auto drop = [&](const char *why) {
      rs_code.resize(pre_tu_size);
      rs_code += "// cpp2rust: DROPPED TU ";
      rs_code += file;
      rs_code += " -- ";
      rs_code += why;
      rs_code += "\n";
      ++tu_guard::g_dropped;
      tu_guard::g_summary += "  " + file + '\n';
    };

    if (sigsetjmp(tu_guard::g_recover, 1) != 0) {
      tu_guard::g_armed = false;
      drop("aborted during translation; see stderr for the site");
      continue;
    }

    // Read the TU's bytes: --dir goes through the same runToolOnCodeWithArgs
    // machinery as --file, because ClangTool's FileManager cannot open any
    // #include it finds (see RunOneTU).
    auto buffer = llvm::MemoryBuffer::getFile(file);
    if (!buffer) {
      llvm::errs() << "cpp2rust: cannot read " << file << ": "
                   << buffer.getError().message() << '\n';
      drop("source file could not be read");
      continue;
    }

    std::vector<std::string> tool_args = getPlatformClangBeginFlags();
    tool_args.push_back("-fparse-all-comments");
    AppendDbFlags(commands[i], tool_args);
    // AFTER the DB's flags, mirroring TranspileSrc: these are the caller's
    // supplement to what the database could not record (a `--sysroot`, an extra
    // `-D`), so a later `-D` wins and the DB's own include dirs keep priority.
    tool_args.insert(tool_args.end(), cxx_flags.begin(), cxx_flags.end());
    auto end_flags = getPlatformClangEndFlags();
    tool_args.insert(tool_args.end(), end_flags.begin(), end_flags.end());

    tu_guard::g_armed = true;
    const bool ok =
        RunOneTU(factory, (*buffer)->getBuffer(), std::move(tool_args), file);
    tu_guard::g_armed = false;

    // MEASUREMENT INTEGRITY: a TU whose compilation emitted an error or fatal
    // diagnostic (an #include that never resolved, say) has not been
    // translated. Before this, --dir reported such a TU as a silent success,
    // which made every translate-rate number from --dir untrustworthy.
    if (!ok) {
      llvm::errs() << "cpp2rust: FATAL DIAGNOSTIC in TU " << file
                   << " -- not translated\n";
      drop("compilation emitted a fatal diagnostic; see stderr");
    }
  }

  if (tu_guard::g_dropped > 0) {
    llvm::errs() << "cpp2rust: DROPPED " << tu_guard::g_dropped << " of "
                 << commands.size() << " TUs:\n"
                 << tu_guard::g_summary;
  }

  Converter::EmitOpaqueRecords(rs_code);
  Converter::EmitVirtualMethods(rs_code);
  if (model == Model::kRefCount) {
    ConverterRefCount::EmitMethodsOnPtr(rs_code);
  }
  Converter::EmitGlobalInits(model, rs_code);
  return rs_code;
}
} // namespace cpp2rust
