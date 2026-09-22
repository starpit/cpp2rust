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

// --- std::shared_ptr<T[]> and std::shared_ptr<void> -------------------------
//
// t3 is a POINTER, not Option<Value<Box<[T1]>>>, and the reason is that this
// model cannot build the latter. `new T[n]` lowers here to
// `Box::leak(..).as_mut_ptr()`, i.e. a bare *mut T1 with the length thrown
// away, and the only way back to a length is malloc_usable_size -- which this
// pod measured as OVER-approximating for 18 of 21 sizes (n=1 reports 24,
// n=64 reports 72). A Box<[T1]> rebuilt from that has the wrong len(), so
// `.len()` and any bounds check on it would be quietly wrong. A pointer has
// no length to get wrong.
//
// t4 is the same *mut c_void the converter already emits for a plain `void *`
// field, so a ProgramFrame carrying a shared_ptr<void> and one carrying a
// void* have the same Rust shape, and the cast between them is a no-op rather
// than a conversion that could disagree.
//
// What this gives up, deliberately: the reference count. Nothing in the tree
// calls use_count() on either type, and the C++ owner of the array is the
// shared_ptr<void> stored in the frame -- so the faithful behaviour is "stays
// alive for the rest of the program", which is exactly what Box::leak already
// did before this rule existed. The array is LEAKED, not dropped. That is a
// resource bug, not a correctness one; the alternative (drop when the local
// handle dies) would leave the frame reading freed memory, which is the
// silent-wrongness class this project ranks worst.

fn t3<T1>() -> *mut T1 {
    ::std::ptr::null_mut()
}

fn t4() -> *mut ::libc::c_void {
    ::std::ptr::null_mut()
}

unsafe fn f34<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

unsafe fn f35<T1>() -> *mut T1 {
    ::std::ptr::null_mut::<T1>()
}

unsafe fn f36<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

unsafe fn f37<T1>(a0: *mut T1) -> *mut ::libc::c_void {
    a0 as *mut ::libc::c_void
}

unsafe fn f38(a0: *mut ::libc::c_void) -> *mut ::libc::c_void {
    a0
}

unsafe fn f39() -> *mut ::libc::c_void {
    ::std::ptr::null_mut()
}

unsafe fn f40(a0: *mut ::libc::c_void) -> bool {
    a0.is_null()
}

unsafe fn f41(a0: *mut ::libc::c_void) -> bool {
    !a0.is_null()
}

unsafe fn f42(a0: &mut *mut ::libc::c_void, a1: *mut ::libc::c_void) {
    *a0 = a1
}

unsafe fn f43(a0: &mut *mut ::libc::c_void, a1: *mut ::libc::c_void) {
    *a0 = a1
}
