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

// ---------------------------------------------------------------------------
// PER-ARITY `std::function` KEYS.  Arities 1..5, because that is what the corpus
// uses: counted by extracting every `std::function<...>` spelling from dt_src with a
// depth-aware scanner and splitting the arrow-parens on depth-0 commas --
// 0:9  1:31  2:16  3:17  4:2  5:7 occurrences, max arity 5, none above.
// Unblocked by 37b35e6 (mapper.cpp returns nullopt on an empty capture), which is what
// made a per-arity key able to coexist with the nullary `t2`.
// Lower-arity keys DO also match a higher-arity instantiation (the last placeholder
// scans to the closing paren and swallows the remaining depth-0 commas), but
// `search()` (mapper.cpp:429-437) breaks the tie by PREFERRING THE LONGER src, and
// src length is monotone in arity, so the exact-arity key always wins.  That is why
// every arity in use must be present: a gap would be silently swallowed by the next
// one down.

template <typename T1, typename T2> using t3 = std::function<T1(T2)>;

template <typename T1, typename T2> T1 f6(const std::function<T1(T2)> &a0, T2 a1) {
  return a0.operator()(a1);
}

template <typename T1, typename T2, typename T3> std::function<T1(T2)> f7(T3 a0) {
  return std::function<T1(T2)>(a0);
}

template <typename T1, typename T2, typename T3> using t4 = std::function<T1(T2, T3)>;

template <typename T1, typename T2, typename T3> T1 f8(const std::function<T1(T2, T3)> &a0, T2 a1, T3 a2) {
  return a0.operator()(a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4> std::function<T1(T2, T3)> f9(T4 a0) {
  return std::function<T1(T2, T3)>(a0);
}

template <typename T1, typename T2, typename T3, typename T4> using t5 = std::function<T1(T2, T3, T4)>;

template <typename T1, typename T2, typename T3, typename T4> T1 f10(const std::function<T1(T2, T3, T4)> &a0, T2 a1, T3 a2, T4 a3) {
  return a0.operator()(a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5> std::function<T1(T2, T3, T4)> f11(T5 a0) {
  return std::function<T1(T2, T3, T4)>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5> using t6 = std::function<T1(T2, T3, T4, T5)>;

template <typename T1, typename T2, typename T3, typename T4, typename T5> T1 f12(const std::function<T1(T2, T3, T4, T5)> &a0, T2 a1, T3 a2, T4 a3, T5 a4) {
  return a0.operator()(a1, a2, a3, a4);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> std::function<T1(T2, T3, T4, T5)> f13(T6 a0) {
  return std::function<T1(T2, T3, T4, T5)>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> using t7 = std::function<T1(T2, T3, T4, T5, T6)>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> T1 f14(const std::function<T1(T2, T3, T4, T5, T6)> &a0, T2 a1, T3 a2, T4 a3, T5 a4, T6 a5) {
  return a0.operator()(a1, a2, a3, a4, a5);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> std::function<T1(T2, T3, T4, T5, T6)> f15(T7 a0) {
  return std::function<T1(T2, T3, T4, T5, T6)>(a0);
}

// ---------------------------------------------------------------------------
// `llvm::function_ref<R()>` -- a NON-OWNING type-erased callable REFERENCE.
// Harvested with `-verbose` off Ktdp/KtdpTypes.cpp; 10 asks, all `result: None`,
// all `searched as: llvm::function_ref<mlir::InFlightDiagnostic ()>`, so the key is
// the ARROW SHAPE `llvm::function_ref<T1 ()>` (same reason as `std::function`
// above: a bare `llvm::function_ref<T1>` would bind T1 to a C++ FUNCTION TYPE,
// which has no model in types_).
//
// ⛔ THE MODEL IS A BORROW, NOT A `Box`.  `function_ref` is documented as never
// stored and the corpus agrees -- every occurrence in bucket A is a PARAMETER,
// zero fields, zero return positions.  An owning `Box<dyn Fn...>` would impose
// `'static` plus an allocation and would not compile at the call site, so the
// faithful model is `Option<&'a (dyn Fn() -> T1 + 'a)>`.
//
// This is the FIRST rule in the tree carrying a non-`'static` lifetime.  It is
// only writable because 63546ba2 taught the converter to collect `'`-binders out
// of the rendered parameter/return spellings and declare them after the function
// name; before that fix every emitted signature carried an UNDECLARED `'a`
// (rustc E0261) and the key had to be reverted.
// RESTATED, NOT #included: cpp-rule-preprocessor compiles this file with a fixed
// flag set that cannot reach an LLVM tree (same reason as rules/support,
// rules/stringref, rules/twine).  Declared for real at
// llvm/ADT/STLFunctionalExtras.h:36.
namespace llvm {
template <typename Fn> class function_ref;
// llvm/ADT/STLFunctionalExtras.h:36
template <typename Ret, typename... Params> class function_ref<Ret(Params...)> {
public:
  function_ref();
};
} // namespace llvm

template <typename T1> using t8 = llvm::function_ref<T1()>;
