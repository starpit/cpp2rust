// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model: a std::istringstream is the byte buffer it was
// built over, Vec<u8>, the same representation rules/sstream and
// rules/basic_stringstream use.  In this model a std::string is a
// NUL-TERMINATED Vec<libc::c_char> whose size() is len()-1, so the terminator is
// stripped on the way in and added on the way out.

fn t1() -> libcc2rs::IStream {
    libcc2rs::IStream::new()
}

fn f1() -> libcc2rs::IStream {
    libcc2rs::IStream::new()
}

// istringstream(const std::string &) -- a COPY of the string's bytes, without
// its NUL terminator, which is representation and not content.  Copying matters:
// the argument is a const reference and must not be emptied.
// ⚠️ CHANGED BY THE t1 FLIP: returns IStream.  `from_bytes` sets the get
// position to 0, which is what `std::istringstream iss(s)` does.
fn f2(a0: Vec<libc::c_char>) -> libcc2rs::IStream {
    let mut __v: Vec<u8> = a0.iter().map(|&__b| __b as u8).collect();
    if __v.last() == Some(&0) {
        __v.pop();
    }
    libcc2rs::IStream::from_bytes(__v)
}

// .str() -- a COPY out of the buffer into std::string's representation, with
// exactly ONE trailing NUL appended.
// ⚠️ CHANGED BY THE t1 FLIP: `a0` is an IStream.  `buf()`, not `remaining()` --
// `.str()` reports the WHOLE buffer even after extraction has consumed part of it.
fn f3(a0: libcc2rs::IStream) -> Vec<libc::c_char> {
    let mut __s: Vec<libc::c_char> = a0.buf().iter().map(|&__b| __b as libc::c_char).collect();
    __s.push(0);
    __s
}
