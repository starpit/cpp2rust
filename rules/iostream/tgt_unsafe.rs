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
// ⛔⛔ THE `a1` PARAMETER TYPE IS A FIX, AND THE CLAIM IT REPLACES WAS WRONG.
// This key shipped with `a1: &mut libc::c_char` and a comment asserting "`a1` IS A
// REFERENCE TO A PRIMITIVE, which the converter DOES emit as `&mut lvalue`
// (rules/string f82 is the standing precedent)".  MEASURED, that is false for this
// key: the translated `.rs` for `iss >> remaining_char` reads, verbatim,
//     let __p = remaining_char;
//     ...
//     *__p = __c as libc::c_char;
// -- the operand re-expands to the BARE LVALUE, exactly as the `a0` note three
// paragraphs up says it does, so `*__p` is `error[E0614]: type `i8` cannot be
// dereferenced`.  ⭐ IT TRANSLATED rc=0 AND DID NOT COMPILE: rustfmt parses it,
// no placeholder appears, and nothing in a bucket census can see it.  The f82
// "precedent" is not one -- nothing has ever compiled an f82 call site.
//
// ⭐ THE FIX USES SUBSTITUTION RATHER THAN A REFERENCE: `a1` is declared at the
// operand's ACTUAL type (a `c_char` VALUE) and assigned to, so after inlining the
// body reads `remaining_char = __c as libc::c_char;` -- a plain assignment to the
// caller's own lvalue, which is what the C++ `char &` out-parameter means.  `a1`
// is still named exactly ONCE, so the caller's expression is not re-evaluated.
// Verified end-to-end, not argued: with this signature the hexrow probe compiles
// and its output is BYTE-EXACT against clang in BOTH models (verif/g3090/).
unsafe fn f100(a0: &mut libcc2rs::IStream, mut a1: libc::c_char) -> *mut libcc2rs::IStream {
    let mut __c: u8 = 0;
    let (__r, __stored) = a0.extract_char_reporting(&mut __c);
    if __stored {
        a1 = __c as libc::c_char;
    }
    __r
}

// ============================================================================
// t6 / f101-f104 -- `in >> std::hex`.  This block REPLACES the "THERE IS
// DELIBERATELY NO f101/f102/f103/f104 IN THIS FILE" note that stood here: the
// four keys are now landed, the reason they were held back (no `std::ios_base`
// type rule, row g2894) turned out to be a spelling worry that the preprocessor
// does not actually have, and the fn-pointer parameter round-trips.  The full
// record -- recorded key strings, the measurement, why `u32`, and what is still
// refused -- is in src.cpp.  Read that, not this.
//
// t6 -- `std::ios_base` as the formatting-state WORD.  Model-independent (a bare
// u32), so tgt_refcount.rs restates it byte-identically rather than inheriting.
// The initializer is base 10 because that is `ios_base`'s default basefield.
fn t6() -> u32 {
    libcc2rs::IOS_BASEFIELD_DEC
}

// f101 -- the MEMBER `operator>>(std::ios_base &(*)(std::ios_base &))`.
//
// ⛔⛔ THE PARAMETER TYPE IS MEASURED FROM THE EMITTED TEXT, NOT CHOSEN.  A keyed
// system function that is NAMED rather than CALLED lowers to the `fn` ITEM
// `libcc2rs::hex_unsafe` (mapper.cpp:2660) -- but the converter does not hand it
// over bare, it CASTS it to the model-mapped C++ type.  The translated
// util/sendefs/sendefs.cpp reads, verbatim:
//     (*ss.shr_ios_manip((libcc2rs::hex_unsafe as unsafe fn(*mut u32) -> *mut u32)))
// so with t6 = `u32` the operand's type is `unsafe fn(*mut u32) -> *mut u32`.
// ⭐ THE FIRST VERSION OF THIS KEY USED `fn(u32) -> u32` AND IT TRANSLATED rc=0 --
// the wrong signature is invisible to rustfmt and would have surfaced as an E0308
// one stage later.  Reading the emitted cast is what caught it.  Same shape as the
// refcount arm's f101, which emits NO cast and therefore needs a DIFFERENT
// signature; see tgt_refcount.rs.
//
// ⛔ ONE METHOD CALL ON `a0`, FOR f8/f100's REASON: an `aN` for an
// `std::istream &` parameter re-expands to the BARE LVALUE, so binding it to a
// local gives E0308 at rc=0 with no placeholder.  A method call is immune because
// Rust auto-refs the receiver.  Each `aN` is named exactly once.
//
// ⭐ THE AUDIT THIS MAKES POSSIBLE: because the body is inlined, a correct
// `inFile >> std::hex` must emit `shr_ios_manip(libcc2rs::hex_unsafe)` into the
// translated `.rs`, so grepping for `hex_unsafe` proves the base was threaded
// through rather than dropped.  An identity body would leave no trace at all.
unsafe fn f101(
    a0: &mut libcc2rs::IStream,
    a1: unsafe fn(*mut u32) -> *mut u32,
) -> *mut libcc2rs::IStream {
    a0.shr_ios_manip(a1)
}

// f102-f104 -- `std::hex` / `std::dec` / `std::oct` themselves, at libcxx's own
// arity-1 signature.  No corpus site CALLS a manipulator, so no corpus site
// inlines these bodies; their job is to put the three names in `exprs_` so that
// `Mapper::MapFunctionName` takes its `libcc2rs::` branch at the DECLREF site
// instead of its mangled-name fallback (which, being an `assert`, is silent under
// -DNDEBUG).  They forward to the same items that branch names, so the two routes
// cannot disagree -- rules/cctype + libcc2rs::cctype is the precedent.
unsafe fn f102(a0: *mut u32) -> *mut u32 {
    libcc2rs::hex_unsafe(a0)
}

unsafe fn f103(a0: *mut u32) -> *mut u32 {
    libcc2rs::dec_unsafe(a0)
}

unsafe fn f104(a0: *mut u32) -> *mut u32 {
    libcc2rs::oct_unsafe(a0)
}
