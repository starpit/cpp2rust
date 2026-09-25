// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn f1<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_last()
}

fn f2<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f7<T1: ByteRepr>(a0: Ptr<Vec<Value<Vec<T1>>>>, a1: Value<Vec<T1>>) {
    a0.with_mut(|__v: &mut Vec<Value<Vec<T1>>>| __v.push(a1))
}

fn f10<T1: Clone + ByteRepr>(a0: Ptr<Vec<T1>>, a1: Vec<T1>) {
    a0.write(a1)
}

fn f11<T1: ByteRepr + Clone>(a0: Ptr<Vec<T1>>, a1: &mut Vec<T1>) {
    a0.write(std::mem::take(&mut *a1))
}

fn f15<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

fn f16<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

// ---------------------------------------------------------------------------
// Reverse iterators. Same model as rules/vector (see the header comment in
// src.cpp): the Ptr<T> to the element it dereferences to, walked BACKWARDS.
// rbegin() is offset len-1, rend() is offset -1 (the wrapped usize), which is
// exactly what PrefixDec leaves behind when the walk steps off the front.
// ---------------------------------------------------------------------------

fn t2<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn t3<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn t4<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn t5<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn f21<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end().offset(-1_isize)
}

fn f22<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    let __len = a0.len() as isize;
    a0.to_end().offset(-(__len + 1))
}

fn f23<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end().offset(-1_isize)
}

fn f24<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    let __len = a0.len() as isize;
    a0.to_end().offset(-(__len + 1))
}

fn f25<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end().offset(-1_isize)
}

fn f26<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    let __len = a0.len() as isize;
    a0.to_end().offset(-(__len + 1))
}

fn f27<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f28<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_dec()
}

fn f29<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_dec()
}

fn f30<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f31<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f32<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

fn f33<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.offset(1_isize)
}

fn f34<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f35<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_dec()
}

fn f36<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_dec()
}

fn f37<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f38<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f39<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

fn f40<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.offset(1_isize)
}

// ---------------------------------------------------------------------------
// The forward iterator: the rules/vector bodies. See the note in src.cpp.
// ---------------------------------------------------------------------------

fn f46<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f47<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f48<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f49<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f50<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f51<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

fn f52<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f53<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

fn f54<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f55<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f56<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

fn f57<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(-(a1 as isize))
}

fn f58<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f59<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_inc()
}

// A REFCOUNT OVERLAY for the copy constructor, which this module did not have.
// With only a tgt_unsafe body the refcount model falls through to it, and its
// `a0.clone()` is SHALLOW: when the element is itself a container of `Value`
// cells the copy shares those handles and aliases the original.  A missing
// overlay is the invisible form of this bug -- the module reads as already
// handled.  See libcc2rs/src/deep_clone.rs for the measurement.
fn f8<T1: DeepClone>(a0: Vec<T1>) -> Vec<T1> {
    a0.deep_clone()
}

// emplace_back through an argument pack, refcount overlay. Upstream's f13,
// renumbered to f61 -- see the note in src.cpp. No deep_clone: emplace_back
// CONSTRUCTS its element at the call site, so there is no original to alias
// (the same reasoning as rules/vector's f130).
fn f61<T1: ByteRepr>(a0: Ptr<Vec<Value<Vec<T1>>>>, init: Vec<T1>) {
    let __init = init;
    a0.with_mut(|__v: &mut Vec<Value<Vec<T1>>>| __v.push(Rc::new(RefCell::new(__init))))
}
