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
