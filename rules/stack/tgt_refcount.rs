// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

// top() returns a C++ reference, which the refcount model represents as a Ptr
// into the container -- the same shape rules/queue uses for back() (`to_last`)
// and rules/deque uses for front()/back().  top() is the BACK of the Vec, so it
// is `to_last`, NOT rules/queue's front()-shaped `a0`.  Everything else has an
// identical representation in both models and is left to the unsafe body.
fn f6<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_last()
}
