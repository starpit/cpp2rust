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
