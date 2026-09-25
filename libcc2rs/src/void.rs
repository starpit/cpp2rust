// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use std::rc::Rc;

use crate::rc::Ptr;
use crate::reinterpret::ByteRepr;

pub(crate) trait ErasedPtr: std::any::Any {
    fn as_bytes(&self) -> Ptr<u8>;
    fn as_any(&self) -> &dyn std::any::Any;
    fn equals(&self, other: &dyn ErasedPtr) -> bool;
    fn is_null(&self) -> bool;
    // The identity of the ORIGINAL typed pointer, without reinterpreting it.
    // See `AnyPtr::to_int` for why this may not go through `as_bytes`.
    fn identity(&self) -> usize;
}

impl PartialEq for dyn ErasedPtr {
    fn eq(&self, other: &Self) -> bool {
        self.equals(other)
    }
}

impl<T> ErasedPtr for Ptr<T>
where
    T: ByteRepr + 'static,
    Ptr<T>: PartialEq,
{
    fn as_bytes(&self) -> Ptr<u8> {
        self.reinterpret_cast::<u8>()
    }

    fn as_any(&self) -> &dyn std::any::Any {
        self
    }

    fn equals(&self, other: &dyn ErasedPtr) -> bool {
        other.as_any().downcast_ref::<Ptr<T>>() == Some(self)
    }

    fn is_null(&self) -> bool {
        Ptr::is_null(self)
    }

    fn identity(&self) -> usize {
        Ptr::cc2_identity(self)
    }
}

#[derive(Clone)]
pub struct AnyPtr {
    pub(crate) ptr: Rc<dyn ErasedPtr>,
}

impl<T: ByteRepr + 'static> Ptr<T> {
    pub fn to_any(&self) -> AnyPtr {
        AnyPtr {
            ptr: Rc::new(self.clone()),
        }
    }
}

impl Default for AnyPtr {
    fn default() -> Self {
        Ptr::<()>::null().to_any()
    }
}

impl AnyPtr {
    pub fn reinterpret_cast<T: ByteRepr>(&self) -> Ptr<T> {
        if self.ptr.is_null() {
            return Ptr::<T>::null();
        }
        if let Some(p) = self.ptr.as_any().downcast_ref::<Ptr<T>>() {
            return p.clone();
        }
        self.ptr.as_bytes().reinterpret_cast::<T>()
    }

    pub fn is_null(&self) -> bool {
        self.ptr.is_null()
    }
}

impl PartialEq for AnyPtr {
    fn eq(&self, other: &Self) -> bool {
        *self.ptr == *other.ptr
    }
}

impl AnyPtr {
    pub fn memcpy(&self, src: &AnyPtr, len: usize) {
        let dst_u8 = self.ptr.as_bytes();
        let src_u8 = src.ptr.as_bytes();
        dst_u8.memcpy(&src_u8, len);
    }

    pub fn memset(&self, value: u8, num: usize) {
        self.ptr.as_bytes().memset(value, num);
    }

    pub fn memcmp(&self, other: &AnyPtr, len: usize) -> i32 {
        let a = self.ptr.as_bytes();
        let b = other.ptr.as_bytes();
        a.memcmp(&b, len)
    }
}

impl ByteRepr for AnyPtr {}

impl AnyPtr {
    /// The pointer's numeric identity -- what a `void *` -> integer cast, and
    /// `operator<<(const void *)`, observe.
    ///
    /// This deliberately does NOT go through `reinterpret_cast::<u8>().to_int()`,
    /// which it used to, for two independent reasons:
    ///
    ///  * `Ptr::<T>::to_int` (rc.rs) is `with_scratch(Self::byte_size(), ..)`,
    ///    and `impl ByteRepr for Ptr<T>` is a MARKER impl, so every method is the
    ///    trait's panicking default.  It therefore panicked unconditionally with
    ///    "byte_size is not implemented for libcc2rs::rc::Ptr<u8>" -- there is no
    ///    input for which the old body returned.  A pointer has no byte
    ///    serialization in this model, and it does not need one to have an
    ///    identity.
    ///  * even with a `byte_size`, reinterpreting is the wrong route: for a
    ///    `Ptr` of a type other than `u8`, `reinterpret_cast` allocates a FRESH
    ///    `Rc<ReinterpretedData>` per call, and `cc2_identity` reads
    ///    `Rc::as_ptr` of it, so two casts of the same pointer would yield two
    ///    different numbers.  Asking the erased `Ptr<T>` directly keeps the
    ///    identity stable across calls and equal to what `Ptr::<T>::cc2_addr`
    ///    reports for the same pointer -- so a `void *` and the typed pointer it
    ///    was converted from print alike, as C++ requires.
    ///
    /// Null is 0, because `Ptr::cc2_identity` returns 0 for `PtrKind::Null`.
    pub fn to_int(&self) -> usize {
        self.ptr.identity()
    }

    // NOT the inverse of `to_int`, and deliberately left alone.  `Ptr::from_int`
    // is `Self::from_bytes(..)` on the same marker `ByteRepr for Ptr<T>` impl, so
    // this panics unconditionally too ("byte_size is not implemented for
    // libcc2rs::rc::Ptr<u8>"), and unlike `to_int` that is not a fixable
    // oversight: an arbitrary integer carries no allocation for a `Ptr` to borrow
    // from, so the refcount model genuinely cannot manufacture one.  The loud
    // panic is the right answer; giving it a body would be inventing provenance.
    //
    // The parameter also stays `usize`: the converter spells integer literals
    // `u64`, so an int-to-pointer cast emits `AnyPtr::from_int(N_u64)` and fails
    // E0308 here.  That mismatch is converter-side and not specific to `AnyPtr`
    // (`Ptr::from_int` takes `usize` as well); widening this one signature to
    // `u64` would only move a compile error to the panic above.
    pub fn from_int(value: usize) -> Self {
        Ptr::<u8>::from_int(value).to_any()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn anyptr_null_cast() {
        // void* nullptr
        let any = Ptr::<()>::null().to_any();
        let p = any.reinterpret_cast::<u32>();
        assert!(p.is_null());

        let p2 = any.reinterpret_cast::<u8>();
        assert!(p2.is_null());

        // int* nullptr
        let any2 = Ptr::<i32>::null().to_any();
        let p3 = any2.reinterpret_cast::<f32>();
        assert!(p3.is_null());
    }

    // The properties `operator<<(const void *)` and `(uintptr_t)p` depend on.
    // Before the fix every one of these panicked instead of returning.
    #[test]
    fn anyptr_to_int_identity() {
        use crate::rc::AsPointer;
        use crate::Value;
        use std::cell::RefCell;

        assert_eq!(Ptr::<()>::null().to_any().to_int(), 0);

        let va: Value<i32> = Rc::new(RefCell::new(1));
        let vb: Value<i32> = Rc::new(RefCell::new(2));
        let pa: Ptr<i32> = va.as_pointer();
        let pb: Ptr<i32> = vb.as_pointer();

        // Stable across calls, and across independent AnyPtr wrappers of the
        // same pointer -- the property a reinterpret-based body could not give.
        assert_eq!(pa.to_any().to_int(), pa.to_any().to_int());
        assert_eq!(pa.to_any().to_int(), pa.clone().to_any().to_int());
        // Distinct objects are distinct, and neither looks like null.
        assert_ne!(pa.to_any().to_int(), pb.to_any().to_int());
        assert_ne!(pa.to_any().to_int(), 0);
        // A void* agrees with the typed pointer it came from.
        assert_eq!(pa.to_any().to_int(), pa.cc2_identity());
        // Pointer arithmetic is visible, as in C++.
        assert_ne!(pa.to_any().to_int(), pa.offset(1).to_any().to_int());
    }

    #[test]
    fn to_any_without_clone() {
        let p: Ptr<std::fs::File> = Ptr::null(); // std::fs::File is not Clone
        let any = p.to_any();
        let recovered = any.reinterpret_cast::<std::fs::File>();
        assert!(recovered.is_null());
    }
}
