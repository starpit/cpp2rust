// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Refcount model for llvm::SmallVector.  Follows rules/array and rules/vector:
// neither defines a target TYPE rule for the plain container (array defines
// none at all; vector defines only the iterator and nested-container ones), so
// t1..t4 are omitted here too, and only the members whose refcount shape is
// established by those two modules are given bodies.  Everything else is left
// out rather than guessed. (An earlier version of this comment said an unmapped
// member becomes an `unimplemented!()`; that is STALE. The sole emitter of that
// was replaced by a loud refusal in converter.cpp, and `grep -rn 'unimplemented!'`
// over the converter, the rule preprocessor and libcc2rs now finds only comments.
// An unmapped member FAILS AT TRANSLATE TIME, which is loud where a wrong body
// would be silent -- and that is the point.)

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn f1<T1: ByteRepr>(a0: Ptr<Vec<T1>>, a1: T1) {
    a0.with_mut(|__v: &mut Vec<T1>| __v.push(a1))
}

fn f7<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

fn f8<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f9<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f10<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f11<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f12<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_last()
}

// `llvm::SmallVectorImpl<T1>::operator==`.  Written EXPLICITLY: omitting it does
// not drop the key, it silently falls back to the unsafe body.  Same shape as
// rules/vector's refcount f115, where a std::vector comparison also takes
// `&Vec<T1>` directly rather than a `Ptr`.
fn f20<T1: PartialEq>(a0: &Vec<T1>, a1: &Vec<T1>) -> bool {
    a0 == a1
}

// llvm::SmallString<_>. Written EXPLICITLY, not omitted: an omitted target does
// not drop the key, it silently falls back to the unsafe body, and the unsafe
// bodies spell the element type `libc::c_char` where the refcount model spells
// it `u8` (compare rules/string's f38/f39, which differ from their unsafe twins
// in exactly that way and in nothing else).
fn t10<T1>() -> Vec<u8> {
    Default::default()
}

fn f21() -> Vec<u8> {
    Vec::new()
}

fn f22(a0: &mut Vec<u8>, a1: u8) {
    a0.push(a1);
}

fn f23(a0: &mut Vec<u8>, a1: Vec<u8>) {
    a0.extend_from_slice(&a1[..a1.len() - 1]);
}

// g796/g797.  Written EXPLICITLY for the same reason f20 is: an omitted target
// does not drop the key, it silently falls back to the unsafe body.
fn f24<T1: PartialEq>(a0: &Vec<T1>, a1: &Vec<T1>) -> bool {
    a0 != a1
}

fn f25<T1: PartialEq>(a0: &Vec<T1>, a1: Vec<T1>) -> bool {
    *a0 != a1
}

// f40/f41 -- `rbegin()`/`rend()`, mirroring f9/f8 verbatim: the refcount receiver
// for `SmallVectorTemplateCommon<T1> &` is already lowered to `Ptr<T1>`, and
// `to_end()` is the same end-of-vector handle f9 returns.
fn f40<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f41<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}
