// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::rc::Rc;

// std::unordered_set<T> is modelled as BTreeMap<T, Value<T>>: the key drives
// lookup/ordering and the mapped slot holds a second copy of the element so
// that `*it`, which is an lvalue of type `const T &` in C++, can be handed back
// as a Ptr into a cell that stays alive as long as the element is in the set.
// Reusing BTreeMap means the existing RefcountMapIter covers the iterators.
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
    if a0.contains_key(&a1) {
        1_usize
    } else {
        0_usize
    }
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
    let __key: T1 = a1;
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), Rc::new(RefCell::new(__key.clone())));
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f12<T1: Ord + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>, a1: T1) -> usize {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| {
        if __v.remove(&a1).is_some() {
            1_usize
        } else {
            0_usize
        }
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

fn f15<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>, a1: BTreeMap<T1, Value<T1>>) {
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

// `*it` is an lvalue of type `const T &`, so the converter expects a Ptr it can
// read through; the mapped slot is the cell that outlives this expression.
fn f18<T1: Ord + Clone + 'static>(a0: RefcountMapIter<T1, T1>) -> Ptr<T1> {
    a0.second().as_pointer()
}

fn f19<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> (Value<RefcountMapIter<T1, T1>>, Value<bool>) {
    let __key: T1 = a1;
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), Rc::new(RefCell::new(__key.clone())));
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f20<T1: Ord + Clone + 'static>(a0: &mut RefcountMapIter<T1, T1>) -> RefcountMapIter<T1, T1> {
    a0.inc();
    a0.clone()
}

fn f21<T1: Ord + Clone + 'static>(a0: &mut RefcountMapIter<T1, T1>) -> RefcountMapIter<T1, T1> {
    let __old = a0.clone();
    a0.inc();
    __old
}

fn f22<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: RefcountMapIter<T1, T1>,
) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::erase(a0, &a1)
}

fn f23<T1: 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> BTreeMap<T1, Value<T1>> {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| std::mem::take(__v))
}

fn f24<T1: 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>, a1: Ptr<BTreeMap<T1, Value<T1>>>) {
    let __src = a1.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| std::mem::take(__v));
    a0.write(__src)
}

fn f25<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> (Value<RefcountMapIter<T1, T1>>, Value<bool>) {
    let __key: T1 = a1;
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), Rc::new(RefCell::new(__key.clone())));
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f26<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> (Value<RefcountMapIter<T1, T1>>, Value<bool>) {
    let __key: T1 = a1;
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), Rc::new(RefCell::new(__key.clone())));
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f27<T1: 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>, a1: Ptr<BTreeMap<T1, Value<T1>>>) {
    let __lhs = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| std::mem::take(__v));
    let __rhs = a1.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| std::mem::take(__v));
    a0.write(__rhs);
    a1.write(__lhs)
}

fn f28<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::begin(a0)
}

fn f29<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::end(a0)
}


fn f30<T1: PartialEq>(a0: BTreeMap<T1, Value<T1>>, a1: BTreeMap<T1, Value<T1>>) -> bool {
    a0 == a1
}

fn f31<T1: PartialEq>(a0: BTreeMap<T1, Value<T1>>, a1: BTreeMap<T1, Value<T1>>) -> bool {
    a0 != a1
}
