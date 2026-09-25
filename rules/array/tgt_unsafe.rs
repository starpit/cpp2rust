// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

fn t1<T1>() -> Vec<T1> {
    Default::default()
}

unsafe fn f1<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr()
}

unsafe fn f2<T1>(a0: Vec<T1>) -> usize {
    a0.len()
}

unsafe fn f3<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr()
}

unsafe fn f4<T1>(a0: &mut Vec<T1>) -> Vec<T1> {
    std::mem::take(&mut *a0)
}

unsafe fn f5<T1>(a0: &mut Vec<T1>, a1: &mut Vec<T1>) {
    *a0 = std::mem::take(&mut *a1)
}

unsafe fn f6<T1: Clone>(a0: Vec<T1>) -> Vec<T1> {
    a0.clone()
}

unsafe fn f7<T1: Clone>(a0: &mut Vec<T1>, a1: Vec<T1>) {
    *a0 = a1.clone()
}

unsafe fn f8<T1: PartialEq>(a0: Vec<T1>, a1: Vec<T1>) -> bool {
    a0 == a1
}

unsafe fn f9<T1: PartialEq>(a0: Vec<T1>, a1: Vec<T1>) -> bool {
    a0 != a1
}

unsafe fn f10<T1>(a0: &mut Vec<T1>, a1: usize) -> *mut T1 {
    if a1 as usize >= a0.len() {
        panic!("out of bounds access")
    } else {
        (a0).as_mut_ptr().add(a1 as usize)
    }
}

unsafe fn f11<T1>(a0: &mut Vec<T1>, a1: usize) -> *const T1 {
    if a1 as usize >= a0.len() {
        panic!("out of bounds access")
    } else {
        (a0).as_ptr().add(a1 as usize)
    }
}

unsafe fn f12<T1: Clone>(a0: &mut Vec<T1>, a1: T1) {
    for __e in a0.iter_mut() {
        *__e = a1.clone();
    }
}

// rbegin / rend on std::array.  The result is rules/vector's t10 spelling --
// the reverse iterator is the POINTER TO THE ELEMENT IT DEREFERENCES TO, so
// rbegin() is the LAST element and rend() the slot one BEFORE the first.
// wrapping_sub, not sub: on an empty array rend() steps off the front, and the
// wrapping spelling is what makes it produce the same address the walk's
// prefix_dec eventually reaches.  See rules/vector f114/f115, whose bodies
// these mirror exactly.
unsafe fn f13<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr().add(a0.len()).wrapping_sub(1)
}

unsafe fn f14<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr().wrapping_sub(1)
}
