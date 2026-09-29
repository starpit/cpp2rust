// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <iostream>
#include <string>

using t1 = std::ostream;
using t2 = std::ostream &;
using t3 = std::ostream *;

std::ostream &f1() { return std::cout; }

std::ostream &f2() { return std::cerr; }

std::ostream *f3() { return &std::cout; }

std::ostream *f4() { return &std::cerr; }

// f5/f6 -- `std::ios_base::in` and `std::ios_base::out`, STATIC DATA MEMBERS of
// std::ios_base (libcxx/ios:287-288, `static const openmode in = 0x08;`,
// `out = 0x10;`, openmode = unsigned int).  MEASURED, not guessed.
//
// WHY THEY ARE HERE AND WHY THEY MATTER FAR BEYOND this module.  The 35-TU
// stringstream row (rules/basic_stringstream f3, the
// `stringstream(const std::string &, unsigned int)` ctor) translated rc=0 and
// then failed to compile in BOTH models with
//     cannot find value `in_0` in this scope
//     cannot find value `out_1` in this scope
// because the DEFAULTED openmode argument `std::ios_base::in |
// std::ios_base::out` is materialised at the CONSTRUCT SITE and lowered by
// Converter::ConvertDeclRefExpr (converter.cpp:3977-3982) through the
// GLOBAL-VARIABLE path -- IsGlobalVar (converter_lib.cpp:80) is
// `isFileVarDecl() || isStaticLocal()` and a class-static data member IS a file
// var decl -- so it emits
//     (*std::cell::LazyCell::force_mut(&mut *&raw mut in_0))
// against a `pub static mut in_0` DECLARATION THAT IS NEVER EMITTED, because the
// declaration is only written when the VarDecl itself is traversed and a
// system-header decl never is.  (Measured against a PORTED static member: a
// `struct K { static const unsigned int A; }` does get
// `pub static mut A_0: std::cell::LazyCell<u32> = ...` plus a force in
// __cpp2rust_init_globals.  So the lowering is complete for project statics and
// dangling for system ones.)
//
// THE FIX IS A RULE, NOT A CONVERTER CHANGE, and the shape is exactly f1/f2's:
// ConvertDeclRefExpr consults ShouldReplaceWithMappedBody/GetMappedAsString
// BEFORE the IsGlobalVar branch, and the -verbose line is
//     search expr std::ios_base::in, result: None
// -- i.e. the search key is the plain qualified name, with no signature and no
// parentheses, the same shape `std::cout` records as (f1).  The rule
// preprocessor's `declref` matcher (cpp_rule_preprocessor.cpp:982-984) binds
// `declRefExpr(to(decl(unless(parmVarDecl()))))` and records
// Mapper::ToString(VarDecl), which is printQualifiedName -- so a class-static
// data member records identically to a namespace-scope global.  There was never
// a mechanism gap; there was only no user.
//
// NO rule body can dodge this by ignoring the parameter: the default expression
// is converted at the construct site and textually substituted into the inlined
// body (`[VisitCXXConstructExpr:4743] {let _ = ( ( (*...in_0)) | ((*...out_1)) );`),
// so basic_stringstream f3's `let _ = a1;` receives the already-emitted
// reference.  Keying the two members is the only available fix.

unsigned int f5() { return std::ios_base::in; }

unsigned int f6() { return std::ios_base::out; }

// f7 -- `std::ios_base::binary`, the third member of the same family as f5/f6 and
// dangling for exactly the same reason: libcxx/ios:286 is
// `static const openmode binary = 0x04;`, a DECLARATION-ONLY in-class static, so no
// `pub static mut binary_<n>` is ever emitted while the use site still emits
//     (*std::cell::LazyCell::force_mut(&mut *&raw mut binary_5))
// -- an E0425 with rc=0, no placeholder token and no diagnostic.  2 of 78 bucket-A
// census TUs carry it (dbo EmitSpyreCode.cpp, util sendefs.cpp).
//
// SEARCHED SPELLING, read back from the converter's -verbose log against ir.v13:
//     search expr std::ios_base::binary, result:
//     None
// i.e. the plain qualified name, identical in shape to f5/f6.
//
// VALUE IS READ OFF THE HEADER THE CONVERTER PARSES WITH, not remembered:
// toolchain/libcxx/ios:284-289 is app=0x01 ate=0x02 binary=0x04 in=0x08 out=0x10
// trunc=0x20, which is self-consistent with f5=0x08 and f6=0x10 already committed
// here.  openmode is `unsigned int` -> u32, same as f5/f6.
unsigned int f7() { return std::ios_base::binary; }

// t4 -- `std::istream`, i.e. `std::basic_istream<char, std::char_traits<char>>`.
// Rows g2818/g2826/g2827 (DataStructDims::read, SuperDsc::importJson,
// SuperDsc::importPcfgJson) each abort with `system type has no rule:
// std::basic_istream<char, std::char_traits<char>>` while the SEARCHED spelling is the
// typedef `std::istream` -- exactly the shape t1 already proves for the output side
// (ir_src.json t1 reads back "std::ostream", not "std::basic_ostream<char, ...>"),
// so the typedef-preferring type printer resolves this one DOWN to `std::istream`
// as required.  Modelled as std::fs::File, the same counterpart t1 uses for
// std::ostream: this is a TYPE key only, so every actual operation on the stream
// (operator>>, getline, read) still has no rule and fails LOUDLY with a placeholder
// token rather than being silently invented here.
using t4 = std::istream;

// ============================================================================
// f8 -- THE FREE `operator>>` FOR std::string, i.e. the whole reason t4 was
// re-modelled.  Recorded key (the converter searches for exactly this):
//     std::istream & operator shr(std::istream &, std::string &)
//
// NOTE HOW THE KEY IS HARVESTED.  The declared parameters below are only how
// OVERLOAD RESOLUTION is steered; the RECORDED signature is the resolved
// library function's own, which for libcxx is
//     template<class C, class T, class A>
//     basic_istream<C,T>& operator>>(basic_istream<C,T>&, basic_string<C,T,A>&)
// printed with the typedef-preferring printer as `std::istream &` /
// `std::string &`.  rules/cstddef/src.cpp:10 is the precedent: it writes
// `operator>>(a0, a1)` with `const std::byte &` parameters and the recorded key
// comes out `std::byte operator shr(std::byte, unsigned int)`, i.e. the
// LIBRARY's signature, not this file's.  `operator>>(...)` is called in its
// FUNCTION form so the free overload -- not the basic_istream member -- is the
// one resolved.
//
// THE MEMBER `operator>>` OVERLOADS (int&, double&, ... ) ARE DELIBERATELY NOT
// HERE, and it is a refusal with a measured cause rather than an omission: a
// member call's receiver arrives through a DerivedToBase cast that the converter
// emits as `(<recv> as Cpp2RustUnmapped_std_ios)` (dip/dip.cpp.rs:5478), a
// non-primitive cast that no rule body can remove.  The free string overload has
// no receiver -- both operands are ARGUMENTS, and an argument-position
// DerivedToBase is emitted with NO cast (dip/dip.cpp.rs:6414 passes a
// std::stringstream to a `std::istream &` parameter as plain `&mut s_stream`).
// That measured asymmetry is the whole reason this key lands and those do not.
// ============================================================================
std::istream &f8(std::istream &a0, std::string &a1) { return operator>>(a0, a1); }

// ============================================================================
// f9 / f10 -- `std::getline`, THE FREE FUNCTION TEMPLATE.  Recorded keys, read
// back CHARACTER FOR CHARACTER from a -verbose leg that EXITED 0 (562,123 lines,
// witness dip/dip.cpp, pin 6b2fbbb5dc85683916ba664db864847c, my own tree cloned
// from ir.v31 with iostream+string regenerated to HEAD):
//     search expr std::istream & std::getline(std::istream &, std::string &, char), result:
//     None                                                                (13 asks)
//     search expr std::istream & std::getline(std::istream &, std::string &), result:
//     None                                                                 (5 asks)
//
// ⭐⭐ THIS CORRECTS A RECORDED DIAGNOSIS THAT WAS WRONG, and the correction is
// the whole point of the row.  The brief for this slot stated that `std::getline`
// "IS NOT RULE-SEARCHED AT ALL, which is a different and worse class than an
// unmapped member", and that a key written for it "would be DEAD -- the converter
// never asks for it".  THAT IS FALSE AS MEASURED.  The converter asks 18 times in
// dip/dip.cpp alone and gets `None` both times, i.e. this is an ORDINARY MISSING
// FREE-FUNCTION KEY, the same class as f8 above, and NOT the fabricated-name
// class of `<Recv>::new_<N>`.
//
// WHERE THE `_99` TELL CAME FROM, since it is what motivated the wrong diagnosis:
// the emitted call really is `getline_99(&mut s_stream, &mut substr, ',')` with
// ZERO `fn getline_99` in the file -- but that is simply what an UNRESOLVED free
// function looks like on the way out.  The verbose log shows the sequence
// explicitly, in this order:
//     search expr std::istream & std::getline(std::istream &, std::string &, char), result:
//     None
//     [VisitDeclRefExpr:4806] getline_99
//     [VisitCallExpr:3381] ( unsafe { getline_99 ( & mut s_stream , ... ) } )
// i.e. the ASK COMES FIRST and the per-decl-counter name is the FALLBACK the
// DeclRefExpr visitor emits AFTER the search misses.  ⭐ So a fabricated
// `name_<N>` with no definition does NOT by itself mean "never searched" --
// the DeclRefExpr fallback is shared between "never asked" and "asked and
// missed", and only `grep -A1` on the search line distinguishes them.  That is
// the reusable discriminator from this row.
//
// WHY THE KEY IS WRITABLE, the same measured asymmetry that lands f8: `getline`
// is a FREE function, so the `std::stringstream`/`std::ifstream` -> `std::istream &`
// conversion happens in ARGUMENT position, where the converter emits a
// DerivedToBase with NO cast at all.  The verbose log confirms it at the site:
// `ImplicitCastExpr ... <DerivedToBase (basic_iostream -> basic_istream)>` in the
// AST, and plain `&mut s_stream` in the emitted text.  A member-call receiver
// would instead get `(recv as Cpp2RustUnmapped_std_ios)`, which is why `eof()`
// and friends stay refused.
//
// TWO KEYS, NOT ONE, because the converter issues TWO DISTINCT SEARCHES: the
// three-argument form with an explicit delimiter and the two-argument form,
// whose delimiter is `'\n'` per [string.io].  A single rule cannot answer both
// -- the recorded signatures differ in arity -- so f10 supplies the newline
// default explicitly rather than defaulting a parameter (a defaulted template
// argument is a recorded rule-authoring trap).
//
// ⛔ WHAT WOULD BE SILENTLY WRONG HERE: `std::getline` differs from `operator>>`
// in TWO ways that a copy of f8 would get wrong.  It does NOT skip leading
// whitespace, and AN EMPTY FIELD IS A SUCCESS -- `"a,,b"` split on `','` must
// yield an empty middle token, i.e. the caller's string must be CLEARED, whereas
// `>>` can never produce an empty token at all.  So the write-back guard cannot
// be "did we read any bytes"; it must be the stream's own sentry result. That is
// what `IStream::getline_reporting` returns, and it is why this row added that
// entry point instead of reusing `extract_token_reporting`.
// ============================================================================
std::istream &f9(std::istream &a0, std::string &a1, char a2) {
  return std::getline(a0, a1, a2);
}
std::istream &f10(std::istream &a0, std::string &a1) {
  return std::getline(a0, a1);
}

// t5 -- `std::ios_base::seekdir` (row g478, 2 TUs).  libcxx/ios:295 is
// `enum seekdir { beg, cur, end };` -- an UNSCOPED enum with no fixed underlying
// type, so it promotes to `int` -> i32.  Note that g478 and g2894 report the SAME
// declaration (`std::ios_base`) but DIFFERENT searched spellings: g478 searches
// `std::ios_base::seekdir`, g2894 searches `std::ios_base` itself.  This key answers
// g478 only; g2894 is deliberately NOT keyed (see the report) because ios_base itself
// is an abstract, never-instantiated base and no honest Rust representation for it
// was established within this slot's budget.
using t5 = std::ios_base::seekdir;

// ============================================================================
// DELIBERATELY NOT KEYED: the STREAM MANIPULATORS.  Rows g858 and g861,
// searched-as (character-for-character, from the queue detail and samples/):
//     std::ios_base & std::boolalpha(std::ios_base &)     (g858, 1 TU)
//     std::ios_base & std::left(std::ios_base &)          (g861, 1 TU)
// libcxx/ios:754 and :829 -- the bodies are
//     inline ios_base& boolalpha(ios_base& __str) { __str.setf(ios_base::boolalpha); return __str; }
//     inline ios_base& left(ios_base& __str) { __str.setf(ios_base::left, ios_base::adjustfield); return __str; }
// i.e. each one MUTATES STICKY FORMATTING STATE on the stream object and hands
// the same object back.  Neither computes anything; the whole observable effect
// is the flag, which is read LATER, by a DIFFERENT insertion, possibly in a
// different function.
//
// WHY A RULE HERE WOULD BE SILENTLY WRONG -- this is the point, not the
// mechanism.  This module models std::ostream (t1/t2/t3) as `std::fs::File`, a
// bare fd handle with NO formatting state whatsoever, and Rust's `{}` has no
// stream-sticky state either.  So the only rule body writable against the
// committed model is the IDENTITY (`return the stream`), which DROPS the flag.
// That does not fail -- it COMPILES, RUNS, and PRINTS DIFFERENT BYTES:
//   * `std::left`: with `os.width(n)`/`std::setw(n)` in effect, C++ pads on the
//     RIGHT (`|abc  |`); dropping the flag leaves the default adjustfield, which
//     pads on the LEFT (`|  abc|`).
//   * `std::boolalpha`: C++ writes `true`/`false` for a bool with the flag and
//     `1`/`0` without it.
// THE OBSERVER IS THE BYTE STREAM WRITTEN TO THE fd: a differential test that
// diffs the C++ program's stdout against the translated program's stdout sees
// it, and NOTHING EARLIER DOES -- not rc, not rustc, not no-placeholders.sh.
// That is the silent-wrongness class, so these stay out and keep failing LOUDLY
// as `system function has no rule`.
//
// ⭐ boolalpha is NOT the "free no-op" it looks like.  The tempting argument is
// that Rust's `{}` on a `bool` prints `true`/`false` unconditionally, i.e.
// already what boolalpha asks for, so the key could be an identity honestly.
// MEASURED AGAINST THIS TREE, THAT IS FALSE: the ostream-family bool insertion
// path here does not reach a Rust `bool` at all.  raw_ostream/src.cpp:78-79
// records that `bool` needs no rule of its own because "bool -> int is an
// integral promotion ... so C++ always picks operator<<(int) for it" -- so a
// bool arrives at the keyed insertion ALREADY WIDENED TO int and prints `1`/`0`.
// Against that path boolalpha is a REAL CHANGE (1/0 -> true/false), not a no-op.
// And even where a `{}`-on-bool path existed, the identity would be right only
// by accident and would be WRONG in the mirror direction for `std::noboolalpha`,
// which must restore `1`/`0` and cannot, because a rule body is a pure function
// of its arguments and cannot clear state that later insertion sites read.
//
// SECOND, INDEPENDENT BLOCKER (either one alone suffices): the signature itself
// is unwritable.  Both keys take and return `std::ios_base &`, and
// `std::ios_base` HAS NO TYPE RULE anywhere in the tree -- t5 above keys only
// the nested `std::ios_base::seekdir`, and the bare `std::ios_base` of row
// g2894 is explicitly refused for want of an honest Rust representation of an
// abstract never-instantiated base.  Keying a manipulator therefore presupposes
// first inventing a formatting-state model for ios_base (flags word + width +
// precision + fill, threaded through every insertion so a later `<<` can read
// what an earlier manipulator set).  That is a model change for the whole
// ostream family, not a two-line rule, and it is the correct place to solve
// BOTH rows -- at which point the manipulators become one-line setf bodies.
// Until then: 1 TU each, refused, loud.
// ============================================================================

// ============================================================================
// f11-f18 -- THE **MEMBER** `operator>>` OVERLOADS, i.e. the numeric
// extractions.  ⭐⭐ THIS IS THE GATE ON `dxp/dxp_standalone.cpp`, the stated
// port goal.  Recorded keys (read back from ir_src.json, and character-for-
// character what the converter's own refusal printed):
//     std::istream & operator shr(int &)                    f11
//     std::istream & operator shr(unsigned int &)           f12
//     std::istream & operator shr(long &)                   f13
//     std::istream & operator shr(unsigned long &)          f14
//     std::istream & operator shr(long long &)              f15
//     std::istream & operator shr(unsigned long long &)     f16
//     std::istream & operator shr(float &)                  f17
//     std::istream & operator shr(double &)                 f18
//
// ⭐ THE KEY IS A MEMBER AND THAT IS THE WHOLE ROW.  f8 above is the FREE
// `operator>>(std::istream &, std::string &)`, TWO parameters.  These are
// `std::basic_istream::operator>>(int &)` etc. -- ONE parameter, receiver
// implicit.  `ss >> std::string` resolves to the free overload, `ss >> int` to
// the member, and a different declaring context is a DIFFERENT KEY: f8 could
// never have matched and must not be widened to try.
//
// WHY THE CLASS QUALIFICATION IS ABSENT while every other member key in the tree
// carries one (`std::map<T1, T2>::operator[]`, `llvm::APInt::operator!=`): for
// the FOUR SHIFT OPERATORS ONLY, Mapper::ToString (mapper.cpp:2853-2870) takes a
// special branch that prints `func_decl->getQualifier()` -- the WRITTEN
// nested-name-specifier, which is empty for a declaration inside its class --
// and then the rewritten name `shl`/`shr`/`shleq`/`shreq`, INSTEAD OF
// `printQualifiedName()`.  The rewrite exists so matchTemplate's bracket-depth
// tracker never sees the `>` of `>>`; dropping the class is a side effect of
// taking that branch.  rules/raw_ostream is the precedent -- its 14 member
// insertions record as `llvm::raw_ostream & operator shl(int)` and friends, with
// no class either.  ⛔ DO NOT widen the rewrite range to cover more operators:
// it changes the STORED key spelling, which a frozen IR pin holds in raw form,
// so every surviving key breaks and a full IR regen is forced.  The sibling
// `operator>=` angle-depth bug was fixed on the CONVERTER-side tracker instead
// (`MaskOperatorNameBrackets`, eb437fae) for exactly this reason.
//
// ⭐ THE PRIOR REFUSAL RECORDED AT f8 ("THE MEMBER `operator>>` OVERLOADS ARE
// DELIBERATELY NOT HERE ... a member call's receiver arrives through a
// DerivedToBase cast that the converter emits as
// `(<recv> as Cpp2RustUnmapped_std_ios)`") DOES NOT APPLY TO THESE, and the
// distinction is the DECLARING CLASS of the member, not member-ness:
//   * `eof()`/`good()`/`operator bool()` are members of `std::basic_ios`, and
//     `std::ios` HAS NO TYPE RULE in this tree (row g2894 refused it), so their
//     receiver cast names an UNMAPPED type -- hence `Cpp2RustUnmapped_std_ios`
//     and E0605.
//   * `operator>>` is a member of `std::basic_istream`, which IS `t4` above, and
//     `std::stringstream` (rules/basic_stringstream t1) is modelled as the SAME
//     `libcc2rs::IStream`.  So the DerivedToBase has a mapped target on both
//     sides.  rules/raw_ostream proves the shape end to end: `llvm::errs()`
//     returns `raw_fd_ostream &` and every insertion is a `raw_ostream` member,
//     i.e. the identical derived-receiver situation, and those keys work in both
//     models.
// The f8 comment's blanket claim was therefore too broad; it is narrowed here
// rather than deleted, because the `eof()` half of it is still true.
//
// ⛔⛔ WHAT WOULD HAVE BEEN SILENTLY WRONG -- and it is the reason this row is
// not a one-liner.  The corpus caller is `util/dtgetenv.hpp:129`:
//     std::stringstream ss(ptr);  T parsed;
//     if ((ss >> parsed) && ss.eof()) { ret = parsed; }
// which READS THE STREAM STATE.  An identity / no-op body returns a TRUTHY
// stream, so EVERY `dtGetEnv<int>` would succeed with an INDETERMINATE `parsed`
// -- code that compiles, runs, and quietly changes what every env-var-driven
// flag in the program does.  Nothing short of a differential run would see it.
// The bodies therefore go through `libcc2rs::IStream`'s sentry + failbit, and
// `libcc2rs/src/istream.rs` carries the round-trip test over the three cases a
// no-op body would pass: `"12"` -> Some(12); `"12abc"` -> None (extraction
// succeeds, `eof()` is FALSE); `"abc"` -> the stream is FALSY.
//
// ⚠️ IT IS A FAMILY, NOT ONE KEY, and `int` is only the one that aborts FIRST.
// `dtGetEnv<T>` is a template, so each `T` instantiates its own member
// `operator>>`.  Census over repos/dt_src (219 `dtGetEnv<...>` sites in 88
// files): std::string 154, bool 45, int 7, int64_t 6, double 2, unsigned long 1,
// size_t 1, long 1, float 1.  `std::string` is f8; `bool` is a FULL
// SPECIALISATION at dtgetenv.hpp:140 that switches on `ptr[0]` and never touches
// a stream at all; the rest are these keys (int64_t == long, size_t ==
// unsigned long on LP64).  f12/f15/f16 are not instantiated by the corpus today
// and are written because they are the same three lines and libcxx declares
// them.
//
// ⛔ DELIBERATELY NOT KEYED, so it keeps failing loudly:
//   * `operator>>(long double &)` -- `libcc2rs::IStream` parses through `f64`,
//     so an 80-bit target would silently lose the range `num_get` accepts.  No
//     corpus site instantiates it.
//   * `operator>>(short &)` / `unsigned short &` -- honest to write, but no
//     corpus site asks and an unused key is an unverified one.
//   * `operator>>(bool &)`, `void *&`, `std::streambuf *`, and the
//     `basic_istream(*pf)(basic_istream&)` manipulator overload -- no corpus
//     caller, and the last one needs a function-pointer-into-stream model.
//
// HOW THE KEY IS HARVESTED: the declared parameter types below only steer
// OVERLOAD RESOLUTION; the recorded signature is the resolved libcxx member's
// own, printed with the typedef-preferring printer (so `basic_istream<char,
// char_traits<char>> &` reads back as `std::istream &`, exactly as t4/f8 prove).
// `o.operator>>(v)` is written in explicit member-call form so the MEMBER, not
// the free string overload, is the one resolved.
// ============================================================================
std::istream &f11(std::istream &o, int &v) { return o.operator>>(v); }

std::istream &f12(std::istream &o, unsigned int &v) { return o.operator>>(v); }

std::istream &f13(std::istream &o, long &v) { return o.operator>>(v); }

std::istream &f14(std::istream &o, unsigned long &v) { return o.operator>>(v); }

std::istream &f15(std::istream &o, long long &v) { return o.operator>>(v); }

std::istream &f16(std::istream &o, unsigned long long &v) {
  return o.operator>>(v);
}

std::istream &f17(std::istream &o, float &v) { return o.operator>>(v); }

std::istream &f18(std::istream &o, double &v) { return o.operator>>(v); }
