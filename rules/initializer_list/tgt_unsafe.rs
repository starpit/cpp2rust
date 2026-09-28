// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

fn t1<T1>() -> Vec<T1> {
    Default::default()
}

unsafe fn f1<T1>(a0: Vec<T1>) -> usize {
    a0.len()
}

// The empty `{}` initializer_list. Nullary, so there is no argument to
// substitute and none of the `&(*s)`-textual-append hazard applies.
unsafe fn f2<T1>() -> Vec<T1> {
    Vec::new()
}
