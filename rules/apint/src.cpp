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
// nobody has checked, so they are left out.
//
// ⛔ CORRECTED 2026-09-28: THE OLD TEXT HERE SAID THE OMITTED KEYS "WILL ABORT
// LOUDLY".  THAT IS FALSE FOR ANY OF THEM THAT IS A MEMBER, AND THE CORRECTION
// IS MEASURED, NOT INFERRED.
// An unmapped MEMBER does not abort: the converter emits the call TEXTUALLY
// against the receiver's target type, so `<recv>.unmappedMethod(...)` reaches
// the emitted Rust as a call to a method that exists nowhere -- at rc=0, with
// no placeholder token, so `pin/no-placeholders.sh` stays at 0.  Loud at rustc
// time, silent at translate time.  Only the CONSTRUCTOR fabricates visibly, as
// `<mangled>::new_<N>`, which is why f1 below exists and is correct as written.
// So of the omissions listed above: the ctor entry really is loud; every member
// entry is SILENT and this comment must not be read as a safety argument.
//
// ⚠️ AND THE DIRECTLY MEASURED MISSES ARE ON A TYPE THIS MODULE DOES NOT MODEL.
// `llvm::APInt` and `llvm::DynamicAPInt` are DIFFERENT LLVM CLASSES.  This
// module models only `DynamicAPInt`; `grep -rn 'llvm::APInt' rules/*/src.cpp`
// over the whole rule tree returns ZERO -- `llvm::APInt` has no type key
// anywhere, so it lowers to `Cpp2RustUnmapped_llvm_APInt`.  Measured on
// `dataflow-scheduler/external/ktir-mlir-frontend/lib/Ktdp/KtdpDialect.cpp`
// (bucket A, rc=0), emitted line 3113-3142:
//   `Cpp2RustUnmapped_llvm_APInt`  3 uses,  0 definitions   (E0412)
//   `llvm_APInt::new_1(...)`       3 calls, 0 impls         (E0433, the LOUD half)
//   `.getValue()`                  3 calls, 0 `fn getValue`
//   `.getZExtValue()`              1 call,  0 `fn getZExtValue`  (SILENT)
// A census naming `rules/apint` as the owner of those misses is naming the
// right module by name and the wrong type by semantics.
//
// ⛔ REFUSED: DO NOT ADD AN `llvm::APInt` TYPE KEY MODELLED AS A FIXED-WIDTH
// INTEGER, AND THEREFORE DO NOT ADD `getZExtValue`/`eq` AS MEMBERS OF ONE.
// The i64 model above is honest for `DynamicAPInt` because that class holds an
// inline int64_t whenever the value fits and arbitrary precision is its
// OVERFLOW path.  `APInt` is the opposite: its bit width is an EXPLICIT,
// per-object field that its members' results are computed FROM, so a model that
// does not carry the width answers some members with a wrong value rather than
// with a failure.
//   ⭐ THE OBSERVER, IN THIS CORPUS, NOT HYPOTHETICAL:
//   dcc/src/Dialect/Sentient/SentientOps.cpp:1074,1080,1081 --
//     `step.getValue().getSExtValue()`, `ub.getValue().getSExtValue() -
//      lb.getValue().getSExtValue()`.
//   `IntegerAttr::getValue()` returns an `llvm::APInt`, and `getSExtValue()`
//   SIGN-EXTENDS FROM BitWidth.  For a 32-bit APInt holding 0xFFFFFFFF the C++
//   answer is -1; a u64/i64 model holding the same bits answers +4294967295.
//   That RUNS and gives a wrong loop bound -- the silent-wrongness class, which
//   is worse than a loud failure.  `getBitWidth()` cannot be answered at all
//   from such a model, and C++ `APInt::operator==` ASSERTS equal bit widths
//   where a plain integer compare would silently succeed across widths.
// A correct `APInt` model must carry `{ bit_width, value }` as a struct (or a
// real bignum); only then are `getZExtValue`, `getSExtValue`, `getBitWidth` and
// `eq` writable.  Until that model exists these members are left out, and the
// omission is SILENT, not loud -- see the correction above.

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

// Restated from llvm/ADT/APInt.h.  A DIFFERENT CLASS from DynamicAPInt above,
// and modelled differently: see the `{ bit_width, value }` argument below.
class APInt {
  unsigned BitWidth;
  uint64_t VAL;

public:
  // llvm/ADT/APInt.h:111 -- APInt(unsigned numBits, uint64_t val,
  //                               bool isSigned = false,
  //                               bool implicitTrunc = false)
  // ⚠️ ONLY ONE CTOR ENTRY IS PERMITTED FOR THIS FAMILY.  Defaulted arguments are
  // written out at the C++ call site, so the 2-, 3- and 4-argument spellings all
  // record as this same 4-argument key; a second entry for the 2-argument form
  // would be a dead duplicate of this one.
  APInt(unsigned numBits, uint64_t val, bool isSigned = false,
        bool implicitTrunc = false);
  // llvm/ADT/APInt.h:1489
  unsigned getBitWidth() const;
  // llvm/ADT/APInt.h:1541 -- `return U.VAL;`
  uint64_t getZExtValue() const;
  // llvm/ADT/APInt.h:1563 -- `return SignExtend64(U.VAL, BitWidth);`
  int64_t getSExtValue() const;
  // llvm/ADT/APInt.h:1080 -- `bool eq(const APInt &RHS) const { return (*this) == RHS; }`
  bool eq(const APInt &RHS) const;
};

} // namespace llvm

using t1 = llvm::DynamicAPInt;

// ---------------------------------------------------------------------------
// t2/f5-f9 -- `llvm::APInt`, ADDED 2026-09-28.  THIS SUPERSEDES THE REFUSAL
// RECORDED ABOVE, AND ONLY BECAUSE THE THING THE REFUSAL DEMANDED NOW EXISTS.
// ---------------------------------------------------------------------------
// The refusal above says, verbatim, "A correct `APInt` model must carry
// `{ bit_width, value }` as a struct (or a real bignum); only then are
// `getZExtValue`, `getSExtValue`, `getBitWidth` and `eq` writable."  That struct
// is now `libcc2rs::APInt` (libcc2rs/src/apint.rs), which carries `bit_width: u32`
// and `value: u64`, maintains LLVM's unused-bits-clear invariant on store, and
// sign-extends FROM `bit_width` in `get_sext_value`.  So the condition the refusal
// set is met, and the members it withheld are written below.
//
// ⛔ WHAT MUST NEVER BE DONE, AND WHY THESE KEYS LAND AS ONE BLOCK:
// A type key for `llvm::APInt` WITHOUT these members is strictly WORSE than no
// key at all.  Today `llvm::APInt` has no key, so it lowers to
// `Cpp2RustUnmapped_llvm_APInt` and every use is a LOUD E0412.  Add the type key
// alone and that name disappears -- the receiver resolves, and
// `.getSExtValue()` is then emitted TEXTUALLY against a real Rust type at rc=0
// with no placeholder token: silent at translate time, and the exact class this
// row exists to remove.  The type key and its members are therefore a single
// atomic change; never land t2 without f5-f9.
//
// WHY A STRUCT AND NOT A TUPLE (the only composite precedent in the rule tree):
// all five keys below are calls on an `APInt` RECEIVER, and a method call whose
// receiver type contains a tuple is not resolved by the rule preprocessor
// (`rules/mlir/tgt_unsafe.rs:362-366`; corroborated here by
// `grep -rn 'Tuple\|tuple' rule-preprocessor/src/*.rs` = ZERO hits, i.e. the
// preprocessor has no tuple-type concept at all).  A named struct in libcc2rs has
// the standing precedent of `libcc2rs::InFlightDiagnostic` (rules/mlir t70/f21),
// which is a libcc2rs struct used as a type key with members in receiver form.
//
// ⚠️ THE ONE THING THIS MODEL CANNOT ANSWER: a `BitWidth` above 64.  LLVM keeps
// those in `U.pVal[]` and a `u64` cannot hold them.  `bit_width` is a RUNTIME
// expression at the call site so no translate-time refusal is possible; the ctor
// PANICS instead.  That is not `todo!()` -- it is a reachable, correct check on an
// unrepresentable value, and it is stricter than release C++, never laxer.
//
// ⚠️ ALSO NOT ANSWERED, deliberately: every arithmetic/bitwise operator, the
// ordering comparisons, `operator!=`, `trunc`/`sext`/`zext`, `getActiveBits`,
// `isNegative`, and `toString`.  Each is short against these two fields, but an
// unmapped MEMBER is SILENT (see the correction above), so they go in when a site
// appears -- not on speculation.
//
// ⭐ PAIRED ROW, NOT OWNED HERE: `llvm::APInt mlir::IntegerAttr::getValue() const`
// (16 asks, the largest single count) is a `rules/mlir` receiver.  Its RETURN type
// is `llvm::APInt`, so it was blocked on this model and is now unblocked by it;
// it is the front half of the observer expression
// `step.getValue().getSExtValue()`.  Until rules/mlir keys it, `getValue()` is
// still emitted textually and the seam stays open on that side.
using t2 = llvm::APInt;

llvm::APInt f5(unsigned numBits, uint64_t val, bool isSigned,
               bool implicitTrunc) {
  return llvm::APInt(numBits, val, isSigned, implicitTrunc);
}

unsigned f6(const llvm::APInt &x) { return x.getBitWidth(); }

uint64_t f7(const llvm::APInt &x) { return x.getZExtValue(); }

int64_t f8(const llvm::APInt &x) { return x.getSExtValue(); }

bool f9(const llvm::APInt &a, const llvm::APInt &b) { return a.eq(b); }

// The default constructor.  Present because a type rule maps the TYPE ONLY: the
// converter looks the default ctor up as an ordinary expr rule and, on a miss,
// emits `<mangled>::new()`, which does not exist -- rc=0 then E0433.
llvm::DynamicAPInt f1() { return llvm::DynamicAPInt(); }

llvm::DynamicAPInt f2(int64_t v) { return llvm::DynamicAPInt(v); }

int64_t f3(const llvm::DynamicAPInt &x) { return x.operator int64_t(); }

bool f4(const llvm::DynamicAPInt &a, int64_t b) { return operator==(a, b); }
