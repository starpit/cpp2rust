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

