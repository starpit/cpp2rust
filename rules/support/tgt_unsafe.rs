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

// See src.cpp: `[[noreturn]]` -- the body DIVERGES and yields no initializer.
// The message is a NUL-terminated C string behind a raw pointer, so it has to
// be read as one; `{:?}` on the pointer would print an address.
unsafe fn f22(a0: *const u8, a1: *const u8, a2: u32) {
    panic!(
        "llvm_unreachable: {} at {}:{}",
        std::ffi::CStr::from_ptr(a0 as *const libc::c_char).to_string_lossy(),
        std::ffi::CStr::from_ptr(a1 as *const libc::c_char).to_string_lossy(),
        a2
    )
}

// t4 = llvm::ParseResult.  It has no state of its own -- its single data member
// is LogicalResult's inherited `bool IsSuccess` -- so it is the same scalar as
// t1 in both models.
fn t4() -> bool {
    false
}

// The converting ParseResult(LogicalResult) ctor: identity on the scalar.
unsafe fn f23(a0: bool) -> bool {
    a0
}

// ⭐ INVERTED ON PURPOSE.  `ParseResult::operator bool()` returns `failed()`,
// not `succeeded()`.  `a0` here would compile and would invert every
// `if (parser.parseX())` in the corpus.
unsafe fn f24(a0: bool) -> bool {
    !a0
}
