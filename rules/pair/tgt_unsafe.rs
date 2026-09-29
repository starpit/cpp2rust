// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1<T1: Default, T2: Default>() -> (T1, T2) {
    <(T1, T2)>::default()
}

unsafe fn f1<T1, T2>(a0: (T1, T2)) -> T2 {
    a0.1
}
unsafe fn f2<T1: Clone, T2: Clone>(a0: (T1, T2)) -> (T1, T2) {
    a0.clone()
}
unsafe fn f4<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}
unsafe fn f5<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}
unsafe fn f6<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}
unsafe fn f7<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}
unsafe fn f9<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}
unsafe fn f10<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}

unsafe fn f11<T1, T2>(a0: (T1, T2)) -> T1 {
    a0.0
}

unsafe fn f12<T1: Default, T2: Default>(a0: &mut (T1, T2)) -> (T1, T2) {
    std::mem::take(&mut *a0)
}

// ===========================================================================
// ⛔⛔ f13/f14 -- WHY `drop(core::mem::replace(a0, ..))` AND NOT `*a0 = ..`,
// 2026-09-28.  Same swallowed-deref class as rules/mlir (0a14e018).
// `*a0 = a1.clone()` and `*a0 = std::mem::take(&mut *a1)` WERE HERE AND THEY
// EMITTED RUST THAT DOES NOT COMPILE.  A placeholder whose rule parameter is
// `&mut T` and which sits in a NON-RECEIVER position is emitted by the converter
// as `&mut <place>` -- converter.cpp:8030-8038, `needs_lvalue() &&
// needs_mut_borrow()` returns `"&mut " + place`, gated on
// `needs_explicit_mut_borrow = !is_method_call_receiver && ParamIsMutRef(..)`.
// The rule's leading `*` is SWALLOWED by the preprocessor, so the body emitted
// `&mut <pair-lvalue> = ..;` -- `error[E0070]: invalid left-hand side of
// assignment`.
// ⭐ THE CLASS rc=0 CANNOT SEE: E0070 is a HIR/type-check error, not a parse
// error.  `rustfmt` -- the only Rust parser in this pipeline -- parses
// `&mut x = v` at rc=0 and re-emits it verbatim, so the TU reports clean.
// ⭐ THE FIX RELIES ON THE SAME RENDERING: bare `a0` records the IDENTICAL
// `{arg 0, access: "borrow_mut"}`, so this emits `replace(&mut <place>, ..)`,
// which is exactly what `core::mem::replace` wants.  The dropped return value is
// the C++ copy/move-assignment destroying the overwritten pair.
// ⚠️ DO NOT "restore" the `*`: both `replace(&mut *a0, ..)` and
// `replace(*a0, ..)` make rule-preprocessor ABORT with
// `semantic.rs:260: unresolved access="unknown"` -- a deref or re-borrow of a
// placeholder inside a CALL ARGUMENT has no access classification.
// ⚠️ TYPE CHECK, because this differs from the mlir sites: `core::mem::replace`
// needs only `Sized`, and `(T1, T2)` with implicitly-Sized generics satisfies
// that with NO added bounds -- the existing `Clone` / `Default` bounds are still
// exactly what the bodies use.
// ⚠️ REFCOUNT NEEDS NO CHANGE and deliberately diverges: there `a0` is
// `Ptr<(Value<T1>, Value<T2>)>` and the bodies go through `a0.write(..)`, a
// METHOD-CALL RECEIVER, for which the converter suppresses the explicit borrow
// entirely.  There is no `&mut`-typed parameter and so no swallow to fix.
// ⚠️ In f14 the whole `std::mem::take(&mut *a1)` collapses to a single
// `{arg 1, access: "take"}` placeholder, which the converter renders as
// `std::mem::take(&mut <place>)` (converter.cpp:8042-8062).  It survives being
// nested one level deeper inside `replace(..)` -- verified in ir_unsafe.json.
// ===========================================================================
unsafe fn f13<T1: Clone, T2: Clone>(a0: &mut (T1, T2), a1: (T1, T2)) {
    drop(core::mem::replace(a0, a1.clone()));
}

unsafe fn f14<T1: Default, T2: Default>(a0: &mut (T1, T2), a1: &mut (T1, T2)) {
    drop(core::mem::replace(a0, std::mem::take(&mut *a1)));
}

unsafe fn f15<T1: From<Vec<libc::c_char>>, T2>(a0: Vec<libc::c_char>, a1: T2) -> (T1, T2) {
    (
        <T1>::from({
            let __p = a0.as_ptr();
            std::slice::from_raw_parts(__p, (0..).take_while(|&i| *__p.add(i) != 0).count() + 1)
                .to_vec()
        }),
        a1.into(),
    )
}

unsafe fn f16<T1: From<Vec<libc::c_char>>, T2>(a0: Vec<libc::c_char>, a1: T2) -> (T1, T2) {
    (
        <T1>::from({
            let __p = a0.as_ptr();
            std::slice::from_raw_parts(__p, (0..).take_while(|&i| *__p.add(i) != 0).count() + 1)
                .to_vec()
        }),
        a1.into(),
    )
}

unsafe fn f17<T1: PartialEq, T2: PartialEq>(a0: &(T1, T2), a1: &(T1, T2)) -> bool {
    a0 == a1
}

unsafe fn f18<T1: PartialEq, T2: PartialEq>(a0: &(T1, T2), a1: &(T1, T2)) -> bool {
    a0 != a1
}

unsafe fn f19<T1, T2: From<Vec<libc::c_char>>>(a0: T1, a1: Vec<libc::c_char>) -> (T1, T2) {
    (
        a0.into(),
        <T2>::from({
            let __p = a1.as_ptr();
            std::slice::from_raw_parts(__p, (0..).take_while(|&i| *__p.add(i) != 0).count() + 1)
                .to_vec()
        }),
    )
}

unsafe fn f20<T1: From<Vec<libc::c_char>>, T2>(a0: Vec<libc::c_char>, a1: T2) -> (T1, T2) {
    (
        <T1>::from({
            let __p = a0.as_ptr();
            std::slice::from_raw_parts(__p, (0..).take_while(|&i| *__p.add(i) != 0).count() + 1)
                .to_vec()
        }),
        a1.into(),
    )
}

// f21/f22 -- std::make_pair with an LVALUE first argument; same tuple
// construction as f9/f10.
unsafe fn f21<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}
unsafe fn f22<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}

// f23 -- `pair<T1,T2>::pair(const pair<T3,T4> &)`.  Body is f2's VERBATIM: both
// pairs share the `(T1, T2)` model, so the converting copy is a plain clone.  See
// rules/pair/src.cpp's f23 note for the blast-radius argument.
unsafe fn f23<T1: Clone, T2: Clone>(a0: (T1, T2)) -> (T1, T2) {
    a0.clone()
}

// f24 -- `pair<T1,T2>::pair()`, the default constructor.  Body is t1's VERBATIM:
// the default-initialised type rule and the default constructor must agree, and
// t1 already fixes this model's zero form.  See rules/pair/src.cpp's f24 note.
unsafe fn f24<T1: Default, T2: Default>() -> (T1, T2) {
    <(T1, T2)>::default()
}
