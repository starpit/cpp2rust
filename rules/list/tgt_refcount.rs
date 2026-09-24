// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// OVERLAY over tgt_unsafe.rs: only the rules whose refcount form differs are
// written here. The container stays a Vec<T>; what changes is the iterator,
// which is the Ptr<T> (Rc + element offset) that rules/vector and rules/deque
// already use. See the header comment in src.cpp for the std::list semantics
// the Vec representation does NOT preserve.

use libcc2rs::*;

fn t2<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn t3<T1>() -> Ptr<T1> {
    Ptr::null()
}

// --- element access --------------------------------------------------------

fn f10<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f11<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f12<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_last()
}

fn f13<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_last()
}

// --- begin / end -----------------------------------------------------------

fn f14<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f15<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f16<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f17<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f18<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f19<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

// --- assignment ------------------------------------------------------------

fn f23<T1: Clone + ByteRepr>(a0: Ptr<Vec<T1>>, a1: Vec<T1>) {
    a0.write(a1)
}

fn f24<T1: ByteRepr + Clone>(a0: Ptr<Vec<T1>>, a1: &mut Vec<T1>) {
    a0.write(std::mem::take(&mut *a1))
}

// --- erase / insert --------------------------------------------------------
//
// Ptr<T> names an element OFFSET, so the incoming iterator is still the right
// handle after the Vec shifts: erase leaves it on the element that followed,
// insert leaves it on the freshly inserted one. Both match what C++ returns.

fn f25<T1: ByteRepr>(a0: &mut Vec<T1>, a1: Ptr<T1>) -> Ptr<T1> {
    let __pos = a1.get_offset();
    a0.remove(__pos);
    a1
}

fn f26<T1: ByteRepr>(a0: &mut Vec<T1>, a1: Ptr<T1>, a2: T1) -> Ptr<T1> {
    let __pos = a1.get_offset();
    a0.insert(__pos, a2);
    a1
}

fn f27<T1: ByteRepr + Default>(a0: &mut Vec<T1>, a1: Ptr<T1>, a2: &mut T1) -> Ptr<T1> {
    let __pos = a1.get_offset();
    a0.insert(__pos, std::mem::take(&mut *a2));
    a1
}

// --- iterator --------------------------------------------------------------

fn f28<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f29<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f30<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_inc()
}

fn f31<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_dec()
}

fn f32<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f33<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

// --- const_iterator --------------------------------------------------------

fn f34<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f35<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f36<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_inc()
}

fn f37<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_dec()
}

fn f38<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f39<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

// --- iterator copy / conversion --------------------------------------------

fn f40<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f41<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

// A REFCOUNT OVERLAY for the copy constructor, which this module did not have.
// With only a tgt_unsafe body the refcount model falls through to it, and its
// `a0.clone()` is SHALLOW: when the element is itself a container of `Value`
// cells the copy shares those handles and aliases the original.  A missing
// overlay is the invisible form of this bug -- the module reads as already
// handled.  See libcc2rs/src/deep_clone.rs for the measurement.
fn f21<T1: DeepClone>(a0: Vec<T1>) -> Vec<T1> {
    a0.deep_clone()
}
