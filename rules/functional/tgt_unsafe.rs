// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1<T1>() -> *mut T1 {
    Default::default()
}

unsafe fn f1<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

unsafe fn f2<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

unsafe fn f3<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

fn t2<T1>() -> Option<Box<dyn Fn() -> T1>> {
    None
}

unsafe fn f4<T1>(a0: &Option<Box<dyn Fn() -> T1>>) -> T1 {
    (a0.as_ref().unwrap())()
}
