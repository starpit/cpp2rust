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

use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::rc::Rc;

fn t1<T1, T2>() -> BTreeMap<T1, Value<T2>> {
    BTreeMap::new()
}

fn t2<T1: Clone + Ord + 'static, T2: 'static>() -> RefcountMapIter<T1, T2> {
    RefcountMapIter::null()
}

fn f1<T1: PartialEq, T2: PartialEq>(
    a0: RefcountMapIter<T1, T2>,
    a1: RefcountMapIter<T1, T2>,
) -> bool {
    a0 == a1
}

fn f2<T1: PartialEq, T2: PartialEq>(
    a0: RefcountMapIter<T1, T2>,
    a1: RefcountMapIter<T1, T2>,
) -> bool {
    a0 != a1
}

fn f3<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f4<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::end(a0)
}

fn f5<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::begin(a0)
}

fn f6<T1, T2>() -> BTreeMap<T1, Value<T2>> {
    BTreeMap::new()
}

fn f7<T1: Ord + Clone + ByteRepr + 'static, T2: Default + ByteRepr + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
) -> Ptr<T2> {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        __v.entry(a1)
            .or_insert_with(|| Rc::new(RefCell::new(<T2>::default())))
            .as_pointer()
    })
}

fn f8<T1: Ord, T2>(a0: BTreeMap<T1, Value<T2>>, a1: T1) -> u32 {
    if a0.contains_key(&a1) {
        1_u32
    } else {
        0_u32
    }
}

fn f9<T1: Ord, T2>(a0: BTreeMap<T1, Value<T2>>, a1: T1) -> bool {
    a0.contains_key(&a1)
}

fn f10<T1, T2>(a0: BTreeMap<T1, Value<T2>>) -> bool {
    a0.is_empty()
}

fn f11<T1, T2>(a0: BTreeMap<T1, Value<T2>>) -> u32 {
    a0.len() as u32
}

fn f12<T1, T2>(a0: &mut BTreeMap<T1, Value<T2>>) {
    a0.clear()
}

fn f14<T1, T2>(a0: u32) -> BTreeMap<T1, Value<T2>> {
    let _ = a0;
    BTreeMap::new()
}

fn f15<T1: Ord + Clone + ByteRepr + 'static, T2: Default + ByteRepr + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
) -> Ptr<T2> {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        __v.entry(a1)
            .or_insert_with(|| Rc::new(RefCell::new(<T2>::default())))
            .as_pointer()
    })
}

fn f16<T1: Ord + Clone + 'static, T2: 'static>(
    a0: &mut RefcountMapIter<T1, T2>,
) -> RefcountMapIter<T1, T2> {
    a0.inc();
    a0.clone()
}

fn f17<T1: Ord + Clone + 'static, T2: 'static>(
    a0: &mut RefcountMapIter<T1, T2>,
) -> RefcountMapIter<T1, T2> {
    a0.postfix_inc()
}

fn f18<T1: Ord + Clone + 'static, T2: 'static>(a0: RefcountMapIter<T1, T2>) -> Value<T1> {
    a0.first()
}

fn f19<T1: Ord + Clone + 'static, T2: 'static>(a0: RefcountMapIter<T1, T2>) -> Value<T2> {
    a0.second()
}

fn t3<T1: Clone + Ord + 'static, T2: 'static>() -> RefcountMapIter<T1, T2> {
    RefcountMapIter::null()
}

fn f20<T1: PartialEq, T2>(
    a0: RefcountMapIter<T1, T2>,
    a1: RefcountMapIter<T1, T2>,
) -> bool {
    a0 == a1
}

fn f21<T1: PartialEq, T2>(
    a0: RefcountMapIter<T1, T2>,
    a1: RefcountMapIter<T1, T2>,
) -> bool {
    a0 != a1
}

fn f22<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f23<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::end(a0)
}

fn f24<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::begin(a0)
}

fn f25<T1: Ord + Clone + 'static, T2: 'static>(
    a0: &mut RefcountMapIter<T1, T2>,
) -> RefcountMapIter<T1, T2> {
    a0.inc();
    a0.clone()
}

fn f26<T1: Ord + Clone + 'static, T2: 'static>(a0: RefcountMapIter<T1, T2>) -> Value<T2> {
    a0.second()
}

fn f27<T1: Ord + Clone + 'static, T2: 'static>(a0: RefcountMapIter<T1, T2>) -> Value<T1> {
    a0.first()
}

// f28..f33 -- lookup/at/insert/try_emplace/erase.  See src.cpp for LLVM's own
// bodies, which are the specification, and for the three return types that
// DISAGREE with std::map: erase(key) is a bool not a count, erase(iterator) is
// void not the next iterator, and lookup does NOT insert.

fn f28<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: &mut (Value<T1>, Value<T2>),
) -> (Value<RefcountMapIter<T1, T2>>, Value<bool>) {
    let __pair = a1;
    let __key: T1 = <T1>::clone(&*__pair.0.borrow());
    let __value: Value<T2> = __pair.1.clone();
    // KEEPS THE INCUMBENT -- DenseMap.h:241. BTreeMap::insert would overwrite.
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), __value);
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f29<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
    a2: T2,
) -> (Value<RefcountMapIter<T1, T2>>, Value<bool>) {
    let __key: T1 = a1;
    let __value: T2 = a2;
    // Same keep-the-incumbent contract -- DenseMap.h:256.
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), Rc::new(RefCell::new(__value)));
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}

// lookup returns the value BY VALUE and DOES NOT INSERT -- DenseMap.h:205.
// `deep_clone` and not `clone`: C++ returns a COPY, so a container-valued
// DenseMap must not hand back an aliasing Rc handle.
fn f30<T1: Ord, T2: Default + DeepClone>(a0: BTreeMap<T1, Value<T2>>, a1: T1) -> T2 {
    match a0.get(&a1) {
        Some(__v) => (*__v.borrow()).deep_clone(),
        None => <T2>::default(),
    }
}

// at aborts on a missing key -- DenseMap.h:224 is an assert -- so `expect`
// keeps it loud rather than inventing an entry.
fn f31<T1: Ord, T2>(a0: &mut BTreeMap<T1, Value<T2>>, a1: T1) -> Ptr<T2> {
    a0.get(&a1)
        .expect("DenseMap::at failed due to a missing key")
        .as_pointer()
}

// erase(key) is a BOOL, not the count rules/map's f43 returns.
fn f32<T1: Ord + 'static, T2: 'static>(a0: Ptr<BTreeMap<T1, Value<T2>>>, a1: T1) -> bool {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| __v.remove(&a1).is_some())
}

// erase(iterator) is VOID, not the following iterator rules/map's f3 returns.
fn f33<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: RefcountMapIter<T1, T2>,
) {
    let _ = RefcountMapIter::erase(a0, &a1);
}

// f34..f36 -- the const-receiver / lvalue-argument overloads. Same bodies as
// f31/f28/f29; only the matched signature differs. See src.cpp.
fn f34<T1: Ord, T2>(a0: &mut BTreeMap<T1, Value<T2>>, a1: T1) -> Ptr<T2> {
    a0.get(&a1)
        .expect("DenseMap::at failed due to a missing key")
        .as_pointer()
}

fn f35<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: &mut (Value<T1>, Value<T2>),
) -> (Value<RefcountMapIter<T1, T2>>, Value<bool>) {
    let __pair = a1;
    let __key: T1 = <T1>::clone(&*__pair.0.borrow());
    let __value: Value<T2> = __pair.1.clone();
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), __value);
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f36<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
    a2: T2,
) -> (Value<RefcountMapIter<T1, T2>>, Value<bool>) {
    let __key: T1 = a1;
    let __value: T2 = a2;
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), Rc::new(RefCell::new(__value)));
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}
