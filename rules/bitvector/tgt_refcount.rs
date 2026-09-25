// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// Overlay on tgt_unsafe.rs: only the rules whose receiver is MUTATED differ,
// because in the refcount model a `llvm::BitVector &` receiver arrives as a
// Ptr<Vec<bool>> and has to be written through with_mut/write.  Every
// const/observer rule (f1, f2, f3, f5..f14, f24, f25) is inherited unchanged.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn f4(a0: Ptr<Vec<bool>>, a1: Vec<bool>) {
    a0.write(a1)
}

fn f15(a0: Ptr<Vec<bool>>) {
    a0.with_mut(|__v: &mut Vec<bool>| __v.clear())
}

fn f16(a0: Ptr<Vec<bool>>, a1: u32, a2: bool) {
    a0.with_mut(|__v: &mut Vec<bool>| __v.resize(a1 as usize, a2))
}

fn f17(a0: Ptr<Vec<bool>>) {
    a0.with_mut(|__v: &mut Vec<bool>| __v.fill(true))
}

fn f18(a0: Ptr<Vec<bool>>, a1: u32) {
    a0.with_mut(|__v: &mut Vec<bool>| __v[a1 as usize] = true)
}

fn f19(a0: Ptr<Vec<bool>>) {
    a0.with_mut(|__v: &mut Vec<bool>| __v.fill(false))
}

fn f20(a0: Ptr<Vec<bool>>, a1: u32) {
    a0.with_mut(|__v: &mut Vec<bool>| __v[a1 as usize] = false)
}

fn f21(a0: Ptr<Vec<bool>>) {
    a0.with_mut(|__v: &mut Vec<bool>| {
        for __b in __v.iter_mut() {
            *__b = !*__b;
        }
    })
}

fn f22(a0: Ptr<Vec<bool>>, a1: u32) {
    a0.with_mut(|__v: &mut Vec<bool>| {
        let __i = a1 as usize;
        __v[__i] = !__v[__i];
    })
}

fn f23(a0: Ptr<Vec<bool>>, a1: bool) {
    a0.with_mut(|__v: &mut Vec<bool>| __v.push(a1))
}
