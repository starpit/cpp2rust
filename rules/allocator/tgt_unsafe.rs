// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::allocator<T1> is EMPTY and STATELESS (no data members; every instance
// compares equal), so the honest model is the unit type: zero-sized, trivially
// constructible, passable by value.  A type rule's target must yield an INITIALIZER
// (syntactic.rs:591) and `()` does; precedented by rules/plus t1 and
// rules/exception t1.
//
// ⛔ This is a HANDLE FOR THE TYPE POSITION ONLY.  It deliberately carries no
// allocate/deallocate: Rust container rules own their storage, and no recorded site
// calls through an allocator object.  A site that did would find no rule and fail
// loudly at translate time, which is the intended outcome -- better than a guessed
// body that compiles.
//
// No tgt_refcount.rs: an empty stateless type has no pointer and no ownership for
// the refcount model to express (rules/cstddef precedent).
fn t1<T1>() -> () {
    ()
}

// The default constructor -- the prvalue `std::allocator<T1>()`.
unsafe fn f1<T1>() -> () {
    ()
}
