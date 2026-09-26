// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::collections::BTreeMap;

// element -> multiplicity.  The mapped slot is a plain usize, not a Value<usize>:
// no rule here hands out a reference into it (there is no iterator and no `*it`),
// so nothing needs a cell to point at.
fn t1<T1>() -> BTreeMap<T1, usize> {
    BTreeMap::new()
}

fn f1<T1>(a0: BTreeMap<T1, usize>) -> usize {
    a0.values().copied().sum::<usize>()
}

fn f2<T1>(a0: BTreeMap<T1, usize>) -> bool {
    a0.is_empty()
}

fn f3<T1: 'static>(a0: Ptr<BTreeMap<T1, usize>>) {
    a0.with_mut(|__v: &mut BTreeMap<T1, usize>| __v.clear())
}

fn f4<T1: Ord>(a0: BTreeMap<T1, usize>, a1: T1) -> usize {
    a0.get(&a1).copied().unwrap_or(0)
}

fn f5<T1>() -> BTreeMap<T1, usize> {
    BTreeMap::new()
}

fn f6<T1: Ord + DeepClone>(a0: BTreeMap<T1, usize>) -> BTreeMap<T1, usize> {
    a0.deep_clone()
}

fn f7<T1: Ord + DeepClone + 'static>(
    a0: Ptr<BTreeMap<T1, usize>>,
    a1: BTreeMap<T1, usize>,
) {
    a0.write(a1.deep_clone())
}

fn f8<T1: PartialEq>(a0: BTreeMap<T1, usize>, a1: BTreeMap<T1, usize>) -> bool {
    a0 == a1
}

fn f9<T1: PartialEq>(a0: BTreeMap<T1, usize>, a1: BTreeMap<T1, usize>) -> bool {
    a0 != a1
}

fn f10<T1: Ord + 'static>(a0: Ptr<BTreeMap<T1, usize>>, a1: T1) -> usize {
    a0.with_mut(|__v: &mut BTreeMap<T1, usize>| __v.remove(&a1).unwrap_or(0))
}

fn f11<T1: Ord>(a0: BTreeMap<T1, usize>) -> BTreeMap<T1, usize> {
    a0
}

fn f12<T1: Ord + 'static>(a0: Ptr<BTreeMap<T1, usize>>, a1: BTreeMap<T1, usize>) {
    a0.write(a1)
}

fn f13<T1: Ord + 'static>(a0: Ptr<BTreeMap<T1, usize>>, a1: Ptr<BTreeMap<T1, usize>>) {
    a0.with_mut(|__x: &mut BTreeMap<T1, usize>| {
        a1.with_mut(|__y: &mut BTreeMap<T1, usize>| std::mem::swap(__x, __y))
    })
}

fn f14<T1: Ord>(a0: Vec<T1>, a1: Option<()>) -> BTreeMap<T1, usize> {
    let mut __m: BTreeMap<T1, usize> = BTreeMap::new();
    for __k in a0.into_iter() {
        *__m.entry(__k).or_insert(0) += 1;
    }
    __m
}
