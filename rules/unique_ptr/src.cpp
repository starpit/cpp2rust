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

// ---------------------------------------------------------------------------
// SIX make_unique RULES OF OURS WERE DELETED HERE, as duplicates that upstream's
// argument-pack f8 (7896632) subsumes. They were f18/f19 (one lvalue and one
// const-lvalue argument) and f20/f21/f22/f23 (two arguments in each value
// category, and the zero-argument form).
//
// They existed because, before packs, `Args&&...` deduced a DIFFERENT signature
// per value category and arity, so each shape needed its own rule -- the reason
// rules/shared_ptr still carries f5/f6/f7. With f8 the preprocessor prints one
// key, `std::unique_ptr<T1> std::make_unique(&&...)`, for every arity and every
// value category; keeping ours put SEVEN rules on that one key and the converter
// aborted at load with "generic T2 declared but missing from src: f22".
//
// Upstream's one rule is not merely equivalent, it is better: it covers any
// arity, where ours stopped at two. Proven by running all seven shapes -- 0, 1
// and 2 arguments, rvalue, lvalue and const lvalue -- against clang-built C++ in
// both models: C++, unsafe and refcount all give
//   z=(-1,-1) o=(3,0) t=(4,10) l=(6,14) c=(8,18) q=111 cq=112
// so the constructor actually selected, and the doubling in P(int,int), are the
// ones C++ picks.
//
// f16 and f17 above are ours and are NOT in upstream, so this module is our file
// with the six rules removed rather than upstream's file taken wholesale.
// ---------------------------------------------------------------------------
