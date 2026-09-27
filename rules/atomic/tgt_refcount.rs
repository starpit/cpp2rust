// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// REFCOUNT model for std::atomic<T>.
//
// WHY THIS FILE MUST EXIST.  Every operation key in this module has a C++
// REFERENCE receiver (`const std::atomic<T> &` for load, `std::atomic<T> &` for
// store/exchange/operator++/operator--).  tgt_unsafe.rs spells those receivers
// as raw `*const`/`*mut`, and omitting tgt_refcount.rs does NOT drop the keys
// from the refcount model -- the converter silently FALLS BACK to the unsafe
// body.  In refcount a reference receiver arrives as `Ptr<T>`, so the fallback
// emits `Ptr<Atomic..> as *mut Atomic..` and every one of these keys fails to
// compile with
//     error[E0605]: non-primitive cast: `Ptr<Atomic{I32,I64,U64,Bool}>` as
//                   `*mut Atomic{...}`
// -- measured on scratch-atomic/probe2.cpp (int/long ++/--) and probe3.cpp
// (the long-committed bool load/store path).  The breakage was module-wide and
// invisible because every probe so far had only exercised the unsafe model.
//
// Rust's atomics take `&self` for load/store/swap/fetch_*, so `Ptr::with` (an
// immutable RefCell borrow) is the right accessor even for the `&`-receiver
// keys; that also avoids the refcount `RefCell already borrowed` panic that a
// `with_mut` would risk if a caller mentions the receiver twice in one
// expression.
//
// The memory ORDER is threaded through exactly as the C++ wrote it (a0/a1/a2),
// never widened or narrowed -- see src.cpp on why this module models real
// atomics rather than Cell<T>.

use libcc2rs::*;

// --- std::atomic<bool> ------------------------------------------------------

fn f7(a0: Ptr<std::sync::atomic::AtomicBool>, a1: std::sync::atomic::Ordering) -> bool {
    a0.with(|__a: &std::sync::atomic::AtomicBool| std::sync::atomic::AtomicBool::load(__a, a1))
}

fn f8(a0: Ptr<std::sync::atomic::AtomicBool>, a1: bool, a2: std::sync::atomic::Ordering) {
    a0.with(|__a: &std::sync::atomic::AtomicBool| {
        std::sync::atomic::AtomicBool::store(__a, a1, a2)
    })
}

fn f9(a0: Ptr<std::sync::atomic::AtomicBool>, a1: bool, a2: std::sync::atomic::Ordering) -> bool {
    a0.with(|__a: &std::sync::atomic::AtomicBool| {
        std::sync::atomic::AtomicBool::swap(__a, a1, a2)
    })
}

// --- std::atomic<unsigned long> --------------------------------------------

fn f14(a0: Ptr<std::sync::atomic::AtomicU64>, a1: std::sync::atomic::Ordering) -> u64 {
    a0.with(|__a: &std::sync::atomic::AtomicU64| std::sync::atomic::AtomicU64::load(__a, a1))
}

fn f15(a0: Ptr<std::sync::atomic::AtomicU64>, a1: u64, a2: std::sync::atomic::Ordering) {
    a0.with(|__a: &std::sync::atomic::AtomicU64| {
        std::sync::atomic::AtomicU64::store(__a, a1, a2)
    })
}

fn f16(a0: Ptr<std::sync::atomic::AtomicU64>, a1: u64, a2: std::sync::atomic::Ordering) -> u64 {
    a0.with(|__a: &std::sync::atomic::AtomicU64| std::sync::atomic::AtomicU64::swap(__a, a1, a2))
}

// --- std::atomic<int> / std::atomic<long>: PREFIX ++ and -- -----------------
//
// PREFIX, so the value yielded is the NEW one: fetch_add returns the OLD value,
// hence the wrapping_add(1) / wrapping_sub(1).  A body that returned the
// fetch_* result directly, or that was shaped like a postfix operator, gives
// a=7 c=100 instead of a=8 c=99 on probe2 -- that is the discriminator.

fn f19(a0: Ptr<std::sync::atomic::AtomicI32>) -> i32 {
    a0.with(|__a: &std::sync::atomic::AtomicI32| {
        std::sync::atomic::AtomicI32::fetch_add(__a, 1, std::sync::atomic::Ordering::SeqCst)
            .wrapping_add(1)
    })
}

fn f22(a0: Ptr<std::sync::atomic::AtomicI64>) -> i64 {
    a0.with(|__a: &std::sync::atomic::AtomicI64| {
        std::sync::atomic::AtomicI64::fetch_sub(__a, 1, std::sync::atomic::Ordering::SeqCst)
            .wrapping_sub(1)
    })
}
