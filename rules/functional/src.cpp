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

// `std::function<R()>` -- a nullary type-erased callable.
//
// The KEY IS PARTIALLY GENERIC ON THE RETURN TYPE ONLY.  A fully generic
// `std::function<T1>` (T1 = the whole function type) DOES bind, and then aborts in
// mapper.cpp:835 `Type is not present in types_`, because a C++ FUNCTION TYPE
// (`int (int)`, `std::unique_ptr<mlir::Pass> ()`) has no model in types_ at all.
// Writing the arrow shape `T1 ()` instead means every template parameter the key
// mentions is an ordinary modelled type, so the same abort cannot happen.
//
// The key the converter searches for has the DEFAULTED DELETER ELIDED:
//     search type std::function<std::unique_ptr<mlir::Pass> ()>
// while the diagnostic prints the 2-arg `std::default_delete` form.  Harvested with
// `-verbose` off dataflow-scheduler/lib/Pipeline.cpp; do not write the printed form.
template <typename T1> using t2 = std::function<T1()>;

// `R std::function<R()>::operator()() const`.  MUST be written in CALL form here:
// the infix `a0()` records nothing at all, silently.
template <typename T1> T1 f4(const std::function<T1()> &a0) {
  return a0.operator()();
}
