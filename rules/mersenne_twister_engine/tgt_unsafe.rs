// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp.  Vec<u64>, length 625: [0..624) state words (each holding 32
// significant bits), [624] the draw index.
fn t1() -> Vec<u64> {
    Vec::new()
}

// Default constructor: default_seed == 5489.
unsafe fn f1() -> Vec<u64> {
    let mut s_: Vec<u64> = vec![0u64; 625];
    s_[0] = 5489u64;
    let mut i_ = 1usize;
    while i_ < 624 {
        let p_ = s_[i_ - 1];
        s_[i_] = (1812433253u64
            .wrapping_mul(p_ ^ (p_ >> 30))
            .wrapping_add(i_ as u64))
            & 0xFFFF_FFFFu64;
        i_ += 1;
    }
    s_[624] = 624u64;
    s_
}

// mersenne_twister_engine(result_type sd) -- libc++'s seed(sd).
unsafe fn f2(a0: u64) -> Vec<u64> {
    let mut s_: Vec<u64> = vec![0u64; 625];
    s_[0] = a0 & 0xFFFF_FFFFu64;
    let mut i_ = 1usize;
    while i_ < 624 {
        let p_ = s_[i_ - 1];
        s_[i_] = (1812433253u64
            .wrapping_mul(p_ ^ (p_ >> 30))
            .wrapping_add(i_ as u64))
            & 0xFFFF_FFFFu64;
        i_ += 1;
    }
    s_[624] = 624u64;
    s_
}

// operator() -- twist on exhaustion, then temper.  Bit-exact MT19937.
unsafe fn f3(a0: &mut Vec<u64>) -> u64 {
    if a0.len() < 625 {
        panic!("std::mt19937: engine used before construction");
    }
    if a0[624] >= 624u64 {
        let mut i_ = 0usize;
        while i_ < 624 {
            let y_ = (a0[i_] & 0x8000_0000u64) | (a0[(i_ + 1) % 624] & 0x7FFF_FFFFu64);
            let mut n_ = a0[(i_ + 397) % 624] ^ (y_ >> 1);
            if (y_ & 1u64) != 0 {
                n_ ^= 0x9908_B0DFu64;
            }
            a0[i_] = n_ & 0xFFFF_FFFFu64;
            i_ += 1;
        }
        a0[624] = 0u64;
    }
    let idx_ = a0[624] as usize;
    a0[624] = a0[624] + 1u64;
    let mut y_ = a0[idx_];
    y_ ^= (y_ >> 11) & 0xFFFF_FFFFu64;
    y_ ^= (y_ << 7) & 0x9D2C_5680u64;
    y_ ^= (y_ << 15) & 0xEFC6_0000u64;
    y_ &= 0xFFFF_FFFFu64;
    y_ ^= y_ >> 18;
    y_ & 0xFFFF_FFFFu64
}

// --- same model, `unsigned long` result_type spelling (see src.cpp) ---
fn t2() -> Vec<u64> {
    Vec::new()
}

// Default constructor: default_seed == 5489.
unsafe fn f4() -> Vec<u64> {
    let mut s_: Vec<u64> = vec![0u64; 625];
    s_[0] = 5489u64;
    let mut i_ = 1usize;
    while i_ < 624 {
        let p_ = s_[i_ - 1];
        s_[i_] = (1812433253u64
            .wrapping_mul(p_ ^ (p_ >> 30))
            .wrapping_add(i_ as u64))
            & 0xFFFF_FFFFu64;
        i_ += 1;
    }
    s_[624] = 624u64;
    s_
}

// mersenne_twister_engine(result_type sd) -- libc++'s seed(sd).
unsafe fn f5(a0: u64) -> Vec<u64> {
    let mut s_: Vec<u64> = vec![0u64; 625];
    s_[0] = a0 & 0xFFFF_FFFFu64;
    let mut i_ = 1usize;
    while i_ < 624 {
        let p_ = s_[i_ - 1];
        s_[i_] = (1812433253u64
            .wrapping_mul(p_ ^ (p_ >> 30))
            .wrapping_add(i_ as u64))
            & 0xFFFF_FFFFu64;
        i_ += 1;
    }
    s_[624] = 624u64;
    s_
}

// operator() -- twist on exhaustion, then temper.  Bit-exact MT19937.
unsafe fn f6(a0: &mut Vec<u64>) -> u64 {
    if a0.len() < 625 {
        panic!("std::mt19937: engine used before construction");
    }
    if a0[624] >= 624u64 {
        let mut i_ = 0usize;
        while i_ < 624 {
            let y_ = (a0[i_] & 0x8000_0000u64) | (a0[(i_ + 1) % 624] & 0x7FFF_FFFFu64);
            let mut n_ = a0[(i_ + 397) % 624] ^ (y_ >> 1);
            if (y_ & 1u64) != 0 {
                n_ ^= 0x9908_B0DFu64;
            }
            a0[i_] = n_ & 0xFFFF_FFFFu64;
            i_ += 1;
        }
        a0[624] = 0u64;
    }
    let idx_ = a0[624] as usize;
    a0[624] = a0[624] + 1u64;
    let mut y_ = a0[idx_];
    y_ ^= (y_ >> 11) & 0xFFFF_FFFFu64;
    y_ ^= (y_ << 7) & 0x9D2C_5680u64;
    y_ ^= (y_ << 15) & 0xEFC6_0000u64;
    y_ &= 0xFFFF_FFFFu64;
    y_ ^= y_ >> 18;
    y_ & 0xFFFF_FFFFu64
}
