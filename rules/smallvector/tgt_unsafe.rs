// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

fn t1<T1>() -> Vec<T1> {
    Default::default()
}

fn t2<T1>() -> Vec<T1> {
    Default::default()
}

fn t3<T1>() -> Vec<T1> {
    Default::default()
}

fn t4<T1>() -> Vec<T1> {
    Default::default()
}

// One-argument `llvm::SmallVector<T1>` -- same model as t1, a DISTINCT key.
fn t5<T1>() -> Vec<T1> {
    Default::default()
}

// `llvm::SmallVectorBase<SizeT>`: the family's storage base.  T1 here is the
// SIZE type, not the element type, so this model is only ever correct in
// receiver position of an inlined member rule (f17..f19).
fn t6<T1>() -> Vec<T1> {
    Default::default()
}

// `llvm::SmallVectorTemplateCommon<T1, void>` -- the explicit-`void` spelling.
fn t9<T1>() -> Vec<T1> {
    Default::default()
}

unsafe fn f1<T1>(a0: &mut Vec<T1>, a1: T1) {
    a0.push(a1)
}

unsafe fn f2<T1>() -> Vec<T1> {
    Vec::new()
}

unsafe fn f3<T1>(a0: &mut Vec<T1>) -> Vec<T1> {
    std::mem::take(&mut *a0)
}

unsafe fn f4<T1: Clone>(a0: *const T1, a1: *const T1) -> Vec<T1> {
    unsafe { std::slice::from_raw_parts(a0, a1.offset_from(a0) as usize).to_vec() }
}

unsafe fn f5<T1>(a0: Vec<T1>) -> usize {
    a0.len()
}

unsafe fn f6<T1>(a0: Vec<T1>) -> bool {
    a0.is_empty()
}

unsafe fn f7<T1>(a0: &mut Vec<T1>, a1: usize) -> *mut T1 {
    &mut (a0)[a1 as usize]
}

unsafe fn f8<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr()
}

unsafe fn f9<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    unsafe { a0.as_mut_ptr().add(Vec::len(&a0)) }
}

unsafe fn f10<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr()
}

unsafe fn f11<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    ((a0).first_mut().unwrap())
}

unsafe fn f12<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    ((a0).last_mut().unwrap())
}

unsafe fn f13<T1>(a0: &mut Vec<T1>) {
    a0.clear()
}

unsafe fn f14<T1: Default>(a0: &mut Vec<T1>, a1: usize) {
    a0.resize_with(a1 as usize, <T1>::default)
}

unsafe fn f15<T1>(a0: &mut Vec<T1>, a1: usize) {
    if a1 as usize > a0.capacity() as usize {
        let len_0 = a0.len();
        a0.reserve_exact(a1 as usize - len_0 as usize);
    }
}

unsafe fn f16<T1>(a0: &mut Vec<T1>) {
    a0.pop();
}

unsafe fn f17<T1>(a0: &Vec<T1>) -> usize {
    Vec::len(a0)
}

unsafe fn f18<T1>(a0: &Vec<T1>) -> usize {
    Vec::capacity(a0)
}

unsafe fn f19<T1>(a0: &Vec<T1>) -> bool {
    Vec::is_empty(a0)
}
