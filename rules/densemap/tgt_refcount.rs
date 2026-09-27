// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::collections::{HashMap, HashSet};
use std::rc::Rc;

// The value is `Value<T2>` and not `Box<T2>`, mirroring rules/unordered_map's
// refcount model: a mapped value the C++ hands out a reference to must be
// shared, not owned by the container.
fn t1<T1, T2>() -> HashMap<T1, Value<T2>> {
    HashMap::new()
}

fn t2<T1>() -> HashSet<T1> {
    HashSet::new()
}

// The CRTP base is the SAME container as the derived DenseMap, so it maps to the
// same HashMap.  T1 (the derived type), T4 (DenseMapInfo traits) and T5
// (DenseMapPair bucket) are representation details with no Rust analogue and are
// deliberately unused -- see the DenseMapInfo note in src.cpp.
fn t3<T1, T2, T3, T4, T5>() -> HashMap<T2, Value<T3>> {
    HashMap::new()
}

// THE ITERATOR -- see the note in tgt_unsafe.rs.  Here the MapRef is
// `Ptr<HashMap<T1, Value<T2>>>`, which is precisely what t1 maps DenseMap to, so
// this is the already-provided `libcc2rs::RefcountHashMapIter<T1, T2>` alias.
// Identity is the KEY, so positions -- not values -- are compared.
fn t4<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::null()
}

fn t5<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::null()
}

fn f1<T1: PartialEq, T2>(
    a0: RefcountHashMapIter<T1, T2>,
    a1: RefcountHashMapIter<T1, T2>,
) -> bool {
    a0 == a1
}

fn f2<T1: PartialEq, T2>(
    a0: RefcountHashMapIter<T1, T2>,
    a1: RefcountHashMapIter<T1, T2>,
) -> bool {
    a0 != a1
}

fn f3<T1: PartialEq, T2>(
    a0: RefcountHashMapIter<T1, T2>,
    a1: RefcountHashMapIter<T1, T2>,
) -> bool {
    a0 == a1
}

fn f4<T1: PartialEq, T2>(
    a0: RefcountHashMapIter<T1, T2>,
    a1: RefcountHashMapIter<T1, T2>,
) -> bool {
    a0 != a1
}

// t6 -- the opaque unit for `llvm::DenseMapInfo<T1>`; see tgt_unsafe.rs and the
// note in src.cpp.  Identical in both models: a stateless traits class carries
// no ownership, so refcounting has nothing to express.
fn t6<T1>() -> () {
    ()
}

fn f5<T1>() -> () {
    ()
}


// t7 -- the bucket type.  rules/pair's refcount t1 is `(Value<T1>, Value<T2>)`;
// here only the SECOND component is shared, because this module's t1 maps
// DenseMap to `HashMap<T1, Value<T2>>` -- the key is owned by the table, the
// mapped value is the thing C++ hands out references to.  `Value<T> =
// Rc<RefCell<T>>` (libcc2rs/src/rc.rs:16), so the constructor is spelled exactly
// as rules/pair spells it: `Rc::new(RefCell::new(..))`, never `Value::new(v)`.
fn t7<T1: Default, T2: Default>() -> (T1, Value<T2>) {
    (T1::default(), Rc::new(RefCell::new(T2::default())))
}

fn f6<T1: Default, T2: Default>() -> (T1, Value<T2>) {
    (T1::default(), Rc::new(RefCell::new(T2::default())))
}

// f7-f10 -- begin()/end().  rules/unordered_map's committed f22/f23/f25/f26
// verbatim in shape: the receiver is `Ptr<HashMap<T1, Value<T2>>>` in BOTH the
// mutable and the const overload, which is exactly what this module's t1 is.
fn f7<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: Ptr<HashMap<T1, Value<T2>>>,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::begin(a0)
}

fn f8<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: Ptr<HashMap<T1, Value<T2>>>,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::end(a0)
}

fn f9<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: Ptr<HashMap<T1, Value<T2>>>,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::begin(a0)
}

fn f10<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: Ptr<HashMap<T1, Value<T2>>>,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::end(a0)
}
