// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp and tgt_unsafe.rs.  THE POSITION HALF ONLY; operator* has no rule
// and stays loud.  The representation is `Ptr<T1>`, the refcount model's checked
// stand-in for a raw pointer -- the same one rules/vector gives a contiguous
// iterator, and it has the same `prefix_inc` (rc.rs:814).
use libcc2rs::*;

fn t1<T1, T2, T3>() -> Ptr<T1> {
    Ptr::null()
}

fn f1<T1, T2, T3>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f2<T1, T2, T3>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f3<T1, T2, T3>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

// The TWO-ARGUMENT CONSTRUCTOR -- LOSSY, exactly as in tgt_unsafe.rs: the
// position is kept, the callable is dropped, and operator* therefore stays loud.
fn f4<T1, T2, T3>(a0: Ptr<T1>, a1: fn(T2) -> T3) -> Ptr<T1> {
    a0
}
