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

fn t2<T1>() -> ::std::marker::PhantomData<T1> {
    ::std::marker::PhantomData
}

unsafe fn f4<T1>() -> ::std::marker::PhantomData<T1> {
    ::std::marker::PhantomData
}

unsafe fn f5<T1: ::std::ops::Add<Output = T1>>(
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

unsafe fn f6<T1>() -> ::std::rc::Rc<dyn Fn() -> T1> {
    ::std::rc::Rc::new(|| ::std::unimplemented!())
}

unsafe fn f7<T1, T2: Fn() -> T1 + 'static>(a0: T2) -> ::std::rc::Rc<dyn Fn() -> T1> {
    ::std::rc::Rc::new(a0)
}

unsafe fn f8<T1>(a0: ::std::rc::Rc<dyn Fn() -> T1>) -> ::std::rc::Rc<dyn Fn() -> T1> {
    a0.clone()
}

unsafe fn f9<T1>(a0: ::std::rc::Rc<dyn Fn() -> T1>) -> T1 {
    (a0)()
}

unsafe fn f10<T1, T2: Fn() -> T1 + 'static>(
    a0: &mut ::std::rc::Rc<dyn Fn() -> T1>,
    a1: T2,
) {
    *a0 = ::std::rc::Rc::new(a1)
}

unsafe fn f11<T1>(
    a0: &mut ::std::rc::Rc<dyn Fn() -> T1>,
    a1: ::std::rc::Rc<dyn Fn() -> T1>,
) {
    *a0 = a1.clone()
}
