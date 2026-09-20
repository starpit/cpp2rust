// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// std::bitset<N> -> Vec<bool>, LSB first, lazily materialized.
// See rules/bitset/src.cpp for the full rationale and the known limitations.
//
// NOTE on the `.as_slice()` calls below: in the unsafe model a by-value
// receiver is substituted as `(*p)` for a raw pointer `p`, so a bare
// `a0.get(i)` / `a0.iter()` expands to `(*p).get(i)`, which autorefs THROUGH
// Vec's Deref to `&[bool]` and trips rustc's deny-by-default
// `dangerous_implicit_autorefs` lint.  Going via `as_slice()` -- an inherent
// Vec method -- keeps the implicit borrow at `&Vec<bool>`, which the lint
// allows.  Do not "simplify" them away.

use libcc2rs::*;

fn t1() -> Vec<bool> {
    Vec::new()
}

// --- construction ----------------------------------------------------------

unsafe fn f1() -> Vec<bool> {
    Vec::new()
}

unsafe fn f2(a0: u64) -> Vec<bool> {
    (0..64).map(|i| (a0 >> i) & 1 == 1).collect()
}

unsafe fn f3(a0: Vec<bool>) -> Vec<bool> {
    a0.clone()
}

unsafe fn f4(a0: &mut Vec<bool>, a1: Vec<bool>) {
    *a0 = a1.clone()
}

// --- element access --------------------------------------------------------

unsafe fn f5(a0: Vec<bool>, a1: usize) -> bool {
    a0.as_slice().get(a1).copied().unwrap_or(false)
}

unsafe fn f6(a0: Vec<bool>, a1: usize) -> bool {
    a0.as_slice().get(a1).copied().unwrap_or(false)
}

// --- mutation --------------------------------------------------------------

unsafe fn f7(a0: &mut Vec<bool>, a1: usize, a2: bool) {
    if a0.len() <= a1 {
        a0.resize(a1 + 1, false);
    }
    a0.as_mut_slice()[a1] = a2;
}

unsafe fn f8(a0: &mut Vec<bool>, a1: usize) {
    if a0.len() <= a1 {
        a0.resize(a1 + 1, false);
    }
    a0.as_mut_slice()[a1] = false;
}

unsafe fn f9(a0: &mut Vec<bool>) {
    a0.clear()
}

unsafe fn f10(a0: &mut Vec<bool>, a1: usize) {
    if a0.len() <= a1 {
        a0.resize(a1 + 1, false);
    }
    let __s = a0.as_mut_slice();
    __s[a1] = !__s[a1];
}

// --- observers -------------------------------------------------------------

unsafe fn f11(a0: Vec<bool>) -> usize {
    a0.as_slice().iter().filter(|b| **b).count()
}

// N-dependent, see src.cpp: this is the materialized length, not N.
unsafe fn f12(a0: Vec<bool>) -> usize {
    a0.len()
}

unsafe fn f13(a0: Vec<bool>) -> bool {
    a0.as_slice().iter().any(|b| *b)
}

unsafe fn f14(a0: Vec<bool>) -> bool {
    !a0.as_slice().iter().any(|b| *b)
}

// N-dependent, see src.cpp: true on an empty (default-constructed) bitset.
unsafe fn f15(a0: Vec<bool>) -> bool {
    a0.as_slice().iter().all(|b| *b)
}

// --- conversion ------------------------------------------------------------
// take(64) keeps the shift in range; C++ would throw std::overflow_error when
// the value does not fit, which is not modelled.

unsafe fn f16(a0: Vec<bool>) -> u64 {
    a0.as_slice()
        .iter()
        .take(64)
        .enumerate()
        .fold(0u64, |acc, (i, b)| acc | ((*b as u64) << i))
}

unsafe fn f17(a0: Vec<bool>) -> u64 {
    a0.as_slice()
        .iter()
        .take(64)
        .enumerate()
        .fold(0u64, |acc, (i, b)| acc | ((*b as u64) << i))
}

// --- comparison ------------------------------------------------------------
// Zero-extend the shorter side: bits past the materialized end are false.

unsafe fn f18(a0: Vec<bool>, a1: Vec<bool>) -> bool {
    (0..a0.len().max(a1.len()))
        .all(|i| a0.as_slice().get(i).copied().unwrap_or(false) == a1.as_slice().get(i).copied().unwrap_or(false))
}

unsafe fn f19(a0: Vec<bool>, a1: Vec<bool>) -> bool {
    (0..a0.len().max(a1.len()))
        .any(|i| a0.as_slice().get(i).copied().unwrap_or(false) != a1.as_slice().get(i).copied().unwrap_or(false))
}

// --- bitwise assignment ----------------------------------------------------
// `a1` is bound once: the placeholder is raw text, so naming it twice would
// re-evaluate the operand expression.

unsafe fn f20(a0: &mut Vec<bool>, a1: Vec<bool>) {
    let __rhs = a1.clone();
    if a0.len() < __rhs.len() {
        a0.resize(__rhs.len(), false);
    }
    for (__d, __s) in a0.as_mut_slice().iter_mut().zip(__rhs.as_slice().iter()) {
        *__d |= *__s;
    }
}
