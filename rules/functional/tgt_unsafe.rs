// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1<T1>() -> *mut T1 {
    Default::default()
}

unsafe fn f1<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

unsafe fn f2<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

unsafe fn f3<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

fn t2<T1>() -> Option<Box<dyn Fn() -> T1>> {
    None
}

unsafe fn f4<T1>(a0: &Option<Box<dyn Fn() -> T1>>) -> T1 {
    (a0.as_ref().unwrap())()
}

unsafe fn f5<T1, T2: Fn() -> T1 + 'static>(a0: T2) -> Option<Box<dyn Fn() -> T1>> {
    Some(Box::new(a0))
}

fn t3<T1, T2>() -> Option<Box<dyn Fn(T2) -> T1>> {
    None
}

unsafe fn f6<T1, T2>(a0: &Option<Box<dyn Fn(T2) -> T1>>, a1: T2) -> T1 {
    (a0.as_ref().unwrap())(a1)
}

unsafe fn f7<T1, T2, T3: Fn(T2) -> T1 + 'static>(a0: T3) -> Option<Box<dyn Fn(T2) -> T1>> {
    Some(Box::new(a0))
}

fn t4<T1, T2, T3>() -> Option<Box<dyn Fn(T2, T3) -> T1>> {
    None
}

unsafe fn f8<T1, T2, T3>(a0: &Option<Box<dyn Fn(T2, T3) -> T1>>, a1: T2, a2: T3) -> T1 {
    (a0.as_ref().unwrap())(a1, a2)
}

unsafe fn f9<T1, T2, T3, T4: Fn(T2, T3) -> T1 + 'static>(a0: T4) -> Option<Box<dyn Fn(T2, T3) -> T1>> {
    Some(Box::new(a0))
}

fn t5<T1, T2, T3, T4>() -> Option<Box<dyn Fn(T2, T3, T4) -> T1>> {
    None
}

unsafe fn f10<T1, T2, T3, T4>(a0: &Option<Box<dyn Fn(T2, T3, T4) -> T1>>, a1: T2, a2: T3, a3: T4) -> T1 {
    (a0.as_ref().unwrap())(a1, a2, a3)
}

unsafe fn f11<T1, T2, T3, T4, T5: Fn(T2, T3, T4) -> T1 + 'static>(a0: T5) -> Option<Box<dyn Fn(T2, T3, T4) -> T1>> {
    Some(Box::new(a0))
}

fn t6<T1, T2, T3, T4, T5>() -> Option<Box<dyn Fn(T2, T3, T4, T5) -> T1>> {
    None
}

unsafe fn f12<T1, T2, T3, T4, T5>(a0: &Option<Box<dyn Fn(T2, T3, T4, T5) -> T1>>, a1: T2, a2: T3, a3: T4, a4: T5) -> T1 {
    (a0.as_ref().unwrap())(a1, a2, a3, a4)
}

unsafe fn f13<T1, T2, T3, T4, T5, T6: Fn(T2, T3, T4, T5) -> T1 + 'static>(a0: T6) -> Option<Box<dyn Fn(T2, T3, T4, T5) -> T1>> {
    Some(Box::new(a0))
}

fn t7<T1, T2, T3, T4, T5, T6>() -> Option<Box<dyn Fn(T2, T3, T4, T5, T6) -> T1>> {
    None
}

unsafe fn f14<T1, T2, T3, T4, T5, T6>(a0: &Option<Box<dyn Fn(T2, T3, T4, T5, T6) -> T1>>, a1: T2, a2: T3, a3: T4, a4: T5, a5: T6) -> T1 {
    (a0.as_ref().unwrap())(a1, a2, a3, a4, a5)
}

unsafe fn f15<T1, T2, T3, T4, T5, T6, T7: Fn(T2, T3, T4, T5, T6) -> T1 + 'static>(a0: T7) -> Option<Box<dyn Fn(T2, T3, T4, T5, T6) -> T1>> {
    Some(Box::new(a0))
}

fn t8<'a, T1>() -> Option<&'a (dyn Fn() -> T1 + 'a)> {
    None
}

fn t9<'a, T1, T2>() -> Option<&'a (dyn Fn(T2) -> T1 + 'a)> {
    None
}

fn t10<'a, T1, T2, T3>() -> Option<&'a (dyn Fn(T2, T3) -> T1 + 'a)> {
    None
}

fn t11<'a, T1, T2, T3, T4>() -> Option<&'a (dyn Fn(T2, T3, T4) -> T1 + 'a)> {
    None
}

fn t12<'a, T1, T2, T3, T4, T5>() -> Option<&'a (dyn Fn(T2, T3, T4, T5) -> T1 + 'a)> {
    None
}

fn t13<'a, T1, T2, T3, T4, T5, T6>() -> Option<&'a (dyn Fn(T2, T3, T4, T5, T6) -> T1 + 'a)> {
    None
}

// `llvm::function_ref<T1 ()>::function_ref(Callable &&, void *, void *)`.  See the
// refcount twin for why the Callable is a `&'a mut` borrow and not an owned value.
unsafe fn f16<'a, T1, T2: Fn() -> T1 + 'a>(a0: &'a mut T2, a1: *mut ::libc::c_void, a2: *mut ::libc::c_void) -> Option<&'a (dyn Fn() -> T1 + 'a)> {
    Some(&*a0)
}

// `T1 llvm::function_ref<T1 (..)>::operator()(..) const`, arities 0..5 -- the
// t8..t13 twins of f4/f6/f8/f10/f12/f14.  See src.cpp for the arity census, the
// swallow-tie argument, and why `unwrap` is the faithful rendering of calling an
// empty non-owning `function_ref` (C++ UB) rather than a papering-over.
unsafe fn f17<'a, T1>(a0: &Option<&'a (dyn Fn() -> T1 + 'a)>) -> T1 {
    (a0.unwrap())()
}

unsafe fn f18<'a, T1, T2>(a0: &Option<&'a (dyn Fn(T2) -> T1 + 'a)>, a1: T2) -> T1 {
    (a0.unwrap())(a1)
}

unsafe fn f19<'a, T1, T2, T3>(a0: &Option<&'a (dyn Fn(T2, T3) -> T1 + 'a)>, a1: T2, a2: T3) -> T1 {
    (a0.unwrap())(a1, a2)
}

unsafe fn f20<'a, T1, T2, T3, T4>(a0: &Option<&'a (dyn Fn(T2, T3, T4) -> T1 + 'a)>, a1: T2, a2: T3, a3: T4) -> T1 {
    (a0.unwrap())(a1, a2, a3)
}

unsafe fn f21<'a, T1, T2, T3, T4, T5>(a0: &Option<&'a (dyn Fn(T2, T3, T4, T5) -> T1 + 'a)>, a1: T2, a2: T3, a3: T4, a4: T5) -> T1 {
    (a0.unwrap())(a1, a2, a3, a4)
}

unsafe fn f22<'a, T1, T2, T3, T4, T5, T6>(a0: &Option<&'a (dyn Fn(T2, T3, T4, T5, T6) -> T1 + 'a)>, a1: T2, a2: T3, a3: T4, a4: T5, a5: T6) -> T1 {
    (a0.unwrap())(a1, a2, a3, a4, a5)
}
