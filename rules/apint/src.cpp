// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::DynamicAPInt -- LLVM's small-value-optimised arbitrary-precision
// integer (llvm/ADT/DynamicAPInt.h:43).  `mlir::DynamicAPInt` is a using-decl
// for this same class (mlir/Support/LLVM.h), so one key covers both spellings.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/ADT/DynamicAPInt.h>
// ----------------------------------------------------------------------------
// Same reason as rules/twine and rules/raw_ostream: cpp-rule-preprocessor
// compiles this file with a fixed flag set, and the only flags that would reach
// LLVM's real headers are absolute -I paths into whatever LLVM tree the target
// project happens to have built.  A rule matches on a signature STRING, so the
// restatement below is faithful iff it agrees with LLVM exactly; each member
// carries the header line it was copied from.
//
// MODEL: i64, AND WHY THAT IS HONEST RATHER THAN A GUESS
// -----------------------------------------------------
// DynamicAPInt is a UNION of an inline `int64_t ValSmall` and a heap
// `SlowDynamicAPInt`, and it holds the int64_t whenever the value fits
// (header:43-50).  Arbitrary precision is therefore an OVERFLOW path, not the
// representation.  Modelling it as i64 is wrong only for a program that
// actually overflows 64 bits.
//
// The corpus was enumerated before modelling (`grep -rn DynamicAPInt` over the
// whole of dt_src -- four sites, all of them small-integer):
//   * dcc/src/Conversion/VectorChainLowering/CommonHelpers/VectorChainHelper.cpp
//     :118,:121 -- `int64_t(mask_set_flat.getConstantBound(LB/UB, 0).value())`.
//     The value is IMMEDIATELY narrowed to int64_t by the source itself, so any
//     precision above 64 bits is discarded by the C++ program too.  This is the
//     site the census first-abort names (via
//     `std::__optional_storage_base<llvm::DynamicAPInt>`).
//   * dcc/src/Dialect/Sentient/SentientOps.cpp:1080-1081 --
//     `DynamicAPInt(ub.getValue().getSExtValue() - lb...getSExtValue())` and
//     `DynamicAPInt(step.getValue().getSExtValue())`.  Constructed FROM an
//     int64_t, so the value provably fits one.
//   * dcc/src/Transform/Sentient/ScalarSimplifications.cpp:80-95 --
//     `ArrayRef<mlir::DynamicAPInt> coefficients` compared against the literals
//     1, -1 and 0, and the function BAILS on anything else ("We don't support
//     multiplication with non-unit value").
// So no site in this corpus can observe a value outside i64, and two of the
// three narrow to int64_t explicitly.  If a future site holds a product of
// coefficients or a symbolic bound, this model becomes silently wrong and must
// be replaced by a real bignum -- that is the one thing to re-check here.
//
// NO Drop.  `~DynamicAPInt()` (header:129) exists but only frees the large
// union arm; with the i64 model there is no large arm and nothing to free, so a
// Drop impl would model an effect the program cannot observe.
//
// NOT COVERED, deliberately, because the corpus has zero sites for them: every
// arithmetic operator (`+ - * / %` and their compound forms, header:194-198),
// the ordering comparisons (header:202-205), `operator!=`, the `APInt`
// constructor (header:120), the `SlowDynamicAPInt` conversion (header:109), and
// `gcd`/`lcm`/`abs`/`ceilDiv`/`floorDiv`.  Each is a one-line body against i64
// and should be added when a site appears -- but a key nobody reaches is a key
// nobody has checked, so they are left out and will abort loudly.

#include <cstdint>

namespace llvm {

// Restated from llvm/ADT/DynamicAPInt.h.  The member layout is irrelevant to
// signature matching, but the class must be complete because f2 and f4 take one
// by const reference and f1 returns one by value.  LLVM's real member is the
// ValSmall/ValLarge union; a plain int64_t stands in for it.
class DynamicAPInt {
  int64_t ValSmall;

public:
  // llvm/ADT/DynamicAPInt.h:116 -- explicit DynamicAPInt(int64_t Val)
  explicit DynamicAPInt(int64_t Val);
  // llvm/ADT/DynamicAPInt.h:128 -- DynamicAPInt() : DynamicAPInt(0) {}
  DynamicAPInt();
  // llvm/ADT/DynamicAPInt.h:151 -- explicit operator int64_t() const
  explicit operator int64_t() const;

  // llvm/ADT/DynamicAPInt.h:200 -- friend bool operator==(const DynamicAPInt &,
  // int64_t).  Declared as a friend in LLVM, so it keys as a FREE function in
  // namespace llvm, not as a member.
  friend bool operator==(const DynamicAPInt &A, int64_t B);
};

} // namespace llvm

using t1 = llvm::DynamicAPInt;

// The default constructor.  Present because a type rule maps the TYPE ONLY: the
// converter looks the default ctor up as an ordinary expr rule and, on a miss,
// emits `<mangled>::new()`, which does not exist -- rc=0 then E0433.
llvm::DynamicAPInt f1() { return llvm::DynamicAPInt(); }

llvm::DynamicAPInt f2(int64_t v) { return llvm::DynamicAPInt(v); }

int64_t f3(const llvm::DynamicAPInt &x) { return x.operator int64_t(); }

bool f4(const llvm::DynamicAPInt &a, int64_t b) { return operator==(a, b); }
