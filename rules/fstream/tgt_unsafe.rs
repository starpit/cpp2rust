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
    libcc2rs::fresh_file(
        ::std::fs::File::create(::std::ffi::CStr::from_ptr(a0).to_str().unwrap()).unwrap(),
    )
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
    libcc2rs::fresh_file(
        ::std::fs::File::open(::std::ffi::CStr::from_ptr(a0).to_str().unwrap()).unwrap(),
    )
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

unsafe fn f9(a0: Vec<libc::c_char>) -> ::std::fs::File {
    libcc2rs::fresh_file(
        ::std::fs::File::create(::std::ffi::CStr::from_ptr(a0.as_ptr()).to_str().unwrap()).unwrap(),
    )
}

unsafe fn f10(a0: Vec<libc::c_char>) -> ::std::fs::File {
    libcc2rs::fresh_file(
        ::std::fs::File::open(::std::ffi::CStr::from_ptr(a0.as_ptr()).to_str().unwrap()).unwrap(),
    )
}

// ---------------------------------------------------------------------------
// The two-step open.  /dev/null is the model's "not open" stream: see the long
// note in src.cpp.
// ---------------------------------------------------------------------------

unsafe fn f11() -> ::std::fs::File {
    libcc2rs::fresh_file(
        ::std::fs::OpenOptions::new()
            .write(true)
            .open("/dev/null")
            .unwrap(),
    )
}

unsafe fn f12() -> ::std::fs::File {
    libcc2rs::fresh_file(::std::fs::File::open("/dev/null").unwrap())
}

unsafe fn f13(a0: &mut ::std::fs::File, a1: Vec<libc::c_char>, a2: u32) {
    let __p = ::std::ffi::CStr::from_ptr(a1.as_ptr())
        .to_str()
        .unwrap()
        .to_owned();
    let mut __o = ::std::fs::OpenOptions::new();
    __o.write(true).create(true);
    if a2 & 1 != 0 {
        __o.append(true);
    } else {
        __o.truncate(true);
    }
    *a0 = libcc2rs::fresh_file(__o.open(__p).unwrap());
}

unsafe fn f14(a0: &mut ::std::fs::File, a1: Vec<libc::c_char>) {
    let __p = ::std::ffi::CStr::from_ptr(a1.as_ptr())
        .to_str()
        .unwrap()
        .to_owned();
    *a0 = libcc2rs::fresh_file(::std::fs::File::open(__p).unwrap());
}

unsafe fn f15(a0: ::std::fs::File) -> bool {
    a0.metadata()
        .map(|__m| __m.file_type().is_file())
        .unwrap_or(false)
}

unsafe fn f16(a0: ::std::fs::File) -> bool {
    a0.metadata()
        .map(|__m| __m.file_type().is_file())
        .unwrap_or(false)
}

unsafe fn f17(a0: &mut ::std::fs::File) {
    libcc2rs::reset_file_fmt(a0);
    *a0 = libcc2rs::fresh_file(
        ::std::fs::OpenOptions::new()
            .write(true)
            .open("/dev/null")
            .unwrap(),
    );
}

unsafe fn f18(a0: &mut ::std::fs::File) {
    libcc2rs::reset_file_fmt(a0);
    *a0 = libcc2rs::fresh_file(::std::fs::File::open("/dev/null").unwrap());
}
