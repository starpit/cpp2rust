// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::APInt -- LLVM's arbitrary-precision integer WITH AN EXPLICIT BIT WIDTH.
//
// READ rules/dynamicapint/src.cpp FIRST.  This module is its stricter sibling
// and it inherits that file's contract; the difference is the reason most of
// APInt is deliberately absent here.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/ADT/APInt.h>
// ---------------------------------------------------------------------------
// Same reason as rules/dynamicapint, rules/stringref, rules/densemap and
// rules/twine: cpp-rule-preprocessor compiles this file with a fixed flag set,
// and the only flags that would reach LLVM's headers are absolute -I paths into
// whatever LLVM tree the target project happens to have built.  A restatement
// matches iff it agrees with LLVM exactly, so every declaration below carries
// the header line it was copied from.
//
// THE MODEL IS u64 = THE FAST-PATH WORD, AND WHAT THAT COSTS
// ---------------------------------------------------------
// APInt's storage is (llvm/ADT/APInt.h, class body):
//
//     union { uint64_t VAL; uint64_t *pVal; } U;
//     unsigned BitWidth;
//
// i.e. a small-value fast path of exactly ONE UNSIGNED 64-BIT WORD, plus a
// separately stored BIT WIDTH.  This module models an APInt as that word, u64,
// and NOTHING models the width.  DynamicAPInt could get away with an i64
// because it has no width at all -- it just grows.  APInt is worse in exactly
// the way the width makes it worse: the width is OBSERVABLE, through
// `getBitWidth()` (:1489), through `trunc`/`zext`/`sext`, and through the
// wrap-at-width that `clearUnusedBits()` applies after every arithmetic op.
//
// So the rule this module follows is the one rules/dynamicapint states and this
// one tightens: MAP ONLY WHAT CANNOT LEAVE THE SINGLE WORD OR NEED THE WIDTH
// BACK, AND LEAVE EVERYTHING ELSE A LOUD ABORT.  Concretely:
//
//   MAPPED, and exact rather than approximate:
//     * `APInt(unsigned numBits, uint64_t val, bool, bool)` (:111).  This is
//       exact for numBits <= 64 in ALL FOUR combinations of the two defaulted
//       bools, which is not obvious and is the one interesting body here.  The
//       constructor's own code is: sign-extend val if isSigned, then
//       `clearUnusedBits()`, which ANDs the stored word with the low-numBits
//       mask.  Masking the RAW argument to numBits gives the same word in every
//       case -- sign extension only ever writes bits at or above numBits, which
//       the mask then discards -- and when !isSigned && !implicitTrunc LLVM
//       asserts `isUIntN(BitWidth, val)`, so the mask is a no-op on a
//       well-formed call.  The emitted body is therefore that mask, not the
//       identity.  THIS MATTERS: the identity is what an i64/u64 model reaches
//       for by reflex, and it is wrong the moment a site truncates (the probe's
//       `APInt(32, 0x1FFFFFFFFull, false, true)` case, which the identity
//       reports as 8589934591 instead of 4294967295).
//     * the copy constructor (:176), which copies BitWidth and the word and so
//       changes no value.
//     * `==` / `!=` against another APInt (:1057, :1088).  LLVM asserts EQUAL
//       BIT WIDTHS and then compares the word: `return U.VAL == RHS.U.VAL`.
//       Comparing the modelled words is that same function.  A comparison
//       cannot create a value, so it cannot leave the fast path.
//     * `==` / `!=` against a uint64_t (:1070, :1096), which for a single word
//       is `getZExtValue() == Val`.
//     * `getZExtValue()` (:1541), which for a single word IS `U.VAL`.
//
//   NOT MAPPED, deliberately LOUD:
//     * `getBitWidth()` and every width query.  The model does not store a
//       width and there is no honest value to return, so this is the one that
//       must abort rather than guess.  Anything derived from it --
//       `getActiveBits`, `getSignificantBits`, `getMinSignedBits` -- goes with
//       it.
//     * ALL ARITHMETIC: + - * / %, the compound assignments, ++/--, the shifts,
//       `abs`, `sdiv`/`udiv`, `srem`/`urem`.  Every one of these wraps at the
//       declared bit width -- `operator+=` ends in `clearUnusedBits()` -- and a
//       width-less u64 model wraps at 64 instead.  That divergence is invisible
//       on small values and silently wrong at the boundary, which is precisely
//       the failure a probe is least likely to stumble into by accident, so
//       these stay an abort.  (The probe below nevertheless exercises a wrap AT
//       THE DECLARED WIDTH through the constructor, because that is the one
//       width-sensitive body this module DOES emit.)
//     * `trunc` / `zext` / `sext` / `zextOrTrunc` and the width-changing family:
//       these are width transformations, so they need the width the model
//       dropped.
//     * The ORDERINGS `<` `<=` `>` `>=` and `ult`/`slt`/`ugt`/`sgt`.  These do
//       not create a value, but the SIGNED ones read the sign bit AT THE
//       DECLARED WIDTH -- `slt` on a 32-bit APInt asks about bit 31, not bit 63
//       -- so they need the width too.  The unsigned ones would be safe; they
//       are left out with the signed ones rather than mapped alone, because a
//       half-mapped comparison surface is how a signed site silently picks up
//       unsigned semantics.  Say NO to the whole family loudly instead.
//     * `APInt(unsigned, ArrayRef<uint64_t>)` (:148) and the string constructor
//       (:171), which are the MULTI-WORD entry points -- the slow path this
//       model has no representation for at all.
//     * The default constructor `APInt()` (:173) is a 1-BIT zero, so its value
//       is honest (0) but its width is 1 and everything downstream of it is a
//       width question.  Left out with the rest.
//
// WHAT CLOSED THE MEASURED ROW.  The gap this module was written for is
// `cpp_type: llvm::APInt` in Transform__Dataflow__MutableAddrSplitting.cpp, and
// the verbose log says it is a TYPE-MAPPING assertion, not an expression one:
//
//     search type std::optional<llvm::APInt>, result: Option<Value<T1>>
//     cpp_type: llvm::APInt  (key='llvm::APInt')
//     mapper.cpp:1185 ... Assertion `0 && "Type is not present in types_"'
//
// -- a `std::optional<llvm::APInt> const_step` declaration whose ELEMENT type
// has no mapping.  So the type entry `t1` is what that row needed; the handful
// of expressions above are the ones that can be honoured at the same time
// without touching the width, and they are here so that a site which merely
// receives, copies and compares such a value does not trip the next assertion
// one line later.
//
// == AND != ARE MEMBERS here, exactly as in rules/dynamicapint: :1057/:1088 and
// :1070/:1096 all declare `bool operator==(...) const` INSIDE the class, and
// none of the four is a hidden friend.  They are therefore reached by the
// MEMBER spelling `a.operator==(b)` below -- the free spelling that
// rules/dynamicapint needs for its int64_t mixed comparisons does not apply,
// because APInt's uint64_t comparison is a member too.  (`inline APInt
// operator-(APInt)` at :41 IS a namespace-scope free function, but it is
// arithmetic and so not mapped.)

namespace llvm {

// Restated from llvm/ADT/APInt.h:78.  The union and BitWidth members are
// irrelevant to signature matching and cannot be restated faithfully anyway, so
// the class carries the public surface this module maps and nothing else.  It
// must be COMPLETE because every signature below takes or returns one.
class APInt {
public:
  // llvm/ADT/APInt.h:111 -- APInt(unsigned numBits, uint64_t val,
  //                              bool isSigned = false,
  //                              bool implicitTrunc = false)
  // uint64_t is `unsigned long` on the x86-64 Linux target; spell it the way
  // the target spells it, the same rule rules/stringref records for size_t.
  APInt(unsigned numBits, unsigned long val, bool isSigned = false,
        bool implicitTrunc = false);

  // llvm/ADT/APInt.h:176 -- APInt(const APInt &that)
  APInt(const APInt &that);

  // llvm/ADT/APInt.h:1057 -- bool operator==(const APInt &RHS) const
  bool operator==(const APInt &RHS) const;
  // llvm/ADT/APInt.h:1088 -- bool operator!=(const APInt &RHS) const
  bool operator!=(const APInt &RHS) const;
  // llvm/ADT/APInt.h:1070 -- bool operator==(uint64_t Val) const
  bool operator==(unsigned long Val) const;
  // llvm/ADT/APInt.h:1096 -- bool operator!=(uint64_t Val) const
  bool operator!=(unsigned long Val) const;

  // llvm/ADT/APInt.h:1541 -- uint64_t getZExtValue() const
  unsigned long getZExtValue() const;
};

} // namespace llvm

using t1 = llvm::APInt;

// The constructor.  NOT the identity -- see the note above on why the mask is
// the exact body and the identity is the wrong reflex.
llvm::APInt f1(unsigned numBits, unsigned long val, bool isSigned,
               bool implicitTrunc) {
  return llvm::APInt(numBits, val, isSigned, implicitTrunc);
}

llvm::APInt f2(const llvm::APInt &that) { return llvm::APInt(that); }

bool f3(const llvm::APInt &a, const llvm::APInt &b) {
  return a.operator==(b);
}

bool f4(const llvm::APInt &a, const llvm::APInt &b) {
  return a.operator!=(b);
}

bool f5(const llvm::APInt &a, unsigned long b) { return a.operator==(b); }

bool f6(const llvm::APInt &a, unsigned long b) { return a.operator!=(b); }

unsigned long f7(const llvm::APInt &a) { return a.getZExtValue(); }
