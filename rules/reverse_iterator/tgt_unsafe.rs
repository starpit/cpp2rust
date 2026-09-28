// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

// A std::reverse_iterator<T*> holds its underlying iterator `current`, which
// points ONE PAST the element `*rit` designates.  It is modelled here as that
// same raw pointer, so `*rit` is `*(current - 1)` and `++rit` DECREMENTS.
fn t1<T1>() -> *mut T1 {
    Default::default()
}

unsafe fn f1<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

unsafe fn f2<T1>(a0: *mut T1, a1: *mut T1) -> bool {
    a0 == a1
}

unsafe fn f3<T1>(a0: *mut T1, a1: *mut T1) -> bool {
    a0 != a1
}

unsafe fn f4<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.prefix_dec()
}

unsafe fn f5<T1>(a0: &mut *mut T1, a1: i32) -> *mut T1 {
    a0.postfix_dec()
}

unsafe fn f6<T1>(a0: *mut T1) -> *mut T1 {
    a0.offset(-1)
}

unsafe fn f7<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

// t2 == t1: std::__wrap_iter<T1 *> IS a raw pointer, so the reverse_iterator
// over it has the very same representation and the very same off-by-one.
fn t2<T1>() -> *mut T1 {
    Default::default()
}

unsafe fn f8<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

unsafe fn f9<T1>(a0: *mut T1, a1: *mut T1) -> bool {
    a0 == a1
}

unsafe fn f10<T1>(a0: *mut T1, a1: *mut T1) -> bool {
    a0 != a1
}

unsafe fn f11<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.prefix_dec()
}

unsafe fn f12<T1>(a0: &mut *mut T1, a1: i32) -> *mut T1 {
    a0.postfix_dec()
}

unsafe fn f13<T1>(a0: *mut T1) -> *mut T1 {
    a0.offset(-1)
}

unsafe fn f14<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

fn t3<T1>() -> *const *mut T1 {
    Default::default()
}

fn t4<T1>() -> *const *mut T1 {
    Default::default()
}
