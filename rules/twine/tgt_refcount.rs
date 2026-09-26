// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Overlay on tgt_unsafe.rs.  Only the byte type and the `const char *`
// argument change shape: a Twine is Vec<u8> rather than Vec<libc::c_char>,
// and a C string arrives as Ptr<u8>, which walks itself to the terminator.
// t1 is repeated because the element type differs from the unsafe model's.
// f7 is the mirror of f5: the LITERAL is the left operand here, so its walk
// stops AT the terminator and never emits it, and the StringRef appended after
// it is what supplies the one terminator the result carries -- there is nothing
// to pop, which is exactly the asymmetry with f5.

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

fn f5(a0: Vec<u8>, a1: Ptr<u8>) -> Vec<u8> {
    let mut __lhs = a0;
    __lhs.pop();
    __lhs.extend(a1.to_c_string_iterator().chain(std::iter::once(0)));
    __lhs
}

fn f6(a0: Vec<u8>) -> Vec<u8> {
    a0
}

fn f7(a0: Ptr<u8>, a1: Vec<u8>) -> Vec<u8> {
    a0.to_c_string_iterator()
        .chain(a1.into_iter())
        .collect::<Vec<u8>>()
}
