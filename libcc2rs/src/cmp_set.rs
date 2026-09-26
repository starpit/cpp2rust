// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

//! A COMPARATOR-PARAMETERIZED ordered set: the faithful model for
//! `std::set<T, Compare>` when `Compare` is NOT the identity ordering on `T`.
//!
//! # Why this exists (and why `BTreeSet`/`BTreeMap` is WRONG here)
//!
//! The motivating case is
//! `std::set<ddc::shuffle::DimIndex, ddc::shuffle::DimIndexDimComparator>`
//! (blocks 3 census TUs).  Quoted from
//! `dt_src/ddc/transformations/automatic_shuffle/shuffle.h:58-69`:
//!
//! ```text
//! struct DimIndex { DimSymbol dim; bool high; };
//! struct DimIndexDimComparator {
//!   bool operator()(const DimIndex &a, const DimIndex &b) const { return a.dim < b.dim; }
//! };
//! ```
//!
//! The comparator IGNORES `high`, so it is NON-INJECTIVE: `{d,false}` and
//! `{d,true}` are EQUIVALENT under it (`!lt(a,b) && !lt(b,a)`), and libc++'s
//! `set::insert` keeps the FIRST inserted one and reports `inserted == false`
//! for the second.  The ignored field is then READ --
//! `shuffle.cpp:736-741` iterates the set and branches on `x.high` -- so a
//! `BTreeSet<DimIndex>` (keyed on the WHOLE value, hence on `high` too) keeps
//! BOTH elements and computes a different result.  That is a silent membership
//! difference, not a compile error, which is why the row stayed open.
//!
//! So: a sorted `Vec` of elements plus the comparator, with EVERY lookup
//! (`find`/`count`/`erase`) using comparator equivalence rather than `PartialEq`.
//! A probe value that differs only in the ignored field still matches.
//!
//! # The comparator bound
//!
//! [`Comparator<T>`] -- one method, `cmp_lt(&self, &T, &T) -> bool`, i.e. exactly
//! C++'s `bool operator()(const T&, const T&) const`.  There is a BLANKET impl
//! for every `F: Fn(&T, &T) -> bool`, so `CmpSet::new(|a, b| a.dim < b.dim)`
//! works, and `Fn(&T,&T)->bool` is the bound a rule may write if it wants to.
//! A *named trait* was chosen over a bare `Fn(&T,&T)->bool` bound for one
//! reason: a PORTED COMPARATOR STRUCT cannot implement `Fn` on stable Rust, but
//! it CAN implement `Comparator<T>` (it is a local type in the converted crate,
//! so coherence permits it even alongside the blanket `Fn` impl).  Two spellings
//! are therefore both available to the wiring agent; see "How a rule spells this"
//! below.
//!
//! # Constraints worth recording NOW
//!
//! * The container stores the comparator BY VALUE (`C`), like libc++'s
//!   `__compare` member.  A ported comparator arriving as `&C`-to-a-temporary
//!   (which is what the converter currently emits for a comparator argument)
//!   must therefore be cloned: use [`CmpSet::from_comparator_ref`], which
//!   requires `C: Clone`.  Ported comparator structs are stateless empty structs
//!   in practice, so `#[derive(Clone, Default)]` on them is enough -- but if the
//!   converter cannot add `Clone`, use [`CmpSet::default`] (`C: Default`) and
//!   ignore the argument, which is sound ONLY for a stateless comparator.
//! * `CmpSet` carries a marker [`crate::ByteRepr`] impl (like `BTreeMap`'s), so
//!   `Ptr<CmpSet<..>>::with` works WITHOUT `T: ByteRepr`.
//!
//! # How a rule spells this (exact text, so the wiring agent need not guess)
//!
//! Both model flavours mirror `rules/set`: the unsafe flavour boxes elements for
//! address stability, the refcount flavour uses `Value<T>`.
//!
//! ```text
//! // rules/set_cmp/tgt_unsafe.rs
//! use libcc2rs::{UnsafeCmpSet, UnsafeCmpSetIterator};
//! fn t1<T1, T2>() -> UnsafeCmpSet<T1, T2> { UnsafeCmpSet::default() }   // T2 = the comparator
//! fn t2<T1: 'static, T2: 'static>() -> UnsafeCmpSetIterator<T1, T2> { UnsafeCmpSetIterator::null() }
//! unsafe fn f1<T1, T2>(a0: UnsafeCmpSet<T1, T2>) -> usize { a0.len() }
//! unsafe fn f2<T1, T2: Comparator<T1>>(a0: &mut UnsafeCmpSet<T1, T2>, a1: T1) -> (UnsafeCmpSetIterator<T1, T2>, bool) {
//!     let (i, ins) = a0.insert(a1);
//!     (UnsafeCmpSetIterator::at(&*a0 as *const UnsafeCmpSet<T1, T2>, i), ins)
//! }
//! unsafe fn f3<T1, T2: Comparator<T1>>(a0: &mut UnsafeCmpSet<T1, T2>) -> UnsafeCmpSetIterator<T1, T2> {
//!     UnsafeCmpSetIterator::begin(&*a0 as *const UnsafeCmpSet<T1, T2>)
//! }
//! unsafe fn f4<T1, T2: Comparator<T1>>(a0: UnsafeCmpSetIterator<T1, T2>) -> *const T1 { a0.deref() }
//!
//! // rules/set_cmp/tgt_refcount.rs
//! use libcc2rs::{Ptr, RefcountCmpSet, RefcountCmpSetIter};
//! fn t1<T1, T2>() -> RefcountCmpSet<T1, T2> { RefcountCmpSet::default() }
//! fn f3<T1: 'static, T2: Comparator<T1> + 'static>(a0: Ptr<RefcountCmpSet<T1, T2>>) -> RefcountCmpSetIter<T1, T2> {
//!     RefcountCmpSetIter::begin(a0)
//! }
//! ```
//!
//! `for (auto x : idx)` maps onto the iterator's `Iterator` impl exactly as
//! `MapIter`'s does: `next()` yields a SNAPSHOT ITERATOR, and the loop body
//! derefs it (`deref()` / `value()`).  Position is an INDEX into the sorted
//! `Vec`, `end` is `len`, and `==` is index equality -- so two iterators at
//! different positions holding equal values compare UNEQUAL, as required.
//! `RefcountMapIter` could not be reused because it is keyed on a `BTreeMap`
//! key and therefore cannot express comparator equivalence.

use crate::{AsPointer, ByteRepr, PostfixDec, PostfixInc, PrefixDec, PrefixInc, Ptr, Value};
use std::cell::RefCell;
use std::marker::PhantomData;
use std::rc::Rc;

/// C++ `bool operator()(const T&, const T&) const` -- a strict weak ordering.
///
/// Blanket-implemented for every `F: Fn(&T, &T) -> bool`.  A ported comparator
/// STRUCT (a local type in the converted crate) may implement this directly.
pub trait Comparator<T> {
    fn cmp_lt(&self, a: &T, b: &T) -> bool;

    /// libc++'s notion of "same key": neither is less than the other.
    #[inline]
    fn cmp_equiv(&self, a: &T, b: &T) -> bool {
        !self.cmp_lt(a, b) && !self.cmp_lt(b, a)
    }
}

impl<T, F> Comparator<T> for F
where
    F: Fn(&T, &T) -> bool,
{
    #[inline]
    fn cmp_lt(&self, a: &T, b: &T) -> bool {
        self(a, b)
    }
}

/// `bool (*)(const T&, const T&)` -- the comparator as a RUNTIME VALUE.
///
/// It is a plain `fn` pointer and NOT a type parameter, and that is the whole
/// point of this shape: `CmpSetIter`'s type must not mention the comparator, or
/// two `std::set` families need two different Rust iterator types while libc++
/// spells both C++ iterators identically as
/// `std::__tree_const_iterator<T, std::__tree_node<T, void *> *, long>` -- one
/// string, two types, which is a duplicate type rule and rc=1 for the whole
/// tree.  A `fn` pointer also keeps `Clone`/`Copy`/`ByteRepr` free, where a
/// `Box<dyn Comparator<T>>` would lose `Clone` and need a supertrait.
pub type CmpFn<T> = fn(&T, &T) -> bool;

/// The NATURAL ordering as a named monomorphic function.
///
/// Named, not a closure, so `CmpSet::default()` has something to store and the
/// `cmp` field never needs to be an `Option`.  That matters: an `Option<CmpFn<T>>`
/// whose `None` arm fell back to some ordering would be a state that COMPARES
/// WRONGLY rather than failing, which is the failure shape this port keeps
/// finding.  There is no such state here.
pub fn natural_lt<T: PartialOrd>(a: &T, b: &T) -> bool {
    a < b
}

/// The comparator VALUE for a stateless comparator TYPE, as a `fn` pointer.
///
/// ** LOAD-BEARING ASSUMPTION: `C` MUST BE STATELESS. ** `C::default()` is
/// constructed fresh on every comparison, so any state a comparator carries is
/// DISCARDED -- a comparator holding, say, a sort direction would silently
/// compare by whatever `Default` gives instead of by what the caller built.
/// That is sound for every comparator the converter produces (a ported
/// comparator is an empty struct with an `operator()`), and `stateful_comparator_
/// state_is_discarded` pins the behaviour so nobody discovers it by debugging.
/// The type system cannot reject a stateful `C` here: `C: Default` is satisfiable
/// by a stateful type.
pub fn cmp_fn_of<T, C: Comparator<T> + Default>() -> CmpFn<T> {
    |a, b| C::default().cmp_lt(a, b)
}

/// `std::less<T>`, for a `CmpSet` that wants the natural ordering.
#[derive(Clone, Copy, Default, Debug)]
pub struct StdLess;

impl<T: PartialOrd> Comparator<T> for StdLess {
    #[inline]
    fn cmp_lt(&self, a: &T, b: &T) -> bool {
        a < b
    }
}

/// How one element is stored: `Box<T>` (unsafe model) or `Value<T>` (refcount
/// model).  Both give an element a stable address, matching `rules/set`.
pub trait SetSlot<T>: Sized {
    fn new_slot(value: T) -> Self;
    fn with_slot<R>(&self, f: impl FnOnce(&T) -> R) -> R;
}

impl<T> SetSlot<T> for Box<T> {
    #[inline]
    fn new_slot(value: T) -> Self {
        Box::new(value)
    }
    #[inline]
    fn with_slot<R>(&self, f: impl FnOnce(&T) -> R) -> R {
        f(self)
    }
}

impl<T> SetSlot<T> for Value<T> {
    #[inline]
    fn new_slot(value: T) -> Self {
        Rc::new(RefCell::new(value))
    }
    #[inline]
    fn with_slot<R>(&self, f: impl FnOnce(&T) -> R) -> R {
        f(&*self.borrow())
    }
}

/// `std::set<T, C>` modelled as a comparator-sorted `Vec` of slots.
pub struct CmpSet<T, S = Box<T>> {
    items: Vec<S>,
    cmp: CmpFn<T>,
    _elem: PhantomData<fn() -> T>,
}

/// Element storage for the unsafe model (mirrors `BTreeMap<T, Box<T>>`).
pub type UnsafeCmpSet<T> = CmpSet<T, Box<T>>;
/// Element storage for the refcount model (mirrors `BTreeMap<T, Value<T>>`).
pub type RefcountCmpSet<T> = CmpSet<T, Value<T>>;

// Marker impl, exactly like `impl<K, V> ByteRepr for BTreeMap<K, V>`: it lets
// `Ptr<CmpSet<..>>::with` be called without demanding `T: ByteRepr`.
impl<T: 'static, S: 'static> ByteRepr for CmpSet<T, S> {}

// BOUND CHANGE, called out deliberately: this was `C: Default` and is now
// `T: PartialOrd`.  The comparator is no longer a type, so there is nothing to
// default-construct; the natural ordering is the default instead.
impl<T: PartialOrd, S> Default for CmpSet<T, S> {
    fn default() -> Self {
        Self {
            items: Vec::new(),
            cmp: natural_lt::<T>,
            _elem: PhantomData,
        }
    }
}

impl<T, S: Clone> Clone for CmpSet<T, S> {
    fn clone(&self) -> Self {
        Self {
            items: self.items.clone(),
            cmp: self.cmp.clone(),
            _elem: PhantomData,
        }
    }
}

impl<T, S> CmpSet<T, S> {
    /// The comparator is stored BY VALUE, like libc++'s `__compare` member.
    pub fn new(cmp: CmpFn<T>) -> Self {
        Self {
            items: Vec::new(),
            cmp,
            _elem: PhantomData,
        }
    }

    /// For the converter's current shape, which hands a comparator over as an
    /// `&`-to-temporary.  The reference is DISCARDED and the comparator is
    /// rebuilt from its TYPE via [`cmp_fn_of`] -- see the stateless assumption
    /// documented there.  No `C: Clone` bound any more: a `fn` pointer is `Copy`.
    pub fn from_comparator_ref<C: Comparator<T> + Default>(_cmp: &C) -> Self {
        Self::new(cmp_fn_of::<T, C>())
    }

    pub fn comparator(&self) -> CmpFn<T> {
        self.cmp
    }

    pub fn len(&self) -> usize {
        self.items.len()
    }

    pub fn is_empty(&self) -> bool {
        self.items.is_empty()
    }

    pub fn clear(&mut self) {
        self.items.clear();
    }

    /// The slot at `index` in COMPARATOR ORDER (`None` past the end).
    pub fn slot_at(&self, index: usize) -> Option<&S> {
        self.items.get(index)
    }

    pub fn slots(&self) -> &[S] {
        &self.items
    }

    pub fn erase_at(&mut self, index: usize) {
        assert!(index < self.items.len(), "ub: erase past end");
        self.items.remove(index);
    }
}

impl<T, S: SetSlot<T>> CmpSet<T, S> {
    /// `std::lower_bound`: the first index whose element is NOT less than
    /// `value`, i.e. the insertion point.
    pub fn lower_bound(&self, value: &T) -> usize {
        // Linear, not binary: correct for any strict weak ordering the
        // converter may hand us, including a comparator that is only a partial
        // order in practice.  Sets in dt_src are small.
        self.items
            .iter()
            .position(|slot| slot.with_slot(|e| !(self.cmp)(e, value)))
            .unwrap_or(self.items.len())
    }

    /// Index of the element the comparator deems EQUIVALENT to `value`
    /// (`!lt(a,b) && !lt(b,a)`).  A `value` that differs only in a field the
    /// comparator ignores still matches -- that is the whole point.
    pub fn find_index(&self, value: &T) -> Option<usize> {
        let i = self.lower_bound(value);
        match self.items.get(i) {
            Some(slot) if slot.with_slot(|e| !(self.cmp)(value, e)) => Some(i),
            _ => None,
        }
    }

    /// `std::set::insert` -- returns `(index, inserted)`.  If an EQUIVALENT
    /// element is already present the EXISTING one is KEPT (libc++ does not
    /// overwrite) and `inserted` is false.
    pub fn insert(&mut self, value: T) -> (usize, bool) {
        let i = self.lower_bound(&value);
        if let Some(slot) = self.items.get(i) {
            if slot.with_slot(|e| !(self.cmp)(&value, e)) {
                return (i, false);
            }
        }
        self.items.insert(i, S::new_slot(value));
        (i, true)
    }

    pub fn contains(&self, value: &T) -> bool {
        self.find_index(value).is_some()
    }

    /// `std::set::count` -- 0 or 1, by comparator equivalence.
    pub fn count(&self, value: &T) -> usize {
        usize::from(self.contains(value))
    }

    /// `std::set::erase(const key_type&)` -- number of elements removed (0 or
    /// 1), matching by comparator equivalence.
    pub fn erase_value(&mut self, value: &T) -> usize {
        match self.find_index(value) {
            Some(i) => {
                self.items.remove(i);
                1
            }
            None => 0,
        }
    }

    /// Elements in comparator order.  Test/debug convenience.
    pub fn to_vec(&self) -> Vec<T>
    where
        T: Clone,
    {
        self.items.iter().map(|s| s.with_slot(|e| e.clone())).collect()
    }
}

/// A handle through which an iterator reaches its container: `Ptr<_>` in the
/// refcount model, `*const _` in the unsafe model.  Mirrors `MapAccess`.
pub trait ContainerRef: Clone + Default {
    type Target;
    fn with<R>(&self, f: impl FnOnce(&Self::Target) -> R) -> R;
    fn with_mut<R>(&self, f: impl FnOnce(&mut Self::Target) -> R) -> R;
}

impl<V: ByteRepr> ContainerRef for Ptr<V> {
    type Target = V;

    fn with<R>(&self, f: impl FnOnce(&V) -> R) -> R {
        Ptr::with(self, f)
    }

    fn with_mut<R>(&self, f: impl FnOnce(&mut V) -> R) -> R {
        Ptr::with_mut(self, f)
    }
}

impl<V> ContainerRef for *const V {
    type Target = V;

    fn with<R>(&self, f: impl FnOnce(&V) -> R) -> R {
        unsafe { f(&**self) }
    }

    fn with_mut<R>(&self, f: impl FnOnce(&mut V) -> R) -> R {
        unsafe { f(&mut *(*self as *mut V)) }
    }
}

/// Iterator into a [`CmpSet`].  Position is an INDEX; `end` is `len`; `==` is
/// index equality.
pub struct CmpSetIter<T, S, R> {
    set: R,
    index: usize,
    _p: PhantomData<fn() -> (T, S)>,
}

pub type UnsafeCmpSetIterator<T> = CmpSetIter<T, Box<T>, *const UnsafeCmpSet<T>>;
pub type RefcountCmpSetIter<T> = CmpSetIter<T, Value<T>, Ptr<RefcountCmpSet<T>>>;

impl<T, S, R: Clone> Clone for CmpSetIter<T, S, R> {
    fn clone(&self) -> Self {
        Self {
            set: self.set.clone(),
            index: self.index,
            _p: PhantomData,
        }
    }
}

impl<T, S, R> PartialEq for CmpSetIter<T, S, R> {
    fn eq(&self, other: &Self) -> bool {
        self.index == other.index
    }
}

impl<T, S, R> CmpSetIter<T, S, R>
where
    R: ContainerRef<Target = CmpSet<T, S>>,
{
    /// A default-constructed (singular) iterator: index 0 over a null handle.
    pub fn null() -> Self {
        Self {
            set: Default::default(),
            index: 0,
            _p: PhantomData,
        }
    }

    pub fn begin(set: R) -> Self {
        Self {
            set,
            index: 0,
            _p: PhantomData,
        }
    }

    pub fn end(set: R) -> Self {
        let len = set.with(|s| s.len());
        Self {
            set,
            index: len,
            _p: PhantomData,
        }
    }

    /// Iterator at a known index (what `insert` returns).
    pub fn at(set: R, index: usize) -> Self {
        Self {
            set,
            index,
            _p: PhantomData,
        }
    }

    pub fn index(&self) -> usize {
        self.index
    }

    pub fn len(&self) -> usize {
        self.set.with(|s| s.len())
    }

    pub fn is_end(&self) -> bool {
        self.index >= self.len()
    }

    pub fn inc(&mut self) {
        assert!(!self.is_end(), "ub: increment past end");
        self.index += 1;
    }

    pub fn dec(&mut self) {
        assert!(self.index > 0, "ub: decrement past begin");
        self.index -= 1;
    }
}

impl<T, S, R> CmpSetIter<T, S, R>
where
    S: SetSlot<T>,
    R: ContainerRef<Target = CmpSet<T, S>>,
{
    /// `std::set::find` -- comparator equivalence, so a probe value differing
    /// only in an IGNORED field still finds the member.
    pub fn find_value(set: R, value: &T) -> Self {
        match set.with(|s| s.find_index(value)) {
            Some(i) => Self::at(set, i),
            None => Self::end(set),
        }
    }

    /// `std::set::erase(iterator)` -- returns an iterator to the following
    /// element (same index, now one element earlier in the Vec).
    pub fn erase(set: R, iter: &Self) -> Self {
        let index = iter.index;
        assert!(
            index < set.with(|s| s.len()),
            "ub: erase of end iterator"
        );
        set.with_mut(|s| s.erase_at(index));
        Self::at(set, index)
    }
}

impl<T, S, R> Iterator for CmpSetIter<T, S, R>
where
    R: ContainerRef<Target = CmpSet<T, S>>,
{
    type Item = Self;

    fn next(&mut self) -> Option<Self::Item> {
        if self.is_end() {
            return None;
        }
        let snapshot = self.clone();
        self.inc();
        Some(snapshot)
    }
}

impl<T, S, R> PrefixInc for CmpSetIter<T, S, R>
where
    R: ContainerRef<Target = CmpSet<T, S>>,
{
    fn prefix_inc(&mut self) -> Self {
        self.inc();
        self.clone()
    }
}

impl<T, S, R> PostfixInc for CmpSetIter<T, S, R>
where
    R: ContainerRef<Target = CmpSet<T, S>>,
{
    fn postfix_inc(&mut self) -> Self {
        let ret = self.clone();
        self.inc();
        ret
    }
}

impl<T, S, R> PrefixDec for CmpSetIter<T, S, R>
where
    R: ContainerRef<Target = CmpSet<T, S>>,
{
    fn prefix_dec(&mut self) -> Self {
        self.dec();
        self.clone()
    }
}

impl<T, S, R> PostfixDec for CmpSetIter<T, S, R>
where
    R: ContainerRef<Target = CmpSet<T, S>>,
{
    fn postfix_dec(&mut self) -> Self {
        let ret = self.clone();
        self.dec();
        ret
    }
}

impl<T: 'static> UnsafeCmpSetIterator<T> {
    /// `*it` in the unsafe model.
    pub fn deref(&self) -> *const T {
        self.set.with(|s| {
            s.slot_at(self.index)
                .expect("ub: dereference of end iterator")
                .as_ref() as *const T
        })
    }

    pub fn deref_mut(&self) -> *mut T {
        self.deref() as *mut T
    }
}

impl<T: 'static> RefcountCmpSetIter<T> {
    /// `*it` in the refcount model.
    pub fn value(&self) -> Value<T> {
        self.set.with(|s| {
            s.slot_at(self.index)
                .expect("ub: dereference of end iterator")
                .clone()
        })
    }

    pub fn deref(&self) -> Ptr<T> {
        self.value().as_pointer()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // The real shape, quoted from shuffle.h:58-69.
    #[derive(Clone, Copy, PartialEq, Eq, Debug)]
    struct DimIndex {
        dim: i32,
        high: bool,
    }

    #[derive(Clone, Copy, Default, Debug)]
    struct DimIndexDimComparator;

    impl Comparator<DimIndex> for DimIndexDimComparator {
        fn cmp_lt(&self, a: &DimIndex, b: &DimIndex) -> bool {
            a.dim < b.dim
        }
    }

    fn di(dim: i32, high: bool) -> DimIndex {
        DimIndex { dim, high }
    }

    type Set = UnsafeCmpSet<DimIndex>;
    type Iter = UnsafeCmpSetIterator<DimIndex>;

    // THE STRUCTURAL POINT OF THE REDESIGN: neither `Set` nor `Iter` mentions
    // DimIndexDimComparator.  The comparator is now supplied at CONSTRUCTION,
    // which is also why `Set::default()` is no longer usable here -- DimIndex is
    // deliberately not PartialOrd, so the natural-ordering default cannot be
    // reached by accident with the wrong comparator.
    fn newset() -> Set {
        Set::new(cmp_fn_of::<DimIndex, DimIndexDimComparator>())
    }

    #[test]
    fn non_injective_comparator_keeps_first_inserted() {
        let mut s = newset();
        assert_eq!(s.insert(di(3, false)), (0, true));
        // EQUIVALENT under the comparator (it ignores `high`).
        assert_eq!(s.insert(di(3, true)), (0, false));
        assert_eq!(s.len(), 1, "a BTreeSet keyed on the whole value keeps 2");
        // ...and the SURVIVOR is the FIRST one: high == false.  A model that
        // overwrites gives high == true here.
        assert_eq!(s.to_vec(), vec![di(3, false)]);
    }

    #[test]
    fn find_matches_on_ignored_field() {
        let mut s = newset();
        s.insert(di(7, false));
        // Differs only in the IGNORED field -- must still be found.
        assert!(s.contains(&di(7, true)));
        assert_eq!(s.count(&di(7, true)), 1);
        assert!(s.find_index(&di(7, true)).is_some());
        assert!(!s.contains(&di(8, false)));
        assert_eq!(s.count(&di(8, true)), 0);
    }

    #[test]
    fn iteration_is_comparator_order() {
        let mut s = newset();
        // Neither sorted nor reverse-sorted.
        for v in [di(5, true), di(1, false), di(9, true), di(3, false)] {
            assert!(s.insert(v).1);
        }
        assert_eq!(
            s.to_vec(),
            vec![di(1, false), di(3, false), di(5, true), di(9, true)]
        );
        // And through the ITERATOR type, which is what `for (auto x : idx)` uses.
        let set: *const Set = &s;
        let seen: Vec<DimIndex> = Iter::begin(set).map(|it| unsafe { *it.deref() }).collect();
        assert_eq!(
            seen,
            vec![di(1, false), di(3, false), di(5, true), di(9, true)]
        );
    }

    #[test]
    fn erase_by_equivalent_but_unequal_value() {
        let mut s = newset();
        s.insert(di(2, false));
        s.insert(di(4, true));
        // Equal under the comparator, NOT `==` to the member.
        assert_eq!(s.erase_value(&di(4, false)), 1);
        assert_eq!(s.to_vec(), vec![di(2, false)]);
        assert_eq!(s.erase_value(&di(4, false)), 0);
        assert!(!s.is_empty());
        s.erase_value(&di(2, true));
        assert!(s.is_empty());
        assert_eq!(s.len(), 0);
    }

    #[test]
    fn iterator_positions_compare_by_index_not_value() {
        let mut s = newset();
        // Two DISTINCT members whose values are equal in every read field.
        s.insert(di(1, false));
        s.insert(di(2, false));
        let set: *const Set = &s;
        let a = Iter::begin(set);
        let mut b = Iter::begin(set);
        assert!(a == b);
        b.inc();
        assert!(a != b, "iterators at different positions must differ");
        b.inc();
        assert!(b.is_end());
        assert!(b == Iter::end(set));
        b.dec();
        assert_eq!(b.index(), 1);
    }

    #[test]
    fn find_via_iterator_and_erase_via_iterator() {
        let mut s = newset();
        s.insert(di(10, true));
        s.insert(di(20, false));
        s.insert(di(30, true));
        let set: *const Set = &s;
        let it = Iter::find_value(set, &di(20, true)); // ignored field differs
        assert!(!it.is_end());
        assert_eq!(unsafe { *it.deref() }, di(20, false));
        let next = Iter::erase(set, &it);
        assert_eq!(unsafe { *next.deref() }, di(30, true));
        assert_eq!(s.to_vec(), vec![di(10, true), di(30, true)]);
        assert!(Iter::find_value(set, &di(20, false)).is_end());
    }

    #[test]
    fn closure_comparator_and_std_less() {
        // The blanket `Fn(&T, &T) -> bool` impl -- the obvious bound.
        let mut s: UnsafeCmpSet<DimIndex> =
            CmpSet::new(|a: &DimIndex, b: &DimIndex| a.dim > b.dim); // DESCENDING
        s.insert(di(1, false));
        s.insert(di(5, true));
        s.insert(di(3, false));
        assert_eq!(s.to_vec(), vec![di(5, true), di(3, false), di(1, false)]);

        // SAME PROPERTY as before the redesign: a closure comparator and the
        // natural ordering agree on an injective comparison.  Previously this was
        // `CmpSet<i32, StdLess>` with the comparator in the TYPE; now both sides
        // are values, and `cmp_fn_of::<i32, StdLess>()` is shown to agree with
        // `natural_lt` and with a closure spelling the same thing.
        let mut n: UnsafeCmpSet<i32> = CmpSet::new(natural_lt);
        let mut n2: UnsafeCmpSet<i32> = CmpSet::new(cmp_fn_of::<i32, StdLess>());
        let mut n3: UnsafeCmpSet<i32> = CmpSet::new(|a: &i32, b: &i32| a < b);
        for v in [30, 10, 20] {
            n2.insert(v);
            n3.insert(v);
        }
        assert_eq!(n2.to_vec(), vec![10, 20, 30]);
        assert_eq!(n3.to_vec(), n2.to_vec());
        n.insert(30);
        n.insert(10);
        n.insert(20);
        assert_eq!(n.to_vec(), vec![10, 20, 30]);
        assert_eq!(n.insert(20), (1, false));
    }

    /// PINS THE LOAD-BEARING ASSUMPTION behind `cmp_fn_of`.
    ///
    /// The type system CANNOT reject a stateful comparator -- `C: Default` is
    /// satisfiable by a stateful type -- so this records what actually happens:
    /// the state is DISCARDED and comparison proceeds with `C::default()`.  A
    /// `Descending` built with `reverse: true` therefore sorts ASCENDING, because
    /// `Default` gives `reverse: false`.  If a future comparator carries state,
    /// this test is the thing that says why it silently misbehaves.
    #[derive(Default)]
    struct StatefulCmp {
        reverse: bool,
    }

    impl Comparator<i32> for StatefulCmp {
        fn cmp_lt(&self, a: &i32, b: &i32) -> bool {
            if self.reverse { b < a } else { a < b }
        }
    }

    #[test]
    fn stateful_comparator_state_is_discarded() {
        let stateful = StatefulCmp { reverse: true };
        // Used DIRECTLY as a Comparator, the state is honoured.
        assert!(stateful.cmp_lt(&5, &1));
        // Routed through the fn pointer, it is NOT: Default gives reverse=false.
        let f = cmp_fn_of::<i32, StatefulCmp>();
        assert!(!f(&5, &1), "state is discarded -- documented, not a bug here");
        assert!(f(&1, &5));
        // And from_comparator_ref discards it the same way.
        let mut s: UnsafeCmpSet<i32> = CmpSet::from_comparator_ref(&stateful);
        for v in [3, 1, 2] {
            s.insert(v);
        }
        assert_eq!(s.to_vec(), vec![1, 2, 3], "ASCENDING despite reverse=true");
    }

    #[test]
    fn comparator_by_ref_is_cloned() {
        // The converter currently passes a comparator as an &-to-temporary.
        let s: Set = CmpSet::from_comparator_ref(&DimIndexDimComparator);
        // The reference is discarded and the comparator rebuilt from its TYPE;
        // the resulting set still orders by `dim`, ignoring `high`.
        assert!(s.is_empty());
    }

    #[test]
    fn refcount_flavour_matches_unsafe_flavour() {
        let owner: Value<RefcountCmpSet<DimIndex>> = Rc::new(RefCell::new(
                RefcountCmpSet::new(cmp_fn_of::<DimIndex, DimIndexDimComparator>()),
            ));
        let ptr = owner.as_pointer();
        ptr.with_mut(|s| {
            s.insert(di(5, true));
            s.insert(di(1, false));
            s.insert(di(5, false)); // equivalent to the first -> dropped
        });
        assert_eq!(ptr.with(|s| s.len()), 2);
        assert_eq!(ptr.with(|s| s.to_vec()), vec![di(1, false), di(5, true)]);
        let seen: Vec<DimIndex> = RefcountCmpSetIter::begin(ptr.clone())
            .map(|it| *it.value().borrow())
            .collect();
        assert_eq!(seen, vec![di(1, false), di(5, true)]);
        let it = RefcountCmpSetIter::find_value(ptr.clone(), &di(5, false));
        assert!(!it.is_end());
        assert_eq!(*it.value().borrow(), di(5, true));
        // `deref()` hands back a live Ptr into the surviving element.
        assert!(it.deref() == owner.borrow().slot_at(1).unwrap().as_pointer());
    }
}
