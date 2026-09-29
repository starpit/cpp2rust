// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// std::optional<T1> -> Option<Value<T1>>.  The payload is a PLACE so that
// operator* / value() can return a Ptr<T1> aliasing the stored object, exactly as
// rules/shared_ptr does.  Unlike shared_ptr, a COPY of an optional must DEEP-COPY
// the payload: std::optional owns its T by value.
//
// `operator->` IS mapped, by f30/f31; see src.cpp.  Its body is the SAME text as
// f11/f12 (`operator*`) because the converter uses the rule's result as a PLACE.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn t1<T1>() -> Option<Value<T1>> {
    None
}

fn t2<T1>() -> Option<Value<T1>> {
    None
}

fn t3<T1>() -> Option<Value<T1>> {
    None
}

fn t4() -> () {
    ()
}

fn f1<T1>() -> Option<Value<T1>> {
    None
}

fn f3<T1>(a0: ()) -> Option<Value<T1>> {
    None
}

fn f4<T1: Clone>(a0: &Option<Value<T1>>) -> Option<Value<T1>> {
    a0.as_ref().map(|v| {
        Rc::new(RefCell::new(<T1>::clone(&*v.borrow())))
    })
}

fn f5<T1>(a0: &mut Option<Value<T1>>) -> Option<Value<T1>> {
    a0.take()
}

fn f6<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: ()) {
    a0.write(None)
}

fn f7<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: T1) {
    a0.write(Some(Rc::new(RefCell::new(a1))))
}

fn f8<T1: Clone + ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: &Option<Value<T1>>) {
    a0.write(
        a1.as_ref().map(|v| {
            Rc::new(RefCell::new(<T1>::clone(&*v.borrow())))
        }),
    )
}

fn f9<T1>(a0: &Option<Value<T1>>) -> bool {
    a0.is_some()
}

fn f10<T1>(a0: &Option<Value<T1>>) -> bool {
    a0.is_some()
}

fn f11<T1>(a0: &Option<Value<T1>>) -> Ptr<T1> {
    Value::as_pointer(a0.as_ref().expect("bad optional access"))
}

fn f12<T1>(a0: &Option<Value<T1>>) -> Ptr<T1> {
    Value::as_pointer(a0.as_ref().expect("bad optional access"))
}

fn f13<T1>(a0: &Option<Value<T1>>) -> Ptr<T1> {
    Value::as_pointer(a0.as_ref().expect("bad optional access"))
}

fn f14<T1>(a0: &Option<Value<T1>>) -> Ptr<T1> {
    Value::as_pointer(a0.as_ref().expect("bad optional access"))
}

fn f15<T1: Clone + ByteRepr>(a0: &Option<Value<T1>>, a1: Ptr<T1>) -> T1 {
    match a0.as_ref() {
        Some(v) => <T1>::clone(&*v.borrow()),
        None => a1.read(),
    }
}

fn f16<T1>(a0: &mut Option<Value<T1>>) {
    a0.take();
}

fn f17<T1>(a0: &Option<Value<T1>>, a1: ()) -> bool {
    a0.is_none()
}

fn f18<T1>(a0: (), a1: &Option<Value<T1>>) -> bool {
    a1.is_none()
}

fn f19<T1>(a0: &Option<Value<T1>>, a1: ()) -> bool {
    a0.is_some()
}

fn f20<T1>(a0: (), a1: &Option<Value<T1>>) -> bool {
    a1.is_some()
}

fn f21<T1: PartialEq>(a0: &Option<Value<T1>>, a1: &Option<Value<T1>>) -> bool {
    match (a0.as_ref(), a1.as_ref()) {
        (Some(x), Some(y)) => *x.borrow() == *y.borrow(),
        (None, None) => true,
        _ => false,
    }
}

fn f22<T1: PartialEq>(a0: &Option<Value<T1>>, a1: &Option<Value<T1>>) -> bool {
    match (a0.as_ref(), a1.as_ref()) {
        (Some(x), Some(y)) => *x.borrow() != *y.borrow(),
        (None, None) => false,
        _ => true,
    }
}

fn f23<T1: PartialEq>(a0: &Option<Value<T1>>, a1: &T1) -> bool {
    match a0.as_ref() {
        Some(v) => *v.borrow() == *a1,
        None => false,
    }
}

fn f24<T1: PartialEq>(a0: &Option<Value<T1>>, a1: &T1) -> bool {
    match a0.as_ref() {
        Some(v) => *v.borrow() != *a1,
        None => true,
    }
}

fn f25<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: &mut Option<Value<T1>>) {
    a0.write(a1.take())
}

// operator-> : same text as f11/f12 (operator*).
fn f30<T1>(a0: &Option<Value<T1>>) -> Ptr<T1> {
    Value::as_pointer(a0.as_ref().expect("bad optional access"))
}

fn f31<T1>(a0: &Option<Value<T1>>) -> Ptr<T1> {
    Value::as_pointer(a0.as_ref().expect("bad optional access"))
}

// f32 -- `std::nullopt`. Byte-identical to tgt_unsafe.rs: `()` is a tag with no
// ownership, so there is nothing for the refcount model to re-shape. Restated because a
// module carrying a tgt_refcount.rs must answer for EVERY key or the converter aborts at
// load time in that model.
fn f32() -> () {
    ()
}

// f33 -- value_or(T1 &&).  Prvalue default: taken by value, returned by value.  The
// present value is read out of the shared cell by CLONE, never moved.
fn f33<T1: Clone + ByteRepr>(a0: &Option<Value<T1>>, a1: T1) -> T1 {
    match a0.as_ref() {
        Some(v) => <T1>::clone(&*v.borrow()),
        None => a1,
    }
}

// f34 -- value_or(const T1 &).  Same shape as f15, whose `T1 &` parameter the refcount
// model also expands to `Ptr<T1>`.
fn f34<T1: Clone + ByteRepr>(a0: &Option<Value<T1>>, a1: Ptr<T1>) -> T1 {
    match a0.as_ref() {
        Some(v) => <T1>::clone(&*v.borrow()),
        None => a1.read(),
    }
}

// f35 / f36 / f37 -- heterogeneous comparisons against a bare value (rows g846,
// g790, g851).  The payload is a shared cell here, so the present case borrows it;
// the DISENGAGED case is answered without touching the payload at all.  An empty
// std::optional compares LESS than every value, hence false / true / false.
fn f35<T1: PartialEq<T2>, T2>(a0: &Option<Value<T1>>, a1: &T2) -> bool {
    match a0.as_ref() {
        Some(v) => *v.borrow() == *a1,
        None => false,
    }
}

fn f36<T1: PartialEq<T2>, T2>(a0: &T2, a1: &Option<Value<T1>>) -> bool {
    match a1.as_ref() {
        Some(v) => *v.borrow() != *a0,
        None => true,
    }
}

fn f37<T1: PartialOrd<T2>, T2>(a0: &Option<Value<T1>>, a1: &T2) -> bool {
    match a0.as_ref() {
        Some(v) => *v.borrow() >= *a1,
        None => false,
    }
}
