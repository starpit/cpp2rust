// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstddef>
#include <memory>

template <typename T1> using t1 = std::shared_ptr<T1>;
template <typename T1> using t2 = std::weak_ptr<T1>;

// ---------------------------------------------------------------------------
// std::make_shared
//
// Rust has no variadic generics, so (like make_unique) we write specialized
// versions for 0, 1 and 2 arguments. Each argument count needs one rule per
// value category because `Args&&...` deduces a different signature for
// rvalues, lvalues and const lvalues.
//
// Caveat on the two-argument form (f5/f6/f7): a rule body cannot call an
// arbitrary C++ constructor, so the target builds T1 with `From<(T2, T3)>`.
// That is right for types that have such an impl and a plain Rust type error
// for those that do not -- cpp2rust does not (yet) emit From impls for
// translated constructors.
// ---------------------------------------------------------------------------

template <typename T1> std::shared_ptr<T1> f1() {
  return std::make_shared<T1>();
}

template <typename T1, typename T2> std::shared_ptr<T1> f2(T2 &&a0) {
  return std::make_shared<T1>(std::move(a0));
}

template <typename T1, typename T2> std::shared_ptr<T1> f3(T2 &a0) {
  return std::make_shared<T1>(a0);
}

template <typename T1, typename T2> std::shared_ptr<T1> f4(const T2 &a0) {
  return std::make_shared<T1>(a0);
}

template <typename T1, typename T2, typename T3>
std::shared_ptr<T1> f5(T2 &&a0, T3 &&a1) {
  return std::make_shared<T1>(std::move(a0), std::move(a1));
}

template <typename T1, typename T2, typename T3>
std::shared_ptr<T1> f6(T2 &a0, T3 &a1) {
  return std::make_shared<T1>(a0, a1);
}

template <typename T1, typename T2, typename T3>
std::shared_ptr<T1> f7(const T2 &a0, const T3 &a1) {
  return std::make_shared<T1>(a0, a1);
}

// ---------------------------------------------------------------------------
// std::shared_ptr construction
// ---------------------------------------------------------------------------

template <typename T1> std::shared_ptr<T1> f8() {
  return std::shared_ptr<T1>();
}

template <typename T1> std::shared_ptr<T1> f9(T1 *a0) {
  return std::shared_ptr<T1>(a0);
}

template <typename T1>
std::shared_ptr<T1> f10(const std::shared_ptr<T1> &a0) {
  return std::shared_ptr<T1>(a0);
}

template <typename T1> std::shared_ptr<T1> f11(std::shared_ptr<T1> &&a0) {
  return std::shared_ptr<T1>(std::move(a0));
}

// ---------------------------------------------------------------------------
// std::shared_ptr assignment
// ---------------------------------------------------------------------------

template <typename T1>
std::shared_ptr<T1> &f12(std::shared_ptr<T1> &dst,
                         const std::shared_ptr<T1> &src) {
  return dst.operator=(src);
}

template <typename T1>
std::shared_ptr<T1> &f13(std::shared_ptr<T1> &dst, std::shared_ptr<T1> &&src) {
  return dst.operator=(std::move(src));
}

// ---------------------------------------------------------------------------
// std::shared_ptr observers
// ---------------------------------------------------------------------------

template <typename T1> T1 &f14(const std::shared_ptr<T1> &a0) {
  return a0.operator*();
}

template <typename T1> T1 *f15(const std::shared_ptr<T1> &a0) {
  return a0.operator->();
}

template <typename T1> T1 *f16(const std::shared_ptr<T1> &a0) {
  return a0.get();
}

template <typename T1> long f17(const std::shared_ptr<T1> &a0) {
  return a0.use_count();
}

template <typename T1> bool f18(const std::shared_ptr<T1> &a0) {
  return a0.operator bool();
}

// ---------------------------------------------------------------------------
// std::shared_ptr modifiers
// ---------------------------------------------------------------------------

template <typename T1> void f19(std::shared_ptr<T1> &a0) { return a0.reset(); }

template <typename T1> void f20(std::shared_ptr<T1> &a0, T1 *a1) {
  return a0.reset(a1);
}

// ---------------------------------------------------------------------------
// std::shared_ptr comparison
// ---------------------------------------------------------------------------

template <typename T1>
bool f21(const std::shared_ptr<T1> &a0, const std::shared_ptr<T1> &a1) {
  return operator==(a0, a1);
}

template <typename T1>
bool f22(const std::shared_ptr<T1> &a0, const std::shared_ptr<T1> &a1) {
  return operator!=(a0, a1);
}

template <typename T1>
bool f23(const std::shared_ptr<T1> &a0, std::nullptr_t a1) {
  return operator==(a0, a1);
}

template <typename T1>
bool f24(const std::shared_ptr<T1> &a0, std::nullptr_t a1) {
  return operator!=(a0, a1);
}

// ---------------------------------------------------------------------------
// std::weak_ptr
// ---------------------------------------------------------------------------

template <typename T1> std::weak_ptr<T1> f25() { return std::weak_ptr<T1>(); }

template <typename T1> std::weak_ptr<T1> f26(const std::shared_ptr<T1> &a0) {
  return std::weak_ptr<T1>(a0);
}

template <typename T1> std::weak_ptr<T1> f27(const std::weak_ptr<T1> &a0) {
  return std::weak_ptr<T1>(a0);
}

template <typename T1>
std::weak_ptr<T1> &f28(std::weak_ptr<T1> &dst, const std::shared_ptr<T1> &src) {
  return dst.operator=(src);
}

template <typename T1>
std::weak_ptr<T1> &f29(std::weak_ptr<T1> &dst, const std::weak_ptr<T1> &src) {
  return dst.operator=(src);
}

template <typename T1> std::shared_ptr<T1> f30(const std::weak_ptr<T1> &a0) {
  return a0.lock();
}

template <typename T1> bool f31(const std::weak_ptr<T1> &a0) {
  return a0.expired();
}

template <typename T1> long f32(const std::weak_ptr<T1> &a0) {
  return a0.use_count();
}

template <typename T1> void f33(std::weak_ptr<T1> &a0) { return a0.reset(); }
