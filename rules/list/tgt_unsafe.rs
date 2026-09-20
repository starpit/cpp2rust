// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::list<T> shares the Vec<T> representation with std::vector<T> and
// std::deque<T>, and its iterators are the same raw-pointer-to-element model
// rules/vector uses. See the header comment in src.cpp for the list semantics
// that representation does NOT preserve (iterator stability across insert /
// erase / reallocation, O(1) middle splice).

use libcc2rs::*;

fn t1<T1>() -> Vec<T1> {
    Default::default()
}

fn t2<T1>() -> *mut T1 {
    Default::default()
}

fn t3<T1>() -> *const T1 {
    Default::default()
}

// --- capacity / modifiers on the container ---------------------------------

unsafe fn f1<T1>(a0: Vec<T1>) -> usize {
    a0.len()
}

unsafe fn f2<T1>(a0: Vec<T1>) -> bool {
    a0.is_empty()
}

unsafe fn f3<T1>(a0: &mut Vec<T1>) {
    a0.clear()
}

unsafe fn f4<T1: Clone>(a0: &mut Vec<T1>, a1: T1) {
    a0.push(a1)
}

unsafe fn f5<T1: Default>(a0: &mut Vec<T1>, a1: &mut T1) {
    a0.push(std::mem::take(&mut *a1))
}

unsafe fn f6<T1: Clone>(a0: &mut Vec<T1>, a1: T1) {
    a0.insert(0, a1)
}

unsafe fn f7<T1: Default>(a0: &mut Vec<T1>, a1: &mut T1) {
    a0.insert(0, std::mem::take(&mut *a1))
}

unsafe fn f8<T1>(a0: &mut Vec<T1>) {
    a0.pop();
}

unsafe fn f9<T1>(a0: &mut Vec<T1>) {
    a0.remove(0);
}

// --- element access --------------------------------------------------------

unsafe fn f10<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    ((a0).first_mut().unwrap())
}

unsafe fn f11<T1>(a0: Vec<T1>) -> *const T1 {
    ((a0).first().unwrap())
}

unsafe fn f12<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    ((a0).last_mut().unwrap())
}

unsafe fn f13<T1>(a0: Vec<T1>) -> *const T1 {
    ((a0).last().unwrap())
}

// --- begin / end -----------------------------------------------------------

unsafe fn f14<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr()
}

unsafe fn f15<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr().add(a0.len())
}

unsafe fn f16<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr()
}

unsafe fn f17<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr().add(a0.len())
}

unsafe fn f18<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr()
}

unsafe fn f19<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr().add(a0.len())
}

// --- construction / assignment ---------------------------------------------

unsafe fn f20<T1>() -> Vec<T1> {
    Vec::new()
}

unsafe fn f21<T1: Clone>(a0: Vec<T1>) -> Vec<T1> {
    a0.clone()
}

unsafe fn f22<T1>(a0: &mut Vec<T1>) -> Vec<T1> {
    std::mem::take(&mut *a0)
}

unsafe fn f23<T1: Clone>(a0: &mut Vec<T1>, a1: Vec<T1>) {
    *a0 = a1.clone()
}

unsafe fn f24<T1>(a0: &mut Vec<T1>, a1: &mut Vec<T1>) {
    *a0 = std::mem::take(&mut *a1)
}

// --- erase / insert --------------------------------------------------------
//
// The returned iterator is rebuilt from the container base AFTER the Vec has
// been mutated: the incoming one may dangle once the buffer moves.

unsafe fn f25<T1>(a0: &mut Vec<T1>, a1: *const T1) -> *mut T1 {
    let __pos = a1.offset_from(a0.as_ptr()) as usize;
    a0.remove(__pos);
    a0.as_mut_ptr().add(__pos)
}

unsafe fn f26<T1: Clone>(a0: &mut Vec<T1>, a1: *const T1, a2: T1) -> *mut T1 {
    let __pos = a1.offset_from(a0.as_ptr()) as usize;
    a0.insert(__pos, a2);
    a0.as_mut_ptr().add(__pos)
}

unsafe fn f27<T1: Default>(a0: &mut Vec<T1>, a1: *const T1, a2: &mut T1) -> *mut T1 {
    let __pos = a1.offset_from(a0.as_ptr()) as usize;
    a0.insert(__pos, std::mem::take(&mut *a2));
    a0.as_mut_ptr().add(__pos)
}

// --- iterator --------------------------------------------------------------

unsafe fn f28<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

unsafe fn f29<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.prefix_inc()
}

unsafe fn f30<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.postfix_inc()
}

unsafe fn f31<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.prefix_dec()
}

unsafe fn f32<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 == a1
}

unsafe fn f33<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 != a1
}

// --- const_iterator --------------------------------------------------------

unsafe fn f34<T1>(a0: *const T1) -> *const T1 {
    a0
}

unsafe fn f35<T1>(a0: &mut *const T1) -> *const T1 {
    a0.prefix_inc()
}

unsafe fn f36<T1>(a0: &mut *const T1) -> *const T1 {
    a0.postfix_inc()
}

unsafe fn f37<T1>(a0: &mut *const T1) -> *const T1 {
    a0.prefix_dec()
}

unsafe fn f38<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 == a1
}

unsafe fn f39<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 != a1
}

// --- iterator copy / conversion --------------------------------------------

unsafe fn f40<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

unsafe fn f41<T1>(a0: *mut T1) -> *const T1 {
    a0
}
