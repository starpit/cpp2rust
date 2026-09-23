// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::rc::Rc;

fn t1<T1>() -> BTreeMap<T1, Value<T1>> {
    BTreeMap::new()
}

fn t2<T1: Clone + Ord + 'static>() -> RefcountMapIter<T1, T1> {
    RefcountMapIter::null()
}

fn f1<T1>(a0: BTreeMap<T1, Value<T1>>) -> usize {
    a0.len()
}

fn f2<T1>(a0: BTreeMap<T1, Value<T1>>) -> bool {
    a0.is_empty()
}

fn f3<T1: 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| __v.clear())
}

fn f4<T1: Ord>(a0: BTreeMap<T1, Value<T1>>, a1: T1) -> usize {
    if a0.contains_key(&a1) { 1 } else { 0 }
}

fn f5<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f6<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f7<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::begin(a0)
}

fn f8<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::end(a0)
}

fn f9<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::begin(a0)
}

fn f10<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::end(a0)
}

fn f11<T1: Ord + Clone + 'static>(
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

fn f12<T1: Ord + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>, a1: T1) -> usize {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| {
        if __v.remove(&a1).is_some() { 1 } else { 0 }
    })
}

fn f13<T1>() -> BTreeMap<T1, Value<T1>> {
    BTreeMap::new()
}

fn f14<T1: Ord + Clone>(a0: BTreeMap<T1, Value<T1>>) -> BTreeMap<T1, Value<T1>> {
    a0.keys()
        .map(|k| (k.clone(), Rc::new(RefCell::new(k.clone()))))
        .collect()
}

fn f15<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: BTreeMap<T1, Value<T1>>,
) {
    a0.write(
        a1.keys()
            .map(|k| (k.clone(), Rc::new(RefCell::new(k.clone()))))
            .collect(),
    )
}

fn f16<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 != a1
}

fn f17<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 == a1
}

fn f18<T1: Ord + Clone + 'static>(a0: RefcountMapIter<T1, T1>) -> Ptr<T1> {
    a0.second().as_pointer()
}

fn f19<T1: Ord + Clone + 'static>(
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

fn f20<T1: Ord + Clone + 'static>(
    a0: &mut RefcountMapIter<T1, T1>,
) -> RefcountMapIter<T1, T1> {
    a0.inc();
    a0.clone()
}

fn f21<T1: Ord + Clone + 'static>(
    a0: &mut RefcountMapIter<T1, T1>,
) -> RefcountMapIter<T1, T1> {
    let __old = a0.clone();
    a0.inc();
    __old
}


fn f22<T1: PartialEq>(a0: BTreeMap<T1, Value<T1>>, a1: BTreeMap<T1, Value<T1>>) -> bool {
    a0 == a1
}

fn f23<T1: PartialEq>(a0: BTreeMap<T1, Value<T1>>, a1: BTreeMap<T1, Value<T1>>) -> bool {
    a0 != a1
}

fn f24<T1: Ord + Clone>(a0: Vec<T1>, a1: Option<()>) -> BTreeMap<T1, Value<T1>> {
    let mut __m: BTreeMap<T1, Value<T1>> = BTreeMap::new();
    for __k in a0.into_iter() {
        if !__m.contains_key(&__k) {
            __m.insert(__k.clone(), Rc::new(RefCell::new(__k)));
        }
    }
    __m
}
