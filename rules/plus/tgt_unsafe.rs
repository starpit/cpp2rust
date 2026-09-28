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

// t2 -- std::multiplies<T1>.  Stateless empty type, so the model is `()`, for
// the identical reason given for t1 above.
fn t2<T1>() -> () {
    ()
}

unsafe fn f3<T1>() -> () {
    ()
}

// f4 -- multiplies::operator()(const T1&, const T1&) const.  a0 is the dropped
// receiver; a1/a2 are each mentioned EXACTLY ONCE so a side-effecting operand is
// evaluated once, matching C++.
//
// SEMANTICS, stated rather than assumed.  For FLOATING-POINT T1 (g2908 is
// std::multiplies<double>) Rust `*` is IEEE-754 multiplication, bit-identical to
// C++'s.  For INTEGER T1 (g2909/g2910 are std::multiplies<long>) Rust `*`
// PANICS on signed overflow in debug and wraps in release, where C++ signed
// overflow is UB.  That divergence is LOUD (a panic, or the same wrap the C++
// implementation actually produces), never a silently different value -- and it
// is exactly the divergence the already-committed f2 (`a1 + a2` for std::plus)
// accepted, so this introduces no new class of risk.
unsafe fn f4<T1: std::ops::Mul<Output = T1> + Copy>(a0: (), a1: T1, a2: T1) -> T1 {
    a1 * a2
}

// t3 -- std::divides<T1>.  Same stateless-empty-type model.
fn t3<T1>() -> () {
    ()
}

unsafe fn f5<T1>() -> () {
    ()
}

// f6 -- divides::operator()(const T1&, const T1&) const.
//
// INTEGER DIVISION, DECIDED EXPLICITLY.  g472's instantiation is
// std::divides<long>, so this key WILL be used on integers, and there are two
// distinct questions:
//
//   1. ROUNDING.  Since C++11 ([expr.mul]/4) integer division TRUNCATES TOWARD
//      ZERO, and Rust's `/` on integers also truncates toward zero.  These
//      AGREE exactly, including for mixed signs (-7/2 == -3 in both).  There is
//      no rounding divergence, so `a1 / a2` is the honest translation and no
//      wrapper is warranted.
//   2. DIVISION BY ZERO.  C++ integer division by zero is UNDEFINED BEHAVIOUR;
//      Rust PANICS ("attempt to divide by zero").  This is a divergence, and I
//      am emitting the key anyway because the divergence is LOUD IN THE SAFE
//      DIRECTION: the C++ is UB (on x86-64 in practice SIGFPE, i.e. also a
//      crash), and the Rust is a defined panic.  A ported program cannot get a
//      plausible-but-wrong NUMBER out of this -- it gets a diagnosable abort --
//      so it is not an instance of the silent-wrongness class this project
//      refuses.  I am NOT emitting a checked_div/wrapping model, because any
//      such choice would have to invent a value for the zero case and THAT
//      would be the silently-wrong outcome.
//
//  For floating-point T1 division by zero yields inf/NaN in BOTH languages,
//  identically, with no panic.
unsafe fn f6<T1: std::ops::Div<Output = T1> + Copy>(a0: (), a1: T1, a2: T1) -> T1 {
    a1 / a2
}
