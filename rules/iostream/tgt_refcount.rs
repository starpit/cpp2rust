// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::os::fd::AsFd;

// TODO: t2 and t3 should be translated to Ptr<dyn Traits>
// t1 -- `std::ostream` by VALUE (t2 is `std::ostream &`, t3 is `std::ostream *`).
//
// THIS WAS MISSING, AND ITS ABSENCE POISONED EVERY REFCOUNT TRANSLATION. src.cpp declares
// `using t1 = std::ostream;` and tgt_unsafe.rs has a `t1`, but this file did not -- the
// `using tN =` with no matching tN target case, which aborts at LOAD TIME
// (translation_rule.cpp:233, `ir_src.json type entry has no matching IR target rule`) and
// under NDEBUG presents as rc=139 with no message. `check-ir.sh` reported
// `LOAD FAILURE (refcount model) -- this tree poisons EVERY translation`.
// It was LATENT rather than new: rules/iostream had not been regenerated since #164 made
// Rust type rules standalone functions, so no published tree had ever exercised it, and it
// only surfaced when this module was regenerated for the ios_base::in/out keys.
//
// The body mirrors tgt_unsafe.rs's t1 verbatim because a plain owned value is
// model-independent -- no Ptr, no raw pointer, nothing to re-shape. Deliberately NOT
// "improved": `File::open("")` always fails so this initializer would panic if it were ever
// materialised, which is worth a separate look, but changing committed unsafe behaviour while
// fixing a load failure would conflate two things.
fn t1() -> std::fs::File {
    std::fs::File::open("").unwrap()
}

fn t2() -> Ptr<std::fs::File> {
    Ptr::null()
}

fn t3() -> Ptr<std::fs::File> {
    Ptr::null()
}

fn f1() -> ::std::fs::File {
    std::fs::File::from(std::io::stdout().as_fd().try_clone_to_owned().unwrap())
}

fn f2() -> ::std::fs::File {
    std::fs::File::from(std::io::stderr().as_fd().try_clone_to_owned().unwrap())
}

fn f3() -> Ptr<::std::fs::File> {
    libcc2rs::cout()
}

fn f4() -> Ptr<::std::fs::File> {
    libcc2rs::cerr()
}

// f5/f6 -- std::ios_base::in / ::out. Bodies identical to tgt_unsafe.rs: these are plain
// `unsigned int` constants (libcxx/ios:283-288, `typedef unsigned int openmode; in = 0x08;
// out = 0x10`), so nothing here is model-dependent.
//
// THEY ARE RESTATED ANYWAY, AND THAT IS NOT OPTIONAL. 0edd6227 added them to src.cpp and
// tgt_unsafe.rs only, on the reasoning that a value-like key needs no refcount override
// because ir_unsafe.json is the unconditional base layer. That reasoning is right about the
// OVERLAY and wrong about THIS module: because rules/iostream HAS a tgt_refcount.rs, omitting
// a key from it leaves ir_refcount.json with 6 keys against ir_src.json's 9, and the
// converter then ABORTS AT LOAD TIME in the refcount model -- `check-ir.sh` reported
// `LOAD FAILURE (refcount model) -- this tree poisons EVERY translation` with a core dump.
// So the rule is: a module that has a tgt_refcount.rs must carry EVERY key in it, even where
// the body is byte-identical. (rules/mlir legitimately runs 151 src / 149 refcount keys, so
// the omission is tolerated somewhere -- but not here, and check-ir.sh is the authority.)
fn f5() -> u32 {
    0x08
}

fn f6() -> u32 {
    0x10
}

// f7 -- std::ios_base::binary. Restated byte-identically from tgt_unsafe.rs because this
// module HAS a tgt_refcount.rs and therefore must carry EVERY key in it -- omitting one
// leaves ir_refcount.json short of ir_src.json and the converter ABORTS AT LOAD TIME in
// the refcount model, poisoning every translation. That is the incident documented above
// for f5/f6; it is not hypothetical.
fn f7() -> u32 {
    0x04
}

// t4 -- std::istream -> `libcc2rs::IStream`. See tgt_unsafe.rs for why the old
// `std::fs::File` model was replaced: a File has no read cursor and no sticky failbit, so
// the key could never carry an operation. The body is model-independent (a plain owned
// value), restated here because this module HAS a tgt_refcount.rs and must therefore carry
// EVERY key -- omitting one aborts at LOAD TIME.
fn t4() -> libcc2rs::IStream {
    libcc2rs::IStream::new()
}

// f8 -- the free `operator>>(std::istream &, std::string &)`. See tgt_unsafe.rs for the
// searched spelling, for the argument-position-vs-receiver-position measurement that makes
// this key writable, and for why the sticky case must round-trip the caller's string
// instead of overwriting it.
//
// THIS OVERRIDE IS REQUIRED and not for pointer syntax: BOTH parameter types are
// model-dependent. `std::string` is Vec<u8> here and Vec<libc::c_char> in the unsafe model,
// and an lvalue reference is `Ptr<T>` here and `&mut T` there, so inheriting the unsafe body
// would give E0308 twice over.
//
// `extract_token` is called directly rather than through `istream_shr_string`, because that
// free function takes a `Ptr<Vec<u8>>` for its out-parameter and the value handed to the
// extractor here is a LOCAL staging buffer (the NUL has to come off before the read and go
// back on after it), which has no `Ptr`. The stream handle is still consumed exactly once
// and handed back, so chaining is unaffected.
// ⚠️ THE `__stored` GUARD IS NOT COSMETIC -- without it a sticky-failed read
// would write the staging buffer back and ERASE the caller's std::string, the
// exact opposite of the specified behaviour.  See tgt_unsafe.rs's f8.
//
// `Ptr` is a VALUE, so unlike the unsafe model a `let` binding is legal here and
// each `aN` is still named exactly once.
fn f8(a0: Ptr<libcc2rs::IStream>, a1: Ptr<Vec<u8>>) -> Ptr<libcc2rs::IStream> {
    let __s = a0;
    let __o = a1;
    let mut __b: Vec<u8> = Vec::new();
    let __stored = __s.with_mut_ref(|__st| __st.extract_token_reporting(&mut __b).1);
    if __stored {
        __b.push(0);
        __o.with_mut_ref(|__v| *__v = __b);
    }
    __s
}

// f9 / f10 -- `std::getline`. See tgt_unsafe.rs and src.cpp for the searched
// spellings, for the corrected diagnosis, and for why the sticky guard here is the
// sentry result rather than "were any bytes read" (getline succeeds on an empty
// field, `>>` cannot produce one).
//
// THIS OVERRIDE IS REQUIRED and not merely for pointer syntax: THREE parameter
// types are model-dependent. `std::string` is Vec<u8> here and Vec<libc::c_char>
// in the unsafe model, an lvalue reference is `Ptr<T>` here and `&mut T` there,
// and `char` is `u8` here against `libc::c_char` there (rules/string/tgt_refcount.rs
// f9/f21 are the precedent). Inheriting the unsafe body would give E0308 three
// times over.
//
// ⛔⛔ THIS BODY WAS `libcc2rs::istream_getline(a0, a1, a2)` AND IT DROPPED THE
// LAST BYTE OF EVERY FIELD.  MEASURED, by DIFFING THE PRINTED BYTES of the
// translated program against the C++ program's own stdout (probe
// /home/agent/work/iosfix/iosq.cpp, both models, pin 968ab2be):
//
//     C++ and unsafe:   tok[a]tok[]tok[bc]tok[d]   line[l1]line[l2]line[l3]
//     refcount, BEFORE: tok[]tok[]tok[b]tok[]      line[l]line[l]line[l]
//
// ⭐ THE DELETED COMMENT'S PREMISE WAS FALSE, and it was falsifiable from THIS
// FILE: it claimed "the refcount `std::string` is already a raw `Vec<u8>` with
// no NUL terminator, which is precisely what `IStream::getline` writes".  But
// `f8` TWENTY LINES ABOVE stages into `__b` and does `__b.push(0)` before
// handing it over, and the converter's own `operator<<`-of-std::string emission
// is `v.iter().take(v.len() - 1)` -- i.e. the refcount `std::string` IS
// NUL-terminated and the print path unconditionally discards the final byte.
// Writing the bare field therefore left a Vec of length n whose last character
// was then thrown away.  ⛔ rc=0, ZERO placeholders, compiles clean in both
// models, runs, exits 0 -- only the byte diff sees it.  That is exactly the
// silent-wrongness class this module's manipulator refusal is written about,
// and it had landed on the get side while the refusal guarded the put side.
//
// SECOND, INDEPENDENT BUG IN THE SAME LINE: `istream_getline` has NO STICKY
// GUARD.  The old comment argued one was unnecessary because "`IStream::getline`
// returns before touching `out` on both failure paths".  True of `getline`
// itself -- and irrelevant, because the NUL now has to be appended by the
// CALLER, so the write-back is no longer `getline`'s to skip.  Without the
// `__stored` guard a sticky-failed read would write `[0]` into the caller's
// string and ERASE it, which is precisely what f8's own comment warns about.
// So f9/f10 now mirror f8 EXACTLY: staging buffer, `getline_reporting`, guard,
// `push(0)`, single move-in.
//
// `Ptr` is a VALUE, so each `aN` is named exactly once and still handed on.
fn f9(a0: Ptr<libcc2rs::IStream>, a1: Ptr<Vec<u8>>, a2: u8) -> Ptr<libcc2rs::IStream> {
    let __s = a0;
    let __o = a1;
    let mut __b: Vec<u8> = Vec::new();
    let __stored = __s.with_mut_ref(|__st| __st.getline_reporting(&mut __b, a2).1);
    if __stored {
        __b.push(0);
        __o.with_mut_ref(|__v| *__v = __b);
    }
    __s
}

fn f10(a0: Ptr<libcc2rs::IStream>, a1: Ptr<Vec<u8>>) -> Ptr<libcc2rs::IStream> {
    let __s = a0;
    let __o = a1;
    let mut __b: Vec<u8> = Vec::new();
    let __stored = __s.with_mut_ref(|__st| __st.getline_reporting(&mut __b, b'\n').1);
    if __stored {
        __b.push(0);
        __o.with_mut_ref(|__v| *__v = __b);
    }
    __s
}

// t5 -- std::ios_base::seekdir == int-promoted unscoped enum -> i32. Byte-identical to
// tgt_unsafe.rs; restated for the same load-time reason.
fn t5() -> i32 {
    0
}

// ============================================================================
// f11-f18 -- the MEMBER `operator>>` numeric extractions.  See src.cpp for the
// eight recorded keys and the semantics; see tgt_unsafe.rs for why each operand
// is named exactly once.
//
// THIS OVERRIDE IS REQUIRED and not merely for pointer syntax: BOTH parameter
// types are model-dependent.  An lvalue reference is `Ptr<T>` here and `&mut T`
// in the unsafe model (rules/string f82 is the precedent for the primitive case,
// `Ptr<i64>` against `&mut i64`), and the return is `Ptr<IStream>` rather than
// `*mut IStream`.  Inheriting the unsafe bodies would give E0308 three times per
// key.
//
// EACH `aN` IS NAMED EXACTLY ONCE, via the `let` prelude -- the same shape f8
// uses.  `Ptr` is Copy, so the local can be mentioned twice; the ARGUMENT
// EXPRESSION cannot.  The nested `with_mut_ref` is safe because the two `Ptr`s
// address different cells (a stream and a scalar); this is f8's pattern with the
// output side generalised from `Vec<u8>` to a primitive.
//
// ⛔ NO IDENTITY SHORTCUT, for the reason src.cpp gives: the caller tests the
// returned stream, so a truthy no-op silently makes every `dtGetEnv<int>`
// succeed with an indeterminate value.
fn f11(a0: Ptr<libcc2rs::IStream>, a1: Ptr<i32>) -> Ptr<libcc2rs::IStream> {
    let __s = a0;
    let __o = a1;
    __s.with_mut_ref(|__st| __o.with_mut_ref(|__v| __st.extract_i32(__v)));
    __s
}

fn f12(a0: Ptr<libcc2rs::IStream>, a1: Ptr<u32>) -> Ptr<libcc2rs::IStream> {
    let __s = a0;
    let __o = a1;
    __s.with_mut_ref(|__st| __o.with_mut_ref(|__v| __st.extract_u32(__v)));
    __s
}

fn f13(a0: Ptr<libcc2rs::IStream>, a1: Ptr<i64>) -> Ptr<libcc2rs::IStream> {
    let __s = a0;
    let __o = a1;
    __s.with_mut_ref(|__st| __o.with_mut_ref(|__v| __st.extract_i64(__v)));
    __s
}

fn f14(a0: Ptr<libcc2rs::IStream>, a1: Ptr<u64>) -> Ptr<libcc2rs::IStream> {
    let __s = a0;
    let __o = a1;
    __s.with_mut_ref(|__st| __o.with_mut_ref(|__v| __st.extract_u64(__v)));
    __s
}

// `long long` / `unsigned long long`: distinct C++ overloads, identical Rust
// width on LP64.
fn f15(a0: Ptr<libcc2rs::IStream>, a1: Ptr<i64>) -> Ptr<libcc2rs::IStream> {
    let __s = a0;
    let __o = a1;
    __s.with_mut_ref(|__st| __o.with_mut_ref(|__v| __st.extract_i64(__v)));
    __s
}

fn f16(a0: Ptr<libcc2rs::IStream>, a1: Ptr<u64>) -> Ptr<libcc2rs::IStream> {
    let __s = a0;
    let __o = a1;
    __s.with_mut_ref(|__st| __o.with_mut_ref(|__v| __st.extract_u64(__v)));
    __s
}

// ⛔ `extract_f32`, NOT `extract_f64` narrowed: `1e40 as f32` saturates to
// infinity silently where `num_get` sets failbit.  See tgt_unsafe.rs f17.
fn f17(a0: Ptr<libcc2rs::IStream>, a1: Ptr<f32>) -> Ptr<libcc2rs::IStream> {
    let __s = a0;
    let __o = a1;
    __s.with_mut_ref(|__st| __o.with_mut_ref(|__v| __st.extract_f32(__v)));
    __s
}

fn f18(a0: Ptr<libcc2rs::IStream>, a1: Ptr<f64>) -> Ptr<libcc2rs::IStream> {
    let __s = a0;
    let __o = a1;
    __s.with_mut_ref(|__st| __o.with_mut_ref(|__v| __st.extract_f64(__v)));
    __s
}

// f100 -- the free `operator>>(std::istream &, char &)`.  See src.cpp for the
// recorded key and the `DT_CHECK` caller that asserts the extraction FAILS, and
// tgt_unsafe.rs for why the `__stored` guard is required and is NOT LWG 2176.
//
// THIS OVERRIDE IS REQUIRED and not merely for pointer syntax: BOTH parameter
// types are model-dependent.  `char` is `u8` here and `libc::c_char` in the
// unsafe model (rules/string f9/f21 are the precedent), and an lvalue reference
// is `Ptr<T>` here against `&mut T` there.  Inheriting the unsafe body would give
// E0308 twice over.
//
// `Ptr` is a VALUE, so each `aN` is named exactly once through the `let` prelude.
// The staging byte is still needed even though `char` is already `u8` here,
// because the write-back must be CONDITIONAL.
fn f100(a0: Ptr<libcc2rs::IStream>, a1: Ptr<u8>) -> Ptr<libcc2rs::IStream> {
    let __s = a0;
    let __o = a1;
    let mut __c: u8 = 0;
    let __stored = __s.with_mut_ref(|__st| __st.extract_char_reporting(&mut __c).1);
    if __stored {
        __o.with_mut_ref(|__v| *__v = __c);
    }
    __s
}

// ⚠️ THE NOTE THAT USED TO SIT HERE SAID "THERE IS DELIBERATELY NO
// f101/f102/f103/f104 IN THIS FILE EITHER".  THAT IS NO LONGER TRUE and the note
// is removed rather than left to mislead a grepper: all four are landed, directly
// below, in both models.  The constraint the old note carried is still live and is
// restated at the new block: this module HAS a tgt_refcount.rs, so it must carry
// EVERY key in ir_src.json or the converter ABORTS AT LOAD TIME in the refcount
// model and poisons every translation (the f5/f6 incident above).

// ============================================================================
// t6 / f101-f104 -- `in >> std::hex`.  See src.cpp for the recorded key strings,
// the measurement that overturned g2894's refusal, and why `std::ios_base` is a
// `u32`.  ⛔ DO NOT COPY THE MANIPULATOR PARAMETER TYPE FROM tgt_unsafe.rs: the two
// models are MEASURABLY different at this position and the reason is recorded at
// f101 below.
//
// t6 is model-independent (a bare u32 formatting-state word), so it is RESTATED
// byte-identically here rather than inherited -- `ir_refcount.json` must carry
// every key `ir_src.json` does or the converter aborts at LOAD TIME, which is the
// incident recorded at f5/f6 above.
fn t6() -> u32 {
    libcc2rs::IOS_BASEFIELD_DEC
}

// f101 -- the MEMBER `operator>>(std::ios_base &(*)(std::ios_base &))`.
// A `Ptr` is a VALUE here, so the `let` prelude is the idiom (f8/f11/f100): each
// `aN` is bound once and the cell is reached through `with_mut_ref`.  Unlike the
// unsafe arm there is no raw pointer to hand back -- the stream `Ptr` itself is
// the result, so the manipulator's effect and the return are separate statements.
//
// ⚠️⚠️ `a1` IS A `fn` ITEM, AND ITS TYPE IS **NOT** THE UNSAFE ARM'S.  MEASURED
// from the emitted text: on this arm the converter passes the operand with NO
// cast at all --
//     let __m = libcc2rs::hex_refcount;
// -- whereas the unsafe arm emits `(libcc2rs::hex_unsafe as unsafe fn(*mut u32)
// -> *mut u32)`.  So there is no `Ptr<u32>` in the emitted text to match and the
// plain state-transformer shape is what type-checks here.  It is `Copy`, so
// naming it inside the `with_mut_ref` closure is sound.
fn f101(a0: Ptr<libcc2rs::IStream>, a1: fn(u32) -> u32) -> Ptr<libcc2rs::IStream> {
    let __s = a0;
    let __m = a1;
    __s.with_mut_ref(|__st| __st.set_basefield_via_value(__m));
    __s
}

// f102-f104 -- `std::hex` / `std::dec` / `std::oct`.  Model-independent apart from
// the name the converter asks for (`MapFunctionName` appends the model suffix
// unconditionally), which is exactly why libcc2rs defines BOTH twins.
fn f102(a0: u32) -> u32 {
    libcc2rs::hex_refcount(a0)
}

fn f103(a0: u32) -> u32 {
    libcc2rs::dec_refcount(a0)
}

fn f104(a0: u32) -> u32 {
    libcc2rs::oct_refcount(a0)
}
