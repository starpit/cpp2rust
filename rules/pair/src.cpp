// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstddef>
#include <utility>

template <typename T1, typename T2> using t1 = std::pair<T1, T2>;

template <typename T1, typename T2> T2 &f1(std::pair<T1, T2> &o) {
  return o.second;
}

template <typename T1, typename T2>
std::pair<T1, T2> f2(const std::pair<T1, T2> &a0) {
  return std::pair<T1, T2>(a0);
}

template <typename T1, typename T2>
std::pair<T1, T2> f4(const T1 &a0, const T2 &a1) {
  return std::pair<T1, T2>(a0, a1);
}

template <class T1, class T2, class T3, class T4>
std::pair<T1, T2> f5(const T3 &a0, T4 &a1) {
  return std::pair<T1, T2>(a0, a1);
}

template <class T1, class T2, class T3, class T4>
std::pair<T1, T2> f6(T3 &a0, T4 &a1) {
  return std::pair<T1, T2>(a0, a1);
}

template <typename T1, typename T2, typename T3, typename T4>
std::pair<T1, T2> f7(T3 &&a0, T4 &&a1) {
  return std::pair<T1, T2>(std::move(a0), std::move(a1));
}

template <class T1, class T2> auto f9(T1 &&a0, T2 &a1) {
  return std::make_pair(std::move(a0), a1);
}

template <class T1, class T2> auto f10(T1 &&a0, T2 &&a1) {
  return std::make_pair(std::move(a0), std::move(a1));
}

template <typename T1, typename T2> T1 &f11(std::pair<T1, T2> &a0) {
  return a0.first;
}

template <typename T1, typename T2>
std::pair<T1, T2> f12(std::pair<T1, T2> &&a0) {
  return std::pair<T1, T2>(std::move(a0));
}

template <typename T1, typename T2>
std::pair<T1, T2> &f13(std::pair<T1, T2> &dst, const std::pair<T1, T2> &src) {
  return dst.operator=(src);
}

template <typename T1, typename T2>
std::pair<T1, T2> &f14(std::pair<T1, T2> &dst, std::pair<T1, T2> &&src) {
  return dst.operator=(std::move(src));
}

// g419 -- REFUTED AND SUPERSEDED BY f17/f18 BELOW (measured 2026-09-28).  Both
// operators ARE keyed now and probe/paireq/p.cpp MATCHes in BOTH models.  The
// note below is kept only as a record of what was wrong with it.  What it got
// right: the rule side really does record `bool std::operator==(...)` WITHOUT
// the `__1` component (rules/deque's identically-written f15/f16 record WITH it).
// What it got wrong: it inferred from that asymmetry that the key was DEAD.  It
// is not -- the CONVERTER prints the pair operators the same `__1`-less way, so
// the two sides AGREE and the key matches.  The quoted `rule key: bool
// std::__1::operator!=(...)` abort text came from a different converter than the
// one it was compared against.  Lesson: a key spelling that merely LOOKS wrong
// next to a sibling module is not evidence of deadness -- run the probe.
//
// ORIGINAL (WRONG) NOTE FOLLOWS.
// g419 -- `bool std::__1::operator!=(const pair<A,B> &, const pair<A,B> &)` is
// NOT KEYED HERE, deliberately.  MEASURED 2026-09-27: an unqualified call form
// `operator!=(a, b)` with two `const std::pair<T1, T2> &` parameters records the
// key as `bool std::operator!=(...)` -- WITHOUT the `__1` inline-namespace
// component that rules/unique_ptr's f16-f19 get for the same shape -- so the
// converter still aborts at the use site:
//   unsupported CXXOperatorCallExpr: != on (std::pair<std::map<int, int>, ...>)
//   rule key: bool std::__1::operator!=(const std::pair<...> &, ...)
//   converter.cpp:3929
// i.e. the recorded key and the required key DISAGREE, and a rule written that
// way is a DEAD KEY.  A qualified `std::__1::operator!=(a, b)` is not an option:
// it aborts the preprocessor at cpp_rule_preprocessor.cpp:888.  The BODY is not
// in question (std::pair compares first-then-second, which is exactly Rust tuple
// PartialEq); only the key spelling is, and it needs a preprocessor-side answer.

// f15 -- the CONSTRUCTOR SELECTED FOR A STRING-LITERAL FIRST ARGUMENT.  libc++
// picks the perfect-forwarding `template<class _U1,class _U2> pair(_U1&&,_U2&&)`;
// with `_U1 = const char (&)[N]` reference-collapsing makes the DECLARED
// signature print as `pair(const char (&)[N], double *&)`, whose first parameter
// ends in `]`, not `&`, so it cannot unify with f4/f5/f6/f7 (each of which needs
// a trailing `&`).  Spelling copied from rules/vector's f40.  This is the key
// behind 240 of dxp_standalone.cpp's placeholders.
// The element type is pinned to `char` DELIBERATELY: the body has to turn the
// array into this project's std::string representation (a NUL-TERMINATED
// Vec<libc::c_char> / Vec<u8>), which is only meaningful for a char array.  A
// generic `T3 const (&)[_]` would record ONE key for every element type and
// answer wrongly for e.g. an int array.  MEASURED: the array extent normalises
// to `[_]`, so `[4]` and `[3]` share this one key -- harmless, the extent is
// unused in the body.
template <typename T1, typename T2, std::size_t T3>
std::pair<T1, T2> f15(char const (&a0)[T3], T2 &a1) {
  return std::pair<T1, T2>(a0, a1);
}

// f16 -- same as f15 but with an RVALUE second argument, which is the form the
// 240-entry NSDMI actually uses: `{"nin", &N_.in_}` passes a PRVALUE `double *`,
// so the forwarding ctor deduces `_U2 = double *` and the declared signature ends
// in `&&`, not `&`.  MEASURED: f15 alone still left `{"nout", &b}` falling back.
template <typename T1, typename T2, std::size_t T3>
std::pair<T1, T2> f16(char const (&a0)[T3], T2 &&a1) {
  return std::pair<T1, T2>(a0, std::move(a1));
}

// f17/f18 -- pair's free comparison operators, written EXACTLY like rules/deque's
// f15/f16.  This SUPERSEDES the g419 note above, which is now known wrong: the
// `std::__1` inline-namespace component comes from the RESOLVED CALLEE's
// declaration (cpp_rule_preprocessor.cpp:232/237 getQualifiedNameAsString,
// mapper.cpp:2162/2165 printQualifiedName), not from how the call is written, so
// an UNQUALIFIED `operator==(a, b)` records WITH `__1` and matches -- as
// rules/deque, rules/vector f115/f116 and rules/set already demonstrate.
template <typename T1, typename T2>
bool f17(const std::pair<T1, T2> &a, const std::pair<T1, T2> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
bool f18(const std::pair<T1, T2> &a, const std::pair<T1, T2> &b) {
  return operator!=(a, b);
}

// f19 -- THE MIRROR OF f15/f16: a STRING LITERAL IN SECOND POSITION.  This is the
// shape of `{ComputeOpType::A, "a"}` inside a std::map init-list, i.e. the top
// fabricated std::pair receivers (std_pair_const_OpFuncs__std_string_ 528 sites,
// std_pair_const_SenComponents__std_string_ 321, ...).  libc++ picks the
// perfect-forwarding `template<class _U1,class _U2> pair(_U1&&,_U2&&)`; with
// `_U2 = const char (&)[N]` reference-collapsing makes the DECLARED signature
// print with a SECOND parameter ending in `]`, not `&&`, so it cannot unify with
// f7 (`pair(T3 &&, T4 &&)`, whose nextLit ` &&)` does not occur at depth 0 in the
// searched spelling) nor with f15/f16 (opposite parameter order).
// PROVEN SPELLING, from a real -verbose log
// (wip/incgroup/cde/ddl_conversion.cpp.unsafe.log:216530):
//   search expr void std::pair<const BaseFuncType, std::string>::pair(
//                    BaseFuncType &&, const char (&)[_]), result:
//   Matching: void std::pair<T1, T2>::pair(T3 &&, const char (&)[_])
// SWALLOW-SAFETY: the `[_]` placeholder is followed by `nextLit = ")"` at
// end-of-signature and no same-depth comma follows it, so against a SHORTER
// instantiation the scan runs to npos and the key fails to match AT ALL rather
// than swallowing a comma-joined tail.  f15/f16 keep 100% of their traffic
// (different parameter order, so no ambiguity).
// The element type is pinned to `char` for the same reason as f15: the body has
// to turn the array into this project's std::string representation.
template <typename T1, typename T2, typename T3, std::size_t T4>
std::pair<T1, T2> f19(T3 &&a0, char const (&a1)[T4]) {
  return std::pair<T1, T2>(std::move(a0), a1);
}

// f20 -- THE `const` SIBLING OF f15, AND THE KEY BEHIND THE LARGEST SINGLE ERROR
// CLASS MEASURED IN THIS PROJECT (733 of 1,246 error lines in the 16-TU crate).
// MEASURED 2026-09-28 from a `-verbose` run on the real corpus TU
// (/home/agent/work/pairctor/isa.log, isa.cpp):
//   search expr void std::pair<std::string, int>::pair(
//                    const char (&)[_], const int &), result:
//   None
// f15 is `pair(const char (&)[_], T2 &)` and f16 is `(..., T2 &&)`; NEITHER can
// unify, because the corpus's second argument is a `const int` LVALUE --
// `const int numLCCRs = 16;` / `const int numJCRs = ...` at isa.cpp:228-229 --
// so the forwarding ctor deduces `_U2 = const int &` and the DECLARED signature
// prints `const int &`, which is `const T2 &`, not `T2 &` and not `T2 &&`.
// This is NOT a swallow (arity 2, clean singular spelling, no comma-joined or
// oddly-decorated operand) and NOT staleness (f15/f16 are recorded and are
// MATCHING 18 other sites in the same log).  It is a plain missing overload.
// SWALLOW-SAFETY: identical to f15 -- `[_]` is followed by `, const T2 &)` and
// the final placeholder `T2` is terminated by `nextLit = " &)"` at
// end-of-signature, so a shorter instantiation fails to match rather than
// swallowing a tail.  No ambiguity with f15/f16: `const T2 &` and `T2 &` /
// `T2 &&` are distinct literal tails at the same position.
// Body is f15's verbatim; the `const` is a C++-side qualifier only and has no
// Rust representation here (a1 arrives by value, as in f15/f16).
template <typename T1, typename T2, std::size_t T3>
std::pair<T1, T2> f20(char const (&a0)[T3], const T2 &a1) {
  return std::pair<T1, T2>(a0, a1);
}

// ---------------------------------------------------------------------------
// f21/f22 -- `std::make_pair` WITH AN LVALUE FIRST ARGUMENT.  5 of
// dxp_standalone.cpp's placeholders, in 3 keys, read off a `--verbose` log:
//   pair<__unwrap_ref_decay_t<int &>, __unwrap_ref_decay_t<int &> > (int &, int &)
//   pair<__unwrap_ref_decay_t<std::deque<long> &>, __unwrap_ref_decay_t<std::vector<int> > >
//       (std::deque<long> &, std::vector<int> &&)
//   pair<__unwrap_ref_decay_t<std::deque<long> &>, __unwrap_ref_decay_t<std::vector<long> > >
//       (std::deque<long> &, std::vector<long> &&)
// f9 is `(T1 &&, T2 &)` and f10 is `(T1 &&, T2 &&)` -- BOTH have an RVALUE first
// parameter, because both call `std::make_pair(std::move(a0), ..)`.  The sites all
// pass an LVALUE first, so libc++ deduces `_T1 = X &` and the declared signature
// prints `(X &, ..)`.  Hence two new shapes: lvalue/lvalue and lvalue/rvalue.
// ⭐⭐ AND THE `__unwrap_ref_decay_t<..>` SCARE IS REFUTED -- MEASURED, NOT
// ASSUMED.  The search exprs above carry libc++'s AS-WRITTEN return type with the
// deduced arguments substituted, i.e. the alias UNEXPANDED, while these two rules
// record a DESUGARED return type (readback from ir_src.json):
//     std::pair<T1, T2> std::make_pair(T1 &, T2 &)
//     std::pair<T1, T2> std::make_pair(T1 &, T2 &&)
// A rule therefore CANNOT reproduce the sugared spelling -- and DOES NOT NEED TO.
// Both keys MATCH: all 5 of the goal TU's make_pair placeholders are gone and the
// body is inlined at the sites (`return (start_vcoord.into(), end_vcoord.into());`
// at dxp_standalone.cpp.rs:10816), and dsc/dims.cpp lost its make_pair placeholder
// too.  The return type is evidently not part of what the search unifies for a
// free function, so the thing to get right here was only the PARAMETER value
// categories.
template <class T1, class T2> auto f21(T1 &a0, T2 &a1) {
  return std::make_pair(a0, a1);
}

template <class T1, class T2> auto f22(T1 &a0, T2 &&a1) {
  return std::make_pair(a0, std::move(a1));
}

// ---------------------------------------------------------------------------
// f23 -- THE CONVERTING COPY CONSTRUCTOR `pair<T1,T2>::pair(const pair<T3,T4> &)`,
// i.e. a pair built from a pair WITH DIFFERENT TEMPLATE ARGUMENTS.
//
// ⭐ WHAT THIS IS ACTUALLY FOR, and it is not what the row it closes guessed.
// `dxp/dxp.cpp:1126-1129` does
//     const std::vector<std::pair<VariableSymbol, ValueType>> &v;   // pair<long, long>
//     std::unordered_map<VariableSymbol, ValueType> values;          // value_type =
//     for (const auto &symVal : v) values.emplace(symVal);           //   pair<CONST long, long>
// so rules/unordered_map's f57 `Init<std::pair<const T1, T2>, Args>` has to construct a
// `pair<const long, long>` FROM a `pair<long, long>`.  That is libc++'s
// `template<class U1, class U2> pair(const pair<U1,U2> &)`, and NO rules/pair key had
// four generics on the source side: f2 is `pair(const std::pair<T1, T2> &)`, which can
// only unify when the argument's arguments are the RECEIVER's arguments.
//
// ⛔ THE HYPOTHESIS THIS REFUTES: the row was filed as "a generic type key may not bind
// when a template argument is cv-qualified -- does T1 refuse to bind `const long`?".
// IT DOES NOT.  Measured 2026-09-29, wt/fta.cpp2rust (md5 01e13ca7a1a986b12c84e497b
// 7d127ff) + ir/ftarow1, on a 10-line probe (cvprobe/p.cpp):
//     search type std::pair<const long, long>, result: (T1, T2)            <- t1 BINDS
//     search expr void std::pair<const long, long>::pair(
//         const std::pair<const long, long> &), result:
//     Matching: void std::pair<T1, T2>::pair(const std::pair<T1, T2> &)    <- f2 BINDS
// `matchTemplate` captures `T1 = const long` without complaint; `GetTypeMapKey` buckets
// on the text before the first `<`, so the cv-qualifier is not in the bucket either.
// The defect is a MISSING RULE SHAPE, not a Mapper-level cv defect.  The reproduction
// that does fail is cvprobe/p2.cpp (14 lines, vector<pair<long,long>> -> emplace):
//     search expr void std::pair<const long, long>::pair(
//         const std::pair<long, long> &), result: None
// `const` is load-bearing only in that it is WHY the two pairs differ here -- any
// `pair<A,B>` built from a `pair<C,D>` hits the same hole.
//
// ⚠️ BLAST RADIUS.  `search()`'s tie-break prefers the LONGER `src`, and f23's src is
// longer than f2's, so f23 DISPLACES f2 on the same-arguments case too (T3=T1, T4=T2).
//
// ⛔⛔ THE ORIGINAL LANDING'S BODY WAS WRONG, AND THIS IS THE CORRECTION (row g3004).
// It emitted f2's body VERBATIM -- `a0.clone()` (unsafe) / the two-cell
// `Rc::new(RefCell::new(a0.N.borrow().clone()))` (refcount) -- on the claim that "both
// pairs share the `(T1, T2)` model".  ⭐ THAT HOLDS ONLY WHEN T3=T1 AND T4=T2.  When the
// element types differ the clone yields the SOURCE tuple type and rustc rejects it:
//   let mut q: (i32, i32) = k.clone();   // k: (i32, f64)
//   error[E0308]: expected `(i32, i32)`, found `(i32, f64)`
// Measured 2026-09-29 on pairctorwk/p3.cpp in BOTH models, wt/cvbind.cpp2rust
// (md5 2229b6e153fb4e9abccb7bfbc0336992) + ir/f23fix.  14 of fresh38's 87 arity-1 pair
// sites are this shape, e.g. dcg/dcg_fe/pcfg_gen/inputNeighFetchOp.cpp:1735, where
// `make_pair(coreId, coord_A.at("i"))` builds a `pair<int, Dtype>` and the receiver is
// `map<pair<int,int>, int>::count`'s key type.  It was never SILENT -- f23 still removes
// the fabricated `::new_N` name -- but the failure only moved from a loud fabricated name
// to a loud E0308.
//
// The body is now f4's PER-ELEMENT idiom: `.into()` in the unsafe model and
// `.try_into().expect(..)` through the cell in refcount.  That is correct for the
// identical case (identity `Into`) and for any element pair with a `From`/`TryFrom` impl
// (int -> double, `const long`/`long`).
//
// ⛔⛔ AND IT STILL DOES NOT COVER A NARROWING NUMERIC CONVERSION, WHICH IS WHAT THOSE 14
// SITES ACTUALLY NEED.  `Dtype` is instantiated `double` there, so the per-element
// conversion is `f64 -> i32`, and std has NEITHER `From<f64> for i32` NOR
// `TryFrom<f64> for i32`.  Measured on p3.cpp after this change:
//   unsafe  : error[E0277]: the trait bound `i32: From<f64>` is not satisfied
//   refcount: error[E0277]: the trait bound `i32: TryFrom<f64>` is not satisfied
// so the failure moves E0308 -> E0277 and stays loud.  ⭐ THE PIPELINE'S CONVENTION FOR AN
// ORDINARY C++ IMPLICIT NARROWING CONVERSION IS A CONVERTER-EMITTED RUST `as` CAST --
// measured on a 8-line probe (`int i = someDouble;`): unsafe emits `let mut i: i32 =
// (d as i32);`, refcount emits `Rc::new(RefCell::new(((*d.borrow()) as i32)))`.  ⛔ A RULE
// BODY CANNOT EXPRESS THAT: writing `(a0.0 as T1, a0.1 as T2)` is rejected by the rule
// preprocessor's own rustc pass with
//   error[E0605]: an `as` expression can only be used to convert between primitive types
//                 or to coerce to a specific trait object
// (measured; the regen then aborts at semantic.rs:260).  So closing the narrowing case
// needs EITHER a libcc2rs conversion trait with `as`-semantics impls over the primitive
// pairs, callable from a rule body, OR the converter inserting the per-element cast when
// it inlines this rule -- it already knows both concrete element types.  Neither is a rule
// change, so neither is done here; this commit makes the body CORRECT for everything a
// trait impl exists for and leaves the narrowing case loud.
//
// cf. the shadow-rule defect (0525e24b), where two rules under one spelling emitted
// DIFFERENT targets and iteration order decided which won.
//
// ⚠️ CONST IS NOT HANDED OUT MUTABLY.  `pair<const long, long>` and `pair<long, long>`
// have the SAME model `(i64, i64)` -- `const` has no Rust representation on a tuple
// element (same argument as f20's `const T2 &`).  The map key reaches the container
// through rules/unordered_map f57, which clones it (`values.insert(__k.clone(), ..)`),
// so nothing derived from this rule yields `&mut` to a key.
//
// NOT ADDED, and why: the rvalue sibling `pair(std::pair<T3,T4> &&)` is UNMEASURED --
// no ask for it appears in either probe log.  Adding an unasked-for key is how a type
// key gets a method key it does not need; the loud path stays loud without it.
template <class T1, class T2, class T3, class T4>
std::pair<T1, T2> f23(const std::pair<T3, T4> &a0) {
  return std::pair<T1, T2>(a0);
}
