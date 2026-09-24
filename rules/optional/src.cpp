// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <optional>

template <typename T1> using t1 = std::optional<T1>;

// ---------------------------------------------------------------------------
// construction
// ---------------------------------------------------------------------------

template <typename T1> std::optional<T1> f1() { return std::optional<T1>(); }

template <typename T1> std::optional<T1> f2(std::nullopt_t a0) {
  return std::optional<T1>(a0);
}

// The converting constructor is `template <class U> optional(U&&)`, so each
// argument value category deduces a different signature.
template <typename T1, typename T2> std::optional<T1> f3(T2 &&a0) {
  return std::optional<T1>(std::move(a0));
}

template <typename T1, typename T2> std::optional<T1> f4(T2 &a0) {
  return std::optional<T1>(a0);
}

template <typename T1, typename T2> std::optional<T1> f5(const T2 &a0) {
  return std::optional<T1>(a0);
}

template <typename T1> std::optional<T1> f6(const std::optional<T1> &a0) {
  return std::optional<T1>(a0);
}

template <typename T1> std::optional<T1> f7(std::optional<T1> &&a0) {
  return std::optional<T1>(std::move(a0));
}

// ---------------------------------------------------------------------------
// assignment
// ---------------------------------------------------------------------------

template <typename T1>
std::optional<T1> &f8(std::optional<T1> &dst, const std::optional<T1> &src) {
  return dst.operator=(src);
}

template <typename T1>
std::optional<T1> &f9(std::optional<T1> &dst, std::optional<T1> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1>
std::optional<T1> &f10(std::optional<T1> &dst, std::nullopt_t src) {
  return dst.operator=(src);
}

template <typename T1, typename T2>
std::optional<T1> &f11(std::optional<T1> &dst, T2 &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1, typename T2>
std::optional<T1> &f12(std::optional<T1> &dst, T2 &src) {
  return dst.operator=(src);
}

template <typename T1, typename T2>
std::optional<T1> &f13(std::optional<T1> &dst, const T2 &src) {
  return dst.operator=(src);
}

// ---------------------------------------------------------------------------
// observers
// ---------------------------------------------------------------------------

template <typename T1> bool f15(const std::optional<T1> &a0) {
  return a0.operator bool();
}

template <typename T1> T1 &f16(std::optional<T1> &a0) { return a0.value(); }

template <typename T1> const T1 &f17(const std::optional<T1> &a0) {
  return a0.value();
}

template <typename T1> T1 &f18(std::optional<T1> &a0) {
  return a0.operator*();
}

template <typename T1> const T1 &f19(const std::optional<T1> &a0) {
  return a0.operator*();
}

template <typename T1> T1 *f20(std::optional<T1> &a0) {
  return a0.operator->();
}

template <typename T1> const T1 *f21(const std::optional<T1> &a0) {
  return a0.operator->();
}

template <typename T1, typename T2>
T1 f22(const std::optional<T1> &a0, T2 &&a1) {
  return a0.value_or(std::move(a1));
}

template <typename T1, typename T2>
T1 f23(const std::optional<T1> &a0, T2 &a1) {
  return a0.value_or(a1);
}

template <typename T1, typename T2>
T1 f24(const std::optional<T1> &a0, const T2 &a1) {
  return a0.value_or(a1);
}

// ---------------------------------------------------------------------------
// comparison
// ---------------------------------------------------------------------------

template <typename T1>
bool f26(const std::optional<T1> &a0, const std::optional<T1> &a1) {
  return operator==(a0, a1);
}

template <typename T1>
bool f27(const std::optional<T1> &a0, const std::optional<T1> &a1) {
  return operator!=(a0, a1);
}

template <typename T1>
bool f28(const std::optional<T1> &a0, std::nullopt_t a1) {
  return operator==(a0, a1);
}

template <typename T1>
bool f29(const std::optional<T1> &a0, std::nullopt_t a1) {
  return operator!=(a0, a1);
}

// ---------------------------------------------------------------------------
// modifiers / has_value()
//
// libc++ declares has_value() and reset() in *private base classes* of
// std::optional and re-exports has_value() with a using-declaration, so the
// rule preprocessor cannot reach either one through std::optional itself.
// Naming the base class directly does work, and the converter looks the call
// up under the base class name too, so that is what these rules do.
//
// The guard is on _LIBCPP_VERSION, not __APPLE__, and the distinction is not
// pedantic: __APPLE__ is the HOST OS, while the question these rules ask is
// WHICH STANDARD LIBRARY THE RULE PREPROCESSOR PARSES WITH.  Those differ here by
// design -- BOOTSTRAP.md records it -- because the preprocessor parses with libc++
// (the rules key on std::__1:: spellings) while the generated Rust is compiled
// against a libstdc++-built toolchain.  On this Linux pod __APPLE__ was false, so
// f14 and f25 were dropped from all three ir_*.json and every opt.has_value()
// fell through to a verbatim .has_value() on a Rust Option: 10 E0599 on one TU.
//
// The target files had the mirror of the same confusion, 4x
// #[cfg(target_os = "macos")] each, evaluated by rule-preprocessor against the
// host that RUNS it -- so they deleted the target entries too.  The two wrongs
// kept the key sets corresponding, which is exactly why check_rules.py, the
// invariant that exists to catch this class, never fired.  Whether a rule EXISTS
// is decided at parse time; the generated Rust is then compiled for whatever
// target, so no cfg belongs on the target side at all.
// ---------------------------------------------------------------------------
#if defined(_LIBCPP_VERSION)
template <typename T1> using t2 = std::__optional_storage_base<T1>;
template <typename T1> using t3 = std::__optional_destruct_base<T1>;

template <typename T1>
bool f14(const std::__optional_storage_base<T1> &a0) {
  return a0.has_value();
}

template <typename T1> void f25(std::__optional_destruct_base<T1> &a0) {
  return a0.reset();
}
#endif
