// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn t2<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn t3<T1>() -> Vec<Value<Vec<T1>>> {
    Vec::new()
}

fn t4<T1>() -> Ptr<T1> {
    Ptr::null()
}

#[cfg(target_os = "linux")]
fn t6<T1>() -> Ptr<T1> {
    Ptr::null()
}

#[cfg(target_os = "linux")]
fn t7<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn f1<T1: ByteRepr>(a0: Ptr<Vec<T1>>, a1: Ptr<T1>) -> Ptr<T1> {
    let idx = a1.get_offset();
    a0.with_mut(|__v: &mut Vec<T1>| __v.remove(idx));
    a0.decay()
}

fn f6<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f7<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

fn f9<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f10<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_last()
}

fn f13<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f17<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f18<T1: ByteRepr>(a0: &mut Vec<T1>, a1: Ptr<T1>, a2: T1) -> Ptr<T1> {
    let __off = a1.get_offset();
    a0.insert(__off, a2);
    a1
}

fn f20<T1: ByteRepr>(a0: &mut Vec<T1>, a1: Ptr<T1>, a2: T1) -> Ptr<T1> {
    let __off = a1.get_offset();
    a0.insert(__off, a2);
    a1
}

fn f22<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f23<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f24<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f25<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

fn f26<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

fn f27<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f28<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_inc()
}

// COPY IS DEEP, RECURSIVELY.  The old body unwrapped exactly ONE Value layer
// (`Rc::new(RefCell::new(v.borrow().clone()))`), which is right for a scalar
// element and WRONG the moment the element is itself a container: the inner
// .clone() then copies Rc HANDLES and the copy ALIASES the original.  Measured
// against clang-built C++: `map<int,map<int,long>> b(a); b[1][2]=22` gave
// C++ a=11, refcount a=22.  DeepClone recurses, so it is correct at every depth.
fn f29<T1: DeepClone>(a0: Vec<Value<Vec<T1>>>) -> Vec<Value<Vec<T1>>> {
    a0.deep_clone()
}

fn f30<T1: Default + Clone>(a0: usize) -> Vec<Value<Vec<T1>>> {
    (0..(a0) as usize)
        .map(|_| <Value<Vec<T1>>>::default())
        .collect::<Vec<_>>()
}

fn f31<T1: ByteRepr + Clone>(a0: Ptr<Vec<Value<Vec<T1>>>>, a1: &mut Vec<T1>) {
    a0.with_mut(|__v: &mut Vec<Value<Vec<T1>>>| {
        __v.push(Rc::new(RefCell::new(std::mem::take(&mut *a1))))
    })
}

fn f32<T1: Default + ByteRepr>(a0: Ptr<Vec<Value<Vec<T1>>>>, a1: usize) {
    let _a0 = a1 as usize;
    a0.with_mut(|__v: &mut Vec<Value<Vec<T1>>>| __v.resize_with(_a0, <Value<Vec<T1>>>::default))
}

fn f33<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> isize {
    ((a0.get_offset() as isize) - (a1.get_offset() as isize))
}

fn f34<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f35<T1: Clone + ByteRepr>(a0: Ptr<T1>, a1: Ptr<T1>) -> Vec<T1> {
    let __count = a1.get_offset() - a0.get_offset();
    PtrValueIter::new(&a0, __count).collect::<Vec<_>>()
}

fn f37<T1: TryFrom<T2>, T2: Clone + ByteRepr>(a0: Ptr<T2>, a1: Ptr<T2>) -> Vec<T1> {
    let __count = a1.get_offset() - a0.get_offset();
    PtrValueIter::new(&a0, __count)
        .map(|item| T1::try_from(item).ok().unwrap())
        .collect::<Vec<_>>()
}

fn f40<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f41<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f42(a0: Ptr<u8>, a1: Ptr<u8>) -> Ptr<u8> {
    let __count = a1.get_offset() - a0.get_offset();
    let max_index = PtrValueIter::new(&a0, __count)
        .enumerate()
        .max_by_key(|&(_, val)| val)
        .map(|(idx, _)| idx)
        .unwrap_or(0);

    a0 + max_index
}

fn f43<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f44<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f50<T1: ByteRepr>(a0: Ptr<Vec<T1>>, a1: usize) -> Ptr<Vec<T1>> {
    if a1 as usize >= (*a0.upgrade().deref()).len() {
        panic!("out of bounds access")
    } else {
        a0.offset(a1 as isize)
    }
}

fn f51<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_last()
}

fn f52<T1: Clone + ByteRepr>(a0: Ptr<Vec<Value<Vec<T1>>>>, a1: Vec<T1>) {
    a0.with_mut(|__v: &mut Vec<Value<Vec<T1>>>| __v.push(Rc::new(RefCell::new(a1))))
}

fn f53<T1: Clone + ByteRepr>(
    a0: Ptr<Vec<T1>>,
    a1: Ptr<T1>,
    a2: Ptr<T1>,
    a3: Ptr<T1>,
) -> Ptr<Vec<T1>> {
    let start_idx = a1.get_offset();
    let count = a3.get_offset() - a2.get_offset();
    let temp_vec: Vec<T1> = PtrValueIter::new(&a2, count).collect();
    a0.with_mut(|v: &mut Vec<T1>| {
        v.splice(start_idx..start_idx, temp_vec);
    });
    a0 + start_idx
}

fn f56<T1: ByteRepr>(a0: &Vec<Value<Vec<T1>>>) -> Ptr<Vec<T1>> {
    a0[a0.len() - 1].as_pointer()
}

fn f55<T1: ByteRepr + Clone>(a0: Ptr<Vec<T1>>, a1: &mut Vec<T1>) {
    a0.write(std::mem::take(&mut *a1))
}

fn f57<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f58<T1: Clone + ByteRepr>(a0: Ptr<Vec<T1>>, a1: Vec<T1>) {
    a0.write(a1)
}

fn f60<T1: ByteRepr>(a0: Ptr<Vec<T1>>, a1: Ptr<T1>) -> Ptr<T1> {
    let idx = a1.get_offset();
    a0.with_mut(|__v: &mut Vec<T1>| __v.remove(idx));
    a0.decay()
}

fn f65<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f66<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

fn f68<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f69<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_last()
}

fn f72<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f76<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f77<T1: ByteRepr>(a0: &mut Vec<T1>, a1: Ptr<T1>, a2: T1) -> Ptr<T1> {
    let __off = a1.get_offset();
    a0.insert(__off, a2);
    a1
}

fn f79<T1: ByteRepr>(a0: &mut Vec<T1>, a1: Ptr<T1>, a2: T1) -> Ptr<T1> {
    let __off = a1.get_offset();
    a0.insert(__off, a2);
    a1
}

fn f81<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f82<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f83<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f84<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

fn f85<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

fn f86<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f87<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_inc()
}

fn f88<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> isize {
    ((a0.get_offset() as isize) - (a1.get_offset() as isize))
}

fn f89<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f90<T1: Clone + ByteRepr>(a0: Ptr<T1>, a1: Ptr<T1>) -> Vec<T1> {
    let __count = a1.get_offset() - a0.get_offset();
    PtrValueIter::new(&a0, __count).collect::<Vec<_>>()
}

fn f92<T1: TryFrom<T2>, T2: Clone + ByteRepr>(a0: Ptr<T2>, a1: Ptr<T2>) -> Vec<T1> {
    let __count = a1.get_offset() - a0.get_offset();
    PtrValueIter::new(&a0, __count)
        .map(|item| T1::try_from(item).ok().unwrap())
        .collect::<Vec<_>>()
}

fn f93<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f94(a0: Ptr<u8>, a1: Ptr<u8>) -> Ptr<u8> {
    let __count = a1.get_offset() - a0.get_offset();
    let max_index = PtrValueIter::new(&a0, __count)
        .enumerate()
        .max_by_key(|&(_, val)| val)
        .map(|(idx, _)| idx)
        .unwrap_or(0);

    a0 + max_index
}

fn f95<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f96<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f98<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

fn f99<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_last()
}

fn f101<T1: Clone + ByteRepr>(
    a0: Ptr<Vec<T1>>,
    a1: Ptr<T1>,
    a2: Ptr<T1>,
    a3: Ptr<T1>,
) -> Ptr<Vec<T1>> {
    let start_idx = a1.get_offset();
    let count = a3.get_offset() - a2.get_offset();
    let temp_vec: Vec<T1> = PtrValueIter::new(&a2, count).collect();
    a0.with_mut(|v: &mut Vec<T1>| {
        v.splice(start_idx..start_idx, temp_vec);
    });
    a0 + start_idx
}

fn f103<T1: ByteRepr + Clone>(a0: Ptr<Vec<T1>>, a1: &mut Vec<T1>) {
    a0.write(std::mem::take(&mut *a1))
}

fn f104<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f105<T1: Clone + ByteRepr>(a0: Ptr<Vec<T1>>, a1: Vec<T1>) {
    a0.write(a1)
}

fn f111<T1: ByteRepr + Clone>(a0: Ptr<Vec<Value<Vec<T1>>>>, a1: &mut Vec<Value<Vec<T1>>>) {
    a0.write(std::mem::take(&mut *a1))
}

fn f112<T1: PartialEq>(a0: Vec<Value<T1>>, a1: Vec<Value<T1>>) -> bool {
    a0 == a1
}

fn f113<T1: PartialEq>(a0: Vec<Value<T1>>, a1: Vec<Value<T1>>) -> bool {
    a0 != a1
}

// ---------------------------------------------------------------------------
// Reverse iterators. See the header comment on the matching rules in src.cpp:
// a reverse iterator is the Ptr<T> to the element it dereferences to and it
// walks BACKWARDS, so rbegin() is offset len-1, rend() is offset -1,
// operator++ decrements and operator-- increments.
//
// Ptr<T>::offset is a usize, so "one before the first element" is the wrapped
// offset; `to_end().offset(-(len + 1))` produces exactly the value that
// PrefixDec leaves behind when the walk steps off the front (both go through
// wrapping_sub of one elem_step), so `it != v.rend()` terminates. Building it
// off to_end() rather than off a0 keeps it correct whatever offset a0 carries,
// and makes the empty-container case (rbegin() == rend()) fall out for free.
// ---------------------------------------------------------------------------

fn t8<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn t9<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn f114<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end().offset(-1_isize)
}

fn f115<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    let __len = a0.len() as isize;
    a0.to_end().offset(-(__len + 1))
}

fn f116<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end().offset(-1_isize)
}

fn f117<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    let __len = a0.len() as isize;
    a0.to_end().offset(-(__len + 1))
}

fn f118<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end().offset(-1_isize)
}

fn f119<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    let __len = a0.len() as isize;
    a0.to_end().offset(-(__len + 1))
}

fn f120<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f121<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_dec()
}

fn f122<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_dec()
}

fn f123<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f124<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f125<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

fn f126<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.offset(1_isize)
}

// vector<bool>::at -- the VALUE at the index, not a pointer to it.  See
// src.cpp: the proxy return type forces a read-only model.
fn f127(a0: Ptr<bool>, a1: usize) -> bool {
    (*a0.offset(a1 as isize).upgrade().deref())
}

// Contiguous-iterator operator-(long). The mirror of f25 (`a0.offset(a1)`).
fn f128<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(-(a1 as isize))
}

// std::vector's copy constructors need a REFCOUNT OVERLAY, which they did not
// have: with only a tgt_unsafe body, the refcount model falls through to it and
// gets its `a0.clone()`.  That is shallow, and a Vec whose element is itself a
// container of `Value` cells is then ALIASED by its own copy.  Measured against
// clang-built C++: `vector<map<int,long>> b(a); b[0][1] = 22` gave C++ a=11 and
// refcount a=22.  This was the LAST site in the nine-case aliasing matrix still
// wrong after the rule-level and converter-level deep_clone work, precisely
// because a missing overlay is invisible -- the module looked already handled.
//
// rules/vector f29 (`vector<vector<T>>`) exists because someone hit exactly this
// for ONE element type and wrote a rule for that shape.  These two rules are the
// general case, so f29 is now redundant with them rather than load-bearing; it
// is left in place because removing a rule is a separate, riskier change.
fn f109<T1: DeepClone>(a0: Vec<T1>) -> Vec<T1> {
    a0.deep_clone()
}

fn f110<T1: DeepClone>(a0: Vec<T1>) -> Vec<T1> {
    a0.deep_clone()
}

// A REFCOUNT OVERLAY for the copying push_back, which this module did not have.
// With only a tgt_unsafe body the refcount model falls through to it, and its
// `a0.clone()` is SHALLOW: when the element is itself a container of `Value`
// cells the copy shares those handles and aliases the original.  A missing
// overlay is the invisible form of this bug -- the module reads as already
// handled.  See libcc2rs/src/deep_clone.rs for the measurement.
fn f21<T1: DeepClone>(a0: &mut Vec<T1>, a1: T1) {
    a0.push(a1.deep_clone())
}

fn f80<T1: DeepClone>(a0: &mut Vec<T1>, a1: T1) {
    a0.push(a1.deep_clone())
}

fn f100<T1: DeepClone>(a0: &mut Vec<Vec<T1>>, a1: Vec<T1>) {
    a0.push(a1.deep_clone())
}
