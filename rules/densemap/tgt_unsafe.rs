// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::{PostfixInc, PrefixInc};

fn t1<T1, T2>() -> std::collections::HashMap<T1, T2> {
    std::collections::HashMap::new()
}

fn t2<T1>() -> std::collections::HashSet<T1> {
    std::collections::HashSet::new()
}

// The CRTP base is the SAME container as the derived DenseMap, so it maps to the
// same std::collections::HashMap.  T1 (the derived type), T4 (DenseMapInfo traits) and T5
// (DenseMapPair bucket) are representation details with no Rust analogue and are
// deliberately unused -- see the DenseMapInfo note in src.cpp.
fn t3<T1, T2, T3, T4, T5>() -> std::collections::HashMap<T2, T3> {
    std::collections::HashMap::new()
}

// THE ITERATOR.  `libcc2rs::HashMapIter<K, MapRef>` already models a std::collections::HashMap
// iterator whose IDENTITY IS THE KEY, not a bucket address -- which is exactly
// the discriminator that matters: two iterators at different positions holding
// EQUAL VALUES compare UNEQUAL, and end() (key None) equals only another end().
// A value-comparing body would be visibly wrong.
//
// The MapRef is `*const std::collections::HashMap<T1, T2>` and NOT the `UnsafeHashMapIterator`
// alias, because that alias is bound to `std::collections::HashMap<K, Box<V>>` while this module's
// t1 maps DenseMap to a BARE `std::collections::HashMap<T1, T2>`.  `impl HashMapAccess for
// *const std::collections::HashMap<K, V>` is generic in V, so the raw form fits without changing
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
// only the MapRef differs, because this module's t1 is a BARE std::collections::HashMap<T1, T2>
// while unordered_map's is std::collections::HashMap<K, Box<V>>.
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

// ---------------------------------------------------------------------------
// f11-f14 -- the iterator's ++ .  Shape is rules/unordered_map's committed
// f32-f35 VERBATIM except for the ITERATOR TYPE: this module's iterator is
// `HashMapIter<T1, *const std::collections::HashMap<T1, T2>>` (bare value) where unordered_map's is
// the `UnsafeHashMapIterator<T1, T2>` alias (`*const std::collections::HashMap<T1, Box<T2>>`) --
// the one place the bodies do not transfer unchanged.  `PrefixInc`/`PostfixInc`
// are implemented GENERICALLY over `MapRef: HashMapAccess<Key = K>`
// (libcc2rs/src/iterators.rs), so both spellings get them.
//
// EACH BODY MENTIONS `a0` EXACTLY ONCE.  A rule body is INLINED as one
// expression, so a second mention would advance the iterator twice.
unsafe fn f11<T1: std::hash::Hash + Eq + Clone, T2>(
    a0: &mut libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
) -> libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>> {
    a0.prefix_inc()
}

unsafe fn f12<T1: std::hash::Hash + Eq + Clone, T2>(
    a0: &mut libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
) -> libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>> {
    a0.postfix_inc()
}

unsafe fn f13<T1: std::hash::Hash + Eq + Clone, T2>(
    a0: &mut libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
) -> libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>> {
    a0.prefix_inc()
}

unsafe fn f14<T1: std::hash::Hash + Eq + Clone, T2>(
    a0: &mut libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
) -> libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>> {
    a0.postfix_inc()
}

// f15-f18 -- `it->first` / `it->second`.  unordered_map's f36-f39 call the
// `MapIterator` trait's `first()`/`second()`, but that trait is implemented for the
// two ALIASES only, and a generic impl for `HashMapIter<K, *const std::collections::HashMap<K, V>>`
// CANNOT be added beside the `Box<V>` one -- they overlap at V = Box<V'> and rustc
// rejects it with E0119.  So libcc2rs grew `key_ptr()`/`value_ptr()`, the same two
// accessors as inherent methods on the generic impl; `value_ptr` returns
// `*mut MapRef::Value`, which for this module's bare map is `*mut T2` -- exactly
// what the C++ `T2 &` needs.
unsafe fn f15<T1: std::hash::Hash + Eq + Clone, T2>(
    a0: libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
) -> *const T1 {
    a0.key_ptr()
}

unsafe fn f16<T1: std::hash::Hash + Eq + Clone, T2>(
    a0: libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
) -> *mut T2 {
    a0.value_ptr()
}

unsafe fn f17<T1: std::hash::Hash + Eq + Clone, T2>(
    a0: libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
) -> *const T1 {
    a0.key_ptr()
}

unsafe fn f18<T1: std::hash::Hash + Eq + Clone, T2>(
    a0: libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
) -> *mut T2 {
    a0.value_ptr()
}

// f19/f20 -- operator[].  rules/unordered_map's f1 is `a0.entry(a1).or_default()
// .as_mut()`; the `.as_mut()` is there only to unwrap its `Box<T2>`, so here --
// where t1 is a BARE `std::collections::HashMap<T1, T2>` -- it must NOT be present.  That is the
// transcription error this module's t1 difference invites.
unsafe fn f19<T1: std::hash::Hash + Eq, T2: Default>(
    a0: &mut std::collections::HashMap<T1, T2>,
    a1: T1,
) -> &mut T2 {
    a0.entry(a1).or_default()
}

unsafe fn f20<T1: std::hash::Hash + Eq, T2: Default>(
    a0: &mut std::collections::HashMap<T1, T2>,
    a1: T1,
) -> &mut T2 {
    a0.entry(a1).or_default()
}

// f21 -- `DenseMap(unsigned InitialReserve)`.  The reserve count is a capacity
// HINT with no observable semantics, so `with_capacity` is faithful and keeps the
// argument USED (a discarded argument would drop any side effect in it).
unsafe fn f21<T1, T2>(a0: u32) -> std::collections::HashMap<T1, T2> {
    std::collections::HashMap::with_capacity(a0 as usize)
}

// ---------------------------------------------------------------------------
// f22-f30 -- the rest of the operation surface.  Shapes copied from
// rules/unordered_map's committed f2 (size), f6 (count), f8 (erase-by-key),
// f24/f27 (find) and f40 (erase-by-iterator); the ONE place they do not transfer
// verbatim is the MapRef, because this module's t1 is a BARE
// `std::collections::HashMap<T1, T2>` while unordered_map's is
// `std::collections::HashMap<K, Box<V>>` -- so there is no `Box` to unwrap here
// and no `UnsafeHashMapIterator` alias to name.
//
// RETURN WIDTHS FOLLOW THE C++ DECLARATIONS, which differ from unordered_map's:
// `unsigned size()`/`unsigned count()` are u32 (not usize) and `erase(key)`
// returns bool (not a count).

unsafe fn f22<T1, T2>(a0: std::collections::HashMap<T1, T2>) -> bool {
    a0.is_empty()
}

unsafe fn f23<T1, T2>(a0: std::collections::HashMap<T1, T2>) -> u32 {
    a0.len() as u32
}

unsafe fn f24<T1: std::hash::Hash + Eq, T2>(
    a0: std::collections::HashMap<T1, T2>,
    a1: T1,
) -> u32 {
    (if a0.contains_key(&a1) { 1 } else { 0 })
}

// f25 -- `lookup`.  LLVM returns a DEFAULT-CONSTRUCTED ValueT when the key is
// absent (DenseMap.h:203-205), so `unwrap_or_default()` is the faithful tail and
// an `Option` or a panic would not be.  `cloned()` is needed because C++ returns
// BY VALUE out of a const receiver.
unsafe fn f25<T1: std::hash::Hash + Eq, T2: Clone + Default>(
    a0: std::collections::HashMap<T1, T2>,
    a1: T1,
) -> T2 {
    a0.get(&a1).cloned().unwrap_or_default()
}

unsafe fn f26<T1: std::hash::Hash + Eq + Clone, T2>(
    a0: &mut std::collections::HashMap<T1, T2>,
    a1: T1,
) -> libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>> {
    libcc2rs::HashMapIter::find_key(&*a0 as *const std::collections::HashMap<T1, T2>, &a1)
}

unsafe fn f27<T1: std::hash::Hash + Eq + Clone, T2>(
    a0: std::collections::HashMap<T1, T2>,
    a1: T1,
) -> libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>> {
    libcc2rs::HashMapIter::find_key(&a0 as *const std::collections::HashMap<T1, T2>, &a1)
}

unsafe fn f28<T1: std::hash::Hash + Eq, T2>(
    a0: &mut std::collections::HashMap<T1, T2>,
    a1: T1,
) -> bool {
    a0.remove(&a1).is_some()
}

// f29 -- `erase(iterator)`, which is VOID in LLVM.  `HashMapIter::erase` returns
// the FOLLOWING iterator, so the result is discarded with `drop(..)` -- one
// expression, yielding `()`, and `a0` is mentioned exactly once.
unsafe fn f29<T1: std::hash::Hash + Eq + Clone, T2>(
    a0: &mut std::collections::HashMap<T1, T2>,
    a1: libcc2rs::HashMapIter<T1, *const std::collections::HashMap<T1, T2>>,
) {
    drop(libcc2rs::HashMapIter::erase(
        &*a0 as *const std::collections::HashMap<T1, T2>,
        &a1,
    ))
}

// f30 -- `DenseSet(unsigned InitialReserve)`.  Same reasoning as f21: the reserve
// is a capacity HINT with no observable semantics, and keeping the argument USED
// preserves any side effect in it.
unsafe fn f30<T1>(a0: u32) -> std::collections::HashSet<T1> {
    std::collections::HashSet::with_capacity(a0 as usize)
}


// t8 -- the opaque unit for the TWO-ARGUMENT `llvm::DenseMapInfo<T1, T2>`.
// Identical body to t6 for an identical reason: the TYPE is modelled so t3's
// argument mapping can proceed, NO member is mapped, so every traits method
// still aborts loudly.  T1 and T2 are deliberately unused.
fn t8<T1, T2>() -> () {
    ()
}

unsafe fn f31<T1, T2>() -> () {
    ()
}

// f32-f34 -- ROW g3057.  `llvm::DenseMapInfo<unsigned int>`'s three traits
// members, for the ONE measured instantiation (KeyT = unsigned int); see the
// f32-f34 comment block in src.cpp for who calls these and where the values
// come from (llvm/ADT/DenseMapInfo.h:114-129 in this toolchain).
unsafe fn f32() -> u32 {
    u32::MAX
}

unsafe fn f33() -> u32 {
    u32::MAX - 1
}

// C++ `static_cast<unsigned>(Val * 37U)` is unsigned multiplication, DEFINED to
// wrap on overflow -- `wrapping_mul`, not `*`, which panics on overflow in a
// debug build and would silently change the hash function's semantics.
unsafe fn f34(a0: u32) -> u32 {
    a0.wrapping_mul(37)
}
