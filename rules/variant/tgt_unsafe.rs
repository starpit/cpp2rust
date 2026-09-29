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

// ---- f21-f36 -- std::get<I> on the 8-ary variant --------------------------
// f21-f28: `const TN & std::get<N-1>(const variant<..8..> &)`, all eight
// alternatives.  f29-f36: the NON-CONST slice, `TN & std::get<N-1>(variant<..8..> &)`.
// The eight keys per slice are distinguished by the RETURN TYPE, which is what
// reaches the recorded key (the explicit index does not) -- see src.cpp.
// A wrong-alternative read PANICS, modelling `throw std::bad_variant_access`;
// with a real enum there is no inactive field that could be read instead.

unsafe fn f21<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *const Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *const T1 {
    unsafe {
        match &*a0 {
            Variant8::V0(v) => v as *const T1,
            _ => panic!("std::get: variant does not hold alternative 0 (std::bad_variant_access)"),
        }
    }
}

unsafe fn f22<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *const Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *const T2 {
    unsafe {
        match &*a0 {
            Variant8::V1(v) => v as *const T2,
            _ => panic!("std::get: variant does not hold alternative 1 (std::bad_variant_access)"),
        }
    }
}

unsafe fn f23<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *const Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *const T3 {
    unsafe {
        match &*a0 {
            Variant8::V2(v) => v as *const T3,
            _ => panic!("std::get: variant does not hold alternative 2 (std::bad_variant_access)"),
        }
    }
}

unsafe fn f24<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *const Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *const T4 {
    unsafe {
        match &*a0 {
            Variant8::V3(v) => v as *const T4,
            _ => panic!("std::get: variant does not hold alternative 3 (std::bad_variant_access)"),
        }
    }
}

unsafe fn f25<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *const Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *const T5 {
    unsafe {
        match &*a0 {
            Variant8::V4(v) => v as *const T5,
            _ => panic!("std::get: variant does not hold alternative 4 (std::bad_variant_access)"),
        }
    }
}

unsafe fn f26<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *const Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *const T6 {
    unsafe {
        match &*a0 {
            Variant8::V5(v) => v as *const T6,
            _ => panic!("std::get: variant does not hold alternative 5 (std::bad_variant_access)"),
        }
    }
}

unsafe fn f27<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *const Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *const T7 {
    unsafe {
        match &*a0 {
            Variant8::V6(v) => v as *const T7,
            _ => panic!("std::get: variant does not hold alternative 6 (std::bad_variant_access)"),
        }
    }
}

unsafe fn f28<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *const Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *const T8 {
    unsafe {
        match &*a0 {
            Variant8::V7(v) => v as *const T8,
            _ => panic!("std::get: variant does not hold alternative 7 (std::bad_variant_access)"),
        }
    }
}

unsafe fn f29<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *mut Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *mut T1 {
    unsafe {
        match &mut *a0 {
            Variant8::V0(v) => v as *mut T1,
            _ => panic!("std::get: variant does not hold alternative 0 (std::bad_variant_access)"),
        }
    }
}

unsafe fn f30<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *mut Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *mut T2 {
    unsafe {
        match &mut *a0 {
            Variant8::V1(v) => v as *mut T2,
            _ => panic!("std::get: variant does not hold alternative 1 (std::bad_variant_access)"),
        }
    }
}

unsafe fn f31<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *mut Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *mut T3 {
    unsafe {
        match &mut *a0 {
            Variant8::V2(v) => v as *mut T3,
            _ => panic!("std::get: variant does not hold alternative 2 (std::bad_variant_access)"),
        }
    }
}

unsafe fn f32<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *mut Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *mut T4 {
    unsafe {
        match &mut *a0 {
            Variant8::V3(v) => v as *mut T4,
            _ => panic!("std::get: variant does not hold alternative 3 (std::bad_variant_access)"),
        }
    }
}

unsafe fn f33<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *mut Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *mut T5 {
    unsafe {
        match &mut *a0 {
            Variant8::V4(v) => v as *mut T5,
            _ => panic!("std::get: variant does not hold alternative 4 (std::bad_variant_access)"),
        }
    }
}

unsafe fn f34<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *mut Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *mut T6 {
    unsafe {
        match &mut *a0 {
            Variant8::V5(v) => v as *mut T6,
            _ => panic!("std::get: variant does not hold alternative 5 (std::bad_variant_access)"),
        }
    }
}

unsafe fn f35<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *mut Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *mut T7 {
    unsafe {
        match &mut *a0 {
            Variant8::V6(v) => v as *mut T7,
            _ => panic!("std::get: variant does not hold alternative 6 (std::bad_variant_access)"),
        }
    }
}

unsafe fn f36<T1, T2, T3, T4, T5, T6, T7, T8>(a0: *mut Variant8<T1, T2, T3, T4, T5, T6, T7, T8>) -> *mut T8 {
    unsafe {
        match &mut *a0 {
            Variant8::V7(v) => v as *mut T8,
            _ => panic!("std::get: variant does not hold alternative 7 (std::bad_variant_access)"),
        }
    }
}
