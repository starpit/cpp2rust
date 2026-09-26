// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1<T1: Default, T2: Default>() -> (T1, T2) {
    <(T1, T2)>::default()
}

fn t2<T1: Default, T2: Default, T3: Default>() -> (T1, T2, T3) {
    <(T1, T2, T3)>::default()
}

fn t3<T1: Default, T2: Default, T3: Default, T4: Default>() -> (T1, T2, T3, T4) {
    <(T1, T2, T3, T4)>::default()
}

// --- 2-element tuples -------------------------------------------------------

unsafe fn f1<T1: Default, T2: Default>() -> (T1, T2) {
    <(T1, T2)>::default()
}

unsafe fn f2<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}

unsafe fn f3<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
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

unsafe fn f8<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}

unsafe fn f9<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}

unsafe fn f10<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}

unsafe fn f11<T1: Clone, T2: Clone>(a0: (T1, T2)) -> (T1, T2) {
    a0.clone()
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

unsafe fn f15<T1, T2>(a0: *mut (T1, T2)) -> *mut T1 {
    &raw mut (*a0).0
}

unsafe fn f16<T1, T2>(a0: *mut (T1, T2)) -> *mut T2 {
    &raw mut (*a0).1
}

unsafe fn f17<T1, T2>(a0: *const (T1, T2)) -> *const T1 {
    &raw const (*a0).0
}

unsafe fn f18<T1, T2>(a0: *const (T1, T2)) -> *const T2 {
    &raw const (*a0).1
}

unsafe fn f19<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
}

unsafe fn f20<T1, T2>(a0: T1, a1: T2) -> (T1, T2) {
    (a0.into(), a1.into())
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

unsafe fn f28<T1: PartialEq, T2: PartialEq>(a0: (T1, T2), a1: (T1, T2)) -> bool {
    a0 == a1
}

unsafe fn f29<T1: PartialEq, T2: PartialEq>(a0: (T1, T2), a1: (T1, T2)) -> bool {
    a0 != a1
}

// --- 3-element tuples -------------------------------------------------------

unsafe fn f30<T1: Default, T2: Default, T3: Default>() -> (T1, T2, T3) {
    <(T1, T2, T3)>::default()
}

unsafe fn f31<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f32<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f33<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f34<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f35<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f36<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f37<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f38<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f39<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f40<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f41<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f42<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f43<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f44<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f45<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f46<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f47<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f48<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f49<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f50<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f51<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f52<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f53<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f54<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f55<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f56<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f57<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f58<T1: Clone, T2: Clone, T3: Clone>(a0: (T1, T2, T3)) -> (T1, T2, T3) {
    a0.clone()
}

unsafe fn f59<T1: Default, T2: Default, T3: Default>(a0: &mut (T1, T2, T3)) -> (T1, T2, T3) {
    std::mem::take(&mut *a0)
}

unsafe fn f60<T1: Clone, T2: Clone, T3: Clone>(a0: &mut (T1, T2, T3), a1: (T1, T2, T3)) {
    *a0 = a1.clone()
}

unsafe fn f61<T1: Default, T2: Default, T3: Default>(a0: &mut (T1, T2, T3), a1: &mut (T1, T2, T3)) {
    *a0 = std::mem::take(&mut *a1)
}

unsafe fn f62<T1, T2, T3>(a0: *mut (T1, T2, T3)) -> *mut T1 {
    &raw mut (*a0).0
}

unsafe fn f63<T1, T2, T3>(a0: *mut (T1, T2, T3)) -> *mut T2 {
    &raw mut (*a0).1
}

unsafe fn f64<T1, T2, T3>(a0: *mut (T1, T2, T3)) -> *mut T3 {
    &raw mut (*a0).2
}

unsafe fn f65<T1, T2, T3>(a0: *const (T1, T2, T3)) -> *const T1 {
    &raw const (*a0).0
}

unsafe fn f66<T1, T2, T3>(a0: *const (T1, T2, T3)) -> *const T2 {
    &raw const (*a0).1
}

unsafe fn f67<T1, T2, T3>(a0: *const (T1, T2, T3)) -> *const T3 {
    &raw const (*a0).2
}

unsafe fn f68<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f69<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f70<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f71<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f72<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f73<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f74<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f75<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f76<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f77<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f78<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f79<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f80<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f81<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f82<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f83<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f84<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f85<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f86<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f87<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f88<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f89<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f90<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f91<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f92<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f93<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f94<T1, T2, T3>(a0: T1, a1: T2, a2: T3) -> (T1, T2, T3) {
    (a0.into(), a1.into(), a2.into())
}

unsafe fn f95<T1: PartialEq, T2: PartialEq, T3: PartialEq>(a0: (T1, T2, T3), a1: (T1, T2, T3)) -> bool {
    a0 == a1
}

unsafe fn f96<T1: PartialEq, T2: PartialEq, T3: PartialEq>(a0: (T1, T2, T3), a1: (T1, T2, T3)) -> bool {
    a0 != a1
}

// --- 4-element tuples -------------------------------------------------------

unsafe fn f97<T1: Default, T2: Default, T3: Default, T4: Default>() -> (T1, T2, T3, T4) {
    <(T1, T2, T3, T4)>::default()
}

unsafe fn f98<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f99<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f100<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f101<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f102<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f103<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f104<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f105<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f106<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f107<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f108<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f109<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f110<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f111<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f112<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f113<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f114<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f115<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f116<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f117<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f118<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f119<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f120<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f121<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f122<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f123<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f124<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f125<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f126<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f127<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f128<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f129<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f130<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f131<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f132<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f133<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f134<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f135<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f136<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f137<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f138<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f139<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f140<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f141<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f142<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f143<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f144<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f145<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f146<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f147<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f148<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f149<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f150<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f151<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f152<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f153<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f154<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f155<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f156<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f157<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f158<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f159<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f160<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f161<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f162<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f163<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f164<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f165<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f166<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f167<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f168<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f169<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f170<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f171<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f172<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f173<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f174<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f175<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f176<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f177<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f178<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f179<T1: Clone, T2: Clone, T3: Clone, T4: Clone>(a0: (T1, T2, T3, T4)) -> (T1, T2, T3, T4) {
    a0.clone()
}

unsafe fn f180<T1: Default, T2: Default, T3: Default, T4: Default>(a0: &mut (T1, T2, T3, T4)) -> (T1, T2, T3, T4) {
    std::mem::take(&mut *a0)
}

unsafe fn f181<T1: Clone, T2: Clone, T3: Clone, T4: Clone>(a0: &mut (T1, T2, T3, T4), a1: (T1, T2, T3, T4)) {
    *a0 = a1.clone()
}

unsafe fn f182<T1: Default, T2: Default, T3: Default, T4: Default>(a0: &mut (T1, T2, T3, T4), a1: &mut (T1, T2, T3, T4)) {
    *a0 = std::mem::take(&mut *a1)
}

unsafe fn f183<T1, T2, T3, T4>(a0: *mut (T1, T2, T3, T4)) -> *mut T1 {
    &raw mut (*a0).0
}

unsafe fn f184<T1, T2, T3, T4>(a0: *mut (T1, T2, T3, T4)) -> *mut T2 {
    &raw mut (*a0).1
}

unsafe fn f185<T1, T2, T3, T4>(a0: *mut (T1, T2, T3, T4)) -> *mut T3 {
    &raw mut (*a0).2
}

unsafe fn f186<T1, T2, T3, T4>(a0: *mut (T1, T2, T3, T4)) -> *mut T4 {
    &raw mut (*a0).3
}

unsafe fn f187<T1, T2, T3, T4>(a0: *const (T1, T2, T3, T4)) -> *const T1 {
    &raw const (*a0).0
}

unsafe fn f188<T1, T2, T3, T4>(a0: *const (T1, T2, T3, T4)) -> *const T2 {
    &raw const (*a0).1
}

unsafe fn f189<T1, T2, T3, T4>(a0: *const (T1, T2, T3, T4)) -> *const T3 {
    &raw const (*a0).2
}

unsafe fn f190<T1, T2, T3, T4>(a0: *const (T1, T2, T3, T4)) -> *const T4 {
    &raw const (*a0).3
}

unsafe fn f191<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f192<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f193<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f194<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f195<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f196<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f197<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f198<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f199<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f200<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f201<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f202<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f203<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f204<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f205<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f206<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f207<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f208<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f209<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f210<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f211<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f212<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f213<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f214<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f215<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f216<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f217<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f218<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f219<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f220<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f221<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f222<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f223<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f224<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f225<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f226<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f227<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f228<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f229<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f230<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f231<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f232<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f233<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f234<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f235<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f236<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f237<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f238<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f239<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f240<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f241<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f242<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f243<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f244<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f245<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f246<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f247<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f248<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f249<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f250<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f251<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f252<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f253<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f254<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f255<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f256<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f257<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f258<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f259<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f260<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f261<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f262<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f263<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f264<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f265<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f266<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f267<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f268<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f269<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f270<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f271<T1, T2, T3, T4>(a0: T1, a1: T2, a2: T3, a3: T4) -> (T1, T2, T3, T4) {
    (a0.into(), a1.into(), a2.into(), a3.into())
}

unsafe fn f272<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq>(a0: (T1, T2, T3, T4), a1: (T1, T2, T3, T4)) -> bool {
    a0 == a1
}

unsafe fn f273<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq>(a0: (T1, T2, T3, T4), a1: (T1, T2, T3, T4)) -> bool {
    a0 != a1
}

// --- std::tie, 2 elements ---------------------------------------------------

fn t4<T1, T2>() -> (*mut T1, *mut T2) {
    (std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f274<T1, T2>(a0: *mut T1, a1: *mut T2) -> (*mut T1, *mut T2) {
    (a0, a1)
}

unsafe fn f275<T1: Clone, T2: Clone>(a0: &mut (*mut T1, *mut T2), a1: (T1, T2)) {
    *a0.0 = a1.0.clone();
    *a0.1 = a1.1.clone();
}

unsafe fn f276<T1, T2>(a0: (*mut T1, *mut T2)) -> *mut T1 {
    a0.0
}

unsafe fn f277<T1, T2>(a0: (*mut T1, *mut T2)) -> *mut T2 {
    a0.1
}

// --- std::tie, 3 elements ---------------------------------------------------

fn t5<T1, T2, T3>() -> (*mut T1, *mut T2, *mut T3) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f278<T1, T2, T3>(a0: *mut T1, a1: *mut T2, a2: *mut T3) -> (*mut T1, *mut T2, *mut T3) {
    (a0, a1, a2)
}

unsafe fn f279<T1: Clone, T2: Clone, T3: Clone>(a0: &mut (*mut T1, *mut T2, *mut T3), a1: (T1, T2, T3)) {
    *a0.0 = a1.0.clone();
    *a0.1 = a1.1.clone();
    *a0.2 = a1.2.clone();
}

unsafe fn f280<T1, T2, T3>(a0: (*mut T1, *mut T2, *mut T3)) -> *mut T1 {
    a0.0
}

unsafe fn f281<T1, T2, T3>(a0: (*mut T1, *mut T2, *mut T3)) -> *mut T2 {
    a0.1
}

unsafe fn f282<T1, T2, T3>(a0: (*mut T1, *mut T2, *mut T3)) -> *mut T3 {
    a0.2
}

// --- std::tie, 4 elements ---------------------------------------------------

fn t6<T1, T2, T3, T4>() -> (*mut T1, *mut T2, *mut T3, *mut T4) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f283<T1, T2, T3, T4>(a0: *mut T1, a1: *mut T2, a2: *mut T3, a3: *mut T4) -> (*mut T1, *mut T2, *mut T3, *mut T4) {
    (a0, a1, a2, a3)
}

unsafe fn f284<T1: Clone, T2: Clone, T3: Clone, T4: Clone>(a0: &mut (*mut T1, *mut T2, *mut T3, *mut T4), a1: (T1, T2, T3, T4)) {
    *a0.0 = a1.0.clone();
    *a0.1 = a1.1.clone();
    *a0.2 = a1.2.clone();
    *a0.3 = a1.3.clone();
}

unsafe fn f285<T1, T2, T3, T4>(a0: (*mut T1, *mut T2, *mut T3, *mut T4)) -> *mut T1 {
    a0.0
}

unsafe fn f286<T1, T2, T3, T4>(a0: (*mut T1, *mut T2, *mut T3, *mut T4)) -> *mut T2 {
    a0.1
}

unsafe fn f287<T1, T2, T3, T4>(a0: (*mut T1, *mut T2, *mut T3, *mut T4)) -> *mut T3 {
    a0.2
}

unsafe fn f288<T1, T2, T3, T4>(a0: (*mut T1, *mut T2, *mut T3, *mut T4)) -> *mut T4 {
    a0.3
}

// ==== high-arity extension (generated by /tmp/gen_tuple_rules_highN.py) ====

// --- std::tie comparison, 2 elements ----------------------------------------

unsafe fn f289<T1: PartialEq, T2: PartialEq>(
    a0: (*mut T1, *mut T2),
    a1: (*mut T1, *mut T2),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1)
}

unsafe fn f290<T1: PartialEq, T2: PartialEq>(
    a0: (*mut T1, *mut T2),
    a1: (*mut T1, *mut T2),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1)
}

// --- std::tie comparison, 3 elements ----------------------------------------

unsafe fn f291<T1: PartialEq, T2: PartialEq, T3: PartialEq>(
    a0: (*mut T1, *mut T2, *mut T3),
    a1: (*mut T1, *mut T2, *mut T3),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2)
}

unsafe fn f292<T1: PartialEq, T2: PartialEq, T3: PartialEq>(
    a0: (*mut T1, *mut T2, *mut T3),
    a1: (*mut T1, *mut T2, *mut T3),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2)
}

// --- std::tie comparison, 4 elements ----------------------------------------

unsafe fn f293<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq>(
    a0: (*mut T1, *mut T2, *mut T3, *mut T4),
    a1: (*mut T1, *mut T2, *mut T3, *mut T4),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3)
}

unsafe fn f294<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq>(
    a0: (*mut T1, *mut T2, *mut T3, *mut T4),
    a1: (*mut T1, *mut T2, *mut T3, *mut T4),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3)
}

// --- std::tie, 5 elements ---------------------------------------------------

fn t7<T1, T2, T3, T4, T5>() -> (*const T1, *const T2, *const T3, *const T4, *const T5) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f295<T1, T2, T3, T4, T5>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5) {
    (a0, a1, a2, a3, a4)
}

unsafe fn f296<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4)
}

unsafe fn f297<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4)
}

// --- std::tie, 6 elements ---------------------------------------------------

fn t8<T1, T2, T3, T4, T5, T6>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f298<T1, T2, T3, T4, T5, T6>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6) {
    (a0, a1, a2, a3, a4, a5)
}

unsafe fn f299<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5)
}

unsafe fn f300<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5)
}

// --- std::tie, 7 elements ---------------------------------------------------

fn t9<T1, T2, T3, T4, T5, T6, T7>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f301<T1, T2, T3, T4, T5, T6, T7>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7) {
    (a0, a1, a2, a3, a4, a5, a6)
}

unsafe fn f302<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6)
}

unsafe fn f303<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6)
}

// --- std::tie, 8 elements ---------------------------------------------------

fn t10<T1, T2, T3, T4, T5, T6, T7, T8>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f304<T1, T2, T3, T4, T5, T6, T7, T8>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8) {
    (a0, a1, a2, a3, a4, a5, a6, a7)
}

unsafe fn f305<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7)
}

unsafe fn f306<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7)
}

// --- std::tie, 9 elements ---------------------------------------------------

fn t11<T1, T2, T3, T4, T5, T6, T7, T8, T9>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f307<T1, T2, T3, T4, T5, T6, T7, T8, T9>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8)
}

unsafe fn f308<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8)
}

unsafe fn f309<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8)
}

// --- std::tie, 10 elements --------------------------------------------------

fn t12<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f310<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9)
}

unsafe fn f311<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9)
}

unsafe fn f312<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9)
}

// --- std::tie, 11 elements --------------------------------------------------

fn t13<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f313<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
}

unsafe fn f314<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10)
}

unsafe fn f315<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10)
}

// --- std::tie, 12 elements --------------------------------------------------

fn t14<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f316<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11)
}

unsafe fn f317<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11)
}

unsafe fn f318<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11)
}

// --- std::tie, 13 elements --------------------------------------------------

fn t15<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f319<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12)
}

unsafe fn f320<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12)
}

unsafe fn f321<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12)
}

// --- std::tie, 14 elements --------------------------------------------------

fn t16<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f322<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13)
}

unsafe fn f323<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13)
}

unsafe fn f324<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13)
}

// --- std::tie, 15 elements --------------------------------------------------

fn t17<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f325<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14)
}

unsafe fn f326<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14)
}

unsafe fn f327<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14)
}

// --- std::tie, 16 elements --------------------------------------------------

fn t18<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f328<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15)
}

unsafe fn f329<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15)
}

unsafe fn f330<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15)
}

// --- std::tie, 17 elements --------------------------------------------------

fn t19<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f331<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16)
}

unsafe fn f332<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16)
}

unsafe fn f333<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16)
}

// --- std::tie, 18 elements --------------------------------------------------

fn t20<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f334<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
    a17: *const T18,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17)
}

unsafe fn f335<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16) &&
        (*a0.17 == *a1.17)
}

unsafe fn f336<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16) ||
        (*a0.17 != *a1.17)
}

// --- std::tie, 19 elements --------------------------------------------------

fn t21<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f337<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
    a17: *const T18,
    a18: *const T19,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18)
}

unsafe fn f338<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16) &&
        (*a0.17 == *a1.17) &&
        (*a0.18 == *a1.18)
}

unsafe fn f339<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16) ||
        (*a0.17 != *a1.17) ||
        (*a0.18 != *a1.18)
}

// --- std::tie, 20 elements --------------------------------------------------

fn t22<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f340<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
    a17: *const T18,
    a18: *const T19,
    a19: *const T20,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19)
}

unsafe fn f341<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16) &&
        (*a0.17 == *a1.17) &&
        (*a0.18 == *a1.18) &&
        (*a0.19 == *a1.19)
}

unsafe fn f342<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16) ||
        (*a0.17 != *a1.17) ||
        (*a0.18 != *a1.18) ||
        (*a0.19 != *a1.19)
}

// --- std::tie, 21 elements --------------------------------------------------

fn t23<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f343<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
    a17: *const T18,
    a18: *const T19,
    a19: *const T20,
    a20: *const T21,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20)
}

unsafe fn f344<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16) &&
        (*a0.17 == *a1.17) &&
        (*a0.18 == *a1.18) &&
        (*a0.19 == *a1.19) &&
        (*a0.20 == *a1.20)
}

unsafe fn f345<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16) ||
        (*a0.17 != *a1.17) ||
        (*a0.18 != *a1.18) ||
        (*a0.19 != *a1.19) ||
        (*a0.20 != *a1.20)
}

// --- std::tie, 22 elements --------------------------------------------------

fn t24<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f346<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
    a17: *const T18,
    a18: *const T19,
    a19: *const T20,
    a20: *const T21,
    a21: *const T22,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21)
}

unsafe fn f347<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16) &&
        (*a0.17 == *a1.17) &&
        (*a0.18 == *a1.18) &&
        (*a0.19 == *a1.19) &&
        (*a0.20 == *a1.20) &&
        (*a0.21 == *a1.21)
}

unsafe fn f348<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16) ||
        (*a0.17 != *a1.17) ||
        (*a0.18 != *a1.18) ||
        (*a0.19 != *a1.19) ||
        (*a0.20 != *a1.20) ||
        (*a0.21 != *a1.21)
}

// --- std::tie, 23 elements --------------------------------------------------

fn t25<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f349<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
    a17: *const T18,
    a18: *const T19,
    a19: *const T20,
    a20: *const T21,
    a21: *const T22,
    a22: *const T23,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22)
}

unsafe fn f350<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16) &&
        (*a0.17 == *a1.17) &&
        (*a0.18 == *a1.18) &&
        (*a0.19 == *a1.19) &&
        (*a0.20 == *a1.20) &&
        (*a0.21 == *a1.21) &&
        (*a0.22 == *a1.22)
}

unsafe fn f351<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16) ||
        (*a0.17 != *a1.17) ||
        (*a0.18 != *a1.18) ||
        (*a0.19 != *a1.19) ||
        (*a0.20 != *a1.20) ||
        (*a0.21 != *a1.21) ||
        (*a0.22 != *a1.22)
}

// --- std::tie, 24 elements --------------------------------------------------

fn t26<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f352<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
    a17: *const T18,
    a18: *const T19,
    a19: *const T20,
    a20: *const T21,
    a21: *const T22,
    a22: *const T23,
    a23: *const T24,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23)
}

unsafe fn f353<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16) &&
        (*a0.17 == *a1.17) &&
        (*a0.18 == *a1.18) &&
        (*a0.19 == *a1.19) &&
        (*a0.20 == *a1.20) &&
        (*a0.21 == *a1.21) &&
        (*a0.22 == *a1.22) &&
        (*a0.23 == *a1.23)
}

unsafe fn f354<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16) ||
        (*a0.17 != *a1.17) ||
        (*a0.18 != *a1.18) ||
        (*a0.19 != *a1.19) ||
        (*a0.20 != *a1.20) ||
        (*a0.21 != *a1.21) ||
        (*a0.22 != *a1.22) ||
        (*a0.23 != *a1.23)
}

// --- std::tie, 25 elements --------------------------------------------------

fn t27<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f355<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
    a17: *const T18,
    a18: *const T19,
    a19: *const T20,
    a20: *const T21,
    a21: *const T22,
    a22: *const T23,
    a23: *const T24,
    a24: *const T25,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24)
}

unsafe fn f356<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16) &&
        (*a0.17 == *a1.17) &&
        (*a0.18 == *a1.18) &&
        (*a0.19 == *a1.19) &&
        (*a0.20 == *a1.20) &&
        (*a0.21 == *a1.21) &&
        (*a0.22 == *a1.22) &&
        (*a0.23 == *a1.23) &&
        (*a0.24 == *a1.24)
}

unsafe fn f357<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16) ||
        (*a0.17 != *a1.17) ||
        (*a0.18 != *a1.18) ||
        (*a0.19 != *a1.19) ||
        (*a0.20 != *a1.20) ||
        (*a0.21 != *a1.21) ||
        (*a0.22 != *a1.22) ||
        (*a0.23 != *a1.23) ||
        (*a0.24 != *a1.24)
}

// --- std::tie, 26 elements --------------------------------------------------

fn t28<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f358<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
    a17: *const T18,
    a18: *const T19,
    a19: *const T20,
    a20: *const T21,
    a21: *const T22,
    a22: *const T23,
    a23: *const T24,
    a24: *const T25,
    a25: *const T26,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25)
}

unsafe fn f359<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16) &&
        (*a0.17 == *a1.17) &&
        (*a0.18 == *a1.18) &&
        (*a0.19 == *a1.19) &&
        (*a0.20 == *a1.20) &&
        (*a0.21 == *a1.21) &&
        (*a0.22 == *a1.22) &&
        (*a0.23 == *a1.23) &&
        (*a0.24 == *a1.24) &&
        (*a0.25 == *a1.25)
}

unsafe fn f360<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16) ||
        (*a0.17 != *a1.17) ||
        (*a0.18 != *a1.18) ||
        (*a0.19 != *a1.19) ||
        (*a0.20 != *a1.20) ||
        (*a0.21 != *a1.21) ||
        (*a0.22 != *a1.22) ||
        (*a0.23 != *a1.23) ||
        (*a0.24 != *a1.24) ||
        (*a0.25 != *a1.25)
}

// --- std::tie, 27 elements --------------------------------------------------

fn t29<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f361<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
    a17: *const T18,
    a18: *const T19,
    a19: *const T20,
    a20: *const T21,
    a21: *const T22,
    a22: *const T23,
    a23: *const T24,
    a24: *const T25,
    a25: *const T26,
    a26: *const T27,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26)
}

unsafe fn f362<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq, T27: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16) &&
        (*a0.17 == *a1.17) &&
        (*a0.18 == *a1.18) &&
        (*a0.19 == *a1.19) &&
        (*a0.20 == *a1.20) &&
        (*a0.21 == *a1.21) &&
        (*a0.22 == *a1.22) &&
        (*a0.23 == *a1.23) &&
        (*a0.24 == *a1.24) &&
        (*a0.25 == *a1.25) &&
        (*a0.26 == *a1.26)
}

unsafe fn f363<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq, T27: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16) ||
        (*a0.17 != *a1.17) ||
        (*a0.18 != *a1.18) ||
        (*a0.19 != *a1.19) ||
        (*a0.20 != *a1.20) ||
        (*a0.21 != *a1.21) ||
        (*a0.22 != *a1.22) ||
        (*a0.23 != *a1.23) ||
        (*a0.24 != *a1.24) ||
        (*a0.25 != *a1.25) ||
        (*a0.26 != *a1.26)
}

// --- std::tie, 28 elements --------------------------------------------------

fn t30<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f364<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
    a17: *const T18,
    a18: *const T19,
    a19: *const T20,
    a20: *const T21,
    a21: *const T22,
    a22: *const T23,
    a23: *const T24,
    a24: *const T25,
    a25: *const T26,
    a26: *const T27,
    a27: *const T28,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27)
}

unsafe fn f365<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq, T27: PartialEq, T28: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16) &&
        (*a0.17 == *a1.17) &&
        (*a0.18 == *a1.18) &&
        (*a0.19 == *a1.19) &&
        (*a0.20 == *a1.20) &&
        (*a0.21 == *a1.21) &&
        (*a0.22 == *a1.22) &&
        (*a0.23 == *a1.23) &&
        (*a0.24 == *a1.24) &&
        (*a0.25 == *a1.25) &&
        (*a0.26 == *a1.26) &&
        (*a0.27 == *a1.27)
}

unsafe fn f366<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq, T27: PartialEq, T28: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16) ||
        (*a0.17 != *a1.17) ||
        (*a0.18 != *a1.18) ||
        (*a0.19 != *a1.19) ||
        (*a0.20 != *a1.20) ||
        (*a0.21 != *a1.21) ||
        (*a0.22 != *a1.22) ||
        (*a0.23 != *a1.23) ||
        (*a0.24 != *a1.24) ||
        (*a0.25 != *a1.25) ||
        (*a0.26 != *a1.26) ||
        (*a0.27 != *a1.27)
}

// --- std::tie, 29 elements --------------------------------------------------

fn t31<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f367<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
    a17: *const T18,
    a18: *const T19,
    a19: *const T20,
    a20: *const T21,
    a21: *const T22,
    a22: *const T23,
    a23: *const T24,
    a24: *const T25,
    a25: *const T26,
    a26: *const T27,
    a27: *const T28,
    a28: *const T29,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28)
}

unsafe fn f368<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq, T27: PartialEq, T28: PartialEq, T29: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16) &&
        (*a0.17 == *a1.17) &&
        (*a0.18 == *a1.18) &&
        (*a0.19 == *a1.19) &&
        (*a0.20 == *a1.20) &&
        (*a0.21 == *a1.21) &&
        (*a0.22 == *a1.22) &&
        (*a0.23 == *a1.23) &&
        (*a0.24 == *a1.24) &&
        (*a0.25 == *a1.25) &&
        (*a0.26 == *a1.26) &&
        (*a0.27 == *a1.27) &&
        (*a0.28 == *a1.28)
}

unsafe fn f369<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq, T27: PartialEq, T28: PartialEq, T29: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16) ||
        (*a0.17 != *a1.17) ||
        (*a0.18 != *a1.18) ||
        (*a0.19 != *a1.19) ||
        (*a0.20 != *a1.20) ||
        (*a0.21 != *a1.21) ||
        (*a0.22 != *a1.22) ||
        (*a0.23 != *a1.23) ||
        (*a0.24 != *a1.24) ||
        (*a0.25 != *a1.25) ||
        (*a0.26 != *a1.26) ||
        (*a0.27 != *a1.27) ||
        (*a0.28 != *a1.28)
}

// --- std::tie, 30 elements --------------------------------------------------

fn t32<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f370<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
    a17: *const T18,
    a18: *const T19,
    a19: *const T20,
    a20: *const T21,
    a21: *const T22,
    a22: *const T23,
    a23: *const T24,
    a24: *const T25,
    a25: *const T26,
    a26: *const T27,
    a27: *const T28,
    a28: *const T29,
    a29: *const T30,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29)
}

unsafe fn f371<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq, T27: PartialEq, T28: PartialEq, T29: PartialEq, T30: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16) &&
        (*a0.17 == *a1.17) &&
        (*a0.18 == *a1.18) &&
        (*a0.19 == *a1.19) &&
        (*a0.20 == *a1.20) &&
        (*a0.21 == *a1.21) &&
        (*a0.22 == *a1.22) &&
        (*a0.23 == *a1.23) &&
        (*a0.24 == *a1.24) &&
        (*a0.25 == *a1.25) &&
        (*a0.26 == *a1.26) &&
        (*a0.27 == *a1.27) &&
        (*a0.28 == *a1.28) &&
        (*a0.29 == *a1.29)
}

unsafe fn f372<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq, T27: PartialEq, T28: PartialEq, T29: PartialEq, T30: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16) ||
        (*a0.17 != *a1.17) ||
        (*a0.18 != *a1.18) ||
        (*a0.19 != *a1.19) ||
        (*a0.20 != *a1.20) ||
        (*a0.21 != *a1.21) ||
        (*a0.22 != *a1.22) ||
        (*a0.23 != *a1.23) ||
        (*a0.24 != *a1.24) ||
        (*a0.25 != *a1.25) ||
        (*a0.26 != *a1.26) ||
        (*a0.27 != *a1.27) ||
        (*a0.28 != *a1.28) ||
        (*a0.29 != *a1.29)
}

// --- std::tie, 31 elements --------------------------------------------------

fn t33<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30, *const T31) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f373<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
    a17: *const T18,
    a18: *const T19,
    a19: *const T20,
    a20: *const T21,
    a21: *const T22,
    a22: *const T23,
    a23: *const T24,
    a24: *const T25,
    a25: *const T26,
    a26: *const T27,
    a27: *const T28,
    a28: *const T29,
    a29: *const T30,
    a30: *const T31,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30, *const T31) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30)
}

unsafe fn f374<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq, T27: PartialEq, T28: PartialEq, T29: PartialEq, T30: PartialEq, T31: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30, *const T31),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30, *const T31),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16) &&
        (*a0.17 == *a1.17) &&
        (*a0.18 == *a1.18) &&
        (*a0.19 == *a1.19) &&
        (*a0.20 == *a1.20) &&
        (*a0.21 == *a1.21) &&
        (*a0.22 == *a1.22) &&
        (*a0.23 == *a1.23) &&
        (*a0.24 == *a1.24) &&
        (*a0.25 == *a1.25) &&
        (*a0.26 == *a1.26) &&
        (*a0.27 == *a1.27) &&
        (*a0.28 == *a1.28) &&
        (*a0.29 == *a1.29) &&
        (*a0.30 == *a1.30)
}

unsafe fn f375<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq, T27: PartialEq, T28: PartialEq, T29: PartialEq, T30: PartialEq, T31: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30, *const T31),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30, *const T31),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16) ||
        (*a0.17 != *a1.17) ||
        (*a0.18 != *a1.18) ||
        (*a0.19 != *a1.19) ||
        (*a0.20 != *a1.20) ||
        (*a0.21 != *a1.21) ||
        (*a0.22 != *a1.22) ||
        (*a0.23 != *a1.23) ||
        (*a0.24 != *a1.24) ||
        (*a0.25 != *a1.25) ||
        (*a0.26 != *a1.26) ||
        (*a0.27 != *a1.27) ||
        (*a0.28 != *a1.28) ||
        (*a0.29 != *a1.29) ||
        (*a0.30 != *a1.30)
}

// --- std::tie, 32 elements --------------------------------------------------

fn t34<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32>() -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30, *const T31, *const T32) {
    (std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut(), std::ptr::null_mut())
}

unsafe fn f376<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32>(
    a0: *const T1,
    a1: *const T2,
    a2: *const T3,
    a3: *const T4,
    a4: *const T5,
    a5: *const T6,
    a6: *const T7,
    a7: *const T8,
    a8: *const T9,
    a9: *const T10,
    a10: *const T11,
    a11: *const T12,
    a12: *const T13,
    a13: *const T14,
    a14: *const T15,
    a15: *const T16,
    a16: *const T17,
    a17: *const T18,
    a18: *const T19,
    a19: *const T20,
    a20: *const T21,
    a21: *const T22,
    a22: *const T23,
    a23: *const T24,
    a24: *const T25,
    a25: *const T26,
    a26: *const T27,
    a27: *const T28,
    a28: *const T29,
    a29: *const T30,
    a30: *const T31,
    a31: *const T32,
) -> (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30, *const T31, *const T32) {
    (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31)
}

unsafe fn f377<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq, T27: PartialEq, T28: PartialEq, T29: PartialEq, T30: PartialEq, T31: PartialEq, T32: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30, *const T31, *const T32),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30, *const T31, *const T32),
) -> bool {
    (*a0.0 == *a1.0) &&
        (*a0.1 == *a1.1) &&
        (*a0.2 == *a1.2) &&
        (*a0.3 == *a1.3) &&
        (*a0.4 == *a1.4) &&
        (*a0.5 == *a1.5) &&
        (*a0.6 == *a1.6) &&
        (*a0.7 == *a1.7) &&
        (*a0.8 == *a1.8) &&
        (*a0.9 == *a1.9) &&
        (*a0.10 == *a1.10) &&
        (*a0.11 == *a1.11) &&
        (*a0.12 == *a1.12) &&
        (*a0.13 == *a1.13) &&
        (*a0.14 == *a1.14) &&
        (*a0.15 == *a1.15) &&
        (*a0.16 == *a1.16) &&
        (*a0.17 == *a1.17) &&
        (*a0.18 == *a1.18) &&
        (*a0.19 == *a1.19) &&
        (*a0.20 == *a1.20) &&
        (*a0.21 == *a1.21) &&
        (*a0.22 == *a1.22) &&
        (*a0.23 == *a1.23) &&
        (*a0.24 == *a1.24) &&
        (*a0.25 == *a1.25) &&
        (*a0.26 == *a1.26) &&
        (*a0.27 == *a1.27) &&
        (*a0.28 == *a1.28) &&
        (*a0.29 == *a1.29) &&
        (*a0.30 == *a1.30) &&
        (*a0.31 == *a1.31)
}

unsafe fn f378<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq, T6: PartialEq, T7: PartialEq, T8: PartialEq, T9: PartialEq, T10: PartialEq, T11: PartialEq, T12: PartialEq, T13: PartialEq, T14: PartialEq, T15: PartialEq, T16: PartialEq, T17: PartialEq, T18: PartialEq, T19: PartialEq, T20: PartialEq, T21: PartialEq, T22: PartialEq, T23: PartialEq, T24: PartialEq, T25: PartialEq, T26: PartialEq, T27: PartialEq, T28: PartialEq, T29: PartialEq, T30: PartialEq, T31: PartialEq, T32: PartialEq>(
    a0: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30, *const T31, *const T32),
    a1: (*const T1, *const T2, *const T3, *const T4, *const T5, *const T6, *const T7, *const T8, *const T9, *const T10, *const T11, *const T12, *const T13, *const T14, *const T15, *const T16, *const T17, *const T18, *const T19, *const T20, *const T21, *const T22, *const T23, *const T24, *const T25, *const T26, *const T27, *const T28, *const T29, *const T30, *const T31, *const T32),
) -> bool {
    (*a0.0 != *a1.0) ||
        (*a0.1 != *a1.1) ||
        (*a0.2 != *a1.2) ||
        (*a0.3 != *a1.3) ||
        (*a0.4 != *a1.4) ||
        (*a0.5 != *a1.5) ||
        (*a0.6 != *a1.6) ||
        (*a0.7 != *a1.7) ||
        (*a0.8 != *a1.8) ||
        (*a0.9 != *a1.9) ||
        (*a0.10 != *a1.10) ||
        (*a0.11 != *a1.11) ||
        (*a0.12 != *a1.12) ||
        (*a0.13 != *a1.13) ||
        (*a0.14 != *a1.14) ||
        (*a0.15 != *a1.15) ||
        (*a0.16 != *a1.16) ||
        (*a0.17 != *a1.17) ||
        (*a0.18 != *a1.18) ||
        (*a0.19 != *a1.19) ||
        (*a0.20 != *a1.20) ||
        (*a0.21 != *a1.21) ||
        (*a0.22 != *a1.22) ||
        (*a0.23 != *a1.23) ||
        (*a0.24 != *a1.24) ||
        (*a0.25 != *a1.25) ||
        (*a0.26 != *a1.26) ||
        (*a0.27 != *a1.27) ||
        (*a0.28 != *a1.28) ||
        (*a0.29 != *a1.29) ||
        (*a0.30 != *a1.30) ||
        (*a0.31 != *a1.31)
}

// --- 5-element tuples -------------------------------------------------------

unsafe fn f379<T1: Default, T2: Default, T3: Default, T4: Default, T5: Default>() -> (T1, T2, T3, T4, T5) {
    <(T1, T2, T3, T4, T5)>::default()
}

unsafe fn f380<T1, T2, T3, T4, T5>(a0: T1, a1: T2, a2: T3, a3: T4, a4: T5) -> (T1, T2, T3, T4, T5) {
    (a0.into(), a1.into(), a2.into(), a3.into(), a4.into())
}

unsafe fn f381<T1: Clone, T2: Clone, T3: Clone, T4: Clone, T5: Clone>(a0: (T1, T2, T3, T4, T5)) -> (T1, T2, T3, T4, T5) {
    a0.clone()
}

unsafe fn f382<T1: Default, T2: Default, T3: Default, T4: Default, T5: Default>(a0: &mut (T1, T2, T3, T4, T5)) -> (T1, T2, T3, T4, T5) {
    std::mem::take(&mut *a0)
}

unsafe fn f383<T1: Clone, T2: Clone, T3: Clone, T4: Clone, T5: Clone>(a0: &mut (T1, T2, T3, T4, T5), a1: (T1, T2, T3, T4, T5)) {
    *a0 = a1.clone()
}

unsafe fn f384<T1: Default, T2: Default, T3: Default, T4: Default, T5: Default>(a0: &mut (T1, T2, T3, T4, T5), a1: &mut (T1, T2, T3, T4, T5)) {
    *a0 = std::mem::take(&mut *a1)
}

unsafe fn f385<T1, T2, T3, T4, T5>(a0: *mut (T1, T2, T3, T4, T5)) -> *mut T1 {
    &raw mut (*a0).0
}

unsafe fn f386<T1, T2, T3, T4, T5>(a0: *mut (T1, T2, T3, T4, T5)) -> *mut T2 {
    &raw mut (*a0).1
}

unsafe fn f387<T1, T2, T3, T4, T5>(a0: *mut (T1, T2, T3, T4, T5)) -> *mut T3 {
    &raw mut (*a0).2
}

unsafe fn f388<T1, T2, T3, T4, T5>(a0: *mut (T1, T2, T3, T4, T5)) -> *mut T4 {
    &raw mut (*a0).3
}

unsafe fn f389<T1, T2, T3, T4, T5>(a0: *mut (T1, T2, T3, T4, T5)) -> *mut T5 {
    &raw mut (*a0).4
}

unsafe fn f390<T1, T2, T3, T4, T5>(a0: *const (T1, T2, T3, T4, T5)) -> *const T1 {
    &raw const (*a0).0
}

unsafe fn f391<T1, T2, T3, T4, T5>(a0: *const (T1, T2, T3, T4, T5)) -> *const T2 {
    &raw const (*a0).1
}

unsafe fn f392<T1, T2, T3, T4, T5>(a0: *const (T1, T2, T3, T4, T5)) -> *const T3 {
    &raw const (*a0).2
}

unsafe fn f393<T1, T2, T3, T4, T5>(a0: *const (T1, T2, T3, T4, T5)) -> *const T4 {
    &raw const (*a0).3
}

unsafe fn f394<T1, T2, T3, T4, T5>(a0: *const (T1, T2, T3, T4, T5)) -> *const T5 {
    &raw const (*a0).4
}

unsafe fn f395<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq>(a0: (T1, T2, T3, T4, T5), a1: (T1, T2, T3, T4, T5)) -> bool {
    a0 == a1
}

unsafe fn f396<T1: PartialEq, T2: PartialEq, T3: PartialEq, T4: PartialEq, T5: PartialEq>(a0: (T1, T2, T3, T4, T5), a1: (T1, T2, T3, T4, T5)) -> bool {
    a0 != a1
}

// The arity-5 by-value tuple type. src.cpp declares `t35`; both target files must
// carry it or the converter aborts with
// `ir_src.json type entry has no matching IR target rule`.
fn t35<T1: Default, T2: Default, T3: Default, T4: Default, T5: Default>() -> (T1, T2, T3, T4, T5) {
    <(T1, T2, T3, T4, T5)>::default()
}

// std::__ignore_type -- see src.cpp.  The unit type: it holds nothing, and a
// write to it must vanish.
fn t36() -> () {
    ()
}

// `std::ignore` as a place that can be assigned to.  A ZST needs no storage, and
// `NonNull::dangling()` is the documented way to get a valid, aligned, non-null
// pointer for one -- writing `()` through it is sound and stores nothing.  A
// null pointer would NOT be: `*p = ()` on null is UB even for a ZST.
unsafe fn f397() -> *mut () {
    std::ptr::NonNull::<()>::dangling().as_ptr()
}

// `std::ignore = expr;` -- the value is evaluated (a0/a1 are already evaluated
// by the caller) and dropped.
unsafe fn f398<T1>(a0: *mut (), a1: T1) -> *mut () {
    a0
}
