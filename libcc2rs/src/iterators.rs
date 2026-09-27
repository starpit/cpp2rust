// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use crate::{PostfixDec, PostfixInc, PrefixDec, PrefixInc, Ptr, Value};
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::collections::BTreeSet;
use std::ops::Bound;
use std::rc::Rc;

pub trait MapAccess: Clone + Default {
    type Key: Ord + Clone;
    type Value;
    fn with<R>(&self, f: impl FnOnce(&BTreeMap<Self::Key, Self::Value>) -> R) -> R;
    fn with_mut<R>(&self, f: impl FnOnce(&mut BTreeMap<Self::Key, Self::Value>) -> R) -> R;
}

impl<K: Ord + Clone + 'static, V: 'static> MapAccess for Ptr<BTreeMap<K, V>> {
    type Key = K;
    type Value = V;

    fn with<R>(&self, f: impl FnOnce(&BTreeMap<K, V>) -> R) -> R {
        Ptr::with(self, f)
    }

    fn with_mut<R>(&self, f: impl FnOnce(&mut BTreeMap<K, V>) -> R) -> R {
        Ptr::with_mut(self, f)
    }
}

impl<K: Ord + Clone, V> MapAccess for *const BTreeMap<K, V> {
    type Key = K;
    type Value = V;

    fn with<R>(&self, f: impl FnOnce(&BTreeMap<K, V>) -> R) -> R {
        unsafe { f(&**self) }
    }

    fn with_mut<R>(&self, f: impl FnOnce(&mut BTreeMap<K, V>) -> R) -> R {
        unsafe { f(&mut *(*self as *mut BTreeMap<K, V>)) }
    }
}

pub struct MapIter<K, MapRef> {
    map: MapRef,
    key: Option<K>,
}

pub type RefcountMapIter<K, V> = MapIter<K, Ptr<BTreeMap<K, Value<V>>>>;
pub type UnsafeMapIterator<K, V> = MapIter<K, *const BTreeMap<K, Box<V>>>;

impl<K: Clone, MapRef: Clone> Clone for MapIter<K, MapRef> {
    fn clone(&self) -> Self {
        Self {
            map: self.map.clone(),
            key: self.key.clone(),
        }
    }
}

impl<K: PartialEq, MapRef> PartialEq for MapIter<K, MapRef> {
    fn eq(&self, other: &Self) -> bool {
        self.key == other.key
    }
}

impl<K: Ord + Clone, MapRef: MapAccess<Key = K>> MapIter<K, MapRef> {
    pub fn null() -> Self {
        Self {
            map: Default::default(),
            key: None,
        }
    }

    pub fn begin(map: MapRef) -> Self {
        let first_key = map.with(|m| m.keys().next().cloned());
        Self {
            map,
            key: first_key,
        }
    }

    pub fn end(map: MapRef) -> Self {
        Self { map, key: None }
    }

    pub fn find_key(map: MapRef, key: &K) -> Self {
        if map.with(|m| m.contains_key(key)) {
            Self {
                map,
                key: Some(key.clone()),
            }
        } else {
            Self::end(map)
        }
    }

    pub fn is_end(&self) -> bool {
        self.key.is_none()
    }

    pub fn key(&self) -> &Option<K> {
        &self.key
    }

    pub fn inc(&mut self) {
        self.key = match &self.key {
            Some(k) => self.map.with(|map| {
                map.range((Bound::Excluded(k), Bound::Unbounded))
                    .next()
                    .map(|(k, _)| k.clone())
            }),
            None => panic!("ub: increment past end"),
        };
    }

    pub fn dec(&mut self) {
        self.key = match &self.key {
            Some(k) => self.map.with(|map| {
                map.range((Bound::Unbounded, Bound::Excluded(k)))
                    .next_back()
                    .map(|(k, _)| k.clone())
            }),
            None => self.map.with(|map| map.keys().next_back().cloned()),
        };
    }

    pub fn erase(map: MapRef, iter: &Self) -> Self {
        let key = iter
            .key
            .as_ref()
            .expect("ub: erase of end iterator")
            .clone();
        let next_key = map.with(|m| {
            m.range((Bound::Excluded(&key), Bound::Unbounded))
                .next()
                .map(|(k, _)| k.clone())
        });
        map.with_mut(|m| m.remove(&key));
        Self { map, key: next_key }
    }
}

pub trait MapIterator {
    type First;
    type Second;
    fn first(&self) -> Self::First;
    fn second(&self) -> Self::Second;
}

impl<K: Ord + Clone + 'static, V: 'static> MapIterator for RefcountMapIter<K, V> {
    type First = Value<K>;
    type Second = Value<V>;

    fn first(&self) -> Value<K> {
        let key = self.key.as_ref().expect("ub: dereference of end iterator");
        Rc::new(RefCell::new(key.clone()))
    }

    fn second(&self) -> Value<V> {
        let key = self.key.as_ref().expect("ub: dereference of end iterator");
        self.map
            .with(|m| m.get(key).expect("ub: key not found in map").clone())
    }
}

impl<K: Ord + Clone, V> MapIterator for UnsafeMapIterator<K, V> {
    type First = *const K;
    type Second = *mut V;

    fn first(&self) -> *const K {
        self.key.as_ref().expect("ub: dereference of end iterator") as *const K
    }

    fn second(&self) -> *mut V {
        unsafe {
            let map_mut = self.map as *mut BTreeMap<K, Box<V>>;
            let key = self.key.as_ref().expect("ub: dereference of end iterator");
            (*map_mut)
                .get_mut(key)
                .expect("ub: key not found in map")
                .as_mut() as *mut V
        }
    }
}

impl<K: Ord + Clone, MapRef: MapAccess<Key = K>> Iterator for MapIter<K, MapRef> {
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

impl<K: Ord + Clone, MapRef: MapAccess<Key = K>> PrefixInc for MapIter<K, MapRef> {
    fn prefix_inc(&mut self) -> Self {
        self.inc();
        self.clone()
    }
}

impl<K: Ord + Clone, MapRef: MapAccess<Key = K>> PostfixInc for MapIter<K, MapRef> {
    fn postfix_inc(&mut self) -> Self {
        let ret = self.clone();
        self.inc();
        ret
    }
}

impl<K: Ord + Clone, MapRef: MapAccess<Key = K>> PrefixDec for MapIter<K, MapRef> {
    fn prefix_dec(&mut self) -> Self {
        self.dec();
        self.clone()
    }
}

impl<K: Ord + Clone, MapRef: MapAccess<Key = K>> PostfixDec for MapIter<K, MapRef> {
    fn postfix_dec(&mut self) -> Self {
        let ret = self.clone();
        self.dec();
        ret
    }
}

// ===========================================================================
// std::unordered_map / std::unordered_set iterators.
//
// ORDER CAVEAT -- READ THIS BEFORE USING THESE TO ITERATE.
// BTreeMap iteration is ordered and matches std::map exactly, so `MapIter` above is a
// faithful model of std::map::iterator. HashMap's iteration order is UNSPECIFIED and
// differs from libc++'s `__hash_table` order (and, being randomly seeded per process,
// differs between runs of the same Rust program). `HashMapIter` is therefore faithful
// for find / lookup / size / equality / erase, and a FULL traversal does visit every
// element exactly once -- but any site that ITERATES an unordered container and depends
// on the order it observes is NOT reproduced by this model. C++ gives no portable
// guarantee there either, yet such a site will still differ from the clang-built
// program, so it must not be claimed as verified.
//
// std::unordered_map::iterator is a ForwardIterator: there is no `operator--`. So this
// type deliberately implements only `inc()` and does NOT implement PrefixDec/PostfixDec.
// ===========================================================================

use std::collections::{HashMap, HashSet};
use std::hash::Hash;

pub trait HashMapAccess: Clone + Default {
    type Key: Hash + Eq + Clone;
    type Value;
    fn with<R>(&self, f: impl FnOnce(&HashMap<Self::Key, Self::Value>) -> R) -> R;
    fn with_mut<R>(&self, f: impl FnOnce(&mut HashMap<Self::Key, Self::Value>) -> R) -> R;
}

impl<K: Hash + Eq + Clone + 'static, V: 'static> HashMapAccess for Ptr<HashMap<K, V>> {
    type Key = K;
    type Value = V;

    fn with<R>(&self, f: impl FnOnce(&HashMap<K, V>) -> R) -> R {
        Ptr::with(self, f)
    }

    fn with_mut<R>(&self, f: impl FnOnce(&mut HashMap<K, V>) -> R) -> R {
        Ptr::with_mut(self, f)
    }
}

impl<K: Hash + Eq + Clone, V> HashMapAccess for *const HashMap<K, V> {
    type Key = K;
    type Value = V;

    fn with<R>(&self, f: impl FnOnce(&HashMap<K, V>) -> R) -> R {
        unsafe { f(&**self) }
    }

    fn with_mut<R>(&self, f: impl FnOnce(&mut HashMap<K, V>) -> R) -> R {
        unsafe { f(&mut *(*self as *mut HashMap<K, V>)) }
    }
}

pub struct HashMapIter<K, MapRef> {
    map: MapRef,
    key: Option<K>,
}

pub type RefcountHashMapIter<K, V> = HashMapIter<K, Ptr<HashMap<K, Value<V>>>>;
pub type UnsafeHashMapIterator<K, V> = HashMapIter<K, *const HashMap<K, Box<V>>>;

impl<K: Clone, MapRef: Clone> Clone for HashMapIter<K, MapRef> {
    fn clone(&self) -> Self {
        Self {
            map: self.map.clone(),
            key: self.key.clone(),
        }
    }
}

// Identity is the KEY, not a bucket address: keys are unique, so two iterators into the
// same table compare equal exactly when they denote the same element, and `end()` (key
// None) compares equal only to another end. Two DIFFERENT elements that happen to hold
// equal VALUES therefore compare unequal, as in C++.
impl<K: PartialEq, MapRef> PartialEq for HashMapIter<K, MapRef> {
    fn eq(&self, other: &Self) -> bool {
        self.key == other.key
    }
}

impl<K: Hash + Eq + Clone, MapRef: HashMapAccess<Key = K>> HashMapIter<K, MapRef> {
    pub fn null() -> Self {
        Self {
            map: Default::default(),
            key: None,
        }
    }

    pub fn begin(map: MapRef) -> Self {
        let first_key = map.with(|m| m.keys().next().cloned());
        Self {
            map,
            key: first_key,
        }
    }

    pub fn end(map: MapRef) -> Self {
        Self { map, key: None }
    }

    pub fn find_key(map: MapRef, key: &K) -> Self {
        if map.with(|m| m.contains_key(key)) {
            Self {
                map,
                key: Some(key.clone()),
            }
        } else {
            Self::end(map)
        }
    }

    pub fn is_end(&self) -> bool {
        self.key.is_none()
    }

    pub fn key(&self) -> &Option<K> {
        &self.key
    }

    /// `it->first` / `it->second` for ANY `MapRef`, including one whose `Value` is the
    /// bare mapped type rather than `Box<V>`.  The `MapIterator` impls below are bound
    /// to the two ALIASES (`*const HashMap<K, Box<V>>` and `Ptr<HashMap<K, Value<V>>>`),
    /// and a generic `impl MapIterator for HashMapIter<K, *const HashMap<K, V>>` cannot
    /// be added alongside the `Box<V>` one -- the two overlap (V = Box<V'>) and rustc
    /// rejects it with E0119.  These inherent methods are therefore the coherence-free
    /// way for a module whose map is a BARE `HashMap<K, V>` (rules/densemap's t1) to
    /// reach the same two accessors.  Names deliberately differ from `first`/`second`
    /// so that no call can silently resolve to the wrong one.
    pub fn key_ptr(&self) -> *const K {
        self.key.as_ref().expect("ub: dereference of end iterator") as *const K
    }

    pub fn value_ptr(&self) -> *mut MapRef::Value {
        let key = self.key.as_ref().expect("ub: dereference of end iterator");
        self.map.with_mut(|m| {
            m.get_mut(key).expect("ub: key not found in map") as *mut MapRef::Value
        })
    }

    /// Advance in HashMap's own iteration order. That order is unspecified but
    /// deterministic for an unmodified table, so walking to the current key and taking
    /// the next one is a stable traversal that visits every element exactly once.
    /// O(n) per step, O(n^2) per traversal -- correctness over speed, and it avoids
    /// imposing an `Ord` bound that the C++ key type need not satisfy.
    pub fn inc(&mut self) {
        let cur = match &self.key {
            Some(k) => k.clone(),
            None => panic!("ub: increment past end"),
        };
        self.key = self.map.with(|m| {
            let mut iter = m.keys();
            let mut found = false;
            for k in iter.by_ref() {
                if *k == cur {
                    found = true;
                    break;
                }
            }
            if !found {
                panic!("ub: increment of an invalidated unordered_map iterator");
            }
            iter.next().cloned()
        });
    }

    pub fn erase(map: MapRef, iter: &Self) -> Self {
        let key = iter
            .key
            .as_ref()
            .expect("ub: erase of end iterator")
            .clone();
        let mut next = Self {
            map: map.clone(),
            key: Some(key.clone()),
        };
        next.inc();
        let next_key = next.key;
        map.with_mut(|m| m.remove(&key));
        Self { map, key: next_key }
    }
}

impl<K: Hash + Eq + Clone + 'static, V: 'static> MapIterator for RefcountHashMapIter<K, V> {
    type First = Value<K>;
    type Second = Value<V>;

    fn first(&self) -> Value<K> {
        let key = self.key.as_ref().expect("ub: dereference of end iterator");
        Rc::new(RefCell::new(key.clone()))
    }

    fn second(&self) -> Value<V> {
        let key = self.key.as_ref().expect("ub: dereference of end iterator");
        self.map
            .with(|m| m.get(key).expect("ub: key not found in map").clone())
    }
}

impl<K: Hash + Eq + Clone, V> MapIterator for UnsafeHashMapIterator<K, V> {
    type First = *const K;
    type Second = *mut V;

    fn first(&self) -> *const K {
        self.key.as_ref().expect("ub: dereference of end iterator") as *const K
    }

    fn second(&self) -> *mut V {
        unsafe {
            let map_mut = self.map as *mut HashMap<K, Box<V>>;
            let key = self.key.as_ref().expect("ub: dereference of end iterator");
            (*map_mut)
                .get_mut(key)
                .expect("ub: key not found in map")
                .as_mut() as *mut V
        }
    }
}

impl<K: Hash + Eq + Clone, MapRef: HashMapAccess<Key = K>> Iterator for HashMapIter<K, MapRef> {
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

impl<K: Hash + Eq + Clone, MapRef: HashMapAccess<Key = K>> PrefixInc for HashMapIter<K, MapRef> {
    fn prefix_inc(&mut self) -> Self {
        self.inc();
        self.clone()
    }
}

impl<K: Hash + Eq + Clone, MapRef: HashMapAccess<Key = K>> PostfixInc for HashMapIter<K, MapRef> {
    fn postfix_inc(&mut self) -> Self {
        let ret = self.clone();
        self.inc();
        ret
    }
}

// std::unordered_set. A set element IS the key, so `first()`/`second()` do not apply and
// the MapIterator trait is deliberately not implemented; the element is read with
// `value()`. Same ORDER CAVEAT as above.
pub trait HashSetAccess: Clone + Default {
    type Key: Hash + Eq + Clone;
    fn with<R>(&self, f: impl FnOnce(&HashSet<Self::Key>) -> R) -> R;
    fn with_mut<R>(&self, f: impl FnOnce(&mut HashSet<Self::Key>) -> R) -> R;
}

impl<K: Hash + Eq + Clone + 'static> HashSetAccess for Ptr<HashSet<K>> {
    type Key = K;

    fn with<R>(&self, f: impl FnOnce(&HashSet<K>) -> R) -> R {
        Ptr::with(self, f)
    }

    fn with_mut<R>(&self, f: impl FnOnce(&mut HashSet<K>) -> R) -> R {
        Ptr::with_mut(self, f)
    }
}

impl<K: Hash + Eq + Clone> HashSetAccess for *const HashSet<K> {
    type Key = K;

    fn with<R>(&self, f: impl FnOnce(&HashSet<K>) -> R) -> R {
        unsafe { f(&**self) }
    }

    fn with_mut<R>(&self, f: impl FnOnce(&mut HashSet<K>) -> R) -> R {
        unsafe { f(&mut *(*self as *mut HashSet<K>)) }
    }
}

pub struct HashSetIter<K, SetRef> {
    set: SetRef,
    key: Option<K>,
}

pub type RefcountHashSetIter<K> = HashSetIter<K, Ptr<HashSet<K>>>;
pub type UnsafeHashSetIterator<K> = HashSetIter<K, *const HashSet<K>>;

impl<K: Clone, SetRef: Clone> Clone for HashSetIter<K, SetRef> {
    fn clone(&self) -> Self {
        Self {
            set: self.set.clone(),
            key: self.key.clone(),
        }
    }
}

impl<K: PartialEq, SetRef> PartialEq for HashSetIter<K, SetRef> {
    fn eq(&self, other: &Self) -> bool {
        self.key == other.key
    }
}

impl<K: Hash + Eq + Clone, SetRef: HashSetAccess<Key = K>> HashSetIter<K, SetRef> {
    pub fn null() -> Self {
        Self {
            set: Default::default(),
            key: None,
        }
    }

    pub fn begin(set: SetRef) -> Self {
        let first = set.with(|s| s.iter().next().cloned());
        Self { set, key: first }
    }

    pub fn end(set: SetRef) -> Self {
        Self { set, key: None }
    }

    pub fn find_key(set: SetRef, key: &K) -> Self {
        if set.with(|s| s.contains(key)) {
            Self {
                set,
                key: Some(key.clone()),
            }
        } else {
            Self::end(set)
        }
    }

    pub fn is_end(&self) -> bool {
        self.key.is_none()
    }

    pub fn value(&self) -> K {
        self.key
            .as_ref()
            .expect("ub: dereference of end iterator")
            .clone()
    }

    pub fn inc(&mut self) {
        let cur = match &self.key {
            Some(k) => k.clone(),
            None => panic!("ub: increment past end"),
        };
        self.key = self.set.with(|s| {
            let mut iter = s.iter();
            let mut found = false;
            for k in iter.by_ref() {
                if *k == cur {
                    found = true;
                    break;
                }
            }
            if !found {
                panic!("ub: increment of an invalidated unordered_set iterator");
            }
            iter.next().cloned()
        });
    }

    pub fn erase(set: SetRef, iter: &Self) -> Self {
        let key = iter
            .key
            .as_ref()
            .expect("ub: erase of end iterator")
            .clone();
        let mut next = Self {
            set: set.clone(),
            key: Some(key.clone()),
        };
        next.inc();
        let next_key = next.key;
        set.with_mut(|s| s.remove(&key));
        Self { set, key: next_key }
    }
}

impl<K: Hash + Eq + Clone, SetRef: HashSetAccess<Key = K>> Iterator for HashSetIter<K, SetRef> {
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

impl<K: Hash + Eq + Clone, SetRef: HashSetAccess<Key = K>> PrefixInc for HashSetIter<K, SetRef> {
    fn prefix_inc(&mut self) -> Self {
        self.inc();
        self.clone()
    }
}

impl<K: Hash + Eq + Clone, SetRef: HashSetAccess<Key = K>> PostfixInc for HashSetIter<K, SetRef> {
    fn postfix_inc(&mut self) -> Self {
        let ret = self.clone();
        self.inc();
        ret
    }
}

pub struct CStringIterator {
    pub(crate) ptr: Ptr<u8>,
}

impl Iterator for CStringIterator {
    type Item = u8;

    fn count(self) -> usize {
        self.ptr.c_str_len()
    }

    fn next(&mut self) -> Option<Self::Item> {
        // read until the null terminator
        match self.ptr.read() {
            0 => None,
            ch => {
                self.ptr += 1;
                Some(ch)
            }
        }
    }
}

impl<T> Ptr<T> {
    pub fn to_string_iterator(&self) -> StringIterator<T> {
        StringIterator { ptr: self.clone() }
    }
}

pub struct StringIterator<T> {
    ptr: Ptr<T>,
}

impl<T> Iterator for StringIterator<T> {
    type Item = Ptr<T>;
    fn next(&mut self) -> Option<Self::Item> {
        // stop before the null terminator at the last position
        if self.ptr.get_offset().wrapping_add(1) < self.ptr.len() {
            let value = self.ptr.clone();
            self.ptr += 1;
            Some(value)
        } else {
            None
        }
    }
}

// std::set / std::multiset -- an ORDERED set, so the element order this iterator walks is
// the sorted order of the container, exactly as libc++'s red-black tree iterator does. A set
// element IS the key, so `first()`/`second()` do not apply and the MapIterator trait is
// deliberately not implemented; the element is read with `value()`. Unlike HashSetIter there
// is NO order caveat: BTreeSet and std::set agree on the traversal order, and `inc()` is a
// range query from the current key rather than a linear rescan.
pub trait SetAccess: Clone + Default {
    type Key: Ord + Clone;
    fn with<R>(&self, f: impl FnOnce(&BTreeSet<Self::Key>) -> R) -> R;
    fn with_mut<R>(&self, f: impl FnOnce(&mut BTreeSet<Self::Key>) -> R) -> R;
}

impl<K: Ord + Clone + 'static> SetAccess for Ptr<BTreeSet<K>> {
    type Key = K;

    fn with<R>(&self, f: impl FnOnce(&BTreeSet<K>) -> R) -> R {
        Ptr::with(self, f)
    }

    fn with_mut<R>(&self, f: impl FnOnce(&mut BTreeSet<K>) -> R) -> R {
        Ptr::with_mut(self, f)
    }
}

impl<K: Ord + Clone> SetAccess for *const BTreeSet<K> {
    type Key = K;

    fn with<R>(&self, f: impl FnOnce(&BTreeSet<K>) -> R) -> R {
        unsafe { f(&**self) }
    }

    fn with_mut<R>(&self, f: impl FnOnce(&mut BTreeSet<K>) -> R) -> R {
        unsafe { f(&mut *(*self as *mut BTreeSet<K>)) }
    }
}

pub struct SetIter<K, SetRef> {
    set: SetRef,
    key: Option<K>,
}

pub type RefcountSetIter<K> = SetIter<K, Ptr<BTreeSet<K>>>;
pub type UnsafeSetIterator<K> = SetIter<K, *const BTreeSet<K>>;

impl<K: Clone, SetRef: Clone> Clone for SetIter<K, SetRef> {
    fn clone(&self) -> Self {
        Self {
            set: self.set.clone(),
            key: self.key.clone(),
        }
    }
}

impl<K: PartialEq, SetRef> PartialEq for SetIter<K, SetRef> {
    fn eq(&self, other: &Self) -> bool {
        self.key == other.key
    }
}

impl<K: Ord + Clone, SetRef: SetAccess<Key = K>> SetIter<K, SetRef> {
    pub fn null() -> Self {
        Self {
            set: Default::default(),
            key: None,
        }
    }

    pub fn begin(set: SetRef) -> Self {
        let first = set.with(|s| s.iter().next().cloned());
        Self { set, key: first }
    }

    pub fn end(set: SetRef) -> Self {
        Self { set, key: None }
    }

    pub fn find_key(set: SetRef, key: &K) -> Self {
        if set.with(|s| s.contains(key)) {
            Self {
                set,
                key: Some(key.clone()),
            }
        } else {
            Self::end(set)
        }
    }

    pub fn is_end(&self) -> bool {
        self.key.is_none()
    }

    pub fn value(&self) -> K {
        self.key
            .as_ref()
            .expect("ub: dereference of end iterator")
            .clone()
    }

    pub fn inc(&mut self) {
        let cur = match &self.key {
            Some(k) => k.clone(),
            None => panic!("ub: increment past end"),
        };
        self.key = self.set.with(|s| {
            if !s.contains(&cur) {
                panic!("ub: increment of an invalidated set iterator");
            }
            s.range((
                std::ops::Bound::Excluded(cur.clone()),
                std::ops::Bound::Unbounded,
            ))
            .next()
            .cloned()
        });
    }

    pub fn erase(set: SetRef, iter: &Self) -> Self {
        let key = iter
            .key
            .as_ref()
            .expect("ub: erase of end iterator")
            .clone();
        let mut next = Self {
            set: set.clone(),
            key: Some(key.clone()),
        };
        next.inc();
        let next_key = next.key;
        set.with_mut(|s| s.remove(&key));
        Self { set, key: next_key }
    }
}

impl<K: Ord + Clone, SetRef: SetAccess<Key = K>> Iterator for SetIter<K, SetRef> {
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

impl<K: Ord + Clone, SetRef: SetAccess<Key = K>> PrefixInc for SetIter<K, SetRef> {
    fn prefix_inc(&mut self) -> Self {
        self.inc();
        self.clone()
    }
}

impl<K: Ord + Clone, SetRef: SetAccess<Key = K>> PostfixInc for SetIter<K, SetRef> {
    fn postfix_inc(&mut self) -> Self {
        let ret = self.clone();
        self.inc();
        ret
    }
}

// `*it` on a set iterator yields `const T1 &`. It lowers by the MapIterator::first()
// precedent above: a raw `*const K` in the unsafe model, a `Value<K>` in the refcount model.
// The unsafe arm points at the element STORED IN THE CONTAINER, not at the copy the iterator
// carries, so the pointer stays valid for as long as the element does.
pub trait SetIterator {
    type Element;
    fn element(&self) -> Self::Element;
}

impl<K: Ord + Clone + 'static> SetIterator for RefcountSetIter<K> {
    type Element = Value<K>;

    fn element(&self) -> Value<K> {
        let key = self.key.as_ref().expect("ub: dereference of end iterator");
        Rc::new(RefCell::new(key.clone()))
    }
}

// The same `*it` lowering for std::unordered_set's iterator.  ORDER CAVEAT does not
// apply here: `element()` reads the element AT the iterator's current position, and the
// position is identified by the key itself, so this is order-independent and exact.
impl<K: Hash + Eq + Clone + 'static> SetIterator for RefcountHashSetIter<K> {
    type Element = Value<K>;

    fn element(&self) -> Value<K> {
        let key = self.key.as_ref().expect("ub: dereference of end iterator");
        Rc::new(RefCell::new(key.clone()))
    }
}

impl<K: Hash + Eq + Clone> SetIterator for UnsafeHashSetIterator<K> {
    type Element = *const K;

    fn element(&self) -> *const K {
        let key = self.key.as_ref().expect("ub: dereference of end iterator");
        unsafe {
            (*self.set)
                .get(key)
                .expect("ub: element not found in unordered_set") as *const K
        }
    }
}

impl<K: Ord + Clone> SetIterator for UnsafeSetIterator<K> {
    type Element = *const K;

    fn element(&self) -> *const K {
        let key = self.key.as_ref().expect("ub: dereference of end iterator");
        unsafe {
            (*self.set)
                .get(key)
                .expect("ub: element not found in set") as *const K
        }
    }
}

// ─── mlir::OperandRange::iterator and friends ────────────────────────────────
//
// An iterator over a sequence that OWNS THE SEQUENCE, rather than borrowing it. Every other
// iterator in this file carries a reference to its container (`*const BTreeMap<..>` in the
// unsafe model, `Ptr<..>` in the refcount model) because its container is a caller-owned
// lvalue: `m.begin()` in rules/map borrows `m`, which outlives the expression. This one
// cannot do that. `mlir::OperandRange` maps to a BY-VALUE `Vec<ir::Value>`
// (rules/mlir/tgt_unsafe.rs:265), so `getOperands()` produces a TEMPORARY; any pointer or
// `Ptr` into it dangles at the end of the inlined rule expression. Owning a clone is the only
// safe model, and it also keeps the type free of raw-pointer text, so the refcount overlay
// needs no override for it (the `rules/atomic` E0605 class -- `struct as *mut T` -- cannot
// arise here).
//
// An index-only iterator was considered and REJECTED: the load-bearing key is the two-iterator
// constructor `OperandRange{first, last}`, whose only parameters ARE the two iterators, so the
// pair must carry the elements. There is no third argument naming the sequence.
//
// DISCLOSURE, and it is the reason `range_from` names `first` as the authoritative owner: the
// two real call sites (KTDFArch.h.inc:521 and :526) make TWO SEPARATE `getOperands()` calls,
// so under owning-t14 the two iterators each own a DISTINCT CLONE of the sequence. That is
// correct given t14's snapshot semantics -- `ir::Value` equality is structural -- but it means
// a genuinely-mismatched iterator pair, which is UB in C++, would silently yield a plausible
// WRONG answer here instead of trapping. `debug_assert_eq!` is the cheap detector. It is a
// debug assert and not a hard panic because the equality scan is O(n) on every single range
// construction on a hot printing path, and because under owning semantics the mismatch is not
// itself memory-unsafe -- it is a wrong value -- so paying for it in release would buy
// nothing that the debug build does not already catch.
pub struct RangeIter<T> {
    seq: Vec<T>,
    idx: usize,
}

impl<T: Clone> Clone for RangeIter<T> {
    fn clone(&self) -> Self {
        Self {
            seq: self.seq.clone(),
            idx: self.idx,
        }
    }
}

// Hand-written rather than derived: `Vec::new()` needs no `T: Default`, and the type key's
// default constructor must work for every element type.
impl<T> Default for RangeIter<T> {
    fn default() -> Self {
        Self {
            seq: Vec::new(),
            idx: 0,
        }
    }
}

// Position only, following the MapIter/SetIter precedent above (which compares `key` and
// ignores `map`). Here it is also REQUIRED, not merely conventional: the two iterators of a
// real range come from separate `getOperands()` calls and so own separate clones, and an
// `it != end` that compared the sequences would still be true but an `it == end` built from
// the other clone would have to compare two Vecs on every loop test.
impl<T> PartialEq for RangeIter<T> {
    fn eq(&self, other: &Self) -> bool {
        self.idx == other.idx
    }
}

impl<T> Eq for RangeIter<T> {}

impl<T> PartialOrd for RangeIter<T> {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.idx.cmp(&other.idx))
    }
}

impl<T> Ord for RangeIter<T> {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.idx.cmp(&other.idx)
    }
}

impl<T> RangeIter<T> {
    pub fn begin(seq: Vec<T>) -> Self {
        Self { seq, idx: 0 }
    }

    pub fn end(seq: Vec<T>) -> Self {
        let idx = seq.len();
        Self { seq, idx }
    }

    pub fn is_end(&self) -> bool {
        self.idx >= self.seq.len()
    }

    pub fn index(&self) -> usize {
        self.idx
    }

    /// `*it`, as a borrow of the element the iterator owns. Deref is also implemented, so a
    /// rule body may spell either `at()` or `*it`.
    pub fn at(&self) -> &T {
        self.seq
            .get(self.idx)
            .expect("ub: dereference of end iterator")
    }

    pub fn inc(&mut self) {
        if self.idx >= self.seq.len() {
            panic!("ub: increment past end");
        }
        self.idx += 1;
    }

    pub fn dec(&mut self) {
        if self.idx == 0 {
            panic!("ub: decrement before begin");
        }
        self.idx -= 1;
    }

    /// `it + n` / `it - n` without going through the operator traits, for a rule body that
    /// needs a call form. A negative `n` steps backwards, as `operator+` does in C++.
    pub fn offset(&self, n: i64) -> Self
    where
        T: Clone,
    {
        let idx = if n >= 0 {
            self.idx
                .checked_add(n as usize)
                .expect("ub: iterator advanced out of range")
        } else {
            self.idx
                .checked_sub(n.unsigned_abs() as usize)
                .expect("ub: iterator moved before begin")
        };
        if idx > self.seq.len() {
            panic!("ub: iterator advanced past end");
        }
        Self {
            seq: self.seq.clone(),
            idx,
        }
    }

    /// `last - first`, the random-access iterator difference.
    pub fn distance(&self, other: &Self) -> i64 {
        self.idx as i64 - other.idx as i64
    }

    /// `OperandRange{first, last}` -- the two-iterator constructor. `first` is the
    /// authoritative owner of the sequence; see the disclosure at the type definition for
    /// why that is safe and what it costs.
    pub fn range_from(first: Self, last: Self) -> Vec<T>
    where
        T: Clone + PartialEq + std::fmt::Debug,
    {
        debug_assert_eq!(
            first.seq, last.seq,
            "ub: range built from iterators into different sequences"
        );
        assert!(
            first.idx <= last.idx && last.idx <= first.seq.len(),
            "ub: inverted or out-of-range iterator pair"
        );
        first.seq[first.idx..last.idx].to_vec()
    }
}

impl<T> std::ops::Deref for RangeIter<T> {
    type Target = T;

    fn deref(&self) -> &T {
        self.at()
    }
}

// `it[n]`. Both integer widths are provided because the converter's index expression carries
// whatever the C++ subscript was typed as; the two impls do not overlap.
impl<T> std::ops::Index<i64> for RangeIter<T> {
    type Output = T;

    fn index(&self, n: i64) -> &T {
        let idx = if n >= 0 {
            self.idx + n as usize
        } else {
            self.idx
                .checked_sub(n.unsigned_abs() as usize)
                .expect("ub: subscript before begin")
        };
        self.seq.get(idx).expect("ub: subscript out of range")
    }
}

impl<T> std::ops::Index<usize> for RangeIter<T> {
    type Output = T;

    fn index(&self, n: usize) -> &T {
        self.seq
            .get(self.idx + n)
            .expect("ub: subscript out of range")
    }
}

impl<T: Clone> std::ops::Add<i64> for RangeIter<T> {
    type Output = Self;

    fn add(self, n: i64) -> Self {
        self.offset(n)
    }
}

impl<T: Clone> std::ops::Sub<i64> for RangeIter<T> {
    type Output = Self;

    fn sub(self, n: i64) -> Self {
        self.offset(-n)
    }
}

// `last - first`. A distinct impl from `Sub<i64>` and it does not overlap it.
impl<T> std::ops::Sub<RangeIter<T>> for RangeIter<T> {
    type Output = i64;

    fn sub(self, other: Self) -> i64 {
        self.distance(&other)
    }
}

impl<T: Clone> std::ops::AddAssign<i64> for RangeIter<T> {
    fn add_assign(&mut self, n: i64) {
        *self = self.offset(n);
    }
}

impl<T: Clone> std::ops::SubAssign<i64> for RangeIter<T> {
    fn sub_assign(&mut self, n: i64) {
        *self = self.offset(-n);
    }
}

// The whole increment/decrement family, not just the one `g048` names: it is a
// `std::random_access_iterator_tag` facade, so keying only the operator a single call site
// happens to use would just move the abort to the next one.
impl<T: Clone> PrefixInc for RangeIter<T> {
    fn prefix_inc(&mut self) -> Self {
        self.inc();
        self.clone()
    }
}

impl<T: Clone> PostfixInc for RangeIter<T> {
    fn postfix_inc(&mut self) -> Self {
        let ret = self.clone();
        self.inc();
        ret
    }
}

impl<T: Clone> PrefixDec for RangeIter<T> {
    fn prefix_dec(&mut self) -> Self {
        self.dec();
        self.clone()
    }
}

impl<T: Clone> PostfixDec for RangeIter<T> {
    fn postfix_dec(&mut self) -> Self {
        let ret = self.clone();
        self.dec();
        ret
    }
}

// Yields a SNAPSHOT of the iterator at each position, matching MapIter/SetIter's
// `Iterator for` impls above rather than yielding elements, so a range-for lowering behaves
// the same way for every iterator in this file.
impl<T: Clone> Iterator for RangeIter<T> {
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

#[cfg(test)]
mod range_iter_tests {
    use super::RangeIter;
    use crate::{PostfixDec, PostfixInc, PrefixDec, PrefixInc};

    fn seq() -> Vec<i32> {
        vec![10, 20, 30, 40, 50]
    }

    // The two real call sites are `begin()..begin()+1` and `begin()+1..end()`, i.e. an
    // interior PREFIX and an interior SUFFIX. A test that only checks `0..len` cannot fail.
    #[test]
    fn range_from_interior_prefix() {
        let first = RangeIter::begin(seq());
        let last = RangeIter::begin(seq()) + 1;
        assert_eq!(RangeIter::range_from(first, last), vec![10]);
    }

    #[test]
    fn range_from_interior_suffix() {
        let first = RangeIter::begin(seq()) + 1;
        let last = RangeIter::end(seq());
        assert_eq!(RangeIter::range_from(first, last), vec![20, 30, 40, 50]);
    }

    #[test]
    fn range_from_interior_middle() {
        let first = RangeIter::begin(seq()) + 1;
        let last = RangeIter::end(seq()) - 2;
        assert_eq!(RangeIter::range_from(first, last), vec![20, 30]);
    }

    #[test]
    fn range_from_empty_is_not_the_whole_range() {
        let first = RangeIter::begin(seq()) + 2;
        let last = RangeIter::begin(seq()) + 2;
        assert_eq!(RangeIter::range_from(first, last), Vec::<i32>::new());
    }

    #[test]
    fn arithmetic_and_comparison() {
        let b = RangeIter::begin(seq());
        let e = RangeIter::end(seq());
        assert!(b < e);
        assert!(b.clone() + 5 == e);
        assert!(e.clone() - 5 == b);
        assert_eq!(e.clone() - b.clone(), 5);
        assert_eq!(*(b.clone() + 3), 40);
        // The index type must be spelled: with both `Index<i64>` and `Index<usize>` in scope
        // an unsuffixed literal defaults to `i32` and does not resolve. Not a problem for the
        // converter, which always emits an index of a concrete type, but a rule body that
        // writes a bare literal subscript will need a suffix.
        assert_eq!(b[2i64], 30);
        assert_eq!(b[2usize], 30);
        assert_eq!((b.clone() + 1)[1i64], 30);
        let mut it = b.clone() + 1;
        it += 2;
        assert_eq!(*it, 40);
        it -= 1;
        assert_eq!(*it, 30);
    }

    #[test]
    fn inc_dec_family() {
        let mut it = RangeIter::begin(seq());
        assert_eq!(*it.prefix_inc(), 20);
        assert_eq!(*it.postfix_inc(), 20);
        assert_eq!(*it, 30);
        assert_eq!(*it.prefix_dec(), 20);
        assert_eq!(*it.postfix_dec(), 20);
        assert_eq!(*it, 10);
        assert_eq!(RangeIter::begin(seq()).count(), 5);
    }

    #[test]
    fn default_is_an_empty_end_iterator() {
        let d: RangeIter<i32> = Default::default();
        assert!(d.is_end());
        assert_eq!(RangeIter::range_from(d.clone(), d), Vec::<i32>::new());
    }

    #[test]
    #[should_panic(expected = "different sequences")]
    fn mismatched_pair_is_caught_in_debug() {
        let first = RangeIter::begin(seq());
        let last = RangeIter::end(vec![1, 2, 3, 4, 5]);
        let _ = RangeIter::range_from(first, last);
    }
}
