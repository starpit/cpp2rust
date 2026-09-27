// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// `std::exception` -- THE EMPTY POLYMORPHIC BASE, AND NOTHING ELSE.
//
// WHY THIS MODULE EXISTS
// ---------------------------------------------------------------------------
// It is not wanted for its own sake.  `DtException` (repos/dt_src/util/
// dt_exception.hpp:18) derives from it, and DtException is the ONE type this
// program throws (100% of 11,525 throw sites).  DtException is USER code, so the
// converter ports it from its own declaration -- but it cannot port a BASE it has
// no model for.  Measured, on four throwing TUs, the survey row is exactly:
//
//   unmapped-type  system type has no rule: `std::exception` (would be emitted
//   as the undefined name `std_exception`)  rule key: searched as:
//   std::exception
//
// count=1 per TU, so ONE type entry is the whole of what those TUs ask for.
//
// THE REPRESENTATION IS `()`, AND THAT IS A STATEMENT ABOUT C++, NOT A STUB.
// ---------------------------------------------------------------------------
// `std::exception` has NO DATA MEMBERS.  Its entire content is a vtable pointer
// plus two virtuals (`what()`, the destructor).  As the base sub-object of
// DtException it therefore contributes ZERO observable state: every byte of a
// thrown DtException lives in DtException's own `message` / `stacktrace`.  The
// empty Rust unit type is the faithful image of an empty base, and the `init` in
// both target files is the unit value.
//
// WHAT IS DELIBERATELY ABSENT, AND WHY ABSENT IS THE CORRECT STATE
// ---------------------------------------------------------------------------
// `what()` IS NOT MAPPED.  It is declared below only so the class is the real
// class; no rule is written for it, so the converter ABORTS loudly on any call
// through a `std::exception` base reference rather than emitting something that
// compiles and lies.  This matters because dt_src really does call it that way --
// `catch (const std::exception &e) { ... e.what() ... }` at 20 recorded sites,
// e.g. sgr/sengraph.cpp:1215, ddb/src/ddb_std.cpp:341,
// dcc/tools/LitAutoTestGen/LitAutoTestGenMain.cpp:107.  That call is a VIRTUAL
// DISPATCH from the base to the derived override, and an empty-unit base cannot
// carry a vtable: there is no honest body for it here.  The only correct place to
// resolve it is the converter's catch lowering (a separate, in-flight row), which
// knows the dynamic type of the payload.  A rule returning, say, an empty string
// would make every diagnostic in the program print nothing, silently.
//
// Likewise absent: the destructor, and `std::bad_alloc` /
// `std::runtime_error` / `std::logic_error` and the rest of <stdexcept>.  None
// of them appears in any measured survey row for this target.
// ---------------------------------------------------------------------------

namespace std {

// The declaration is LOCAL rather than `#include <exception>` for the reason
// rules/mlir, rules/raw_ostream and rules/stringref all give: a type rule needs
// only the class NAME, and cpp-rule-preprocessor compiles this file with a fixed
// flag set.  A declared-but-unmapped member records nothing, so declaring
// `what()` here does not create a rule for it.
class exception {
public:
  virtual const char *what() const noexcept;
};

}  // namespace std

// THE TYPE ENTRY.  A type rule is recorded by a `using tN = <type>;` alias, the
// same form rules/mlir uses (`using t1 = mlir::Operation;`) -- the class
// declaration above on its own records NOTHING.  Measured: without this line the
// module generated `OK` and wrote an EMPTY `ir_src.json` (`{}`), i.e. a module
// that loads fine and maps nothing at all.
using t1 = std::exception;
