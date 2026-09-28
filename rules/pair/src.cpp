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
