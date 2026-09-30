// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

// TODO: it should be VecDeque, but we don't have the neccessary infrastructure in Ptr<> to
// make this work.
fn t1<T1>() -> Vec<T1> {
    Default::default()
}

unsafe fn f1<T1>(a0: &mut Vec<T1>) -> *const T1 {
    (a0.last_mut().unwrap())
}

unsafe fn f2<T1>(a0: &mut Vec<T1>) -> *const T1 {
    ((a0).first_mut().unwrap())
}

unsafe fn f3<T1>(a0: Vec<T1>) -> bool {
    a0.is_empty()
}

unsafe fn f4<T1: Default>(a0: &mut Vec<T1>, a1: &mut T1) {
    a0.push(std::mem::take(&mut *a1))
}

unsafe fn f5<T1>(a0: &mut Vec<T1>) -> T1 {
    a0.remove(0)
}

unsafe fn f7<T1>(a0: &mut Vec<Vec<T1>>, a1: Vec<T1>) {
    a0.push(a1)
}

unsafe fn f8<T1: Clone>(a0: Vec<T1>) -> Vec<T1> {
    a0.clone()
}

unsafe fn f9<T1>(a0: &mut Vec<T1>) -> Vec<T1> {
    std::mem::take(&mut *a0)
}

unsafe fn f10<T1: Clone>(a0: &mut Vec<T1>, a1: Vec<T1>) {
    let __src = a1.clone();
    a0.clear();
    a0.extend(__src);
}

unsafe fn f11<T1>(a0: &mut Vec<T1>, a1: &mut Vec<T1>) {
    let __src = std::mem::take(&mut *a1);
    a0.clear();
    a0.extend(__src);
}

unsafe fn f12<T1>(a0: &mut Vec<T1>, init: T1) {
    let __init = init;
    a0.push(__init)
}

unsafe fn f13<T1>(a0: &mut Vec<Vec<T1>>, init: Vec<T1>) {
    let __init = init;
    a0.push(__init)
}

unsafe fn f14<T1>() -> Vec<T1> {
    Vec::new()
}

unsafe fn f15<T1: PartialEq>(a0: &Vec<T1>, a1: &Vec<T1>) -> bool {
    a0 == a1
}

unsafe fn f16<T1: PartialEq>(a0: &Vec<T1>, a1: &Vec<T1>) -> bool {
    a0 != a1
}

unsafe fn f17<T1>(a0: &mut Vec<T1>, a1: T1) {
    a0.push(a1)
}

// f18 -- `at(i)`, NON-CONST.  Body copied verbatim from `rules/vector`'s f7: the
// `&mut Vec<T1>` index yields a `&mut T1`, which coerces to the `*mut T1` the
// caller writes through.  A by-value or cloning body would drop the write.
unsafe fn f18<T1>(a0: &mut Vec<T1>, a1: usize) -> *mut T1 {
    &mut (a0)[a1 as usize]
}

// f19 -- `at(i) const`.  Body copied from `rules/vector`'s f98/f50 unsafe arm,
// which spells the bounds check explicitly because `at` throws where `[]` is UB.
unsafe fn f19<T1>(a0: &mut Vec<T1>, a1: usize) -> *mut T1 {
    if a1 as usize >= a0.len() {
        panic!("out of bounds access")
    } else {
        (a0).as_mut_ptr().add(a1 as usize)
    }
}

// f20 -- `resize(n)`.  Body copied from `rules/vector`'s f15/f74.
unsafe fn f20<T1: Default>(a0: &mut Vec<T1>, a1: usize) {
    let __a0 = a1 as usize;
    a0.resize_with(__a0, || <T1>::default())
}

// f21 -- `resize(n, v)`.  Body copied from `rules/vector`'s f54/f102.
unsafe fn f21<T1: Default + Clone>(a0: &mut Vec<T1>, a1: usize, a2: T1) {
    let __a0 = a1 as usize;
    a0.resize(__a0, a2)
}

