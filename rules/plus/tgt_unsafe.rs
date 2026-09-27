// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::plus<T1> is STATELESS, so the model is the unit type -- a zero-sized
// struct, which is exactly what `()` is.  A type rule's target must yield an
// INITIALIZER (syntactic.rs:591), and `()` does; the shape is precedented by
// rules/exception's t1.
//
// No CALL METHOD is needed on the type, and that is not a shortcut.  A rule body
// is INLINED at the use site (ir.rs:52), so the ported `op(a, b)` inside a
// project-defined apply() becomes the f2 body TEXTUALLY -- i.e. `a1 + a2`.  The
// receiver a0 is therefore unused by construction, which is correct: std::plus
// has no state to consult.
//
// The unsafe and refcount targets are IDENTICAL, on purpose: a stateless empty
// type has no pointer and no ownership for either model to express.
fn t1<T1>() -> () {
    ()
}

unsafe fn f1<T1>() -> () {
    ()
}

// operator()(const T1&, const T1&) const.  a0 is the std::plus receiver and is
// deliberately dropped; a1/a2 are the operands.  Each `aN` RE-EVALUATES its
// argument, and each is mentioned EXACTLY ONCE here, so a side-effecting operand
// is evaluated once -- which matches C++.
unsafe fn f2<T1: std::ops::Add<Output = T1> + Copy>(a0: (), a1: T1, a2: T1) -> T1 {
    a1 + a2
}
