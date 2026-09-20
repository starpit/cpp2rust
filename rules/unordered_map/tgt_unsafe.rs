// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::{MapIterator, UnsafeMapIterator};
use std::collections::BTreeMap;

fn t1<T1, T2>() -> BTreeMap<T1, Box<T2>> {
    BTreeMap::new()
}

fn t2<T1: Clone + Ord, T2>() -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::null()
}

fn t3<T1: Clone + Ord, T2>() -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::null()
}

unsafe fn f1<T1: Ord + Clone, T2: Default>(a0: &mut BTreeMap<T1, Box<T2>>, a1: T1) -> &mut T2 {
    a0.entry(a1).or_default().as_mut()
}

unsafe fn f2<T1, T2>(a0: BTreeMap<T1, Box<T2>>) -> usize {
    a0.len()
}

unsafe fn f3<T1: Ord + Clone, T2>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: UnsafeMapIterator<T1, T2>,
) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::erase(&*a0 as *const BTreeMap<T1, Box<T2>>, &a1)
}

unsafe fn f4<T1, T2>(a0: BTreeMap<T1, Box<T2>>) -> bool {
    a0.is_empty()
}

unsafe fn f5<T1, T2>() -> BTreeMap<T1, Box<T2>> {
    BTreeMap::new()
}

unsafe fn f6<T1: Clone, T2: Clone>(a0: BTreeMap<T1, Box<T2>>) -> BTreeMap<T1, Box<T2>> {
    a0.clone()
}

unsafe fn f7<T1: Ord, T2>(a0: &mut BTreeMap<T1, Box<T2>>, a1: T1) -> *const T2 {
    (a0.get(&a1).expect("out of range!").as_ref() as *const T2)
}

unsafe fn f8<T1: Ord + Clone, T2: Default>(a0: &mut BTreeMap<T1, Box<T2>>, a1: T1) -> &mut T2 {
    a0.entry(a1).or_default().as_mut()
}

unsafe fn f9<T1: Ord + Clone, T2>(a0: BTreeMap<T1, Box<T2>>) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::end(&a0 as *const BTreeMap<T1, Box<T2>>)
}

unsafe fn f10<T1: Ord + Clone, T2>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: T1,
) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T2>>, &a1)
}

unsafe fn f11<T1: PartialEq, T2: PartialEq>(
    a0: UnsafeMapIterator<T1, T2>,
    a1: UnsafeMapIterator<T1, T2>,
) -> bool {
    a0 != a1
}

unsafe fn f12<T1: Ord + Clone, T2>(a0: &mut BTreeMap<T1, Box<T2>>) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::begin(&*a0 as *const BTreeMap<T1, Box<T2>>)
}

unsafe fn f13<T1: PartialEq, T2: PartialEq>(
    a0: UnsafeMapIterator<T1, T2>,
    a1: UnsafeMapIterator<T1, T2>,
) -> bool {
    a0 == a1
}

unsafe fn f14<T1: Ord + Clone, T2>(a0: &mut BTreeMap<T1, Box<T2>>) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::end(&*a0 as *const BTreeMap<T1, Box<T2>>)
}

unsafe fn f15<T1: Ord, T2>(a0: &mut BTreeMap<T1, Box<T2>>, a1: T1) -> *const T2 {
    (a0.get(&a1).expect("out of range!").as_ref() as *const T2)
}

unsafe fn f16<T1: PartialEq, T2: PartialEq>(
    a0: UnsafeMapIterator<T1, T2>,
    a1: UnsafeMapIterator<T1, T2>,
) -> bool {
    a0 == a1
}

unsafe fn f17<T1: Ord + Clone, T2>(a0: BTreeMap<T1, Box<T2>>, a1: T1) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::find_key(&a0 as *const BTreeMap<T1, Box<T2>>, &a1)
}

unsafe fn f18<T1: Ord + Clone, T2>(a0: BTreeMap<T1, Box<T2>>) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::begin(&a0 as *const BTreeMap<T1, Box<T2>>)
}

unsafe fn f19<T1: Clone, T2>(a0: UnsafeMapIterator<T1, T2>) -> UnsafeMapIterator<T1, T2> {
    a0.clone()
}

unsafe fn f20<T1: Ord + Clone, T2>(a0: UnsafeMapIterator<T1, T2>) -> *const T1 {
    a0.first()
}

unsafe fn f21<T1: Ord + Clone, T2>(a0: UnsafeMapIterator<T1, T2>) -> *mut T2 {
    a0.second()
}

unsafe fn f22<T1: Ord + Clone, T2>(a0: UnsafeMapIterator<T1, T2>) -> *const T1 {
    a0.first()
}

unsafe fn f23<T1: Ord + Clone, T2>(a0: UnsafeMapIterator<T1, T2>) -> *mut T2 {
    a0.second()
}

unsafe fn f24<T1, T2>(a0: &mut BTreeMap<T1, Box<T2>>) -> BTreeMap<T1, Box<T2>> {
    std::mem::take(&mut *a0)
}

unsafe fn f25<T1, T2>(a0: &mut BTreeMap<T1, Box<T2>>, a1: &mut BTreeMap<T1, Box<T2>>) {
    *a0 = std::mem::take(&mut *a1)
}

unsafe fn f26<T1: Clone, T2: Clone>(a0: &mut BTreeMap<T1, Box<T2>>, a1: BTreeMap<T1, Box<T2>>) {
    *a0 = a1.clone()
}

unsafe fn f27<T1: PartialEq, T2: PartialEq>(
    a0: UnsafeMapIterator<T1, T2>,
    a1: UnsafeMapIterator<T1, T2>,
) -> bool {
    a0 != a1
}

unsafe fn f28<T1: Ord + Clone, T2>(
    a0: &mut UnsafeMapIterator<T1, T2>,
) -> UnsafeMapIterator<T1, T2> {
    a0.inc();
    a0.clone()
}

unsafe fn f29<T1: Ord + Clone, T2>(
    a0: &mut UnsafeMapIterator<T1, T2>,
) -> UnsafeMapIterator<T1, T2> {
    a0.inc();
    a0.clone()
}

unsafe fn f30<T1, T2>(a0: &mut BTreeMap<T1, Box<T2>>) {
    a0.clear()
}

unsafe fn f31<T1: Ord, T2>(a0: BTreeMap<T1, Box<T2>>, a1: T1) -> usize {
    if a0.contains_key(&a1) {
        1_usize
    } else {
        0_usize
    }
}

unsafe fn f32<T1: Ord + Clone, T2: Clone>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: (T1, T2),
) -> (UnsafeMapIterator<T1, T2>, bool) {
    let __pair = a1;
    let __key: T1 = <T1>::clone(&__pair.0);
    let __value: T2 = <T2>::clone(&__pair.1);
    let __inserted = if a0.contains_key(&__key) {
        false
    } else {
        a0.insert(__key.clone(), Box::new(__value));
        true
    };
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T2>>, &__key),
        __inserted,
    )
}

unsafe fn f33<T1: Ord + Clone, T2: Clone>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: &mut (T1, T2),
) -> (UnsafeMapIterator<T1, T2>, bool) {
    let __pair = a1;
    let __key: T1 = <T1>::clone(&__pair.0);
    let __value: T2 = <T2>::clone(&__pair.1);
    let __inserted = if a0.contains_key(&__key) {
        false
    } else {
        a0.insert(__key.clone(), Box::new(__value));
        true
    };
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T2>>, &__key),
        __inserted,
    )
}

unsafe fn f34<T1: Ord, T2>(a0: &mut BTreeMap<T1, Box<T2>>, a1: T1) -> usize {
    if a0.remove(&a1).is_some() {
        1_usize
    } else {
        0_usize
    }
}

unsafe fn f35<T1: Ord + Clone, T2>(
    a0: &mut UnsafeMapIterator<T1, T2>,
) -> UnsafeMapIterator<T1, T2> {
    let __old = a0.clone();
    a0.inc();
    __old
}

unsafe fn f36<T1: Ord + Clone, T2>(
    a0: &mut UnsafeMapIterator<T1, T2>,
) -> UnsafeMapIterator<T1, T2> {
    let __old = a0.clone();
    a0.inc();
    __old
}

unsafe fn f37<T1: Ord + Clone, T2>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: UnsafeMapIterator<T1, T2>,
) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::erase(&*a0 as *const BTreeMap<T1, Box<T2>>, &a1)
}

unsafe fn f38<T1, T2>(a0: &mut BTreeMap<T1, Box<T2>>, a1: &mut BTreeMap<T1, Box<T2>>) {
    std::mem::swap(&mut *a0, &mut *a1)
}

unsafe fn f39<T1: Ord + Clone, T2>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: T1,
    a2: T2,
) -> (UnsafeMapIterator<T1, T2>, bool) {
    let __key: T1 = a1;
    let __value: T2 = a2;
    let __inserted = if a0.contains_key(&__key) {
        false
    } else {
        a0.insert(__key.clone(), Box::new(__value));
        true
    };
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T2>>, &__key),
        __inserted,
    )
}

unsafe fn f40<T1: Ord + Clone, T2>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: T1,
    a2: T2,
) -> (UnsafeMapIterator<T1, T2>, bool) {
    let __key: T1 = a1;
    let __value: T2 = a2;
    let __inserted = if a0.contains_key(&__key) {
        false
    } else {
        a0.insert(__key.clone(), Box::new(__value));
        true
    };
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T2>>, &__key),
        __inserted,
    )
}

unsafe fn f41<T1: Ord + Clone, T2: Clone>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: &mut (T1, T2),
) -> (UnsafeMapIterator<T1, T2>, bool) {
    let __pair = a1;
    let __key: T1 = <T1>::clone(&__pair.0);
    let __value: T2 = <T2>::clone(&__pair.1);
    let __inserted = if a0.contains_key(&__key) {
        false
    } else {
        a0.insert(__key.clone(), Box::new(__value));
        true
    };
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T2>>, &__key),
        __inserted,
    )
}

unsafe fn f42<T1: Ord + Clone, T2: Clone>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: &mut (T1, T2),
) -> (UnsafeMapIterator<T1, T2>, bool) {
    let __pair = a1;
    let __key: T1 = <T1>::clone(&__pair.0);
    let __value: T2 = <T2>::clone(&__pair.1);
    let __inserted = if a0.contains_key(&__key) {
        false
    } else {
        a0.insert(__key.clone(), Box::new(__value));
        true
    };
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T2>>, &__key),
        __inserted,
    )
}

unsafe fn f43<T1: Ord + Clone, T2: Clone>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: (T1, T2),
) -> (UnsafeMapIterator<T1, T2>, bool) {
    let __pair = a1;
    let __key: T1 = <T1>::clone(&__pair.0);
    let __value: T2 = <T2>::clone(&__pair.1);
    let __inserted = if a0.contains_key(&__key) {
        false
    } else {
        a0.insert(__key.clone(), Box::new(__value));
        true
    };
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T2>>, &__key),
        __inserted,
    )
}

unsafe fn f44<T1: Ord + Clone, T2: Clone>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: &mut (T1, T2),
) -> (UnsafeMapIterator<T1, T2>, bool) {
    let __pair = a1;
    let __key: T1 = <T1>::clone(&__pair.0);
    let __value: T2 = <T2>::clone(&__pair.1);
    let __inserted = if a0.contains_key(&__key) {
        false
    } else {
        a0.insert(__key.clone(), Box::new(__value));
        true
    };
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T2>>, &__key),
        __inserted,
    )
}

unsafe fn f45<T1: Ord + Clone, T2>(a0: BTreeMap<T1, Box<T2>>) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::begin(&a0 as *const BTreeMap<T1, Box<T2>>)
}

unsafe fn f46<T1: Ord + Clone, T2>(a0: BTreeMap<T1, Box<T2>>) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::end(&a0 as *const BTreeMap<T1, Box<T2>>)
}

unsafe fn f47<T1: PartialEq, T2: PartialEq>(a0: BTreeMap<T1, Box<T2>>, a1: BTreeMap<T1, Box<T2>>) -> bool {
    a0 == a1
}

unsafe fn f48<T1: PartialEq, T2: PartialEq>(a0: BTreeMap<T1, Box<T2>>, a1: BTreeMap<T1, Box<T2>>) -> bool {
    a0 != a1
}
