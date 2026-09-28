// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::variant<...> -- modelled as a REAL Rust enum (libcc2rs::Variant2 /
// Variant3), one type key per ARITY, exactly as rules/tuple does.
//
// WHY THIS IS EXPRESSIBLE AT ALL: a rule module cannot declare an enum, but a
// rule TARGET CAN NAME A libcc2rs TYPE (rules/map/tgt_unsafe.rs names
// UnsafeMapIterator), so the enum lives in libcc2rs and these keys only name
// it.  The per-arity shape is the one std::tuple proved up to 27-ary.
//
// WHY AN ENUM AND NOT A TUPLE: a tuple lets a read of an INACTIVE alternative
// succeed, which is the one thing a variant exists to forbid.
//
// std::get IS DELIBERATELY NOT KEYED.  `get<0>` and `get<1>` record
// BYTE-IDENTICAL keys (the explicit non-type template argument does not reach
// the key -- the same defect queue row c004 records for std::tuple), so a rule
// for it could only ever return one fixed alternative.  That would be silent
// wrongness; it stays a LOUD abort instead.  Same reason holds_alternative<T>
// is absent: its alternative is likewise an explicit template argument.
// index() is keyable because it carries no explicit template argument.

#include <cstddef>
#include <variant>

template <typename T1, typename T2> using t1 = std::variant<T1, T2>;

template <typename T1, typename T2, typename T3>
using t2 = std::variant<T1, T2, T3>;

// ARITY 8 -- THE ONLY ARITY THE CORPUS ACTUALLY INSTANTIATES.  Measured on
// dxp_standalone: exactly one std::variant instantiation exists,
// `OperandAttr::data_` at sys-arch-spec/progir/progir.h:256, and it is 8-ary;
// arities 2 and 3 have ZERO uses.  t1/t2 above were therefore not merely
// unused, they were HARMFUL: GetTypeMapKey (mapper.cpp:94) truncates the bucket
// key at the first `<`, so ARITY IS NOT PART OF THE KEY and t1/t2 were
// candidates for the 8-ary type.  matchTemplate (mapper.cpp:250ff) resolves each
// Tn by scanning to the literal text that follows it in the rule src, and for
// `std::variant<T1, T2, T3>` the text after T3 is `>`, which
// findNextLiteralSameDepth finds at the FINAL depth-0 position -- so T3 captured
// alternatives 3..8 JOINED INTO ONE STRING, mapper.cpp:1242 recursed on that
// non-type, and the assert at :1233 (a no-op under the release build's NDEBUG)
// fell through to a null deref: rc=139 on Pipeline.cpp, StageCoarsening.cpp and
// RunProgramPipelines.cpp.  An 8-placeholder src beats t2 DETERMINISTICALLY
// rather than by luck: `search` (mapper.cpp:383) tie-breaks on src.size() and
// prefers the LONGER src.  kMaxGenerics is 64 (translation_rule.h:23), so 8 is
// well inside the limit.
template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
using t3 = std::variant<T1, T2, T3, T4, T5, T6, T7, T8>;

// Default construction value-initialises ALTERNATIVE 0.
template <typename T1, typename T2> std::variant<T1, T2> f1() {
  return std::variant<T1, T2>();
}

template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f2() {
  return std::variant<T1, T2, T3>();
}

// Construction from a value.  The ALTERNATIVE is identified by the PARAMETER
// TYPE, not by an explicit template argument, so each of these records a
// distinct key -- which is what makes them safe to model where get<> is not.
template <typename T1, typename T2> std::variant<T1, T2> f3(T1 a0) {
  return std::variant<T1, T2>(a0);
}

template <typename T1, typename T2> std::variant<T1, T2> f4(T2 a0) {
  return std::variant<T1, T2>(a0);
}

template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f5(T1 a0) {
  return std::variant<T1, T2, T3>(a0);
}

template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f6(T2 a0) {
  return std::variant<T1, T2, T3>(a0);
}

template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f7(T3 a0) {
  return std::variant<T1, T2, T3>(a0);
}

template <typename T1, typename T2>
std::size_t f8(const std::variant<T1, T2> &a0) {
  return a0.index();
}

template <typename T1, typename T2, typename T3>
std::size_t f9(const std::variant<T1, T2, T3> &a0) {
  return a0.index();
}

// ---- arity 8 ------------------------------------------------------------
// A TYPE KEY WITHOUT ITS CONSTRUCTORS GIVES rc=0 AND THEN E0433: the converter
// looks the default ctor up as an ordinary expr rule and, on a miss, falls back
// to `<mangled-type>::new()`, which does not exist.  So t3 is accompanied by its
// default ctor and ALL EIGHT value ctors.  The alternative is identified by the
// PARAMETER TYPE, never by an explicit template argument, so each of the eight
// records a DISTINCT key -- which is exactly why these are safe to model where
// std::get<I> is not.
template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f10() {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>();
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f11(T1 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f12(T2 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f13(T3 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f14(T4 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f15(T5 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f16(T6 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f17(T7 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f18(T8 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::size_t f19(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return a0.index();
}

// ---- f20 -- operator== on two 8-ary variants (row g848) ---------------------
//
// Site: sys-arch-spec/progir/progir.cpp:69 `return attr1.data_ == attr2.data_;`
// on the 8-ary `OperandAttr::data_`, the only std::variant the corpus
// instantiates.
//
// ⛔ WHAT MAKES THIS CORRECT RATHER THAN A PAYLOAD COMPARISON.  std::variant
// equality compares the ACTIVE INDEX FIRST and only then the held value: two
// variants holding equal-looking data in DIFFERENT alternatives are NOT equal.
// A model that unwrapped and compared payloads would answer `true` for
// `variant<long, float>(1L)` vs `variant<long, float>(1.0f)`.
//
// This module models std::variant as a REAL Rust enum (libcc2rs::Variant8), and
// that enum is `#[derive(Clone, Debug, PartialEq)]` (libcc2rs/src/variant.rs:42).
// A derived PartialEq on an enum compares the DISCRIMINANT first and the payload
// only within the matching arm -- byte-for-byte the C++ rule, for free. This is
// the payoff of the enum decision recorded at the top of this file; had variant
// been modelled as a tuple there would be no faithful body to write here.
//
// Eight placeholders, not two or three, for the reason t3 documents: arity is not
// part of the bucket key and `search` (mapper.cpp:474) prefers the LONGER src, so
// the 8-placeholder form beats a shorter one deterministically instead of by luck,
// and it cannot swallow alternatives 3..8 into T3 the way `t2` did.
template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
bool f20(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0,
         const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a1) {
  return operator==(a0, a1);
}
