// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// mlir::Operation/Block/Region -> the .td-generated model in `dataflowir-gen`.
// src.cpp records why this replaces the fieldless `--opaque-namespace=mlir`
// handle, and why stepping is expressible here (Block::ops is a real Vec, so it
// is CONTIGUOUS) when rules/ilist must refuse it over the opaque handle.
//
// The iterator representation is rules/ilist's, not this module's: `*mut T1`.
// prefix_inc/postfix_inc/prefix_dec/postfix_dec are libcc2rs', reused exactly as
// rules/vector uses them for std::vector's iterator.

use dataflowir_gen::fmt::{Block as MlirBlock, OpInst, Region as MlirRegion};
use libcc2rs::*;

// A default-constructed Operation/Block/Region has no meaning in MLIR either:
// they are always produced by a builder. Left loud rather than invented.
fn t1() -> OpInst {
    unimplemented!()
}

fn t2() -> MlirBlock {
    unimplemented!()
}

fn t3() -> MlirRegion {
    unimplemented!()
}

fn t4<T1>() -> Vec<T1> {
    Vec::new()
}

unsafe fn f1(a0: &mut MlirBlock) -> *mut Vec<OpInst> {
    &mut a0.ops
}

unsafe fn f2<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr()
}

unsafe fn f3<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr().add(a0.len())
}

// rbegin() addresses the LAST element and rend() the slot one BEFORE the first,
// the same representation rules/vector uses (f114/f115): the reverse iterator
// points AT the element it dereferences to, so stepping it is a decrement.
unsafe fn f4<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr().add(a0.len()).wrapping_sub(1)
}

unsafe fn f5<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr().wrapping_sub(1)
}

// FORWARD ++: prefix yields the NEW position, postfix the OLD one.
unsafe fn f6<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.prefix_inc()
}

unsafe fn f7<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.postfix_inc()
}

// REVERSE ++ STEPS BACKWARD. Not a transcription slip: IsReverse=true.
unsafe fn f8<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.prefix_dec()
}

unsafe fn f9<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.postfix_dec()
}

// FORWARD --
unsafe fn f10<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.prefix_dec()
}

unsafe fn f11<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.postfix_dec()
}

// REVERSE -- STEPS FORWARD.
unsafe fn f12<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.prefix_inc()
}

unsafe fn f13<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.postfix_inc()
}

unsafe fn f14(a0: &mut MlirBlock) -> *mut OpInst {
    a0.ops.as_mut_ptr()
}

unsafe fn f15(a0: &mut MlirBlock) -> *mut OpInst {
    a0.ops.as_mut_ptr().add(a0.ops.len())
}

unsafe fn f16(a0: &mut MlirBlock) -> *mut OpInst {
    a0.ops.as_mut_ptr().add(a0.ops.len()).wrapping_sub(1)
}

unsafe fn f17(a0: &mut MlirBlock) -> *mut OpInst {
    a0.ops.as_mut_ptr().wrapping_sub(1)
}

// push_back takes ownership of the node; against a `Vec` of values that is a
// push of the pointee.
unsafe fn f18<T1>(a0: &mut Vec<T1>, a1: *mut T1) {
    a0.push(a1.read())
}

// ROW 2: the MLIR range iterator. State is `{ BaseT base; ptrdiff_t index; }`,
// so the representation is that PAIR -- `(T3, i64)`, where T3 is BaseT itself
// (the key binds the whole `mlir::OpOperand *`, not its pointee). A tuple with
// concrete members is a legal representation; rules/tuple's t4 is the
// precedent. T1 and T2 appear because the key names them.
fn t5<T1, T2, T3: Default>() -> (T3, i64) {
    (T3::default(), 0)
}

// `!=` on a range iterator is POSITION IDENTITY over BOTH fields: MLIR's
// operator== is `base == rhs.base && index == rhs.index`. Comparing only the
// index would call iterators into two DIFFERENT ranges equal.
unsafe fn f19<T1, T2, T3: PartialEq>(a0: (T3, i64), a1: (T3, i64)) -> bool {
    a0.0 != a1.0 || a0.1 != a1.1
}

// ROW 1: `==` is f19 negated -- POSITION IDENTITY over BOTH fields of the pair.
// `base` alone would call begin() and end() of the same range equal; `index`
// alone would call iterators into two DIFFERENT ranges equal.
unsafe fn f20<T1, T2, T3: PartialEq>(a0: (T3, i64), a1: (T3, i64)) -> bool {
    a0.0 == a1.0 && a0.1 == a1.1
}

// ROW 2: PRE-increment yields the NEW position. `base` is untouched: stepping
// moves the INDEX (STLExtras.h:1179). One block expression, no `let` holding a
// borrow -- a rule body is inlined into the caller.
unsafe fn f21<T1, T2, T3: Clone>(a0: &mut (T3, i64)) -> (T3, i64) {
    {
        a0.1 += 1;
        (a0.0.clone(), a0.1)
    }
}

// The RANGE, `(base, count)` -- STLExtras.h:1241 stores exactly those two
// fields, so this is the iterator's pair shape reused, not a new model.
fn t6<T1, T2, T3: Default>() -> (T3, i64) {
    (T3::default(), 0)
}

unsafe fn f22<T1, T2, T3>(a0: T3, a1: i64) -> (T3, i64) {
    (a0, a1)
}

// begin() = `iterator(base, 0)`; end() = `iterator(base, count)`. Both carry the
// range's OWN base, so two ranges over distinct bases yield iterators that must
// compare unequal at equal index.
unsafe fn f23<T1, T2, T3: Clone>(a0: (T3, i64)) -> (T3, i64) {
    (a0.0.clone(), 0)
}

unsafe fn f24<T1, T2, T3: Clone>(a0: (T3, i64)) -> (T3, i64) {
    (a0.0.clone(), a0.1)
}

// ROW 3: mlir::Operation's USER iterator. A single pointer is the entire state
// (UseDefLists.h:332 `detail::IROperandBase *current`; ValueUserIterator adds no
// member, it stores the wrapped iterator at iterator.h:244), and operator== is
// `current == rhs.current` (UseDefLists.h:326). So POSITION IDENTITY IS POINTER
// IDENTITY and no model of mlir::OpOperand's contents is needed.
fn t7<T1>() -> *mut T1 {
    std::ptr::null_mut()
}

fn t8<T1, T2>() -> *mut T2 {
    std::ptr::null_mut()
}

// Pointer identity, NOT value equality: two iterators over distinct OpOperands
// holding equal values must compare UNEQUAL.
unsafe fn f25<T1, T2>(a0: *mut T2, a1: *mut T2) -> bool {
    a0 != a1
}

// Both producers are identity on the pointer.
unsafe fn f26<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

unsafe fn f27<T1>(a0: *mut T1) -> *mut T1 {
    a0
}
