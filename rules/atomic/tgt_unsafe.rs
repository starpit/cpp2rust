// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1() -> std::sync::atomic::AtomicBool {
    std::sync::atomic::AtomicBool::new(false)
}

fn t2() -> std::sync::atomic::Ordering {
    std::sync::atomic::Ordering::SeqCst
}

unsafe fn f1() -> std::sync::atomic::Ordering {
    std::sync::atomic::Ordering::Relaxed
}

unsafe fn f2() -> std::sync::atomic::Ordering {
    std::sync::atomic::Ordering::Acquire
}

unsafe fn f3() -> std::sync::atomic::Ordering {
    std::sync::atomic::Ordering::Acquire
}

unsafe fn f4() -> std::sync::atomic::Ordering {
    std::sync::atomic::Ordering::Release
}

unsafe fn f5() -> std::sync::atomic::Ordering {
    std::sync::atomic::Ordering::AcqRel
}

unsafe fn f6() -> std::sync::atomic::Ordering {
    std::sync::atomic::Ordering::SeqCst
}

unsafe fn f7(a0: *const std::sync::atomic::AtomicBool, a1: std::sync::atomic::Ordering) -> bool {
    unsafe { std::sync::atomic::AtomicBool::load(&*a0, a1) }
}

unsafe fn f8(a0: *mut std::sync::atomic::AtomicBool, a1: bool, a2: std::sync::atomic::Ordering) {
    unsafe { std::sync::atomic::AtomicBool::store(&*a0, a1, a2) }
}

unsafe fn f9(a0: *mut std::sync::atomic::AtomicBool, a1: bool, a2: std::sync::atomic::Ordering) -> bool {
    unsafe { std::sync::atomic::AtomicBool::swap(&*a0, a1, a2) }
}

fn t3() -> std::sync::atomic::AtomicBool {
    std::sync::atomic::AtomicBool::new(false)
}

unsafe fn f10() -> std::sync::atomic::AtomicBool {
    std::sync::atomic::AtomicBool::new(false)
}

unsafe fn f11(a0: bool) -> std::sync::atomic::AtomicBool {
    std::sync::atomic::AtomicBool::new(a0)
}

fn t4() -> std::sync::atomic::AtomicU64 {
    std::sync::atomic::AtomicU64::new(0)
}

fn t5() -> std::sync::atomic::AtomicU64 {
    std::sync::atomic::AtomicU64::new(0)
}

unsafe fn f12() -> std::sync::atomic::AtomicU64 {
    std::sync::atomic::AtomicU64::new(0)
}

unsafe fn f13(a0: u64) -> std::sync::atomic::AtomicU64 {
    std::sync::atomic::AtomicU64::new(a0)
}

unsafe fn f14(a0: *const std::sync::atomic::AtomicU64, a1: std::sync::atomic::Ordering) -> u64 {
    unsafe { std::sync::atomic::AtomicU64::load(&*a0, a1) }
}

unsafe fn f15(a0: *mut std::sync::atomic::AtomicU64, a1: u64, a2: std::sync::atomic::Ordering) {
    unsafe { std::sync::atomic::AtomicU64::store(&*a0, a1, a2) }
}

unsafe fn f16(a0: *mut std::sync::atomic::AtomicU64, a1: u64, a2: std::sync::atomic::Ordering) -> u64 {
    unsafe { std::sync::atomic::AtomicU64::swap(&*a0, a1, a2) }
}

fn t6() -> std::sync::atomic::AtomicI32 {
    std::sync::atomic::AtomicI32::new(0)
}

fn t7() -> std::sync::atomic::AtomicI32 {
    std::sync::atomic::AtomicI32::new(0)
}

fn t8() -> std::sync::atomic::AtomicI32 {
    std::sync::atomic::AtomicI32::new(0)
}

unsafe fn f17() -> std::sync::atomic::AtomicI32 {
    std::sync::atomic::AtomicI32::new(0)
}

unsafe fn f18(a0: i32) -> std::sync::atomic::AtomicI32 {
    std::sync::atomic::AtomicI32::new(a0)
}

unsafe fn f19(a0: *mut std::sync::atomic::AtomicI32) -> i32 {
    unsafe {
        std::sync::atomic::AtomicI32::fetch_add(&*a0, 1, std::sync::atomic::Ordering::SeqCst)
            .wrapping_add(1)
    }
}

fn t9() -> std::sync::atomic::AtomicI64 {
    std::sync::atomic::AtomicI64::new(0)
}

fn t10() -> std::sync::atomic::AtomicI64 {
    std::sync::atomic::AtomicI64::new(0)
}

fn t11() -> std::sync::atomic::AtomicI64 {
    std::sync::atomic::AtomicI64::new(0)
}

unsafe fn f20() -> std::sync::atomic::AtomicI64 {
    std::sync::atomic::AtomicI64::new(0)
}

unsafe fn f21(a0: i64) -> std::sync::atomic::AtomicI64 {
    std::sync::atomic::AtomicI64::new(a0)
}

unsafe fn f22(a0: *mut std::sync::atomic::AtomicI64) -> i64 {
    unsafe {
        std::sync::atomic::AtomicI64::fetch_sub(&*a0, 1, std::sync::atomic::Ordering::SeqCst)
            .wrapping_sub(1)
    }
}
