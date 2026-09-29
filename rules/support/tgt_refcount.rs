// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model.  llvm::LogicalResult is its single `bool
// IsSuccess` field and llvm::hash_code is its single `size_t value` field, so
// every body for those two is arithmetic on a scalar and nothing is
// model-dependent.  The FailureOr block at the bottom DOES diverge between the
// two files -- see there.

fn t1() -> bool {
    false
}

fn t2() -> u64 {
    0
}

unsafe fn f1(a0: bool) -> bool {
    a0
}

unsafe fn f2(a0: bool) -> bool {
    !a0
}

unsafe fn f3(a0: bool) -> bool {
    a0
}

unsafe fn f4(a0: bool) -> bool {
    !a0
}

unsafe fn f5(a0: bool) -> bool {
    a0
}

unsafe fn f6(a0: bool) -> bool {
    !a0
}

unsafe fn f7(a0: bool) -> bool {
    a0
}

unsafe fn f8(a0: bool) -> bool {
    !a0
}

unsafe fn f9(a0: bool) -> bool {
    a0
}

unsafe fn f10(a0: u64) -> u64 {
    a0
}

unsafe fn f11(a0: u64) -> u64 {
    a0
}

unsafe fn f12(a0: u64, a1: u64) -> bool {
    a0 == a1
}

unsafe fn f13(a0: u64, a1: u64) -> bool {
    a0 != a1
}

unsafe fn f14(a0: u64) -> u64 {
    a0
}

unsafe fn f15(a0: u64) -> u64 {
    a0
}

// --- llvm::FailureOr<T1> -> Option<Value<T1>> ------------------------------
// DIVERGES FROM tgt_unsafe.rs ON PURPOSE.  The std::optional base is dropped
// (see src.cpp), so FailureOr's readers are its own and must follow the same
// refcount convention rules/optional uses for std::optional: the payload is a
// PLACE, so `operator*` / `operator->` / `value()` return a `Ptr<T1>` that
// aliases the stored object rather than a Rust reference into an Option.
// Everything above this line is still byte-identical to tgt_unsafe.rs: those
// keys are scalars (`bool`, `u64`).

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn t3<T1>() -> Option<Value<T1>> {
    None
}

fn f16<T1>() -> Option<Value<T1>> {
    None
}

fn f17<T1>(a0: bool) -> Option<Value<T1>> {
    None
}

fn f18<T1>(a0: T1) -> Option<Value<T1>> {
    Some(Rc::new(RefCell::new(a0)))
}

fn f19<T1: Clone>(a0: &T1) -> Option<Value<T1>> {
    Some(Rc::new(RefCell::new(<T1>::clone(a0))))
}

fn f20<T1: Clone>(a0: &Option<Value<T1>>) -> Option<Value<T1>> {
    a0.as_ref().map(|v| {
        Rc::new(RefCell::new(<T1>::clone(&*v.borrow())))
    })
}

fn f21<T1>(a0: &Option<Value<T1>>) -> bool {
    a0.is_some()
}

// See src.cpp.  `Ptr<u8>::to_rust_string` (libcc2rs/src/cstr.rs:167) walks to
// the NUL, which is the refcount counterpart of CStr::from_ptr in tgt_unsafe.rs.
fn f22(a0: Ptr<u8>, a1: Ptr<u8>, a2: u32) {
    panic!(
        "llvm_unreachable: {} at {}:{}",
        a0.to_rust_string(),
        a1.to_rust_string(),
        a2
    )
}

// t4 = llvm::ParseResult.  It has no state of its own -- its single data member
// is LogicalResult's inherited `bool IsSuccess` -- so it is the same scalar as
// t1 in both models.
fn t4() -> bool {
    false
}

// The converting ParseResult(LogicalResult) ctor: identity on the scalar.
unsafe fn f23(a0: bool) -> bool {
    a0
}

// ⭐ INVERTED ON PURPOSE.  `ParseResult::operator bool()` returns `failed()`,
// not `succeeded()`.  `a0` here would compile and would invert every
// `if (parser.parseX())` in the corpus.
unsafe fn f24(a0: bool) -> bool {
    !a0
}

// t5 = llvm::SMLoc -> Ptr<u8>.  Same opaque pointer-shaped handle as
// tgt_unsafe.rs's `*const u8`, in this model's pointer type (the f22 pairing).
// Nothing reads through it, so the null handle is never borrowed.
fn t5() -> Ptr<u8> {
    Ptr::null()
}

// `SMLoc() = default` leaves the `const char *Ptr = nullptr` NSDMI alone.
fn f25() -> Ptr<u8> {
    Ptr::null()
}

// f26/f27/f28 -- the `To == From` slice of llvm::cast / dyn_cast /
// dyn_cast_or_null.  See src.cpp for why `a0.clone()` is the EXACT body of all
// three and not an approximation of a downcast, and for the three things
// deliberately left unkeyed.  Identical to tgt_unsafe.rs: a `const T1 &`
// parameter is `&T1` in both models and a by-value `T1` return stays `T1`.
fn f26<T1: Clone>(a0: &T1) -> T1 {
    a0.clone()
}

fn f27<T1: Clone>(a0: &T1) -> T1 {
    a0.clone()
}

fn f28<T1: Clone>(a0: &T1) -> T1 {
    a0.clone()
}

// f29/f30/f31 -- the SAME `To == From` slice reached through the NON-CONST
// lvalue overload (Casting.h:565/:571/:577).  ⭐ A non-const `T1 &` parameter
// does NOT force `Ptr<T1>`: the target parameter type is declared here and the
// key string comes from the resolved C++ overload in src.cpp, independently.
// `&T1` is what this needs and it is what both models get, so the two overlays
// are byte-identical, exactly as for f26/f27/f28.
fn f29<T1: Clone>(a0: &T1) -> T1 {
    a0.clone()
}

fn f30<T1: Clone>(a0: &T1) -> T1 {
    a0.clone()
}

fn f31<T1: Clone>(a0: &T1) -> T1 {
    a0.clone()
}
