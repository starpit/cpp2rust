// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model.  A Twine is the NUL-terminated byte string it
// denotes -- the same representation rules/string gives std::string -- so
// str() (f3) and the std::string constructor (f2) are the identity, and
// operator+ (f4) drops the left operand's terminator before appending the
// right one, which still carries its own.

fn t1() -> Vec<libc::c_char> {
    vec![0]
}

unsafe fn f1(a0: *const libc::c_char) -> Vec<libc::c_char> {
    let __s = a0;
    std::slice::from_raw_parts(__s, (0..).take_while(|&i| *__s.add(i) != 0).count() + 1).to_vec()
}

unsafe fn f2(a0: Vec<libc::c_char>) -> Vec<libc::c_char> {
    a0.clone()
}

unsafe fn f3(a0: Vec<libc::c_char>) -> Vec<libc::c_char> {
    a0.clone()
}

unsafe fn f4(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let mut __lhs = a0.clone();
    __lhs.pop();
    __lhs.extend_from_slice(&a1);
    __lhs
}
