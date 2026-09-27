// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// std::optional<T1> -> Option<T1>.  All three C++ types (optional and the two
// libc++ base class templates that actually hold has_value()/reset()) map to the
// same Rust type.  std::nullopt_t -> () : it is a tag, and every rule that takes
// one ignores it.
//
// `operator->` IS mapped, by f30/f31; see src.cpp.  Its body is the SAME text as
// f11/f12 (`operator*`) because the converter uses the rule's result as a PLACE.

fn t1<T1>() -> Option<T1> {
    None
}

fn t2<T1>() -> Option<T1> {
    None
}

fn t3<T1>() -> Option<T1> {
    None
}

fn t4() -> () {
    ()
}

unsafe fn f1<T1>() -> Option<T1> {
    None
}

unsafe fn f3<T1>(a0: ()) -> Option<T1> {
    None
}

unsafe fn f4<T1: Clone>(a0: &Option<T1>) -> Option<T1> {
    a0.clone()
}

unsafe fn f5<T1>(a0: &mut Option<T1>) -> Option<T1> {
    a0.take()
}

unsafe fn f6<T1>(a0: &mut Option<T1>, a1: ()) {
    *a0 = None
}

unsafe fn f7<T1>(a0: &mut Option<T1>, a1: T1) {
    *a0 = Some(a1)
}

unsafe fn f8<T1: Clone>(a0: &mut Option<T1>, a1: &Option<T1>) {
    *a0 = a1.clone()
}

unsafe fn f9<T1>(a0: &Option<T1>) -> bool {
    a0.is_some()
}

unsafe fn f10<T1>(a0: &Option<T1>) -> bool {
    a0.is_some()
}

unsafe fn f11<T1>(a0: &mut Option<T1>) -> &mut T1 {
    a0.as_mut().expect("bad optional access")
}

unsafe fn f12<T1>(a0: &Option<T1>) -> &T1 {
    a0.as_ref().expect("bad optional access")
}

unsafe fn f13<T1>(a0: &mut Option<T1>) -> &mut T1 {
    a0.as_mut().expect("bad optional access")
}

unsafe fn f14<T1>(a0: &Option<T1>) -> &T1 {
    a0.as_ref().expect("bad optional access")
}

unsafe fn f15<T1: Clone>(a0: &Option<T1>, a1: &mut T1) -> T1 {
    match a0.as_ref() {
        Some(v) => T1::clone(v),
        None => T1::clone(a1),
    }
}

unsafe fn f16<T1>(a0: &mut Option<T1>) {
    *a0 = None
}

unsafe fn f17<T1>(a0: &Option<T1>, a1: ()) -> bool {
    a0.is_none()
}

unsafe fn f18<T1>(a0: (), a1: &Option<T1>) -> bool {
    a1.is_none()
}

unsafe fn f19<T1>(a0: &Option<T1>, a1: ()) -> bool {
    a0.is_some()
}

unsafe fn f20<T1>(a0: (), a1: &Option<T1>) -> bool {
    a1.is_some()
}

unsafe fn f21<T1: PartialEq>(a0: &Option<T1>, a1: &Option<T1>) -> bool {
    a0 == a1
}

unsafe fn f22<T1: PartialEq>(a0: &Option<T1>, a1: &Option<T1>) -> bool {
    a0 != a1
}

unsafe fn f23<T1: PartialEq>(a0: &Option<T1>, a1: &T1) -> bool {
    match a0.as_ref() {
        Some(v) => v == a1,
        None => false,
    }
}

unsafe fn f24<T1: PartialEq>(a0: &Option<T1>, a1: &T1) -> bool {
    match a0.as_ref() {
        Some(v) => v != a1,
        None => true,
    }
}

unsafe fn f25<T1>(a0: &mut Option<T1>, a1: &mut Option<T1>) {
    *a0 = a1.take()
}

// operator-> : the converter treats the result as a PLACE of type T1, so this is
// the SAME text as f11/f12 (operator*), not a pointer.
unsafe fn f30<T1>(a0: &mut Option<T1>) -> &mut T1 {
    a0.as_mut().expect("bad optional access")
}

unsafe fn f31<T1>(a0: &Option<T1>) -> &T1 {
    a0.as_ref().expect("bad optional access")
}
