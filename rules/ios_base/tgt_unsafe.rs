// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1() -> u32 {
    0
}

fn t2() -> u32 {
    0
}

fn t3() -> u32 {
    0
}

// std::ios_base itself -- the flags word.  See the long note in src.cpp for why
// the class collapses to the same u32 as its fmtflags typedef.
fn t4() -> u32 {
    0
}

unsafe fn f1() {
    ()
}

// std::ios_base::app
unsafe fn f2() -> u32 {
    1
}

// std::ios_base::ate
unsafe fn f3() -> u32 {
    2
}

// std::ios_base::binary
unsafe fn f4() -> u32 {
    4
}

// std::ios_base::in
unsafe fn f5() -> u32 {
    8
}

// std::ios_base::out
unsafe fn f6() -> u32 {
    16
}

// std::ios_base::trunc
unsafe fn f7() -> u32 {
    32
}

// std::ios_base::boolalpha
unsafe fn f8() -> u32 {
    1
}

// std::ios_base::dec
unsafe fn f9() -> u32 {
    2
}

// std::ios_base::fixed
unsafe fn f10() -> u32 {
    4
}

// std::ios_base::hex
unsafe fn f11() -> u32 {
    8
}

// std::ios_base::internal
unsafe fn f12() -> u32 {
    16
}

// std::ios_base::left
unsafe fn f13() -> u32 {
    32
}

// std::ios_base::oct
unsafe fn f14() -> u32 {
    64
}

// std::ios_base::right
unsafe fn f15() -> u32 {
    128
}

// std::ios_base::scientific
unsafe fn f16() -> u32 {
    256
}

// std::ios_base::showbase
unsafe fn f17() -> u32 {
    512
}

// std::ios_base::showpoint
unsafe fn f18() -> u32 {
    1024
}

// std::ios_base::showpos
unsafe fn f19() -> u32 {
    2048
}

// std::ios_base::skipws
unsafe fn f20() -> u32 {
    4096
}

// std::ios_base::unitbuf
unsafe fn f21() -> u32 {
    8192
}

// std::ios_base::uppercase
unsafe fn f22() -> u32 {
    16384
}

// std::ios_base::adjustfield
unsafe fn f23() -> u32 {
    176
}

// std::ios_base::basefield
unsafe fn f24() -> u32 {
    74
}

// std::ios_base::floatfield
unsafe fn f25() -> u32 {
    260
}

// std::ios_base::goodbit
unsafe fn f26() -> u32 {
    0
}

// std::ios_base::badbit
unsafe fn f27() -> u32 {
    1
}

// std::ios_base::eofbit
unsafe fn f28() -> u32 {
    2
}

// std::ios_base::failbit
unsafe fn f29() -> u32 {
    4
}

// ---------------------------------------------------------------------------
// The base manipulators.  Each is a real function, so `std::hex` used as a
// VALUE -- which is how it arrives, as a function pointer -- resolves to
// `libcc2rs::f30_unsafe`-shaped callable rather than an undefined symbol.
// `with_basefield` keeps the non-base flags, matching setf(base, basefield).
// ---------------------------------------------------------------------------

// std::hex
unsafe fn f30(a0: *mut u32) -> *mut u32 {
    (*a0) = libcc2rs::with_basefield(*a0, libcc2rs::CC2_HEX);
    a0
}

// std::dec
unsafe fn f31(a0: *mut u32) -> *mut u32 {
    (*a0) = libcc2rs::with_basefield(*a0, libcc2rs::CC2_DEC);
    a0
}

// std::oct
unsafe fn f32(a0: *mut u32) -> *mut u32 {
    (*a0) = libcc2rs::with_basefield(*a0, libcc2rs::CC2_OCT);
    a0
}
