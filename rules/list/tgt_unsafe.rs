// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model: a std::list is a Vec<T1>, the same representation
// rules/deque and rules/queue use, because Ptr<> has no linked-list
// infrastructure.  Every operation keyed here is observably identical on a Vec;
// only the complexity differs (push_front/pop_front are O(n) here).
//
// The ITERATOR is not modelled -- see src.cpp.  Nothing below returns or
// consumes one.

fn t1<T1>() -> Vec<T1> {
    Default::default()
}

unsafe fn f1<T1>() -> Vec<T1> {
    Vec::new()
}

unsafe fn f2<T1>(a0: Vec<T1>) -> usize {
    a0.len()
}

unsafe fn f3<T1>(a0: Vec<T1>) -> bool {
    a0.is_empty()
}

unsafe fn f4<T1>(a0: &mut Vec<T1>) {
    a0.clear()
}

// push_back(const T1&) -- the source keeps its value, so CLONE.
unsafe fn f5<T1: Clone>(a0: &mut Vec<T1>, a1: &T1) {
    a0.push(a1.clone())
}

// push_back(T1&&) -- the source is left in a moved-from state, which
// std::mem::take models for a Default type (same shape as rules/queue f5).
unsafe fn f6<T1: Default>(a0: &mut Vec<T1>, a1: &mut T1) {
    a0.push(std::mem::take(&mut *a1))
}

// push_front -- insert at index 0.  This is the operation that makes a list a
// DEQUE rather than a stack, and it is what the FIFO/LIFO probe discriminates:
// if this were a push() the printed order would reverse.
unsafe fn f7<T1: Clone>(a0: &mut Vec<T1>, a1: &T1) {
    a0.insert(0, a1.clone())
}

unsafe fn f8<T1: Default>(a0: &mut Vec<T1>, a1: &mut T1) {
    a0.insert(0, std::mem::take(&mut *a1))
}

// pop_back / pop_front are void in C++; the removed value is dropped.  Written
// as an expression that yields the element (as rules/queue f8 does) -- the
// caller discards it.
unsafe fn f9<T1>(a0: &mut Vec<T1>) -> T1 {
    a0.pop().unwrap()
}

unsafe fn f10<T1>(a0: &mut Vec<T1>) -> T1 {
    a0.remove(0)
}

// front()/back() on a MUTABLE list return T1&, which the unsafe model
// represents as a raw pointer into the buffer (same as rules/queue f6/f7).
unsafe fn f11<T1>(a0: &mut Vec<T1>) -> *const T1 {
    (a0.first_mut().unwrap())
}

unsafe fn f12<T1>(a0: &mut Vec<T1>) -> *const T1 {
    (a0.last_mut().unwrap())
}

// front()/back() on a CONST list return const T1&.  A borrowed receiver, not a
// by-value one: a pointer into a by-value parameter would dangle.  Shape taken
// from rules/vector f51, which is the same C++ signature.
unsafe fn f13<T1>(a0: &Vec<T1>) -> &T1 {
    ((a0).first().unwrap())
}

unsafe fn f14<T1>(a0: &Vec<T1>) -> &T1 {
    ((a0).last().unwrap())
}

unsafe fn f15<T1: Clone>(a0: Vec<T1>) -> Vec<T1> {
    a0.clone()
}

unsafe fn f16<T1>(a0: &mut Vec<T1>) -> Vec<T1> {
    std::mem::take(&mut *a0)
}

unsafe fn f17<T1: Clone>(a0: &mut Vec<T1>, a1: Vec<T1>) {
    let __src = a1.clone();
    a0.clear();
    a0.extend(__src);
}

unsafe fn f18<T1>(a0: &mut Vec<T1>, a1: &mut Vec<T1>) {
    let __src = std::mem::take(&mut *a1);
    a0.clear();
    a0.extend(__src);
}
