// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp, and see tgt_unsafe.rs for why neither ValueInfoT nor the bucket
// DenseMap appears on this side.  The same reuse: rules/set's representation,
// BTreeMap<T1, Value<T1>> with RefcountMapIter<T1, T1>.
//
// The receiver forms follow rules/densemap exactly: a NON-CONST receiver, and a
// const receiver whose result is an ITERATOR, arrive as Ptr<BTreeMap<..>>
// (f22..f24 there); a const receiver whose result is a VALUE arrives as the map
// by value (f30 there).  Mixing the two up is a hard error in this model, not a
// silent difference.

use libcc2rs::*;
use libcc2rs::MapIterator;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::rc::Rc;

fn t1<T1>() -> BTreeMap<T1, Value<T1>> {
    BTreeMap::new()
}

fn t2<T1: Clone + Ord + 'static>() -> RefcountMapIter<T1, T1> {
    RefcountMapIter::null()
}

fn t3<T1: Clone + Ord + 'static>() -> RefcountMapIter<T1, T1> {
    RefcountMapIter::null()
}

fn f1<T1>() -> BTreeMap<T1, Value<T1>> {
    BTreeMap::new()
}

// The bucket-count hint is dropped -- see tgt_unsafe.rs.
fn f2<T1>(a0: u32) -> BTreeMap<T1, Value<T1>> {
    let _ = a0;
    BTreeMap::new()
}

// insert -- PROBE-AND-INSERT, keeping the incumbent.  rules/set's / rules/
// smallset's refcount body; tgt_unsafe.rs says why BTreeMap::insert is wrong.
fn f3<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> (Value<RefcountMapIter<T1, T1>>, Value<bool>) {
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| {
        let __new = !__v.contains_key(&a1);
        if __new {
            __v.insert(a1.clone(), Rc::new(RefCell::new(a1.clone())));
        }
        __new
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &a1))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f4<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> (Value<RefcountMapIter<T1, T1>>, Value<bool>) {
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| {
        let __new = !__v.contains_key(&a1);
        if __new {
            __v.insert(a1.clone(), Rc::new(RefCell::new(a1.clone())));
        }
        __new
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &a1))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f5<T1: Ord>(a0: BTreeMap<T1, Value<T1>>, a1: T1) -> bool {
    a0.contains_key(&a1)
}

fn f6<T1: Ord>(a0: BTreeMap<T1, Value<T1>>, a1: T1) -> u32 {
    if a0.contains_key(&a1) {
        1_u32
    } else {
        0_u32
    }
}

fn f7<T1>(a0: BTreeMap<T1, Value<T1>>) -> u32 {
    a0.len() as u32
}

fn f8<T1>(a0: BTreeMap<T1, Value<T1>>) -> bool {
    a0.is_empty()
}

fn f9<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::begin(a0)
}

fn f10<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::end(a0)
}

fn f11<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::begin(a0)
}

fn f12<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::end(a0)
}

fn f13<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f14<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f15<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 == a1
}

fn f16<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 != a1
}

fn f17<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 == a1
}

fn f18<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 != a1
}

// operator* -- the element, rules/set's f18 in this model.
fn f19<T1: Ord + Clone + 'static>(a0: RefcountMapIter<T1, T1>) -> Ptr<T1> {
    a0.second().as_pointer()
}

fn f20<T1: Ord + Clone + 'static>(a0: RefcountMapIter<T1, T1>) -> Ptr<T1> {
    a0.second().as_pointer()
}

fn f21<T1: Ord + Clone + 'static>(
    a0: &mut RefcountMapIter<T1, T1>,
) -> RefcountMapIter<T1, T1> {
    a0.inc();
    a0.clone()
}

fn f22<T1: Ord + Clone + 'static>(
    a0: &mut RefcountMapIter<T1, T1>,
) -> RefcountMapIter<T1, T1> {
    a0.inc();
    a0.clone()
}

// FAMILY TWO -- see src.cpp.  Keyed on the two-argument
// `llvm::DenseMapInfo<T1, void>` spelling that Autopilot.cpp asks for.  The
// key-info argument is hashing policy and is not represented on this side at
// all, so these bodies are f1..f22 verbatim.

fn t4<T1>() -> BTreeMap<T1, Value<T1>> {
    BTreeMap::new()
}

fn t5<T1: Clone + Ord + 'static>() -> RefcountMapIter<T1, T1> {
    RefcountMapIter::null()
}

fn t6<T1: Clone + Ord + 'static>() -> RefcountMapIter<T1, T1> {
    RefcountMapIter::null()
}

fn f23<T1>() -> BTreeMap<T1, Value<T1>> {
    BTreeMap::new()
}

// The bucket-count hint is dropped -- see tgt_unsafe.rs.
fn f24<T1>(a0: u32) -> BTreeMap<T1, Value<T1>> {
    let _ = a0;
    BTreeMap::new()
}

// insert -- PROBE-AND-INSERT, keeping the incumbent.  rules/set's / rules/
// smallset's refcount body; tgt_unsafe.rs says why BTreeMap::insert is wrong.
fn f25<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> (Value<RefcountMapIter<T1, T1>>, Value<bool>) {
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| {
        let __new = !__v.contains_key(&a1);
        if __new {
            __v.insert(a1.clone(), Rc::new(RefCell::new(a1.clone())));
        }
        __new
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &a1))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f26<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> (Value<RefcountMapIter<T1, T1>>, Value<bool>) {
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| {
        let __new = !__v.contains_key(&a1);
        if __new {
            __v.insert(a1.clone(), Rc::new(RefCell::new(a1.clone())));
        }
        __new
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &a1))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f27<T1: Ord>(a0: BTreeMap<T1, Value<T1>>, a1: T1) -> bool {
    a0.contains_key(&a1)
}

fn f28<T1: Ord>(a0: BTreeMap<T1, Value<T1>>, a1: T1) -> u32 {
    if a0.contains_key(&a1) {
        1_u32
    } else {
        0_u32
    }
}

fn f29<T1>(a0: BTreeMap<T1, Value<T1>>) -> u32 {
    a0.len() as u32
}

fn f30<T1>(a0: BTreeMap<T1, Value<T1>>) -> bool {
    a0.is_empty()
}

fn f31<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::begin(a0)
}

fn f32<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::end(a0)
}

fn f33<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::begin(a0)
}

fn f34<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::end(a0)
}

fn f35<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f36<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f37<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 == a1
}

fn f38<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 != a1
}

fn f39<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 == a1
}

fn f40<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 != a1
}

// operator* -- the element, rules/set's f40 in this model.
fn f41<T1: Ord + Clone + 'static>(a0: RefcountMapIter<T1, T1>) -> Ptr<T1> {
    a0.second().as_pointer()
}

fn f42<T1: Ord + Clone + 'static>(a0: RefcountMapIter<T1, T1>) -> Ptr<T1> {
    a0.second().as_pointer()
}

fn f43<T1: Ord + Clone + 'static>(
    a0: &mut RefcountMapIter<T1, T1>,
) -> RefcountMapIter<T1, T1> {
    a0.inc();
    a0.clone()
}

fn f44<T1: Ord + Clone + 'static>(
    a0: &mut RefcountMapIter<T1, T1>,
) -> RefcountMapIter<T1, T1> {
    a0.inc();
    a0.clone()
}

// FAMILY THREE -- llvm::SmallDenseSet, see src.cpp.  The inline bucket count is
// dropped (rules/smallvector's N), so these take T1 alone.  Bodies are f1..f22's
// verbatim apart from the copy constructor and erase, which family three adds.

fn t7<T1>() -> BTreeMap<T1, Value<T1>> {
    BTreeMap::new()
}

fn t8<T1>() -> BTreeMap<T1, Value<T1>> {
    BTreeMap::new()
}

fn t9<T1: Clone + Ord + 'static>() -> RefcountMapIter<T1, T1> {
    RefcountMapIter::null()
}

fn t10<T1: Clone + Ord + 'static>() -> RefcountMapIter<T1, T1> {
    RefcountMapIter::null()
}

fn f45<T1>() -> BTreeMap<T1, Value<T1>> {
    BTreeMap::new()
}

fn f46<T1>(a0: u32) -> BTreeMap<T1, Value<T1>> {
    let _ = a0;
    BTreeMap::new()
}

// The COPY CONSTRUCTOR, and it must be a DEEP copy.  `a0.clone()` would clone the
// Rc handles and the two sets would SHARE every element, so a later
// `new_clique.insert(v)` would be visible through cur_clique -- exactly the
// aliasing GraphStats.cpp:81 must not have.  So each Value is rebuilt from the
// borrowed element.
fn f47<T1: Ord + Clone + 'static>(a0: BTreeMap<T1, Value<T1>>) -> BTreeMap<T1, Value<T1>> {
    a0.iter()
        .map(|(__k, __v)| (__k.clone(), Rc::new(RefCell::new(__v.borrow().clone()))))
        .collect()
}

fn f48<T1>() -> BTreeMap<T1, Value<T1>> {
    BTreeMap::new()
}

fn f49<T1>(a0: u32) -> BTreeMap<T1, Value<T1>> {
    let _ = a0;
    BTreeMap::new()
}

fn f50<T1: Ord + Clone + 'static>(a0: BTreeMap<T1, Value<T1>>) -> BTreeMap<T1, Value<T1>> {
    a0.iter()
        .map(|(__k, __v)| (__k.clone(), Rc::new(RefCell::new(__v.borrow().clone()))))
        .collect()
}

fn f51<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> (Value<RefcountMapIter<T1, T1>>, Value<bool>) {
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| {
        let __new = !__v.contains_key(&a1);
        if __new {
            __v.insert(a1.clone(), Rc::new(RefCell::new(a1.clone())));
        }
        __new
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &a1))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f52<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> (Value<RefcountMapIter<T1, T1>>, Value<bool>) {
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| {
        let __new = !__v.contains_key(&a1);
        if __new {
            __v.insert(a1.clone(), Rc::new(RefCell::new(a1.clone())));
        }
        __new
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &a1))),
        Rc::new(RefCell::new(__inserted)),
    )
}

// erase(key) -- true iff an element was removed.
fn f53<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>, a1: T1) -> bool {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T1>>| __v.remove(&a1).is_some())
}

fn f54<T1: Ord>(a0: BTreeMap<T1, Value<T1>>, a1: T1) -> bool {
    a0.contains_key(&a1)
}

fn f55<T1: Ord>(a0: BTreeMap<T1, Value<T1>>, a1: T1) -> u32 {
    if a0.contains_key(&a1) {
        1_u32
    } else {
        0_u32
    }
}

fn f56<T1>(a0: BTreeMap<T1, Value<T1>>) -> u32 {
    a0.len() as u32
}

fn f57<T1>(a0: BTreeMap<T1, Value<T1>>) -> bool {
    a0.is_empty()
}

fn f58<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::begin(a0)
}

fn f59<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::end(a0)
}

fn f60<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::begin(a0)
}

fn f61<T1: Ord + Clone + 'static>(a0: Ptr<BTreeMap<T1, Value<T1>>>) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::end(a0)
}

fn f62<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f63<T1: Ord + Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T1>>>,
    a1: T1,
) -> RefcountMapIter<T1, T1> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f64<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 == a1
}

fn f65<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 != a1
}

fn f66<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 == a1
}

fn f67<T1: PartialEq>(a0: RefcountMapIter<T1, T1>, a1: RefcountMapIter<T1, T1>) -> bool {
    a0 != a1
}

fn f68<T1: Ord + Clone + 'static>(a0: RefcountMapIter<T1, T1>) -> Ptr<T1> {
    a0.second().as_pointer()
}

fn f69<T1: Ord + Clone + 'static>(a0: RefcountMapIter<T1, T1>) -> Ptr<T1> {
    a0.second().as_pointer()
}

fn f70<T1: Ord + Clone + 'static>(
    a0: &mut RefcountMapIter<T1, T1>,
) -> RefcountMapIter<T1, T1> {
    a0.inc();
    a0.clone()
}

fn f71<T1: Ord + Clone + 'static>(
    a0: &mut RefcountMapIter<T1, T1>,
) -> RefcountMapIter<T1, T1> {
    a0.inc();
    a0.clone()
}
