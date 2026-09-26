// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// mlir::Operation / Block / Region / Value / Type / Attribute
//   -> THE .td-GENERATED RUST MODEL IN `dataflowir-gen`.
//
// WHY THIS MODULE EXISTS
// ---------------------------------------------------------------------------
// The port does NOT translate MLIR.  `dataflowir-gen` already carries a model of
// these IR shapes, generated from the same TableGen the C++ reads, and verified
// byte-exact against 9,075 lines of real DataflowIR.  A TYPE rule is the first
// thing a translation needs: the converter aborts on an unmapped record type
// before it ever reaches a member call, so this module maps the SIX types and
// deliberately maps no members at all (see the absence note at the bottom).
//
//     mlir::Operation -> dataflowir_gen::fmt::OpInst     (fmt.rs:391)
//     mlir::Block     -> dataflowir_gen::fmt::Block      (fmt.rs:435)
//     mlir::Region    -> dataflowir_gen::fmt::Region     (fmt.rs:444)
//     mlir::Value     -> dataflowir_gen::ir::Value       (ir.rs:21)
//     mlir::Type      -> dataflowir_gen::ir::Ty          (ir.rs:37)
//     mlir::Attribute -> dataflowir_gen::ir::Attr        (ir.rs:466)
//
// WHY THE DECLARATIONS BELOW ARE LOCAL
// ---------------------------------------------------------------------------
// The reason rules/raw_ostream, rules/stringref and rules/ilist all give at
// length: cpp-rule-preprocessor compiles this file with a fixed flag set, and
// reaching real MLIR headers would need an absolute -I into whatever MLIR tree
// the target project built.  A type rule needs only the class NAME, so the
// declarations are deliberately empty -- no member is declared, because no
// member rule is written, and a declared-but-unmapped member records nothing.
//
// WHAT THIS MODULE DELIBERATELY DOES NOT DO -- read tgt_unsafe.rs' header and
// the report for the full absence list.  In one line: every MEMBER is absent.
// Nothing in the rule tree maps `getOperations()`, `begin()/end()`, the range
// iterators, `getType()`, `getDefiningOp()` or any builder.  Absent is the
// correct state for them: the converter then ABORTS loudly on the call site
// instead of emitting a body that compiles and lies.
// ---------------------------------------------------------------------------

#include <cstddef>

namespace mlir {

class Operation;

class Block {};

class Region {};

// `Value`, `Type` and `Attribute` are HANDLES in real MLIR -- each wraps one
// pointer into the uniquer/the defining op, and each is DEFAULT-CONSTRUCTIBLE to
// a null handle.  That default IS reachable from ported code (`Value v;`,
// `Type()`, a null return), which is why the target files' `init` for these
// three is load-bearing in a way `Operation`'s is not; tgt_unsafe.rs records the
// sentinel each one uses and why the sentinel is unreachable as a real value.
class Value {
public:
  // mlir/include/mlir/IR/Value.h -- the SSA-identity comparison.
  //
  // TAKE THE PARAMETER BY CONST REFERENCE, not by value. This was written as
  // `operator==(Value rhs)` from MLIR's documented signature, and the resulting
  // key never matched: the converter's own diagnostic asks for
  //     bool mlir::Value::operator==(const mlir::Value &) const
  // because THIS toolchain's Value.h declares it that way. The rule was
  // committed, regenerated, and silently did nothing.
  //
  // And the convention is NOT uniform across the handles -- mlir::Attribute
  // really is by value (`bool mlir::Attribute::operator==(mlir::Attribute)
  // const`, f1, which does match). So the parameter form has to be read off the
  // diagnostic PER TYPE; it cannot be inferred from a sibling handle.
  bool operator==(const Value &rhs) const;
  bool operator!=(const Value &rhs) const;
};

class Type {
public:
  // Same const-reference form as Value, confirmed from the diagnostic rather
  // than assumed from Attribute's by-value spelling.
  bool operator==(const Type &rhs) const;
  bool operator!=(const Type &rhs) const;
};

class Attribute {
public:
  // mlir/include/mlir/IR/Attributes.h -- `bool operator==(Attribute other) const`.
  // A HANDLE COMPARISON: MLIR uniques attributes, so two Attributes are equal
  // exactly when they are the same uniqued instance, which is exactly when they
  // print the same -- and `ir::Attr`'s derived `PartialEq` (ir.rs:465) compares
  // the spelling/structure, so the two agree.  This is the DOMINANT missing key
  // in dxp_standalone: its call sites are inside tablegen-GENERATED verify()
  // bodies (Dataflow.h.inc:408, KTDF.h.inc:528), so every dialect TU needs it.
  bool operator==(Attribute rhs) const;

  // mlir/include/mlir/IR/Attributes.h -- `bool operator!=(Attribute other) const
  // { return !(*this == other); }`.  A SEPARATE rule key from the `==` above:
  // the converter keys on the resolved callee signature, and C++17 has no
  // rewriting of `!=` into `==`, so a TU spelling `a != b` consults this key and
  // no other.
  bool operator!=(Attribute rhs) const;

  // mlir/include/mlir/IR/Attributes.h -- `bool operator!() const { return !impl; }`,
  // the NULL-HANDLE TEST.  Distinct from `operator bool`: `!attr` resolves to this
  // member directly, not to a negation of the conversion.
  bool operator!() const;
};

// mlir/include/mlir/IR/Attributes.h -- a DISTINCT key from the member `==`
// above: a free ADL operator found on `mlir::StringAttr`, comparing a handle
// against `nullptr` (the null-handle test spelled `attr != nullptr`).  Declared
// locally for the same reason as everything else in this file.
class StringAttr {};

bool operator!=(StringAttr lhs, std::nullptr_t rhs);

// mlir/include/mlir/IR/BuiltinAttributes.h declares FOUR free comparisons for
// StringAttr in one block, whose own comment says they exist "to avoid the
// StringRef overloads from being chosen when not desirable":
//     inline bool operator==(StringAttr lhs, std::nullptr_t)
//     inline bool operator!=(StringAttr lhs, std::nullptr_t)
//     inline bool operator==(StringAttr lhs, StringAttr rhs)
//     inline bool operator!=(StringAttr lhs, StringAttr rhs)
// The second was already f2.  The other three are declared here because the
// StringAttr-vs-StringAttr `==` is the SINGLE LARGEST first-abort gate measured
// in dxp_standalone (21 of 68 sampled TUs, and 3 of the 6 TUs measured for this
// row), and because the `!=` and the nullptr `==` are separate rule keys that
// the same generated verify()/parse() bodies reach.
bool operator==(StringAttr lhs, std::nullptr_t rhs);
bool operator==(StringAttr lhs, StringAttr rhs);
bool operator!=(StringAttr lhs, StringAttr rhs);

// mlir/include/mlir/IR/AffineMap.h.  `mlir::AffineMap` is the SAME handle shape
// as Attribute: `AffineMap() : map(nullptr) {}`, one uniquer pointer, and
//     bool operator==(AffineMap other) const { return other.map == map; }
//     bool operator!=(AffineMap other) const { return !(other.map == map); }
// It is the #1 first-abort gate in dxp_standalone: 84 of the survey-v2 TSVs
// record `unmapped-type mlir::AffineMap`, i.e. the mapper had NO rule for the
// type at all, and 8 more record `mlir::AffineMap::operator!=(mlir::AffineMap)`
// (PropagationAnalysis.h:58).
//
// THE MODEL IS NOT INVENTED HERE.  dataflowir-gen already carries a faithful
// one, generated/hand-transliterated for the .td printer:
//     dataflowir-gen/src/ir.rs:313  /// `mlir::AffineMap` -- what
//                                   /// `AffineMap::get(nDims,nSymbols,results,ctx)` builds
//     dataflowir-gen/src/ir.rs:315  pub struct AffineMap { n_dims, n_symbols, results }
//     dataflowir-gen/src/ir.rs:322  pub fn get(n_dims, n_symbols, results) -> Self
//     dataflowir-gen/src/ir.rs:326  pub fn permutation(..)   // getPermutationMap
// with `AffineExpr` at ir.rs:124 as the result tree.
//
// ⛔ ONE DOCUMENTED LIMIT, ir.rs:119-123: the model is CONSTRUCTION ONLY and
// does NOT canonicalise.  MLIR flattens/constant-folds on construction and
// `simplifyAffineMap` is a real simplifier; this corpus DOES call it
// (SentientOps.cpp:2123, :2141, PropagationAnalysis.h:92).  Those keys are
// therefore deliberately NOT added here -- a `simplifyAffineMap` that returned
// its argument unchanged would compile and lie.  Only the handle-level
// operations, whose semantics the value model reproduces exactly, are mapped.
class AffineMap {
public:
  bool operator==(AffineMap rhs) const;
  bool operator!=(AffineMap rhs) const;
};

// mlir/include/mlir/IR/Value.h -- `bool operator==(Value other) const { return
// impl == other.impl; }`.  A single-site gate in the 68-TU sample, and the same
// handle comparison as Attribute's: a Value is a pointer to its defining
// op-result / block-argument, so two Values are equal exactly when they are the
// same SSA value.  `ir::Value` (ir.rs:21) carries the value's NAME including its
// `%` sigil, and a name is unique within a function in MLIR's own printer, so
// name+type equality is SSA identity here.

} // namespace mlir

// ---- type rules, and nothing else ----------------------------------------
using t1 = mlir::Operation;
using t2 = mlir::Block;
using t3 = mlir::Region;
using t4 = mlir::Value;
using t5 = mlir::Type;
using t6 = mlir::Attribute;
using t7 = mlir::StringAttr;
using t8 = mlir::AffineMap;

// ---- the two operator rules ----------------------------------------------
// The member `==` on mlir::Attribute.  Spelled `.operator==(...)` rather than
// `a == b` so the recorded callee is unambiguously the MEMBER, not some free
// candidate found by ADL.
bool f1(mlir::Attribute a, mlir::Attribute b) { return a.operator==(b); }

// The free `!=` against nullptr.  `nullptr` is passed through as a parameter
// (not written as a literal in the body) because a rule's parameter list must
// correspond to the CALLEE's parameter list -- the converter inlines the target
// at the call site with the call's own arguments.
bool f2(mlir::StringAttr a, std::nullptr_t b) { return mlir::operator!=(a, b); }

// ---- constructors --------------------------------------------------------
// WHY THESE ARE NEEDED, and why their absence was invisible.  A TYPE rule maps
// the type; it does NOT map `mlir::Attribute a;`.  With t6 alone the converter
// translates the declaration rc=0 and emits
//     let mut a: dataflowir_gen::ir::Attr = mlir_Attribute::new();
// i.e. a call on a MANGLED FALLBACK PATH that nothing defines, so every TU that
// default-constructs a handle translated cleanly and then failed to compile with
// `error[E0433]: cannot find module or crate mlir_Attribute`.  That is the
// rc=0-is-not-compiles class, caught at rule level.  The four keys below are the
// exact ones `-verbose` reports as `result: None` for a TU that declares and
// copies a handle:
//     void mlir::Attribute::Attribute()
//     void mlir::Attribute::Attribute(const mlir::Attribute &)
//     void mlir::StringAttr::StringAttr()
//     void mlir::StringAttr::StringAttr(const mlir::StringAttr &)
// The default ctors give the SAME null handle as t6/t7's `init` -- they must, or
// `Attribute a;` and a defaulted member would disagree.  The copy ctors are a
// plain clone: these are POINTER-SIZED handles into MLIR's uniquer, so a C++ copy
// duplicates the handle and both then compare equal to the original, which is
// exactly what `Attr::clone()` gives.
mlir::Attribute f3() { return mlir::Attribute(); }
mlir::Attribute f4(const mlir::Attribute &o) { return mlir::Attribute(o); }
mlir::StringAttr f5() { return mlir::StringAttr(); }
mlir::StringAttr f6(const mlir::StringAttr &o) { return mlir::StringAttr(o); }

// ---- the rest of the comparison family ------------------------------------
// Each is spelled in the UNAMBIGUOUS form (`.operator==(..)` for a member,
// `mlir::operator==(..)` for a free one) so the recorded callee cannot be some
// other ADL candidate.  Every one of these is real MLIR API, quoted above at its
// declaration site; nothing here is invented.
bool f7(mlir::StringAttr a, mlir::StringAttr b) { return mlir::operator==(a, b); }

bool f8(mlir::StringAttr a, mlir::StringAttr b) { return mlir::operator!=(a, b); }

bool f9(mlir::StringAttr a, std::nullptr_t b) { return mlir::operator==(a, b); }

bool f10(mlir::Attribute a, mlir::Attribute b) { return a.operator!=(b); }

bool f11(mlir::Attribute a) { return a.operator!(); }

// ---- mlir::AffineMap: the comparison family and the CONSTRUCTORS -----------
// A TYPE RULE ALONE IS NOT ENOUGH, measured: `rules/mlir` mapped
// `mlir::Attribute` and `mlir::Attribute a;` still lowered to
// `mlir_Attribute::new()`, which no rule defined -- the TU translated rc=0 and
// THEN failed to compile with `error[E0433]: cannot find module or crate
// mlir_Attribute`.  `PropagationAnalysis.h:99` is exactly that shape for this
// type (`AffineMap propagated_map_;`, a member with no initializer), so the
// default and copy constructors are mapped alongside the type.
mlir::AffineMap f12() { return mlir::AffineMap(); }
mlir::AffineMap f13(const mlir::AffineMap &o) { return mlir::AffineMap(o); }
bool f14(mlir::AffineMap a, mlir::AffineMap b) { return a.operator==(b); }
bool f15(mlir::AffineMap a, mlir::AffineMap b) { return a.operator!=(b); }

// ---- mlir::Value::operator== ----------------------------------------------
bool f16(mlir::Value a, mlir::Value b) { return a.operator==(b); }

bool f17(mlir::Value a, mlir::Value b) { return a.operator!=(b); }

bool f18(mlir::Type a, mlir::Type b) { return a.operator==(b); }

bool f19(mlir::Type a, mlir::Type b) { return a.operator!=(b); }
