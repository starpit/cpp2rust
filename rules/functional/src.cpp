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
