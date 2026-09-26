// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::MapVector -- see src.cpp and tgt_unsafe.rs.  Vec of entries so that
// iteration is INSERTION ORDER; `Value<T2>` mirrors rules/densemap's
// `BTreeMap<T1, Value<T2>>`.

use libcc2rs::*;

fn t1<T1, T2>() -> Vec<(T1, Value<T2>)> {
    Vec::new()
}
