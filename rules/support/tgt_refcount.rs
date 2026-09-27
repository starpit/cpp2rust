// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model.  llvm::LogicalResult is its single `bool
// IsSuccess` field and llvm::hash_code is its single `size_t value` field, so
// every body here is arithmetic on a scalar and nothing is model-dependent --
// tgt_refcount.rs is byte-identical on purpose.

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
