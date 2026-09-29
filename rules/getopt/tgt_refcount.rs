// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// ⛔ DELIBERATELY EMPTY -- every key in this module has an UNSAFE-MODEL body
// only, and it is a refusal with a measured cause.  In the refcount model the
// converter emits the corpus's
// `struct option longopts[] = {{"help", no_argument, nullptr, 'h'}, ...}` with
// EVERY FIELD Value-wrapped --
//     ::libc::option { name: Rc::new(RefCell::new(Ptr::<u8>::from_string_literal(b"help"))),
//                      has_arg: Rc::new(RefCell::new(0)), ... }
// (measured, snap/coord43/cpp2rust --model=refcount on a 19-line probe TU) -- so
// `::libc::option`, whose fields are `*const c_char`/`c_int`, cannot be the
// refcount representation.  The honest one is a `libcc2rs` struct of
// `Value<Ptr<u8>>`/`Value<i32>` in rules/dirent `Dirent`'s shape, driven by a
// scanner over `Ptr<Ptr<u8>>` rather than `*const *mut c_char`.  That is a second
// model, not a second spelling, and it did not fit this slot's budget.
// rules/errno is the precedent for a legitimately partial refcount target
// (1 key of 153).
//
// ⛔⛔ AND DO NOT BELIEVE THIS MAKES THE REFCOUNT LEG FAIL LOUDLY AT TRANSLATE
// TIME.  An earlier revision of this comment claimed exactly that and it is
// FALSE, caught by check-ir.sh on this very module:
//     NOTE: 17 type keys are in tgt_unsafe.rs but NOT in the module's
//       tgt_refcount.rs. ... the refcount model then uses the UNSAFE body for
//       them ... getopt/t1 ...
// i.e. the two models are a UNION, so an absent refcount body falls back to the
// unsafe one.  The real refcount behaviour is therefore: `option` is MAPPED (to
// `::libc::option`), translation proceeds, and the mismatch surfaces as a
// type-error wall in the generated file -- loud at rustc, NOT loud at translate
// time.  Recorded because a false "it aborts loudly" safety claim is the exact
// failure class this project keeps paying for.
//
// ⭐ IT IS STILL NOT REACHED ON THE PORT GOAL.  `dxp/dxp_standalone.cpp` in
// refcount aborts well before `main` with
//     LLVM ERROR: unsupported structured binding / DecompositionDecl with 2
//     bindings [k, v] of type `const std::pair<const BaseFuncType, std::string> &`
//     is not implemented, reached while converting
//     `flipMap(...)::(lambda)::operator()` at util/utils.h:148:16
// so nothing in this module changes that leg today.
