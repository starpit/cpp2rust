// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// llvm::SmallSet<T, _> -> Vec<T> with a LINEAR SCAN.  This mirrors SmallSet's
// own below-N representation (SmallSet.h:136-138, a SmallVector plus a linear
// `vfind`) rather than imposing an order the C++ side does not have, and it
// needs only `PartialEq` -- never `Ord` or `Hash` -- so it places no
// requirement on `mlir::Operation *`'s still-unsettled element model.
// Uniqueness is enforced by the bodies below, not by the container.
//
// The insert-return iterator is `*const T1`: a handle that is produced and
// dropped at every one of the corpus's 8 call sites and never dereferenced.
// See src.cpp for the enumeration and for why begin()/end() are NOT keyed.
//
// Every body is a self-contained expression over `std` only -- no libcc2rs
// item, and no file-level helper (a file-level item in a tgt_*.rs is NOT
// copied into the emitted output, which would leave a bare name behind).

fn t1<T1>() -> Vec<T1> {
    Vec::new()
}

fn t2<T1>() -> *const T1 {
    std::ptr::null()
}

fn f1<T1>(a0: Vec<T1>) -> bool {
    a0.is_empty()
}

fn f2<T1>(a0: Vec<T1>) -> usize {
    a0.len()
}

fn f3<T1: PartialEq>(a0: Vec<T1>, a1: T1) -> usize {
    (a0.iter().any(|__e| *__e == a1) as usize)
}

fn f4<T1: PartialEq>(a0: Vec<T1>, a1: T1) -> bool {
    a0.iter().any(|__e| *__e == a1)
}

fn f5<T1: PartialEq>(a0: &mut Vec<T1>, a1: T1) -> (*const T1, bool) {
    {
        let __v = &mut *a0;
        match __v.iter().position(|__e| *__e == a1) {
            Some(__i) => (&__v[__i] as *const T1, false),
            None => {
                __v.push(a1);
                let __n = __v.len() - 1;
                (&__v[__n] as *const T1, true)
            }
        }
    }
}

fn f6<T1: PartialEq>(a0: &mut Vec<T1>, a1: T1) -> (*const T1, bool) {
    {
        let __v = &mut *a0;
        match __v.iter().position(|__e| *__e == a1) {
            Some(__i) => (&__v[__i] as *const T1, false),
            None => {
                __v.push(a1);
                let __n = __v.len() - 1;
                (&__v[__n] as *const T1, true)
            }
        }
    }
}

fn f7<T1: PartialEq>(a0: &mut Vec<T1>, a1: T1) -> bool {
    {
        let __v = &mut *a0;
        match __v.iter().position(|__e| *__e == a1) {
            Some(__i) => {
                __v.remove(__i);
                true
            }
            None => false,
        }
    }
}

fn f8<T1>(a0: &mut Vec<T1>) {
    a0.clear()
}

fn f9<T1>() -> Vec<T1> {
    Vec::new()
}

fn f10<T1: Clone>(a0: Vec<T1>) -> Vec<T1> {
    a0.clone()
}
