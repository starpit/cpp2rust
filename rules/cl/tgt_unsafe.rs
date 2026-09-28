// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model, the corpus enumeration, the argv-parsing caveat and
// the list of names deliberately left unmapped.
//
// MODEL: `llvm::cl::opt<T>` IS its value.  opt_storage<T, _, _> is
// `{ T Value; ... }` with `operator T()`, and opt derives from it, so both map
// to `T1`.  `cl::initializer<T>` is a one-field carrier for the default that
// `cl::init(x)` builds, so it too maps to `T1` and `f1` is the identity.
//
// No tgt_refcount.rs: every target here is a plain value with no reference
// shape to differ on, and a partial tgt_refcount.rs that omitted a type key
// would abort at load (the rules/iostream t1 shape).

use libcc2rs::*;

fn t1<T1: Default>() -> T1 {
    Default::default()
}

fn t2<T1: Default>() -> T1 {
    Default::default()
}

fn t4() -> i32 {
    0
}


fn t5<T1: Default>() -> T1 {
    Default::default()
}

fn t6<T1: Default, T2>() -> T1 {
    Default::default()
}
