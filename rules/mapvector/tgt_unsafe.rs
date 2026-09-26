// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::MapVector -- see src.cpp.  A Vec of entries, NOT a BTreeMap: iteration
// must yield INSERTION ORDER, which is the whole reason MapVector exists over
// DenseMap.  `Box<T2>` mirrors rules/densemap's `BTreeMap<T1, Box<T2>>`: the
// value is boxed so an entry's address is stable.

use libcc2rs::*;

fn t1<T1, T2>() -> Vec<(T1, Box<T2>)> {
    Vec::new()
}
