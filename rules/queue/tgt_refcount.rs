// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

// front()/back() return C++ references, which the refcount model represents as
// a Ptr into the container -- the same shape rules/deque uses for its front()
// and back().  Everything else has an identical representation in both models,
// so it is left to the unsafe body.
fn f6<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f7<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_last()
}
