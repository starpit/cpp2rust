// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

//! Per-stream format state: the conversion base that `std::hex` / `std::dec` /
//! `std::oct` set and that every later extraction on the SAME stream reads.
//!
//! WHY THIS EXISTS AT ALL
//! ---------------------------------------------------------------------------
//! `inFile >> std::hex >> lineno` used to abort the translator, and before the
//! assertions build made it abort it silently emitted a DECIMAL parse of
//! hexadecimal input: no placeholder, no `from_str_radix`, and the
//! `DT_CHECK(lineno != -1)` downstream then passed on the wrong number.  So the
//! bar for this module is not "make the construct translate", it is "reproduce
//! C++'s numbers or stay loud".
//!
//! THE REASON A SIDE-CHANNEL IS NEEDED, i.e. why the obvious fix is wrong
//! ---------------------------------------------------------------------------
//! The tempting fix is to read the manipulator at the extraction site: pattern
//! match `>> std::hex >>` as one chain and emit a base-16 parse for the operand
//! that follows.  That handles the chained spelling and is WRONG for the
//! spelling that actually appears in the tree, because **the base is sticky per
//! stream, not per extraction**.  Measured against clang-compiled C++
//! (an indented block would be taken for a Rust doctest and fail to compile,
//! so the C++ is fenced as text):
//!
//! ```text
//! std::istringstream ss("ff 10");
//! ss >> std::hex >> a;   // a == 255
//! ss >> b;               // b == 16  -- hex, with NO manipulator in sight
//! ```
//!
//! and the same on a `std::ifstream`.  `dcg/tools/mda/memDumpAnalyzer.h` needs
//! the even less local form, where the manipulator is applied through `<<` --
//! on a `std::string`, where it has no output effect whatsoever -- and the base
//! it leaves behind governs a LATER `>>`:
//!
//! ```text
//! sStream << std::hex << buffer;   // sets basefield, prints nothing numeric
//! sStream >> lineno;               // reads "ff" as 255
//! ```
//!
//! No amount of looking at the extraction site can see either of those.  The
//! state has to outlive the statement that sets it, which means the stream has
//! to carry it.  `std::hex` also cannot be pattern-matched as a constant: it
//! arrives as a FUNCTION POINTER, resolved signature
//! `std::istream &operator>>(std::ios_base &(*)(std::ios_base &))`.
//!
//! WHERE THE STATE LIVES, AND WHY IT IS TWO DIFFERENT PLACES
//! ---------------------------------------------------------------------------
//! The port has two stream representations and they admit different answers:
//!
//!   * string streams (`rules/sstream`) are `Box<Vec<u8>>`, a drain buffer that
//!     nothing outside `rules/sstream` and `rules/basic_ios` names.  So they
//!     can simply grow a field: `Box<StringStream>` below, which `Deref`s to
//!     `Vec<u8>` precisely so that every existing body's `.len()`, `.drain()`,
//!     `[i]` and `.iter()` keep working untouched.  A field cannot go stale and
//!     cannot be confused with another stream's state, so this side needs no
//!     identity argument at all.
//!
//!   * file streams (`rules/fstream`, `rules/iostream`) are `::std::fs::File`,
//!     which is NOT available to wrap: `libcc2rs::cout()/cerr()/cin()` hand out
//!     `Ptr<File>`/`*mut File`, `rules/raw_ostream` names it 50 times, and nine
//!     checked-in expected outputs spell it.  So their state goes in a table
//!     keyed by file descriptor, `FD_BASE` below.
//!
//! THE ONE HAZARD THE TABLE HAS, AND HOW IT IS CLOSED
//! ---------------------------------------------------------------------------
//! A descriptor is only unique among LIVE streams: close fd 7 with hex set,
//! open a different file that is handed the same fd 7, and the new stream would
//! inherit a base its C++ counterpart never had -- silent wrongness of exactly
//! the kind this module exists to prevent.  It is closed at the root rather
//! than papered over: **every way a File is born in this model resets its
//! entry**, via `reset_fd` called from all of `rules/fstream`'s constructors
//! (f1, f5, f9, f10), both default constructors (f11, f12), both `open()`s
//! (f13, f14) and both `close()`s (f17, f18, which reopen /dev/null).  C++ also
//! starts every fresh stream at `dec`, so "no entry" and "freshly reset" mean
//! the same thing, and the absent-entry default below is `dec` for that reason.
//!
//! THE FLAG VALUES ARE NOT ARBITRARY
//! ---------------------------------------------------------------------------
//! They are libc++'s `std::ios_base::fmtflags`, the same ones `rules/ios_base`
//! publishes (dec = 2, oct = 64, hex = 8, basefield = 74).  That is what lets a
//! manipulator be applied as an ordinary function to a `u32`: `rules/ios_base`
//! maps `std::ios_base` itself to `u32`, so `std::hex` becomes a real callable
//! `fn(*mut u32) -> *mut u32` and `operator>>(ios_base &(*)(ios_base &))` can
//! just CALL it on the stream's flag word.  Nothing special-cases the three
//! manipulator names, so a user-written manipulator with the same signature
//! works for free.
//!
//! One deliberate non-feature: a zero `basefield` means "detect the base from
//! the prefix" in C++ (`std::setbase(0)`).  No site in scope sets it, so
//! `radix_of_flags` folds it into decimal rather than implementing detection --
//! and decimal is what an unset field already meant before this module existed.

use std::cell::RefCell;
use std::collections::HashMap;
use std::io::Write;
use std::ops::{Deref, DerefMut};
use std::os::fd::AsRawFd;

use crate::reinterpret::ByteRepr;

/// libc++ `std::ios_base::dec`.
pub const CC2_DEC: u32 = 2;
/// libc++ `std::ios_base::hex`.
pub const CC2_HEX: u32 = 8;
/// libc++ `std::ios_base::oct`.
pub const CC2_OCT: u32 = 64;
/// libc++ `std::ios_base::basefield` == dec | oct | hex.
pub const CC2_BASEFIELD: u32 = CC2_DEC | CC2_OCT | CC2_HEX;

/// The numeric radix a `fmtflags` word selects.
///
/// An unset `basefield` folds to 10; see the note on `std::setbase(0)` above.
#[inline]
pub fn radix_of_flags(flags: u32) -> u32 {
    match flags & CC2_BASEFIELD {
        CC2_HEX => 16,
        CC2_OCT => 8,
        _ => 10,
    }
}

/// Replace the `basefield` of `flags` with `base`, leaving every other flag
/// alone -- which is what `std::hex` and friends do (`setf(hex, basefield)`).
#[inline]
pub fn with_basefield(flags: u32, base: u32) -> u32 {
    (flags & !CC2_BASEFIELD) | (base & CC2_BASEFIELD)
}

// ---------------------------------------------------------------------------
// String streams: the state is a field.
// ---------------------------------------------------------------------------

/// `rules/sstream`'s stream representation: the drain buffer it always was,
/// plus the format state a manipulator leaves behind.
///
/// `Deref`/`DerefMut` to `Vec<u8>` are the point of the design: every rule body
/// that was written against the bare buffer (`a0.len()`, `a0.drain(..)`,
/// `a0[i]`, `&a0[b..i]`, `a0.iter()`, `a0.to_vec()`) goes on compiling with no
/// edit, so growing the field could not silently change any existing
/// string-stream translation.
#[derive(Default)]
pub struct StringStream {
    buf: Vec<u8>,
    flags: u32,
}

impl StringStream {
    #[inline]
    pub fn new() -> Self {
        StringStream {
            buf: Vec::new(),
            flags: CC2_DEC,
        }
    }

    /// Wrap an existing byte buffer; the stream starts at `dec`, as C++ does.
    #[inline]
    pub fn from_vec(buf: Vec<u8>) -> Self {
        StringStream {
            buf,
            flags: CC2_DEC,
        }
    }

    /// The whole `fmtflags` word.
    #[inline]
    pub fn cc2_flags(&self) -> u32 {
        self.flags
    }

    #[inline]
    pub fn cc2_set_flags(&mut self, flags: u32) {
        self.flags = flags;
    }

    /// The radix later extractions on this stream must use.
    #[inline]
    pub fn cc2_radix(&self) -> u32 {
        radix_of_flags(self.flags)
    }
}

impl Deref for StringStream {
    type Target = Vec<u8>;
    #[inline]
    fn deref(&self) -> &Vec<u8> {
        &self.buf
    }
}

impl DerefMut for StringStream {
    #[inline]
    fn deref_mut(&mut self) -> &mut Vec<u8> {
        &mut self.buf
    }
}

/// Insertion goes through the converter's built-in ostream path, which emits
/// `write!(stream, ..)` and `stream.write_all(..)`; appending to the buffer is
/// what `io::Write for Vec<u8>` already did, so this forwards and the put
/// pointer stays implicit.
impl Write for StringStream {
    #[inline]
    fn write(&mut self, data: &[u8]) -> std::io::Result<usize> {
        self.buf.write(data)
    }
    #[inline]
    fn flush(&mut self) -> std::io::Result<()> {
        self.buf.flush()
    }
}

impl ByteRepr for StringStream {}

// ---------------------------------------------------------------------------
// File streams: the state is a table keyed by descriptor, reset on every open.
// ---------------------------------------------------------------------------

thread_local! {
    static FD_BASE: RefCell<HashMap<i32, u32>> = RefCell::new(HashMap::new());
}

/// The `fmtflags` currently set on the stream behind `fd`, defaulting to `dec`
/// for a descriptor nobody has touched -- which is also what a freshly opened
/// C++ stream reports.
pub fn flags_of_fd(fd: i32) -> u32 {
    FD_BASE.with(|m| m.borrow().get(&fd).copied().unwrap_or(CC2_DEC))
}

pub fn set_flags_of_fd(fd: i32, flags: u32) {
    FD_BASE.with(|m| {
        m.borrow_mut().insert(fd, flags);
    });
}

/// Forget any state recorded for `fd`.
///
/// Called from every constructor, `open()` and `close()` in `rules/fstream`.
/// That is what makes descriptor reuse safe: see the hazard note above.
pub fn reset_fd(fd: i32) {
    FD_BASE.with(|m| {
        m.borrow_mut().remove(&fd);
    });
}

/// `reset_fd` for a `File`, so a rule body can spell it in one call.
pub fn reset_file_fmt(f: &std::fs::File) {
    reset_fd(f.as_raw_fd());
}

/// A freshly born file stream: clear any format state left behind by an earlier
/// stream that happened to be handed the same descriptor number, then hand the
/// `File` back so a constructor rule body stays a single expression.
///
/// This is the whole of the fd-reuse mitigation, and it is not optional.  Before
/// it existed the following diverged from C++ with no diagnostic -- exactly the
/// silent-wrongness class this module is here to remove:
///
/// ```text
/// { std::ifstream f(p); f >> std::hex; f >> a; }  // a = 16 from "10"
/// { std::ifstream f(p); f >> a; }                 // C++: 10.  Was: 16.
/// ```
///
/// The second stream is a different C++ object that starts at `dec`, but the
/// closed descriptor's number was recycled, so it inherited the first stream's
/// `hex`.  Measured, and now covered by A9b in the end-to-end probe.  Every
/// constructor, `open()` and `close()` in `rules/fstream` routes through here.
pub fn fresh_file(f: std::fs::File) -> std::fs::File {
    reset_fd(f.as_raw_fd());
    f
}

pub fn flags_of_file(f: &std::fs::File) -> u32 {
    flags_of_fd(f.as_raw_fd())
}

pub fn set_flags_of_file(f: &std::fs::File, flags: u32) {
    set_flags_of_fd(f.as_raw_fd(), flags)
}

// ---------------------------------------------------------------------------
// The one parser both representations extract through.
// ---------------------------------------------------------------------------

/// Scan one integer token out of `bytes` under `radix` and return how many
/// bytes C++ would have consumed alongside the parsed digits.
///
/// Every behaviour here was taken from clang-compiled C++ rather than from the
/// standard's prose, because several of them are surprising:
///
///   * leading whitespace is skipped (`skipws`), and the SIGN may be followed
///     by the base prefix: `-0xff` is -255;
///   * under radix 16 a `0x`/`0X` prefix is consumed, and a prefix with no hex
///     digit after it FAILS -- but it is still EATEN: `"0xg"` fails and leaves
///     `g`, not `xg`, so the failure path cannot simply rewind.  A lone `"0"`
///     succeeds, and `"00x1"` reads 0 and leaves `x1`, so the `x` only counts
///     as a prefix directly after the FIRST `0`;
///   * under radix 8 there is no prefix at all: `"0x"` reads 0 and leaves `x`;
///   * scanning stops at the first character that is not a digit in the CURRENT
///     radix and does NOT consume it, so `"ffzz"` under hex reads 255 and the
///     next extraction still sees `zz`, and `"178"` under oct reads 15 and
///     leaves `8`;
///   * an empty or all-junk token fails and, in C++11 and later, writes 0 to
///     the operand rather than leaving it alone -- `"zz"` turns a variable that
///     held 42 into 0;
///   * overflow saturates and sets failbit.
///
/// Returns `(text, consumed)` where `text` is the sign-and-digits string ready
/// for `from_str_radix` and `consumed` counts every byte the stream must give
/// up, including the skipped whitespace and any base prefix.
pub fn scan_int_token(bytes: &[u8], radix: u32) -> (String, usize) {
    let mut i = 0usize;
    while i < bytes.len() && bytes[i].is_ascii_whitespace() {
        i += 1;
    }
    let mut out = String::new();
    if i < bytes.len() && (bytes[i] == b'-' || bytes[i] == b'+') {
        out.push(bytes[i] as char);
        i += 1;
    }
    // Under hex a `0x`/`0X` directly after the sign is a base prefix.  C++
    // CONSUMES it either way: with a hex digit after it the token continues,
    // and without one the extraction fails but the prefix is still gone --
    // `"0xg"` leaves `g`, not `xg`.
    let mut prefix_failed = false;
    if radix == 16
        && bytes.get(i) == Some(&b'0')
        && matches!(bytes.get(i + 1), Some(&b'x') | Some(&b'X'))
    {
        i += 2;
        if !bytes
            .get(i)
            .is_some_and(|&d| (d as char).is_ascii_hexdigit())
        {
            prefix_failed = true;
        }
    }
    if prefix_failed {
        // Failbit, operand set to 0, and the prefix consumed.
        return (String::new(), i.min(bytes.len()));
    }
    let digits_at = i;
    while i < bytes.len() && (bytes[i] as char).is_digit(radix) {
        out.push(bytes[i] as char);
        i += 1;
    }
    if i == digits_at {
        // No digit at all: C++ sets failbit and stores 0.  Nothing beyond the
        // whitespace is consumed, so a following read still sees the junk.
        return (String::new(), digits_at.min(bytes.len()));
    }
    (out, i)
}

/// Read one integer token from a `File` under `radix`, leaving the descriptor
/// exactly where C++ leaves it.
///
/// A `File` has no pushback buffer, so the terminating byte is read and then
/// seeked back over -- the same technique `rules/sstream`'s token extractor
/// already uses, and for the same reason: `f >> num; getline(f, rest);` must
/// still see the delimiter.
pub fn read_int_token_file(f: &mut std::fs::File, radix: u32) -> String {
    use std::io::Read;
    use std::io::Seek;
    let mut out = String::new();
    let mut seen_digit;
    let mut b = [0u8; 1];
    // Skip leading whitespace.
    loop {
        match f.read(&mut b) {
            Ok(0) => return String::new(),
            Ok(_) => {
                if b[0].is_ascii_whitespace() {
                    continue;
                }
                break;
            }
            Err(_) => return String::new(),
        }
    }
    if b[0] == b'-' || b[0] == b'+' {
        out.push(b[0] as char);
        if f.read(&mut b).unwrap_or(0) == 0 {
            return String::new();
        }
    }
    // A `0x`/`0X` prefix under hex.  It is consumed whether or not a hex digit
    // follows: with one the token continues, without one the extraction fails
    // and the prefix is still gone, which is why the failure path below does
    // NOT rewind over it.
    if radix == 16 && b[0] == b'0' {
        let after_zero = f.stream_position().unwrap_or(0);
        let mut p = [0u8; 1];
        if f.read(&mut p).unwrap_or(0) == 1 && (p[0] == b'x' || p[0] == b'X') {
            let mut d = [0u8; 1];
            if f.read(&mut d).unwrap_or(0) == 1 {
                if (d[0] as char).is_ascii_hexdigit() {
                    out.push(d[0] as char);
                    seen_digit = true;
                } else {
                    // Prefix eaten, the non-digit left for the next read.
                    let _ = f.seek(std::io::SeekFrom::Current(-1));
                    return String::new();
                }
            } else {
                // "0x" at end of file: failbit, prefix consumed.
                return String::new();
            }
        } else {
            // A lone `0`: rewind whatever followed it and stop.
            let _ = f.seek(std::io::SeekFrom::Start(after_zero));
            out.push('0');
            seen_digit = true;
        }
    } else if (b[0] as char).is_digit(radix) {
        out.push(b[0] as char);
        seen_digit = true;
    } else {
        // Junk where a digit was required: C++ leaves it unconsumed for the
        // next extraction and stores 0.
        let _ = f.seek(std::io::SeekFrom::Current(-1));
        return String::new();
    }
    loop {
        match f.read(&mut b) {
            Ok(0) => break,
            Ok(_) => {
                if (b[0] as char).is_digit(radix) {
                    out.push(b[0] as char);
                    seen_digit = true;
                } else {
                    // Stop AT the terminator without consuming it.
                    let _ = f.seek(std::io::SeekFrom::Current(-1));
                    break;
                }
            }
            Err(_) => break,
        }
    }
    if seen_digit { out } else { String::new() }
}

/// `from_str_radix` for the signed operands, with C++'s failure answers:
/// nothing parsed stores 0, and overflow saturates.
pub fn parse_i64(text: &str, radix: u32) -> i64 {
    if text.is_empty() || text == "-" || text == "+" {
        return 0;
    }
    match i64::from_str_radix(text, radix) {
        Ok(v) => v,
        Err(_) => {
            if text.starts_with('-') {
                i64::MIN
            } else {
                i64::MAX
            }
        }
    }
}

/// The unsigned counterpart.  C++ accepts a negative spelling here and wraps
/// it, which is why this parses as signed first when a sign is present:
/// `-1` under hex into an `unsigned` is 4294967295.
pub fn parse_u64(text: &str, radix: u32) -> u64 {
    if text.is_empty() || text == "-" || text == "+" {
        return 0;
    }
    if let Some(rest) = text.strip_prefix('-') {
        return match u64::from_str_radix(rest, radix) {
            Ok(v) => (v as i64).wrapping_neg() as u64,
            Err(_) => u64::MAX,
        };
    }
    let text = text.strip_prefix('+').unwrap_or(text);
    u64::from_str_radix(text, radix).unwrap_or(u64::MAX)
}

// ---------------------------------------------------------------------------
// The three manipulators as named shims.
//
// When a mapped function is used as a VALUE rather than called -- which is
// exactly how a manipulator reaches `operator>>` -- the converter spells it
// `libcc2rs::<name>_<model>` (Mapper::MapFunctionName), because a std::
// function with a rule has no translated definition to point at.  So the names
// below are load-bearing and are what makes `>> std::hex` link; rules/ios_base
// carries the same three bodies for the case where the manipulator is CALLED.
//
// Each takes the stream's flags word by pointer and replaces its basefield,
// mirroring libc++'s `setf(base, basefield)`, and returns the same pointer so
// the C++ signature's `std::ios_base &` return is preserved.
// ---------------------------------------------------------------------------

/// # Safety
///
/// `flags` must point to a valid `u32`.
pub unsafe fn hex_unsafe(flags: *mut u32) -> *mut u32 {
    unsafe {
        *flags = with_basefield(*flags, CC2_HEX);
    }
    flags
}

/// # Safety
///
/// `flags` must point to a valid `u32`.
pub unsafe fn dec_unsafe(flags: *mut u32) -> *mut u32 {
    unsafe {
        *flags = with_basefield(*flags, CC2_DEC);
    }
    flags
}

/// # Safety
///
/// `flags` must point to a valid `u32`.
pub unsafe fn oct_unsafe(flags: *mut u32) -> *mut u32 {
    unsafe {
        *flags = with_basefield(*flags, CC2_OCT);
    }
    flags
}

pub fn hex_refcount(flags: crate::Ptr<u32>) -> crate::Ptr<u32> {
    flags.with_mut(|f| *f = with_basefield(*f, CC2_HEX));
    flags
}

pub fn dec_refcount(flags: crate::Ptr<u32>) -> crate::Ptr<u32> {
    flags.with_mut(|f| *f = with_basefield(*f, CC2_DEC));
    flags
}

pub fn oct_refcount(flags: crate::Ptr<u32>) -> crate::Ptr<u32> {
    flags.with_mut(|f| *f = with_basefield(*f, CC2_OCT));
    flags
}

/// Reborrow a stream receiver without naming its representation.
///
/// Every extraction rule needs the receiver exactly ONCE: the converter
/// re-emits the receiver EXPRESSION at each placeholder, so a body that
/// mentions `a0` twice makes a chained `ss >> a >> b` evaluate -- and therefore
/// re-run -- the inner extraction twice.  (That was measured: `u` came out 0
/// instead of 57005 because its token had already been consumed by the
/// duplicate call.)  Binding the receiver to a local fixes that, but the
/// binding must not be type-annotated, because one body serves both
/// `Box<StringStream>` and `std::fs::File`.  This infers it.
#[inline]
pub fn stream_mut<T: ?Sized>(s: &mut T) -> &mut T {
    s
}

// ---------------------------------------------------------------------------
// Representation dispatch, AS A LIBRARY TRAIT RATHER THAN A RULE-LOCAL ONE.
//
// rules/basic_ios established the private-trait trick: a rule body declares a
// trait, impls it once per Rust representation, and calls it on the receiver,
// so ONE C++ signature reached from two representations dispatches statically.
// That works for the state predicates, and it does NOT work here, for a reason
// that only shows up at runtime and was measured rather than predicted.
//
// A rule body is inlined by TEXTUAL SUBSTITUTION with the receiver expression
// spliced in at the placeholder.  When the receiver of a rule is a call to THE
// SAME rule -- which is exactly what `ss >> a >> b` is, since `operator>>`
// returns the stream -- the body nests inside itself, and a body carrying its
// own `trait`/`impl` items nests those too.  Two identical `impl __Cc2Num_f21
// for StringStream` blocks then land in one function body:
//
//     error[E0034]: multiple applicable items in scope
//       --> multiple `__cc2_num_f21` found
//
// Per-rule method names (`__cc2_num_f21` vs `__cc2_num_f22`) do not help: the
// collision is a rule nested in ITSELF, so the names are equal by
// construction.  rules/basic_ios never hit this because `good()` returns bool,
// so a predicate can never be its own receiver.
//
// So for the extractors the trait lives HERE, declared once in the library,
// and the rule bodies only CALL it.  Nesting then just nests two method calls,
// which is what the C++ does anyway.  The property that made the rule-local
// trait worth having is preserved exactly: dispatch is static, chosen from the
// receiver's Rust type, and a representation nobody implemented is a compile
// error naming the trait and the type rather than silently wrong output.
// ---------------------------------------------------------------------------

/// One extraction step, per stream representation.
///
/// Implemented for `StringStream` (the drain buffer `rules/sstream` maps string
/// streams to) and for `std::fs::File` (what `rules/fstream`, `rules/iostream`
/// and `rules/raw_ostream` map file streams and `cin`/`cout`/`cerr` to).  A
/// third representation added later needs one more impl here and no rule edit.
pub trait Cc2Extract {
    /// The stream's current `fmtflags`.
    fn cc2_get_flags(&self) -> u32;
    /// Replace the stream's `fmtflags` -- this is what a manipulator does, and
    /// it is why the radix outlives the statement that set it.
    fn cc2_put_flags(&mut self, flags: u32);
    /// Consume one integer token under the stream's current radix and return
    /// it with the radix it was scanned in.
    fn cc2_int_token(&mut self) -> (String, u32);
    /// Consume one floating-point token.  `fmtflags` has no bearing on these:
    /// C++ float extraction ignores the basefield.
    fn cc2_float_token(&mut self) -> String;

    /// Apply a manipulator expressed as a function on a flags word.  Nothing
    /// here knows the names hex/dec/oct, so a user-written manipulator with the
    /// same C++ signature works unchanged.
    fn cc2_apply_manip(&mut self, m: impl FnOnce(&mut u32)) {
        let mut flags = self.cc2_get_flags();
        m(&mut flags);
        self.cc2_put_flags(flags);
    }

    /// Apply a `*mut u32` manipulator -- the unsafe model's spelling of one.
    ///
    /// # Safety
    ///
    /// `m` must be one of the manipulator shims below, or a translated function
    /// with the same contract: it may only write through the pointer it is
    /// given.
    unsafe fn cc2_apply_unsafe(&mut self, m: unsafe fn(*mut u32) -> *mut u32) {
        self.cc2_apply_manip(|flags| unsafe {
            m(flags);
        });
    }

    /// Apply a `Ptr<u32>` manipulator -- the refcount model's spelling.
    fn cc2_apply_refcount(&mut self, m: fn(crate::Ptr<u32>) -> crate::Ptr<u32>) {
        self.cc2_apply_manip(|flags| {
            let cell: crate::Value<u32> = std::rc::Rc::new(RefCell::new(*flags));
            m(crate::AsPointer::as_pointer(&cell));
            *flags = *cell.borrow();
        });
    }

    /// The radix a later extraction on this stream must use.
    #[inline]
    fn cc2_extract_radix(&self) -> u32 {
        radix_of_flags(self.cc2_get_flags())
    }
}

// ---------------------------------------------------------------------------
// The rule bodies call these free functions rather than the trait methods
// directly, and that is not cosmetic.
//
// The refcount model spells a `&mut Box<StringStream>` receiver placeholder as
// `&mut (*ss.borrow_mut())`.  `borrow_mut()` yields a TEMPORARY `RefMut`, which
// lives to the end of the enclosing STATEMENT.  So a body shaped as several
// statements --
//
//     let __s = stream_mut(&mut (*ss.borrow_mut()));   // RefMut dies here
//     let (t, r) = __s.cc2_int_token();                // E0716: borrow later used
//
// does not compile, while the same body written as ONE expression does, because
// the temporary then outlives the whole expression it appears in.  Each function
// below is therefore the entire body: receiver in, side effect done, receiver
// back out, in a single expression.
//
// They take `&mut S` generically so `S` is inferred from the call site, which is
// what keeps one rule body serving both representations.  A representation with
// no `Cc2Extract` impl is a compile error naming the trait and the type.
// ---------------------------------------------------------------------------

/// Extract one integer under the stream's own radix, write it through `store`,
/// and hand the stream back so a chained `>>` can use it as its receiver.
#[inline]
pub fn extract_int<S: Cc2Extract + ?Sized>(s: &mut S, store: impl FnOnce(&str, u32)) -> &mut S {
    let (text, radix) = s.cc2_int_token();
    store(&text, radix);
    s
}

/// The floating-point counterpart.  `fmtflags` does not affect these: C++ float
/// extraction ignores the basefield.
#[inline]
pub fn extract_float<S: Cc2Extract + ?Sized>(s: &mut S, store: impl FnOnce(&str)) -> &mut S {
    let text = s.cc2_float_token();
    store(&text);
    s
}

/// Apply an unsafe-model manipulator and hand the stream back.
///
/// # Safety
///
/// `m` must only write through the pointer it is given.
#[inline]
pub unsafe fn manip_unsafe<S: Cc2Extract + ?Sized>(
    s: &mut S,
    m: unsafe fn(*mut u32) -> *mut u32,
) -> &mut S {
    unsafe { s.cc2_apply_unsafe(m) };
    s
}

/// Apply a refcount-model manipulator and hand the stream back.
#[inline]
pub fn manip_refcount<S: Cc2Extract + ?Sized>(
    s: &mut S,
    m: fn(crate::Ptr<u32>) -> crate::Ptr<u32>,
) -> &mut S {
    s.cc2_apply_refcount(m);
    s
}

impl Cc2Extract for StringStream {
    #[inline]
    fn cc2_get_flags(&self) -> u32 {
        self.flags
    }
    #[inline]
    fn cc2_put_flags(&mut self, flags: u32) {
        self.flags = flags;
    }
    fn cc2_int_token(&mut self) -> (String, u32) {
        let radix = self.cc2_extract_radix();
        let (text, consumed) = scan_int_token(&self.buf[..], radix);
        self.buf.drain(..consumed);
        (text, radix)
    }
    fn cc2_float_token(&mut self) -> String {
        let (text, consumed) = scan_float_token(&self.buf[..]);
        self.buf.drain(..consumed);
        text
    }
}

/// A boxed stream is still that stream.
///
/// `rules/sstream` maps a string stream to `Box<StringStream>`, so a rule body's
/// receiver reaches these helpers as `&mut Box<StringStream>`; a file stream
/// reaches them as `&mut File`.  Generic parameters do not deref-coerce, so
/// without this forwarding impl one of the two spellings has to be written by
/// hand in the rule body -- and whichever one is chosen breaks the other
/// representation, which is the whole class of bug rules/basic_ios exists to
/// prevent.  Forwarding here keeps `&mut *a0` correct for both.
impl<S: Cc2Extract + ?Sized> Cc2Extract for Box<S> {
    #[inline]
    fn cc2_get_flags(&self) -> u32 {
        (**self).cc2_get_flags()
    }
    #[inline]
    fn cc2_put_flags(&mut self, flags: u32) {
        (**self).cc2_put_flags(flags)
    }
    #[inline]
    fn cc2_int_token(&mut self) -> (String, u32) {
        (**self).cc2_int_token()
    }
    #[inline]
    fn cc2_float_token(&mut self) -> String {
        (**self).cc2_float_token()
    }
}

impl Cc2Extract for std::fs::File {
    #[inline]
    fn cc2_get_flags(&self) -> u32 {
        flags_of_file(self)
    }
    #[inline]
    fn cc2_put_flags(&mut self, flags: u32) {
        set_flags_of_file(self, flags)
    }
    fn cc2_int_token(&mut self) -> (String, u32) {
        let radix = self.cc2_extract_radix();
        (read_int_token_file(self, radix), radix)
    }
    fn cc2_float_token(&mut self) -> String {
        read_float_token_file(self)
    }
}

/// Scan one floating-point token out of `bytes`, returning `(text, consumed)`.
///
/// This is the pre-existing `rules/sstream` float scanner moved verbatim into
/// the library so that the `File` side can agree with it; the radix plays no
/// part, matching C++, where `std::hex` does not affect float extraction.
pub fn scan_float_token(bytes: &[u8]) -> (String, usize) {
    let mut i = 0usize;
    while i < bytes.len() && bytes[i].is_ascii_whitespace() {
        i += 1;
    }
    let begin = i;
    if i < bytes.len() && (bytes[i] == b'-' || bytes[i] == b'+') {
        i += 1;
    }
    while i < bytes.len()
        && (bytes[i].is_ascii_digit()
            || bytes[i] == b'.'
            || bytes[i] == b'e'
            || bytes[i] == b'E'
            || ((bytes[i] == b'-' || bytes[i] == b'+')
                && (bytes[i - 1] == b'e' || bytes[i - 1] == b'E')))
    {
        i += 1;
    }
    (String::from_utf8_lossy(&bytes[begin..i]).into_owned(), i)
}

/// The `File` counterpart of `scan_float_token`, seeking back over the
/// terminator so the read position lands where C++ leaves it.
pub fn read_float_token_file(f: &mut std::fs::File) -> String {
    use std::io::Read;
    use std::io::Seek;
    let mut out = String::new();
    let mut b = [0u8; 1];
    loop {
        match f.read(&mut b) {
            Ok(0) => break,
            Ok(_) => {
                let c = b[0];
                if out.is_empty() && c.is_ascii_whitespace() {
                    continue;
                }
                let prev = out.as_bytes().last().copied().unwrap_or(0);
                let ok = c.is_ascii_digit()
                    || c == b'.'
                    || c == b'e'
                    || c == b'E'
                    || ((c == b'-' || c == b'+')
                        && (out.is_empty() || prev == b'e' || prev == b'E'));
                if !ok {
                    let _ = f.seek(std::io::SeekFrom::Current(-1));
                    break;
                }
                out.push(c as char);
            }
            Err(_) => break,
        }
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    // Every expectation below is the output of the clang-compiled C++ probe,
    // not a reading of the standard.
    #[test]
    fn hex_tokens() {
        assert_eq!(scan_int_token(b"ff", 16).0, "ff");
        assert_eq!(parse_i64(&scan_int_token(b"ff", 16).0, 16), 255);
        assert_eq!(parse_i64(&scan_int_token(b"0xff", 16).0, 16), 255);
        assert_eq!(parse_i64(&scan_int_token(b"0Xff", 16).0, 16), 255);
        assert_eq!(parse_i64(&scan_int_token(b"-ff", 16).0, 16), -255);
        assert_eq!(parse_i64(&scan_int_token(b"-0xff", 16).0, 16), -255);
        assert_eq!(parse_i64(&scan_int_token(b"DEAD", 16).0, 16), 57005);
        assert_eq!(parse_i64(&scan_int_token(b"AbCdEf", 16).0, 16), 11259375);
        // Stops at the first non-hex digit without consuming it.
        let (t, n) = scan_int_token(b"ffzz", 16);
        assert_eq!((parse_i64(&t, 16), n), (255, 2));
        // A bare "0" is fine.  A "0x" with no hex digit after it FAILS (stores
        // 0) but still consumes the prefix: C++ leaves `g`, not `xg`.  These
        // consumed-counts are what gt5.cpp measured.
        assert_eq!(parse_i64(&scan_int_token(b"0", 16).0, 16), 0);
        assert_eq!(scan_int_token(b"0x", 16), (String::new(), 2));
        assert_eq!(scan_int_token(b"0xg", 16), (String::new(), 2));
        assert_eq!(scan_int_token(b"0xzz", 16), (String::new(), 2));
        // The `x` is only a prefix directly after the FIRST `0`: "00x1" reads
        // 0 and leaves "x1".
        let (t, n) = scan_int_token(b"00x1", 16);
        assert_eq!((parse_i64(&t, 16), n), (0, 2));
        // A lone "0" followed by a space stops at the space.
        assert_eq!(scan_int_token(b"0 1", 16), ("0".to_string(), 1));
        // Junk stores 0 rather than leaving the operand alone.
        assert_eq!(parse_i64(&scan_int_token(b"zz", 16).0, 16), 0);
        // Leading whitespace is skipped.
        assert_eq!(parse_i64(&scan_int_token(b"  \t\n ff", 16).0, 16), 255);
    }

    #[test]
    fn oct_and_dec_tokens() {
        assert_eq!(parse_i64(&scan_int_token(b"17", 8).0, 8), 15);
        assert_eq!(parse_i64(&scan_int_token(b"017", 8).0, 8), 15);
        // Oct stops at 8, and has no 0x prefix.
        let (t, _) = scan_int_token(b"178", 8);
        assert_eq!(parse_i64(&t, 8), 15);
        let (t, n) = scan_int_token(b"0x17", 8);
        assert_eq!((parse_i64(&t, 8), n), (0, 1));
        assert_eq!(parse_i64(&scan_int_token(b"19", 10).0, 10), 19);
        assert_eq!(parse_i64(&scan_int_token(b"019", 10).0, 10), 19);
        // Decimal does not auto-detect a prefix.
        let (t, _) = scan_int_token(b"0x19", 10);
        assert_eq!(parse_i64(&t, 10), 0);
    }

    #[test]
    fn unsigned_negative_wraps() {
        // `-1` under hex into an unsigned is 4294967295 in C++.
        assert_eq!(parse_u64(&scan_int_token(b"-1", 16).0, 16) as u32, u32::MAX);
    }

    #[test]
    fn basefield_round_trip() {
        assert_eq!(radix_of_flags(CC2_DEC), 10);
        assert_eq!(radix_of_flags(CC2_HEX), 16);
        assert_eq!(radix_of_flags(CC2_OCT), 8);
        // A manipulator replaces the basefield and keeps other flags.
        let flags = with_basefield(CC2_DEC | 512, CC2_HEX);
        assert_eq!(radix_of_flags(flags), 16);
        assert_eq!(flags & 512, 512);
    }

    #[test]
    fn string_stream_derefs_to_its_buffer() {
        let mut s = StringStream::from_vec(b"abc".to_vec());
        assert_eq!(s.len(), 3);
        assert_eq!(s[0], b'a');
        s.drain(..1);
        assert_eq!(&s[..], b"bc");
        // ... and carries format state across that.
        s.cc2_set_flags(CC2_HEX);
        assert_eq!(s.cc2_radix(), 16);
    }

    // The File extractor has to agree with the slice extractor token for
    // token, including where it leaves the read position -- that is the part
    // no amount of staring at the code establishes.
    #[test]
    fn file_tokens_match_the_slice_scanner() {
        use std::io::Read;
        let path = std::env::temp_dir().join("cc2_stream_fmt_file_tokens");
        for (input, radix, want, want_rest) in [
            ("ff 10", 16, 255i64, " 10"),
            ("ffzz", 16, 255, "zz"),
            ("0xff", 16, 255, ""),
            ("-0xff", 16, -255, ""),
            ("0xg", 16, 0, "g"),
            ("0x", 16, 0, ""),
            ("00x1", 16, 0, "x1"),
            ("0 1", 16, 0, " 1"),
            ("  \t ff", 16, 255, ""),
            ("zz", 16, 0, "zz"),
            ("178", 8, 15, "8"),
            ("0x17", 8, 0, "x17"),
            ("19", 10, 19, ""),
            ("DEAD", 16, 57005, ""),
        ] {
            std::fs::write(&path, input).unwrap();
            let mut f = std::fs::File::open(&path).unwrap();
            let text = read_int_token_file(&mut f, radix);
            assert_eq!(parse_i64(&text, radix), want, "value for {input:?}");
            let mut rest = String::new();
            f.read_to_string(&mut rest).unwrap();
            assert_eq!(rest, want_rest, "leftover for {input:?}");

            // ... and the slice scanner consumes exactly as much.
            let (stext, n) = scan_int_token(input.as_bytes(), radix);
            assert_eq!(parse_i64(&stext, radix), want, "slice value {input:?}");
            assert_eq!(&input[n..], want_rest, "slice leftover for {input:?}");
        }
        let _ = std::fs::remove_file(&path);
    }

    #[test]
    fn fd_state_resets() {
        set_flags_of_fd(4242, CC2_HEX);
        assert_eq!(radix_of_flags(flags_of_fd(4242)), 16);
        reset_fd(4242);
        assert_eq!(radix_of_flags(flags_of_fd(4242)), 10);
    }
}
