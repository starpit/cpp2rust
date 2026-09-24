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

fn f3<T1: Default, T2: Default>() -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(T1::default())),
        Rc::new(RefCell::new(T2::default())),
    )
}

// COPY IS DEEP, RECURSIVELY.  The old body unwrapped exactly ONE Value layer
// (`Rc::new(RefCell::new(v.borrow().clone()))`), which is right for a scalar
// element and WRONG the moment the element is itself a container: the inner
// .clone() then copies Rc HANDLES and the copy ALIASES the original.  Measured
// against clang-built C++: `map<int,map<int,long>> b(a); b[1][2]=22` gave
// C++ a=11, refcount a=22.  DeepClone recurses, so it is correct at every depth.
fn f2<T1: DeepClone, T2: DeepClone>(a0: (Value<T1>, Value<T2>)) -> (Value<T1>, Value<T2>) {
    a0.deep_clone()
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

fn f15<T1: PartialEq, T2: PartialEq>(
    a0: (Value<T1>, Value<T2>),
    a1: (Value<T1>, Value<T2>),
) -> bool {
    a0 == a1
}

fn f16<T1: PartialEq, T2: PartialEq>(
    a0: (Value<T1>, Value<T2>),
    a1: (Value<T1>, Value<T2>),
) -> bool {
    a0 != a1
}

fn f18<T1: TryFrom<T3>, T2: FromIterator<u8>, T3: Clone>(
    a0: T3,
    a1: &'static [u8],
) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(
            T1::try_from(a0).ok().expect("failed conversion"),
        )),
        Rc::new(RefCell::new(
            a1.iter()
                .copied()
                .chain(std::iter::once(0))
                .collect::<T2>(),
        )),
    )
}

fn f19<T1: FromIterator<u8>, T2: TryFrom<T3>, T3: Clone>(
    a0: &'static [u8],
    a1: T3,
) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(
            a0.iter()
                .copied()
                .chain(std::iter::once(0))
                .collect::<T1>(),
        )),
        Rc::new(RefCell::new(
            T2::try_from(a1).ok().expect("failed conversion"),
        )),
    )
}

fn f20<T1: FromIterator<u8>, T2: TryFrom<T3>, T3: Clone>(
    a0: &'static [u8],
    a1: T3,
) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(
            a0.iter()
                .copied()
                .chain(std::iter::once(0))
                .collect::<T1>(),
        )),
        Rc::new(RefCell::new(
            T2::try_from(a1).ok().expect("failed conversion"),
        )),
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

fn f17<T1: TryFrom<T3>, T2: TryFrom<T4>, T3: Clone, T4: Clone>(
    a0: (Value<T3>, Value<T4>),
) -> (Value<T1>, Value<T2>) {
    (
        Rc::new(RefCell::new(
            T1::try_from(a0.0.borrow().clone()).ok().expect("failed conversion"),
        )),
        Rc::new(RefCell::new(
            T2::try_from(a0.1.borrow().clone()).ok().expect("failed conversion"),
        )),
    )
}
