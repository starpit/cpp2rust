// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstddef>
#include <memory>

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

// template <typename T, typename... Args>
// std::unique_ptr<T> f8(Args&&... args) {
//     return std::make_unique<T>(std::forward<Args>(args)...);
// }

// Rust does not have variadic generics. We should consider writing specialized
// versions for make_unique with 1, 2, 3, etc arguments and translate the
// specialized versions.

template <typename T1, typename T2> std::unique_ptr<T1> f8(T2 &&a0) {
  return std::make_unique<T1>(std::move(a0));
}

// The lvalue overloads.  f8 only covers the rvalue spelling, so
// `std::make_unique<std::string>(s)` for an lvalue `s` -- which is what
// dsc/pcfg.cpp and dsc/superdsc.cpp do -- resolved to
// `std::make_unique(const std::string &)` / `std::make_unique(std::string &)`
// and matched nothing.  Both COPY the argument, exactly as C++ does: T's copy
// constructor is what make_unique forwards to.
template <typename T1, typename T2> std::unique_ptr<T1> f18(const T2 &a0) {
  return std::make_unique<T1>(a0);
}

template <typename T1, typename T2> std::unique_ptr<T1> f19(T2 &a0) {
  return std::make_unique<T1>(a0);
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

// std::unique_ptr::release -- relinquish ownership WITHOUT destroying, and
// hand back the raw pointer. The caller becomes responsible for the object.
//
// In the refcount model the object is an Rc the Option owns outright (the
// unique_ptr ctor from a raw pointer called Ptr::to_owned_opt, which dropped
// the leaked strong reference). Releasing puts it back into the leaked state
// -- std::mem::forget on the taken Option -- so every raw pointer get() handed
// out earlier stays upgradeable, which is exactly what C++ guarantees.
//
// The returned Ptr is a StackSingle handle, the same kind get() returns, so
// re-owning it through std::unique_ptr<T>(p) panics loudly rather than
// silently double-owning. PtrKind::HeapSingle is pub(crate) in libcc2rs, so a
// rule body cannot build one; that is the same limitation get() already has.
template <typename T1> T1 *f16(std::unique_ptr<T1> &o) { return o.release(); }

// `std::unique_ptr<T>(nullptr)`.  Distinct from the default constructor (f10):
// it is a one-argument CXXConstructExpr, so f10's `unique_ptr()` never matches
// and the call falls back to a mangled placeholder.  As in rules/shared_ptr,
// the std::nullptr_t parameter has no Rust counterpart and is dropped.
template <typename T1> std::unique_ptr<T1> f17(std::nullptr_t a0) {
  return std::unique_ptr<T1>(a0);
}

// Two-argument std::make_unique. Mirrors rules/shared_ptr's f5/f6/f7 exactly,
// including the reason there are three: `Args&&...` deduces a different
// signature for rvalues, lvalues and const lvalues, so each value category
// needs its own rule or the call matches nothing and falls back to an
// undefined `libcc2rs::make_unique_<model>`.
//
// Rust has no variadic generics, so the target cannot call an arbitrary
// constructor; it builds T1 with `From<(T2, T3)>`, which the converter now
// emits for every translated constructor of arity >= 2 (Converter::
// AddFromTraits). Before that impl existed this rule could only have produced
// a type error, which is why shared_ptr's src.cpp documented the gap instead.
template <typename T1, typename T2, typename T3>
std::unique_ptr<T1> f20(T2 &&a0, T3 &&a1) {
  return std::make_unique<T1>(std::move(a0), std::move(a1));
}

template <typename T1, typename T2, typename T3>
std::unique_ptr<T1> f21(T2 &a0, T3 &a1) {
  return std::make_unique<T1>(a0, a1);
}

template <typename T1, typename T2, typename T3>
std::unique_ptr<T1> f22(const T2 &a0, const T3 &a1) {
  return std::make_unique<T1>(a0, a1);
}

// Zero-argument std::make_unique. rules/shared_ptr has had the corresponding
// f1 all along; unique_ptr's f10/f11 are the `unique_ptr<T>()` CONSTRUCTOR,
// whose resolved signature is different, so `std::make_unique<T>()` matched
// nothing and fell back to an undefined `libcc2rs::make_unique_<model>`.
// Unlike the constructor it is NOT None -- it default-constructs a T.
template <typename T1> std::unique_ptr<T1> f23() {
  return std::make_unique<T1>();
}
