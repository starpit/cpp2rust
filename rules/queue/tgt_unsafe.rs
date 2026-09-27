// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

// FIFO on a Vec: push() appends at the BACK, pop()/front() take the FRONT.
// (Same representation choice, and the same reason, as rules/deque: Ptr<> has
// no VecDeque infrastructure.  remove(0) is O(n) but it is FIFO-correct.)
fn t1<T1>() -> Vec<T1> {
    Default::default()
}

unsafe fn f1<T1>() -> Vec<T1> {
    Vec::new()
}

unsafe fn f2<T1>(a0: Vec<T1>) -> bool {
    a0.is_empty()
}

unsafe fn f3<T1>(a0: Vec<T1>) -> usize {
    a0.len()
}

unsafe fn f4<T1: Clone>(a0: &mut Vec<T1>, a1: &T1) {
    a0.push(a1.clone())
}

unsafe fn f5<T1: Default>(a0: &mut Vec<T1>, a1: &mut T1) {
    a0.push(std::mem::take(&mut *a1))
}

unsafe fn f6<T1>(a0: &mut Vec<T1>) -> *const T1 {
    (a0.first_mut().unwrap())
}

unsafe fn f7<T1>(a0: &mut Vec<T1>) -> *const T1 {
    (a0.last_mut().unwrap())
}

unsafe fn f8<T1>(a0: &mut Vec<T1>) -> T1 {
    a0.remove(0)
}
