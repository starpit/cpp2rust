// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp: rules/deque models std::deque<T1> as Vec<T1> (its tgt_unsafe t1
// is `Vec<T1>` with the VecDeque TODO), so its iterator is rules/vector's
// iterator -- a raw element pointer.  `iterator` -> *mut T1,
// `const_iterator` -> *const T1.

use libcc2rs::*;

fn t1<T1>() -> *mut T1 {
    Default::default()
}

fn t2<T1>() -> *const T1 {
    Default::default()
}

// t3/t4 -- the pointer-monomorphised siblings of t2/t1 (see src.cpp).  Same
// representation, one more level of indirection because the ELEMENT is a
// pointer: identical to rules/vector t8's `*const *mut T1`.
fn t3<T1>() -> *const *mut T1 {
    Default::default()
}

fn t4<T1>() -> *mut *mut T1 {
    Default::default()
}

unsafe fn f1<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr()
}

unsafe fn f2<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr().add(a0.len())
}

// A begin()/end() target receives the container BY VALUE even for a `const &`
// parameter, so `a0 as *const _` would be E0605; take it by value, exactly as
// rules/vector f43/f44 and rules/list_iterator f3/f4 do.
unsafe fn f3<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr()
}

unsafe fn f4<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr().add(a0.len())
}

// POSITION comparison, not value comparison: two iterators at different
// positions holding EQUAL elements must compare UNEQUAL.
unsafe fn f5<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 == a1
}

unsafe fn f6<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 != a1
}

unsafe fn f7<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.prefix_inc()
}

// postfix ++ yields the OLD position; UnsafePostfixInc does exactly that.
unsafe fn f8<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.postfix_inc()
}

unsafe fn f9<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

// difference_type is `long` -> i64, not isize.
unsafe fn f10<T1>(a0: *mut T1, a1: i64) -> *mut T1 {
    a0.offset(a1 as isize)
}

// ===========================================================================
// ⛔⛔ f11/f18 -- WHY THESE ARE SEQUENCED THROUGH `__p` AND USE
// `drop(core::mem::replace(a0, ..))` RATHER THAN `*a0 = (*a0).offset(..)`,
// 2026-09-28.  Same swallowed-deref class as rules/mlir (0a14e018), but these
// two swallowed on BOTH SIDES of the assignment, so they needed reshaping rather
// than a one-token substitution.
//
// A placeholder whose rule parameter is `&mut T` and which sits in a NON-RECEIVER
// position is emitted by the converter as `&mut <place>` -- converter.cpp:
// 8030-8038, `needs_lvalue() && needs_mut_borrow()` returns `"&mut " + place`,
// gated on `needs_explicit_mut_borrow = !is_method_call_receiver &&
// ParamIsMutRef(..)`.  The rule's leading `*` is SWALLOWED by the preprocessor,
// never emitted.  So the assignment TARGET emitted `&mut <iter-lvalue> = ..;`,
// i.e. `error[E0070]: invalid left-hand side of assignment`.
// ⭐ THE CLASS rc=0 CANNOT SEE: E0070 is raised in HIR lowering / type-check, not
// in the parser.  `rustfmt` -- the only Rust parser in this pipeline -- parses
// `&mut x = v` at rc=0 and re-emits it verbatim, so the TU reports clean and no
// placeholder census can see the breakage.
//
// ⚠️ AND THE `(*a0)` RECEIVER WAS ALSO UNSOUND, for the opposite reason: the two
// overlays classified the SAME source text differently -- in ir_unsafe.json f11's
// receiver recorded `access: "borrow_mut"` while f18's recorded `access: "move"`,
// from character-identical `(*a0).offset(..)`.  A body whose emitted receiver
// depends on which of two equivalent spellings the classifier happened to pick is
// a latent bug even while it compiles by autoderef, so the deref is removed
// instead: `a0.offset(..)` is a METHOD-CALL RECEIVER, and for a receiver the
// converter suppresses the explicit borrow altogether and lets Rust's autoref
// supply it (`!is_method_call_receiver` in the gate above).  That is the spelling
// f14/f15/f17 and the refcount overlay already use.
// ⭐ THE FIX RELIES ON THE SWALLOW, DELIBERATELY: bare `a0` in the `replace`
// argument records the IDENTICAL `{arg 0, access: "borrow_mut"}`, so it emits
// `replace(&mut <place>, __p)` -- and `&mut place` is exactly what
// `core::mem::replace` wants.
// ⚠️ DO NOT "restore" the `*`: both `replace(&mut *a0, __p)` and
// `replace(*a0, __p)` make rule-preprocessor ABORT with
// `semantic.rs:260: unresolved access="unknown"` -- a deref or re-borrow of a
// placeholder inside a CALL ARGUMENT has no access classification.
// ⚠️ SEQUENCING THROUGH `__p` IS ALSO REQUIRED, not cosmetic: it is the same
// reason the refcount overlay already gives at f11/f18 and at rules/vector f121 --
// two mentions of a `&mut` param in ONE expression inline to a `borrow()` inside a
// `borrow_mut()` and panic "RefCell already borrowed".  Keeping both overlays on
// one shape means that hazard cannot be reintroduced here by copying the unsafe
// body across.
// ⚠️ `*mut T1` / `*const T1` are `Copy`, so `replace` is trivially well-typed and
// the `drop` of the old pointer is a no-op -- it is kept for one-to-one
// correspondence with the C++ assignment and with the other sites of this class.
// Returning `__p` is identical to the old trailing `*a0`: it is the value just
// stored, and the rule's return type is `*mut T1` BY VALUE, not a reference.
// ⚠️ NEGATIVE CONTROL, do not "fix" it: rules/builtin f9/f10/f12/f13 also read
// `*a2 = val;` but `a2` is `*mut i64`, a RAW POINTER -- there the `*` is kept as
// literal TEXT in the IR and borrow_mut renders the pointer, so the deref is
// genuine and correct.  This defect requires a `&mut`-typed parameter.
// ===========================================================================
unsafe fn f11<T1>(a0: &mut *mut T1, a1: i64) -> *mut T1 {
    let __p: *mut T1 = a0.offset(a1 as isize);
    drop(core::mem::replace(a0, __p));
    __p
}

unsafe fn f12<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 == a1
}

unsafe fn f13<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 != a1
}

unsafe fn f14<T1>(a0: &mut *const T1) -> *const T1 {
    a0.prefix_inc()
}

unsafe fn f15<T1>(a0: &mut *const T1) -> *const T1 {
    a0.postfix_inc()
}

unsafe fn f16<T1>(a0: *const T1) -> *const T1 {
    a0
}

unsafe fn f17<T1>(a0: *const T1, a1: i64) -> *const T1 {
    a0.offset(a1 as isize)
}

// f18 -- the `*const` twin of f11; see the comment block at f11 for why this is
// sequenced through `__p` and uses `core::mem::replace`.
unsafe fn f18<T1>(a0: &mut *const T1, a1: i64) -> *const T1 {
    let __p: *const T1 = a0.offset(a1 as isize);
    drop(core::mem::replace(a0, __p));
    __p
}
