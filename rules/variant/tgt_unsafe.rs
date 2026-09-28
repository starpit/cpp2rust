// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::{Variant2, Variant3, Variant8};

fn t1<T1: Default, T2>() -> Variant2<T1, T2> {
    Variant2::V0(T1::default())
}

fn t2<T1: Default, T2, T3>() -> Variant3<T1, T2, T3> {
    Variant3::V0(T1::default())
}

unsafe fn f1<T1: Default, T2>() -> Variant2<T1, T2> {
    Variant2::V0(T1::default())
}

unsafe fn f2<T1: Default, T2, T3>() -> Variant3<T1, T2, T3> {
    Variant3::V0(T1::default())
}

unsafe fn f3<T1, T2>(a0: T1) -> Variant2<T1, T2> {
    Variant2::V0(a0)
}

unsafe fn f4<T1, T2>(a0: T2) -> Variant2<T1, T2> {
    Variant2::V1(a0)
}

unsafe fn f5<T1, T2, T3>(a0: T1) -> Variant3<T1, T2, T3> {
    Variant3::V0(a0)
}

unsafe fn f6<T1, T2, T3>(a0: T2) -> Variant3<T1, T2, T3> {
    Variant3::V1(a0)
}

unsafe fn f7<T1, T2, T3>(a0: T3) -> Variant3<T1, T2, T3> {
    Variant3::V2(a0)
}

unsafe fn f8<T1, T2>(a0: *const Variant2<T1, T2>) -> usize {
    unsafe { Variant2::index(&*a0) }
}

unsafe fn f9<T1, T2, T3>(a0: *const Variant3<T1, T2, T3>) -> usize {
    unsafe { Variant3::index(&*a0) }
}

// ---- arity 8: the ONLY arity the corpus instantiates ---------------------
// `search` (mapper.cpp:383) tie-breaks on src.size() and prefers the LONGER
// src, so these eight-placeholder keys beat the 2-/3-ary ones on the 8-ary
// `OperandAttr::data_` deterministically.

fn t3<T1: Default, T2, T3, T4, T5, T6, T7, T8>() -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V0(T1::default())
}

unsafe fn f10<T1: Default, T2, T3, T4, T5, T6, T7, T8>() -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V0(T1::default())
}

unsafe fn f11<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T1) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V0(a0)
}

unsafe fn f12<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T2) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V1(a0)
}

unsafe fn f13<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T3) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V2(a0)
}

unsafe fn f14<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T4) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V3(a0)
}

unsafe fn f15<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T5) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V4(a0)
}

unsafe fn f16<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T6) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V5(a0)
}

unsafe fn f17<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T7) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V6(a0)
}

unsafe fn f18<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T8) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V7(a0)
}

unsafe fn f19<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *const Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> usize {
    unsafe { Variant8::index(&*a0) }
}

// f20 -- operator== on two 8-ary variants.  `Variant8` derives PartialEq, whose
// enum implementation compares the DISCRIMINANT (the active alternative) before
// the payload, which is exactly std::variant's rule.  Every alternative must be
// comparable, hence the eight bounds -- that mirrors C++, where variant's
// operator== is only viable when all alternatives are equality-comparable.
unsafe fn f20<
    T1: PartialEq,
    T2: PartialEq,
    T3: PartialEq,
    T4: PartialEq,
    T5: PartialEq,
    T6: PartialEq,
    T7: PartialEq,
    T8: PartialEq,
>(
    a0: *const Variant8<T1, T2, T3, T4, T5, T6, T7, T8>,
    a1: *const Variant8<T1, T2, T3, T4, T5, T6, T7, T8>,
) -> bool {
    unsafe { *a0 == *a1 }
}
