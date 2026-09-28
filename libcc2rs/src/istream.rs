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

/// The `std::ios_base::iostate` bits this model carries.  `badbit` is present
/// because `operator!()` is `fail() || bad()` and a model that dropped it would
/// answer `!in` wrongly for a stream that only ever went bad.
#[derive(Debug, Default, Clone, PartialEq, Eq)]
pub struct IStream {
    buf: Vec<u8>,
    pos: usize,
    failbit: bool,
    eofbit: bool,
    badbit: bool,
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
            pos: 0,
            failbit: false,
            eofbit: false,
            badbit: false,
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
        match self.signed_field().and_then(|s| s.parse::<i64>().ok()) {
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
        match self.unsigned_field().and_then(|s| s.parse::<u64>().ok()) {
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

    /// `operator>>(char&)`: one non-whitespace character.  A char extraction
    /// that fails leaves the argument alone -- `num_get` is not involved, so
    /// LWG 2176's zeroing does not apply here.
    pub fn extract_u8(&mut self, out: &mut u8) {
        if !self.sentry() {
            return;
        }
        *out = self.buf[self.pos];
        self.pos += 1;
        if self.pos >= self.buf.len() {
            self.eofbit = true;
        }
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
}
