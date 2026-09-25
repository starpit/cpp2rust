// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp.  DenseSet reuses the representation rules/set gives std::set and
// rules/smallset gives SmallSet -- BTreeMap<T1, Box<T1>>, the set-as-map-keyed-
// on-its-own-element shape, with UnsafeMapIterator<T1, T1> as the iterator.  No
// new runtime type: the existing PartialEq (position by key, end() = None) is
// what DenseSet.h:144's `LHS.I == RHS.I` MEANS for a deduplicating container,
// and src.cpp argues that rather than assuming it.
//
// DenseSet's ValueInfoT DOES NOT APPEAR HERE, and neither does the bucket
// DenseMap: they are hashing policy and storage layout, not observable through
// the surface mapped here, the same way rules/smallset drops SmallSet's N.
//
// ORDER.  BTreeMap walks in key order; DenseSet leaves its order unspecified
// (it is a hash table), so this is an order-REFINEMENT, as rules/densemap
// records for DenseMap.  The probe prints a sorted signature so no evidence
// here depends on the refinement.

use libcc2rs::*;
use std::collections::BTreeMap;

fn t1<T1>() -> BTreeMap<T1, Box<T1>> {
    BTreeMap::new()
}

fn t2<T1: Clone + Ord>() -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::null()
}

fn t3<T1: Clone + Ord>() -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::null()
}

unsafe fn f1<T1>() -> BTreeMap<T1, Box<T1>> {
    BTreeMap::new()
}

// The reserve-taking constructor.  The argument is a bucket-count HINT and is
// dropped: BTreeMap has no capacity to reserve, and a hint is not observable.
// rules/densemap's f14 drops DenseMap's InitialReserve the same way.
unsafe fn f2<T1>(a0: u32) -> BTreeMap<T1, Box<T1>> {
    let _ = a0;
    BTreeMap::new()
}

// insert(const T1 &) -- PROBE-AND-INSERT, not BTreeMap::insert, and that is the
// whole contract at the three abort sites.  C++ insert KEEPS the incumbent and
// reports false when the element is already present (DenseSet.h:198 forwards to
// DenseMap::try_emplace); BTreeMap::insert would OVERWRITE the stored value and
// report the old one.  rules/set's f11 and rules/smallset's f5 are this body.
unsafe fn f3<T1: Ord + Clone>(
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

// insert(T1 &&) -- same contract, separate signature (src.cpp says why).
unsafe fn f4<T1: Ord + Clone>(
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

unsafe fn f5<T1: Ord>(a0: BTreeMap<T1, Box<T1>>, a1: T1) -> bool {
    a0.contains_key(&a1)
}

// count returns DenseSet's size_type, `unsigned` -> u32, and is 1 or 0 only
// (DenseSet.h:176 is documented "Return 1 if the specified key is in the set, 0
// otherwise").  rules/densemap's f8 is the same body for DenseMap.
unsafe fn f6<T1: Ord>(a0: BTreeMap<T1, Box<T1>>, a1: T1) -> u32 {
    if a0.contains_key(&a1) {
        1_u32
    } else {
        0_u32
    }
}

unsafe fn f7<T1>(a0: BTreeMap<T1, Box<T1>>) -> u32 {
    a0.len() as u32
}

unsafe fn f8<T1>(a0: BTreeMap<T1, Box<T1>>) -> bool {
    a0.is_empty()
}

unsafe fn f9<T1: Ord + Clone>(a0: &mut BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::begin(&*a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f10<T1: Ord + Clone>(a0: &mut BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::end(&*a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f11<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::begin(&a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f12<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::end(&a0 as *const BTreeMap<T1, Box<T1>>)
}

// find -- an iterator to the element, or end() when absent.  find_key already
// has exactly that contract (iterators.rs:86), which is why rules/set's f5 and
// rules/densemap's f3 are the same one-liner.
unsafe fn f13<T1: Ord + Clone>(
    a0: &mut BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T1>>, &a1)
}

unsafe fn f14<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>, a1: T1) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::find_key(&a0 as *const BTreeMap<T1, Box<T1>>, &a1)
}

// The four comparisons.  POSITION IDENTITY, which for a deduplicating set is the
// same relation as key identity -- see src.cpp.
unsafe fn f15<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 == a1
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

unsafe fn f18<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 != a1
}

// operator* -- the ELEMENT.  In this representation the element is stored as
// both the key and the value, and `second()` is the pointer into the stored
// value, which is what rules/set's f18 yields for `*it` on a std::set iterator.
unsafe fn f19<T1: Ord + Clone>(a0: UnsafeMapIterator<T1, T1>) -> *mut T1 {
    a0.second()
}

unsafe fn f20<T1: Ord + Clone>(a0: UnsafeMapIterator<T1, T1>) -> *const T1 {
    a0.second()
}

// PREFIX ++ -- advance and yield the advanced iterator, rules/set's f20.
unsafe fn f21<T1: Ord + Clone>(
    a0: &mut UnsafeMapIterator<T1, T1>,
) -> UnsafeMapIterator<T1, T1> {
    a0.inc();
    a0.clone()
}

unsafe fn f22<T1: Ord + Clone>(
    a0: &mut UnsafeMapIterator<T1, T1>,
) -> UnsafeMapIterator<T1, T1> {
    a0.inc();
    a0.clone()
}
