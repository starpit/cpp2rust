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

// ============================================================================
// f100 -- THE FREE `operator>>` FOR `char`.  Recorded key, verbatim from the
// converter's own refusal on the gate TU (pin a0b0a707, pin/ir.v45, 98 modules):
//     LLVM ERROR: unsupported CXXOperatorCallExpr: >> on (std::istream, char)
//     rule key: std::istream & operator shr(std::istream &, char &)
//     at /home/agent/work/repos/dt_src/sys-arch-spec/progir/regvisitor.cpp:285:5
//
// ⭐ IT IS A FREE FUNCTION, NOT A MEMBER -- TWO parameters, the stream among
// them -- which is f8's shape and NOT f11-f18's.  libcxx declares
//     template<class C, class T> basic_istream<C,T>& operator>>(basic_istream<C,T>&, C&);
// as a free template for the CHARACTER types while the arithmetic extractions are
// members, so `iss >> some_char` resolves to the free overload.  That is why this
// key must be written with a leading `std::istream &` parameter: a member-shaped
// key (`operator shr(char &)`) would be a DIFFERENT SPELLING and would never be
// asked for.  The `operator>>(a0, a1)` FUNCTION-call form below is what steers
// overload resolution to the free overload rather than to a member.
//
// ⛔⛔ THE EXTRACTION'S RESULT IS THE POINT AND THE CHARACTER IS NOT -- read the
// caller, because a body that "reads a char" and returns a truthy stream is the
// silent-miscompile trade:
//     // sys-arch-spec/progir/regvisitor.cpp:283-286
//     bool success = static_cast<bool>(iss >> regnum);
//     char remaining_char;
//     DT_CHECK(success && regnum >= 0 &&
//              !static_cast<bool>(iss >> remaining_char));
// The code asserts the extraction FAILS: it is checking that the register name
// held no trailing garbage after the number.  So the key is correct only if
//   (a) an exhausted stream sets failbit (the sentry does: eofbit AND failbit,
//       which is the same mechanism that terminates `while (in >> x)`), and
//   (b) `static_cast<bool>` observes it (`IStream::to_bool()` is `!fail()`), and
//   (c) a FAILED read leaves `remaining_char` UNTOUCHED.
// (c) is not LWG 2176: `operator>>(char&)` is not a `num_get` extraction, so the
// zero-on-failure rule does not apply to it and the argument must be left alone.
// `IStream::extract_char_reporting` reports (a) so the rule body can honour (c).
// An identity body would make every `DT_CHECK` in that function pass regardless
// of the input -- an assertion that cannot fire is exactly as loud as no
// assertion, and nothing short of a differential run would see it.
// ============================================================================
std::istream &f100(std::istream &a0, char &a1) { return operator>>(a0, a1); }

// ============================================================================
// t6 / f101-f104 -- `in >> std::hex`, THE INPUT FORMATTING-STATE FAMILY.
// Row g3090; the `std::ios_base` TYPE KEY is row g2894.
//
// The four keys below are the ones `d51eb9da` deliberately left out, and the
// block they replace refused them for ONE stated reason: "there is no type rule
// for `std::ios_base`, so the parameter type `std::ios_base &(*)(std::ios_base &)`
// cannot be spelled here."  THAT REFUSAL DID NOT SURVIVE CONTACT.  Measured, in
// this module, with the pinned preprocessor (18cd96ef): the type key records, and
// the fn-pointer parameter ROUND-TRIPS -- the recorded key reads back out of
// ir_src.json as
//     "f101": "std::istream & operator shr(std::ios_base &(*)(std::ios_base &))"
// which is the census string CHARACTER FOR CHARACTER (verif/g3090/).
//
// RECORDED KEYS, verbatim from the 295-TU refcount census
// (verif/g3085CTL.tus.json, binary md5 086c2ff4363bfda3647a8eb5b5d6500f):
//     >> on (std::istream, std::ios_base &(*)(std::ios_base &))
//     rule key: std::istream & operator shr(std::ios_base &(*)(std::ios_base &))
// at dcg/tools/mda/memDumpAnalyzer.h:244:18 (2 TUs) and
// util/sendefs/sendefs.cpp:432:10 (1 TU).  All three sites are `>> std::hex >>`
// into an integer whose value is then used arithmetically.
//
// ⭐ IT IS THE **MEMBER** FORM: ONE parameter, no `std::istream &` in the key.
// libcxx declares the manipulator overload as a member of `basic_istream`
// (`basic_istream& operator>>(ios_base& (*__pf)(ios_base&))`), so this is
// f11-f18's shape and NOT f8/f100's free two-parameter shape.  `o.operator>>(m)`
// is written in explicit member-call form for the same reason f11 is.
//
// 1. WHAT THE MODEL NOW HAS, AND IT IS THE WHOLE OF IT.  `libcc2rs::IStream`
//    carries a `basefield` (10/16/8, default 10) that every `extract_*` consumes
//    through `from_str_radix`; `shr_ios_manip` applies a manipulator to it and
//    hands the stream back, so `inFile >> std::hex >> lineno` lowers to
//        IStream::shr_i64(&mut *inFile.shr_ios_manip(libcc2rs::hex_unsafe), &mut lineno)
//    with each operand named exactly once.  Verified against EXECUTED C++
//    (g3084probe/hexrow.cpp prints hex[31,32,156] / decrestore[16,10] / oct[15] /
//    dec[42,-7] / hexfail[0,0]); the Rust side reproduces all five in
//    `ios_manipulators_thread_the_base_through_and_match_cpp`.
//
// 2. WHY `std::ios_base` IS `u32` AND WHY THAT IS FAITHFUL, NOT A FUDGE.  g2894's
//    stored refusal reads "no honest Rust model for abstract never-instantiated
//    `std::ios_base` itself".  The abstractness is real and IRRELEVANT: nothing in
//    this corpus instantiates an `ios_base` OBJECT.  What the corpus needs is a
//    type that can be the pointee of a function-pointer parameter, and the thing
//    `ios_base` contributes to these sites is exactly one word of formatting
//    state.  t6 is that word, and `libcc2rs::IOS_BASEFIELD_*` are its values.
//    ⛔⛔ AND THE SCOPE IS A LOAD-BEARING LIMIT, NOT A CAVEAT: t6 models the
//    BASEFIELD only, not `adjustfield`/`floatfield`/`fill`/`width`.  So
//    `std::left`/`std::right`/`std::setw`/`std::setfill`/`std::setprecision`/
//    `std::fixed`/`std::boolalpha` MUST NOT be keyed onto it -- each would
//    silently REPLACE the radix while modelling none of its own effect, which is
//    a worse trade than the abort they produce today.  They stay out; rows
//    g858/g861, and section 4 below.
//
// 3. HOW `std::hex` REACHES RUST, AND WHY f102-f104 HAVE BODIES NOBODY INLINES.
//    The corpus never CALLS `std::hex`; it only NAMES it, as the operand of `>>`.
//    An inlined rule body has no address, so for a NAMED keyed system function
//    the converter emits `libcc2rs::<name>_<model>` instead
//    (`Mapper::MapFunctionName`, cpp2rust/converter/mapper.cpp:2660).  d51eb9da
//    measured exactly that -- it saw `libcc2rs::hex_refcount` and recorded it as
//    "an undefined name (E0425)".  ⭐ IT IS ONLY UNDEFINED IF NOBODY DEFINES IT:
//    `libcc2rs::{hex,dec,oct}_{unsafe,refcount}` are now real `fn` items in
//    libcc2rs/src/istream.rs.  `libcc2rs/src/cctype.rs` is the standing precedent
//    for this exact two-route shape (`::tolower` named vs called), so this is a
//    convention already in the tree and not a new mechanism.
//    ⚠️ THEREFORE f102-f104's ROLE IS TO MAKE `exprsContain(ToString(decl))` TRUE
//    (mapper.cpp:2661) so MapFunctionName takes the `libcc2rs::` branch instead of
//    its mangled-name fallback.  Their bodies are correct and kept in step with
//    the libcc2rs items, but no corpus site inlines one; the arity-1 spelling is
//    libcxx's own (`std::ios_base & std::hex(std::ios_base &)`).
//
// 4. WHAT THIS DOES **NOT** CLOSE, AND THE SUCCESSOR GATE IT LEAVES STANDING.
//    `dsc/sdsc-perfmodel/perfmodel.cpp:435` is `oss << std::left << std::setw(w)
//    << std::setfill(' ') << t` -- the OUTPUT half.  Its first abort today is this
//    row's missing `std::ios_base` type -- BUT ONLY IN THE REFCOUNT MODEL, and
//    that qualifier is the whole finding.  ⛔⛔ THE OUTPUT MANIPULATORS DO NOT
//    ABORT AT ALL IN THE UNSAFE MODEL, AND THAT IS PRE-EXISTING, NOT CAUSED HERE.
//    Measured on the same 99-module tree, BOTH legs, one binary: perfmodel.cpp
//    translates `complete rc=0` at 45,995 lines in the unsafe model BEFORE this
//    row and AFTER it, and the two emitted files are BYTE-IDENTICAL (`diff -q`
//    silent).  What line 435 -- `oss << std::left << std::setw(width) <<
//    std::setfill(' ') << t` -- becomes is, verbatim:
//        write!((*oss), "{:}{:}{:}", Some(left_166), (*width),
//               (unsafe { Cpp2RustUnmappedFn_setfill_167((' ' as libc::c_char),) }),);
//    Three different failure modes in one statement, and only one of them is
//    detectable by the census: `std::left` -> the undefined name `left_166`
//    (MapFunctionName's fallback ASSERTS, and an assert is a NO-OP under -DNDEBUG);
//    `std::setfill(' ')` -> the g3058 `Cpp2RustUnmappedFn_` sentinel, which IS a
//    detectable marker; and ⭐ `std::setw(width)` -> PLAIN `(*width)`, i.e. the
//    manipulator silently became DATA -- the width integer is printed as a field.
//    So if a later slot keys `left` and `setfill` without keying `setw`, the TU
//    compiles and prints the width in the middle of the table.  ⛔ DO NOT TREAT
//    "the output half aborts loudly" AS TRUE: it is true in refcount only.
//    In the REFCOUNT model t6 does move perfmodel.cpp, and the successor gate was
//    also NOT the predicted one.  Predicted: a `<<` operator-call abort on
//    `(std::ostream, std::ios_base &(*)(std::ios_base &))`.  Measured, verbatim:
//        LLVM ERROR: unsupported structured binding / DecompositionDecl with 3
//        bindings [sdscName, opCategory, idealCycles] of type
//        `const std::tuple<std::string, std::string, long> &` is not implemented,
//        reached while converting `PerfModel::exportS...`
//    -- an unrelated decomposition gate EARLIER in emission order, so the `<<`
//    sites are not reached on this TU at all and the `<<` abort is UNOBSERVED
//    here.  The refcount arm therefore stays loud; the unsafe arm was never loud.
//    Closing the output half needs a NEW `OStream`
//    model with sticky format state (`libcc2rs`'s ostream is `std::fs::File`,
//    t1/t2/t3, which has no format state at all), which is a separate row.
// ============================================================================
using t6 = std::ios_base;

std::istream &f101(std::istream &o, std::ios_base &(*m)(std::ios_base &)) {
  return o.operator>>(m);
}

std::ios_base &f102(std::ios_base &a0) { return std::hex(a0); }

std::ios_base &f103(std::ios_base &a0) { return std::dec(a0); }

std::ios_base &f104(std::ios_base &a0) { return std::oct(a0); }

// ============================================================================
// ⛔ STILL DELIBERATELY NOT KEYED, AND THIS HALF OF THE OLD REFUSAL STANDS:
// THE **OUTPUT** MANIPULATORS.  `oss << std::left`, `<< std::setw(n)`,
// `<< std::setfill(c)`, and their `std::setprecision`/`std::fixed`/
// `std::boolalpha` siblings (rows g858/g861).
//
// WHAT IS ABSENT FROM THE MODEL: an ostream with format state.  This module's
// ostream is `std::fs::File` (t1/t2/t3, f1-f4) -- a File has no basefield, no
// width, no fill and no adjustfield, exactly as `IStream` had no read cursor
// before it was written.  So this is a MODEL gap, not a missing key, and t6 above
// does not help: t6 is the BASEFIELD word and `std::left`/`std::setw` live in
// `adjustfield`/`width`, which nothing in either model holds.
//
// FAILURE MODE OF THE ALTERNATIVE: an identity `std::setw` drops the padding and
// an identity `std::boolalpha` prints `1` where C++ prints `true` -- a silently
// mis-shaped column in a performance report, rc=0, no placeholder, no diagnostic
// at any stage of this harness.  That is the forbidden trade, and it is why the
// output half is left aborting.
//
// DISCRIMINATOR -- a landed key where the same shape IS correct: f101 above.
// Same `ios_base` manipulator shape, same fn-pointer operand, and it lands
// because the ONLY state its three manipulators touch is the basefield, and
// `IStream` holds a real basefield.  So the blocker is the OSTREAM MODEL and not
// manipulators being unkeyable.
//
// NAMED UNBLOCKER, IN THE RIGHT LAYER: a new `libcc2rs::OStream` carrying sticky
// `basefield`/`adjustfield`/`width`/`fill`/`precision`/`boolalpha`, plus
// re-pointing t1/t2/t3 and f1-f4 off `std::fs::File` onto it.  Sized in
// verif/g3090/ostream-surface.txt: 4 manipulator families over 8 spellings,
// measured site and TU counts.
//
// ⛔⛔ AND IT IS **NOT** UNIFORMLY LOUD -- THE OPPOSITE OF WHAT THIS FILE USED TO
// CLAIM, AND THE CORRECTION IS MEASURED.  Detail in section 4 above; the summary a
// later slot needs is: in the UNSAFE model `perfmodel.cpp` does not abort at all,
// before OR after this row (byte-identical 45,995-line output in both legs), and
// `<< std::setw(width)` lowers to a bare `(*width)` argument -- the manipulator
// becomes DATA.  Only `std::left` (undefined name `left_166`) and
// `std::setfill(' ')` (the g3058 `Cpp2RustUnmappedFn_` sentinel) stop it
// compiling, so the loudness the g858/g861 refusal was resting on is an artifact
// of two OTHER unkeyed names, not of a gate.  In the REFCOUNT model t6 does move
// perfmodel.cpp, to an unrelated EARLIER decomposition gate, so the `<<` abort is
// UNOBSERVED there too.  ⚠️ A LATER SLOT MUST NOT READ "the output half aborts on
// <<" OFF THIS FILE.  The `>>` sibling abort WAS seen, in isolation, in
// verif/g3090/hexrow.*.tlog before f101 landed.
//
// ⛔ WHAT THIS ROW NEWLY EXPOSES, STATED PLAINLY BECAUSE IT IS A FALSE-CLOSURE
// RISK IT CREATES: clearing the `>>` gate lets `dip/dip.cpp` and
// `dip/dip_standalone1.cpp` translate PAST it, and what lies behind it on those
// two TUs is more of the SAME silent class, in `std::ios`/`std::ios_base` roles
// this row does not key --
//     (*inpFile).seekg_i64_i32(0_i64, std_ios_base_seekdir_end)
//     std_ios_base::sync_with_stdio(Some(false))
// -- `seekg` itself, the `seekdir` enumerators and the `sync_with_stdio` static
// are ALL unkeyed and all lower to undefined names at rc=0.  They were previously
// unreachable behind the `>>` abort.  So `dip.cpp`/`dip_standalone1.cpp` go from
// "aborts loudly" to "rc=0 and does not compile (E0425)": still detectable, but
// ONLY at rustc, never by a translate-time census.  89 nested `std::ios::`/
// `std::ios_base::` sites across the corpus (37 of them `sync_with_stdio`) are in
// that class.  Keying them needs a get-position model on the ifstream mapping,
// which `rules/istringstream` already refuses by name -- a separate row.
// ============================================================================

