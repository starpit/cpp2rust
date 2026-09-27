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
