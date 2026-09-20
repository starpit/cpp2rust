// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

// The same representation rules/deque uses: a Vec<T1> with the queue's head at
// index 0.
fn t1<T1>() -> Vec<T1> {
    Default::default()
}

fn t6<T1, T2>() -> Vec<T1> {
    Default::default()
}

unsafe fn f1<T1>() -> Vec<T1> {
    Vec::new()
}

unsafe fn f2<T1: Clone>(a0: &mut Vec<T1>, a1: T1) {
    a0.push(a1)
}

unsafe fn f8<T1: Default>(a0: &mut Vec<T1>, a1: &mut T1) {
    a0.push(std::mem::take(&mut *a1))
}

// std::queue::pop() returns void, so the removed element is dropped.  The
// discarded T1 return is rules/deque's f5 shape: the converter throws the
// value away, and naming it keeps the body a single expression.
unsafe fn f3<T1>(a0: &mut Vec<T1>) -> T1 {
    a0.remove(0)
}

unsafe fn f4<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_slice().first_mut().unwrap()
}

unsafe fn f5<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_slice().first().unwrap()
}

unsafe fn f6<T1>(a0: Vec<T1>) -> bool {
    a0.is_empty()
}

unsafe fn f7<T1>(a0: Vec<T1>) -> usize {
    a0.len()
}
