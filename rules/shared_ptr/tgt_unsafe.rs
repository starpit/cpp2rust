// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use std::rc::Rc;

fn t1<T1>() -> Option<Rc<T1>> {
    None
}

unsafe fn f1<T1>(init: T1) -> Option<Rc<T1>> {
    Some(Rc::new(init))
}

unsafe fn f3<T1>(a0: &Option<Rc<T1>>) -> Option<Rc<T1>> {
    a0.clone()
}

unsafe fn f4<T1>() -> Option<Rc<T1>> {
    None
}

unsafe fn f5<T1>(a0: &mut Option<Rc<T1>>, a1: &mut Option<Rc<T1>>) {
    match a1.take() {
        Some(v) => {
            a0.replace(v);
        }
        None => {
            a0.take();
        }
    }
}

unsafe fn f6<T1>(a0: &mut Option<Rc<T1>>, a1: &Option<Rc<T1>>) {
    match a1.clone() {
        Some(v) => {
            a0.replace(v);
        }
        None => {
            a0.take();
        }
    }
}

unsafe fn f7<T1>(a0: &Option<Rc<T1>>) -> bool {
    a0.is_some()
}

unsafe fn f8<T1>(a0: &Option<Rc<T1>>) -> &mut T1 {
    &mut *(a0.as_ref().map_or(::std::ptr::null_mut(), |r| Rc::as_ptr(r) as *mut T1))
}

// operator-> : the converter uses the result as a PLACE, so this is the SAME text
// as f8 (operator*), not a pointer.
unsafe fn f9<T1>(a0: &Option<Rc<T1>>) -> &mut T1 {
    &mut *(a0.as_ref().map_or(::std::ptr::null_mut(), |r| Rc::as_ptr(r) as *mut T1))
}

unsafe fn f10<T1>(a0: &Option<Rc<T1>>) -> *mut T1 {
    a0.as_ref().map_or(::std::ptr::null_mut(), |r| Rc::as_ptr(r) as *mut T1)
}

unsafe fn f11<T1>(a0: &mut Option<Rc<T1>>) {
    a0.take();
}

unsafe fn f16<T1>(a0: &mut Option<Rc<T1>>) -> Option<Rc<T1>> {
    a0.take()
}

// f17/f18 -- `p == nullptr` / `p != nullptr`.  a1 is the `nullptr` literal,
// which carries no information and is deliberately unused.  The annotation on
// `let _: () = a1` is LOAD-BEARING: the converter inlines the nullptr literal as
// a BARE UNTYPED `Default::default()`, and `let _ = Default::default();` is
// error[E0790] on its own.  Measured: the probe that first ran these keys did NOT
// report E0790 only because f19's E0308 (below) had already tainted the same
// function body, and rustc suppresses unresolved-inference-variable errors in a
// body that already has an error.  Fix f19 and the E0790 surfaces.
unsafe fn f17<T1>(a0: &Option<Rc<T1>>, a1: ()) -> bool {
    let _: () = a1;
    a0.is_none()
}

unsafe fn f18<T1>(a0: &Option<Rc<T1>>, a1: ()) -> bool {
    let _: () = a1;
    a0.is_some()
}

// f19 -- shared_ptr's operator== compares the STORED POINTERS, so this is
// Rc::ptr_eq and not a comparison of the pointees.  Discriminator EXECUTED: two
// make_shared<S>(5) at distinct allocations compare UNEQUAL, a copy compares
// EQUAL.
//
// `match (a0, a1)` was WRONG and did not compile: a declared `&Option<..>`
// parameter constrains NOTHING because the body is INLINED and the converter
// substitutes its own by-value expression (`(*a.borrow())` in refcount), so the
// arms bound `x: Rc<..>` by value and `Rc::ptr_eq` wants `&Rc<..>`
// (error[E0308]).  `.as_ref()` works for BOTH a by-value and a by-reference
// substitution -- it takes `&self`, so it autorefs a place and auto-derefs a
// reference -- and yields `Option<&..>` either way.
unsafe fn f19<T1>(a0: &Option<Rc<T1>>, a1: &Option<Rc<T1>>) -> bool {
    match (a0.as_ref(), a1.as_ref()) {
        (None, None) => true,
        (Some(x), Some(y)) => Rc::ptr_eq(x, y),
        _ => false,
    }
}
