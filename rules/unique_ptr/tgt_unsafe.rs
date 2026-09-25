// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1<T1>() -> Option<Box<T1>> {
    None
}

fn t2<T1>() -> Option<Box<[T1]>> {
    None
}

unsafe fn f1<T1: Default>(a0: usize) -> Option<Box<[T1]>> {
    Some((0..a0).map(|_| <T1>::default()).collect::<Box<[_]>>())
}
unsafe fn f2<T1>(a0: &mut Option<Box<T1>>) -> *mut T1 {
    a0.as_deref_mut()
        .map_or(::std::ptr::null_mut(), |v| v as *mut T1)
}
unsafe fn f3<T1>(a0: *mut T1) -> Option<Box<T1>> {
    Some(Box::from_raw(a0))
}
unsafe fn f4<T1>(a0: *mut T1) -> Option<Box<T1>> {
    Some(Box::from_raw(a0))
}
unsafe fn f5<T1: Default>(a0: &mut Option<Box<T1>>, a1: *mut T1) {
    let _a0: *mut T1 = a1;
    *a0 = if _a0.is_null() {
        None
    } else {
        Some(Box::from_raw(_a0))
    }
}
unsafe fn f6<T1>(a0: &mut Option<Box<T1>>, a1: *mut T1) {
    *a0 = Some(Box::from_raw(a1))
}
unsafe fn f7<T1>(a0: &mut Option<Box<[T1]>>) -> *mut T1 {
    a0.as_deref_mut()
        .map_or(::std::ptr::null_mut(), |s| s.as_mut_ptr())
}

unsafe fn f8<T1>(init: T1) -> Option<Box<T1>> {
    Some(Box::new(init))
}

unsafe fn f9<T1>(a0: &mut Option<Box<[T1]>>) {
    *a0 = None
}

unsafe fn f10<T1: Default>() -> Option<Box<T1>> {
    None
}

unsafe fn f11<T1>() -> Option<Box<[T1]>> {
    None
}

unsafe fn f12<T1>(a0: &mut Option<Box<T1>>) -> Option<Box<T1>> {
    a0.take()
}

unsafe fn f13<T1>(a0: &mut Option<Box<[T1]>>) -> Option<Box<[T1]>> {
    a0.take()
}

unsafe fn f14<T1>(a0: &mut Option<Box<T1>>, a1: &mut Option<Box<T1>>) {
    *a0 = a1.take()
}

unsafe fn f15<T1>(a0: &mut Option<Box<[T1]>>, a1: &mut Option<Box<[T1]>>) {
    *a0 = a1.take()
}

unsafe fn f16<T1>(a0: &mut Option<Box<T1>>) -> *mut T1 {
    a0.take()
        .map_or(::std::ptr::null_mut(), |__b| Box::into_raw(__b))
}

unsafe fn f17<T1>() -> Option<Box<T1>> {
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
unsafe fn f18<T1>(a0: &Option<Box<T1>>) -> bool {
    a0.is_some()
}

unsafe fn f19<T1>(a0: &Option<Box<T1>>) -> bool {
    a0.is_none()
}

unsafe fn f20<T1>(a0: (), a1: &Option<Box<T1>>) -> bool {
    let () = a0;
    a1.is_some()
}

unsafe fn f21<T1>(a0: (), a1: &Option<Box<T1>>) -> bool {
    let () = a0;
    a1.is_none()
}
