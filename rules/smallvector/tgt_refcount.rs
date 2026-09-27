// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Refcount model for llvm::SmallVector.  Follows rules/array and rules/vector:
// neither defines a target TYPE rule for the plain container (array defines
// none at all; vector defines only the iterator and nested-container ones), so
// t1..t4 are omitted here too, and only the members whose refcount shape is
// established by those two modules are given bodies.  Everything else is left
// out rather than guessed. (An earlier version of this comment said an unmapped
// member becomes an `unimplemented!()`; that is STALE. The sole emitter of that
// was replaced by a loud refusal in converter.cpp, and `grep -rn 'unimplemented!'`
// over the converter, the rule preprocessor and libcc2rs now finds only comments.
// An unmapped member FAILS AT TRANSLATE TIME, which is loud where a wrong body
// would be silent -- and that is the point.)

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn f1<T1: ByteRepr>(a0: Ptr<Vec<T1>>, a1: T1) {
    a0.with_mut(|__v: &mut Vec<T1>| __v.push(a1))
}

fn f7<T1>(a0: Ptr<T1>, a1: usize) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

fn f8<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f9<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f10<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f11<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f12<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_last()
}
