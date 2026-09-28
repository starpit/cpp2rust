// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp: (a, b), the closed range.
fn t1() -> (i32, i32) {
    (0, 0)
}

// Default constructor: [0, i32::MAX], matching libc++'s
// uniform_int_distribution() : uniform_int_distribution(0) {}.
unsafe fn f1() -> (i32, i32) {
    (0, i32::MAX)
}

// uniform_int_distribution(a, b).
unsafe fn f2(a0: i32, a1: i32) -> (i32, i32) {
    (a0, a1)
}

// operator()(mt19937 &) -- UNBIASED rejection sampling over the 32-bit engine
// words, so the result is uniform on [a, b].  The engine word sequence is the
// real MT19937 (rules/mersenne_twister_engine); only the word-to-range mapping
// differs from libc++, and that mapping is implementation-defined.
unsafe fn f3(a0: &mut (i32, i32), a1: &mut Vec<u64>) -> i32 {
    let lo_ = a0.0 as i64;
    let hi_ = a0.1 as i64;
    if hi_ < lo_ {
        panic!("std::uniform_int_distribution: empty range (b < a)");
    }
    let span_ = (hi_ - lo_ + 1) as u64;
    // Draw enough engine words to cover the span, then reject the incomplete
    // top block so every value of [a, b] is equally likely.
    let words_ = if span_ > 0x1_0000_0000u64 { 2u32 } else { 1u32 };
    let bound_ = if words_ == 2 { u64::MAX } else { 0xFFFF_FFFFu64 };
    let limit_ = bound_ - (bound_ % span_ + span_ - 1) % span_;
    loop {
        let mut w_ = 0u64;
        let mut k_ = 0u32;
        while k_ < words_ {
            if a1.len() < 625 {
                panic!("std::mt19937: engine used before construction");
            }
            if a1[624] >= 624u64 {
                let mut i_ = 0usize;
                while i_ < 624 {
                    let y_ = (a1[i_] & 0x8000_0000u64) | (a1[(i_ + 1) % 624] & 0x7FFF_FFFFu64);
                    let mut n_ = a1[(i_ + 397) % 624] ^ (y_ >> 1);
                    if (y_ & 1u64) != 0 {
                        n_ ^= 0x9908_B0DFu64;
                    }
                    a1[i_] = n_ & 0xFFFF_FFFFu64;
                    i_ += 1;
                }
                a1[624] = 0u64;
            }
            let idx_ = a1[624] as usize;
            a1[624] = a1[624] + 1u64;
            let mut y_ = a1[idx_];
            y_ ^= (y_ >> 11) & 0xFFFF_FFFFu64;
            y_ ^= (y_ << 7) & 0x9D2C_5680u64;
            y_ ^= (y_ << 15) & 0xEFC6_0000u64;
            y_ &= 0xFFFF_FFFFu64;
            y_ ^= y_ >> 18;
            w_ = (w_ << 32) | (y_ & 0xFFFF_FFFFu64);
            k_ += 1;
        }
        if w_ <= limit_ {
            return (lo_ + (w_ % span_) as i64) as i32;
        }
    }
}

// a()
unsafe fn f4(a0: &mut (i32, i32)) -> i32 {
    a0.0
}

// b()
unsafe fn f5(a0: &mut (i32, i32)) -> i32 {
    a0.1
}

// min() == a()
unsafe fn f6(a0: &mut (i32, i32)) -> i32 {
    a0.0
}

// max() == b()
unsafe fn f7(a0: &mut (i32, i32)) -> i32 {
    a0.1
}

// operator()(engine) for the `unsigned long` engine spelling (see src.cpp).
unsafe fn f8(a0: &mut (i32, i32), a1: &mut Vec<u64>) -> i32 {
    let lo_ = a0.0 as i64;
    let hi_ = a0.1 as i64;
    if hi_ < lo_ {
        panic!("std::uniform_int_distribution: empty range (b < a)");
    }
    let span_ = (hi_ - lo_ + 1) as u64;
    // Draw enough engine words to cover the span, then reject the incomplete
    // top block so every value of [a, b] is equally likely.
    let words_ = if span_ > 0x1_0000_0000u64 { 2u32 } else { 1u32 };
    let bound_ = if words_ == 2 { u64::MAX } else { 0xFFFF_FFFFu64 };
    let limit_ = bound_ - (bound_ % span_ + span_ - 1) % span_;
    loop {
        let mut w_ = 0u64;
        let mut k_ = 0u32;
        while k_ < words_ {
            if a1.len() < 625 {
                panic!("std::mt19937: engine used before construction");
            }
            if a1[624] >= 624u64 {
                let mut i_ = 0usize;
                while i_ < 624 {
                    let y_ = (a1[i_] & 0x8000_0000u64) | (a1[(i_ + 1) % 624] & 0x7FFF_FFFFu64);
                    let mut n_ = a1[(i_ + 397) % 624] ^ (y_ >> 1);
                    if (y_ & 1u64) != 0 {
                        n_ ^= 0x9908_B0DFu64;
                    }
                    a1[i_] = n_ & 0xFFFF_FFFFu64;
                    i_ += 1;
                }
                a1[624] = 0u64;
            }
            let idx_ = a1[624] as usize;
            a1[624] = a1[624] + 1u64;
            let mut y_ = a1[idx_];
            y_ ^= (y_ >> 11) & 0xFFFF_FFFFu64;
            y_ ^= (y_ << 7) & 0x9D2C_5680u64;
            y_ ^= (y_ << 15) & 0xEFC6_0000u64;
            y_ &= 0xFFFF_FFFFu64;
            y_ ^= y_ >> 18;
            w_ = (w_ << 32) | (y_ & 0xFFFF_FFFFu64);
            k_ += 1;
        }
        if w_ <= limit_ {
            return (lo_ + (w_ % span_) as i64) as i32;
        }
    }
}

