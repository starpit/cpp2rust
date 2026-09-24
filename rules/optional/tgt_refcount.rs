// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::optional<T> -> Option<Value<T>>: the contained value lives behind an Rc
// so that value() / operator* / operator-> can hand out a Ptr (resp. a borrow)
// into it, the same way a std::vector element does.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn t1<T1>() -> Option<Value<T1>> {
    None
}

// --- construction ----------------------------------------------------------

fn f1<T1>() -> Option<Value<T1>> {
    None
}

fn f2<T1>() -> Option<Value<T1>> {
    None
}

fn f3<T1>(a0: T1) -> Option<Value<T1>> {
    Some(Rc::new(RefCell::new(a0)))
}

fn f4<T1>(a0: T1) -> Option<Value<T1>> {
    Some(Rc::new(RefCell::new(a0)))
}

fn f5<T1>(a0: T1) -> Option<Value<T1>> {
    Some(Rc::new(RefCell::new(a0)))
}

// COPY IS DEEP, RECURSIVELY.  The old body unwrapped exactly ONE Value layer
// (`Rc::new(RefCell::new(v.borrow().clone()))`), which is right for a scalar
// element and WRONG the moment the element is itself a container: the inner
// .clone() then copies Rc HANDLES and the copy ALIASES the original.  Measured
// against clang-built C++: `map<int,map<int,long>> b(a); b[1][2]=22` gave
// C++ a=11, refcount a=22.  DeepClone recurses, so it is correct at every depth.
fn f6<T1: DeepClone>(a0: Option<Value<T1>>) -> Option<Value<T1>> {
    a0.deep_clone()
}

fn f7<T1>(a0: &mut Option<Value<T1>>) -> Option<Value<T1>> {
    a0.take()
}

// --- assignment ------------------------------------------------------------

fn f8<T1: DeepClone + ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: Option<Value<T1>>) {
    a0.write(a1.deep_clone())
}

fn f9<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: &mut Option<Value<T1>>) {
    a0.write(a1.take())
}

fn f10<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>) {
    a0.write(None)
}

fn f11<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: T1) {
    a0.write(Some(Rc::new(RefCell::new(a1))))
}

fn f12<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: T1) {
    a0.write(Some(Rc::new(RefCell::new(a1))))
}

fn f13<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: T1) {
    a0.write(Some(Rc::new(RefCell::new(a1))))
}

// --- observers -------------------------------------------------------------

fn f15<T1>(a0: Option<Value<T1>>) -> bool {
    a0.is_some()
}

fn f16<T1>(a0: &mut Option<Value<T1>>) -> Ptr<T1> {
    a0.as_pointer()
}

fn f17<T1>(a0: Option<Value<T1>>) -> Ptr<T1> {
    a0.as_pointer()
}

fn f18<T1>(a0: &mut Option<Value<T1>>) -> Ptr<T1> {
    a0.as_pointer()
}

fn f19<T1>(a0: Option<Value<T1>>) -> Ptr<T1> {
    a0.as_pointer()
}

fn f20<T1>(a0: &Option<Value<T1>>) -> std::cell::Ref<'_, T1> {
    a0.as_ref().unwrap().borrow()
}

fn f21<T1>(a0: &Option<Value<T1>>) -> std::cell::Ref<'_, T1> {
    a0.as_ref().unwrap().borrow()
}

fn f22<T1: Clone>(a0: Option<Value<T1>>, a1: T1) -> T1 {
    a0.as_ref().map_or(a1, |v| v.borrow().clone())
}

fn f23<T1: Clone>(a0: Option<Value<T1>>, a1: T1) -> T1 {
    a0.as_ref().map_or(a1, |v| v.borrow().clone())
}

fn f24<T1: Clone>(a0: Option<Value<T1>>, a1: T1) -> T1 {
    a0.as_ref().map_or(a1, |v| v.borrow().clone())
}

// --- comparison ------------------------------------------------------------

fn f26<T1: PartialEq>(a0: Option<Value<T1>>, a1: Option<Value<T1>>) -> bool {
    a0 == a1
}

fn f27<T1: PartialEq>(a0: Option<Value<T1>>, a1: Option<Value<T1>>) -> bool {
    a0 != a1
}

fn f28<T1>(a0: Option<Value<T1>>) -> bool {
    a0.is_none()
}

fn f29<T1>(a0: Option<Value<T1>>) -> bool {
    a0.is_some()
}

// --- has_value()/reset() ---------------------------------------------------
// libc++ declares these two in private base classes of std::optional, so both
// the type rules and the expression rules name the base class (see src.cpp).

#[cfg(target_os = "macos")]
fn t2<T1>() -> Option<Value<T1>> {
    None
}

#[cfg(target_os = "macos")]
fn t3<T1>() -> Option<Value<T1>> {
    None
}

#[cfg(target_os = "macos")]
fn f14<T1>(a0: Option<Value<T1>>) -> bool {
    a0.is_some()
}

#[cfg(target_os = "macos")]
fn f25<T1>(a0: &mut Option<Value<T1>>) {
    *a0 = None
}
