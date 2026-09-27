// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model: a std::istringstream is the byte buffer it was
// built over, Vec<u8>, the same representation rules/sstream and
// rules/basic_stringstream use.  In this model a std::string is a
// NUL-TERMINATED Vec<libc::c_char> whose size() is len()-1, so the terminator is
// stripped on the way in and added on the way out.

fn t1() -> Vec<u8> {
    Vec::new()
}

fn f1() -> Vec<u8> {
    Vec::new()
}

// istringstream(const std::string &) -- a COPY of the string's bytes, without
// its NUL terminator, which is representation and not content.  Copying matters:
// the argument is a const reference and must not be emptied.
fn f2(a0: Vec<libc::c_char>) -> Vec<u8> {
    let mut __v: Vec<u8> = a0.iter().map(|&__b| __b as u8).collect();
    if __v.last() == Some(&0) {
        __v.pop();
    }
    __v
}

// .str() -- a COPY out of the buffer into std::string's representation, with
// exactly ONE trailing NUL appended.
fn f3(a0: Vec<u8>) -> Vec<libc::c_char> {
    let mut __s: Vec<libc::c_char> = a0.iter().map(|&__b| __b as libc::c_char).collect();
    __s.push(0);
    __s
}
