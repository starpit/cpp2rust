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

fn t1() -> libcc2rs::IStream {
    libcc2rs::IStream::new()
}

fn f1() -> libcc2rs::IStream {
    libcc2rs::IStream::new()
}

// istringstream(const std::string &) -- a COPY of the string's bytes without its
// NUL terminator, which is representation and not content.
// ⚠️ CHANGED BY THE t1 FLIP: returns IStream.
fn f2(a0: Vec<u8>) -> libcc2rs::IStream {
    let mut __v: Vec<u8> = a0.clone();
    if __v.last() == Some(&0) {
        __v.pop();
    }
    libcc2rs::IStream::from_bytes(__v)
}

// .str() -- a COPY out of the buffer, plus exactly ONE trailing NUL, because
// rules/string's size() is len()-1.
// ⚠️ CHANGED BY THE t1 FLIP: `a0` is an IStream.  `buf()`, not `remaining()`.
fn f3(a0: libcc2rs::IStream) -> Vec<u8> {
    let mut __s: Vec<u8> = a0.buf().to_vec();
    __s.push(0);
    __s
}
