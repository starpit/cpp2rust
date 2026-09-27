// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Written EXPLICITLY: omitting this file does not drop the keys, it silently
// falls back to the unsafe bodies.  The representation is model-independent
// (a plain enum, no Ptr/Rc), so the value-taking bodies are identical; only
// the two by-reference index() bodies differ in how the receiver arrives.

use libcc2rs::{Variant2, Variant3};

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
