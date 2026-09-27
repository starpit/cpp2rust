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
} // namespace detail

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
