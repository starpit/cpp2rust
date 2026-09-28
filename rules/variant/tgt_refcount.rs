// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Written EXPLICITLY: omitting this file does not drop the keys, it silently
// falls back to the unsafe bodies.  The representation is model-independent
// (a plain enum, no Ptr/Rc), so the value-taking bodies are identical; only
// the two by-reference index() bodies differ in how the receiver arrives.

use libcc2rs::{Variant2, Variant3, Variant8};

fn t1<T1: Default, T2>() -> Variant2<T1, T2> {
    Variant2::V0(T1::default())
}

fn t2<T1: Default, T2, T3>() -> Variant3<T1, T2, T3> {
    Variant3::V0(T1::default())
}

fn f1<T1: Default, T2>() -> Variant2<T1, T2> {
    Variant2::V0(T1::default())
}

fn f2<T1: Default, T2, T3>() -> Variant3<T1, T2, T3> {
    Variant3::V0(T1::default())
}

fn f3<T1, T2>(a0: T1) -> Variant2<T1, T2> {
    Variant2::V0(a0)
}

fn f4<T1, T2>(a0: T2) -> Variant2<T1, T2> {
    Variant2::V1(a0)
}

fn f5<T1, T2, T3>(a0: T1) -> Variant3<T1, T2, T3> {
    Variant3::V0(a0)
}

fn f6<T1, T2, T3>(a0: T2) -> Variant3<T1, T2, T3> {
    Variant3::V1(a0)
}

fn f7<T1, T2, T3>(a0: T3) -> Variant3<T1, T2, T3> {
    Variant3::V2(a0)
}

// index() is omitted here on purpose: refcount passes the receiver as
// `Ptr<Variant2<..>>`, and Ptr's accessors are all bounded on `T: ByteRepr`,
// which an enum carrying arbitrary alternatives cannot satisfy.  The unsafe
// body is used as the fallback and fails to compile under refcount (E0605)
// rather than returning a wrong index.

// ---- arity 8: the ONLY arity the corpus instantiates ---------------------
// `search` (mapper.cpp:383) tie-breaks on src.size() and prefers the LONGER
// src, so these eight-placeholder keys beat the 2-/3-ary ones on the 8-ary
// `OperandAttr::data_` deterministically.

fn t3<T1: Default, T2, T3, T4, T5, T6, T7, T8>() -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V0(T1::default())
}

fn f10<T1: Default, T2, T3, T4, T5, T6, T7, T8>() -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V0(T1::default())
}

fn f11<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T1) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V0(a0)
}

fn f12<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T2) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V1(a0)
}

fn f13<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T3) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V2(a0)
}

fn f14<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T4) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V3(a0)
}

fn f15<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T5) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V4(a0)
}

fn f16<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T6) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V5(a0)
}

fn f17<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T7) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V6(a0)
}

fn f18<T1, T2, T3, T4, T5, T6, T7, T8>(a0: T8) -> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    Variant8::V7(a0)
}

// f19 (index() on the 8-ary variant) is omitted here for the same reason f8/f9
// are: refcount passes the receiver as `Ptr<Variant8<..>>`, and Ptr's accessors
// are bounded on `T: ByteRepr`, which an enum carrying arbitrary alternatives
// cannot satisfy.  The unsafe body is the fallback and fails to COMPILE under
// refcount (E0605) rather than returning a wrong index.

// f20 (operator== on the 8-ary variant) is OMITTED here for exactly the reason
// f8/f9/f19 are: the refcount model passes a `const variant &` as
// `Ptr<Variant8<..>>`, and every Ptr accessor is bounded on `T: ByteRepr`, which
// an enum carrying arbitrary alternatives cannot satisfy.  Per the union rule in
// translation_rule.cpp:418-430 the key is still LOADED (tgt_unsafe.rs is the
// unconditional base layer), so this is not a missing-key abort; refcount simply
// inherits the unsafe body and fails to COMPILE there (E0605) rather than
// returning a wrong answer.  That is the same deliberate loud-failure choice the
// index() keys above make, and it is why no todo!()/unimplemented!() appears.
