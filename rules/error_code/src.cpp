// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::error_code -- the LLVM output idiom's out-parameter.
//
// All eight rows that ask for this type (g2854-g2861) are the same construct:
//
//     std::error_code EC;
//     llvm::raw_fd_ostream OS(path, EC);
//     if (EC) { report; return; }
//
// at emitOutputFiles / printModule / dumpModule / ensureDir / EmitSpyreCodePass.
// `rules/raw_ostream` already owns `llvm::raw_fd_ostream` (t4/t5), so the TYPE
// key here is what lets that already-modelled type's declaration be reached.
//
// ⛔ WHAT THIS MODULE DELIBERATELY DOES NOT DO, AND WHY -- READ BEFORE ADDING.
//
// The whole POINT of `std::error_code EC` is that the callee writes into it and
// the caller BRANCHES on it.  `llvm::ToolOutputFile` is a standing REFUSAL in
// this tree for exactly that reason: a stub that leaves the out-param "no
// error" makes the corpus take the WRONG BRANCH -- it keeps a file it should
// have deleted, or reports success on a failed write -- and it does so
// SILENTLY, compiling and running.
//
// So: CAN THE BRANCH BE TAKEN HONESTLY HERE?  **No, and it is not this
// module's decision.**  The thing that would have to report the failure is
// `llvm::raw_fd_ostream`'s two-argument constructor
// `raw_fd_ostream(StringRef, std::error_code &)`, and `rules/raw_ostream` has
// NO constructor key at all (verified by readback of pin/ir.v16/raw_ostream:
// its keys are f1-f18 = errs/outs/dbgs/flush/operator<<, t1-t5 = the two
// classes and their reference/pointer forms -- no ctor).  No rule anywhere can
// therefore ever WRITE a nonzero value into an error_code.  Any `value()`,
// `message()`, `==`/`!=`, `category()` or `clear()` rule added here would be a
// rule that CLAIMS TO REPORT A REAL ERROR while structurally unable to observe
// one, which is the ToolOutputFile failure mode.
//
// Therefore, per the brief's instruction for exactly this case, this module
// keys ONLY:
//   * the TYPE (`std::error_code`, and its reference form for the out-param),
//   * the DEFAULT CONSTRUCTOR -- which in C++ yields a "no error" code, i.e.
//     the value the model represents faithfully, and
//   * `operator bool` -- which on that value is FALSE, which is CORRECT for a
//     default-constructed error_code.
// Everything else is LEFT OUT so it fails loudly at translate time.
//
// ⛔ `message()` IS REFUSED ON A SECOND, INDEPENDENT GROUND: its text is
// locale- and implementation-defined ("No such file or directory" is libc++ on
// glibc and nothing guarantees it elsewhere), so ANY mapping returns a
// PLAUSIBLE WRONG STRING into program output.  Do not add it.
//
// HONEST RESIDUAL RISK, stated rather than hidden: if `rules/raw_ostream` later
// keys the `(StringRef, std::error_code &)` constructor WITHOUT plumbing a real
// `std::io::Error` into the out-param, every site will take the success branch
// unconditionally.  That would be a defect in THAT rule, and the comment here
// exists so whoever writes it sees this first.  As of this module landing, the
// ctor is unkeyed, so those sites still abort LOUDLY on the constructor.
//
// MODEL: a plain `i32` errno.  0 means "no error", which is what the default
// constructor produces and the only value reachable through the keys above.
// Not `Option<std::io::Error>`: an Option would suggest a real error can be
// carried, and none can.

#include <system_error>

using t1 = std::error_code;
using t2 = std::error_code &;

// The default constructor: libc++ gives `__val_ = 0` with the system category,
// i.e. no error.
std::error_code f1() { return std::error_code(); }

// `explicit operator bool() const noexcept` -- true iff value() != 0.  On a
// default-constructed code that is FALSE, which is the correct answer and the
// only one reachable (see the header comment).
bool f2(const std::error_code &o) { return o.operator bool(); }
