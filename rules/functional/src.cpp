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
  // ⛔ THE THREE-ARY CTOR.  Real LLVM declares this as
  //     template <typename Callable> function_ref(Callable &&callable,
  //         std::enable_if_t<...> * = nullptr, std::enable_if_t<...> * = nullptr)
  // i.e. the Callable plus TWO DEFAULTED SFINAE POINTER PARAMETERS.  Both
  // `enable_if_t`s resolve to `void`, so clang renders the signature -- and the
  // converter's search key -- as `(Callable &&, void *, void *)`, and the recorder
  // WRITES THE DEFAULTED ARGUMENTS OUT AT THE CALL SITE.  Harvested off
  // Ktdp/KtdpTypes.cpp with `-verbose`:
  //   search expr void llvm::function_ref<mlir::InFlightDiagnostic ()>::function_ref(
  //       (lambda at .../Ktdp/KtdpTypes.cpp:_:_) &&, void *, void *), result:
  //   None
  // ⭐ SO A ONE-ARGUMENT CTOR RULE IS DEAD ON ARRIVAL: the key must restate BOTH
  // defaulted pointer parameters or it cannot match.  They are declared here WITHOUT
  // defaults and passed explicitly in the rule body below, which renders identically.
  template <typename Callable>
  function_ref(Callable &&callable, void * = nullptr, void * = nullptr);
  // ⛔ THE CALL OPERATOR.  IT WAS MISSING FROM THIS RESTATED CLASS UNTIL 2026-09-29,
  // and the omission was not cosmetic: t8..t13 were TYPE KEYS WITH NO METHOD KEY,
  // which is STRICTLY WORSE THAN NO KEY AT ALL.  A type key routes the receiver away
  // from the loud `no rule for type` path, and the converter then reaches EmitCall's
  // terminal `else` (converter.cpp:5163) and aborts
  //   `unsupported call operator has no rule: ... (the receiver's mapped VALUE would
  //    be emitted in callee position, e.g. `0(v)`)`
  // -- 15/15 of the gating TUs measured at 31edbf97.  (Without the guard the SAME
  // shape emits `0(v)` silently; that is the `std::hash` 427-site precedent.)
  // Real LLVM declares it at llvm/ADT/STLFunctionalExtras.h:68 as
  //     Ret operator()(Params ...params) const
  // i.e. parameters BY VALUE and the method `const`, which is exactly what the
  // refusal text prints:
  //     `void llvm::function_ref<void (mlir::Value, llvm::StringRef)>
  //          ::operator()(mlir::Value, llvm::StringRef) const`
  Ret operator()(Params...) const;
};
} // namespace llvm

template <typename T1> using t8 = llvm::function_ref<T1()>;

// PER-ARITY SIBLINGS OF t8.  Same BORROW model, same reasoning; the only thing
// that changes is the number of C++ call parameters.  Harvested off the bucket-A
// rc=0 corpus: `llvm::function_ref<void (mlir::Value, llvm::StringRef)>` alone is
// 41 anchored emissions, and 1/3/4/5-parameter spellings account for the rest
// (`void (mlir::OpBuilder &, mlir::Location, mlir::Value[, mlir::ValueRange[, bool]])`,
// `llvm::LogicalResult (dataflowir::gen::irTy, ...)`,
// `std::unique_ptr<...> (const mlir::ktdf::arch::Device &)`).
// ⛔ THE ARROW SPELLING IS LOAD-BEARING: a bare `llvm::function_ref<T1>` would bind
// T1 to a C++ FUNCTION TYPE, which has no model in types_.  Same reason the
// `std::function` keys t2..t7 above are spelled with arrows.
// The BORROW (not `Box`) model and the non-`'static` lifetime are justified at t8;
// the precondition was re-checked for these arities too -- every corpus occurrence
// is a PARAMETER, so no `'a` ever reaches a field, alias or return position.

template <typename T1, typename T2> using t9 = llvm::function_ref<T1(T2)>;

template <typename T1, typename T2, typename T3> using t10 = llvm::function_ref<T1(T2, T3)>;

template <typename T1, typename T2, typename T3, typename T4> using t11 = llvm::function_ref<T1(T2, T3, T4)>;

template <typename T1, typename T2, typename T3, typename T4, typename T5> using t12 = llvm::function_ref<T1(T2, T3, T4, T5)>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> using t13 = llvm::function_ref<T1(T2, T3, T4, T5, T6)>;

// THE CONSTRUCTOR for the NULLARY `llvm::function_ref<T1 ()>`.  A TYPE key supplies no
// constructor, so without this the converter FABRICATES a call to
// `llvm_function_ref_..._::new_N(...)` -- a Rust function defined nowhere, at rc=0,
// carrying NO `Cpp2RustUnmapped_` anchor and therefore invisible to
// pin/no-placeholders.sh.  Measured on Ktdp/KtdpTypes.cpp: the anchored placeholder
// went 3 -> 0 when t8 landed while the fabricated ctor count did not move at all.
//
// ⚠️ `Callable &&` is an LVALUE inside the body, so the forward must be `std::move`d or
// the recorded key is a dead duplicate of the lvalue-reference spelling.
// The Callable has no nameable type (it is a closure type), so it is taken generically
// as T2 -- same as the `std::function` ctor f5 above.
template <typename T1, typename T2>
llvm::function_ref<T1()> f16(T2 &&a0) {
  return llvm::function_ref<T1()>(std::move(a0));
}

// PER-ARITY SIBLINGS OF f16, arities 1..5 -- the CONSTRUCTOR twins of t9..t13, the same
// way f17..f22 (below) are the CALL-OPERATOR twins.  ⛔⛔ THIS WAS THE ACTUAL GAP IN THE
// FAMILY, found 2026-09-29 by measuring the CURRENT pin (a0b0a707 + ir.v45, which already
// carries t8-t13 and f16-f22) against a real corpus TU:
// dataflow-scheduler/lib/Analysis/OperationTree.cpp (`mlir::function_ref<OperationTreeNode
// *(OperationTreeNode *)>`, an ARITY-1 instantiation) comes back `A complete rc=0 762`
// while its EMITTED `.rs` contains, verbatim, three sites of
//   llvm_function_ref_scheduler_OperationTreeNode_ptr_scheduler_OperationTreeNode_ptr__::new_1(...)
// -- a FABRICATED constructor, defined nowhere, invisible to every bucket census AND to
// no-placeholders.sh (same shape as f16's own note above, and the same class as the
// `std::hash<int>` 427-site precedent).  The type key (t9) and the call operator (f18)
// both matched -- `dyn Fn(` appears in the same file -- but NOTHING matched the
// CONSTRUCTOR for any arity above 0, because f16 alone only covers `T1()`.
// ⛔ So "one generic key" was true for the TYPE and the CALL, but NOT for the
// CONSTRUCTOR: that one is arity-specific and was only ever written once.
template <typename T1, typename T2, typename T3>
llvm::function_ref<T1(T2)> f23(T3 &&a0) {
  return llvm::function_ref<T1(T2)>(std::move(a0));
}

template <typename T1, typename T2, typename T3, typename T4>
llvm::function_ref<T1(T2, T3)> f24(T4 &&a0) {
  return llvm::function_ref<T1(T2, T3)>(std::move(a0));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
llvm::function_ref<T1(T2, T3, T4)> f25(T5 &&a0) {
  return llvm::function_ref<T1(T2, T3, T4)>(std::move(a0));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
llvm::function_ref<T1(T2, T3, T4, T5)> f26(T6 &&a0) {
  return llvm::function_ref<T1(T2, T3, T4, T5)>(std::move(a0));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
llvm::function_ref<T1(T2, T3, T4, T5, T6)> f27(T7 &&a0) {
  return llvm::function_ref<T1(T2, T3, T4, T5, T6)>(std::move(a0));
}

// ---------------------------------------------------------------------------
// PER-ARITY `llvm::function_ref` CALL OPERATORS -- the t8..t13 twins of the
// `std::function` keys f4/f6/f8/f10/f12/f14.  MUST be written in CALL form
// (`a0.operator()(...)`); the infix `a0(...)` records nothing at all, silently --
// the same trap documented at f4.
//
// ⭐ ARITY CENSUS, 2026-09-29, depth-aware scan of every `function_ref<...>`
// spelling in repos/dt_src + toolchain/gen-inc + the LLVM 22.1.3 include tree,
// splitting the arrow-parens on depth-0 commas (`arity_census.py`):
//     arity   spellings   files
//       0        14310      319     (13721 of them `::mlir::InFlightDiagnostic()`,
//                                    the generated op/type `verify` hooks)
//       1          479      224
//       2          357      127
//       3          200       60
//       4           83       29
//       5           32        7
//      10            1        1
// So arities 0..5 are the set in use, exactly matching the existing t8..t13, and
// EVERY ONE OF THEM IS WRITTEN HERE.  ⛔ A GAP WOULD BE SILENTLY SWALLOWED: the last
// placeholder of a lower-arity key scans to the closing paren and eats the remaining
// depth-0 commas, so e.g. the arity-2 key alone would also match an arity-3
// instantiation and bind T3 to `llvm::StringRef, bool`.  `search()`
// (mapper.cpp:429-437) breaks that tie by PREFERRING THE LONGER src, and src length
// is monotone in arity, so the exact-arity key always wins -- but only if it exists.
// That is the identical argument recorded for t3..t7 and for t9..t13 above.
// ⭐ The arity-0 key is immune to the swallow by construction: its literal is `()`,
// which requires EMPTY parens and so cannot match any instantiation with arguments.
// ⛔ THE LONE ARITY-10 SPELLING IS DELIBERATELY NOT COVERED.  It is
// `void (IRBuilderBase &, Value *, Value *, Value *, Align, AtomicOrdering,
//  SyncScope::ID, Value *&, Value *&, Instruction *)` -- one occurrence, in LLVM's
// own AtomicExpand header, with no type key (t8..t13 stop at 5) and no corpus use.
// With no type key it stays on the LOUD path, which is the correct outcome.
//
// ⚠️ WHY THE BODY MAY `unwrap`, stated here and not in the body because a rule body's
// COMMENTS AND STRING LITERALS ARE INLINED INTO THE EMITTED `.rs`.  The model is
// `Option<&'a dyn Fn..>` only because a TYPE key must have a default value and t8's
// is `None`; `llvm::function_ref` is non-owning and is ALWAYS callable when invoked,
// so the only way a `None` reaches one of these bodies is a default-constructed
// `function_ref` being called -- which in C++ dereferences a null callback pointer and
// is UNDEFINED BEHAVIOUR.  A `panic` is therefore a strictly LOUDER and faithful
// rendering of that C++ semantics, not a papering-over, and it is the same choice
// f4..f14 already make for `std::function` (where an empty `std::function` throws
// `std::bad_function_call`).

template <typename T1> T1 f17(const llvm::function_ref<T1()> &a0) {
  return a0.operator()();
}

template <typename T1, typename T2> T1 f18(const llvm::function_ref<T1(T2)> &a0, T2 a1) {
  return a0.operator()(a1);
}

template <typename T1, typename T2, typename T3> T1 f19(const llvm::function_ref<T1(T2, T3)> &a0, T2 a1, T3 a2) {
  return a0.operator()(a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4> T1 f20(const llvm::function_ref<T1(T2, T3, T4)> &a0, T2 a1, T3 a2, T4 a3) {
  return a0.operator()(a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5> T1 f21(const llvm::function_ref<T1(T2, T3, T4, T5)> &a0, T2 a1, T3 a2, T4 a3, T5 a4) {
  return a0.operator()(a1, a2, a3, a4);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> T1 f22(const llvm::function_ref<T1(T2, T3, T4, T5, T6)> &a0, T2 a1, T3 a2, T4 a3, T5 a4, T6 a5) {
  return a0.operator()(a1, a2, a3, a4, a5);
}
