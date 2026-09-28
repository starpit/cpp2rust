// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model and for the list of members deliberately NOT keyed.
// An error_code is a bare errno-shaped i32; 0 is "no error".  A reference to one
// is a raw pointer, exactly as rules/string models `std::string::iterator`.
fn t1() -> i32 {
    0
}

fn t2() -> *mut i32 {
    ::std::ptr::null_mut()
}

// Default constructor: no error.
unsafe fn f1() -> i32 {
    0
}

// operator bool: true iff a nonzero errno.  Correct for every value this model
// can hold; the only value it CAN hold today is 0, because nothing is able to
// write into the out-param (src.cpp).
unsafe fn f2(a0: i32) -> bool {
    a0 != 0
}
