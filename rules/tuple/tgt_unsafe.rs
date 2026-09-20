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

