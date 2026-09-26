// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model and for what is deliberately not covered.  A
// DynamicAPInt is the int64_t its own fast-path union member is, so
// construction from an int64_t and the copy constructor are the IDENTITY and
// == / != are Rust's, which is exactly LLVM's `getSmall() == O.getSmall()`.

fn t1() -> i64 {
    0_i64
}

unsafe fn f1(a0: i64) -> i64 {
    a0
}

unsafe fn f2(a0: i64) -> i64 {
    a0
}

unsafe fn f3(a0: i64, a1: i64) -> bool {
    a0 == a1
}

unsafe fn f4(a0: i64, a1: i64) -> bool {
    a0 != a1
}

// The int64_t mixed comparisons (see src.cpp).  Comparison cannot overflow, so
// these need no widening decision; arithmetic still does, and still aborts.

unsafe fn f5(a0: i64, a1: i64) -> bool {
    a0 == a1
}

unsafe fn f6(a0: i64, a1: i64) -> bool {
    a0 != a1
}
