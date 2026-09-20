// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <bitset>
#include <cstddef>
// <vector>/<map>/<string> are pulled in deliberately: with only <bitset>,
// libc++ prints free operators as std::operator== rather than
// std::__1::operator==, and a rule keyed on the wrong spelling silently never
// matches in a real TU (which will always have included a larger header).
#include <map>
#include <string>
#include <vector>

// ===========================================================================
// REPRESENTATION: std::bitset<N>  ->  Vec<bool>, LSB first (index i == bit i).
// ===========================================================================
//
// Why Vec<bool> and not a fixed-width Rust integer:
//
//   The non-type template parameter N is ERASED before a rule is ever matched.
//   normalizeTranslationRule (converter/mapper.cpp:852-868) rewrites every
//   free-standing integer in a printed signature to `_`, on BOTH the rule side
//   and the AST side, so `std::bitset<128>` and `std::bitset<32>` are the same
//   string -- `std::bitset<_>` -- and share a single rule and a single Rust
//   type.  (This is the same mechanism rules/array relies on for
//   `std::array<T1, _>`, which likewise maps to Vec<T1>.)
//
//   A fixed-width integer therefore has to pick one width for every N in the
//   program.  u64 would SILENTLY TRUNCATE the std::bitset<128> at
//   sys-arch-spec/progir/progir.h:535, which is the bitset this module exists
//   for; u128 would silently truncate anything wider.  Silent bit loss is the
//   worst possible failure mode for a translator, so the representation has to
//   be one that grows.
//
// Semantics: the Vec holds the bits that have been MATERIALIZED so far.
//   * a read past the end yields `false` -- a default-constructed bitset is
//     all-zero, so this is exact;
//   * a write past the end grows the Vec with `false` padding.
//   test, operator[], set(pos, val), reset(pos), flip(pos), reset(), count(),
//   any(), none(), to_ulong(), to_ullong(), == and != are therefore FAITHFUL
//   for every N.
//
// KNOWN LIMITATION -- the two operations that need N itself:
//   * size()  returns the materialized length, not N.
//   * all()   is true when every materialized bit is set, so it is true on a
//             freshly default-constructed (empty) bitset, where C++ says false
//             for N > 0.
//   Both are listed here rather than silently omitted because the alternative
//   (a fixed-width integer) gets them wrong too AND loses bits.  The no-argument
//   set() and flip(), which would have to materialize N bits, are deliberately
//   NOT provided at all: they cannot be approximated safely, so they surface as
//   a loud survey gap instead.
//   std::bitset<N>::reference (the proxy returned by the non-const operator[])
//   is likewise not modelled, so `b[i] = v` is a gap; use b.set(i, v).
//
// ENGINE LIMITATION, not a rule gap: `b.set(i)` -- relying on set's defaulted
//   second argument -- ABORTS the converter with
//   `Assertion failed: (false && "computed_expr_type_ not set")`
//   (converter/converter.cpp:1634).  VisitCXXDefaultArgExpr
//   (converter/converter.cpp:3862-3868) only sets computed_expr_type_ for a
//   POINTER-typed default argument, so any rule-mapped call whose body
//   references a non-pointer CXXDefaultArgExpr dies there.  `b.set(i, true)`
//   is fine.  This is pre-existing and affects any module, not just this one;
//   it is invisible today only because no other rule body reads a non-pointer
//   default argument (rules/string f16 find_last_of has one, but its target
//   never references a1).
// ===========================================================================

template <std::size_t T1> using t1 = std::bitset<T1>;

// --- construction ----------------------------------------------------------

template <std::size_t T1> std::bitset<T1> f1() { return std::bitset<T1>(); }

template <std::size_t T1> std::bitset<T1> f2(unsigned long long a0) {
  return std::bitset<T1>(a0);
}

template <std::size_t T1> std::bitset<T1> f3(const std::bitset<T1> &a0) {
  return std::bitset<T1>(a0);
}

template <std::size_t T1>
std::bitset<T1> &f4(std::bitset<T1> &dst, const std::bitset<T1> &src) {
  return dst.operator=(src);
}

// --- element access --------------------------------------------------------

template <std::size_t T1>
bool f5(const std::bitset<T1> &o, std::size_t a1) {
  return o.test(a1);
}

template <std::size_t T1>
bool f6(const std::bitset<T1> &o, std::size_t a1) {
  return o.operator[](a1);
}

// --- mutation --------------------------------------------------------------
// libc++ declares `bitset &set(size_t pos, bool val = true)`, so this one rule
// covers both `b.set(i)` and `b.set(i, v)`.

template <std::size_t T1>
std::bitset<T1> &f7(std::bitset<T1> &o, std::size_t a1, bool a2) {
  return o.set(a1, a2);
}

template <std::size_t T1>
std::bitset<T1> &f8(std::bitset<T1> &o, std::size_t a1) {
  return o.reset(a1);
}

template <std::size_t T1> std::bitset<T1> &f9(std::bitset<T1> &o) {
  return o.reset();
}

template <std::size_t T1>
std::bitset<T1> &f10(std::bitset<T1> &o, std::size_t a1) {
  return o.flip(a1);
}

// --- observers -------------------------------------------------------------

template <std::size_t T1> std::size_t f11(const std::bitset<T1> &o) {
  return o.count();
}

// N-dependent: returns the materialized length, not N.  See the header note.
template <std::size_t T1> std::size_t f12(const std::bitset<T1> &o) {
  return o.size();
}

template <std::size_t T1> bool f13(const std::bitset<T1> &o) {
  return o.any();
}

template <std::size_t T1> bool f14(const std::bitset<T1> &o) {
  return o.none();
}

// N-dependent: true on an empty (default-constructed) bitset.  See the header.
template <std::size_t T1> bool f15(const std::bitset<T1> &o) {
  return o.all();
}

// --- conversion ------------------------------------------------------------

template <std::size_t T1> unsigned long f16(const std::bitset<T1> &o) {
  return o.to_ulong();
}

template <std::size_t T1> unsigned long long f17(const std::bitset<T1> &o) {
  return o.to_ullong();
}

// --- comparison ------------------------------------------------------------

template <std::size_t T1>
bool f18(const std::bitset<T1> &a, const std::bitset<T1> &b) {
  return a.operator==(b);
}

template <std::size_t T1>
bool f19(const std::bitset<T1> &a, const std::bitset<T1> &b) {
  return a.operator!=(b);
}
