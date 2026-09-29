// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

fn t1<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn f1<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f2<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f3<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn t2<T1>() -> Option<Box<dyn Fn() -> T1>> {
    None
}

fn f4<T1>(a0: &Option<Box<dyn Fn() -> T1>>) -> T1 {
    (a0.as_ref().unwrap())()
}

fn f5<T1, T2: Fn() -> T1 + 'static>(a0: T2) -> Option<Box<dyn Fn() -> T1>> {
    Some(Box::new(a0))
}

fn t3<T1, T2>() -> Option<Box<dyn Fn(T2) -> T1>> {
    None
}

fn f6<T1, T2>(a0: &Option<Box<dyn Fn(T2) -> T1>>, a1: T2) -> T1 {
    (a0.as_ref().unwrap())(a1)
}

fn f7<T1, T2, T3: Fn(T2) -> T1 + 'static>(a0: T3) -> Option<Box<dyn Fn(T2) -> T1>> {
    Some(Box::new(a0))
}

fn t4<T1, T2, T3>() -> Option<Box<dyn Fn(T2, T3) -> T1>> {
    None
}

fn f8<T1, T2, T3>(a0: &Option<Box<dyn Fn(T2, T3) -> T1>>, a1: T2, a2: T3) -> T1 {
    (a0.as_ref().unwrap())(a1, a2)
}

fn f9<T1, T2, T3, T4: Fn(T2, T3) -> T1 + 'static>(a0: T4) -> Option<Box<dyn Fn(T2, T3) -> T1>> {
    Some(Box::new(a0))
}

fn t5<T1, T2, T3, T4>() -> Option<Box<dyn Fn(T2, T3, T4) -> T1>> {
    None
}

fn f10<T1, T2, T3, T4>(a0: &Option<Box<dyn Fn(T2, T3, T4) -> T1>>, a1: T2, a2: T3, a3: T4) -> T1 {
    (a0.as_ref().unwrap())(a1, a2, a3)
}

fn f11<T1, T2, T3, T4, T5: Fn(T2, T3, T4) -> T1 + 'static>(a0: T5) -> Option<Box<dyn Fn(T2, T3, T4) -> T1>> {
    Some(Box::new(a0))
}

fn t6<T1, T2, T3, T4, T5>() -> Option<Box<dyn Fn(T2, T3, T4, T5) -> T1>> {
    None
}

fn f12<T1, T2, T3, T4, T5>(a0: &Option<Box<dyn Fn(T2, T3, T4, T5) -> T1>>, a1: T2, a2: T3, a3: T4, a4: T5) -> T1 {
    (a0.as_ref().unwrap())(a1, a2, a3, a4)
}

fn f13<T1, T2, T3, T4, T5, T6: Fn(T2, T3, T4, T5) -> T1 + 'static>(a0: T6) -> Option<Box<dyn Fn(T2, T3, T4, T5) -> T1>> {
    Some(Box::new(a0))
}

fn t7<T1, T2, T3, T4, T5, T6>() -> Option<Box<dyn Fn(T2, T3, T4, T5, T6) -> T1>> {
    None
}

fn f14<T1, T2, T3, T4, T5, T6>(a0: &Option<Box<dyn Fn(T2, T3, T4, T5, T6) -> T1>>, a1: T2, a2: T3, a3: T4, a4: T5, a5: T6) -> T1 {
    (a0.as_ref().unwrap())(a1, a2, a3, a4, a5)
}

fn f15<T1, T2, T3, T4, T5, T6, T7: Fn(T2, T3, T4, T5, T6) -> T1 + 'static>(a0: T7) -> Option<Box<dyn Fn(T2, T3, T4, T5, T6) -> T1>> {
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

// `llvm::function_ref<T1 ()>::function_ref(Callable &&, void *, void *)`.
// ⭐ `'a` GENUINELY LANDS IN A RETURN POSITION here, and it is constrainable only
// through the Callable parameter: the borrow must outlive the `function_ref`, so the
// Callable is taken as `&'a mut T2` and re-borrowed.  Ownership (`Some(Box::new(a0))`)
// is impossible for a BORROW model -- it would demand `'static` and an allocation.
// The two trailing SFINAE pointers are the defaulted `enable_if_t<...> *` params;
// `void *` is `AnyPtr` in the refcount model.
fn f16<'a, T1, T2: Fn() -> T1 + 'a>(a0: &'a mut T2, a1: AnyPtr, a2: AnyPtr) -> Option<&'a (dyn Fn() -> T1 + 'a)> {
    Some(&*a0)
}

// `T1 llvm::function_ref<T1 (..)>::operator()(..) const`, arities 0..5 -- the
// t8..t13 twins of f4/f6/f8/f10/f12/f14.  See src.cpp for the arity census, the
// swallow-tie argument, and why `unwrap` is the faithful rendering of calling an
// empty non-owning `function_ref` (C++ UB) rather than a papering-over.
fn f17<'a, T1>(a0: &Option<&'a (dyn Fn() -> T1 + 'a)>) -> T1 {
    (a0.unwrap())()
}

fn f18<'a, T1, T2>(a0: &Option<&'a (dyn Fn(T2) -> T1 + 'a)>, a1: T2) -> T1 {
    (a0.unwrap())(a1)
}

fn f19<'a, T1, T2, T3>(a0: &Option<&'a (dyn Fn(T2, T3) -> T1 + 'a)>, a1: T2, a2: T3) -> T1 {
    (a0.unwrap())(a1, a2)
}

fn f20<'a, T1, T2, T3, T4>(a0: &Option<&'a (dyn Fn(T2, T3, T4) -> T1 + 'a)>, a1: T2, a2: T3, a3: T4) -> T1 {
    (a0.unwrap())(a1, a2, a3)
}

fn f21<'a, T1, T2, T3, T4, T5>(a0: &Option<&'a (dyn Fn(T2, T3, T4, T5) -> T1 + 'a)>, a1: T2, a2: T3, a3: T4, a4: T5) -> T1 {
    (a0.unwrap())(a1, a2, a3, a4)
}

fn f22<'a, T1, T2, T3, T4, T5, T6>(a0: &Option<&'a (dyn Fn(T2, T3, T4, T5, T6) -> T1 + 'a)>, a1: T2, a2: T3, a3: T4, a4: T5, a5: T6) -> T1 {
    (a0.unwrap())(a1, a2, a3, a4, a5)
}
