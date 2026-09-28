// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp: an opaque handle; the entropy lives in /dev/urandom, which is the
// token libc++'s std::random_device itself opens.
fn t1() -> u8 {
    0
}

// Default constructor.
unsafe fn f1() -> u8 {
    0
}

// operator() -- one 32-bit word of kernel entropy.  PANICS on failure: a
// random_device that silently returned a constant would make every seeded
// engine in the corpus deterministic without saying so.
unsafe fn f2(a0: &mut u8) -> u32 {
    let _ = a0;
    let mut buf_ = [0u8; 4];
    let mut f_ = ::std::fs::File::open("/dev/urandom")
        .expect("std::random_device: cannot open /dev/urandom");
    ::std::io::Read::read_exact(&mut f_, &mut buf_)
        .expect("std::random_device: short read from /dev/urandom");
    u32::from_le_bytes(buf_)
}
