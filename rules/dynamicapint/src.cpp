// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::DynamicAPInt -- LLVM's arbitrary-precision signed integer.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/ADT/DynamicAPInt.h>
// ---------------------------------------------------------------------------
// Same reason as rules/stringref, rules/densemap, rules/raw_ostream and
// rules/twine: cpp-rule-preprocessor compiles this file with a fixed flag set,
// and the only flags that would reach LLVM's headers are absolute -I paths into
// whatever LLVM tree the target project happens to have built.  That would make
// `ninja` in this repo fail for anyone without that tree.  So the signatures
// LLVM declares are restated here instead, and a restatement matches iff it
// agrees with LLVM exactly.
//
// THE MODEL IS i64, AND THE HEADER IS WHAT LICENSES THAT
// -----------------------------------------------------
// DynamicAPInt is a UNION of a fast path and a slow path
// (llvm/ADT/DynamicAPInt.h:49-52):
//
//     union {
//       int64_t ValSmall;
//       detail::SlowDynamicAPInt ValLarge;
//     };
//
// so the small representation is a SIGNED 64-BIT INT, verbatim -- not an
// unsigned word, not a 32-bit int, not a pointer.  The public surface agrees in
// both directions: the only integral constructor is
// `explicit DynamicAPInt(int64_t Val)` (:116) and the only integral conversion
// is `explicit operator int64_t() const` (:151).  int64_t is `long` on the
// x86-64 Linux target, which is how the signatures below spell it -- the same
// "spell it the way src.cpp spells the C++ one" rule rules/stringref records
// for size_t/`unsigned long`.
//
// COMPARISON IS VALUE EQUALITY, not identity.  LLVM's own body (:265-269) is
//
//     if (LLVM_LIKELY(isSmall() && O.isSmall()))
//       return getSmall() == O.getSmall();
//     return detail::SlowDynamicAPInt(*this) == detail::SlowDynamicAPInt(O);
//
// i.e. both paths compare the NUMBER.  This is the exact opposite of the
// iterator case rules/densemap and rules/stringmap record, and mixing the two
// up is silent wrongness in both directions, so it is measured rather than
// argued: the verification probe reaches one value by two different
// expressions (4000000000*2 and 4000000000+4000000000) and checks they compare
// EQUAL, which a position/identity body would report as 0.
//
// WHAT THE i64 MODEL COSTS, SAID OUT LOUD
// ---------------------------------------
// The whole point of DynamicAPInt is that it does NOT overflow: past 64 bits it
// switches to SlowDynamicAPInt and keeps going.  This model stops at 64 bits,
// so a computation whose INTERMEDIATE values exceed int64_t would wrap here and
// widen in C++.  That is a real divergence and it is named here rather than
// discovered later.  It is accepted because:
//
//   * This module covers exactly the operations below: construction from an
//     int64_t, copy, and == / !=.  NONE of them can create a value that did not
//     already fit in an int64_t -- there is no arithmetic rule here to widen
//     past the fast path.  A value built by `DynamicAPInt(int64_t)` is small by
//     construction (:116-119 writes ValSmall and zeroes the BitWidth tag), and
//     `==` on two small values is `getSmall() == O.getSmall()`, which is
//     precisely the emitted body.  So within this module's surface the model is
//     not an approximation, it is the same function.
//   * Arithmetic (+ - * / %, abs, gcd, lcm, ceilDiv, floorDiv, mod) and the
//     orderings < <= > >= are therefore NOT modelled, deliberately: those are
//     the operators that could leave the fast path, and each would need the
//     widening decision above answered with a site to check it against.  They
//     stay a loud abort rather than a guessed body.
//   * The probe checks the boundary that is reachable: a value well beyond
//     32 bits (8000000000) must not truncate, which an i32 or u32 model would
//     get wrong.
//
// == AND != ARE MEMBERS here, not free functions and not hidden friends:
// :157-158 declares `bool operator==(const DynamicAPInt &O) const` inside the
// class.  (The class also declares hidden-friend `operator==(const DynamicAPInt
// &, int64_t)` forms at :200-201 for the int64_t mixed comparison; those are a
// different signature and are not reached by the sites in scope, so they are
// not restated.)  The receiver and the operand are both `const DynamicAPInt &`,
// and a const-reference parameter of a value-model type arrives by value in the
// Rust signature -- the same shape rules/densemap's f1/f2 use for their
// `const DenseMapIterator &` operands.

namespace llvm {

// Restated from llvm/ADT/DynamicAPInt.h:48.  The union member layout is
// irrelevant to signature matching -- and it cannot be restated anyway without
// dragging in APInt -- so the class is declared with the public surface this
// module needs and nothing else.  It must be COMPLETE because every signature
// below takes or returns one.
class DynamicAPInt {
public:
  // llvm/ADT/DynamicAPInt.h:116 -- explicit DynamicAPInt(int64_t Val)
  explicit DynamicAPInt(long Val);
  // llvm/ADT/DynamicAPInt.h:133 -- DynamicAPInt(const DynamicAPInt &O)
  DynamicAPInt(const DynamicAPInt &O);

  // llvm/ADT/DynamicAPInt.h:157 -- bool operator==(const DynamicAPInt &) const
  bool operator==(const DynamicAPInt &O) const;
  // llvm/ADT/DynamicAPInt.h:158 -- bool operator!=(const DynamicAPInt &) const
  bool operator!=(const DynamicAPInt &O) const;

  // llvm/ADT/DynamicAPInt.h:200-201 -- the int64_t MIXED comparisons, declared
  // as HIDDEN FRIENDS in-class.  The header comment above said these "are not
  // reached by the sites in scope"; they now are --
  // `coefficients[dim_id] == 1` compares a DynamicAPInt element against a bare
  // integer literal, which resolves to this overload after the int -> int64_t
  // conversion, NOT to the member operator== above.  A hidden friend MUST be
  // restated in-class: declared at namespace scope the rule silently never
  // resolves (the trap rules/smallvector's operator== pair records).
  //
  // These are safe under the i64 model for the same reason == / != on two
  // DynamicAPInts are: a COMPARISON cannot create a value, so it cannot leave
  // the fast path.  This does NOT extend the module's contract on arithmetic.
  friend bool operator==(const DynamicAPInt &A, long B);
  friend bool operator!=(const DynamicAPInt &A, long B);
};

} // namespace llvm

using t1 = llvm::DynamicAPInt;

llvm::DynamicAPInt f1(long v) { return llvm::DynamicAPInt(v); }

llvm::DynamicAPInt f2(const llvm::DynamicAPInt &o) {
  return llvm::DynamicAPInt(o);
}

bool f3(const llvm::DynamicAPInt &a, const llvm::DynamicAPInt &b) {
  return a.operator==(b);
}

bool f4(const llvm::DynamicAPInt &a, const llvm::DynamicAPInt &b) {
  return a.operator!=(b);
}

// A hidden friend is reached by the FREE-function spelling, not `a.operator==`
// (the member overload above is the only candidate for the member syntax, and
// it cannot bind a long).  Still never the bare `a == b`, which records no src
// entry.
bool f5(const llvm::DynamicAPInt &a, long b) { return operator==(a, b); }

bool f6(const llvm::DynamicAPInt &a, long b) { return operator!=(a, b); }
