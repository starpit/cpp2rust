// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

// A std::reverse_iterator<T*> holds its underlying iterator `current`, which
// points ONE PAST the element `*rit` designates.  It is modelled here as that
// same pointer, so `*rit` is `*(current - 1)` and `++rit` DECREMENTS.
fn t1<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn f1<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f2<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 == a1
}

fn f3<T1>(a0: Ptr<T1>, a1: Ptr<T1>) -> bool {
    a0 != a1
}

fn f4<T1>(a0: &mut Ptr<T1>) -> Ptr<T1> {
    a0.prefix_dec()
}

fn f5<T1>(a0: &mut Ptr<T1>, a1: i32) -> Ptr<T1> {
    a0.postfix_dec()
}

fn f6<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    Ptr::offset(&a0, -1)
}

fn f7<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}
