// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

// front() returns a reference to the head element, which in this model is the
// Ptr the receiver already is: rules/deque's f2 (front of a deque) is the same
// identity.
fn f4<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f5<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}
