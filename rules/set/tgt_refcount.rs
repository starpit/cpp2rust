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

fn f11<T1: Clone + Ord>(a0: &mut std::collections::BTreeSet<T1>, a1: std::collections::BTreeSet<T1>) {
    let __src = a1.clone();
    a0.clear();
    a0.extend(__src);
}

fn f12<T1: Ord>(a0: &mut std::collections::BTreeSet<T1>, a1: &mut std::collections::BTreeSet<T1>) {
    let __src = std::mem::take(&mut *a1);
    a0.clear();
    a0.extend(__src);
}

fn t2<T1: Ord + Clone + 'static>() -> libcc2rs::RefcountSetIter<T1> {
    libcc2rs::RefcountSetIter::null()
}

// ⛔⛔ g3099 -- THE RETURN IS `(Value<Iter>, Value<bool>)`, NOT `(Iter, bool)`.
// `src.cpp`'s f13/f23 return `std::pair<std::set<T1>::iterator, bool>`, and
// `std::pair` is modelled by **rules/pair**, whose `t1` is `(T1, T2)` under
// unsafe but `(Value<T1>, Value<T2>)` under refcount, while its `::second`
// (pair/f1) is `a0.1` in BOTH.  So under refcount the converter emits a
// `.borrow()` on `.1`, and a plain `bool` there compiles as a RULE and fails in
// the EMITTED code with
//   error[E0599]: no method named `borrow` found for type `bool`
// Landed precedent, both measured: `rules/smallptrset/tgt_refcount.rs` f1 and
// `rules/unordered_map` f57 (commit e17a894c -- 2 sites on
// sys-arch-spec/isa/isa.cpp, 2 E0599 -> 0, zero new errors).
// ⭐⭐ THIS CLASS HAS NOW BEEN REDISCOVERED THREE TIMES; THIS NOTE IS SO THAT IS
// THE LAST TIME.  It hides because a call site that DISCARDS the returned pair
// reports a CLEAN PASS (14 of 23 corpus sites in the smallptrset note discard
// it), so "0 errors on this key today" is NOT evidence the shape is fine.
// Both elements are boxed because rules/pair boxes both uniformly.
// ⛔ The UNSAFE arm's plain `(Iter, bool)` is CORRECT and must stay.
fn f13<T1: Ord + Clone + libcc2rs::ByteRepr + 'static>(
    a0: libcc2rs::Ptr<std::collections::BTreeSet<T1>>,
    a1: T1,
) -> (
    libcc2rs::Value<libcc2rs::RefcountSetIter<T1>>,
    libcc2rs::Value<bool>,
) {
    {
        let __p = a0;
        let __k = a1;
        let __inserted = libcc2rs::Ptr::with_mut(
            &__p,
            |__s: &mut std::collections::BTreeSet<T1>| {
                std::collections::BTreeSet::insert(__s, __k.clone())
            },
        );
        (
            std::rc::Rc::new(std::cell::RefCell::new(
                libcc2rs::RefcountSetIter::find_key(__p, &__k),
            )),
            std::rc::Rc::new(std::cell::RefCell::new(__inserted)),
        )
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

// g3099 -- same `(Value<Iter>, Value<bool>)` fix as f13; see the note there.
fn f23<T1: Ord + Clone + libcc2rs::ByteRepr + 'static>(
    a0: libcc2rs::Ptr<std::collections::BTreeSet<T1>>,
    a1: T1,
) -> (
    libcc2rs::Value<libcc2rs::RefcountSetIter<T1>>,
    libcc2rs::Value<bool>,
) {
    {
        let __p = a0;
        let __k = a1;
        let __inserted = libcc2rs::Ptr::with_mut(
            &__p,
            |__s: &mut std::collections::BTreeSet<T1>| {
                std::collections::BTreeSet::insert(__s, __k.clone())
            },
        );
        (
            std::rc::Rc::new(std::cell::RefCell::new(
                libcc2rs::RefcountSetIter::find_key(__p, &__k),
            )),
            std::rc::Rc::new(std::cell::RefCell::new(__inserted)),
        )
    }
}
