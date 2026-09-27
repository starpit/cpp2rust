// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::{PostfixInc, PrefixInc, SetIterator};


fn t1<T1>() -> std::collections::BTreeSet<T1> {
    std::collections::BTreeSet::new()
}

fn f1<T1>() -> std::collections::BTreeSet<T1> {
    std::collections::BTreeSet::new()
}

fn f2<T1>(a0: std::collections::BTreeSet<T1>) -> usize {
    a0.len()
}

fn f3<T1>(a0: std::collections::BTreeSet<T1>) -> bool {
    a0.is_empty()
}

fn f4<T1>(a0: &mut std::collections::BTreeSet<T1>) {
    a0.clear()
}

fn f5<T1: Ord>(a0: std::collections::BTreeSet<T1>, a1: T1) -> usize {
    (std::collections::BTreeSet::contains(&a0, &a1) as usize)
}

fn f6<T1: Ord>(a0: &mut std::collections::BTreeSet<T1>, a1: T1) -> usize {
    (std::collections::BTreeSet::remove(&mut *a0, &a1) as usize)
}

fn f7<T1: PartialEq>(a0: &std::collections::BTreeSet<T1>, a1: &std::collections::BTreeSet<T1>) -> bool {
    a0 == a1
}

fn f8<T1: PartialEq>(a0: &std::collections::BTreeSet<T1>, a1: &std::collections::BTreeSet<T1>) -> bool {
    a0 != a1
}

fn f9<T1: Clone>(a0: std::collections::BTreeSet<T1>) -> std::collections::BTreeSet<T1> {
    a0.clone()
}

fn f10<T1>(a0: &mut std::collections::BTreeSet<T1>) -> std::collections::BTreeSet<T1> {
    std::mem::take(&mut *a0)
}

fn f11<T1: Clone>(a0: &mut std::collections::BTreeSet<T1>, a1: std::collections::BTreeSet<T1>) {
    *a0 = a1.clone()
}

fn f12<T1>(a0: &mut std::collections::BTreeSet<T1>, a1: &mut std::collections::BTreeSet<T1>) {
    *a0 = std::mem::take(&mut *a1)
}

fn t2<T1: Ord + Clone + 'static>() -> libcc2rs::RefcountSetIter<T1> {
    libcc2rs::RefcountSetIter::null()
}

fn f13<T1: Ord + Clone + libcc2rs::ByteRepr + 'static>(
    a0: libcc2rs::Ptr<std::collections::BTreeSet<T1>>,
    a1: T1,
) -> (libcc2rs::RefcountSetIter<T1>, bool) {
    {
        let __p = a0;
        let __k = a1;
        let __inserted = libcc2rs::Ptr::with_mut(
            &__p,
            |__s: &mut std::collections::BTreeSet<T1>| {
                std::collections::BTreeSet::insert(__s, __k.clone())
            },
        );
        (libcc2rs::RefcountSetIter::find_key(__p, &__k), __inserted)
    }
}

fn f14<T1: Ord + Clone + 'static>(
    a0: libcc2rs::Ptr<std::collections::BTreeSet<T1>>,
) -> libcc2rs::RefcountSetIter<T1> {
    libcc2rs::RefcountSetIter::begin(a0)
}

fn f15<T1: Ord + Clone + 'static>(
    a0: libcc2rs::Ptr<std::collections::BTreeSet<T1>>,
) -> libcc2rs::RefcountSetIter<T1> {
    libcc2rs::RefcountSetIter::end(a0)
}

fn f16<T1: Ord + Clone + 'static>(
    a0: libcc2rs::Ptr<std::collections::BTreeSet<T1>>,
) -> libcc2rs::RefcountSetIter<T1> {
    libcc2rs::RefcountSetIter::begin(a0)
}

fn f17<T1: Ord + Clone + 'static>(
    a0: libcc2rs::Ptr<std::collections::BTreeSet<T1>>,
) -> libcc2rs::RefcountSetIter<T1> {
    libcc2rs::RefcountSetIter::end(a0)
}

fn f18<T1: PartialEq>(
    a0: libcc2rs::RefcountSetIter<T1>,
    a1: libcc2rs::RefcountSetIter<T1>,
) -> bool {
    a0 == a1
}

fn f19<T1: PartialEq>(
    a0: libcc2rs::RefcountSetIter<T1>,
    a1: libcc2rs::RefcountSetIter<T1>,
) -> bool {
    a0 != a1
}

fn f20<T1: Ord + Clone + 'static>(
    a0: &mut libcc2rs::RefcountSetIter<T1>,
) -> libcc2rs::RefcountSetIter<T1> {
    a0.prefix_inc()
}

fn f21<T1: Ord + Clone + 'static>(
    a0: &mut libcc2rs::RefcountSetIter<T1>,
) -> libcc2rs::RefcountSetIter<T1> {
    a0.postfix_inc()
}

fn f22<T1: Ord + Clone + 'static>(
    a0: libcc2rs::RefcountSetIter<T1>,
) -> libcc2rs::Value<T1> {
    libcc2rs::SetIterator::element(&a0)
}

fn f23<T1: Ord + Clone + libcc2rs::ByteRepr + 'static>(
    a0: libcc2rs::Ptr<std::collections::BTreeSet<T1>>,
    a1: T1,
) -> (libcc2rs::RefcountSetIter<T1>, bool) {
    {
        let __p = a0;
        let __k = a1;
        let __inserted = libcc2rs::Ptr::with_mut(
            &__p,
            |__s: &mut std::collections::BTreeSet<T1>| {
                std::collections::BTreeSet::insert(__s, __k.clone())
            },
        );
        (libcc2rs::RefcountSetIter::find_key(__p, &__k), __inserted)
    }
}
