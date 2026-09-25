// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// llvm::BitVector -> Vec<bool>, index i == bit i.
// See rules/bitvector/src.cpp for the representation argument and for what is
// deliberately left unmapped.
//
// NOTE on the `.as_slice()` calls, copied from rules/bitset for the same
// reason: in the unsafe model a by-value receiver is substituted as `(*p)` for
// a raw pointer `p`, so a bare `a0.get(i)` expands to `(*p).get(i)`, which
// autorefs THROUGH Vec's Deref to `&[bool]` and trips rustc's deny-by-default
// `dangerous_implicit_autorefs` lint.  Going via `as_slice()` -- an inherent
// Vec method -- keeps the implicit borrow at `&Vec<bool>`.  Do not "simplify"
// them away.

use libcc2rs::*;

fn t1() -> Vec<bool> {
    Vec::new()
}

// --- construction ----------------------------------------------------------

unsafe fn f1() -> Vec<bool> {
    Vec::new()
}

unsafe fn f2(a0: u32, a1: bool) -> Vec<bool> {
    vec![a1; a0 as usize]
}

unsafe fn f3(a0: Vec<bool>) -> Vec<bool> {
    a0.clone()
}

unsafe fn f4(a0: &mut Vec<bool>, a1: Vec<bool>) {
    *a0 = a1.clone()
}

// --- observers -------------------------------------------------------------

unsafe fn f5(a0: Vec<bool>) -> bool {
    a0.is_empty()
}

unsafe fn f6(a0: Vec<bool>) -> usize {
    a0.len()
}

// POPCOUNT.
unsafe fn f7(a0: Vec<bool>) -> usize {
    a0.as_slice().iter().filter(|b| **b).count()
}

// Empty: false, matching C++'s any_of over zero words.
unsafe fn f8(a0: Vec<bool>) -> bool {
    a0.as_slice().iter().any(|b| *b)
}

// Empty: true, matching BitVector.h:194-206 falling through to `return true`.
unsafe fn f9(a0: Vec<bool>) -> bool {
    a0.as_slice().iter().all(|b| *b)
}

// Empty: true.
unsafe fn f10(a0: Vec<bool>) -> bool {
    !a0.as_slice().iter().any(|b| *b)
}

// C++ asserts Idx < Size; indexing directly panics instead of growing.
unsafe fn f11(a0: Vec<bool>, a1: u32) -> bool {
    a0.as_slice()[a1 as usize]
}

unsafe fn f12(a0: Vec<bool>, a1: u32) -> bool {
    a0.as_slice()[a1 as usize]
}

// --- search ----------------------------------------------------------------
// -1 when there is no such bit, as C++ does.

unsafe fn f13(a0: Vec<bool>) -> i32 {
    match a0.as_slice().iter().position(|b| *b) {
        Some(__i) => __i as i32,
        None => -1,
    }
}

// find_next(Prev) is find_first_in(Prev + 1, Size): the search starts AFTER
// Prev, and the returned index is absolute.
// The traversal is ONE expression on purpose: in the refcount model a
// `const BitVector &` receiver is substituted as `(*a.borrow())`, so binding
// `let __s = a0.as_slice();` and using it in a later statement is E0716
// (the Ref temporary dies at the end of its own statement).  skip() past the
// end yields an empty iterator, which gives the -1 C++ gives.
unsafe fn f14(a0: Vec<bool>, a1: u32) -> i32 {
    let __from = a1 as usize + 1;
    match a0.as_slice().iter().skip(__from).position(|b| *b) {
        Some(__i) => (__from + __i) as i32,
        None => -1,
    }
}

// --- mutation --------------------------------------------------------------

unsafe fn f15(a0: &mut Vec<bool>) {
    a0.clear()
}

// Grows filling with a1, truncates when shrinking: Vec::resize exactly.
unsafe fn f16(a0: &mut Vec<bool>, a1: u32, a2: bool) {
    a0.resize(a1 as usize, a2)
}

// NO ARGUMENT: ALL bits.
unsafe fn f17(a0: &mut Vec<bool>) {
    a0.as_mut_slice().fill(true)
}

// One bit.
unsafe fn f18(a0: &mut Vec<bool>, a1: u32) {
    a0.as_mut_slice()[a1 as usize] = true
}

unsafe fn f19(a0: &mut Vec<bool>) {
    a0.as_mut_slice().fill(false)
}

unsafe fn f20(a0: &mut Vec<bool>, a1: u32) {
    a0.as_mut_slice()[a1 as usize] = false
}

unsafe fn f21(a0: &mut Vec<bool>) {
    for __b in a0.as_mut_slice().iter_mut() {
        *__b = !*__b;
    }
}

unsafe fn f22(a0: &mut Vec<bool>, a1: u32) {
    let __s = a0.as_mut_slice();
    __s[a1 as usize] = !__s[a1 as usize];
}

unsafe fn f23(a0: &mut Vec<bool>, a1: bool) {
    a0.push(a1)
}

// --- comparison ------------------------------------------------------------
// BitVector::operator== compares Size first (BitVector.h:518), so vectors of
// different length are NEVER equal -- no zero-extension here, unlike
// rules/bitset where the length is only how much has been materialized.

unsafe fn f24(a0: Vec<bool>, a1: Vec<bool>) -> bool {
    a0.as_slice() == a1.as_slice()
}

unsafe fn f25(a0: Vec<bool>, a1: Vec<bool>) -> bool {
    a0.as_slice() != a1.as_slice()
}
