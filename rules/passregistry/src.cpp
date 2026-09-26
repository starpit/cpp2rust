// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// mlir::registerPass -- the largest single undefined-name family in the emitted
// Rust that is a ONE-RULE fix: 482 errors across the 18-TU compile sample,
// spread over the per-TU numbered families registerPass_143 (816 references),
// _146, _149, _145, _147, _150, _144, _65, _148, _152.
//
// THE REAL DECLARATION
// ---------------------------------------------------------------------------
//   mlir/Pass/PassRegistry.h:142
//     void registerPass(const PassAllocatorFunction &function);
//   and PassAllocatorFunction is std::function<std::unique_ptr<Pass>()>.
//
// Nothing in dt_src calls it directly.  Every call site is the INLINED body of
// PassRegistration<T>'s constructor (PassRegistry.h:157, `registerPass(constructor)`),
// which is why the emitted Rust shows it as `registerPass_180(_function.as_pointer())`
// -- the `_function` name is that constructor's own parameter.
//
// WHY THE BODY IS A NO-OP, AND WHY THAT IS NOT A FUDGE
// ---------------------------------------------------------------------------
// registerPass returns void and its ONLY effect is to add an entry to MLIR's
// global pass registry.  That registry has exactly one consumer: the textual
// pipeline machinery (`mlir::parsePassPipeline` / `PassPipelineCLParser` /
// `MlirOptMain`), reached in dt_src only from the *-opt tools' command-line path
// (dbo/tools/dbo-opt/dbo-opt.cpp:169 `::registerPasses()`, dcc-standalone.cpp:18-21).
// The port does not implement that machinery -- it is on the far side of the
// MLIR boundary this project deliberately does not cross -- so NO code the port
// emits can observe whether the entry was added.  A no-op is therefore
// observationally equivalent *for the program the port produces*.
//
// THIS BODY BECOMES WRONG the day the port gains a pipeline parser, because then
// a pass registered here would have to be findable by name.  Left as a comment
// rather than an unimplemented!() because unimplemented!() would abort at
// program START (registration happens in static initialisers and in main's first
// statements), turning 482 compile errors into an immediate runtime abort in
// every affected TU -- strictly worse, and it would hide the rest of the TU.
//
// WHAT IS NOT A NO-OP: THE ARGUMENT.
// ---------------------------------------------------------------------------
// C++ evaluates the argument.  At every real call site that argument is a
// non-trivial lambda-to-std::function conversion, and a rule body that simply
// omitted `a0` would DROP that conversion -- the argument expression would never
// be emitted at all, since a rule body is inlined TEXTUALLY and an `aN` that
// appears nowhere is an argument that is never evaluated.  That is exactly the
// defect 1b4042e found in std::tie/std::ignore.  So the target body binds `a0`
// to `_` and drops it: evaluated exactly once, result discarded.
//
// KEYED GENERICALLY OVER THE PASS TYPE, ON PURPOSE.  `mlir::Pass` itself gets no
// type rule here (--opaque-namespace=mlir already gives it a handle, and a type
// rule would take precedence over that for every TU -- see rules/mlir/src.cpp on
// the ordering).  Writing `T1` for the pass type also keeps the TARGET files
// free of a converter-mangled name like `mlir_Pass`, which a rule module cannot
// spell: the emitted preamble is a fixed list and does not reproduce a rule
// module's own `use` lines, so any such name would be undefined at the use site.

#include <functional>
#include <memory>

namespace mlir {
class Pass;
void registerPass(const std::function<std::unique_ptr<Pass>()> &function);
} // namespace mlir

// mlir::Pass gets a type rule after all -- NOT to model a pass, but because the
// KEY the preprocessor records for f1 is the CONCRETE resolved declaration
// (`void mlir::registerPass(const std::function<std::unique_ptr<mlir::Pass> ()> &)`),
// with no T1 in it.  A generic target over a src key that carries no generic is
// `LLVM ERROR: Absent generic from src`, which poisons the whole IR tree at load
// time -- check-ir.sh caught exactly that on the first attempt here.  So the
// target must spell the argument type concretely, and to spell it at all the pass
// type needs a name the target file may use: u64, which is precisely the payload
// of the `--opaque-namespace=mlir` handle this replaces.
void f1(const std::function<std::unique_ptr<mlir::Pass>()> &a0) {
  // `return` a void call: cpp-rule-preprocessor requires every rule body to BE a
  // return statement ("ERROR: f1: body must be a return statement" otherwise), and
  // returning a void-typed expression is how the existing void rules spell it.
  return mlir::registerPass(a0);
}
