// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use crate::{PostfixDec, PostfixInc, PrefixDec, PrefixInc};
use std::any::{Any, TypeId};

use std::{
    cell::{Ref, RefCell},
    fmt,
    ops::Sub,
    rc::{Rc, Weak},
};

use crate::reinterpret::{ByteRepr, OriginalAlloc, with_scratch};

pub type Value<T> = Rc<RefCell<T>>;

pub(crate) struct ReinterpretedView {
    // Pointer to the source of reinterpret
    pub(crate) alloc: OriginalAlloc,
    // C++ size of the reinterpreted view
    elem_byte_size: usize,
}

#[derive(Default)]
pub(crate) enum PtrKind<T> {
    #[default]
    Null,
    StackSingle(Weak<RefCell<T>>),
    StackArray(Weak<RefCell<Box<[T]>>>),
    HeapSingle(Weak<RefCell<T>>),
    HeapArray(Weak<RefCell<Box<[T]>>>),
    StackVec(Weak<RefCell<Vec<T>>>),
    HeapVec(Weak<RefCell<Vec<T>>>),
    Reinterpreted(Rc<ReinterpretedView>),
}

pub enum StrongPtr<T> {
    StackSingle(Rc<RefCell<T>>),
    Vec {
        rc: Rc<RefCell<Vec<T>>>,
        offset: usize,
    },
    StackArray {
        rc: Rc<RefCell<Box<[T]>>>,
        offset: usize,
    },
    Reinterpreted {
        alloc: OriginalAlloc,
        byte_offset: usize,
        // Local buffer for deref(). None until first access.
        // Read-through: refreshed from alloc on every deref() call.
        cell: RefCell<Option<T>>,
    },
}

impl<T: ByteRepr> StrongPtr<T> {
    pub fn deref(&self) -> Ref<'_, T> {
        match self {
            StrongPtr::StackSingle(rc) => rc.borrow(),
            StrongPtr::Vec { rc, offset } => Ref::map(rc.borrow(), |v| &v[*offset]),
            StrongPtr::StackArray { rc, offset } => Ref::map(rc.borrow(), |a| &a[*offset]),
            StrongPtr::Reinterpreted {
                alloc,
                byte_offset,
                cell,
            } => {
                // Read-through: always re-read from the original allocation.
                with_scratch(T::byte_size(), |buf| {
                    alloc.read_bytes(*byte_offset, buf);
                    *cell.borrow_mut() = Some(T::from_bytes(buf));
                });
                Ref::map(cell.borrow(), |opt| opt.as_ref().unwrap())
            }
        }
    }
}

impl<T> StrongPtr<T> {
    /// ⭐ `deref` without the `T: ByteRepr` bound -- the `StrongPtr` half of [`Ptr::with_ref`].
    ///
    /// `StrongPtr::deref` above is `where T: ByteRepr` for one reason only: its `Reinterpreted`
    /// arm decodes the pointee out of another allocation's bytes into its own `cell`. The
    /// `StackSingle` / `Vec` / `StackArray` arms just `RefCell::borrow()` (plus `Ref::map` and an
    /// index) and need no byte-level guarantee, so they are reachable for ANY `T` -- including a
    /// non-POD struct from a crate that cannot implement `ByteRepr` because of the orphan rule.
    ///
    /// ⚠️ PANICS on `Reinterpreted`: there is no `T` in memory to borrow, only bytes to decode.
    pub fn deref_ref(&self) -> Ref<'_, T> {
        match self {
            StrongPtr::StackSingle(rc) => rc.borrow(),
            StrongPtr::Vec { rc, offset } => Ref::map(rc.borrow(), |v| &v[*offset]),
            StrongPtr::StackArray { rc, offset } => Ref::map(rc.borrow(), |a| &a[*offset]),
            StrongPtr::Reinterpreted { .. } => panic!(
                "ub: deref_ref on a reinterpreted pointer: {} has no byte representation",
                std::any::type_name::<T>()
            ),
        }
    }
}

impl<T> fmt::Debug for PtrKind<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            PtrKind::Null => write!(f, "Null"),
            PtrKind::StackVec(w) => write!(f, "StackVec({:?})", w.as_ptr()),
            PtrKind::HeapVec(w) => write!(f, "HeapVec({:?})", w.as_ptr()),
            PtrKind::StackSingle(w) => write!(f, "StackSingle({:?})", w.as_ptr()),
            PtrKind::HeapSingle(w) => write!(f, "HeapSingle({:?})", w.as_ptr()),
            PtrKind::StackArray(w) => write!(f, "StackArray({:?})", w.as_ptr()),
            PtrKind::HeapArray(w) => write!(f, "HeapArray({:?})", w.as_ptr()),
            PtrKind::Reinterpreted(data) => {
                write!(f, "Reinterpreted(0x{:x})", data.alloc.address())
            }
        }
    }
}

impl<T> Clone for PtrKind<T> {
    fn clone(&self) -> Self {
        match self {
            PtrKind::Null => PtrKind::Null,
            PtrKind::StackVec(weak) => PtrKind::StackVec(weak.clone()),
            PtrKind::HeapVec(weak) => PtrKind::HeapVec(weak.clone()),
            PtrKind::StackSingle(weak) => PtrKind::StackSingle(weak.clone()),
            PtrKind::HeapSingle(weak) => PtrKind::HeapSingle(weak.clone()),
            PtrKind::StackArray(weak) => PtrKind::StackArray(weak.clone()),
            PtrKind::HeapArray(weak) => PtrKind::HeapArray(weak.clone()),
            PtrKind::Reinterpreted(data) => PtrKind::Reinterpreted(Rc::clone(data)),
        }
    }
}

impl<T> PtrKind<T> {
    fn address(&self) -> usize {
        match self {
            PtrKind::Null => 0,
            PtrKind::StackSingle(w) | PtrKind::HeapSingle(w) => w.as_ptr() as usize,
            PtrKind::StackVec(w) | PtrKind::HeapVec(w) => w.as_ptr() as usize,
            PtrKind::StackArray(w) | PtrKind::HeapArray(w) => w.as_ptr() as usize,
            PtrKind::Reinterpreted(data) => data.alloc.address(),
        }
    }
}

impl<T> Eq for PtrKind<T> {}

impl<T> PartialEq for PtrKind<T> {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (PtrKind::Null, PtrKind::Null) => true,
            _ => self.address() == other.address(),
        }
    }
}

impl<T> PartialOrd for PtrKind<T> {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        match (self, other) {
            (PtrKind::Null, PtrKind::Null) => Some(std::cmp::Ordering::Equal),
            _ => self.address().partial_cmp(&other.address()),
        }
    }
}

pub struct Ptr<T> {
    pub(crate) offset: usize,
    pub(crate) kind: PtrKind<T>,
}

impl<T> Default for Ptr<T> {
    fn default() -> Self {
        Self {
            offset: 0,
            kind: Default::default(),
        }
    }
}

impl<T> Clone for Ptr<T> {
    fn clone(&self) -> Self {
        Self {
            offset: self.offset,
            kind: self.kind.clone(),
        }
    }
}

impl<T> PartialEq for Ptr<T> {
    fn eq(&self, other: &Self) -> bool {
        self.byte_offset() == other.byte_offset() && self.kind == other.kind
    }
}

impl<T> Eq for Ptr<T> {}

impl<T> Ord for Ptr<T> {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        match self.kind.partial_cmp(&other.kind) {
            Some(std::cmp::Ordering::Equal) | None => self.byte_offset().cmp(&other.byte_offset()),
            Some(ord) => ord,
        }
    }
}

impl<T> PartialOrd for Ptr<T> {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}

impl<T> Ptr<T> {
    #[inline]
    pub fn null() -> Self {
        Self {
            offset: 0,
            kind: PtrKind::Null,
        }
    }

    #[inline]
    pub fn alloc(value: T) -> Self {
        let owner = Rc::new(RefCell::new(value));
        let weak = Rc::downgrade(&owner);
        let _ = Rc::into_raw(owner);
        Self {
            offset: 0,
            kind: PtrKind::HeapSingle(weak),
        }
    }

    #[inline]
    pub fn alloc_array(array: Box<[T]>) -> Self {
        let owner = Rc::new(RefCell::new(array));
        let weak = Rc::downgrade(&owner);
        let _ = Rc::into_raw(owner);
        Self {
            offset: 0,
            kind: PtrKind::HeapArray(weak),
        }
    }

    /// BORROW-PROVENANCE constructor: a `Ptr<T>` that ALIASES element 0 of an
    /// already-owned `Vec<T>` without allocating anything.
    ///
    /// This is the capability an aliasing `begin()` needs (e.g. `mlir::Region`'s block list,
    /// whose callers deref the iterator and mutate through it). The two allocating
    /// constructors are both WRONG for that job: `Ptr::null()` deref-of-nulls on the first
    /// access, and `Ptr::alloc(v[0].clone())` fabricates a fresh allocation, so `*begin()` is
    /// a COPY -- mutation through it is lost and `end()` comparison is against the wrong
    /// object.
    ///
    /// ⭐ NO NEW PROVENANCE IS INTRODUCED. This is deliberately a named, discoverable
    /// spelling of the EXISTING `PtrKind::StackVec` borrow provenance, which is already
    /// reachable as `<Rc<RefCell<Vec<T>>> as AsPointer<T>>::as_pointer().decay()`. Because the
    /// provenance is unchanged, every downstream rule is already correct and already tested:
    /// * `Drop` -- `Ptr<T>` has NO `Drop` impl at all, for ANY provenance. Ownership is
    ///   severed from `Ptr` by design: `alloc`/`alloc_array` LEAK the strong `Rc` via
    ///   `Rc::into_raw` and retain only a `Weak`, and freeing is the explicit `delete()`.
    ///   Dropping a `Ptr` therefore only decrements a weak count -- a no-op for the owner in
    ///   both provenances. **So `Drop` never has to distinguish them, because it does nothing
    ///   for either, and no discriminant is needed.**
    /// * `delete()` -- already discriminates: it matches only `Heap*`/`Reinterpreted` and
    ///   panics `"ub: invalid delete"` on every `Stack*` kind, so a borrowed `Ptr` cannot free
    ///   its owner's storage (see `decay_stack_vec_cannot_be_freed`).
    /// * `Clone` -- shallow: clones the `Weak` and copies `offset`. Both clones alias the same
    ///   `Vec`; neither owns it.
    /// * `PartialEq`/`Ord` -- compare `(byte_offset, kind.address())`, where `address()` is the
    ///   OWNER CELL's address (`Weak::as_ptr`), not the element's. So two `Ptr`s derived from
    ///   the same `Vec` compare equal iff their offsets match, which is exactly the
    ///   `it == end()` semantics an iterator needs, and pointers into different `Vec`s never
    ///   compare equal.
    ///
    /// ⚠️ LIVENESS, not lifetimes, is what keeps this safe: the retained `Weak` means every
    /// access goes through `weak.upgrade().expect("ub: dangling pointer")`, so a `Ptr` that
    /// outlives its owner panics loudly instead of reading freed memory. That is why the owner
    /// must be an `Rc<RefCell<Vec<T>>>` (`Value<Vec<T>>`) and NOT a plain `&mut Vec<T>`: a
    /// plain reference has no owner cell to downgrade, and a raw-pointer variant would silently
    /// lose this check on exactly the reallocating (splicing) mutations it must catch.
    #[inline]
    pub fn borrow_vec(owner: &Value<Vec<T>>) -> Self {
        Self {
            offset: 0,
            kind: PtrKind::StackVec(Rc::downgrade(owner)),
        }
    }

    #[inline]
    pub fn delete(&self) {
        match &self.kind {
            PtrKind::HeapSingle(weak) => {
                assert_eq!(Weak::strong_count(weak), 1, "ub: invalid delete");
                unsafe {
                    let strong = weak.upgrade().expect("ub: dangling pointer");
                    Rc::from_raw(Rc::as_ptr(&strong));
                }
                assert_eq!(Weak::strong_count(weak), 0, "ub: double free");
            }
            PtrKind::HeapArray(weak) => {
                assert_eq!(Weak::strong_count(weak), 1, "ub: invalid delete");
                unsafe {
                    let strong = weak.upgrade().expect("ub: dangling pointer");
                    Rc::from_raw(Rc::as_ptr(&strong));
                }
                assert_eq!(Weak::strong_count(weak), 0, "ub: double free");
            }
            PtrKind::HeapVec(weak) => {
                assert_eq!(Weak::strong_count(weak), 1, "ub: invalid delete");
                unsafe {
                    let strong = weak.upgrade().expect("ub: dangling pointer");
                    Rc::from_raw(Rc::as_ptr(&strong));
                }
                assert_eq!(Weak::strong_count(weak), 0, "ub: double free");
            }
            PtrKind::Reinterpreted(data) => data.alloc.delete(),
            PtrKind::Null => {}
            _ => panic!("ub: invalid delete"),
        }
    }

    #[inline]
    pub fn is_null(&self) -> bool {
        matches!(self.kind, PtrKind::Null)
    }

    // Normalize offset to bytes for cross-variant comparison.
    #[inline]
    fn byte_offset(&self) -> usize {
        match &self.kind {
            PtrKind::Reinterpreted(_) => self.offset,
            _ => self.offset.wrapping_mul(std::mem::size_of::<T>()),
        }
    }

    // For Reinterpreted, Ptr::offset is in bytes. For all other variants,
    // Ptr::offset is in elements (step = 1). This helper converts between
    // user-facing element counts and the internal offset units.
    #[inline]
    fn elem_step(&self) -> usize {
        match &self.kind {
            PtrKind::Reinterpreted(data) => data.elem_byte_size,
            _ => 1,
        }
    }

    #[inline]
    pub fn len(&self) -> usize {
        match &self.kind {
            PtrKind::Null => 0,
            PtrKind::StackSingle(_) | PtrKind::HeapSingle(_) => 1,
            PtrKind::StackVec(weak) | PtrKind::HeapVec(weak) => {
                weak.upgrade().expect("ub: dangling pointer").borrow().len()
            }
            PtrKind::StackArray(weak) | PtrKind::HeapArray(weak) => {
                weak.upgrade().expect("ub: dangling pointer").borrow().len()
            }
            PtrKind::Reinterpreted(data) => data.alloc.total_byte_len() / data.elem_byte_size,
        }
    }

    #[inline]
    pub fn is_empty(&self) -> bool {
        match &self.kind {
            PtrKind::Null => true,
            PtrKind::StackSingle(_) | PtrKind::HeapSingle(_) => false,
            PtrKind::StackVec(weak) | PtrKind::HeapVec(weak) => weak
                .upgrade()
                .expect("ub: dangling pointer")
                .borrow()
                .is_empty(),
            PtrKind::StackArray(weak) | PtrKind::HeapArray(weak) => weak
                .upgrade()
                .expect("ub: dangling pointer")
                .borrow()
                .is_empty(),
            PtrKind::Reinterpreted(data) => self.offset >= data.alloc.total_byte_len(),
        }
    }

    #[inline]
    pub fn offset(&self, offset: impl TryInto<isize>) -> Self {
        let offset = offset
            .try_into()
            .ok()
            .expect("the offset must fit in a isize");
        let step = self.elem_step();
        Self {
            kind: self.kind.clone(),
            offset: self
                .offset
                .wrapping_add(offset.wrapping_mul(step as isize) as usize),
        }
    }

    #[inline]
    pub fn get_offset(&self) -> usize {
        self.offset / self.elem_step()
    }

    #[inline]
    pub fn to_last(&self) -> Self {
        Self {
            kind: self.kind.clone(),
            offset: self.len().wrapping_sub(1).wrapping_mul(self.elem_step()),
        }
    }

    #[inline]
    pub fn to_end(&self) -> Self {
        Self {
            kind: self.kind.clone(),
            offset: self.len().wrapping_mul(self.elem_step()),
        }
    }

    pub fn upgrade(&self) -> StrongPtr<T> {
        match &self.kind {
            PtrKind::Null => panic!("ub: null pointer"),
            PtrKind::StackSingle(weak) | PtrKind::HeapSingle(weak) => {
                assert_eq!(self.offset, 0, "ub: invalid offset");
                StrongPtr::StackSingle(weak.upgrade().expect("ub: dangling pointer"))
            }
            PtrKind::StackVec(weak) | PtrKind::HeapVec(weak) => StrongPtr::Vec {
                rc: weak.upgrade().expect("ub: dangling pointer"),
                offset: self.offset,
            },
            PtrKind::StackArray(weak) | PtrKind::HeapArray(weak) => StrongPtr::StackArray {
                rc: weak.upgrade().expect("ub: dangling pointer"),
                offset: self.offset,
            },
            PtrKind::Reinterpreted(data) => StrongPtr::Reinterpreted {
                alloc: data.alloc.clone(),
                byte_offset: self.offset,
                cell: RefCell::new(None),
            },
        }
    }

    pub fn write(&self, value: T)
    where
        T: ByteRepr,
    {
        self.with_mut(|v| *v = value);
    }

    pub fn reinterpret_cast<U: ByteRepr>(&self) -> Ptr<U>
    where
        T: ByteRepr,
    {
        if TypeId::of::<T>() == TypeId::of::<U>() {
            let self_any: &dyn Any = self;
            return self_any.downcast_ref::<Ptr<U>>().unwrap().clone();
        }

        match self.original_alloc() {
            Some((alloc, byte_offset)) => Ptr::<U>::from_original_alloc(alloc, byte_offset),
            None => Ptr::null(),
        }
    }

    // The original allocation this pointer refers to, together with the byte
    // offset of the pointer into it. None for null pointers.
    pub(crate) fn original_alloc(&self) -> Option<(OriginalAlloc, usize)>
    where
        T: ByteRepr,
    {
        Some(match &self.kind {
            PtrKind::Null => return None,
            PtrKind::StackSingle(weak) | PtrKind::HeapSingle(weak) => (
                OriginalAlloc::single(weak),
                self.offset.wrapping_mul(T::byte_size()),
            ),
            PtrKind::StackVec(weak) | PtrKind::HeapVec(weak) => (
                OriginalAlloc::slice(weak),
                self.offset.wrapping_mul(T::byte_size()),
            ),
            PtrKind::StackArray(weak) | PtrKind::HeapArray(weak) => (
                OriginalAlloc::slice(weak),
                self.offset.wrapping_mul(T::byte_size()),
            ),
            PtrKind::Reinterpreted(data) => (data.alloc.clone(), self.offset),
        })
    }

    // A pointer that views the bytes of `alloc`, starting at `byte_offset`, as
    // a sequence of `T`. This is the only allocation of a reinterpret cast.
    pub(crate) fn from_original_alloc(alloc: OriginalAlloc, byte_offset: usize) -> Self
    where
        T: ByteRepr,
    {
        if T::byte_size() == 0 {
            panic!("cannot reinterpret_cast to zero-sized type");
        }
        Ptr {
            offset: byte_offset,
            kind: PtrKind::Reinterpreted(Rc::new(ReinterpretedView {
                alloc,
                elem_byte_size: T::byte_size(),
            })),
        }
    }
}

impl<T> Ptr<T> {
    pub fn with_mut<R>(&self, f: impl FnOnce(&mut T) -> R) -> R
    where
        T: ByteRepr,
    {
        match &self.kind {
            PtrKind::Null => panic!("ub: null pointer"),
            PtrKind::StackSingle(weak) | PtrKind::HeapSingle(weak) => {
                assert_eq!(self.offset, 0, "ub: invalid offset");
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let mut borrow = rc.borrow_mut();
                f(&mut *borrow)
            }
            PtrKind::StackVec(weak) | PtrKind::HeapVec(weak) => {
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let mut borrow = rc.borrow_mut();
                f(&mut borrow[self.offset])
            }
            PtrKind::StackArray(weak) | PtrKind::HeapArray(weak) => {
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let mut borrow = rc.borrow_mut();
                f(&mut borrow[self.offset])
            }
            PtrKind::Reinterpreted(data) => with_scratch(T::byte_size(), |buf| {
                data.alloc.read_bytes(self.offset, buf);
                let mut val = T::from_bytes(buf);
                let ret = f(&mut val);
                val.to_bytes(buf);
                data.alloc.write_bytes(self.offset, buf);
                ret
            }),
        }
    }

    pub fn with<R>(&self, f: impl FnOnce(&T) -> R) -> R
    where
        T: ByteRepr,
    {
        match &self.kind {
            PtrKind::Null => panic!("ub: null pointer"),
            PtrKind::StackSingle(weak) | PtrKind::HeapSingle(weak) => {
                assert_eq!(self.offset, 0, "ub: invalid offset");
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let borrow = rc.borrow();
                f(&*borrow)
            }
            PtrKind::StackVec(weak) | PtrKind::HeapVec(weak) => {
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let borrow = rc.borrow();
                f(&borrow[self.offset])
            }
            PtrKind::StackArray(weak) | PtrKind::HeapArray(weak) => {
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let borrow = rc.borrow();
                f(&borrow[self.offset])
            }
            PtrKind::Reinterpreted(data) => with_scratch(T::byte_size(), |buf| {
                data.alloc.read_bytes(self.offset, buf);
                f(&T::from_bytes(buf))
            }),
        }
    }

    /// ⭐ IN-PLACE deref that does NOT require `T: ByteRepr` -- read a pointee that has no byte
    /// representation.
    ///
    /// WHY THIS EXISTS. `with` / `with_mut` / `read` / `write` and `StrongPtr::deref` are all
    /// `where T: ByteRepr`, but a reading of their bodies shows **exactly one arm actually uses
    /// the trait**: `PtrKind::Reinterpreted`, which calls `T::byte_size()` / `T::from_bytes` /
    /// `T::to_bytes` to materialise a `T` out of the bytes of a DIFFERENT allocation. Every other
    /// arm is ordinary safe Rust -- `weak.upgrade()`, `RefCell::borrow{,_mut}()`, and an index --
    /// and never mentions a byte. So the bound is a *whole-method* requirement imposed by a
    /// *single* arm, and it is spurious for `StackSingle`/`StackVec`/`StackArray` **and equally
    /// for `HeapSingle`/`HeapVec`/`HeapArray`**: the `Heap*` kinds differ from `Stack*` only in
    /// whether `delete()` may free the owner, not in how a deref reaches the pointee.
    ///
    /// This matters because `ByteRepr` lives in `libcc2rs` while the non-POD pointee types
    /// (e.g. `dataflowir_gen::fmt::Block`) live in another crate, so **the orphan rule forbids
    /// any third crate from bridging them**: a `Ptr<NonPodStruct>` was previously impossible to
    /// dereference at all. `with_ref` gives the bound-free path.
    ///
    /// ⚠️ PANICS on `Reinterpreted` (and on `Null`, like every other accessor). A reinterpreted
    /// pointer's pointee does not exist as a `T` anywhere in memory -- it is a byte range that
    /// must be *decoded*, which is precisely the operation `ByteRepr` provides, so there is no
    /// reference to hand out. That case is a real requirement, not a spurious bound, and it fails
    /// loudly in the house style rather than returning something wrong.
    ///
    /// ⚠️ Holds the owner's `RefCell` borrow for the duration of `f` (as `with` does), so `f` must
    /// not re-enter a mutating accessor on the same owner.
    pub fn with_ref<R>(&self, f: impl FnOnce(&T) -> R) -> R {
        match &self.kind {
            PtrKind::Null => panic!("ub: null pointer"),
            PtrKind::StackSingle(weak) | PtrKind::HeapSingle(weak) => {
                assert_eq!(self.offset, 0, "ub: invalid offset");
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let borrow = rc.borrow();
                f(&*borrow)
            }
            PtrKind::StackVec(weak) | PtrKind::HeapVec(weak) => {
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let borrow = rc.borrow();
                f(&borrow[self.offset])
            }
            PtrKind::StackArray(weak) | PtrKind::HeapArray(weak) => {
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let borrow = rc.borrow();
                f(&borrow[self.offset])
            }
            PtrKind::Reinterpreted(_) => panic!(
                "ub: with_ref on a reinterpreted pointer: {} has no byte representation",
                std::any::type_name::<T>()
            ),
        }
    }

    /// ⭐ Mutating counterpart of [`Ptr::with_ref`]: in-place `&mut` deref with NO `T: ByteRepr`
    /// bound. Mutations are performed on the owner's storage itself, so they are visible through
    /// the owner and through every other `Ptr` aliasing it.
    ///
    /// See [`Ptr::with_ref`] for why the bound is spurious on all six `Stack*`/`Heap*` kinds, and
    /// why `Reinterpreted` legitimately panics.
    pub fn with_mut_ref<R>(&self, f: impl FnOnce(&mut T) -> R) -> R {
        match &self.kind {
            PtrKind::Null => panic!("ub: null pointer"),
            PtrKind::StackSingle(weak) | PtrKind::HeapSingle(weak) => {
                assert_eq!(self.offset, 0, "ub: invalid offset");
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let mut borrow = rc.borrow_mut();
                f(&mut *borrow)
            }
            PtrKind::StackVec(weak) | PtrKind::HeapVec(weak) => {
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let mut borrow = rc.borrow_mut();
                f(&mut borrow[self.offset])
            }
            PtrKind::StackArray(weak) | PtrKind::HeapArray(weak) => {
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let mut borrow = rc.borrow_mut();
                f(&mut borrow[self.offset])
            }
            PtrKind::Reinterpreted(_) => panic!(
                "ub: with_mut_ref on a reinterpreted pointer: {} has no byte representation",
                std::any::type_name::<T>()
            ),
        }
    }
}

impl Ptr<u8> {
    pub fn with_slice_mut<R>(&self, len: usize, f: impl FnOnce(&mut [u8]) -> R) -> R {
        let off = self.offset;
        match &self.kind {
            PtrKind::Null => panic!("ub: null pointer"),
            PtrKind::StackSingle(weak) | PtrKind::HeapSingle(weak) => {
                assert!(off == 0 && len <= 1, "ub: with_slice_mut out of bounds");
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let mut b = rc.borrow_mut();
                f(&mut std::slice::from_mut(&mut *b)[..len])
            }
            PtrKind::StackArray(weak) | PtrKind::HeapArray(weak) => {
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let mut b = rc.borrow_mut();
                f(&mut b[off..off + len])
            }
            PtrKind::StackVec(weak) | PtrKind::HeapVec(weak) => {
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let mut b = rc.borrow_mut();
                f(&mut b[off..off + len])
            }
            PtrKind::Reinterpreted(data) => with_scratch(len, |buf| {
                data.alloc.read_bytes(off, buf);
                let r = f(buf);
                data.alloc.write_bytes(off, buf);
                r
            }),
        }
    }

    pub fn with_slice<R>(&self, len: usize, f: impl FnOnce(&[u8]) -> R) -> R {
        let off = self.offset;
        match &self.kind {
            PtrKind::Null => panic!("ub: null pointer"),
            PtrKind::StackSingle(weak) | PtrKind::HeapSingle(weak) => {
                assert!(off == 0 && len <= 1, "ub: with_slice out of bounds");
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let b = rc.borrow();
                f(&std::slice::from_ref(&*b)[..len])
            }
            PtrKind::StackArray(weak) | PtrKind::HeapArray(weak) => {
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let b = rc.borrow();
                f(&b[off..off + len])
            }
            PtrKind::StackVec(weak) | PtrKind::HeapVec(weak) => {
                let rc = weak.upgrade().expect("ub: dangling pointer");
                let b = rc.borrow();
                f(&b[off..off + len])
            }
            PtrKind::Reinterpreted(data) => with_scratch(len, |buf| {
                data.alloc.read_bytes(off, buf);
                f(buf)
            }),
        }
    }

    pub fn slice_until(&self, end: &Self) -> Vec<u8> {
        assert!(self.kind == end.kind, "ub: invalid slice");
        assert!(self.offset <= end.offset);
        assert!(end.offset <= self.len());
        self.with_slice(end.offset - self.offset, |s| s.to_vec())
    }
}

impl<T: Clone + ByteRepr> Ptr<T> {
    pub fn read(&self) -> T {
        self.with(|v| v.clone())
    }
}

impl<T: std::io::Write + ByteRepr> Ptr<T> {
    pub fn write_fmt(&self, args: std::fmt::Arguments<'_>) -> std::io::Result<()> {
        self.with_mut(|inner| inner.write_fmt(args))
    }

    pub fn write_all(&self, buf: &[u8]) -> std::io::Result<()> {
        self.with_mut(|inner| inner.write_all(buf))
    }
}

impl<T: std::cmp::Ord> Ptr<T> {
    pub fn sort(&self, last: usize) {
        match self.kind {
            PtrKind::Null => panic!("ub: dereference of null pointer"),
            PtrKind::StackSingle(_) | PtrKind::HeapSingle(_) => {
                panic!("only vecs and arrays can be sorted")
            }
            PtrKind::StackVec(ref weak) | PtrKind::HeapVec(ref weak) => {
                let strong = weak.upgrade().expect("ub: dangling pointer");
                (*strong.borrow_mut())[self.get_offset()..last].sort();
            }
            PtrKind::StackArray(ref weak) | PtrKind::HeapArray(ref weak) => {
                let strong = weak.upgrade().expect("ub: dangling pointer");
                (*strong.borrow_mut())[self.get_offset()..last].sort();
            }
            PtrKind::Reinterpreted(_) => {
                panic!("sorting not supported for reinterpreted pointers")
            }
        }
    }
}

impl<T: Clone> Ptr<T> {
    pub fn sort_with_cmp<F>(&self, last: usize, mut cmp: F)
    where
        F: FnMut(Ptr<T>, Ptr<T>) -> bool,
    {
        fn sort<T: Clone, F: FnMut(Ptr<T>, Ptr<T>) -> bool>(
            slice: &mut [T],
            offset: usize,
            last: usize,
            cmp: &mut F,
        ) {
            slice[offset..last].sort_by(|a, b| {
                let val_a = Rc::new(RefCell::new(a.clone()));
                let val_b = Rc::new(RefCell::new(b.clone()));
                if cmp(val_a.as_pointer(), val_b.as_pointer()) {
                    std::cmp::Ordering::Less
                } else if cmp(val_b.as_pointer(), val_a.as_pointer()) {
                    std::cmp::Ordering::Greater
                } else {
                    std::cmp::Ordering::Equal
                }
            });
        }
        match self.kind {
            PtrKind::Null => panic!("ub: dereference of null pointer"),
            PtrKind::StackSingle(_) | PtrKind::HeapSingle(_) => {
                panic!("only vecs and arrays can be sorted")
            }
            PtrKind::StackVec(ref weak) | PtrKind::HeapVec(ref weak) => {
                let strong = weak.upgrade().expect("ub: dangling pointer");
                let mut borrow = strong.borrow_mut();
                sort(&mut borrow, self.get_offset(), last, &mut cmp);
            }
            PtrKind::StackArray(ref weak) | PtrKind::HeapArray(ref weak) => {
                let strong = weak.upgrade().expect("ub: dangling pointer");
                let mut borrow = strong.borrow_mut();
                sort(&mut borrow, self.get_offset(), last, &mut cmp);
            }
            PtrKind::Reinterpreted(_) => {
                panic!("sorting not supported for reinterpreted pointers")
            }
        }
    }
}

impl<T> IntoIterator for &Ptr<T>
where
    T: Clone,
{
    type Item = Ptr<T>;
    type IntoIter = Ptr<T>;

    fn into_iter(self) -> Self::IntoIter {
        self.clone()
    }
}

impl<T> Iterator for Ptr<T> {
    type Item = Ptr<T>;
    fn next(&mut self) -> Option<Self::Item> {
        if self.get_offset() < self.len() {
            let value = self.clone();
            self.offset += self.elem_step();
            Some(value)
        } else {
            None
        }
    }

    // Lets collections built from the iterator (e.g. `std::string`'s
    // constructors) allocate their storage once instead of growing it.
    fn size_hint(&self) -> (usize, Option<usize>) {
        let remaining = self.len().saturating_sub(self.get_offset());
        (remaining, Some(remaining))
    }
}

// Ptr iterator that yields values instead of pointers.
// It's more efficient and it's useful to implement idiomatic iterator patterns
pub struct PtrValueIter<T> {
    ptr: Ptr<T>,
    n: usize,
}

impl<T> PtrValueIter<T> {
    pub fn new(ptr: &Ptr<T>, n: usize) -> Self {
        Self {
            ptr: ptr.clone(),
            n,
        }
    }
}

impl<T: Clone + ByteRepr> Iterator for PtrValueIter<T> {
    type Item = T;

    fn next(&mut self) -> Option<Self::Item> {
        if self.n > 0 {
            let value = self.ptr.read();
            self.ptr += 1;
            self.n -= 1;
            Some(value)
        } else {
            None
        }
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        (self.n, Some(self.n))
    }
}

impl<T: Clone + ByteRepr> ExactSizeIterator for PtrValueIter<T> {}

impl<T> Sub for Ptr<T> {
    type Output = isize;
    fn sub(self, other: Self) -> Self::Output {
        assert!(self.kind == other.kind, "ub: invalid subtraction");
        (self.get_offset() as isize).wrapping_sub(other.get_offset() as isize)
    }
}

macro_rules! impl_ptr_add_sub_assign {
    ($($rhs:ty),+) => { $(
        impl<T> std::ops::AddAssign<$rhs> for Ptr<T> {
            #[inline]
            fn add_assign(&mut self, other: $rhs) {
                let step = self.elem_step();
                self.offset = self.offset.wrapping_add(
                    ((other as isize).wrapping_mul(step as isize)) as usize,
                );
            }
        }
        impl<T> std::ops::SubAssign<$rhs> for Ptr<T> {
            #[inline]
            fn sub_assign(&mut self, other: $rhs) {
                let step = self.elem_step();
                self.offset = self.offset.wrapping_sub(
                    ((other as isize).wrapping_mul(step as isize)) as usize,
                );
            }
        }
    )+ }
}
impl_ptr_add_sub_assign!(i32, u32, i64, u64, isize, usize);

macro_rules! impl_ptr_add_sub {
    ($($rhs:ty),+) => { $(
        impl<T> std::ops::Add<$rhs> for &Ptr<T> {
            type Output = Ptr<T>;
            #[inline]
            fn add(self, other: $rhs) -> Ptr<T> { let mut r = self.clone(); r += other; r }
        }
        impl<T> std::ops::Sub<$rhs> for &Ptr<T> {
            type Output = Ptr<T>;
            #[inline]
            fn sub(self, other: $rhs) -> Ptr<T> { let mut r = self.clone(); r += -(other as isize); r }
        }
        impl<T> std::ops::Add<$rhs> for Ptr<T> {
            type Output = Self;
            #[inline]
            fn add(mut self, other: $rhs) -> Self { self += other; self }
        }
        impl<T> std::ops::Sub<$rhs> for Ptr<T> {
            type Output = Self;
            #[inline]
            fn sub(mut self, other: $rhs) -> Self { self += -(other as isize); self }
        }
    )+ }
}
impl_ptr_add_sub!(i32, u32, u64, isize, usize);

impl<T> PostfixInc for Ptr<T> {
    #[inline]
    fn postfix_inc(&mut self) -> Self {
        let ret = self.clone();
        self.offset = self.offset.wrapping_add(self.elem_step());
        ret
    }
}

impl<T> PostfixDec for Ptr<T> {
    #[inline]
    fn postfix_dec(&mut self) -> Self {
        let ret = self.clone();
        self.offset = self.offset.wrapping_sub(self.elem_step());
        ret
    }
}

impl<T> PrefixInc for Ptr<T> {
    #[inline]
    fn prefix_inc(&mut self) -> Self {
        self.offset = self.offset.wrapping_add(self.elem_step());
        self.clone()
    }
}

impl<T> PrefixDec for Ptr<T> {
    #[inline]
    fn prefix_dec(&mut self) -> Self {
        self.offset = self.offset.wrapping_sub(self.elem_step());
        self.clone()
    }
}

pub trait AsPointer<T> {
    fn as_pointer(&self) -> Ptr<T>;
}

pub struct ScopedDestructor<T> {
    owner: Rc<RefCell<T>>,
    destroy: fn(Ptr<T>),
}

impl<T> ScopedDestructor<T> {
    pub fn new(owner: &Value<T>, destroy: fn(Ptr<T>)) -> Self {
        Self {
            owner: owner.clone(),
            destroy,
        }
    }
}

impl<T> Drop for ScopedDestructor<T> {
    fn drop(&mut self) {
        (self.destroy)(self.owner.as_pointer());
    }
}

pub struct ScopedDestructorUnsafe<T> {
    owner: *mut T,
    destroy: unsafe fn(&mut T),
}

impl<T> ScopedDestructorUnsafe<T> {
    pub fn new(owner: *mut T, destroy: unsafe fn(&mut T)) -> Self {
        Self { owner, destroy }
    }
}

impl<T> Drop for ScopedDestructorUnsafe<T> {
    fn drop(&mut self) {
        unsafe { (self.destroy)(&mut *self.owner) }
    }
}

impl<T> AsPointer<T> for Rc<RefCell<T>> {
    #[inline]
    fn as_pointer(&self) -> Ptr<T> {
        Ptr {
            offset: 0,
            kind: PtrKind::StackSingle(Rc::downgrade(self)),
        }
    }
}

impl<T> AsPointer<T> for Option<Rc<RefCell<T>>> {
    #[inline]
    fn as_pointer(&self) -> Ptr<T> {
        match self {
            None => Ptr::null(),
            Some(p) => p.as_pointer(),
        }
    }
}

impl<T> AsPointer<T> for Rc<RefCell<Box<[T]>>> {
    #[inline]
    fn as_pointer(&self) -> Ptr<T> {
        Ptr {
            offset: 0,
            kind: PtrKind::StackArray(Rc::downgrade(self)),
        }
    }
}

impl<T> AsPointer<T> for Option<Rc<RefCell<Box<[T]>>>> {
    #[inline]
    fn as_pointer(&self) -> Ptr<T> {
        match self {
            None => Ptr::null(),
            Some(p) => p.as_pointer(),
        }
    }
}

impl<T> AsPointer<T> for Rc<RefCell<Vec<T>>> {
    #[inline]
    fn as_pointer(&self) -> Ptr<T> {
        Ptr {
            offset: 0,
            kind: PtrKind::StackVec(Rc::downgrade(self)),
        }
    }
}

impl<T> Ptr<Vec<T>> {
    #[inline]
    pub fn decay(&self) -> Ptr<T> {
        match &self.kind {
            PtrKind::Null => Ptr::null(),
            PtrKind::StackSingle(weak) => Ptr {
                offset: self.offset,
                kind: PtrKind::StackVec(weak.clone()),
            },
            PtrKind::HeapSingle(weak) => Ptr {
                offset: self.offset,
                kind: PtrKind::HeapVec(weak.clone()),
            },
            _ => panic!("ub: invalid decay"),
        }
    }
}

impl<T> Ptr<Box<[T]>> {
    // Named `decay_array`, NOT `decay`, because a second inherent `decay` here made the
    // method UNRESOLVABLE whenever the receiver's pointee is still an inference variable.
    // Measured in scratch-fo/probe.refcount.rs:56, where the converter emits
    //   (Value::as_pointer(opt.as_ref().expect(..)).decay() as Ptr<u8>)
    // over a `Value<Vec<u8>>`: `Rc<RefCell<Vec<u8>>>` satisfies BOTH `AsPointer<Vec<u8>>`
    // (the generic impl above) and `AsPointer<u8>` (the Vec shortcut), so `as_pointer`
    // hands back `Ptr<_>` and rustc reported
    //   error[E0034]: multiple applicable items in scope ... multiple `decay` found
    //     candidate #1 ... `Ptr<Box<[T]>>`   candidate #2 ... `Ptr<Vec<T>>`
    // With one candidate left, the method probe UNIFIES the receiver with `Ptr<Vec<_>>`,
    // which in turn kills the `AsPointer<u8>` candidate and the chain type-checks.
    // (The same overlap is why `decay_stack_array_cannot_be_freed` below has to spell its
    // `as_pointer` through `&dyn AsPointer<Box<[i32]>>`.)
    #[inline]
    pub fn decay_array(&self) -> Ptr<T> {
        match &self.kind {
            PtrKind::Null => Ptr::null(),
            PtrKind::StackSingle(weak) => Ptr {
                offset: self.offset,
                kind: PtrKind::StackArray(weak.clone()),
            },
            PtrKind::HeapSingle(weak) => Ptr {
                offset: self.offset,
                kind: PtrKind::HeapArray(weak.clone()),
            },
            _ => panic!("ub: invalid decay"),
        }
    }
}

pub trait ToOwnedOption<T, O> {
    fn to_owned_opt(&self) -> Option<Rc<RefCell<O>>>;
}

impl<T> ToOwnedOption<T, T> for Ptr<T> {
    #[inline]
    fn to_owned_opt(&self) -> Option<Rc<RefCell<T>>> {
        match self.kind {
            PtrKind::Null => None,
            PtrKind::HeapSingle(ref weak) => {
                assert_eq!(self.offset, 0, "ub: invalid offset");
                assert_eq!(Weak::strong_count(weak), 1, "ub: invalid pointer");
                let strong = weak.upgrade().expect("ub: dangling pointer");
                // Delete the leaked reference
                unsafe {
                    Rc::from_raw(Rc::as_ptr(&strong));
                }
                assert_eq!(Rc::strong_count(&strong), 1, "wrong refs");
                Some(strong)
            }
            PtrKind::StackSingle(_) | PtrKind::StackArray(_) => {
                panic!("Can't own a stack variable")
            }
            PtrKind::StackVec(_) | PtrKind::HeapVec(_) => panic!("Can't own a vector"),
            PtrKind::HeapArray(_) => panic!("Can't own an array variable as single"),
            PtrKind::Reinterpreted(_) => panic!("Can't own a reinterpreted pointer"),
        }
    }
}

impl<T> ToOwnedOption<T, Box<[T]>> for Ptr<T> {
    #[inline]
    fn to_owned_opt(&self) -> Option<Rc<RefCell<Box<[T]>>>> {
        match self.kind {
            PtrKind::Null => None,
            PtrKind::HeapArray(ref weak) => {
                assert_eq!(self.offset, 0, "ub: invalid offset");
                assert_eq!(Weak::strong_count(weak), 1, "ub: invalid pointer");
                let strong = weak.upgrade().expect("ub: dangling pointer");
                // Delete the leaked reference
                unsafe {
                    Rc::from_raw(Rc::as_ptr(&strong));
                }
                assert_eq!(Rc::strong_count(&strong), 1, "wrong refs");
                Some(strong)
            }
            PtrKind::StackSingle(_) | PtrKind::StackArray(_) => {
                panic!("Can't own a stack variable")
            }
            PtrKind::StackVec(_) | PtrKind::HeapVec(_) => panic!("Can't own a vector"),
            PtrKind::HeapSingle(_) => panic!("Can't own a single variable as an array"),
            PtrKind::Reinterpreted(_) => panic!("Can't own a reinterpreted pointer"),
        }
    }
}

impl<T> fmt::Debug for Ptr<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let addr = match &self.kind {
            PtrKind::Null => 0,
            PtrKind::StackSingle(w) | PtrKind::HeapSingle(w) => {
                (Weak::as_ptr(w) as usize).wrapping_add(self.byte_offset())
            }
            PtrKind::StackArray(w) | PtrKind::HeapArray(w) => {
                (Weak::as_ptr(w) as usize).wrapping_add(self.byte_offset())
            }
            PtrKind::StackVec(w) | PtrKind::HeapVec(w) => {
                (Weak::as_ptr(w) as usize).wrapping_add(self.byte_offset())
            }
            PtrKind::Reinterpreted(data) => data.alloc.address().wrapping_add(self.byte_offset()),
        };
        write!(f, "0x{:x}", addr)
    }
}

impl<T: 'static> ByteRepr for Ptr<T> {}

impl<T: 'static> Ptr<T> {
    pub fn to_int(&self) -> usize {
        with_scratch(Self::byte_size(), |buf| {
            self.to_bytes(buf);
            usize::from_bytes(&buf[..std::mem::size_of::<usize>()])
        })
    }

    pub fn from_int(value: usize) -> Self {
        with_scratch(Self::byte_size(), |buf| {
            value.to_bytes(&mut buf[..std::mem::size_of::<usize>()]);
            Self::from_bytes(buf)
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // A NON-POD pointee that DELIBERATELY does not implement `ByteRepr`, standing in for a
    // foreign-crate struct (e.g. `dataflowir_gen::fmt::Block`) that the orphan rule forbids any
    // third crate from bridging to this crate's `ByteRepr`. It owns a `String` and a `Vec`, so it
    // has no meaningful byte representation and could not honestly implement the trait anyway.
    //
    // ⚠️ Do NOT add `impl ByteRepr for NonPod`: the whole point of the tests below is that they
    // reach the pointee through a path that never needs it. `p.read()` / `p.with(..)` /
    // `p.write(..)` / `upgrade().deref()` DO NOT COMPILE for this type -- only the `*_ref`
    // methods do.
    #[derive(Clone, Debug, PartialEq)]
    struct NonPod {
        name: String,
        args: Vec<String>,
    }

    impl NonPod {
        fn new(name: &str, args: &[&str]) -> Self {
            Self {
                name: name.to_string(),
                args: args.iter().map(|s| s.to_string()).collect(),
            }
        }
        // The shape the 8 consumer sites need after `begin()`: deref, then call a method.
        fn get_argument(&self, i: usize) -> &str {
            &self.args[i]
        }
    }

    // ⭐ THE PROOF of the new capability: a `Ptr<T>` for a `T` that does NOT implement `ByteRepr`
    // is dereferenced, read through, and mutated through -- with the mutation visible in the
    // owner. This is exactly the `begin()` + deref + `getArgument(0)` sequence.
    #[test]
    fn with_ref_derefs_a_non_byterepr_pointee_and_mutates_the_owner() {
        let owner: Value<Vec<NonPod>> = Rc::new(RefCell::new(vec![
            NonPod::new("entry", &["a0", "a1"]),
            NonPod::new("exit", &["b0"]),
        ]));
        let begin: Ptr<NonPod> = Ptr::borrow_vec(&owner);

        // Read a field, and call a method, through the Ptr -- no `ByteRepr` anywhere.
        assert_eq!(begin.with_ref(|b| b.name.clone()), "entry");
        assert_eq!(begin.with_ref(|b| b.get_argument(0).to_string()), "a0");
        assert_eq!(begin.with_ref(|b| b.args.len()), 2);

        // Offsetting the Ptr walks the owner's Vec, as for any other provenance.
        assert_eq!(begin.offset(1).with_ref(|b| b.name.clone()), "exit");
        assert_eq!(begin.offset(1).with_ref(|b| b.get_argument(0).to_string()), "b0");

        // ⭐ MUTATE THROUGH THE Ptr, OBSERVE IT IN THE OWNER: in-place, not a copy.
        begin.with_mut_ref(|b| b.name.push_str("_block"));
        begin.with_mut_ref(|b| b.args.push("a2".to_string()));
        begin.offset(1).with_mut_ref(|b| b.args[0] = "b0'".to_string());

        assert_eq!(owner.borrow()[0].name, "entry_block");
        assert_eq!(owner.borrow()[0].args, vec!["a0", "a1", "a2"]);
        assert_eq!(owner.borrow()[1].args, vec!["b0'"]);

        // And the reverse direction: mutate the owner, read it back through the Ptr.
        owner.borrow_mut()[0].name = "changed".to_string();
        assert_eq!(begin.with_ref(|b| b.name.clone()), "changed");

        // The aliasing is genuine: a clone of the Ptr sees the same storage, and no allocation
        // was made (the owner's strong count is still 1).
        assert_eq!(begin.clone().with_ref(|b| b.name.clone()), "changed");
        assert_eq!(Rc::strong_count(&owner), 1);

        // `StrongPtr::deref_ref` is the same capability through the upgrade path.
        assert_eq!(begin.upgrade().deref_ref().name, "changed");
        assert_eq!(begin.offset(1).upgrade().deref_ref().get_argument(0), "b0'");
    }

    // The bound is spurious for `Heap*` exactly as it is for `Stack*`: the `Heap*` arms of
    // `with`/`with_mut` never touch a byte either. So a heap-allocated non-POD pointee is
    // readable and mutable through the new path too, and `delete()` still frees it.
    #[test]
    fn with_ref_works_for_heap_provenance_too() {
        let single: Ptr<NonPod> = Ptr::alloc(NonPod::new("heap", &["h0"]));
        assert!(matches!(single.kind, PtrKind::HeapSingle(_)));
        assert_eq!(single.with_ref(|b| b.get_argument(0).to_string()), "h0");
        single.with_mut_ref(|b| b.name.push('!'));
        assert_eq!(single.with_ref(|b| b.name.clone()), "heap!");
        assert_eq!(single.upgrade().deref_ref().name, "heap!");
        single.delete();

        let array: Ptr<Box<[NonPod]>> = Ptr::alloc(
            vec![NonPod::new("a", &["0"]), NonPod::new("b", &["1"])].into_boxed_slice(),
        );
        let elems: Ptr<NonPod> = array.decay_array();
        assert!(matches!(elems.kind, PtrKind::HeapArray(_)));
        assert_eq!(elems.offset(1).with_ref(|b| b.name.clone()), "b");
        elems.offset(1).with_mut_ref(|b| b.args.clear());
        assert_eq!(elems.offset(1).with_ref(|b| b.args.len()), 0);
        elems.delete();
    }

    // ⚠️ `Reinterpreted` is the ONE arm where the `ByteRepr` bound is real, so the bound-free
    // path cannot serve it: there is no `T` in memory to borrow, only bytes to decode. It must
    // fail loudly, in the house style of `delete()`'s `"ub: ..."`.
    #[test]
    #[should_panic(expected = "ub: with_ref on a reinterpreted pointer")]
    fn with_ref_on_a_reinterpreted_ptr_panics() {
        let p: Ptr<u64> = Ptr::alloc(0x0807060504030201u64);
        let bytes: Ptr<u8> = p.reinterpret_cast::<u8>();
        assert!(matches!(bytes.kind, PtrKind::Reinterpreted(_)));
        bytes.with_ref(|b| *b);
    }

    #[test]
    #[should_panic(expected = "ub: with_mut_ref on a reinterpreted pointer")]
    fn with_mut_ref_on_a_reinterpreted_ptr_panics() {
        let p: Ptr<u64> = Ptr::alloc(0x0807060504030201u64);
        let bytes: Ptr<u8> = p.reinterpret_cast::<u8>();
        bytes.with_mut_ref(|b| *b = 0);
    }

    #[test]
    #[should_panic(expected = "ub: deref_ref on a reinterpreted pointer")]
    fn deref_ref_on_a_reinterpreted_ptr_panics() {
        let p: Ptr<u64> = Ptr::alloc(0x0807060504030201u64);
        let bytes: Ptr<u8> = p.reinterpret_cast::<u8>();
        let _ = *bytes.upgrade().deref_ref();
    }

    // Null still fails the same way it does for every other accessor.
    #[test]
    #[should_panic(expected = "ub: null pointer")]
    fn with_ref_on_null_panics() {
        Ptr::<NonPod>::null().with_ref(|b| b.args.len());
    }

    // THE ALIASING PROOF for `Ptr::borrow_vec` -- the capability an aliasing `begin()` needs.
    // Every assertion below is one the two allocating constructors FAIL:
    // `Ptr::alloc(v[0].clone())` fabricates a copy, so the write-through would not be visible in
    // the owner and the `end()`/identity comparisons would be against the wrong object;
    // `Ptr::null()` panics on the first deref.
    #[test]
    fn borrow_vec_ptr_aliases_the_owners_vec_and_does_not_allocate() {
        let owner: Value<Vec<i32>> = Rc::new(RefCell::new(vec![10, 20, 30]));
        let begin: Ptr<i32> = Ptr::borrow_vec(&owner);

        // Borrow provenance, and NO allocation: the Ptr only downgraded the caller's Rc, so the
        // strong count is untouched (an allocating ctor would have made a second allocation).
        assert!(matches!(begin.kind, PtrKind::StackVec(_)));
        assert_eq!(Rc::strong_count(&owner), 1);
        assert_eq!(begin.len(), 3);

        // ⭐ WRITE THROUGH THE Ptr, READ THE Vec: the mutation must be visible in the owner.
        begin.write(11);
        begin.offset(2).write(33);
        assert_eq!(*owner.borrow(), vec![11, 20, 33]);

        // ⭐ And the reverse direction: mutate the Vec, read it back through the Ptr.
        owner.borrow_mut()[1] = 22;
        assert_eq!(begin.read(), 11);
        assert_eq!(begin.offset(1).read(), 22);
        assert_eq!(begin.offset(2).read(), 33);

        // Iterator identity: `it == end()` semantics over the same owner.
        let end = begin.to_end();
        assert_ne!(begin, end);
        assert_eq!(end, begin.offset(3));
        assert!(begin < end);
        assert_eq!(begin.to_last(), begin.offset(2));

        // Same owner => equal provenance; a Ptr re-derived independently compares equal, and the
        // pre-existing spelling of this provenance is the SAME Ptr.
        assert_eq!(begin, Ptr::borrow_vec(&owner));
        assert_eq!(begin, owner.as_pointer().decay());

        // A different Vec with identical contents must NOT compare equal.
        let other: Value<Vec<i32>> = Rc::new(RefCell::new(vec![11, 22, 33]));
        assert_ne!(begin, Ptr::borrow_vec(&other));

        // Clone is shallow: the clone aliases the same Vec, it does not copy it.
        let cloned = begin.clone();
        cloned.offset(1).write(99);
        assert_eq!(*owner.borrow(), vec![11, 99, 33]);
    }

    // A borrowed Ptr must never free its owner's storage: provenance IS discriminated where it
    // matters, by `delete()`, even though `Drop` does nothing for either provenance.
    #[test]
    #[should_panic(expected = "ub: invalid delete")]
    fn borrow_vec_ptr_cannot_be_freed() {
        let owner: Value<Vec<i32>> = Rc::new(RefCell::new(vec![1, 2, 3]));
        Ptr::borrow_vec(&owner).delete();
    }

    // Dropping a borrow-provenance Ptr is a no-op for the owner: the Vec is still alive and
    // readable through a second Ptr afterwards.
    #[test]
    fn dropping_a_borrow_vec_ptr_does_not_disturb_the_owner() {
        let owner: Value<Vec<i32>> = Rc::new(RefCell::new(vec![7, 8, 9]));
        {
            let tmp: Ptr<i32> = Ptr::borrow_vec(&owner);
            tmp.offset(1).write(88);
        } // tmp dropped here
        assert_eq!(Rc::strong_count(&owner), 1);
        assert_eq!(*owner.borrow(), vec![7, 88, 9]);
        assert_eq!(Ptr::borrow_vec(&owner).offset(1).read(), 88);
    }

    #[test]
    fn decay_heap_vec_can_be_freed() {
        let p: Ptr<Vec<i32>> = Ptr::alloc(vec![1, 2, 3]);
        let q = p.decay();
        assert_eq!(q.offset(2).read(), 3);
        q.delete();
    }

    #[test]
    fn decay_heap_array_can_be_freed() {
        let p: Ptr<Box<[i32]>> = Ptr::alloc(vec![1, 2, 3].into_boxed_slice());
        let q = p.decay_array();
        assert_eq!(q.offset(2).read(), 3);
        q.delete();
    }

    #[test]
    #[should_panic(expected = "ub: invalid delete")]
    fn decay_stack_vec_cannot_be_freed() {
        let v: Value<Vec<i32>> = Rc::new(RefCell::new(vec![1, 2, 3]));
        let p: Ptr<Vec<i32>> = v.as_pointer();
        p.decay().delete();
    }

    #[test]
    #[should_panic(expected = "ub: invalid delete")]
    fn decay_stack_array_cannot_be_freed() {
        let v: Value<Box<[i32]>> = Rc::new(RefCell::new(vec![1, 2, 3].into_boxed_slice()));
        let p: Ptr<Box<[i32]>> = (&v as &dyn AsPointer<Box<[i32]>>).as_pointer();
        p.decay_array().delete();
    }

    // Reproduces the converter's refcount emission for `*opt` on an
    // `std::optional<std::vector<char>>` (scratch-fo/probe.refcount.rs:56): the pointee of
    // `as_pointer` is NOT annotated, so this whole chain fails to compile at all while a
    // second inherent `decay` exists (E0034).  It can also FAIL AT RUNTIME rather than
    // merely compile: if `decay` picked the array kind, or dropped `offset`, the reads
    // below land on the wrong element or panic with "ub: invalid decay", and reading
    // element 2 (not 0) makes a zero-offset bug observable.
    #[test]
    fn decay_of_unannotated_as_pointer_resolves_to_vec() {
        let v: Value<Vec<u8>> = Rc::new(RefCell::new(b"abc".to_vec()));
        let p = Value::as_pointer(&v).decay() as Ptr<u8>;
        assert_eq!(p.read(), b'a');
        assert_eq!(p.offset(2).read(), b'c');
        assert!(matches!(p.kind, PtrKind::StackVec(_)));
    }

    #[test]
    fn reinterpreted_unaligned_access_spans_elements() {
        let p: Ptr<u16> = Ptr::alloc_array(vec![0u16; 4].into_boxed_slice());
        let words = p
            .reinterpret_cast::<u8>()
            .offset(2)
            .reinterpret_cast::<u32>();
        words.write(0xAABBCCDD);
        assert_eq!(words.read(), 0xAABBCCDD);
        let halves: Vec<u16> = (0..4).map(|i| p.offset(i).read()).collect();
        assert_eq!(halves, vec![0, 0xCCDD, 0xAABB, 0]);
        p.delete();
    }

    #[test]
    fn reinterpreted_u8_storage_is_read_and_written_bytewise() {
        let p: Ptr<u8> = Ptr::alloc_array(vec![0u8; 16].into_boxed_slice());
        let ints = p.reinterpret_cast::<u32>();
        for i in 0..4 {
            ints.offset(i).write(0x01010101 * (i as u32 + 1));
        }
        assert_eq!(p.offset(5).read(), 0x02);
        assert_eq!(ints.offset(3).read(), 0x04040404);
        // An unaligned view over the same bytes.
        let unaligned = p.offset(1).reinterpret_cast::<u32>();
        assert_eq!(unaligned.read(), 0x02010101);
        p.delete();
    }

    #[test]
    #[should_panic]
    fn reinterpreted_write_past_the_end_panics() {
        let p: Ptr<u32> = Ptr::alloc_array(vec![0u32; 2].into_boxed_slice());
        // A u32 straddling the end of the allocation.
        let q = p
            .reinterpret_cast::<u8>()
            .offset(6)
            .reinterpret_cast::<u32>();
        q.write(1);
    }

    #[test]
    #[should_panic]
    fn reinterpreted_u8_write_past_the_end_panics() {
        let p: Ptr<u8> = Ptr::alloc_array(vec![0u8; 6].into_boxed_slice());
        p.reinterpret_cast::<u32>().offset(1).write(1);
    }

    #[test]
    fn reinterpreted_cast() {
        let p: Ptr<u64> = Ptr::alloc(0x0807060504030201u64);

        // Reinterpreted Ptr views using with/get.
        let bytes: Ptr<u8> = p.reinterpret_cast::<u8>();

        assert_eq!(bytes.read(), 0x01);
        assert_eq!(bytes.offset(3).read(), 0x04);
        assert_eq!(bytes.offset(7).read(), 0x08);

        // Write through original, Ptr reads must see the new data.
        p.write(0xAABBCCDDEEFF1122);
        assert_eq!(bytes.read(), 0x22);
        assert_eq!(bytes.offset(3).read(), 0xEE);
        assert_eq!(bytes.offset(7).read(), 0xAA);

        // Create a second reinterpreted view (u16).
        let words: Ptr<u16> = p.reinterpret_cast::<u16>();

        assert_eq!(words.read(), 0x1122);
        assert_eq!(words.offset(1).read(), 0xEEFF);
        assert_eq!(words.offset(3).read(), 0xAABB);

        // Write through original again. Both views must update.
        p.write(0x0000000000000000);
        assert_eq!(bytes.read(), 0x00);
        assert_eq!(bytes.offset(7).read(), 0x00);
        assert_eq!(words.read(), 0x0000);
        assert_eq!(words.offset(3).read(), 0x0000);

        // Write through byte Ptr, read through word Ptr.
        bytes.write(0xCE);
        bytes.offset(1).write(0xFA);
        assert_eq!(words.read(), 0xFACE);
        assert_eq!(bytes.read(), 0xCE);
        assert_eq!(bytes.offset(3).read(), 0x00);

        // Write through word Ptr, read through byte Ptr.
        words.offset(1).write(0xDEAD);
        assert_eq!(bytes.offset(3).read(), 0xDE);
        assert_eq!(words.offset(1).read(), 0xDEAD);

        // Final state: 0x00000000DEADFACE
        assert_eq!(p.read(), 0x00000000DEADFACE);

        p.delete();
    }
}
