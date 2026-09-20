// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Overlay on tgt_unsafe.rs.  Only the byte type and the `const char *`
// argument change shape: a Twine is Vec<u8> rather than Vec<libc::c_char>,
// and a C string arrives as Ptr<u8>, which walks itself to the terminator.
// t1 is repeated because the element type differs from the unsafe model's.

use libcc2rs::*;

fn t1() -> Vec<u8> {
    vec![0]
}

fn f1(a0: Ptr<u8>) -> Vec<u8> {
    a0.to_c_string_iterator()
        .chain(std::iter::once(0))
        .collect::<Vec<u8>>()
}

fn f2(a0: Vec<u8>) -> Vec<u8> {
    a0
}

fn f3(a0: Vec<u8>) -> Vec<u8> {
    a0
}

fn f4(a0: Vec<u8>, a1: Vec<u8>) -> Vec<u8> {
    let mut __lhs = a0;
    __lhs.pop();
    __lhs.extend_from_slice(&a1);
    __lhs
}
