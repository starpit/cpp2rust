// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include "cpp2rust_lib.h"

#include <clang/Tooling/ArgumentsAdjusters.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CrashRecoveryContext.h>
#include <llvm/Support/raw_ostream.h>

#include <algorithm>
#include <csignal>
#include <filesystem>
#include <vector>

#include "compat/platform_flags.h"
#include "converter/converter.h"
#include "converter/models/converter_refcount.h"
#include "frontend_action.h"

namespace cpp2rust {
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
  clang::tooling::runToolOnCodeWithArgs(
      std::make_unique<FrontendAction>(rs_code, model, /*first=*/true,
                                       rules_dir),
      cc_code, tool_args, std::filesystem::path(filename).filename().string(),
      filename.ends_with(".c") ? CLANG_C_COMPILER : CLANG_CXX_COMPILER);
  Converter::EmitOpaqueRecords(rs_code);
  Converter::EmitOpaqueEnumConstants(rs_code);
  Converter::EmitVirtualMethods(rs_code);
  if (model == Model::kRefCount) {
    ConverterRefCount::EmitMethodsOnPtr(rs_code);
  }
  Converter::EmitGlobalInits(model, rs_code);
  return rs_code;
}

const char *TuStateName(TuStatus::State state) {
  switch (state) {
  case TuStatus::State::kTranslated:
    return "translated";
  case TuStatus::State::kClangError:
    return "clang-error";
  case TuStatus::State::kCrashed:
    return "crashed";
  }
  return "?";
}

namespace {
// Make a stack-exhaustion SIGSEGV catchable.
//
// CrashRecoveryContext installs a SIGSEGV handler, but by default that handler
// runs on the faulting thread's own stack -- and when the fault IS stack
// exhaustion there is no stack left to run it on, so the kernel escalates to an
// uncatchable kill. Measured: the converter's deep-AST recursion on
// ProgramCorrection.cpp died with a bare signal 11 and no "contained" report
// even with RunSafely wrapped around it, while an assert (SIGABRT) and an
// ordinary null-deref were both contained fine.
//
// The fix is a dedicated signal stack plus SA_ONSTACK on the handler LLVM
// already installed. Measured after this: the same overflow is contained,
// twice in a row, and the process carries on.
//
// This handles the frames clang itself does not guard. It is not a fix for
// unbounded recursion -- it turns an uncatchable crash into a reported per-TU
// failure, which is all it does.
void EnableStackOverflowRecovery() {
#ifndef _WIN32
  static std::vector<char> alt_stack;
  if (!alt_stack.empty()) {
    return;
  }
  alt_stack.resize(std::max<size_t>(SIGSTKSZ, 1 << 20));
  stack_t ss{};
  ss.ss_sp = alt_stack.data();
  ss.ss_size = alt_stack.size();
  ss.ss_flags = 0;
  if (sigaltstack(&ss, nullptr) != 0) {
    return;
  }
  // Re-register LLVM's own handler with SA_ONSTACK; do not replace it.
  for (int sig : {SIGSEGV, SIGBUS}) {
    struct sigaction sa{};
    if (sigaction(sig, nullptr, &sa) != 0) {
      continue;
    }
    sa.sa_flags |= SA_ONSTACK;
    sigaction(sig, &sa, nullptr);
  }
#endif
}

// The three argument adjusters every `--dir` invocation needs, applied to a
// tool that will run over exactly one file.
void AddDirArgumentAdjusters(clang::tooling::ClangTool &tool) {
  tool.appendArgumentsAdjuster(clang::tooling::getInsertArgumentAdjuster(
      getPlatformClangBeginFlags(),
      clang::tooling::ArgumentInsertPosition::BEGIN));
  tool.appendArgumentsAdjuster(clang::tooling::getInsertArgumentAdjuster(
      getPlatformClangEndFlags(), clang::tooling::ArgumentInsertPosition::END));
  // Redefine __FILE__ to use just the basename, so the generated code
  // doesn't contain system-specific absolute paths.
  tool.appendArgumentsAdjuster(
      [](const clang::tooling::CommandLineArguments &args,
         llvm::StringRef filename) {
        auto result = args;
        auto basename =
            std::filesystem::path(filename.str()).filename().string();
        result.push_back("-Wno-builtin-macro-redefined");
        result.push_back("-D__FILE__=\"" + basename + "\"");
        return result;
      });
}
} // namespace

// Whether a converter crash inside one TU is contained (default) or left to
// kill the process. Fail-fast is what a debugger or a core dump needs, so it
// stays reachable.
static bool dir_contain_crashes_ = true;
void SetDirContainCrashes(bool contain) { dir_contain_crashes_ = contain; }

std::string TranspileDir(std::string_view build_dir, Model model,
                         const std::string &rules_dir, bool *ok,
                         std::vector<TuStatus> *tus) {
  if (ok) {
    *ok = false;
  }
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

  // ONE ClangTool PER TU, all in ONE process -- deliberately, and the
  // distinction matters.
  //
  // What has to be shared across TUs is the Converter's cross-TU dedup state.
  // That state is not held in a Converter object: `ASTConsumer::
  // HandleTranslationUnit` already builds a FRESH Converter for every TU, and
  // the dedup indexes (`decl_ids_`, `record_decls_`, `inner_structs_`,
  // `globals_`, `global_inits_`, `abstract_structs_`, `virtual_methods_`) are
  // static members, i.e. process-global. Together with the single `rs_code`
  // accumulator below and the one-shot `first_` preamble flag, THAT is the
  // "shared Converter". Splitting one ClangTool over 58 files into 58 tools
  // over one file each does not touch any of it, so cross-TU dedup is
  // preserved exactly -- which is the whole reason not to run a process per TU
  // and concatenate (measured: 22,902 duplicate-name errors that way, 6 this
  // way).
  //
  // What a per-TU tool buys is a boundary a crash can be caught at. The
  // converter's failure mode is `assert()`, which calls abort(); abort() cannot
  // be caught by any in-process construct except a signal handler, and the
  // build is deliberately -fno-exceptions besides. llvm::CrashRecoveryContext
  // is exactly that signal handler: measured here, it returns control with
  // RetCode 134 for an assert and 139 for a fault, and the next RunSafely
  // still works. So no supervisor process is needed at all.
  //
  // Recovery is best-effort, and honestly so: the crashed TU's ASTContext is
  // never destroyed (a leak, irrelevant to a one-shot tool), and the dedup
  // indexes may retain a half-finished entry from it. A contained crash is
  // therefore reported as a FAILED TU, never quietly folded into the total.
  std::string rs_code;
  bool any_clang_error = false;
  bool any_crash = false;

  if (dir_contain_crashes_) {
    llvm::CrashRecoveryContext::Enable();
    // Must come AFTER Enable(), which is what installs the handler this
    // re-registers with SA_ONSTACK.
    EnableStackOverflowRecovery();
  }

  // `first_` is per-factory, and the preamble must be emitted exactly once
  // across the whole run, so the factory outlives the per-TU tools.
  FrontendActionFactory factory(rs_code, model, rules_dir);

  for (size_t i = 0; i < files.size(); ++i) {
    const std::string &file = files[i];
    llvm::errs() << "[" << (i + 1) << '/' << files.size() << "] " << file
                 << '\n';

    clang::tooling::ClangTool tool(*compile_dbase, {file});
    AddDirArgumentAdjusters(tool);

    // Where this TU's output starts, so a crash can be rolled back to it
    // rather than leaving a half-translated function in the file.
    const size_t mark = rs_code.size();

    TuStatus status;
    status.file = file;

    // ClangTool::run() is non-zero if clang failed on the file, and its
    // failures are not all recoverable: a `#include` that cannot be opened is
    // fatal, yet the action still runs over the truncated AST and still
    // produces Rust. Report it rather than letting non-empty output speak for
    // success.
    int rc = 0;
    if (dir_contain_crashes_) {
      llvm::CrashRecoveryContext crc;
      crc.DumpStackAndCleanupOnFailure = false;
      if (!crc.RunSafely([&] { rc = tool.run(&factory); })) {
        status.state = TuStatus::State::kCrashed;
        status.signum = crc.RetCode;
        status.reason =
            crc.RetCode == 134
                ? "converter abort (assert failed) -- signal 6"
                : (crc.RetCode == 139
                       ? "converter fault (signal 11; a bad deref, or stack "
                         "exhaustion from deep AST recursion)"
                       : "converter crash, RetCode " +
                             std::to_string(crc.RetCode));
        // Drop this TU's partial Rust. Keeping it would put a truncated item
        // into an output the summary calls incomplete anyway.
        rs_code.resize(mark);
        any_crash = true;
        llvm::errs() << "    CONTAINED: " << status.reason
                     << "; continuing with the remaining "
                     << (files.size() - i - 1) << " file(s)\n";
        if (tus) {
          tus->push_back(std::move(status));
        }
        continue;
      }
    } else {
      rc = tool.run(&factory);
    }

    if (rc != 0) {
      status.state = TuStatus::State::kClangError;
      status.reason = "clang reported errors (ClangTool rc=" +
                      std::to_string(rc) + "); AST may be truncated";
      any_clang_error = true;
    }
    if (tus) {
      tus->push_back(std::move(status));
    }
  }

  Converter::EmitOpaqueRecords(rs_code);
  Converter::EmitOpaqueEnumConstants(rs_code);
  Converter::EmitVirtualMethods(rs_code);
  if (model == Model::kRefCount) {
    ConverterRefCount::EmitMethodsOnPtr(rs_code);
  }
  Converter::EmitGlobalInits(model, rs_code);
  // `ok` keeps its original meaning -- "clang was happy with every TU" -- so
  // that --strict-dir still classifies exactly what it classified before. A
  // crash is NOT folded in here: it is unconditionally fatal at the caller,
  // where the per-TU summary is printed, because a contained crash that only
  // showed up under an opt-in flag would be the same silent truncation in a new
  // costume.
  (void)any_crash;
  if (ok) {
    *ok = !any_clang_error;
  }
  return rs_code;
}
} // namespace cpp2rust
