// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.


fn t1<T1>() -> std::collections::BTreeSet<T1> {
    std::collections::BTreeSet::new()
}

unsafe fn f1<T1>() -> std::collections::BTreeSet<T1> {
    std::collections::BTreeSet::new()
}

unsafe fn f2<T1>(a0: std::collections::BTreeSet<T1>) -> usize {
    a0.len()
}

unsafe fn f3<T1>(a0: std::collections::BTreeSet<T1>) -> bool {
    a0.is_empty()
}

unsafe fn f4<T1>(a0: &mut std::collections::BTreeSet<T1>) {
    a0.clear()
}

unsafe fn f5<T1: Ord>(a0: std::collections::BTreeSet<T1>, a1: T1) -> usize {
    (std::collections::BTreeSet::contains(&a0, &a1) as usize)
}

unsafe fn f6<T1: Ord>(a0: &mut std::collections::BTreeSet<T1>, a1: T1) -> usize {
    (std::collections::BTreeSet::remove(&mut *a0, &a1) as usize)
}

unsafe fn f7<T1: PartialEq>(a0: &std::collections::BTreeSet<T1>, a1: &std::collections::BTreeSet<T1>) -> bool {
    a0 == a1
}

unsafe fn f8<T1: PartialEq>(a0: &std::collections::BTreeSet<T1>, a1: &std::collections::BTreeSet<T1>) -> bool {
    a0 != a1
}

unsafe fn f9<T1: Clone>(a0: std::collections::BTreeSet<T1>) -> std::collections::BTreeSet<T1> {
    a0.clone()
}

unsafe fn f10<T1>(a0: &mut std::collections::BTreeSet<T1>) -> std::collections::BTreeSet<T1> {
    std::mem::take(&mut *a0)
}

unsafe fn f11<T1: Clone>(a0: &mut std::collections::BTreeSet<T1>, a1: std::collections::BTreeSet<T1>) {
    *a0 = a1.clone()
}

unsafe fn f12<T1>(a0: &mut std::collections::BTreeSet<T1>, a1: &mut std::collections::BTreeSet<T1>) {
    *a0 = std::mem::take(&mut *a1)
}
