// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model, for the header lines every signature was copied
// from, and -- most importantly -- for the long list of what is deliberately
// NOT here (getBitWidth, all arithmetic, all width changes, all orderings).
// An APInt is modelled as its fast-path word, u64; no width is stored.

fn t1() -> u64 {
    0
}

// APInt(numBits, val, isSigned, implicitTrunc).  The MASK, not the identity:
// the constructor ends in clearUnusedBits(), which ANDs the stored word with
// the low-numBits mask, and that mask is the correct word for all four
// combinations of the two bools (sign extension only writes bits at or above
// numBits, which the mask discards).  So a0 is USED here even though the model
// keeps no width -- it is used once, at construction, and then dropped.
// One expression, because a rule body is inlined into its caller.
unsafe fn f1(a0: u32, a1: u64, a2: bool, a3: bool) -> u64 {
    (if a0 >= 64 { a1 } else { a1 & ((1u64 << a0) - 1) })
}

// Copy constructor: copies BitWidth and the word, so it changes no value.
unsafe fn f2(a0: u64) -> u64 {
    a0
}

// LLVM asserts equal bit widths and then compares the word, so comparing the
// modelled words is the same function.
unsafe fn f3(a0: u64, a1: u64) -> bool {
    a0 == a1
}

unsafe fn f4(a0: u64, a1: u64) -> bool {
    a0 != a1
}

unsafe fn f5(a0: u64, a1: u64) -> bool {
    a0 == a1
}

unsafe fn f6(a0: u64, a1: u64) -> bool {
    a0 != a1
}

// Single word: getZExtValue() IS U.VAL.
unsafe fn f7(a0: u64) -> u64 {
    a0
}
