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
// The insert-return iterator is `*const T1`: produced and dropped at all 23
// corpus call sites, never dereferenced.  See src.cpp for the census and for why
// begin()/end()/find() are NOT keyed.
//
// ⛔⛔ f6/f7 NEED THE `as *const T1` ON THE ELEMENT, AND THAT IS MEASURED, NOT
// STYLE.  They are the two ConstPtrType members and are POINTEE-parametrised, so
// their `T1` is the pointee: the CONTAINER element is a C++ `T1 *` -> `*mut T1`,
// while the ARGUMENT is `const T1 *` -> `*const T1`.  A cast-free `*__e == a1`
// looks right and is WRONG -- the converter emits `.cast_const()` on the argument
// at the call site, so the comparison is `*mut T` vs `*const T`:
//
//   error[E0308]: mismatched types ... types differ in mutability
//     .any(|__e| *__e == (&mut a as *mut proj_Node).cast_const()) as u8),
//     = note: expected raw pointer `*mut proj_Node`
//                found raw pointer `*const proj_Node`
//
// (4 occurrences, probe/g272b.crate-unsafe/build.err).  I wrote the cast-free
// form first, on the reasoning that the declared types are never emitted because
// the body is INLINED.  That reasoning is true and IRRELEVANT: what is emitted is
// the BODY, so an operator the body applies to the element has to be valid for
// the element type the CALL SITE has, not for the one declared here.  ⭐ `as
// *const T1` is the one spelling valid for BOTH a `*mut T` and a `*const T`
// element, which `.cast_const()` (inherent on `*mut T` only) is not.
//
// The refcount arm needs NO cast: there the element and the argument are both
// `Ptr<T1>` and the converter emits no conversion.  See tgt_refcount.rs.
//
// Every body is a self-contained expression over `std` only -- no libcc2rs item,
// and no file-level helper (a file-level item in a tgt_*.rs is NOT copied into
// the emitted output, which would leave a bare name behind).

fn t1<T1>() -> Vec<T1> {
    Vec::new()
}

fn t2<T1>() -> Vec<T1> {
    Vec::new()
}

fn t3<T1>() -> *const T1 {
    std::ptr::null()
}

unsafe fn f1<T1: PartialEq>(a0: &mut Vec<T1>, a1: T1) -> (*const T1, bool) {
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

unsafe fn f2<T1>() -> Vec<T1> {
    Vec::new()
}

unsafe fn f3<T1: Clone>(a0: Vec<T1>) -> Vec<T1> {
    a0.clone()
}

// initializer_list construction.  The list is de-duplicated, preserving the
// FIRST occurrence, which is what SmallPtrSet's repeated `insert` does.
unsafe fn f4<T1: PartialEq>(a0: Vec<T1>) -> Vec<T1> {
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

unsafe fn f5<T1: PartialEq>(a0: &mut Vec<T1>, a1: T1) -> bool {
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

unsafe fn f6<T1>(a0: &Vec<*mut T1>, a1: *const T1) -> bool {
    a0.iter().any(|__e| *__e as *const T1 == a1)
}

unsafe fn f7<T1>(a0: &Vec<*mut T1>, a1: *const T1) -> u32 {
    (a0.iter().any(|__e| *__e as *const T1 == a1) as u32)
}

// f8/f9 -- the same two members on a CONST-ELEMENT set, where the element is
// already `*const T1`.  The `as *const T1` is kept deliberately: it is a no-op
// here and is the one spelling that stays valid whichever of the two element
// mutabilities the call site turns out to have.
unsafe fn f8<T1>(a0: &Vec<*const T1>, a1: *const T1) -> bool {
    a0.iter().any(|__e| *__e as *const T1 == a1)
}

unsafe fn f9<T1>(a0: &Vec<*const T1>, a1: *const T1) -> u32 {
    (a0.iter().any(|__e| *__e as *const T1 == a1) as u32)
}
