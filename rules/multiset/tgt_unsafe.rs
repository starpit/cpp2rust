// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use std::collections::BTreeMap;

// element -> multiplicity.  See src.cpp for why this is not a BTreeSet and why
// no iterator is mapped.
fn t1<T1>() -> BTreeMap<T1, usize> {
    BTreeMap::new()
}

// size() is the SUM of the multiplicities, not the entry count.
unsafe fn f1<T1>(a0: BTreeMap<T1, usize>) -> usize {
    a0.values().copied().sum::<usize>()
}

unsafe fn f2<T1>(a0: BTreeMap<T1, usize>) -> bool {
    a0.is_empty()
}

unsafe fn f3<T1>(a0: &mut BTreeMap<T1, usize>) {
    a0.clear()
}

unsafe fn f4<T1: Ord>(a0: BTreeMap<T1, usize>, a1: T1) -> usize {
    a0.get(&a1).copied().unwrap_or(0)
}

unsafe fn f5<T1>() -> BTreeMap<T1, usize> {
    BTreeMap::new()
}

unsafe fn f6<T1: Ord + Clone>(a0: BTreeMap<T1, usize>) -> BTreeMap<T1, usize> {
    a0.clone()
}

unsafe fn f7<T1: Ord + Clone>(a0: &mut BTreeMap<T1, usize>, a1: BTreeMap<T1, usize>) {
    *a0 = a1.clone()
}

// Equality compares MULTIPLICITIES because they are the mapped values:
// {1=>2, 2=>1} != {1=>1, 2=>1}.
unsafe fn f8<T1: PartialEq>(a0: BTreeMap<T1, usize>, a1: BTreeMap<T1, usize>) -> bool {
    a0 == a1
}

unsafe fn f9<T1: PartialEq>(a0: BTreeMap<T1, usize>, a1: BTreeMap<T1, usize>) -> bool {
    a0 != a1
}

// erase(key) removes the whole multiplicity and returns it.
unsafe fn f10<T1: Ord>(a0: &mut BTreeMap<T1, usize>, a1: T1) -> usize {
    a0.remove(&a1).unwrap_or(0)
}

unsafe fn f11<T1: Ord>(a0: BTreeMap<T1, usize>) -> BTreeMap<T1, usize> {
    a0
}

unsafe fn f12<T1: Ord>(a0: &mut BTreeMap<T1, usize>, a1: BTreeMap<T1, usize>) {
    *a0 = a1
}

unsafe fn f13<T1: Ord>(a0: &mut BTreeMap<T1, usize>, a1: &mut BTreeMap<T1, usize>) {
    std::mem::swap(a0, a1)
}

// Counts repeats instead of deduplicating -- the one place the model earns its
// keep versus a BTreeSet.
unsafe fn f14<T1: Ord>(a0: Vec<T1>, a1: Option<()>) -> BTreeMap<T1, usize> {
    let mut __m: BTreeMap<T1, usize> = BTreeMap::new();
    for __k in a0.into_iter() {
        *__m.entry(__k).or_insert(0) += 1;
    }
    __m
}
