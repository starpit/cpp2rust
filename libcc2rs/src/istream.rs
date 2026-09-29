// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

//! `IStream` -- the STICKY-FAILBIT input-stream wrapper.
//!
//! WHY THIS TYPE EXISTS AND WHY A `Result`-PER-CALL MODEL IS NOT A SUBSTITUTE.
//! C++ stream error state is STICKY.  `operator>>` opens with a `sentry` that
//! checks `good()`; if the stream is already failed the sentry converts to
//! `false` and the extractor RETURNS WITHOUT TOUCHING ITS ARGUMENT
//! (libcxx/istream, `basic_istream::operator>>`; [istream.formatted.reqmts]).
//! So after one failed extraction EVERY LATER `>>` on that stream is a no-op.
//! A model that returns a per-call `Result`, or that silently succeeds again
//! after a failure, CHANGES PROGRAM MEANING: the canonical corpus shape
//!
//! ```text
//! while (ss >> tok) { ... }                        // util/dtgetenv.hpp:127
//! ```
//!
//! terminates in C++ at the first unparsable token and would loop forever, or
//! consume past the end, under a non-sticky model.  Nothing downstream catches
//! that -- it compiles, runs, and produces different output.  Hence: the flag
//! lives in the stream, `>>` is a no-op once it is set, and `operator!()` /
//! `operator bool()` / `fail()` / `good()` read it.
//!
//! `>>` RETURNS THE STREAM so `in >> a >> b` chains, and because a rule body is
//! INLINED as one expression the chained form lowers to
//! `shr(shr(in, a), b)` -- the inner call hands the SAME stream out, so the
//! outer one observes the inner one's failbit.  That is why every `shr_*` free
//! function below takes and returns the stream handle and uses it EXACTLY ONCE
//! (`Ptr<T>` is `Clone`, not `Copy`, and a rule body's `aN` re-expands to the
//! receiver expression verbatim, so a body that named the receiver twice would
//! both duplicate side effects and risk a move).
//!
//! FIRST-FAILURE VALUE.  C++11 [facet.num.get.virtuals]/3 makes a FAILED
//! numeric extraction store `0` in its argument (the C++98 "leave it alone"
//! behaviour was changed by LWG 2176).  So the first failing extraction zeroes
//! the argument and sets failbit; every SUBSEQUENT extraction leaves the
//! argument strictly untouched.  Both halves are tested below.
//!
//! SCOPE.  This is the byte-buffer half: the state machine, the stickiness and
//! the character-level parsers, which is what the ten queue rows carrying
//! `needs-sticky-failbit-wrapper-in-libcc2rs` were blocked on.  Attaching it to
//! a FILE-backed stream (`std::ifstream`, `std::cin`) needs a refill source and
//! belongs to `rules/fstream`'s owner; `IStream::from_bytes` is the seam for it.

use crate::Ptr;

/// `std::ios_base::basefield` values, i.e. the conversion base a numeric
/// extraction uses.  These are the THREE bases C++ can select
/// ([ios.fmtflags]: `dec`, `hex`, `oct`); `basefield == 0` -- "prefix decides"
/// -- is deliberately NOT modelled, because no corpus site sets it and an
/// unmodelled value must not be guessable as a silent 10.
pub const IOS_BASEFIELD_DEC: u32 = 10;
/// See [`IOS_BASEFIELD_DEC`].
pub const IOS_BASEFIELD_HEX: u32 = 16;
/// See [`IOS_BASEFIELD_DEC`].
pub const IOS_BASEFIELD_OCT: u32 = 8;

/// The `std::ios_base::iostate` bits this model carries.  `badbit` is present
/// because `operator!()` is `fail() || bad()` and a model that dropped it would
/// answer `!in` wrongly for a stream that only ever went bad.
///
/// ⭐ `basefield` IS FORMATTING STATE, NOT ERROR STATE, AND IT IS HERE FOR A
/// MEASURED REASON.  `dcg/tools/mda/memDumpAnalyzer.h:244` is
/// `inFile >> std::hex >> lineno;` and then `addr = lineno * 4 + bank;`.  Before
/// this field existed there was no honest rule for `std::hex`: the only body
/// writable against the model was the identity, which type-checks, translates
/// `rc=0`, and then parses `"1f"` as DECIMAL -- `1` instead of `31` -- so a
/// memory-dump analyzer computes a WRONG ADDRESS with no diagnostic at any
/// stage.  That is the same silent-wrongness class `rules/iostream`'s
/// `std::left`/`std::boolalpha` refusal is written about, and it is why the
/// radix lives in the stream instead of being dropped.
///
/// ⛔ IT IS NOT RESET BY `clear()`.  `basic_ios::clear()` resets `iostate`; the
/// FORMAT FLAGS are independent and survive it ([ios.base.locales],
/// [basic.ios.members]).  A `clear()` that also reset the base would silently
/// change what a later `>>` parsed.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct IStream {
    buf: Vec<u8>,
    pos: usize,
    failbit: bool,
    eofbit: bool,
    badbit: bool,
    basefield: u32,
}

/// ⛔ HAND-WRITTEN, NOT DERIVED, AND THAT IS THE WHOLE POINT OF THE DEFAULT-PATH
/// GUARANTEE.  `#[derive(Default)]` would give `basefield == 0`, which is not a
/// base at all, and every already-landed `f11`..`f18` extraction would then parse
/// through `from_str_radix(_, 0)` -- a PANIC in `core`, on a path that is today
/// `rc=0` and correct.  Defaulting to 10 is what makes this change a pure
/// addition: a stream nobody applied a manipulator to behaves exactly as it did
/// before the field existed.
impl Default for IStream {
    fn default() -> Self {
        Self {
            buf: Vec::new(),
            pos: 0,
            failbit: false,
            eofbit: false,
            badbit: false,
            basefield: IOS_BASEFIELD_DEC,
        }
    }
}

impl IStream {
    /// An empty stream.  Immediately at EOF, but NOT failed: in C++ a
    /// default-constructed stringstream is `good()` until something is read
    /// from it.
    pub fn new() -> Self {
        Self::default()
    }

    /// Wrap an owned byte buffer.  The read position starts at 0, which is what
    /// `std::istringstream iss(s)` and `std::stringstream ss(s)` (mode
    /// `in | out`, no `ate`) both do.
    pub fn from_bytes(buf: Vec<u8>) -> Self {
        Self {
            buf,
            ..Self::default()
        }
    }

    /// Bytes not yet consumed -- the observable that proves a failed extraction
    /// consumed nothing and that a sticky no-op read nothing.
    pub fn remaining(&self) -> &[u8] {
        &self.buf[self.pos.min(self.buf.len())..]
    }

    /// The WHOLE underlying buffer, independent of the read cursor -- this is
    /// what `std::basic_stringstream::str()` returns.  `str()` is specified on
    /// the `basic_stringbuf`, not on the get area, so a stream that has already
    /// been read from still reports every byte it holds; a model that returned
    /// `remaining()` here would silently shrink `ss.str()` after each `>>`.
    pub fn buf(&self) -> &[u8] {
        &self.buf
    }

    // -- state, exactly the `basic_ios` predicates the corpus asks for --------

    pub fn fail(&self) -> bool {
        self.failbit || self.badbit
    }

    pub fn bad(&self) -> bool {
        self.badbit
    }

    pub fn eof(&self) -> bool {
        self.eofbit
    }

    pub fn good(&self) -> bool {
        !self.failbit && !self.badbit && !self.eofbit
    }

    /// `operator!()` -- `basic_ios::operator!` is specified as `fail()`.
    pub fn not_op(&self) -> bool {
        self.fail()
    }

    /// `operator bool()` -- `!fail()`.
    pub fn to_bool(&self) -> bool {
        !self.fail()
    }

    /// `clear()` with the default argument: all bits reset.
    pub fn clear(&mut self) {
        self.failbit = false;
        self.eofbit = false;
        self.badbit = false;
    }

    pub fn setstate_fail(&mut self) {
        self.failbit = true;
    }

    // -- basefield, i.e. `std::hex` / `std::dec` / `std::oct` -----------------

    /// The conversion base a numeric extraction will use.  Exposed so a test --
    /// and a reader auditing whether a manipulator was actually threaded through
    /// -- can observe it; nothing in the rule bodies reads it.
    pub fn basefield(&self) -> u32 {
        self.basefield
    }

    /// `std::ios_base::setf(base, basefield)`, i.e. the whole observable effect
    /// of `std::hex` / `std::dec` / `std::oct`.
    ///
    /// ⛔ THE ARGUMENT IS NOT VALIDATED HERE AND MUST NOT BE.  The only callers
    /// are the three `rules/iostream` manipulator keys, each of which inlines one
    /// of the three `IOS_BASEFIELD_*` constants LITERALLY into the emitted Rust,
    /// so an out-of-range base cannot be constructed by translated code.  A
    /// silent clamp would be the forbidden trade in miniature: it would turn a
    /// future mis-keyed manipulator into a wrong parse instead of a panic.
    pub fn set_basefield(&mut self, base: u32) {
        self.basefield = base;
    }

    /// The RULE-ABI form of [`Self::set_basefield`]: hands the stream back so a
    /// rule body can name each operand exactly once, exactly as `shr_i32` and
    /// friends do.  This is what `std::istream::operator>>(ios_base &(*)(ios_base &))`
    /// lowers to.
    pub fn shr_basefield(&mut self, base: u32) -> *mut Self {
        self.set_basefield(base);
        self
    }

    /// Apply an `std::ios_base` MANIPULATOR to this stream's formatting state,
    /// in the UNSAFE model's spelling.
    ///
    /// This is the lowering of `in >> std::hex`, i.e. of the member
    /// `basic_istream::operator>>(ios_base &(*)(ios_base &))`
    /// (`rules/iostream` f101).  The manipulator arrives as a real Rust `fn`
    /// item, because that is what the converter emits for a keyed system
    /// function that is NAMED rather than CALLED: `libcc2rs::hex_unsafe`
    /// (`Mapper::MapFunctionName`, cpp2rust/converter/mapper.cpp:2660).  Same
    /// two-route shape as `rules/cctype` + `libcc2rs::cctype`.
    ///
    /// ⛔⛔ THE PARAMETER TYPE IS NOT A DESIGN CHOICE, IT IS MEASURED.  The
    /// converter does not hand the operand over bare -- it CASTS it to the
    /// model-mapped C++ type.  The emitted text on
    /// `util/sendefs/sendefs.cpp:432`, read out of the translated `.rs`, is
    /// literally
    ///     (*ss.shr_ios_manip((libcc2rs::hex_unsafe as unsafe fn(*mut u32) -> *mut u32)))
    /// so with `rules/iostream` t6 mapping `std::ios_base` to `u32`, the operand's
    /// type is `unsafe fn(*mut u32) -> *mut u32` and nothing else type-checks.
    /// A first attempt used `fn(u32) -> u32` here; it translated rc=0 and would
    /// have failed one stage later with E0308, which is exactly the class of
    /// error `rc=0` cannot see.
    ///
    /// ⭐ AND IT IS THE MORE FAITHFUL SHAPE ANYWAY: `ios_base& hex(ios_base& s)`
    /// really does mutate its argument and hand it back, so the state word is
    /// passed BY ADDRESS, the manipulator writes through it, and the returned
    /// pointer is read back.  A null return is treated as "no change" rather than
    /// dereferenced, because a manipulator that loses its argument must not turn
    /// into a segfault inside a stream read.
    ///
    /// # Safety
    /// `manip` must be one of the `libcc2rs::{hex,dec,oct}_unsafe` items, or any
    /// function that accepts a valid `*mut u32` and returns either null or a
    /// pointer that is valid for reads.
    pub unsafe fn set_basefield_via(&mut self, manip: unsafe fn(*mut u32) -> *mut u32) {
        let mut word = self.basefield;
        // SAFETY: `&mut word` is valid for reads and writes for the call, and the
        // contract on `manip` says the pointer it returns is null or readable.
        let next = unsafe {
            let out = manip(&mut word as *mut u32);
            if out.is_null() {
                word
            } else {
                *out
            }
        };
        self.set_basefield(next);
    }

    /// The RULE-ABI form of [`Self::set_basefield_via`]: hands the stream back,
    /// so `inFile >> std::hex >> lineno` chains exactly as `shr_i64` does and each
    /// operand is named exactly once.
    ///
    /// # Safety
    /// See [`Self::set_basefield_via`].
    pub unsafe fn shr_ios_manip(
        &mut self,
        manip: unsafe fn(*mut u32) -> *mut u32,
    ) -> *mut Self {
        // SAFETY: forwarding this function's own contract on `manip`.
        unsafe { self.set_basefield_via(manip) };
        self
    }

    /// The REFCOUNT model's spelling of the same operation.
    ///
    /// ⚠️ THE TWO MODELS ARE ASYMMETRIC HERE AND THAT IS MEASURED, NOT SLOPPY.
    /// On the refcount arm the converter emits the operand with NO cast at all --
    /// the translated `.rs` for the same C++ site reads
    ///     let __m = libcc2rs::hex_refcount;
    ///     __s.with_mut_ref(|__st| __st.set_basefield_via_value(__m));
    /// -- so there is no `Ptr<u32>` in the emitted text to match and the plain
    /// state-transformer shape is both what type-checks and what is simplest.
    /// `hex_refcount` therefore has a different signature from `hex_unsafe`; they
    /// are different names to begin with, because `MapFunctionName` appends the
    /// model suffix unconditionally.
    pub fn set_basefield_via_value(&mut self, manip: fn(u32) -> u32) {
        let next = manip(self.basefield);
        self.set_basefield(next);
    }

    // -- the sentry -----------------------------------------------------------

    /// `basic_istream::sentry` with `noskipws == false`: returns false when the
    /// stream is already failed (THE STICKY CASE -- the caller must then return
    /// without touching its argument), otherwise skips whitespace and reports
    /// whether anything is left.
    fn sentry(&mut self) -> bool {
        if self.fail() {
            return false;
        }
        while self.pos < self.buf.len() && self.buf[self.pos].is_ascii_whitespace() {
            self.pos += 1;
        }
        if self.pos >= self.buf.len() {
            // Ran out of input while looking for a field: eofbit AND failbit,
            // which is what makes `while (in >> x)` terminate.
            self.eofbit = true;
            self.failbit = true;
            return false;
        }
        true
    }

    /// Consume the longest prefix accepted by `accept`, starting at `pos`.
    /// Returns `None` (and rewinds nothing, because nothing was consumed) when
    /// the first character is not accepted.
    fn take_field(&mut self, accept: impl Fn(usize, u8) -> bool) -> Option<&str> {
        let start = self.pos;
        let mut end = start;
        while end < self.buf.len() && accept(end - start, self.buf[end]) {
            end += 1;
        }
        if end == start {
            return None;
        }
        self.pos = end;
        if self.pos >= self.buf.len() {
            self.eofbit = true;
        }
        std::str::from_utf8(&self.buf[start..end]).ok()
    }

    fn signed_field(&mut self) -> Option<String> {
        self.take_field(|i, c| c.is_ascii_digit() || (i == 0 && (c == b'-' || c == b'+')))
            .map(str::to_owned)
    }

    fn unsigned_field(&mut self) -> Option<String> {
        self.take_field(|i, c| c.is_ascii_digit() || (i == 0 && c == b'+'))
            .map(str::to_owned)
    }

    /// Scan an integer field at `pos` in `base`, returned in the form
    /// `from_str_radix` accepts (optional sign, then digits, no `0x`).
    ///
    /// ⛔ THIS IS A SEPARATE SCANNER FROM `take_field` BECAUSE IT NEEDS
    /// LOOKAHEAD.  Base 16 accepts an OPTIONAL `0x`/`0X` prefix, but only when a
    /// hex digit follows: `strtol("0x", &e, 16)` consumes just the `0` and leaves
    /// the `x` behind, so a one-character-at-a-time predicate cannot make the
    /// decision.  A predicate that accepted `x` unconditionally would turn `"0x"`
    /// into a parse FAILURE where C++ reports 0 and success.
    ///
    /// ⭐ IT IS ONLY EVER CALLED FOR `base != 10`.  The base-10 path still runs
    /// `signed_field`/`unsigned_field` verbatim, which is what makes the
    /// already-landed `f11`..`f18` behaviour provably unchanged: it is literally
    /// the same code as before this field existed, not a re-derivation that
    /// happens to agree.
    fn take_radix_field(&mut self, base: u32, signed: bool) -> Option<String> {
        let mut i = self.pos;
        let mut out = String::new();
        if i < self.buf.len() && (self.buf[i] == b'+' || (signed && self.buf[i] == b'-')) {
            out.push(self.buf[i] as char);
            i += 1;
        }
        if base == 16
            && i + 2 < self.buf.len()
            && self.buf[i] == b'0'
            && (self.buf[i + 1] | 0x20) == b'x'
            && (self.buf[i + 2] as char).is_digit(16)
        {
            i += 2;
        }
        let dstart = i;
        while i < self.buf.len() && (self.buf[i] as char).is_digit(base) {
            i += 1;
        }
        if i == dstart {
            // Nothing consumed -- and `pos` was never advanced, because the scan
            // ran on a local cursor.  The caller turns this into failbit + 0.
            return None;
        }
        out.push_str(std::str::from_utf8(&self.buf[dstart..i]).ok()?);
        self.pos = i;
        if self.pos >= self.buf.len() {
            self.eofbit = true;
        }
        Some(out)
    }

    /// `signed_field` under the stream's current basefield.  See
    /// [`Self::take_radix_field`] for why base 10 is routed to the ORIGINAL code
    /// rather than through the new scanner.
    fn signed_field_radix(&mut self) -> Option<String> {
        if self.basefield == IOS_BASEFIELD_DEC {
            self.signed_field()
        } else {
            self.take_radix_field(self.basefield, true)
        }
    }

    /// `unsigned_field` under the stream's current basefield.
    fn unsigned_field_radix(&mut self) -> Option<String> {
        if self.basefield == IOS_BASEFIELD_DEC {
            self.unsigned_field()
        } else {
            self.take_radix_field(self.basefield, false)
        }
    }

    fn float_field(&mut self) -> Option<String> {
        self.take_field(|i, c| {
            c.is_ascii_digit()
                || c == b'.'
                || c == b'e'
                || c == b'E'
                || c == b'-'
                || c == b'+'
                || (i == 0 && (c == b'-' || c == b'+'))
        })
        .map(str::to_owned)
    }

    // -- extraction -----------------------------------------------------------
    //
    // Every one of these is: sentry (returns early and UNTOUCHED when sticky),
    // parse, and on a parse miss set failbit and store 0 (C++11 LWG 2176).

    pub fn extract_i64(&mut self, out: &mut i64) {
        if !self.sentry() {
            return;
        }
        // ⭐ `from_str_radix(_, 10)` IS EXACTLY `parse::<i64>()` for the strings
        // `signed_field` can produce (optional single `+`/`-`, then ASCII
        // digits), which is why routing the default path through it is not a
        // behaviour change.  With `std::hex` in effect the base is 16 and the
        // digits came from `take_radix_field`.
        let __base = self.basefield;
        match self
            .signed_field_radix()
            .and_then(|s| i64::from_str_radix(&s, __base).ok())
        {
            Some(v) => *out = v,
            None => {
                self.failbit = true;
                *out = 0;
            }
        }
    }

    // ⚠️ THE STICKY CHECK MUST COME BEFORE THE DELEGATION.  Written without it,
    // this body reached `extract_i64`, saw the ALREADY-SET failbit on return and
    // zeroed `*out` -- i.e. a sticky-failed `>>` still wrote to its argument.
    // The stickiness test below caught exactly that; it is the reason the test
    // asserts on the ARGUMENT and not only on the flag.
    pub fn extract_i32(&mut self, out: &mut i32) {
        if self.fail() {
            return;
        }
        let mut wide: i64 = 0;
        self.extract_i64(&mut wide);
        if self.failbit {
            // `num_get` also fails when the value does not fit the target.
            *out = 0;
            return;
        }
        match i32::try_from(wide) {
            Ok(v) => *out = v,
            Err(_) => {
                self.failbit = true;
                *out = 0;
            }
        }
    }

    pub fn extract_u64(&mut self, out: &mut u64) {
        if !self.sentry() {
            return;
        }
        // See `extract_i64` for why base 10 through `from_str_radix` is
        // byte-for-byte the previous `parse::<u64>()` behaviour.
        let __base = self.basefield;
        match self
            .unsigned_field_radix()
            .and_then(|s| u64::from_str_radix(&s, __base).ok())
        {
            Some(v) => *out = v,
            None => {
                self.failbit = true;
                *out = 0;
            }
        }
    }

    // Sticky check first, for the reason documented on `extract_i32`.
    pub fn extract_u32(&mut self, out: &mut u32) {
        if self.fail() {
            return;
        }
        let mut wide: u64 = 0;
        self.extract_u64(&mut wide);
        if self.failbit {
            *out = 0;
            return;
        }
        match u32::try_from(wide) {
            Ok(v) => *out = v,
            Err(_) => {
                self.failbit = true;
                *out = 0;
            }
        }
    }

    pub fn extract_f64(&mut self, out: &mut f64) {
        if !self.sentry() {
            return;
        }
        match self.float_field().and_then(|s| s.parse::<f64>().ok()) {
            Some(v) => *out = v,
            None => {
                self.failbit = true;
                *out = 0.0;
            }
        }
    }

    /// `operator>>(float&)`.
    ///
    /// ⛔ NOT `extract_f64` NARROWED WITH `as f32`, and the difference is a
    /// SILENT WRONG ANSWER rather than a rounding nicety.  `num_get` sets
    /// failbit when the parsed value does not fit the target type
    /// ([facet.num.get.virtuals]/3, C++11 LWG 2176), and `1e40 as f32` in Rust
    /// is a SATURATING cast that yields `f32::INFINITY` with no error -- so a
    /// `dtGetEnv<float>("X")` over `"1e40"` would come back `Some(inf)` where
    /// C++ returns `std::nullopt`.  The overflow is therefore detected
    /// explicitly, in the one place where the f64 is still finite.
    ///
    /// UNDERFLOW TO ZERO IS DELIBERATELY *NOT* A FAILURE: `1e-50` is a denormal
    /// loss that C++ also reports as success (num_get only fails on values
    /// outside the representable RANGE), so `0.0` is the right answer there.
    pub fn extract_f32(&mut self, out: &mut f32) {
        if !self.sentry() {
            return;
        }
        match self.float_field().and_then(|s| s.parse::<f64>().ok()) {
            Some(v) if v.is_finite() && (v as f32).is_finite() => *out = v as f32,
            // Either the field did not parse at all, or it parsed to a magnitude
            // no `float` can hold.  Both are `num_get` failures.
            _ => {
                self.failbit = true;
                *out = 0.0;
            }
        }
    }

    /// `operator>>(char&)`: one non-whitespace character.  A char extraction
    /// that fails leaves the argument alone -- `num_get` is not involved, so
    /// LWG 2176's zeroing does not apply here.
    pub fn extract_u8(&mut self, out: &mut u8) {
        self.extract_char_reporting(out);
    }

    /// ⭐ `extract_u8` FOR A RULE BODY, and both return values exist for the
    /// rule-ABI reason `extract_token_reporting` documents, not for elegance.
    ///
    /// The free `operator>>(std::istream &, char &)` key needs BOTH:
    ///
    /// 1. THE STREAM HANDLE BACK, because the corpus caller is
    ///    `sys-arch-spec/progir/regvisitor.cpp:285`
    ///    `DT_CHECK(success && regnum >= 0 && !static_cast<bool>(iss >> remaining_char));`
    ///    -- the extraction's RESULT is the point and the character is not. The
    ///    assertion is that the read FAILS, i.e. that no trailing garbage
    ///    follows the number, so `static_cast<bool>` must observe the failbit the
    ///    sentry sets on empty input.
    /// 2. A "was it written" FLAG, because this tree's `char` is
    ///    `libc::c_char` (unsafe model) / `u8` (refcount model) while this type
    ///    reads raw bytes, so the rule body needs a STAGING byte and must convert
    ///    on the way out -- and it MUST NOT write on the sticky/empty path.
    ///    `operator>>(char&)` is not a `num_get` extraction, so LWG 2176's
    ///    zero-on-failure does NOT apply: a failed char read leaves the
    ///    caller's variable strictly alone. An unconditional write-back would
    ///    zero `remaining_char` at the exact site that asserts the read failed.
    pub fn extract_char_reporting(&mut self, out: &mut u8) -> (*mut Self, bool) {
        if !self.sentry() {
            return (self, false);
        }
        *out = self.buf[self.pos];
        self.pos += 1;
        if self.pos >= self.buf.len() {
            self.eofbit = true;
        }
        (self, true)
    }

    // -- the numeric `operator>>` rule ABI ------------------------------------
    //
    // ⭐ EVERY ONE OF THESE EXISTS FOR A RULE-ABI REASON, not for convenience,
    // and the reason is the SAME ONE that made `extract_token_reporting` return
    // `*mut Self`: a rule body is INLINED and each `aN` re-expands to the
    // caller's argument expression VERBATIM, so a body may name each operand
    // EXACTLY ONCE.  `std::basic_istream::operator>>` must hand the stream back
    // (that is what makes `if (ss >> parsed)` a stream test and `in >> a >> b`
    // chain), and the `extract_*` methods above return `()`.  A body written as
    //     { a0.extract_i32(a1); a0 }
    // names `a0` twice -- for a caller like `f(v) >> x` that RE-EVALUATES
    // `f(v)`, on a fresh stream, so the extraction would be thrown away.  These
    // wrappers make the single-mention body possible.
    //
    // ⛔ AND THEY MUST BE METHODS, not free functions taking `&mut IStream`: for
    // a `std::istream &` parameter the converter emits the BARE LVALUE, not
    // `&mut lvalue` (measured; see `rules/iostream/tgt_unsafe.rs` f8, where
    // `let __s: *mut IStream = a0;` came out `let __s = ss;` with E0308, rc=0
    // and no placeholder).  A method call is immune because Rust auto-refs the
    // receiver, so `a0.shr_i32(..)` compiles both as `(&mut IStream).shr_i32`
    // and as `ss.shr_i32`.
    //
    // ⚠️ NO `shr_u8`/`shr_token` HERE, and the asymmetry is real rather than an
    // oversight.  `std::string` extraction needs `extract_token_reporting`'s
    // extra bool because the RULE BODY has to do the write-back itself (the
    // tree's `std::string` is a NUL-terminated `Vec<libc::c_char>`, so the rule
    // stages into a `Vec<u8>` and converts), and it must not write on the sticky
    // path.  A numeric target is written by the extractor DIRECTLY, and the two
    // failure cases are already distinguished in there:
    //   * sentry failed (sticky / empty input) -> the argument is UNTOUCHED;
    //   * sentry succeeded but the conversion failed -> zero, per LWG 2176.
    // So there is no decision left for the rule body to make, and no flag.

    /// `std::istream & operator>>(int &)`.
    pub fn shr_i32(&mut self, out: &mut i32) -> *mut Self {
        self.extract_i32(out);
        self
    }

    /// `std::istream & operator>>(unsigned int &)`.
    pub fn shr_u32(&mut self, out: &mut u32) -> *mut Self {
        self.extract_u32(out);
        self
    }

    /// `std::istream & operator>>(long &)` -- and `long long &`, which is a
    /// DISTINCT C++ overload with the identical Rust representation (both i64 on
    /// LP64), so both rule keys land on this one method.
    pub fn shr_i64(&mut self, out: &mut i64) -> *mut Self {
        self.extract_i64(out);
        self
    }

    /// `std::istream & operator>>(unsigned long &)` / `unsigned long long &`.
    pub fn shr_u64(&mut self, out: &mut u64) -> *mut Self {
        self.extract_u64(out);
        self
    }

    /// `std::istream & operator>>(float &)`.
    pub fn shr_f32(&mut self, out: &mut f32) -> *mut Self {
        self.extract_f32(out);
        self
    }

    /// `std::istream & operator>>(double &)`.
    pub fn shr_f64(&mut self, out: &mut f64) -> *mut Self {
        self.extract_f64(out);
        self
    }

    /// `operator>>(std::string&)`: one whitespace-delimited token.  The buffer
    /// is REPLACED (C++ calls `erase()` first) and the NUL terminator this
    /// tree's `std::string` model carries is NOT added here -- the caller's
    /// model decides, which is why the two string entry points below differ.
    pub fn extract_token(&mut self, out: &mut Vec<u8>) {
        self.extract_token_reporting(out);
    }

    /// ⭐ `extract_token` FOR A CALLER THAT MUST CONVERT THE STRING, and both of
    /// its return values exist for a rule-ABI reason, not for elegance.
    ///
    /// The tree's `std::string` is a NUL-TERMINATED `Vec<libc::c_char>` (unsafe
    /// model) while this type reads raw bytes, so the rule body needs a STAGING
    /// buffer and must convert back afterwards. Two problems follow, and this
    /// signature answers both:
    ///
    /// 1. ⛔ THE STICKY CASE MUST NOT WRITE THE ARGUMENT. `operator>>` on a
    ///    failed stream leaves its `std::string` strictly untouched
    ///    ([istream.formatted.reqmts]; libcxx clears the string only AFTER the
    ///    sentry succeeds). A rule body that copied its staging buffer back
    ///    unconditionally would ERASE the caller's string on every sticky read --
    ///    a silent wrong answer, and exactly the failbit-dropping mistake this
    ///    family is warned about. `bool` here is "the argument was written",
    ///    i.e. the sentry succeeded, so the body can guard the write.
    /// 2. A rule body is INLINED and every `aN` RE-EXPANDS to the argument
    ///    expression verbatim, so the body must name each operand EXACTLY ONCE.
    ///    Returning `*mut Self` lets the body mention the receiver once and still
    ///    hand the stream on, which is what makes `in >> a >> b` chain.
    pub fn extract_token_reporting(&mut self, out: &mut Vec<u8>) -> (*mut Self, bool) {
        if !self.sentry() {
            return (self, false);
        }
        let start = self.pos;
        while self.pos < self.buf.len() && !self.buf[self.pos].is_ascii_whitespace() {
            self.pos += 1;
        }
        if self.pos >= self.buf.len() {
            self.eofbit = true;
        }
        out.clear();
        out.extend_from_slice(&self.buf[start..self.pos]);
        (self, true)
    }

    /// `std::getline(in, s, delim)`: everything up to (and consuming) `delim`.
    /// Unlike `>>` this does NOT skip leading whitespace and DOES accept an
    /// empty field; it fails only when no character at all could be read.
    pub fn getline(&mut self, out: &mut Vec<u8>, delim: u8) {
        if self.fail() {
            return;
        }
        if self.pos >= self.buf.len() {
            self.eofbit = true;
            self.failbit = true;
            return;
        }
        let start = self.pos;
        while self.pos < self.buf.len() && self.buf[self.pos] != delim {
            self.pos += 1;
        }
        let end = self.pos;
        if self.pos < self.buf.len() {
            self.pos += 1; // consume the delimiter, which is not stored
        } else {
            self.eofbit = true;
        }
        out.clear();
        out.extend_from_slice(&self.buf[start..end]);
    }

    /// ⭐ `getline` FOR A RULE BODY THAT MUST CONVERT THE STRING -- the exact
    /// counterpart of `extract_token_reporting`, and it exists for the same two
    /// rule-ABI reasons, not for symmetry.
    ///
    /// 1. ⛔ THE STICKY CASE MUST NOT WRITE THE ARGUMENT. `std::getline` on an
    ///    already-failed stream, and on a stream with nothing left to read,
    ///    leaves its `std::string` strictly untouched (it sets failbit and
    ///    returns; libcxx erases the string only AFTER the sentry succeeds).
    ///    The tree's `std::string` is a NUL-terminated `Vec<libc::c_char>` in
    ///    the unsafe model, so a rule body must stage into a `Vec<u8>` and copy
    ///    back -- and an UNCONDITIONAL copy-back would ERASE the caller's
    ///    string on every sticky read, which compiles, runs, and is silently
    ///    wrong. The `bool` is "the argument was written", so the body can
    ///    guard the write-back.
    /// 2. A rule body is INLINED and every `aN` RE-EXPANDS to the argument
    ///    expression verbatim, so the body must name each operand EXACTLY ONCE.
    ///    Returning `*mut Self` lets the body mention the receiver once and
    ///    still hand the stream back, which is what makes `std::getline(...)`
    ///    usable as the condition of `while (std::getline(f, line))`.
    ///
    /// ⚠️ NOTE THE DIVERGENCE FROM `extract_token_reporting`: an EMPTY FIELD is
    /// a SUCCESS here (`"a,,b"` split on `','` yields an empty second token and
    /// the string MUST be cleared), whereas `>>` cannot produce one. So the
    /// flag is false only on the genuine no-character-read failure, never
    /// merely because the result is empty.
    pub fn getline_reporting(
        &mut self,
        out: &mut Vec<u8>,
        delim: u8,
    ) -> (*mut Self, bool) {
        if self.fail() || self.pos >= self.buf.len() {
            // Delegate so the bit-setting stays in ONE place: `getline` sets
            // eofbit+failbit on the exhausted-stream case and returns without
            // touching `out`.
            self.getline(out, delim);
            return (self, false);
        }
        self.getline(out, delim);
        (self, true)
    }
}

// ---------------------------------------------------------------------------
// THE PUT SIDE.  ⛔ THIS IS NOT OPTIONAL DECORATION -- IT IS THE ONE HARD
// CONSTRAINT ON USING `IStream` AS THE MODEL FOR `std::stringstream`.
//
// A `stringstream` is read AND written, and insertion into it is NOT
// rule-driven: `converter_lib.cpp:552` IsCallToOstream matches on the RESULT
// type being `basic_ostream`, and `converter.cpp` ConvertCallToOstream then
// emits, textually, `write!(<stream>, "<fmt>", args)` / `<stream>.write_all(..)`
// against `ToString()` of the left-most operand.  In the refcount model those
// land on `Ptr<T>`, whose `write_fmt`/`write_all` exist ONLY for
// `T: std::io::Write + ByteRepr` (`rc.rs:723`).  So a `T` without BOTH traits
// makes every put-side site an E0599 -- i.e. flipping `std::stringstream`'s
// model to `IStream` without these two impls would trade a read-side gap for a
// write-side regression.  Both are therefore supplied here.
//
// SEMANTICS OF THE WRITE: APPEND, and the divergence it carries is the one
// `rules/basic_stringstream` already measured and documented, unchanged by this
// type.  `std::stringstream ss(s)` opens `in|out` WITHOUT `ate`, so C++'s put
// position is 0 and an insertion OVERWRITES from the front (`abcXY` + `12` ->
// `12cXY`), while appending gives `abcXY12`.  `IStream` carries a GET cursor
// only, so it appends -- exactly what `Vec<u8>` did before the flip.  This is
// preserved deliberately rather than "fixed" here: a put cursor is a model
// change for the whole family, the corpus has no construct-then-insert site
// (the 36-site grep is in `rules/basic_stringstream/src.cpp`), and the members
// that would observe the difference (`seekp`/`tellp`/the `str()` setter) stay
// unmapped.  ⭐ The point: this impl is a strict no-regression move, not a new
// claim.
// ---------------------------------------------------------------------------

impl std::io::Write for IStream {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        self.buf.extend_from_slice(buf);
        Ok(buf.len())
    }

    fn flush(&mut self) -> std::io::Result<()> {
        Ok(())
    }
}

/// `ByteRepr` with every method left at its PANICKING default -- the same
/// claim-nothing shape `reinterpret.rs:160-172` already uses for `BTreeMap`,
/// `HashMap` and `HashSet`.  ⭐ WHY THIS IS HONEST: the bound on
/// `Ptr::with_mut` / `Ptr::write_fmt` is a whole-method requirement imposed by a
/// SINGLE arm, `PtrKind::Reinterpreted`, which is the only place `byte_size` /
/// `to_bytes` / `from_bytes` are called (see the `Ptr::with_ref` doc at
/// `rc.rs:566`).  A stream is never reached through a reinterpreted pointer, so
/// the impl claims no byte layout and any attempt to use one fails LOUDLY with
/// the default's panic rather than fabricating bytes for a `Vec` + cursor +
/// three flags.
impl crate::ByteRepr for IStream {}

// ---------------------------------------------------------------------------
// The refcount-model entry points.
//
// Each takes the stream handle ONCE and hands it back so `in >> a >> b` chains
// through the SAME object and the outer extraction sees the inner one's
// failbit.  `with_mut_ref` is the bound-free deref (no `T: ByteRepr`), which is
// what makes an `IStream` usable behind a `Ptr` at all.
// ---------------------------------------------------------------------------

macro_rules! shr_fn {
    ($name:ident, $t:ty, $method:ident) => {
        pub fn $name(s: Ptr<IStream>, out: Ptr<$t>) -> Ptr<IStream> {
            s.with_mut_ref(|st| out.with_mut_ref(|o| st.$method(o)));
            s
        }
    };
}

shr_fn!(istream_shr_i64, i64, extract_i64);
shr_fn!(istream_shr_i32, i32, extract_i32);
shr_fn!(istream_shr_u64, u64, extract_u64);
shr_fn!(istream_shr_u32, u32, extract_u32);
shr_fn!(istream_shr_f64, f64, extract_f64);
shr_fn!(istream_shr_u8, u8, extract_u8);
shr_fn!(istream_shr_string, Vec<u8>, extract_token);

// ════════════════════════════════════════════════════════════════════════════
// `std::hex` / `std::dec` / `std::oct` AS REAL FUNCTION ITEMS.
//
// WHY THESE EXIST AT ALL, AND WHY THEY ARE NOT RULE BODIES.  `rules/iostream`
// f102/f103/f104 key the three manipulators, but the corpus never CALLS one --
// it only NAMES one, as the operand of `in >> std::hex`.  An inlined rule body
// has no address, so for a NAMED keyed system function the converter emits
// `libcc2rs::<name>_<model>` instead (`Mapper::MapFunctionName`,
// cpp2rust/converter/mapper.cpp:2660: `std::format("libcc2rs::{}_{}", ...)`).
// These are those six names.  `libcc2rs/src/cctype.rs` is the standing
// precedent for the identical two-route shape (`::tolower` named vs called).
//
// ⛔ THE `_unsafe` TWINS ARE NOT DECORATION.  `MapFunctionName` appends the
// model suffix unconditionally, so the unsafe arm asks for `hex_unsafe` and the
// refcount arm for `hex_refcount`; a single name satisfies exactly one of the
// two models and leaves the other with an E0425 at rc=0.  They forward to one
// body so the two models cannot drift, exactly as `tolower_unsafe` does.
//
// SEMANTICS, MEASURED AGAINST EXECUTED C++ (g3084probe/hexrow.cpp):
// `ios_base& hex(ios_base& s) { s.setf(ios_base::hex, ios_base::basefield); return s; }`
// -- the whole observable effect on an INPUT stream is the new basefield, and
// it is STICKY (`in >> std::hex >> a >> b` reads BOTH in hex; `>> std::dec`
// restores 10).  The incoming state word is therefore DISCARDED rather than
// masked: `setf(v, mask)` REPLACES the masked field, it does not or-in.
//
// ⛔⛔ THE STATE WORD IS THE BASEFIELD, NOT THE WHOLE `fmtflags`.  `rules/iostream`
// t6 maps `std::ios_base` to this one `u32`, which is honest for the three radix
// manipulators and is ALL the corpus's input half needs.  It is NOT a model of
// `adjustfield`/`floatfield`/`fill`/`width`, so `std::left`, `std::right`,
// `std::setw`, `std::setfill`, `std::setprecision`, `std::fixed` and
// `std::boolalpha` MUST NOT be keyed onto it -- each would silently clobber the
// radix while modelling none of its own effect.  Those stay unkeyed and loud
// (rows g858/g861); see `rules/iostream/src.cpp`.
pub fn hex_refcount(a0: u32) -> u32 {
    let _ = a0;
    IOS_BASEFIELD_HEX
}

pub fn dec_refcount(a0: u32) -> u32 {
    let _ = a0;
    IOS_BASEFIELD_DEC
}

pub fn oct_refcount(a0: u32) -> u32 {
    let _ = a0;
    IOS_BASEFIELD_OCT
}

/// The UNSAFE arm's spelling: `std::ios_base &` maps to `*mut u32`, so the
/// manipulator mutates the state word in place and returns it, exactly as
/// `ios_base& hex(ios_base&)` does.  See [`IStream::set_basefield_via`] for the
/// emitted cast this signature is derived from.
///
/// # Safety
/// `a0` must be null or valid for reads and writes of a `u32`.
pub unsafe fn hex_unsafe(a0: *mut u32) -> *mut u32 {
    if !a0.is_null() {
        // SAFETY: the caller guarantees `a0` is valid for reads and writes.
        unsafe { *a0 = hex_refcount(*a0) };
    }
    a0
}

/// See [`hex_unsafe`].
///
/// # Safety
/// See [`hex_unsafe`].
pub unsafe fn dec_unsafe(a0: *mut u32) -> *mut u32 {
    if !a0.is_null() {
        // SAFETY: the caller guarantees `a0` is valid for reads and writes.
        unsafe { *a0 = dec_refcount(*a0) };
    }
    a0
}

/// See [`hex_unsafe`].
///
/// # Safety
/// See [`hex_unsafe`].
pub unsafe fn oct_unsafe(a0: *mut u32) -> *mut u32 {
    if !a0.is_null() {
        // SAFETY: the caller guarantees `a0` is valid for reads and writes.
        unsafe { *a0 = oct_refcount(*a0) };
    }
    a0
}

pub fn istream_fail(s: Ptr<IStream>) -> bool {
    s.with_ref(IStream::fail)
}

pub fn istream_good(s: Ptr<IStream>) -> bool {
    s.with_ref(IStream::good)
}

pub fn istream_eof(s: Ptr<IStream>) -> bool {
    s.with_ref(IStream::eof)
}

pub fn istream_not(s: Ptr<IStream>) -> bool {
    s.with_ref(IStream::not_op)
}

pub fn istream_to_bool(s: Ptr<IStream>) -> bool {
    s.with_ref(IStream::to_bool)
}

pub fn istream_clear(s: Ptr<IStream>) {
    s.with_mut_ref(IStream::clear);
}

pub fn istream_getline(s: Ptr<IStream>, out: Ptr<Vec<u8>>, delim: u8) -> Ptr<IStream> {
    s.with_mut_ref(|st| out.with_mut_ref(|o| st.getline(o, delim)));
    s
}

// ---------------------------------------------------------------------------
// The unsafe-model entry points: the same bodies over raw pointers, so an
// unsafe-model rule body is a one-liner too.
// ---------------------------------------------------------------------------

macro_rules! shr_unsafe_fn {
    ($name:ident, $t:ty, $method:ident) => {
        /// # Safety
        ///
        /// Both pointers must be valid and non-aliasing for the call.
        pub unsafe fn $name(s: *mut IStream, out: *mut $t) -> *mut IStream {
            unsafe { (*s).$method(&mut *out) };
            s
        }
    };
}

shr_unsafe_fn!(istream_shr_i64_unsafe, i64, extract_i64);
shr_unsafe_fn!(istream_shr_i32_unsafe, i32, extract_i32);
shr_unsafe_fn!(istream_shr_u64_unsafe, u64, extract_u64);
shr_unsafe_fn!(istream_shr_u32_unsafe, u32, extract_u32);
shr_unsafe_fn!(istream_shr_f64_unsafe, f64, extract_f64);
shr_unsafe_fn!(istream_shr_u8_unsafe, u8, extract_u8);

// ⭐ THE TWO MIRRORS THE FIRST CUT OF THIS FILE WAS MISSING, and the reason they
// are needed is the rule ABI, not symmetry.  `std::string` and `getline` are the
// two shapes the corpus actually writes (`ss >> tok` at util/dtgetenv.hpp:127
// and the `std::getline(ss, tok, ',')` comma split), and an UNSAFE-model rule
// body for either one must (a) hand the stream back so `in >> a >> b` chains and
// (b) MENTION THE RECEIVER EXACTLY ONCE, because a rule body is inlined and each
// `aN` re-expands to the argument expression verbatim -- a body written
// `{ (*a0).extract_token(&mut *a1); a0 }` names `a0` twice and would evaluate the
// receiver expression twice at every call site.  A free function that consumes
// the pointer once and returns it is the only shape that cannot do that.
//
// NOTE the std::string element type: these take `*mut Vec<u8>`, the RAW BYTES,
// not the unsafe model's NUL-terminated `Vec<libc::c_char>`.  The terminator is
// representation, and converting it is the RULE's job (see
// `rules/iostream/tgt_unsafe.rs`), not this crate's -- libcc2rs must not encode
// one model's string layout.

/// # Safety
///
/// Both pointers must be valid and non-aliasing for the call.
pub unsafe fn istream_shr_string_unsafe(s: *mut IStream, out: *mut Vec<u8>) -> *mut IStream {
    unsafe { (*s).extract_token(&mut *out) };
    s
}

/// # Safety
///
/// Both pointers must be valid and non-aliasing for the call.
pub unsafe fn istream_getline_unsafe(
    s: *mut IStream,
    out: *mut Vec<u8>,
    delim: u8,
) -> *mut IStream {
    unsafe { (*s).getline(&mut *out, delim) };
    s
}

#[cfg(test)]
mod tests {
    use super::*;

    fn s(text: &str) -> IStream {
        IStream::from_bytes(text.as_bytes().to_vec())
    }

    // ⭐ THE DELIVERABLE'S PROOF.  Extract once successfully, force a failure,
    // then extract AGAIN and assert the argument is UNCHANGED and the flag is
    // still set.  This is the property a per-call `Result` model cannot have.
    #[test]
    fn failbit_is_sticky_and_later_extractions_are_no_ops() {
        let mut st = s("7 zzz 9");

        // 1. a successful extraction.
        let mut a: i32 = -1;
        st.extract_i32(&mut a);
        assert_eq!(a, 7);
        assert!(!st.fail());
        assert!(st.good());

        // 2. force a failure on a non-numeric field.  C++11 LWG 2176: the
        //    argument is zeroed, failbit is set, and the offending field is NOT
        //    consumed past.
        let mut b: i32 = 1234;
        st.extract_i32(&mut b);
        assert_eq!(b, 0, "first failure zeroes the argument (C++11)");
        assert!(st.fail(), "failbit must be set");
        assert!(!st.good());

        // 3. THE STICKINESS.  `9` is still sitting in the buffer, so a
        //    non-sticky model would happily parse it.  The sentry must refuse.
        let mut c: i32 = 4242;
        st.extract_i32(&mut c);
        assert_eq!(c, 4242, "a sticky-failed stream must NOT touch its argument");
        assert!(st.fail(), "failbit must STILL be set");

        // 4. the OFFENDING FIELD IS STILL THERE: a failed numeric extraction
        //    consumes none of it (only the leading whitespace the sentry had
        //    already skipped before the parse was attempted), and the sticky
        //    call after it consumed nothing at all.  So it is the FLAG, not
        //    exhausted input, that stopped the read.
        assert_eq!(st.remaining(), b"zzz 9");

        // 5. every reader of the flag agrees.
        assert!(st.not_op(), "operator!() reads the failbit");
        assert!(!st.to_bool(), "operator bool() reads the failbit");
        assert!(!st.good());

        // 6. clear() un-sticks it, and only then does extraction resume.
        st.clear();
        let mut d: i32 = 0;
        st.extract_i32(&mut d);
        assert_eq!(d, 0, "the `zzz` field is still there and still unparsable");
        assert!(st.fail());
        st.clear();
        st.extract_token(&mut Vec::new());
        st.clear();
        let mut e: i32 = 0;
        st.extract_i32(&mut e);
        assert_eq!(e, 9);
    }

    // The loop shape the row is about: `while (in >> x)` must terminate.
    #[test]
    fn extraction_loop_terminates_at_end_of_input() {
        let mut st = s("1 2 3");
        let mut seen = Vec::new();
        let mut guard = 0;
        loop {
            let mut v: i32 = 0;
            st.extract_i32(&mut v);
            if st.fail() {
                break;
            }
            seen.push(v);
            guard += 1;
            assert!(guard < 100, "sticky failbit did not stop the loop");
        }
        assert_eq!(seen, vec![1, 2, 3]);
        assert!(st.eof());
    }

    #[test]
    fn chaining_lets_the_second_extraction_see_the_first_failure() {
        let owner = std::rc::Rc::new(std::cell::RefCell::new(s("oops 5")));
        let a = std::rc::Rc::new(std::cell::RefCell::new(1i32));
        let b = std::rc::Rc::new(std::cell::RefCell::new(99i32));
        let sp = crate::AsPointer::as_pointer(&owner);
        // `in >> a >> b` lowers to shr(shr(in, a), b): the inner call hands the
        // SAME stream out, so the outer one is a no-op.
        let out = istream_shr_i32(istream_shr_i32(sp, crate::AsPointer::as_pointer(&a)), crate::AsPointer::as_pointer(&b));
        assert_eq!(*a.borrow(), 0, "first extraction failed, argument zeroed");
        assert_eq!(*b.borrow(), 99, "second extraction must be a NO-OP");
        assert!(istream_fail(out));
        assert!(istream_not(crate::AsPointer::as_pointer(&owner)));
        assert!(!istream_to_bool(crate::AsPointer::as_pointer(&owner)));
    }

    #[test]
    fn tokens_strings_chars_and_floats() {
        let mut st = s("  hello  x 2.5 ");
        let mut tok = Vec::new();
        st.extract_token(&mut tok);
        assert_eq!(tok, b"hello");
        let mut c = 0u8;
        st.extract_u8(&mut c);
        assert_eq!(c, b'x');
        let mut f = 0.0f64;
        st.extract_f64(&mut f);
        assert_eq!(f, 2.5);
        assert!(!st.fail());
    }

    #[test]
    fn unsigned_rejects_a_negative_field_and_narrowing_overflow_fails() {
        let mut st = s("-1");
        let mut u = 7u32;
        st.extract_u32(&mut u);
        assert!(st.fail());
        assert_eq!(u, 0);

        let mut st2 = s("4294967296");
        let mut i = 7i32;
        st2.extract_i32(&mut i);
        assert!(st2.fail(), "a value that does not fit the target fails");
        assert_eq!(i, 0);
    }

    #[test]
    fn getline_splits_on_the_delimiter_and_then_fails_at_the_end() {
        let mut st = s("a,bb,");
        let mut out = Vec::new();
        let mut fields = Vec::new();
        loop {
            st.getline(&mut out, b',');
            if st.fail() {
                break;
            }
            fields.push(String::from_utf8(out.clone()).unwrap());
        }
        // Two fields, not three: after the trailing delimiter is consumed the
        // stream is at EOF, so the next `getline` extracts NO characters and
        // sets failbit ([istream.unformatted]/21).  A model that produced a
        // third empty field would add a phantom token to every comma split in
        // the corpus (perfdsc/perfDscImportHelper.cpp:28 and 25 more sites).
        assert_eq!(fields, vec!["a", "bb"]);
        assert!(st.eof());
    }

    #[test]
    fn a_default_stream_is_good_until_read_from() {
        let mut st = IStream::new();
        assert!(st.good());
        assert!(!st.fail());
        let mut v = 5i32;
        st.extract_i32(&mut v);
        assert!(st.fail());
        assert!(st.eof());
        assert_eq!(v, 0);
    }

    // ⭐ THE PUT-SIDE PROOF, and the reason it asserts on `buf()` and not on
    // `remaining()`: a `stringstream` is read AND written, and `str()` must keep
    // reporting bytes that have ALREADY been consumed by `>>`.  Also proves the
    // written bytes become readable, which is what makes `ss << x; ss >> y;`
    // work at all.
    #[test]
    fn writing_appends_and_does_not_disturb_the_read_cursor() {
        use std::io::Write;
        let mut st = s("7 ");
        let mut a: i32 = 0;
        st.extract_i32(&mut a);
        assert_eq!(a, 7);
        write!(st, "{}", 42).unwrap();
        // str() sees EVERYTHING, including the already-consumed `7`.
        assert_eq!(st.buf(), b"7 42");
        // and the appended bytes are readable through the same cursor.
        let mut b: i32 = 0;
        st.extract_i32(&mut b);
        assert_eq!(b, 42);
        assert!(!st.fail());
    }

    // The `Ptr` put path the converter's built-in ostream lowering actually
    // emits: `write!(ss.as_pointer(), ...)`, which resolves to
    // `Ptr::write_fmt` and needs `IStream: std::io::Write + ByteRepr`
    // (rc.rs:723).  This test IS the E0599-would-not-compile check.
    #[test]
    fn a_stream_behind_a_ptr_is_writable() {
        let owner = std::rc::Rc::new(std::cell::RefCell::new(IStream::new()));
        let p = crate::AsPointer::as_pointer(&owner);
        p.write_all(b"abc").unwrap();
        write!(p, "{}", 9).unwrap();
        assert_eq!(owner.borrow().buf(), b"abc9");
    }

    #[test]
    fn unsafe_string_and_getline_mirrors_chain_and_stay_sticky() {
        let mut st = s("aa bb");
        let mut x: Vec<u8> = Vec::new();
        let mut y: Vec<u8> = Vec::new();
        unsafe {
            let p = istream_shr_string_unsafe(&mut st, &mut x);
            istream_shr_string_unsafe(p, &mut y);
        }
        assert_eq!(x, b"aa");
        assert_eq!(y, b"bb");
        assert!(!st.fail());

        let mut st2 = s("p,q");
        let mut f: Vec<u8> = Vec::new();
        unsafe {
            let p = istream_getline_unsafe(&mut st2, &mut f, b',');
            assert_eq!(f, b"p");
            istream_getline_unsafe(p, &mut f, b',');
        }
        assert_eq!(f, b"q");
        // third call: nothing left, failbit, and the argument is left ALONE.
        unsafe { istream_getline_unsafe(&mut st2, &mut f, b',') };
        assert!(st2.fail());
        assert_eq!(f, b"q", "a failed getline must not clear its argument");
    }

    #[test]
    fn unsafe_entry_points_are_sticky_too() {
        let mut st = s("zz 3");
        let mut a = 11i32;
        let mut b = 22i32;
        unsafe {
            let p = istream_shr_i32_unsafe(&mut st, &mut a);
            istream_shr_i32_unsafe(p, &mut b);
        }
        assert_eq!(a, 0);
        assert_eq!(b, 22, "sticky no-op through the unsafe entry point");
        assert!(st.fail());
    }

    // ========================================================================
    // ⭐⭐ THE ROUND-TRIP TEST FOR THE `dtGetEnv<T>` ROW -- i.e. the gate on
    // `dxp/dxp_standalone.cpp`.  This is deliberately NOT a unit test of
    // `shr_i32`; it REPRODUCES `util/dtgetenv.hpp:121-131` line for line, so
    // that the property under test is the one the corpus depends on:
    //
    //     std::stringstream ss(ptr);
    //     T parsed;
    //     if ((ss >> parsed) && ss.eof()) { ret = parsed; }
    //
    // ⛔ THE FORBIDDEN OUTCOME THIS EXISTS TO CATCH: an identity / no-op
    // `operator>>` body makes `(ss >> parsed)` ALWAYS TRUE, so EVERY
    // `dtGetEnv<int>` returns `Some(<uninitialised>)`.  That compiles, runs, and
    // silently changes program behaviour -- nothing upstream of a differential
    // run would see it.  Every assertion below fails against such a body.
    fn dt_get_env_i32(text: &str) -> Option<i32> {
        let mut ss = s(text);
        // `T parsed;` -- DEFAULT-INITIALISED, i.e. indeterminate in C++.  The
        // sentinel stands in for that garbage: if it ever reaches the `Some`,
        // the test says so by value and not just by flag.
        let mut parsed: i32 = i32::MIN;
        // `(ss >> parsed) && ss.eof()`: the rule body is `a0.shr_i32(a1)`, whose
        // value is the stream, and the stream's truthiness is `to_bool()`.
        let stream_ok = {
            let r = ss.shr_i32(&mut parsed);
            unsafe { (*r).to_bool() }
        };
        if stream_ok && ss.eof() {
            Some(parsed)
        } else {
            None
        }
    }

    #[test]
    fn dtgetenv_shr_int_parses_rejects_trailing_garbage_and_fails_loudly() {
        // 1. "12" -> PARSES, and the whole input was consumed so eof() is true.
        assert_eq!(dt_get_env_i32("12"), Some(12));

        // 2. "12abc" -> the extraction SUCCEEDS (C++ stops at the first
        //    non-digit) but `eof()` is FALSE, because "abc" is still there.
        //    `dtGetEnv` therefore returns nullopt.  ⭐ This is the case a body
        //    built on `str::parse` over the WHOLE field would get wrong in the
        //    other direction, and the case an identity body gets wrong in this
        //    one.
        let mut st = s("12abc");
        let mut v: i32 = -1;
        st.extract_i32(&mut v);
        assert_eq!(v, 12, "the numeric prefix IS consumed");
        assert!(!st.fail(), "a trailing-garbage extraction is not a failure");
        assert!(!st.eof(), "but the stream is NOT at eof -- this is the guard");
        assert_eq!(dt_get_env_i32("12abc"), None);

        // 3. "abc" -> the stream is FALSY, so the sentinel never escapes.
        assert_eq!(dt_get_env_i32("abc"), None);
        let mut st = s("abc");
        let mut v: i32 = 7;
        assert!(!unsafe { (*st.shr_i32(&mut v)).to_bool() }, "stream is falsy");
        assert!(st.fail());
        // ⚠️ MEASURED C++ SEMANTICS, and it differs from "untouched": C++11
        // LWG 2176 / [facet.num.get.virtuals] has a FAILED NUMERIC extraction
        // STORE ZERO, so `parsed` becomes 0 rather than keeping its old value.
        // (The leave-it-alone rule is for `std::string` and `char`, which is why
        // `extract_token_reporting` carries a write-back flag and the numeric
        // wrappers do not.)  What protects the caller here is the FALSY STREAM,
        // not an untouched argument -- so that is what is asserted.
        assert_eq!(v, 0, "LWG 2176: a failed numeric extraction stores 0");

        // 4. Leading/trailing WHITESPACE is consumed by the sentry and by the
        //    `eof()` check respectively, so `" 12 "` is still a success -- as it
        //    is in C++, and as a corpus env var with a stray space needs.
        assert_eq!(dt_get_env_i32("  12"), Some(12));

        // 5. An EMPTY / whitespace-only value: the sentry sets eofbit AND
        //    failbit, so the stream is falsy.  `dtGetEnv` guards `ptr[0] != 0`
        //    itself, but a whitespace-only var reaches here.
        assert_eq!(dt_get_env_i32("   "), None);

        // 6. OUT OF RANGE for the target type is a failure, not a wrap.
        assert_eq!(dt_get_env_i32("99999999999"), None);
    }

    // The remaining widths the `dtGetEnv<T>` census found in the corpus:
    // `int64_t`/`long` (i64), `size_t`/`unsigned long` (u64), `double`, `float`.
    // One case each for the three properties that matter, per width.
    #[test]
    fn dtgetenv_shr_other_widths_agree_with_the_int_case() {
        // long / int64_t
        let mut st = s("-9007199254740993");
        let mut i: i64 = 0;
        assert!(unsafe { (*st.shr_i64(&mut i)).to_bool() });
        assert_eq!(i, -9007199254740993);
        assert!(st.eof());

        // unsigned long / size_t -- and a NEGATIVE value must FAIL, not wrap.
        let mut st = s("-1");
        let mut u: u64 = 5;
        assert!(!unsafe { (*st.shr_u64(&mut u)).to_bool() });
        assert_eq!(u, 0);

        let mut st = s("134217727");
        let mut u: u64 = 0;
        assert!(unsafe { (*st.shr_u64(&mut u)).to_bool() });
        assert_eq!(u, 134217727);
        assert!(st.eof());

        // double -- and trailing garbage leaves eof() false, as for int.
        let mut st = s("0.2x");
        let mut d: f64 = 0.0;
        assert!(unsafe { (*st.shr_f64(&mut d)).to_bool() });
        assert_eq!(d, 0.2);
        assert!(!st.eof(), "the `x` is still unread");

        // unsigned int
        let mut st = s("16");
        let mut w: u32 = 0;
        assert!(unsafe { (*st.shr_u32(&mut w)).to_bool() });
        assert_eq!(w, 16);
    }

    // ⛔ THE ONE PLACE `float` IS NOT `double`-NARROWED, and the reason it has
    // its own extractor: `1e40 as f32` is a SATURATING cast in Rust and yields
    // `f32::INFINITY` silently, whereas `num_get` sets failbit.  Without this,
    // `dtGetEnv<float>("1e40")` would answer `Some(inf)` where C++ answers
    // `std::nullopt` -- a wrong value with no diagnostic anywhere.
    #[test]
    fn shr_float_fails_on_a_value_no_float_can_hold() {
        let mut st = s("1.5");
        let mut f: f32 = 0.0;
        assert!(unsafe { (*st.shr_f32(&mut f)).to_bool() });
        assert_eq!(f, 1.5);
        assert!(st.eof());

        let mut st = s("1e40");
        let mut f: f32 = 3.0;
        assert!(
            !unsafe { (*st.shr_f32(&mut f)).to_bool() },
            "out of float range must set failbit, not saturate to inf"
        );
        assert_eq!(f, 0.0);

        // Underflow to a denormal/zero is NOT a failure in C++ either.
        let mut st = s("1e-50");
        let mut f: f32 = 3.0;
        assert!(unsafe { (*st.shr_f32(&mut f)).to_bool() });
        assert_eq!(f, 0.0);
    }

    // The CHAINING property every one of these wrappers exists for: the returned
    // pointer is the SAME stream, so `in >> a >> b` observes one failbit.
    #[test]
    fn shr_wrappers_chain_and_share_one_failbit() {
        let mut st = s("1 zz 3");
        let mut a: i32 = -1;
        let mut b: i32 = -1;
        let mut c: i32 = -1;
        unsafe {
            let p = st.shr_i32(&mut a);
            let p = (*p).shr_i32(&mut b);
            (*p).shr_i32(&mut c);
        }
        assert_eq!(a, 1);
        // ⭐ THE TWO FAILURE CASES ARE NOT THE SAME, and this assertion pair is
        // what proved it (a first cut of this test asserted `c == 0` and FAILED):
        //   * `b` -- the sentry SUCCEEDED and the CONVERSION failed, so LWG 2176
        //     applies and `num_get` stores 0.
        //   * `c` -- the stream was ALREADY failed, so the sentry returns false,
        //     `num_get` is never reached, and the argument is left STRICTLY
        //     UNTOUCHED.  That is why `extract_i32`/`extract_u32` check
        //     `self.fail()` before delegating, and it is the reason a numeric
        //     extractor needs no write-back flag: the conditional lives inside.
        assert_eq!(b, 0, "LWG 2176: zero on the failing conversion");
        assert_eq!(c, -1, "sticky: the sentry fails, so nothing is written");
        assert!(st.fail());
    }

    // ⭐ EXECUTED AGAINST C++ GROUND TRUTH, not asserted from reasoning.
    // /home/agent/work/g3084probe/hexrow.cpp was BUILT AND RUN through
    // toolchain/shim4/clang++ and printed, verbatim:
    //     hex[31,32,156]  decrestore[16,10]  oct[15]  dec[42,-7]
    //     hexfail[0,0]    clean[1,12,1,Q]    dirty[1,12,0,x]
    // Every number below is that program's output, copied over. (The brief says
    // this pod has no linker so Rust tests cannot be run -- it DOES: rustc links
    // with `-C linker=/home/agent/work/toolchain/shim4/clang++`, which is how these
    // were actually executed rather than merely type-checked.)
    #[test]
    fn basefield_is_honoured_and_matches_cpp() {
        let mk = |t: &str| IStream::from_bytes(t.as_bytes().to_vec());

        // hex[31,32,156]: `in >> std::hex >> a >> b` -- the base is STICKY across
        // both extractions, and 31*4+32 is the memDumpAnalyzer.h:244 address computation.
        let mut st = mk("1f 20");
        st.set_basefield(IOS_BASEFIELD_HEX);
        let (mut a, mut b) = (0i64, 0i64);
        st.extract_i64(&mut a);
        st.extract_i64(&mut b);
        assert_eq!((a, b, a * 4 + b), (31, 32, 156));

        // decrestore[16,10]: `>> std::hex >> c >> std::dec >> d` on "10 10".
        // THIS IS THE ASSERTION AN IDENTITY BODY FOR `std::hex` CANNOT PASS:
        // with no basefield both reads give 10.
        let mut st = mk("10 10");
        st.set_basefield(IOS_BASEFIELD_HEX);
        let mut c = 0i64;
        st.extract_i64(&mut c);
        st.set_basefield(IOS_BASEFIELD_DEC);
        let mut d = 0i64;
        st.extract_i64(&mut d);
        assert_eq!((c, d), (16, 10));

        // oct[15]
        let mut st = mk("17");
        st.set_basefield(IOS_BASEFIELD_OCT);
        let mut e = 0i64;
        st.extract_i64(&mut e);
        assert_eq!(e, 15);

        // dec[42,-7]: the DEFAULT path, which must route to the ORIGINAL
        // signed_field() and be indistinguishable from pre-basefield behaviour.
        let mut st = mk("42 -7");
        assert_eq!(st.basefield(), IOS_BASEFIELD_DEC);
        let (mut f, mut g) = (0i64, 0i64);
        st.extract_i64(&mut f);
        st.extract_i64(&mut g);
        assert_eq!((f, g), (42, -7));

        // hexfail[0,0]: 'z' is not a hex digit, so LWG 2176 zeroes the target and
        // the stream converts to false. ⛔ And `pos` must NOT have advanced.
        let mut st = mk("zz");
        st.set_basefield(IOS_BASEFIELD_HEX);
        let mut h = 5i64;
        st.extract_i64(&mut h);
        assert_eq!(h, 0);
        assert!(st.fail());
        assert!(!st.to_bool());
    }

    // ⭐ THE OTHER HALF OF ROW g3084: the f100 key's body. regvisitor.cpp:285 asserts
    // the extraction FAILS, so what must hold is (a) `to_bool()` reports it and
    // (b) ⛔ a FAILED read leaves the caller's `char` UNTOUCHED. `operator>>(char&)`
    // is not a num_get extraction, so LWG 2176 zero-on-failure does NOT apply here --
    // that is exactly why extract_char_reporting returns a `stored` flag.
    #[test]
    fn char_extraction_matches_cpp() {
        let mk = |t: &str| IStream::from_bytes(t.as_bytes().to_vec());

        // clean[1,12,1,Q]: trailing garbage IS present, so the read succeeds.
        let mut st = mk("12 Q");
        let mut r = 0i64;
        st.extract_i64(&mut r);
        let mut ch = b'?';
        let (_p, stored) = st.extract_char_reporting(&mut ch);
        assert_eq!((r, stored, ch), (12, true, b'Q'));

        // dirty[1,12,0,x]: nothing left, so the sentry fails, `stored` is false and
        // the initial b'x' SURVIVES -- the DT_CHECK at regvisitor.cpp:285 fires.
        let mut st = mk("12");
        let mut r = 0i64;
        st.extract_i64(&mut r);
        let mut ch = b'x';
        let (_p, stored) = st.extract_char_reporting(&mut ch);
        assert_eq!((r, stored, ch), (12, false, b'x'));
        assert!(st.fail());
        assert!(!st.to_bool());
    }

    // THE f101 LOWERING ITSELF, in the exact shape the converter emits.
    // `rules/iostream` bodies are INLINED, so `inFile >> std::hex >> lineno`
    // becomes literally
    //     libcc2rs::IStream::shr_i64(&mut *inFile.shr_ios_manip(libcc2rs::hex_unsafe), &mut lineno)
    // and the test below is that text, with the same `fn` items the converter
    // names.  The test above proved the RADIX; this one proves the MANIPULATOR
    // is threaded through it rather than dropped -- which is precisely what an
    // identity `std::hex` would make undetectable.  Values are the C++ program's
    // own output (g3084probe/hexrow.cxxout), copied over unchanged.
    #[test]
    fn ios_manipulators_thread_the_base_through_and_match_cpp() {
        // ⭐ THE TWO ARRAY TYPES ARE THE EXACT TYPES THE CONVERTER WRITES, copied
        // from the emitted `.rs` -- the unsafe arm casts the operand
        // (`libcc2rs::hex_unsafe as unsafe fn(*mut u32) -> *mut u32`), the refcount
        // arm emits it bare (`let __m = libcc2rs::hex_refcount;`).  If either
        // signature drifts from what its `set_basefield_via*` accepts, these stop
        // compiling here instead of as an E0308 in a translated TU.
        let unsafe_arm: [unsafe fn(*mut u32) -> *mut u32; 3] =
            [hex_unsafe, dec_unsafe, oct_unsafe];
        let refcount_arm: [fn(u32) -> u32; 3] = [hex_refcount, dec_refcount, oct_refcount];
        for m in refcount_arm {
            assert!(matches!(
                m(IOS_BASEFIELD_DEC),
                IOS_BASEFIELD_HEX | IOS_BASEFIELD_DEC | IOS_BASEFIELD_OCT
            ));
        }
        for m in unsafe_arm {
            let mut w = IOS_BASEFIELD_DEC;
            let out = unsafe { m(&mut w as *mut u32) };
            assert!(!out.is_null());
            assert!(matches!(
                unsafe { *out },
                IOS_BASEFIELD_HEX | IOS_BASEFIELD_DEC | IOS_BASEFIELD_OCT
            ));
            // The pointer arm mutates IN PLACE and returns the same address, which
            // is what `set_basefield_via` relies on.
            assert_eq!(out as usize, &mut w as *mut u32 as usize);
            assert_eq!(w, unsafe { *out });
        }
        // `setf` REPLACES the masked field, so the incoming state is irrelevant.
        let via = |m: unsafe fn(*mut u32) -> *mut u32, incoming: u32| -> u32 {
            let mut w = incoming;
            unsafe { *m(&mut w as *mut u32) }
        };
        assert_eq!(via(hex_unsafe, IOS_BASEFIELD_OCT), IOS_BASEFIELD_HEX);
        assert_eq!(via(dec_unsafe, IOS_BASEFIELD_HEX), IOS_BASEFIELD_DEC);
        assert_eq!(via(oct_unsafe, IOS_BASEFIELD_HEX), IOS_BASEFIELD_OCT);
        assert_eq!(hex_refcount(IOS_BASEFIELD_OCT), IOS_BASEFIELD_HEX);
        assert_eq!(dec_refcount(IOS_BASEFIELD_HEX), IOS_BASEFIELD_DEC);
        assert_eq!(oct_refcount(IOS_BASEFIELD_HEX), IOS_BASEFIELD_OCT);

        // hex[31,32,156] -- memDumpAnalyzer.h:244, `>> std::hex >> lineno` then a
        // second STICKY extraction, then the address arithmetic.
        let mut st = IStream::from_bytes(b"1f 20".to_vec());
        let (mut lineno, mut bank) = (0i64, 0i64);
        unsafe {
            IStream::shr_i64(&mut *st.shr_ios_manip(hex_unsafe), &mut lineno);
        }
        st.shr_i64(&mut bank);
        assert_eq!((lineno, bank, lineno * 4 + bank), (31, 32, 156));

        // decrestore[16,10] -- `>> std::hex >> a >> std::dec >> b`, one chain.
        let mut st = IStream::from_bytes(b"10 10".to_vec());
        let (mut a, mut b) = (0i64, 0i64);
        unsafe {
            IStream::shr_i64(&mut *st.shr_ios_manip(hex_unsafe), &mut a);
            IStream::shr_i64(&mut *st.shr_ios_manip(dec_unsafe), &mut b);
        }
        assert_eq!((a, b), (16, 10));

        // oct[15]
        let mut st = IStream::from_bytes(b"17".to_vec());
        let mut e = 0i64;
        unsafe {
            IStream::shr_i64(&mut *st.shr_ios_manip(oct_unsafe), &mut e);
        }
        assert_eq!(e, 15);

        // hexfail[0,0] -- "zz" is not hex; failbit set and LWG 2176 zeroes the out.
        let mut st = IStream::from_bytes(b"zz".to_vec());
        let mut z = 5i64;
        unsafe {
            IStream::shr_i64(&mut *st.shr_ios_manip(hex_unsafe), &mut z);
        }
        assert_eq!((i32::from(st.to_bool()), z), (0, 0));

        // AND THE CONTROL: no manipulator anywhere, dec[42,-7] unchanged.
        let mut st = IStream::from_bytes(b"42 -7".to_vec());
        let (mut p, mut q) = (0i64, 0i64);
        st.shr_i64(&mut p);
        st.shr_i64(&mut q);
        assert_eq!((p, q), (42, -7));
    }
}
