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
// ⭐ g3099: `Value` appears in f5/f6's DECLARED signatures only (it is
// `Rc<RefCell<T>>`, libcc2rs/src/rc.rs:16), so the `use libcc2rs::*` below is
// needed purely so the preprocessor's rustc pass can RESOLVE that name -- the
// `Rc`/`RefCell` the bodies name are `std` and are in scope in every
// refcount-model emission.  This mirrors the measured note at
// `rules/smallptrset/tgt_refcount.rs:36-41`: omitting the import there failed
// the regen with four E0425s plus a `semantic.rs:260` panic naming an INNOCENT
// key.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

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

// ⛔⛔ g3099 -- THE RETURN IS `(Value<*const T1>, Value<bool>)`, NOT
// `(*const T1, bool)`.  `src.cpp`'s f5/f6 return
// `std::pair<llvm::SmallSet<T1,4>::const_iterator, bool>` -- an ordinary
// `std::pair`, modelled by **rules/pair**, whose `t1` is `(T1, T2)` under unsafe
// but `(Value<T1>, Value<T2>)` under refcount while its `::second` (pair/f1) is
// `a0.1` in BOTH.  So under refcount the converter emits a `.borrow()` on `.1`
// and a plain `bool` there compiles as a RULE and fails in the EMITTED code:
//   error[E0599]: no method named `borrow` found for type `bool`
// ⭐⭐ THE DISCRIMINATOR IS THIS MODULE'S DIRECT SIBLING: `(*const T1, bool)` is
// EXACTLY `rules/smallptrset` f1's PRE-FIX signature, and that file's note
// (`tgt_refcount.rs:43-58`) records the measured failure and the fix.  Second
// landed precedent: `rules/unordered_map` f57, commit e17a894c (2 sites on
// sys-arch-spec/isa/isa.cpp, 2 E0599 -> 0, zero new errors).
// ⭐⭐ REDISCOVERED THREE TIMES; THIS NOTE IS SO THAT IS THE LAST TIME.  The
// reason the error count for this key can read 0 is that a call site which
// DISCARDS the returned pair reports a CLEAN PASS -- and this module's own
// header note records that all 8 corpus sites drop the iterator handle, so THIS
// KEY HAS NO ERROR-DRIVEN WITNESS AND IS FIXED ON SHAPE/PRECEDENT GROUNDS.
// Both elements are boxed because rules/pair boxes both uniformly.
// ⛔ The UNSAFE arm's plain `(*const T1, bool)` is CORRECT and must stay.
fn f5<T1: PartialEq>(a0: &mut Vec<T1>, a1: T1) -> (Value<*const T1>, Value<bool>) {
    {
        let __v = &mut *a0;
        match __v.iter().position(|__e| *__e == a1) {
            Some(__i) => (
                Rc::new(RefCell::new(&__v[__i] as *const T1)),
                Rc::new(RefCell::new(false)),
            ),
            None => {
                __v.push(a1);
                let __n = __v.len() - 1;
                (
                    Rc::new(RefCell::new(&__v[__n] as *const T1)),
                    Rc::new(RefCell::new(true)),
                )
            }
        }
    }
}

// g3099 -- same fix as f5; see the note there.
fn f6<T1: PartialEq>(a0: &mut Vec<T1>, a1: T1) -> (Value<*const T1>, Value<bool>) {
    {
        let __v = &mut *a0;
        match __v.iter().position(|__e| *__e == a1) {
            Some(__i) => (
                Rc::new(RefCell::new(&__v[__i] as *const T1)),
                Rc::new(RefCell::new(false)),
            ),
            None => {
                __v.push(a1);
                let __n = __v.len() - 1;
                (
                    Rc::new(RefCell::new(&__v[__n] as *const T1)),
                    Rc::new(RefCell::new(true)),
                )
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
