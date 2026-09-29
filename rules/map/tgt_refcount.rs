// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

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

fn t3<T1: Clone + Ord + 'static, T2: 'static>() -> RefcountMapIter<T1, T2> {
    RefcountMapIter::null()
}

fn f1<T1: Ord + Clone + ByteRepr + 'static, T2: Default + ByteRepr + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
) -> Ptr<T2> {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        __v.entry(a1)
            .or_insert_with(|| Rc::new(RefCell::new(<T2>::default())))
            .as_pointer()
    })
}

fn f2<T1, T2>(a0: BTreeMap<T1, Value<T2>>) -> usize {
    a0.len()
}

fn f3<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: RefcountMapIter<T1, T2>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::erase(a0, &a1)
}

fn f5<T1, T2>() -> BTreeMap<T1, Value<T2>> {
    BTreeMap::new()
}

fn f6<T1: Ord + Clone, T2: Clone>(a0: BTreeMap<T1, Value<T2>>) -> BTreeMap<T1, Value<T2>> {
    a0.iter()
        .map(|(k, v)| (k.clone(), Rc::new(RefCell::new(v.borrow().clone()))))
        .collect()
}

fn f7<T1: Ord, T2: Default>(a0: &mut BTreeMap<T1, Value<T2>>, a1: T1) -> Ptr<T2> {
    a0.get(&a1).expect("out of range!").as_pointer()
}

fn f8<T1: Ord + Clone + ByteRepr + 'static, T2: Default + ByteRepr + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
) -> Ptr<T2> {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        __v.entry(a1)
            .or_insert_with(|| Rc::new(RefCell::new(<T2>::default())))
            .as_pointer()
    })
}

fn f9<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::end(a0)
}

fn f10<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f11<T1: PartialEq, T2: PartialEq>(
    a0: RefcountMapIter<T1, T2>,
    a1: RefcountMapIter<T1, T2>,
) -> bool {
    a0 != a1
}

fn f12<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::begin(a0)
}

fn f13<T1: PartialEq, T2: PartialEq>(
    a0: RefcountMapIter<T1, T2>,
    a1: RefcountMapIter<T1, T2>,
) -> bool {
    a0 == a1
}

fn f14<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::end(a0)
}

fn f15<T1: Ord, T2: Default>(a0: &mut BTreeMap<T1, Value<T2>>, a1: T1) -> Ptr<T2> {
    a0.get(&a1).expect("out of range!").as_pointer()
}

fn f16<T1: PartialEq, T2: PartialEq>(
    a0: RefcountMapIter<T1, T2>,
    a1: RefcountMapIter<T1, T2>,
) -> bool {
    a0 == a1
}

fn f17<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f19<T1: Clone, T2>(a0: RefcountMapIter<T1, T2>) -> RefcountMapIter<T1, T2> {
    a0
}

fn f20<T1: Ord + Clone + 'static, T2: 'static>(a0: RefcountMapIter<T1, T2>) -> Value<T1> {
    a0.first()
}
fn f21<T1: Ord + Clone + 'static, T2: 'static>(a0: RefcountMapIter<T1, T2>) -> Value<T2> {
    a0.second()
}

fn f22<T1: Ord + Clone + 'static, T2: 'static>(a0: RefcountMapIter<T1, T2>) -> Value<T1> {
    a0.first()
}
fn f23<T1: Ord + Clone + 'static, T2: 'static>(a0: RefcountMapIter<T1, T2>) -> Value<T2> {
    a0.second()
}

fn f24<T1: 'static, T2: 'static>(a0: Ptr<BTreeMap<T1, Value<T2>>>) -> BTreeMap<T1, Value<T2>> {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| std::mem::take(__v))
}

fn f25<T1: 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: Ptr<BTreeMap<T1, Value<T2>>>,
) {
    let __src = a1.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| std::mem::take(__v));
    a0.write(__src)
}

fn f26<T1: Ord + Clone + 'static, T2: Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: BTreeMap<T1, Value<T2>>,
) {
    a0.write(
        a1.iter()
            .map(|(k, v)| (k.clone(), Rc::new(RefCell::new(v.borrow().clone()))))
            .collect(),
    )
}

fn f27<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::begin(a0)
}

fn f28<T1: PartialEq, T2: PartialEq>(
    a0: RefcountMapIter<T1, T2>,
    a1: RefcountMapIter<T1, T2>,
) -> bool {
    a0 != a1
}

fn f29<T1: PartialEq, T2: PartialEq>(
    a0: &BTreeMap<T1, Value<T2>>,
    a1: &BTreeMap<T1, Value<T2>>,
) -> bool {
    a0 == a1
}

fn f30<T1: PartialEq, T2: PartialEq>(
    a0: &BTreeMap<T1, Value<T2>>,
    a1: &BTreeMap<T1, Value<T2>>,
) -> bool {
    a0 != a1
}

fn f31<T1: Ord + Clone + 'static, T2: 'static>(
    a0: &mut RefcountMapIter<T1, T2>,
) -> RefcountMapIter<T1, T2> {
    a0.prefix_inc()
}
fn f32<T1: Ord + Clone + 'static, T2: 'static>(
    a0: &mut RefcountMapIter<T1, T2>,
) -> RefcountMapIter<T1, T2> {
    a0.postfix_inc()
}

fn f33<T1: Ord + Clone + 'static, T2: 'static>(
    a0: &mut RefcountMapIter<T1, T2>,
) -> RefcountMapIter<T1, T2> {
    a0.prefix_inc()
}
fn f34<T1: Ord + Clone + 'static, T2: 'static>(
    a0: &mut RefcountMapIter<T1, T2>,
) -> RefcountMapIter<T1, T2> {
    a0.postfix_inc()
}

fn f35<T1: Ord + Clone, T2>(
    a0: Vec<(Value<T1>, Value<T2>)>,
    a1: Option<T1>,
) -> BTreeMap<T1, Value<T2>> {
    a0.into_iter()
        .rev()
        .map(|(__k, __v)| (__k.borrow().clone(), __v))
        .collect::<BTreeMap<T1, Value<T2>>>()
}

fn t4<T1, T2, T3>() -> BTreeMap<T1, Value<T2>> {
    BTreeMap::new()
}

fn f36<T1: Ord, T2: Ord>(
    a0: &BTreeMap<T1, Value<T2>>,
    a1: &BTreeMap<T1, Value<T2>>,
) -> bool {
    a0 >= a1
}

// llvm::MapVector<KeyT, ValueT> -> the `Vector` member itself: insertion-ordered
// storage.  NOT a BTreeMap/HashMap -- see the src.cpp comment; key order is not
// insertion order, and MapVector is chosen in LLVM exactly to get insertion order.
fn t5<T1, T2>() -> Vec<(T1, Value<T2>)> {
    Vec::new()
}

// f37-f42 -- MapVector members, keyed against t5's Vec<(KeyT, Value<ValueT>)>.
// See src.cpp for the census; mirrors tgt_unsafe.rs's scope exactly, written
// independently for this arm (the two arms are not each other's `sed`).
//
// ⭐ insert()'s incoming pair is `(Value<T1>, Value<T2>)`, NOT `(T1, T2)`: the
// C++ parameter `std::pair<KeyT,ValueT> &&` is converted through rules/pair's
// OWN refcount model (t1 = `(Value<T1>, Value<T2>)`), exactly as this same
// module's own f35 receives its initializer-list pairs boxed on both fields.
// The key half is unwrapped with `.borrow().clone()` before it goes in the
// Vec, matching f35's `__k.borrow().clone()` line for line.
//
// PLAIN REFERENCE receivers, not `Ptr<Vec<...>>` -- see tgt_unsafe.rs's note:
// the census majority of call sites is a local/by-ref-parameter MapVector,
// not a struct field, so this is the majority-first, documented shape. A
// struct-field site (e.g. RoutingGraph::nodes_) may need a `Ptr`-based
// overload later; that is a follow-on gap, not silently assumed away.
// `__pos` is computed in its OWN statement, not inlined into the `match`
// scrutinee -- MEASURED (probe-mapvec/probe2.cpp): `a0` is inlined per-use as
// either `m.borrow()` or `m.borrow_mut()` depending on that use's mutability,
// and a `match a0.iter()...  { .. None => { a0.push(..) .. } }` shape holds
// the scrutinee's `Ref` guard alive for the WHOLE match statement (Rust
// temporary-lifetime rule), so the `None` arm's `borrow_mut()` panics at
// runtime with "RefCell already borrowed" -- reproduced byte-for-byte.
// Ending the read in its own statement drops that guard before the match.
fn f37<T1: PartialEq, T2: Default>(a0: &mut Vec<(T1, Value<T2>)>, a1: T1) -> Ptr<T2> {
    let __pos = a0.iter().position(|__e| __e.0 == a1);
    match __pos {
        Some(__i) => a0[__i].1.as_pointer(),
        None => {
            // `<T2>::default()`, NOT `T2::default()` -- see tgt_unsafe.rs's f37 comment;
            // same fix, same measured cause (UnitTypeDiscovery.cpp, T2 = a generic Vec).
            a0.push((a1, Rc::new(RefCell::new(<T2>::default()))));
            let __n = a0.len() - 1;
            a0[__n].1.as_pointer()
        }
    }
}

// Returns `(Value<()>, Value<bool>)`, not a plain `bool` -- see
// tgt_unsafe.rs's f38 comment: src.cpp's f38 keys the `insert()` call alone
// and returns the whole C++ pair, and rules/pair's OWN generic accessor (its
// refcount f1, `(Value<T1>, Value<T2>) -> Value<T2>`) matches the corpus's
// `.second` MemberExpr separately at the call site -- both elements of a
// refcount pair are boxed generically, hence `Value<()>` for the unused
// "iterator" half rather than a plain `()`.
fn f38<T1: PartialEq + Clone, T2>(a0: &mut Vec<(T1, Value<T2>)>, a1: (Value<T1>, Value<T2>)) -> (Value<()>, Value<bool>) {
    // ANNOTATED, not `let __kv = a1;` -- MEASURED (probe-mapvec/probe2.cpp):
    // without the type annotation rustc cannot infer the target of the
    // converting pair-construction's `.try_into()` (emitted by rules/pair's
    // own converting-constructor rule at the CALL SITE, not by this body) and
    // fails E0282 on `3.try_into().expect(..)`. The annotation here supplies
    // exactly the concrete `T1`/`T2` this call site substitutes.
    let __kv: (Value<T1>, Value<T2>) = a1;
    let __k = __kv.0.borrow().clone();
    // Same `Ref`-guard-lifetime fix as f37: compute the position in its own
    // statement so the `None` arm's `a0.push(..)` (a `borrow_mut()` once
    // inlined) does not run while the scrutinee's `borrow()` guard is still
    // alive.
    let __pos = a0.iter().position(|__e| __e.0 == __k);
    match __pos {
        Some(_) => (Rc::new(RefCell::new(())), Rc::new(RefCell::new(false))),
        None => {
            a0.push((__k, __kv.1));
            (Rc::new(RefCell::new(())), Rc::new(RefCell::new(true)))
        }
    }
}

fn f39<T1: PartialEq, T2>(a0: &Vec<(T1, Value<T2>)>, a1: T1) -> usize {
    a0.iter().any(|__e| __e.0 == a1) as usize
}

fn f40<T1, T2>(a0: &Vec<(T1, Value<T2>)>) -> usize {
    a0.len()
}

fn f41<T1, T2>(a0: &Vec<(T1, Value<T2>)>) -> bool {
    a0.is_empty()
}

fn f42<T1, T2>(a0: &mut Vec<(T1, Value<T2>)>) {
    a0.clear();
}
