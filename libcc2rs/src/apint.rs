// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

//! `llvm::APInt` -- A FIXED-WIDTH INTEGER WHOSE WIDTH IS PART OF THE VALUE.
//!
//! WHY THIS TYPE LIVES IN `libcc2rs` AND NOT IN A RULE BODY
//! -------------------------------------------------------
//! A rule is `fn tN() -> <target type> { <init> }`; there is no mechanism to
//! introduce a named `struct` alongside a rule, and
//! `grep -rn '^\s*\(pub \)\?struct ' rules/*/tgt_*.rs` over the whole rule tree
//! returns zero.  The only composite ever written in a rule body is a tuple
//! (`rules/mlir/tgt_unsafe.rs:368`), and that tuple key carries NO member rules
//! because a method call whose receiver type contains a tuple is not resolved by
//! the rule preprocessor.  Every `APInt` member this models is a call on an
//! `APInt` RECEIVER, so a tuple model could not carry them even if the values
//! were right.  A named struct declared here sidesteps that entirely, exactly as
//! `libcc2rs::InFlightDiagnostic` (diag.rs) does for `mlir::InFlightDiagnostic`,
//! which is the standing precedent for a libcc2rs struct used as a rule type key
//! with members written in receiver form.
//!
//! WHY A WIDTH-LESS MODEL IS WRONG, AND NOT HYPOTHETICALLY
//! ------------------------------------------------------
//! `getSExtValue()` SIGN-EXTENDS FROM `BitWidth` (APInt.h:1563,
//! `SignExtend64(U.VAL, BitWidth)`).  A 32-bit `APInt` holding `0xFFFFFFFF` is
//! `-1` in C++ and `+4294967295` read out of a width-less `u64`/`i64` model.
//! That does not fail -- it RUNS and yields a wrong answer.  The observer is in
//! this corpus: `dcc/src/Dialect/Sentient/SentientOps.cpp:1074,1080,1081` --
//! `step.getValue().getSExtValue()` and
//! `ub.getValue().getSExtValue() - lb.getValue().getSExtValue()`, i.e. a LOOP
//! BOUND.  Carrying `bit_width` is what makes `getSExtValue`, `getBitWidth` and
//! `eq` writable at all.
//!
//! THE INVARIANT: UNUSED BITS ARE ALWAYS CLEAR
//! -------------------------------------------
//! LLVM maintains "the bits above `BitWidth` in the single-word arm are zero"
//! (`clearUnusedBits`), and APInt.h:132 shows the store:
//!     `if (isSingleWord()) { U.VAL = val; if (implicitTrunc || isSigned) clearUnusedBits(); }`
//! In every case that does not abort, the stored value is therefore `val` masked
//! to `bit_width`, so MASKING IS ALWAYS THE CORRECT STORE and this model masks
//! unconditionally.  `get_zext_value` can then be a plain field read, matching
//! APInt.h:1541 (`return U.VAL;`).
//!
//! ⚠️ WHAT THIS MODEL CANNOT ANSWER: WIDTHS ABOVE 64.
//! `isSingleWord()` means `BitWidth <= 64`; above that the value lives in
//! `U.pVal[]` and a single `u64` cannot hold it.  `bit_width` is a RUNTIME
//! expression at the C++ call site, so a translate-time refusal is impossible.
//! The three alternatives are: silently truncate (the silent-wrongness class this
//! model exists to remove), leave the type unmapped (which is worse -- an
//! unmapped MEMBER is emitted textually at rc=0 with no placeholder token), or
//! fail at run time on the one input that cannot be represented.  This picks the
//! third: `new` PANICS when `num_bits > 64`.  That is not `todo!()`/
//! `unimplemented!()` -- it is a real, reachable, correct check on an
//! unrepresentable value, and it is STRICTER than release C++, never laxer.
//!
//! ⚠️ NOT MODELLED, deliberately: every arithmetic and bitwise operator, the
//! ordering comparisons, `trunc`/`sext`/`zext`/`getActiveBits`/`countLeadingZeros`,
//! the multi-word constructors, and `toString`.  Each would be a short body
//! against these two fields, but a member nobody reaches is a member nobody has
//! checked, and an unmapped member is silent rather than loud -- so they are
//! added when a site appears, not before.

/// `llvm::APInt`, modelled as its `BitWidth` plus its single-word value.
///
/// `Copy` because C++ copies an `APInt` by value freely and the single-word arm
/// owns nothing; `Clone` follows.
///
/// ⚠️ `PartialEq` IS DERIVED AND IS NOT `llvm::APInt::operator==`.  Derived
/// equality compares `bit_width` too, so it answers `false` for operands of
/// different widths where C++'s `operator==` ASSERTS (APInt.h:1080, `eq` is
/// `return (*this) == RHS;`).  The divergence is confined to a path on which the
/// C++ program aborts, and the keyed member is [`APInt::eq`], which reproduces
/// the assertion.  Derived `PartialEq` exists only so this type can sit inside a
/// container that the converter compares structurally.
///
/// ```
/// let a = libcc2rs::APInt::new(32, 0xFFFF_FFFF, false, false);
/// assert_eq!(a.get_bit_width(), 32);
/// assert_eq!(a.get_zext_value(), 0xFFFF_FFFF);
/// // The whole point: sign-extension is FROM BitWidth, not from 64.
/// assert_eq!(a.get_sext_value(), -1);
/// let b = libcc2rs::APInt::new(64, 0xFFFF_FFFF, false, false);
/// assert_eq!(b.get_sext_value(), 4294967295);
/// ```
#[derive(Copy, Clone, PartialEq, Eq, Debug)]
pub struct APInt {
    /// `APInt.h:1943 -- unsigned BitWidth = 1;`  Always `1 ..= 64` here.
    bit_width: u32,
    /// The single-word value, ALWAYS masked to `bit_width` (see the invariant
    /// above), so no reader has to mask again.
    value: u64,
}

impl APInt {
    /// `APInt.h:111 -- APInt(unsigned numBits, uint64_t val, bool isSigned = false, bool implicitTrunc = false)`.
    ///
    /// This is the ONLY constructor entry, and deliberately so: defaulted
    /// arguments are written out at the C++ call site, so the 2-, 3- and 4-argument
    /// spellings all record as the same 4-argument key.
    ///
    /// `is_signed` and `implicit_trunc` are accepted and then not branched on.
    /// That is not an oversight -- it is what APInt.h:111-132 says.  They select
    /// only WHICH DEBUG ASSERTION guards the store (`isIntN` vs `isUIntN`, and
    /// `implicitTrunc` suppresses it); the value actually stored is `val` masked
    /// to `numBits` in every case that does not abort.  Reproducing a debug-only
    /// assertion as a release panic would reject programs that release C++ accepts
    /// and that this model already answers identically, so the range check is
    /// deliberately NOT reproduced while the masking -- the part that is always
    /// observable -- is.
    ///
    /// # Panics
    /// If `num_bits` is 0 (`APInt` requires at least one bit) or greater than 64
    /// (unrepresentable in this model -- see the module note; the alternative is
    /// silent truncation).
    pub fn new(num_bits: u32, val: u64, is_signed: bool, implicit_trunc: bool) -> Self {
        let _ = (is_signed, implicit_trunc);
        assert!(num_bits != 0, "llvm::APInt requires BitWidth >= 1, got 0");
        assert!(
            num_bits <= 64,
            "libcc2rs::APInt models only the single-word arm of llvm::APInt \
             (BitWidth <= 64); BitWidth {num_bits} needs the multi-word U.pVal[] \
             representation and CANNOT be held in this model -- refusing rather \
             than silently truncating"
        );
        APInt {
            bit_width: num_bits,
            value: mask_to_width(val, num_bits),
        }
    }

    /// `APInt.h:1489 -- getBitWidth()`, the stored width.
    pub fn get_bit_width(&self) -> u32 {
        self.bit_width
    }

    /// `APInt.h:1541 -- getZExtValue()`, `return U.VAL;`.  A plain field read is
    /// correct because the unused-bits-clear invariant is maintained on store.
    /// C++ asserts `getActiveBits() <= 64` here, which `bit_width <= 64` makes
    /// unconditionally true, so there is nothing left to check.
    pub fn get_zext_value(&self) -> u64 {
        self.value
    }

    /// `APInt.h:1563 -- getSExtValue()`, `return SignExtend64(U.VAL, BitWidth);`.
    /// ⭐ A REAL SIGN-EXTENSION FROM `bit_width`, NOT A CAST: shift the value up
    /// so its top bit lands in bit 63, then shift back down ARITHMETICALLY.
    pub fn get_sext_value(&self) -> i64 {
        let shift = 64 - self.bit_width;
        ((self.value << shift) as i64) >> shift
    }

    /// `APInt.h:1080 -- bool eq(const APInt &RHS) const { return (*this) == RHS; }`.
    ///
    /// ⛔ A BARE VALUE COMPARE WOULD BE THE BUG, NOT THE FIX: `eq` delegates to
    /// `operator==`, which ASSERTS that both operands have the same bit width.
    /// Comparing values across widths is undefined behaviour in the C++ program,
    /// so this reproduces the assertion instead of inventing an answer for it.
    ///
    /// # Panics
    /// If the two operands have different bit widths, as C++ asserts.
    pub fn eq(&self, rhs: &APInt) -> bool {
        assert!(
            self.bit_width == rhs.bit_width,
            "llvm::APInt::operator== asserts equal bit widths; compared {} bits \
             against {} bits",
            self.bit_width,
            rhs.bit_width
        );
        self.value == rhs.value
    }
}

/// `clearUnusedBits` as a pure function: keep the low `bit_width` bits.
/// Split on 64 because `1u64 << 64` overflows.
fn mask_to_width(val: u64, bit_width: u32) -> u64 {
    if bit_width >= 64 {
        val
    } else {
        val & ((1u64 << bit_width) - 1)
    }
}

impl Default for APInt {
    /// `APInt.h:1943` gives `BitWidth` the in-class initialiser `1`, so a
    /// default-constructed `llvm::APInt` is a ONE-BIT ZERO -- not a 64-bit zero.
    fn default() -> Self {
        APInt {
            bit_width: 1,
            value: 0,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::APInt;

    #[test]
    fn sext_is_from_bit_width_not_from_64() {
        // The observer: SentientOps.cpp:1074,1080,1081 reads a loop bound out of
        // an IntegerAttr's APInt via getSExtValue().
        assert_eq!(APInt::new(32, 0xFFFF_FFFF, false, false).get_sext_value(), -1);
        assert_eq!(APInt::new(8, 0xFF, false, false).get_sext_value(), -1);
        assert_eq!(APInt::new(1, 1, false, false).get_sext_value(), -1);
        assert_eq!(APInt::new(64, u64::MAX, false, false).get_sext_value(), -1);
        // Positive values are unaffected, including at the width boundary.
        assert_eq!(APInt::new(32, 7, false, false).get_sext_value(), 7);
        assert_eq!(APInt::new(64, 0xFFFF_FFFF, false, false).get_sext_value(), 4294967295);
        assert_eq!(APInt::new(33, 0xFFFF_FFFF, false, false).get_sext_value(), 4294967295);
    }

    #[test]
    fn store_masks_to_width() {
        let a = APInt::new(8, 0x1FF, false, true);
        assert_eq!(a.get_zext_value(), 0xFF);
        assert_eq!(a.get_bit_width(), 8);
        // A negative value passed as uint64_t, as `isSigned` callers do.
        let b = APInt::new(32, (-1i64) as u64, true, false);
        assert_eq!(b.get_zext_value(), 0xFFFF_FFFF);
        assert_eq!(b.get_sext_value(), -1);
    }

    #[test]
    fn eq_requires_equal_widths() {
        assert!(APInt::new(32, 5, false, false).eq(&APInt::new(32, 5, false, false)));
        assert!(!APInt::new(32, 5, false, false).eq(&APInt::new(32, 6, false, false)));
    }

    #[test]
    #[should_panic(expected = "equal bit widths")]
    fn eq_across_widths_panics_as_cxx_asserts() {
        let _ = APInt::new(32, 5, false, false).eq(&APInt::new(64, 5, false, false));
    }

    #[test]
    #[should_panic(expected = "CANNOT be held in this model")]
    fn above_single_word_refuses() {
        let _ = APInt::new(128, 1, false, false);
    }

    #[test]
    fn default_is_a_one_bit_zero() {
        let d = APInt::default();
        assert_eq!(d.get_bit_width(), 1);
        assert_eq!(d.get_zext_value(), 0);
    }
}
