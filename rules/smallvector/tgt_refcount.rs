// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// The refcount OVERRIDES only. TranslationRule::Load (translation_rule.cpp:423)
// reads ir_unsafe.json first and then overlays ir_refcount.json, so a rule whose
// unsafe body is already right in this model is deliberately absent here --
// size(), empty(), push_back, pop_back, clear, the default constructors, the
// move constructors and the initializer-list constructors all are. rules/vector
// carries the same asymmetry for the same reason (its t1/t5, f2, f3, f4, f16 are
// unsafe-only).
//
// What DOES differ is every operation that hands out a reference to an element:
// the unsafe model's `*mut T1` is `Ptr<T1>` here, and the receiver arrives
// already decayed to a pointer at the first element. These are rules/vector's
// f9/f10/f7/f13/f17/f6 bodies for front/back/index/begin/end/data.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

// An element that is itself a container is BOXED here -- rules/vector's t3 for
// the std::vector-in-std::vector case, and for the same reason.
fn t6<T1>() -> Vec<Value<Vec<T1>>> {
    Vec::new()
}

fn f3<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f4<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_last()
}

fn f5<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

fn f6<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f7<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f8<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

// COPY IS DEEP in this model -- the same decision rules/vector records at its
// f109: a shallow clone of a Vec<Value<T>> copies the Rc handles, so the copy
// and the original would share every element and a write through one would be
// visible through the other, which the C++ copy constructor does not do.
fn f12<T1: DeepClone>(a0: &mut Vec<T1>, a1: Vec<T1>) {
    a0.extend(a1.iter().map(|e| e.deep_clone()))
}

fn f14<T1>(a0: &mut Vec<T1>, a1: Ptr<T1>, a2: Ptr<T1>) -> Ptr<T1> {
    let first = a1.get_offset();
    let last = a2.get_offset();
    a0.drain(first..last);
    a1
}

fn f17<T1: DeepClone>(a0: Vec<T1>) -> Vec<T1> {
    a0.deep_clone()
}

fn f18<T1: DeepClone>(a0: Vec<T1>) -> Vec<T1> {
    a0.deep_clone()
}

fn f23<T1: ByteRepr + Clone>(a0: Ptr<Vec<T1>>, a1: &mut Vec<T1>) {
    a0.write(std::mem::take(&mut *a1))
}

fn f24<T1: ByteRepr + Clone>(a0: Ptr<Vec<T1>>, a1: &mut Vec<T1>) {
    a0.write(std::mem::take(&mut *a1))
}

fn f25<T1: Clone + ByteRepr>(a0: Ptr<Vec<T1>>, a1: Vec<T1>) {
    a0.write(a1)
}

fn f26<T1: Clone + ByteRepr>(a0: Ptr<Vec<T1>>, a1: Vec<T1>) {
    a0.write(a1)
}

fn f27<T1: ByteRepr>(a0: Ptr<Vec<T1>>, a1: Vec<T1>) {
    a0.write(a1)
}

fn f28<T1: ByteRepr>(a0: Ptr<Vec<T1>>, a1: Vec<T1>) {
    a0.write(a1)
}
