// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::{Variant2, Variant3};

fn t1<T1: Default, T2>() -> Variant2<T1, T2> {
    Variant2::V0(T1::default())
}

fn t2<T1: Default, T2, T3>() -> Variant3<T1, T2, T3> {
    Variant3::V0(T1::default())
}

unsafe fn f1<T1: Default, T2>() -> Variant2<T1, T2> {
    Variant2::V0(T1::default())
}

unsafe fn f2<T1: Default, T2, T3>() -> Variant3<T1, T2, T3> {
    Variant3::V0(T1::default())
}

unsafe fn f3<T1, T2>(a0: T1) -> Variant2<T1, T2> {
    Variant2::V0(a0)
}

unsafe fn f4<T1, T2>(a0: T2) -> Variant2<T1, T2> {
    Variant2::V1(a0)
}

unsafe fn f5<T1, T2, T3>(a0: T1) -> Variant3<T1, T2, T3> {
    Variant3::V0(a0)
}

unsafe fn f6<T1, T2, T3>(a0: T2) -> Variant3<T1, T2, T3> {
    Variant3::V1(a0)
}

unsafe fn f7<T1, T2, T3>(a0: T3) -> Variant3<T1, T2, T3> {
    Variant3::V2(a0)
}

unsafe fn f8<T1, T2>(a0: *const Variant2<T1, T2>) -> usize {
    unsafe { Variant2::index(&*a0) }
}

unsafe fn f9<T1, T2, T3>(a0: *const Variant3<T1, T2, T3>) -> usize {
    unsafe { Variant3::index(&*a0) }
}
