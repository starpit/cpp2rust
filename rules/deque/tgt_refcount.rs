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
