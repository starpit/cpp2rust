// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

fn t1<T1>() -> Ptr<T1> {
    Ptr::null()
}

fn f1<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f2<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn f3<T1>(a0: Ptr<T1>) -> Ptr<T1> {
    a0
}

fn t2<T1>() -> Option<Box<dyn Fn() -> T1>> {
    None
}

fn f4<T1>(a0: &Option<Box<dyn Fn() -> T1>>) -> T1 {
    (a0.as_ref().unwrap())()
}

fn f5<T1, T2: Fn() -> T1 + 'static>(a0: T2) -> Option<Box<dyn Fn() -> T1>> {
    Some(Box::new(a0))
}
