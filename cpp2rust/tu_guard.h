#pragma once

// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Per-TU abort containment for --dir runs.
//
// WHY: `TranspileDir` used to be a single `ClangTool Tool(db, files);
// Tool.run(&factory)` -- one process, one invocation, no per-TU guard. Any
// `assert(0)` (SIGABRT), `exit(1)` or segfault anywhere in ANY TU killed the
// process and `-o` was never written. Measured on the 13-TU ddl_standalone DB
// at 0683c180: the run died on TU 1 of 13 and emitted nothing.
//
// The guard is ARMED ONLY on the --dir path. On the --file path `Armed()` is
// false and `BailOut` behaves exactly as the bare `exit(1)` it replaced, so a
// single-TU translation is byte-for-byte unchanged.
//
// LOUDNESS OVER CONTAINMENT: every dropped TU is reported on stderr *and* as a
// `// cpp2rust: DROPPED TU <file> -- <site>` comment in the emitted file, and
// the process exits with a distinct nonzero code. A --dir run that quietly
// omits part of a target is the failure mode this project has hit repeatedly.

#include <string>

namespace cpp2rust::tu_guard {

// True while a --dir per-TU guard is installed and a longjmp target is live.
bool Armed();

// Called from a translation abort site. If the guard is armed, unwinds to the
// per-TU recovery point in TranspileDir; otherwise exits(1) as before.
[[noreturn]] void BailOut(const char *site);

// Number of TUs dropped by the guard during the last --dir run.
int DroppedCount();

// Human-readable list of dropped TUs (for the final stderr summary).
const std::string &DroppedSummary();

} // namespace cpp2rust::tu_guard
