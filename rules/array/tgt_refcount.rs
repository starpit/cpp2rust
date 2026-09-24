// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn f1<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f3<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f5<T1: ByteRepr + Clone>(a0: Ptr<Vec<T1>>, a1: &mut Vec<T1>) {
    a0.write(std::mem::take(&mut *a1))
}

fn f7<T1: Clone + ByteRepr>(a0: Ptr<Vec<T1>>, a1: Vec<T1>) {
    a0.write(a1)
}

fn f8<T1: PartialEq>(a0: Vec<Value<T1>>, a1: Vec<Value<T1>>) -> bool {
    a0 == a1
}

fn f9<T1: PartialEq>(a0: Vec<Value<T1>>, a1: Vec<Value<T1>>) -> bool {
    a0 != a1
}


fn f10<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

fn f11<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

// Element-wise store; see the note on f12 in src.cpp for why this must not be
// `__v.fill(...)` over a shared handle.
fn f12<T1: Clone + ByteRepr>(a0: Ptr<Vec<T1>>, a1: T1) {
    a0.with_mut(|__v: &mut Vec<T1>| {
        for __e in __v.iter_mut() {
            *__e = a1.clone();
        }
    })
}

// A REFCOUNT OVERLAY for the copy constructor, which this module did not have.
// With only a tgt_unsafe body the refcount model falls through to it, and its
// `a0.clone()` is SHALLOW: when the element is itself a container of `Value`
// cells the copy shares those handles and aliases the original.  A missing
// overlay is the invisible form of this bug -- the module reads as already
// handled.  See libcc2rs/src/deep_clone.rs for the measurement.
fn f6<T1: DeepClone>(a0: Vec<T1>) -> Vec<T1> {
    a0.deep_clone()
}
