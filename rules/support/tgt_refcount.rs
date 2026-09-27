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
    Some(Rc::new(RefCell::new(T1::clone(a0))))
}

fn f20<T1: Clone>(a0: &Option<Value<T1>>) -> Option<Value<T1>> {
    a0.as_ref().map(|v| {
        Rc::new(RefCell::new(T1::clone(&*v.borrow())))
    })
}

fn f21<T1>(a0: &Option<Value<T1>>) -> bool {
    a0.is_some()
}
