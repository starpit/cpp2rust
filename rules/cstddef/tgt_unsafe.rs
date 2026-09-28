// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1() -> u8 {
    Default::default()
}

fn f1(a0: &mut u8, a1: u32) -> u8 {
    *a0 << a1
}

fn f2(a0: &mut u8, a1: u32) -> u8 {
    *a0 >> a1
}

// ===========================================================================
// ⛔⛔ WHY `drop(core::mem::replace(a0, n_))` AND NOT `*a0 = n_`, 2026-09-28.
// Same swallowed-deref class as rules/mlir f480/f481/f520-f531/f560 (0a14e018).
// `*a0 = n_;` WAS HERE AND IT EMITTED RUST THAT DOES NOT COMPILE.  A placeholder
// whose rule parameter is `&mut T` and which sits in a NON-RECEIVER position is
// emitted by the converter as `&mut <place>` -- see converter.cpp:8030-8038,
// `needs_lvalue() && needs_mut_borrow()` returns `"&mut " + place`, gated on
// `needs_explicit_mut_borrow = !is_method_call_receiver && ParamIsMutRef(..)`.
// The rule's leading `*` is SWALLOWED by the preprocessor, not emitted, so the
// body emitted `&mut <byte-lvalue> = n_;` -- `error[E0070]: invalid left-hand
// side of assignment`.
// ⭐ AND THIS IS THE CLASS rc=0 CANNOT SEE.  E0070 is raised in HIR lowering /
// type-check, NOT in the parser; `rustfmt` parses `&mut x = v` happily and
// re-emits it verbatim at rc=0, and rustfmt is the only Rust parser in this
// pipeline.  No placeholder census and no rc=0 gate can find this.
// ⭐ THE FIX RELIES ON THE SAME RENDERING, DELIBERATELY: bare `a0` records the
// IDENTICAL `{arg 0, access: "borrow_mut"}`, so `replace(a0, n_)` emits
// `replace(&mut <place>, n_)` -- and `&mut place` is exactly what
// `core::mem::replace` wants.  `drop(..)` of the returned old value is what the
// C++ assignment does to the overwritten byte.
// ⚠️ TWO SPELLINGS THAT DO NOT WORK, both measured: `replace(&mut *a0, n_)` and
// `replace(*a0, n_)` both make rule-preprocessor ABORT with
// `semantic.rs:260: unresolved access="unknown"` -- a deref or re-borrow of a
// placeholder inside a CALL ARGUMENT has no access classification.  Only the
// bare placeholder is classifiable there.  Do not "restore" the `*`.
// ⚠️ `*a0` on the RIGHT of the `let` and as the trailing return expression is
// FINE and is left alone: those record `access: "move"`, not `borrow_mut`, so
// nothing is swallowed there (verified in ir_unsafe.json).
// ⚠️ NEGATIVE CONTROL, do not "fix" it: rules/builtin f9/f10/f12/f13 also read
// `*a2 = val;` but `a2` is `*mut i64`, a RAW POINTER -- for those the `*` is
// kept as literal TEXT in the IR and borrow_mut renders the pointer, so the
// deref is genuine and correct.  The defect needs a `&mut`-typed parameter.
// `u8` is `Copy`, so `replace` is trivially well-typed here.  cstddef ships an
// unsafe overlay ONLY (there is no tgt_refcount.rs), so there is no second
// model to keep in step.
// ===========================================================================
fn f3(a0: &mut u8, a1: u32) -> u8 {
    let n_ = *a0 << a1;
    drop(core::mem::replace(a0, n_));
    *a0
}

fn f4(a0: &mut u8, a1: u32) -> u8 {
    let n_ = *a0 >> a1;
    drop(core::mem::replace(a0, n_));
    *a0
}
