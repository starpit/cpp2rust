// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::{PostfixInc, PrefixInc, SetIterator};


fn t1<T1>() -> std::collections::BTreeSet<T1> {
    std::collections::BTreeSet::new()
}

unsafe fn f1<T1>() -> std::collections::BTreeSet<T1> {
    std::collections::BTreeSet::new()
}

unsafe fn f2<T1>(a0: std::collections::BTreeSet<T1>) -> usize {
    a0.len()
}

unsafe fn f3<T1>(a0: std::collections::BTreeSet<T1>) -> bool {
    a0.is_empty()
}

unsafe fn f4<T1>(a0: &mut std::collections::BTreeSet<T1>) {
    a0.clear()
}

unsafe fn f5<T1: Ord>(a0: std::collections::BTreeSet<T1>, a1: T1) -> usize {
    (std::collections::BTreeSet::contains(&a0, &a1) as usize)
}

unsafe fn f6<T1: Ord>(a0: &mut std::collections::BTreeSet<T1>, a1: T1) -> usize {
    (std::collections::BTreeSet::remove(&mut *a0, &a1) as usize)
}

unsafe fn f7<T1: PartialEq>(a0: &std::collections::BTreeSet<T1>, a1: &std::collections::BTreeSet<T1>) -> bool {
    a0 == a1
}

unsafe fn f8<T1: PartialEq>(a0: &std::collections::BTreeSet<T1>, a1: &std::collections::BTreeSet<T1>) -> bool {
    a0 != a1
}

unsafe fn f9<T1: Clone>(a0: std::collections::BTreeSet<T1>) -> std::collections::BTreeSet<T1> {
    a0.clone()
}

unsafe fn f10<T1>(a0: &mut std::collections::BTreeSet<T1>) -> std::collections::BTreeSet<T1> {
    std::mem::take(&mut *a0)
}

unsafe fn f11<T1: Clone + Ord>(a0: &mut std::collections::BTreeSet<T1>, a1: std::collections::BTreeSet<T1>) {
    let __src = a1.clone();
    a0.clear();
    a0.extend(__src);
}

unsafe fn f12<T1: Ord>(a0: &mut std::collections::BTreeSet<T1>, a1: &mut std::collections::BTreeSet<T1>) {
    let __src = std::mem::take(&mut *a1);
    a0.clear();
    a0.extend(__src);
}

fn t2<T1: Ord + Clone>() -> libcc2rs::UnsafeSetIterator<T1> {
    libcc2rs::UnsafeSetIterator::null()
}

unsafe fn f13<T1: Ord + Clone>(
    a0: &mut std::collections::BTreeSet<T1>,
    a1: T1,
) -> (libcc2rs::UnsafeSetIterator<T1>, bool) {
    {
        let __k = a1;
        let __set = &mut *a0;
        let __inserted = std::collections::BTreeSet::insert(__set, __k.clone());
        (
            libcc2rs::UnsafeSetIterator::find_key(
                __set as *const std::collections::BTreeSet<T1>,
                &__k,
            ),
            __inserted,
        )
    }
}

unsafe fn f14<T1: Ord + Clone>(
    a0: std::collections::BTreeSet<T1>,
) -> libcc2rs::UnsafeSetIterator<T1> {
    libcc2rs::UnsafeSetIterator::begin(&a0 as *const std::collections::BTreeSet<T1>)
}

unsafe fn f15<T1: Ord + Clone>(
    a0: std::collections::BTreeSet<T1>,
) -> libcc2rs::UnsafeSetIterator<T1> {
    libcc2rs::UnsafeSetIterator::end(&a0 as *const std::collections::BTreeSet<T1>)
}

unsafe fn f16<T1: Ord + Clone>(
    a0: std::collections::BTreeSet<T1>,
) -> libcc2rs::UnsafeSetIterator<T1> {
    libcc2rs::UnsafeSetIterator::begin(&a0 as *const std::collections::BTreeSet<T1>)
}

unsafe fn f17<T1: Ord + Clone>(
    a0: std::collections::BTreeSet<T1>,
) -> libcc2rs::UnsafeSetIterator<T1> {
    libcc2rs::UnsafeSetIterator::end(&a0 as *const std::collections::BTreeSet<T1>)
}

unsafe fn f18<T1: PartialEq>(
    a0: libcc2rs::UnsafeSetIterator<T1>,
    a1: libcc2rs::UnsafeSetIterator<T1>,
) -> bool {
    a0 == a1
}

unsafe fn f19<T1: PartialEq>(
    a0: libcc2rs::UnsafeSetIterator<T1>,
    a1: libcc2rs::UnsafeSetIterator<T1>,
) -> bool {
    a0 != a1
}

unsafe fn f20<T1: Ord + Clone>(
    a0: &mut libcc2rs::UnsafeSetIterator<T1>,
) -> libcc2rs::UnsafeSetIterator<T1> {
    a0.prefix_inc()
}

unsafe fn f21<T1: Ord + Clone>(
    a0: &mut libcc2rs::UnsafeSetIterator<T1>,
) -> libcc2rs::UnsafeSetIterator<T1> {
    a0.postfix_inc()
}

unsafe fn f22<T1: Ord + Clone>(a0: libcc2rs::UnsafeSetIterator<T1>) -> *const T1 {
    libcc2rs::SetIterator::element(&a0)
}

unsafe fn f23<T1: Ord + Clone>(
    a0: &mut std::collections::BTreeSet<T1>,
    a1: T1,
) -> (libcc2rs::UnsafeSetIterator<T1>, bool) {
    {
        let __k = a1;
        let __set = &mut *a0;
        let __inserted = std::collections::BTreeSet::insert(__set, __k.clone());
        (
            libcc2rs::UnsafeSetIterator::find_key(
                __set as *const std::collections::BTreeSet<T1>,
                &__k,
            ),
            __inserted,
        )
    }
}
