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
// For `std::unique_ptr` / `std::default_delete`, which appear inside the
// fully-spelled `RegionRange` range-base key below.
#include <memory>
// For `std::string`, which appears inside the ListOption key (t67).  The key
// printer renders it `std::string`, not `std::__1::basic_string<...>`: verified,
// 55 recorded keys across the published IR tree spell it that way.
#include <string>

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
  // TAKE THE PARAMETER BY VALUE.  The previous text here claimed the
  // const-reference form was "confirmed from the diagnostic"; it was not -- it
  // was copied from Value above, and it made f18/f19 DEAD KEYS that matched
  // nothing for their whole lifetime.  Ground truth, read off the toolchain
  // header the corpus actually includes:
  //     llvm/LLVM-22.1.3-Linux-X64/include/mlir/IR/Types.h:93
  //       bool operator==(Type other) const { return impl == other.impl; }
  //     Types.h:94  bool operator!=(Type other) const
  //     Types.h:97  bool operator!() const { return impl == nullptr; }
  // and the converter's own diagnostic agrees: the work-queue rows g1183/g544
  // ask for `bool mlir::Type::operator==(mlir::Type) const` /
  // `operator!=(mlir::Type) const`, by value.  So the three handles use THREE
  // different spellings -- Value const&, Attribute by value, Type by value --
  // and each must be read off the header/diagnostic, never inferred.
  bool operator==(Type rhs) const;
  bool operator!=(Type rhs) const;
  // `Types.h:97  bool operator!() const { return impl == nullptr; }` -- the
  // NULL-HANDLE TEST, work-queue row g375 (6 TUs), the same shape as
  // Attribute's f11.  NOTE ON THE RENUMBERING RISK the previous slot flagged:
  // the fN keys are numbered by the ORDER OF THE FREE FUNCTIONS at the bottom of
  // this file, NOT by the order of declarations inside these restated classes.
  // Adding this member declaration here therefore renumbers nothing, and its
  // rule function is APPENDED AS f20 after f19 so f1..f19 keep their numbers and
  // both target files need no re-verification beyond the new tail entry.
  bool operator!() const;
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


// ---------------------------------------------------------------------------
// TYPES ADDED FROM THE FIRST REAL COMPILE MEASUREMENT (compile1.unsafe.rs).
// Every one of these is a `cannot find type` in the emitted Rust of
// dataflow-scheduler/lib/Dialect/KTDF/Utils/Utils.cpp -- the mangled fallback
// name the converter emits for an MLIR type with no rule.  Declarations are
// EMPTY for the reason the header gives: a type rule needs only the name, and
// declaring a member that has no rule records nothing.
//
// The ORDER of these declarations is load-bearing: tN in the target files is
// matched to the Nth class declared in this file.
// ---------------------------------------------------------------------------

// mlir/include/mlir/IR/BuiltinAttributes.h -- a sorted key->Attribute map.
// -> dataflowir_gen::ir::AttrDict (ir.rs:559), which is literally documented as
// "`mlir::DictionaryAttr`" and is a BTreeMap because MLIR sorts by key on
// construction and the printer walks that order.
class DictionaryAttr {};

// mlir/include/mlir/IR/BuiltinAttributes.h.  ir::Attr::Int(i64, Ty) (ir.rs:470)
// is documented as "`mlir::IntegerAttr`".  There is no distinct Rust type for
// the SUBCLASS, so the map is to the whole `Attr` enum: that WIDENS the C++
// static guarantee (an `IntegerAttr` is known integral; an `Attr` is not).  The
// widening is sound for storage and comparison and is the same choice already
// made for mlir::Attribute (t6).
class IntegerAttr {};

// ir.rs:491 `Attr::I32Array` is documented as "`mlir::ArrayAttr` of IntegerAttrs".
// Same widening as IntegerAttr, and for the same reason.
class ArrayAttr {};

// ir.rs:480/482 -- `Attr::AffineMapAlias` / `Attr::AffineMap` are both
// documented as `mlir::AffineMapAttr` spellings.  Same widening.
class AffineMapAttr {};

// mlir/include/mlir/IR/OpDefinition.h -- `struct EmptyProperties {};`, the
// properties type ODS gives an op with no `let arguments` properties.  It has NO
// MEMBERS, so the Rust unit type is not an approximation of it, it is the same
// thing.  (The one difference, sizeof 1 vs 0, is only observable through
// pointer identity in an array of them, which ODS never builds.)
struct EmptyProperties {};

// mlir/include/mlir/IR/ValueRange.h.  BORROWED VIEWS in C++ -> OWNING Vec here;
// see the cost note in tgt_unsafe.rs.
class OperandRange {};
class ResultRange {};
class ValueRange {};

// mlir/include/mlir/IR/Region.h -- a view over an op's regions.
class RegionRange {};

// mlir/include/mlir/IR/OperationName.h -- a HANDLE to the registered operation
// info (`Impl *`, nullable, uniqued per name).  dataflowir_gen's
// `TdOpDef` (generated, re-exported at lib.rs:72) is exactly that record: it
// carries def_name/base/mnemonic/traits and the ODS `arguments` -- which is what
// `OperationName::getAttributeNames()` reads.  The handle is therefore
// `Option<&'static TdOpDef>`, and `None` is the null handle a
// default-constructed `OperationName` is.  No MEMBER is mapped, so
// `getAttributeNames()` still aborts loudly rather than lying.
class OperationName {};

// mlir/include/mlir/IR/OpDefinition.h:272 -- `class OpFoldResult : public
// PointerUnion<Attribute, Value>`.  THE RESULT OF FOLDING: either a constant
// `Attribute` or an existing SSA `Value`, never both and never a third thing.
//
// WHY IT IS THE HIGHEST-VALUE TYPE LEFT IN THIS MODULE.  It is not reached
// directly; it is reached as the ELEMENT of a container.  Once rules/smallvector
// maps the SmallVector CRTP family, the mapper RECURSES into the element type of
// `llvm::SmallVector<mlir::OpFoldResult, _>` (every `getMixed*` accessor ODS
// generates for an op with `$static_sizes`/`$dynamic_sizes` returns one), and an
// element with no model is NOT a countable mangled name -- it is a hard
// `mapper.cpp:722` abort that emits nothing.  Measured: with rules/smallvector
// published into pin/ir, the reference TU (KTDF/Utils/Utils.cpp) goes rc=134 with
// zero lines emitted, on exactly this type.
//
// ⛔ IT IS A UNION AND THE MODEL MUST BE A SUM.  Mapping it to `ir::Attr` alone
// or `ir::Value` alone would compile and would be silently wrong: folding really
// does return both, so either collapse makes half of all fold results read as the
// wrong kind, and no probe distinguishes them.  See tgt_unsafe.rs' t28 for the
// representation actually used and the note that this rule INTRODUCES it -- the
// crate has no OpFoldResult type (`grep -rnE 'OpFoldResult' dataflowir-gen/src`
// = 0 hits), but both ALTERNATIVES are already modelled here (t6 Attribute ->
// ir::Attr, t4 Value -> ir::Value), so the sum is built out of existing models
// rather than invented.
//
// NO MEMBER IS MAPPED, deliberately: `is<Attribute>()`, `get<Value>()`,
// `dyn_cast` and `PointerUnion`'s conversions all stay absent, so a TU that
// INSPECTS a fold result still aborts loudly instead of guessing a discriminant.
// This rule is only about making the CONTAINER mappable.
class OpFoldResult {};

// ---------------------------------------------------------------------------
// TYPES ADDED FROM THE `--mangle-unmapped` TRIAGE PASS (commit bac7590) over
// dataflow-scheduler/lib/Dialect/KTDF/Utils/Utils.cpp, plus the first-abort
// ranking over the 68-TU dxp sample.  Same rule as everything above: a type rule
// needs only the NAME, no member is declared because no member rule is written,
// and anything that could not be GROUNDED in the dataflowir-gen model was left
// out rather than guessed (the absence list is in tgt_unsafe.rs).
// ---------------------------------------------------------------------------

// mlir/include/mlir/IR/BuiltinAttributes.h -- `mlir::BoolAttr`.
// dataflowir-gen models it EXPLICITLY: ir.rs:471 `Attr::Bool(bool)` is documented
// as "`mlir::BoolAttr` -- true / false, with no type suffix".  There is no
// distinct Rust type for the subclass, so the map is to the whole `Attr` enum --
// the same WIDENING already made for IntegerAttr (t10), ArrayAttr (t11) and
// AffineMapAttr (t12), sound for storage and comparison, losing only the C++
// static guarantee that the handle is a boolean attribute.
class BoolAttr {};

// mlir/include/mlir/IR/Attributes.h -- `class NamedAttribute { StringAttr name;
// Attribute value; }`.  ONE ENTRY OF A DICTIONARY, and that is exactly what
// dataflowir-gen models: `ir::AttrDict = BTreeMap<String, Attr>` (ir.rs:559) is
// documented as `mlir::DictionaryAttr` and its ENTRY TYPE is (key, Attr).  So the
// pair is the crate's own entry shape, not a wrapper invented here.
// The key is carried as a plain `String` rather than a second `Attr`, matching
// AttrDict: MLIR's name is a StringAttr but a dictionary key is only ever its
// text, and `print_attr_dict` (ir.rs) walks `String` keys.
class NamedAttribute {};

// mlir/include/mlir/IR/Dialect.h -- the registered dialect record.
// GROUNDED, and not by analogy: dataflowir-gen's .td parser produces a real model
// of a dialect record, `td_ext::TdDialect` (td_ext.rs:22), built from
// `def X_Dialect { let name = ... }` (td_ext.rs:364) and carrying def_name/name/
// cpp_namespace/file/line.  That is the same data `mlir::Dialect` holds for the
// printer's purposes (`getNamespace()` is `name`).
class Dialect {};

// mlir/include/mlir/IR/MLIRContext.h -- the UNIQUER/ALLOCATOR.  It has NO
// analogue in dataflowir-gen and cannot have one: the crate models printed IR,
// not the storage MLIR interns it in.
//
// MAPPED AS AN OPAQUE UNIT, AND ONLY BECAUSE NOTHING READS THROUGH IT.  Measured
// before mapping, on the emitted Rust of the reference TU: every one of the 35
// occurrences of the mangled name is the same shape,
//     let mut ___args_1: *mut mlir_MLIRContext = ...
// i.e. a context POINTER threaded through a call and never dereferenced --
//     grep -c 'mlir_MLIRContext' base.rs                     = 35
//     grep 'mlir_MLIRContext' base.rs | grep -v '\*mut ...'  = 0 lines
// so no member of it is reached in this TU.  DELIBERATELY NO MEMBER RULE IS
// WRITTEN: a context whose `getLoadedDialect()` silently did nothing would be the
// exact silent-wrongness this port exists to prevent, so any call on it still
// ABORTS loudly.  If a future TU calls a method here, the honest answer is a real
// model, not a member rule on a unit.
class MLIRContext {};

// mlir/include/mlir/IR/Value.h:28 -- `class OpResult : public Value`.  It IS a
// Value (a narrowing subclass that additionally knows its owner op and result
// number), so `ir::Value` (ir.rs:21) is the faithful representation and the only
// thing lost is the static narrowing -- the same widening direction as t10-t12.
class OpResult {};

// mlir/include/mlir/IR/OpDefinition.h:100 -- `class OpState`, the base of every
// ODS-generated op class: one `Operation *`.  So it maps where `mlir::Operation`
// maps, `fmt::OpInst`.
//
// ⛔ NO COMPARISON OR IDENTITY OPERATION IS MAPPED ON THIS TYPE, DELIBERATELY,
// AND NOBODY SHOULD ADD ONE.  A C++ OpState is a HANDLE: two OpStates are the
// same op exactly when their `Operation *` are equal.  `fmt::OpInst` is a VALUE
// (an op's printed content), so a derived `PartialEq` on it would say two
// distinct operations with identical content ARE the same operation -- silently
// wrong, and invisible to any probe that does not build two identical ops.  The
// TYPE is mapped so TUs can name it (33 rustc errors in the reference TU, and it
// is a first-abort gate); equality is NOT, and stays absent so that a comparison
// site aborts loudly instead.
class OpState {};

// mlir/include/mlir/IR/Value.h:490 -- `mlir::BlockArgument`, a `Value` subclass
// whose defining "op" is a block.  It IS an `mlir::Value` by inheritance, so it
// gets t4's representation, `ir::Value` (ir.rs:21 -- `{ name: String, ty: Ty }`).
// The widening it costs is stated: `ir::Value` does not record that the value is
// a block argument nor its argument NUMBER, so `getArgNumber()`/`getOwner()` are
// NOT mapped and still abort loudly rather than returning 0.
class BlockArgument {};

// DECLARED ONLY SO THE CONCRETE RANGE-BASE KEYS BELOW CAN BE SPELLED.  Neither
// of these gets a `using tN =`, so NEITHER IS MAPPED -- see the note on the
// IndexType/MemRefType group above for why a bare declaration registers nothing.
//
// ⛔ `mlir::OpOperand` IS AN HONEST REFUSAL AND MUST STAY ONE.  It is MLIR's USE
// EDGE (`IROperand`), not a value: it records that operand slot N of some op uses
// some Value.  `grep -rn OpOperand dataflowir-gen/src` = 0 hits, and mapping it to
// `ir::Value` would CONFLATE a use with the value it uses -- silently wrong at
// every `getOwner()`/`getOperandNumber()`.  It appears below only as a template
// ARGUMENT of a CONCRETE key, which needs no model for the argument.
class OpOperand;

// The five concrete MLIR type handles that appear as `TypedValue` arguments in
// the reference TU.  They are declared ONLY so the five concrete `TypedValue`
// instantiations below can be spelled; NO type rule is registered for any of
// them, because none of them appears as a type in its own right at any use site
// measured (they are absent from the triage list).  Declaring without a `using
// tN =` registers nothing -- see the note in the header.
class IndexType {};
class MemRefType {};
class RankedTensorType {};
namespace ktdf {
class TokenType {};
class FifoSlotType {};
} // namespace ktdf

// `mlir::sdscbundle::InputArgType` -- the SIXTH concrete `TypedValue` element type
// (t45).  Restated as an EMPTY TAG for exactly the reason t30-t34's elements are:
// only the SPELLING matters for the key, the element type itself is never mapped
// on its own, and TypedValue's model discards it (see t45).
namespace sdscbundle {
class InputArgType {};
} // namespace sdscbundle

namespace detail {
// mlir/include/mlir/IR/Value.h -- the storage behind an `OpResult`.  Declared
// ONLY to spell the ResultRange base key; NOT mapped, for the same reason
// `OpOperand` is not: it is implementation storage, absent from the crate.
class OpResultImpl;

// mlir/include/mlir/IR/Value.h:106 -- `TypedValue<T>`, a `Value` whose static
// type is known to be `T`.  It derives from `mlir::Value`, so it maps where t4
// maps: `dataflowir_gen::ir::Value` (ir.rs:21).
// ⛔ MAPPED AT THE FIVE CONCRETE INSTANTIATIONS, NEVER AS A TEMPLATE, and that
// is a MEASURED constraint for exactly the reason recorded on t26 below: a
// generic `template<typename T1> using tN = mlir::detail::TypedValue<T1>` makes
// the converter map the template ARGUMENT too, and the first argument without a
// rule turns a countable mangled name into a hard abort
//   mapper.cpp:722 Assertion `0 && "Type is not present in types_"'
// This rule was dropped TWICE for that reason before being keyed concretely.
// What the widening costs, stated: `ir::Value` carries its type DYNAMICALLY in
// `ty: Ty`, so the STATIC guarantee `T` encodes is not represented; nothing that
// depends on the static type (e.g. `.getType()` returning a `T` rather than a
// `Type`) is mapped, and such a site aborts loudly.
template <typename T>
class TypedValue {};

// mlir/include/mlir/IR/BuiltinAttributes.h:760 -- `DenseArrayAttrImpl<T>`, the
// implementation base that `DenseI64ArrayAttr` (T = long) is a typedef of.
// ⛔ MAPPED AT THE CONCRETE `<long>` INSTANTIATION, NOT AS A TEMPLATE, AND THAT
// IS A MEASURED CONSTRAINT, NOT A STYLE CHOICE.  Written generically first
// (`template<typename T1> using t26 = ...<T1>`), the converter then has to map
// the ARGUMENT too and aborts rc=134 on the first argument that has no rule:
//   unsupported unmapped type `mlir::IndexType` has no model in types_, while
//   mapping `mlir::detail::TypedValue<mlir::IndexType>`
//   mapper.cpp:722 Assertion `0 && "Type is not present in types_"'
// i.e. a GENERIC type rule over MLIR types turns a countable mangled fallback
// into a hard abort.  `long` is a builtin and already has a model, so the
// concrete key is safe.  For the same reason `mlir::detail::TypedValue<T>` is
// NOT mapped at all -- see the absence list in tgt_unsafe.rs.  It is a member of the ONE uniqued
// `mlir::Attribute` hierarchy, and `ir::Attr` is the closed union of that
// hierarchy -- so this is the SAME widening as t10/t11/t12/t20, not a new kind of
// claim.  ⛔ What it costs, stated: `ir::Attr` has no dense-array variant
// (ir.rs:466-499 lists Str/Int/Bool/Unit/AffineMap*/I32Array/Aliasable/Raw), so a
// dense i64 array can only be carried as its printed spelling; no ELEMENT
// ACCESSOR is mapped, so `operator[]`/`asArrayRef()` still abort loudly rather
// than returning an empty array.  It is the largest single mangled type in the
// triage pass (24 of 93).
template <typename T>
class DenseArrayAttrImpl {};

} // namespace detail

namespace scf {
// mlir/include/mlir/Dialect/SCF/IR/SCF.h -- `scf::ForOp`, an ODS-generated op
// class, i.e. an `OpState` subclass wrapping one `Operation *`.  Maps where
// `mlir::Operation`/`OpState` map, `fmt::OpInst`, and carries the SAME
// prohibition: no equality, for the handle-vs-value reason given on OpState.
class ForOp {};
} // namespace scf

// mlir/include/mlir/Pass/Pass.h -- `class Pass`, an ABSTRACT BASE with the pure
// virtual `runOnOperation()`.  Mapped as an OPAQUE TYPE: no members, no init
// beyond the unit, exactly like `mlir::MLIRContext` (t23) and
// `mlir::EmptyProperties` (t13).
//
// WHY OPAQUE IS THE HONEST ANSWER HERE, and what it costs.  The crate models
// DataflowIR's DATA (ops, types, attrs, regions); it models no PASS
// INFRASTRUCTURE at all -- there is no PassManager, no pipeline, no
// runOnOperation in `dataflowir-gen`.  So there is nothing to map Pass's
// BEHAVIOUR onto and this rule deliberately maps none of it.  What the corpus
// needs is not behaviour: the five blocked TUs reach this type only through
// `std::unique_ptr<mlir::Pass>` -- a pass is CONSTRUCTED by a factory
// (`createXPass()`), MOVED into a `std::function<std::unique_ptr<Pass>()>`
// registration callback, and handed to a PassManager.  An OWNED OPAQUE HANDLE
// is a faithful model of exactly that, and it is what unblocks the
// `std::unique_ptr<mlir::Pass, std::default_delete<mlir::Pass>>` lookup that
// aborts at mapper.cpp:835 with `Type is not present in types_`.
//
// ⛔ AND THE COST IS STATED, NOT HIDDEN: because NO MEMBER IS MAPPED, any
// translated call THROUGH a Pass -- `pass->runOnOperation()`,
// `getArgument()`, `pm.addPass(...)`'s own body -- still ABORTS LOUDLY in the
// mapper rather than emitting something that compiles and lies.  That is the
// intended state.  This rule buys the TYPE and nothing else.
class Pass {};

// mlir/include/mlir/IR/BuiltinTypeInterfaces.h -- `ShapedType`, a TYPE
// INTERFACE (not a class hierarchy) over the shaped builtin types: vector,
// memref, tensor.  Every ShapedType IS a `mlir::Type`, and it is passed and
// returned BY VALUE like every other Type handle.
//
// -> `dataflowir_gen::ir::Ty` (ir.rs:37), WHICH IS A WIDENING and is the same
// widening this module already performs five times (t24/t30-t35 widen
// OpResult/TypedValue<T>/BlockArgument to `ir::Value`; t10-t12/t20/t26/t29
// widen six concrete attribute classes to `ir::Attr`).  Here the widening is
// ShapedType -> the whole `Ty` enum, i.e. the rule forgets the INTERFACE
// CONSTRAINT "this type is shaped".
//
// THE SHAPE ITSELF IS NOT LOST -- that is what makes this widening a good one
// rather than a lossy one.  `ir::Ty` carries shape in the variants that have
// it: `Ty::Vector(Vec<i64>, Box<Ty>)` (ir.rs:30) and
// `Ty::MemRef(Vec<i64>, Box<Ty>)` (ir.rs:32) both hold the dimension list and
// the element type, and a negative dimension is the dynamic `?` (ir.rs:60).
// So a future `getShape()` / `getElementType()` / `hasRank()` rule has real
// data to read.  ⛔ What the widening costs, stated plainly: `Ty` has no
// TENSOR variant, so `mlir::RankedTensorType` lands in `Ty::Opaque(spelling)`
// and its shape is then only recoverable by reparsing the spelling.
//
// ⛔ NO SHAPE ACCESSOR IS MAPPED, deliberately.  `getShape()`,
// `getElementType()`, `getRank()`, `hasStaticShape()`, `cloneWith()` are ALL
// absent, so every shape QUERY still aborts loudly in the mapper.  The rule
// buys the abort at `mlir::ktdp::AccessTileType::cloneWith`'s SIGNATURE (which
// is where KtdpTypes.cpp dies today) and nothing more.
class ShapedType {};


// mlir/include/mlir/IR/BuiltinTypeInterfaces.h -- `TensorType`, the TYPE
// INTERFACE over `RankedTensorType` / `UnrankedTensorType`.  It is the DIRECT
// follow-on from t41: with `mlir::ShapedType` mapped, `KtdpTypes.cpp` moved its
// first abort onto this one.  MEASURED, this private tree, t41 present and t42
// absent:
//   KtdpTypes.cpp rc=134 | unsupported system type has no rule:
//     `mlir::TensorType` ... rule key: searched as: mlir::TensorType
//
// -> `dataflowir_gen::ir::Ty` (ir.rs:37).  A WIDENING, the seventh in this
// module and the same shape as t41's.
//
// ⛔ THE COST, STATED: `ir::Ty` has `Ty::Vector(Vec<i64>, Box<Ty>)` (ir.rs:30)
// and `Ty::MemRef(Vec<i64>, Box<Ty>)` (ir.rs:32) but NO TENSOR VARIANT, so a
// `RankedTensorType` lands in `Ty::Opaque(spelling)` (ir.rs:47) and its shape is
// recoverable only by REPARSING the spelling.  The widening also forgets the
// interface constraint "this type is a tensor".
//
// ⭐ WHY WIDENING THE TYPE IS STILL DEFENSIBLE HERE: no shape ACCESSOR is
// mapped.  `getShape()`, `getElementType()`, `getRank()`, `hasRank()`,
// `cloneWith()` are ALL absent, so every shape QUERY still aborts LOUDLY in the
// mapper instead of reading a shape that is not there.  The rule buys the
// SIGNATURE and nothing else -- exactly the bargain t41 struck.
class TensorType {};

// mlir/include/mlir/IR/BuiltinTypes.h -- `IntegerSetAttr`, an ATTRIBUTE
// subclass wrapping an `mlir::IntegerSet`.  9 TUs in the v6 403-TU sweep and 8
// in v7 abort on it, making it the largest mlir row not standing refused.
//
// -> `dataflowir_gen::ir::Attr` (ir.rs:466), the SEVENTH time this module widens
// an attribute subclass to the `Attr` enum (t10-t12/t20/t26/t29 do the other
// six).
//
// ⭐ THE WIDENING IS NOT A GUESS -- the crate already names this exact
// attribute.  `Attr::Aliasable { base, text }` exists BECAUSE of it: the doc
// comment at ir.rs:505-508 cites `Builtin_IntegerSetAttr`'s
// `OpAsmAttrInterface::getAlias` (BuiltinAttributes.td:797-800), whose whole
// body is `os << "set"`, and the crate models the set itself as
// `pub struct IntegerSet` (ir.rs:425).
//
// ⛔ THE COST: `Attr` has no `IntegerSet(IntegerSet)` variant, so an
// IntegerSetAttr lands in `Attr::Aliasable`/`Attr::Raw` by spelling and its
// constraint rows are recoverable only by reparsing.  NO MEMBER IS MAPPED --
// `getValue()`, `IntegerSetAttr::get()` are absent -- so every query through one
// still aborts loudly.  This rule buys the TYPE.
class IntegerSetAttr {};

// mlir/include/mlir/IR/Value.h:239 -- `OpOperand`, the USE EDGE: an
// `IROperand<OpOperand, OpaqueValue>`, an intrusive node in a Value's use-list.
// 27 of 403 TUs in the v7 sweep abort here, the largest single mlir row.
//
// ⛔ MAPPING IT TO `ir::Value` REMAINS REFUSED, and that refusal is correct and
// load-bearing: an OpOperand is NOT the value, it is one USE of a value, and the
// two have different identity (a Value has many OpOperands; `getOwner()` and
// `getOperandNumber()` are properties of the EDGE, not of the value).  A
// `-> ir::Value` rule would let `op.getProducerMutable()` silently return
// something that compares equal to the operand's value and lose the edge.
// DO NOT "UPGRADE" THIS TO ir::Value.
//
// -> AN OPAQUE UNIT `()`, the representation t23 (`mlir::MLIRContext`), t40
// (`mlir::Pass`) and t13 (`mlir::EmptyProperties`) already use.  An opaque
// mapping CONFLATES NOTHING: it carries no value, so it cannot be mistaken for
// one.  ⛔ NO MEMBER IS MAPPED -- `get()`, `set()`, `getOwner()`,
// `getOperandNumber()`, `assign()`, use-list traversal are all absent -- so
// every real use of an OpOperand still ABORTS LOUDLY in the mapper.  The row
// becomes COUNTABLE instead of fatal, which is the whole and only claim.
class OpOperandOpaqueTag {};

// ---------------------------------------------------------------------------
// SPELLING-ONLY RESTATEMENTS for the SEVEN further concrete `TypedValue<T>`
// instantiations (t47-t53).  Exactly like `IndexType`/`MemRefType`/
// `ktdf::TokenType`/`ktdf::FifoSlotType` above: an EMPTY TAG in the right
// namespace, declared ONLY so the concrete key can be SPELLED.  None of them
// gets a `using tN =`, so NONE of them is mapped as a type in its own right --
// see the note in the header for why a bare declaration registers nothing.
// (`mlir::TensorType` is the one exception and it already has t42.)
// ⛔ THE ARGUMENT TYPES MUST NOT BE MAPPED AND THE KEYS MUST NOT BE GENERIC:
// a generic `TypedValue<T1>` forces the converter to map the template ARGUMENT
// and regresses to `mapper.cpp:722 Assertion 0 && "Type is not present in
// types_"`.  Six recorded regressions.  Concrete for MLIR types, always.
class VectorType {};
class IntegerType {};
namespace ktdf_arch {
class MemoryType {};
class ExecutionUnitType {};
} // namespace ktdf_arch
namespace ktdp {
class RuntimeArgType {};
class AccessTileType {};
} // namespace ktdp

// mlir/include/mlir/IR/BuiltinAttributes.h -- `TypeAttr`, the attribute that
// WRAPS A TYPE (`#tf.type<...>`, ODS `TypeAttr`).  39 occurrences in the sweep.
// -> `dataflowir_gen::ir::Attr` (ir.rs:466).  ⛔ THIS IS A WIDENING, the EIGHTH
// attribute subclass this module widens to the `Attr` enum (t7/t9/t10/t11/t12/
// t20/t44 are the other seven), and what it costs is stated: `Attr` has NO
// variant that carries an `ir::Ty` (ir.rs:466-499 is Str/Int/Bool/Unit/
// AffineMapAlias/AffineMap/I32Array/Aliasable/Raw -- `Int(i64, Ty)` carries a Ty
// only as an integer's suffix, which is not a TypeAttr), so a wrapped type lands
// in `Attr::Raw(spelling)` and is recoverable only by REPARSING that spelling.
// ⭐ Sound only because NO ACCESSOR IS MAPPED: `getValue()`, `TypeAttr::get()`
// are absent, so every attempt to read the wrapped type still aborts LOUDLY in
// the mapper rather than handing back a `Ty::Opaque("")` that is not the type.
class TypeAttr {};

// mlir/include/mlir/IR/BuiltinAttributeInterfaces.h -- `ElementsAttr`, the
// ATTRIBUTE INTERFACE over the dense/sparse element-array attributes (the same
// interface-over-subclasses shape t41 `ShapedType` and t42 `TensorType` have on
// the TYPE side).  41 occurrences.  Its two subclasses below appear once each
// and are keyed separately because `mapTypeStringRecursive` (mapper.cpp:709) is
// purely STRING-based -- an interface key cannot satisfy a lookup of a
// subclass's spelling, exactly as t1's key cannot satisfy `mlir::Operation *`
// (t36).
// -> `dataflowir_gen::ir::Attr` (ir.rs:466) for all three.  ⛔ WIDENINGS, and
// the cost is the LARGEST in this family: `Attr` HAS NO DENSE OR SPARSE
// ELEMENT-ARRAY VARIANT.  `Attr::I32Array(Vec<i32>)` (ir.rs:491) is NOT it and
// must not be used -- its own doc comment says it is `mlir::ArrayAttr` of
// `IntegerAttr`s, ODS `I32ArrayAttr`, printed `[1 : i32, 2 : i32]`, a DIFFERENT
// MLIR class with a different printed form from a `DenseIntElementsAttr`'s
// `dense<[1, 2]> : tensor<2xi32>`; it is also fixed at i32 where a dense int
// attr's elements are APInts of arbitrary width, and it carries no SHAPED TYPE,
// which is half of what a DenseElementsAttr IS.  So the payload lands in
// `Attr::Raw`/`Attr::Aliasable` BY SPELLING for all three.
// ⭐ Sound only because NO ELEMENT QUERY IS MAPPED: `getValues<T>()`,
// `getElementType()`, `getNumElements()`, `isSplat()`, `operator[]`,
// `SparseElementsAttr::getIndices()`/`getValues()` are ALL absent, so every
// element access still ABORTS LOUDLY instead of reading an empty array.  These
// rules buy the SIGNATURE and nothing else.
class ElementsAttr {};
class SparseElementsAttr {};
class DenseIntElementsAttr {};

namespace detail {
// mlir/include/mlir/Support/InterfaceSupport.h -- `mlir::detail::InterfaceMap`,
// MLIR's RUNTIME INTERFACE DISPATCH TABLE: a sorted array of
// (TypeID, void *concept) pairs that `Op<...>`/`AbstractOperation` consults to
// answer `isa<SomeInterface>` / `cast<SomeInterface>` on an opaque op.
// `grep -rn InterfaceMap /home/agent/work/repos/dt_src/cpp2rust-port/dataflowir-gen/src`
// = 0 HITS: the crate models PRINTED IR (ops as `fmt::OpInst` content), and has
// no notion of runtime interface dispatch at all, so there is nothing to map it
// ONTO.
// -> AN OPAQUE UNIT `()`, exactly the representation t23 (`MLIRContext`), t40
// (`Pass`), t43 (`OpOperand`) and t13 (`EmptyProperties`) already use.  An
// opaque mapping CONFLATES NOTHING: it carries no value, so it cannot be
// mistaken for one.  ⛔ NO MEMBER IS MAPPED -- `lookup<T>()`, `contains()`,
// `insert()`, `InterfaceMap::get<Interfaces...>()` are all absent -- so any TU
// that actually DISPATCHES through an interface map still ABORTS LOUDLY in the
// mapper.  It appears only in ODS-generated op-definition machinery, where the
// spelling must be nameable and nothing calls through it.
class InterfaceMap {};

// `mlir::detail::IROperandBase` -- mlir/IR/UseDefLists.h:35.  The UNTYPED BASE of
// the use edge; `mlir::OpOperand` derives from it.  Its single corpus site is
// `visitLinksImpl`.
class IROperandBase {};
} // namespace detail


// ---------------------------------------------------------------------------
// PASS 2026-09-27 (rules/mlir slot): six handle rows, 500+ summed occurrences.
// Each is declared here ONLY so its key can be spelled; the model is argued at
// its `using tN =` below.
// ---------------------------------------------------------------------------
// `mlir::Builder` is declared INCOMPLETE on purpose, because that is EXACTLY how
// the corpus reaches it: all 102 TUs resolve to ONE site, the bare forward
// declaration `class Builder;` at mlir/IR/AffineMap.h:37:7, reached while the
// converter emits an AffineMap signature.  No TU names Builder itself.
class Builder;

// `mlir::ModuleOp` -- BuiltinOps.h.inc:199, reached as the RETURN type of
// `runOnOperation`.  An OP HANDLE (a pointer-sized wrapper over Operation*).
class ModuleOp {};

// `mlir::OperationState` -- OperationSupport.h:948, a `struct`.  MLIR's mutable
// construction bag handed to `Operation::create`.
struct OperationState {};

// `mlir::PassManager` -- Pass/PassManager.h:232.  See t62 for why this is the
// same refusal-to-model that t40 (`mlir::Pass`) already made.
class PassManager {};

// ---------------------------------------------------------------------------
// PASS 2026-09-27 (second rules/mlir slot): the four rows c39aaa6 left out.
// Declared ONLY so the key can be SPELLED; each is argued at its `using tN =`.
// ---------------------------------------------------------------------------
// `mlir::OpBuilder::Listener` -- Builders.h:285:10, `struct Listener : public
// ListenerBase`.  ⭐ IT IS NESTED INSIDE `class OpBuilder`, NOT in a namespace,
// so `OpBuilder` must be declared COMPLETE here purely to hold it.  OpBuilder
// itself is NOT mapped (no `using tN =` names it) -- a previous slot landed
// `IROperandBase` in `llvm::detail` by reopening the wrong `detail`, and this is
// the same hazard one level down: get the ENCLOSER wrong and the key is dead.
class OpBuilder {
public:
  struct Listener {};
};

namespace detail {
// `mlir::detail::PassOptions::Option` / `::ListOption` -- PassOptions.h:192:9 and
// :239:9.  ⭐ THE TEMPLATES ARE DECLARED WITH ONE PARAMETER, NOT TWO, AND THAT IS
// DELIBERATE.  The real MLIR declarations are
//     template <typename DataType, typename OptionParser = OptionParser<DataType>>
// and the converter prints the row TWO WAYS: the canonicalised `from decl` form
// KEEPS the defaulted argument (`Option<int, llvm::cl::parser<int>>`) while the
// `searched as:` form -- the ONLY one a rule key is looked up by -- ELIDES it
// (`Option<int>`).  That is the sugar-vs-canonical axis that has already burned
// this project (see the DenseMapInfo note in the common brief).  Declaring ONE
// parameter here makes it IMPOSSIBLE for the recorded key to carry the defaulted
// argument, so the key cannot silently drift to the dead canonical spelling; it
// does not need the default to exist, and so does not need `llvm::cl::parser`
// declared at all.  Verified by reading all four keys back out of ir_src.json.
// NO MEMBER IS DECLARED: `getValue()`, `operator=`, the `llvm::cl::opt` /
// `llvm::cl::list` bases and the `OptionBase` virtuals are all absent.
class PassOptions {
public:
  template <typename DataType>
  class Option {};
  template <typename DataType>
  class ListOption {};
};
} // namespace detail

// ---------------------------------------------------------------------------
// PASS 2026-09-27 (third rules/mlir slot).  Declared ONLY so the key can be
// SPELLED; argued at `using t69 =` below.
// ---------------------------------------------------------------------------
// `mlir::OpPrintingFlags` -- OperationSupport.h:1176:7, `class OpPrintingFlags`.
// A plain namespace-scope class, no template, no nesting: the key is the bare
// name.  NO MEMBER IS DECLARED -- `enableDebugInfo`, `printGenericOpForm`,
// `useLocalScope`, `skipRegions`, `elideLargeElementsAttrs` are all absent, and
// that absence is what keeps the opaque claim enforceable (see t69).
class OpPrintingFlags {};

// ---------------------------------------------------------------------------
// PASS 2026-09-27 (fourth rules/mlir slot): `mlir::InFlightDiagnostic`, the
// `operator<<` family, queue rows g286/g320/g346/... (22 rows, 66 TU-rows).
//
// ⛔ A UNIT MODEL IS FORBIDDEN HERE.  Diagnostics.h:325-328 is
// `~InFlightDiagnostic() { if (isInFlight()) report(); }` and Diagnostics.h:
// 319-324's move ctor explicitly `rhs.abandon()`s -- the accumulated message
// reaches the DiagnosticEngine ON DESTRUCTION, exactly once.  A destructor with
// an observable effect is the axis on which `mlir::OwningOpRef` was REFUSED
// above; a unit plus unit-returning `<<` keys would translate, compile, and
// SILENTLY DELETE every diagnostic, and the dominant call sites are the
// tablegen-generated parse/verify bodies (KTDFAttributes.cpp.inc:132,
// KTDFLowering.cpp.inc:30, SDSCBundleTypes.cpp.inc:216) whose ONLY externally
// visible behaviour IS the message.  So it maps to a REAL accumulating type,
// `libcc2rs::InFlightDiagnostic`, which prints on `Drop` (libcc2rs/src/diag.rs).
//
// ⭐ `operator<<` IS DECLARED AS MLIR DECLARES IT -- a `template <typename Arg>`
// on an `&&`-qualified member returning `InFlightDiagnostic &&`
// (Diagnostics.h:344-349).  The KEY is recorded from the RESOLVED signature at
// the call in the rule body below, so `Arg` deduces to `const char (&)[36]` for
// a 36-byte literal and the key prints `const char (&)[_]` -- ONE key for every
// literal length, because normalizeTranslationRule (mapper.cpp:1200-1213)
// rewrites `\b\d+\b` to `_` across the whole key.  g286's 21 TUs therefore need
// ONE key, not 21.
//
// ⭐ THE RECEIVER ARRIVES AS a0.  For the shift family only, mapper.cpp:1597-
// 1602 prints the return type then the WRITTEN nested-name-specifier, which is
// empty for a member declared in its own class body -- hence the bare
// `operator shl(...)` in the key, with the implicit object argument absent from
// the printed parameter list (mapper.cpp:1648-1666) but PRESENT in the call.
// So the target body sees the receiver as a0 and the streamed value as a1.
// Confirmed by f11, whose key has an empty parameter list while its target is
// `fn f11(a0: ir::Attr) -> bool`.
class InFlightDiagnostic {
public:
  InFlightDiagnostic();
  // Diagnostics.h:344-349 -- `template <typename Arg> InFlightDiagnostic
  // &&operator<<(Arg &&arg) &&`.
  template <typename Arg> InFlightDiagnostic &&operator<<(Arg &&arg) &&;
};

} // namespace mlir

// ---- type rules, and nothing else ----------------------------------------
namespace llvm {
template <typename T>
class ArrayRef {};

// `llvm::MutableArrayRef<T>` -- t46, the biggest LIVE mlir-owned row in the
// 2026-09-27 403-TU sweep (12 TUs).  It is `ArrayRef`'s mutable twin: the same
// {pointer, length} view, non-const element access.  The diagnostic prints it
// instantiated at `llvm::MutableArrayRef<mlir::Region>` -- by VALUE, because
// `Operation::getRegions()` hands out the operation's inline region storage.
// GENERIC IS SAFE HERE, unlike for an MLIR handle type (t26/t29/t37-t39): the
// abort a generic rule risks is the converter having to map the template
// ARGUMENT, and the only argument this row is instantiated at, `mlir::Region`,
// already has a model (t3 -> fmt::Region).  t19 is the identical shape and has
// been generic project-wide without incident.
template <typename T>
class MutableArrayRef {};

// llvm/ADT/PointerUnion.h -- declared ONLY so the RegionRange base key can be
// spelled.  NOT mapped (no `using tN =`).
template <typename... PTs>
class PointerUnion {};

namespace detail {
// llvm/ADT/STLExtras.h -- `indexed_accessor_range_base<DerivedT, BaseT, T,
// PointerT, ReferenceT>`, the CRTP base of MLIR's range families.
template <typename DerivedT, typename BaseT, typename T, typename PointerT,
          typename ReferenceT>
class indexed_accessor_range_base {};
} // namespace detail
} // namespace llvm

using t1 = mlir::Operation;
using t2 = mlir::Block;
using t3 = mlir::Region;
using t4 = mlir::Value;
using t5 = mlir::Type;
using t6 = mlir::Attribute;
using t7 = mlir::StringAttr;
using t8 = mlir::AffineMap;
using t9 = mlir::DictionaryAttr;
using t10 = mlir::IntegerAttr;
using t11 = mlir::ArrayAttr;
using t12 = mlir::AffineMapAttr;
using t13 = mlir::EmptyProperties;
using t14 = mlir::OperandRange;
using t15 = mlir::ResultRange;
using t16 = mlir::ValueRange;
using t17 = mlir::RegionRange;
using t18 = mlir::OperationName;
template <typename T1> using t19 = llvm::ArrayRef<T1>;
using t20 = mlir::BoolAttr;
using t21 = mlir::NamedAttribute;
using t22 = mlir::Dialect;
using t23 = mlir::MLIRContext;
using t24 = mlir::OpResult;
using t25 = mlir::OpState;
using t26 = mlir::detail::DenseArrayAttrImpl<long>;
using t27 = mlir::scf::ForOp;
using t28 = mlir::OpFoldResult;

// ⛔ t29 IS t26 SPELLED THE WAY THE MAPPER ACTUALLY ASKS, AND t26 ALONE NEVER
// MATCHED ANYTHING.  Measured 2026-09-27 with `-verbose` on the reference TU:
// the only two lookups the mapper performs for this type are
//     search type mlir::detail::DenseArrayAttrImpl<int64_t>, result: None
//     search type const mlir::detail::DenseArrayAttrImpl<int64_t> &, result: None
// i.e. the key is spelled with the TYPEDEF `int64_t`, never with `long`.  The
// triage/mangling message disagrees: `Converter::ReportUnmappedSystemType`
// (converter.cpp:3426) builds its printed "rule key" from
// `Mapper::ToString(Mapper::GetTypeForDecl(decl))`, which goes through the
// RecordDecl and therefore CANONICALISES the template argument to `long`.  So
// the converter told us to write `<long>`, t26 was committed as `<long>` at
// 950ff94, and it could never match: 24 sites stayed mangled.  Same shape as the
// `llvm::SmallVector<long>` / `<T1, _>` mismatch -- TWO PRINTERS DISAGREE, and
// only the `search type` line is the one that decides a lookup.
// t26 is KEPT (a canonical `<long>` spelling costs nothing and would match a TU
// that writes `long` directly); t29 is the one that does the work.
// A `typedef long int64_t;` DOES NOT WORK: cpp-rule-preprocessor records the
// CANONICAL spelling, so `using t29 = ...<int64_t>` came back from ir_src.json as
// `...<long>` -- an exact duplicate of t26, measured.  The key therefore has to be
// GENERIC so `matchTemplate` (mapper.cpp:382) can bind T1 to whatever the use
// site's printer produced.  That is SAFE HERE and ONLY here: the abort a generic
// MLIR type rule causes is the converter having to map the template ARGUMENT, and
// this argument is always a BUILTIN INTEGER, which already has a model.  Contrast
// t30-t34, whose arguments are MLIR type handles -- those must stay concrete.
template <typename T1> using t29 = mlir::detail::DenseArrayAttrImpl<T1>;

// t30-t34: the five concrete `TypedValue` instantiations.  Concrete, never
// generic -- see the prohibition on the class declaration above.
using t30 = mlir::detail::TypedValue<mlir::IndexType>;
using t31 = mlir::detail::TypedValue<mlir::MemRefType>;
using t32 = mlir::detail::TypedValue<mlir::RankedTensorType>;
using t33 = mlir::detail::TypedValue<mlir::ktdf::TokenType>;
using t34 = mlir::detail::TypedValue<mlir::ktdf::FifoSlotType>;

using t35 = mlir::BlockArgument;

// t36: THE POINTER SPELLING OF `mlir::Operation`.  `mapTypeStringRecursive`
// (mapper.cpp:709) is purely STRING-based and does NO pointer stripping, so t1's
// key (`mlir::Operation`) cannot satisfy a lookup of the spelling
// `mlir::Operation *`.  An EXPLICIT pointer-spelled key is the established
// precedent in this tree -- `llvm::raw_ostream *` (rules/raw_ostream t3),
// `std::ostream *` (rules/iostream t3), `FILE *`, `DIR *` -- and this is the same
// shape: SAME model as t1, pointer representation per target model.
using t36 = mlir::Operation *;

// t37-t39: the CRTP base of MLIR's range families, keyed at the THREE CONCRETE
// INSTANTIATIONS the diagnostics print verbatim.
// ⛔ ONE GENERIC RULE WOULD BE WRONG.  OperandRange/ResultRange/ValueRange and
// RegionRange all derive from this base at DIFFERENT element types, and RegionRange's
// element is a `Region`, not a `Value` -- a generic rule would have to pick one
// representation and would silently give three of the four the wrong element type.
// It would ALSO be the generic-MLIR-type regression recorded on t26/t29: a generic
// key forces the converter to map the template ARGUMENTS, and `mlir::OpOperand` /
// `mlir::detail::OpResultImpl` deliberately have no model, so it would turn a
// countable mangled name into a hard mapper.cpp:722 abort.
// Each maps WHERE ITS DERIVED RANGE ALREADY MAPS: t14/t15 -> Vec<ir::Value>,
// t17 -> Vec<fmt::Region>.  Nothing new is claimed; these are the same
// representations, reached through the base-class spelling.
using t37 = llvm::detail::indexed_accessor_range_base<
    mlir::OperandRange, mlir::OpOperand *, mlir::Value, mlir::Value, mlir::Value>;
using t38 = llvm::detail::indexed_accessor_range_base<
    mlir::ResultRange, mlir::detail::OpResultImpl *, mlir::OpResult,
    mlir::OpResult, mlir::OpResult>;
using t39 = llvm::detail::indexed_accessor_range_base<
    mlir::RegionRange,
    llvm::PointerUnion<mlir::Region *,
                       const std::unique_ptr<mlir::Region,
                                             std::default_delete<mlir::Region>> *,
                       mlir::Region **>,
    mlir::Region *, mlir::Region *, mlir::Region *>;

// t40/t41: see the class declarations above for the full reasoning.  t40 is an
// OPAQUE OWNED HANDLE (no members mapped, every call through it still aborts);
// t41 is a WIDENING to `ir::Ty` (no shape accessor mapped).
using t40 = mlir::Pass;
using t41 = mlir::ShapedType;

// t42/t43/t44: see the class declarations above for the full reasoning.
// t42 is a WIDENING to `ir::Ty` (no shape accessor mapped); t44 is the seventh
// WIDENING to `ir::Attr` (no member mapped); t43 is an OPAQUE UNIT and the
// `ir::Value` mapping for it stays REFUSED.
using t42 = mlir::TensorType;
using t43 = mlir::OpOperand;
using t44 = mlir::IntegerSetAttr;

// t45: the SIXTH concrete `TypedValue` instantiation, beyond t30-t34.  CONCRETE,
// never generic -- the prohibition on the TypedValue class declaration above
// applies unchanged.  Same model as t30-t34: an `ir::Value` (ir.rs:21), which is
// WELL-GROUNDED, not a widening -- a TypedValue<T> IS an SSA value whose static
// type is known, and the crate's Value carries exactly {name, Ty}.  The element
// type is DISCARDED into `Ty::Opaque` (ir.rs:47) because the crate has no
// `InputArgType`; that is the same honest loss t30-t34 already take, and no
// accessor is mapped, so any query through one still aborts loudly.
using t45 = mlir::detail::TypedValue<mlir::sdscbundle::InputArgType>;

// t46: `llvm::MutableArrayRef<T1>` -- see the class declaration above for why
// this one may be generic where the MLIR-handle rows may not.  SAME model as
// t19: `Vec<T1>`.  This is a WIDENING of ownership, not of type: a C++
// MutableArrayRef is a non-owning mutable view and a `Vec` owns its buffer, so
// writes through a translated MutableArrayRef do NOT propagate to the viewed
// container.  That is the identical, already-accepted tradeoff t19 makes for the
// const view, and the crate has no borrowed-slice spelling a rule target can
// name.  NO accessor is mapped here, so `data()`/`size()`/`operator[]` on one
// still abort loudly rather than reading a detached copy.
template <typename T1> using t46 = llvm::MutableArrayRef<T1>;

// t47-t53: the SEVEN further CONCRETE `mlir::detail::TypedValue<T>`
// instantiations, 208 occurrences in the 2026-09-27 sweep.  SAME MODEL as
// t30-t34/t45: `dataflowir_gen::ir::Value` (ir.rs:21).  ⭐ WELL-GROUNDED, NOT A
// WIDENING: `TypedValue<T> : public Value` (mlir/IR/Value.h:106), a TypedValue IS
// an SSA value whose static type is known, and `ir::Value` carries exactly
// `{ name: String, ty: Ty }`.  ⛔ What is lost, and it is the SAME already-accepted
// loss t30-t34 take: the STATIC type `T` is discarded -- the init's `ty` is
// `Ty::Opaque("")` (ir.rs:47) because the crate has no `VectorType`/`MemoryType`/
// `ExecutionUnitType`/`RuntimeArgType`/`AccessTileType`/`IntegerType` handle -- so
// nothing that depends on `T` (e.g. `.getType()` returning a `T`) is mapped, and
// such a site still aborts loudly.
// ⛔ CONCRETE, NEVER GENERIC -- the prohibition on the `TypedValue` class
// declaration applies unchanged; a generic key over an MLIR type argument has
// regressed to `mapper.cpp:722` six times.  Each ARGUMENT type is restated as an
// empty tag above and gets NO `using tN =` of its own (except `mlir::TensorType`,
// which already had t42 for independent reasons).
using t47 = mlir::detail::TypedValue<mlir::VectorType>;
using t48 = mlir::detail::TypedValue<mlir::ktdf_arch::MemoryType>;
using t49 = mlir::detail::TypedValue<mlir::ktdf_arch::ExecutionUnitType>;
using t50 = mlir::detail::TypedValue<mlir::ktdp::RuntimeArgType>;
using t51 = mlir::detail::TypedValue<mlir::ktdp::AccessTileType>;
using t52 = mlir::detail::TypedValue<mlir::TensorType>;
using t53 = mlir::detail::TypedValue<mlir::IntegerType>;

// t54-t57: FOUR attribute rows.  ALL FOUR ARE WIDENINGS to
// `dataflowir_gen::ir::Attr` (ir.rs:466) -- the 8th through 11th time this module
// widens an attribute subclass to that enum.  See the class declarations above
// for exactly what each loses; in summary: `Attr` has no Type-wrapping variant
// and NO dense/sparse element-array variant, so all four payloads land in
// `Attr::Raw`/`Attr::Aliasable` BY SPELLING, and NO accessor is mapped for any of
// them, so every read through one still aborts loudly.
// ⛔ `Attr::I32Array` (ir.rs:491) is NOT the right target for
// `DenseIntElementsAttr`: it is documented as `mlir::ArrayAttr` of `IntegerAttr`s
// (ODS `I32ArrayAttr`), a different MLIR class with a different printed form, i32
// elements rather than APInts, and no shaped type.  Checked, not assumed.
using t54 = mlir::TypeAttr;
using t55 = mlir::ElementsAttr;
using t56 = mlir::SparseElementsAttr;
using t57 = mlir::DenseIntElementsAttr;

// t58: `mlir::detail::InterfaceMap` -> AN OPAQUE UNIT.  See the class
// declaration for the full reasoning and the zero-hit grep of the crate.
using t58 = mlir::detail::InterfaceMap;

// ---------------------------------------------------------------------------
// t59-t64: THE SIX ROWS ADDED 2026-09-27.  Keys read off `searched as:` in the
// survey diagnostic, NOT off the canonicalised `from decl` spelling (trap 1).
// ---------------------------------------------------------------------------

// t59: `mlir::Builder` -> AN OPAQUE UNIT.  102 TUs, the largest single row left
// in this module.  ⭐ THE OPAQUE GATE IS CLEARED BY MEASUREMENT, not by
// reasoning.  The worry was that a survey records only RECOVERABLE gaps, so a
// method call on Builder could be hiding behind an rc=134 abort.  Cleared with
// `-verbose ... 2>&1 | grep -A1 'search expr'` on the two TUs that REACH the
// site (WriteSetScan.cpp, 62,440 log lines, 726 AffineMap mentions;
// SplitDFIROutput.cpp, 69,039 lines, 494 AffineMap mentions -- the third TU
// tried, StageCoarsening/Materializer.cpp, aborted at mapper.cpp:1191 with ZERO
// AffineMap mentions and is therefore NOT a valid gate TU):
//     `search expr` lines mentioning Builder, summed over all three logs: 0
// while the same grep finds 124 for IndexType.  So the converter NEVER searches
// for a rule on any Builder method or expression -- it needs the TYPE and
// nothing else.  The 645/754 raw "Builder" hits in those logs are AST-dump and
// signature text (`::mlir::OpBuilder &` parameters), not rule lookups.
// ⛔ THE COST, STATED: no member is mapped, so `builder.getIndexType()`,
// `getContext()`, `getI32IntegerAttr(...)` -- every real call through a Builder
// -- still ABORTS LOUDLY in the mapper rather than compiling and lying.  A
// Builder is a FACTORY over an MLIRContext, and t23 already maps that context to
// the same opaque unit for the same reason: `dataflowir-gen` models printed IR
// DATA and has no builder/uniquer infrastructure to map behaviour onto.
using t59 = mlir::Builder;

// t60: `mlir::IndexType` -> `dataflowir_gen::ir::Ty` (ir.rs:37).  101 TUs.
// A WIDENING, the same one t41 (`ShapedType`) and t42 (`TensorType`) make: every
// `IndexType` IS an `mlir::Type` and t5 already maps `mlir::Type -> ir::Ty`.
// ⭐ BETTER GROUNDED THAN t41/t42 IN ONE RESPECT: `ir::Ty` has a REAL `Index`
// variant (ir.rs:39, printing `index`), so the crate can represent this type
// exactly -- nothing lands in `Ty::Opaque(spelling)` the way a tensor does.
// The site is the CHEAPEST kind: a RETURN type
// (`mlir::agen::...::getMulticastInfoType`, BuiltinTypes.h.inc:958).
// ⛔ WHAT THE `init` IS AND WHY.  `Ty::Opaque("")` -- t5's EMPTY-SPELLING NULL
// SENTINEL, which no real MLIR type can print as -- NOT `Ty::Index`.  A
// default-constructed `mlir::IndexType` is a NULL handle, and returning
// `Ty::Index` would claim a live index type where C++ has none.  The whole
// t5/t41/t42 family uses this sentinel; disagreeing here would make
// `IndexType t;` and `Type t;` compare unequal.
// ⛔ AND THE ONE THING THIS ROW DOES NOT BUY, measured: unlike the five other
// rows in this pass, IndexType IS reached in EXPRESSION position.  The verbose
// logs show exactly ONE real lookup on it,
//     mlir::IndexType mlir::IndexType::get(mlir::MLIRContext *)
// (the other 123 hits are IndexType appearing inside OTHER keys' signatures --
// `OneTypedResult<mlir::IndexType>::Impl`, `TypedValue<mlir::IndexType>`, both
// already covered by t30).  That FACTORY IS DELIBERATELY NOT MAPPED HERE: see
// the note at the end of this block for why, and it still aborts loudly.
using t60 = mlir::IndexType;

// t61: `mlir::ModuleOp` -> AN OPAQUE UNIT.  83 TUs.  Reached as a RETURN type
// (`runOnOperation`, BuiltinOps.h.inc:199).  Gate cleared the same way as t59:
// `search expr` hits mentioning ModuleOp across the three verbose logs = 0.
// ⛔ `fmt::OpInst` (fmt.rs:391) WAS CHECKED AND REFUSED as the target.  An
// `OpInst` carries a `def` that is "a row of the GENERATED `TD_OPS` table, so an
// `OpInst` cannot name an op the table does not contain" (fmt.rs:388) -- and
// `builtin.module` is an MLIR BUILTIN, not a DataflowIR op, so it has no TD_OPS
// row.  Mapping ModuleOp to OpInst would require naming an op the generated
// table cannot spell.  t40 (`Pass`) / t43 (`OpOperand`) precedent applies.
// ⛔ NO EQUALITY IS ADDED for this or any op handle, on purpose.  Two distinct
// handles to ONE operation must compare EQUAL, and two handles to two ops that
// happen to print identically must compare UNEQUAL -- mapping a handle onto a
// printed-content type inverts exactly that, which is a DIFFERENT RELATION.
// ⛔ COST: no member mapped, so `getBody()`, `walk()`, `getOps<...>()` abort.
using t61 = mlir::ModuleOp;

// t62: `mlir::detail::IROperandBase` -> AN OPAQUE UNIT.  92 TUs, single site
// `visitLinksImpl` via mlir/IR/UseDefLists.h.  ⭐ THIS IS NOT A NEW JUDGEMENT:
// it is the UNTYPED BASE of `mlir::OpOperand`, the same USE-EDGE family, and
// t43's refusal to map that as `ir::Value` STANDS.  t43's opaque mapping is now
// EVIDENCE rather than reasoning -- it took its row to 0 across 5 TUs with
// nothing reading through it.  An IROperandBase records "slot N of some op uses
// some value"; `grep -rn IROperandBase dataflowir-gen/src` has nothing to map it
// to, and calling it a Value would CONFLATE a use with the value used.
// ⛔ COST: `getOwner()`, `getNextOperandUsingThisValue()`, the whole link
// walk -- none mapped, all still abort loudly.
using t62 = mlir::detail::IROperandBase;

// t63: `mlir::OperationState` -> AN OPAQUE UNIT.  87 TUs (CONFIRMED LIVE this
// slot -- the handover listed it as unconfirmed; 87 of 403 survey-v3 TSVs carry
// the row, first site `mlir::sentient::SyncOp::build`).  Gate cleared: 0
// `search expr` hits across the three verbose logs.  MLIR's MUTABLE
// CONSTRUCTION BAG (name, location, operands, result types, attributes,
// regions) filled by a generated `build()` and consumed by `Operation::create`.
// ⛔ WHY OPAQUE AND NOT `fmt::OpInst`: an OpInst is a BUILT op bound to a TD_OPS
// row, whereas an OperationState is the half-filled argument pack on the way in,
// and the generated `build()` methods that fill it are exactly the code this
// port does not reproduce.  The unit is not a claim the bag is empty -- it is a
// claim this port never READS one, enforced by there being NO member rule, so
// every `state.addOperands(...)`/`addTypes(...)` still ABORTS LOUDLY.
using t63 = mlir::OperationState;

// t64: `mlir::PassManager` -> AN OPAQUE UNIT.  59 TUs (also CONFIRMED LIVE this
// slot, first site `mlir::init::SymLocOp::getAttributeNameForIndex`).  Gate
// cleared: 0 `search expr` hits across the three verbose logs.  ⭐ t40's comment
// on `mlir::Pass` ALREADY ARGUES THIS ROW: "the crate models DataflowIR's DATA
// ...; it models no PASS INFRASTRUCTURE at all -- THERE IS NO PassManager, no
// pipeline, no runOnOperation in `dataflowir-gen`".  This is that same sentence
// applied to the type it names, so it is the same refusal, not a new one.
// ⛔ COST: `pm.addPass(...)`, `pm.run(module)`, `nest<...>()` -- no member is
// mapped, all still abort loudly.  This rule buys the TYPE and nothing else.
using t64 = mlir::PassManager;

// ---------------------------------------------------------------------------
// PASS 2026-09-27 (second slot): the four rows the note below used to list as
// LEFT OUT.  Both reasons it gave were MEASUREMENT ERRORS, now corrected:
//   * `OpBuilder::Listener` is NOT "one row in one TU" -- it is 87 TUs.  The
//     "1" came from an exact-spelling grep; `cat survey-v4/*.tsv | grep -F
//     'mlir::OpBuilder::Listener'` returns 87 lines.  It is the BIGGEST
//     unclaimed row in this module.
//   * `PassOptions::Option<int>` is NOT zero.  A grep for `Option<int>` finds
//     nothing because the survey records the CANONICALISED `from decl` form
//     WITH the defaulted template argument KEPT.
// ⭐ ALL FOUR KEYS BELOW ARE THE `searched as:` SPELLING, TAKEN VERBATIM FROM THE
// CONVERTER'S OWN unmapped-type DIAGNOSTIC, which prints both sides:
//     `... rule key: searched as: mlir::detail::PassOptions::Option<int>;
//      from decl (NOT a key -- canonicalised, defaulted args kept):
//      mlir::detail::PassOptions::Option<int, llvm::cl::parser<int>>`
// The left side is the key; the right side is a DEAD key that LOOKS like
// coverage.  See the declarations above for why the templates are declared with
// ONE parameter so the dead spelling is unreachable.
//
// GATE FOR ALL FOUR -- MEASURED, not argued.  On an aborting TU
// (Transform/Sentient/ToggleReordering.cpp), `grep -c 'search expr'` = 4229 rule
// lookups, of which the number mentioning `OpBuilder::Listener` or `PassOptions`
// is ZERO.  The raw mentions of those names in the log are ALL clang AST-dump
// lines (`ParmVarDecl '::mlir::OperationState &'`, `CXXMethodDecl build`) --
// DECLARATIONS being lowered, never EXPRESSIONS being looked up, which matches
// the survey's own context column (every site is a member or parameter type in
// an MLIR header).  So `pm.addPass(...)`, `option.getValue()`,
// `listener->notifyOperationInserted(...)` DO NOT OCCUR in this corpus, and an
// OPAQUE unit with NO MEMBER MAPPED is honest for all four.
// ⛔ NOT ONE MEMBER IS MAPPED, deliberately: an unmapped member ABORTS LOUDLY,
// and that abort is the ENFORCEMENT that makes the opaque claim TRUE rather than
// merely convenient.  If a TU ever does read one of these, it stops, loudly.

// t65: `mlir::OpBuilder::Listener` -> AN OPAQUE UNIT.  87 TUs, 4 occurrences in
// the 4-TU measurement set, single site `Builders.h:285:10` reached while the
// converter lowers `mlir::sentient::IfOp::getThenBodyBuilder`'s SIGNATURE (the
// `Listener *listener` parameter).  MLIR's INSERTION-CALLBACK INTERFACE: a set
// of `notifyOperationInserted` / `notifyBlockInserted` virtuals an OpBuilder
// calls as it mutates IR.  ⛔ WHY OPAQUE: `dataflowir-gen` BUILDS NO IR -- it
// models printed DataflowIR, so there is no insertion to be notified OF, and no
// callback registry to map this onto (`grep -rn Listener` over
// `dataflowir-gen/src` = 0).  Same ground as t40 (`Pass`) and t64
// (`PassManager`): the pass/builder INFRASTRUCTURE is exactly what this port
// does not reproduce.  ⛔ COST: every virtual is absent, so a TU that actually
// implements or invokes a listener hook still aborts.
using t65 = mlir::OpBuilder::Listener;

// t66: `mlir::detail::PassOptions::Option<int>` -> AN OPAQUE UNIT.  59 TUs, 4
// occurrences, site `PassOptions.h:192:9`, reached while lowering `DCC::getModule`
// -- i.e. a pass's OPTION MEMBER declaration, not a read of it.  A
// command-line-backed pass option: an `llvm::cl::opt<int>` plus MLIR's
// `OptionBase` bookkeeping.  ⛔ WHY OPAQUE: this is COMMAND-LINE PLUMBING.  Its
// value comes from argv parsing inside `llvm::cl`, which this port does not
// translate at all, so there is no value to model -- mapping it to `i32` would
// invent a DEFAULT-INITIALISED ZERO and silently substitute it for whatever the
// user passed on the command line.  That is the silently-wrong outcome the
// playbook ranks worse than an abort.  ⛔ COST: `getValue()`, `operator=`,
// `operator int`, `hasValue()` -- none mapped, all abort loudly.
using t66 = mlir::detail::PassOptions::Option<int>;

// t67: `mlir::detail::PassOptions::ListOption<std::string>` -> AN OPAQUE UNIT.
// 59 TUs, 8 occurrences (2 per TU), site `PassOptions.h:239:9`, same
// `DCC::getModule` declaration context.  The comma-separated LIST form, backed by
// `llvm::cl::list<std::string>`.  ⛔ Same refusal as t66, and the list form makes
// it sharper: an empty `Vec<String>` is not a neutral stand-in for an unparsed
// option list -- a loop over it would run ZERO iterations and the TU would
// silently do nothing.  ⛔ COST: `begin()`/`end()`, `size()`, `operator[]`,
// `operator=` -- none mapped.
using t67 = mlir::detail::PassOptions::ListOption<std::string>;

// t68: `mlir::detail::PassOptions::ListOption<int>` -> AN OPAQUE UNIT.  59 TUs,
// 12 occurrences (3 per TU -- the largest single count in the measurement set),
// same site and same context as t67.  Identical model, identical refusal; it is a
// SECOND INSTANTIATION of one template and is keyed separately because the key
// carries the concrete argument.  ⛔ Same absent members.
using t68 = mlir::detail::PassOptions::ListOption<int>;

// ---------------------------------------------------------------------------
// PASS 2026-09-27 (third slot).  ONE key, deliberately.  The slot's other two
// queue rows are resolved WITHOUT a new key and the reasons are recorded at the
// bottom of this file -- one was ALREADY DONE, one FAILED ITS GATE.
// ---------------------------------------------------------------------------
// t69: `mlir::OpPrintingFlags` -> AN OPAQUE UNIT.  59 TUs.  MLIR's PRINTING-
// OPTIONS BAG (OperationSupport.h:1176) -- debug-info, generic-op form,
// local-scope and elision switches consulted by `Operation::print`/`AsmState`.
// ⭐ SPELLING, READ NOT INFERRED.  Taken verbatim from the converter's own
// unmapped-type diagnostic in the baseline survey TSVs, which prints both sides:
//     searched as: mlir::OpPrintingFlags;
//     from decl (NOT a key -- canonicalised, defaulted args kept):
//       mlir::OpPrintingFlags
// Both sides agree here (no template, no defaulted args), so unlike t66-t68
// there is no dead sibling spelling to drift onto.
// GATE, MEASURED on TWO TUs that PROVABLY REACH the type (the third TU tried,
// StageCoarsening/Materializer.cpp, has ZERO OpPrintingFlags mentions in its log
// and its zero is therefore WORTHLESS -- the same trap the t59 note records):
//     SplitDFIROutput.cpp   3691 `search expr` lookups, 168 raw mentions, 0 hits
//     WriteSetScan.cpp      3267 `search expr` lookups, 192 raw mentions, 0 hits
// i.e. 0 of 6958 rule lookups mention OpPrintingFlags, while 360 raw mentions
// are clang AST-dump and signature text (`ParmVarDecl ... '::mlir::
// OpPrintingFlags'`) -- DECLARATIONS being lowered, never EXPRESSIONS looked up.
// So the converter needs the TYPE and nothing else at every site it reaches.
// ⛔ WHY OPAQUE AND NOT A STRUCT OF FLAGS.  `dataflowir-gen` has ONE printer and
// it is not configurable: `grep -rn 'PrintingFlags\|printGenericOpForm\|
// enableDebugInfo' dataflowir-gen/src` is empty.  Modelling this as a bag of
// bools would let a TU SET a switch that the Rust printer then IGNORES -- the
// output would differ from C++ while the code looked like it had asked for the
// change.  That is the silently-wrong class, ranked below an abort.  A unit
// cannot lie about a switch it does not have.
// ⛔ THE COST, STATED AND ENFORCED -- and this row's honest caveat.  The
// measured gate zero was taken with the type UNMAPPED, so it cannot prove what
// gets looked up AFTER the declaration stops being a gap.  The source says
// plainly that at least one member IS called:
//     SplitDFIROutput.cpp:130   flags.enableDebugInfo(false);
//     dr5/src/Passes/SPMDizer/DebugIndexer.h:60
//                               OpPrintingFlags().useLocalScope().skipRegions()
// NOT ONE OF THEM IS MAPPED, deliberately -- an unmapped member ABORTS LOUDLY,
// and that abort is the enforcement that makes this unit TRUE rather than
// convenient.  This is exactly t59's bargain (`mlir::Builder`, opaque, every
// real call through it still aborting), and the same reasoning: the bare
// DECLARATION sites -- `mlir::OpPrintingFlags flags;` at dcc.cpp:95 and
// PCFGToDFManager.cpp:132, plus every `const OpPrintingFlags &` parameter -- are
// the 59-TU row, and they need the type alone.  The mutator chain still stops.
// (It would stop anyway one call later: `mod.print(os, flags)` is a ModuleOp
// member and t61 maps no member either.)
using t69 = mlir::OpPrintingFlags;

// t70: `mlir::InFlightDiagnostic` -> `libcc2rs::InFlightDiagnostic`, a REAL
// accumulating buffer that prints on `Drop`.  Argued in full at the class
// declaration above and in libcc2rs/src/diag.rs; in one line, this type's
// destructor is its entire purpose, so an opaque unit would silently delete
// every diagnostic the ported compiler emits.
using t70 = mlir::InFlightDiagnostic;

// ---- WHAT THIS PASS DELIBERATELY LEFT OUT, and why -------------------------
// * `mlir::IndexType::get(mlir::MLIRContext *)`, the ONE factory the verbose logs
//   show in expression position (see t60).  NOT ADDED, because its only honest
//   body is `ir::Ty::Index` -- a body that IGNORES `a0`.  A rule body is INLINED
//   as ONE EXPRESSION at the call site, so dropping `a0` DROPS THE EVALUATION of
//   whatever expression the caller wrote for the context (commonly a call such as
//   `op.getContext()`), changing C++ evaluation.  Getting that right needs the
//   per-overload parameter form read off `-verbose` for the specific call, which
//   this slot did not have time to measure.  LEFT OUT rather than guessed; it
//   aborts loudly, which is the correct state.
// * `mlir::OpBuilder::Listener` and the PassOptions family were listed here as
//   left out; BOTH ENTRIES WERE WRONG and are now DONE as t65-t68 above -- see
//   there for the corrected counts (87 TUs, not 1) and for the `searched as:`
//   spellings.
// * `mlir::detail::PassOptions::ListOption<long>`, and any `mlir::Pass::ListOption
//   <...>` spelling under a DIFFERENT enclosing class.  NOT ADDED: neither
//   appears in the 4-TU measurement set this slot could read a `searched as:`
//   line off, and the whole point of t65-t68 is that the spelling must be READ,
//   never inferred from a sibling.  A key in the wrong spelling is a DEAD key
//   that LOOKS like coverage, so these wait for a slot that can measure them.
//
// ---- THIRD SLOT (2026-09-27): the two rows that got NO key, and why --------
// * `mlir::detail::TypedValue<mlir::VectorType>` (queue row g047, 82 TUs) --
//   NOT LEFT OUT: **ALREADY DONE**, it is `t47` above, landed before this slot
//   began.  Read back out of ir_src.json to be certain (`/t47 ::
//   mlir::detail::TypedValue<mlir::VectorType>`), and the spelling is
//   CHARACTER-FOR-CHARACTER the survey's `searched as:` line.  The queue row is
//   STALE, not open; keying it again would have produced a DUPLICATE, not
//   coverage.  Recorded here so the next slot does not re-derive it.  For the
//   record, the concrete-vs-generic question it poses was already settled by the
//   seven siblings t47-t53: CONCRETE, because a generic `TypedValue<T1>` forces
//   the converter to map the template ARGUMENT and turns a countable mangled
//   name into a hard abort at mapper.cpp:722/:835.
//
// * `mlir::OwningOpRef<mlir::ModuleOp>` (queue row g055, 59 TUs) -- LEFT OUT.
//   ITS GATE FAILS, and it fails on the axis that matters for an OWNING handle.
//   An OwningOpRef is `unique_ptr` for an MLIR op: its DESTRUCTOR ERASES the
//   operation it adopted.  The corpus says so in its own words at
//   dcc/src/Driver/dcc.cpp:63 -- "module_ (OwningOpRef) erases the module it
//   adopted".  So unlike t59/t61/t64/t69, this type has an OBSERVABLE EFFECT AT
//   END OF SCOPE, and an opaque unit has no destructor: mapping it to `()` would
//   SILENTLY DROP the erase rather than abort on it.  That is the one outcome
//   this project ranks below a loud abort, and it is why "no member is mapped, so
//   reads abort" does NOT rescue this row the way it rescues the other four.
//   AND the reads are there too -- `release()`/`get()`/`operator*`/`operator->`
//   are really called on `OwningOpRef<ModuleOp>` values in the 403 TUs:
//       SplitDFIROutput.cpp:158  global_mod->getBodyRegion()
//       SplitDFIROutput.cpp:166  writeModule(global_mod.get(), "global.mlir")
//       SplitDFIROutput.cpp:181  impl_mod->getBodyRegion()
//       SplitDFIROutput.cpp:187  writeModule(impl_mod.get(), filename)
//       dcc/src/Driver/dcc.cpp:96 module_->print(llvm::outs(), flags)
//       dr5/src/Driver/DR5.h:84   returns `OwningOpRef<ModuleOp> &` by reference
//   That is the same TU this slot used as its OpPrintingFlags gate, so it is
//   unambiguously in scope.  ⭐ NOTE the measurement limit that makes this row
//   different from t69's: the verbose logs show 0 raw mentions of OwningOpRef at
//   all, so the log neither confirms nor denies a lookup -- the SOURCE is the
//   evidence, and it is decisive.  A slot that wants this row must model the
//   OWNERSHIP (an owning wrapper whose drop erases), not a unit; there are two
//   further instantiations waiting behind it (`OwningOpRef<mlir::Operation *>`
//   and `OwningOpRef<mlir::ktdf_arch::DeviceOp>`, both with `searched as:` lines
//   already in the baseline TSVs) and they want the same model.



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

// ---------------------------------------------------------------------------
// llvm::ArrayRef<T>.  Declared HERE, not in a new module, because the compile
// measurement that found it is this module's and a fresh module dir that is not
// wired into the build is an INCOMPLETE MODULE DIR that aborts every
// translation project-wide.  `llvm::ArrayRef<llvm::StringRef>` (49 errors) and
// `llvm::ArrayRef<mlir::Attribute>` (35) are both instances of this one rule.
// ---------------------------------------------------------------------------

// ---- mlir::Type::operator! (row g375) -------------------------------------
// `llvm/.../include/mlir/IR/Types.h:97  bool operator!() const { return impl ==
// nullptr; }`.  Spelled `.operator!()` rather than `!a` so the recorded callee is
// unambiguously THIS member and not a negation of some conversion operator --
// `!type` in C++ resolves to the member directly, and that is the callee the
// converter keys on.  No parameter: the receiver is the implicit object argument
// and arrives in the target body as a0 (exactly as for f11 on mlir::Attribute,
// whose key `bool mlir::Attribute::operator!() const` likewise has an empty
// parameter list while its target takes a0).
bool f20(mlir::Type a) { return a.operator!(); }

// ---- f21: the InFlightDiagnostic `<<` family, row g286 --------------------
// `mlir::InFlightDiagnostic && operator shl(const char (&)[_]) &&`, 21 TUs.
// Spelled `std::move(d).operator<<(s)` -- EXPLICIT member-call form on an
// EXPLICIT rvalue -- so the recorded callee is unambiguously the `&&`-qualified
// member template and not any free ADL candidate.  The literal length is written
// concretely (36) because normalizeTranslationRule rewrites every `\b\d+\b` in
// the key to `_`, making this ONE key that serves every literal length; g286's
// sites carry lengths 2, 14, 15, 19 and 36.
mlir::InFlightDiagnostic &&f21(mlir::InFlightDiagnostic &&d,
                               const char (&s)[36]) {
  return std::move(d).operator<<(s);
}
