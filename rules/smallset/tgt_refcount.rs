// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp, and see tgt_unsafe.rs for why N does not appear on this side.
// The same reuse: rules/set's iterator type, RefcountMapIter<T1, T1>.

use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::rc::Rc;

fn t1<T1: Clone + Ord + 'static>() -> RefcountMapIter<T1, T1> {
    RefcountMapIter::null()
}

fn t2<T1: Clone + Ord + 'static>() -> RefcountMapIter<T1, T1> {
    RefcountMapIter::null()
}

fn f1<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 == a1
}

fn f2<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 != a1
}


// The container type -- the SAME representation rules/set gives std::set.
// See src.cpp: the TYPE only; no member of SmallSet is modelled here.
fn t3<T1>() -> BTreeMap<T1, Value<T1>> {
    BTreeMap::new()
}

// SmallSet's members -- rules/set's bodies for the refcount model.  See src.cpp.
fn f3<T1>() -> BTreeMap<T1, Value<T1>> {
    BTreeMap::new()
}

fn f4<T1>(a0: BTreeMap<T1, Value<T1>>) -> usize {
    a0.len()
}

fn f5<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> (Value<RefcountMapIter<T1, T1>>, Value<bool>) {
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| {
        let __new = !__v.contains_key(&a1);
        if __new {
            __v.insert(a1.clone(), Rc::new(RefCell::new(a1.clone())));
        }
        __new
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &a1))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f6<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> (Value<RefcountMapIter<T1, T1>>, Value<bool>) {
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| {
        let __new = !__v.contains_key(&a1);
        if __new {
            __v.insert(a1.clone(), Rc::new(RefCell::new(a1.clone())));
        }
        __new
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &a1))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f7<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::begin(a0)
}

fn f8<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::end(a0)
}
