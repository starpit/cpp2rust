// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// `std::exception` -> `()`.  src.cpp says why: the class has no data members, so
// as a base sub-object it contributes no observable state, and the unit type is
// the faithful image of an empty base.
//
// The body is `{ () }`, not an empty block: a type rule's target MUST YIELD AN
// INITIALIZER, and `fn t1() -> () {}` panics the rule preprocessor at
// syntactic.rs:591.
//
// The unsafe and refcount targets are IDENTICAL here, on purpose: there is no
// pointer and no ownership in an empty type, so neither model's representation
// choice has anything to express.
fn t1() -> () {
    ()
}
