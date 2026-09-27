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

#include <llvm/Support/CommandLine.h>

#include "converter/survey.h"
#include "cpp2rust_lib.h"
#include "logging.h"

namespace fs = std::filesystem;

namespace {
llvm::cl::OptionCategory cpp2rust_cmdargs("Cpp2Rust options");

llvm::cl::opt<bool> Verbose("verbose", llvm::cl::desc("Enable verbose logging"),
                            llvm::cl::init(false),
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

llvm::cl::opt<bool>
    Survey("survey",
           llvm::cl::desc("Discovery mode: record every translation gap in the "
                          "TU instead of aborting on the first one. Emits NO "
                          "Rust output at all"),
           llvm::cl::init(false), llvm::cl::cat(cpp2rust_cmdargs));

llvm::cl::opt<std::string>
    SurveyOut("survey-out",
              llvm::cl::desc("Path for the --survey gap report (TSV)"),
              llvm::cl::value_desc("gaps.tsv"), llvm::cl::cat(cpp2rust_cmdargs));

llvm::cl::opt<bool> MangleUnmapped(
    "mangle-unmapped",
    llvm::cl::desc("TRIAGE ONLY. Emit an undefined mangled name for an unmapped "
                   "system type instead of failing, so every missing type in a "
                   "TU can be counted in one pass. The output DOES NOT COMPILE; "
                   "never use this in a harness default"),
    llvm::cl::init(false), llvm::cl::cat(cpp2rust_cmdargs));

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
  llvm::cl::HideUnrelatedOptions(cpp2rust_cmdargs);
  llvm::cl::ParseCommandLineOptions(argc, argv);

  cpp2rust::SetVerbose(Verbose);

  cpp2rust::survey::MangleUnmapped() = MangleUnmapped;
  if (Survey) {
    if (SurveyOut.empty()) {
      llvm::errs() << "ERROR: --survey requires --survey-out=<file>\n";
      return EXIT_FAILURE;
    }
    cpp2rust::survey::Enable(SurveyOut, CcFile.empty() ? BuildDir : CcFile);
  } else if (!SurveyOut.empty()) {
    llvm::errs() << "ERROR: --survey-out requires --survey\n";
    return EXIT_FAILURE;
  } else if (RsFile.empty()) {
    llvm::errs() << "ERROR: -o is required\n";
    return EXIT_FAILURE;
  }

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

  auto rs_code =
      BuildDir.empty()
          ? cpp2rust::TranspileSrc(cc_code, model, cxx_flags, RulesDir, CcFile)
          : cpp2rust::TranspileDir(BuildDir, model, RulesDir);

  if (Survey) {
    // Survey mode is a discovery pass: gaps out, no code out. Deliberately
    // never writes -o, never runs rustfmt, never leaves a partial .rs behind.
    cpp2rust::survey::Write();
    llvm::errs() << "survey: " << cpp2rust::survey::state().gaps.size()
                 << " distinct gaps written to " << SurveyOut << '\n';
    return EXIT_SUCCESS;
  }

  if (rs_code.empty()) {
    llvm::errs() << "ERROR: empty output file\n";
    return EXIT_FAILURE;
  }

  std::ofstream file(RsFile);
  if (!file) {
    llvm::errs() << "ERROR: failed to open " << RsFile << '\n';
    return EXIT_FAILURE;
  }

  file << rs_code;
  file.close();

  // call rustfmt.
  std::string rustfmt_command =
      "rustfmt +" RUST_STABLE_VERSION " --edition 2024 " + RsFile;
  if (std::system(rustfmt_command.c_str()) != 0) {
    llvm::errs() << "ERROR: failed to run rustfmt\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
