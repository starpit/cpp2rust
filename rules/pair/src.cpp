// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <utility>
// <vector> is pulled in deliberately: with only <utility>, libc++ prints the
// free comparison operators as std::operator== rather than std::__1::operator==,
// and the rules then silently fail to match in any real TU (which will always
// have included a larger header first).
#include <vector>

template <typename T1, typename T2> using t1 = std::pair<T1, T2>;

template <typename T1, typename T2> T2 &f1(std::pair<T1, T2> &o) {
  return o.second;
}

// The default constructor. t1 supplies the default VALUE wherever the mapper
// materialises one, but an explicit `std::pair<A, B> p;` is a CXXConstructExpr
// and needs a function rule of its own, otherwise it falls back to the mangled
// `std_pair_A__B_::std_pair_A__B_()` placeholder.
template <typename T1, typename T2> std::pair<T1, T2> f3() {
  return std::pair<T1, T2>();
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

template <typename T1, typename T2>
bool f15(const std::pair<T1, T2> &a, const std::pair<T1, T2> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
bool f16(const std::pair<T1, T2> &a, const std::pair<T1, T2> &b) {
  return operator!=(a, b);
}

// ---------------------------------------------------------------------------
// The converting constructors.  Everything below exists because libc++'s pair
// constructors are themselves templates: the argument types are deduced from
// the CALL, not from T1/T2, so `std::pair<long long, long long>(p)` where `p`
// is a `std::pair<int, int>` resolves to a signature that f2 (the same-type
// copy constructor) cannot unify with.  The resolved spellings these produce
// were read straight out of `cpp2rust --verbose` ("search expr ..., result:"
// with an empty result immediately before a mangled-fallback emission).
// ---------------------------------------------------------------------------

// NOT ADDED: `pair<A, B>` from a `pair<C, D>` with DIFFERENT element types
// (e.g. `std::pair<long long, long long>(std::pair<int, int>)`).  Written as
//     template <typename T1, typename T2, typename T3, typename T4>
//     std::pair<T1, T2> f17(const std::pair<T3, T4> &a0);
// it resolves to `pair(const std::pair<T3, T4> &)`, which the matcher rates
// EXACTLY as specific as f2's `pair(const std::pair<T1, T2> &)`.  The
// converter then reports "ambiguous translation rule ... Refusing to guess"
// and silently falls back to a whole-tuple `.clone()` -- which in the refcount
// model clones the Rc handles, so the copy ALIASES the original.  That broke
// tests/unit/clone_vs_move.cpp.  Reverted; the converting copy constructor
// stays unmapped until the matcher can rank one pattern above the other.

// `std::pair<const Enum, std::string>(e, "literal")`.  The second argument is
// a string LITERAL, so libc++ deduces the parameter as `const char (&)[N]`
// rather than std::string, and f7's `T4 &&` cannot unify with an array
// reference.  char is spelled concretely because the Rust side has to know the
// element width to walk the NUL-terminated literal.
template <typename T1, typename T2, typename T3, std::size_t T4>
std::pair<T1, T2> f18(T3 &&a0, const char (&a1)[T4]) {
  return std::pair<T1, T2>(std::move(a0), a1);
}

// `std::pair<const std::string, V>("literal", lvalue)`.
template <typename T1, typename T2, typename T3, std::size_t T4>
std::pair<T1, T2> f19(const char (&a0)[T4], T3 &a1) {
  return std::pair<T1, T2>(a0, a1);
}

// `std::pair<const std::string, V>("literal", rvalue)`.
template <typename T1, typename T2, typename T3, std::size_t T4>
std::pair<T1, T2> f20(const char (&a0)[T4], T3 &&a1) {
  return std::pair<T1, T2>(a0, std::move(a1));
}

// std::make_pair from two LVALUES, and from two const lvalues.  f9/f10 only
// cover the rvalue forms, so `std::make_pair(a, b)` on two named variables --
// by far the common spelling -- had no rule at all.
template <class T1, class T2> auto f21(T1 &a0, T2 &a1) {
  return std::make_pair(a0, a1);
}

template <class T1, class T2> auto f22(const T1 &a0, const T2 &a1) {
  return std::make_pair(a0, a1);
}

// The remaining make_pair value-category combinations.  Each one is a
// separate resolved signature: libc++ deduces the parameter from the CALL, so
// `make_pair(lvalue, rvalue)` and `make_pair(lvalue, lvalue)` never share a
// rule.  Spellings read out of `cpp2rust --verbose`.
template <class T1, class T2> auto f23(const T1 &a0, T2 &a1) {
  return std::make_pair(a0, a1);
}

template <class T1, class T2> auto f24(const T1 &a0, T2 &&a1) {
  return std::make_pair(a0, std::move(a1));
}

template <class T1, class T2> auto f25(T1 &a0, const T2 &a1) {
  return std::make_pair(a0, a1);
}

template <class T1, class T2> auto f26(T1 &a0, T2 &&a1) {
  return std::make_pair(a0, std::move(a1));
}

template <class T1, class T2> auto f27(T1 &&a0, const T2 &a1) {
  return std::make_pair(std::move(a0), a1);
}
