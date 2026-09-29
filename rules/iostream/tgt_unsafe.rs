// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::os::fd::{AsFd, FromRawFd, IntoRawFd};

fn t1() -> std::fs::File {
    std::fs::File::open("").unwrap()
}

// TODO: t2 and t3 should be translated to *mut dyn Traits
unsafe fn t2() -> *mut std::fs::File {
    libcc2rs::cout_unsafe()
}

unsafe fn t3() -> *mut std::fs::File {
    libcc2rs::cout_unsafe()
}

unsafe fn f1() -> ::std::fs::File {
    std::fs::File::from_raw_fd(
        std::io::stdout()
            .as_fd()
            .try_clone_to_owned()
            .unwrap()
            .into_raw_fd(),
    )
}

unsafe fn f2() -> ::std::fs::File {
    std::fs::File::from_raw_fd(
        std::io::stderr()
            .as_fd()
            .try_clone_to_owned()
            .unwrap()
            .into_raw_fd(),
    )
}

unsafe fn f3() -> *mut ::std::fs::File {
    libcc2rs::cout_unsafe()
}

unsafe fn f4() -> *mut ::std::fs::File {
    libcc2rs::cerr_unsafe()
}

// f5/f6 -- std::ios_base::in / out.  libcxx/ios:287-288 gives in = 0x08 and
// out = 0x10 with `typedef unsigned int openmode`, and the converter maps
// `unsigned int` to u32 (`search type unsigned int, result: u32`).  These are
// plain constants: no receiver, no pointer text, so the refcount model inherits
// them unchanged (ir_unsafe.json is the unconditional base layer).
fn f5() -> u32 {
    0x08
}

fn f6() -> u32 {
    0x10
}

// f7 -- std::ios_base::binary == 0x04 (libcxx/ios:286). Plain `unsigned int` constant,
// nothing model-dependent, so tgt_refcount.rs restates it byte-identically.
fn f7() -> u32 {
    0x04
}

// t4 -- std::istream, i.e. `std::basic_istream<char, std::char_traits<char>>`.
//
// ⭐ FLIPPED FROM `std::fs::File` TO `libcc2rs::IStream`, 2026-09-28, and the reason is
// that the old model made the type key UNUSABLE BY ANYTHING. `std::fs::File` was chosen
// as "t1's counterpart", but the counterpart argument only holds for the OUTPUT side: the
// ostream lowering is not rule-driven and only ever needs `std::io::Write`, whereas every
// input operation IS rule-driven and needs a READ CURSOR plus the STICKY FAILBIT that
// makes `while (ss >> tok)` terminate. A `File` has neither, so t4 could be a type key
// and nothing else -- exactly as its old comment admitted.
//
// `libcc2rs::IStream` (libcc2rs/src/istream.rs) is that model: buffer + get position +
// failbit/eofbit/badbit, with a private `sentry()` that makes every extractor a no-op once
// the failbit is set. THAT IS WHAT UNBLOCKS f8 BELOW.
//
// ⛔ NOTE WHAT THIS DOES *NOT* FIX, so the next reader does not mistake it: the
// `basic_ios` PREDICATES (`eof()`, `good()`, `operator bool()`, `operator!()`) reach their
// receiver through a DerivedToBase cast to `std::ios`, which the converter emits as a
// non-primitive `as` cast. `IStream` implements all four as methods, but keying them
// requires a type rule for `std::ios` -- the common base of the input AND output stream
// families, modelled three different ways in this tree -- and any such key turns a census
// number to zero while leaving E0605. They stay out; see the report.
fn t4() -> libcc2rs::IStream {
    libcc2rs::IStream::new()
}

// ============================================================================
// f8 -- `std::istream & operator shr(std::istream &, std::string &)`, THE FREE
// `operator>>` FOR std::string. This is the key the 35-TU `ss >> tok` row needed.
//
// SEARCHED SPELLING, read back from a -verbose leg that EXITED 0 (19,656 lines,
// /home/agent/work/g080_vb2/vb.log, witness util/utils.cpp, pin 6b2fbbb5, ir.v31):
//     search expr std::istream & operator shr(std::istream &, std::string &), result:
//     None
// -- character for character, and note `operator shr`, which is how Mapper::ToString
// spells the shifts. The same log shows sibling asks that RESOLVE (e.g.
// `std::__cxx11::basic_string<char>::c_str()`), so the miss is a miss and not a log that
// stopped early.
//
// ⭐ WHY THIS ONE IS WRITABLE WHEN THE PREDICATES ARE NOT -- the distinction is
// ARGUMENT POSITION vs RECEIVER POSITION, and it was measured, not reasoned:
//   * a DerivedToBase conversion in a FUNCTION-ARGUMENT position is emitted with NO cast.
//     dip/dip.cpp.rs:6414 passes a `std::stringstream` to a `std::istream &` parameter as
//     plain `&mut s_stream`.
//   * the same conversion on a MEMBER-CALL RECEIVER is emitted as an `as` cast:
//     dip/dip.cpp.rs:5478 is `(ss as Cpp2RustUnmapped_std_ios).eof()`.
// `operator>>` for std::string is a FREE function, so both its operands are arguments and
// no cast appears. That is why this key lands and `eof()` cannot.
//
// STICKINESS IS PRESERVED THROUGH THE STRING CONVERSION, and this is the part that would
// have been silently wrong. `std::string` in the unsafe model is a NUL-TERMINATED
// Vec<libc::c_char> while `IStream::extract_token` writes raw bytes, so the rule must
// convert in both directions. A body that wrote `*out` unconditionally would CLEAR the
// caller's string on a sticky-failed extraction -- the opposite of the specified
// leave-it-untouched behaviour, and the failbit-dropping mistake this family is warned
// about. Instead the current content is converted IN, handed to the extractor, and
// converted back OUT: if the extractor was a no-op the value round-trips unchanged.
//
// EVERY `aN` IS BOUND TO A LOCAL EXACTLY ONCE. A rule body is inlined and each `aN`
// re-expands to the argument expression verbatim, so naming `a1` twice would evaluate the
// caller's expression twice. `istream_shr_string_unsafe` returns the SAME handle so
// `in >> a >> b` lowers to `shr(shr(in, a), b)` and the outer extraction observes the
// inner one's failbit.
// ============================================================================
// ⛔ THE BODY USES NOTHING BUT METHOD CALLS ON `a0`/`a1`, AND THAT IS A HARD
// REQUIREMENT MEASURED HERE, not a style choice.  The declared `&mut` parameter
// types are a fiction as far as the emitted code is concerned: an `aN` re-expands
// to the ARGUMENT EXPRESSION VERBATIM, and for a `std::istream &` parameter the
// converter emits the bare lvalue, NOT `&mut lvalue`.  A first cut of this body
// wrote `let __s: *mut libcc2rs::IStream = a0;` and the emitted code came out
//     let __s: *mut libcc2rs::IStream = ss;      // dip/dip.cpp.rs, E0308
// -- rc=0, no placeholder, and a type error only a compile would find.  ⭐ Method
// calls are immune because Rust auto-refs the receiver: `a0.m()` compiles both as
// `(&mut IStream).m()` (the declared shape, auto-deref) and as `ss.m()` (the
// substituted shape, auto-ref).  Same for `a1.splice(..)`.
//
// EACH OPERAND APPEARS EXACTLY ONCE, which is why `extract_token_reporting`
// returns the stream pointer and a "was it written" flag, and why the write-back
// is a single `splice(.., ..)` rather than `clear()` + `extend()`: a second
// mention of `a1` would re-evaluate the caller's expression (harmless for a plain
// variable, a double evaluation for anything with an effect).
//
// THE `if __stored` GUARD IS THE STICKY CASE and dropping it would be silently
// wrong: a failed stream must leave its `std::string` untouched, so an
// unconditional write-back would ERASE the caller's string on every no-op read.
unsafe fn f8(a0: &mut libcc2rs::IStream, a1: &mut Vec<libc::c_char>) -> *mut libcc2rs::IStream {
    let mut __b: Vec<u8> = Vec::new();
    let (__r, __stored) = a0.extract_token_reporting(&mut __b);
    if __stored {
        __b.push(0);
        a1.splice(.., __b.iter().map(|&__c| __c as libc::c_char))
            .for_each(drop);
    }
    __r
}

// ============================================================================
// f9 / f10 -- `std::getline`. See src.cpp for the two searched spellings read
// back from a -verbose leg that exited 0, and for the correction of the recorded
// "getline is never searched" diagnosis (it IS searched, 18 times in dip/dip.cpp,
// `result: None` both spellings -- an ordinary missing free-function key).
//
// ⛔ THE BODY USES NOTHING BUT METHOD CALLS ON `a0`/`a1`, for exactly the reason
// f8 above records: a `&mut` parameter's `aN` re-expands to the BARE LVALUE, not
// to `&mut lvalue`, so `let __s: *mut IStream = a0;` emits `let __s = ss;` and
// gives E0308 with rc=0 and no placeholder. Method calls compile under both
// shapes because Rust auto-refs the receiver.
//
// EACH `aN` IS NAMED EXACTLY ONCE. `a2` appears once, inside the delimiter
// conversion; the emitted argument at the measured site is `(',' as libc::c_char)`
// so `a2 as u8` expands to `(',' as libc::c_char) as u8`, a legal primitive cast
// chain.
//
// ⛔ THE `if __stored` GUARD IS THE STICKY CASE, and here it is NOT the same
// predicate as f8's. `getline` succeeds on an EMPTY FIELD (`"a,,b"` split on
// `','` must clear the caller's string for the middle token), so the guard cannot
// be "did any bytes arrive"; it is the stream's sentry result, which is what
// `getline_reporting` reports. Dropping the guard would erase the caller's string
// on every read past end-of-stream -- the loop-exit iteration of
// `while (std::getline(f, line))` writes nothing and must leave `line` alone.
unsafe fn f9(
    a0: &mut libcc2rs::IStream,
    a1: &mut Vec<libc::c_char>,
    a2: libc::c_char,
) -> *mut libcc2rs::IStream {
    let mut __b: Vec<u8> = Vec::new();
    let (__r, __stored) = a0.getline_reporting(&mut __b, a2 as u8);
    if __stored {
        __b.push(0);
        a1.splice(.., __b.iter().map(|&__c| __c as libc::c_char))
            .for_each(drop);
    }
    __r
}

// f10 -- the two-argument form. The delimiter is `'\n'` per [string.io]; it is
// spelled out rather than defaulted because a defaulted argument is a recorded
// rule-authoring trap and because the recorded key's arity is what it is.
unsafe fn f10(
    a0: &mut libcc2rs::IStream,
    a1: &mut Vec<libc::c_char>,
) -> *mut libcc2rs::IStream {
    let mut __b: Vec<u8> = Vec::new();
    let (__r, __stored) = a0.getline_reporting(&mut __b, b'\n');
    if __stored {
        __b.push(0);
        a1.splice(.., __b.iter().map(|&__c| __c as libc::c_char))
            .for_each(drop);
    }
    __r
}

// t5 -- std::ios_base::seekdir, `enum seekdir { beg, cur, end }` (libcxx/ios:295).
// Unscoped enum, no fixed underlying type, promotes to int -> i32. Nothing
// model-dependent, so tgt_refcount.rs restates it byte-identically.
fn t5() -> i32 {
    0
}

// ============================================================================
// f11-f18 -- the MEMBER `operator>>` numeric extractions.  See src.cpp for the
// eight recorded key spellings, for why the shift keys carry no class
// qualification, for the `dtGetEnv<T>` instantiation census, and for the
// narrowing of the blanket "member operators are unwritable" claim at f8.
//
// ⛔ THE BODY IS A SINGLE METHOD CALL ON `a0`, AND BOTH HALVES OF THAT ARE HARD
// REQUIREMENTS measured on this family, not style:
//   * METHOD CALL, because an `aN` for a `std::istream &` parameter re-expands to
//     the BARE LVALUE and not to `&mut lvalue` (f8's comment records the E0308
//     that taught us: `let __s: *mut IStream = a0;` emitted `let __s = ss;`).
//     Rust auto-refs a receiver, so `a0.shr_i32(..)` compiles under both the
//     declared `&mut IStream` shape and the substituted `ss` shape.
//   * EACH OPERAND EXACTLY ONCE, because the body is inlined verbatim.  That is
//     the entire reason `libcc2rs::IStream::shr_*` returns `*mut Self` instead of
//     `()`: the two-statement alternative `{ a0.extract_i32(a1); a0 }` names `a0`
//     twice and would re-evaluate the caller's receiver expression.
//
// `a1` IS A REFERENCE TO A PRIMITIVE, which the converter DOES emit as
// `&mut lvalue` -- rules/string f82 (`std::from_chars(const char *, const char *,
// long &)`, `a2: &mut i64`) is the standing precedent for that shape, and it is
// the one place this family differs from f8, where the `std::string &` argument
// needed a staging buffer and a conditional write-back.  A numeric extractor
// writes its argument itself and already distinguishes the two failure cases
// (sentry-failed leaves it UNTOUCHED, conversion-failed stores 0 per LWG 2176),
// so there is no flag and no `if`.
//
// ⛔ AN IDENTITY BODY HERE WOULD BE THE FORBIDDEN OUTCOME, not a stub: the corpus
// caller tests the returned stream (`if ((ss >> parsed) && ss.eof())`), so a
// truthy no-op makes every `dtGetEnv<int>` return an indeterminate value with no
// diagnostic anywhere.  `shr_*` goes through the sentry and the failbit; the
// round-trip proof is in libcc2rs/src/istream.rs
// (`dtgetenv_shr_int_parses_rejects_trailing_garbage_and_fails_loudly`).
unsafe fn f11(a0: &mut libcc2rs::IStream, a1: &mut i32) -> *mut libcc2rs::IStream {
    a0.shr_i32(a1)
}

unsafe fn f12(a0: &mut libcc2rs::IStream, a1: &mut u32) -> *mut libcc2rs::IStream {
    a0.shr_u32(a1)
}

unsafe fn f13(a0: &mut libcc2rs::IStream, a1: &mut i64) -> *mut libcc2rs::IStream {
    a0.shr_i64(a1)
}

unsafe fn f14(a0: &mut libcc2rs::IStream, a1: &mut u64) -> *mut libcc2rs::IStream {
    a0.shr_u64(a1)
}

// `long long` is a DISTINCT C++ overload from `long` -- overload resolution is
// exact, so it needs its own key -- but both are i64 on LP64, so both land on
// `shr_i64`.  Same for f16 against f14.
unsafe fn f15(a0: &mut libcc2rs::IStream, a1: &mut i64) -> *mut libcc2rs::IStream {
    a0.shr_i64(a1)
}

unsafe fn f16(a0: &mut libcc2rs::IStream, a1: &mut u64) -> *mut libcc2rs::IStream {
    a0.shr_u64(a1)
}

// ⛔ f17 IS NOT f18 NARROWED.  `shr_f32` parses through f64 and then REJECTS a
// magnitude no `float` can hold, because `1e40 as f32` in Rust is a SATURATING
// cast that yields `f32::INFINITY` silently while `num_get` sets failbit --
// i.e. `dtGetEnv<float>("1e40")` would answer `Some(inf)` where C++ answers
// `std::nullopt`.  Tested: `shr_float_fails_on_a_value_no_float_can_hold`.
unsafe fn f17(a0: &mut libcc2rs::IStream, a1: &mut f32) -> *mut libcc2rs::IStream {
    a0.shr_f32(a1)
}

unsafe fn f18(a0: &mut libcc2rs::IStream, a1: &mut f64) -> *mut libcc2rs::IStream {
    a0.shr_f64(a1)
}

// ============================================================================
// f100 -- the free `operator>>(std::istream &, char &)`.  See src.cpp for the
// verbatim recorded key, for why it is FREE rather than a member, and for the
// `DT_CHECK` caller that asserts the extraction FAILS.
//
// ⛔ THE BODY USES NOTHING BUT A METHOD CALL ON `a0`, for the reason f8 records:
// an `aN` for a `std::istream &` parameter re-expands to the BARE LVALUE, not to
// `&mut lvalue`, so a `let __s: *mut IStream = a0;` emits `let __s = ss;` and
// gives E0308 with rc=0 and no placeholder.  A method call is immune because Rust
// auto-refs the receiver.
//
// ⛔ THE `if __stored` GUARD IS NOT COSMETIC, AND IT IS **NOT** LWG 2176 HERE.
// `operator>>(char&)` is not a `num_get` extraction, so a FAILED read leaves the
// argument strictly untouched rather than zeroing it.  The one corpus caller is
// `!static_cast<bool>(iss >> remaining_char)` -- the failing path is the EXPECTED
// path -- so an unconditional write-back would zero `remaining_char` on exactly
// the read the program asserts must fail.
//
// `a1` IS A REFERENCE TO A PRIMITIVE, which the converter DOES emit as
// `&mut lvalue` (rules/string f82 is the standing precedent).  It is bound to
// `__p` once so the staging byte can be written back without naming `a1` twice --
// an `aN` re-expands to the caller's expression verbatim, so a second mention
// would re-evaluate it.
unsafe fn f100(
    a0: &mut libcc2rs::IStream,
    a1: &mut libc::c_char,
) -> *mut libcc2rs::IStream {
    let __p = a1;
    let mut __c: u8 = 0;
    let (__r, __stored) = a0.extract_char_reporting(&mut __c);
    if __stored {
        *__p = __c as libc::c_char;
    }
    __r
}

// ============================================================================
// ⛔⛔ THERE IS DELIBERATELY NO f101/f102/f103/f104 IN THIS FILE, AND THE WHOLE
// POINT OF THIS BLOCK IS TO SAY SO IN THE PLACE A READER WILL LOOK FOR THEM.
// The member `operator>>(std::ios_base &(*)(std::ios_base &))` (i.e.
// `in >> std::hex`) and the three manipulators `std::hex`/`std::dec`/`std::oct`
// are UNKEYED, so `in >> std::hex` still aborts LOUDLY at translate time.  The
// four-part refusal, the verbatim aborts, and the measurement that keying the
// operator alone only moves the abort one step onto `std::ios_base` (row g2894)
// are all in src.cpp -- read that, not this.
//
// ⭐ WHAT *IS* LANDED IS THE MODEL UNDERNEATH THEM: `libcc2rs::IStream` now
// carries a real `basefield` (10/16/8, defaulting to 10) that `extract_i64` /
// `extract_u64` consume, verified against executed C++ ground truth.  It is
// landed ahead of its keys on purpose, on this module's own f12/f15/f16
// precedent, so that whoever lands g2894 has only the keys left to write.
//
// THE SHAPE THOSE FOUR KEYS SHOULD TAKE, recorded so it is not re-derived:
//
//   * `u32`, NOT A FUNCTION POINTER, for the manipulator parameter.  The C++
//     parameter is `std::ios_base &(*)(std::ios_base &)`, but a rule body is
//     INLINED and the only argument text that can ever appear at that position
//     is one of the three `libcc2rs::IOS_BASEFIELD_*` constants.  Modelling it
//     as an actual `fn(&mut …) -> &mut …` would require the `std::ios_base`
//     type rule and would buy nothing: there is no `ios_base` VALUE anywhere in
//     either model to pass to such a function.
//   * `shr_basefield` returns the stream for the same reason `shr_i64` does, so
//     `inFile >> std::hex >> lineno` would lower to
//     `shr_i64(shr_basefield(inFile, HEX), &mut lineno)` and the read would see
//     the base the manipulator set.  Each operand is named exactly once.
//
// ⭐ AND THE AUDIT THAT WOULD MAKE THE LANDED VERSION CHECKABLE: because bodies
// are inlined, a correct `in >> std::hex` must emit
// `inFile.shr_basefield(libcc2rs::IOS_BASEFIELD_HEX)`, so grepping the emitted
// `.rs` for `IOS_BASEFIELD_HEX` proves the base was threaded through rather than
// dropped.  An identity body would leave NO trace at all -- which is exactly what
// would make that trade undetectable at every stage of this harness.
// ============================================================================
