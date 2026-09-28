// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <tuple>

template <typename T1, typename T2> using t1 = std::tuple<T1, T2>;

template <typename T1, typename T2, typename T3>
using t2 = std::tuple<T1, T2, T3>;

template <typename T1, typename T2, typename T3, typename T4>
using t3 = std::tuple<T1, T2, T3, T4>;

template <typename T1, typename T2, typename T3, typename T4, typename T5>
using t4 = std::tuple<T1, T2, T3, T4, T5>;

// CONSTRUCTOR probe: the converter reports the ctor key as
// `void std::tuple<int, std::string, double>::tuple(&&...)`, i.e. an
// unexpanded-pack parameter list.  Read the key the preprocessor actually
// records for this body before believing a rule can express it.
template <typename T1, typename T2>
std::tuple<T1, T2> f1(T1 a0, T2 a1) {
  return std::tuple<T1, T2>(a0, a1);
}

template <typename T1, typename T2, typename T3>
std::tuple<T1, T2, T3> f2(T1 a0, T2 a1, T3 a2) {
  return std::tuple<T1, T2, T3>(a0, a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4>
std::tuple<T1, T2, T3, T4> f3(T1 a0, T2 a1, T3 a2, T4 a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::tuple<T1, T2, T3, T4, T5> f4(T1 a0, T2 a1, T3 a2, T4 a3, T5 a4) {
  return std::tuple<T1, T2, T3, T4, T5>(a0, a1, a2, a3, a4);
}

// 27-ary: the arity of DataStructDims::tie() (dsc/dims.h:283).
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27>
using t5 = std::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27>;

// g018: `operator==` on a 27-ary tuple of const-lvalue-references
// (`DataStructDims::operator==` at dsc/dims.h:283 is `tie() == A.tie()`, and
// `tie()` returns a 27-element std::tuple of `const double &` / `const std::map<...> &`).
// 27 is the ACTUAL corpus arity, counted from the depth-0 commas of the recorded
// rule key in queue/samples/g018.txt -- 21 `const double &` + 6 `const std::map<...> &`.
// Unqualified `operator==` per rules/vector f115; a qualified `std::operator==` aborts
// the rule preprocessor with "No viable function".
// ELEMENTS ARE SPELLED `const TN &`, NOT `TN`, AND THAT IS LOAD-BEARING, NOT cosmetic.
// With plain `TN` the placeholder binds the WHOLE element type `const double &`, which
// the converter maps to `*const f64`; the target's element-wise `__x.0 == __y.0` then
// compared POINTERS and silently answered `false` for two distinct objects with equal
// values (probe/tup27b: C++ 101, Rust 100).  Deref cannot fix that shape: a target body
// written `(*__x.0) == (*__y.0)` is REJECTED by the rule preprocessor with
// `error[E0614]: type `T1` cannot be dereferenced` -- a generic `T1` is not a pointer at
// rule-check time, whatever it instantiates to.  Binding `T1 = double` instead lets the
// target declare its elements as `*const T1` and deref them legally.
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27>
bool f5(const std::tuple<const T1 &, const T2 &, const T3 &, const T4 &, const T5 &, const T6 &, const T7 &, const T8 &, const T9 &, const T10 &, const T11 &, const T12 &, const T13 &, const T14 &, const T15 &, const T16 &, const T17 &, const T18 &, const T19 &, const T20 &, const T21 &, const T22 &, const T23 &, const T24 &, const T25 &, const T26 &, const T27 &> &a0, const std::tuple<const T1 &, const T2 &, const T3 &, const T4 &, const T5 &, const T6 &, const T7 &, const T8 &, const T9 &, const T10 &, const T11 &, const T12 &, const T13 &, const T14 &, const T15 &, const T16 &, const T17 &, const T18 &, const T19 &, const T20 &, const T21 &, const T22 &, const T23 &, const T24 &, const T25 &, const T26 &, const T27 &> &a1) {
  return operator==(a0, a1);
}

// g018 / dsc/dims.h:283: `std::tie(a,b,...)` inside `DataStructDims::tie() const`.
// The key the preprocessor records is
//   `std::tuple<const T1 &, ..., const T27 &> std::tie(&&...)`
// -- the parameter list COLLAPSES to the unexpanded pack `&&...` exactly as the
// tuple constructors f1-f4 do, so the arity lives entirely in the RETURN type.
// Elements are spelled `const TN &` rather than plain `TN` because that is what
// `std::tie(Types&...) -> std::tuple<Types&...>` deduces, and it is the whole
// point of the function: `tie()` REFERS to the members, it does not copy them.
// ARITY: 27 only.  Every other `std::tie` call site in the corpus (50 of them,
// `grep -rn 'std::tie' dt_src`) is an ASSIGNMENT target -- `std::tie(x,y) = f()`
// -- at arity 2-4, which needs assign-through-reference semantics this key does
// NOT provide; adding a short key would ALSO swallow longer calls (GetTypeMapKey
// strips at `<`, matchTemplate scans to the final depth-0 `>`), so a 2-ary key is
// active harm.  27 is what g018 needs and 27 is what this covers.
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27>
std::tuple<const T1 &, const T2 &, const T3 &, const T4 &, const T5 &, const T6 &, const T7 &, const T8 &, const T9 &, const T10 &, const T11 &, const T12 &, const T13 &, const T14 &, const T15 &, const T16 &, const T17 &, const T18 &, const T19 &, const T20 &, const T21 &, const T22 &, const T23 &, const T24 &, const T25 &, const T26 &, const T27 &> f6(const T1 &a0, const T2 &a1, const T3 &a2, const T4 &a3, const T5 &a4, const T6 &a5, const T7 &a6, const T8 &a7, const T9 &a8, const T10 &a9, const T11 &a10, const T12 &a11, const T13 &a12, const T14 &a13, const T15 &a14, const T16 &a15, const T17 &a16, const T18 &a17, const T19 &a18, const T20 &a19, const T21 &a20, const T22 &a21, const T23 &a22, const T24 &a23, const T25 &a24, const T26 &a25, const T27 &a26) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
}

// `std::tie` at the arities the corpus ACTUALLY uses besides 27.  Counted from
// dt_src (`grep -rhno 'std::tie([^;]*)'`): 34 sites at arity 2, 11 at arity 3,
// 3 at arity 4, plus the 27-ary DataStructDims::tie().  Nothing at 5-26, so
// nothing is keyed there.
// ELEMENTS ARE NON-CONST `TN &`, NOT `const TN &`, AND THAT IS A DIFFERENT KEY
// FROM f6.  Measured on probe/refbind/min.cpp (`int x, y; std::tie(x, y)`), whose
// searched key is verbatim
//     std::tuple<int &, int &> std::tie(&&...)
// -- `std::tie(Types&...)` deduces `Types = int` for a non-const lvalue, where
// f6's 27 arguments are members read through a `const` member function and so
// deduce `Types = const double`.  f6 therefore does NOT cover these sites.
// WHY ALL THREE OF 2/3/4 AND NOT JUST 2: `GetTypeMapKey` strips at `<` so arity
// is not in the bucket key and `matchTemplate` scans to the FINAL depth-0 `>`,
// which means a lone 2-ary key would MATCH a 3- or 4-ary call and swallow the
// extra elements into its last placeholder (the rules/variant defect).  With
// every used arity present as a distinct src in the one bucket, `search()`'s
// longer-src tie-break (mapper.cpp:433-437) resolves each call to its own key --
// the same reason the per-arity `std::function` keys are safe.
// THE 48 ASSIGNMENT-TARGET SITES (`std::tie(a, b) = f()`) ARE STILL NOT HANDLED,
// DELIBERATELY: a tuple of raw pointers cannot provide assign-through-reference.
// These keys only make the `std::tie(...)` CALL itself mapped; the enclosing
// `std::tuple::operator=` has NO key in this module (grep: none), so an
// assignment site now fails on that unmapped operator instead of on an
// unmapped-function `tie_N` fallback.  Both are LOUD (an undefined name at link
// time), so this is strictly a step forward and cannot silently drop a write.
// Modelling assign-through-reference is a separate row and is NOT attempted here.
template <typename T1, typename T2>
std::tuple<T1 &, T2 &> f7(T1 &a0, T2 &a1) {
  return std::tie(a0, a1);
}

template <typename T1, typename T2, typename T3>
std::tuple<T1 &, T2 &, T3 &> f8(T1 &a0, T2 &a1, T3 &a2) {
  return std::tie(a0, a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4>
std::tuple<T1 &, T2 &, T3 &, T4 &> f9(T1 &a0, T2 &a1, T3 &a2, T4 &a3) {
  return std::tie(a0, a1, a2, a3);
}

// ARITY 1: `std::tuple<X>` -- 7 queue rows (g2925-g2930, g2935), every one an
// MLIR attribute-storage `getAsKey()` returning a ONE-element tuple
// (`std::tuple<DdlDummy>`, `std::tuple<unsigned int>`, ...).  t1..t5 cover
// arities 2/3/4/5/27 and NONE of them matches a 1-element tuple, which is why
// those rows abort with `system type has no rule: std::tuple<X>`.
// WHY A 1-ARY KEY IS SAFE HERE, unlike the 2-ary `std::tie` key this module
// deliberately refuses: `GetTypeMapKey` strips at `<` and `matchTemplate` scans
// to the final depth-0 `>`, so a SHORT src can swallow a LONGER instantiation --
// but `search()`'s longer-src tie-break (mapper.cpp:433-437) means this src can
// only ever win where nothing longer matches.  For any arity >= 2, `t1`
// (`std::tuple<T1, T2>`) is strictly longer and is already a candidate by the
// very same mechanism, so it wins; and if `matchTemplate` does NOT in fact
// swallow commas, then this src cannot match an arity >= 2 instantiation either.
// Either way this key is confined to arity 1 and changes no existing outcome.
template <typename T1> using t6 = std::tuple<T1>;
