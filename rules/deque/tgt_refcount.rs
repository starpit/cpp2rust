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

fn f13<T1: ByteRepr>(a0: Ptr<Vec<Value<Vec<T1>>>>, init: Vec<T1>) {
    let __init = init;
    a0.with_mut(|__v: &mut Vec<Value<Vec<T1>>>| __v.push(Rc::new(RefCell::new(__init))))
}

fn f15<T1: PartialEq>(a0: &Vec<T1>, a1: &Vec<T1>) -> bool {
    a0 == a1
}

fn f16<T1: PartialEq>(a0: &Vec<T1>, a1: &Vec<T1>) -> bool {
    a0 != a1
}

fn f17<T1: ByteRepr>(a0: Ptr<Vec<T1>>, a1: T1) {
    a0.with_mut(|__v: &mut Vec<T1>| __v.push(a1))
}

// f18 -- `at(i)`, NON-CONST.  Copied from `rules/vector`'s f7, which is the key
// that is measurably correct at these sites: it emits
// `(recv.decay() as Ptr<i64>).offset(i)`, an ELEMENT-stride step producing a
// writable `Ptr`, so `coordinates.at(ci).at(di) = rhs` lowers to a `.write()` on
// the owner's storage rather than on a temporary.
fn f18<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

// f19 -- `at(i) const`.  Same handle; `Ptr` carries no const distinction.
// ⛔ Deliberately f98's `Ptr<T1>` and NOT f50's `Ptr<Vec<T1>>` -- see the note in
// src.cpp: f50's spelling gives a `Vec<T1>`-stride offset and a `.read()` of the
// wrong type, which is a latent bug there and must not be propagated.
fn f19<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

