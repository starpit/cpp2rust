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

fn f9(a0: Vec<u8>) -> ::std::fs::File {
    ::std::fs::File::create(String::from_utf8_lossy(
        &a0[..a0.iter().position(|&__c| __c == 0).unwrap_or(a0.len())],
    ).into_owned())
    .expect("Failed to open file")
}

fn f10(a0: Vec<u8>) -> ::std::fs::File {
    ::std::fs::File::open(String::from_utf8_lossy(
        &a0[..a0.iter().position(|&__c| __c == 0).unwrap_or(a0.len())],
    ).into_owned())
    .expect("Failed to open file")
}

// Only the two open() rules need an overlay: they are the only ones of the
// eight that touch the std::string argument, which is Vec<u8> here and
// Vec<libc::c_char> in the unsafe model.  The rest of the set uses neither
// libc nor the string representation, so the unsafe bodies serve both.
fn f13(a0: &mut ::std::fs::File, a1: Vec<u8>, a2: u32) {
    let __p = String::from_utf8_lossy(
        &a1[..a1.iter().position(|&__c| __c == 0).unwrap_or(a1.len())],
    )
    .into_owned();
    let mut __o = ::std::fs::OpenOptions::new();
    __o.write(true).create(true);
    if a2 & 1 != 0 {
        __o.append(true);
    } else {
        __o.truncate(true);
    }
    *a0 = __o.open(__p).expect("Failed to open file");
}

fn f14(a0: &mut ::std::fs::File, a1: Vec<u8>) {
    let __p = String::from_utf8_lossy(
        &a1[..a1.iter().position(|&__c| __c == 0).unwrap_or(a1.len())],
    )
    .into_owned();
    *a0 = ::std::fs::File::open(__p).expect("Failed to open file");
}
