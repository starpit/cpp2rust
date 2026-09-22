// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Overlays tgt_unsafe.rs: only the rules whose Rust differs in the refcount
// model are repeated here. Those are exactly the rules that name a raw C
// pointer (Ptr<T> instead of *mut T), plus the assignment operators, which
// take their destination as a Ptr in this model.

use libcc2rs::*;

fn f9<T1>(a0: Ptr<T1>) -> Option<Value<T1>> {
    a0.to_owned_opt()
}

fn f12<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: Option<Value<T1>>) {
    a0.write(a1.clone())
}

fn f13<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: &mut Option<Value<T1>>) {
    a0.write(a1.take())
}

fn f14<T1>(a0: Option<Value<T1>>) -> Ptr<T1> {
    a0.as_pointer()
}

// operator-> must yield the pointee itself, not a pointer: the converter
// appends the field access straight onto whatever this rule produces.
fn f15<T1>(a0: &Option<Value<T1>>) -> std::cell::Ref<'_, T1> {
    a0.as_ref().unwrap().borrow()
}

fn f16<T1>(a0: Option<Value<T1>>) -> Ptr<T1> {
    a0.as_pointer()
}

fn f20<T1>(a0: &mut Option<Value<T1>>, a1: Ptr<T1>) {
    let _p: Ptr<_> = a1;
    *a0 = _p.to_owned_opt()
}

// --- std::shared_ptr<T[]> and std::shared_ptr<void> -------------------------
//
// Same shape as tgt_unsafe.rs, with this model's pointer spellings: Ptr<T1>
// for `T1 *` and AnyPtr for `void *`. Both already exist and both are already
// what the converter emits for the raw-pointer forms of those types, so a
// shared_ptr<T[]> and the `T *` that `.get()` hands to memcpy are the same
// Rust type and no conversion sits between them.
//
// AnyPtr is the load-bearing choice. It keeps the element type it was built
// from inside an Rc<dyn ErasedPtr>, so f44's reinterpret_cast::<T1>() gives
// back the ORIGINAL Ptr<T1> when the type matches and a byte-level
// Reinterpreted view when it does not -- which is reinterpret_pointer_cast's
// C++ semantics. Mapping shared_ptr<void> to Option<Value<()>> instead (what
// the generic t1 does) cannot work at all: `()` is zero-sized and
// Ptr::reinterpret_cast panics outright on a zero-sized destination.
//
// f37 forgets the Rc rather than dropping it, for the reason src.cpp gives at
// length: in C++ the frame's shared_ptr<void> is the surviving owner, and
// Ptr::alloc_array has ALREADY leaked one strong reference (that is how
// PtrKind::HeapArray works), so simply not calling delete_array is the leak.
// to_any() only clones the handle, so nothing here can free the buffer.

fn t3<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn t4() -> AnyPtr {
    Ptr::<u8>::null().to_any()
}

fn f34<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f35<T1>() -> Ptr<T1> {
    Ptr::<T1>::null()
}

fn f36<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f37<T1: ByteRepr>(a0: Ptr<T1>) -> AnyPtr {
    a0.to_any()
}

fn f38(a0: AnyPtr) -> AnyPtr {
    a0
}

fn f39() -> AnyPtr {
    Ptr::<u8>::null().to_any()
}

fn f40(a0: AnyPtr) -> bool {
    a0.is_null()
}

fn f41(a0: AnyPtr) -> bool {
    !a0.is_null()
}

fn f42(a0: Ptr<AnyPtr>, a1: AnyPtr) {
    a0.write(a1)
}

fn f43(a0: Ptr<AnyPtr>, a1: AnyPtr) {
    a0.write(a1)
}

// No f44: see src.cpp. AnyPtr::reinterpret_cast would give a perfectly good
// Ptr<T1>, but the C++ result type is shared_ptr<T1>, which t1 maps to the
// OWNING Option<Value<T1>> -- and to_owned_opt() on a Reinterpreted pointer
// panics by design ("Can't own a reinterpreted pointer").
