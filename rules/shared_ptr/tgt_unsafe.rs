// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::shared_ptr<T> is shared ownership, so it maps to an Rc in both models:
// Option<Value<T>> == Option<Rc<RefCell<T>>>, `None` being the null state.
// std::weak_ptr<T> is the matching non-owning handle: std::rc::Weak.
//
// RefCell and Weak are spelled out in full because the unsafe model's prelude
// only imports Rc; Value/Ptr come from libcc2rs, which both preludes import.

use libcc2rs::*;
use std::rc::Rc;

fn t1<T1>() -> Option<Value<T1>> {
    None
}

fn t2<T1>() -> Option<std::rc::Weak<std::cell::RefCell<T1>>> {
    None
}

// --- std::make_shared ------------------------------------------------------

unsafe fn f1<T1: Default>() -> Option<Value<T1>> {
    Some(Rc::new(std::cell::RefCell::new(<T1>::default())))
}

unsafe fn f2<T1>(a0: T1) -> Option<Value<T1>> {
    Some(Rc::new(std::cell::RefCell::new(a0)))
}

unsafe fn f3<T1>(a0: T1) -> Option<Value<T1>> {
    Some(Rc::new(std::cell::RefCell::new(a0)))
}

unsafe fn f4<T1>(a0: T1) -> Option<Value<T1>> {
    Some(Rc::new(std::cell::RefCell::new(a0)))
}

// Rust has no variadic generics, so the two-argument form has to name a way to
// build T1 out of its arguments; `From<(T2, T3)>` is the only generic one.
unsafe fn f5<T1: From<(T2, T3)>, T2, T3>(a0: T2, a1: T3) -> Option<Value<T1>> {
    Some(Rc::new(std::cell::RefCell::new(<T1>::from((a0, a1)))))
}

unsafe fn f6<T1: From<(T2, T3)>, T2, T3>(a0: T2, a1: T3) -> Option<Value<T1>> {
    Some(Rc::new(std::cell::RefCell::new(<T1>::from((a0, a1)))))
}

unsafe fn f7<T1: From<(T2, T3)>, T2, T3>(a0: T2, a1: T3) -> Option<Value<T1>> {
    Some(Rc::new(std::cell::RefCell::new(<T1>::from((a0, a1)))))
}

// --- construction ----------------------------------------------------------

unsafe fn f8<T1>() -> Option<Value<T1>> {
    None
}

unsafe fn f9<T1>(a0: *mut T1) -> Option<Value<T1>> {
    if a0.is_null() {
        None
    } else {
        Some(Rc::new(std::cell::RefCell::new(unsafe { *Box::from_raw(a0) })))
    }
}

unsafe fn f10<T1>(a0: Option<Value<T1>>) -> Option<Value<T1>> {
    a0.clone()
}

unsafe fn f11<T1>(a0: &mut Option<Value<T1>>) -> Option<Value<T1>> {
    a0.take()
}

// --- assignment ------------------------------------------------------------

unsafe fn f12<T1>(a0: &mut Option<Value<T1>>, a1: Option<Value<T1>>) {
    *a0 = a1.clone()
}

unsafe fn f13<T1>(a0: &mut Option<Value<T1>>, a1: &mut Option<Value<T1>>) {
    *a0 = a1.take()
}

// --- observers -------------------------------------------------------------

unsafe fn f14<T1>(a0: Option<Value<T1>>) -> *mut T1 {
    a0.as_ref()
        .map_or(::std::ptr::null_mut(), |rc| {
            std::cell::RefCell::as_ptr(&**rc)
        })
}

// In this model the converter wraps operator-> in `(*...)`, so a raw pointer
// is what it wants; the refcount overlay needs a borrow instead.
unsafe fn f15<T1>(a0: Option<Value<T1>>) -> *mut T1 {
    a0.as_ref()
        .map_or(::std::ptr::null_mut(), |rc| {
            std::cell::RefCell::as_ptr(&**rc)
        })
}

unsafe fn f16<T1>(a0: Option<Value<T1>>) -> *mut T1 {
    a0.as_ref()
        .map_or(::std::ptr::null_mut(), |rc| {
            std::cell::RefCell::as_ptr(&**rc)
        })
}

unsafe fn f17<T1>(a0: Option<Value<T1>>) -> i64 {
    a0.as_ref().map_or(0, |rc| Rc::strong_count(rc) as i64)
}

unsafe fn f18<T1>(a0: Option<Value<T1>>) -> bool {
    a0.is_some()
}

// --- modifiers -------------------------------------------------------------

unsafe fn f19<T1>(a0: &mut Option<Value<T1>>) {
    *a0 = None
}

unsafe fn f20<T1>(a0: &mut Option<Value<T1>>, a1: *mut T1) {
    *a0 = if a1.is_null() {
        None
    } else {
        Some(Rc::new(std::cell::RefCell::new(unsafe { *Box::from_raw(a1) })))
    }
}

// --- comparison ------------------------------------------------------------
// shared_ptr comparison is identity, not value equality: compare the addresses
// the two handles point at (a disengaged handle yields a null Ptr).

unsafe fn f21<T1>(a0: Option<Value<T1>>, a1: Option<Value<T1>>) -> bool {
    a0.as_pointer() == a1.as_pointer()
}

unsafe fn f22<T1>(a0: Option<Value<T1>>, a1: Option<Value<T1>>) -> bool {
    a0.as_pointer() != a1.as_pointer()
}

unsafe fn f23<T1>(a0: Option<Value<T1>>) -> bool {
    a0.is_none()
}

unsafe fn f24<T1>(a0: Option<Value<T1>>) -> bool {
    a0.is_some()
}

// --- std::weak_ptr ---------------------------------------------------------

unsafe fn f25<T1>() -> Option<std::rc::Weak<std::cell::RefCell<T1>>> {
    None
}

unsafe fn f26<T1>(a0: Option<Value<T1>>) -> Option<std::rc::Weak<std::cell::RefCell<T1>>> {
    a0.as_ref().map(Rc::downgrade)
}

unsafe fn f27<T1>(
    a0: Option<std::rc::Weak<std::cell::RefCell<T1>>>,
) -> Option<std::rc::Weak<std::cell::RefCell<T1>>> {
    a0.clone()
}

unsafe fn f28<T1>(
    a0: &mut Option<std::rc::Weak<std::cell::RefCell<T1>>>,
    a1: Option<Value<T1>>,
) {
    *a0 = a1.as_ref().map(Rc::downgrade)
}

unsafe fn f29<T1>(
    a0: &mut Option<std::rc::Weak<std::cell::RefCell<T1>>>,
    a1: Option<std::rc::Weak<std::cell::RefCell<T1>>>,
) {
    *a0 = a1.clone()
}

unsafe fn f30<T1>(a0: Option<std::rc::Weak<std::cell::RefCell<T1>>>) -> Option<Value<T1>> {
    a0.as_ref().and_then(|w| w.upgrade())
}

unsafe fn f31<T1>(a0: Option<std::rc::Weak<std::cell::RefCell<T1>>>) -> bool {
    a0.as_ref().map_or(true, |w| w.strong_count() == 0)
}

unsafe fn f32<T1>(a0: Option<std::rc::Weak<std::cell::RefCell<T1>>>) -> i64 {
    a0.as_ref().map_or(0, |w| w.strong_count() as i64)
}

unsafe fn f33<T1>(a0: &mut Option<std::rc::Weak<std::cell::RefCell<T1>>>) {
    *a0 = None
}
