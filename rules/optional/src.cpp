// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// std::optional<T1>.  THREE type rules, all mapping to the same Rust type:
// libc++ splits std::optional's members over two base class templates, and
// `cxxMethodNameLookup` (cpp_rule_preprocessor.cpp:725) resolves a receiver from
// its DECLARED PARAMETER TYPE -- so `has_value()` and `reset()` MUST be written
// against __optional_storage_base / __optional_destruct_base or the preprocessor
// aborts with assert(0 && "Rule resolution failed") (:888).
//
// std::nullopt_t is taken as a PARAMETER everywhere, never spelled as
// `std::nullopt`: copy-initialising a by-value nullopt_t from the `const
// nullopt_t` lvalue `std::nullopt` finds no viable copy constructor in the
// preprocessor's synthetic context.
//
// `operator->` IS DELIBERATELY NOT MAPPED, for the same reason as
// rules/shared_ptr: the converter treats an operator-> rule's result as a PLACE
// of type T1, so a rule returning Ptr<T1> gives error[E0609].  `(*o).f` routes
// through operator* and works in both models.

#include <optional>
#include <utility>

template <typename T1> using t1 = std::optional<T1>;
template <typename T1> using t2 = std::__optional_storage_base<T1>;
template <typename T1> using t3 = std::__optional_destruct_base<T1>;
using t4 = std::nullopt_t;

template <typename T1> std::optional<T1> f1() { return std::optional<T1>(); }

// NO RULE FOR `optional(T1 &&)`: the preprocessor resolves
// `std::optional<T1>(std::move(v))` in this dependent context to the COPY
// constructor, so the entry records `optional(const std::optional<T1> &)` --
// a DUPLICATE of f4 with a different body, i.e. an ambiguous rule.  Left out
// rather than shipped wrong.  `operator=(T1 &&)` (f7) does record correctly.

template <typename T1> std::optional<T1> f3(std::nullopt_t n) {
  return std::optional<T1>(n);
}

template <typename T1> std::optional<T1> f4(const std::optional<T1> &o) {
  return std::optional<T1>(o);
}

template <typename T1> std::optional<T1> f5(std::optional<T1> &&o) {
  return std::optional<T1>(std::move(o));
}

template <typename T1>
std::optional<T1> &f6(std::optional<T1> &d, std::nullopt_t n) {
  return d.operator=(n);
}

template <typename T1> std::optional<T1> &f7(std::optional<T1> &d, T1 &&v) {
  return d.operator=(std::move(v));
}

template <typename T1>
std::optional<T1> &f8(std::optional<T1> &d, const std::optional<T1> &s) {
  return d.operator=(s);
}

template <typename T1> bool f9(const std::__optional_storage_base<T1> &o) {
  return o.has_value();
}

template <typename T1> bool f10(const std::optional<T1> &o) {
  return o.operator bool();
}

template <typename T1> T1 &f11(std::optional<T1> &o) {
  return o.operator*();
}

template <typename T1> const T1 &f12(const std::optional<T1> &o) {
  return o.operator*();
}

template <typename T1> T1 &f13(std::optional<T1> &o) { return o.value(); }

template <typename T1> const T1 &f14(const std::optional<T1> &o) {
  return o.value();
}

template <typename T1> T1 f15(const std::optional<T1> &o, T1 &d) {
  return o.value_or(d);
}

template <typename T1> void f16(std::__optional_destruct_base<T1> &o) {
  return o.reset();
}

template <typename T1> bool f17(const std::optional<T1> &o, std::nullopt_t n) {
  return operator==(o, n);
}

template <typename T1> bool f18(std::nullopt_t n, const std::optional<T1> &o) {
  return operator==(n, o);
}

template <typename T1> bool f19(const std::optional<T1> &o, std::nullopt_t n) {
  return operator!=(o, n);
}

template <typename T1> bool f20(std::nullopt_t n, const std::optional<T1> &o) {
  return operator!=(n, o);
}

template <typename T1>
bool f21(const std::optional<T1> &a, const std::optional<T1> &b) {
  return operator==(a, b);
}

template <typename T1>
bool f22(const std::optional<T1> &a, const std::optional<T1> &b) {
  return operator!=(a, b);
}

template <typename T1> bool f23(const std::optional<T1> &o, const T1 &v) {
  return operator==(o, v);
}

template <typename T1> bool f24(const std::optional<T1> &o, const T1 &v) {
  return operator!=(o, v);
}

template <typename T1>
std::optional<T1> &f25(std::optional<T1> &d, std::optional<T1> &&s) {
  return d.operator=(std::move(s));
}
