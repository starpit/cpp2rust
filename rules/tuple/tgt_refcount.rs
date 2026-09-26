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

fn f11<T1: DeepClone, T2: DeepClone>(a0: (Value<T1>, Value<T2>)) -> (Value<T1>, Value<T2>) {
    a0.deep_clone()
}

fn f12<T1: Default, T2: Default>(a0: &mut (Value<T1>, Value<T2>)) -> (Value<T1>, Value<T2>) {
    std::mem::take(&mut *a0)
}

fn f13<T1: DeepClone + ByteRepr, T2: DeepClone + ByteRepr>(
    a0: Ptr<(Value<T1>, Value<T2>)>,
    a1: (Value<T1>, Value<T2>),
) {
    a0.write(a1.deep_clone())
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

fn f58<T1: DeepClone, T2: DeepClone, T3: DeepClone>(a0: (Value<T1>, Value<T2>, Value<T3>)) -> (Value<T1>, Value<T2>, Value<T3>) {
    a0.deep_clone()
}

fn f59<T1: Default, T2: Default, T3: Default>(a0: &mut (Value<T1>, Value<T2>, Value<T3>)) -> (Value<T1>, Value<T2>, Value<T3>) {
    std::mem::take(&mut *a0)
}

fn f60<T1: DeepClone + ByteRepr, T2: DeepClone + ByteRepr, T3: DeepClone + ByteRepr>(
    a0: Ptr<(Value<T1>, Value<T2>, Value<T3>)>,
    a1: (Value<T1>, Value<T2>, Value<T3>),
) {
    a0.write(a1.deep_clone())
}

fn f61<T1: Default + ByteRepr, T2: Default + ByteRepr, T3: Default + ByteRepr>(
    a0: Ptr<(Value<T1>, Value<T2>, Value<T3>)>,
    a1: &mut (Value<T1>, Value<T2>, Value<T3>),
) {
    a0.write(std::mem::take(&mut *a1))
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

fn f179<T1: DeepClone, T2: DeepClone, T3: DeepClone, T4: DeepClone>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>)) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    a0.deep_clone()
}

fn f180<T1: Default, T2: Default, T3: Default, T4: Default>(a0: &mut (Value<T1>, Value<T2>, Value<T3>, Value<T4>)) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>) {
    std::mem::take(&mut *a0)
}

fn f181<T1: DeepClone + ByteRepr, T2: DeepClone + ByteRepr, T3: DeepClone + ByteRepr, T4: DeepClone + ByteRepr>(
    a0: Ptr<(Value<T1>, Value<T2>, Value<T3>, Value<T4>)>,
    a1: (Value<T1>, Value<T2>, Value<T3>, Value<T4>),
) {
    a0.write(a1.deep_clone())
}

fn f182<T1: Default + ByteRepr, T2: Default + ByteRepr, T3: Default + ByteRepr, T4: Default + ByteRepr>(
    a0: Ptr<(Value<T1>, Value<T2>, Value<T3>, Value<T4>)>,
    a1: &mut (Value<T1>, Value<T2>, Value<T3>, Value<T4>),
) {
    a0.write(std::mem::take(&mut *a1))
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
    a0.with(|__dst: &(Ptr<T1>, Ptr<T2>)| {
        __dst.0.write(a1.0.borrow().clone());
        __dst.1.write(a1.1.borrow().clone());
    })
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
    a0.with(|__dst: &(Ptr<T1>, Ptr<T2>, Ptr<T3>)| {
        __dst.0.write(a1.0.borrow().clone());
        __dst.1.write(a1.1.borrow().clone());
        __dst.2.write(a1.2.borrow().clone());
    })
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
    a0.with(|__dst: &(Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>)| {
        __dst.0.write(a1.0.borrow().clone());
        __dst.1.write(a1.1.borrow().clone());
        __dst.2.write(a1.2.borrow().clone());
        __dst.3.write(a1.3.borrow().clone());
    })
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

// ==== high-arity extension (generated by /tmp/gen_tuple_rules_highN.py) ====

// COMPARING A TUPLE OF REFERENCES COMPARES THE POINTED-TO VALUES, NOT THE
// HANDLES.  That is C++ semantics -- std::tuple's operator== compares elements
// -- and it is what these bodies have always done, but the reason is worth
// spelling out because the obvious migration silently breaks it.
//
// These rules used to read `a0.N.to_strong() == a1.N.to_strong()`.  Upstream
// deleted Ptr::to_strong() in favour of Ptr::decay(), and neither the deletion
// nor decay() is the right substitution here, because to_strong() was never
// doing pointer work in this position: it returned `Value<T>` = `Rc<RefCell<T>>`,
// and Rc's PartialEq DELEGATES TO T.  So `to_strong() == to_strong()` was a
// VALUE comparison that merely looked like a handle comparison.
//
// The tempting one-line migration is to drop the calls and let `Ptr: PartialEq`
// take over.  Do not: that impl (libcc2rs/src/rc.rs:166) compares
// `byte_offset()` and `kind`, i.e. ADDRESS IDENTITY.  Two tuples holding equal
// values in distinct allocations would then compare UNEQUAL, with no
// diagnostic anywhere -- which is the exact bug the src.cpp note above these
// rules records as the reason they were written in the first place.
//
// `*a0.N.upgrade().deref()` reaches the pointee and compares it, preserving the
// old behaviour exactly.  Measured, all three cases agreeing between the old
// to_strong() spelling and this one, and the dropped-call variant differing on
// the equal-values case: distinct allocations + equal values -> true,
// distinct + different -> false, same allocation aliased twice -> true.
// deref() is why every TN here carries ByteRepr in addition to PartialEq.

// --- std::tie comparison, 2 elements ----------------------------------------

fn f289<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>),
    a1: (Ptr<T1>, Ptr<T2>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref())
}

fn f290<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>),
    a1: (Ptr<T1>, Ptr<T2>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref())
}

// --- std::tie comparison, 3 elements ----------------------------------------

fn f291<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref())
}

fn f292<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref())
}

// --- std::tie comparison, 4 elements ----------------------------------------

fn f293<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref())
}

fn f294<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref())
}

// --- std::tie, 5 elements ---------------------------------------------------

fn t7<T1, T2, T3, T4, T5>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f295<T1, T2, T3, T4, T5>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>) {
    (a0, a1, a2, a3, a4)
}

fn f296<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref())
}

fn f297<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref())
}

// --- std::tie, 6 elements ---------------------------------------------------

fn t8<T1, T2, T3, T4, T5, T6>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f298<T1, T2, T3, T4, T5, T6>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>) {
    (a0, a1, a2, a3, a4, a5)
}

fn f299<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref())
}

fn f300<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref())
}

// --- std::tie, 7 elements ---------------------------------------------------

fn t9<T1, T2, T3, T4, T5, T6, T7>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f301<T1, T2, T3, T4, T5, T6, T7>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>) {
    (a0, a1, a2, a3, a4, a5, a6)
}

fn f302<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref())
}

fn f303<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref())
}

// --- std::tie, 8 elements ---------------------------------------------------

fn t10<T1, T2, T3, T4, T5, T6, T7, T8>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f304<T1, T2, T3, T4, T5, T6, T7, T8>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>) {
    (a0, a1, a2, a3, a4, a5, a6, a7)
}

fn f305<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref())
}

fn f306<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref())
}

// --- std::tie, 9 elements ---------------------------------------------------

fn t11<T1, T2, T3, T4, T5, T6, T7, T8, T9>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f307<T1, T2, T3, T4, T5, T6, T7, T8, T9>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8)
}

fn f308<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref())
}

fn f309<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref())
}

// --- std::tie, 10 elements --------------------------------------------------

fn t12<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f310<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9)
}

fn f311<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref())
}

fn f312<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref())
}

// --- std::tie, 11 elements --------------------------------------------------

fn t13<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f313<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
}

fn f314<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref())
}

fn f315<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref())
}

// --- std::tie, 12 elements --------------------------------------------------

fn t14<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f316<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11)
}

fn f317<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref())
}

fn f318<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref())
}

// --- std::tie, 13 elements --------------------------------------------------

fn t15<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f319<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12)
}

fn f320<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref())
}

fn f321<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref())
}

// --- std::tie, 14 elements --------------------------------------------------

fn t16<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f322<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13)
}

fn f323<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref())
}

fn f324<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref())
}

// --- std::tie, 15 elements --------------------------------------------------

fn t17<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f325<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14)
}

fn f326<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref())
}

fn f327<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref())
}

// --- std::tie, 16 elements --------------------------------------------------

fn t18<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f328<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15)
}

fn f329<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref())
}

fn f330<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref())
}

// --- std::tie, 17 elements --------------------------------------------------

fn t19<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f331<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16)
}

fn f332<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref())
}

fn f333<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref())
}

// --- std::tie, 18 elements --------------------------------------------------

fn t20<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f334<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
    a17: Ptr<T18>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17)
}

fn f335<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref()) &&
        (*a0.17.upgrade().deref() == *a1.17.upgrade().deref())
}

fn f336<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref()) ||
        (*a0.17.upgrade().deref() != *a1.17.upgrade().deref())
}

// --- std::tie, 19 elements --------------------------------------------------

fn t21<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f337<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
    a17: Ptr<T18>,
    a18: Ptr<T19>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18)
}

fn f338<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref()) &&
        (*a0.17.upgrade().deref() == *a1.17.upgrade().deref()) &&
        (*a0.18.upgrade().deref() == *a1.18.upgrade().deref())
}

fn f339<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref()) ||
        (*a0.17.upgrade().deref() != *a1.17.upgrade().deref()) ||
        (*a0.18.upgrade().deref() != *a1.18.upgrade().deref())
}

// --- std::tie, 20 elements --------------------------------------------------

fn t22<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f340<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
    a17: Ptr<T18>,
    a18: Ptr<T19>,
    a19: Ptr<T20>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19)
}

fn f341<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref()) &&
        (*a0.17.upgrade().deref() == *a1.17.upgrade().deref()) &&
        (*a0.18.upgrade().deref() == *a1.18.upgrade().deref()) &&
        (*a0.19.upgrade().deref() == *a1.19.upgrade().deref())
}

fn f342<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref()) ||
        (*a0.17.upgrade().deref() != *a1.17.upgrade().deref()) ||
        (*a0.18.upgrade().deref() != *a1.18.upgrade().deref()) ||
        (*a0.19.upgrade().deref() != *a1.19.upgrade().deref())
}

// --- std::tie, 21 elements --------------------------------------------------

fn t23<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f343<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
    a17: Ptr<T18>,
    a18: Ptr<T19>,
    a19: Ptr<T20>,
    a20: Ptr<T21>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20)
}

fn f344<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref()) &&
        (*a0.17.upgrade().deref() == *a1.17.upgrade().deref()) &&
        (*a0.18.upgrade().deref() == *a1.18.upgrade().deref()) &&
        (*a0.19.upgrade().deref() == *a1.19.upgrade().deref()) &&
        (*a0.20.upgrade().deref() == *a1.20.upgrade().deref())
}

fn f345<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref()) ||
        (*a0.17.upgrade().deref() != *a1.17.upgrade().deref()) ||
        (*a0.18.upgrade().deref() != *a1.18.upgrade().deref()) ||
        (*a0.19.upgrade().deref() != *a1.19.upgrade().deref()) ||
        (*a0.20.upgrade().deref() != *a1.20.upgrade().deref())
}

// --- std::tie, 22 elements --------------------------------------------------

fn t24<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f346<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
    a17: Ptr<T18>,
    a18: Ptr<T19>,
    a19: Ptr<T20>,
    a20: Ptr<T21>,
    a21: Ptr<T22>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21)
}

fn f347<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref()) &&
        (*a0.17.upgrade().deref() == *a1.17.upgrade().deref()) &&
        (*a0.18.upgrade().deref() == *a1.18.upgrade().deref()) &&
        (*a0.19.upgrade().deref() == *a1.19.upgrade().deref()) &&
        (*a0.20.upgrade().deref() == *a1.20.upgrade().deref()) &&
        (*a0.21.upgrade().deref() == *a1.21.upgrade().deref())
}

fn f348<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref()) ||
        (*a0.17.upgrade().deref() != *a1.17.upgrade().deref()) ||
        (*a0.18.upgrade().deref() != *a1.18.upgrade().deref()) ||
        (*a0.19.upgrade().deref() != *a1.19.upgrade().deref()) ||
        (*a0.20.upgrade().deref() != *a1.20.upgrade().deref()) ||
        (*a0.21.upgrade().deref() != *a1.21.upgrade().deref())
}

// --- std::tie, 23 elements --------------------------------------------------

fn t25<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f349<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
    a17: Ptr<T18>,
    a18: Ptr<T19>,
    a19: Ptr<T20>,
    a20: Ptr<T21>,
    a21: Ptr<T22>,
    a22: Ptr<T23>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22)
}

fn f350<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref()) &&
        (*a0.17.upgrade().deref() == *a1.17.upgrade().deref()) &&
        (*a0.18.upgrade().deref() == *a1.18.upgrade().deref()) &&
        (*a0.19.upgrade().deref() == *a1.19.upgrade().deref()) &&
        (*a0.20.upgrade().deref() == *a1.20.upgrade().deref()) &&
        (*a0.21.upgrade().deref() == *a1.21.upgrade().deref()) &&
        (*a0.22.upgrade().deref() == *a1.22.upgrade().deref())
}

fn f351<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref()) ||
        (*a0.17.upgrade().deref() != *a1.17.upgrade().deref()) ||
        (*a0.18.upgrade().deref() != *a1.18.upgrade().deref()) ||
        (*a0.19.upgrade().deref() != *a1.19.upgrade().deref()) ||
        (*a0.20.upgrade().deref() != *a1.20.upgrade().deref()) ||
        (*a0.21.upgrade().deref() != *a1.21.upgrade().deref()) ||
        (*a0.22.upgrade().deref() != *a1.22.upgrade().deref())
}

// --- std::tie, 24 elements --------------------------------------------------

fn t26<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f352<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
    a17: Ptr<T18>,
    a18: Ptr<T19>,
    a19: Ptr<T20>,
    a20: Ptr<T21>,
    a21: Ptr<T22>,
    a22: Ptr<T23>,
    a23: Ptr<T24>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23)
}

fn f353<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref()) &&
        (*a0.17.upgrade().deref() == *a1.17.upgrade().deref()) &&
        (*a0.18.upgrade().deref() == *a1.18.upgrade().deref()) &&
        (*a0.19.upgrade().deref() == *a1.19.upgrade().deref()) &&
        (*a0.20.upgrade().deref() == *a1.20.upgrade().deref()) &&
        (*a0.21.upgrade().deref() == *a1.21.upgrade().deref()) &&
        (*a0.22.upgrade().deref() == *a1.22.upgrade().deref()) &&
        (*a0.23.upgrade().deref() == *a1.23.upgrade().deref())
}

fn f354<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref()) ||
        (*a0.17.upgrade().deref() != *a1.17.upgrade().deref()) ||
        (*a0.18.upgrade().deref() != *a1.18.upgrade().deref()) ||
        (*a0.19.upgrade().deref() != *a1.19.upgrade().deref()) ||
        (*a0.20.upgrade().deref() != *a1.20.upgrade().deref()) ||
        (*a0.21.upgrade().deref() != *a1.21.upgrade().deref()) ||
        (*a0.22.upgrade().deref() != *a1.22.upgrade().deref()) ||
        (*a0.23.upgrade().deref() != *a1.23.upgrade().deref())
}

// --- std::tie, 25 elements --------------------------------------------------

fn t27<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f355<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
    a17: Ptr<T18>,
    a18: Ptr<T19>,
    a19: Ptr<T20>,
    a20: Ptr<T21>,
    a21: Ptr<T22>,
    a22: Ptr<T23>,
    a23: Ptr<T24>,
    a24: Ptr<T25>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24)
}

fn f356<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref()) &&
        (*a0.17.upgrade().deref() == *a1.17.upgrade().deref()) &&
        (*a0.18.upgrade().deref() == *a1.18.upgrade().deref()) &&
        (*a0.19.upgrade().deref() == *a1.19.upgrade().deref()) &&
        (*a0.20.upgrade().deref() == *a1.20.upgrade().deref()) &&
        (*a0.21.upgrade().deref() == *a1.21.upgrade().deref()) &&
        (*a0.22.upgrade().deref() == *a1.22.upgrade().deref()) &&
        (*a0.23.upgrade().deref() == *a1.23.upgrade().deref()) &&
        (*a0.24.upgrade().deref() == *a1.24.upgrade().deref())
}

fn f357<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref()) ||
        (*a0.17.upgrade().deref() != *a1.17.upgrade().deref()) ||
        (*a0.18.upgrade().deref() != *a1.18.upgrade().deref()) ||
        (*a0.19.upgrade().deref() != *a1.19.upgrade().deref()) ||
        (*a0.20.upgrade().deref() != *a1.20.upgrade().deref()) ||
        (*a0.21.upgrade().deref() != *a1.21.upgrade().deref()) ||
        (*a0.22.upgrade().deref() != *a1.22.upgrade().deref()) ||
        (*a0.23.upgrade().deref() != *a1.23.upgrade().deref()) ||
        (*a0.24.upgrade().deref() != *a1.24.upgrade().deref())
}

// --- std::tie, 26 elements --------------------------------------------------

fn t28<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f358<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
    a17: Ptr<T18>,
    a18: Ptr<T19>,
    a19: Ptr<T20>,
    a20: Ptr<T21>,
    a21: Ptr<T22>,
    a22: Ptr<T23>,
    a23: Ptr<T24>,
    a24: Ptr<T25>,
    a25: Ptr<T26>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25)
}

fn f359<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr, T26: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref()) &&
        (*a0.17.upgrade().deref() == *a1.17.upgrade().deref()) &&
        (*a0.18.upgrade().deref() == *a1.18.upgrade().deref()) &&
        (*a0.19.upgrade().deref() == *a1.19.upgrade().deref()) &&
        (*a0.20.upgrade().deref() == *a1.20.upgrade().deref()) &&
        (*a0.21.upgrade().deref() == *a1.21.upgrade().deref()) &&
        (*a0.22.upgrade().deref() == *a1.22.upgrade().deref()) &&
        (*a0.23.upgrade().deref() == *a1.23.upgrade().deref()) &&
        (*a0.24.upgrade().deref() == *a1.24.upgrade().deref()) &&
        (*a0.25.upgrade().deref() == *a1.25.upgrade().deref())
}

fn f360<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr, T26: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref()) ||
        (*a0.17.upgrade().deref() != *a1.17.upgrade().deref()) ||
        (*a0.18.upgrade().deref() != *a1.18.upgrade().deref()) ||
        (*a0.19.upgrade().deref() != *a1.19.upgrade().deref()) ||
        (*a0.20.upgrade().deref() != *a1.20.upgrade().deref()) ||
        (*a0.21.upgrade().deref() != *a1.21.upgrade().deref()) ||
        (*a0.22.upgrade().deref() != *a1.22.upgrade().deref()) ||
        (*a0.23.upgrade().deref() != *a1.23.upgrade().deref()) ||
        (*a0.24.upgrade().deref() != *a1.24.upgrade().deref()) ||
        (*a0.25.upgrade().deref() != *a1.25.upgrade().deref())
}

// --- std::tie, 27 elements --------------------------------------------------

fn t29<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f361<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
    a17: Ptr<T18>,
    a18: Ptr<T19>,
    a19: Ptr<T20>,
    a20: Ptr<T21>,
    a21: Ptr<T22>,
    a22: Ptr<T23>,
    a23: Ptr<T24>,
    a24: Ptr<T25>,
    a25: Ptr<T26>,
    a26: Ptr<T27>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26)
}

fn f362<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr, T26: PartialEq + ByteRepr, T27: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref()) &&
        (*a0.17.upgrade().deref() == *a1.17.upgrade().deref()) &&
        (*a0.18.upgrade().deref() == *a1.18.upgrade().deref()) &&
        (*a0.19.upgrade().deref() == *a1.19.upgrade().deref()) &&
        (*a0.20.upgrade().deref() == *a1.20.upgrade().deref()) &&
        (*a0.21.upgrade().deref() == *a1.21.upgrade().deref()) &&
        (*a0.22.upgrade().deref() == *a1.22.upgrade().deref()) &&
        (*a0.23.upgrade().deref() == *a1.23.upgrade().deref()) &&
        (*a0.24.upgrade().deref() == *a1.24.upgrade().deref()) &&
        (*a0.25.upgrade().deref() == *a1.25.upgrade().deref()) &&
        (*a0.26.upgrade().deref() == *a1.26.upgrade().deref())
}

fn f363<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr, T26: PartialEq + ByteRepr, T27: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref()) ||
        (*a0.17.upgrade().deref() != *a1.17.upgrade().deref()) ||
        (*a0.18.upgrade().deref() != *a1.18.upgrade().deref()) ||
        (*a0.19.upgrade().deref() != *a1.19.upgrade().deref()) ||
        (*a0.20.upgrade().deref() != *a1.20.upgrade().deref()) ||
        (*a0.21.upgrade().deref() != *a1.21.upgrade().deref()) ||
        (*a0.22.upgrade().deref() != *a1.22.upgrade().deref()) ||
        (*a0.23.upgrade().deref() != *a1.23.upgrade().deref()) ||
        (*a0.24.upgrade().deref() != *a1.24.upgrade().deref()) ||
        (*a0.25.upgrade().deref() != *a1.25.upgrade().deref()) ||
        (*a0.26.upgrade().deref() != *a1.26.upgrade().deref())
}

// --- std::tie, 28 elements --------------------------------------------------

fn t30<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f364<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
    a17: Ptr<T18>,
    a18: Ptr<T19>,
    a19: Ptr<T20>,
    a20: Ptr<T21>,
    a21: Ptr<T22>,
    a22: Ptr<T23>,
    a23: Ptr<T24>,
    a24: Ptr<T25>,
    a25: Ptr<T26>,
    a26: Ptr<T27>,
    a27: Ptr<T28>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27)
}

fn f365<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr, T26: PartialEq + ByteRepr, T27: PartialEq + ByteRepr, T28: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref()) &&
        (*a0.17.upgrade().deref() == *a1.17.upgrade().deref()) &&
        (*a0.18.upgrade().deref() == *a1.18.upgrade().deref()) &&
        (*a0.19.upgrade().deref() == *a1.19.upgrade().deref()) &&
        (*a0.20.upgrade().deref() == *a1.20.upgrade().deref()) &&
        (*a0.21.upgrade().deref() == *a1.21.upgrade().deref()) &&
        (*a0.22.upgrade().deref() == *a1.22.upgrade().deref()) &&
        (*a0.23.upgrade().deref() == *a1.23.upgrade().deref()) &&
        (*a0.24.upgrade().deref() == *a1.24.upgrade().deref()) &&
        (*a0.25.upgrade().deref() == *a1.25.upgrade().deref()) &&
        (*a0.26.upgrade().deref() == *a1.26.upgrade().deref()) &&
        (*a0.27.upgrade().deref() == *a1.27.upgrade().deref())
}

fn f366<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr, T26: PartialEq + ByteRepr, T27: PartialEq + ByteRepr, T28: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref()) ||
        (*a0.17.upgrade().deref() != *a1.17.upgrade().deref()) ||
        (*a0.18.upgrade().deref() != *a1.18.upgrade().deref()) ||
        (*a0.19.upgrade().deref() != *a1.19.upgrade().deref()) ||
        (*a0.20.upgrade().deref() != *a1.20.upgrade().deref()) ||
        (*a0.21.upgrade().deref() != *a1.21.upgrade().deref()) ||
        (*a0.22.upgrade().deref() != *a1.22.upgrade().deref()) ||
        (*a0.23.upgrade().deref() != *a1.23.upgrade().deref()) ||
        (*a0.24.upgrade().deref() != *a1.24.upgrade().deref()) ||
        (*a0.25.upgrade().deref() != *a1.25.upgrade().deref()) ||
        (*a0.26.upgrade().deref() != *a1.26.upgrade().deref()) ||
        (*a0.27.upgrade().deref() != *a1.27.upgrade().deref())
}

// --- std::tie, 29 elements --------------------------------------------------

fn t31<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f367<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
    a17: Ptr<T18>,
    a18: Ptr<T19>,
    a19: Ptr<T20>,
    a20: Ptr<T21>,
    a21: Ptr<T22>,
    a22: Ptr<T23>,
    a23: Ptr<T24>,
    a24: Ptr<T25>,
    a25: Ptr<T26>,
    a26: Ptr<T27>,
    a27: Ptr<T28>,
    a28: Ptr<T29>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28)
}

fn f368<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr, T26: PartialEq + ByteRepr, T27: PartialEq + ByteRepr, T28: PartialEq + ByteRepr, T29: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref()) &&
        (*a0.17.upgrade().deref() == *a1.17.upgrade().deref()) &&
        (*a0.18.upgrade().deref() == *a1.18.upgrade().deref()) &&
        (*a0.19.upgrade().deref() == *a1.19.upgrade().deref()) &&
        (*a0.20.upgrade().deref() == *a1.20.upgrade().deref()) &&
        (*a0.21.upgrade().deref() == *a1.21.upgrade().deref()) &&
        (*a0.22.upgrade().deref() == *a1.22.upgrade().deref()) &&
        (*a0.23.upgrade().deref() == *a1.23.upgrade().deref()) &&
        (*a0.24.upgrade().deref() == *a1.24.upgrade().deref()) &&
        (*a0.25.upgrade().deref() == *a1.25.upgrade().deref()) &&
        (*a0.26.upgrade().deref() == *a1.26.upgrade().deref()) &&
        (*a0.27.upgrade().deref() == *a1.27.upgrade().deref()) &&
        (*a0.28.upgrade().deref() == *a1.28.upgrade().deref())
}

fn f369<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr, T26: PartialEq + ByteRepr, T27: PartialEq + ByteRepr, T28: PartialEq + ByteRepr, T29: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref()) ||
        (*a0.17.upgrade().deref() != *a1.17.upgrade().deref()) ||
        (*a0.18.upgrade().deref() != *a1.18.upgrade().deref()) ||
        (*a0.19.upgrade().deref() != *a1.19.upgrade().deref()) ||
        (*a0.20.upgrade().deref() != *a1.20.upgrade().deref()) ||
        (*a0.21.upgrade().deref() != *a1.21.upgrade().deref()) ||
        (*a0.22.upgrade().deref() != *a1.22.upgrade().deref()) ||
        (*a0.23.upgrade().deref() != *a1.23.upgrade().deref()) ||
        (*a0.24.upgrade().deref() != *a1.24.upgrade().deref()) ||
        (*a0.25.upgrade().deref() != *a1.25.upgrade().deref()) ||
        (*a0.26.upgrade().deref() != *a1.26.upgrade().deref()) ||
        (*a0.27.upgrade().deref() != *a1.27.upgrade().deref()) ||
        (*a0.28.upgrade().deref() != *a1.28.upgrade().deref())
}

// --- std::tie, 30 elements --------------------------------------------------

fn t32<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f370<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
    a17: Ptr<T18>,
    a18: Ptr<T19>,
    a19: Ptr<T20>,
    a20: Ptr<T21>,
    a21: Ptr<T22>,
    a22: Ptr<T23>,
    a23: Ptr<T24>,
    a24: Ptr<T25>,
    a25: Ptr<T26>,
    a26: Ptr<T27>,
    a27: Ptr<T28>,
    a28: Ptr<T29>,
    a29: Ptr<T30>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29)
}

fn f371<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr, T26: PartialEq + ByteRepr, T27: PartialEq + ByteRepr, T28: PartialEq + ByteRepr, T29: PartialEq + ByteRepr, T30: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref()) &&
        (*a0.17.upgrade().deref() == *a1.17.upgrade().deref()) &&
        (*a0.18.upgrade().deref() == *a1.18.upgrade().deref()) &&
        (*a0.19.upgrade().deref() == *a1.19.upgrade().deref()) &&
        (*a0.20.upgrade().deref() == *a1.20.upgrade().deref()) &&
        (*a0.21.upgrade().deref() == *a1.21.upgrade().deref()) &&
        (*a0.22.upgrade().deref() == *a1.22.upgrade().deref()) &&
        (*a0.23.upgrade().deref() == *a1.23.upgrade().deref()) &&
        (*a0.24.upgrade().deref() == *a1.24.upgrade().deref()) &&
        (*a0.25.upgrade().deref() == *a1.25.upgrade().deref()) &&
        (*a0.26.upgrade().deref() == *a1.26.upgrade().deref()) &&
        (*a0.27.upgrade().deref() == *a1.27.upgrade().deref()) &&
        (*a0.28.upgrade().deref() == *a1.28.upgrade().deref()) &&
        (*a0.29.upgrade().deref() == *a1.29.upgrade().deref())
}

fn f372<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr, T26: PartialEq + ByteRepr, T27: PartialEq + ByteRepr, T28: PartialEq + ByteRepr, T29: PartialEq + ByteRepr, T30: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref()) ||
        (*a0.17.upgrade().deref() != *a1.17.upgrade().deref()) ||
        (*a0.18.upgrade().deref() != *a1.18.upgrade().deref()) ||
        (*a0.19.upgrade().deref() != *a1.19.upgrade().deref()) ||
        (*a0.20.upgrade().deref() != *a1.20.upgrade().deref()) ||
        (*a0.21.upgrade().deref() != *a1.21.upgrade().deref()) ||
        (*a0.22.upgrade().deref() != *a1.22.upgrade().deref()) ||
        (*a0.23.upgrade().deref() != *a1.23.upgrade().deref()) ||
        (*a0.24.upgrade().deref() != *a1.24.upgrade().deref()) ||
        (*a0.25.upgrade().deref() != *a1.25.upgrade().deref()) ||
        (*a0.26.upgrade().deref() != *a1.26.upgrade().deref()) ||
        (*a0.27.upgrade().deref() != *a1.27.upgrade().deref()) ||
        (*a0.28.upgrade().deref() != *a1.28.upgrade().deref()) ||
        (*a0.29.upgrade().deref() != *a1.29.upgrade().deref())
}

// --- std::tie, 31 elements --------------------------------------------------

fn t33<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>, Ptr<T31>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f373<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
    a17: Ptr<T18>,
    a18: Ptr<T19>,
    a19: Ptr<T20>,
    a20: Ptr<T21>,
    a21: Ptr<T22>,
    a22: Ptr<T23>,
    a23: Ptr<T24>,
    a24: Ptr<T25>,
    a25: Ptr<T26>,
    a26: Ptr<T27>,
    a27: Ptr<T28>,
    a28: Ptr<T29>,
    a29: Ptr<T30>,
    a30: Ptr<T31>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>, Ptr<T31>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30)
}

fn f374<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr, T26: PartialEq + ByteRepr, T27: PartialEq + ByteRepr, T28: PartialEq + ByteRepr, T29: PartialEq + ByteRepr, T30: PartialEq + ByteRepr, T31: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>, Ptr<T31>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>, Ptr<T31>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref()) &&
        (*a0.17.upgrade().deref() == *a1.17.upgrade().deref()) &&
        (*a0.18.upgrade().deref() == *a1.18.upgrade().deref()) &&
        (*a0.19.upgrade().deref() == *a1.19.upgrade().deref()) &&
        (*a0.20.upgrade().deref() == *a1.20.upgrade().deref()) &&
        (*a0.21.upgrade().deref() == *a1.21.upgrade().deref()) &&
        (*a0.22.upgrade().deref() == *a1.22.upgrade().deref()) &&
        (*a0.23.upgrade().deref() == *a1.23.upgrade().deref()) &&
        (*a0.24.upgrade().deref() == *a1.24.upgrade().deref()) &&
        (*a0.25.upgrade().deref() == *a1.25.upgrade().deref()) &&
        (*a0.26.upgrade().deref() == *a1.26.upgrade().deref()) &&
        (*a0.27.upgrade().deref() == *a1.27.upgrade().deref()) &&
        (*a0.28.upgrade().deref() == *a1.28.upgrade().deref()) &&
        (*a0.29.upgrade().deref() == *a1.29.upgrade().deref()) &&
        (*a0.30.upgrade().deref() == *a1.30.upgrade().deref())
}

fn f375<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr, T26: PartialEq + ByteRepr, T27: PartialEq + ByteRepr, T28: PartialEq + ByteRepr, T29: PartialEq + ByteRepr, T30: PartialEq + ByteRepr, T31: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>, Ptr<T31>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>, Ptr<T31>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref()) ||
        (*a0.17.upgrade().deref() != *a1.17.upgrade().deref()) ||
        (*a0.18.upgrade().deref() != *a1.18.upgrade().deref()) ||
        (*a0.19.upgrade().deref() != *a1.19.upgrade().deref()) ||
        (*a0.20.upgrade().deref() != *a1.20.upgrade().deref()) ||
        (*a0.21.upgrade().deref() != *a1.21.upgrade().deref()) ||
        (*a0.22.upgrade().deref() != *a1.22.upgrade().deref()) ||
        (*a0.23.upgrade().deref() != *a1.23.upgrade().deref()) ||
        (*a0.24.upgrade().deref() != *a1.24.upgrade().deref()) ||
        (*a0.25.upgrade().deref() != *a1.25.upgrade().deref()) ||
        (*a0.26.upgrade().deref() != *a1.26.upgrade().deref()) ||
        (*a0.27.upgrade().deref() != *a1.27.upgrade().deref()) ||
        (*a0.28.upgrade().deref() != *a1.28.upgrade().deref()) ||
        (*a0.29.upgrade().deref() != *a1.29.upgrade().deref()) ||
        (*a0.30.upgrade().deref() != *a1.30.upgrade().deref())
}

// --- std::tie, 32 elements --------------------------------------------------

fn t34<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32>() -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>, Ptr<T31>, Ptr<T32>) {
    (Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null(), Ptr::null())
}

fn f376<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32>(
    a0: Ptr<T1>,
    a1: Ptr<T2>,
    a2: Ptr<T3>,
    a3: Ptr<T4>,
    a4: Ptr<T5>,
    a5: Ptr<T6>,
    a6: Ptr<T7>,
    a7: Ptr<T8>,
    a8: Ptr<T9>,
    a9: Ptr<T10>,
    a10: Ptr<T11>,
    a11: Ptr<T12>,
    a12: Ptr<T13>,
    a13: Ptr<T14>,
    a14: Ptr<T15>,
    a15: Ptr<T16>,
    a16: Ptr<T17>,
    a17: Ptr<T18>,
    a18: Ptr<T19>,
    a19: Ptr<T20>,
    a20: Ptr<T21>,
    a21: Ptr<T22>,
    a22: Ptr<T23>,
    a23: Ptr<T24>,
    a24: Ptr<T25>,
    a25: Ptr<T26>,
    a26: Ptr<T27>,
    a27: Ptr<T28>,
    a28: Ptr<T29>,
    a29: Ptr<T30>,
    a30: Ptr<T31>,
    a31: Ptr<T32>,
) -> (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>, Ptr<T31>, Ptr<T32>) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31)
}

fn f377<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr, T26: PartialEq + ByteRepr, T27: PartialEq + ByteRepr, T28: PartialEq + ByteRepr, T29: PartialEq + ByteRepr, T30: PartialEq + ByteRepr, T31: PartialEq + ByteRepr, T32: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>, Ptr<T31>, Ptr<T32>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>, Ptr<T31>, Ptr<T32>),
) -> bool {
    (*a0.0.upgrade().deref() == *a1.0.upgrade().deref()) &&
        (*a0.1.upgrade().deref() == *a1.1.upgrade().deref()) &&
        (*a0.2.upgrade().deref() == *a1.2.upgrade().deref()) &&
        (*a0.3.upgrade().deref() == *a1.3.upgrade().deref()) &&
        (*a0.4.upgrade().deref() == *a1.4.upgrade().deref()) &&
        (*a0.5.upgrade().deref() == *a1.5.upgrade().deref()) &&
        (*a0.6.upgrade().deref() == *a1.6.upgrade().deref()) &&
        (*a0.7.upgrade().deref() == *a1.7.upgrade().deref()) &&
        (*a0.8.upgrade().deref() == *a1.8.upgrade().deref()) &&
        (*a0.9.upgrade().deref() == *a1.9.upgrade().deref()) &&
        (*a0.10.upgrade().deref() == *a1.10.upgrade().deref()) &&
        (*a0.11.upgrade().deref() == *a1.11.upgrade().deref()) &&
        (*a0.12.upgrade().deref() == *a1.12.upgrade().deref()) &&
        (*a0.13.upgrade().deref() == *a1.13.upgrade().deref()) &&
        (*a0.14.upgrade().deref() == *a1.14.upgrade().deref()) &&
        (*a0.15.upgrade().deref() == *a1.15.upgrade().deref()) &&
        (*a0.16.upgrade().deref() == *a1.16.upgrade().deref()) &&
        (*a0.17.upgrade().deref() == *a1.17.upgrade().deref()) &&
        (*a0.18.upgrade().deref() == *a1.18.upgrade().deref()) &&
        (*a0.19.upgrade().deref() == *a1.19.upgrade().deref()) &&
        (*a0.20.upgrade().deref() == *a1.20.upgrade().deref()) &&
        (*a0.21.upgrade().deref() == *a1.21.upgrade().deref()) &&
        (*a0.22.upgrade().deref() == *a1.22.upgrade().deref()) &&
        (*a0.23.upgrade().deref() == *a1.23.upgrade().deref()) &&
        (*a0.24.upgrade().deref() == *a1.24.upgrade().deref()) &&
        (*a0.25.upgrade().deref() == *a1.25.upgrade().deref()) &&
        (*a0.26.upgrade().deref() == *a1.26.upgrade().deref()) &&
        (*a0.27.upgrade().deref() == *a1.27.upgrade().deref()) &&
        (*a0.28.upgrade().deref() == *a1.28.upgrade().deref()) &&
        (*a0.29.upgrade().deref() == *a1.29.upgrade().deref()) &&
        (*a0.30.upgrade().deref() == *a1.30.upgrade().deref()) &&
        (*a0.31.upgrade().deref() == *a1.31.upgrade().deref())
}

fn f378<T1: PartialEq + ByteRepr, T2: PartialEq + ByteRepr, T3: PartialEq + ByteRepr, T4: PartialEq + ByteRepr, T5: PartialEq + ByteRepr, T6: PartialEq + ByteRepr, T7: PartialEq + ByteRepr, T8: PartialEq + ByteRepr, T9: PartialEq + ByteRepr, T10: PartialEq + ByteRepr, T11: PartialEq + ByteRepr, T12: PartialEq + ByteRepr, T13: PartialEq + ByteRepr, T14: PartialEq + ByteRepr, T15: PartialEq + ByteRepr, T16: PartialEq + ByteRepr, T17: PartialEq + ByteRepr, T18: PartialEq + ByteRepr, T19: PartialEq + ByteRepr, T20: PartialEq + ByteRepr, T21: PartialEq + ByteRepr, T22: PartialEq + ByteRepr, T23: PartialEq + ByteRepr, T24: PartialEq + ByteRepr, T25: PartialEq + ByteRepr, T26: PartialEq + ByteRepr, T27: PartialEq + ByteRepr, T28: PartialEq + ByteRepr, T29: PartialEq + ByteRepr, T30: PartialEq + ByteRepr, T31: PartialEq + ByteRepr, T32: PartialEq + ByteRepr>(
    a0: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>, Ptr<T31>, Ptr<T32>),
    a1: (Ptr<T1>, Ptr<T2>, Ptr<T3>, Ptr<T4>, Ptr<T5>, Ptr<T6>, Ptr<T7>, Ptr<T8>, Ptr<T9>, Ptr<T10>, Ptr<T11>, Ptr<T12>, Ptr<T13>, Ptr<T14>, Ptr<T15>, Ptr<T16>, Ptr<T17>, Ptr<T18>, Ptr<T19>, Ptr<T20>, Ptr<T21>, Ptr<T22>, Ptr<T23>, Ptr<T24>, Ptr<T25>, Ptr<T26>, Ptr<T27>, Ptr<T28>, Ptr<T29>, Ptr<T30>, Ptr<T31>, Ptr<T32>),
) -> bool {
    (*a0.0.upgrade().deref() != *a1.0.upgrade().deref()) ||
        (*a0.1.upgrade().deref() != *a1.1.upgrade().deref()) ||
        (*a0.2.upgrade().deref() != *a1.2.upgrade().deref()) ||
        (*a0.3.upgrade().deref() != *a1.3.upgrade().deref()) ||
        (*a0.4.upgrade().deref() != *a1.4.upgrade().deref()) ||
        (*a0.5.upgrade().deref() != *a1.5.upgrade().deref()) ||
        (*a0.6.upgrade().deref() != *a1.6.upgrade().deref()) ||
        (*a0.7.upgrade().deref() != *a1.7.upgrade().deref()) ||
        (*a0.8.upgrade().deref() != *a1.8.upgrade().deref()) ||
        (*a0.9.upgrade().deref() != *a1.9.upgrade().deref()) ||
        (*a0.10.upgrade().deref() != *a1.10.upgrade().deref()) ||
        (*a0.11.upgrade().deref() != *a1.11.upgrade().deref()) ||
        (*a0.12.upgrade().deref() != *a1.12.upgrade().deref()) ||
        (*a0.13.upgrade().deref() != *a1.13.upgrade().deref()) ||
        (*a0.14.upgrade().deref() != *a1.14.upgrade().deref()) ||
        (*a0.15.upgrade().deref() != *a1.15.upgrade().deref()) ||
        (*a0.16.upgrade().deref() != *a1.16.upgrade().deref()) ||
        (*a0.17.upgrade().deref() != *a1.17.upgrade().deref()) ||
        (*a0.18.upgrade().deref() != *a1.18.upgrade().deref()) ||
        (*a0.19.upgrade().deref() != *a1.19.upgrade().deref()) ||
        (*a0.20.upgrade().deref() != *a1.20.upgrade().deref()) ||
        (*a0.21.upgrade().deref() != *a1.21.upgrade().deref()) ||
        (*a0.22.upgrade().deref() != *a1.22.upgrade().deref()) ||
        (*a0.23.upgrade().deref() != *a1.23.upgrade().deref()) ||
        (*a0.24.upgrade().deref() != *a1.24.upgrade().deref()) ||
        (*a0.25.upgrade().deref() != *a1.25.upgrade().deref()) ||
        (*a0.26.upgrade().deref() != *a1.26.upgrade().deref()) ||
        (*a0.27.upgrade().deref() != *a1.27.upgrade().deref()) ||
        (*a0.28.upgrade().deref() != *a1.28.upgrade().deref()) ||
        (*a0.29.upgrade().deref() != *a1.29.upgrade().deref()) ||
        (*a0.30.upgrade().deref() != *a1.30.upgrade().deref()) ||
        (*a0.31.upgrade().deref() != *a1.31.upgrade().deref())
}

// --- 5-element tuples -------------------------------------------------------

fn f379<T1: Default, T2: Default, T3: Default, T4: Default, T5: Default>() -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>) {
    (
        Rc::new(RefCell::new(T1::default())),
        Rc::new(RefCell::new(T2::default())),
        Rc::new(RefCell::new(T3::default())),
        Rc::new(RefCell::new(T4::default())),
        Rc::new(RefCell::new(T5::default())),
    )
}

fn f380<T1, T2, T3, T4, T5>(a0: T1, a1: T2, a2: T3, a3: T4, a4: T5) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>) {
    (
        Rc::new(RefCell::new(a0.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a2.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a3.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(a4.try_into().expect("failed conversion"))),
    )
}

fn f381<T1: DeepClone, T2: DeepClone, T3: DeepClone, T4: DeepClone, T5: DeepClone>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>) {
    a0.deep_clone()
}

fn f382<T1: Default, T2: Default, T3: Default, T4: Default, T5: Default>(a0: &mut (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)) -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>) {
    std::mem::take(&mut *a0)
}

fn f383<T1: DeepClone + ByteRepr, T2: DeepClone + ByteRepr, T3: DeepClone + ByteRepr, T4: DeepClone + ByteRepr, T5: DeepClone + ByteRepr>(a0: Ptr<(Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)>, a1: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)) {
    a0.write(a1.deep_clone())
}

fn f384<T1: Default + ByteRepr, T2: Default + ByteRepr, T3: Default + ByteRepr, T4: Default + ByteRepr, T5: Default + ByteRepr>(a0: Ptr<(Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)>, a1: &mut (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)) {
    a0.write(std::mem::take(&mut *a1))
}

fn f385<T1, T2, T3, T4, T5>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)) -> Ptr<T1> {
    (a0.0.as_pointer() as Ptr<T1>)
}

fn f386<T1, T2, T3, T4, T5>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)) -> Ptr<T2> {
    (a0.1.as_pointer() as Ptr<T2>)
}

fn f387<T1, T2, T3, T4, T5>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)) -> Ptr<T3> {
    (a0.2.as_pointer() as Ptr<T3>)
}

fn f388<T1, T2, T3, T4, T5>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)) -> Ptr<T4> {
    (a0.3.as_pointer() as Ptr<T4>)
}

fn f389<T1, T2, T3, T4, T5>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)) -> Ptr<T5> {
    (a0.4.as_pointer() as Ptr<T5>)
}

fn f390<T1, T2, T3, T4, T5>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)) -> Ptr<T1> {
    (a0.0.as_pointer() as Ptr<T1>)
}

fn f391<T1, T2, T3, T4, T5>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)) -> Ptr<T2> {
    (a0.1.as_pointer() as Ptr<T2>)
}

fn f392<T1, T2, T3, T4, T5>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)) -> Ptr<T3> {
    (a0.2.as_pointer() as Ptr<T3>)
}

fn f393<T1, T2, T3, T4, T5>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)) -> Ptr<T4> {
    (a0.3.as_pointer() as Ptr<T4>)
}

fn f394<T1, T2, T3, T4, T5>(a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)) -> Ptr<T5> {
    (a0.4.as_pointer() as Ptr<T5>)
}

fn f395<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq>(
    a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>),
    a1: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>),
) -> bool {
    a0 == a1
}

fn f396<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq>(
    a0: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>),
    a1: (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>),
) -> bool {
    a0 != a1
}

// See tgt_unsafe.rs: src.cpp's `t35` needs a target entry in BOTH files.
fn t35<T1: Default, T2: Default, T3: Default, T4: Default, T5: Default>() -> (Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>) {
    <(Value<T1>, Value<T2>, Value<T3>, Value<T4>, Value<T5>)>::default()
}

// std::__ignore_type -- see src.cpp and tgt_unsafe.rs.
fn t36() -> () {
    ()
}

fn f397() -> Ptr<()> {
    Ptr::alloc(())
}

fn f398<T1>(a0: Ptr<()>, a1: T1) -> Ptr<()> {
    a0
}
