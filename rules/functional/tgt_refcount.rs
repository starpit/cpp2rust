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
