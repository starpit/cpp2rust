// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1() -> ::std::fs::File {
    ::std::fs::File::open("").unwrap()
}

fn t2() -> ::std::fs::File {
    ::std::fs::File::open("").unwrap()
}

fn t3() -> ::std::fs::File {
    ::std::fs::File::open("").unwrap()
}

unsafe fn f1(a0: *const libc::c_char) -> ::std::fs::File {
    ::std::fs::File::create(::std::ffi::CStr::from_ptr(a0).to_str().unwrap()).unwrap()
}

unsafe fn f2(a0: ::std::fs::File) -> ::std::fs::File {
    a0.try_clone().unwrap()
}
unsafe fn f3(a0: &mut ::std::fs::File) -> &mut ::std::fs::File {
    a0
}
unsafe fn f4(a0: ::std::fs::File) -> ::std::fs::File {
    a0
}
unsafe fn f5(a0: *const libc::c_char) -> ::std::fs::File {
    ::std::fs::File::open(::std::ffi::CStr::from_ptr(a0).to_str().unwrap()).unwrap()
}

unsafe fn f6(a0: ::std::fs::File) -> ::std::fs::File {
    a0.try_clone().unwrap()
}

unsafe fn f7(a0: ::std::fs::File) -> ::std::fs::File {
    a0.try_clone().unwrap()
}

unsafe fn f8(a0: ::std::fs::File) -> ::std::fs::File {
    a0
}

// t4 -- std::istreambuf_iterator<char>.  Modelled as the File it reads from,
// which is NOT a new decision: f7 and f8 in this module are already committed
// returning ::std::fs::File for istreambuf_iterator, so the type entry has to
// match them.
//
// (A `std::basic_ofstream<char>` entry was written, measured, and REMOVED: it
// read back from ir_src.json as `std::ofstream`, a duplicate of t2, so it would
// have been a dead key.  See src.cpp for the verbatim readback.)
//
// ⚠️ SCOPE LIMIT, stated because the model is lossy: this serves the
// "iterator standing in for the stream" surface that is already keyed (construct
// from a streambuf, copy-construct, hand the pair to a string ctor).  It does NOT
// model iterator arithmetic, comparison against the default-constructed END
// iterator, or per-character increment, none of which are keyed here.
fn t4() -> ::std::fs::File {
    ::std::fs::File::open("").unwrap()
}
