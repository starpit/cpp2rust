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
