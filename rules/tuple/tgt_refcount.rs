// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn t1<T1: Default, T2: Default>() -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(T1::default())),
        Rc::new(RefCell::new(T2::default())),
    )
}

fn t2<T1: Default, T2: Default, T3: Default>() -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(T1::default())),
        Rc::new(RefCell::new(T2::default())),
        Rc::new(RefCell::new(T3::default())),
    )
}

fn t3<T1: Default, T2: Default, T3: Default, T4: Default>() -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(T1::default())),
        Rc::new(RefCell::new(T2::default())),
        Rc::new(RefCell::new(T3::default())),
        Rc::new(RefCell::new(T4::default())),
    )
}

// --- 2-element tuples -------------------------------------------------------

fn f1<T1: Default, T2: Default>() -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(T1::default())),
        Rc::new(RefCell::new(T2::default())),
    )
}

fn f2<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f3<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f4<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f5<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f6<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f7<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f8<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f9<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f10<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f11<T1: Clone, T2: Clone>(a0: (Value<T1>, Value<T2>)) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.0.borrow().clone())),
        Rc::new(RefCell::new(a0.1.borrow().clone())),
    )
}

fn f12<T1: Default, T2: Default>(a0: &mut (Value<T1>, Value<T2>)) -> (Value<T1>, Value<T2>) {
    std::mem::take(&mut *a0)
}

fn f13<T1: Clone + ByteRepr, T2: Clone + ByteRepr>(
    a0: Ptr<(Value<T1>, Value<T2>)>,
    a1: (Value<T1>, Value<T2>),
) {
    a0.write((
        Rc::new(RefCell::new(a1.0.borrow().clone())),
        Rc::new(RefCell::new(a1.1.borrow().clone())),
    ))
}

fn f14<T1: Default + ByteRepr, T2: Default + ByteRepr>(
    a0: Ptr<(Value<T1>, Value<T2>)>,
    a1: &mut (Value<T1>, Value<T2>),
) {
    a0.write(std::mem::take(&mut *a1))
}

fn f15<T1, T2>(a0: (Value<T1>, Value<T2>)) -> Ptr<T1> {
    (a0.0.as_pointer() as Ptr<T1>)
}

fn f16<T1, T2>(a0: (Value<T1>, Value<T2>)) -> Ptr<T2> {
    (a0.1.as_pointer() as Ptr<T2>)
}

fn f17<T1, T2>(a0: (Value<T1>, Value<T2>)) -> Ptr<T1> {
    (a0.0.as_pointer() as Ptr<T1>)
}

fn f18<T1, T2>(a0: (Value<T1>, Value<T2>)) -> Ptr<T2> {
    (a0.1.as_pointer() as Ptr<T2>)
}

fn f19<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f20<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f21<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f22<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f23<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f24<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f25<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f26<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f27<T1, T2>(a0: T1, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
    )
}

fn f28<T1: PartialEq, T2: PartialEq>(
    a0: (Value<T1>, Value<T2>),
    a1: (Value<T1>, Value<T2>),
) -> bool {
    a0 == a1
}

fn f29<T1: PartialEq, T2: PartialEq>(
    a0: (Value<T1>, Value<T2>),
    a1: (Value<T1>, Value<T2>),
) -> bool {
    a0 != a1
}

// --- 3-element tuples -------------------------------------------------------

fn f30<T1: Default, T2: Default, T3: Default>() -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(T1::default())),
        Rc::new(RefCell::new(T2::default())),
        Rc::new(RefCell::new(T3::default())),
    )
}

fn f31<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f32<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f33<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f34<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f35<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f36<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f37<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f38<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f39<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f40<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f41<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f42<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f43<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f44<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f45<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f46<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f47<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f48<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f49<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f50<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f51<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f52<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f53<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f54<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f55<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f56<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f57<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f58<T1: Clone, T2: Clone, T3: Clone>(a0: (Value<T1>, Value<T2>, Value<T3>)) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.0.borrow().clone())),
        Rc::new(RefCell::new(a0.1.borrow().clone())),
        Rc::new(RefCell::new(a0.2.borrow().clone())),
    )
}

fn f59<T1: Default, T2: Default, T3: Default>(a0: &mut (Value<T1>, Value<T2>, Value<T3>)) -> (Value<T1>, Value<T2>, Value<T3>) {
    std::mem::take(&mut *a0)
}

fn f60<T1: Clone, T2: Clone, T3: Clone>(
    a0: Ptr<(Value<T1>, Value<T2>, Value<T3>)>,
    a1: (Value<T1>, Value<T2>, Value<T3>),
) {
    let __dst = a0.to_strong();
    *(*__dst).borrow_mut() = (
        Rc::new(RefCell::new(a1.0.borrow().clone())),
        Rc::new(RefCell::new(a1.1.borrow().clone())),
        Rc::new(RefCell::new(a1.2.borrow().clone())),
    );
}

fn f61<T1: Default, T2: Default, T3: Default>(
    a0: Ptr<(Value<T1>, Value<T2>, Value<T3>)>,
    a1: &mut (Value<T1>, Value<T2>, Value<T3>),
) {
    let __dst = a0.to_strong();
    *(*__dst).borrow_mut() = std::mem::take(&mut *a1);
}

fn f62<T1, T2, T3>(a0: (Value<T1>, Value<T2>, Value<T3>)) -> Ptr<T1> {
    (a0.0.as_pointer() as Ptr<T1>)
}

fn f63<T1, T2, T3>(a0: (Value<T1>, Value<T2>, Value<T3>)) -> Ptr<T2> {
    (a0.1.as_pointer() as Ptr<T2>)
}

fn f64<T1, T2, T3>(a0: (Value<T1>, Value<T2>, Value<T3>)) -> Ptr<T3> {
    (a0.2.as_pointer() as Ptr<T3>)
}

fn f65<T1, T2, T3>(a0: (Value<T1>, Value<T2>, Value<T3>)) -> Ptr<T1> {
    (a0.0.as_pointer() as Ptr<T1>)
}

fn f66<T1, T2, T3>(a0: (Value<T1>, Value<T2>, Value<T3>)) -> Ptr<T2> {
    (a0.1.as_pointer() as Ptr<T2>)
}

fn f67<T1, T2, T3>(a0: (Value<T1>, Value<T2>, Value<T3>)) -> Ptr<T3> {
    (a0.2.as_pointer() as Ptr<T3>)
}

fn f68<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f69<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f70<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f71<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f72<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f73<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f74<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f75<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f76<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f77<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f78<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f79<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f80<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f81<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f82<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f83<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f84<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f85<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f86<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f87<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f88<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f89<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f90<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f91<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f92<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f93<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f94<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (Value<T1>, Value<T2>, Value<T3>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
    )
}

fn f95<T1: PartialEq, T2: PartialEq, T3: PartialEq>(
    a0: (Value<T1>, Value<T2>, Value<T3>),
    a1: (Value<T1>, Value<T2>, Value<T3>),
) -> bool {
    a0 == a1
}

fn f96<T1: PartialEq, T2: PartialEq, T3: PartialEq>(
    a0: (Value<T1>, Value<T2>, Value<T3>),
    a1: (Value<T1>, Value<T2>, Value<T3>),
) -> bool {
    a0 != a1
}

// --- 4-element tuples -------------------------------------------------------

fn f97<T1: Default, T2: Default, T3: Default, T4: Default>() -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(T1::default())),
        Rc::new(RefCell::new(T2::default())),
        Rc::new(RefCell::new(T3::default())),
        Rc::new(RefCell::new(T4::default())),
    )
}

fn f98<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f99<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f100<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f101<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f102<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f103<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f104<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f105<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f106<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f107<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f108<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f109<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f110<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f111<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f112<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f113<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f114<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f115<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f116<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f117<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f118<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f119<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f120<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f121<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f122<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f123<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f124<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f125<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f126<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f127<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f128<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f129<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f130<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f131<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f132<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f133<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f134<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f135<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f136<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f137<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f138<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f139<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f140<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f141<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f142<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f143<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f144<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f145<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f146<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f147<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f148<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f149<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f150<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f151<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f152<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f153<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f154<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f155<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f156<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f157<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f158<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f159<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f160<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f161<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f162<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f163<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f164<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f165<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f166<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f167<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f168<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f169<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f170<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f171<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f172<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f173<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f174<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f175<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f176<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f177<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f178<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f179<T1: Clone, T2: Clone, T3: Clone, T4: Clone>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>)) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.0.borrow().clone())),
        Rc::new(RefCell::new(a0.1.borrow().clone())),
        Rc::new(RefCell::new(a0.2.borrow().clone())),
        Rc::new(RefCell::new(a0.3.borrow().clone())),
    )
}

fn f180<T1: Default, T2: Default, T3: Default, T4: Default>(a0: &mut (Value<T1>, Value<T2>, Value<T3>, Value<T4>)) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    std::mem::take(&mut *a0)
}

fn f181<T1: Clone, T2: Clone, T3: Clone, T4: Clone>(
    a0: Ptr<(Value<T1>, Value<T2>, Value<T3>, Value<T4>)>,
    a1: (Value<T1>, Value<T2>, Value<T3>, Value<T4>),
) {
    let __dst = a0.to_strong();
    *(*__dst).borrow_mut() = (
        Rc::new(RefCell::new(a1.0.borrow().clone())),
        Rc::new(RefCell::new(a1.1.borrow().clone())),
        Rc::new(RefCell::new(a1.2.borrow().clone())),
        Rc::new(RefCell::new(a1.3.borrow().clone())),
    );
}

fn f182<T1: Default, T2: Default, T3: Default, T4: Default>(
    a0: Ptr<(Value<T1>, Value<T2>, Value<T3>, Value<T4>)>,
    a1: &mut (Value<T1>, Value<T2>, Value<T3>, Value<T4>),
) {
    let __dst = a0.to_strong();
    *(*__dst).borrow_mut() = std::mem::take(&mut *a1);
}

fn f183<T1, T2, T3, T4>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>)) -> Ptr<T1> {
    (a0.0.as_pointer() as Ptr<T1>)
}

fn f184<T1, T2, T3, T4>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>)) -> Ptr<T2> {
    (a0.1.as_pointer() as Ptr<T2>)
}

fn f185<T1, T2, T3, T4>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>)) -> Ptr<T3> {
    (a0.2.as_pointer() as Ptr<T3>)
}

fn f186<T1, T2, T3, T4>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>)) -> Ptr<T4> {
    (a0.3.as_pointer() as Ptr<T4>)
}

fn f187<T1, T2, T3, T4>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>)) -> Ptr<T1> {
    (a0.0.as_pointer() as Ptr<T1>)
}

fn f188<T1, T2, T3, T4>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>)) -> Ptr<T2> {
    (a0.1.as_pointer() as Ptr<T2>)
}

fn f189<T1, T2, T3, T4>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>)) -> Ptr<T3> {
    (a0.2.as_pointer() as Ptr<T3>)
}

fn f190<T1, T2, T3, T4>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>)) -> Ptr<T4> {
    (a0.3.as_pointer() as Ptr<T4>)
}

fn f191<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f192<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f193<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f194<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f195<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f196<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f197<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f198<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f199<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f200<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f201<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f202<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f203<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f204<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f205<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f206<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f207<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f208<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f209<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f210<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f211<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f212<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f213<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f214<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f215<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f216<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f217<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f218<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f219<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f220<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f221<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f222<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f223<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f224<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f225<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f226<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f227<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f228<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f229<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f230<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f231<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f232<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f233<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f234<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f235<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f236<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f237<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f238<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f239<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f240<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f241<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f242<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f243<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f244<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f245<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f246<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f247<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f248<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f249<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f250<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f251<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f252<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f253<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f254<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f255<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f256<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f257<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f258<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f259<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f260<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f261<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f262<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f263<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f264<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f265<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f266<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f267<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f268<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f269<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f270<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f271<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
    )
}

fn f272<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq>(
    a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>),
    a1: (Value<T1>, Value<T2>, Value<T3>, Value<T4>),
) -> bool {
    a0 == a1
}

fn f273<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq>(
    a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>),
    a1: (Value<T1>, Value<T2>, Value<T3>, Value<T4>),
) -> bool {
    a0 != a1
}

// --- std::tie, 2 elements ---------------------------------------------------

fn t4<T1, T2>() -> (Ptr<T1>, Ptr<T2>) {
    (Ptr::null(), Ptr::null())
}

fn f274<T1, T2>(a0: Ptr<T1>, a1: Ptr<T2>) -> (Ptr<T1>, Ptr<T2>) {
    (a0, a1)
}

fn f275<T1: Clone + ByteRepr, T2: Clone + ByteRepr>(
    a0: Ptr<(Ptr<T1>, Ptr<T2>)>,
    a1: (Value<T1>, Value<T2>),
) {
    let __dst = a0.to_strong();
    (*__dst).borrow().0.write(a1.0.borrow().clone());
    (*__dst).borrow().1.write(a1.1.borrow().clone());
}

fn f276<T1, T2>(a0: (Ptr<T1>, Ptr<T2>)) -> Ptr<T1> {
    a0.0
}

fn f277<T1, T2>(a0: (Ptr<T1>, Ptr<T2>)) -> Ptr<T2> {
    a0.1
}

// --- std::tie, 3 elements ---------------------------------------------------

fn t5<T1, T2, T3>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>) {
    (Ptr::null(), Ptr::null(), Ptr::null())
}

fn f278<T1, T2, T3>(a0: Ptr<T1>, a1: Ptr<T2>, a2: Ptr<T3>) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>) {
    (a0, a1, a2)
}

fn f279<T1: Clone + ByteRepr, T2: Clone + ByteRepr, T3: Clone + ByteRepr>(
    a0: Ptr<(Ptr<T1>, Ptr<T2>, Ptr<T3>)>,
    a1: (Value<T1>, Value<T2>, Value<T3>),
) {
    let __dst = a0.to_strong();
    (*__dst).borrow().0.write(a1.0.borrow().clone());
    (*__dst).borrow().1.write(a1.1.borrow().clone());
    (*__dst).borrow().2.write(a1.2.borrow().clone());
}

fn f280<T1, T2, T3>(a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>)) -> Ptr<T1> {
    a0.0
}

fn f281<T1, T2, T3>(a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>)) -> Ptr<T2> {
    a0.1
}

fn f282<T1, T2, T3>(a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>)) -> Ptr<T3> {
    a0.2
}

// --- std::tie, 4 elements ---------------------------------------------------

fn t6<T1, T2, T3, T4>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f283<T1, T2, T3, T4>(a0: Ptr<T1>, a1: Ptr<T2>, a2: Ptr<T3>, a3: Ptr<T4>) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>) {
    (a0, a1, a2, a3)
}

fn f284<T1: Clone + ByteRepr, T2: Clone + ByteRepr, T3: Clone + ByteRepr, T4: Clone + ByteRepr>(
    a0: Ptr<(Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>)>,
    a1: (Value<T1>, Value<T2>, Value<T3>, Value<T4>),
) {
    let __dst = a0.to_strong();
    (*__dst).borrow().0.write(a1.0.borrow().clone());
    (*__dst).borrow().1.write(a1.1.borrow().clone());
    (*__dst).borrow().2.write(a1.2.borrow().clone());
    (*__dst).borrow().3.write(a1.3.borrow().clone());
}

fn f285<T1, T2, T3, T4>(a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>)) -> Ptr<T1> {
    a0.0
}

fn f286<T1, T2, T3, T4>(a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>)) -> Ptr<T2> {
    a0.1
}

fn f287<T1, T2, T3, T4>(a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>)) -> Ptr<T3> {
    a0.2
}

fn f288<T1, T2, T3, T4>(a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>)) -> Ptr<T4> {
    a0.3
}

