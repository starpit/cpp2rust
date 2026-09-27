// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp.  The refcount model represents a std::list iterator as a
// `Ptr<T1>` INTO the Vec<T1> container -- identical to rules/vector's
// tgt_refcount t2/t4.  Note this file MUST exist: omitting it does not leave
// these keys out of the refcount model, it silently falls back to the UNSAFE
// bodies, which are raw pointers and the wrong model.
//
// `operator*` here returns Ptr<T1>, NOT Value<T1>: the converter consumes
// operator*'s result with `.read()`.  Because the iterator is ALREADY an
// element Ptr (not a container Ptr), there is no weak-downgrade problem --
// which is exactly why rules/set's refcount operator* had to fail loudly and
// this one does not.

use libcc2rs::*;

fn t1<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn t2<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn f1<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f2<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f3<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f4<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_end()
}

fn f5<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f6<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

fn f7<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f8<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_inc()
}

fn f9<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f10<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f11<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

fn f12<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f13<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_inc()
}

fn f14<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}
