// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::rc::Rc;

// The value is `Value<T2>` and not `Box<T2>`, mirroring rules/unordered_map's
// refcount model: a mapped value the C++ hands out a reference to must be
// shared, not owned by the container.
fn t1<T1, T2>() -> std::collections::HashMap<T1, Value<T2>> {
    std::collections::HashMap::new()
}

fn t2<T1>() -> std::collections::HashSet<T1> {
    std::collections::HashSet::new()
}

// The CRTP base is the SAME container as the derived DenseMap, so it maps to the
// same std::collections::HashMap.  T1 (the derived type), T4 (DenseMapInfo traits) and T5
// (DenseMapPair bucket) are representation details with no Rust analogue and are
// deliberately unused -- see the DenseMapInfo note in src.cpp.
fn t3<T1, T2, T3, T4, T5>() -> std::collections::HashMap<T2, Value<T3>> {
    std::collections::HashMap::new()
}

// THE ITERATOR -- see the note in tgt_unsafe.rs.  Here the MapRef is
// `Ptr<std::collections::HashMap<T1, Value<T2>>>`, which is precisely what t1 maps DenseMap to, so
// this is the already-provided `libcc2rs::RefcountHashMapIter<T1, T2>` alias.
// Identity is the KEY, so positions -- not values -- are compared.
fn t4<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::null()
}

fn t5<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::null()
}

fn f1<T1: PartialEq, T2>(
    a0: RefcountHashMapIter<T1, T2>,
    a1: RefcountHashMapIter<T1, T2>,
) -> bool {
    a0 == a1
}

fn f2<T1: PartialEq, T2>(
    a0: RefcountHashMapIter<T1, T2>,
    a1: RefcountHashMapIter<T1, T2>,
) -> bool {
    a0 != a1
}

fn f3<T1: PartialEq, T2>(
    a0: RefcountHashMapIter<T1, T2>,
    a1: RefcountHashMapIter<T1, T2>,
) -> bool {
    a0 == a1
}

fn f4<T1: PartialEq, T2>(
    a0: RefcountHashMapIter<T1, T2>,
    a1: RefcountHashMapIter<T1, T2>,
) -> bool {
    a0 != a1
}

// t6 -- the opaque unit for `llvm::DenseMapInfo<T1>`; see tgt_unsafe.rs and the
// note in src.cpp.  Identical in both models: a stateless traits class carries
// no ownership, so refcounting has nothing to express.
fn t6<T1>() -> () {
    ()
}

fn f5<T1>() -> () {
    ()
}


// t7 -- the bucket type.  rules/pair's refcount t1 is `(Value<T1>, Value<T2>)`;
// here only the SECOND component is shared, because this module's t1 maps
// DenseMap to `std::collections::HashMap<T1, Value<T2>>` -- the key is owned by the table, the
// mapped value is the thing C++ hands out references to.  `Value<T> =
// Rc<std::cell::RefCell<T>>` (libcc2rs/src/rc.rs:16), so the constructor is spelled exactly
// as rules/pair spells it: `Rc::new(std::cell::RefCell::new(..))`, never `Value::new(v)`.
fn t7<T1: Default, T2: Default>() -> (T1, Value<T2>) {
    (<T1>::default(), Rc::new(std::cell::RefCell::new(<T2>::default())))
}

fn f6<T1: Default, T2: Default>() -> (T1, Value<T2>) {
    (<T1>::default(), Rc::new(std::cell::RefCell::new(<T2>::default())))
}

// f7-f10 -- begin()/end().  rules/unordered_map's committed f22/f23/f25/f26
// verbatim in shape: the receiver is `Ptr<std::collections::HashMap<T1, Value<T2>>>` in BOTH the
// mutable and the const overload, which is exactly what this module's t1 is.
fn f7<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: Ptr<std::collections::HashMap<T1, Value<T2>>>,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::begin(a0)
}

fn f8<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: Ptr<std::collections::HashMap<T1, Value<T2>>>,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::end(a0)
}

fn f9<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: Ptr<std::collections::HashMap<T1, Value<T2>>>,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::begin(a0)
}

fn f10<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: Ptr<std::collections::HashMap<T1, Value<T2>>>,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::end(a0)
}

// ---------------------------------------------------------------------------
// f11-f14 -- ++ .  Here the iterator IS `RefcountHashMapIter<T1, T2>` =
// `HashMapIter<T1, Ptr<std::collections::HashMap<T1, Value<T2>>>>`, which is precisely what this
// module's t1 maps DenseMap to, so these are rules/unordered_map's committed
// f32-f35 VERBATIM.  One mention of `a0` each: the body is inlined, and two
// mentions would advance the iterator twice.
//
// WHY THESE FOUR NEED A REFCOUNT OVERRIDE AT ALL: the unsafe bodies are textually
// identical (`a0.prefix_inc()`), but the declared TYPES are not -- the unsafe
// signature names `*const std::collections::HashMap<T1, T2>` as the MapRef, which is raw-pointer text
// and is copied verbatim into the refcount arm if not overridden (the rules/atomic
// failure mode).  So the override is required by the SIGNATURE, not the body.
fn f11<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: &mut RefcountHashMapIter<T1, T2>,
) -> RefcountHashMapIter<T1, T2> {
    a0.prefix_inc()
}

fn f12<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: &mut RefcountHashMapIter<T1, T2>,
) -> RefcountHashMapIter<T1, T2> {
    a0.postfix_inc()
}

fn f13<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: &mut RefcountHashMapIter<T1, T2>,
) -> RefcountHashMapIter<T1, T2> {
    a0.prefix_inc()
}

fn f14<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: &mut RefcountHashMapIter<T1, T2>,
) -> RefcountHashMapIter<T1, T2> {
    a0.postfix_inc()
}

// f15-f18 -- `it->first` / `it->second`.  The refcount arm CAN use the
// `MapIterator` trait, because `RefcountHashMapIter<K, V>` is exactly the alias
// that trait is implemented for (libcc2rs/src/iterators.rs:393), and it hands back
// `Value<K>` / `Value<V>` -- shared handles, not raw pointers.  Identical to
// rules/unordered_map's committed f36-f39.
fn f15<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: RefcountHashMapIter<T1, T2>,
) -> Value<T1> {
    a0.first()
}

fn f16<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: RefcountHashMapIter<T1, T2>,
) -> Value<T2> {
    a0.second()
}

fn f17<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: RefcountHashMapIter<T1, T2>,
) -> Value<T1> {
    a0.first()
}

fn f18<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: RefcountHashMapIter<T1, T2>,
) -> Value<T2> {
    a0.second()
}

// f19/f20 -- operator[].  rules/unordered_map's committed f1 VERBATIM: its t1 and
// this module's t1 agree in the refcount model (both `std::collections::HashMap<T1, Value<T2>>`), so
// unlike the unsafe arm there is nothing to adjust.  `as_pointer()` is why this key
// MUST be overridden: under refcount it returns `Ptr<T2>`, and the unsafe arm's
// `&mut T2` return would be wrong here.
fn f19<T1: Eq + std::hash::Hash + Clone + 'static, T2: Default + 'static>(
    a0: Ptr<std::collections::HashMap<T1, Value<T2>>>,
    a1: T1,
) -> Ptr<T2> {
    a0.with_mut(|__v: &mut std::collections::HashMap<T1, Value<T2>>| {
        __v.entry(a1)
            .or_insert_with(|| Rc::new(std::cell::RefCell::new(<T2>::default())))
            .as_pointer()
    })
}

fn f20<T1: Eq + std::hash::Hash + Clone + 'static, T2: Default + 'static>(
    a0: Ptr<std::collections::HashMap<T1, Value<T2>>>,
    a1: T1,
) -> Ptr<T2> {
    a0.with_mut(|__v: &mut std::collections::HashMap<T1, Value<T2>>| {
        __v.entry(a1)
            .or_insert_with(|| Rc::new(std::cell::RefCell::new(<T2>::default())))
            .as_pointer()
    })
}

// f21 -- the constructor.  Overridden because the VALUE TYPE differs: this arm
// must yield `std::collections::HashMap<T1, Value<T2>>`, matching t1.
fn f21<T1, T2>(a0: u32) -> std::collections::HashMap<T1, Value<T2>> {
    std::collections::HashMap::with_capacity(a0 as usize)
}

// ---------------------------------------------------------------------------
// f22-f30 -- the rest of the operation surface, refcount arm.
// EVERY ONE OF THESE NEEDS THE OVERRIDE, and for the same two reasons the keys
// above do: (a) the unsafe signatures name `*const std::collections::HashMap<T1,
// T2>` -- LITERAL raw-pointer text, which is copied VERBATIM into the refcount
// arm if not overridden (the rules/atomic E0605 failure mode); (b) the receiver
// here is `Ptr<std::collections::HashMap<T1, Value<T2>>>`, so a mutation must go
// through `with_mut` and the mapped value is a `Value<T2>` that has to be
// borrowed rather than read directly.
// EACH BODY MENTIONS `a0` EXACTLY ONCE: the body is inlined as one expression,
// and two mentions of a `Ptr` receiver borrow it twice -- the measured
// `RefCell already mutably borrowed` panic in rules/mlir.

fn f22<T1, T2>(a0: std::collections::HashMap<T1, Value<T2>>) -> bool {
    a0.is_empty()
}

fn f23<T1, T2>(a0: std::collections::HashMap<T1, Value<T2>>) -> u32 {
    a0.len() as u32
}

fn f24<T1: std::hash::Hash + Eq, T2>(
    a0: std::collections::HashMap<T1, Value<T2>>,
    a1: T1,
) -> u32 {
    (if a0.contains_key(&a1) { 1 } else { 0 })
}

// f25 -- `lookup`.  Default-on-miss exactly as in the unsafe arm; the extra step
// is that a present value is a `Value<T2>` and must be BORROWED and cloned out,
// because C++ returns ValueT by value.
fn f25<T1: std::hash::Hash + Eq, T2: Clone + Default>(
    a0: std::collections::HashMap<T1, Value<T2>>,
    a1: T1,
) -> T2 {
    a0.get(&a1)
        .map(|__v: &Value<T2>| __v.borrow().clone())
        .unwrap_or_default()
}

fn f26<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: Ptr<std::collections::HashMap<T1, Value<T2>>>,
    a1: T1,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::find_key(a0, &a1)
}

fn f27<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: Ptr<std::collections::HashMap<T1, Value<T2>>>,
    a1: T1,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::find_key(a0, &a1)
}

fn f28<T1: std::hash::Hash + Eq + 'static, T2: 'static>(
    a0: Ptr<std::collections::HashMap<T1, Value<T2>>>,
    a1: T1,
) -> bool {
    a0.with_mut(|__v: &mut std::collections::HashMap<T1, Value<T2>>| {
        __v.remove(&a1).is_some()
    })
}

fn f29<T1: std::hash::Hash + Eq + Clone + 'static, T2: 'static>(
    a0: Ptr<std::collections::HashMap<T1, Value<T2>>>,
    a1: RefcountHashMapIter<T1, T2>,
) {
    drop(RefcountHashMapIter::erase(a0, &a1))
}

// f30 -- `DenseSet(unsigned)`.  Overridden only so the two arms stay in step;
// t2 is a plain `std::collections::HashSet<T1>` in both models (a set holds no
// mapped value for C++ to hand out a reference to, so there is nothing to share).
fn f30<T1>(a0: u32) -> std::collections::HashSet<T1> {
    std::collections::HashSet::with_capacity(a0 as usize)
}


// t8 -- the opaque unit for the two-argument `llvm::DenseMapInfo<T1, T2>`; see
// tgt_unsafe.rs and the note in src.cpp.  Identical in both models: a stateless
// traits class carries no ownership, so refcounting has nothing to express.
fn t8<T1, T2>() -> () {
    ()
}

fn f31<T1, T2>() -> () {
    ()
}

// f32-f34 -- ROW g3057.  Identical to tgt_unsafe.rs: three plain u32-in/u32-out
// functions, no ownership involved, so refcount and unsafe agree byte-for-byte
// modulo the `unsafe fn` keyword every function in tgt_unsafe.rs carries.
fn f32() -> u32 {
    u32::MAX
}

fn f33() -> u32 {
    u32::MAX - 1
}

fn f34(a0: u32) -> u32 {
    a0.wrapping_mul(37)
}
