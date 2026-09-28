// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp.  The refcount model represents a std::deque iterator as a
// `Ptr<T1>` INTO the Vec<T1> container -- identical to rules/vector's
// tgt_refcount t2/t4 and rules/list_iterator's t1/t2.  Note this file MUST
// exist: omitting it does not leave these keys out of the refcount model, it
// silently falls back to the UNSAFE bodies, which are raw pointers and the
// wrong model.
//
// `operator*` here returns Ptr<T1>, NOT Value<T1>: the converter consumes
// operator*'s result with `.read()`.  Because the iterator is ALREADY an
// element Ptr (not a container Ptr), there is no weak-downgrade problem.

use libcc2rs::*;

fn t1<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn t2<T1>() -> Ptr<T1> {
    Ptr::null()
}

// t3/t4 -- the pointer-monomorphised siblings of t2/t1 (see src.cpp).  The
// element is itself a pointer, so this is `Ptr<Ptr<T1>>`, exactly as
// rules/vector tgt_refcount t8.
fn t3<T1>() -> Ptr<Ptr<T1>> {
    Ptr::null()
}

fn t4<T1>() -> Ptr<Ptr<T1>> {
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

// difference_type is `long` -> i64, not isize.
fn f10<T1>(a0: Ptr<T1>, a1: i64) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

// Sequenced into separate statements: two mentions of a `&mut` param in ONE
// expression panic at runtime with `RefCell already borrowed`.  Same shape as
// rules/vector f121.
fn f11<T1>(a0: &mut Ptr<T1>, a1: i64) -> Ptr<T1> {
    let __p: Ptr<T1> = a0.offset(a1 as isize);
    *a0 = __p.clone();
    __p
}

fn f12<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f13<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

fn f14<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_inc()
}

fn f15<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.postfix_inc()
}

fn f16<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f17<T1>(a0: Ptr<T1>, a1: i64) -> Ptr<T1> {
    a0.offset(a1 as isize)
}

fn f18<T1>(a0: &mut Ptr<T1>, a1: i64) -> Ptr<T1> {
    let __p: Ptr<T1> = a0.offset(a1 as isize);
    *a0 = __p.clone();
    __p
}
