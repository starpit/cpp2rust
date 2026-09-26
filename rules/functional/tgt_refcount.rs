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

fn t2<T1>() -> ::std::marker::PhantomData<T1> {
    ::std::marker::PhantomData
}

fn f4<T1>() -> ::std::marker::PhantomData<T1> {
    ::std::marker::PhantomData
}

fn f5<T1: ::std::ops::Add<Output = T1>>(
    a0: ::std::marker::PhantomData<T1>,
    a1: T1,
    a2: T1,
) -> T1 {
    let __x = a1;
    let __y = a2;
    __x + __y
}

// --- std::function<T1()> ---------------------------------------------------

fn t3<T1>() -> ::std::rc::Rc<dyn Fn() -> T1> {
    ::std::rc::Rc::new(|| ::std::unimplemented!())
}

fn f6<T1>() -> ::std::rc::Rc<dyn Fn() -> T1> {
    ::std::rc::Rc::new(|| ::std::unimplemented!())
}

fn f7<T1, T2: Fn() -> T1 + 'static>(a0: T2) -> ::std::rc::Rc<dyn Fn() -> T1> {
    ::std::rc::Rc::new(a0)
}

fn f8<T1>(a0: ::std::rc::Rc<dyn Fn() -> T1>) -> ::std::rc::Rc<dyn Fn() -> T1> {
    a0.clone()
}

fn f9<T1>(a0: ::std::rc::Rc<dyn Fn() -> T1>) -> T1 {
    (a0)()
}

fn f10<T1, T2: Fn() -> T1 + 'static>(
    a0: &mut ::std::rc::Rc<dyn Fn() -> T1>,
    a1: T2,
) {
    *a0 = ::std::rc::Rc::new(a1)
}

fn f11<T1>(
    a0: &mut ::std::rc::Rc<dyn Fn() -> T1>,
    a1: ::std::rc::Rc<dyn Fn() -> T1>,
) {
    *a0 = a1.clone()
}
