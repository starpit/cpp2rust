#pragma once

// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <string>
#include <vector>

#include "converter/factory.h"

namespace cpp2rust {

std::string TranspileSrc(std::string_view cc_code, Model model,
                         const std::vector<std::string_view> &cxx_flags,
                         const std::string &rules_dir,
                         std::string_view filename);

// The outcome of ONE translation unit of a `--dir` run.
//
// This type exists because a multi-TU run used to have no per-TU outcome at
// all: it had one process exit status for all of them. One converter assert in
// one TU aborted the process, the output file was whatever had been written
// before it, and every alphabetically-later TU was simply absent -- with
// nothing in the output saying so. That is how "27 of 58 TUs translate" was
// believed for a long time when the real number was 50: the 27 was the count
// of TUs that ran before TU number 2 killed the run.
struct TuStatus {
  enum class State {
    // Converted, and clang diagnosed no error for it.
    kTranslated,
    // clang reported an error for this TU. It still yields Rust, from a
    // truncated AST, so this is NOT a success -- see the `ok` note below.
    kClangError,
    // The converter itself aborted (an assert) or faulted inside this TU. The
    // partial Rust it had emitted is rolled back out of the output.
    kCrashed,
  };
  std::string file;
  State state = State::kTranslated;
  // Human-readable detail; empty when kTranslated.
  std::string reason;
  // Signal number for kCrashed (6 = SIGABRT, i.e. an assert; 11 = SIGSEGV),
  // 0 otherwise.
  int signum = 0;
};

const char *TuStateName(TuStatus::State state);

// Containment is on by default. Turn it off to let a converter crash kill the
// process where a debugger or a core dump wants it (`--dir-fail-fast`).
void SetDirContainCrashes(bool contain);

// `ok` is false when clang failed on any TU, or when any TU crashed.
//
// A TU whose header could not be opened still yields a non-empty string,
// translated from an incomplete AST, so "did we produce output" is not the same
// question as "did it work". Scoring that as success is how survey.py came to
// report OK for TUs that had silently skipped a header.
//
// `tus`, when given, receives one entry per translation unit the compilation
// database named. A per-TU converter crash is CONTAINED: it is recorded in
// `tus`, its partial output is rolled back, and the remaining TUs still run.
// The caller must report the failures and exit non-zero -- a contained crash
// that nobody mentions would be strictly worse than the abort it replaced.
std::string TranspileDir(std::string_view build_dir, Model model,
                         const std::string &rules_dir, bool *ok = nullptr,
                         std::vector<TuStatus> *tus = nullptr);

// `ok` is false when clang failed on any TU -- a missing header, a parse error.
// The caller MUST honour it: a TU whose header could not be opened still yields
// a non-empty string, translated from an incomplete AST, so "did we produce
// output" is not the same question as "did it work". Scoring that as success is
// how survey.py came to report OK for TUs that had silently skipped a header.
std::string TranspileDir(std::string_view build_dir, Model model,
                         const std::string &rules_dir, bool *ok = nullptr);

} // namespace cpp2rust
