// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp: this module KEYS the container that already exists in
// libcc2rs/src/iterators.rs:1432 and adds nothing to libcc2rs.
//
// The bound is `Ord + Clone` because that is the bound on the struct itself
// (iterators.rs:1437) -- NOT `Ord + Copy`; `EquivalenceClasses<mlir::Value>` and
// `<void *>` are instantiated in this corpus and neither is Copy-only.
//
// The initialiser is `EquivalenceClasses::new()`, i.e. LLVM's
// `EquivalenceClasses()` default ctor (header:107): three empty maps/vectors.
// `Default for EquivalenceClasses` (iterators.rs:1447) forwards to exactly this.
//
// There is no tgt_refcount.rs.  This module has exactly ONE type key, its body
// contains no raw-pointer text and no `as_pointer()`, and placeholder access
// modes are expanded model-awarely, so the unsafe base layer (ir_unsafe.json is
// the unconditional base for BOTH models) is correct under refcount too.  A
// tgt_refcount.rs whose body would be byte-identical is omitted rather than
// restated -- and omitting the FILE is safe, whereas omitting a tN from a
// tgt_refcount.rs that exists is the rules/iostream t1 load abort.

fn t1<T1: Ord + Clone>() -> libcc2rs::EquivalenceClasses<T1> {
    libcc2rs::EquivalenceClasses::new()
}

// `llvm::EquivalenceClasses<T>::member_iterator` -> libcc2rs::MemberIter<T>
// (iterators.rs:1348). No bound: `impl<T> Default for MemberIter<T>`
// (iterators.rs:1354) is unconditional, and C++'s
// `explicit member_iterator() = default` default-constructs to the end sentinel,
// which is exactly what that impl gives (it leaves `Node` uninitialised in C++;
// the only value a default can usefully denote is `member_end()`).
fn t2<T1>() -> libcc2rs::MemberIter<T1> {
    libcc2rs::MemberIter::default()
}

// `llvm::EquivalenceClasses<T>::ECValue` -> libcc2rs::ECValue<T>
// (iterators.rs:1316). There is NO default initialiser to give: LLVM's ECValue
// declares only `ECValue(const ElemTy &Data)` and `ECValue(const ECValue &)`
// (EquivalenceClasses.h:76-80), so a default-init of an ECValue is ill-formed
// C++ and cannot occur in any faithfully translated TU. Every use in this corpus
// obtains one from `*I` / `insert()`. A `panic!` is therefore the only honest
// body -- loud if anything ever reaches it -- and it is `!`, so it needs no
// bound and constructs nothing. (libcc2rs::ECValue has private fields and no
// public ctor; giving it a Default would need a libcc2rs change plus a
// target_preprocessor re-pin, and an unreachable initialiser does not justify
// one.)
fn t3<T1>() -> libcc2rs::ECValue<T1> {
    panic!("ub: llvm::EquivalenceClasses<T>::ECValue has no default constructor")
}
