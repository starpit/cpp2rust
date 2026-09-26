// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::PointerUnion<A, B, C> -> libcc2rs::Variant3, the SAME discriminated
// runtime rules/variant uses for std::variant (its t2 is this exact type).  The
// discriminant is therefore preserved, not erased, even though no accessor is
// mapped yet -- src.cpp says why every accessor is unreachable in dt_src and why
// leaving them absent is the loud answer rather than a guess.

use libcc2rs::*;

fn t1<T1: Default, T2: Default, T3: Default>() -> Variant3<T1, T2, T3> {
    Default::default()
}

// Default-constructed: the FIRST alternative, null.  Variant3's own Default is
// `V1(T1::default())` (variant.rs:149), so this matches C++'s initial
// discriminant rather than picking one.
fn f1<T1: Default, T2: Default, T3: Default>() -> Variant3<T1, T2, T3> {
    Default::default()
}
