// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1<T1: Default, T2: Default>() -> (T1, T2) {
    <(T1, T2)>::default()
}

unsafe fn f1<T1, T2>(a0: (T1, T2)) -> T2 {
    a0.1
}
unsafe fn f3<T1: Default, T2: Default>() -> (T1, T2) {
    <(T1, T2)>::default()
}
unsafe fn f2<T1: Clone, T2: Clone>(a0: (T1, T2)) -> (T1, T2) {
    a0.clone()
}
unsafe fn f4<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}
unsafe fn f5<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}
unsafe fn f6<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}
unsafe fn f7<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}
unsafe fn f9<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}
unsafe fn f10<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}

unsafe fn f11<T1, T2>(a0: (T1, T2)) -> T1 {
    a0.0
}

unsafe fn f12<T1: Default, T2: Default>(a0: &mut (T1, T2)) -> (T1, T2) {
    std::mem::take(&mut *a0)
}

unsafe fn f13<T1: Clone, T2: Clone>(a0: &mut (T1, T2), a1: (T1, T2)) {
    *a0 = a1.clone()
}

unsafe fn f14<T1: Default, T2: Default>(a0: &mut (T1, T2), a1: &mut (T1, T2)) {
    *a0 = std::mem::take(&mut *a1)
}

unsafe fn f15<T1: PartialEq, T2: PartialEq>(a0: (T1, T2), a1: (T1, T2)) -> bool {
    a0 == a1
}

unsafe fn f16<T1: PartialEq, T2: PartialEq>(a0: (T1, T2), a1: (T1, T2)) -> bool {
    a0 != a1
}

unsafe fn f18<T1: TryFrom<T3>, T2: FromIterator<libc::c_char>, T3: Clone>(
    a0: T3,
    a1: &'static [u8],
) -> (T1, T2) {
    (
        T1::try_from(a0).ok().expect("failed conversion"),
        a1.iter()
            .map(|&__c| __c as libc::c_char)
            .chain(std::iter::once(0))
            .collect::<T2>(),
    )
}

unsafe fn f19<T1: FromIterator<libc::c_char>, T2: TryFrom<T3>, T3: Clone>(
    a0: &'static [u8],
    a1: T3,
) -> (T1, T2) {
    (
        a0.iter()
            .map(|&__c| __c as libc::c_char)
            .chain(std::iter::once(0))
            .collect::<T1>(),
        T2::try_from(a1).ok().expect("failed conversion"),
    )
}

unsafe fn f20<T1: FromIterator<libc::c_char>, T2: TryFrom<T3>, T3: Clone>(
    a0: &'static [u8],
    a1: T3,
) -> (T1, T2) {
    (
        a0.iter()
            .map(|&__c| __c as libc::c_char)
            .chain(std::iter::once(0))
            .collect::<T1>(),
        T2::try_from(a1).ok().expect("failed conversion"),
    )
}

unsafe fn f21<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}

unsafe fn f22<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}

unsafe fn f23<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}

unsafe fn f24<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}

unsafe fn f25<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}

unsafe fn f26<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}

unsafe fn f27<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}
