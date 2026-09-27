// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

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
