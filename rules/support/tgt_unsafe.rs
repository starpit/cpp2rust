// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model.  llvm::LogicalResult is its single `bool
// IsSuccess` field and llvm::hash_code is its single `size_t value` field, so
// every body for those two is arithmetic on a scalar and nothing is
// model-dependent.  The FailureOr block at the bottom DOES diverge between the
// two files -- see there.

fn t1() -> bool {
    false
}

fn t2() -> u64 {
    0
}

unsafe fn f1(a0: bool) -> bool {
    a0
}

unsafe fn f2(a0: bool) -> bool {
    !a0
}

unsafe fn f3(a0: bool) -> bool {
    a0
}

unsafe fn f4(a0: bool) -> bool {
    !a0
}

unsafe fn f5(a0: bool) -> bool {
    a0
}

unsafe fn f6(a0: bool) -> bool {
    !a0
}

unsafe fn f7(a0: bool) -> bool {
    a0
}

unsafe fn f8(a0: bool) -> bool {
    !a0
}

unsafe fn f9(a0: bool) -> bool {
    a0
}

unsafe fn f10(a0: u64) -> u64 {
    a0
}

unsafe fn f11(a0: u64) -> u64 {
    a0
}

unsafe fn f12(a0: u64, a1: u64) -> bool {
    a0 == a1
}

unsafe fn f13(a0: u64, a1: u64) -> bool {
    a0 != a1
}

unsafe fn f14(a0: u64) -> u64 {
    a0
}

unsafe fn f15(a0: u64) -> u64 {
    a0
}

// --- llvm::FailureOr<T1> -> Option<T1> -------------------------------------
// See src.cpp: the std::optional base is DROPPED and every reader is an own
// member, so no derived-to-base conversion is generated.  THIS FILE AND
// tgt_refcount.rs NOW DIVERGE for these keys: the refcount model must store the
// payload as a PLACE (`Option<Value<T1>>`, exactly as rules/optional does) so a
// reader can hand back a `Ptr<T1>` aliasing it.

fn t3<T1>() -> Option<T1> {
    None
}

unsafe fn f16<T1>() -> Option<T1> {
    None
}

unsafe fn f17<T1>(a0: bool) -> Option<T1> {
    None
}

unsafe fn f18<T1>(a0: T1) -> Option<T1> {
    Some(a0)
}

unsafe fn f19<T1: Clone>(a0: &T1) -> Option<T1> {
    Some(a0.clone())
}

unsafe fn f20<T1: Clone>(a0: &Option<T1>) -> Option<T1> {
    a0.clone()
}

unsafe fn f21<T1>(a0: &Option<T1>) -> bool {
    a0.is_some()
}
