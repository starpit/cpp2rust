// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model: an ostringstream is a Vec<u8>, which already
// implements std::io::Write by appending, and that is the only operation the
// converter's built-in ostream lowering performs on it.

fn t1() -> Vec<u8> {
    Vec::new()
}

fn f1() -> Vec<u8> {
    Vec::new()
}

// .str() -- a COPY out of the buffer into std::string's representation, which
// in the unsafe model is a NUL-terminated Vec<libc::c_char>.  The terminator is
// not part of the string (rules/string's size() is len()-1), so exactly one is
// appended here; writing the buffer through unchanged would make every
// `os.str()` one byte short of its own representation and an embedded NUL would
// then truncate it.
fn f2(a0: Vec<u8>) -> Vec<libc::c_char> {
    let mut __s: Vec<libc::c_char> = a0.iter().map(|&__b| __b as libc::c_char).collect();
    __s.push(0);
    __s
}
