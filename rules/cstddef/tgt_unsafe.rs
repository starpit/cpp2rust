// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1() -> u8 {
    Default::default()
}

// std::nullptr_t: one C++ value, one Rust inhabitant.  See src.cpp.
fn t2() -> () {
    ()
}

fn f1(a0: &mut u8, a1: u32) -> u8 {
    *a0 << a1
}

fn f2(a0: &mut u8, a1: u32) -> u8 {
    *a0 >> a1
}

fn f3(a0: &mut u8, a1: u32) -> u8 {
    let n_ = *a0 << a1;
    *a0 = n_;
    *a0
}

fn f4(a0: &mut u8, a1: u32) -> u8 {
    let n_ = *a0 >> a1;
    *a0 = n_;
    *a0
}
