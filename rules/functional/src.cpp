// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <functional>

template <typename T1> using t1 = std::reference_wrapper<T1>;

template <typename T1> std::reference_wrapper<T1> f1(T1 &a0) {
  return std::reference_wrapper<T1>(a0);
}

template <typename T1> T1 &f2(const std::reference_wrapper<T1> &a0) {
  return a0.operator T1 &();
}

template <typename T1> T1 &f3(const std::reference_wrapper<T1> &a0) {
  return a0.get();
}

// ---------------------------------------------------------------------------
// std::plus
//
// A stateless functor.  It is only ever *named* (as a local's type, and as the
// deduced argument of a project template), then CALLED, so the Rust side needs
// no representation beyond something zero-sized, Default and Copy --
// PhantomData is exactly that.  operator() ignores the receiver entirely.
// ---------------------------------------------------------------------------

template <typename T1> using t2 = std::plus<T1>;

template <typename T1> std::plus<T1> f4() { return std::plus<T1>(); }

template <typename T1>
T1 f5(const std::plus<T1> &a0, const T1 &a1, const T1 &a2) {
  return a0.operator()(a1, a2);
}

// ---------------------------------------------------------------------------
// std::function<R()> -- the nullary, type-erased callable.
//
// This is the shape mlir's pass registration uses
// (`std::function<std::unique_ptr<mlir::Pass>()>`), and it needs no parameter
// pack: the zero-argument spelling `T1()` matches it exactly, so we stay well
// clear of the pack-marker arity collapse (see BLOCKER-pack-arity.md).
//
// Rust side: `Rc<dyn Fn() -> T1>`.  `operator()` is const on std::function, so
// `Fn` (not `FnMut`) is the honest bound, and `Rc` -- unlike `Box` -- is Clone,
// which is what makes the copy constructor representable.  Since the callable
// cannot mutate its own captures, sharing one allocation between copies is
// observationally identical to std::function's value semantics.
// ---------------------------------------------------------------------------

template <typename T1> using t3 = std::function<T1()>;

template <typename T1> std::function<T1()> f6() { return std::function<T1()>(); }

// construction from any callable (a lambda, a function pointer, a functor)
template <typename T1, typename T2> std::function<T1()> f7(T2 a0) {
  return std::function<T1()>(a0);
}

// copy construction
template <typename T1>
std::function<T1()> f8(const std::function<T1()> &a0) {
  return std::function<T1()>(a0);
}

// the call itself
template <typename T1> T1 f9(const std::function<T1()> &a0) {
  return a0.operator()();
}

// assignment from another callable, and from another std::function
template <typename T1, typename T2>
std::function<T1()> &f10(std::function<T1()> &a0, T2 a1) {
  return a0.operator=(a1);
}

template <typename T1>
std::function<T1()> &f11(std::function<T1()> &a0,
                         const std::function<T1()> &a1) {
  return a0.operator=(a1);
}
