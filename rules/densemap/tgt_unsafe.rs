// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use std::collections::{HashMap, HashSet};

fn t1<T1, T2>() -> HashMap<T1, T2> {
    HashMap::new()
}

fn t2<T1>() -> HashSet<T1> {
    HashSet::new()
}
