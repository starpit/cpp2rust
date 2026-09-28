// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

// f1 -- ofstream(const char *, openmode).  HONOURS THE OPENMODE.  Bit values read
// out of $TC/libcxx/ios:284-289 (the libc++ the converter PARSES with):
//     app 0x01  ate 0x02  binary 0x04  in 0x08  out 0x10  trunc 0x20
// corroborated by an emitted body showing the literal `0x10` for a defaulted
// `std::ios::out`.  See tgt_unsafe.rs f1 for the full fopen-mode table, the
// runtime-branch argument, and the ate/binary/in/trunc dispositions.
fn f1(a0: Ptr<u8>, a1: libc::c_uint) -> ::std::fs::File {
    let __mode: libc::c_uint = a1;
    let __app: bool = __mode & 0x01 != 0;
    let __ate: bool = __mode & 0x02 != 0;
    let __rd: bool = __mode & 0x08 != 0;
    let __tr: bool = __mode & 0x20 != 0;
    let mut __f = ::std::fs::OpenOptions::new()
        .write(true)
        .read(__rd)
        .append(__app)
        .truncate(__tr || !(__app || __rd))
        .create(__app || __tr || !__rd)
        .open(a0.to_string())
        .expect("Failed to open file");
    if __ate {
        ::std::io::Seek::seek(&mut __f, ::std::io::SeekFrom::End(0))
            .expect("Failed to honour std::ios::ate");
    }
    __f
}

fn f5(a0: Ptr<u8>) -> ::std::fs::File {
    ::std::fs::File::open(a0.to_string()).expect("Failed to open file")
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

// f9 -- ofstream(const std::string &, openmode).  The `const std::string &`
// parameter arrives as a NUL-terminated Vec<u8>; the strip-the-terminator shape
// is copied verbatim from the committed rules/filesystem f2, which takes the same
// C++ parameter type.  The openmode is HONOURED, by the same measured bit values
// and the same fopen-mode table as f1 -- see tgt_unsafe.rs f1 for both.
fn f9(a0: Vec<u8>, a1: libc::c_uint) -> ::std::fs::File {
    let __mode: libc::c_uint = a1;
    let __app: bool = __mode & 0x01 != 0;
    let __ate: bool = __mode & 0x02 != 0;
    let __rd: bool = __mode & 0x08 != 0;
    let __tr: bool = __mode & 0x20 != 0;
    let mut __p: Vec<u8> = a0;
    if __p.last() == Some(&0) {
        __p.pop();
    }
    let mut __f = ::std::fs::OpenOptions::new()
        .write(true)
        .read(__rd)
        .append(__app)
        .truncate(__tr || !(__app || __rd))
        .create(__app || __tr || !__rd)
        .open(String::from_utf8_lossy(&__p).into_owned())
        .expect("Failed to open file");
    if __ate {
        ::std::io::Seek::seek(&mut __f, ::std::io::SeekFrom::End(0))
            .expect("Failed to honour std::ios::ate");
    }
    __f
}
