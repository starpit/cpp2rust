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

// THE ONE-ARGUMENT ARROW SHAPE `std::function<T1 (T2)>` IS LEFT OUT DELIBERATELY.
// Measured 2026-09-27, both halves:
//  1. It does NOT rescue a user-typed argument.  On the only unary instantiation in
//     the 403-TU sweep -- `std::function<bool (const scheduler::PipelineTreeNode *)>`
//     in dataflow-scheduler/lib/Analysis/PipelineTree.cpp -- the key BINDS and then
//     aborts anyway: `unmapped type `const scheduler::PipelineTreeNode *` has no
//     model in types_, while mapping `std::function<bool (const ...PipelineTreeNode *)>`.
//     So binding an argument as T2 still requires that argument to have a model; the
//     arrow shape only helps when the thing with no model is the FUNCTION TYPE itself.
//  2. It REGRESSES the nullary key.  With t3 present, Pipeline.cpp (rc=0, 685 lines
//     with t2 alone) aborts with `unmapped type `` has no model in types_, while
//     mapping `std::function<std::unique_ptr<mlir::Pass> ()>` -- i.e. `T1 (T2)` also
//     matches the ZERO-argument instantiation, binding T2 to the EMPTY type.  The
//     matcher does not check arrow-shape arity.  Until that is fixed in the converter,
//     a unary key cannot coexist with the nullary one.

// `std::function<R()>::function(F)` -- the CONSTRUCTOR, needed because every
// registration site is `registerPass([]{ return createXPass(); })` and the harvested
// key renders the parameter as the CLOSURE TYPE via its own operator():
//   void std::function<std::unique_ptr<mlir::Pass> ()>::function(
//         std::unique_ptr<mlir::Pass> operator()() const)
// There is no nameable parameter type, so the parameter is taken generically as T2.
template <typename T1, typename T2> std::function<T1()> f5(T2 a0) {
  return std::function<T1()>(a0);
}
