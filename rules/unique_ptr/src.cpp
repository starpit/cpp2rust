// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstddef>
#include <memory>

template <typename T, typename A> using Init = A;

template <typename T1> using t1 = std::unique_ptr<T1>;
template <typename T1> using t2 = std::unique_ptr<T1[]>;

template <typename T2, typename T1> std::unique_ptr<T1[]> f1(std::size_t n) {
  return std::make_unique<T1[]>(n);
}

template <typename T1> T1 *f2(std::unique_ptr<T1> &o) { return o.get(); }

template <typename T1> std::unique_ptr<T1> f3(T1 *p) {
  return std::unique_ptr<T1>(p);
}

template <typename T1> std::unique_ptr<T1[]> f4(T1 *p) {
  return std::unique_ptr<T1[]>(p);
}

template <typename T1> void f5(std::unique_ptr<T1> &o, T1 *p) {
  return o.reset(p);
}

template <typename T1> void f6(std::unique_ptr<T1[]> &o, T1 *p) {
  return o.reset(p);
}

template <typename T1> T1 *f7(std::unique_ptr<T1[]> &o) { return o.get(); }

template <typename T1, typename... Args>
std::unique_ptr<T1> f8(Init<T1, Args> &&...args) {
  return std::make_unique<T1>(std::forward<Args>(args)...);
}

template <typename T1> void f9(std::unique_ptr<T1[]> &o) {
  return o.reset(nullptr);
}

template <typename T1> std::unique_ptr<T1> f10() {
  return std::unique_ptr<T1>();
}

template <typename T1> std::unique_ptr<T1[]> f11() {
  return std::unique_ptr<T1[]>();
}

template <typename T1> std::unique_ptr<T1> f12(std::unique_ptr<T1> &&o) {
  return std::unique_ptr<T1>(std::move(o));
}

template <typename T1> std::unique_ptr<T1[]> f13(std::unique_ptr<T1[]> &&o) {
  return std::unique_ptr<T1[]>(std::move(o));
}

template <typename T1>
std::unique_ptr<T1> &f14(std::unique_ptr<T1> &dst, std::unique_ptr<T1> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1>
std::unique_ptr<T1[]> &f15(std::unique_ptr<T1[]> &dst,
                           std::unique_ptr<T1[]> &&src) {
  return dst.operator=(std::move(src));
}

// ---------------------------------------------------------------------------
// NULL TESTS -- `p == nullptr` / `p != nullptr`, and the REVERSED spelling
// `nullptr == p`.  Exactly the shape rules/shared_ptr landed as f17/f18
// (commit 0934519); the target codebase writes all four.
//
// Called UNQUALIFIED: an infix `o == nullptr` records NOTHING (the preprocessor
// needs call form) and a QUALIFIED `std::operator==` aborts the preprocessor at
// cpp_rule_preprocessor.cpp:888.
//
// `std::nullptr_t` carries no information -- the converter emits the `nullptr`
// literal as `Default::default()` -- so the target binds that operand to `()`
// and leaves it deliberately unused.
//
// Unlike shared_ptr there is no pointer-identity variant here: two distinct
// unique_ptrs can never hold the same pointer, so `p == q` is not a shape the
// target codebase can write and no key is added for it.
template <typename T1>
bool f16(const std::unique_ptr<T1> &o, std::nullptr_t n) {
  return operator==(o, n);
}

template <typename T1>
bool f17(const std::unique_ptr<T1> &o, std::nullptr_t n) {
  return operator!=(o, n);
}

template <typename T1>
bool f18(std::nullptr_t n, const std::unique_ptr<T1> &o) {
  return operator==(n, o);
}

template <typename T1>
bool f19(std::nullptr_t n, const std::unique_ptr<T1> &o) {
  return operator!=(n, o);
}

// ---------------------------------------------------------------------------
// f20 -- `std::unique_ptr<T1>(std::nullptr_t)`.
//
// The ctor OVERLOAD the `::new_N` fabrication class was missing for this module
// (FABRICATED-NEWN.md: 234 sites / 117 files at receiver
// `std_unique_ptr_std_string__std_default_delete_std_string__`).  The 117 files
// are ONE C++ row: the inline `InstrInfo::operator=(const InstrInfo &)` in
// sys-arch-spec/progir/progir.h:292-294, re-emitted per including TU, whose
// ternary
//     comment_ = other.comment_ ? std::make_unique<std::string>(*other.comment_)
//                               : nullptr;
// converts the `nullptr` arm to `unique_ptr<std::string>`, i.e. exactly this
// ctor.  `f10` (the default ctor) already exists and RESOLVES; this overload did
// not, so the converter emitted
//     std_unique_ptr_std_string__std_default_delete_std_string__::new_1({
//         Default::default() })
// -- a function defined nowhere -- at rc=0 with no placeholder token.
//
// Same shape, same fix as rules/shared_ptr f20.  `std::nullptr_t` carries no
// information (the converter emits the `nullptr` literal as
// `Default::default()`), so the target binds that operand to `()` and leaves it
// deliberately unused.
//
// OWNERSHIP (the refusal criterion for this module): a null unique_ptr owns
// NOTHING.  No pointer is transferred in, none is duplicated, and the result is
// the same empty `Option` that `f10` yields, so there is no observer of a
// double-free, a leak, or two live owners.  Safe to express.
template <typename T1> std::unique_ptr<T1> f20(std::nullptr_t n) {
  return std::unique_ptr<T1>(n);
}
