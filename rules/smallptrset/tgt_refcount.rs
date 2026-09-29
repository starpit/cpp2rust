// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// llvm::SmallPtrSet<T, _> -> Vec<T> with a LINEAR SCAN.  This mirrors
// SmallPtrSet's own small mode -- SmallPtrSet.h:529 says "In small mode
// SmallPtrSet uses linear search for the elements" -- and needs only
// `PartialEq`, never `Ord` or `Hash`, so it places no requirement on
// `mlir::Operation *`'s still-unsettled element model.  Uniqueness is enforced
// by the bodies below, not by the container.
//
// t2 (SmallPtrSetImpl) shares t1's model because an upcast is the identity on a
// Vec, and t2 is the key EVERY member ask actually names -- see src.cpp.
//
// The insert-return iterator is `*const T1` in THIS model too, exactly as
// rules/smallset's t2 is: it is produced and dropped at all 23 corpus call
// sites and never dereferenced, so it never needs refcount provenance.  See
// src.cpp for the census and for why begin()/end()/find() are NOT keyed.
//
// ⭐ f6/f7 ARE THE ONLY BODIES THAT DIFFER FROM tgt_unsafe.rs, and only in their
// DECLARED types: they are the two ConstPtrType members and are POINTEE-
// parametrised, so their `T1` is the pointee and the element they compare is a
// C++ `T1 *` -- which is `Ptr<T1>` here and `*const T1` under unsafe.
// `impl<T> PartialEq for Ptr<T>` (libcc2rs/src/rc.rs:189) is unconditional, so
// the cast-free `==` needs no bound on `T1`.  Both bodies are INLINED at the
// call site, where the element and the argument have the SAME Rust type by
// construction (the C++ argument is the container's own element implicitly
// converted to `const T *`), so no cast is correct rather than merely omitted.
// Same soundness argument as rules/smallvector's t6.
//
// Every BODY is a self-contained expression over `std` only -- no libcc2rs item
// appears in any body, and there is no file-level helper (a file-level item in a
// tgt_*.rs is NOT copied into the emitted output, which would leave a bare name
// behind).  The `use libcc2rs::*` below is needed only so that f6/f7's DECLARED
// signatures name `Ptr`; nothing that reaches the emitted output depends on it.
//
// ⛔ MEASURED: pin/cpp-rule-preprocessor DOES run rustc semantic analysis over
// this file, so a declared type that is merely never emitted still has to
// resolve.  Writing `Ptr<T1>` without this import failed the regen with four
// E0425s plus a `src/semantic.rs:260 unresolved access="unknown" in f1` panic
// -- and the panic named f1, which is NOT the broken function.  A regen panic
// naming an innocent key is downstream of the first rustc error; scroll UP.

// ⛔⛔ f1 RETURNS `(Value<..>, Value<bool>)` HERE AND A PLAIN TUPLE UNDER UNSAFE,
// BECAUSE THE TWO MODELS MODEL `std::pair` DIFFERENTLY.  rules/pair's t1 is
// `(T1, T2)` under unsafe but `(Value<T1>, Value<T2>)` here, and its `::second`
// (pair/f1) is `a0.1` in BOTH -- so under refcount the converter then emits a
// `.borrow()` on the result of `.1`.  A plain `(*const T1, bool)` return
// therefore compiles as a rule and fails in the EMITTED code:
//
//   error[E0599]: no method named `borrow` found for type `bool`
//     68 |         .1
//     69 |         .borrow()),
//
// (probe/g272b.crate-refcount/build.err).  ⭐ A rule whose return type is a
// std:: type another module models must be written in THAT MODULE'S MODEL, per
// model -- the two arms of a rule are not each other's `sed`.  This one was found
// only because the probe READS `.second`; a probe that discarded the pair, like
// the 14 of 23 corpus sites that discard it, would have reported a clean pass.
// `Rc`/`RefCell` are in scope in every refcount-model output (`let a:
// Value<proj_Node> = Rc::new(RefCell::new(..))`), so naming them in a body is
// safe here and would NOT be under unsafe.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn t1<T1>() -> Vec<T1> {
    Vec::new()
}

fn t2<T1>() -> Vec<T1> {
    Vec::new()
}

fn t3<T1>() -> *const T1 {
    std::ptr::null()
}

fn f1<T1: PartialEq>(a0: &mut Vec<T1>, a1: T1) -> (Value<*const T1>, Value<bool>) {
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

fn f2<T1>() -> Vec<T1> {
    Vec::new()
}

fn f3<T1: Clone>(a0: Vec<T1>) -> Vec<T1> {
    a0.clone()
}

// initializer_list construction.  The list is de-duplicated, preserving the
// FIRST occurrence, which is what SmallPtrSet's repeated `insert` does.
fn f4<T1: PartialEq>(a0: Vec<T1>) -> Vec<T1> {
    {
        let mut __v: Vec<T1> = Vec::new();
        for __e in a0.into_iter() {
            if !__v.iter().any(|__x| *__x == __e) {
                __v.push(__e);
            }
        }
        __v
    }
}

fn f5<T1: PartialEq>(a0: &mut Vec<T1>, a1: T1) -> bool {
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

fn f6<T1>(a0: &Vec<Ptr<T1>>, a1: Ptr<T1>) -> bool {
    a0.iter().any(|__e| *__e == a1)
}

fn f7<T1>(a0: &Vec<Ptr<T1>>, a1: Ptr<T1>) -> u32 {
    (a0.iter().any(|__e| *__e == a1) as u32)
}

// f8/f9 -- the same two members on a CONST-ELEMENT set.  Under refcount a
// `const T *` and a `T *` share the single `Ptr<T>` model, so these are f6/f7
// verbatim; they exist as separate keys only because the C++ SPELLING differs and
// the key is a string.  See src.cpp's f8 note for the measured `None`.
fn f8<T1>(a0: &Vec<Ptr<T1>>, a1: Ptr<T1>) -> bool {
    a0.iter().any(|__e| *__e == a1)
}

fn f9<T1>(a0: &Vec<Ptr<T1>>, a1: Ptr<T1>) -> u32 {
    (a0.iter().any(|__e| *__e == a1) as u32)
}
