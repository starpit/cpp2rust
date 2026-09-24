// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

//! `DeepClone`: what a C++ copy constructor actually means in the refcount model.
//!
//! # The defect this exists to fix
//!
//! In the refcount model a container's ELEMENT is a `Value<T>` = `Rc<RefCell<T>>`,
//! so `std::map<K, V>` is `BTreeMap<K, Value<V>>`.  Every container rule's copy
//! constructor therefore cannot use a plain `.clone()` -- that would clone the
//! `Rc` HANDLES and the copy would alias the original -- and each module instead
//! spelled out ONE level of unwrapping:
//!
//! ```ignore
//! a0.iter().map(|(k, v)| (k.clone(), Rc::new(RefCell::new(v.borrow().clone())))).collect()
//! ```
//!
//! That is correct when `V` is a scalar and WRONG the moment `V` is itself a
//! container, because the inner `v.borrow().clone()` is then a `BTreeMap`
//! containing `Value`s and `BTreeMap::clone` clones those handles.  Measured,
//! `$TC/shim4/clang++` as the arbiter:
//!
//! ```text
//! std::map<int, std::map<int,long>> a; a[1][2]=11; b(a); b[1][2]=22;
//!   C++ b=22 a=11 | unsafe b=22 a=11 | refcount b=22 a=22   <- aliased
//! ```
//!
//! The same shape was measured wrong for `map` copy-assign, `optional<map>`,
//! `pair<map,_>`, `vector<map>` and `variant`, i.e. it is systemic to the "one
//! level of Value unwrapping, hand-written per module" idiom rather than being
//! any one module's bug.  dt_src has 617 nested-container declarations, so it is
//! not a corner case either.
//!
//! # Why a trait rather than more hand-unwrapping
//!
//! Depth is not knowable at the rule's level: `rules/map`'s body sees `T2` and
//! cannot ask how many `Value` layers are inside it.  Recursion is the only thing
//! that terminates correctly for every depth, and a trait is how Rust spells
//! "recurse, dispatching on the static type".  `deep_clone` is therefore defined
//! once per structure -- `Value<T>` unwraps and recurses, the containers map over
//! their elements and recurse, everything else bottoms out in `Clone` -- and each
//! module's copy constructor becomes one call.
//!
//! # Why there is NO blanket impl over `Clone`
//!
//! `impl<T: Clone> DeepClone for T` (or the same thing behind a marker trait)
//! overlaps `impl<T: DeepClone> DeepClone for Value<T>` and rustc rejects it,
//! E0119 -- a `Value<T>` is itself `Clone`, and coherence does no negative
//! reasoning, so no amount of sealing the marker helps.  So every leaf is listed
//! EXPLICITLY by the `leaf!` macro below, which is the better failure mode
//! anyway: a type that is neither a listed leaf nor one of the recursing
//! structures is a COMPILE ERROR at the rule body rather than a silent shallow
//! copy, which is the direction this whole change exists to move in.
//!
//! Records are leaves, and legitimately so: the converter already emits a
//! per-field `Rc::new(RefCell::new((*self.f.borrow()).clone()))`, so a record
//! whose field is a container recurses through THAT field's container
//! `deep_clone` as soon as the impls below exist.  `EmitDeepCloneLeaf` in the
//! refcount converter emits the one-line `impl DeepClone for R` beside the
//! record's `Clone`.  Verified before this change: a
//! `struct R { std::map<int,long> m; int tag; }` copy was already correct, and
//! so was `std::map<int, R>` where `R` holds a map.

use crate::rc::{Ptr, Value};
use std::cell::RefCell;
use std::collections::{BTreeMap, BTreeSet, HashMap, HashSet, VecDeque};
use std::rc::Rc;

/// A copy constructor's semantics: copy the VALUE, not the handle.
pub trait DeepClone {
    fn deep_clone(&self) -> Self;
}

/// `deep_clone` for a type with no `Value` cell inside it, where copying the
/// value IS a `Clone`.  Used by the `leaf!` list here and by the converter for
/// each record it emits (whose `Clone` is already a deep per-field copy).
#[macro_export]
macro_rules! impl_deep_clone_leaf {
    ($ty:ty) => {
        impl $crate::DeepClone for $ty {
            #[inline]
            fn deep_clone(&self) -> Self {
                ::std::clone::Clone::clone(self)
            }
        }
    };
}

macro_rules! leaf {
    ($($ty:ty),* $(,)?) => { $(
        impl DeepClone for $ty {
            #[inline]
            fn deep_clone(&self) -> Self { self.clone() }
        }
    )* };
}

leaf!(
    u8, i8, u16, i16, u32, i32, u64, i64, u128, i128, usize, isize, f32, f64, bool, char, (),
    String, std::ffi::CString,
);

// The remaining libcc2rs types, each a leaf because it holds no `Value` cell.
// `StrongPtr`, `PtrDyn` and `AnyPtr` are POINTERS and so are leaves for the same
// reason `Ptr` is, below.
impl DeepClone for crate::void::AnyPtr {
    #[inline]
    fn deep_clone(&self) -> Self {
        self.clone()
    }
}

impl<T: ?Sized> DeepClone for crate::ptr_dyn::PtrDyn<T> {
    #[inline]
    fn deep_clone(&self) -> Self {
        self.clone()
    }
}

impl<T: crate::fn_ptr::FnSig> DeepClone for crate::fn_ptr::FnPtr<T> {
    #[inline]
    fn deep_clone(&self) -> Self {
        self.clone()
    }
}

// A raw pointer and a `Ptr` are POINTERS: C++ copies the pointer, not the
// pointee, so a shallow copy is the correct semantics here and not a shortcut.
// The `ptr_stays_shallow` test is the negative control for exactly this.
impl<T> DeepClone for *const T {
    #[inline]
    fn deep_clone(&self) -> Self {
        *self
    }
}

impl<T> DeepClone for *mut T {
    #[inline]
    fn deep_clone(&self) -> Self {
        *self
    }
}

impl<T> DeepClone for Ptr<T> {
    #[inline]
    fn deep_clone(&self) -> Self {
        self.clone()
    }
}

// --- the recursing structures ----------------------------------------------

/// The case the whole file is about: unwrap the cell, recurse into the value,
/// and put the result in a FRESH cell so the copy shares nothing.
impl<T: DeepClone> DeepClone for Value<T> {
    #[inline]
    fn deep_clone(&self) -> Self {
        Rc::new(RefCell::new(self.borrow().deep_clone()))
    }
}

impl<T: DeepClone> DeepClone for Option<T> {
    #[inline]
    fn deep_clone(&self) -> Self {
        self.as_ref().map(DeepClone::deep_clone)
    }
}

impl<T: DeepClone> DeepClone for Box<T> {
    #[inline]
    fn deep_clone(&self) -> Self {
        Box::new((**self).deep_clone())
    }
}

impl<T: DeepClone> DeepClone for Vec<T> {
    #[inline]
    fn deep_clone(&self) -> Self {
        self.iter().map(DeepClone::deep_clone).collect()
    }
}

impl<T: DeepClone> DeepClone for Box<[T]> {
    #[inline]
    fn deep_clone(&self) -> Self {
        self.iter().map(DeepClone::deep_clone).collect()
    }
}

impl<T: DeepClone> DeepClone for VecDeque<T> {
    #[inline]
    fn deep_clone(&self) -> Self {
        self.iter().map(DeepClone::deep_clone).collect()
    }
}

impl<K: DeepClone + Ord, V: DeepClone> DeepClone for BTreeMap<K, V> {
    #[inline]
    fn deep_clone(&self) -> Self {
        self.iter()
            .map(|(k, v)| (k.deep_clone(), v.deep_clone()))
            .collect()
    }
}

impl<K: DeepClone + Eq + std::hash::Hash, V: DeepClone> DeepClone for HashMap<K, V> {
    #[inline]
    fn deep_clone(&self) -> Self {
        self.iter()
            .map(|(k, v)| (k.deep_clone(), v.deep_clone()))
            .collect()
    }
}

impl<T: DeepClone + Ord> DeepClone for BTreeSet<T> {
    #[inline]
    fn deep_clone(&self) -> Self {
        self.iter().map(DeepClone::deep_clone).collect()
    }
}

impl<T: DeepClone + Eq + std::hash::Hash> DeepClone for HashSet<T> {
    #[inline]
    fn deep_clone(&self) -> Self {
        self.iter().map(DeepClone::deep_clone).collect()
    }
}

macro_rules! tuple_deep_clone {
    ($($name:ident.$idx:tt),+) => {
        impl<$($name: DeepClone),+> DeepClone for ($($name,)+) {
            #[inline]
            fn deep_clone(&self) -> Self {
                ($(self.$idx.deep_clone(),)+)
            }
        }
    };
}

tuple_deep_clone!(A.0);
tuple_deep_clone!(A.0, B.1);
tuple_deep_clone!(A.0, B.1, C.2);
tuple_deep_clone!(A.0, B.1, C.2, D.3);
tuple_deep_clone!(A.0, B.1, C.2, D.3, E.4);
tuple_deep_clone!(A.0, B.1, C.2, D.3, E.4, F.5);
tuple_deep_clone!(A.0, B.1, C.2, D.3, E.4, F.5, G.6);
tuple_deep_clone!(A.0, B.1, C.2, D.3, E.4, F.5, G.6, H.7);

// Fixed-size arrays: `std::array<T, N>` maps to one of these in the unsafe
// model and to a Vec in refcount, but a record field can still be a raw array.
impl<T: DeepClone, const N: usize> DeepClone for [T; N] {
    #[inline]
    fn deep_clone(&self) -> Self {
        std::array::from_fn(|i| self[i].deep_clone())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // The exact shape the C++ arbiter disagreed with: a map whose VALUE is a
    // map.  A handle clone makes the write visible through the original.
    #[test]
    fn nested_map_does_not_alias() {
        let mut a: BTreeMap<i32, Value<BTreeMap<i32, Value<i64>>>> = BTreeMap::new();
        let mut inner: BTreeMap<i32, Value<i64>> = BTreeMap::new();
        inner.insert(2, Rc::new(RefCell::new(11i64)));
        a.insert(1, Rc::new(RefCell::new(inner)));

        let b = a.deep_clone();
        *b[&1].borrow_mut().get_mut(&2).unwrap().borrow_mut() = 22;

        assert_eq!(*a[&1].borrow()[&2].borrow(), 11, "original was aliased");
        assert_eq!(*b[&1].borrow()[&2].borrow(), 22);
    }

    // Three levels, to show the recursion does not stop at two.
    #[test]
    fn triple_nesting_does_not_alias() {
        type L3 = BTreeMap<i32, Value<BTreeMap<i32, Value<BTreeMap<i32, Value<i64>>>>>>;
        let mut l1: BTreeMap<i32, Value<i64>> = BTreeMap::new();
        l1.insert(3, Rc::new(RefCell::new(7i64)));
        let mut l2: BTreeMap<i32, Value<BTreeMap<i32, Value<i64>>>> = BTreeMap::new();
        l2.insert(2, Rc::new(RefCell::new(l1)));
        let mut a: L3 = BTreeMap::new();
        a.insert(1, Rc::new(RefCell::new(l2)));

        let b = a.deep_clone();
        *b[&1].borrow()[&2].borrow().get(&3).unwrap().borrow_mut() = 8;

        assert_eq!(*a[&1].borrow()[&2].borrow()[&3].borrow(), 7);
        assert_eq!(*b[&1].borrow()[&2].borrow()[&3].borrow(), 8);
    }

    // A scalar element must still behave, i.e. the fix does not regress the
    // one-level case every module already had right.
    #[test]
    fn flat_map_still_copies() {
        let mut a: BTreeMap<i32, Value<i64>> = BTreeMap::new();
        a.insert(1, Rc::new(RefCell::new(11i64)));
        let b = a.deep_clone();
        *b[&1].borrow_mut() = 22;
        assert_eq!(*a[&1].borrow(), 11);
        assert_eq!(*b[&1].borrow(), 22);
    }

    #[test]
    fn vector_of_map_does_not_alias() {
        let mut inner: BTreeMap<i32, Value<i64>> = BTreeMap::new();
        inner.insert(1, Rc::new(RefCell::new(11i64)));
        let a: Vec<Value<BTreeMap<i32, Value<i64>>>> = vec![Rc::new(RefCell::new(inner))];
        let b = a.deep_clone();
        *b[0].borrow_mut().get_mut(&1).unwrap().borrow_mut() = 22;
        assert_eq!(*a[0].borrow()[&1].borrow(), 11);
    }

    #[test]
    fn optional_of_map_does_not_alias() {
        let mut inner: BTreeMap<i32, Value<i64>> = BTreeMap::new();
        inner.insert(1, Rc::new(RefCell::new(11i64)));
        let a: Option<Value<BTreeMap<i32, Value<i64>>>> = Some(Rc::new(RefCell::new(inner)));
        let b = a.deep_clone();
        *b.as_ref().unwrap().borrow_mut().get_mut(&1).unwrap().borrow_mut() = 22;
        assert_eq!(*a.as_ref().unwrap().borrow()[&1].borrow(), 11);
    }

    #[test]
    fn pair_of_map_does_not_alias() {
        let mut inner: BTreeMap<i32, Value<i64>> = BTreeMap::new();
        inner.insert(1, Rc::new(RefCell::new(11i64)));
        let a: (Value<BTreeMap<i32, Value<i64>>>, Value<i64>) =
            (Rc::new(RefCell::new(inner)), Rc::new(RefCell::new(5i64)));
        let b = a.deep_clone();
        *b.0.borrow_mut().get_mut(&1).unwrap().borrow_mut() = 22;
        assert_eq!(*a.0.borrow()[&1].borrow(), 11);
    }

    // A Ptr is a POINTER: C++ copies the pointer, so the copy must still see
    // the same pointee.  This is the negative control for the impls above --
    // "deep" must not mean "deep through pointers too".
    #[test]
    fn ptr_stays_shallow() {
        let owner: Value<i64> = Rc::new(RefCell::new(1i64));
        let a: Vec<Ptr<i64>> = vec![crate::rc::AsPointer::as_pointer(&owner)];
        let b = a.deep_clone();
        *owner.borrow_mut() = 9;
        assert_eq!(b[0].read(), 9, "a Ptr copy must still alias its pointee");
    }
}
