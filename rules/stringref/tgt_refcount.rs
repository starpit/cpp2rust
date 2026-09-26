// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Overlay on tgt_unsafe.rs.  Only the byte type and the pointer spellings
// change shape, exactly as rules/twine's overlay does: a StringRef is Vec<u8>
// rather than Vec<libc::c_char>, a `const char *` arrives as Ptr<u8>, and the
// mutable receiver of operator= is a Ptr<Vec<u8>> written through rather than a
// &mut (the shape rules/string's f29 uses).  t1 is repeated because the element
// type differs from the unsafe model's.

use libcc2rs::*;

fn t1() -> Vec<u8> {
    vec![0]
}

fn t2() -> Vec<u8> {
    vec![0]
}

fn f1(a0: Vec<u8>) -> Vec<u8> {
    a0
}

fn f2(a0: Vec<u8>, a1: Vec<u8>) -> bool {
    a0 == a1
}

fn f3(a0: Ptr<u8>) -> Vec<u8> {
    let mut __bytes = a0.to_c_bytes();
    __bytes.push(0);
    __bytes
}

fn f4(a0: Vec<u8>) -> Vec<u8> {
    a0
}

fn f5(a0: Vec<u8>) -> bool {
    a0.len() <= 1
}

fn f6(a0: Ptr<u8>, a1: u64) -> Vec<u8> {
    a0.map(|c| c.read())
        .take(a1 as usize)
        .chain(std::iter::once(0))
        .collect::<Vec<u8>>()
}

fn f7(a0: Vec<u8>) -> u64 {
    (a0.len() - 1) as u64
}


fn f8(a0: Ptr<u8>) -> Vec<u8> {
    let mut __bytes = a0.to_c_bytes();
    __bytes.push(0);
    __bytes
}

fn f9() -> Vec<u8> {
    vec![0]
}

fn f10(a0: Vec<u8>) -> Vec<u8> {
    a0
}

fn f11(a0: Vec<u8>) -> Vec<u8> {
    a0
}

fn f12(a0: Ptr<Vec<u8>>, a1: Vec<u8>) {
    a0.write(a1)
}

fn f13(a0: Vec<u8>, a1: Vec<u8>) -> bool {
    a0 != a1
}
