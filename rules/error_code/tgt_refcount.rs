// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

// An i32 has an identical representation in both models, so only the reference
// form needs a refcount body: a C++ reference is a Ptr, as in rules/string t2.
fn t1() -> i32 {
    0
}

fn t2() -> Ptr<i32> {
    Ptr::null()
}
