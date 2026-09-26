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
pub struct StringStream {
    buf: Vec<u8>,
    flags: u32,
    /// `std::setw`, consumed by the next inserted item.  See the insertion
    /// section below for why width is taken while base and fill persist.
    width: usize,
    /// `std::setfill`, which persists until changed.
    fill: u8,
    /// `iostate`; see the field of the same name on `Cc2FmtState`.
    state: u32,
}

impl Default for StringStream {
    fn default() -> Self {
        StringStream::new()
    }
}

impl StringStream {
    #[inline]
    pub fn new() -> Self {
        StringStream::from_vec(Vec::new())
    }

    /// Wrap an existing byte buffer; the stream starts at `dec` with no width
    /// and a space fill, as a fresh C++ stream does.
    #[inline]
    pub fn from_vec(buf: Vec<u8>) -> Self {
        StringStream {
            buf,
            flags: CC2_DEC,
            width: 0,
            fill: b' ',
            state: CC2_GOODBIT,
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
    static FD_BASE: RefCell<HashMap<i32, Cc2FmtState>> = RefCell::new(HashMap::new());
}

/// The whole format state of the stream behind `fd`, defaulting to a fresh
/// C++ stream's (`dec`, no width, space fill) for a descriptor nobody has
/// touched.
/// The table key for a descriptor.
///
/// A CLONE of stdout is a different descriptor NUMBER but the same stream, and
/// that distinction is the whole cout bug. `rules/iostream` f1/f2 build
/// `std::cout` as `File::from_raw_fd(stdout().try_clone_to_owned())`, which the
/// converter re-evaluates per statement, so:
///
/// ```text
/// std::cout << std::hex;      // clone -> fd 3, state recorded for 3, fd 3 closed
/// std::ofstream f("...");     // takes fd 3
/// std::cout << 255;           // clone -> fd 4, finds no state
/// ```
///
/// printed 255 where C++ prints ff -- measured. The value form cannot stop being
/// a clone (a `std::ostream &os = std::cout;` binding needs a File, not a
/// pointer), so instead every clone of a standard stream is keyed on the STREAM
/// it duplicates. `same_stream_as` compares by device+inode, which is what makes
/// a dup of fd 1 answer 1 while a genuine file keeps its own number.
fn stream_key(fd: i32) -> i32 {
    // Only the three standard streams need this: they are the ones the converter
    // rematerialises per statement. A file stream's descriptor is held by the
    // ofstream object for its whole lifetime, so its number is already stable,
    // and reset_fd on every open/close covers reuse.
    for std_fd in [0, 1, 2] {
        if fd == std_fd {
            return std_fd;
        }
        if same_stream_as(fd, std_fd) {
            return std_fd;
        }
    }
    fd
}

/// Whether two descriptors refer to the same open stream, by device and inode.
///
/// Uses `fstat` on a BORROWED descriptor -- no clone, so nothing can be closed by
/// accident, which matters because getting that wrong here is exactly the class of
/// bug this function exists to fix.
fn same_stream_as(a: i32, b: i32) -> bool {
    fn ino(fd: i32) -> Option<(u64, u64)> {
        let mut st: libc::stat = unsafe { std::mem::zeroed() };
        // SAFETY: fstat only reads; it does not take ownership of the descriptor.
        if unsafe { libc::fstat(fd, &mut st) } != 0 {
            return None;
        }
        Some((st.st_dev as u64, st.st_ino as u64))
    }
    match (ino(a), ino(b)) {
        (Some(x), Some(y)) => x == y,
        _ => false,
    }
}

pub fn state_of_fd(fd: i32) -> Cc2FmtState {
    let fd = stream_key(fd);
    FD_BASE.with(|m| m.borrow().get(&fd).copied().unwrap_or_default())
}

pub fn set_state_of_fd(fd: i32, state: Cc2FmtState) {
    let fd = stream_key(fd);
    FD_BASE.with(|m| {
        m.borrow_mut().insert(fd, state);
    });
}

/// The `fmtflags` currently set on the stream behind `fd`, defaulting to `dec`
/// for a descriptor nobody has touched -- which is also what a freshly opened
/// C++ stream reports.
pub fn flags_of_fd(fd: i32) -> u32 {
    state_of_fd(fd).flags
}

pub fn set_flags_of_fd(fd: i32, flags: u32) {
    let mut st = state_of_fd(fd);
    st.flags = flags;
    set_state_of_fd(fd, st);
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

    /// The stream's `iostate`.
    fn cc2_get_state(&self) -> u32;
    /// Replace the `iostate`.
    fn cc2_put_state(&mut self, state: u32);

    /// Set bits in the `iostate`, as `setstate` does.
    #[inline]
    fn cc2_set_state(&mut self, bits: u32) {
        let s = self.cc2_get_state();
        self.cc2_put_state(s | bits);
    }

    /// `clear()` -- back to goodbit.
    #[inline]
    fn cc2_clear_state(&mut self) {
        self.cc2_put_state(CC2_GOODBIT);
    }

    /// `operator bool`, i.e. `!fail()`.  This is the predicate
    /// `while (getline(f, line))` needs, and unlike a position-derived one it
    /// answers "the last read succeeded" rather than "not at end".
    #[inline]
    fn cc2_ok(&self) -> bool {
        self.cc2_get_state() & (CC2_FAILBIT | CC2_BADBIT) == 0
    }

    /// Read one delimited line, C++'s `std::getline` semantics exactly.
    ///
    /// Returns the line, and sets the `iostate` the way C++ does -- which is
    /// subtler than "empty means done" and was measured rather than assumed:
    ///
    /// ```text
    /// istringstream("a\nb\n"):  getline -> a, b, then FAIL+EOF
    /// istringstream("x"):       getline -> x with eof=1 fail=0  (SUCCEEDS)
    /// istringstream(""):        getline -> fail=1 eof=1 immediately
    /// ```
    ///
    /// So a final line with no trailing newline succeeds while already at eof,
    /// and eofbit alone must never be read as failure.
    fn cc2_getline(&mut self, delim: u8) -> Vec<u8>;
}

// ---------------------------------------------------------------------------
// INSERTION: the same flags word, read on the way OUT.
//
// Everything above is the extraction side.  `operator<<` had its own, entirely
// separate mechanism in the converter -- a printf-style format string built per
// STATEMENT, with the radix baked into the placeholder (`{:x}`) and a local
// `const char *fmt_trait` asserting at the end of the statement that no trait
// was still pending (converter.cpp, "Stream state was not restored after
// call").  That assert encodes an assumption C++ does not make: `std::hex` is
// sticky on the STREAM OBJECT, so real code sets it in one statement and
// restores it two statements later.  dsc/pcfg.cpp:2274 is exactly that shape.
//
// So insertion now reads the stream's flags at RUNTIME, from the same word the
// extractors use.  Sharing the word is not tidiness -- it is required, because
// a manipulator applied with `<<` governs a later `>>`.  This is real code at
// dcg/tools/mda/memDumpAnalyzer.h:251, and measured against clang:
//
// ```text
// sStream << std::hex << buffer;   // sets basefield via <<, prints "ff"
// sStream >> lineno;               // C++ reads it back as 255
// ```
//
// Reading the base at runtime is also what makes the undecidable cases right
// rather than guessed.  With the base in the format string, a manipulator
// inside a branch or a loop has no correct answer -- `if (c) { o << std::hex; }
// o << v;` prints decimal or hex depending on a runtime value, and a converter
// that must choose one is silently wrong on the other half of its inputs.
// Nothing here chooses: the branch writes the flags word and the insertion
// reads whatever is there.
//
// WIDTH AND FILL LIVE HERE TOO, because they have the identical lifetime bug
// and it was already SILENT.  `o << "[" << std::setw(4) << c << "]"` with c=7
// emitted `write!(o, "[{:}{:}]", 4, c)` -- the WIDTH printed as a value,
// giving `[47]` where C++ gives `[   7]`.  The guard that was meant to catch it
// tested `arg_str.contains("Setw")`, but libc++ spells the manipulator's return
// type `std::__iom_t6`, so that test had never once fired.  Five TUs in
// dcg/ ddc/ dsc/ dbo/ use setw or setfill with no hex anywhere, so they were
// wrong with nothing to catch them.
//
// THE THREE STICKINESS RULES ARE NOT THE SAME, and all three were measured
// against clang-compiled C++ rather than read off the standard:
//
//   * BASE persists until another manipulator changes it;
//   * FILL persists likewise;
//   * WIDTH is consumed by ONE inserted item and then resets to 0.
//
//     o << std::hex << std::setw(6) << std::setfill('.') << 255 << "|" << 255
//       ....ff|ff          <- fill and base still on, width gone after the 255
//
// and width applies to the NEXT item whatever it is, not merely to a number:
// `o << std::setw(5) << "ab"` is `   ab`, `o << std::setw(3) << 'x'` is `  x`,
// and even `o << std::setw(4) << '\n'` pads the newline.  That is why the
// converter can no longer fold adjacent string literals into one format string
// when a width might be pending, and why every inserted item -- literal text
// included -- goes through one of the helpers below.
// ---------------------------------------------------------------------------

/// libc++ `std::ios_base::goodbit`.
pub const CC2_GOODBIT: u32 = 0;
/// libc++ `std::ios_base::eofbit`.
pub const CC2_EOFBIT: u32 = 2;
/// libc++ `std::ios_base::failbit`.
pub const CC2_FAILBIT: u32 = 4;
/// libc++ `std::ios_base::badbit`.
pub const CC2_BADBIT: u32 = 1;

/// The per-stream state, beyond the flags word.
///
/// Split out from the flags so that the `File` side can keep one table entry
/// per descriptor rather than four.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Cc2FmtState {
    /// `fmtflags`, shared with the extraction side.
    pub flags: u32,
    /// `std::setw`, consumed by the next inserted item.
    pub width: usize,
    /// `std::setfill`, which persists.
    pub fill: u8,
    /// `iostate` -- eofbit/failbit/badbit.
    ///
    /// This is the piece `rules/sstream` was missing, and its absence was not a
    /// gap but a RUNTIME TRAP in the refcount model: `while (getline(f, line))`
    /// compiled, printed nothing and panicked `ub: null pointer` at rc.rs:350,
    /// because the rule body ended in `Ptr::null()` and using that as a
    /// condition dereferences it.
    ///
    /// The reason it needed this state rather than a cleverer rule is recorded
    /// in `rules/sstream/src.cpp`: returning the stream so `operator bool`
    /// applies was tried and reverted, because `rules/basic_ios` answered
    /// `operator bool` from position-vs-length -- "not at end" rather than "the
    /// last read succeeded" -- which never terminates (measured, rc=124, a
    /// hang).  C++ sets failbit when a read obtains nothing, and no
    /// position-derived predicate can express that: a getline that consumes the
    /// final `"x"` with no trailing newline SUCCEEDS while already at eof
    /// (measured: `ok=1 eof=1 fail=0`), so eof and failure are genuinely
    /// independent and one cannot be derived from the other.
    pub state: u32,
}

impl Default for Cc2FmtState {
    fn default() -> Self {
        Cc2FmtState {
            flags: CC2_DEC,
            width: 0,
            fill: b' ',
            state: CC2_GOODBIT,
        }
    }
}

/// One insertion step, per stream representation.
///
/// A supertrait bound on `Cc2Extract` is what makes "one flags word for both
/// directions" a type-level fact rather than a convention: an implementor
/// cannot supply insertion state without also supplying the extraction state,
/// and `cc2_get_flags`/`cc2_put_flags` are inherited rather than redeclared, so
/// the two sides cannot drift apart.  `Write` is required because every helper
/// below ends by writing bytes.
pub trait Cc2Insert: Cc2Extract {
    /// Append bytes to the stream.
    ///
    /// `Cc2Insert` does NOT require `io::Write`, deliberately: `rules/iostream`
    /// hands back `*mut File` for std::cout (see the raw-pointer impls below for
    /// why an owned File is not an option), and Rust's orphan rule forbids
    /// `impl Write for *mut S` in this crate. So the byte sink is a method of
    /// this trait, which every representation can implement.
    fn cc2_write(&mut self, bytes: &[u8]);

    /// The width pending on this stream, taking it (C++ resets width after one
    /// inserted item).
    fn cc2_take_width(&mut self) -> usize;
    /// Set the pending width -- `std::setw`.
    fn cc2_set_width(&mut self, width: usize);
    /// The current fill character -- persists until changed.
    fn cc2_fill(&self) -> u8;
    /// Set the fill character -- `std::setfill`.
    fn cc2_set_fill(&mut self, fill: u8);

    /// Write `body` padded to the pending width with the pending fill, then
    /// clear the width.
    ///
    /// Right-alignment is the default for every type C++ prints here, numbers
    /// and strings alike (`std::setw(5) << 42` is `   42` and
    /// `std::setw(5) << "hi"` is `   hi`); `std::left`/`std::internal` are not
    /// modelled, see the note on `cc2_insert_bytes`.
    fn cc2_pad_and_write(&mut self, body: &[u8]) {
        let width = self.cc2_take_width();
        if width > body.len() {
            // A runtime fill CANNOT go through a Rust format spec: the fill
            // character in `{:f>w$}` must be a literal, only the width can be
            // `w$`.  Verified by compiling.  So the padding is built by hand.
            let fill = self.cc2_fill();
            let pad = vec![fill; width - body.len()];
            self.cc2_write(&pad);
        }
        self.cc2_write(body);
    }
}

/// Insert raw bytes -- a string literal, a `std::string`, a char.
///
/// The basefield does not apply (`o << std::hex << "ff"` is `ff`, measured),
/// but the pending width does, which is the whole reason literal text cannot
/// stay folded into a format string.
///
/// Not modelled, deliberately, because no site in `dcg/ ddc/ dsc/ dbo/` uses
/// them and a wrong guess here is silent: `std::left`/`std::right`/
/// `std::internal` (alignment other than right) and `std::uppercase` (which
/// would make this print `FF` rather than `ff`).  A program that used one would
/// be mis-padded rather than refused -- the one place in this module where that
/// is true, and it is called out here so the next person does not have to
/// rediscover it.
#[inline]
pub fn cc2_insert_bytes<S: Cc2Insert>(mut s: S, bytes: &[u8]) {
    s.cc2_pad_and_write(bytes);
}

/// Insert a POINTER -- what `<<` on any non-char pointer means in C++, i.e. the
/// address, `%p` in printf terms.
///
/// The format is implementation-defined, so this is not a guess: it is what
/// libstdc++ 15.2 -- the standard library the ground-truth oracle
/// (`$TC/shim4/clang++`) links -- actually does.  `num_put::do_put(const void*)`
/// at `bits/locale_facets.tcc:1205` REPLACES the stream's basefield:
///
/// ```text
/// __io.flags((__flags & ~(basefield | uppercase)) | (hex | showbase));
/// ```
///
/// Three consequences follow from that line, and all three were confirmed by
/// running clang-compiled C++ rather than read off the standard:
///
///   * the stream's own base is IGNORED -- `o << std::dec << p` and
///     `o << std::oct << p` both print hex, so this must not consult the flags
///     word the way `cc2_insert_int` does;
///   * the `0x` prefix comes from `showbase`, and showbase emits NO prefix for
///     zero, so a null pointer prints `0` and not `0x0`.  Measured for
///     `int *`, `void *` and `const void *` -- all three give `0`.  This is the
///     part most likely to differ on another standard library, and the one the
///     brief flagged; libc++ cannot be linked on this box, so it is not
///     measurable here and only the libstdc++ answer is claimed;
///   * `uppercase` is cleared too, so the digits are always lowercase.
///
/// The pending width still applies (`setw(24) << p` right-pads to 24 with the
/// fill, measured), so this goes through `cc2_pad_and_write` like everything
/// else.
/// The numeric address of a pointer operand, per model.
///
/// The unsafe model has a real one.  The refcount model does NOT: a `Ptr<T>` is
/// a `Weak` plus an element offset, deliberately carrying no machine address, so
/// there is nothing to print that equals what C++ printed.  What it can supply
/// is a stable IDENTITY -- the address of the heap cell it points at, plus the
/// offset -- which reproduces the two properties a translated program can
/// actually depend on: the same object prints the same text twice, and two
/// different objects print differently.
///
/// The exact digits will not match a C++ run, but they do not match between two
/// C++ runs either (ASLR), so no correct program can depend on them; anything
/// that printed an address for a human to read still gets a usable address, and
/// anything comparing two printed addresses still gets the right answer.  A null
/// pointer is 0 in both models, which is the one address value that IS
/// observable and portable.
pub trait Cc2Addr {
    fn cc2_addr(&self) -> usize;
}

impl<T> Cc2Addr for *const T {
    #[inline]
    fn cc2_addr(&self) -> usize {
        *self as usize
    }
}

impl<T> Cc2Addr for *mut T {
    #[inline]
    fn cc2_addr(&self) -> usize {
        *self as usize
    }
}

impl<T: crate::ByteRepr> Cc2Addr for crate::Ptr<T> {
    #[inline]
    fn cc2_addr(&self) -> usize {
        if self.is_null() {
            return 0;
        }
        // Identity, not a machine address -- see the note above.  The offset is
        // folded in so that `p` and `p + 1` print differently, as C++ does.
        self.cc2_identity()
    }
}

/// The refcount model's `void *` / `const void *`, which is exactly the type a
/// pointer insertion resolves to in C++.  `to_int` is the identity this type
/// already uses for pointer-to-integer casts, so printing agrees with any
/// comparison the translated program makes.
impl Cc2Addr for crate::AnyPtr {
    #[inline]
    fn cc2_addr(&self) -> usize {
        if self.is_null() { 0 } else { self.to_int() }
    }
}

/// A reference to a pointer is a pointer -- the converter sometimes spells the
/// operand as a place rather than a value.
impl<P: Cc2Addr + ?Sized> Cc2Addr for &P {
    #[inline]
    fn cc2_addr(&self) -> usize {
        (**self).cc2_addr()
    }
}

/// Address of whatever pointer spelling the converter hands over.
#[inline]
pub fn cc2_addr_of<P: Cc2Addr + ?Sized>(p: &P) -> usize {
    p.cc2_addr()
}

pub fn cc2_insert_ptr<S: Cc2Insert>(mut s: S, addr: usize) {
    // hex | showbase, with showbase's "nothing for zero" rule -- NOT the
    // stream's basefield.
    let body = if addr == 0 {
        "0".to_string()
    } else {
        format!("0x{:x}", addr)
    };
    s.cc2_pad_and_write(body.as_bytes());
}

/// Insert a NUL-terminated C string -- a `char *` / `const char *` operand.
///
/// C++ prints up to but not including the terminator, and the pending width
/// applies to the whole string.  A null pointer is printed as nothing rather
/// than panicking: C++ has undefined behaviour there, so there is no observable
/// behaviour to reproduce, and a panic would be a worse failure than the UB.
///
/// The two models spell a `const char *` differently -- a raw `*const c_char`
/// in the unsafe model, a checked `Ptr<u8>` in the refcount one -- so the
/// operand arrives through this trait rather than as a concrete pointer type.
/// One helper then serves both, and a third spelling would be a compile error
/// naming the trait rather than silently wrong output.
pub trait Cc2CStr {
    /// The bytes up to, but not including, the NUL terminator.
    fn cc2_cstr_bytes(&self) -> Vec<u8>;
}

impl Cc2CStr for *const std::ffi::c_char {
    fn cc2_cstr_bytes(&self) -> Vec<u8> {
        if self.is_null() {
            return Vec::new();
        }
        unsafe { std::ffi::CStr::from_ptr(*self) }.to_bytes().to_vec()
    }
}

impl Cc2CStr for *mut std::ffi::c_char {
    fn cc2_cstr_bytes(&self) -> Vec<u8> {
        (*self as *const std::ffi::c_char).cc2_cstr_bytes()
    }
}

/// The refcount model's `const char *`.  Reads through the checked pointer one
/// byte at a time and stops at the terminator, so a buffer that is not
/// NUL-terminated raises Ptr's own bounds panic rather than running off the end.
impl Cc2CStr for crate::Ptr<u8> {
    fn cc2_cstr_bytes(&self) -> Vec<u8> {
        let mut out = Vec::new();
        if self.is_null() {
            return out;
        }
        let mut i = 0usize;
        loop {
            let b = self.offset(i as isize).read();
            if b == 0 {
                return out;
            }
            out.push(b);
            i += 1;
        }
    }
}

impl Cc2CStr for crate::Ptr<i8> {
    fn cc2_cstr_bytes(&self) -> Vec<u8> {
        let mut out = Vec::new();
        if self.is_null() {
            return out;
        }
        let mut i = 0usize;
        loop {
            let b = self.offset(i as isize).read();
            if b == 0 {
                return out;
            }
            out.push(b as u8);
            i += 1;
        }
    }
}

/// Takes the pointer BY REFERENCE: the refcount model spells the operand
/// `(*p.borrow())`, a `Ref` deref that cannot be moved out of (E0507), so
/// passing by value would compile for one model and not the other.
pub fn cc2_insert_cstr<S: Cc2Insert, P: Cc2CStr + ?Sized>(mut s: S, p: &P) {
    let bytes = p.cc2_cstr_bytes();
    s.cc2_pad_and_write(&bytes);
}

/// Insert an integer under the stream's CURRENT base.
///
/// `bytes` is the width of the original C++ type and `signed` its signedness,
/// and both are load-bearing rather than decoration.  C++ prints a negative
/// value under hex or oct as the unsigned reinterpretation *at that type's own
/// width*, which is the one place the original type cannot be recovered from a
/// widened value.  Measured:
///
/// ```text
/// o << std::hex << -1 << "|" << (short)-1 << "|" << (long)-1;
///   ffffffff|ffff|ffffffffffffffff
/// ```
///
/// so the same `-1` must print 8, 4 and 16 digits depending on its static type.
/// Under `dec` the sign is kept instead, which is why this cannot simply always
/// mask.
pub fn cc2_insert_int<S: Cc2Insert>(mut s: S, value: i128, bytes: u32, signed: bool) {
    let radix = radix_of_flags(s.cc2_get_flags());
    let body = if radix == 10 {
        if signed {
            format!("{}", value)
        } else {
            format!("{}", value as u128)
        }
    } else {
        // Reinterpret at the ORIGINAL type's width, as C++ does.
        let bits = (bytes * 8).min(128);
        let masked: u128 = if bits >= 128 {
            value as u128
        } else {
            (value as u128) & ((1u128 << bits) - 1)
        };
        match radix {
            16 => format!("{:x}", masked),
            8 => format!("{:o}", masked),
            _ => format!("{}", masked),
        }
    };
    s.cc2_pad_and_write(body.as_bytes());
}

/// Insert a floating-point value.  The basefield does not apply -- measured,
/// `o << std::hex << 1.5` is `1.5` -- but the pending width does.
///
/// C++'s default is `defaultfloat` with precision 6, which is what Rust's
/// `{}` already produces for the values in scope; `std::fixed`,
/// `std::scientific` and `std::setprecision` are not modelled.
pub fn cc2_insert_f64<S: Cc2Insert>(mut s: S, value: f64) {
    let body = format!("{}", value);
    s.cc2_pad_and_write(body.as_bytes());
}

/// Insert a `bool`.  C++ without `std::boolalpha` prints `1`/`0`, and the
/// basefield does not change that (measured: `o << std::hex << true` is `1`).
pub fn cc2_insert_bool<S: Cc2Insert>(mut s: S, value: bool) {
    let body = if value { b"1" as &[u8] } else { b"0" as &[u8] };
    s.cc2_pad_and_write(body);
}

/// Apply a base manipulator on the INSERTION side, unsafe model.
///
/// Distinct from `manip_unsafe` only in that it returns nothing: insertion is
/// emitted as a statement per item, so there is no chained receiver to hand
/// back, and returning `&mut S` would borrow the receiver for longer than the
/// statement in which it appears.  The state it writes is the same flags word
/// `manip_unsafe` writes, which is what lets `<<  std::hex` govern a later `>>`.
///
/// # Safety
///
/// `m` must only write through the pointer it is given.
#[inline]
pub unsafe fn cc2_manip_unsafe<S: Cc2Insert>(
    mut s: S,
    m: unsafe fn(*mut u32) -> *mut u32,
) {
    unsafe { s.cc2_apply_unsafe(m) };
}

/// The refcount counterpart.  Takes the receiver BY VALUE because the refcount
/// model's receiver is a `Ptr<..>`, which is itself a handle.
#[inline]
pub fn cc2_manip_refcount<S: Cc2Insert>(mut s: S, m: fn(crate::Ptr<u32>) -> crate::Ptr<u32>) {
    s.cc2_apply_refcount(m);
}

/// `std::setw` as the converter reaches it: a value inserted into the stream.
#[inline]
pub fn cc2_apply_setw<S: Cc2Insert>(mut s: S, width: i64) {
    s.cc2_set_width(if width > 0 { width as usize } else { 0 });
}

/// `std::setfill`.
#[inline]
pub fn cc2_apply_setfill<S: Cc2Insert>(mut s: S, fill: i8) {
    s.cc2_set_fill(fill as u8);
}

impl Cc2Insert for StringStream {
    fn cc2_write(&mut self, bytes: &[u8]) {
        self.buf.extend_from_slice(bytes);
    }

    #[inline]
    fn cc2_take_width(&mut self) -> usize {
        std::mem::replace(&mut self.width, 0)
    }
    #[inline]
    fn cc2_set_width(&mut self, width: usize) {
        self.width = width;
    }
    #[inline]
    fn cc2_fill(&self) -> u8 {
        self.fill
    }
    #[inline]
    fn cc2_set_fill(&mut self, fill: u8) {
        self.fill = fill;
    }
}

/// A boxed stream is still that stream -- see the `Cc2Extract` forwarding impl.
impl<S: Cc2Insert + ?Sized> Cc2Insert for Box<S> {
    #[inline]
    fn cc2_write(&mut self, bytes: &[u8]) {
        (**self).cc2_write(bytes)
    }

    #[inline]
    fn cc2_take_width(&mut self) -> usize {
        (**self).cc2_take_width()
    }
    #[inline]
    fn cc2_set_width(&mut self, width: usize) {
        (**self).cc2_set_width(width)
    }
    #[inline]
    fn cc2_fill(&self) -> u8 {
        (**self).cc2_fill()
    }
    #[inline]
    fn cc2_set_fill(&mut self, fill: u8) {
        (**self).cc2_set_fill(fill)
    }
}

// The refcount model's receiver.
//
// A refcount stream parameter arrives as `Ptr<Box<StringStream>>` (or
// `Ptr<File>`), and the converter emits the receiver as that pointer, not as a
// borrow -- so the helpers have to accept it directly.  Going through
// `with_mut` for each method keeps the RefCell borrow scoped to one call, which
// matters because a `RefMut` held across a whole helper body is the E0716 that
// the extraction rules were rewritten to avoid.
//
// `Write` is implemented here too rather than relying on Ptr's inherent
// `write_fmt`/`write_all`: the `Cc2Insert: Write` bound needs the TRAIT, and
// having it means `cc2_pad_and_write` works unchanged for this representation.
impl<T: Cc2Extract + crate::ByteRepr> Cc2Extract for crate::Ptr<T> {
    #[inline]
    fn cc2_get_flags(&self) -> u32 {
        self.with(|s| s.cc2_get_flags())
    }
    #[inline]
    fn cc2_put_flags(&mut self, flags: u32) {
        self.with_mut(|s| s.cc2_put_flags(flags))
    }
    #[inline]
    fn cc2_get_state(&self) -> u32 {
        self.with(|s| s.cc2_get_state())
    }
    #[inline]
    fn cc2_put_state(&mut self, state: u32) {
        self.with_mut(|s| s.cc2_put_state(state))
    }
    #[inline]
    fn cc2_getline(&mut self, delim: u8) -> Vec<u8> {
        self.with_mut(|s| s.cc2_getline(delim))
    }
    #[inline]
    fn cc2_int_token(&mut self) -> (String, u32) {
        self.with_mut(|s| s.cc2_int_token())
    }
    #[inline]
    fn cc2_float_token(&mut self) -> String {
        self.with_mut(|s| s.cc2_float_token())
    }
}

// A SHARED borrow of a Ptr is a stream too.
//
// The refcount model spells a `std::ostream *` receiver `(*os.borrow())`, which
// is a `Ptr<File>` behind a `Ref` -- a place that cannot be moved out of
// (E0507, measured on tests/unit/cout_alias.cpp).  A `Ptr` is a handle and is
// `Clone`, so a shared borrow is enough to operate on the stream it points at;
// cloning here is cloning the HANDLE, not the stream, so both spellings act on
// the same underlying object.
impl<T: Cc2Extract + crate::ByteRepr> Cc2Extract for &crate::Ptr<T> {
    #[inline]
    fn cc2_get_flags(&self) -> u32 {
        (*self).cc2_get_flags()
    }
    #[inline]
    fn cc2_put_flags(&mut self, flags: u32) {
        (*self).clone().cc2_put_flags(flags)
    }
    #[inline]
    fn cc2_int_token(&mut self) -> (String, u32) {
        (*self).clone().cc2_int_token()
    }
    #[inline]
    fn cc2_float_token(&mut self) -> String {
        (*self).clone().cc2_float_token()
    }
    #[inline]
    fn cc2_get_state(&self) -> u32 {
        (*self).cc2_get_state()
    }
    #[inline]
    fn cc2_put_state(&mut self, state: u32) {
        (*self).clone().cc2_put_state(state)
    }
    #[inline]
    fn cc2_getline(&mut self, delim: u8) -> Vec<u8> {
        (*self).clone().cc2_getline(delim)
    }
}

impl<T: Cc2Insert + crate::ByteRepr> Cc2Insert for &crate::Ptr<T> {
    #[inline]
    fn cc2_write(&mut self, bytes: &[u8]) {
        self.with_mut(|s| s.cc2_write(bytes))
    }

    #[inline]
    fn cc2_take_width(&mut self) -> usize {
        (*self).clone().cc2_take_width()
    }
    #[inline]
    fn cc2_set_width(&mut self, width: usize) {
        (*self).clone().cc2_set_width(width)
    }
    #[inline]
    fn cc2_fill(&self) -> u8 {
        (*self).cc2_fill()
    }
    #[inline]
    fn cc2_set_fill(&mut self, fill: u8) {
        (*self).clone().cc2_set_fill(fill)
    }
}

impl<T: Cc2Insert + crate::ByteRepr> Cc2Insert for crate::Ptr<T> {
    #[inline]
    fn cc2_write(&mut self, bytes: &[u8]) {
        self.with_mut(|s| s.cc2_write(bytes))
    }

    #[inline]
    fn cc2_take_width(&mut self) -> usize {
        self.with_mut(|s| s.cc2_take_width())
    }
    #[inline]
    fn cc2_set_width(&mut self, width: usize) {
        self.with_mut(|s| s.cc2_set_width(width))
    }
    #[inline]
    fn cc2_fill(&self) -> u8 {
        self.with(|s| s.cc2_fill())
    }
    #[inline]
    fn cc2_set_fill(&mut self, fill: u8) {
        self.with_mut(|s| s.cc2_set_fill(fill))
    }
}

// A mutable BORROW of a stream is a stream.
//
// The insertion helpers take their receiver BY VALUE, because the refcount
// model's receiver is a `Ptr<..>` -- itself a handle, which the converter emits
// as a value, not as a place that can be borrowed.  The unsafe model hands over
// `&mut o` or `&mut (*p)`.  These forwarding impls let ONE helper signature
// serve both spellings, instead of each model needing its own entry point.
impl<S: Cc2Extract + ?Sized> Cc2Extract for &mut S {
    #[inline]
    fn cc2_get_flags(&self) -> u32 {
        (**self).cc2_get_flags()
    }
    #[inline]
    fn cc2_put_flags(&mut self, flags: u32) {
        (**self).cc2_put_flags(flags)
    }
    #[inline]
    fn cc2_get_state(&self) -> u32 {
        (**self).cc2_get_state()
    }
    #[inline]
    fn cc2_put_state(&mut self, state: u32) {
        (**self).cc2_put_state(state)
    }
    #[inline]
    fn cc2_getline(&mut self, delim: u8) -> Vec<u8> {
        (**self).cc2_getline(delim)
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

impl<S: Cc2Insert + ?Sized> Cc2Insert for &mut S {
    #[inline]
    fn cc2_write(&mut self, bytes: &[u8]) {
        (**self).cc2_write(bytes)
    }

    #[inline]
    fn cc2_take_width(&mut self) -> usize {
        (**self).cc2_take_width()
    }
    #[inline]
    fn cc2_set_width(&mut self, width: usize) {
        (**self).cc2_set_width(width)
    }
    #[inline]
    fn cc2_fill(&self) -> u8 {
        (**self).cc2_fill()
    }
    #[inline]
    fn cc2_set_fill(&mut self, fill: u8) {
        (**self).cc2_set_fill(fill)
    }
}

// A RAW POINTER to a stream is a stream.
//
// `rules/iostream` hands back `*mut File` for `std::cout`/`std::cerr` rather than
// an owned `File`, and that spelling is forced: any owned File made from the
// shared descriptor closes fd 1 when it drops, which showed up first as silently
// printing `255` where C++ printed `ff` (the format state was recorded against a
// descriptor that had already been recycled) and then, via `ptr::read`, as a
// runtime abort, "IO Safety violation: owned file descriptor already closed".
// Operating through the pointer materialises no owner, so nothing can drop one.
//
// # Safety
//
// The pointer must be valid for the duration of the call, which holds for the
// thread-locals `cout_unsafe`/`cerr_unsafe` return.
impl<S: Cc2Extract + ?Sized> Cc2Extract for *mut S {
    #[inline]
    fn cc2_get_flags(&self) -> u32 {
        unsafe { (**self).cc2_get_flags() }
    }
    #[inline]
    fn cc2_put_flags(&mut self, flags: u32) {
        unsafe { (**self).cc2_put_flags(flags) }
    }
    #[inline]
    fn cc2_int_token(&mut self) -> (String, u32) {
        unsafe { (**self).cc2_int_token() }
    }
    #[inline]
    fn cc2_float_token(&mut self) -> String {
        unsafe { (**self).cc2_float_token() }
    }
    #[inline]
    fn cc2_get_state(&self) -> u32 {
        unsafe { (**self).cc2_get_state() }
    }
    #[inline]
    fn cc2_put_state(&mut self, state: u32) {
        unsafe { (**self).cc2_put_state(state) }
    }
    #[inline]
    fn cc2_getline(&mut self, delim: u8) -> Vec<u8> {
        unsafe { (**self).cc2_getline(delim) }
    }
}

impl<S: Cc2Insert + ?Sized> Cc2Insert for *mut S {
    #[inline]
    fn cc2_write(&mut self, bytes: &[u8]) {
        unsafe { (**self).cc2_write(bytes) }
    }

    #[inline]
    fn cc2_take_width(&mut self) -> usize {
        unsafe { (**self).cc2_take_width() }
    }
    #[inline]
    fn cc2_set_width(&mut self, width: usize) {
        unsafe { (**self).cc2_set_width(width) }
    }
    #[inline]
    fn cc2_fill(&self) -> u8 {
        unsafe { (**self).cc2_fill() }
    }
    #[inline]
    fn cc2_set_fill(&mut self, fill: u8) {
        unsafe { (**self).cc2_set_fill(fill) }
    }
}

impl Cc2Insert for std::fs::File {
    fn cc2_write(&mut self, bytes: &[u8]) {
        use std::io::Write as _;
        let _ = self.write_all(bytes);
    }

    #[inline]
    fn cc2_take_width(&mut self) -> usize {
        let fd = self.as_raw_fd();
        let mut st = state_of_fd(fd);
        let w = std::mem::replace(&mut st.width, 0);
        set_state_of_fd(fd, st);
        w
    }
    #[inline]
    fn cc2_set_width(&mut self, width: usize) {
        let fd = self.as_raw_fd();
        let mut st = state_of_fd(fd);
        st.width = width;
        set_state_of_fd(fd, st);
    }
    #[inline]
    fn cc2_fill(&self) -> u8 {
        state_of_fd(self.as_raw_fd()).fill
    }
    #[inline]
    fn cc2_set_fill(&mut self, fill: u8) {
        let fd = self.as_raw_fd();
        let mut st = state_of_fd(fd);
        st.fill = fill;
        set_state_of_fd(fd, st);
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
    #[inline]
    fn cc2_get_state(&self) -> u32 {
        self.state
    }
    #[inline]
    fn cc2_put_state(&mut self, state: u32) {
        self.state = state;
    }
    fn cc2_getline(&mut self, delim: u8) -> Vec<u8> {
        if self.buf.is_empty() {
            // Nothing at all to read: failbit AND eofbit, and no line.
            self.cc2_set_state(CC2_FAILBIT | CC2_EOFBIT);
            return Vec::new();
        }
        match self.buf.iter().position(|&c| c == delim) {
            Some(at) => {
                // The delimiter is consumed but not returned.
                let line: Vec<u8> = self.buf.drain(..=at).take(at).collect();
                line
            }
            None => {
                // A final line with no trailing delimiter SUCCEEDS and sets
                // eofbit only -- measured, `istringstream("x")` gives
                // ok=1 eof=1 fail=0.
                self.cc2_set_state(CC2_EOFBIT);
                self.buf.drain(..).collect()
            }
        }
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
    fn cc2_get_state(&self) -> u32 {
        (**self).cc2_get_state()
    }
    #[inline]
    fn cc2_put_state(&mut self, state: u32) {
        (**self).cc2_put_state(state)
    }
    #[inline]
    fn cc2_getline(&mut self, delim: u8) -> Vec<u8> {
        (**self).cc2_getline(delim)
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
    #[inline]
    fn cc2_get_state(&self) -> u32 {
        state_of_fd(self.as_raw_fd()).state
    }
    #[inline]
    fn cc2_put_state(&mut self, state: u32) {
        let fd = self.as_raw_fd();
        let mut st = state_of_fd(fd);
        st.state = state;
        set_state_of_fd(fd, st);
    }
    fn cc2_getline(&mut self, delim: u8) -> Vec<u8> {
        use std::io::Read;
        let mut out: Vec<u8> = Vec::new();
        let mut b = [0u8; 1];
        let mut saw_delim = false;
        loop {
            match self.read(&mut b) {
                Ok(0) => break,
                Ok(_) => {
                    if b[0] == delim {
                        saw_delim = true;
                        break;
                    }
                    out.push(b[0]);
                }
                Err(_) => break,
            }
        }
        if saw_delim {
            return out;
        }
        if out.is_empty() {
            self.cc2_set_state(CC2_FAILBIT | CC2_EOFBIT);
        } else {
            self.cc2_set_state(CC2_EOFBIT);
        }
        out
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

    // ---------------------------------------------------------------------
    // Insertion.  Every expectation below is the output of the
    // clang-compiled C++ probe in iso/hexfix/gt/, not a reading of the
    // standard.
    // ---------------------------------------------------------------------

    fn ins_str(s: &StringStream) -> String {
        String::from_utf8_lossy(&s[..]).into_owned()
    }

    /// The minimal repro this whole change exists for: `std::hex` set in one
    /// statement and restored two statements later.  C++: `a:0xff;255`.
    #[test]
    fn base_survives_the_statement_that_set_it() {
        let mut o = StringStream::new();
        cc2_insert_bytes(&mut o, b"a:");
        o.cc2_apply_manip(|f| *f = with_basefield(*f, CC2_HEX));
        cc2_insert_bytes(&mut o, b"0x");
        cc2_insert_int(&mut o, 255, 4, true);
        cc2_insert_bytes(&mut o, b";");
        o.cc2_apply_manip(|f| *f = with_basefield(*f, CC2_DEC));
        cc2_insert_int(&mut o, 255, 4, true);
        assert_eq!(ins_str(&o), "a:0xff;255");
    }

    /// Two streams must not share state.  It is a field, so they cannot -- but
    /// this is the case most likely to be silently wrong, so it is pinned.
    #[test]
    fn state_does_not_leak_between_streams() {
        let mut a = StringStream::new();
        let mut b = StringStream::new();
        a.cc2_apply_manip(|f| *f = with_basefield(*f, CC2_HEX));
        cc2_insert_int(&mut a, 255, 4, true);
        cc2_insert_int(&mut b, 255, 4, true);
        assert_eq!((ins_str(&a).as_str(), ins_str(&b).as_str()), ("ff", "255"));
    }

    /// `o << std::hex << setw(6) << setfill('.') << 255 << "|" << 255 << "|"
    ///    << std::dec << 255` is `....ff|ff|255`: width is consumed by ONE
    /// item, base and fill persist.
    #[test]
    fn width_resets_but_base_and_fill_persist() {
        let mut o = StringStream::new();
        o.cc2_apply_manip(|f| *f = with_basefield(*f, CC2_HEX));
        cc2_apply_setw(&mut o, 6);
        cc2_apply_setfill(&mut o, b'.' as i8);
        cc2_insert_int(&mut o, 255, 4, true);
        cc2_insert_bytes(&mut o, b"|");
        cc2_insert_int(&mut o, 255, 4, true);
        cc2_insert_bytes(&mut o, b"|");
        o.cc2_apply_manip(|f| *f = with_basefield(*f, CC2_DEC));
        cc2_insert_int(&mut o, 255, 4, true);
        assert_eq!(ins_str(&o), "....ff|ff|255");
    }

    /// The silent-wrongness case that had no assert: `setw(4)` then 7 is
    /// `   7`, NOT `47`.  And width pads a string or a char just the same.
    #[test]
    fn setw_pads_rather_than_printing_itself() {
        let mut o = StringStream::new();
        cc2_insert_bytes(&mut o, b"[");
        cc2_apply_setw(&mut o, 4);
        cc2_insert_int(&mut o, 7, 4, true);
        cc2_insert_bytes(&mut o, b"]");
        assert_eq!(ins_str(&o), "[   7]");

        // `o << std::setw(5) << "ab"` -> `   ab`
        let mut o = StringStream::new();
        cc2_apply_setw(&mut o, 5);
        cc2_insert_bytes(&mut o, b"ab");
        assert_eq!(ins_str(&o), "   ab");

        // Width shorter than the value does not truncate: setw(2) << 12345.
        let mut o = StringStream::new();
        cc2_apply_setw(&mut o, 2);
        cc2_insert_int(&mut o, 12345, 4, true);
        assert_eq!(ins_str(&o), "12345");

        // setw(0) is a no-op.
        let mut o = StringStream::new();
        cc2_apply_setw(&mut o, 0);
        cc2_insert_int(&mut o, 42, 4, true);
        assert_eq!(ins_str(&o), "42");
    }

    /// A negative value under hex/oct prints the unsigned reinterpretation at
    /// the ORIGINAL type's width: `-1` is 8, 4 and 16 digits as int, short and
    /// long.  Under dec the sign is kept.
    #[test]
    fn negative_under_hex_masks_at_the_declared_width() {
        let mut o = StringStream::new();
        o.cc2_apply_manip(|f| *f = with_basefield(*f, CC2_HEX));
        cc2_insert_int(&mut o, -1, 4, true);
        cc2_insert_bytes(&mut o, b"|");
        cc2_insert_int(&mut o, -1, 2, true);
        cc2_insert_bytes(&mut o, b"|");
        cc2_insert_int(&mut o, -1, 8, true);
        assert_eq!(ins_str(&o), "ffffffff|ffff|ffffffffffffffff");

        let mut o = StringStream::new();
        cc2_insert_int(&mut o, -1, 4, true);
        assert_eq!(ins_str(&o), "-1");
    }

    /// The basefield applies to integers only: a string, a float and a bool are
    /// unaffected.  `o << std::hex << "ff" << 1.5 << true` is `ff1.51`.
    #[test]
    fn basefield_does_not_touch_non_integers() {
        let mut o = StringStream::new();
        o.cc2_apply_manip(|f| *f = with_basefield(*f, CC2_HEX));
        cc2_insert_bytes(&mut o, b"ff");
        cc2_insert_f64(&mut o, 1.5);
        cc2_insert_bool(&mut o, true);
        cc2_insert_int(&mut o, 255, 4, true);
        assert_eq!(ins_str(&o), "ff1.51ff");
    }

    #[test]
    fn oct_inserts() {
        let mut o = StringStream::new();
        o.cc2_apply_manip(|f| *f = with_basefield(*f, CC2_OCT));
        cc2_insert_int(&mut o, 64, 4, true);
        cc2_insert_bytes(&mut o, b"|");
        o.cc2_apply_manip(|f| *f = with_basefield(*f, CC2_DEC));
        cc2_insert_int(&mut o, 64, 4, true);
        assert_eq!(ins_str(&o), "100|64");
    }

    /// ONE flags word for both directions, which is what
    /// dcg/tools/mda/memDumpAnalyzer.h:251 needs: the base is set by `<<` and
    /// consumed by a later `>>`.  C++ reads "ff" back as 255.
    #[test]
    fn insertion_and_extraction_share_one_flags_word() {
        let mut s = StringStream::new();
        s.cc2_apply_manip(|f| *f = with_basefield(*f, CC2_HEX));
        cc2_insert_bytes(&mut s, b"ff");
        let (text, radix) = s.cc2_int_token();
        assert_eq!(radix, 16);
        assert_eq!(parse_i64(&text, radix), 255);
    }

    /// `while (getline(f, line))` over `"a\nb\nc\n"`: three lines, then the
    /// loop ends because the fourth read FAILED, not because a position
    /// happened to reach a length.  Ground truth from the clang-compiled probe:
    /// `ok=1,1,0,0` with `eof=0,0,1,1` and `fail=0,0,1,1`.
    #[test]
    fn getline_sets_failbit_so_a_while_loop_terminates() {
        let mut s = StringStream::from_vec(b"a\nb\nc\n".to_vec());
        let mut lines = Vec::new();
        while {
            let l = s.cc2_getline(b'\n');
            if s.cc2_ok() {
                lines.push(String::from_utf8_lossy(&l).into_owned());
            }
            s.cc2_ok()
        } {}
        assert_eq!(lines, vec!["a", "b", "c"]);
        assert_eq!(s.cc2_get_state() & CC2_FAILBIT, CC2_FAILBIT);
    }

    /// The case that makes eof and failure genuinely independent, and so rules
    /// out deriving one from the other: a final line with NO trailing newline
    /// succeeds while already at eof.  Measured: `ok=1 eof=1 fail=0`.
    #[test]
    fn a_last_line_without_a_newline_succeeds_at_eof() {
        let mut s = StringStream::from_vec(b"x".to_vec());
        let line = s.cc2_getline(b'\n');
        assert_eq!(&line[..], b"x");
        assert!(s.cc2_ok(), "C++ reports fail=0 here");
        assert_eq!(s.cc2_get_state() & CC2_EOFBIT, CC2_EOFBIT);
        // The NEXT read fails.
        let _ = s.cc2_getline(b'\n');
        assert!(!s.cc2_ok());
    }

    /// An empty stream fails on the FIRST read -- `ok=0 eof=1 fail=1`.
    #[test]
    fn an_empty_stream_fails_immediately() {
        let mut s = StringStream::new();
        let line = s.cc2_getline(b'\n');
        assert!(line.is_empty());
        assert!(!s.cc2_ok());
        assert_eq!(s.cc2_get_state() & CC2_EOFBIT, CC2_EOFBIT);
    }

    /// The `File` side must agree with the buffer side line for line and bit
    /// for bit -- that is the part no amount of reading the code establishes.
    #[test]
    fn file_getline_agrees_with_the_buffer_getline() {
        for input in ["a\nb\nc\n", "x", "", "a\n", "a\nb"] {
            let path = std::env::temp_dir().join("cc2_getline_agree");
            std::fs::write(&path, input).unwrap();
            let mut f = std::fs::File::open(&path).unwrap();
            f.cc2_clear_state();
            let mut s = StringStream::from_vec(input.as_bytes().to_vec());

            for step in 0..4 {
                let fl = f.cc2_getline(b'\n');
                let sl = s.cc2_getline(b'\n');
                assert_eq!(fl, sl, "line {step} of {input:?}");
                assert_eq!(
                    f.cc2_ok(),
                    s.cc2_ok(),
                    "ok after line {step} of {input:?}"
                );
            }
            let _ = std::fs::remove_file(&path);
        }
    }

    /// The `File` representation carries the same three pieces of state, and a
    /// fresh descriptor starts where a fresh C++ stream does.
    #[test]
    fn file_side_carries_width_and_fill() {
        let mut st = state_of_fd(4243);
        assert_eq!((st.flags, st.width, st.fill), (CC2_DEC, 0, b' '));
        st.width = 5;
        st.fill = b'*';
        st.flags = CC2_HEX;
        set_state_of_fd(4243, st);
        let st = state_of_fd(4243);
        assert_eq!((radix_of_flags(st.flags), st.width, st.fill), (16, 5, b'*'));
        // reset_fd must clear all three, not just the base -- otherwise a
        // recycled descriptor inherits a width its C++ counterpart never had.
        reset_fd(4243);
        let st = state_of_fd(4243);
        assert_eq!((st.flags, st.width, st.fill), (CC2_DEC, 0, b' '));
    }
}

/// `std::chrono::steady_clock::now()`, as nanoseconds since ONE process-wide origin.
///
/// This lives in libcc2rs rather than in a rule body for a reason that was measured:
/// a rule body is INLINED AT EVERY CALL SITE, so a `static OnceLock` inside it becomes
/// a SEPARATE origin per site.  Each `now()` then returned "nanos since this site's
/// first call" -- roughly 100 ns at a cold site and 40 ns at a warm one -- so `end`
/// was systematically SMALLER than `start` and every elapsed-time measurement in the
/// port was approximately zero.  That also masked an inverted subtraction in
/// `rules/chrono` f2, because the reversed body came out positive every time.
///
/// `Instant` is `CLOCK_MONOTONIC`, which is what `steady_clock` requires; it is never
/// the wall clock.
pub fn cc2_steady_now_nanos() -> i64 {
    static ORIGIN: std::sync::OnceLock<std::time::Instant> = std::sync::OnceLock::new();
    ORIGIN.get_or_init(std::time::Instant::now).elapsed().as_nanos() as i64
}
