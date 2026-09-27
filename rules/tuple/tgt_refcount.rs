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
    (T1::default(), T2::default(), T3::default(), T4::default(), T5::default(), T6::default(), T7::default(), T8::default(), T9::default(), T10::default(), T11::default(), T12::default(), T13::default(), T14::default(), T15::default(), T16::default(), T17::default(), T18::default(), T19::default(), T20::default(), T21::default(), T22::default(), T23::default(), T24::default(), T25::default(), T26::default(), T27::default())
}

// g018: element-wise, because Rust std implements PartialEq for tuples only up to
// arity 12 -- `a0 == a1` on a 27-tuple is E0369.  Each operand is bound ONCE to a
// local (by reference, so the binding is correct whether the converter substitutes a
// value or a reference; field access auto-derefs either way) so that `a0`/`a1` are
// mentioned EXACTLY ONCE:
// the body is inlined as one expression, and the caller's operands here are the
// calls `tie()` / `A.tie()`, which 27 mentions would re-evaluate 27 times.
fn f5<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq, T27: PartialEq>(a0: &(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27), a1: &(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27)) -> bool {
    let __x = &a0;
    let __y = &a1;
    __x.0 == __y.0 && __x.1 == __y.1 && __x.2 == __y.2 && __x.3 == __y.3 && __x.4 == __y.4 && __x.5 == __y.5 && __x.6 == __y.6 && __x.7 == __y.7 && __x.8 == __y.8 && __x.9 == __y.9 && __x.10 == __y.10 && __x.11 == __y.11 && __x.12 == __y.12 && __x.13 == __y.13 && __x.14 == __y.14 && __x.15 == __y.15 && __x.16 == __y.16 && __x.17 == __y.17 && __x.18 == __y.18 && __x.19 == __y.19 && __x.20 == __y.20 && __x.21 == __y.21 && __x.22 == __y.22 && __x.23 == __y.23 && __x.24 == __y.24 && __x.25 == __y.25 && __x.26 == __y.26
}
