// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Refcount model for llvm::SetVector.  The MODEL is unchanged -- a `Vec<T1>` in
// insertion order -- only the reference shapes differ, exactly as in
// rules/smallvector's refcount target, which this mirrors key for key:
//   * a MUTABLE container receiver becomes `Ptr<Vec<T1>>` and is written through
//     `with_mut` (smallvector f1);
//   * a `begin()`/`end()` receiver becomes `Ptr<T1>` -- the mapper's
//     reference-type derivation collapses the container away here -- and the
//     bodies are `a0` and `a0.to_end()` (smallvector f8/f9);
//   * a CONST container receiver stays a plain value/borrow (smallvector f20).
//
// t1 and t2 are RESTATED rather than omitted.  Their bodies are identical to the
// unsafe ones, but a module that carries a tgt_refcount.rs at all is safer
// restating every type key than relying on the loader's union.

use libcc2rs::*;

fn t1<T1, T2, T3>() -> Vec<T1> {
    Default::default()
}

fn t2<T1>() -> Vec<T1> {
    Default::default()
}

fn f1<T1>() -> Vec<T1> {
    Vec::new()
}

fn f2<T1: PartialEq + ByteRepr, T2, T3>(a0: Ptr<Vec<T1>>, a1: T1) -> bool {
    a0.with_mut(|__v: &mut Vec<T1>| {
        let __x = a1;
        if __v.contains(&__x) {
            false
        } else {
            __v.push(__x);
            true
        }
    })
}

fn f3<T1: PartialEq, T2, T3>(a0: Vec<T1>, a1: T1) -> bool {
    a0.contains(&a1)
}

fn f4<T1, T2, T3>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f5<T1, T2, T3>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}
