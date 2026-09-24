// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <vector>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <limits.h>
#include <unistd.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

#include <clang/Basic/Stack.h>
#include <llvm/Support/CommandLine.h>

#include "cpp2rust_lib.h"
#include "logging.h"
#include "opaque.h"
#include "survey.h"

namespace fs = std::filesystem;

namespace {
llvm::cl::OptionCategory cpp2rust_cmdargs("Cpp2Rust options");

llvm::cl::opt<bool> Verbose("verbose", llvm::cl::desc("Enable verbose logging"),
                            llvm::cl::init(false),
                            llvm::cl::cat(cpp2rust_cmdargs));

llvm::cl::opt<bool> StrictDir(
    "strict-dir",
    llvm::cl::desc(
        "With --dir, fail if clang reported an error for any file. Output is "
        "still written; without this a truncated AST -- an unopenable #include "
        "is fatal to clang but not to the run -- yields Rust that looks fine "
        "and exits zero. Measurement should always pass this"),
    llvm::cl::init(false), llvm::cl::cat(cpp2rust_cmdargs));

llvm::cl::opt<bool> DirFailFast(
    "dir-fail-fast",
    llvm::cl::desc(
        "With --dir, let a converter abort or fault kill the process instead of "
        "containing it at the translation-unit boundary. By default one bad TU "
        "is recorded as failed and the remaining TUs still run. Pass this when "
        "you want the core dump"),
    llvm::cl::init(false), llvm::cl::cat(cpp2rust_cmdargs));

llvm::cl::opt<std::string>
    Survey("survey",
           llvm::cl::desc("Do not abort on an unsupported construct: record it, "
                          "carry on, and write every gap found to this file. "
                          "The Rust output of a survey run is not usable"),
           llvm::cl::value_desc("survey.tsv"),
           llvm::cl::cat(cpp2rust_cmdargs));

llvm::cl::opt<std::string> CcFile("file",
                                  llvm::cl::desc("Path to the C++ file"),
                                  llvm::cl::value_desc("file.cpp"),
                                  llvm::cl::cat(cpp2rust_cmdargs));

llvm::cl::opt<std::string>
    BuildDir("dir",
             llvm::cl::desc("Directory that contains compile_commands.json"),
             llvm::cl::value_desc("dir"), llvm::cl::cat(cpp2rust_cmdargs));

llvm::cl::opt<std::string> RsFile("o", llvm::cl::desc("Path to the Rust file"),
                                  llvm::cl::value_desc("output.rs"),
                                  llvm::cl::Required,
                                  llvm::cl::cat(cpp2rust_cmdargs));

llvm::cl::opt<std::string>
    Model("model",
          llvm::cl::desc(
              "Name of the translation model (unsafe, refcount [default])"),
          llvm::cl::value_desc("model"), llvm::cl::init("refcount"),
          llvm::cl::cat(cpp2rust_cmdargs));

llvm::cl::opt<std::string>
    RulesDir("rules",
             llvm::cl::desc("Directory where translation rules are located"),
             llvm::cl::value_desc("rules"), llvm::cl::cat(cpp2rust_cmdargs));

llvm::cl::list<std::string> OpaqueNamespaces(
    "opaque-namespace",
    llvm::cl::desc("Treat this C++ namespace as an opaque API boundary: do "
                   "not translate its declarations, and spell a reference to "
                   "one as an undefined Rust type instead of failing. "
                   "Repeatable, or comma-separated"),
    llvm::cl::value_desc("mlir"), llvm::cl::ZeroOrMore,
    llvm::cl::CommaSeparated, llvm::cl::cat(cpp2rust_cmdargs));

llvm::cl::list<std::string> CXXFlags("cxxflags",
                                     llvm::cl::desc("Additional CXXFLAGS"),
                                     llvm::cl::value_desc("cxxflags"),
                                     llvm::cl::ZeroOrMore,
                                     llvm::cl::cat(cpp2rust_cmdargs));

} // namespace

// Get the directory of the running executable
static fs::path GetExecutableDir() {
#if defined(_WIN32)
  char path[MAX_PATH];
  GetModuleFileNameA(NULL, path, MAX_PATH);
  return fs::path(path).parent_path();
#elif defined(__linux__)
  char path[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
  return fs::path(std::string_view(path, std::max((ssize_t)0, count)))
      .parent_path();
#elif defined(__APPLE__)
  uint32_t size = 0;
  _NSGetExecutablePath(nullptr, &size); // get path length
  std::vector<char> buffer(size);
  _NSGetExecutablePath(buffer.data(), &size);
  return fs::path(buffer.data()).parent_path();
#endif
  return ".";
}

static bool HasIRFiles(const fs::path &dir) {
  std::error_code ec;
  for (auto it = fs::recursive_directory_iterator(dir, ec);
       !ec && it != fs::recursive_directory_iterator(); it.increment(ec)) {
    if (!it->is_directory()) {
      continue;
    }
    const auto &p = it->path();
    if (fs::exists(p / "ir_src.json") && (fs::exists(p / "ir_unsafe.json") ||
                                          fs::exists(p / "ir_refcount.json"))) {
      return true;
    }
  }
  return false;
}

static bool ResolveRulesDir() {
  std::array<fs::path, 2> candidates = {
      fs::path("./rules"), GetExecutableDir().parent_path() / "rules"};

  for (const auto &dir : candidates) {
    if (fs::exists(dir) && fs::is_directory(dir) && HasIRFiles(dir)) {
      RulesDir = fs::canonical(dir).string();
      llvm::errs() << "Using rules directory: " << RulesDir << '\n';
      return true;
    }
  }
  return false;
}

int main(int argc, char *argv[]) {
  // Records the approximate bottom of the stack, which is what makes
  // clang::isStackNearlyExhausted() -- and so the converter's stack guard --
  // able to answer at all. Must run here, as near the bottom as possible;
  // clang's own drivers do the same thing in their main().
  clang::noteBottomOfStack(/*ForceSet=*/true);
  llvm::cl::HideUnrelatedOptions(cpp2rust_cmdargs);
  llvm::cl::ParseCommandLineOptions(argc, argv);

  cpp2rust::SetVerbose(Verbose);
  cpp2rust::SetSurvey(Survey);
  cpp2rust::Opaque::SetNamespaces(
      std::vector<std::string>(OpaqueNamespaces.begin(),
                               OpaqueNamespaces.end()));

  if (CcFile.empty() && BuildDir.empty()) {
    llvm::errs() << "ERROR: please provide either --file or --dir\n";
    return EXIT_FAILURE;
  }

  if (!CcFile.empty() && !BuildDir.empty()) {
    llvm::errs() << "ERROR: please provide only one of --file or --dir\n";
    return EXIT_FAILURE;
  }

  if (!BuildDir.empty() && !CXXFlags.empty()) {
    llvm::errs() << "ERROR: can't combine --dir with --cxxflags\n";
    return EXIT_FAILURE;
  }

  auto model = cpp2rust::Model::kRefCount;
  if (Model == "refcount") {
    // ok
  } else if (Model == "unsafe") {
    model = cpp2rust::Model::kUnsafe;
  } else {
    llvm::errs() << "ERROR: unknown model: " << Model << '\n';
    return EXIT_FAILURE;
  }

  std::string cc_code;
  if (!CcFile.empty()) {
    std::ifstream file(CcFile);
    if (!file) {
      llvm::errs() << "ERROR: failed to open " << CcFile << '\n';
      return EXIT_FAILURE;
    }
    cc_code = {std::istreambuf_iterator<char>(file),
               std::istreambuf_iterator<char>()};
    if (cc_code.empty()) {
      llvm::errs() << "ERROR: empty source file\n";
      return EXIT_FAILURE;
    }
  }

  std::vector<std::string_view> cxx_flags(CXXFlags.begin(), CXXFlags.end());

  if (RulesDir.empty() && !ResolveRulesDir()) {
    llvm::errs() << "ERROR: rules directory not found. "
                    "Please specify one with --rules\n";
    return EXIT_FAILURE;
  }

  bool dir_ok = true;
  std::vector<cpp2rust::TuStatus> tus;
  cpp2rust::SetDirContainCrashes(!DirFailFast);
  auto rs_code =
      BuildDir.empty()
          ? cpp2rust::TranspileSrc(cc_code, model, cxx_flags, RulesDir, CcFile)
          : cpp2rust::TranspileDir(BuildDir, model, RulesDir, &dir_ok, &tus);

  // The per-TU summary of a --dir run.
  //
  // A multi-TU run used to have exactly one outcome for all of its TUs: the
  // process exit status. One converter assert in one TU aborted the process,
  // the output was whatever had been written before it, and the
  // alphabetically-later TUs were absent with nothing saying so -- which is how
  // "27 of 58 translate" was believed when the truth was 50 of 58. Printing
  // one line per TU is the fix for the measurement; containing the crash is
  // only what makes the later lines exist.
  bool any_tu_crashed = false;
  if (!tus.empty()) {
    size_t translated = 0;
    for (const auto &tu : tus) {
      if (tu.state == cpp2rust::TuStatus::State::kTranslated) {
        ++translated;
      } else if (tu.state == cpp2rust::TuStatus::State::kCrashed) {
        any_tu_crashed = true;
      }
    }
    llvm::errs() << "\n=== per-TU status: " << translated << '/' << tus.size()
                 << " translated ===\n";
    for (const auto &tu : tus) {
      if (tu.state == cpp2rust::TuStatus::State::kTranslated) {
        continue;
      }
      llvm::errs() << "  " << cpp2rust::TuStateName(tu.state) << '\t' << tu.file
                   << '\t' << tu.reason << '\n';
    }
    if (translated != tus.size()) {
      llvm::errs() << "  (" << (tus.size() - translated)
                   << " TU(s) did not translate; the Rust output is missing "
                      "their contents)\n";
    }
  }

  if (rs_code.empty()) {
    llvm::errs() << "ERROR: empty output file\n";
    return EXIT_FAILURE;
  }

  // A clang failure in --dir mode is not survivable even though output exists:
  // an unopenable `#include` is fatal, the AST is truncated, and whatever Rust
  // came out is translated from an incomplete program. Exiting zero here is what
  // let survey.py score such TUs as OK.
  //
  // Warn always, fail only under --strict-dir. Making it fatal by default would
  // be the honest thing, but clang also reports errors that this converter has
  // always translated through, so flipping the default silently reclassifies
  // every such TU. Measurement wants the strict reading; pass the flag there.
  //
  // The exit is deferred until after the file is written. Returning here threw
  // away the TUs that DID translate, which is the same loss this change exists
  // to stop -- and it contradicted this flag's own documented "Output is still
  // written".
  if (!dir_ok) {
    llvm::errs() << (StrictDir ? "ERROR" : "WARNING")
                 << ": clang reported errors for at least one file in "
                 << BuildDir << "; the Rust output is not trustworthy\n";
  }
  const bool strict_dir_failed = !dir_ok && StrictDir;

  std::ofstream file(RsFile);
  if (!file) {
    llvm::errs() << "ERROR: failed to open " << RsFile << '\n';
    return EXIT_FAILURE;
  }

  file << rs_code;
  file.close();

  // call rustfmt. A survey run emits placeholders where it could not
  // translate, so its output is not expected to parse. Neither does the output
  // of a run that lost a TU to a crash: it is missing that TU's items, so
  // anything referring to them does not resolve. Skip rustfmt there rather than
  // reporting its failure as the problem.
  if (Survey.empty() && !any_tu_crashed) {
    std::string rustfmt_command =
        "rustfmt +" RUST_STABLE_VERSION " --edition 2024 " + RsFile;
    if (std::system(rustfmt_command.c_str()) != 0) {
      llvm::errs() << "ERROR: failed to run rustfmt\n";
      return EXIT_FAILURE;
    }
  }

  // A contained crash is fatal unconditionally: the whole point is that the run
  // reports it instead of dying silently, and an exit status of 0 with a TU
  // missing from the output would be a worse lie than the abort it replaced.
  if (any_tu_crashed) {
    llvm::errs() << "ERROR: at least one TU crashed during conversion; the "
                    "output is incomplete (see the per-TU status above)\n";
    return EXIT_FAILURE;
  }
  if (strict_dir_failed) {
    return EXIT_FAILURE;
  }

  // A survey run that recorded gaps did NOT translate this input, and must not
  // report success -- see SurveyFoundGaps. The TSV is already written and
  // flushed, so the enumeration is not lost by exiting non-zero; what changes is
  // that a harness scoring by exit status can no longer count a surveyed TU as
  // OK.
  if (cpp2rust::SurveyFoundGaps()) {
    llvm::errs() << "ERROR: survey recorded at least one unsupported construct; "
                    "see " << Survey << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
