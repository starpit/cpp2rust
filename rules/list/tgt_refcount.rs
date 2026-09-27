// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

// See src.cpp.  The container representation (Vec<T1>) is IDENTICAL in both
// models, so only the entries whose SIGNATURE differs are overridden here; every
// other key falls through to tgt_unsafe.rs, which is the correct body for it.
//
// THIS FILE MUST EXIST even though it overrides only four entries: omitting it
// does NOT leave those keys out of the refcount model -- the converter silently
// falls back to the UNSAFE body, which for a C++-reference return is a raw
// pointer and the wrong model (measured in rules/sstream).
//
// front()/back() return a C++ reference, which the refcount model represents as
// a Ptr INTO the container -- the same shape rules/queue f6/f7 and
// rules/vector f51 use.
fn f11<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f12<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_last()
}

fn f13<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f14<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0.to_last()
}
