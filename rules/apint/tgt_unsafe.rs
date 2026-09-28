// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model and for the corpus enumeration that justifies it.
// A DynamicAPInt is its value; every corpus site is small-integer, so the type
// IS i64 and all four bodies are the identity or a plain comparison.  There is
// no tgt_refcount.rs: an i64 is value-like, no body here contains raw-pointer
// text or `as_pointer()`, and placeholder access modes are expanded
// model-awarely, so the unsafe base layer is correct under refcount too.

fn t1() -> i64 {
    0
}

unsafe fn f1() -> i64 {
    0
}

unsafe fn f2(a0: i64) -> i64 {
    a0
}

unsafe fn f3(a0: i64) -> i64 {
    a0
}

unsafe fn f4(a0: i64, a1: i64) -> bool {
    a0 == a1
}

// ---------------------------------------------------------------------------
// t2/f5-f9 -- `llvm::APInt`, A DIFFERENT CLASS FROM t1's `llvm::DynamicAPInt`.
// src.cpp carries the full argument; in one paragraph: `APInt`'s bit width is an
// explicit per-object field that its members' results are computed FROM, so it is
// modelled by the named struct `libcc2rs::APInt { bit_width: u32, value: u64 }`
// (libcc2rs/src/apint.rs) and NOT by any width-less integer.  The type key and
// these five members are ATOMIC: t2 without f5-f9 would turn a LOUD
// `Cpp2RustUnmapped_llvm_APInt` (E0412) into a resolved type on which
// `.getSExtValue()` is emitted textually at rc=0 with no placeholder token.
// ---------------------------------------------------------------------------

// THE INIT is a ONE-BIT ZERO, not a 64-bit zero: `APInt.h:1943` gives `BitWidth`
// the in-class initialiser `1`, so that is what a default-constructed
// `llvm::APInt` is.
fn t2() -> libcc2rs::APInt {
    libcc2rs::APInt::default()
}

// `APInt(unsigned numBits, uint64_t val, bool isSigned, bool implicitTrunc)`.
// The two flags are passed through and then not branched on -- APInt.h:111-132
// says they select only WHICH DEBUG ASSERTION guards the store, never the stored
// value, which is always `val` masked to `numBits`.  PANICS above 64 bits, the
// one input this model cannot represent.
unsafe fn f5(a0: u32, a1: u64, a2: bool, a3: bool) -> libcc2rs::APInt {
    libcc2rs::APInt::new(a0, a1, a2, a3)
}

unsafe fn f6(a0: &libcc2rs::APInt) -> u32 {
    a0.get_bit_width()
}

// `getZExtValue()` is a plain field read because the unused-bits-clear invariant
// is maintained on store, exactly as `APInt.h:1541` (`return U.VAL;`) relies on.
unsafe fn f7(a0: &libcc2rs::APInt) -> u64 {
    a0.get_zext_value()
}

// ⭐ THE ROW THAT MOTIVATES THE WHOLE STRUCT: a REAL sign-extension from
// `bit_width`, not a cast.  A 32-bit APInt holding 0xFFFFFFFF answers -1 here and
// would answer +4294967295 from a width-less model -- which RUNS, and which is a
// loop bound at dcc/src/Dialect/Sentient/SentientOps.cpp:1074,1080,1081.
unsafe fn f8(a0: &libcc2rs::APInt) -> i64 {
    a0.get_sext_value()
}

// `eq` delegates to `operator==`, which ASSERTS equal bit widths; a bare value
// compare across widths would be the bug, so the assertion is reproduced.
unsafe fn f9(a0: &libcc2rs::APInt, a1: &libcc2rs::APInt) -> bool {
    a0.eq(a1)
}
