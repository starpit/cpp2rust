// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::DenseMap reuses the SAME runtime types std::map and std::unordered_map
// already use -- BTreeMap plus MapIter -- for the reason src.cpp records: the
// iteration order of a hash table is unspecified, so an ordered map is an
// order-refining translation, and MapIter's PartialEq is already position
// identity (by key), which is what LLVM's `LHS.Ptr == RHS.Ptr` means.
//
// So f1/f2 (and the const forms f20/f21) are one-liners, and that is the point:
// the work was establishing that the existing representation is the correct
// one, not building a new one.

use libcc2rs::{MapIterator, PostfixInc, UnsafeMapIterator};
use std::collections::BTreeMap;

fn t1<T1, T2>() -> BTreeMap<T1, Box<T2>> {
    BTreeMap::new()
}

fn t2<T1: Clone + Ord, T2>() -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::null()
}

unsafe fn f1<T1: PartialEq, T2>(
    a0: UnsafeMapIterator<T1, T2>,
    a1: UnsafeMapIterator<T1, T2>,
) -> bool {
    a0 == a1
}

unsafe fn f2<T1: PartialEq, T2>(
    a0: UnsafeMapIterator<T1, T2>,
    a1: UnsafeMapIterator<T1, T2>,
) -> bool {
    a0 != a1
}

unsafe fn f3<T1: Ord + Clone, T2>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: T1,
) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T2>>, &a1)
}

unsafe fn f4<T1: Ord + Clone, T2>(
    a0: &mut BTreeMap<T1, Box<T2>>,
) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::end(&*a0 as *const BTreeMap<T1, Box<T2>>)
}

unsafe fn f5<T1: Ord + Clone, T2>(
    a0: &mut BTreeMap<T1, Box<T2>>,
) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::begin(&*a0 as *const BTreeMap<T1, Box<T2>>)
}

unsafe fn f6<T1, T2>() -> BTreeMap<T1, Box<T2>> {
    BTreeMap::new()
}

unsafe fn f7<T1: Ord + Clone, T2: Default>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: T1,
) -> &mut T2 {
    a0.entry(a1).or_default().as_mut()
}

unsafe fn f8<T1: Ord, T2>(a0: BTreeMap<T1, Box<T2>>, a1: T1) -> u32 {
    if a0.contains_key(&a1) {
        1_u32
    } else {
        0_u32
    }
}

unsafe fn f9<T1: Ord, T2>(a0: BTreeMap<T1, Box<T2>>, a1: T1) -> bool {
    a0.contains_key(&a1)
}

unsafe fn f10<T1, T2>(a0: BTreeMap<T1, Box<T2>>) -> bool {
    a0.is_empty()
}

unsafe fn f11<T1, T2>(a0: BTreeMap<T1, Box<T2>>) -> u32 {
    a0.len() as u32
}

unsafe fn f12<T1, T2>(a0: &mut BTreeMap<T1, Box<T2>>) {
    a0.clear()
}

unsafe fn f14<T1, T2>(a0: u32) -> BTreeMap<T1, Box<T2>> {
    let _ = a0;
    BTreeMap::new()
}

unsafe fn f15<T1: Ord + Clone, T2: Default>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: T1,
) -> &mut T2 {
    a0.entry(a1).or_default().as_mut()
}

unsafe fn f16<T1: Ord + Clone, T2>(
    a0: &mut UnsafeMapIterator<T1, T2>,
) -> UnsafeMapIterator<T1, T2> {
    a0.inc();
    a0.clone()
}

unsafe fn f17<T1: Ord + Clone, T2>(
    a0: &mut UnsafeMapIterator<T1, T2>,
) -> UnsafeMapIterator<T1, T2> {
    a0.postfix_inc()
}

unsafe fn f18<T1: Ord + Clone, T2>(a0: UnsafeMapIterator<T1, T2>) -> *const T1 {
    a0.first()
}

unsafe fn f19<T1: Ord + Clone, T2>(a0: UnsafeMapIterator<T1, T2>) -> *mut T2 {
    a0.second()
}

fn t3<T1: Clone + Ord, T2>() -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::null()
}

unsafe fn f20<T1: PartialEq, T2>(
    a0: UnsafeMapIterator<T1, T2>,
    a1: UnsafeMapIterator<T1, T2>,
) -> bool {
    a0 == a1
}

unsafe fn f21<T1: PartialEq, T2>(
    a0: UnsafeMapIterator<T1, T2>,
    a1: UnsafeMapIterator<T1, T2>,
) -> bool {
    a0 != a1
}

unsafe fn f22<T1: Ord + Clone, T2>(
    a0: BTreeMap<T1, Box<T2>>,
    a1: T1,
) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::find_key(&a0 as *const BTreeMap<T1, Box<T2>>, &a1)
}

unsafe fn f23<T1: Ord + Clone, T2>(
    a0: BTreeMap<T1, Box<T2>>,
) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::end(&a0 as *const BTreeMap<T1, Box<T2>>)
}

unsafe fn f24<T1: Ord + Clone, T2>(
    a0: BTreeMap<T1, Box<T2>>,
) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::begin(&a0 as *const BTreeMap<T1, Box<T2>>)
}

unsafe fn f25<T1: Ord + Clone, T2>(
    a0: &mut UnsafeMapIterator<T1, T2>,
) -> UnsafeMapIterator<T1, T2> {
    a0.inc();
    a0.clone()
}

unsafe fn f26<T1: Ord + Clone, T2>(a0: UnsafeMapIterator<T1, T2>) -> *mut T2 {
    a0.second()
}

unsafe fn f27<T1: Ord + Clone, T2>(a0: UnsafeMapIterator<T1, T2>) -> *const T1 {
    a0.first()
}

// f28..f33 -- lookup/at/insert/try_emplace/erase.  See src.cpp for LLVM's own
// bodies, which are the specification, and for the three return types that
// DISAGREE with std::map: erase(key) is a bool not a count, erase(iterator) is
// void not the next iterator, and lookup does NOT insert.

unsafe fn f28<T1: Ord + Clone, T2: Clone>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: &mut (T1, T2),
) -> (UnsafeMapIterator<T1, T2>, bool) {
    let __pair = a1;
    let __key: T1 = <T1>::clone(&__pair.0);
    let __value: T2 = <T2>::clone(&__pair.1);
    // KEEPS THE INCUMBENT -- DenseMap.h:241. BTreeMap::insert would overwrite.
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

unsafe fn f29<T1: Ord + Clone, T2>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: T1,
    a2: T2,
) -> (UnsafeMapIterator<T1, T2>, bool) {
    let __key: T1 = a1;
    let __value: T2 = a2;
    // Same keep-the-incumbent contract -- DenseMap.h:256.
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

// lookup returns the value BY VALUE and DOES NOT INSERT -- DenseMap.h:205.
// Using f7's entry().or_default() here would grow the map, which is the whole
// difference between lookup and operator[].
unsafe fn f30<T1: Ord, T2: Default + Clone>(a0: BTreeMap<T1, Box<T2>>, a1: T1) -> T2 {
    match a0.get(&a1) {
        Some(__v) => <T2>::clone(__v.as_ref()),
        None => <T2>::default(),
    }
}

// at aborts on a missing key -- DenseMap.h:224 is an assert -- so `expect`
// keeps it loud rather than inventing an entry.
unsafe fn f31<T1: Ord, T2>(a0: &mut BTreeMap<T1, Box<T2>>, a1: T1) -> &mut T2 {
    a0.get_mut(&a1)
        .expect("DenseMap::at failed due to a missing key")
        .as_mut()
}

// erase(key) is a BOOL, not the count rules/map's f43 returns.
unsafe fn f32<T1: Ord, T2>(a0: &mut BTreeMap<T1, Box<T2>>, a1: T1) -> bool {
    a0.remove(&a1).is_some()
}

// erase(iterator) is VOID, not the following iterator rules/map's f3 returns.
unsafe fn f33<T1: Ord + Clone, T2>(
    a0: &mut BTreeMap<T1, Box<T2>>,
    a1: UnsafeMapIterator<T1, T2>,
) {
    let _ = UnsafeMapIterator::erase(&*a0 as *const BTreeMap<T1, Box<T2>>, &a1);
}

// f34..f36 -- the const-receiver / lvalue-argument overloads. Same bodies as
// f31/f28/f29; only the matched signature differs. See src.cpp.
unsafe fn f34<T1: Ord, T2>(a0: &mut BTreeMap<T1, Box<T2>>, a1: T1) -> *const T2 {
    (a0.get(&a1)
        .expect("DenseMap::at failed due to a missing key")
        .as_ref() as *const T2)
}

unsafe fn f35<T1: Ord + Clone, T2: Clone>(
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

unsafe fn f36<T1: Ord + Clone, T2>(
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
