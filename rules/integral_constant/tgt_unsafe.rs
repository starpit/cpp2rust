// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::integral_constant<bool, false> is an EMPTY, ZERO-SIZED, DEFAULT-CONSTRUCTIBLE
// type whose only job at the recorded sites is to select an overload.  The honest
// model is therefore the unit type: `()` is zero-sized, trivially constructible, and
// passable by value.  A type rule's target must yield an INITIALIZER
// (syntactic.rs:591) and `()` does; the shape is precedented by rules/exception t1
// and rules/plus t1.
//
// ⛔ NOT a `const bool` and NOT a struct with state.  The sites never read
// `::value`, so a bool would invent a value that no ported expression consumes, and
// a stateful struct would invent a field.  Keeping it a unit also makes this module
// and rules/is_array share ONE representation, which is required: the argument
// `std::is_array<T>()` binds to this type's parameter position.
//
// No tgt_refcount.rs: a zero-sized type has no pointer and no ownership for the
// refcount model to express, so the unconditional unsafe layer is correct for both
// models (rules/cstddef is the precedent for a module with no refcount target).
fn t1() -> () {
    ()
}

// The default constructor -- the prvalue `std::integral_constant<bool,false>()`.
unsafe fn f1() -> () {
    ()
}
