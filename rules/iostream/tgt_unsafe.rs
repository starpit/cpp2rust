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

// std::cout as a `std::ostream` value.
//
// This still hands back an owned clone -- the value form has to, because
// `std::ostream &os = std::cout;` binds it and a raw pointer there is an E0606
// cast -- but the clone's DESCRIPTOR NUMBER is not what the format state is keyed
// on. See libcc2rs::cc2_stream_key: a clone of stdout is keyed on the stdio
// stream it duplicates (1), not on the transient number it was handed, so
// `std::cout << std::hex;` in one statement and `std::cout << 255` in another
// reach the same state even though each statement makes a new clone and an
// ofstream recycles the number in between. Measured: C++ ff, was 255.
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
