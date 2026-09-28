// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::os::fd::{AsFd, FromRawFd, IntoRawFd};

fn t1() -> std::fs::File {
    std::fs::File::open("").unwrap()
}

// TODO: t2 and t3 should be translated to *mut dyn Traits
unsafe fn t2() -> *mut std::fs::File {
    libcc2rs::cout_unsafe()
}

unsafe fn t3() -> *mut std::fs::File {
    libcc2rs::cout_unsafe()
}

unsafe fn f1() -> ::std::fs::File {
    std::fs::File::from_raw_fd(
        std::io::stdout()
            .as_fd()
            .try_clone_to_owned()
            .unwrap()
            .into_raw_fd(),
    )
}

unsafe fn f2() -> ::std::fs::File {
    std::fs::File::from_raw_fd(
        std::io::stderr()
            .as_fd()
            .try_clone_to_owned()
            .unwrap()
            .into_raw_fd(),
    )
}

unsafe fn f3() -> *mut ::std::fs::File {
    libcc2rs::cout_unsafe()
}

unsafe fn f4() -> *mut ::std::fs::File {
    libcc2rs::cerr_unsafe()
}

// f5/f6 -- std::ios_base::in / out.  libcxx/ios:287-288 gives in = 0x08 and
// out = 0x10 with `typedef unsigned int openmode`, and the converter maps
// `unsigned int` to u32 (`search type unsigned int, result: u32`).  These are
// plain constants: no receiver, no pointer text, so the refcount model inherits
// them unchanged (ir_unsafe.json is the unconditional base layer).
fn f5() -> u32 {
    0x08
}

fn f6() -> u32 {
    0x10
}

// f7 -- std::ios_base::binary == 0x04 (libcxx/ios:286). Plain `unsigned int` constant,
// nothing model-dependent, so tgt_refcount.rs restates it byte-identically.
fn f7() -> u32 {
    0x04
}

// t4 -- std::istream. Same counterpart as t1 (std::ostream -> std::fs::File): a byte
// stream handle. TYPE key only; no istream operation is keyed, so `>>`/getline still
// fail loudly.
fn t4() -> std::fs::File {
    std::fs::File::open("").unwrap()
}

// t5 -- std::ios_base::seekdir, `enum seekdir { beg, cur, end }` (libcxx/ios:295).
// Unscoped enum, no fixed underlying type, promotes to int -> i32. Nothing
// model-dependent, so tgt_refcount.rs restates it byte-identically.
fn t5() -> i32 {
    0
}
