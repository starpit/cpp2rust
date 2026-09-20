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
    *a0 = a1.clone()
}

unsafe fn f11<T1>(a0: &mut Vec<T1>, a1: &mut Vec<T1>) {
    *a0 = std::mem::take(&mut *a1)
}

unsafe fn f12<T1>(a0: Vec<T1>) -> usize {
    a0.len()
}

unsafe fn f13<T1: Default>(a0: &mut Vec<T1>, a1: usize) {
    let __a0 = a1 as usize;
    a0.resize_with(__a0, || <T1>::default())
}

unsafe fn f14<T1>(a0: &mut Vec<T1>) {
    a0.clear()
}

unsafe fn f15<T1>(a0: &mut Vec<T1>, a1: usize) -> *mut T1 {
    &mut (a0)[a1 as usize]
}

unsafe fn f16<T1>(a0: Vec<T1>, a1: usize) -> *const T1 {
    &(a0)[a1 as usize]
}

unsafe fn f17<T1: Clone>(a0: &mut Vec<T1>, a1: T1) {
    a0.push(a1)
}

unsafe fn f18<T1: Default>(a0: &mut Vec<T1>, a1: &mut T1) {
    a0.insert(0, std::mem::take(&mut *a1))
}

unsafe fn f19<T1: Clone>(a0: &mut Vec<T1>, a1: T1) {
    a0.insert(0, a1)
}

unsafe fn f20<T1>(a0: &mut Vec<T1>) {
    a0.pop();
}

// ---------------------------------------------------------------------------
// Reverse iterators. Same model as rules/vector (see the header comment in
// src.cpp): the raw pointer to the element it dereferences to, walked
// BACKWARDS. rbegin() is `base + len - 1`, rend() is `base - 1` built with
// wrapping_sub so it is well defined for an empty Vec and matches the address
// UnsafePrefixDec::prefix_dec leaves behind when the walk steps off the front.
//
// t2/t3 are the forward iterator (returned by base()); t4/t5 are the reverse
// iterator itself.
// ---------------------------------------------------------------------------

fn t2<T1>() -> *mut T1 {
    Default::default()
}

fn t3<T1>() -> *const T1 {
    Default::default()
}

fn t4<T1>() -> *mut T1 {
    Default::default()
}

fn t5<T1>() -> *const T1 {
    Default::default()
}

unsafe fn f21<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr().add(a0.len()).wrapping_sub(1)
}

unsafe fn f22<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr().wrapping_sub(1)
}

unsafe fn f23<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr().add(a0.len()).wrapping_sub(1)
}

unsafe fn f24<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr().wrapping_sub(1)
}

unsafe fn f25<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr().add(a0.len()).wrapping_sub(1)
}

unsafe fn f26<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr().wrapping_sub(1)
}

unsafe fn f27<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

unsafe fn f28<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.prefix_dec()
}

unsafe fn f29<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.postfix_dec()
}

unsafe fn f30<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.prefix_inc()
}

unsafe fn f31<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 == a1
}

unsafe fn f32<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 != a1
}

unsafe fn f33<T1>(a0: *mut T1) -> *mut T1 {
    a0.wrapping_add(1)
}

unsafe fn f34<T1>(a0: *const T1) -> *const T1 {
    a0
}

unsafe fn f35<T1>(a0: &mut *const T1) -> *const T1 {
    a0.prefix_dec()
}

unsafe fn f36<T1>(a0: &mut *const T1) -> *const T1 {
    a0.postfix_dec()
}

unsafe fn f37<T1>(a0: &mut *const T1) -> *const T1 {
    a0.prefix_inc()
}

unsafe fn f38<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 == a1
}

unsafe fn f39<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 != a1
}

unsafe fn f40<T1>(a0: *const T1) -> *const T1 {
    a0.wrapping_add(1)
}

unsafe fn f41<T1: PartialEq>(a0: Vec<T1>, a1: Vec<T1>) -> bool {
    a0 == a1
}

unsafe fn f42<T1: PartialEq>(a0: Vec<T1>, a1: Vec<T1>) -> bool {
    a0 != a1
}

fn t6<T1, T2>() -> Vec<T1> {
    Default::default()
}

unsafe fn f43<T1: Clone>(a0: usize, a1: T1) -> Vec<T1> {
    vec![a1; a0 as usize]
}

unsafe fn f44<T1: Default>(a0: usize) -> Vec<T1> {
    (0..(a0) as usize).map(|_| <T1>::default()).collect::<Vec<_>>()
}

unsafe fn f45<T1>() -> Vec<T1> {
    Vec::new()
}
