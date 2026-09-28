// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model and for the corpus enumeration that justifies it.
// A DynamicAPInt is its value; every corpus site is small-integer, so the type
// IS i64 and all four bodies are the identity or a plain comparison.  There is
// no tgt_refcount.rs: an i64 is value-like, no body here contains raw-pointer
// text or `as_pointer()`, and placeholder access modes are expanded
// model-awarely, so the unsafe base layer is correct under refcount too.

fn t1() -> i64 {
    0
}

unsafe fn f1() -> i64 {
    0
}

unsafe fn f2(a0: i64) -> i64 {
    a0
}

unsafe fn f3(a0: i64) -> i64 {
    a0
}

unsafe fn f4(a0: i64, a1: i64) -> bool {
    a0 == a1
}
