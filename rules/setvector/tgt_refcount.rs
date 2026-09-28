// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Refcount model for llvm::SetVector.  The MODEL is unchanged -- a `Vec<T1>` in
// insertion order -- only the reference shapes differ, exactly as in
// rules/smallvector's refcount target, which this mirrors key for key:
//   * a MUTABLE container receiver becomes `Ptr<Vec<T1>>` and is written through
//     `with_mut` (smallvector f1);
//   * a `begin()`/`end()` receiver becomes `Ptr<T1>` -- the mapper's
//     reference-type derivation collapses the container away here -- and the
//     bodies are `a0` and `a0.to_end()` (smallvector f8/f9);
//   * a CONST container receiver stays a plain value/borrow (smallvector f20).
//
// t1 and t2 are RESTATED rather than omitted.  Their bodies are identical to the
// unsafe ones, but a module that carries a tgt_refcount.rs at all is safer
// restating every type key than relying on the loader's union.

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

fn f1<T1>() -> Vec<T1> {
    Vec::new()
}

fn f2<T1: PartialEq + ByteRepr, T2, T3>(a0: Ptr<Vec<T1>>, a1: T1) -> bool {
    a0.with_mut(|__v: &mut Vec<T1>| {
        let __x = a1;
        if __v.contains(&__x) {
            false
        } else {
            __v.push(__x);
            true
        }
    })
}

fn f3<T1: PartialEq, T2, T3>(a0: Vec<T1>, a1: T1) -> bool {
    a0.contains(&a1)
}

fn f4<T1, T2, T3>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f5<T1, T2, T3>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

// ============================================================================
// ARITY-0 CONCRETE MEMBER KEYS, 2026-09-28.  See src.cpp: f2..f5 are keyed
// 4-ARY GENERIC while the converter asks ARITY-0 CONCRETE, and the 4-ary key
// cannot match because matchTemplate's `T1` swallows the whole argument list
// (a comma is not a delimiter).  These carry the SAME MODEL -- a `Vec` in
// insertion order, insert = membership-tested push -- spelled with no template
// parameters at all, so no capture and therefore no swallow is possible.
// ⛔ DO NOT generalise these back to `T1`: that is the defect.
//
// Element `mlir::Attribute` -> `dataflowir_gen::ir::Attr`, which derives
// PartialEq/Eq (ir.rs:465), so the membership test is honest.

fn f6(a0: &mut Vec<dataflowir_gen::ir::Attr>, a1: dataflowir_gen::ir::Attr) -> bool {
    let __x = a1;
    if a0.contains(&__x) {
        false
    } else {
        a0.push(__x);
        true
    }
}

fn f7(a0: Vec<dataflowir_gen::ir::Attr>, a1: dataflowir_gen::ir::Attr) -> bool {
    a0.contains(&a1)
}

// `size()` is `size_type`, which the ask prints as `unsigned long`.
fn f8(a0: Vec<dataflowir_gen::ir::Attr>) -> u64 {
    Vec::len(&a0) as u64
}

// Element `mlir::Operation *` -> `*mut dataflowir_gen::fmt::OpInst` (t3's
// model).  Raw pointers are PartialEq, so the membership test is fine here even
// though the by-value op-handle model is not (see src.cpp's refusal).
fn f9(a0: &mut Vec<*mut dataflowir_gen::fmt::OpInst>, a1: *mut dataflowir_gen::fmt::OpInst) -> bool {
    let __x = a1;
    if a0.contains(&__x) {
        false
    } else {
        a0.push(__x);
        true
    }
}

fn f10(a0: Vec<*mut dataflowir_gen::fmt::OpInst>) -> u64 {
    Vec::len(&a0) as u64
}
