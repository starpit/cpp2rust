// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::ArrayRef<T> -> `(Ptr<T1>, i64)`: the same (base, length) pair as
// tgt_unsafe.rs, with the refcount model's pointer in place of the raw one.
// That is rules/mlir's t5/t6 convention exactly -- there BaseT is itself a
// `Ptr<_>` -- rather than a third shape.  src.cpp has the survey, the -verbose
// keys, and why initializer_list is left loud.
//
// NOTE, a trap paid for here: `unsafe fn` does NOT supply an unsafe context for
// its own body in this toolchain (Rust 2024 `unsafe_op_in_unsafe_fn`), and
// cpp-rule-preprocessor TYPE-CHECKS target bodies.  A raw-pointer op needs an
// explicit `unsafe { .. }` block; without it the preprocessor dies with
// `_Unwind_Resume` at semantic.rs:261 -- the SAME line as the tuple-receiver
// trap, so the panic site actively misleads.

use libcc2rs::*;

fn t1<T1>() -> (Ptr<T1>, i64) {
    (Default::default(), 0)
}

fn f1<T1>() -> (Ptr<T1>, i64) {
    (Default::default(), 0)
}

// ONE element -> LENGTH 1.  Not 0, and not the length of whatever container the
// element happens to live in -- the classic silent-wrongness trap for this type.
fn f2<T1>(a0: Ptr<T1>) -> (Ptr<T1>, i64) {
    (a0, 1)
}

fn f3<T1>(a0: Ptr<T1>, a1: u64) -> (Ptr<T1>, i64) {
    (a0, a1 as i64)
}

// From a container.  A BORROW: `decay()` turns a pointer AT the Vec into a
// pointer INTO it (libcc2rs/src/rc.rs:921, the same call rules/mlir's f2 uses
// for Block::begin()), so element reads reach the live container and a later
// mutation through it IS visible -- as C++ requires.  This is NOT the
// projection wall rules/mlir/tgt_refcount.rs f14-f17 records: that one is a
// projection through a STRUCT FIELD (Ptr<Block> -> its `ops`), which `decay`
// cannot express.  A pointer at a Vec to a pointer into it is exactly what
// `decay` is for.
//
// `a0` is bound ONCE.  A rule body is inlined TEXTUALLY, so a second `a0` would
// re-evaluate the caller's argument expression; `__p` is one evaluation used
// twice.  `with` takes a shared borrow and releases it inside the same
// expression, so no second guard is live on the receiver.
fn f4<T1: ByteRepr>(a0: Ptr<Vec<T1>>) -> (Ptr<T1>, i64) {
    {
        let __p = a0;
        let __n = __p.with(|__v: &Vec<T1>| __v.len() as i64);
        (__p.decay(), __n)
    }
}

// Field access, not a method call: the trap about a TUPLE receiver
// (semantic.rs:261) applies to method calls, which these are not.
fn f5<T1>(a0: (Ptr<T1>, i64)) -> u64 {
    a0.1 as u64
}

fn f6<T1>(a0: (Ptr<T1>, i64)) -> bool {
    a0.1 == 0
}

// operator[] returns `const T1 &`, i.e. a Ptr in this model.  `Ptr::offset`
// (rc.rs:332) is the refcount counterpart of the raw `.add`.
fn f7<T1: ByteRepr>(a0: (Ptr<T1>, i64), a1: u64) -> Ptr<T1> {
    a0.0.offset(a1)
}
