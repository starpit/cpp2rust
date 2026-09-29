// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1<T1: Default, T2: Default>() -> (T1, T2) {
    Default::default()
}

fn t2<T1: Default, T2: Default, T3: Default>() -> (T1, T2, T3) {
    Default::default()
}

fn t3<T1: Default, T2: Default, T3: Default, T4: Default>() -> (T1, T2, T3, T4) {
    Default::default()
}

fn t4<T1: Default, T2: Default, T3: Default, T4: Default, T5: Default>() -> (T1, T2, T3, T4, T5) {
    Default::default()
}

fn f1<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0, a1)
}

fn f2<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0, a1, a2)
}

fn f3<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0, a1, a2, a3)
}

fn f4<T1, T2, T3, T4, T5>(a0: T1, a1: T2, a2: T3, a3: T4, a4: T5) -> (T1, T2, T3, T4, T5) {
    (a0, a1, a2, a3, a4)
}

fn t5<T1: Default, T2: Default, T3: Default, T4: Default, T5: Default, T6: Default, T7: Default, T8: Default, T9: Default, T10: Default, T11: Default, T12: Default, T13: Default, T14: Default, T15: Default, T16: Default, T17: Default, T18: Default, T19: Default, T20: Default, T21: Default, T22: Default, T23: Default, T24: Default, T25: Default, T26: Default, T27: Default>() -> (T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27) {
    (<T1>::default(), <T2>::default(), <T3>::default(), <T4>::default(), <T5>::default(), <T6>::default(), <T7>::default(), <T8>::default(), <T9>::default(), <T10>::default(), <T11>::default(), <T12>::default(), <T13>::default(), <T14>::default(), <T15>::default(), <T16>::default(), <T17>::default(), <T18>::default(), <T19>::default(), <T20>::default(), <T21>::default(), <T22>::default(), <T23>::default(), <T24>::default(), <T25>::default(), <T26>::default(), <T27>::default())
}

// g018: element-wise, because Rust std implements PartialEq for tuples only up to
// arity 12 -- `a0 == a1` on a 27-tuple is E0369.  Each operand is bound ONCE to a
// local (by reference, so the binding is correct whether the converter substitutes a
// value or a reference; field access auto-derefs either way) so that `a0`/`a1` are
// mentioned EXACTLY ONCE:
// the body is inlined as one expression, and the caller's operands here are the
// calls `tie()` / `A.tie()`, which 27 mentions would re-evaluate 27 times.
// Elements are `*const TN` because the src key binds `T1 = double` from a
// `std::tuple<const double &, ...>` (see src.cpp) and f6/`std::tie` produces exactly a
// tuple of raw const pointers.  So each comparison must DEREFERENCE both sides: `==` on
// the pointers themselves compares ADDRESSES, which is not what C++ `operator==` on a
// tuple of references does.  Operator form (`==`), matching every other body in this
// module; the whole conjunction is one `unsafe { }` block, as rules/atomic and
// rules/mutex do, because the inlining site is not guaranteed to be an `unsafe fn`.
fn f5<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq, T27: PartialEq>(a0: &(*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27), a1: &(*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27)) -> bool {
    let __x = &a0;
    let __y = &a1;
    unsafe { (*__x.0) == (*__y.0) && (*__x.1) == (*__y.1) && (*__x.2) == (*__y.2) && (*__x.3) == (*__y.3) && (*__x.4) == (*__y.4) && (*__x.5) == (*__y.5) && (*__x.6) == (*__y.6) && (*__x.7) == (*__y.7) && (*__x.8) == (*__y.8) && (*__x.9) == (*__y.9) && (*__x.10) == (*__y.10) && (*__x.11) == (*__y.11) && (*__x.12) == (*__y.12) && (*__x.13) == (*__y.13) && (*__x.14) == (*__y.14) && (*__x.15) == (*__y.15) && (*__x.16) == (*__y.16) && (*__x.17) == (*__y.17) && (*__x.18) == (*__y.18) && (*__x.19) == (*__y.19) && (*__x.20) == (*__y.20) && (*__x.21) == (*__y.21) && (*__x.22) == (*__y.22) && (*__x.23) == (*__y.23) && (*__x.24) == (*__y.24) && (*__x.25) == (*__y.25) && (*__x.26) == (*__y.26) }
}

// std::tie -> a tuple of RAW CONST POINTERS, one per element: the converter already
// maps `const double &` to `*const f64`, so the emitted `tie()` method's declared
// return type is `(*const f64, ...)` and this target must produce exactly that.
// A tuple of VALUES would be a silent copy where C++ referenced (and would not even
// typecheck against the mapped return type).  Each `aN` appears EXACTLY ONCE.
fn f6<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27>(a0: *const T1, a1: *const T2, a2: *const T3, a3: *const T4, a4: *const T5, a5: *const T6, a6: *const T7, a7: *const T8, a8: *const T9, a9: *const T10, a10: *const T11, a11: *const T12, a12: *const T13, a13: *const T14, a14: *const T15, a15: *const T16, a16: *const T17, a17: *const T18, a18: *const T19, a19: *const T20, a20: *const T21, a21: *const T22, a22: *const T23, a23: *const T24, a24: *const T25, a25: *const T26, a26: *const T27) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26)
}

// std::tie at arity 2/3/4 over NON-CONST lvalues -> a tuple of raw MUT pointers.
// `*mut TN`, not `*const TN`, and not a tuple of values: the converter maps a
// non-const `int &` to `*mut i32`, so the emitted declared type at
// probe/refbind/min.cpp:5 is literally `(*mut i32, *mut i32)` and the call site
// passes `&mut x`.  A tuple of values would be a silent COPY where C++ referred.
// Each `aN` appears EXACTLY ONCE, so nothing is cloned or moved twice.
fn f7<T1, T2>(a0: *mut T1, a1: *mut T2) -> (*mut T1, *mut T2) {
    (a0, a1)
}

fn f8<T1, T2, T3>(a0: *mut T1, a1: *mut T2, a2: *mut T3) -> (*mut T1, *mut T2, *mut T3) {
    (a0, a1, a2)
}

fn f9<T1, T2, T3, T4>(a0: *mut T1, a1: *mut T2, a2: *mut T3, a3: *mut T4) -> (*mut T1, *mut T2, *mut T3, *mut T4) {
    (a0, a1, a2, a3)
}

// arity-1 tuple: Rust's 1-tuple is `(T1,)` -- the trailing comma is load-bearing,
// `(T1)` is just T1 in parentheses and would silently erase the tuple.
fn t6<T1: Default>() -> (T1,) {
    Default::default()
}

// std::get<0>/std::get<1> -> a FIELD PROJECTION, returning a RAW POINTER, not a value.
// `const T1 &` maps to `*const T1` exactly as f6 documents, and the emitted call site
// already declares the hoisted local as `*const Vec<i64>` and DEREFERENCES it, so a
// by-value body would not typecheck AND would be a silent copy where C++ referred.
// `addr_of!` rather than `&(*a0).0 as *const _`: it forms the pointer without creating
// an intermediate Rust reference, which is what a C++ `const &` return into a
// possibly-unaligned/uniqued storage object needs. `a0` appears EXACTLY ONCE.
fn f10<T1, T2>(a0: *const (T1, T2)) -> *const T1 {
    unsafe { ::core::ptr::addr_of!((*a0).0) }
}

fn f11<T1, T2>(a0: *const (T1, T2)) -> *const T2 {
    unsafe { ::core::ptr::addr_of!((*a0).1) }
}
