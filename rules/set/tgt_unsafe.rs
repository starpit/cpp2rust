// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::{MapIterator, UnsafeMapIterator};
use std::collections::BTreeMap;

fn t1<T1>() -> BTreeMap<T1, Box<T1>> {
    BTreeMap::new()
}

fn t2<T1: Clone + Ord>() -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::null()
}

unsafe fn f1<T1>(a0: BTreeMap<T1, Box<T1>>) -> usize {
    a0.len()
}

unsafe fn f2<T1>(a0: BTreeMap<T1, Box<T1>>) -> bool {
    a0.is_empty()
}

unsafe fn f3<T1>(a0: &mut BTreeMap<T1, Box<T1>>) {
    a0.clear()
}

unsafe fn f4<T1: Ord>(a0: BTreeMap<T1, Box<T1>>, a1: T1) -> usize {
    if a0.contains_key(&a1) { 1 } else { 0 }
}

unsafe fn f5<T1: Ord + Clone>(
    a0: &mut BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T1>>, &a1)
}

unsafe fn f6<T1: Ord + Clone>(
    a0: BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::find_key(&a0 as *const BTreeMap<T1, Box<T1>>, &a1)
}

unsafe fn f7<T1: Ord + Clone>(a0: &mut BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::begin(&*a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f8<T1: Ord + Clone>(a0: &mut BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::end(&*a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f9<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::begin(&a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f10<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::end(&a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f11<T1: Ord + Clone>(
    a0: &mut BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> (UnsafeMapIterator<T1, T1>, bool) {
    let __inserted = a0.insert(a1.clone(), Box::new(a1.clone())).is_none();
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T1>>, &a1),
        __inserted,
    )
}

unsafe fn f12<T1: Ord>(a0: &mut BTreeMap<T1, Box<T1>>, a1: T1) -> usize {
    if a0.remove(&a1).is_some() { 1 } else { 0 }
}

unsafe fn f13<T1>() -> BTreeMap<T1, Box<T1>> {
    BTreeMap::new()
}

unsafe fn f14<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>) -> BTreeMap<T1, Box<T1>> {
    a0.keys().map(|k| (k.clone(), Box::new(k.clone()))).collect()
}

unsafe fn f15<T1: Ord + Clone>(a0: &mut BTreeMap<T1, Box<T1>>, a1: BTreeMap<T1, Box<T1>>) {
    *a0 = a1.keys().map(|k| (k.clone(), Box::new(k.clone()))).collect()
}

unsafe fn f16<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 != a1
}

unsafe fn f17<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 == a1
}

unsafe fn f18<T1: Ord + Clone>(a0: UnsafeMapIterator<T1, T1>) -> *const T1 {
    a0.second()
}

unsafe fn f19<T1: Ord + Clone>(
    a0: &mut BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> (UnsafeMapIterator<T1, T1>, bool) {
    let __inserted = a0.insert(a1.clone(), Box::new(a1.clone())).is_none();
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T1>>, &a1),
        __inserted,
    )
}

unsafe fn f20<T1: Ord + Clone>(
    a0: &mut UnsafeMapIterator<T1, T1>,
) -> UnsafeMapIterator<T1, T1> {
    a0.inc();
    a0.clone()
}

unsafe fn f21<T1: Ord + Clone>(
    a0: &mut UnsafeMapIterator<T1, T1>,
) -> UnsafeMapIterator<T1, T1> {
    let __old = a0.clone();
    a0.inc();
    __old
}

unsafe fn f22<T1: PartialEq>(a0: BTreeMap<T1, Box<T1>>, a1: BTreeMap<T1, Box<T1>>) -> bool {
    a0 == a1
}

unsafe fn f23<T1: PartialEq>(a0: BTreeMap<T1, Box<T1>>, a1: BTreeMap<T1, Box<T1>>) -> bool {
    a0 != a1
}

unsafe fn f24<T1: Ord + Clone>(a0: Vec<T1>, a1: Option<()>) -> BTreeMap<T1, Box<T1>> {
    a0.into_iter()
        .map(|__k: T1| (__k.clone(), Box::new(__k)))
        .collect()
}
