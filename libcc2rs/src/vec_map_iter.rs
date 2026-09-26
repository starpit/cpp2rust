// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

//! `VecMapIterator` -- the iterator for a Vec-of-entries map, i.e. for
//! `llvm::MapVector` as `rules/mapvector` (commit `0ed8580`) models it:
//! `Vec<(T1, Box<T2>)>` in the unsafe model and `Vec<(T1, Value<T2>)>` in the
//! refcount model.
//!
//! # Why not `MapIterator`
//!
//! `iterators.rs` has only the BTreeMap-backed `MapIter`, whose POSITION IS A
//! KEY and which therefore visits entries in KEY order.  MapVector's entire
//! reason to exist over `DenseMap` is INSERTION-ORDER iteration, so a
//! BTreeMap-backed iterator silently reorders whatever a pass emits -- which is
//! why `rules/mapvector` landed explicitly UNVERIFIED: the order property could
//! not be observed without this type.  Here the position is an INDEX into the
//! entry `Vec`, `end` is `len`, `==` is index equality, and iteration yields
//! `(key, value)` pairs in insertion order.
//!
//! # Public surface
//!
//! Deliberately the same shape as `MapIter`/`MapIterator`, so a rule can key
//! `llvm::MapVector<K, V>::iterator` to it with NO new converter support:
//! `null()`, `begin()`, `end()`, `find_key()`, `at()`, `is_end()`, `inc()`,
//! `dec()`, `erase()`, `PartialEq`, `Clone`, `Iterator` (yields a snapshot
//! iterator), `PrefixInc`/`PostfixInc`/`PrefixDec`/`PostfixDec`, and the
//! [`crate::MapIterator`] trait's `first()`/`second()`.
//!
//! # How a rule spells this
//!
//! ```text
//! // rules/mapvector/tgt_unsafe.rs
//! use libcc2rs::{MapIterator, UnsafeVecMapIterator};
//! fn t2<T1: Clone + PartialEq, T2>() -> UnsafeVecMapIterator<T1, T2> { UnsafeVecMapIterator::null() }
//! unsafe fn fN<T1: Clone + PartialEq, T2>(a0: &mut Vec<(T1, Box<T2>)>) -> UnsafeVecMapIterator<T1, T2> {
//!     UnsafeVecMapIterator::begin(&*a0 as *const Vec<(T1, Box<T2>)>)
//! }
//! unsafe fn fM<T1: Clone + PartialEq, T2>(a0: UnsafeVecMapIterator<T1, T2>) -> *const T1 { a0.first() }
//!
//! // rules/mapvector/tgt_refcount.rs
//! use libcc2rs::{ByteRepr, MapIterator, Ptr, RefcountVecMapIter, Value};
//! fn fN<T1: Clone + PartialEq + ByteRepr, T2: ByteRepr>(
//!     a0: Ptr<Vec<(T1, Value<T2>)>>,
//! ) -> RefcountVecMapIter<T1, T2> { RefcountVecMapIter::begin(a0) }
//! ```
//!
//! ONE BOUND TO KNOW ABOUT, refcount flavour only: the container is a plain
//! `Vec`, and `Ptr::with` requires its pointee to be `ByteRepr`; the blanket
//! `impl<T: ByteRepr> ByteRepr for Vec<T>` then forces `T1: ByteRepr` and
//! `T2: ByteRepr`.  (`rules/set`'s `BTreeMap` escapes this because `BTreeMap`
//! has a marker impl with only `'static` bounds; `Vec` does not.)  If the
//! converter cannot supply those bounds, the alternative is a newtype container
//! carrying its own marker `ByteRepr` impl, exactly like [`crate::CmpSet`] does
//! -- but that would change the type `rules/mapvector` already records, so it is
//! left to the wiring agent to decide.  The unsafe flavour has no such bound.

use crate::{ContainerRef, MapIterator, PostfixDec, PostfixInc, PrefixDec, PrefixInc, Ptr, Value};
use std::cell::RefCell;
use std::marker::PhantomData;
use std::rc::Rc;

/// Iterator into a Vec-of-entries map.  Position is an INDEX, `end` is `len`.
pub struct VecMapIter<K, S, R> {
    entries: R,
    index: usize,
    _p: PhantomData<fn() -> (K, S)>,
}

pub type UnsafeVecMapIterator<K, V> = VecMapIter<K, Box<V>, *const Vec<(K, Box<V>)>>;
pub type RefcountVecMapIter<K, V> = VecMapIter<K, Value<V>, Ptr<Vec<(K, Value<V>)>>>;

impl<K, S, R: Clone> Clone for VecMapIter<K, S, R> {
    fn clone(&self) -> Self {
        Self {
            entries: self.entries.clone(),
            index: self.index,
            _p: PhantomData,
        }
    }
}

impl<K, S, R> PartialEq for VecMapIter<K, S, R> {
    fn eq(&self, other: &Self) -> bool {
        self.index == other.index
    }
}

impl<K, S, R> VecMapIter<K, S, R>
where
    R: ContainerRef<Target = Vec<(K, S)>>,
{
    pub fn null() -> Self {
        Self {
            entries: Default::default(),
            index: 0,
            _p: PhantomData,
        }
    }

    pub fn begin(entries: R) -> Self {
        Self {
            entries,
            index: 0,
            _p: PhantomData,
        }
    }

    pub fn end(entries: R) -> Self {
        let len = entries.with(|v| v.len());
        Self {
            entries,
            index: len,
            _p: PhantomData,
        }
    }

    pub fn at(entries: R, index: usize) -> Self {
        Self {
            entries,
            index,
            _p: PhantomData,
        }
    }

    pub fn index(&self) -> usize {
        self.index
    }

    pub fn len(&self) -> usize {
        self.entries.with(|v| v.len())
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

    /// `MapVector::find` -- linear scan in INSERTION ORDER, which is what LLVM's
    /// index map resolves to.
    pub fn find_key(entries: R, key: &K) -> Self
    where
        K: PartialEq,
    {
        match entries.with(|v| v.iter().position(|(k, _)| k == key)) {
            Some(i) => Self::at(entries, i),
            None => Self::end(entries),
        }
    }

    /// `erase(iterator)`: removes the entry, keeping the order of the rest, and
    /// returns an iterator to the following entry.
    pub fn erase(entries: R, iter: &Self) -> Self {
        let index = iter.index;
        assert!(
            index < entries.with(|v| v.len()),
            "ub: erase of end iterator"
        );
        entries.with_mut(|v| {
            v.remove(index);
        });
        Self::at(entries, index)
    }
}

impl<K, S, R> Iterator for VecMapIter<K, S, R>
where
    R: ContainerRef<Target = Vec<(K, S)>>,
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

impl<K, S, R> PrefixInc for VecMapIter<K, S, R>
where
    R: ContainerRef<Target = Vec<(K, S)>>,
{
    fn prefix_inc(&mut self) -> Self {
        self.inc();
        self.clone()
    }
}

impl<K, S, R> PostfixInc for VecMapIter<K, S, R>
where
    R: ContainerRef<Target = Vec<(K, S)>>,
{
    fn postfix_inc(&mut self) -> Self {
        let ret = self.clone();
        self.inc();
        ret
    }
}

impl<K, S, R> PrefixDec for VecMapIter<K, S, R>
where
    R: ContainerRef<Target = Vec<(K, S)>>,
{
    fn prefix_dec(&mut self) -> Self {
        self.dec();
        self.clone()
    }
}

impl<K, S, R> PostfixDec for VecMapIter<K, S, R>
where
    R: ContainerRef<Target = Vec<(K, S)>>,
{
    fn postfix_dec(&mut self) -> Self {
        let ret = self.clone();
        self.dec();
        ret
    }
}

impl<K: 'static, V: 'static> MapIterator for UnsafeVecMapIterator<K, V> {
    type First = *const K;
    type Second = *mut V;

    fn first(&self) -> *const K {
        self.entries.with(|v| {
            &v.get(self.index)
                .expect("ub: dereference of end iterator")
                .0 as *const K
        })
    }

    fn second(&self) -> *mut V {
        self.entries.with(|v| {
            v.get(self.index)
                .expect("ub: dereference of end iterator")
                .1
                .as_ref() as *const V as *mut V
        })
    }
}

impl<K: Clone + 'static, V: 'static> MapIterator for RefcountVecMapIter<K, V>
where
    Vec<(K, Value<V>)>: crate::ByteRepr,
{
    type First = Value<K>;
    type Second = Value<V>;

    fn first(&self) -> Value<K> {
        self.entries.with(|v| {
            Rc::new(RefCell::new(
                v.get(self.index)
                    .expect("ub: dereference of end iterator")
                    .0
                    .clone(),
            ))
        })
    }

    fn second(&self) -> Value<V> {
        self.entries.with(|v| {
            v.get(self.index)
                .expect("ub: dereference of end iterator")
                .1
                .clone()
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::AsPointer;

    type UIter = UnsafeVecMapIterator<i32, i32>;

    fn unsafe_entries() -> Vec<(i32, Box<i32>)> {
        // Insertion order is NEITHER sorted NOR reverse-sorted.
        vec![(30, Box::new(300)), (10, Box::new(100)), (20, Box::new(200))]
    }

    #[test]
    fn iteration_is_insertion_order_not_key_order() {
        let entries = unsafe_entries();
        let p: *const Vec<(i32, Box<i32>)> = &entries;
        let keys: Vec<i32> = UIter::begin(p)
            .map(|it| unsafe { *it.first() })
            .collect();
        // A BTreeMap-backed iterator yields 10, 20, 30 and FAILS here.
        assert_eq!(keys, vec![30, 10, 20]);
        let vals: Vec<i32> = UIter::begin(p).map(|it| unsafe { *it.second() }).collect();
        assert_eq!(vals, vec![300, 100, 200]);
    }

    #[test]
    fn end_is_len_and_eq_is_index() {
        let entries = unsafe_entries();
        let p: *const Vec<(i32, Box<i32>)> = &entries;
        let b = UIter::begin(p);
        let mut it = UIter::begin(p);
        assert!(it == b);
        it.inc();
        assert!(it != b, "different positions must compare unequal");
        it.inc();
        it.inc();
        assert!(it.is_end());
        assert!(it == UIter::end(p));
        assert_eq!(UIter::end(p).index(), 3);
        it.dec();
        assert_eq!(unsafe { *it.first() }, 20);
    }

    #[test]
    fn find_key_and_erase_preserve_insertion_order() {
        let mut entries = unsafe_entries();
        let p: *const Vec<(i32, Box<i32>)> = &mut entries;
        let it = UIter::find_key(p, &10);
        assert!(!it.is_end());
        assert_eq!(it.index(), 1);
        assert_eq!(unsafe { *it.second() }, 100);
        let next = UIter::erase(p, &it);
        assert_eq!(unsafe { *next.first() }, 20);
        let keys: Vec<i32> = UIter::begin(p).map(|it| unsafe { *it.first() }).collect();
        assert_eq!(keys, vec![30, 20]);
        assert!(UIter::find_key(p, &10).is_end());
    }

    #[test]
    fn second_points_at_the_stored_box() {
        let entries = unsafe_entries();
        let p: *const Vec<(i32, Box<i32>)> = &entries;
        let it = UIter::find_key(p, &20);
        unsafe { *it.second() = 999 };
        assert_eq!(*entries[2].1, 999, "iterator must alias the entry, not a copy");
    }

    #[test]
    fn refcount_flavour_is_insertion_order() {
        let owner: Value<Vec<(i32, Value<i32>)>> = Rc::new(RefCell::new(vec![
            (30, Rc::new(RefCell::new(300))),
            (10, Rc::new(RefCell::new(100))),
            (20, Rc::new(RefCell::new(200))),
        ]));
        let p = owner.as_pointer();
        let keys: Vec<i32> = RefcountVecMapIter::begin(p.clone())
            .map(|it| *it.first().borrow())
            .collect();
        assert_eq!(keys, vec![30, 10, 20]);
        let it = RefcountVecMapIter::find_key(p.clone(), &10);
        assert_eq!(it.index(), 1);
        assert_eq!(*it.second().borrow(), 100);
        // The value slot is SHARED with the entry, not copied.
        *it.second().borrow_mut() = 111;
        assert_eq!(*owner.borrow()[1].1.borrow(), 111);
        let next = RefcountVecMapIter::erase(p.clone(), &it);
        assert_eq!(*next.first().borrow(), 20);
        assert_eq!(owner.borrow().len(), 2);
    }
}
