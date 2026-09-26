// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn t1<T1>() -> Option<Value<T1>> {
    None
}

fn t2<T1>() -> Option<Value<Box<[T1]>>> {
    None
}

fn f1<T1: Default>(a0: usize) -> Option<Value<Box<[T1]>>> {
    Some(Rc::new(RefCell::new(
        (0..a0).map(|_| <T1>::default()).collect::<Box<[_]>>(),
    )))
}

fn f2<T1>(a0: Value<T1>) -> Ptr<T1> {
    a0.as_pointer()
}

fn f3<T1>(a0: Ptr<T1>) -> Option<Value<T1>> {
    a0.to_owned_opt()
}

fn f4<T1>(a0: Ptr<T1>) -> Option<Value<T1>> {
    a0.to_owned_opt()
}

fn f5<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: Ptr<T1>) {
    let _p: Ptr<T1> = a1;
    a0.write(_p.to_owned_opt())
}

fn f6<T1>(a0: &mut Option<Value<Box<[T1]>>>, a1: Ptr<T1>) {
    *a0 = a1.to_owned_opt()
}

fn f7<T1>(a0: Value<T1>) -> Ptr<T1> {
    a0.as_pointer()
}

fn f8<T1>(init: T1) -> Option<Value<T1>> {
    Some(Rc::new(RefCell::new(init)))
}

fn f9<T1>(a0: &mut Option<Value<Box<[T1]>>>) {
    *a0 = None
}

fn f10<T1>() -> Option<Value<T1>> {
    None
}

fn f11<T1>() -> Option<Value<Box<[T1]>>> {
    None
}

fn f12<T1>(a0: &mut Option<Value<T1>>) -> Option<Value<T1>> {
    a0.take()
}

fn f13<T1>(a0: &mut Option<Value<Box<[T1]>>>) -> Option<Value<Box<[T1]>>> {
    a0.take()
}

fn f14<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: &mut Option<Value<T1>>) {
    a0.write(a1.take())
}

fn f15<T1: ByteRepr>(a0: Ptr<Option<Value<Box<[T1]>>>>, a1: &mut Option<Value<Box<[T1]>>>) {
    a0.write(a1.take())
}

fn f16<T1>(a0: &mut Option<Value<T1>>) -> Ptr<T1> {
    let __owned = a0.take();
    let __p: Ptr<T1> = __owned.as_pointer();
    ::std::mem::forget(__owned);
    __p
}

fn f17<T1>() -> Option<Value<T1>> {
    None
}

// Comparison against nullptr. Ownership IS the Option's discriminant, so all
// four forms are `.is_some()` / `.is_none()`. See src.cpp for why this is the
// top ==/!= row by TUs blocked, and for the f3 null-check defect these bodies
// deliberately do not paper over.
//
// The std::nullptr_t operand takes NO Rust parameter: the rule preprocessor
// elided ONLY WHEN TRAILING -- which is what f9 (`reset(std::nullptr_t)`) and
// f17 (`unique_ptr(std::nullptr_t)`) in this module already rely on. A LEADING
// one cannot be dropped: positions are matched in C++ order, so for the
// reversed forms f20/f21 the nullptr is a0 and the unique_ptr is a1. The
// converter emits a bare `Default::default()` for it with no type, which is an
// E0282 unless the parameter names a type that can satisfy it -- measured, not
// guessed: declaring f20 with one parameter produced
// `(Default::default().is_some() as i32)` and `cannot infer type`. `()` is the
// smallest type that both implements Default and cannot be confused with a
// real value, and the `let () = a0;` binding consumes it so the body does not
// carry an unused parameter.
fn f18<T1>(a0: &Option<Value<T1>>) -> bool {
    a0.is_some()
}

fn f19<T1>(a0: &Option<Value<T1>>) -> bool {
    a0.is_none()
}

fn f20<T1>(a0: (), a1: &Option<Value<T1>>) -> bool {
    let () = a0;
    a1.is_some()
}

fn f21<T1>(a0: (), a1: &Option<Value<T1>>) -> bool {
    let () = a0;
    a1.is_none()
}

fn f22<T1>(a0: &Option<Value<T1>>) -> bool {
    a0.is_some()
}
