// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::{MapIterator, PostfixInc, PrefixInc, UnsafeMapIterator};
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

unsafe fn f25<T1: Ord, T2>(a0: &mut BTreeMap<T1, Box<T2>>, a1: &mut BTreeMap<T1, Box<T2>>) {
    let __src = std::mem::take(&mut *a1);
    a0.clear();
    a0.extend(__src);
}

unsafe fn f26<T1: Clone + Ord, T2: Clone>(a0: &mut BTreeMap<T1, Box<T2>>, a1: BTreeMap<T1, Box<T2>>) {
    let __src = a1.clone();
    a0.clear();
    a0.extend(__src);
}

unsafe fn f27<T1: Ord + Clone, T2>(a0: BTreeMap<T1, Box<T2>>) -> UnsafeMapIterator<T1, T2> {
    UnsafeMapIterator::begin(&a0 as *const BTreeMap<T1, Box<T2>>)
}
unsafe fn f28<T1: PartialEq, T2: PartialEq>(
    a0: UnsafeMapIterator<T1, T2>,
    a1: UnsafeMapIterator<T1, T2>,
) -> bool {
    a0 != a1
}
unsafe fn f29<T1: PartialEq, T2: PartialEq>(
    a0: &BTreeMap<T1, Box<T2>>,
    a1: &BTreeMap<T1, Box<T2>>,
) -> bool {
    a0 == a1
}
unsafe fn f30<T1: PartialEq, T2: PartialEq>(
    a0: &BTreeMap<T1, Box<T2>>,
    a1: &BTreeMap<T1, Box<T2>>,
) -> bool {
    a0 != a1
}

unsafe fn f31<T1: Ord + Clone, T2>(
    a0: &mut UnsafeMapIterator<T1, T2>,
) -> UnsafeMapIterator<T1, T2> {
    a0.prefix_inc()
}
unsafe fn f32<T1: Ord + Clone, T2>(
    a0: &mut UnsafeMapIterator<T1, T2>,
) -> UnsafeMapIterator<T1, T2> {
    a0.postfix_inc()
}

unsafe fn f33<T1: Ord + Clone, T2>(
    a0: &mut UnsafeMapIterator<T1, T2>,
) -> UnsafeMapIterator<T1, T2> {
    a0.prefix_inc()
}
unsafe fn f34<T1: Ord + Clone, T2>(
    a0: &mut UnsafeMapIterator<T1, T2>,
) -> UnsafeMapIterator<T1, T2> {
    a0.postfix_inc()
}

unsafe fn f35<T1: Ord, T2>(a0: Vec<(T1, T2)>, a1: Option<T1>) -> BTreeMap<T1, Box<T2>> {
    a0.into_iter()
        .rev()
        .map(|(__k, __v)| (__k, Box::new(__v)))
        .collect::<BTreeMap<T1, Box<T2>>>()
}

fn t4<T1, T2, T3>() -> BTreeMap<T1, Box<T2>> {
    BTreeMap::new()
}

unsafe fn f36<T1: Ord, T2: Ord>(
    a0: &BTreeMap<T1, Box<T2>>,
    a1: &BTreeMap<T1, Box<T2>>,
) -> bool {
    a0 >= a1
}

// llvm::MapVector<KeyT, ValueT> -> the `Vector` member itself: insertion-ordered
// storage.  NOT a BTreeMap/HashMap -- see the src.cpp comment; key order is not
// insertion order, and MapVector is chosen in LLVM exactly to get insertion order.
fn t5<T1, T2>() -> Vec<(T1, Box<T2>)> {
    Vec::new()
}

// f37-f42 -- MapVector members, keyed against t5's Vec<(KeyT, Box<ValueT>)>.
// See src.cpp for the census and for why insert()'s full pair-of-iterator
// return, find()/end()/it->second, and the 5 decomposing range-for sites are
// NOT keyed here.  PLAIN REFERENCE receivers throughout (not a container
// wrapped in any handle type -- this model has none): the census majority
// (8 of 10 operator[] sites, and every insert/count/size/empty/clear site) is
// a LOCAL variable or by-ref parameter, not a struct field, so this is the
// majority-first shape; documented, not silently assumed.
unsafe fn f37<T1: PartialEq, T2: Default>(a0: &mut Vec<(T1, Box<T2>)>, a1: T1) -> &mut T2 {
    match a0.iter().position(|__e| __e.0 == a1) {
        Some(__i) => a0[__i].1.as_mut(),
        None => {
            // `<T2>::default()`, NOT `T2::default()` -- MEASURED on the real corpus TU
            // UnitTypeDiscovery.cpp: when T2 substitutes a generic type
            // (`Vec<dataflowir_gen::ir::Value>`), the un-angle-bracketed form emits
            // `Vec<dataflowir_gen::ir::Value>::default()`, which rustfmt/rustc parses as
            // a CHAINED COMPARISON (`Vec < ... > ::default()`), not a path -- the same
            // placeholder-before-`::` bug class already fixed in rules/support+optional
            // (31 sites, "a placeholder pasted before a literal `::`").
            a0.push((a1, Box::new(<T2>::default())));
            let __n = a0.len() - 1;
            a0[__n].1.as_mut()
        }
    }
}

// The incoming pair is bound via ONE local (`__kv`), not two occurrences of
// `a1`, per rules/map's own f35 lesson: a rule body is inlined, so every `aN`
// occurrence re-evaluates the CALL SITE'S argument expression, not a local.
//
// Returns a TUPLE, `((), bool)`, not a plain `bool` -- src.cpp's f38 keys the
// `insert()` call alone and returns the whole C++ pair
// (`std::pair<std::pair<KeyT,ValueT> *, bool>`); the corpus's `.second` read
// is matched SEPARATELY by rules/pair's own generic accessor at the call
// site (see src.cpp's f38 comment for why the two cannot be fused into one
// rule body -- doing so segfaults cpp-rule-preprocessor). The census has zero
// sites reading `.first` (the "iterator"), so its Rust representation is an
// unused `()` placeholder rather than a modeled pointer/index.
unsafe fn f38<T1: PartialEq, T2>(a0: &mut Vec<(T1, Box<T2>)>, a1: (T1, T2)) -> ((), bool) {
    let __kv = a1;
    match a0.iter().position(|__e| __e.0 == __kv.0) {
        Some(_) => ((), false),
        None => {
            a0.push((__kv.0, Box::new(__kv.1)));
            ((), true)
        }
    }
}

unsafe fn f39<T1: PartialEq, T2>(a0: &Vec<(T1, Box<T2>)>, a1: T1) -> usize {
    a0.iter().any(|__e| __e.0 == a1) as usize
}

unsafe fn f40<T1, T2>(a0: &Vec<(T1, Box<T2>)>) -> usize {
    a0.len()
}

unsafe fn f41<T1, T2>(a0: &Vec<(T1, Box<T2>)>) -> bool {
    a0.is_empty()
}

unsafe fn f42<T1, T2>(a0: &mut Vec<(T1, Box<T2>)>) {
    a0.clear();
}
