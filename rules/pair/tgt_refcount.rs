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

fn f1<T1, T2>(a0: (Value<T1>, Value<T2>)) -> Value<T2> {
    a0.1
}

fn f2<T1: Clone, T2: Clone>(a0: (Value<T1>, Value<T2>)) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.0.borrow().clone())),
        Rc::new(RefCell::new(a0.1.borrow().clone())),
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

fn f11<T1, T2>(a0: (Value<T1>, Value<T2>)) -> Value<T1> {
    a0.0
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

fn f15<T1: TryFrom<Vec<u8>>, T2>(a0: Vec<u8>, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(
            <T1>::try_from({
                let mut __b = a0.to_vec();
                __b.push(0);
                __b
            })
            .ok()
            .expect("failed conversion"),
        )),
        Rc::new(RefCell::new(a1.try_into().ok().expect("failed conversion"))),
    )
}

fn f16<T1: TryFrom<Vec<u8>>, T2>(a0: Vec<u8>, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(
            <T1>::try_from({
                let mut __b = a0.to_vec();
                __b.push(0);
                __b
            })
            .ok()
            .expect("failed conversion"),
        )),
        Rc::new(RefCell::new(a1.try_into().ok().expect("failed conversion"))),
    )
}

fn f17<T1: PartialEq, T2: PartialEq>(
    a0: &(Value<T1>, Value<T2>),
    a1: &(Value<T1>, Value<T2>),
) -> bool {
    a0 == a1
}

fn f18<T1: PartialEq, T2: PartialEq>(
    a0: &(Value<T1>, Value<T2>),
    a1: &(Value<T1>, Value<T2>),
) -> bool {
    a0 != a1
}

fn f19<T1, T2: TryFrom<Vec<u8>>>(a0: T1, a1: Vec<u8>) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(a0.try_into().ok().expect("failed conversion"))),
        Rc::new(RefCell::new(
            <T2>::try_from({
                let mut __b = a1.to_vec();
                __b.push(0);
                __b
            })
            .ok()
            .expect("failed conversion"),
        )),
    )
}

fn f20<T1: TryFrom<Vec<u8>>, T2>(a0: Vec<u8>, a1: T2) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(
            <T1>::try_from({
                let mut __b = a0.to_vec();
                __b.push(0);
                __b
            })
            .ok()
            .expect("failed conversion"),
        )),
        Rc::new(RefCell::new(a1.try_into().ok().expect("failed conversion"))),
    )
}

// f21/f22 -- std::make_pair with an LVALUE first argument; same tuple
// construction as f9/f10.
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
