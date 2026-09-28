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
// ===========================================================================
// ⛔⛔ WHY THE THREE STORES BELOW GO THROUGH `.as_mut_slice()[..]` AND NOT
// `a1[..] = ..`, 2026-09-28.  Same root cause as the swallowed-deref class fixed
// in rules/mlir (0a14e018) and in cstddef/pair/vector/deque_iterator, but a
// BROADER sub-class: there is no `*` here to swallow at all.
//
// A placeholder whose rule parameter is `&mut T` and which is NOT a method-call
// receiver is emitted by the converter as `&mut <place>` -- converter.cpp:8262-8271,
// `needs_lvalue() && needs_mut_borrow()` returns `"&mut " + place`, gated on
// `needs_explicit_mut_borrow = !is_method_call_receiver && ParamIsMutRef(..)`
// (converter.cpp:8380-8382).  That gate does not care what follows the
// placeholder, so an INDEXED store `a1[i_] = v` emits
//     &mut (*gen_)[i_] = n_ & 0xFFFF_FFFFu64;
// i.e. `error[E0070]: invalid left-hand side of assignment`.  Measured live at
// dip__dip.cpp.rs:19686/19689/19692 before this change.
// ⭐ SO THE CLASS IS NOT "a swallowed deref".  It is "a `&mut`-declared
// placeholder in an assignment-LHS position", with ARBITRARY `[..]` / `.field`
// projection allowed between the placeholder and the operator.  A grep for a
// literal `*aN`, or a detector that wants the `=` ADJACENT to the placeholder,
// finds none of these six sites.
// ⭐ AND rc=0 CANNOT SEE IT: `&mut x = v` is a legal EXPRESSION PARSE.  E0070 is
// raised in HIR lowering / type-check, not in the parser, and `rustfmt` -- the only
// Rust parser in this pipeline -- returns rc=0 on the broken text and re-emits it
// verbatim.  These emitted at `A complete rc=0` with no placeholder token.
//
// ⚠️ WHY `.as_mut_slice()` AND NOT `core::mem::replace`, which fixed the other
// modules of this class: those sites stored to the WHOLE `&mut` place, so
// `drop(core::mem::replace(a1, v))` was right -- the bare placeholder records the
// identical `borrow_mut` and emits the `&mut place` that `replace` wants.  HERE the
// store target is an ELEMENT, so `replace(a1, v)` would replace the whole
// `Vec<u64>` -- a silent wrong-place write, strictly worse than the E0070.  And the
// `&mut` cannot be written or dereferenced by hand: both `replace(&mut *a1, v)` and
// `replace(*a1, v)` make rule-preprocessor ABORT with
// `semantic.rs:260: unresolved access="unknown"` -- a deref or re-borrow of a
// placeholder inside a call argument has no access classification.
// ⭐ THE ROUTE THAT WORKS IS THE RECEIVER ROUTE, straight off the gate above: as the
// receiver of `.as_mut_slice()` the placeholder gets `is_method_call_receiver=true`,
// `needs_explicit_mut_borrow` is false, and the converter emits the bare place and
// lets Rust's autoref supply the `&mut`.  Emitted text becomes
//     (*gen_).as_mut_slice()[i_] = n_ & 0xFFFF_FFFFu64;
// which indexes the SLICE -- still the element, still in place, no copy of the Vec.
// Proof the receiver route is what the converter does: `a1.len()` in the guard above
// already emits `(*gen_).len()` with no `&mut`.
// ⚠️ THE DRAW-INDEX BUMP IS ALSO RESEQUENCED, and it has to be: the old
// `a1[624] = a1[624] + 1u64` mentions the param TWICE in one expression, and with
// the LHS now an `&mut`-autoref receiver that is an E0502 against the shared read on
// the RHS.  `idx_` on the line above is exactly `a1[624] as usize`, so
// `idx_ as u64 + 1u64` is the same value with one mention.  (Values here are < 624,
// so the `as usize`/`as u64` round-trip is exact.)
// ⚠️ This module ships an unsafe overlay ONLY -- there is no tgt_refcount.rs, so
// there is no second model to keep in step.  Do not assume symmetry with
// vector/deque_iterator, which do have one.
// ⚠️ NEGATIVE CONTROL, do not "fix" it: rules/builtin f9/f10/f12/f13 also store
// through a parameter, but `a2` is `*mut i64`, a RAW POINTER -- there the `*` is kept
// as literal TEXT in the IR and `borrow_mut` renders the pointer, so the deref is
// genuine and correct.  This defect requires a `&mut`-declared parameter.
// ===========================================================================
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
                    a1.as_mut_slice()[i_] = n_ & 0xFFFF_FFFFu64;
                    i_ += 1;
                }
                a1.as_mut_slice()[624] = 0u64;
            }
            let idx_ = a1[624] as usize;
            a1.as_mut_slice()[624] = idx_ as u64 + 1u64;
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
// f8 -- the `unsigned long`-engine twin of f3; see the comment block at f3 for why
// the three engine-state stores go through `.as_mut_slice()[..]` and why the
// draw-index bump reads `idx_ as u64 + 1u64`.
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
                    a1.as_mut_slice()[i_] = n_ & 0xFFFF_FFFFu64;
                    i_ += 1;
                }
                a1.as_mut_slice()[624] = 0u64;
            }
            let idx_ = a1[624] as usize;
            a1.as_mut_slice()[624] = idx_ as u64 + 1u64;
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

