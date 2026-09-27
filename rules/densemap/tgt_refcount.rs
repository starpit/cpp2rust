// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::collections::{HashMap, HashSet};

// The value is `Value<T2>` and not `Box<T2>`, mirroring rules/unordered_map's
// refcount model: a mapped value the C++ hands out a reference to must be
// shared, not owned by the container.
fn t1<T1, T2>() -> HashMap<T1, Value<T2>> {
    HashMap::new()
}

fn t2<T1>() -> HashSet<T1> {
    HashSet::new()
}

// The CRTP base is the SAME container as the derived DenseMap, so it maps to the
// same HashMap.  T1 (the derived type), T4 (DenseMapInfo traits) and T5
// (DenseMapPair bucket) are representation details with no Rust analogue and are
// deliberately unused -- see the DenseMapInfo note in src.cpp.
fn t3<T1, T2, T3, T4, T5>() -> HashMap<T2, Value<T3>> {
    HashMap::new()
}
