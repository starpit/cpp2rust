// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn f1(a0: Ptr<u8>) -> ::std::fs::File {
    ::std::fs::File::create(a0.to_string()).expect("Failed to open file")
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
