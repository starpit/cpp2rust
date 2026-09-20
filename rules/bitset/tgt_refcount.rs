// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// Overlay on tgt_unsafe.rs: only the rules whose receiver is MUTATED differ,
// because in the refcount model a `std::bitset<N> &` receiver arrives as a
// Ptr<Vec<bool>> and has to be written through with_mut/write.  Every
// const/observer rule (f1, f2, f3, f5, f6, f11..f19) is inherited unchanged.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn f4(a0: Ptr<Vec<bool>>, a1: Vec<bool>) {
    a0.write(a1)
}

fn f7(a0: Ptr<Vec<bool>>, a1: usize, a2: bool) {
    a0.with_mut(|__v: &mut Vec<bool>| {
        if __v.len() <= a1 {
            __v.resize(a1 + 1, false);
        }
        __v[a1] = a2;
    })
}

fn f8(a0: Ptr<Vec<bool>>, a1: usize) {
    a0.with_mut(|__v: &mut Vec<bool>| {
        if __v.len() <= a1 {
            __v.resize(a1 + 1, false);
        }
        __v[a1] = false;
    })
}

fn f9(a0: Ptr<Vec<bool>>) {
    a0.with_mut(|__v: &mut Vec<bool>| __v.clear())
}

fn f10(a0: Ptr<Vec<bool>>, a1: usize) {
    a0.with_mut(|__v: &mut Vec<bool>| {
        if __v.len() <= a1 {
            __v.resize(a1 + 1, false);
        }
        __v[a1] = !__v[a1];
    })
}
