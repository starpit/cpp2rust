// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include "cpp2rust_lib.h"

#include <clang/Tooling/ArgumentsAdjusters.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <csetjmp>
#include <csignal>
#include <cstdlib>
#include <filesystem>

#include "compat/platform_flags.h"
#include "converter/converter.h"
#include "converter/models/converter_refcount.h"
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
std::string TranspileSrc(std::string_view cc_code, Model model,
                         const std::vector<std::string_view> &cxx_flags,
                         const std::string &rules_dir,
                         std::string_view filename) {
  auto tool_args = getPlatformClangBeginFlags();
  tool_args.push_back("-fparse-all-comments");
  tool_args.insert(tool_args.end(), cxx_flags.begin(), cxx_flags.end());
  auto end_flags = getPlatformClangEndFlags();
  tool_args.insert(tool_args.end(), end_flags.begin(), end_flags.end());

  // The in-memory TU must keep the real path of the file it was read from:
  // a quoted #include is resolved relative to the *including file's*
  // directory, so passing a bare basename makes clang believe the TU lives in
  // the process CWD and every sibling header becomes invisible.
  // Redefine __FILE__ to just the basename (as TranspileDir does) so the
  // generated code still does not contain system-specific absolute paths.
  auto basename = std::filesystem::path(filename).filename().string();
  tool_args.push_back("-Wno-builtin-macro-redefined");
  tool_args.push_back("-D__FILE__=\"" + basename + "\"");

  std::string rs_code;
  clang::tooling::runToolOnCodeWithArgs(
      std::make_unique<FrontendAction>(rs_code, model, /*first=*/true,
                                       rules_dir),
      cc_code, tool_args, std::string(filename),
      filename.ends_with(".c") ? CLANG_C_COMPILER : CLANG_CXX_COMPILER);
  Converter::EmitOpaqueRecords(rs_code);
  Converter::EmitVirtualMethods(rs_code);
  if (model == Model::kRefCount) {
    ConverterRefCount::EmitMethodsOnPtr(rs_code);
  }
  Converter::EmitGlobalInits(model, rs_code);
  return rs_code;
}

std::string TranspileDir(std::string_view build_dir, Model model,
                         const std::string &rules_dir) {
  std::string error_message;
  auto compile_dbase = clang::tooling::CompilationDatabase::loadFromDirectory(
      build_dir, error_message);
  if (!compile_dbase) {
    return {};
  }

  std::vector<std::string> files;
  for (const auto &compile_command : compile_dbase->getAllCompileCommands()) {
    files.emplace_back(compile_command.Filename);
  }

  std::string rs_code;
  // ONE factory for the whole run: its `first_` flag gates the prelude, so it
  // must NOT be recreated per TU. The per-TU state that makes the merge/dedup
  // work lives in Converter:: process-static storage (decl_ids_, record_decls_,
  // virtual_methods_, globals_, global_inits_), not in the ClangTool, so
  // running one ClangTool per file preserves it.
  FrontendActionFactory factory(rs_code, model, rules_dir);

  tu_guard::InstallHandlers();

  for (size_t i = 0; i < files.size(); ++i) {
    const std::string &file = files[i];
    llvm::errs() << '[' << (i + 1) << '/' << files.size() << "] Processing "
                 << file << '\n';

    // Snapshot the emitted code so a failing TU can be rolled back whole.
    const size_t pre_tu_size = rs_code.size();
    tu_guard::g_current_file = file.c_str();

    if (sigsetjmp(tu_guard::g_recover, 1) != 0) {
      // A TU aborted. Truncate back to the pre-TU boundary and record it
      // LOUDLY: on stderr above, and as a comment in the emitted file.
      tu_guard::g_armed = false;
      rs_code.resize(pre_tu_size);
      rs_code += "// cpp2rust: DROPPED TU ";
      rs_code += file;
      rs_code += " -- aborted during translation; see stderr for the site\n";
      ++tu_guard::g_dropped;
      tu_guard::g_summary += "  " + file + '\n';
      continue;
    }

    tu_guard::g_armed = true;
    {
      clang::tooling::ClangTool Tool(*compile_dbase,
                                     std::vector<std::string>{file});
      Tool.appendArgumentsAdjuster(clang::tooling::getInsertArgumentAdjuster(
          getPlatformClangBeginFlags(),
          clang::tooling::ArgumentInsertPosition::BEGIN));
      Tool.appendArgumentsAdjuster(clang::tooling::getInsertArgumentAdjuster(
          getPlatformClangEndFlags(),
          clang::tooling::ArgumentInsertPosition::END));
      // Redefine __FILE__ to use just the basename, so the generated code
      // doesn't contain system-specific absolute paths.
      Tool.appendArgumentsAdjuster(
          [](const clang::tooling::CommandLineArguments &args,
             llvm::StringRef filename) {
            auto result = args;
            auto basename =
                std::filesystem::path(filename.str()).filename().string();
            result.push_back("-Wno-builtin-macro-redefined");
            result.push_back("-D__FILE__=\"" + basename + "\"");
            return result;
          });
      Tool.run(&factory);
    }
    tu_guard::g_armed = false;
  }

  if (tu_guard::g_dropped > 0) {
    llvm::errs() << "cpp2rust: DROPPED " << tu_guard::g_dropped << " of "
                 << files.size() << " TUs:\n"
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
