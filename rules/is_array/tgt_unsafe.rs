// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::is_array<T1> is an EMPTY, ZERO-SIZED, DEFAULT-CONSTRUCTIBLE tag type at every
// recorded site, so the honest model is the unit type -- `()` is zero-sized,
// trivially constructible and passable by value.  A type rule's target must yield an
// INITIALIZER (syntactic.rs:591) and `()` does; precedented by rules/plus t1 and
// rules/exception t1.
//
// ⛔ NOT a `const bool`: no site reads `::value`, so a bool would invent a value no
// ported expression consumes.  Deliberately THE SAME representation as
// rules/integral_constant t1, because the prvalue produced by f1 is what binds to
// the `std::false_type` parameter of g3log's make_unique_helper.
//
// No tgt_refcount.rs: a zero-sized tag has no pointer and no ownership for the
// refcount model to express (rules/cstddef precedent).
fn t1<T1>() -> () {
    ()
}

// The default constructor -- the prvalue `std::is_array<T1>()`.
unsafe fn f1<T1>() -> () {
    ()
}
