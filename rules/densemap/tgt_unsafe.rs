// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use std::collections::{HashMap, HashSet};

fn t1<T1, T2>() -> HashMap<T1, T2> {
    HashMap::new()
}

fn t2<T1>() -> HashSet<T1> {
    HashSet::new()
}

// The CRTP base is the SAME container as the derived DenseMap, so it maps to the
// same HashMap.  T1 (the derived type), T4 (DenseMapInfo traits) and T5
// (DenseMapPair bucket) are representation details with no Rust analogue and are
// deliberately unused -- see the DenseMapInfo note in src.cpp.
fn t3<T1, T2, T3, T4, T5>() -> HashMap<T2, T3> {
    HashMap::new()
}

// THE ITERATOR.  `libcc2rs::HashMapIter<K, MapRef>` already models a HashMap
// iterator whose IDENTITY IS THE KEY, not a bucket address -- which is exactly
// the discriminator that matters: two iterators at different positions holding
// EQUAL VALUES compare UNEQUAL, and end() (key None) equals only another end().
// A value-comparing body would be visibly wrong.
//
// The MapRef is `*const HashMap<T1, T2>` and NOT the `UnsafeHashMapIterator`
// alias, because that alias is bound to `HashMap<K, Box<V>>` while this module's
// t1 maps DenseMap to a BARE `HashMap<T1, T2>`.  `impl HashMapAccess for
// *const HashMap<K, V>` is generic in V, so the raw form fits without changing
// t1 (changing t1 would alter a committed, published key's semantics).
//
// Const and mutable iterator map to the SAME Rust type: Rust has no const-ness
// in the type, and both C++ spellings denote a position in the same table.
fn t4<T1: std::hash::Hash + Eq + Clone, T2>(
) -> libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>> {
    libcc2rs::HashMapIter::null()
}

fn t5<T1: std::hash::Hash + Eq + Clone, T2>(
) -> libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>> {
    libcc2rs::HashMapIter::null()
}

unsafe fn f1<T1: PartialEq, T2>(
    a0: libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
    a1: libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
) -> bool {
    a0 == a1
}

unsafe fn f2<T1: PartialEq, T2>(
    a0: libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
    a1: libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
) -> bool {
    a0 != a1
}

unsafe fn f3<T1: PartialEq, T2>(
    a0: libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
    a1: libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
) -> bool {
    a0 == a1
}

unsafe fn f4<T1: PartialEq, T2>(
    a0: libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
    a1: libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
) -> bool {
    a0 != a1
}

// t6 -- the opaque unit for `llvm::DenseMapInfo<T1>`.  See src.cpp: the TYPE is
// modelled so that t3's argument mapping can proceed; NO member is mapped, so
// every traits method still aborts loudly.  T1 is deliberately unused.
fn t6<T1>() -> () {
    ()
}

unsafe fn f5<T1>() -> () {
    ()
}


// t7 -- `llvm::detail::DenseMapPair<T1, T2>` is `std::pair<KeyT, ValueT>`
// (DenseMap.h:45), so this is rules/pair's committed t1 body verbatim.
fn t7<T1: Default, T2: Default>() -> (T1, T2) {
    <(T1, T2)>::default()
}

unsafe fn f6<T1: Default, T2: Default>() -> (T1, T2) {
    <(T1, T2)>::default()
}

// f7-f10 -- begin()/end().  Shape copied from rules/unordered_map's committed
// f22/f23 (mutable, `&mut` receiver) and f25/f26 (const, by-value receiver);
// only the MapRef differs, because this module's t1 is a BARE HashMap<T1, T2>
// while unordered_map's is HashMap<K, Box<V>>.
unsafe fn f7<T1: std::hash::Hash + Eq + Clone, T2>(
    a0: &mut std::collections::HashMap<T1, T2>,
) -> libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>> {
    libcc2rs::HashMapIter::begin(&*a0 as *const std::collections::HashMap<T1, T2>)
}

unsafe fn f8<T1: std::hash::Hash + Eq + Clone, T2>(
    a0: &mut std::collections::HashMap<T1, T2>,
) -> libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>> {
    libcc2rs::HashMapIter::end(&*a0 as *const std::collections::HashMap<T1, T2>)
}

unsafe fn f9<T1: std::hash::Hash + Eq + Clone, T2>(
    a0: std::collections::HashMap<T1, T2>,
) -> libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>> {
    libcc2rs::HashMapIter::begin(&a0 as *const std::collections::HashMap<T1, T2>)
}

unsafe fn f10<T1: std::hash::Hash + Eq + Clone, T2>(
    a0: std::collections::HashMap<T1, T2>,
) -> libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>> {
    libcc2rs::HashMapIter::end(&a0 as *const std::collections::HashMap<T1, T2>)
}
