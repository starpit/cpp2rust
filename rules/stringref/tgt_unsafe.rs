// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model and for what is deliberately not covered.  A
// StringRef is the NUL-terminated byte string it denotes -- the same
// representation rules/string gives std::string -- so str() (f1), the
// std::string constructor (f4) and the copy/move constructors (f11, f12) are
// the identity, and size()/empty() are spelled exactly as rules/string's f2
// and f22 are, discounting the terminator the Vec carries.

fn t1() -> Vec<libc::c_char> {
    vec![0]
}

fn t2() -> Vec<libc::c_char> {
    vec![0]
}

unsafe fn f1(a0: Vec<libc::c_char>) -> Vec<libc::c_char> {
    a0.clone()
}

unsafe fn f2(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> bool {
    a0 == a1
}

unsafe fn f3(a0: *const libc::c_char) -> Vec<libc::c_char> {
    let __s = a0;
    std::slice::from_raw_parts(__s, (0..).take_while(|&i| *__s.add(i) != 0).count() + 1).to_vec()
}

unsafe fn f4(a0: Vec<libc::c_char>) -> Vec<libc::c_char> {
    a0.clone()
}

unsafe fn f5(a0: Vec<libc::c_char>) -> bool {
    a0.len() <= 1
}

unsafe fn f6(a0: *const libc::c_char, a1: u64) -> Vec<libc::c_char> {
    let mut __v = std::slice::from_raw_parts(a0, a1 as usize).to_vec();
    __v.push(0);
    __v
}

unsafe fn f7(a0: Vec<libc::c_char>) -> u64 {
    (a0.len() - 1) as u64
}


unsafe fn f8(a0: *const libc::c_char) -> Vec<libc::c_char> {
    let __s = a0;
    std::slice::from_raw_parts(__s, (0..).take_while(|&i| *__s.add(i) != 0).count() + 1).to_vec()
}

unsafe fn f9() -> Vec<libc::c_char> {
    vec![0]
}

unsafe fn f10(a0: Vec<libc::c_char>) -> Vec<libc::c_char> {
    a0.clone()
}

unsafe fn f11(a0: Vec<libc::c_char>) -> Vec<libc::c_char> {
    a0.clone()
}

unsafe fn f12(a0: &mut Vec<libc::c_char>, a1: Vec<libc::c_char>) {
    *a0 = a1.clone()
}

unsafe fn f13(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> bool {
    a0 != a1
}
