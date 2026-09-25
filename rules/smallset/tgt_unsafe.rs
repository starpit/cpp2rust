// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp.  SmallSet's iterator reuses the SAME runtime type rules/set gives
// std::set's iterator -- UnsafeMapIterator<T1, T1> over BTreeMap<T1, Box<T1>>,
// the set-as-map-keyed-on-its-own-element representation -- so the comparisons
// are one-liners.  The work was establishing that the existing PartialEq
// (position by key, end() = None) is what LLVM's union-of-VecIter-and-SetIter
// comparison means for the same-container case that is the only DEFINED one, and
// that for a DEDUPLICATING set position identity and key identity are the same
// relation.  No new runtime type, as rules/stringmap also concluded.
//
// The unsigned N of SmallSet<T, N, C> DOES NOT APPEAR HERE: it is a capacity
// hint selecting which half of the union is live, and the small/large split is
// not observable through this surface (src.cpp says why, and the probe measures
// it on both halves).  rules/smallvector drops SmallVector's N the same way --
// its t2 is `fn t2<T1>() -> Vec<T1>`.

use libcc2rs::UnsafeMapIterator;
use std::collections::BTreeMap;

fn t1<T1: Clone + Ord>() -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::null()
}

fn t2<T1: Clone + Ord>() -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::null()
}

unsafe fn f1<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 == a1
}

unsafe fn f2<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 != a1
}


// The container type -- the SAME representation rules/set gives std::set.
// See src.cpp: the TYPE only; no member of SmallSet is modelled here.
fn t3<T1>() -> BTreeMap<T1, Box<T1>> {
    BTreeMap::new()
}

// SmallSet's members -- the SAME bodies rules/set gives std::set's, over the
// same BTreeMap<T1, Box<T1>> set-as-map-keyed-on-its-own-element representation.
// See src.cpp for why they are mapped at all (t3 alone did not stop the
// converter porting the class) and for what is still left loud.
unsafe fn f3<T1>() -> BTreeMap<T1, Box<T1>> {
    BTreeMap::new()
}

unsafe fn f4<T1>(a0: BTreeMap<T1, Box<T1>>) -> usize {
    a0.len()
}

// PROBE-AND-INSERT, not BTreeMap::insert -- rules/set's f11 body verbatim, and
// the reason is the same: C++ insert KEEPS the incumbent when the element is
// already present, while BTreeMap::insert overwrites the value and keeps the old
// key.  For a set whose element type has equal-but-distinguishable values those
// differ.  It is also exactly what makes the probe's `insert(a).first` a find():
// the second insert must not disturb the first.
unsafe fn f5<T1: Ord + Clone>(
    a0: &mut BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> (UnsafeMapIterator<T1, T1>, bool) {
    let __inserted = !a0.contains_key(&a1);
    if __inserted {
        a0.insert(a1.clone(), Box::new(a1.clone()));
    }
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T1>>, &a1),
        __inserted,
    )
}

unsafe fn f6<T1: Ord + Clone>(
    a0: &mut BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> (UnsafeMapIterator<T1, T1>, bool) {
    let __inserted = !a0.contains_key(&a1);
    if __inserted {
        a0.insert(a1.clone(), Box::new(a1.clone()));
    }
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T1>>, &a1),
        __inserted,
    )
}

unsafe fn f7<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::begin(&a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f8<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::end(&a0 as *const BTreeMap<T1, Box<T1>>)
}
