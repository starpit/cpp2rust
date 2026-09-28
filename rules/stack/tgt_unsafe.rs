// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// LIFO on a Vec: push() appends at the BACK and top()/pop() take the BACK.
// (Same representation choice, and the same reason, as rules/queue and
// rules/deque: Ptr<> has no VecDeque infrastructure.)  rules/queue's f6/f8 use
// `first_mut()` / `remove(0)` because a queue is FIFO -- using those here would
// reverse the iteration order of every stack in the corpus and still compile,
// which is why the two modules differ in exactly these two bodies.
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

// top(): a reference to the LAST element.  Does NOT remove -- `Vec::pop()`
// would, and `x = s.top(); s.pop();` would then drop two elements.
unsafe fn f6<T1>(a0: &mut Vec<T1>) -> *const T1 {
    (a0.last_mut().unwrap())
}

// pop(): removes the LAST element and yields nothing.  The Option is discarded
// because C++'s pop() returns void; popping empty is UB in C++, and
// `Vec::pop()` returning None here mirrors "nothing happened" rather than
// inventing a value.
unsafe fn f7<T1>(a0: &mut Vec<T1>) {
    let _ = a0.pop();
}
