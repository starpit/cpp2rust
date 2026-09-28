// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model, the corpus enumeration, and why the member rules
// are keyed on the BASE `llvm::SetVector<T1, T2, T3, _>` and not on
// `llvm::SmallSetVector`.
//
// The model is `Vec<T1>`: INSERTION-ORDERED unique elements.  NOT a HashSet and
// not a BTreeSet -- the iteration order is observable at every corpus site
// (DoubleBuffering.cpp:219 copies begin()..end() into a SmallVector that leaves
// the function, and :243's `visited` drives a BFS), and it is the whole reason
// llvm/ADT/SetVector.h exists.
//
// T2 and T3 are the Vector and Set policy parameters; they are part of the KEY
// (the key is 4-ary, and a 1-ary key would swallow the tail) but not part of the
// model, so they appear in the generic list and nowhere else.  T4, the
// `unsigned N` small-size, is elided to `_` by the mapper and so does not appear
// at all.

use libcc2rs::*;

// ARITY REPAIR 2026-09-28: the generic 4-ary `t1` key is GONE (it was dead in
// both directions -- see src.cpp).  These are the four distinct `searched as:`
// spellings, arity-0 so no capture and therefore no swallow is possible.  The
// MODEL IS UNCHANGED: `Vec<element>`, insertion order observable.

// `llvm::SetVector<mlir::Attribute>` (1-ary).  Element model = rules/mlir t6.
fn t1() -> Vec<dataflowir_gen::ir::Attr> {
    Default::default()
}

// `llvm::SetVector<mlir::Operation *>` (1-ary).  Element model = a pointer to
// rules/mlir t1's `mlir::Operation` model.
fn t3() -> Vec<*mut dataflowir_gen::fmt::OpInst> {
    Default::default()
}

// `llvm::SetVector<mlir::Attribute, llvm::SmallVector<mlir::Attribute, _>,
//  llvm::DenseSet<mlir::Attribute>, _>` -- the 4-ary spelling of t1's type.
fn t4() -> Vec<dataflowir_gen::ir::Attr> {
    Default::default()
}

// The two 4-ary op-element forms.  An MLIR op handle held by value models as
// `OpInst`, the same model rules/mlir gives `mlir::Operation` (t1) and the
// op-trait `Impl<...>` keys (t167-t216).
fn t5() -> Vec<dataflowir_gen::fmt::OpInst> {
    Default::default()
}

fn t6() -> Vec<dataflowir_gen::fmt::OpInst> {
    Default::default()
}

fn t2<T1>() -> Vec<T1> {
    Default::default()
}

// `llvm::SmallSetVector<T1, _>`'s implicit default ctor -- the only key on the
// DERIVED class, because an inheriting-constructor declaration never inherits a
// default constructor.  See src.cpp.
unsafe fn f1<T1>() -> Vec<T1> {
    Vec::new()
}

// `insert`: a membership-tested push, returning whether it inserted.  This is
// literally what LLVM's small path does (SetVector.h:157-161,
// `if (!llvm::is_contained(vector_, X)) vector_.push_back(X)`), so the bound is
// `PartialEq`, not `Hash` -- equality is the only operation the C++ requires.
// The element is bound to a local first so the inlined argument expression is
// evaluated exactly once even though the body mentions it twice.
unsafe fn f2<T1: PartialEq, T2, T3>(a0: &mut Vec<T1>, a1: T1) -> bool {
    {
        let __x = a1;
        if a0.contains(&__x) {
            false
        } else {
            a0.push(__x);
            true
        }
    }
}

// `contains`.  The receiver is `const SetVector &`, which the mapper spells
// by value (as rules/set's f5 does for `const std::set<T1> &`).
unsafe fn f3<T1: PartialEq, T2, T3>(a0: Vec<T1>, a1: T1) -> bool {
    a0.contains(&a1)
}

// `begin()` / `end()`.  SetVector::iterator is `vector_type::const_iterator`
// (SetVector.h:72) even on the non-const overload, i.e. `const T *` -- a
// SetVector never hands out a mutable element, since mutating through the
// vector would desynchronise the set.  Same shape as rules/smallvector's
// f8/f9, with `as_ptr` for the const-ness.
unsafe fn f4<T1, T2, T3>(a0: &mut Vec<T1>) -> *const T1 {
    a0.as_ptr()
}

unsafe fn f5<T1, T2, T3>(a0: &mut Vec<T1>) -> *const T1 {
    unsafe { a0.as_ptr().add(Vec::len(&a0)) }
}
