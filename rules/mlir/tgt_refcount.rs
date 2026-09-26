// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// The refcount half of rules/mlir. See src.cpp for why this module exists and
// tgt_unsafe.rs for the stepping semantics.
//
// The iterator representation is rules/ilist's `Ptr<T1>`, which carries its own
// offset into the owning allocation, so stepping is the same prefix_inc /
// postfix_inc / prefix_dec / postfix_dec that rules/vector uses for
// std::vector's iterator (tgt_refcount.rs f28/f34) -- reused, not restated.
//
// rend() is named as an offset from the END exactly as rules/vector's f115 does
// rather than by subtracting 1 from the start: `Ptr::offset` walks a usize
// offset field, so `begin().offset(-1)` would underflow and panic, while
// `to_end().offset(-(len + 1))` reaches the same slot arithmetically.

use dataflowir_gen::fmt::{Block as MlirBlock, OpInst, Region as MlirRegion};
use libcc2rs::*;

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

// `Block::getOperations()` hands out a reference to the `ops` field.  In the
// refcount model a `Ptr` is a `Weak<RefCell<_>>` plus an offset, and Rc/RefCell
// cannot be PROJECTED to a field: there is no way to build a `Ptr<Vec<OpInst>>`
// that aliases `block.ops` while the block itself stays the owner.  The two
// wrong answers are both silent: cloning the Vec (`Ptr::alloc(... .ops.clone())`)
// makes every subsequent mutation invisible to the block, and a
// reinterpret_cast of the block pointer assumes `ops` sits at offset 0, which
// fmt.rs:435 says it does not.  Left as a loud panic rather than either.
// The unsafe model has no such restriction (`&mut a0.ops`), and the operators
// below -- the rules the 11 aborting TUs actually need -- work in both models,
// because they act on an iterator rules/ilist has already produced.
fn f1(a0: Ptr<MlirBlock>) -> Ptr<Vec<OpInst>> {
    let _ = &a0;
    unimplemented!("refcount model cannot project Ptr<Block> to its `ops` field")
}

// On the list itself the receiver IS the owning allocation, so `decay()` turns
// `Ptr<Vec<T1>>` (pointer AT the Vec) into `Ptr<T1>` (pointer INTO it), which is
// precisely what begin() means.
fn f2<T1>(a0: Ptr<Vec<T1>>) -> Ptr<T1> {
    a0.decay()
}

fn f3<T1>(a0: Ptr<Vec<T1>>) -> Ptr<T1> {
    a0.decay().to_end()
}

// rbegin() addresses the LAST element, so stepping a reverse iterator is a
// decrement; rend() is the slot one BEFORE the first.
fn f4<T1>(a0: Ptr<Vec<T1>>) -> Ptr<T1> {
    a0.decay().to_end().offset(-1_isize)
}

fn f5<T1>(a0: Ptr<Vec<T1>>) -> Ptr<T1> {
    let __len = a0.decay().len() as isize;
    a0.decay().to_end().offset(-(__len + 1))
}

// FORWARD ++: prefix yields the NEW position, postfix the OLD one.
fn f6<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f7<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_inc()
}

// REVERSE ++ STEPS BACKWARD.  Not a transcription slip: IsReverse=true.
fn f8<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_dec()
}

fn f9<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_dec()
}

// FORWARD --
fn f10<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_dec()
}

fn f11<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_dec()
}

// REVERSE -- STEPS FORWARD.
fn f12<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f13<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_inc()
}

// Block's own begin/end/rbegin/rend hit the same projection wall as f1.
fn f14(a0: Ptr<MlirBlock>) -> Ptr<OpInst> {
    let _ = &a0;
    unimplemented!("refcount model cannot project Ptr<Block> to its `ops` field")
}

fn f15(a0: Ptr<MlirBlock>) -> Ptr<OpInst> {
    let _ = &a0;
    unimplemented!("refcount model cannot project Ptr<Block> to its `ops` field")
}

fn f16(a0: Ptr<MlirBlock>) -> Ptr<OpInst> {
    let _ = &a0;
    unimplemented!("refcount model cannot project Ptr<Block> to its `ops` field")
}

fn f17(a0: Ptr<MlirBlock>) -> Ptr<OpInst> {
    let _ = &a0;
    unimplemented!("refcount model cannot project Ptr<Block> to its `ops` field")
}

// push_back on the list itself: the receiver IS the owning allocation, so
// `with_mut` reaches the Vec -- the f1 projection wall does not apply here.
fn f18<T1: ByteRepr + Clone>(a0: Ptr<Vec<T1>>, a1: Ptr<T1>) {
    a0.with_mut(|__v: &mut Vec<T1>| __v.push(a1.read()))
}

// ROW 2: the MLIR range iterator -- see tgt_unsafe.rs. The pair is the same in
// both models; only BaseT's own representation differs (`Ptr<_>` rather than
// `*mut _`), and that is T3, supplied by the caller. No projection is involved,
// so the f1/f14 wall does not apply here.
fn t5<T1, T2, T3: Default>() -> (T3, i64) {
    (T3::default(), 0)
}

fn f19<T1, T2, T3: PartialEq>(a0: (T3, i64), a1: (T3, i64)) -> bool {
    a0.0 != a1.0 || a0.1 != a1.1
}

// ROW 1: `==` is f19 negated -- POSITION IDENTITY over BOTH fields of the pair.
// `base` alone would call begin() and end() of the same range equal; `index`
// alone would call iterators into two DIFFERENT ranges equal.
fn f20<T1, T2, T3: PartialEq>(a0: (T3, i64), a1: (T3, i64)) -> bool {
    a0.0 == a1.0 && a0.1 == a1.1
}

// ROW 2: PRE-increment yields the NEW position. `base` is untouched: stepping
// moves the INDEX (STLExtras.h:1179). One block expression, no `let` holding a
// borrow -- a rule body is inlined into the caller.
fn f21<T1, T2, T3: Clone>(a0: &mut (T3, i64)) -> (T3, i64) {
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

fn f22<T1, T2, T3>(a0: T3, a1: i64) -> (T3, i64) {
    (a0, a1)
}

// begin() = `iterator(base, 0)`; end() = `iterator(base, count)`. Both carry the
// range's OWN base, so two ranges over distinct bases yield iterators that must
// compare unequal at equal index.
fn f23<T1, T2, T3: Clone>(a0: (T3, i64)) -> (T3, i64) {
    (a0.0.clone(), 0)
}

fn f24<T1, T2, T3: Clone>(a0: (T3, i64)) -> (T3, i64) {
    (a0.0.clone(), a0.1)
}
