// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp.  The refcount model represents a std::deque iterator as a
// `Ptr<T1>` INTO the Vec<T1> container -- identical to rules/vector's
// tgt_refcount t2/t4 and rules/list_iterator's t1/t2.  Note this file MUST
// exist: omitting it does not leave these keys out of the refcount model, it
// silently falls back to the UNSAFE bodies, which are raw pointers and the
// wrong model.
//
// `operator*` here returns Ptr<T1>, NOT Value<T1>: the converter consumes
// operator*'s result with `.read()`.  Because the iterator is ALREADY an
// element Ptr (not a container Ptr), there is no weak-downgrade problem.

use libcc2rs::*;

fn t1<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn t2<T1>() -> Ptr<T1> {
    Ptr::null()
}

// t3/t4 -- the pointer-monomorphised siblings of t2/t1 (see src.cpp).  The
// element is itself a pointer, so this is `Ptr<Ptr<T1>>`, exactly as
// rules/vector tgt_refcount t8.
fn t3<T1>() -> Ptr<Ptr<T1>> {
    Ptr::null()
}

fn t4<T1>() -> Ptr<Ptr<T1>> {
    Ptr::null()
}

fn f1<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f2<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f3<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f4<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f5<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f6<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

fn f7<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f8<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_inc()
}

fn f9<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

// difference_type is `long` -> i64, not isize.
fn f10<T1>(a0: Ptr<T1>, a1: i64) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

// Sequenced into separate statements: two mentions of a `&mut` param in ONE
// expression panic at runtime with `RefCell already borrowed`.  Same shape as
// rules/vector f121.
//
// ⛔⛔ AND THE STORE IS `drop(core::mem::replace(a0, ..))`, NOT `*a0 = ..`,
// 2026-09-28 -- same swallowed-deref class as rules/mlir (0a14e018).
// `*a0 = __p.clone();` WAS HERE AND IT EMITTED RUST THAT DOES NOT COMPILE.  A
// placeholder whose rule parameter is `&mut T` and which sits in a NON-RECEIVER
// position is emitted as `&mut <place>` -- converter.cpp:8030-8038,
// `needs_lvalue() && needs_mut_borrow()` returns `"&mut " + place`, gated on
// `needs_explicit_mut_borrow = !is_method_call_receiver && ParamIsMutRef(..)`.
// The rule's leading `*` is SWALLOWED by the preprocessor, so this emitted
// `&mut <iter-lvalue> = ..;` -- `error[E0070]: invalid left-hand side of
// assignment`.
// ⭐ THE CLASS rc=0 CANNOT SEE: E0070 is raised in HIR lowering / type-check, not
// in the parser.  `rustfmt` -- the only Rust parser in this pipeline -- parses
// `&mut x = v` at rc=0 and re-emits it verbatim, so the TU reports clean.
// ⭐ THE FIX RELIES ON THE SAME RENDERING: bare `a0` records the IDENTICAL
// `{arg 0, access: "borrow_mut"}`, so `replace(a0, ..)` emits
// `replace(&mut <place>, ..)`, exactly what `core::mem::replace` wants.
// ⚠️ DO NOT "restore" the `*`: both `replace(&mut *a0, ..)` and
// `replace(*a0, ..)` make rule-preprocessor ABORT with
// `semantic.rs:260: unresolved access="unknown"` -- a deref or re-borrow of a
// placeholder inside a CALL ARGUMENT has no access classification.
// ⚠️ TYPE CHECK, because this differs from the mlir sites, where `a1` was a plain
// `ir::Attr` enum: `libcc2rs::Ptr<T>` is NOT `Copy`.  That does not matter --
// `core::mem::replace` requires only `Sized`, which `Ptr<T>` satisfies for every
// `T` (libcc2rs/src/rc.rs:166, a two-field struct of `usize` + `PtrKind<T>`), and
// `impl<T> Clone for Ptr<T>` at rc.rs:180 is unconditional, so no generic bound
// has to be added to `T1`.  Dropping the replaced `Ptr<T1>` releases exactly the
// refcount the old `*a0 = ..` assignment released, so this is refcount-neutral.
// ⚠️ `a0.offset(..)` above is a METHOD-CALL RECEIVER and is CORRECT as written:
// for a receiver the converter suppresses the explicit borrow and lets Rust's
// autoref supply it, so no deref may be written there either.
fn f11<T1>(a0: &mut Ptr<T1>, a1: i64) -> Ptr<T1> {
    let __p: Ptr<T1> = a0.offset(a1 as isize);
    drop(core::mem::replace(a0, __p.clone()));
    __p
}

fn f12<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f13<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

fn f14<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f15<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_inc()
}

fn f16<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f17<T1>(a0: Ptr<T1>, a1: i64) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

// f18 -- the `const_iterator` twin of f11; see the comment block at f11 for why
// the store is `drop(core::mem::replace(a0, ..))` and not `*a0 = ..`.
fn f18<T1>(a0: &mut Ptr<T1>, a1: i64) -> Ptr<T1> {
    let __p: Ptr<T1> = a0.offset(a1 as isize);
    drop(core::mem::replace(a0, __p.clone()));
    __p
}
