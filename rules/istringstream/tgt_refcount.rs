// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp.  In this model a std::string is a NUL-terminated Vec<u8>, so f2
// and f3 have DIFFERENT bodies from the unsafe ones (Vec<u8> vs
// Vec<libc::c_char>).
//
// THIS FILE MUST EXIST.  For a model-DEPENDENT parameter or return type,
// deleting it does not leave the key out of the refcount model -- the converter
// falls back to the UNSAFE body, which here would take/return
// Vec<libc::c_char> and give error[E0308] "expected Vec<u8>, found Vec<i8>"
// (measured for rules/sstream, same shape).

fn t1() -> Vec<u8> {
    Vec::new()
}

fn f1() -> Vec<u8> {
    Vec::new()
}

// istringstream(const std::string &) -- a COPY of the string's bytes without its
// NUL terminator, which is representation and not content.
fn f2(a0: Vec<u8>) -> Vec<u8> {
    let mut __v: Vec<u8> = a0.clone();
    if __v.last() == Some(&0) {
        __v.pop();
    }
    __v
}

// .str() -- a COPY out of the buffer, plus exactly ONE trailing NUL, because
// rules/string's size() is len()-1.
fn f3(a0: Vec<u8>) -> Vec<u8> {
    let mut __s: Vec<u8> = a0.clone();
    __s.push(0);
    __s
}
