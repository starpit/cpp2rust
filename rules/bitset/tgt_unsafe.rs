// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

fn t1() -> Vec<bool> {
    Default::default()
}

unsafe fn f1() -> Vec<bool> {
    Vec::new()
}

unsafe fn f2(a0: u64) -> Vec<bool> {
    let mut x = a0;
    let mut v: Vec<bool> = Vec::new();
    while x != 0 {
        v.push((x & 1) != 0);
        x >>= 1;
    }
    v
}

unsafe fn f3(a0: Vec<bool>) -> Vec<bool> {
    a0.clone()
}

unsafe fn f4(a0: &mut Vec<bool>, a1: usize, a2: bool) {
    let i = a1;
    let val = a2;
    let b = &mut *a0;
    if b.len() <= i {
        b.resize(i + 1, false);
    }
    b[i] = val;
}

unsafe fn f5(a0: Vec<bool>) -> usize {
    let b = a0;
    let mut n: usize = 0;
    let mut i: usize = 0;
    while i < b.len() {
        if b[i] {
            n += 1;
        }
        i += 1;
    }
    n
}

unsafe fn f6(a0: Vec<bool>) -> u64 {
    let b = a0;
    let mut r: u64 = 0;
    let mut i: usize = 0;
    while i < b.len() {
        if b[i] {
            r |= 1u64 << i;
        }
        i += 1;
    }
    r
}

unsafe fn f7(a0: Vec<bool>, a1: usize) -> bool {
    let b = a0;
    let i = a1;
    if i < b.len() {
        b[i]
    } else {
        false
    }
}

unsafe fn f8(a0: &mut Vec<bool>, a1: usize) {
    let i = a1;
    let b = &mut *a0;
    if i < b.len() {
        b[i] = false;
    }
}

unsafe fn f9(a0: Vec<bool>) -> bool {
    let b = a0;
    let mut r = false;
    let mut i: usize = 0;
    while i < b.len() {
        if b[i] {
            r = true;
        }
        i += 1;
    }
    r
}

unsafe fn f10(a0: Vec<bool>) -> bool {
    let b = a0;
    let mut r = true;
    let mut i: usize = 0;
    while i < b.len() {
        if b[i] {
            r = false;
        }
        i += 1;
    }
    r
}
