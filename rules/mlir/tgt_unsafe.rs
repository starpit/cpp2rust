// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// t2601/t2602's f2509 needs `prefix_inc` on a raw pointer, brought in here
// rather than fully-qualified at each call site.
use libcc2rs::UnsafePrefixInc;

// The six MLIR types -> the .td-generated model in `dataflowir-gen`.  src.cpp
// says why the port maps MLIR instead of translating it; this file is only about
// the two things a type rule records: the REPRESENTATION and the `init` that a
// DEFAULT-CONSTRUCTED value of that type becomes.
//
// THE REPRESENTATIONS ARE THE CRATE'S OWN TYPES, NOT WRAPPERS.  `fmt::Block`'s
// `ops` is an ordinary `Vec<OpInst>` and `fmt::Region`'s `blocks` an ordinary
// `Vec<Block>`, so the shapes compose without any handle/arena indirection.  The
// unsafe and refcount targets are IDENTICAL here on purpose: all six are VALUE
// types in this model, so neither model's pointer representation appears.
//
// ---------------------------------------------------------------------------
// THE `init`s, ONE ARGUMENT EACH.  A wrong `init` is silent, so each is either
// the crate's own `Default` or a sentinel that NO REAL VALUE CAN EQUAL.
//
// t1 `mlir::Operation` -- THE INIT IS UNREACHABLE FROM ANY WELL-FORMED C++.
//   `mlir::Operation` has no public default constructor: every Operation is made
//   by `Operation::create` and handed out as `Operation *`.  So no translated
//   program can default-construct one, and this `init` can never be emitted.
//   It still has to be a REAL expression that type-checks, and `OpInst` cannot
//   derive `Default` (its `def` is a `&'static TdOpDef`, a row of the generated
//   table -- there is no "no op").  `mlir.unrealized_conversion_cast` is the row
//   chosen because it is MLIR's OWN stand-in op: zero operands, zero results,
//   and a generated marker exists for it (`ops::mlir_UnrealizedConversionCastOp`),
//   so this is a row the dialect really declares rather than a fabricated one.
//
// t2/t3 `mlir::Block`/`mlir::Region` -- the crate's `Default` IS the faithful
//   answer: `fmt.rs:434,443` derive it, and it gives an unlabelled block with no
//   args and no ops / a region with no blocks, which is exactly what
//   `mlir::Block()` and `mlir::Region()` are.
//
// t4/t5/t6 `Value`/`Type`/`Attribute` -- A NULL HANDLE, MODELLED AS AN EMPTY
//   SPELLING, and this is the one modelling decision in the file.  Unlike
//   Operation, these three ARE default-constructible in C++ and the null handle
//   is a live idiom (`Value v; if (!v) ...`).  The crate has no null variant, so
//   null is carried as the EMPTY SPELLING of the catch-all variant:
//     * `ir::Value { name: "", .. }` -- `name` INCLUDES the sigil (ir.rs:23), so
//       every real value's name starts with `%` and none is empty;
//     * `Ty::Opaque("")` -- a type is carried by its exact printed spelling
//       (ir.rs:44) and no MLIR type prints as nothing;
//     * `Attr::Raw("")` -- likewise (ir.rs:499).  NOT `Attr::Unit`: that is
//       `mlir::UnitAttr`, an attribute that IS PRESENT with no value, which is a
//       different thing from a null Attribute and conflating them would make a
//       null test read as present.
//   So the sentinel is unreachable as a real value in all three cases, which is
//   what makes it a representation of null rather than a collision with data.
// ---------------------------------------------------------------------------

fn t1() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t2() -> dataflowir_gen::fmt::Block {
    dataflowir_gen::fmt::Block::default()
}

fn t3() -> dataflowir_gen::fmt::Region {
    dataflowir_gen::fmt::Region::default()
}

fn t4() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}

fn t5() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(::std::string::String::new())
}

fn t6() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

// t7 `mlir::StringAttr` -> the SAME `ir::Attr`.  In real MLIR StringAttr is a
// derived handle over the one uniqued Attribute hierarchy, and `ir::Attr` is the
// closed union of that hierarchy (`Attr::Str` is the StringAttr row, ir.rs:469),
// so one representation is the faithful one -- not a second wrapper type.  Its
// `init` is the same null handle as t6's: `Attr::Raw("")`.  NOTE the sentinel does
// NOT collide with a StringAttr holding the empty string, which is `Attr::Str("")`
// -- a different variant, so `Attr`'s derived PartialEq reports them unequal and a
// null test on `StringAttr("")` correctly answers "not null".

fn t7() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

// t8 `mlir::AffineMap` -> `dataflowir_gen::ir::AffineMap` (ir.rs:315), whose own
// doc comment names the C++ type it models and the factory it corresponds to.
// The `init` is the NULL HANDLE (`AffineMap() : map(nullptr)`), represented as
// the empty map.  WHY THAT SENTINEL IS SOUND HERE, with evidence: MLIR can in
// principle build a 0-dim/0-symbol/0-result map via `AffineMap::get(0,0,{},ctx)`,
// so the sentinel is not unreachable in MLIR-at-large -- it is unreachable in
// THIS corpus.  Every `AffineMap::get` call site in dt_src passes a non-empty
// result list and >=1 dim (PCFGToDataflowIR.cpp:993/1387/1607/1671/1752-4/2042/
// 2086/2130/2646/2660/3137/4093/4967/5014, SNSyncLowering.cpp:188,
// SNComputeLowering.cpp:137/142/149, PropagationAnalysis.h:91,
// SentientOps.cpp:2122/2140), and `getMultiDimIdentityMap(1,..)`
// (SentientOps.cpp:2106) yields one result.  So no constructible map in this
// program equals the sentinel, and a null test cannot collide with real data.
// ⚠ If a future site builds an empty map, this sentinel becomes AMBIGUOUS and
// the honest fix is an `Option<AffineMap>` representation, not a different
// magic value.
fn t8() -> dataflowir_gen::ir::AffineMap {
    dataflowir_gen::ir::AffineMap::get(0, 0, ::std::vec::Vec::new())
}

// ---------------------------------------------------------------------------
// f1 -- `bool mlir::Attribute::operator==(mlir::Attribute) const`.
// MLIR UNIQUES attributes, so C++ handle equality is instance equality, which is
// value equality of the attribute's contents.  `ir::Attr` derives `PartialEq, Eq`
// (ir.rs:465) over exactly those contents, so `a0 == a1` IS the C++ semantics.
// Both parameters are C++ by-value handles, so both arrive as owned `Attr` and
// the comparison needs no reference juggling.
unsafe fn f1(a0: dataflowir_gen::ir::Attr, a1: dataflowir_gen::ir::Attr) -> bool {
    a0 == a1
}

// f2 -- `bool mlir::operator!=(mlir::StringAttr, std::nullptr_t)`.
// The null-handle test.  `a1` is the `nullptr` literal, which the converter emits
// as `Default::default()` (converter.cpp:3651); it carries no information, so it
// is bound to `()` and deliberately UNUSED -- the answer depends only on a0.
// Null is `Attr::Raw("")`, the same sentinel t6/t7 hand back for a default-
// constructed handle, and it is unreachable as a real attribute spelling.
unsafe fn f2(a0: dataflowir_gen::ir::Attr, a1: ()) -> bool {
    a0 != dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

// ---------------------------------------------------------------------------
// f3/f5 -- the DEFAULT CONSTRUCTORS of the two handles.  Same body as t6/t7's
// `init` and that identity is load-bearing: `mlir::Attribute a;` and a defaulted
// member of a ported struct must produce the same null handle, or a null test
// answers differently depending on how the handle came into being.
unsafe fn f3() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

// f4/f6 -- the COPY CONSTRUCTORS.  A C++ Attribute is a single pointer into the
// uniquer, so a copy is a second handle to the SAME uniqued instance and compares
// equal to its source.  `Attr::clone()` is that: an equal value, independently
// owned, which is what the by-value handle semantics need.
unsafe fn f4(a0: dataflowir_gen::ir::Attr) -> dataflowir_gen::ir::Attr {
    a0.clone()
}

unsafe fn f5() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

unsafe fn f6(a0: dataflowir_gen::ir::Attr) -> dataflowir_gen::ir::Attr {
    a0.clone()
}

// ---------------------------------------------------------------------------
// f7/f8 -- the FREE `==`/`!=` on two `mlir::StringAttr`.  BuiltinAttributes.h
// defines the free `==` as `(Attribute)lhs == (Attribute)rhs`, i.e. it is the
// SAME uniquer-handle comparison as f1, reached through a derived handle.  t7
// maps StringAttr onto the same `ir::Attr`, so `a0 == a1` is that comparison with
// no conversion step to model, and `!=` is its exact negation -- which is how
// MLIR spells it (`!(lhs == rhs)`).
unsafe fn f7(a0: dataflowir_gen::ir::Attr, a1: dataflowir_gen::ir::Attr) -> bool {
    a0 == a1
}

unsafe fn f8(a0: dataflowir_gen::ir::Attr, a1: dataflowir_gen::ir::Attr) -> bool {
    a0 != a1
}

// f9 -- the free `==` against `nullptr`, f2's counterpart and its exact negation.
// `a1` is the `nullptr` literal, emitted as `Default::default()`, carrying no
// information; the answer depends only on a0 being the null sentinel.
unsafe fn f9(a0: dataflowir_gen::ir::Attr, a1: ()) -> bool {
    a0 == dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

// f10 -- the MEMBER `!=` on mlir::Attribute, `!(*this == other)`.
unsafe fn f10(a0: dataflowir_gen::ir::Attr, a1: dataflowir_gen::ir::Attr) -> bool {
    a0 != a1
}

// f11 -- `bool mlir::Attribute::operator!() const`, MLIR's null-handle test
// (`return !impl;`).  True exactly when the handle is null, which in this model is
// exactly the `Attr::Raw("")` sentinel that t6/t7's `init` and f3/f5 produce.
// COMPARED BY VALUE, NOT BY DISPLAY: `Attr::Unit` also prints as the empty string
// (ir.rs), so a Display-based test would report a PRESENT UnitAttr as null.  The
// derived `PartialEq` compares the variant first, so `Raw("") != Unit` and the
// test answers correctly.
unsafe fn f11(a0: dataflowir_gen::ir::Attr) -> bool {
    a0 == dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

// ---------------------------------------------------------------------------
// f12/f13 -- `mlir::AffineMap`'s DEFAULT and COPY constructors.  Mapped for the
// same reason `mlir::Attribute`'s were: without them a TU translates rc=0 and
// then fails to compile with `error[E0433]: cannot find module or crate
// mlir_AffineMap`.  f12's body is byte-identical to t8's `init` on purpose --
// `AffineMap m;` and a defaulted member must produce the same null handle.
// f13 is `clone()`: a C++ AffineMap is one uniquer pointer, so a copy is a second
// handle to the same uniqued map and compares equal to its source.
unsafe fn f12() -> dataflowir_gen::ir::AffineMap {
    dataflowir_gen::ir::AffineMap::get(0, 0, ::std::vec::Vec::new())
}

unsafe fn f13(a0: dataflowir_gen::ir::AffineMap) -> dataflowir_gen::ir::AffineMap {
    a0.clone()
}

// f14/f15 -- `bool mlir::AffineMap::operator==(mlir::AffineMap) const` and its
// `!=`.  AffineMap.h spells them `other.map == map` / `!(other.map == map)`:
// a UNIQUER-HANDLE comparison, and MLIR uniques affine maps on
// (nDims, nSymbols, results), so handle identity IS structural equality of
// exactly those three fields.  `ir::AffineMap` derives `PartialEq, Eq`
// (ir.rs:314) over exactly n_dims/n_symbols/results, with `AffineExpr`'s own
// derived `PartialEq` (ir.rs:123) comparing the result trees structurally.  So
// the two agree, and `!=` is the exact negation MLIR itself writes.
// ⚠ ONE KNOWN DIVERGENCE, stated rather than hidden: MLIR canonicalises on
// construction (constant folding, flattening) and this model does not, so two
// maps that MLIR would unique to one instance -- e.g. `d0 + 0` vs `d0` -- compare
// UNEQUAL here.  Both operands in this corpus come from the same builder path, so
// the shapes match; a `simplifyAffineMap` rule would be required to close it and
// is deliberately absent (see src.cpp).
unsafe fn f14(a0: dataflowir_gen::ir::AffineMap, a1: dataflowir_gen::ir::AffineMap) -> bool {
    a0 == a1
}

unsafe fn f15(a0: dataflowir_gen::ir::AffineMap, a1: dataflowir_gen::ir::AffineMap) -> bool {
    a0 != a1
}

// f16 -- `bool mlir::Value::operator==(mlir::Value) const`, `impl == other.impl`.
// `ir::Value` (ir.rs:21) carries the printed name INCLUDING its `%` sigil plus the
// type; MLIR's printer gives every distinct SSA value in a region a distinct name,
// so name equality is SSA identity and the derived `PartialEq` is the C++
// semantics.  Note this is NOT value-content equality: two structurally identical
// but distinct SSA values have different names and correctly compare unequal.
unsafe fn f16(a0: dataflowir_gen::ir::Value, a1: dataflowir_gen::ir::Value) -> bool {
    a0 == a1
}

// f17-f19 -- the `!=` partners and the mlir::Type pair. Each is a SEPARATE rule
// key: the converter keys on the resolved callee signature and C++17 does not
// rewrite `!=` into `==`, so a TU spelling `a != b` consults f17/f19 and nothing
// else. Both handles compare by the identity their model carries: ir::Value is
// the %-sigilled SSA name (unique per region in MLIR's printer) and ir::Type is
// the printed type, so derived PartialEq is the right relation.
unsafe fn f17(a0: dataflowir_gen::ir::Value, a1: dataflowir_gen::ir::Value) -> bool {
    a0 != a1
}

unsafe fn f18(a0: dataflowir_gen::ir::Ty, a1: dataflowir_gen::ir::Ty) -> bool {
    a0 == a1
}

unsafe fn f19(a0: dataflowir_gen::ir::Ty, a1: dataflowir_gen::ir::Ty) -> bool {
    a0 != a1
}

// --- t9..t19: the types from the first real compile measurement ------------
// THE BORROWED-VIEW DECISION, stated once for OperandRange / ResultRange /
// ValueRange / RegionRange / llvm::ArrayRef.
//
// In C++ all five are NON-OWNING VIEWS: a pointer/iterator pair into storage the
// Operation owns.  Rust has slices, but a slice needs a lifetime, and a rule
// target is INLINED into arbitrary caller code where no lifetime is in scope --
// a `&[T]` target would make every struct FIELD of this type (`odsRegions:
// mlir_RegionRange`, compile1.unsafe.rs:280) unspellable.  So these map to an
// OWNING `Vec<T>`, the same choice rules/array already makes for
// `std::array<T,N>` (extent erased, ownership added).
//
// WHAT THAT COSTS, explicitly:
//   * ALIASING.  A C++ range sees writes made through the Operation after the
//     range was taken; a Vec is a SNAPSHOT.  Code that mutates an operand and
//     re-reads it through a previously-obtained range will read the old value.
//   * COPY COST turns O(1) into O(n), and a `Value` clone is no longer the
//     same object (ir::Value derives Clone, and equality is structural, so
//     comparisons still agree -- but pointer identity does not exist to begin
//     with in the Rust model, so nothing depended on it).
//   * ArrayRef's NULL-vs-EMPTY distinction is lost; both become an empty Vec.
//     MLIR's own accessors never distinguish them.
// A borrowing model is the right long-term answer and needs converter support
// for lifetime-carrying rule targets; it is not something a rule can express.

fn t9() -> dataflowir_gen::ir::AttrDict {
    Default::default()
}

fn t10() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}

fn t11() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}

fn t12() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}

fn t13() -> () {
    ()
}

fn t14() -> Vec<dataflowir_gen::ir::Value> {
    Default::default()
}

fn t15() -> Vec<dataflowir_gen::ir::Value> {
    Default::default()
}

fn t16() -> Vec<dataflowir_gen::ir::Value> {
    Default::default()
}

fn t17() -> Vec<dataflowir_gen::fmt::Region> {
    Default::default()
}

// The null handle.  `OperationName()` is not default-constructible in MLIR, but
// `std::optional<OperationName>`'s empty state and an unregistered name both
// reach a "no registered op" value, and `None` is that value.
// NO LIFETIME IN A RULE TARGET TYPE.  This was first written
// `Option<&'static dataflowir_gen::TdOpDef>` -- the exact shape of MLIR's
// nullable `Impl *` handle -- and the converter MANGLES a parameter's Rust type
// text into the overload-disambiguating function name, producing
// `fn getNumPhasesAttrName_Option&'staticdataflowir_genTdOpDef(...)`, which is
// not an identifier: 2009 lines emitted, then rustc `missing parameters for
// function definition`.  So the handle is spelled as an OWNING copy of the
// registry row.  That is sound because a TdOpDef is immutable static data, and
// it costs only the copy; `None` is still the null handle.
fn t18() -> Option<dataflowir_gen::TdOpDef> {
    None
}

fn t19<T1>() -> Vec<T1> {
    Default::default()
}

// --- t20..t27: the types from the `--mangle-unmapped` triage pass ------------
// Every `init` below is either the crate's own `Default` or the SAME null-handle
// sentinel t6/t7 already use, and each is justified at its declaration in
// src.cpp.  The unsafe and refcount targets are identical here for the reason the
// header gives: these are all VALUE types in this model, so no pointer
// representation appears.
//
// t20 `mlir::BoolAttr` -> `ir::Attr`, whose `Attr::Bool` row (ir.rs:471) IS this
// C++ type.  Null handle = `Attr::Raw("")`, the same sentinel as t6/t7/t10-t12,
// and it does NOT collide with a BoolAttr holding `false` (that is `Attr::Bool
// (false)`, a different variant, so derived PartialEq reports them unequal and a
// null test on `BoolAttr(false)` correctly answers "not null").
fn t20() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}

// t21 `mlir::NamedAttribute` -> the ENTRY TYPE of `ir::AttrDict` (ir.rs:559).
// A default-constructed NamedAttribute is a null name plus a null value, which is
// the empty key plus the same `Attr::Raw("")` null handle.  NOTE: no member rule
// is written for this type, and that is not only the usual caution -- a method
// call whose RECEIVER TYPE CONTAINS A TUPLE is unresolved by the rule
// preprocessor (semantic.rs:261), so `getName()`/`getValue()` could not be
// written in receiver form even if a model existed for them.
fn t21() -> (::std::string::String, dataflowir_gen::ir::Attr) {
    (
        ::std::string::String::new(),
        dataflowir_gen::ir::Attr::Raw(String::new()),
    )
}

// t22 `mlir::Dialect` -> `td_ext::TdDialect` (td_ext.rs:22), the .td parser's own
// dialect record.  `TdDialect` derives `Default` (td_ext.rs:20), and that default
// -- empty def_name/name/cpp_namespace -- is the faithful "no dialect": a real
// registered dialect always has a non-empty `let name`, so the default is not a
// value any registered dialect can equal.
fn t22() -> dataflowir_gen::td_ext::TdDialect {
    Default::default()
}

// t23 `mlir::MLIRContext` -> an OPAQUE UNIT.  See src.cpp for the measurement
// that licenses this: in the reference TU the context appears only as a `*mut`
// threaded through calls and is never dereferenced.  The unit is not a claim that
// a context is empty -- it is a claim that this port never reads one, enforced by
// there being NO member rule, so any call on it aborts loudly.
fn t23() -> () {
    ()
}

// t24 `mlir::OpResult` -> `ir::Value`.  OpResult IS a Value (Value.h:28), so the
// representation is the base's and the `init` is the base's null handle, byte
// identical to t4's -- which it must be, or `Value v = someOpResult;` and a
// default-constructed OpResult would disagree about being null.
fn t24() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}

// t25 `mlir::OpState` -> `fmt::OpInst`, the same representation `mlir::Operation`
// (t1) gets, because an OpState IS one `Operation *`.
//
// ⛔ EQUALITY IS DELIBERATELY ABSENT ON THIS TYPE AND MUST STAY ABSENT.  In C++
// two OpStates are equal iff their `Operation *` are the same object; `OpInst` is
// an op's printed CONTENT, so any comparison written here would report two
// distinct operations with identical content as the same operation.  That is
// silent wrongness no probe catches unless it deliberately builds two identical
// ops.  The TYPE is mapped only so a TU can name it and get as far as the real
// gap; no `operator==`, no `operator!=`, no identity test is provided, so such a
// site still aborts loudly.  Do not add one without a handle-identity model.
//
// The `init` is t1's, and for t1's reason: `OpState` has no public default
// constructor (it is constructed only from an `Operation *`), so this init is
// unreachable from any well-formed C++ and exists only to type-check.
fn t25() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// t26 `mlir::detail::DenseArrayAttrImpl<T>` -> `ir::Attr`, the same widening onto
// the uniqued-Attribute union that t10-t12 and t20 make.  The element type `T` is
// NOT reflected in the representation, because `ir::Attr` has no dense-array
// variant; that is stated as a cost in src.cpp, and it is why no element accessor
// is mapped.  Null handle = the usual `Attr::Raw("")`.
fn t26() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}

// t27 `mlir::scf::ForOp` -> `fmt::OpInst`, an ODS op handle, so t25's
// representation and t25's PROHIBITION: no equality, no identity test.
fn t27() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// t29 -- t26's key spelled `<int64_t>`, the ONLY spelling the mapper ever looks
// up (see the long note on t29 in src.cpp).  Same representation as t26 and for
// t26's reason: a dense i64 array is a member of the one uniqued Attribute
// hierarchy and `ir::Attr` is that hierarchy's closed union; no dense-array
// variant exists (ir.rs:466-499) so no element accessor is mapped.
fn t29<T1>() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}

// t30-t34 `mlir::detail::TypedValue<T>` at its five concrete instantiations ->
// `ir::Value` (ir.rs:21).  `TypedValue<T> : public Value` (mlir/IR/Value.h), so
// this is inheritance, not a widening of kind -- but the STATIC type `T` is not
// represented: `ir::Value` holds its type dynamically in `ty: Ty`, so nothing
// that depends on `T` is mapped.  The init is t4's null handle.
fn t30() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}
fn t31() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}
fn t32() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}
fn t33() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}
fn t34() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}

// t35 `mlir::BlockArgument` -> `ir::Value` (ir.rs:21).  `BlockArgument : public
// Value`.  Neither the owning block nor the argument NUMBER is represented, so
// `getArgNumber()`/`getOwner()` are deliberately unmapped and abort loudly.
fn t35() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}

// t36 `mlir::Operation *` -> `*mut fmt::OpInst`.  SAME MODEL AS t1, POINTER
// SPELLING.  src.cpp says why an explicit pointer key is needed (mapper.cpp:709
// does no pointer stripping) and names the four precedents.  The `init` is the
// NULL POINTER, which is the faithful default: an `Operation *` with no operation
// is exactly `nullptr` in C++, and a null here traps on use rather than
// pretending to be an operation.
fn t36() -> *mut dataflowir_gen::fmt::OpInst {
    ::std::ptr::null_mut()
}

// t37-t39: the range CRTP base at its three concrete instantiations.  Each body
// is IDENTICAL to the derived range's (t14/t15 -> Vec<ir::Value>, t17 ->
// Vec<fmt::Region>) because the base IS the range -- no new representation is
// introduced here.  See src.cpp for why these are concrete and not generic.
fn t37() -> Vec<dataflowir_gen::ir::Value> {
    Default::default()
}

fn t38() -> Vec<dataflowir_gen::ir::Value> {
    Default::default()
}

fn t39() -> Vec<dataflowir_gen::fmt::Region> {
    Default::default()
}

// t40 `mlir::Pass` -> AN OPAQUE UNIT, the same representation `mlir::MLIRContext`
//   (t23) and `mlir::EmptyProperties` (t13) already use, and for the same reason:
//   `dataflowir-gen` models DataflowIR's DATA and carries NO pass
//   infrastructure -- no PassManager, no pipeline, no runOnOperation -- so there
//   is nothing to map a Pass's behaviour onto and this rule maps none of it.
//   What the corpus needs is an OWNED OPAQUE HANDLE: every blocked site reaches
//   the type only as `std::unique_ptr<mlir::Pass>`, constructed by a
//   `createXPass()` factory, moved into a
//   `std::function<std::unique_ptr<Pass>()>` and registered.  `()` is a faithful
//   model of a handle whose contents the port never inspects, and the `init` is
//   the unit value -- which is also the ONLY init this type can have, since
//   `mlir::Pass` is ABSTRACT and no translated program can construct one
//   directly.  (The body is `()`, not empty: `fn t40() -> () {}` panics at
//   syntactic.rs:591 because a type rule's target must YIELD an initializer.)
//   THE COST, STATED: no member is mapped, so any call THROUGH a Pass still
//   aborts loudly in the mapper instead of compiling and lying.
fn t40() -> () {
    ()
}

// t41 `mlir::ShapedType` -> `ir::Ty` (ir.rs:37).  A WIDENING, and the same one
//   this module performs five times over for Value-likes and six times for
//   Attribute-likes: ShapedType is MLIR's type INTERFACE over vector/memref/
//   tensor, every ShapedType IS a `mlir::Type`, and t5 already maps
//   `mlir::Type -> ir::Ty`.  Mapping the interface to the same enum forgets only
//   the CONSTRAINT "this type is shaped"; the SHAPE ITSELF SURVIVES, because
//   `Ty::Vector(Vec<i64>, Box<Ty>)` and `Ty::MemRef(Vec<i64>, Box<Ty>)` carry
//   the dimension list and element type (negative dim = dynamic `?`).  What it
//   costs: `Ty` has no tensor variant, so a RankedTensorType lands in
//   `Ty::Opaque(spelling)` and its shape is only recoverable by reparsing.
//   The `init` is t5's: the EMPTY SPELLING, the null-handle sentinel no real
//   MLIR type can print as.  NO shape accessor is mapped -- getShape,
//   getElementType, getRank, hasStaticShape and cloneWith are all absent and
//   still abort loudly.
fn t41() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(::std::string::String::new())
}

// --- WHAT THIS PASS DELIBERATELY LEFT OUT, with the reason ------------------
// * `mlir::OpOperand` (23 rustc errors).  NOT GROUNDED.  It is not a Value: it is
//   the USE EDGE (`IROperand`, an intrusive node in a value's use-list holding
//   owner + value).  dataflowir-gen models printed IR and has no use-list at all
//   (`grep -rn OpOperand dataflowir-gen/src` = 0 hits), so any mapping would
//   either conflate a use with the value it uses or invent a representation.
//   Left unmapped, so it stays a loud undefined name.
// * `mlir::detail::TypedValue<T>` (5 mangled sites).  A TypedValue IS a Value and
//   `ir::Value` would be the faithful representation -- but a GENERIC type rule
//   makes the converter map the template ARGUMENT, and it then aborts rc=134 with
//   `mlir::IndexType` has no model in types_ (mapper.cpp:722).  Mapping it would
//   therefore require type rules for all five instantiating MLIR types
//   (TokenType, FifoSlotType, RankedTensorType, MemRefType, IndexType) first.
//   Left out: a hard abort in exchange for 5 undefined names is a regression.
// * `mlir::FileLineColLoc` and the `mlir::Location` family.  NOT GROUNDED: the
//   crate has no location type (`grep -n 'Location\|FileLineCol' src/*.rs` finds
//   only doc prose), because DataflowIR's printer emits no locations.
// * `llvm::detail::indexed_accessor_range_base<mlir::RegionRange, ...>` (4
//   errors).  This is the IMPLEMENTATION BASE that OperandRange, ResultRange,
//   ValueRange AND RegionRange all derive from at DIFFERENT element types, so a
//   single generic rule would have to pick one element type and would then be
//   wrong for the other three -- exactly the base-class trap std::atomic hit.  A
//   correct rule is the fully-spelled 5-argument instantiation (it needs local
//   restatements of `llvm::PointerUnion` and `std::unique_ptr`), which is worth
//   doing but is not a guess I will commit blind.

// ---------------------------------------------------------------------------
// t28 `mlir::OpFoldResult` -- THE ONE TYPE IN THIS FILE WHOSE REPRESENTATION IS
// NOT A CRATE TYPE, AND THIS COMMENT IS THE DISCLOSURE.
//
// What the C++ is: `class OpFoldResult : public PointerUnion<Attribute, Value>`
// (mlir/IR/OpDefinition.h:272) -- the result of folding, EITHER a constant
// attribute OR an SSA value.  A two-alternative tagged union, nothing else.
//
// WHAT THE CRATE MODELS FOR IT: NOTHING.  `grep -rnE 'OpFoldResult' \
// dataflowir-gen/src` returns zero hits, and that is expected -- the crate models
// what DataflowIR PRINTS, and a fold result is a transient of the folder, never
// printed.  So unlike every other `tN` here I could not point at a generated row.
//
// WHAT I DID INSTEAD, and why it is not an invention: both ALTERNATIVES already
// have models in this module -- t6 `mlir::Attribute` -> `ir::Attr` and t4
// `mlir::Value` -> `ir::Value`.  The faithful shape is therefore the SUM of two
// existing models, and `Result<A, B>` is std's two-variant sum, used here purely
// structurally: `Ok` = the constant-attribute alternative, `Err` = the SSA-value
// alternative.  No error semantics are implied by `Err`; it is the second variant
// and nothing more.
//
// ⛔ WHY NOT `ir::Attr` ALONE (or `ir::Value` alone).  It would compile, keep the
// TU at rc=0, and be SILENTLY WRONG: `OpFoldResult` genuinely carries both kinds
// (`getMixedSourceSizes()` returns a static size as an `IntegerAttr` and a
// dynamic one as the `Value` that computes it, in one list), so a collapse makes
// half the elements the wrong kind with nothing to observe the difference.  The
// union has to stay a union.
//
// ⚠ WHY NOT A NAMED `ir::OpFoldResult` ENUM.  That is the nicer spelling and it
// is the recommended follow-up, but it means editing
// `dataflowir-gen/src/ir.rs`, which is outside this module and shared: the sweep
// driver `verif/sweep/chk.sh` REFUSES to measure when a crate source file is
// newer than the prebuilt rlib, so adding the enum would have broken every other
// agent's in-flight measurement.  `Result<Attr, Value>` is the same two-variant
// sum with no cross-repo edit, so it was preferred for this pass.
//
// THE `init` is the NULL UNION.  `OpFoldResult()` leaves the PointerUnion null;
// that is represented as the FIRST variant holding the SAME null-Attribute
// sentinel t6/t7/t10-t12/t20 already use, `Attr::Raw("")`, which is unreachable
// as a real attribute spelling.  Choosing the `Ok` side for null is a
// representation choice, not a claim that null is an attribute -- a null fold
// result is what a failed fold returns and this corpus never inspects one.
//
// NO MEMBER IS MAPPED: `is<T>()`, `get<T>()`, `dyn_cast<T>()` and PointerUnion's
// conversions are all absent on purpose, so any TU that actually DISCRIMINATES a
// fold result aborts loudly instead of getting a guessed variant.
fn t28() -> ::std::result::Result<dataflowir_gen::ir::Attr, dataflowir_gen::ir::Value> {
    ::std::result::Result::Ok(dataflowir_gen::ir::Attr::Raw(::std::string::String::new()))
}

// t42 `mlir::TensorType` -> `ir::Ty` (ir.rs:37).  A WIDENING; see src.cpp.  The
//   COST: `Ty` has no tensor variant, so a RankedTensorType is `Ty::Opaque`.
//   Sound only because NO shape accessor is mapped, so every shape query still
//   aborts loudly rather than reading a shape that is not represented.
fn t42() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(String::new())
}

// t43 `mlir::OpOperand` -> AN OPAQUE UNIT.  The USE EDGE.  Mapping it to
//   `ir::Value` is REFUSED (it would conflate a use with the value it uses);
//   see src.cpp.  No member is mapped, so every use still aborts.  The body is
//   `()`, not empty: `fn t43() -> () {}` panics at syntactic.rs:591.
fn t43() -> () {
    ()
}

// t44 `mlir::IntegerSetAttr` -> `ir::Attr` (ir.rs:466).  The seventh attribute
//   widening in this module.  `Attr` has no IntegerSet variant, so the value
//   lands in `Attr::Raw` by spelling; no member is mapped.
fn t44() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}

// t45: the sixth concrete TypedValue -- same body as t30-t34.  The element type
// is not representable in the crate, so the value's static type is Ty::Opaque.
fn t45() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}

// t46: llvm::MutableArrayRef<T1> -- same representation as t19 (ArrayRef).
fn t46<T1>() -> Vec<T1> {
    Vec::new()
}

// t47-t53: the seven further CONCRETE `mlir::detail::TypedValue<T>`
// instantiations (208 occurrences) -> `ir::Value` (ir.rs:21).  IDENTICAL BODY to
// t30-t34/t45, deliberately: `TypedValue<T> : public Value`, so this is
// inheritance and the model is WELL-GROUNDED, not a widening.  The STATIC type
// `T` is DISCARDED -- `ty` is `Ty::Opaque("")` (ir.rs:47) because the crate has
// no handle for VectorType / ktdf_arch::MemoryType / ktdf_arch::ExecutionUnitType
// / ktdp::RuntimeArgType / ktdp::AccessTileType / TensorType / IntegerType -- and
// nothing that depends on `T` is mapped, so such a site still aborts loudly.
fn t47() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}
fn t48() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}
fn t49() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}
fn t50() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}
fn t51() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}
fn t52() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}
fn t53() -> dataflowir_gen::ir::Value {
    dataflowir_gen::ir::Value::new(
        ::std::string::String::new(),
        dataflowir_gen::ir::Ty::Opaque(::std::string::String::new()),
    )
}

// t54 `mlir::TypeAttr` -> `ir::Attr` (ir.rs:466).  A WIDENING, the 8th in this
//   module.  `Attr` has NO variant carrying an `ir::Ty`, so the wrapped type
//   lands in `Attr::Raw` by spelling; `getValue()`/`TypeAttr::get()` are NOT
//   mapped, so reading the wrapped type still aborts loudly.  The init is the
//   same null-handle sentinel t6/t7/t44 use.
fn t54() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}

// t55-t57 `mlir::ElementsAttr` and its two subclasses `SparseElementsAttr` /
//   `DenseIntElementsAttr` -> `ir::Attr` (ir.rs:466).  WIDENINGS, 9th-11th.
//   ⛔ `Attr` HAS NO DENSE OR SPARSE ELEMENT-ARRAY VARIANT, so the element
//   payload lands in `Attr::Raw`/`Attr::Aliasable` BY SPELLING.  `Attr::I32Array`
//   (ir.rs:491) is NOT the right target for DenseIntElementsAttr and is not used:
//   its doc comment says it is `mlir::ArrayAttr` of `IntegerAttr`s (ODS
//   `I32ArrayAttr`, printed `[1 : i32, 2 : i32]`), a different MLIR class with a
//   different printed form, i32 rather than APInt elements, and no shaped type.
//   NO element query is mapped (`getValues<T>()`, `getElementType()`,
//   `getNumElements()`, `isSplat()`, `operator[]`, `getIndices()`), so every
//   element access still aborts loudly instead of reading an empty array.
fn t55() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}
fn t56() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}
fn t57() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}

// t58 `mlir::detail::InterfaceMap` -> AN OPAQUE UNIT, the representation t23
//   (MLIRContext), t40 (Pass) and t43 (OpOperand) already use.  MLIR's runtime
//   interface dispatch table; `grep -rn InterfaceMap dataflowir-gen/src` = 0
//   hits, the crate models printed IR and has no interface dispatch to map onto.
//   ⛔ NO MEMBER IS MAPPED (`lookup`, `contains`, `insert`, `get<...>`), so any
//   real dispatch through one still aborts loudly.  The body is `()`, not empty:
//   `fn t58() -> () {}` panics at syntactic.rs:591.
fn t58() -> () {
    ()
}

// t59 `mlir::Builder` -> AN OPAQUE UNIT.  102 TUs, the biggest row in the module.
//   THE GATE IS CLEARED BY MEASUREMENT: `grep -A1 'search expr'` over the two
//   verbose logs that REACH the site found ZERO lookups on any Builder member
//   (the same grep finds 124 for IndexType), so the converter needs only the
//   TYPE.  See src.cpp for the log line counts.  NO member is mapped, so every
//   real call through a Builder still aborts loudly.  The body is `()`, not
//   empty: `fn t59() -> () {}` panics at syntactic.rs:591.
fn t59() -> () {
    ()
}

// t60 `mlir::IndexType` -> `ir::Ty` (ir.rs:37).  A WIDENING, the same one t41 and
//   t42 make.  The `init` is t5's EMPTY-SPELLING NULL SENTINEL, NOT `Ty::Index`:
//   a default-constructed IndexType is a NULL handle and claiming `Ty::Index`
//   would assert a live index type where C++ has none.  `Ty::Index` (ir.rs:39)
//   IS the value a real one carries, which is why this row is better grounded
//   than t41/t42 -- nothing lands in `Ty::Opaque(spelling)`.  NO accessor and
//   NOT the `IndexType::get(MLIRContext*)` factory are mapped; see src.cpp.
fn t60() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(::std::string::String::new())
}

// t61 `mlir::ModuleOp` -> AN OPAQUE UNIT.  `fmt::OpInst` was CHECKED AND REFUSED:
//   an OpInst's `def` must be a row of the GENERATED TD_OPS table (fmt.rs:388)
//   and `builtin.module` is an MLIR builtin with no such row.  NO EQUALITY is
//   added for this or any op handle -- mapping a handle onto a printed-content
//   type would make two handles to ONE op, and two handles to two
//   identically-printing ops, compare the wrong way round.  Body is `()`.
// ⭐⭐ THE "no such row" HALF IS RETRACTED -- `builtin` IS in build.rs's
//   UPSTREAM_FILES and `cargo test --test isa` proves the row and the marker
//   `mlir_ModuleOp` exist.  The FULL retraction, and the measurement that keeps
//   `mlir::OperationPass<mlir::ModuleOp>` aborting loudly anyway, is at the t61
//   block in `src.cpp`.  The NO-EQUALITY half stands unchanged.
//
// ⛔⛔ SPECIFICATION FOR `dataflowir-gen`, NOT EDITED HERE (crate slot's half).
//   Nothing in this module changes until this lands.  Everything below is READ OFF
//   THE CONVERTER'S OWN EMITTED TEXT, not designed:
//
//   FILE      dataflowir-gen/src/fmt.rs
//   IMPL      `impl OpInst` -- the one opened at fmt.rs:728 (NOT the `Debug` impl
//             at :716).  ANCHOR: immediately AFTER `pub fn find_op` / before
//             `pub fn print`, i.e. anywhere in that block; order is not load-bearing.
//
//   STEP 0 (MANDATORY, AND IT IS A HARD ERROR IF SKIPPED)
//             RENAME the existing PRIVATE `fn walk(&self, elems, elided, e, emit,
//             ctx)` at fmt.rs:1238 to `fn walk_format(...)` and update its two
//             callers (fmt.rs:1135 and the self-recursive call at fmt.rs:1347).
//             ⛔ The name collides: real rustc on the converter's emitted shape
//             gives `error[E0624]: method `walk` is private`, NOT E0599, so a
//             `pub fn walk` added beside it does not compile.  This is MEASURED.
//
//   STEP 1    pub fn walk<T: crate::isa::MlirOp>(&self, f: &mut impl FnMut(&OpInst))
//             -- MLIR's `Operation::walk` with the callback's argument type as the
//             FILTER.  PRE-ORDER, and pre-order is the semantics not a choice:
//             `dcc/src/Utils/Utils.cpp:279` returns `WalkResult::skip()` to collect
//             the OUTERMOST `ProgramUnitOp`s, which only works pre-order.
//             Traversal: `self` first if `self.is_a::<T>()`, then for each
//             `r in &self.regions`, each `b in r.get_blocks()`, each
//             `o in b.get_operations()`, recurse.
//   STEP 2    pub fn walk_r<T: crate::isa::MlirOp>(&self, f: &mut impl FnMut(&OpInst)
//                 -> crate::ir::LocWalkResult) -> crate::ir::LocWalkResult
//             -- the `WalkResult`-returning C++ OVERLOAD.  BOTH are needed: MLIR
//             deduces `RetT` from the lambda and the corpus uses both forms.
//             `Interrupt` abandons the whole walk and PROPAGATES; `Skip` prunes THIS
//             node's subtree and continues with siblings and NEVER propagates out
//             (ir.rs:988 already states that contract for `Location::walk`; match it).
//   STEP 3    the MUTATING overloads, `walk_mut` / `walk_r_mut`, taking
//             `&mut self` and `&mut impl FnMut(&mut OpInst)`.  `get_blocks_mut`
//             (fmt.rs:639) and `get_operations_mut` (fmt.rs:497) already exist, and
//             22 of the 42 `OperationPass<ModuleOp>` walk bodies MUTATE (they
//             `push` into a vector, set insertion points, or rewrite).
//   STEP 4    an UNTYPED form for the 3 corpus sites whose callback takes
//             `Operation*` rather than a typed op -- either `walk::<AnyOp>` with a
//             blanket marker, or a separate `walk_any`.  ⛔ DO NOT make `T`
//             defaultable to "match everything" silently; an unfiltered walk where
//             the C++ filtered visits ops the C++ never saw.
//
//   ⛔ IT CANNOT LIVE IN A RULE BODY, and this is not a preference.  The traversal is
//   SELF-RECURSIVE, and `converter c86b7748` records that "a self-recursive lambda has
//   no finite inline expansion" -- it was the last SEGV.  Same argument fmt.rs:917
//   makes for `set_operands_flat`.
//
//   THE TEST THAT PROVES IT, and it must be this one rather than a synthetic tree:
//   `tests/module.rs` already builds ONE `OpInst` for the whole `builtin.module` of
//   two real 1,792- and 7,283-line reference files.  Add to `tests/` :
//     (a) `walk::<ops::mlir_dataflow_ProgramUnitOp>` over `matmul_demo`'s module
//         visits exactly as many ops as `print_generic` shows `dataflow.program_unit`
//         lines -- a count DERIVED from the file, not chosen;
//     (b) a `walk_r` returning `Skip` at the first `ProgramUnitOp` collects only the
//         OUTERMOST ones, and the count is STRICTLY LESS than (a) when nesting
//         exists -- this is the `Utils.cpp:279` semantics and the one thing a
//         two-variant `WalkResult` could not express;
//     (c) `walk_r` returning `Interrupt` visits exactly 1 op and the returned value
//         satisfies `was_interrupted()`, while a walk that only ever `Skip`ped does
//         NOT (ir.rs's own contract);
//     (d) pre-order: the FIRST op visited by an unfiltered walk is the module itself.
//
//   ⭐ WHAT IT BUYS IMMEDIATELY, measured, independent of this row: the 10 already-
//   emitted `.walk(&mut _callback)` sites in 7 bucket-A TUs of the `fresh39` sweep
//   stop being silent compile errors.  See src.cpp for the file list.
fn t61() -> () {
    ()
}

// t62 `mlir::detail::IROperandBase` -> AN OPAQUE UNIT.  The UNTYPED BASE of the
//   use edge; `OpOperand` derives from it, so t43's refusal to map the use edge
//   as `ir::Value` applies unchanged and is now backed by t43 taking its row to
//   0 across 5 TUs.  NO member mapped; the link walk still aborts.  Body `()`.
fn t62() -> () {
    ()
}

// t63 `mlir::OperationState` -> AN OPAQUE UNIT.  MLIR's construction bag, filled
//   by a generated `build()` and consumed by `Operation::create` -- neither of
//   which this port reproduces.  The unit claims this port never READS one, and
//   that claim is ENFORCED: no member is mapped, so `addOperands`/`addTypes`
//   abort loudly.  Body `()`.
fn t63() -> () {
    ()
}

// t64 `mlir::PassManager` -> AN OPAQUE UNIT.  t40's comment on `mlir::Pass`
//   already states the ground: `dataflowir-gen` has NO PassManager, no pipeline,
//   no runOnOperation to map behaviour onto.  Same refusal, same cost -- no
//   member mapped, `addPass`/`run` abort loudly.  Body `()`.
fn t64() -> () {
    ()
}

// t65 `mlir::OpBuilder::Listener` -> AN OPAQUE UNIT.  87 TUs -- the BIGGEST row
//   left in this module.  MLIR's insertion-callback interface (the
//   `notifyOperationInserted` / `notifyBlockInserted` virtuals an OpBuilder calls
//   as it mutates IR).  `dataflowir-gen` BUILDS NO IR: it models PRINTED
//   DataflowIR, so there is no insertion to be notified of and no callback
//   registry to map onto.  Same ground as t40 (`Pass`) and t64 (`PassManager`).
//   Reached ONLY while lowering a signature (`Listener *listener`), measured: 0
//   of 4229 rule lookups on an aborting TU mention it.  NO member mapped -- a
//   real hook invocation still aborts loudly.  Body is `()`.
fn t65() -> () {
    ()
}

// t66 `mlir::detail::PassOptions::Option<int>` -> AN OPAQUE UNIT.  59 TUs.  A
//   command-line-backed pass option (`llvm::cl::opt<int>` + MLIR's `OptionBase`).
//   ⛔ NOT `i32`: the value comes from argv parsing inside `llvm::cl`, which this
//   port does not translate, so an `i32` would be a DEFAULT-INITIALISED ZERO
//   silently standing in for whatever the user passed -- the silently-wrong
//   class the playbook ranks below an abort.  `getValue()` and friends are
//   absent, so any READ aborts.  Body is `()`.
fn t66() -> () {
    ()
}

// t67 `mlir::detail::PassOptions::ListOption<std::string>` -> AN OPAQUE UNIT.
//   59 TUs, 8 occurrences in the measurement set.  The comma-separated list form.
//   ⛔ NOT `Vec<String>`: an EMPTY vec is not a neutral stand-in for an unparsed
//   option list -- a loop over it runs ZERO iterations and the TU silently does
//   nothing.  No iteration or indexing mapped.  Body is `()`.
fn t67() -> () {
    ()
}

// t68 `mlir::detail::PassOptions::ListOption<int>` -> AN OPAQUE UNIT.  59 TUs, 12
//   occurrences -- the largest single count in the measurement set.  A SECOND
//   INSTANTIATION of t67's template, keyed separately because the key carries the
//   concrete argument.  Identical model, identical refusal.  Body is `()`.
fn t68() -> () {
    ()
}

// t69 `mlir::OpPrintingFlags` -> AN OPAQUE UNIT.  59 TUs.  MLIR's PRINTING-OPTIONS
//   BAG (OperationSupport.h:1176): the debug-info / generic-op-form / local-scope
//   / region-elision switches that `Operation::print` and `AsmState` consult.
//   ⛔ NOT A STRUCT OF BOOLS: `dataflowir-gen` has ONE printer and it is NOT
//   configurable (`grep -rn "PrintingFlags|printGenericOpForm|enableDebugInfo"
//   dataflowir-gen/src` is EMPTY).  A bag of bools would let a TU SET a switch
//   the Rust printer then IGNORES -- output silently differing from C++ while the
//   code looks like it asked for the change.  A unit cannot lie about a switch it
//   does not have.
//   GATE: 0 of 6958 `search expr` rule lookups mention it, across the two TUs
//   that PROVABLY reach it (SplitDFIROutput.cpp 3691 lookups / 168 raw mentions;
//   WriteSetScan.cpp 3267 / 192).  All 360 raw mentions are clang AST-dump and
//   signature text -- declarations being lowered, not expressions looked up.
//   ⛔ NO MEMBER MAPPED, and the source DOES call two of them
//   (`flags.enableDebugInfo(false)` at SplitDFIROutput.cpp:130,
//   `OpPrintingFlags().useLocalScope().skipRegions()` at DebugIndexer.h:60).
//   They ABORT LOUDLY, and that abort is what makes this unit true rather than
//   convenient -- exactly t59's (`mlir::Builder`) bargain.  What the row buys is
//   the 59 TUs' bare DECLARATION and PARAMETER sites.  Body is `()`.
fn t69() -> () {
    ()
}

// f20 -- `bool mlir::Type::operator!() const` (Types.h:97, `return impl ==
// nullptr;`), MLIR's null-handle test on the Type handle.  The model of a null
// `mlir::Type` in this module is t5's EMPTY SPELLING of the catch-all variant,
// `Ty::Opaque("")` -- see the t4/t5/t6 note at the top of this file -- so the
// null test is a value comparison against exactly that sentinel.  COMPARED BY
// VALUE, NOT BY DISPLAY, for the same reason as f11: `Ty::Int(0)` etc. are
// distinct variants that must not be mistaken for a null handle, and no real
// MLIR type prints as the empty string.  a0 is the receiver.
unsafe fn f20(a0: dataflowir_gen::ir::Ty) -> bool {
    a0 == dataflowir_gen::ir::Ty::Opaque(::std::string::String::new())
}

// t70 `mlir::InFlightDiagnostic` -> `libcc2rs::InFlightDiagnostic`, A REAL
//   ACCUMULATING BUFFER THAT PRINTS ON `Drop` -- NOT an opaque unit.  This is the
//   one type in this module whose DESTRUCTOR is its entire purpose:
//     Diagnostics.h:325-328  ~InFlightDiagnostic() { if (isInFlight()) report(); }
//     Diagnostics.h:319-324  the move ctor explicitly `rhs.abandon()`s
//   so the accumulated message reaches the DiagnosticEngine ON DESTRUCTION,
//   exactly once.  ⛔ A UNIT WOULD SILENTLY DELETE EVERY DIAGNOSTIC the ported
//   compiler emits -- the same axis on which `mlir::OwningOpRef` was REFUSED
//   (src.cpp, "THIRD SLOT"), and worse here because the dominant call sites are
//   tablegen-generated parse/verify bodies (KTDFAttributes.cpp.inc:132,
//   KTDFLowering.cpp.inc:30, SDSCBundleTypes.cpp.inc:216) whose ONLY externally
//   visible behaviour IS the message.
//   THE INIT is a fresh in-flight diagnostic with an empty message, which is
//   exactly what `InFlightDiagnostic()` (Diagnostics.h:316) is.
//   WHERE IT PRINTS: stderr, prefixed `error: `, matching MLIR's default handler
//   (`llvm::errs()`); libcc2rs/src/diag.rs argues this.
fn t70() -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::new()
}

// ---------------------------------------------------------------------------
// t71-t76 -- THE SIX ROWS ADDED 2026-09-27 (fourth slot).  src.cpp carries the
// full argument for each, including the OWNING HEADER LINE each spelling was
// read from and the zero-hit `~TypeName` destructor grep that clears all six for
// a non-Drop model.  In one paragraph:
//
// t71 `mlir::UnitAttr`   -> `ir::Attr` (ir.rs:466).  ⭐ WELL-GROUNDED, not a
//   widening: `Attr::Unit` (ir.rs:473) is documented in the crate as
//   `mlir::UnitAttr` itself.  ⛔ But the INIT IS NOT `Attr::Unit` -- a
//   default-constructed UnitAttr is a NULL HANDLE and `Attr::Unit` is a REAL
//   present attribute, so the init is t5's empty-spelling `Attr::Raw("")`
//   sentinel.  `UnitAttr::get(ctx)`, the factory that would yield `Attr::Unit`,
//   is NOT mapped and still aborts loudly.
fn t71() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

// t72 `mlir::TypeID` -> AN OPAQUE UNIT, the same model as t58/t59/t69.  MLIR's
//   RTTI token, one pointer into a per-type static `Storage` (TypeID.h:112-115);
//   no `~TypeID` exists and it owns nothing, so the drop is effect-free.
//   ⛔ WHAT IS LOST IS THE ENTIRE POINT OF THE TYPE -- IDENTITY.  Every
//   translated TypeID is the same `()`, so `operator==`/`operator!=`
//   (TypeID.h:118-123) are DELIBERATELY NOT MAPPED: mapping them would make
//   every type compare EQUAL to every other, which is silently wrong and ranks
//   BELOW an abort.  `TypeID::get<T>()` is likewise absent.  The body is `()`,
//   not empty: an empty body panics at syntactic.rs:591.
fn t72() -> () {
    ()
}

// t73 `mlir::MemRefType` -> `ir::Ty` (ir.rs:37).  A WIDENING, the same one t41,
//   t42 and t60 make.  ⭐ BETTER GROUNDED than t41/t42: `Ty::MemRef(Vec<i64>,
//   Box<Ty>)` (ir.rs:44) carries the dimension list and element type, negative
//   dim = dynamic `?`, so the SHAPE SURVIVES.  ⛔ The init is still the
//   empty-spelling `Ty::Opaque("")` null sentinel, because an empty-shaped
//   memref is a REAL type (`memref<f32>`) and a null handle is not.  NO
//   accessor is mapped -- getShape/getElementType/getRank/getLayout/
//   getMemorySpace/MemRefType::get all still abort.
fn t73() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(::std::string::String::new())
}

// t74 `mlir::TypedAttr` -> `ir::Attr` (ir.rs:466).  A WIDENING, the 12th, with
//   the interface-over-subclasses shape of t41/t42/t55.  ⛔ `Attr` has no
//   variant pairing an arbitrary payload with a `Ty` except the
//   IntegerAttr-specific `Attr::Int(i64, Ty)` (ir.rs:469), so a non-integer
//   TypedAttr lands in `Attr::Raw` BY SPELLING and `getType()` -- the one member
//   the interface exists for -- is NOT mapped and still aborts.
fn t74() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

// t75 `mlir::VectorType` -> `ir::Ty` (ir.rs:37).  The BEST-GROUNDED of the two
//   type rows: `Ty::Vector(Vec<i64>, Box<Ty>)` is documented at ir.rs:41-42 as
//   `vector<64xf16> -- mlir::VectorType`, this exact class by name.  ⛔ Same two
//   losses as t73: null-handle init rather than a real empty vector type, and NO
//   accessor mapped.
fn t75() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(::std::string::String::new())
}

// t76 `mlir::FlatSymbolRefAttr` -> `ir::Attr` (ir.rs:466).  A WIDENING, the
//   13th.  ⛔ `Attr` HAS NO SYMBOL-REFERENCE VARIANT (`grep -n
//   'SymbolRef\|Flat' ir.rs` = zero symbol-ref hits), so the referenced symbol
//   NAME lands in `Attr::Raw` BY SPELLING -- the printed `@name` form
//   round-trips, the ability to RESOLVE the symbol does not, and the crate has
//   that capability for no attribute.  getValue()/getAttr()/get are NOT mapped.
fn t76() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

// f21 -- `mlir::InFlightDiagnostic && operator shl(const char (&)[_]) &&`
// (Diagnostics.h:344-349's member template, instantiated on a string literal),
// work-queue row g286, 21 TUs.  a0 IS THE RECEIVER (see src.cpp for why the
// shift family's key omits it) and a1 the streamed literal.
//
// `a0` and `a1` ARE EACH MENTIONED EXACTLY ONCE.  A rule body is INLINED as one
// expression and every `aN` RE-EVALUATES its argument, so mentioning a0 twice
// would DUPLICATE the diagnostic -- for this type that is a second message, not
// a wasted copy.
//
// EXACTLY-ONCE COMES FROM BY-VALUE `self`.  `shl_c_str` takes `self` by value and
// returns `Self`, so the chain MOVES one value through and a moved-from binding
// is statically dead -- Rust does not run `Drop` for it.  The single `drop` fires
// at the end of the enclosing full expression, which is when the C++ temporary
// `InFlightDiagnostic` dies.  This is the model of C++'s
// move-ctor-plus-`abandon()`, without needing an explicit abandon per link.
//
// `a1` IS A `*const c_char`, NOT A SLICE: rules/stringref f8 records why -- the
// converter materialises a string literal in this position as a `c"..."` CStr and
// adds `.as_ptr()` only when the declared parameter is a pointer.
unsafe fn f21(a0: libcc2rs::InFlightDiagnostic, a1: &std::ffi::CStr) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_bytes(a0, a1.to_bytes())
}

// ---------------------------------------------------------------------------
// f22-f38 -- the rest of the InFlightDiagnostic `<<` family.  src.cpp carries the
// reasoning; the two invariants that matter in EVERY body below are:
//   * `a0` AND `a1` ARE EACH MENTIONED EXACTLY ONCE.  A rule body is inlined as
//     one expression and every `aN` re-evaluates its argument, so a second
//     mention of a0 would emit a SECOND diagnostic and a second mention of a1
//     would re-run whatever produced the streamed value.
//   * exactly-once reporting comes from `self` BY VALUE: the chain moves one
//     buffer through and `Drop` runs once, at end of full expression.
// An LVALUE-reference argument (`T &` in the key) takes a Rust SHARED REFERENCE
// rather than a value, so inlining cannot move the caller's variable.

// f22 -- row g320, `llvm::StringRef &&`, 10 TUs
unsafe fn f22(a0: libcc2rs::InFlightDiagnostic, a1: Vec<libc::c_char>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_c_chars(a0, &a1)
}

// f23 -- row g346, `llvm::StringRef &`, 8 TUs
unsafe fn f23(a0: libcc2rs::InFlightDiagnostic, a1: &Vec<libc::c_char>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_c_chars(a0, a1)
}

// f24 -- row g396, `std::string &`, 5 TUs
unsafe fn f24(a0: libcc2rs::InFlightDiagnostic, a1: &Vec<libc::c_char>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_c_chars(a0, a1)
}

// f25 -- row g491, `mlir::Type &`, 3 TUs -- Display is MLIR's type syntax (ir.rs:50)
unsafe fn f25(a0: libcc2rs::InFlightDiagnostic, a1: &dataflowir_gen::ir::Ty) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f26 -- row g553, `unsigned int &`, 2 TUs
unsafe fn f26(a0: libcc2rs::InFlightDiagnostic, a1: &u32) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f27 -- row g1151 + g1161, `int &`
unsafe fn f27(a0: libcc2rs::InFlightDiagnostic, a1: &i32) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f28 -- row g1152 + g1164, `long &`
unsafe fn f28(a0: libcc2rs::InFlightDiagnostic, a1: &i64) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f29 -- row g1150, `const long &`
unsafe fn f29(a0: libcc2rs::InFlightDiagnostic, a1: &i64) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f30 -- row g1160, `const int &`
unsafe fn f30(a0: libcc2rs::InFlightDiagnostic, a1: &i32) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f31 -- row g1162, `const unsigned int &`
unsafe fn f31(a0: libcc2rs::InFlightDiagnostic, a1: &u32) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f32 -- row g1163, `unsigned int &&`
unsafe fn f32(a0: libcc2rs::InFlightDiagnostic, a1: u32) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f33 -- row g1157, `unsigned long &&`
unsafe fn f33(a0: libcc2rs::InFlightDiagnostic, a1: u64) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f34 -- row g1158, `const std::string &`
unsafe fn f34(a0: libcc2rs::InFlightDiagnostic, a1: &Vec<libc::c_char>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_c_chars(a0, a1)
}

// f35 -- row g1159, `std::string &&`
unsafe fn f35(a0: libcc2rs::InFlightDiagnostic, a1: Vec<libc::c_char>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_c_chars(a0, &a1)
}

// f36 -- row g1155, `mlir::Attribute &` -- Display is MLIR's attr syntax (ir.rs:521)
unsafe fn f36(a0: libcc2rs::InFlightDiagnostic, a1: &dataflowir_gen::ir::Attr) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f37 -- row g1156, `mlir::StringAttr &&` (t7 -> the same Attr)
unsafe fn f37(a0: libcc2rs::InFlightDiagnostic, a1: dataflowir_gen::ir::Attr) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f38 -- row g1154, `llvm::StringLiteral &&`
unsafe fn f38(a0: libcc2rs::InFlightDiagnostic, a1: Vec<libc::c_char>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_c_chars(a0, &a1)
}

// f39 -- the default constructor, `mlir::InFlightDiagnostic d;`.  A fresh,
// in-flight, empty diagnostic; `live` is true so Drop reports it (diag.rs).
unsafe fn f39() -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::new()
}

// f40-f45 -- THE DEFAULT CONSTRUCTORS FOR t71-t76, one per type key.  A `using
// tN =` maps the TYPE ONLY; the converter looks `void <T>::<T>()` up as an
// ORDINARY EXPR RULE and on a miss emits `<mangled type>::new()`, which does not
// exist -- rc=0 and then `error[E0433]`.  Each body is the SAME null-handle
// sentinel as its type's `init` above, NOT a valid value.
unsafe fn f40() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}
unsafe fn f41() -> () {
    ()
}
unsafe fn f42() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(::std::string::String::new())
}
unsafe fn f43() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}
unsafe fn f44() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(::std::string::String::new())
}
unsafe fn f45() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

// t77 -- `llvm::sys::SmartMutex<true>`, the same model rules/mutex gives
// std::mutex.  lock/unlock/try_lock are NOT keyed, so the recursive-vs-plain
// difference is unobservable; see the src.cpp note.
fn t77() -> ::std::sync::Mutex<()> {
    ::std::sync::Mutex::new(())
}

// t78 -- `llvm::cl::desc`, a one-field StringRef modifier; the StringRef model
// from rules/stringref t1.
fn t78() -> Vec<libc::c_char> {
    vec![0]
}

// f46 -- the default constructor for t77, a fresh unlocked mutex.
unsafe fn f46() -> ::std::sync::Mutex<()> {
    ::std::sync::Mutex::new(())
}

// f47 -- `cl::desc(StringRef)`: the modifier IS its payload, so the constructor
// is the identity on the StringRef bytes.
unsafe fn f47(a0: Vec<libc::c_char>) -> Vec<libc::c_char> {
    a0
}

// t79 -- `llvm::BitVector` -> one Rust bool per bit.  The payload IS read by the
// C++ (operator[]/test/count), so a unit would lose it; no member is keyed, so the
// packed-word representation difference is unobservable.  See the src.cpp note.
fn t79() -> Vec<bool> {
    Vec::new()
}

// t80 -- `mlir::detail::PreservedAnalyses` -> `()`.  Its payload is a set of
// `mlir::TypeID`, and t72 maps TypeID to `()` with `==` deliberately absent, so a
// set of them has no representable contents.  `()` says exactly as much as t72.
fn t80() -> () {
    ()
}

// f48 -- the default constructor for t79: `BitVector() = default;` leaves the
// storage empty and Size = 0, i.e. an empty Vec.
unsafe fn f48() -> Vec<bool> {
    Vec::new()
}

// f49 -- the implicit default constructor for t80; the unit, per t80.
unsafe fn f49() -> () {
    ()
}

// t81 -- `mlir::CopyOnWriteArrayRef<T1>` -> `Vec<T1>`, the SAME representation
// t19 gives `llvm::ArrayRef<T1>`.  Unusually for this module the honest model is
// a real OWNING container rather than a unit: the C++ holds an ArrayRef view and
// a SmallVector copy and switches between them on write, which is an allocation
// strategy, not something any operation can observe.  No member is keyed, so the
// difference cannot leak.  See the src.cpp note for the destructor test.
fn t81<T1>() -> Vec<T1> {
    Vec::new()
}

// t82 -- `mlir::DominanceInfo` -> `()`.  Its payload is a per-Region CACHE of
// dominator trees, and every query that would read it is deliberately UNMAPPED,
// so no code can ask this model a dominance question -- it aborts instead of
// answering `true`/`false` from nothing.  t72/t79/t80 precedent.
fn t82() -> () {
    ()
}

// f50 -- `CopyOnWriteArrayRef(ArrayRef<T> array)`, the class's ONLY constructor,
// taking its ArrayRef BY VALUE (ADTExtras.h:27).  Identity on the elements: the
// C++ parks the view in `nonOwning` and copies into `owningStorage` only on the
// first write, and `Vec<T1>` collapses both storages into one.
unsafe fn f50<T1>(a0: Vec<T1>) -> Vec<T1> {
    a0
}

// f51 -- `DominanceInfo di;`, the 0-ary form.  The unit, per t82.
unsafe fn f51() -> () {
    ()
}

// f52 -- `DominanceInfo di(op);`.  The unit per t82, but the body CONSUMES `a0`
// rather than ignoring it: a rule body is inlined as one expression, so dropping
// `a0` would drop the caller's expression (`unit_op`, `func`, `module_op`) and
// change C++ evaluation.  `drop` evaluates it and yields `()`.
unsafe fn f52(a0: *mut dataflowir_gen::fmt::OpInst) -> () {
    drop(a0)
}

// f53/f54 -- the free ArrayRef comparisons.  Shape copied verbatim from
// rules/vector f115-f118, including the `&Vec<T1>` parameters and the
// `T1: PartialEq` bound.
unsafe fn f53<T1: PartialEq>(a0: &Vec<T1>, a1: &Vec<T1>) -> bool {
    a0 == a1
}

unsafe fn f54<T1: PartialEq>(a0: &Vec<T1>, a1: &Vec<T1>) -> bool {
    a0 != a1
}

// f55 -- ArrayRef(std::initializer_list<T1>).  rules/initializer_list maps
// `std::initializer_list<T1>` to `Vec<T1>`, so this is the identity, exactly as
// rules/vector f36 is for `std::vector`'s initializer-list constructor.
unsafe fn f55<T1>(a0: Vec<T1>) -> Vec<T1> {
    a0
}

unsafe fn f56<T1>(a0: &Vec<T1>) -> usize {
    a0.len()
}

// f57 -- `erase(index)` is void in C++ and `Vec::remove` yields the element, so
// the result is discarded.  `drop(..)` keeps the body ONE expression yielding ().
unsafe fn f57<T1>(a0: &mut Vec<T1>, a1: usize) {
    drop(a0.remove(a1))
}

unsafe fn f58<T1>(a0: &mut Vec<T1>, a1: usize, a2: T1) {
    a0.insert(a1, a2)
}

unsafe fn f59<T1>(a0: &mut Vec<T1>, a1: Vec<T1>) {
    let __src = a1;
    a0.clear();
    a0.extend(__src);
}

unsafe fn f60<T1: Clone>(a0: &Vec<T1>) -> Vec<T1> {
    a0.clone()
}

// t83 -- `mlir::AffineExpr` -> `dataflowir_gen::ir::AffineExpr` (ir.rs:124), the
// real affine tree the .td model already carries.  FORCED by t8: `ir::AffineMap`
// (ir.rs:315) has `results: Vec<AffineExpr>`, so no other representation types.
// THE `init` IS A SENTINEL NO REAL VALUE CAN EQUAL, exactly as t4/t5/t6 are.
// C++ `AffineExpr()` leaves `expr == nullptr` and null-testing is a live idiom
// (`explicit operator bool()`, `operator!`, AffineExpr.h:81-83); the enum has no
// null variant and `Default` is deliberately NOT derived on it.  `Symbol(u32::MAX)`
// is the null: a symbol POSITION indexes an AffineMap's symbol list, so u32::MAX is
// unreachable for any expression a real map can hold.  NOT `Constant(..)` -- every
// i64 is a legitimate affine constant, so a constant sentinel would COLLIDE with
// data, which is the mistake the `Attr::Unit` note at the top of this file refuses.
unsafe fn t83() -> dataflowir_gen::ir::AffineExpr {
    dataflowir_gen::ir::AffineExpr::Symbol(u32::MAX)
}

// f61 -- the default constructor for t83.  Byte-identical to t83's `init` on
// purpose: they describe the same value by two routes (the type's `init` and the
// expression a written `AffineExpr e;` becomes), the same way f12 mirrors t8.
unsafe fn f61() -> dataflowir_gen::ir::AffineExpr {
    dataflowir_gen::ir::AffineExpr::Symbol(u32::MAX)
}

// t84 -- `mlir::OpAsmParser::UnresolvedOperand` -> `String`.  ⭐⭐ REMODELLED from `()`
// by slot `t84`, which is the change the old body's own comment committed to ("the
// moment a TU reads one of them, this key must be REPLACED ... rather than extended").
// THE MODEL IS THE PRINTED SPELLING -- `%arg0`, `%3#2`, sigil included -- because that
// is the ONE field of MLIR's `{ SMLoc location; StringRef name; unsigned number; }`
// that the reader half actually consumes: `dataflowir_gen::AsmParser::parse_operand`
// writes it (and `resolve_operands` would look it up in the parser's SSA environment,
// keyed by exactly that string, asm.rs:613).  Against the old `()` neither was writable
// -- `&mut ()` has nowhere to put a name and `Vec<()>` has no name to look up -- so the
// whole 122-site operand family was unkeyable, and THAT, not a field read, is what
// forced the remodel.
// ⛔ WHAT IS STILL LOST: `location` and the `number`/`name` SPLIT.  `number` is folded
// into the string (`%3#2` carries its own result number), and `location` has no model
// because `llvm::SMLoc` has no type key -- a site reading either is `E0609` at rustc,
// which is the same loud answer the `()` model gave, not a new silence.
// ⭐ ON `==`: still NOT mapped, and the reasoning INVERTS rather than survives.  Under
// `()` an `==` would have made every token compare equal (the t72 TypeID error); under
// `String` an `==` would be *correct* (two uses are the same operand iff they spell the
// same name).  It stays out only because C++ declares none on `UnresolvedOperand`, so a
// rule would resolve to nothing -- there is no longer a semantic objection to it.
// ⛔ THE VALUE IS `String::new()`, NOT A PLACEHOLDER NAME.  An empty spelling is the
// honest zero: no SSA environment can contain `""`, so a default-constructed operand
// that is never parsed into resolves to nothing rather than to some other op's value.
// A sentinel like `"%?"` would be a name that could collide.
unsafe fn t84() -> ::std::string::String {
    ::std::string::String::new()
}

// f62 -- the default constructor for t84; the empty spelling, per t84.  0-ary, so there
// is no argument whose evaluation could be dropped (the f52 DominanceInfo hazard).
// ⭐ This is the constructor `SmallVector::resize`/value-init reaches, and it is why
// `SmallVector<UnresolvedOperand, N>` is now a `Vec<String>` whose elements f880 can
// fill, and `ArrayRef<UnresolvedOperand>` exactly the `&[String]` the crate asks for.
unsafe fn f62() -> ::std::string::String {
    ::std::string::String::new()
}

// t85 -- `mlir::IntegerSet` -> `dataflowir_gen::ir::IntegerSet` (ir.rs:425), the
// model the .td parser ALREADY generates: `{ n_dims, n_symbols, constraints:
// Vec<Constraint> }` over t83's `ir::AffineExpr`.  Forced by t83 the way t83 was
// forced by t8.  No member is mapped, `==`/`!=` are LEFT OUT, and the type has no
// destructor anywhere in mlir/include -- see src.cpp for all of it.
// The `u32::MAX` dim/symbol counts are the NULL-HANDLE sentinel: C++
// `IntegerSet()` leaves `set == nullptr`, which is NOT the empty constraint
// system, and `0/0/vec![]` would conflate the two.
unsafe fn t85() -> dataflowir_gen::ir::IntegerSet {
    dataflowir_gen::ir::IntegerSet {
        n_dims: u32::MAX,
        n_symbols: u32::MAX,
        constraints: Vec::new(),
    }
}

// f119 -- the default constructor for t85.  Byte-identical to t85's `init` on
// purpose (t85 supplies the type's zero value, f119 is the expression a written
// `IntegerSet s;` becomes), the same way f61 mirrors t83 and f12 mirrors t8.
unsafe fn f119() -> dataflowir_gen::ir::IntegerSet {
    dataflowir_gen::ir::IntegerSet {
        n_dims: u32::MAX,
        n_symbols: u32::MAX,
        constraints: Vec::new(),
    }
}

// t86 -- `llvm::SetVector<T1>` -> `Vec<T1>`.  SetVector is INSERTION-ORDERED and
// the corpus ITERATES them into emitted MLIR (six sites, see src.cpp), so a
// HashSet would silently permute the output and a Vec keeps the order exactly.
// NO member is mapped -- notably not `insert`, whose `bool` return the corpus
// relies on at DoubleBuffering.cpp:268 -- so every operation aborts loudly rather
// than dropping the dedup.  The t79 BitVector shape.
fn t86<T1>() -> Vec<T1> {
    Vec::new()
}

// f120 -- the default constructor for t86.  `SetVector() = default;` leaves both
// the vector and the membership set empty.
unsafe fn f120<T1>() -> Vec<T1> {
    Vec::new()
}

// t151 -- `mlir::RegisteredOperationName` -> the SAME model as t18
// (`mlir::OperationName`), mirrored VERBATIM.  A registered op name IS the
// .td-parsed op def, and the derived type carries no additional state: the base
// holds the single nullable `Impl *`.  Spelled as an OWNING copy of the registry
// row rather than `Option<&'static TdOpDef>` for exactly t18's reason -- the
// converter mangles a parameter's Rust type text into the overload-disambiguating
// function name, and a lifetime in that text is not an identifier.  `None` is the
// null handle.  NO member is mapped, so any read still aborts loudly.
fn t151() -> Option<dataflowir_gen::TdOpDef> {
    None
}

// t152 -- `mlir::func::CallOp` -> `fmt::OpInst`.  An ordinary ODS-generated op
// class (FuncOps.h.inc:574, `::mlir::Op<CallOp, ...>`), i.e. one `Operation *`
// through its OpState base, so this is t25's representation and t27's precedent.
//
// ⛔ t25's PROHIBITION APPLIES UNCHANGED: no `operator==`, no `operator!=`, no
// identity test is mapped, and none may be added without a handle-identity
// model.  A C++ op handle is equal iff the `Operation *` are the same object;
// `OpInst` is the op's printed CONTENT, so equality here would call two distinct
// calls to the same callee with the same operands the SAME operation.  No member
// is mapped either, so every read (`getCallee`, `getLoc`, `create`) still aborts
// loudly in the mapper rather than returning a plausible lie.
//
// The `init` is t25's, and for t25's reason: a default-constructed ODS op handle
// is the NULL handle, `fmt::OpInst` has no null, and this expression exists only
// to type-check the type key.
fn t152() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// f121 -- the default constructor for t152, required because the gating TU writes
// `func::CallOp found;` (dbo/src/InitBin.cpp:41).  Byte-identical to t152's init
// on purpose, and for the same reason t85/f119 are: t152 supplies the type's zero
// value while f121 is the expression a written `func::CallOp x;` lowers to.  It is
// the null handle in C++ and `fmt::OpInst` cannot represent null, so this is a
// PLACEHOLDER -- the value is never read, because no CallOp member is mapped.
fn f121() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// t153 -- `mlir::linalg::LinalgOp` -> `fmt::OpInst`.  IT IS AN OP INTERFACE, not
// an op class (LinalgInterfaces.h.inc:477, `::mlir::OpInterface<LinalgOp, ...>`),
// so its C++ state is the PAIR (`Operation *`, `const Concept *`).  The payload is
// an op and every corpus value comes from `dyn_cast<LinalgOp>(op)` or a `walk`
// lambda, so t27's widening is the faithful one.
//
// ⛔ WHAT IS LOST, STATED: the `conceptImpl` pointer -- i.e. INTERFACE DISPATCH,
// which is exactly what t58 (`mlir::detail::InterfaceMap`) already declines to
// model because `dataflowir_gen` has no notion of it.  That refusal does NOT
// forbid this key -- t58 is itself mapped -- it constrains it: NO MEMBER IS
// MAPPED, so `getNumDpsInits`/`getDpsInitOperand`/`getMatchingIndexingMap`/
// `emitError`/`getOperation` all abort loudly instead of dispatching to nothing.
// t25's equality prohibition applies here too.  No constructor key: nothing in
// the corpus default-constructs one.
fn t153() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// ---------------------------------------------------------------------------
// PASS 2026-09-28: the `mlir::Location` row.  Model: dt_src `1e03497`,
// `cpp2rust-port/dataflowir-gen/src/ir.rs:607-800`, re-exported at the crate root
// (`lib.rs:72-73`), so `dataflowir_gen::Location` resolves too; the fully-qualified
// `ir::` path is used here to match every other key in this file.
// ---------------------------------------------------------------------------

// t154 `mlir::Location` -> `ir::Location` (ir.rs:681).  The CLOSED hierarchy of
//   `Builtin_LocationAttr` defs as a tree, NOT an opaque unit and NOT a widening.
//   ⛔ THE `init` IS `Location::Unknown` AND IT IS NOT A NULL SENTINEL.  The model
//   has no null variant on purpose (`mlir::Location` is documented non-nullable,
//   Location.h:76) and `Unknown` is REAL REACHABLE DATA -- six corpus sites build
//   it through `builder.getUnknownLoc()` and f124 maps exactly that.  So this
//   expression is here ONLY to type-check the type key; it is not reachable as a
//   default construction, because NO corpus site default-constructs a Location
//   (measured: `grep -rn 'Location [a-zA-Z_]*;'` over dcc/dbo/dataflow-scheduler/
//   dxp is zero hits) and NO default-constructor key is provided, so a site that
//   wrote one would abort loudly rather than silently get `Unknown`.
fn t154() -> dataflowir_gen::ir::Location {
    dataflowir_gen::ir::Location::Unknown
}

// t155 `mlir::FileLineColLoc` -> `ir::FileLineColLoc` (ir.rs:621).  A SEPARATE type
//   from t154 because in C++ it is a VIEW over a degenerate `FileLineColRange`
//   (Location.h:174), so the cast that produces one is a RANGE-IS-ONE-LINE test,
//   not a kind test.
//   ⛔ THE `init` IS A TYPE-CHECK PLACEHOLDER, for the same reason as t154's and
//   with the same measurement behind it (`FileLineColLoc [a-zA-Z_]*;` is zero hits
//   in the corpus).  Every real value comes from `dcc::utils::getLocation(op)`,
//   which is the PROJECT's own function -- ported, not keyed.  The empty filename
//   is NOT claimed as a sentinel: nothing reads this value.
fn t155() -> dataflowir_gen::ir::FileLineColLoc {
    dataflowir_gen::ir::FileLineColLoc::get("", 0, 0)
}

// f122 `FileLineColLoc::getLine()` -> `a0.line()` (ir.rs:644).  ⭐ THE KEY THE ROW
//   EXISTS FOR -- the nine structure-reading TUs all reach it through
//   `dcc::utils::getLocation(op).getLine()`.  `unsigned` -> `u32`, exact, and the
//   model's own doc comment records that `-1` at Utils.cpp:46 converts to
//   `4294967295`, so a u32 is the faithful width and not a narrowing.
fn f122(a0: &dataflowir_gen::ir::FileLineColLoc) -> u32 {
    a0.line()
}

// f123 `FileLineColLoc::getColumn()` -> `a0.column()` (ir.rs:648).  Same shape.
fn f123(a0: &dataflowir_gen::ir::FileLineColLoc) -> u32 {
    a0.column()
}

// f124 `Builder::getUnknownLoc()` -> `Location::Unknown`.  ⭐ THE CONSTRUCTOR KEY
//   FOR t154, IN ITS ONLY AVAILABLE FORM: `mlir::Location` has no meaningful
//   default ctor, so the usual `void T::T()` key has nothing to bind to; the
//   expression the corpus actually writes is `builder.getUnknownLoc()`.  Keyed on
//   `Builder` (which DECLARES it) and not `OpBuilder` (which INHERITS it), because
//   a rule cannot relocate an inherited member's key.
//   `a0` is t59's OPAQUE UNIT `()`, and that is faithful for THIS member alone: an
//   unknown location carries nothing from the builder.  No other Builder member is
//   keyed, so `getIndexType()` and friends still abort loudly.
fn f124(a0: &()) -> dataflowir_gen::ir::Location {
    dataflowir_gen::ir::Location::Unknown
}

// t156 `mlir::SideEffects::EffectInstance<mlir::MemoryEffects::Effect>` ->
//   `dataflowir_gen::EffectInstance` (`ods_effects`, dt_src c7e551a).  ⭐ THE GATE
//   for `dialects/VarExpr/VarExprOps.cpp`, whose abort is the ELEMENT type of the
//   `SmallVectorImpl` every generated `getEffects` override takes.  The concrete
//   instantiation is keyed because the corpus has exactly one (98 definitions, all
//   `<mlir::MemoryEffects::Effect>`).  No constructor key: `EffectInstance` declares
//   twelve constructors and none of them is a default constructor, so no site can
//   ask for `mlir_SideEffects_EffectInstance::new()`.
//   ⛔ The producer and consumer keys are OUT BY DECISION -- the per-operand
//   `emplace_back` needs an `OpOperand *` a ported body cannot form, `hasEffect<T>()`
//   loses its effect to a non-recorded explicit template argument, and
//   `isMemoryEffectFree(Operation *)` asks a different question than
//   `is_memory_effect_free(&[EffectInstance])`.  See src.cpp at t156.
fn t156() -> dataflowir_gen::EffectInstance {
    dataflowir_gen::EffectInstance::new(
        dataflowir_gen::EffectKind::Read,
        0,
        false,
        dataflowir_gen::ods_effects::DefaultResource::get(),
    )
}

// t157 -- `mlir::func::FuncOp` -> `fmt::OpInst`.  An ordinary ODS-generated op class
// (FuncOps.h.inc, `::mlir::Op<FuncOp, ...>`), i.e. one `Operation *` through its
// OpState base, so this is t25's representation and t152's immediate precedent.
//
// ⛔ t25's PROHIBITION APPLIES UNCHANGED: no `operator==`, no `operator!=`, no
// identity test is mapped, and none may be added without a handle-identity model.
// No member is mapped either, so every read (`getName`, `getBody`,
// `getFunctionType`, `FuncOp::create`) still aborts loudly in the mapper rather
// than returning a plausible lie.
//
// The `init` is t25's/t152's, and for their reason: a default-constructed ODS op
// handle is the NULL handle, `fmt::OpInst` has no null, and this expression exists
// only to type-check the type key.
fn t157() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// f125 -- the default constructor for t157.  Byte-identical to t157's init on
// purpose, exactly as f121 is to t152's: t157 supplies the type's zero value while
// f125 is the expression a written `func::FuncOp x;` lowers to.  It is the null
// handle in C++ and `fmt::OpInst` cannot represent null, so this is a PLACEHOLDER --
// the value is never read, because no FuncOp member is mapped.  See src.cpp at f125
// for the six default-construction sites that license it.
fn f125() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// t158 -- `mlir::affine::AffineForOp` -> `fmt::OpInst`.  An ordinary ODS-generated op
// class (`::mlir::Op<AffineForOp, ...>`), i.e. one `Operation *` through its OpState
// base, so this is t25's representation and t152/t157's immediate precedent.  The model
// for this op is HAND-WRITTEN rather than `.td`-derived: `custom.rs:10,47,68` registers
// the printer `affine_for(op: &OpInst, ctx: &PrintCtx)` for `("affine","for")`, citing
// MLIR's `AffineForOp::print`.
//
// ⛔ t25's PROHIBITION APPLIES UNCHANGED: no `operator==`, no `operator!=`, no identity
// test is mapped, and none may be added without a handle-identity model.  NO MEMBER and
// NO CONSTRUCTOR is mapped either -- nothing in the four gating TUs default-constructs
// one, and every read (`hasConstantBounds`, `getConstantUpperBound`, `getInductionVar`,
// `getBody`, `getStepAsInt`, `AffineForOp::create`) still aborts loudly in the mapper
// rather than returning a plausible lie.  See src.cpp at t158 for both greps.
//
// The `init` is t25's/t152's/t157's, and for their reason: a default-constructed ODS op
// handle is the NULL handle, `fmt::OpInst` has no null, and this expression exists only
// to type-check the type key.
fn t158() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// ---------------------------------------------------------------------------
// f126-f128 -- the free `llvm::raw_ostream <<` family.  See src.cpp for the three
// searched spellings, the 20-site ceiling, and the FOUR refusals of this family
// that must stay refused (Value / Operation / OpState / OperationName).
//
// Shape follows rules/raw_ostream's own insertion bodies: `raw_ostream &` is
// `*mut std::fs::File`, the text goes out through the fully qualified
// `::std::io::Write::write_all` so the inlined body does not depend on `Write`
// being in scope in the translated crate, the write error is DROPPED (raw_ostream
// records it on the stream rather than reporting it at the call), and the receiver
// is RETURNED so `o << a << b` and `return o << x;` both have a value.
//
// `to_string` is spelled through `::std::string::ToString` for the same reason.
// ---------------------------------------------------------------------------

unsafe fn f126(a0: *mut std::fs::File, a1: dataflowir_gen::ir::Ty) -> *mut std::fs::File {
    let __o = a0;
    let __b = ::std::string::ToString::to_string(&a1).into_bytes();
    let _ = ::std::io::Write::write_all(&mut *__o, &__b);
    __o
}

unsafe fn f127(a0: *mut std::fs::File, a1: dataflowir_gen::ir::Attr) -> *mut std::fs::File {
    let __o = a0;
    let __b = ::std::string::ToString::to_string(&a1).into_bytes();
    let _ = ::std::io::Write::write_all(&mut *__o, &__b);
    __o
}

unsafe fn f128(
    a0: *mut std::fs::File,
    a1: &dataflowir_gen::ir::Location,
) -> *mut std::fs::File {
    let __o = a0;
    let __b = ::std::string::ToString::to_string(a1).into_bytes();
    let _ = ::std::io::Write::write_all(&mut *__o, &__b);
    __o
}

// ---------------------------------------------------------------------------
// t159 -- `mlir::scf::IfOp` -> `fmt::OpInst`.  An ODS-generated op class in the SAME
// header as `scf::ForOp` (t27) and of the SAME shape: one `Operation *` through its
// OpState base.  So this row adds NO new claim about the model -- t27 already maps
// that representation to `fmt::OpInst`, and this is the second key against it.
//
// ⛔ t25's PROHIBITION APPLIES UNCHANGED: no `operator==`, no `operator!=`, no
// identity test is mapped, and none may be added without a handle-identity model.  NO
// MEMBER is mapped either -- every read the gating TU performs (`getResults`,
// `getThenBodyBuilder`, `getElseBodyBuilder`, `getLoc`, `IfOp::create`) still aborts
// loudly in the mapper rather than returning a plausible lie.  See src.cpp at t159 for
// both greps, including the `OpState::` one.
//
// The `init` is t25's/t27's/t152's/t157's/t158's, and for their reason: a
// default-constructed ODS op handle is the NULL handle, `fmt::OpInst` has no null, and
// this expression exists only to type-check the type key.
fn t159() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// f129 -- the default constructor for t159, required because the gating TU writes
// `mlir::scf::IfOp scf_ifop;` at five sites (SNControlFlowLowering.cpp:86, :179, :204,
// :307, :1055) and PCFG2ToDataflowIR.cpp:1197 writes a sixth.  Byte-identical to
// t159's init on purpose, exactly as f121 is to t152's and f125 is to t157's: t159
// supplies the type's zero value while f129 is the expression a written
// `mlir::scf::IfOp x;` lowers to.  It is the null handle in C++ and `fmt::OpInst`
// cannot represent null, so this is a PLACEHOLDER -- the value is never read, because
// no IfOp member is mapped.
fn f129() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// t160 -- `mlir::UnrealizedConversionCastOp` -> `fmt::OpInst`.  An ODS-generated op in
// the Builtin dialect, of the SAME shape as t27's `scf::ForOp` and t158's
// `affine::AffineForOp`: one `Operation *` through its `OpState` base.  This row adds
// NO new claim about the model at all -- `ops::mlir_UnrealizedConversionCastOp` is the
// very `DEF` that t25, t27, t152, t157, t158 and t159 already name below, so the type
// being keyed here is the one whose model was already load-bearing.
//
// ⛔ t25's PROHIBITION APPLIES UNCHANGED: no `operator==`, no `operator!=`, no identity
// test is mapped, and none may be added without a handle-identity model.  NO MEMBER is
// mapped either -- the corpus's `UnrealizedConversionCastOp::create` (7 sites),
// `addLegalOp<...>`, `getDefiningOp<...>` and `::Adaptor` all still abort loudly in the
// mapper rather than returning a plausible lie.  The middle two are the
// collapsed-template-argument trap; see src.cpp at t160 for the enumerated sites and
// for the zero-hit declaration grep that is why there is NO `f` key.
//
// The `init` is t25's/t27's/t152's/t157's/t158's/t159's, and for their reason: a
// default-constructed ODS op handle is the NULL handle, `fmt::OpInst` has no null, and
// this expression exists only to type-check the type key.
fn t160() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// t161 -- `mlir::arith::ConstantOp` -> `fmt::OpInst`.  An ODS-generated op in the Arith
// dialect, of the SAME shape as t27's `scf::ForOp`, t157's `func::FuncOp`, t158's
// `affine::AffineForOp` and t160's `UnrealizedConversionCastOp`: one `Operation *`
// through its `OpState` base.  Unlike those rows this one names a NEW `DEF`, so it was
// verified to exist rather than assumed -- `mlir_arith_ConstantOp` is present in the
// pinned `libdataflowir_gen` rmeta (alongside `mlir_arith_AddIOp`, `mlir_arith_CmpIOp`,
// ... 20+ Arith ops), and `lib.rs:103` lists `arith` among the dialects whose op rows
// this crate holds.  So no new claim about the model is made; the Arith `.td` front end
// already parses it.
//
// ⛔ t25's PROHIBITION APPLIES UNCHANGED: no `operator==`, no `operator!=`, no identity
// test is mapped, and NO MEMBER is mapped -- the corpus's `arith::ConstantOp::create`
// (an op-creating SINK) and `dyn_cast<arith::ConstantOp>` (the
// collapsed-template-argument trap) both still abort loudly in the mapper rather than
// returning a plausible lie.  See src.cpp at t161 for the enumerated sites and for the
// declaration grep that is why there is NO `f` key.
//
// The `init` is t25's/t27's/t152's/t157's/t158's/t159's/t160's, and for their reason: a
// default-constructed ODS op handle is the NULL handle, `fmt::OpInst` has no null, and
// this expression exists only to type-check the type key.
fn t161() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_ConstantOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// t162 -- `mlir::arith::ConstantIndexOp` -> `fmt::OpInst`.  `Arith.h:113` declares it as
// `class ConstantIndexOp : public arith::ConstantOp` -- a hand-written convenience
// subclass adding no state, so it is the same `Operation *` handle and, at runtime, the
// same `arith.constant` op with an `index` result type.
//
// ⚠️ IT SHARES t161's `DEF` DELIBERATELY, and the model agrees with the header: the
// pinned `libdataflowir_gen` rmeta carries `mlir_arith_ConstantOp` and NO
// `mlir_arith_ConstantIndexOp`, because there is no separate ODS `def` for it.  Using
// t161's DEF is therefore the faithful mapping, not an approximation.
//
// Same prohibitions as t161, and NO `f` key: the declaration grep for
// `arith::ConstantIndexOp x;` is ZERO hits and all 344 corpus mentions are
// `ConstantIndexOp::create(...)`, the op-creating SINK refusal.
fn t162() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_ConstantOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// ============================================================================================
// t163 = mlir::OptionalParseResult -> Option<bool>.  See src.cpp for the full model note.
// It is NOT rules/support t4 (`bool`): OpDefinition.h:56 gives it its own
// `std::optional<ParseResult> impl`, and its own comment says the point is a TRI-STATE
// absent / present-success / present-failure that a bool would collapse.
// Default-constructed (`OptionalParseResult() = default`) leaves `impl` empty -> None.
// ⚠️ THIS BLOCK IS BYTE-IDENTICAL IN tgt_refcount.rs AND tgt_unsafe.rs ON PURPOSE: the model is
// `Option<bool>`, a Copy scalar, and no member returns a reference into the receiver, so there
// is nothing for the two models to disagree about.
fn t163() -> Option<bool> {
    None
}

// f130 -- has_value().  `impl.has_value()`; no polarity involved.
unsafe fn f130(a0: &Option<bool>) -> bool {
    a0.is_some()
}

// f131 -- value().  Returns the contained ParseResult BY VALUE, so this is an unwrap of a Copy
// scalar and borrows nothing from the receiver.
// ⭐⭐ NO `!` HERE, AND THAT IS THE LOAD-BEARING DECISION.  `ParseResult::operator bool()`
// returns `failed()`, not `succeeded()` -- but that inversion is rules/support f24`s job
// (`unsafe fn f24(a0: bool) -> bool { !a0 }`).  `if (*optRes)` in C++ is TWO calls, `operator*`
// then `operator bool`, so the emitted Rust is f24(f132(x)) and negating here as well would
// DOUBLE-INVERT every parse branch -- which still compiles and still type-checks.  The inner
// bool is in the SUCCESS polarity, matching t1/t4.
unsafe fn f131(a0: &Option<bool>) -> bool {
    a0.expect("OptionalParseResult::value() on an absent result")
}

// f132 -- operator*(), which C++ defines as `return value();`.  Same text as f131 by
// construction, not by coincidence.
unsafe fn f132(a0: &Option<bool>) -> bool {
    a0.expect("OptionalParseResult::operator*() on an absent result")
}

// f133 -- OptionalParseResult(std::nullopt_t): THE ABSENT STATE.  The parameter is rules/optional
// t4, which models std::nullopt_t as `()`; the unit type has exactly one value so it is exact.
unsafe fn f133(a0: ()) -> Option<bool> {
    let _ = a0;
    None
}

// f134 -- OptionalParseResult(LogicalResult): PRESENT, carrying the LogicalResult unchanged.
// rules/support t1 models LogicalResult as `bool` with true == success, and the C++ ctor is
// `impl(result)`, an identity on that scalar (rules/support f23 is likewise identity).
unsafe fn f134(a0: bool) -> Option<bool> {
    Some(a0)
}

// f135 -- OptionalParseResult(ParseResult): identical for the identical reason; rules/support t4
// is the same `bool` in the same polarity as t1.
unsafe fn f135(a0: bool) -> Option<bool> {
    Some(a0)
}

// f136 -- OptionalParseResult(const InFlightDiagnostic &) : OptionalParseResult(failure()).
// UNCONDITIONALLY PRESENT-FAILURE, and the diagnostic is discarded by C++ itself (the parameter
// is unnamed at OpDefinition.h:45).  `false` == failure in the t1/t4 polarity.  Taking the
// diagnostic by value here would drop it, and `libcc2rs::InFlightDiagnostic` PRINTS ON `Drop`
// (t70`s note, src.cpp:1680-), so it is taken BY REFERENCE and left alive for its real owner.
unsafe fn f136(a0: &libcc2rs::InFlightDiagnostic) -> Option<bool> {
    let _ = a0;
    Some(false)
}

// t164 `mlir::RankedTensorType` -> `ir::Ty` (ir.rs:37).  A WIDENING, the same one t41,
//   t42, t60 and t73 make; see src.cpp for why t42 (`TensorType`, the interface OVER this
//   type) already committed to this model.  ⛔ `Ty` has NO TENSOR VARIANT, so this lands in
//   `Ty::Opaque(spelling)` and the shape is recoverable only by reparsing.  The init is the
//   empty-spelling null sentinel, NOT a real type.  NO accessor is mapped.
fn t164() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(String::new())
}

// t165 `mlir::FunctionType` -> `ir::Ty` (ir.rs:37).  A WIDENING, the same one t164 makes.
//   ⛔ `Ty` has NO FUNCTION VARIANT, so the input/result type lists are NOT carried; they
//   land in `Ty::Opaque(spelling)`.  NO accessor and NOT `FunctionType::get` are mapped, so
//   every structure query and every construction still aborts LOUDLY.
fn t165() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(String::new())
}

// f137 / f138 -- the default constructors for t164 / t165.  Same null-handle sentinel as
// each type's `init` above, for the f40-f45 / f42 reason (type key alone -> E0433).
unsafe fn f137() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(String::new())
}
unsafe fn f138() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(String::new())
}

// f139/f140/f141 -- three more `llvm::ArrayRef<T1>` constructor overloads, each one
// PROVEN ABSENT by the `-verbose` readback in /home/agent/work/mlirslot/aref.vlog
// (`result: None`), against f55's init-list key which that same readback shows
// `Matching:`.  t19 is `Vec<T1>`, an OWNING model of a borrowed view -- this
// module's settled position (src.cpp:245-249) -- so each body is a COPY, and no
// caller can observe the lost aliasing because `llvm::ArrayRef` declares no
// mutating member.  See src.cpp at f139 for why the other two `None` spellings
// (`(const T *, size_t)` and `(const SmallVectorImpl<T> &)`) are deliberately
// left fabricating.
unsafe fn f139<T1>() -> Vec<T1> {
    Vec::new()
}

// f140 -- the one-element constructor.  `T1: Clone` and `a0.clone()` follow
// rules/support f19 exactly: at `a0: &T1` the by-value probe step picks
// `<T1 as Clone>::clone`, so this yields `T1`, not `&T1`.
unsafe fn f140<T1: Clone>(a0: &T1) -> Vec<T1> {
    // PATH FORM, not `a0.clone()`: the converter substitutes `&(*s)` for a `const &`
    // argument and appends the method textually, so `a0.clone()` emitted
    // `vec![&(*s).clone()]` -- `&((*s).clone())`, a reference to a temporary and the
    // WRONG element type.  MEASURED, then fixed.  Same lesson as AGENT-COMMON's
    // `Vec::len(&a0)` / `Ptr::decay(&a0)` note.
    vec![Clone::clone(a0)]
}

// f141 -- from a `std::vector<T1>`, which rules/vector also models as `Vec<T1>`,
// so the copy is `Vec::clone`.
unsafe fn f141<T1: Clone>(a0: &Vec<T1>) -> Vec<T1> {
    a0.clone()
}

// f142 -- `mlir::RegionRange`'s own constructor (Region.h:342-356), the largest
// fabricated-`::new_N` receiver in the corpus.  PROVEN ABSENT by the `-verbose`
// readback in /home/agent/work/mlirslot2/probe-regionrange.vlog (`result: None`).
// CONCRETE, not generic: `RegionRange` is not a template, so both sides are
// already-mapped concrete types -- the parameter `llvm::MutableArrayRef<mlir::Region>`
// is t46 at `mlir::Region` -> `Vec<fmt::Region>`, and the result t17 is the SAME
// Rust type.  The body is therefore the identity, which is the only thing it can
// be: an owning `Vec` model of a non-owning view means construction is a copy
// (src.cpp:245-249, this module's settled position for t14-t17), and no mapped
// operation can observe the lost aliasing because `RegionRange` declares no
// mutating member.  See src.cpp at f142 for the aliasing argument in full and for
// why the sibling `ArrayRef<mlir::Region *>` overload is deliberately left
// fabricating.
unsafe fn f142(a0: Vec<dataflowir_gen::fmt::Region>) -> Vec<dataflowir_gen::fmt::Region> {
    a0
}

// f143/f144/f145 -- three of the four MEASURED `mlir::ValueRange` constructor
// spellings (/home/agent/work/VALUERANGE-SPELLINGS.md; the `-verbose` leg exited on
// its own at 27,901 asks and its emission matches the plain run's 31,212 lines, so
// its `result: None` is admissible).  t14 (`OperandRange`), t16 (`ValueRange`),
// t19 at `mlir::Value` and rules/vector's `std::vector<mlir::Value>` are ALL the
// same Rust type `Vec<ir::Value>`, so f143/f144 are the IDENTITY and f145 is a
// clone.  The owning-`Vec` model of a borrowed view is this module's settled
// position (src.cpp:245-249) and is admissible here for f142's reason: ValueRange's
// whole public surface past the constructors is `getTypes()`/`getType()`, both
// `const`, so it declares NO mutating member and the lost aliasing is
// unobservable.  See src.cpp at f143 for the four asks, for why the COPY
// CONSTRUCTOR is deliberately absent (written, measured 7 -> 7 sites, deleted), and
// for why f145's non-const reference is NOT write-through.
unsafe fn f143(a0: Vec<dataflowir_gen::ir::Value>) -> Vec<dataflowir_gen::ir::Value> {
    a0
}

unsafe fn f144(a0: Vec<dataflowir_gen::ir::Value>) -> Vec<dataflowir_gen::ir::Value> {
    a0
}

// f145 -- `std::vector<mlir::Value> &` is the forwarding template's DEDUCED
// parameter (ValueRange.h:397-401), read-only in fact, so `&mut Vec` here (the
// rules/array f3-f5 convention for a non-const reference) is only borrowed, never
// written.  PATH FORM with an explicit reborrow, NOT `a0.clone()`: the converter
// substitutes the argument textually and appends methods textually, so a bare
// `a0.clone()` risks emitting `&mut (*s).clone()` = `&mut ((*s).clone())`, a
// reference to a temporary -- the bug measured and fixed in f140.
unsafe fn f145(a0: &mut Vec<dataflowir_gen::ir::Value>) -> Vec<dataflowir_gen::ir::Value> {
    Clone::clone(&*a0)
}

// t166 -- the range CRTP base at its FOURTH concrete instantiation,
// `DerivedT = mlir::ValueRange` (queue row g090).  Body IDENTICAL to t16
// (`mlir::ValueRange`) and to t37, because the base IS the range: no new
// representation is introduced.  See src.cpp for the full spelling read off the
// abort and the swallow-safety argument.
fn t166() -> Vec<dataflowir_gen::ir::Value> {
    Default::default()
}

// f146 -- `llvm::APInt mlir::IntegerAttr::getValue() const`, 16 asks on
// KtdpDialect.cpp and the largest single count in that TU's diagnosis.  The full
// argument is in src.cpp; in one paragraph: t10 widens `mlir::IntegerAttr` to the
// whole `ir::Attr` enum, so this body RECOVERS the integral case, and it can
// recover it EXACTLY because `Attr::Int(i64, Ty)` carries the type suffix as data
// and `Ty::Int(u32)` is the width.  The width is the whole point of the key --
// `rules/apint` f8 (`getSExtValue`) sign-extends FROM BitWidth, so a width-less
// model of this accessor turns a 32-bit -1 into +4294967295 in the loop bound at
// `dcc/src/Dialect/Sentient/SentientOps.cpp:1074`.
//
// `v as u64` is a two's-complement reinterpretation, and `APInt::new` then masks
// to the width -- which is what MLIR's IntegerAttr storage holds.  `implicit_trunc
// = true` says exactly that: the high bits of a negative `i64` are DELIBERATELY
// dropped, not accidentally.
//
// ⛔ THE PANIC ARM IS THE LOUD FAILURE, NOT A `todo!()`.  In C++ the static type
// `IntegerAttr` already guaranteed integrality; these arms are reachable only
// because t10 widened, and the message names the arm so a real occurrence is
// diagnosable instead of silently answering 0.
unsafe fn f146(a0: dataflowir_gen::ir::Attr) -> libcc2rs::APInt {
    match a0 {
        // `w`-bit integer attribute -- the ordinary case.
        dataflowir_gen::ir::Attr::Int(v, dataflowir_gen::ir::Ty::Int(w)) => {
            libcc2rs::APInt::new(w, v as u64, true, true)
        }
        // `index` -- MLIR's IndexType stores IntegerAttrs at
        // kInternalStorageBitWidth = 64.
        dataflowir_gen::ir::Attr::Int(v, dataflowir_gen::ir::Ty::Index) => {
            libcc2rs::APInt::new(64, v as u64, true, true)
        }
        // `mlir::BoolAttr` IS an IntegerAttr of i1; its `getValue()` is the 1-bit
        // arm.  Omitting this would have sent every boolean attribute to the panic.
        dataflowir_gen::ir::Attr::Bool(b) => {
            libcc2rs::APInt::new(1, b as u64, false, true)
        }
        other => panic!(
            "mlir::IntegerAttr::getValue() reached a non-integral ir::Attr arm: \
             {other:?}.  The C++ static type IntegerAttr guaranteed integrality; \
             this state exists only because rules/mlir t10 widens IntegerAttr to \
             the whole ir::Attr enum, so reaching it means the attribute was built \
             through a path that lost its integral type"
        ),
    }
}

// ============================================================================================
// t167-t216 -- the fifty `mlir::OpTrait::OneTypedResult<RT>::Impl<ConcreteOp>` trait bases ->
// `fmt::OpInst`.  Each body is the t161/t162 body with the CONCRETE OP'S OWN `DEF`, because the
// trait base of an op IS that op -- no new representation is introduced and nothing is claimed
// beyond what t161 already claims.  Every corpus occurrence is a CAST TARGET and no member is
// ever read, so these expressions exist only to type-check their type keys; t25's prohibition
// (no equality, no identity, no member) carries to all fifty.
// ⛔ Each op's `DEF` was verified to EXIST in dataflow_ods.rs before its key was written -- a
// key naming an absent DEF records fine, passes the load smoke test, and fails at rustc.  The
// 25 spellings whose dialects (`mlir::LLVM`, `mlir::math`, `mlir::memref`) have NO generated DEF
// are deliberately unkeyed and stay loud; see src.cpp at t167 for the per-dialect measurement.
// ⚠️ Plain `fn`, not `unsafe fn`: the t37-t39 / t166 convention for a TYPE rule.
// ============================================================================================
fn t167() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_ConstantOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t168() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_SelectOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t169() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vector_ExtractOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t170() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_sentient_ConstantOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t171() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_CmpIOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t172() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_MulIOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t173() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vector_InsertOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t174() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_AddIOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t175() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_dataflow_GetLogicalMemoryViewOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t176() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vectorchain_ShuffleOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t177() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vector_ShapeCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t178() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_AddFOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t179() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_MulFOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t180() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_SubIOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t181() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vectorchain_RotateOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t182() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_TruncIOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t183() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vector_BitCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t184() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_XOrIOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t185() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_IndexCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t186() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_ExtSIOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t187() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_BitcastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t188() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_DivFOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t189() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_dataflow_ReceiveOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t190() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_CmpFOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t191() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_AndIOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t192() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_DivSIOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t193() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_NegFOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t194() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_MinimumFOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t195() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_MinSIOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t196() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_MaximumFOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t197() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_MaxSIOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t198() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_OrIOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t199() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_RemUIOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t200() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vector_ExtractStridedSliceOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t201() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vector_InsertStridedSliceOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t202() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_ExtFOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t203() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_TruncFOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t204() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_SubFOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t205() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_ShRSIOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t206() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vectorchain_MultiplyOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t207() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vectorchain_MultiplyAndAccumulateOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t208() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vectorchain_CreateAffineMaskOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t209() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vectorchain_SelectOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t210() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vectorchain_PackOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t211() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_uniform_DefImmutableMappingOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t212() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_dataflow_GetLocalUnitOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t213() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_RemSIOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t214() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vector_LoadOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t215() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vector_FromElementsOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t216() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vector_ShuffleOp as dataflowir_gen::MlirOp>::DEF,
    )
}


// ============================================================================================
// t217-t235 -- the nineteen `mlir::OpTrait` trait bases of the three families
// (`detail::MultiResultTraitBase`, `SingleBlock`, `detail::MultiOperandTraitBase`) ->
// `fmt::OpInst`.  Each body is the t161/t162 body with the CONCRETE OP'S OWN `DEF`, because the
// trait base of an op IS that op -- no new representation is introduced and nothing is claimed
// beyond what t161 already claims.  Every corpus occurrence is a CAST TARGET and no member is
// ever read, so these expressions exist only to type-check their type keys; t25's prohibition
// (no equality, no identity, no member) carries to all nineteen.
// ⛔ Each op's `DEF` was verified to EXIST in dataflow_ods.rs before its key was written -- a key
// naming an absent DEF records fine, passes the load smoke test, and fails at rustc.  The three
// `mlir::ktdf` spellings have NO generated DEF (whole-dialect gap) and are deliberately unkeyed
// and stay loud; see src.cpp at t217 for the measurement and for the four `NOperands`-arity
// spellings left out for a separate reason.
// ⚠️ Plain `fn`, not `unsafe fn`: the t37-t39 / t166 / t167-t216 convention for a TYPE rule.
// ============================================================================================
fn t217() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_dataflow_GetUnitOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t218() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_scf_ForOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t219() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t220() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_scf_IfOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t221() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_sentient_IfOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t222() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_sentient_ForOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t223() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_uniform_UniformizeRegionsOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t224() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_sentient_MacOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t225() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_func_CallOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t226() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_affine_AffineForOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t227() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_sentient_IfOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t228() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_scf_ForOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t229() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_sentient_ForOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t230() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_dataflow_ProgramUnitOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t231() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_agen_CompositeStoreOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t232() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_ModuleOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t233() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_agen_CompositeLoadOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t234() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_sentient_YieldOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t235() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}


// t236-t242 -- `llvm::SmallSet` / `llvm::detail::DenseSetImpl` -> HashSet.
// See src.cpp for the seven keys, the monomorphic-not-generic prohibition, the
// `_`-erasure warning, and the nine spellings deliberately left unkeyed.
// IDENTICAL in both models on purpose: the three element types landed here are
// PRIMITIVES, so neither model's pointer representation appears.  Plain `fn`,
// not `unsafe fn`, per the t37-t39 / t166 convention for a TYPE rule.  The
// `init` is the empty set, which is what a default-constructed `SmallSet` /
// `DenseSet` IS -- not a sentinel, an exact match.
fn t236() -> std::collections::HashSet<i64> {
    std::collections::HashSet::new()
}

fn t237() -> std::collections::HashSet<u64> {
    std::collections::HashSet::new()
}

fn t238() -> std::collections::HashSet<i64> {
    std::collections::HashSet::new()
}

fn t239() -> std::collections::HashSet<i64> {
    std::collections::HashSet::new()
}

fn t240() -> std::collections::HashSet<u32> {
    std::collections::HashSet::new()
}

fn t241() -> std::collections::HashSet<u32> {
    std::collections::HashSet::new()
}

fn t242() -> std::collections::HashSet<u64> {
    std::collections::HashSet::new()
}

// t243-t246: llvm::ilist_iterator over mlir::Operation / mlir::Block.  See
// src.cpp for the four spellings, why the IsReverse flag is two distinct keys,
// and why the erase-during-walk question is settled in favour of this model.
// The representation is a raw element pointer INTO the Vec the container
// already is -- identical to t36 (`mlir::Operation *`) and to
// rules/list_iterator's unsafe iterator against the same Vec representation.
// The init is the NULL POINTER, the same faithful default t36 gives.
// IsReverse does not change the REPRESENTATION, only the direction a
// (deliberately unkeyed) operator++ would step.
fn t243() -> *mut dataflowir_gen::fmt::OpInst {
    ::std::ptr::null_mut()
}

fn t244() -> *mut dataflowir_gen::fmt::OpInst {
    ::std::ptr::null_mut()
}

fn t245() -> *mut dataflowir_gen::fmt::Block {
    ::std::ptr::null_mut()
}

fn t246() -> *mut dataflowir_gen::fmt::Block {
    ::std::ptr::null_mut()
}

// t250 / t251 -- `mlir::TypeRange` and its CRTP base at
// `DerivedT = mlir::TypeRange` (queue row g111; 12,045 + 2,098 ground-truth
// emitted sites).  SAME body for both, because the base IS the range -- the
// t37-t39 / t166 discipline.  Element model is `ir::Ty` (t5), NOT `ir::Value`:
// TypeRange's element type is `mlir::Type`, which is the whole reason t166 could
// not be shared.  Returned BY VALUE, so nothing can dangle under refcount.
// ⚠️ Plain `fn`, not `unsafe fn`: the t37-t39 / t166 convention for a TYPE rule.
// See src.cpp for the spelling read off TypeRange.h:33-37, the arity-0
// swallow-safety argument, and the aliasing licence RE-DERIVED for TypeRange
// (no member function of any kind on its public surface, so unlike
// mlir::MutableOperandRange there is nothing to write through).
fn t250() -> Vec<dataflowir_gen::ir::Ty> {
    Default::default()
}

fn t251() -> Vec<dataflowir_gen::ir::Ty> {
    Default::default()
}


// ---------------------------------------------------------------------------
// t260 `mlir::SelfOwningTypeID` -> AN OPAQUE UNIT, the SAME model as t72
// (`mlir::TypeID`) and for the same reason: the crate has no RTTI concept, and
// every emitted site is a static declaration plus its zero-initialiser
// (`LazyCell<T>` / `std::mem::zeroed::<T>()`), never a comparison and never a
// member read.  `==` and `getTypeID()` stay UNDECLARED in src.cpp, so identity is
// not silently faked -- it aborts.  See src.cpp at `using t260 =`.
// ⚠️ Plain `fn`, not `unsafe fn`: the t37-t39 / t166 / t167-t216 convention for a
// TYPE rule.
fn t260() -> () {
    ()
}

// t261 `mlir::NamedAttrList` -> `dataflowir_gen::ir::AttrDict` (ir.rs:559), the
// ENTRY-TYPE completion of t21: t21 is one `(String, Attr)` entry, t261 is the
// dictionary of them.  ORDER IS CORRECT BY THE CRATE'S OWN DOCUMENTED FACT --
// ir.rs:552-558 says MLIR sorts a dictionary by key on construction and the
// printer walks THAT order, so a `Vec` (the `SetVector` model) would print in
// insertion order and differ from the reference.  The init is the EMPTY
// dictionary, which is what a default-constructed NamedAttrList is: MLIR's
// `NamedAttrList()` holds no attributes, and `getDictionary()` on it yields the
// empty DictionaryAttr, so `BTreeMap::new()` is faithful rather than a sentinel.
// ⚠️ Plain `fn`, not `unsafe fn`: the t37-t39 / t166 convention for a TYPE rule.
fn t261() -> dataflowir_gen::ir::AttrDict {
    dataflowir_gen::ir::AttrDict::new()
}

// ---------------------------------------------------------------------------
// f150-f157 -- eight more deductions of the InFlightDiagnostic `<<` member
// template.  src.cpp carries the census, the f22-f38 diff and the fidelity
// argument.  Same two invariants as the rest of the family: `a0` and `a1` each
// mentioned EXACTLY ONCE, and exactly-once reporting from `self` by value.  An
// lvalue-reference argument takes a shared reference, not a value.
// A Twine is `Vec<libc::c_char>` in THIS model (rules/twine's unsafe overlay),
// `Vec<u8>` in the refcount one, so the byte sink here is `shl_c_chars` and there
// `shl_bytes` -- exactly the f22/f23/f34 split.  Both stop at the NUL.
// ---------------------------------------------------------------------------

// f150 -- `llvm::Twine &&`
unsafe fn f150(a0: libcc2rs::InFlightDiagnostic, a1: Vec<libc::c_char>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_c_chars(a0, &a1)
}

// f151 -- `const llvm::Twine &`
unsafe fn f151(a0: libcc2rs::InFlightDiagnostic, a1: &Vec<libc::c_char>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_c_chars(a0, a1)
}

// f152 -- `unsigned long &`
unsafe fn f152(a0: libcc2rs::InFlightDiagnostic, a1: &u64) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f153 -- `mlir::Type &&` -- Display is MLIR's type syntax (ir.rs:50), as f25
unsafe fn f153(a0: libcc2rs::InFlightDiagnostic, a1: dataflowir_gen::ir::Ty) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f154 -- `long &&`
unsafe fn f154(a0: libcc2rs::InFlightDiagnostic, a1: i64) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f155 -- `int &&`
unsafe fn f155(a0: libcc2rs::InFlightDiagnostic, a1: i32) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f156 -- `const unsigned long &`
unsafe fn f156(a0: libcc2rs::InFlightDiagnostic, a1: &u64) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f157 -- `const llvm::StringRef &`
unsafe fn f157(a0: libcc2rs::InFlightDiagnostic, a1: &Vec<libc::c_char>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_c_chars(a0, a1)
}

fn t300() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_ktdf_PipelineOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t301() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_ktdf_StageOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t302() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_ktdf_PrivateOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t303() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_ktdf_PrivateOp as dataflowir_gen::MlirOp>::DEF,
    )
}

fn t304() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_ktdf_PrivateYieldOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// t320 -- `mlir::OpTrait::OneRegion<mlir::ModuleOp>`.  DEF verified present at
// dataflow_ods.rs:4800 (`pub struct mlir_ModuleOp;`) BEFORE this key was written.
fn t320() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_ModuleOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// t400 -- `mlir::detail::SymbolOpInterfaceTrait<mlir::ktdf_arch::DeviceOp>`, 18 occurrences in 9
// files, the biggest remaining `mlir_` survivor.  The trait base of an op IS that op (t161/t162,
// as t300-t304 and t320 already apply it), so the model is the concrete op's own DEF.
// ⭐ DEF VERIFIED 0 -> 1 BEFORE THIS KEY WAS WRITTEN, which is the whole point of the row: the
// refusal this replaces measured `pub struct mlir_ktdf_arch_DeviceOp;` at 0 hits and was right to
// refuse; `dataflowir-gen` ee457cc added the `ktdf_arch` dialect and the identical grep against
// dataflowir-gen-654e676bccb4a05f/out/dataflow_ods.rs (mtime 2026-09-28 16:30:18, newer than
// ee457cc) now returns 1.
fn t400() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_ktdf_arch_DeviceOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// t340 -- `mlir::OpaqueProperties` -> `*mut ::libc::c_void`, THE TYPE'S OWN LAYOUT.
// src.cpp carries the six-line declaration and the complete member census.  The
// `init` is `null_mut()`, and it is UNREACHABLE from any well-formed C++ for the
// t1 reason: OpaqueProperties has NO default constructor, only `OpaqueProperties(
// void *)`, so no translated program can default-construct one.  It is still the
// faithful zero value -- a null `void *` is precisely the state for which the
// class's own `operator bool()` returns false.
// IDENTICAL IN BOTH MODELS on purpose: `void *` cannot be refcounted, so the
// refcount model spells it as a raw pointer too (the brotli/f6 precedent).
fn t340() -> *mut ::libc::c_void {
    ::std::ptr::null_mut()
}

// f240 -- `OpaqueProperties(void *prop)`, the class's only constructor.  The
// representation IS the argument, so this is the identity; the 30 corpus sites
// already emit `(... as *mut ::libc::c_void)` for the argument.
fn f240(a0: *mut ::libc::c_void) -> *mut ::libc::c_void {
    a0
}

// t420-t422: a NULL-HANDLE model for the three `mlir::memref` ODS op wrappers.  See src.cpp for
// what the 8 sites do after the default-construct, why `Option<fmt::OpInst>` is rejected (the two
// `mc` sites MUTATE THROUGH THE HANDLE: `mc.erase()`, `sel.getResult().setType(...)`), and why
// this needs NO `fN` -- `*mut T` implements `Default` as `null_mut()` in std, VERIFIED by
// compiling `let p: *mut Foo = <*mut Foo>::default();` (rc=0, `is_null()` true).  That is the whole
// difference from f42/f137/f138, whose `ir::Ty`/`ir::Attr` enum targets have no `Default`.
// The representation is BIT-FOR-BIT t36's (`mlir::Operation *`, :503), because an ODS op wrapper IS
// an `Operation *` plus a static type assertion -- `OpState` holds exactly one `Operation *`.  The
// init is the NULL POINTER: a default-constructed MLIR op wrapper is `state == nullptr`, so this is
// the faithful default and a `dyn_cast` against it is the one that must FAIL.
// ⚠️ NO MEMBER IS KEYED, the t166 / t243-t246 discipline.
fn t420() -> *mut dataflowir_gen::fmt::OpInst {
    ::std::ptr::null_mut()
}

fn t421() -> *mut dataflowir_gen::fmt::OpInst {
    ::std::ptr::null_mut()
}

fn t422() -> *mut dataflowir_gen::fmt::OpInst {
    ::std::ptr::null_mut()
}

// t440 -- `mlir::OpBuilder` -> `dataflowir_gen::OpBuilder` (build.rs:461, re-exported
// at the crate root, lib.rs:74-77).  THE REAL MUTABLE IR BUILDER, not an opaque
// stand-in: `create_op`, `insert_op`, `create_block`, the six `set_insertion_point*`
// forms and a `region()`.  2,241 corpus sites.
// ⛔ THE INIT VALUE IS NOT A SENTINEL AND NOT A NULL -- `OpBuilder::new` takes the
// `BlockList` it will insert into, so the only honest init is a builder over a FRESH
// single-entry-block list (`new_block_list_with_entry`, build.rs:94).  That is a real,
// usable builder rather than a poisoned one; a `Default`-style null builder has no
// representation in this model at all.
fn t440() -> dataflowir_gen::OpBuilder {
    dataflowir_gen::OpBuilder::new(dataflowir_gen::new_block_list_with_entry())
}

// t441 -- `mlir::ImplicitLocOpBuilder` -> `dataflowir_gen::ImplicitLocOpBuilder`
// (build.rs:630).  474 corpus sites, the CHEAP TAIL of t440 rather than separate
// work: in C++ it is `class ImplicitLocOpBuilder : public mlir::OpBuilder`
// (Builders.h:630), OpBuilder plus one `Location`, and the Rust type is that same
// shape with `Deref/DerefMut` to `OpBuilder`.
// ⛔ `Location::Unknown` HERE IS THE SAME CHOICE t154 ALREADY MADE, not a new
// sentinel: `mlir::Location` is non-nullable, so an init must name a real location
// and `Unknown` is the only one that needs no file/line.
fn t441() -> dataflowir_gen::ImplicitLocOpBuilder {
    dataflowir_gen::ImplicitLocOpBuilder::new(
        dataflowir_gen::ir::Location::Unknown,
        dataflowir_gen::new_block_list_with_entry(),
    )
}

// t460-t463 / f360-f366 -- THE MLIR PRINTER SINK AND ITS `<<` FAMILY.  src.cpp
// carries the model, the overload-resolution reason the two receivers are declared
// unrelated, and the three spellings deliberately left out.
//
// `dataflowir_gen::AsmPrinter` is re-exported at the crate root (lib.rs:86), so the
// path below is the crate-root one and not `dataflowir_gen::asm::AsmPrinter`.
// t460/t462 are the sink BY VALUE and t461/t463 the reference to it, the
// rules/raw_ostream t1/t2 split; in this model a reference is a raw pointer.
//
// ⚠️ EVERY BODY RETURNS ITS OWN `a0`.  The C++ returns the printer BY REFERENCE so
// `p << a << b` chains, and rules/raw_ostream f5-f18 is the same invariant.
//
// ⚠️ WRITES GO THROUGH AN EXPLICIT `&mut *p` REBORROW, fully qualified as an
// inherent associated function, for the two reasons rules/raw_ostream spells
// `::std::io::Write::write_all(&mut *__o, ..)`: the body is inlined into the
// translated crate so nothing may depend on what is in scope there, and a bare
// `(*p).method()` autoref trips the deny-by-default `dangerous_implicit_autorefs`.
fn t460() -> dataflowir_gen::AsmPrinter {
    dataflowir_gen::AsmPrinter::new()
}

fn t461() -> *mut dataflowir_gen::AsmPrinter {
    ::std::ptr::null_mut()
}

fn t462() -> dataflowir_gen::AsmPrinter {
    dataflowir_gen::AsmPrinter::new()
}

fn t463() -> *mut dataflowir_gen::AsmPrinter {
    ::std::ptr::null_mut()
}

// f360 -- `const char (&)[_]`, 155 sites.  The converter materialises a string
// literal in this position as a `c"..."` CStr (the f21 note above), so the
// parameter is `&CStr` and `to_bytes()` already stops at the NUL.
unsafe fn f360(a0: *mut dataflowir_gen::AsmPrinter, a1: &std::ffi::CStr) -> *mut dataflowir_gen::AsmPrinter {
    let __p = a0;
    let __s = String::from_utf8_lossy(a1.to_bytes()).into_owned();
    dataflowir_gen::AsmPrinter::print_str(&mut *__p, &__s);
    __p
}

// f361 -- `const char &`, 34 sites.
unsafe fn f361(a0: *mut dataflowir_gen::AsmPrinter, a1: &::libc::c_char) -> *mut dataflowir_gen::AsmPrinter {
    let __p = a0;
    let __c = (*a1 as u8) as char;
    dataflowir_gen::AsmPrinter::print_char(&mut *__p, __c);
    __p
}

// f362 -- `const mlir::OperandRange &`, 26 sites.  t14 is `Vec<ir::Value>` and
// `print_operands` takes `IntoIterator<Item = &Value>`, so the slice iterator is
// the argument and nothing is cloned.
unsafe fn f362(a0: *mut dataflowir_gen::AsmPrinter, a1: &Vec<dataflowir_gen::ir::Value>) -> *mut dataflowir_gen::AsmPrinter {
    let __p = a0;
    dataflowir_gen::AsmPrinter::print_operands(&mut *__p, a1.iter());
    __p
}

// f363 -- `mlir::Value` by value, 25 sites.  Prints the SSA NAME; see src.cpp.
unsafe fn f363(a0: *mut dataflowir_gen::AsmPrinter, a1: dataflowir_gen::ir::Value) -> *mut dataflowir_gen::AsmPrinter {
    let __p = a0;
    dataflowir_gen::AsmPrinter::print_operand(&mut *__p, &a1);
    __p
}

// f364 -- `mlir::AsmPrinter &` + `const char &`, 1 site.  f361's body, distinct key.
unsafe fn f364(a0: *mut dataflowir_gen::AsmPrinter, a1: &::libc::c_char) -> *mut dataflowir_gen::AsmPrinter {
    let __p = a0;
    let __c = (*a1 as u8) as char;
    dataflowir_gen::AsmPrinter::print_char(&mut *__p, __c);
    __p
}

// f365 -- `const llvm::StringRef &`, 1 site.  The trailing NUL is NOT part of the
// string in this port (rules/stringref), so it is dropped exactly as
// rules/raw_ostream f7 does.
unsafe fn f365(a0: *mut dataflowir_gen::AsmPrinter, a1: &Vec<::libc::c_char>) -> *mut dataflowir_gen::AsmPrinter {
    let __p = a0;
    let __b: Vec<u8> = a1
        .iter()
        .take(a1.len().saturating_sub(1))
        .map(|&c| c as u8)
        .collect();
    let __s = String::from_utf8_lossy(&__b).into_owned();
    dataflowir_gen::AsmPrinter::print_str(&mut *__p, &__s);
    __p
}

// f366 -- `const llvm::StringLiteral &`, 1 site.  Same payload as f365.
unsafe fn f366(a0: *mut dataflowir_gen::AsmPrinter, a1: &Vec<::libc::c_char>) -> *mut dataflowir_gen::AsmPrinter {
    let __p = a0;
    let __b: Vec<u8> = a1
        .iter()
        .take(a1.len().saturating_sub(1))
        .map(|&c| c as u8)
        .collect();
    let __s = String::from_utf8_lossy(&__b).into_owned();
    dataflowir_gen::AsmPrinter::print_str(&mut *__p, &__s);
    __p
}

// t480..t482 -- llvm::SmallDenseSet<T>.  Rows g1129, g1130, g1131, g1136, g1137,
// g1138.  Same HashSet in both models: a set hands out no mapped value, so there is
// nothing for the refcount model to share and the two spellings differ only where
// the ELEMENT does.
fn t480() -> std::collections::HashSet<i64> {
    std::collections::HashSet::new()
}

fn t481() -> std::collections::HashSet<u32> {
    std::collections::HashSet::new()
}

// ⚠️ THE ELEMENT DIFFERS FROM THE REFCOUNT MODEL: `llvm::StringRef` is
// `Vec<libc::c_char>` here (f22/f23 above) and `Vec<u8>` there.  `Vec<i8>` is
// Hash + Eq, so the set is usable in both.
fn t482() -> std::collections::HashSet<Vec<libc::c_char>> {
    std::collections::HashSet::new()
}

// f400-f406 -- the `mlir::Builder` attribute factory on a t440/t441 receiver.
// 148 sites; argued in full at `f400` in src.cpp.  `getIntegerType` is LEFT OUT
// (its C++ return type `mlir::IntegerType` is unmapped), so its 1 site stays loud.

// f400 -- `getNamedAttr(StringRef, Attribute)`, 79 sites.  t21 maps
// `mlir::NamedAttribute` to the TUPLE `(String, Attr)`, so the model struct is
// destructured here rather than changing t21.  Both halves are carried.
fn f400(
    a0: &dataflowir_gen::OpBuilder,
    a1: Vec<::libc::c_char>,
    a2: dataflowir_gen::ir::Attr,
) -> (::std::string::String, dataflowir_gen::ir::Attr) {
    let __na = a0.get_named_attr(
        ::std::string::String::from_utf8_lossy(
            &a1.iter().map(|&c| c as u8).take_while(|b| *b != 0).collect::<Vec<u8>>(),
        )
        .into_owned(),
        a2,
    );
    (__na.name, __na.attr)
}

// f401 -- `getDictionaryAttr(ArrayRef<NamedAttribute>)`, 51 sites.  t19 maps
// `llvm::ArrayRef<T>` to `Vec<T>` and t21 maps the element to a tuple, so the
// pairs are rebuilt as model `NamedAttribute`s.  Returns `ir::AttrDict` (t9).
fn f401(
    a0: &dataflowir_gen::OpBuilder,
    a1: Vec<(::std::string::String, dataflowir_gen::ir::Attr)>,
) -> dataflowir_gen::ir::AttrDict {
    a0.get_dictionary_attr(
        &a1.iter()
            .map(|p| dataflowir_gen::ir::NamedAttribute::new(p.0.clone(), p.1.clone()))
            .collect::<Vec<dataflowir_gen::ir::NamedAttribute>>(),
    )
}

// f402 -- `getBoolAttr(bool)`, 6 sites.  `a1` is forwarded UNCHANGED: this is the
// `PassOptions::Option<bool>` lesson, so `true` and `false` cannot collapse.
fn f402(a0: &dataflowir_gen::OpBuilder, a1: bool) -> dataflowir_gen::ir::Attr {
    a0.get_bool_attr(a1)
}

// f403 -- `getStringAttr(const Twine &)`, 4 sites.  f365/f366 decode idiom, but
// `take_while` on the NUL rather than `take(a1.len()-1)`: that form names `a1`
// TWICE and a rule body is inlined as one expression.
fn f403(a0: &dataflowir_gen::OpBuilder, a1: &Vec<::libc::c_char>) -> dataflowir_gen::ir::Attr {
    a0.get_string_attr(
        ::std::string::String::from_utf8_lossy(
            &a1.iter().map(|&c| c as u8).take_while(|b| *b != 0).collect::<Vec<u8>>(),
        )
        .into_owned(),
    )
}

// f404 -- `getIntegerAttr(Type, int64_t)`, 1 site.  TYPE FIRST, as MLIR.
fn f404(
    a0: &dataflowir_gen::OpBuilder,
    a1: dataflowir_gen::ir::Ty,
    a2: i64,
) -> dataflowir_gen::ir::Attr {
    a0.get_integer_attr(a1, a2)
}

// f405 -- `getI64ArrayAttr(ArrayRef<int64_t>)`, 2 sites.
fn f405(a0: &dataflowir_gen::OpBuilder, a1: Vec<i64>) -> dataflowir_gen::ir::Attr {
    a0.get_i64_array_attr(&a1)
}

// f406 -- `getStrArrayAttr(ArrayRef<StringRef>)`, 4 sites.
fn f406(a0: &dataflowir_gen::OpBuilder, a1: Vec<Vec<::libc::c_char>>) -> dataflowir_gen::ir::Attr {
    a0.get_str_array_attr(
        &a1.iter()
            .map(|s| {
                ::std::string::String::from_utf8_lossy(
                    &s.iter().map(|&c| c as u8).take_while(|b| *b != 0).collect::<Vec<u8>>(),
                )
                .into_owned()
            })
            .collect::<Vec<::std::string::String>>(),
    )
}

// t540 -- `mlir::DenseArrayAttr` -> `dataflowir_gen::ir::Attr`, the same closed
// union every other *Attr key lands on (t6/t7/t9/t10/t11/t12 and the already-mapped
// `DenseArrayAttrImpl<T>` at t26/t29).  See src.cpp for why it is NOT
// `Attr::Array`/`Attr::I32Array` (those two are `mlir::ArrayAttr`, a different MLIR
// type with different rendered text) and why no member is keyed.
fn t540() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

// t541/t542/t543 -- `mlir::RewriterBase` / `mlir::PatternRewriter` /
// `mlir::IRRewriter` -> `dataflowir_gen::OpBuilder` (build.rs:461), the SAME type
// t440 lands `mlir::OpBuilder` on, because PatternMatch.h:368/780/799 make all
// three OpBuilders by inheritance.  The type carrier is t440's body verbatim.
// ⛔ NO rewrite verb is keyed -- see src.cpp: they take `mlir::Operation *`
// (t1 -> `fmt::OpInst`, a DETACHED record) and only `OpHandle` reaches an op in its
// block, so a key would silently mutate a copy.  They stay loud at rustc.
fn t541() -> dataflowir_gen::OpBuilder {
    dataflowir_gen::OpBuilder::new(dataflowir_gen::new_block_list_with_entry())
}

fn t542() -> dataflowir_gen::OpBuilder {
    dataflowir_gen::OpBuilder::new(dataflowir_gen::new_block_list_with_entry())
}

fn t543() -> dataflowir_gen::OpBuilder {
    dataflowir_gen::OpBuilder::new(dataflowir_gen::new_block_list_with_entry())
}

// ===========================================================================
// t560 / t561 / f460 / f461 -- the `llvm::simple_ilist<mlir::Block>` row (64
// asks / 16 emitted placeholder sites) and `llvm::iplist<mlir::Block>` (32 asks
// / 9 sites).  See src.cpp for the shape of all 16 sites, the all-or-nothing
// argument, and the bucket/swallow safety.  `fmt::Region { pub blocks:
// Vec<Block> }` (fmt.rs:513), and `iplist<T>` derives from `simple_ilist<T>`, so
// both keys are the SAME container and get the SAME body.  ⭐ IDENTICAL to the
// refcount model here: the container is an owning `Vec` in both, and only the
// ITERATOR representation differs (t245).
fn t560() -> Vec<dataflowir_gen::fmt::Block> {
    Vec::new()
}

fn t561() -> Vec<dataflowir_gen::fmt::Block> {
    Vec::new()
}

// f460 -- `mlir::Region::getBlocks()` -> `fmt::Region::get_blocks_mut()`
// (fmt.rs:557).  ⚠️ `a0` named EXACTLY ONCE, method call only.
unsafe fn f460(a0: &mut dataflowir_gen::fmt::Region) -> &mut Vec<dataflowir_gen::fmt::Block> {
    a0.get_blocks_mut()
}

// f461 -- `llvm::simple_ilist<mlir::Block>::begin()` -> t245's
// `*mut fmt::Block`.  ⭐ AN INTERIOR POINTER INTO THE LIVE BUFFER, not a copy:
// rules/vector f13's body verbatim (`a0.as_mut_ptr()`), so the 16 sites'
// `(*it).getArgument(0)` reads the first block in place.  t245's `null_mut()`
// init is the no-position default and would trap here, which is why begin() must
// have its own body.
unsafe fn f461(a0: &mut Vec<dataflowir_gen::fmt::Block>) -> *mut dataflowir_gen::fmt::Block {
    a0.as_mut_ptr()
}

// t2601 / t2602 / f2501-f2511 -- the `llvm::iplist<mlir::Operation>` row, one
// template argument over from t560/t561/f460/f461 above.  See src.cpp for the
// member census (39 range-for sites drove the extra end()/!=/++/* keys that
// f461 alone never needed) and the splice()/iterator-ctor sites left loud on
// purpose.  `fmt::Block { pub ops: Vec<OpInst> }` (fmt.rs:480) is the direct
// analogue of `fmt::Region { pub blocks: Vec<Block> }` t560/t561 alias into.
unsafe fn t2601() -> Vec<dataflowir_gen::fmt::OpInst> {
    Vec::new()
}
unsafe fn t2602() -> Vec<dataflowir_gen::fmt::OpInst> {
    Vec::new()
}

// f2501 -- `mlir::Block::getOperations()` -> `fmt::Block::get_operations_mut()`
// (fmt.rs:497).  f460's pattern verbatim: snake_case target on purpose, `a0`
// named exactly once.
unsafe fn f2501(
    a0: &mut dataflowir_gen::fmt::Block,
) -> &mut Vec<dataflowir_gen::fmt::OpInst> {
    a0.get_operations_mut()
}

// f2502/f2503 -- begin()/end(), rules/vector f13/f17 verbatim (an interior
// pointer into the live buffer, not a copy).
unsafe fn f2502(
    a0: &mut Vec<dataflowir_gen::fmt::OpInst>,
) -> *mut dataflowir_gen::fmt::OpInst {
    a0.as_mut_ptr()
}
unsafe fn f2503(
    a0: &mut Vec<dataflowir_gen::fmt::OpInst>,
) -> *mut dataflowir_gen::fmt::OpInst {
    a0.as_mut_ptr().add(a0.len())
}

// f2504-f2507 -- empty/front/back/size, rules/vector f3/f9/f10/f2 verbatim.
unsafe fn f2504(a0: Vec<dataflowir_gen::fmt::OpInst>) -> bool {
    a0.is_empty()
}
unsafe fn f2505(
    a0: &mut Vec<dataflowir_gen::fmt::OpInst>,
) -> *mut dataflowir_gen::fmt::OpInst {
    (a0.first_mut().unwrap())
}
unsafe fn f2506(
    a0: &mut Vec<dataflowir_gen::fmt::OpInst>,
) -> *mut dataflowir_gen::fmt::OpInst {
    (a0.last_mut().unwrap())
}
unsafe fn f2507(a0: Vec<dataflowir_gen::fmt::OpInst>) -> usize {
    a0.len()
}

// f2508-f2511 -- the range-for iterator protocol, rules/vector f22/f34/f26/f27
// verbatim: a reference/`bool` in C++ is a raw pointer/`bool` here, same as
// the iterator itself is a bare `*mut OpInst`.
unsafe fn f2508(
    a0: *mut dataflowir_gen::fmt::OpInst,
) -> *mut dataflowir_gen::fmt::OpInst {
    a0
}
unsafe fn f2509(
    a0: &mut *mut dataflowir_gen::fmt::OpInst,
) -> *mut dataflowir_gen::fmt::OpInst {
    a0.prefix_inc()
}
unsafe fn f2510(
    a0: *const dataflowir_gen::fmt::OpInst,
    a1: *const dataflowir_gen::fmt::OpInst,
) -> bool {
    a0 != a1
}
unsafe fn f2511(
    a0: *const dataflowir_gen::fmt::OpInst,
    a1: *const dataflowir_gen::fmt::OpInst,
) -> bool {
    a0 == a1
}

// ===========================================================================
// t580/t581 + f480-f487 -- THE BYTECODE STREAM.  60 placeholder sites, 200
// member calls.  Argued in full at `t580` in src.cpp; the short version is that
// an unmapped member is emitted TEXTUALLY at rc=0, so the type keys without the
// member keys would have traded 60 loud placeholders for 200 silent calls.
// `dataflowir-gen` a235c65, `src/bytecode.rs`.
// ⛔ `readSparseArray` (6 sites) IS LEFT OUT: t46 maps `MutableArrayRef<T>` to an
// OWNING `Vec<T>` by value, so the decoded values could not reach the caller's
// buffer.  It stays spelled `readSparseArray`, which the snake_case target struct
// does not have, so it fails at Rust compile time instead of succeeding wrongly.
// ===========================================================================

// t580 -- `mlir::DialectBytecodeReader` -> `dataflowir_gen::DialectBytecodeReader`.
// ⭐ `new(&[1u8])` CANNOT FAIL, and the byte is not arbitrary: a VarInt whose
// length is in the trailing zeros of byte 0 encodes 0 as `0b00000001`, so this is
// "a table of zero attributes, empty body" -- `decode_var_int` returns (0, 1), the
// decode loop runs zero times, and `bytes[1..]` is the empty body.  The reader has
// no `Default` (it FAILS rather than defaults on a malformed table, deliberately),
// so the empty-but-valid stream is the honest zero value.
unsafe fn t580() -> dataflowir_gen::DialectBytecodeReader {
    dataflowir_gen::DialectBytecodeReader::new(&[1u8]).unwrap()
}

// t581 -- `mlir::DialectBytecodeWriter` -> `dataflowir_gen::DialectBytecodeWriter`.
// An empty sink with an empty attribute table.
unsafe fn t581() -> dataflowir_gen::DialectBytecodeWriter {
    dataflowir_gen::DialectBytecodeWriter::new()
}

// f480 -- `LogicalResult readAttribute(mlir::Attribute &)`, 46 sites.
// rules/support t1 models LogicalResult as `bool`, true == success.
// ⭐ `a1` IS NAMED EXACTLY ONCE, in one arm, because a `&mut` parameter's `aN`
// re-expands to the bare lvalue at the call site.
unsafe fn f480(
    a0: &mut dataflowir_gen::DialectBytecodeReader,
    a1: &mut dataflowir_gen::ir::Attr,
) -> bool {
    match a0.read_attribute() {
        Ok(__v) => {
            drop(core::mem::replace(a1, __v));
            true
        }
        Err(_) => false,
    }
}

// f481 -- `LogicalResult readOptionalAttribute(mlir::Attribute &)`, 33 sites.
// ⭐ A DIFFERENT BODY FROM f480, not a copy: the model's `read_optional_attribute`
// returns `Option`, and the `Ok(None)` arm LEAVES THE STORAGE UNTOUCHED rather
// than writing a default-constructed Attr.  C++ leaves the caller's `Attribute`
// NULL in that case; `ir::Attr` has no null state, so "untouched" is the closest
// faithful behaviour available and is the one place this key is weaker than its
// source.  ⛔ It is NOT the "absence becomes a default" failure: nothing is
// fabricated into the slot, and `Err` is still reported as failure.
unsafe fn f481(
    a0: &mut dataflowir_gen::DialectBytecodeReader,
    a1: &mut dataflowir_gen::ir::Attr,
) -> bool {
    match a0.read_optional_attribute() {
        Ok(Some(__v)) => {
            drop(core::mem::replace(a1, __v));
            true
        }
        Ok(None) => true,
        Err(_) => false,
    }
}

// f482 -- `uint64_t getBytecodeVersion() const` on the READER, 12 sites.
// ⚠️ `u64` HERE, `i64` AT f486.  Not a typo and not to be tidied: it is the real
// C++ split (BytecodeImplementation.h:65 vs :405) and the emitted sites already
// compare against `6_u64` here and `6_i64` there.
unsafe fn f482(a0: &dataflowir_gen::DialectBytecodeReader) -> u64 {
    a0.get_bytecode_version()
}

// f483 -- `InFlightDiagnostic emitError(const llvm::Twine &) const`, 6 sites.
// `&self`, not `&mut self`: the model keeps C++'s `const` receiver by holding the
// failure flag in a `Cell<bool>`.  ⭐ THE LATCH SURVIVES THIS KEY -- calling it
// poisons the reader, so a reader that reported an error can never afterwards
// hand back a value, which is what makes the downstream `failed(...)` tests mean
// something.  The return is a real `libcc2rs::InFlightDiagnostic` (t70), so the
// `emitError(...) << x << y` chains at the sites keep working and it reports once
// on Drop.
unsafe fn f483(
    a0: &dataflowir_gen::DialectBytecodeReader,
    a1: &Vec<libc::c_char>,
) -> libcc2rs::InFlightDiagnostic {
    a0.emit_error(&::std::string::String::from_utf8_lossy(
        &a1.iter().map(|&c| c as u8).take_while(|b| *b != 0).collect::<Vec<u8>>(),
    ))
}

// f484 -- `void writeAttribute(mlir::Attribute)`, 46 sites.  C++ takes the
// attribute BY VALUE; the model borrows, so the owned parameter is lent.
unsafe fn f484(a0: &mut dataflowir_gen::DialectBytecodeWriter, a1: dataflowir_gen::ir::Attr) {
    a0.write_attribute(&a1)
}

// f485 -- `void writeOptionalAttribute(mlir::Attribute)`, 33 sites.
// ⭐ A DIFFERENT BODY FROM f484, and it has to be: the optional form emits
// `index + 1` with 0 reserved as the null sentinel, while the bare form emits the
// index itself.  The two are DIFFERENT ON THE WIRE, so one body cannot serve both.
// `Some(&a1)` is unconditional because `ir::Attr` has no null state -- the same
// asymmetry f481 records from the reading side.
unsafe fn f485(a0: &mut dataflowir_gen::DialectBytecodeWriter, a1: dataflowir_gen::ir::Attr) {
    a0.write_optional_attribute(Some(&a1))
}

// f486 -- `int64_t getBytecodeVersion() const` on the WRITER, 12 sites.
// ⚠️ `i64`, against f482's `u64`.  See f482.
unsafe fn f486(a0: &dataflowir_gen::DialectBytecodeWriter) -> i64 {
    a0.get_bytecode_version()
}

// f487 -- `void writeSparseArray(llvm::ArrayRef<int>)`, 6 sites, keyed at the ONE
// corpus instantiation `T = int`.  ⭐ THE WRITE SIDE IS KEYABLE WHERE THE READ SIDE
// IS NOT: the writer only READS the array, so t19's by-value `ArrayRef<int> ->
// Vec<i32>` copy loses nothing, whereas readSparseArray's out-param writes would
// have been dropped into a temporary (see the refusal at t580).
unsafe fn f487(a0: &mut dataflowir_gen::DialectBytecodeWriter, a1: Vec<i32>) {
    a0.write_sparse_array(&a1)
}

// ---------------------------------------------------------------------------
// t520 / f420 / f421 -- `mlir::MutableOperandRange` AS A WRITE-THROUGH VIEW.
//
// REPRESENTATION: `(*mut fmt::OpInst, u32, u32)` == `(owner, start, length)`,
// EXACTLY the triple the refusal at src.cpp:312 asked for and could not build.
//
// ⭐⭐ WHY THIS ALIASES AND DOES NOT COPY -- THE ONE THING THAT MATTERS HERE.  The
// failure this refusal existed to prevent is a view that hands back a detached
// `Vec<ir::Value>`: `assign()` would then write to a private copy, the operand
// rewrite would never reach the op, and it would all be rc=0 and compile.  This
// representation cannot do that, because it stores NO VALUES AT ALL.  It stores
// t36's own model of `mlir::Operation *` -- a raw pointer AT the `fmt::OpInst`
// that lives in its `Block`'s `ops: Vec<OpInst>` -- plus two indices.  Every read
// and every write has to go back through that pointer into
// `(*owner).operands: BTreeMap<String, Vec<ir::Value>>` (fmt.rs:441, a PUBLIC
// field), which IS the op's operand storage.  There is nowhere for a lost write to
// hide.
//
// ⭐ MEASURED, NOT ASSERTED.  `/home/agent/work/verif/mor_alias.rs` builds a
// `fmt::Block` holding one `OpInst`, takes the `*mut OpInst` at
// `block.ops.as_mut_ptr()`, builds f420's triple over it, WRITES a new `ir::Value`
// through the triple, and then reads the operand back OUT OF `block` -- not out of
// the triple.  The new value is there.  The same program also runs the
// copy-semantics control (clone the operand group first, write to the clone) and
// shows the op UNCHANGED, which is the failure mode this row was refused over.
//
// ⛔ `u32`, NOT `usize`, for `start`/`length`: the C++ parameters are `unsigned`,
// and `getODSOperandIndexAndLength` already returns `(u32, u32)` in the emitted
// corpus, so a `usize` here would put a cast at all 36 construction sites.
//
// INIT: the NULL OWNER.  `mlir::MutableOperandRange` declares no default
// constructor (ValueRange.h:128-133 -- three constructors, all with parameters),
// so no translated program can default-construct one and this `init` can never be
// emitted; it is t1's situation.  `null_mut()` is nonetheless the RIGHT sentinel
// rather than an arbitrary one: a null owner is the one state in which the view
// provably addresses no op, and any access through it traps.
fn t520() -> (*mut dataflowir_gen::fmt::OpInst, u32, u32) {
    (::std::ptr::null_mut(), 0, 0)
}

// f420 -- the 4-argument constructor.  `a3` (the `operand_segment_sizes`
// bookkeeping) is BOUND AND DROPPED, not left unmentioned: naming it exactly once
// keeps the C++ argument expression evaluated, and a rule body must name each `aN`
// at most once because the body is inlined as a single expression.  See src.cpp at
// t520 for why discarding it is sound while no resizing member is keyed.
unsafe fn f420(
    a0: *mut dataflowir_gen::fmt::OpInst,
    a1: u32,
    a2: u32,
    a3: Vec<(u32, (::std::string::String, dataflowir_gen::ir::Attr))>,
) -> (*mut dataflowir_gen::fmt::OpInst, u32, u32) {
    // ⛔ THE ANNOTATION IS LOAD-BEARING, AND THIS WAS MEASURED, NOT GUESSED.  The 4th
    // argument is DEFAULTED (`= {}`) and the recorder writes the default out at the call
    // site, so `a3` re-expands to a bare `Vec::new()` at 29 of the 30 DdlOps sites.
    // `let __segments = a3; drop(__segments);` therefore emitted `let __segments =
    // Vec::new(); drop(__segments);` -- E0282 `type annotations needed`, at rc=0 and with no
    // placeholder token, i.e. invisible to every census.  Naming the element type here
    // pins the inference at every site.
    let __segments: Vec<(u32, (::std::string::String, dataflowir_gen::ir::Attr))> = a3;
    drop(__segments);
    (a0, a1, a2)
}

// f421 -- `MutableOperandRange(Operation *owner)`: the WHOLE operand list, so
// `length` is the op's total operand count and this body has to DEREF.
// ⛔ THE SUM IS OVER THE MAP'S VALUES AND THAT IS CORRECT *ONLY* FOR A TOTAL.  The
// map iterates alphabetically by ODS argument name, which is NOT ODS declaration
// order -- but a sum does not care about order.  Any member that turns `start` into
// a particular group MUST walk `(*owner).def.arguments` instead; see src.cpp at
// t520.
unsafe fn f421(
    a0: *mut dataflowir_gen::fmt::OpInst,
) -> (*mut dataflowir_gen::fmt::OpInst, u32, u32) {
    let __owner = a0;
    let __n: usize = (*__owner).operands.values().map(|g| g.len()).sum();
    (__owner, 0u32, __n as u32)
}

// f500 -- `mlir::BlockArgument mlir::Block::getArgument(unsigned)` (Block.h:139)
// -> `fmt::Block::get_argument()`.  THE LAST LINK in the chain t560/t561/f460/f461
// built: those took the corpus from `Cpp2RustUnmapped_llvm_simple_ilist_mlir_Block_`
// to a real `*mut fmt::Block`, and then SIX `.getArgument(...)` calls on that pointer
// were emitted TEXTUALLY -- rc=0, no placeholder token, `E0599` at rustc.  See
// src.cpp for the six sites, all of them INLINED from Agen.td rather than written in
// any `.cpp`.
//
// ⚠️ THE `clone()` IS THE C++ SIGNATURE, NOT A SHORTCUT, and it is the one thing to
// get right here.  Block.h:139 returns `BlockArgument` BY VALUE, so the translated
// expression is a value and the Rust return type must be `ir::Value` (t35's target
// for `mlir::BlockArgument`, identical to t4's for `mlir::Value`).  The accessor in
// dataflowir-gen deliberately returns `&Value` INTO THE LIVE `args` -- an accessor
// that cloned internally would lose a write silently -- so the single `clone()` here
// is where the by-value C++ return is honoured, at the boundary where C++ itself
// copies the handle.  ⛔ WHAT IT COSTS, STATED: `BlockArgument` in MLIR is a handle
// callers compare by IDENTITY, and `ir::Value { name, ty }` is compared by content,
// so a site that replaced a block argument through this result would mutate a copy.
// That is t35's pre-existing widening (`getArgNumber`/`getOwner` are unmapped for the
// same reason), not a new loss, and all six corpus sites only READ.
// ⚠️ `get_argument` (the `&` half) and not `get_argument_mut`: no site writes.
// ⚠️ `a0` and `a1` are each named EXACTLY ONCE and `a0` carries nothing but a method
// call, because a `&mut` formal's `aN` re-expands to the bare lvalue.
// ⛔ OUT OF RANGE PANICS in the accessor rather than defaulting -- C++ `arguments[i]`
// past the end is UB and there is no correct value; a fabricated `Value` would be the
// `PassOptions::Option<bool>` mistake.
unsafe fn f500(a0: &mut dataflowir_gen::fmt::Block, a1: u32) -> dataflowir_gen::ir::Value {
    a0.get_argument(a1).clone()
}

// ===========================================================================
// ⛔⛔ WHY `drop(core::mem::replace(a1, __v))` AND NOT `*a1 = __v`, 2026-09-28.
// `*a1 = __v;` WAS HERE AND IT EMITTED RUST THAT DOES NOT COMPILE.  A `&mut T`
// placeholder records as `{arg 1, access: "borrow_mut"}` and the emitter renders
// that placeholder as `&mut <argexpr>` -- the rule's leading `*` is SWALLOWED, it
// is not emitted separately.  So the body emitted, at all 67 sites on
// `ddc/ddl/Dialect/DdlOps.cpp`:
//     &mut (*prop).memory = __v;        // and `&mut attr = __v;`
// which is `error[E0070]: invalid left-hand side of assignment` -- measured with
// `rustc --edition 2021` on a reduced scratch, 2 errors, rc=1.
// ⭐ AND THIS IS THE CLASS rc=0 CANNOT SEE.  E0070 is a HIR/type-check error, NOT
// a parse error: `rustfmt` PARSES `&mut x = v` happily (measured: rustfmt rc=0, it
// reformats the line and leaves it verbatim).  rustfmt is the only Rust parser in
// this pipeline, so the TU stayed `A complete rc=0` with a healthy 64705 lines and
// no `Cpp2RustUnmapped_*` token.  No placeholder census can find this.
// ⭐ THE FIX RELIES ON THE SAME RENDERING, DELIBERATELY: bare `a1` records the
// IDENTICAL `{arg 1, access: "borrow_mut"}` placeholder, so `replace(a1, __v)`
// emits `replace(&mut (*prop).memory, __v)` -- and `&mut place` is exactly what
// `core::mem::replace` wants.  `drop(...)` of the returned old value is the C++
// copy-assignment's destruction of the overwritten attribute, so this is a
// fidelity IMPROVEMENT over the assignment, not a workaround.
// ⚠️ TWO SPELLINGS THAT DO NOT WORK, both measured: `replace(&mut *a1, __v)` and
// `replace(*a1, __v)` both make rule-preprocessor ABORT with
// `semantic.rs:260: unresolved access="unknown"` -- a deref or a re-borrow of a
// placeholder inside a CALL ARGUMENT has no access classification.  Only the bare
// placeholder is classifiable there.  Do not "restore" the `*`.
// ⚠️ IDENTICAL IN BOTH MODELS: `a1` is `&mut dataflowir_gen::ir::Attr` in unsafe
// AND in refcount (t7/t10/t11/t12/t20/t44/t29 all map to the same `ir::Attr`, an
// enum -> `Sized` + movable), so `replace` is well-typed in both and there is no
// `Ptr`/`Value` wrapper to defeat it.  Verified by the f520-f531/f560/f480/f481
// bodies in the two overlays being character-identical apart from `unsafe`.
// f520-f531 -- THE TEMPLATE OVERLOADS of readAttribute/readOptionalAttribute,
// i.e. what the ODS-generated `readProperties` ACTUALLY calls.  f480/f481 key
// the virtual `Attribute &` forms and were measured DEAD (37 -> 37, 32 -> 32
// textual on `ddc/ddl/Dialect/DdlOps.cpp`); a `-verbose` leg that EXITED 0 on
// `dialects/ExPlan/ExPlanOps.cpp` names the real key as
// `readAttribute(mlir::IntegerAttr &)` -- a per-attribute-type instantiation.
// ⭐ EVERY ONE OF THESE PARAMETER TYPES MAPS TO THE SAME `ir::Attr`
// (t7/t10/t11/t12/t20/t44/t29), which is why one Rust body shape serves all of
// them AND why the C++ template's `dyn_cast<T>` is not reproducible here: the
// subtype it tests is erased by the TYPE mapping, upstream of any expression
// rule.  Nothing is fabricated; `Err` is still failure.
// ⭐ THE OPTIONAL BODIES' `Ok(None) => true` LEAVES THE CALLER'S SLOT UNTOUCHED
// AND THAT IS EXACT HERE, not a compromise: the C++ template at
// BytecodeImplementation.h:121 does `if (!baseResult) return success();` and
// likewise never assigns `result` on absence.
// ⛔ `DictionaryAttr` (5 sites, t9 -> `ir::AttrDict`) and the project's own
// attribute classes (`mlir::sentient::*`, `mlir::explan::PhaseAttr`, ...) are
// DELIBERATELY LEFT LOUD -- see the argument in src.cpp.
// ===========================================================================

// f520 -- IntegerAttr, 27 sites.
unsafe fn f520(
    a0: &mut dataflowir_gen::DialectBytecodeReader,
    a1: &mut dataflowir_gen::ir::Attr,
) -> bool {
    match a0.read_attribute() {
        Ok(__v) => {
            drop(core::mem::replace(a1, __v));
            true
        }
        Err(_) => false,
    }
}

// f521 -- StringAttr, 27 sites.
unsafe fn f521(
    a0: &mut dataflowir_gen::DialectBytecodeReader,
    a1: &mut dataflowir_gen::ir::Attr,
) -> bool {
    match a0.read_attribute() {
        Ok(__v) => {
            drop(core::mem::replace(a1, __v));
            true
        }
        Err(_) => false,
    }
}

// f522 -- ArrayAttr, 31 sites.
unsafe fn f522(
    a0: &mut dataflowir_gen::DialectBytecodeReader,
    a1: &mut dataflowir_gen::ir::Attr,
) -> bool {
    match a0.read_attribute() {
        Ok(__v) => {
            drop(core::mem::replace(a1, __v));
            true
        }
        Err(_) => false,
    }
}

// f523 -- BoolAttr, 4 sites.
unsafe fn f523(
    a0: &mut dataflowir_gen::DialectBytecodeReader,
    a1: &mut dataflowir_gen::ir::Attr,
) -> bool {
    match a0.read_attribute() {
        Ok(__v) => {
            drop(core::mem::replace(a1, __v));
            true
        }
        Err(_) => false,
    }
}

// f524 -- AffineMapAttr, 3 sites.
unsafe fn f524(
    a0: &mut dataflowir_gen::DialectBytecodeReader,
    a1: &mut dataflowir_gen::ir::Attr,
) -> bool {
    match a0.read_attribute() {
        Ok(__v) => {
            drop(core::mem::replace(a1, __v));
            true
        }
        Err(_) => false,
    }
}

// f525 -- IntegerSetAttr, 3 sites.
unsafe fn f525(
    a0: &mut dataflowir_gen::DialectBytecodeReader,
    a1: &mut dataflowir_gen::ir::Attr,
) -> bool {
    match a0.read_attribute() {
        Ok(__v) => {
            drop(core::mem::replace(a1, __v));
            true
        }
        Err(_) => false,
    }
}

// f526 -- detail::DenseArrayAttrImpl<int> (= DenseI32ArrayAttr), 24 sites.
unsafe fn f526(
    a0: &mut dataflowir_gen::DialectBytecodeReader,
    a1: &mut dataflowir_gen::ir::Attr,
) -> bool {
    match a0.read_attribute() {
        Ok(__v) => {
            drop(core::mem::replace(a1, __v));
            true
        }
        Err(_) => false,
    }
}

// f527 -- detail::DenseArrayAttrImpl<long> (= DenseI64ArrayAttr), 2 sites.
unsafe fn f527(
    a0: &mut dataflowir_gen::DialectBytecodeReader,
    a1: &mut dataflowir_gen::ir::Attr,
) -> bool {
    match a0.read_attribute() {
        Ok(__v) => {
            drop(core::mem::replace(a1, __v));
            true
        }
        Err(_) => false,
    }
}

// f528 -- OPTIONAL, IntegerAttr, 56 sites.
unsafe fn f528(
    a0: &mut dataflowir_gen::DialectBytecodeReader,
    a1: &mut dataflowir_gen::ir::Attr,
) -> bool {
    match a0.read_optional_attribute() {
        Ok(Some(__v)) => {
            drop(core::mem::replace(a1, __v));
            true
        }
        Ok(None) => true,
        Err(_) => false,
    }
}

// f529 -- OPTIONAL, StringAttr, 28 sites.
unsafe fn f529(
    a0: &mut dataflowir_gen::DialectBytecodeReader,
    a1: &mut dataflowir_gen::ir::Attr,
) -> bool {
    match a0.read_optional_attribute() {
        Ok(Some(__v)) => {
            drop(core::mem::replace(a1, __v));
            true
        }
        Ok(None) => true,
        Err(_) => false,
    }
}

// f530 -- OPTIONAL, BoolAttr, 26 sites.
unsafe fn f530(
    a0: &mut dataflowir_gen::DialectBytecodeReader,
    a1: &mut dataflowir_gen::ir::Attr,
) -> bool {
    match a0.read_optional_attribute() {
        Ok(Some(__v)) => {
            drop(core::mem::replace(a1, __v));
            true
        }
        Ok(None) => true,
        Err(_) => false,
    }
}

// f531 -- OPTIONAL, ArrayAttr, 8 sites.
unsafe fn f531(
    a0: &mut dataflowir_gen::DialectBytecodeReader,
    a1: &mut dataflowir_gen::ir::Attr,
) -> bool {
    match a0.read_optional_attribute() {
        Ok(Some(__v)) => {
            drop(core::mem::replace(a1, __v));
            true
        }
        Ok(None) => true,
        Err(_) => false,
    }
}

// f540 / f541 -- `MutableOperandRange::size()` / `::slice()`.  See src.cpp at f540
// for the witness (both callers are bucket A now), for why these two and not the
// resizing members, and for the converter-side `as`-parenthesisation bug that is
// the ACTUAL gate on those two TUs.
unsafe fn f540(a0: (*mut dataflowir_gen::fmt::OpInst, u32, u32)) -> u32 {
    a0.2
}

// f541 -- `(owner, start + subStart, subLen)`.  The owner pointer is CARRIED
// THROUGH unchanged, so the sub-range is still a write-through view of the same op.
// ⛔ `a1` is added to the FLAT ODS index and is NEVER resolved to a named operand
// group, which is why this body does not need the `(*owner).def.arguments` walk
// t520 requires of any member that does.
// ⛔ THE ANNOTATION ON `__segment` IS LOAD-BEARING for the same measured reason as
// f420's: the parameter is DEFAULTED and the recorder writes the default out at the
// call site as a bare `None`, so an unannotated `let __segment = a3;` is E0282 at
// rc=0 with no placeholder token.
unsafe fn f541(
    a0: (*mut dataflowir_gen::fmt::OpInst, u32, u32),
    a1: u32,
    a2: u32,
    a3: Option<(u32, (::std::string::String, dataflowir_gen::ir::Attr))>,
) -> (*mut dataflowir_gen::fmt::OpInst, u32, u32) {
    let __segment: Option<(u32, (::std::string::String, dataflowir_gen::ir::Attr))> = a3;
    drop(__segment);
    // ⛔ THE RECEIVER IS BORROWED IN THE BODY, NOT MOVED, AND THAT IS NOT COSMETIC.
    // `let __view = a0;` MOVES the triple.  In the unsafe model the triple is `Copy`
    // so it is harmless, but in the refcount model `libcc2rs::Ptr` is NOT `Copy`
    // (rc.rs:166 has no `derive(Copy)`), and the two witness sites slice the SAME
    // receiver twice -- `all_opnds.slice(0,1)` then `all_opnds.slice(1,1)`
    // (RegisterTypeAssignment.cpp:626-627) -- so a move is E0382 on the second one, at
    // rc=0 and with no placeholder token.  Both models borrow, so the two bodies stay
    // identical apart from the handle type.
    let __view: &(*mut dataflowir_gen::fmt::OpInst, u32, u32) = &a0;
    (__view.0, __view.1 + a1, a2)
}

// ===========================================================================
// t700/t701 + f600/f601 -- `llvm::cl::initializer<bool>` / `<int>` and
// `llvm::cl::init<Ty>`.  ⭐ THE PAYLOAD IS THE MODEL, exactly as t78/f47 model
// `llvm::cl::desc` by its `StringRef` payload: `initializer<Ty>` is a one-field
// carrier of a `Ty` plus an `apply` that this port does not map, so the honest
// Rust type is `Ty` itself and `init` is the identity.  ⛔ THE VALUE MUST SURVIVE
// -- `init(true)` silently becoming `false` is the `PassOptions::Option<bool>`
// defect t66 was refused over -- and neither body below can invent a value.
// The full argument, the four things that are lost, and the two instantiations
// left out are at `using t700 =` in src.cpp.
fn t700() -> bool {
    false
}

fn t701() -> i32 {
    0
}

// f600 -- `llvm::cl::init<bool>(const bool &)`.  IDENTITY, by value: `bool` is
// Copy and the C++ `const Ty &Init` member outlives nothing, so the copy is
// observationally identical and cannot dangle.  ⚠️ `a0` named EXACTLY ONCE.
unsafe fn f600(a0: &bool) -> bool {
    *a0
}

// f601 -- `llvm::cl::init<int>(const int &)`.  Same identity, `int` -> `i32` per
// f30's `const int &` -> `&i32`.
unsafe fn f601(a0: &i32) -> i32 {
    *a0
}


// t720 `mlir::DialectRegistry` -> AN OPAQUE UNIT, the t58/t59/t72 model.
//   122 open queue rows, 13 emitted sites, ALL of them the parameter type of a
//   generated pass base's `getDependentDialects` override.  The target has no
//   registry, no MLIRContext and no dynamic dialect loading, so there is no model
//   type to widen to -- see src.cpp for the census and the argument.
//   The body is `()`, not empty: an empty body panics at syntactic.rs:591 (t59).
fn t720() -> () {
    ()
}

// f620 -- `void mlir::DialectRegistry::insert()` -> A NO-OP, 13 sites.
//   ⭐ THE HALF THAT MAKES t720 LEGITIMATE.  `insert` is the ONLY member the corpus
//   ever reads on a registry (census pattern C = `13 insert`, nothing else), so with
//   this key the row leaves NOTHING silently textual behind it.  Without it, t720
//   would be the `OperationState -> ()` bargain: 13 loud placeholders traded for 13
//   silent calls to a method `()` does not have.
//   ⭐ THE NO-OP IS EXACT, NOT LOSSY.  `registry.insert<XDialect>()` records a
//   dialect ALLOCATOR so an MLIRContext can load the dialect before the pass runs;
//   it never touches the IR.  Here every op is a compiled-in Rust type, always
//   available, so there is nothing to register.
//   ⭐ `a0` IS NAMED ZERO TIMES, which is f124's established shape (`a0: &()`,
//   dropped, probe-confirmed reached).  The receiver is a bare ParmVarDecl
//   DeclRefExpr at all 13 sites, so dropping it drops no side effect.
unsafe fn f620(a0: &mut ()) {
    ()
}

// f560 -- GENERIC detail::DenseArrayAttrImpl<T1>, i.e. DenseI32ArrayAttr (12 asks
// on Ktdp/KtdpOps.cpp) and DenseI64ArrayAttr (8).  `T1` is unused: both map to
// `ir::Attr`.  See the src-side comment for why a concrete key cannot work.
unsafe fn f560<T1>(
    a0: &mut dataflowir_gen::DialectBytecodeReader,
    a1: &mut dataflowir_gen::ir::Attr,
) -> bool {
    match a0.read_attribute() {
        Ok(__v) => {
            drop(core::mem::replace(a1, __v));
            true
        }
        Err(_) => false,
    }
}

// ---------------------------------------------------------------------------
// t730 + f630 -- `mlir::IntegerType` and the 1-arg `Builder::getIntegerType`.
// Full argument at `using t730 =` in src.cpp.
//
// t730 `mlir::IntegerType` -> `ir::Ty` (ir.rs:37).  ⭐ THE WIDTH SURVIVES, which
//   is the whole reason this key is writable: `Ty::Int(u32)` (ir.rs:39) carries a
//   width and prints `i{w}` (ir.rs:54), so this is the t42/t75 WIDENING ("an
//   integer type" -> "a builtin type") and NOT a width erasure.
//   ⛔ TWO LOSSES, both the same ones t42/t75 carry plus one of its own:
//   (1) NO SIGNEDNESS.  Real `IntegerType` carries `SignednessSemantics`;
//       `Ty::Int(u32)` has no such field.  This is why f630 keys ONLY the 1-arg
//       overload and why `IntegerType::get` is unkeyed -- see src.cpp for the
//       arity argument that makes the first safe and the second not.
//   (2) NULL-HANDLE INIT, not a real empty integer type -- `Ty::Opaque("")` is
//       t5's empty-spelling null sentinel, exactly as t42/t73/t75 do it.  ⛔ It is
//       deliberately NOT `Ty::Int(0)`: a default-constructed `mlir::IntegerType`
//       is a NULL HANDLE, not the `i0` type, and `Int(0)` would print `i0` and
//       compare equal to a real zero-width type.
//   (3) NO ACCESSOR mapped (`getWidth()`/`isSigned()`/`isSignless()` all still
//       abort), which follows from (1).
fn t730() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(::std::string::String::new())
}

// f630 -- `Builder::getIntegerType(unsigned width)`, the 1-arg overload only.
// `build.rs:546` is `pub fn get_integer_type(&self, width: u32) -> Ty` and
// `build.rs:62` documents this exact correspondence.  Receiver is
// `&dataflowir_gen::OpBuilder`, same as f402/f404, because the declaration sits on
// the `Builder` base and both t440/t441 receivers resolve through it.  `a1` is
// forwarded UNCHANGED so two widths cannot collapse (f402's lesson).
fn f630(a0: &dataflowir_gen::OpBuilder, a1: u32) -> dataflowir_gen::ir::Ty {
    a0.get_integer_type(a1)
}

// t590 / t591 -- `mlir::DialectAsmPrinter` and `mlir::DialectAsmPrinter &`.  The
// measurement, the header check and the "restated unrelated" reason are all in
// `src.cpp` at the t590 block.  Same model as t462/t463 BY CONSTRUCTION, because the
// 5 reached sites forward the printer into a `::mlir::AsmPrinter &` parameter and the
// two spellings have to agree; a reference is a raw pointer in this model.
fn t590() -> dataflowir_gen::AsmPrinter {
    dataflowir_gen::AsmPrinter::new()
}

fn t591() -> *mut dataflowir_gen::AsmPrinter {
    ::std::ptr::null_mut()
}

// t750 `mlir::detail::ShapedTypeTrait<mlir::MemRefType>` -> `ir::Ty`, 4 sites.
//   THE CRTP BASE of the same object t73 maps, so it maps to what t73 maps to --
//   the t560/t561 "the base IS the same container" discipline.  No new model
//   claim.  Init is t73's own `Ty::Opaque("")` null sentinel.
fn t750() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(::std::string::String::new())
}

// t751 `mlir::detail::ShapedTypeTrait<mlir::VectorType>` -> `ir::Ty`, 3 sites.
//   Same, over t75.
fn t751() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(::std::string::String::new())
}

// f650 `ShapedTypeTrait<MemRefType>::getRank() const` -> THE SHAPE LENGTH.
//   BuiltinTypeInterfaces.h.inc:604-608 is `assert(hasRank()); return
//   getShape().size();` and `Ty::MemRef(Vec<i64>, Box<Ty>)` (ir.rs:44) IS that
//   shape, so this is computed from the model, not invented.  ⛔ The fallback arm
//   is the C++ `assert(hasRank())`, not a placeholder: t73's init is the
//   `Ty::Opaque("")` NULL handle and querying the rank of one is UB in C++ too.
//   Returning 0 would be silently wrong -- rank 0 is a LEGAL rank.
//   `a0` is named ONCE (f403's inlining constraint).
fn f650(a0: &dataflowir_gen::ir::Ty) -> i64 {
    match a0 {
        dataflowir_gen::ir::Ty::MemRef(shape, _) => shape.len() as i64,
        _ => panic!("ub: cannot query rank of unranked shaped type"),
    }
}

// f651 `ShapedTypeTrait<VectorType>::getNumElements() const` -> THE SHAPE PRODUCT.
//   BuiltinTypeInterfaces.h.inc:609-612 is `assert(hasStaticShape());
//   return ShapedType::getNumElements(getShape());`, i.e. the product of the
//   dimensions.  `Ty::Vector(Vec<i64>, Box<Ty>)` (ir.rs:42) carries them, with a
//   NEGATIVE dim meaning dynamic `?` (ir.rs:56-60) -- which is exactly the
//   `hasStaticShape()` the C++ asserts, so a dynamic dim panics here as it aborts
//   there rather than multiplying a sentinel into the answer.
fn f651(a0: &dataflowir_gen::ir::Ty) -> i64 {
    match a0 {
        dataflowir_gen::ir::Ty::Vector(shape, _) => {
            let mut n: i64 = 1;
            for d in shape {
                if *d < 0 {
                    panic!("ub: cannot get element count of dynamic shaped type");
                }
                n *= *d;
            }
            n
        }
        _ => panic!("ub: cannot get element count of dynamic shaped type"),
    }
}

// t770 `mlir::detail::SymbolOpInterfaceTrait<mlir::func::FuncOp>` -> `fmt::OpInst`,
//   4 emitted cast sites / 2 files.  A DerivedToBase cast does not change the object,
//   so the trait base of `func::FuncOp` is whatever `func::FuncOp` is: t157's
//   `fmt::OpInst`.  The init is t157's/f125's, byte-identical and for their reason --
//   a default-constructed op handle is the NULL handle, `fmt::OpInst` has no null, and
//   this expression exists only to type-check the type key.  Nothing default-constructs
//   this trait base in the corpus, so the value is never read.
fn t770() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// f670 `SymbolOpInterfaceTrait<func::FuncOp>::getName()` -> THE SYMBOL NAME.
//   SymbolInterfaces.h.inc:383-385 is `return getNameAttr().getValue();` and :265-267 is
//   `return mlir::SymbolTable::getSymbolName(this->getOperation());`, i.e. the `sym_name`
//   ATTRIBUTE -- NOT `Operation::getName()`, which would be the mnemonic `"func.func"`.
//   So this is `attrs["sym_name"]`, read exactly as the crate's own func.func printer
//   reads it (`custom.rs:751`, `op.attrs.get("sym_name")`).
//   RETURN SHAPE: the corpus StringRef model, a NUL-TERMINATED `Vec<libc::c_char>`
//   (rules/stringref `t1` is `vec![0]`, its `f7` size() is `len() - 1`), so the
//   terminator is pushed.  The four emitted sites already spell
//   `.iter().take(len().saturating_sub(1)).map(|&c| c as u8)`, which is precisely this
//   representation -- the key makes the text that is already emitted compile.
//   ⛔ THE FALLBACK ARM IS THE C++ BEHAVIOUR, NOT A PLACEHOLDER: with no `sym_name`,
//   `SymbolTable::getSymbolName` hands back a NULL `StringAttr` and `.getValue()` on one
//   dereferences null in C++.  `dataflowir_gen`'s own printer errors `Missing("$sym_name")`
//   in the same situation.  Returning an empty name would be silently wrong -- an empty
//   symbol name is not a legal func.func -- so this panics, the f650 discipline.
//   `a0` is named ONCE (f403's inlining constraint).
fn f670(a0: &dataflowir_gen::fmt::OpInst) -> Vec<libc::c_char> {
    match a0.attrs.get("sym_name") {
        Some(dataflowir_gen::ir::Attr::Str(s)) => {
            let mut __v: Vec<libc::c_char> = s.bytes().map(|b| b as libc::c_char).collect();
            __v.push(0);
            __v
        }
        _ => panic!("ub: getName() on an operation carrying no sym_name attribute"),
    }
}

// f750-f753 -- the four missing members of the t460-t463 printer `<<` family.
// src.cpp carries the aggregated site table, the receiver/non-inheritance reason
// these are distinct keys rather than duplicates, and the ten sites left out.
// Writes go through an explicit `&mut *p` reborrow, fully qualified as an inherent
// associated function, for the f360-f366 reasons: the body is inlined into the
// translated crate so nothing may depend on what is in scope there, and a bare
// `(*p).method()` autoref trips the deny-by-default `dangerous_implicit_autorefs`.
// Every body returns its own `a0` so `p << a << b` chains.

// f750 -- `mlir::AsmPrinter &` + `const char (&)[_]`, 23 sites.  f360's body on the
// base receiver: the converter materialises a string literal in this position as a
// `c"..."` CStr, so the parameter is `&CStr` and `to_bytes()` already stops at the NUL.
unsafe fn f750(a0: *mut dataflowir_gen::AsmPrinter, a1: &std::ffi::CStr) -> *mut dataflowir_gen::AsmPrinter {
    let __p = a0;
    let __s = String::from_utf8_lossy(a1.to_bytes()).into_owned();
    dataflowir_gen::AsmPrinter::print_str(&mut *__p, &__s);
    __p
}

// f751 -- `mlir::AsmPrinter &` + `const long &`, 3 sites.  f29's `&i64` spelling;
// `print_i64` is plain decimal, which is what MLIR's `getStream() << value` produces.
unsafe fn f751(a0: *mut dataflowir_gen::AsmPrinter, a1: &i64) -> *mut dataflowir_gen::AsmPrinter {
    let __p = a0;
    let __v = *a1;
    dataflowir_gen::AsmPrinter::print_i64(&mut *__p, __v);
    __p
}

// f752 -- `mlir::AsmPrinter &` + `mlir::Type` by value, 2 sites.  t5 is
// `dataflowir_gen::ir::Ty`; `print_type` takes it by reference, so the by-value
// parameter is borrowed in place and nothing is cloned.
unsafe fn f752(a0: *mut dataflowir_gen::AsmPrinter, a1: dataflowir_gen::ir::Ty) -> *mut dataflowir_gen::AsmPrinter {
    let __p = a0;
    dataflowir_gen::AsmPrinter::print_type(&mut *__p, &a1);
    __p
}

// f753 -- `mlir::OpAsmPrinter &` + `mlir::Type` by value, 17 sites.  f752's body on
// the derived receiver; both receivers are the same `AsmPrinter` sink in this model.
unsafe fn f753(a0: *mut dataflowir_gen::AsmPrinter, a1: dataflowir_gen::ir::Ty) -> *mut dataflowir_gen::AsmPrinter {
    let __p = a0;
    dataflowir_gen::AsmPrinter::print_type(&mut *__p, &a1);
    __p
}

// ===========================================================================
// t950-t956 + f850-f871 -- THE ASM-PARSER READER HALF (slot `parserkeys`).
// Model: `dataflowir_gen::AsmParser` (dt_src c81086e, dataflowir-gen/src/asm.rs:605).
// ⭐ THE CONTRACT: `true` == success, and any method returning `false` HAS CONSUMED
// NOTHING.  rules/support t4 models `llvm::ParseResult` as `bool` the same way round,
// so every body below is a bare forward and nothing has to be inverted.  See src.cpp
// for the operand family's refusal (t84 maps `UnresolvedOperand` to `()`, so there is
// no place to put a name and no name to look up) and for the six other omissions.

// t950 -- `mlir::OpAsmParser::Argument` -> `()`.  THE BUCKET GATE: the first abort of
// 4 TUs, which emit ZERO lines today.  An opaque unit for t84's exact reason -- every
// corpus use declares, forwards and collects it, and the two sites that DO read a field
// fail at rustc with `E0609` rather than silently.  See src.cpp for why a faithful
// struct is not expressible (a field access is a MemberExpr, unkeyable, and no Rust
// struct can have a field spelled `type`).
unsafe fn t950() -> () {
    ()
}

// t951-t956 -- `mlir::AsmParser`, `mlir::OpAsmParser` and `mlir::DialectAsmParser`,
// by value and by reference.  ONE model for all three: the reader is the reader, and
// the derived spellings add no state (t590/t591 made the identical call on the
// printer side for `DialectAsmPrinter`).  `AsmParser::new("")` is the honest zero
// value -- an EMPTY buffer, on which every method correctly returns `false`.  ⛔ It
// must not be a buffer with content: that would make a default-constructed parser
// accept tokens nobody supplied.
unsafe fn t951() -> dataflowir_gen::AsmParser {
    dataflowir_gen::AsmParser::new("")
}
unsafe fn t952() -> *mut dataflowir_gen::AsmParser {
    ::core::ptr::null_mut()
}
unsafe fn t953() -> dataflowir_gen::AsmParser {
    dataflowir_gen::AsmParser::new("")
}
unsafe fn t954() -> *mut dataflowir_gen::AsmParser {
    ::core::ptr::null_mut()
}
unsafe fn t955() -> dataflowir_gen::AsmParser {
    dataflowir_gen::AsmParser::new("")
}
unsafe fn t956() -> *mut dataflowir_gen::AsmParser {
    ::core::ptr::null_mut()
}

// f850 -- the constructor for t950.  f62's shape.
unsafe fn f850() -> () {
    ()
}

// f851 -- `parseArrow()`. ⭐ NOT `parse_punct('-')`: `->` is TWO chars, and `parse_punct('-')`
// would consume the `-` and leave the `>`.
unsafe fn f851(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_arrow(&mut *a0)
}

// f852 -- `parseOptionalArrow()`. Absent is not an error and consumes nothing.
unsafe fn f852(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_optional_arrow(&mut *a0)
}

// f853 -- `parseColon()`, 9 sites.
unsafe fn f853(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_punct(&mut *a0, ':')
}

// f854 -- `parseOptionalColon()`.
unsafe fn f854(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_optional_punct(&mut *a0, ':')
}

// f855 -- `parseComma()`.
unsafe fn f855(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_punct(&mut *a0, ',')
}

// f856 -- `parseOptionalComma()`.
unsafe fn f856(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_optional_punct(&mut *a0, ',')
}

// f857 -- `parseEqual()`.
unsafe fn f857(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_punct(&mut *a0, '=')
}

// f858 -- `parseOptionalEqual()`.
unsafe fn f858(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_optional_punct(&mut *a0, '=')
}

// f859 -- `parseLess()`.
unsafe fn f859(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_punct(&mut *a0, '<')
}

// f860 -- `parseOptionalLess()`.
unsafe fn f860(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_optional_punct(&mut *a0, '<')
}

// f861 -- `parseGreater()`.
unsafe fn f861(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_punct(&mut *a0, '>')
}

// f862 -- `parseOptionalGreater()`.
unsafe fn f862(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_optional_punct(&mut *a0, '>')
}

// f863 -- `parseLParen()`.
unsafe fn f863(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_punct(&mut *a0, '(')
}

// f864 -- `parseOptionalLParen()`.
unsafe fn f864(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_optional_punct(&mut *a0, '(')
}

// f865 -- `parseRParen()`.
unsafe fn f865(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_punct(&mut *a0, ')')
}

// f866 -- `parseOptionalRParen()`.
unsafe fn f866(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_optional_punct(&mut *a0, ')')
}

// f867 -- `parseLSquare()`.
unsafe fn f867(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_punct(&mut *a0, '[')
}

// f868 -- `parseOptionalLSquare()`.
unsafe fn f868(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_optional_punct(&mut *a0, '[')
}

// f869 -- `parseRSquare()`.
unsafe fn f869(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_punct(&mut *a0, ']')
}

// f870 -- `parseOptionalRSquare()`.
unsafe fn f870(a0: *mut dataflowir_gen::AsmParser) -> bool {
    dataflowir_gen::AsmParser::parse_optional_punct(&mut *a0, ']')
}

// f871 -- `parseType(mlir::Type &)`.  t5 maps `mlir::Type` to `ir::Ty` and a `T &`
// parameter to `&mut <mapped>`, so the decoded type is written through.
// ⚠️ On a FUNCTIONAL type this reads `(i32, i32)` and leaves ` -> i32` unconsumed
// (`ir::Ty` has no function variant) -- loud and documented in the crate, not a
// silent acceptance introduced here.
unsafe fn f871(a0: *mut dataflowir_gen::AsmParser, a1: &mut dataflowir_gen::ir::Ty) -> bool {
    dataflowir_gen::AsmParser::parse_type(&mut *a0, a1)
}

// t920-t927 -- the DenseSet family over the mlir handle types, 24 sites.  Argued
// in full at `t920` in src.cpp.  SAME `HashSet` in BOTH models, for t480-t482's
// reason: a set hands out no mapped value, so there is nothing for the refcount
// model to share, and unlike t482's `llvm::StringRef` these three elements are
// spelled identically under both models.
//
// ⭐ `Hash` IS WHAT MAKES THESE WRITABLE AT ALL.  `dataflowir-gen` 308a547 derives
// it on `ir::Attr` (ir.rs:540) and it was already on `ir::Ty` (ir.rs:36, the only
// non-trivial payload `Attr` carries) and on `ir::Value` (ir.rs:20).  Without it a
// `HashSet<ir::Attr>` is a well-formed TYPE that records cleanly and then no
// membership operation on it can compile -- which is the silent lie the refusal
// these keys replace was right to refuse.
//
// ⛔ CORRECTED: this said "NO MEMBER RULE ... `insert` ... stays loud".  The
// members ARE keyed now, at t921/t928 + f900-f909 below, with a REAL iterator
// (`libcc2rs::UnsafeHashSetIterator`, which already existed).  What stays loud is
// the RANGE-FOR, and not for a rules reason: `converter.cpp ConvertLoopVariable`
// lowers every range-for as `0..len()` + `as_ptr().add(i)` and never consults
// `begin`/`end`, so `HashSet` has no `as_ptr` and that one site is E0599 no
// matter what this file says.  Full census at `t920` in src.cpp.

// t920 -- `llvm::SmallDenseSet<mlir::Attribute>` (the SEARCHED-AS spelling; the
// arity-3 `from decl` text is NOT a key).  14 sites, ScratchpadConflicts.cpp.
fn t920() -> std::collections::HashSet<dataflowir_gen::ir::Attr> {
    std::collections::HashSet::new()
}

// t922 / t923 -- `mlir::Attribute`'s `DenseSetImpl` base, `DenseMap` and
// `SmallDenseMap` MapTy respectively.  t6 maps the element to `ir::Attr`.
fn t922() -> std::collections::HashSet<dataflowir_gen::ir::Attr> {
    std::collections::HashSet::new()
}

fn t923() -> std::collections::HashSet<dataflowir_gen::ir::Attr> {
    std::collections::HashSet::new()
}

// t924 / t925 -- `mlir::StringAttr`'s `DenseSetImpl` base.  ⭐ THE ELEMENT IS THE
// SAME `ir::Attr`, because t7 maps `mlir::StringAttr` to it: in real MLIR a
// `StringAttr` IS an `Attribute` sharing the uniquer handle, and this module
// already commits to that identity.  Distinct C++ search keys, one Rust type.
fn t924() -> std::collections::HashSet<dataflowir_gen::ir::Attr> {
    std::collections::HashSet::new()
}

fn t925() -> std::collections::HashSet<dataflowir_gen::ir::Attr> {
    std::collections::HashSet::new()
}

// t926 / t927 -- `mlir::Value`'s `DenseSetImpl` base.  t4 maps the element to
// `ir::Value`, which derives `Hash + Eq` at ir.rs:20.
fn t926() -> std::collections::HashSet<dataflowir_gen::ir::Value> {
    std::collections::HashSet::new()
}

fn t927() -> std::collections::HashSet<dataflowir_gen::ir::Value> {
    std::collections::HashSet::new()
}

// ===========================================================================
// THE MEMBER HALF -- t921/t928 + f900-f909, `mlir::Attribute` only.  Argued in
// full at the member-half header in src.cpp.  IDENTICAL IN BOTH MODELS for the
// t920 reason: a set hands out no mapped value, so the container is a plain
// `HashSet` under both, and therefore the iterator is the UNSAFE one under both
// (`RefcountHashSetIter` is keyed on `Ptr<HashSet<K>>`, which is NOT this
// module's representation).
//
// ⭐ `.first` IS A REAL ITERATOR, NOT A PLACEHOLDER.  `find_key` positions it on
// the inserted-or-already-present element, which is exactly what C++
// `DenseSetImpl::insert` returns; `.second` is the real `HashSet::insert`
// boolean.  So neither element of the tuple is a lie.  This is a REUSE of
// `rules/unordered_map`'s t5/f41/f42 over the identical Rust container.
//
// ⚠️ `__k.clone()` IS LOAD-BEARING: the key has to outlive the `insert` so
// `find_key` can locate it, and `HashSet::insert` consumes its argument.
// ⚠️ THE CAST `__set as *const HashSet<..>` re-borrows the receiver the iterator
// points into -- the unsafe model's established iterator shape
// (rules/unordered_map f41, rules/set f13), not a new liberty taken here.
// ===========================================================================

fn t921() -> libcc2rs::UnsafeHashSetIterator<dataflowir_gen::ir::Attr> {
    libcc2rs::UnsafeHashSetIterator::null()
}

fn t928() -> libcc2rs::UnsafeHashSetIterator<dataflowir_gen::ir::Attr> {
    libcc2rs::UnsafeHashSetIterator::null()
}

// f900/f901 -- `insert` on t922 (`DenseMap` MapTy), const-ref and rvalue.
unsafe fn f900(
    a0: &mut std::collections::HashSet<dataflowir_gen::ir::Attr>,
    a1: dataflowir_gen::ir::Attr,
) -> (
    libcc2rs::UnsafeHashSetIterator<dataflowir_gen::ir::Attr>,
    bool,
) {
    {
        // ⚠️⚠️ THE SPELLING OF THE RECEIVER HERE IS MEASURED, NOT IDIOMATIC, AND
        // TWO NATURAL FORMS WERE WRONG.  The converter substitutes a `&mut`
        // parameter with the CALLER'S PLACE EXPRESSION (`keys`, `(*result)`),
        // not with a reference to it, and it inserts its own autoref in a
        // reference context.  Measured, on this module, at the pin:
        //   `&mut *a0` -> `&mut &mut keys`  (E0308 on `HashSet::insert`'s UFCS
        //                                    receiver, E0606 on the cast)
        //   `a0`       -> `keys`            (E0605: `HashSet as *const HashSet`)
        //   `&raw const *a0` -> `&raw const keys`   ✓ -- a RAW-REF context gets
        //                                    no autoref, so both readings agree.
        // So `a0` appears exactly once, inside `&raw const`, and the insert goes
        // through the resulting pointer.  rules/unordered_map f41's `&mut *a0`
        // shape does NOT transplant here.
        let __k = a1;
        let __p = &raw const *a0;
        let __inserted = (*(__p as *mut std::collections::HashSet<dataflowir_gen::ir::Attr>))
            .insert(__k.clone());
        (libcc2rs::UnsafeHashSetIterator::find_key(__p, &__k), __inserted)
    }
}

unsafe fn f901(
    a0: &mut std::collections::HashSet<dataflowir_gen::ir::Attr>,
    a1: dataflowir_gen::ir::Attr,
) -> (
    libcc2rs::UnsafeHashSetIterator<dataflowir_gen::ir::Attr>,
    bool,
) {
    {
        // ⚠️⚠️ THE SPELLING OF THE RECEIVER HERE IS MEASURED, NOT IDIOMATIC, AND
        // TWO NATURAL FORMS WERE WRONG.  The converter substitutes a `&mut`
        // parameter with the CALLER'S PLACE EXPRESSION (`keys`, `(*result)`),
        // not with a reference to it, and it inserts its own autoref in a
        // reference context.  Measured, on this module, at the pin:
        //   `&mut *a0` -> `&mut &mut keys`  (E0308 on `HashSet::insert`'s UFCS
        //                                    receiver, E0606 on the cast)
        //   `a0`       -> `keys`            (E0605: `HashSet as *const HashSet`)
        //   `&raw const *a0` -> `&raw const keys`   ✓ -- a RAW-REF context gets
        //                                    no autoref, so both readings agree.
        // So `a0` appears exactly once, inside `&raw const`, and the insert goes
        // through the resulting pointer.  rules/unordered_map f41's `&mut *a0`
        // shape does NOT transplant here.
        let __k = a1;
        let __p = &raw const *a0;
        let __inserted = (*(__p as *mut std::collections::HashSet<dataflowir_gen::ir::Attr>))
            .insert(__k.clone());
        (libcc2rs::UnsafeHashSetIterator::find_key(__p, &__k), __inserted)
    }
}

// f902/f903 -- `insert` on t923 (`SmallDenseMap` MapTy).  The two the corpus
// actually reaches.
unsafe fn f902(
    a0: &mut std::collections::HashSet<dataflowir_gen::ir::Attr>,
    a1: dataflowir_gen::ir::Attr,
) -> (
    libcc2rs::UnsafeHashSetIterator<dataflowir_gen::ir::Attr>,
    bool,
) {
    {
        // ⚠️⚠️ THE SPELLING OF THE RECEIVER HERE IS MEASURED, NOT IDIOMATIC, AND
        // TWO NATURAL FORMS WERE WRONG.  The converter substitutes a `&mut`
        // parameter with the CALLER'S PLACE EXPRESSION (`keys`, `(*result)`),
        // not with a reference to it, and it inserts its own autoref in a
        // reference context.  Measured, on this module, at the pin:
        //   `&mut *a0` -> `&mut &mut keys`  (E0308 on `HashSet::insert`'s UFCS
        //                                    receiver, E0606 on the cast)
        //   `a0`       -> `keys`            (E0605: `HashSet as *const HashSet`)
        //   `&raw const *a0` -> `&raw const keys`   ✓ -- a RAW-REF context gets
        //                                    no autoref, so both readings agree.
        // So `a0` appears exactly once, inside `&raw const`, and the insert goes
        // through the resulting pointer.  rules/unordered_map f41's `&mut *a0`
        // shape does NOT transplant here.
        let __k = a1;
        let __p = &raw const *a0;
        let __inserted = (*(__p as *mut std::collections::HashSet<dataflowir_gen::ir::Attr>))
            .insert(__k.clone());
        (libcc2rs::UnsafeHashSetIterator::find_key(__p, &__k), __inserted)
    }
}

unsafe fn f903(
    a0: &mut std::collections::HashSet<dataflowir_gen::ir::Attr>,
    a1: dataflowir_gen::ir::Attr,
) -> (
    libcc2rs::UnsafeHashSetIterator<dataflowir_gen::ir::Attr>,
    bool,
) {
    {
        // ⚠️⚠️ THE SPELLING OF THE RECEIVER HERE IS MEASURED, NOT IDIOMATIC, AND
        // TWO NATURAL FORMS WERE WRONG.  The converter substitutes a `&mut`
        // parameter with the CALLER'S PLACE EXPRESSION (`keys`, `(*result)`),
        // not with a reference to it, and it inserts its own autoref in a
        // reference context.  Measured, on this module, at the pin:
        //   `&mut *a0` -> `&mut &mut keys`  (E0308 on `HashSet::insert`'s UFCS
        //                                    receiver, E0606 on the cast)
        //   `a0`       -> `keys`            (E0605: `HashSet as *const HashSet`)
        //   `&raw const *a0` -> `&raw const keys`   ✓ -- a RAW-REF context gets
        //                                    no autoref, so both readings agree.
        // So `a0` appears exactly once, inside `&raw const`, and the insert goes
        // through the resulting pointer.  rules/unordered_map f41's `&mut *a0`
        // shape does NOT transplant here.
        let __k = a1;
        let __p = &raw const *a0;
        let __inserted = (*(__p as *mut std::collections::HashSet<dataflowir_gen::ir::Attr>))
            .insert(__k.clone());
        (libcc2rs::UnsafeHashSetIterator::find_key(__p, &__k), __inserted)
    }
}

// f904/f905 -- `contains`.  Exact in both directions.
unsafe fn f904(
    a0: std::collections::HashSet<dataflowir_gen::ir::Attr>,
    a1: dataflowir_gen::ir::Attr,
) -> bool {
    a0.contains(&a1)
}

unsafe fn f905(
    a0: std::collections::HashSet<dataflowir_gen::ir::Attr>,
    a1: dataflowir_gen::ir::Attr,
) -> bool {
    a0.contains(&a1)
}

// f906/f907 -- `empty`.
unsafe fn f906(a0: std::collections::HashSet<dataflowir_gen::ir::Attr>) -> bool {
    a0.is_empty()
}

unsafe fn f907(a0: std::collections::HashSet<dataflowir_gen::ir::Attr>) -> bool {
    a0.is_empty()
}

// f908/f909 -- `size`.  C++ `unsigned`, so `u32`, not `usize`.
unsafe fn f908(a0: std::collections::HashSet<dataflowir_gen::ir::Attr>) -> u32 {
    (a0.len() as u32)
}

unsafe fn f909(a0: std::collections::HashSet<dataflowir_gen::ir::Attr>) -> u32 {
    (a0.len() as u32)
}

// t980 -- `mlir::OwningOpRef<mlir::ModuleOp>` -> `libcc2rs::OwningOpRef<()>`.
// The OpTy argument is t61's model for `mlir::ModuleOp`, i.e. `()`; see src.cpp for
// why ownership-with-a-tripwire is the model and why no member is keyed.
//
// The default value is the NULL handle, which is what C++'s
// `OwningOpRef(std::nullptr_t = nullptr)` gives a declaration the converter cannot
// initialise -- and it is exactly what `dxp/dxp.h:76`'s member holds for the whole
// of `dxp_standalone.cpp`.  ⚠️ Unlike t800 (`rules/tooloutputfile`) this default is
// NOT a panic and must not be: a null handle is a real, valid, observable C++ state
// with a no-op destructor, so inventing a failure here would be wrong.
fn t980() -> libcc2rs::OwningOpRef<()> {
    libcc2rs::OwningOpRef::null()
}

// ===========================================================================
// f880 -- `parseOperand`, unblocked by the t84 remodel (slot `t84`).  28 sites.
// Model: `dataflowir_gen::AsmParser::parse_operand` (dt_src c81086e, asm.rs:1044).
// ⭐ THE CONTRACT: `true` == success, and any body returning `false` HAS CONSUMED
// NOTHING.  The crate guarantees both halves (it restores `self.pos` on failure) and
// this body does not weaken either.
// ⛔ tgt_unsafe.rs-ONLY, exactly as t950-t956/f850-f871 are, and the reason is structural
// rather than an omission: `dataflowir_gen::AsmParser` has no `impl libcc2rs::ByteRepr`,
// so `Ptr::<AsmParser>::with_mut` does not exist (libcc2rs/src/rc.rs:507) and a refcount
// body is `E0277`.  check-ir.sh lists such keys in its `rc_only_types` NOTE, which is the
// documented green-tree shape.  The one-line crate fix (`impl ByteRepr for AsmParser {}`)
// belongs to a crate slot, not here.
// ⚠️ t84 ITSELF IS STILL TWO-SIDED -- `String` needs neither -- so nothing that merely
// STORES an `UnresolvedOperand` loses its container in the refcount tree.
//
// ⛔ `false` IS DIAGNOSED, NOT DISCARDED.  `allowResultNumber=false` must reject `%foo#2`,
// and the crate's `parse_operand` hardcodes `true` (the flag lives on the private
// `ssa_name_end`).  Silently treating it as `true` would make a parser accept what it must
// reject, so that case records an error and returns `false` -- consuming nothing, per the
// contract.  `emit_error` RETURNS `false`, so it is the whole answer.
//
// ⛔⛔ TRAP 1, WORTH MORE THAN THIS KEY: **A RULE BODY IS INLINED INTO THE EMITTED `.rs`
// -- ITS STRING LITERALS *AND* ITS COMMENTS.**  The diagnostic below first read
// "parseOperand(allowResultNumber=false) is not modelled...", and then a comment
// explaining that.  Both put the camelCase token BACK into the output, once per site, so
// the residue count in Symbol.cpp stayed at exactly 5 -- indistinguishable from a DEAD KEY
// -- while `-verbose` showed `Matching: llvm::ParseResult mlir::OpAsmParser::
// parseOperand(...)` at all 5.  Two regen cycles were spent on it.  ⭐ SO: a key's own
// message and its in-body comments must never contain the token its witness counts, and a
// flat residue count must ALWAYS be cross-checked against the snake_case ARRIVAL count
// before it is read as a dead key.  Hence "allow-result-number", hyphenated, and this note
// living OUTSIDE the body.
//
// ⛔⛔ TRAP 2, AND IT IS THE ONE THAT WOULD HAVE SHIPPED BROKEN.  `a2` WAS `Option<bool>`
// here for one regen cycle, on the f541 precedent that "a DEFAULTED parameter arrives as a
// bare `None`".  THAT PRECEDENT DOES NOT GENERALISE: f541's parameter is a C++
// `std::optional<...>`, so its `Option` comes from the TYPE, not from the default.  For a
// plain `bool` with `= true` the converter substitutes THE DEFAULT EXPRESSION ITSELF, and
// the emitted site read
//     if !match true { Option::None | Some(true) => ..., Some(false) => ... }
// -- a `bool` scrutinee against `Option` patterns, i.e. `E0308` at EVERY site.
// ⚠️ AND rc WAS 0, rustfmt WAS CLEAN, THE `-verbose` LOG SAID `Matching:`, AND THE
// snake_case ARRIVAL COUNT WENT 0 -> 5/10.  Every signal this row is normally measured by
// said the key had landed.  Only extracting the emitted region and running REAL rustc
// found it, which is why that step is mandatory and not optional.
// ⭐ THE TELL, for the next slot: the BEFORE (unmapped) emission spells the elided
// argument `None` because the converter does not know the parameter's type yet; once a
// rule exists it spells the C++ DEFAULT.  So the BEFORE text is NOT evidence for the
// target parameter type -- it is evidence of the absence of a rule.
unsafe fn f880(a0: *mut dataflowir_gen::AsmParser, a1: &mut ::std::string::String, a2: bool) -> bool {
    if a2 {
        dataflowir_gen::AsmParser::parse_operand(&mut *a0, a1)
    } else {
        let __loc: usize = dataflowir_gen::AsmParser::current_location(&*a0);
        dataflowir_gen::AsmParser::emit_error(
            &mut *a0,
            __loc,
            "allow-result-number=false is not modelled: the reader cannot reject a \
             result number, so refusing rather than over-accepting",
        )
    }
}

// t1040 `mlir::StorageUniquer::StorageAllocator` -> AN OPAQUE UNIT, the t720 model.
//   7 gating TUs. The allocator is WRITE-ONLY at every site in the corpus: the only
//   two members named anywhere are `allocate<T>()` (36 uses, all placement-new
//   operands, which `VisitCXXNewExpr` never converts) and `copyInto` (2 uses, whose
//   value is read back through the RETURNED ArrayRef). See src.cpp for the census
//   and for why the standing "an arena cannot be `()`" refusal does not survive it.
//   The body is `()`, not empty: an empty body panics at syntactic.rs:591 (t59).
fn t1040() -> () {
    ()
}

// f1000 -- `ArrayRef<T1> StorageAllocator::copyInto(ArrayRef<T1>)` -> IDENTITY.
//   ⭐ THE HALF THAT MAKES t1040 LEGITIMATE. `llvm::ArrayRef<T1>` is t19 -> `Vec<T1>`,
//   an OWNED vector, so handing the argument straight back is not a discard: the
//   Rust value owns exactly the elements C++ copied into the arena, and outlives the
//   arena rather than borrowing from it. `a0` is unused because the arena it names
//   has no representation and needs none.
unsafe fn f1000<T1>(a0: &mut (), a1: Vec<T1>) -> Vec<T1> {
    a1
}

// t990 `mlir::detail::DialectInterfaceBase<mlir::ktdf_arch::FeatureDialectInterface,
// mlir::DialectInterface>` -> AN OPAQUE UNIT.  42 TUs, the largest single type gap in
// the corpus.  See src.cpp for the six-TU `--survey` member census (count=1 per TU,
// role = rule-mapped base class, ZERO member rows) and for why `()` is exact: the
// class declares no data member, has no destructor of its own, and its only two
// members are a static RTTI accessor the corpus never calls on this instantiation and
// a protected forwarding constructor.
//
// ⚠️ IDENTICAL IN BOTH MODELS and spelled in BOTH overlays, the t72 `mlir::TypeID`
// precedent: a unit type key is repeated rather than relying on a "value types are not
// repeated" convention, which is the guess that reads as a dead key.
// ⚠️ The body is `()`, NOT empty -- an empty rule body panics at syntactic.rs:591.
fn t990() -> () {
    ()
}

// t1200 `mlir::detail::PassOptions::Option<bool>` -> AN OPAQUE UNIT.  10 gating TUs.
//   The bool instantiation of MLIR's command-line-backed pass option
//   (PassOptions.h:192:9): an `llvm::cl::opt<bool>` plus MLIR's `OptionBase`
//   bookkeeping.  ⛔ NOT `bool`, and that is the WHOLE POINT of this row -- the value
//   comes from argv parsing inside `llvm::cl`, which this port does not translate, so a
//   `bool` model would hand back a DEFAULT-INITIALISED `false` and silently flip
//   dcc-pass-option.h:52's `check_progir{..., llvm::cl::init(true)}`.  A unit has no
//   value to default.  Same model, same argument, same header line as t66
//   (`Option<int>`) / t67 / t68, which have held this shape since they landed.
//   ⛔ COST, censused: `operator=(bool)` at dcc.cpp:141,142,152,154,155 -- five WRITES,
//   no read anywhere in the corpus -- are left UNDECLARED and become loud rustc errors
//   once the downstream `Option<DCC::ProgIRFormat>` gate clears.  None is emitted today.
// ⚠️ IDENTICAL IN BOTH MODELS and spelled in BOTH overlays (the t72/t990 precedent).
// ⚠️ The body is `()`, NOT empty -- an empty rule body panics at syntactic.rs:591.
fn t1200() -> () {
    ()
}

// t1201 `mlir::Pass::Option<std::string>` -> AN OPAQUE UNIT.  2 gating TUs.
//   Pass.h:93:10 -- a REAL derived struct over `detail::PassOptions::Option`, not an
//   alias, which is why it needs its own key (the recorded key names the DECLARING
//   class).  Same model and same refusal as t1200/t66.
//   ⛔ COST, censused: three sites, all in dbo/src/Transforms/EmitSpyreCode.cpp -- :78
//   `this->exportDir = export_dir.str()` (operator=), :85 `exportDir.empty()`, :99
//   `std::string(exportDir)` (operator DataType) -- left UNDECLARED.  None is emitted
//   today: the TU still aborts LOUDLY one step later, on `llvm::cl::initializer<char[_]>`
//   inside `EmitSpyreCodePassBase::EmitSpyreCodePassBase`, which is the type of the
//   `cl::init("")` argument and therefore PROOF that the converter does lower the
//   variadic ctor's init argument.  The `char[_]` payload is the standing rules/cl `t3`
//   refusal and is NOT keyed here.
// ⚠️ IDENTICAL IN BOTH MODELS and spelled in BOTH overlays.
fn t1201() -> () {
    ()
}

// ===========================================================================
// PASS 2026-09-29: THE `mlir::OperandRange` ITERATOR FAMILY.
// t1100/t1101 (the two TYPES) and f1100-f1106 (the SEVEN expressions), landed as
// ONE SET.  See src.cpp for the row, the five TUs, the verbatim abort and the three
// measured corrections (`getOperands()` needs no key; `end()` does; `operand_end()`
// has zero log lines and is NOT invented).
//
// THE MODEL: `libcc2rs::RangeIter<T>` (libcc2rs/src/iterators.rs:929) -- a
// `{Vec<T>, usize}` OWNING cursor.  C++'s iterator is a borrowed
// `{BaseT base, ptrdiff_t index}` pair; this owns its sequence instead.
//
// ⚠️ WHAT OWNING COSTS, stated rather than assumed.  `:521` and `:526` make TWO
// SEPARATE `getOperands()` calls, so the two iterators of each pair own DISTINCT
// CLONES of the operand list.  Under t14's settled snapshot semantics
// (`mlir::OperandRange` -> `Vec<ir::Value>`, and `ir::Value` equality is
// STRUCTURAL) that is correct: the pair still denotes the same subrange.  It does
// mean a genuinely mismatched iterator pair -- UB in C++ -- yields a plausible wrong
// value here instead of trapping, so `RangeIter::range_from` carries a
// `debug_assert_eq!` on the two sequences.  It is a debug assert because the scan is
// O(n) on a hot printing path and because under owning semantics a mismatch is a
// wrong VALUE, not memory unsafety.
// ===========================================================================

// t1100 -- `<base>::iterator`, and t1101 -- its CRTP base
// `llvm::iterator_facade_base<...>`.  ⭐ BOTH map to the SAME Rust type and that is
// ONE claim, not two: the facade base IS the iterator (CRTP), exactly as
// `indexed_accessor_range_base` IS the range for t37-t39 / t166 / t251.  Element
// model is `ir::Value`, matching t14, so nothing new is claimed about elements.
// ⚠️ Plain `fn`, not `unsafe fn`: the t37-t39 / t166 convention for a TYPE rule.
// ⚠️ Spelled in BOTH overlays (the t72 `mlir::TypeID` / t990 precedent) rather than
// relying on the unsafe-layer union, which is the guess that reads as a dead key.
fn t1100() -> libcc2rs::RangeIter<dataflowir_gen::ir::Value> {
    Default::default()
}

fn t1101() -> libcc2rs::RangeIter<dataflowir_gen::ir::Value> {
    Default::default()
}

// f1100 / f1101 -- `begin()` / `end()`, recorded on the DECLARING base class.
// The receiver arrives as the already-mapped `Vec<ir::Value>` (t14 / t37) and is
// MOVED into the cursor, which is what makes the cursor own its sequence.
unsafe fn f1100(
    a0: Vec<dataflowir_gen::ir::Value>,
) -> libcc2rs::RangeIter<dataflowir_gen::ir::Value> {
    libcc2rs::RangeIter::begin(a0)
}

unsafe fn f1101(
    a0: Vec<dataflowir_gen::ir::Value>,
) -> libcc2rs::RangeIter<dataflowir_gen::ir::Value> {
    libcc2rs::RangeIter::end(a0)
}

// f1102 -- `it + n` (`llvm::iterator_facade_base::operator+`), the operator the five
// TUs aborted on -- and f1103, `std::next(it, n)`.  SAME BODY, deliberately: `next`
// is specified as `advance(i, n); return i;` and `advance` on a random-access
// category is `i += n`.  Two keys because the corpus spells both and a recorded key
// is a spelling; one body because they are one operation.
// `RangeIter::offset` bounds-checks both directions and panics on the C++ UB cases
// (advance past end, move before begin) rather than fabricating a position.
unsafe fn f1102(
    a0: libcc2rs::RangeIter<dataflowir_gen::ir::Value>,
    a1: i64,
) -> libcc2rs::RangeIter<dataflowir_gen::ir::Value> {
    a0.offset(a1)
}

unsafe fn f1103(
    a0: libcc2rs::RangeIter<dataflowir_gen::ir::Value>,
    a1: i64,
) -> libcc2rs::RangeIter<dataflowir_gen::ir::Value> {
    a0.offset(a1)
}

// f1104 -- `*it`.  This family's `ReferenceT` is `mlir::Value` BY VALUE (t37's fifth
// template argument), because an MLIR `Value` is itself a handle, so the faithful
// body CLONES the element rather than borrowing it.  `at()` panics on an end
// iterator, which is the C++ UB, instead of returning a fabricated element.
unsafe fn f1104(
    a0: libcc2rs::RangeIter<dataflowir_gen::ir::Value>,
) -> dataflowir_gen::ir::Value {
    a0.at().clone()
}

// f1105 -- `mlir::OperandRange{first, last}`, the inherited two-iterator
// constructor.  Returns the CONSTRUCTED type's model (`Vec<ir::Value>` = t14), the
// f142 `RegionRange` constructor precedent.  `range_from` takes `first` as the
// authoritative owner of the sequence and slices `[first.idx .. last.idx]`; it
// asserts the pair is ordered and in range.
unsafe fn f1105(
    a0: libcc2rs::RangeIter<dataflowir_gen::ir::Value>,
    a1: libcc2rs::RangeIter<dataflowir_gen::ir::Value>,
) -> Vec<dataflowir_gen::ir::Value> {
    libcc2rs::RangeIter::range_from(a0, a1)
}

// f1106 -- `mlir::Operation::operand_begin()`, the bottom of the chain: this is where
// the operand sequence is actually produced, because `getOperands()` inlines down to
// it.  FIRST member of `mlir::Operation` ever keyed in this module.
//
// ⛔⛔ `ordered_operands()`, NEVER `operands.values()`.  `fmt::OpInst::operands` is a
// `BTreeMap` keyed by operand NAME, so iterating it is ALPHABETICAL: fmt.rs:925
// records that for `(ins Index:$z, Variadic<Index>:$a)` the map yields `a` first
// while ODS position 0 is `z`.  A `operands.values().flatten()` body would make the
// generated `getSources()` / `getTargets()` partition of `{begin(), begin()+1}` /
// `{begin()+1, end()}` pick the WRONG OPERAND, silently, at rc=0 -- strictly worse
// than the abort this row removes.  `OpInst::ordered_operands() -> Vec<&Value>`
// (fmt.rs:873) is public and yields ODS DECLARATION order, which is the order
// `operand_begin()` must agree with.
unsafe fn f1106(
    a0: *mut dataflowir_gen::fmt::OpInst,
) -> libcc2rs::RangeIter<dataflowir_gen::ir::Value> {
    libcc2rs::RangeIter::begin((*a0).ordered_operands().into_iter().cloned().collect())
}

// f1300 `void mlir::Operation::walk((lambda at <path>:_:_) &&)` ->
//   `fmt::OpInst::walk_any_mut` (fmt.rs:1299, dataflowir-gen f32f967).  The
//   UNFILTERED form, which is exact for this key: all four corpus sites that ask
//   it take `mlir::Operation *` (see src.cpp for the four-declaring-class census).
// ⭐ THE BOUND IS THE SHAPE THE CONVERTER ALREADY EMITS, MEASURED, not the shape
//   the crate signature wants.  fresh39's emitted text at Liveness.cpp:64 is
//       let mut _callback: _ = (|op: *mut dataflowir_gen::fmt::OpInst| { ... });
//       (*op).walk(&mut _callback)
//   so the closure is `FnMut(*mut OpInst)`, NOT `FnMut(&OpInst)`.  The adapter
//   below bridges them with `::core::ptr::from_mut` on a LIVE `&mut OpInst`.
// ⭐ THAT IS NOT A CLONE, and the distinction is the whole reason this half is
//   landable in a rule at all: `walk_any_mut` hands the callback `&mut OpInst`, so
//   the pointer names the node IN the tree and a mutating body's writes land in the
//   tree.  A by-VALUE `OpInst` adapter would have had to deep-copy the subtree and
//   discard every write -- which is why the crate slot refused to bend the
//   signature, and why `walk_any_mut` rather than `walk_any` is used here even for
//   read-only bodies (`walk_any` yields `&OpInst`, from which no `*mut` can be
//   made without UB).
// ⚠️ `&mut` IS WRITTEN ON A LOCAL TEMPORARY CLOSURE, never on `a0` or `a1`: the
//   "never write the `&mut` yourself" rule is about the RECEIVER and the rule
//   arguments, both of which arrive already in their model's shape.
unsafe fn f1300<T1: FnMut(*mut dataflowir_gen::fmt::OpInst)>(
    a0: *mut dataflowir_gen::fmt::OpInst,
    mut a1: T1,
) -> () {
    (*a0).walk_any_mut(&mut |o: &mut dataflowir_gen::fmt::OpInst| {
        a1(::core::ptr::from_mut(o))
    })
}

// t1500 `mlir::detail::PassOptions::Option<DCC::ProgIRFormat>` -> AN OPAQUE UNIT.
//   13 gating TUs across dcc/dbo/hcc/dvs/dsm/dr5; the gate standing in FRONT of
//   t1200's own win.  The PROJECT-ENUM instantiation of MLIR's command-line-backed
//   pass option (PassOptions.h:192:9), parsed by `GenericOptionParser` rather than
//   `llvm::cl::parser` because `cl::parser<E>` for an enum derives from
//   `generic_parser_base`.
//   ⛔ NOT an enum, and the reason is sharper here than for t66/t1200:
//   `dcc-pass-option.h:98` is `llvm::cl::init(DCC::ProgIRFormat::kGeneral)` and
//   `kGeneral == 0` (dcc.hpp:58), so a value model's `Default::default()` would
//   COINCIDE with the compiled-in default and the wrongness would be INVISIBLE in
//   this corpus -- while `--progir-format=smc` would still silently read `kGeneral`,
//   because the value comes from argv parsing inside `llvm::cl`, which this port does
//   not translate.  A unit has no value to default.  ⭐ NO VALUE IS MODELLED.
//   ⛔ COST, censused over the whole corpus: exactly ONE read exists --
//   `dcc-standalone-main.cpp:757` `options.progir_format` through the inherited
//   `operator DataType` -- and it is MEASURED NOT EMITTED (that TU aborts first on
//   `mlir::tracing::DebugConfig`).  No `.getValue()`, no `hasValue()`, no `operator=`
//   on this instantiation anywhere.  All left UNDECLARED and loud.
//   ⛔ The sibling `mlir::Pass::Option<DCC::ProgIRFormat>` (g344) is deliberately NOT
//   keyed: its value decides control flow at SentientToProgIR.cpp:716/:722.
// ⚠️ IDENTICAL IN BOTH MODELS and spelled in BOTH overlays (the t72/t990/t1200
// precedent).  The body is `()`, NOT empty -- an empty rule body panics at
// syntactic.rs:591.
fn t1500() -> () {
    ()
}

// t2600 `llvm::cl::initializer<DCC::ProgIRFormat>` -> AN OPAQUE UNIT.
//   Row g063: the measured first abort of 42 of 42 TUs once `cl::OptionEnumValue`
//   (rules/cl t2410) cleared the gate in front of it.  The carrier that
//   `cl::init(DCC::ProgIRFormat::kGeneral)` builds is consumed ONLY by the variadic
//   ctor of the option -- and MEASURED on the emitted Rust, that consumer is the
//   2-ARY `Option<DCC::ProgIRFormat, GenericOptionParser<DCC::ProgIRFormat>>`, which
//   has NO key (t1500 above is the 1-ary spelling) and is emitted as a FABRICATED
//   `::new_1` on an undefined name.  ⭐ So the payload's only consumer is itself an
//   undefined name: nothing can observe it, and no `fN` can deliver it.
//   ⛔ NOT the t700/t701 payload model: those map `<bool>`/`<int>` to `bool`/`i32`
//   because a rules target can NAME those Rust types.  `DCC::ProgIRFormat` is a
//   PROJECT enum the converter ports under its own name, which a system rules module
//   cannot spell, so a payload model here would be `i32` standing in for a ported
//   enum -- the wrong type, silently.  ⭐ NO VALUE IS MODELLED.
//   ⛔ NO MEMBER DECLARED (`Init` is the only one and is never read by name on any
//   `cl::initializer` receiver in the corpus), and NO `fN` for
//   `cl::init<DCC::ProgIRFormat>`: an unkeyed `cl::init` is the marked placeholder
//   `Cpp2RustUnmappedFn_init_<N>`, not an abort, so leaving it unkeyed keeps the
//   unmodelled payload VISIBLE.  See src.cpp.
// ⚠️ IDENTICAL IN BOTH MODELS and spelled in BOTH overlays, exactly as t1500 is.
fn t2600() -> () {
    ()
}

// ===========================================================================
// ⭐⭐ THE RESULT HALF OF THE RANGE-ITERATOR FAMILY -- queue row g2958 (`next`)
// t1700/t1701 + f1700-f1706, the one-for-one mirror of t1100/t1101 + f1100-f1106.
// See src.cpp for the row census (the 69 `_N` suffixes are TWO families: 14,810
// operand sites = f1103, already landed; 14,810 result sites = f1703, here) and for
// the seven recorded key spellings.
// ===========================================================================

// t1700 / t1701 -- the iterator and its CRTP base, BOTH `RangeIter<ir::Value>`.
// One claim, not two: `iterator_facade_base` IS the iterator's base, so the base is
// the iterator.  The element is `ir::Value` because t15 (`mlir::ResultRange`), t38
// (its `indexed_accessor_range_base`) and t24 (`mlir::OpResult`) already map there.
fn t1700() -> libcc2rs::RangeIter<dataflowir_gen::ir::Value> {
    Default::default()
}

fn t1701() -> libcc2rs::RangeIter<dataflowir_gen::ir::Value> {
    Default::default()
}

// f1700 / f1701 -- `begin()` / `end()`, recorded on the DECLARING base class.  The
// receiver arrives as the already-mapped `Vec<ir::Value>` (t15 / t38) and is MOVED
// into the cursor, which is what makes the cursor own its sequence.
unsafe fn f1700(
    a0: Vec<dataflowir_gen::ir::Value>,
) -> libcc2rs::RangeIter<dataflowir_gen::ir::Value> {
    libcc2rs::RangeIter::begin(a0)
}

unsafe fn f1701(
    a0: Vec<dataflowir_gen::ir::Value>,
) -> libcc2rs::RangeIter<dataflowir_gen::ir::Value> {
    libcc2rs::RangeIter::end(a0)
}

// f1702 -- `it + n` (`llvm::iterator_facade_base::operator+`) -- and f1703,
// `std::next(it, n)`.  SAME BODY, deliberately, the f1102/f1103 reason verbatim:
// `next` is specified as `advance(i, n); return i;` and `advance` on a
// random-access category is `i += n`.  Two keys because the corpus spells both and
// a recorded key is a spelling; one body because they are one operation.
// `RangeIter::offset` bounds-checks both directions and panics on the C++ UB cases
// (advance past end, move before begin) rather than fabricating a position.
unsafe fn f1702(
    a0: libcc2rs::RangeIter<dataflowir_gen::ir::Value>,
    a1: i64,
) -> libcc2rs::RangeIter<dataflowir_gen::ir::Value> {
    a0.offset(a1)
}

unsafe fn f1703(
    a0: libcc2rs::RangeIter<dataflowir_gen::ir::Value>,
    a1: i64,
) -> libcc2rs::RangeIter<dataflowir_gen::ir::Value> {
    a0.offset(a1)
}

// f1704 -- `*it`.  This family's `ReferenceT` is `mlir::OpResult` BY VALUE (t38's
// fifth template argument), because an MLIR `OpResult` is itself a handle, so the
// faithful body CLONES the element rather than borrowing it.  `at()` panics on an
// end iterator, which is the C++ UB, instead of returning a fabricated element.
unsafe fn f1704(
    a0: libcc2rs::RangeIter<dataflowir_gen::ir::Value>,
) -> dataflowir_gen::ir::Value {
    a0.at().clone()
}

// f1705 -- `mlir::ResultRange{first, last}`, the inherited two-iterator
// constructor.  Returns the CONSTRUCTED type's model (`Vec<ir::Value>` = t15), the
// f1105 / f142 precedent.  `range_from` takes `first` as the authoritative owner of
// the sequence and slices `[first.idx .. last.idx]`; it asserts the pair is ordered
// and in range.
unsafe fn f1705(
    a0: libcc2rs::RangeIter<dataflowir_gen::ir::Value>,
    a1: libcc2rs::RangeIter<dataflowir_gen::ir::Value>,
) -> Vec<dataflowir_gen::ir::Value> {
    libcc2rs::RangeIter::range_from(a0, a1)
}

// f1706 -- `mlir::Operation::result_begin()`, the bottom of the chain and the ONLY
// body in this block that produces a sequence: `getResults()` inlines down to it,
// exactly as `getOperands()` inlines to `operand_begin()` (f1106).
// ⭐ THE ALPHABETICAL-ORDER HAZARD DOES NOT EXIST ON THIS SIDE, and that is a
//   property of the crate, not an assumption.  f1106 had to use
//   `ordered_operands()` because `fmt::OpInst::operands` is a `BTreeMap` keyed by
//   operand NAME, so `.values()` is alphabetical.  `fmt::OpInst::results`
//   (fmt.rs:436, dataflowir-gen f32f967) is a plain `pub Vec<Value>` -- an ordered
//   sequence with no key and therefore no second order to get wrong.  It is cloned
//   because the C++ returns a borrowed view over results the operation still owns,
//   and the cursor here owns its sequence.
unsafe fn f1706(
    a0: *mut dataflowir_gen::fmt::OpInst,
) -> libcc2rs::RangeIter<dataflowir_gen::ir::Value> {
    libcc2rs::RangeIter::begin((*a0).results.clone())
}

// ===========================================================================
// PASS 2026-09-29: `mlir::WalkResult` -- t1710 + f1710-f1713, and
// `mlir::Region::walk` (f1800).  See rules/mlir/src.cpp for the census that
// decided this row is FIVE WalkResult keys and not six.
//
// ⭐⭐ `LocWalkResult` NEEDED NOTHING ADDED.  All four members already exist and
// are `pub` in dataflowir-gen (ir.rs:804 the enum; :813 `advance`, :818
// `interrupt`, :823 `skip`, :833 `was_interrupted`), and the derived
// `PartialEq`/`Eq` already covers the corpus's `==`/`!=` against a result.  The
// only thing this row needed from the crate is `Region::walk_any_mut`.
//
// ⭐ THIS BLOCK IS IDENTICAL TEXT IN BOTH OVERLAYS (modulo `unsafe fn`).
// `LocWalkResult` is a plain `Copy` enum -- a VALUE, not a handle into an owner --
// so no `Ptr<...>` / `*mut` adapter arises for it and there is nothing for the two
// memory models to disagree about.  f1800's `&mut fmt::Region` receiver is
// likewise identical in both, the f460 (`getBlocks`) precedent verbatim.
// ===========================================================================

// t1710 `mlir::WalkResult` -> `ir::LocWalkResult`.
// ⭐⭐ THE INIT IS `Advance`.  MLIR's `WalkResult(ResultEnum result = Advance)`
// (Visitors.h:33) makes a default-initialised WalkResult mean "keep going".
// ⛔ NOT `Default::default()` -- that would make the init an accident of whatever
// `#[derive(Default)]` or the first variant happens to be, and it is not derived
// on this enum anyway.  ⛔ NOT `Interrupt` -- that would silently turn every
// default-init into an early exit: the walk stops at the first node and the
// converted program still compiles and still runs.
fn t1710() -> dataflowir_gen::ir::LocWalkResult {
    dataflowir_gen::ir::LocWalkResult::Advance
}

// f1710 -- `static WalkResult mlir::WalkResult::advance()`, 60 sites.
fn f1710() -> dataflowir_gen::ir::LocWalkResult {
    dataflowir_gen::ir::LocWalkResult::advance()
}

// f1711 -- `static WalkResult mlir::WalkResult::interrupt()`, 24 sites.
fn f1711() -> dataflowir_gen::ir::LocWalkResult {
    dataflowir_gen::ir::LocWalkResult::interrupt()
}

// f1712 -- `static WalkResult mlir::WalkResult::skip()`, 36 sites.
// ⭐ `Skip` is a REAL THIRD STATE, not an alias for `Advance`: `walk_any_r_mut`
// prunes the node's children on it and then continues.  `tests/walk.rs` proves the
// pruning NON-VACUOUSLY -- it asserts the pruned traversal is a strict subsequence
// of the full one behind a `pruned.len() > 100` anti-vacuity gate, so a
// `Skip`-means-`Advance` regression fails the test rather than passing it trivially.
fn f1712() -> dataflowir_gen::ir::LocWalkResult {
    dataflowir_gen::ir::LocWalkResult::skip()
}

// f1713 -- `bool mlir::WalkResult::wasInterrupted() const`, 13 sites.
// ⚠️ BY-VALUE FORMAL in src.cpp (the f1 precedent) is what records the `const`
// member's key; the Rust side takes it by value too because `LocWalkResult` is
// `Copy`.
// ⛔ `Skip` IS NOT AN INTERRUPT: a walk that ended on a pruned subtree COMPLETED,
// so this is false for `Skip` exactly as it is for `Advance`.  The crate method is
// `matches!(self, LocWalkResult::Interrupt)` -- it does NOT collapse the two
// "not advance" cases, which would turn every prune into an early exit at the
// caller's `if (...wasInterrupted())`.
fn f1713(a0: dataflowir_gen::ir::LocWalkResult) -> bool {
    a0.was_interrupted()
}

// f1800 -- `void mlir::Region::walk(T1 &&)`.  The f1300 shape with a REGION
// receiver; real site `dcc/src/Transform/Sentient/Analyses/Liveness.cpp:75`, which
// before this key emitted the SILENT TEXTUAL `(*region).walk(&mut _callback)`.
// ⛔ NEVER A HAND-ROLLED TRAVERSAL.  A two-level `for b in
// (*region).get_blocks_mut() { for o in b.get_operations_mut() { ... } }` body
// converts at rc=0 and is SILENTLY WRONG: it visits the TOP-LEVEL ops of each
// block only, with NO DESCENT into nested regions.  `fmt::Region::walk_any_mut`
// (fmt.rs:784) is pre-order over the whole subtree, and `tests/walk.rs` proves the
// order by SEQUENCE EQUALITY OF NODE ADDRESSES for every region at every depth
// (328 regions in one reference file, 752 in another) plus a hand-built
// three-block region -- the reference files are 100% single-block, so that
// hand-built case is the only thing that can catch a reverse-block-order or
// entry()-only bug.
// ⭐ The closure adapter is f1300's verbatim: the converter hands us a callback
// over the overlay's own op representation, and `walk_any_mut` wants
// `&mut impl FnMut(&mut OpInst)`, so the one line here is the representation
// bridge and nothing else.
unsafe fn f1800<T1: FnMut(*mut dataflowir_gen::fmt::OpInst)>(
    a0: &mut dataflowir_gen::fmt::Region,
    mut a1: T1,
) -> () {
    a0.walk_any_mut(&mut |o: &mut dataflowir_gen::fmt::OpInst| {
        a1(::core::ptr::from_mut(o))
    })
}

// ===========================================================================
// opname slot, 2026-09-29 -- f2100/f2101/f2102.  Index band f2100+ is disjoint
// from the `oppass` slot's f2000+ tail append (see src.cpp's tail banner).
// ===========================================================================

// f2100 -- `llvm::StringRef mlir::OperationName::getStringRef() const`.
// 200 of 312 TUs, 6,052 sites: THE #1 SILENT-MEMBER ROW (queue g2977).
// ⛔ THIS WAS NEVER AN ABORT, which is why no bucket census, placeholder census or
// first-abort ranking could see it.  `mlir::OperationName` is keyed (t18), so the
// converter emitted `name.getStringRef()` TEXTUALLY: the TU stayed bucket A, rc=0,
// rustfmt-clean, and failed only under real rustc as E0599.  Exactly the
// `std::hash<int>` shape measured at 427 sites across 190 TUs -- a type key with no
// method key is strictly worse than no key, because it bypasses the loud path.
//
// ⭐ THE BODY IS THE DIALECT-QUALIFIED NAME, NOT THE MNEMONIC.  C++
// `getStringRef()` is `return getIdentifier();` (OperationSupport.h:473) -- the
// fully-qualified name, `dataflow.program_unit`.  The crate says the same thing in
// its own words: isa.rs:78 on `op_mnemonic` reads "NOT an identity on its own:
// `arith.constant` and `func.constant`" share it, and isa.rs:86 composes exactly
// this pair, documented as "the printed op name, what C++ `getOperationName()`
// returns".  `row_dialect` (lib.rs:154) is "the printed dialect prefix of a row ...
// from its ODS base class"; it takes `&TdOpDef` and is re-exported at the crate
// root, so the receiver needs no crate change.
//
// ⛔ THE `None` ARM IS A LOUD PANIC, NEVER AN EMPTY STRING.  `mlir::OperationName`
// is not default-constructible and every value in the corpus comes from `lookup()`
// / `getRegisteredInfo()` / `getRegisteredOperations()`, so reading the name out of
// a null handle is C++ UB.  The `panic!("ub: ...")` idiom is f670's and
// `rules/equivalenceclasses`'.  A fabricated `""` would satisfy rustc and lie.
//
// The `Vec<libc::c_char>` + trailing NUL is f670's verbatim: `rules/stringref` t1
// maps `llvm::StringRef` to `Vec<libc::c_char>`.
fn f2100(a0: Option<dataflowir_gen::TdOpDef>) -> Vec<libc::c_char> {
    match a0 {
        Some(ref __d) => {
            let __s = format!("{}.{}", dataflowir_gen::row_dialect(__d), __d.mnemonic);
            let mut __v: Vec<libc::c_char> = __s.bytes().map(|b| b as libc::c_char).collect();
            __v.push(0);
            __v
        }
        None => panic!(
            "ub: mlir::OperationName::getStringRef() on a null op-name handle"
        ),
    }
}

// f2101 -- `bool mlir::OperationName::isRegistered() const`.
// 199 of 312 TUs, 5,962 sites: the #2 silent-member row, SAME receiver variable and
// SAME mapped type as f2100 (2,001 of 2,218 sampled `.getStringRef(` receivers were
// `name: Option<dataflowir_gen::TdOpDef>`), so one declaring class closes both.
//
// ⭐⭐ WHY THIS IS HONEST AND NOT A GUESS, which is the whole bar for this key.
// C++ `isRegistered()` is `typeID != TypeID::get<void>()` (OperationSupport.h:158)
// -- "is there a registry record behind this handle".  t18's model is
// `Option<TdOpDef>` and t18's OWN comment already states that `None` is the null
// handle and the unregistered name, `Some` a `TD_OPS` registry row.  So
// `is_some()` is not an approximation of the C++ predicate -- it is the same
// predicate read off the same discriminant, and `TdOpDef` rows only ever come from
// the generated table, so a `Some` cannot be an unregistered op.
// ⛔ A HARDCODED `-> true` WAS THE NAMED FORBIDDEN OUTCOME for this key and would
// have forced it to be left out.  It is not needed: the discriminant answers.
fn f2101(a0: Option<dataflowir_gen::TdOpDef>) -> bool {
    a0.is_some()
}

// f2102 -- `bool mlir::operator==(mlir::OpState, mlir::OpState)`, the first abort
// on 10 TUs (hidden behind `rules/functional` f17-f22 until they landed).
//
// ⭐⭐ IDENTITY, NOT STRUCTURAL EQUALITY, AND THE MODEL CAN EXPRESS IT.  t25 maps
// `mlir::OpState` to `fmt::OpInst`, and `OpInst` MODELS PRINTED CONTENT, so a
// derived/structural `PartialEq` would report two DISTINCT operations with the same
// operands and attributes as EQUAL -- in a pass that dedups or caches ops that is a
// silent wrong answer, not a cosmetic one.  fmt.rs:386-401 says this in the crate's
// own words ("THE IDENTITY OF ONE OPERATION, which is not the same thing as its
// contents ... a worklist that dedups an `OpInst` by value silently drops a
// legitimate repeat visit") AND supplies the fix: `OpInst.id: OpId`, reachable as
// `OpInst::op_id()` (fmt.rs:892), stamped fresh per insertion by
// `build.rs:708 inst.id = OpId::fresh();`.  `OpId` is `PartialEq`.  So this body
// compares `op_id()`, i.e. the modelled `Operation *`, and touches NO content field.
// ⛔ The in-tree note on `class OpState` in src.cpp previously refused every
// comparison outright; its premise ("`OpInst` is a VALUE") is true of the CONTENT
// fields and incomplete -- it predates/overlooks the `id` field.  The note is
// updated in place rather than left contradicting this key.
//
// ⭐ THE ONE CASE THE MODEL CANNOT ANSWER IS LOUD, NOT GUESSED.  `OpId::NONE` is
// documented as "the only id that is not unique, and never equal to a stamped one"
// -- it is what `OpInst::new` gives an instance no builder inserted (the printer's
// round-trip corpus).  So:
//   * stamped vs stamped -> exact identity, both directions correct.
//   * stamped vs NONE    -> correctly UNEQUAL; an inserted op and a never-inserted
//                           one are genuinely two operations.
//   * NONE vs NONE       -> THE MODEL HOLDS NO IDENTITY FOR EITHER SIDE.  A bare
//                           `x == y` here returns `true` and is a FALSE EQUAL --
//                           the exact silent-wrong-answer direction a structural
//                           compare fails in.  It PANICS instead.
// A conservative `!x.is_none() && x == y` was rejected: it never false-EQUALs, but
// it silently false-UNEQUALs two handles on the SAME un-inserted op, so an
// `if (op == target) erase` site would just never fire -- still a lie that compiles.
// The panic is the only branch that neither fabricates an answer nor hides.
fn f2102(
    a0: dataflowir_gen::fmt::OpInst,
    a1: dataflowir_gen::fmt::OpInst,
) -> bool {
    let __x = a0.op_id();
    let __y = a1.op_id();
    if __x.is_none() && __y.is_none() {
        panic!(
            "mlir::operator==(OpState, OpState): both handles carry OpId::NONE, so the model holds no identity for either side and equality cannot be decided without comparing printed content"
        );
    }
    __x == __y
}

// f2103 -- `bool mlir::operator!=(mlir::OpState, mlir::OpState)`, the NEGATION of
// f2102 and a SEPARATE KEY BECAUSE THE CORPUS ASKS FOR IT SEPARATELY:
// `.../StageCoarsening/ScopeCorrection.cpp` aborts on `!= on (mlir::OpState,
// mlir::OpState)` verbatim, and pre-C++20 there is no rewriting from `==`, so
// without this key that TU stays gated no matter what f2102 does.
// ⛔ THE BODY IS NOT `!f2102(...)` SPELT OUT -- it repeats the identity test and the
// same loud NONE-vs-NONE refusal, because a rule body is INLINED into the translated
// crate and may not depend on another key being in scope there (the f360-f366
// reason).  Read f2102's note above for why this compares `op_id()` and never
// content, and for why two `OpId::NONE` handles panic instead of answering.
fn f2103(
    a0: dataflowir_gen::fmt::OpInst,
    a1: dataflowir_gen::fmt::OpInst,
) -> bool {
    let __x = a0.op_id();
    let __y = a1.op_id();
    if __x.is_none() && __y.is_none() {
        panic!(
            "mlir::operator!=(OpState, OpState): both handles carry OpId::NONE, so the model holds no identity for either side and inequality cannot be decided without comparing printed content"
        );
    }
    __x != __y
}

// t2300 -- `mlir::detail::PassOptions::Option<std::string>` -> THE `std::string`
// MODEL ITSELF, i.e. `rules/string` t1 = a NUL-TERMINATED `Vec<libc::c_char>`.
// ⭐ NOT a new `libcc2rs` type.  The Rust stage type-checks rule targets against the
// PREBUILT `liblibcc2rs-*.rmeta` in pin/target_preprocessor, so a
// `libcc2rs::PassOptionString` added in a worktree is INVISIBLE (E0425, then a core
// dump with no OK line).
// ⛔⛔ AND IT IS NOT `String` EITHER, WHICH IS WHAT THIS KEY SHIPPED AS ON THE
// slot-optstr2 BRANCH.  MEASURED.  `Option<std::string>` IS-A `std::string` by
// inheritance (`opt_storage : public DataType`, CommandLine.h:1354) and the real
// PassOptions.h:208 re-exposes the base assignment with `using llvm::cl::opt<...>::
// operator=`, so the two member keys that serve it -- `rules/cl` f1910 `getValue()`
// and f1911 `operator=`, the PAIRED HALF of this block -- are generic on `T1` and
// instantiate with `T1 = std::string`, which the converter resolves through
// `rules/string` t1 to `Vec<libc::c_char>`.  A `String` target makes every one of those
// sites a `Vec<libc::c_char>` <-> `String` mismatch: `opts.outputPath.getValue()`
// yields `&mut String` where a `std::string` is wanted and
// `progIROpt.artifacts_outfile = s` passes a `Vec<libc::c_char>` into a `&mut String`
// -- `error[E0308]` on BOTH, measured on ir/escrowfix.probe.cpp.
// ⭐ The two keys are only consistent when this one spells the SAME value model as
// `rules/string` t1, which is also the only spelling the header's own IS-A statement
// supports.
fn t2300() -> Vec<libc::c_char> {
    Vec::new()
}

// t2301 -- `mlir::detail::PassOptions` -> an opaque unit; see the src note.
fn t2301() -> () {
    ()
}

// f2300 -- the constructor.  a0 (the parent `PassOptions`) and a2 (`cl::desc`, help
// text) are DISCARDED; a1 is the option NAME, which the value model does not carry.
// a3 is `cl::init("...")` -- THE COMPILED-IN DEFAULT -- and it is the only argument
// that becomes the payload.
// ⭐ a3 IS ALREADY THE PAYLOAD, RE-TYPED, AND THE NUL IS KEPT.  `rules/cl` t1900
// models `cl::initializer<char[N]>` as the `char[N]` itself (`Vec<u8>`), and a
// `char[N]` built from a string literal carries its terminator as element N-1 --
// which is exactly the NUL-TERMINATED invariant `rules/string` t1 requires and that
// `f2` (`len() - 1`) and `f46..f48` there read off.  So the body is an elementwise
// `u8 -> libc::c_char` re-type and nothing else: no NUL is added (it is already
// there) and none is stripped.  ⛔ THE OLD BODY WAS `String::from_utf8_lossy(&a3)
// .trim_end_matches('\0').to_owned()`, which both produced the wrong type AND dropped
// the terminator, so even a `String` consumer would have been handed a value one
// element short of the `std::string` model.
// ⚠️ a3 IS MENTIONED EXACTLY ONCE.  The converter substitutes a parameter name with
// the argument TEXT, so a second mention duplicates that expression -- here the
// unresolved `unsafe { Cpp2RustUnmappedFn_init_1(&[0 as libc::c_char; 1]) }` -- and
// would report its `E0425` twice.  A NUL-termination `assert!` was written and
// dropped for that reason; the invariant is a property of `char[N]`, not of the site.
unsafe fn f2300(
    a0: &mut (),
    a1: Vec<libc::c_char>,
    a2: Vec<libc::c_char>,
    a3: Vec<u8>,
) -> Vec<libc::c_char> {
    let _ = a0;
    let _ = a1;
    let _ = a2;
    a3.into_iter().map(|__b| __b as libc::c_char).collect()
}

// t2400 `mlir::AttributeStorage` -> AN OPAQUE UNIT, the representation t23
//   (MLIRContext), t40 (Pass), t43 (OpOperand), t58 (InterfaceMap) and t59
//   (Builder) already use.  MLIR's attribute storage base (AttributeSupport.h:169).
//   First abort of 12 TUs on the matched pair; `grep -rnoF AttributeStorage
//   repos/dt_src` = 0, and the reaching site is the `return nullptr` of
//   Uniform.td:102 `getRegIndicesIfExist`, i.e. the type appears only in the
//   `Attribute(const AttributeStorage *)` conversion-ctor signature.
//   ⛔ NO MEMBER IS MAPPED (`getType`, `getAbstractAttribute`, the StorageUniquer
//   hooks), so any real dereference still aborts loudly.  The body is `()`, not
//   empty: `fn t2400() -> () {}` panics at syntactic.rs:591.
fn t2400() -> () {
    ()
}

// t2401 `mlir::detail::ValueImpl` -> AN OPAQUE UNIT, same representation and same
//   licence.  The base behind every `mlir::Value` (Value.h:40); t4 already models
//   `Value` itself, and this is only its private `impl` pointee.  First abort of 3
//   TUs; `mlir::detail::ValueImpl` in repos/dt_src = 0 occurrences (a bare
//   `ValueImpl` grep returns 11 and all eleven are substring noise:
//   `IsGenericValueImpl`, `evaluateValueImpl`, `getValueImpl`).  Reached through
//   `dcc::CondNode::CondNode` (dcc/src/Analysis/ConditionalTree.hpp:32).
//   ⛔ NO MEMBER IS MAPPED (`getKind`, `getType`, `setType`, use-list traversal),
//   so any real dereference still aborts loudly.
fn t2401() -> () {
    ()
}

// ===========================================================================
// t2410 / f2410 / f2411 -- `mlir::ValueTypeRange<llvm::MutableArrayRef<
// mlir::BlockArgument>>`, `mlir::Block::getArgumentTypes()`, and the `operator[]`.
// ONE ATOMIC SET: the type was LOUD (17 TUs aborted on it) and `getArgumentTypes` was
// SILENT (an unmapped member is emitted textually, rc=0, no placeholder token), so
// landing either alone is worse than landing neither.  See src.cpp for the 17-TU
// census, the 4 Agen.td sites, and the omission list.
//
// ⭐ THE MODEL IS t250's, NOT A NEW ONE.  `Vec<ir::Ty>` is the elementwise lift of t5
// (`mlir::Type` -> `ir::Ty`) exactly as t14/t16 are the lift of t4, and it is the
// shape `dataflowir-gen/src/fmt.rs:579` already declares for this very member.
//
// ⚠️ IDENTICAL IN BOTH MODELS, and that is expected rather than sloppy -- the f500
// argument verbatim: the receiver is a C++ `mlir::Block &`, which is
// `&mut fmt::Block` in both, and the results are by-value `Vec<ir::Ty>` / `ir::Ty` in
// both.  Only the ITERATOR that PRODUCES the receiver differs (t245/f461), and that
// difference is absorbed before these calls.
unsafe fn t2410() -> Vec<dataflowir_gen::ir::Ty> {
    Default::default()
}

// f2410 -- `Block::getArgumentTypes()` -> `fmt::Block::get_argument_types()`.
// ⚠️ NO `clone()` HERE: unlike f500, the witness already returns an OWNED
// `Vec<ir::Ty>` (it clones each small `Ty` enum internally, because the types are a
// field of each `Value` and are not contiguous anywhere, so there is no slice to
// borrow).  The C++ return is by value, so owned is the faithful shape.
// ⚠️ `a0` is named EXACTLY ONCE and carries nothing but the method call, because a
// `&mut` formal's `aN` re-expands to the bare lvalue.
// ⚠️ snake_case ON PURPOSE, the f460/f500 rule: a camelCase target name would let
// `mlir::Block`'s dozens of still-unkeyed members resolve BY ACCIDENT against
// dataflowir-gen and destroy the diagnostic.
unsafe fn f2410(a0: &mut dataflowir_gen::fmt::Block) -> Vec<dataflowir_gen::ir::Ty> {
    a0.get_argument_types()
}

// f2411 -- `ValueTypeRange::operator[](size_t) const` -> index + clone.
// ⚠️ THE `clone()` IS THE C++ SIGNATURE, the f500/f1104 argument: TypeRange.h:152
// returns `Type` BY VALUE, and an MLIR `Type` is itself a handle, so C++ copies the
// handle here too.  `ir::Ty` is a small enum; nothing large is copied.
// ⚠️ `a1 as usize` because the C++ parameter is `size_t` -> `u64` and Rust indexes
// with `usize`; on this target they are the same width and the cast cannot truncate.
// ⛔ OUT OF RANGE PANICS rather than fabricating a `Ty`.  That is FAITHFUL and
// deliberate: TypeRange.h:153 is `assert(index < size() && "invalid index into type
// range")`, so C++ traps here too in a debug build and is UB in a release one -- a
// fabricated `Ty::…` would be the `PassOptions::Option<bool>` mistake.
unsafe fn f2411(a0: Vec<dataflowir_gen::ir::Ty>, a1: u64) -> dataflowir_gen::ir::Ty {
    a0[a1 as usize].clone()
}

// ===========================================================================
// f2500 -- `mlir::WalkResult mlir::OpState::walk((lambda at <path>:_:_) &&)`
//          -> `fmt::OpInst::walk_any_r_mut` (fmt.rs:1446).  Row g3037.
// THE SIXTH KEY OF THE WALK FAMILY: f1300 (`Operation::walk`, void), f1800
// (`Region::walk`, void), t1710 + f1710-f1713 (`WalkResult` and its four members),
// and now the `WalkResult`-returning `OpState::walk`.  See rules/mlir/src.cpp for
// the re-verified return-form split and the corpus census.
//
// ⭐⭐ NOTHING IS ADDED TO `dataflowir-gen` AND NOTHING NEEDED TO BE.
// `OpInst::walk_any_r_mut(&mut self, f: &mut impl FnMut(&mut OpInst) ->
// ir::LocWalkResult) -> ir::LocWalkResult` (fmt.rs:1446) is the exact shape, it is
// `pub`, and it is `walk_pre_mut(&|_| true, f)` -- UNFILTERED PRE-ORDER, which is
// what MLIR's untyped `walk` does.  ⛔ NEVER a hand-rolled traversal: f1800's block
// records that a two-level `get_blocks_mut()`/`get_operations_mut()` body converts
// at rc=0 and visits only the TOP-LEVEL ops, with NO DESCENT into nested regions --
// silently wrong at rc=0, the one outcome the hard rules forbid outright.
//
// ⭐ `walk_any_r_mut` AND NOT `walk_any_mut`, and the difference is semantic and not
// cosmetic: `walk_any_mut` DISCARDS the callback's result (fmt.rs:1438 returns
// `LocWalkResult::Advance` unconditionally), so a callback returning `skip()` or
// `interrupt()` would be IGNORED -- the walk would keep going and keep descending.
// Collector.cpp:143/157 both return `WalkResult::skip()` to PRUNE a subtree, so
// `walk_any_mut` here would silently visit ops the C++ never visits.  That is the
// whole reason this key had to wait for t1710.
//
// ⭐ THE BOUND IS THE SHAPE THE CONVERTER EMITS, f1300's measured finding: in the
// UNSAFE model the callback parameter arrives as `*mut OpInst`, so the bound is
// `FnMut(*mut OpInst) -> LocWalkResult` and the one line of body is the
// representation bridge (`::core::ptr::from_mut` on a LIVE `&mut OpInst`, so a
// mutating body's writes land IN the tree and nothing is deep-copied).
// ⚠️ `&mut` IS WRITTEN ONLY ON A LOCAL TEMPORARY CLOSURE, never on `a0` or `a1`.
// ⭐ The result is RETURNED, not dropped: `wasInterrupted()` (f1713) is a real
// corpus consumer of a walk's result, so swallowing it would be the
// `Skip`-means-`Advance` mistake one level up.
unsafe fn f2500<
    T1: FnMut(*mut dataflowir_gen::fmt::OpInst) -> dataflowir_gen::ir::LocWalkResult,
>(
    a0: &mut dataflowir_gen::fmt::OpInst,
    mut a1: T1,
) -> dataflowir_gen::ir::LocWalkResult {
    a0.walk_any_r_mut(&mut |o: &mut dataflowir_gen::fmt::OpInst| {
        a1(::core::ptr::from_mut(o))
    })
}

// ============================================================================================
// ROW g3064 -- GAP FAMILY F1: the seven dialect-op direct type keys, t2610-t2616.  UNSAFE arm.
//
// Each maps to `fmt::OpInst` carrying THE OP'S OWN generated `DEF`, which is t161's body with the
// concrete op substituted -- the same substitution the t167-t2xx `OneTypedResult` block already
// performs for these very ops as trait arguments.  See src.cpp at t2610 for the DEF verification
// (positive AND negative control, two independent instruments), for the position census that is
// why there is NO `fN` default constructor on any of the seven, and for the MEMBER finding: no
// member of any of the seven is mapped, and unlike t161 that is a NAMED residue rather than a
// closed argument.
//
// The `init` is t25's/t27's/t152's/t157's/t158's/t159's/t160's/t161's, and for their reason: a
// default-constructed ODS op handle is the NULL handle, `fmt::OpInst` has no null, and this
// expression exists only to type-check the type key.
//
// ⛔ t25's PROHIBITION APPLIES UNCHANGED: no `operator==`, no `operator!=`, no identity test.
// ⚠️ THIS ARM IS NOT THE OTHER ARM'S `sed`: it happens to come out byte-identical to the other,
// for the reason t157's arms record -- `fmt::OpInst` is a plain generated value type, it is not
// behind a `Value<T>`/`Ptr<T>` wrapper in either model, and NO `ptr_bindings_` is registered here
// (registering one on the refcount arm is `E0614`).  Each body was reasoned for its own model and
// the identity is the conclusion, not the method.
fn t2610() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_AddIOp as dataflowir_gen::MlirOp>::DEF,
    )
}
fn t2611() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_SubIOp as dataflowir_gen::MlirOp>::DEF,
    )
}
fn t2612() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_CmpIOp as dataflowir_gen::MlirOp>::DEF,
    )
}
fn t2613() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_SelectOp as dataflowir_gen::MlirOp>::DEF,
    )
}
fn t2614() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_affine_AffineIfOp as dataflowir_gen::MlirOp>::DEF,
    )
}
fn t2615() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_affine_AffineYieldOp as dataflowir_gen::MlirOp>::DEF,
    )
}
fn t2616() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_func_ReturnOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// ===========================================================================
// ⭐ ROW g3066 -- `mlir::tracing::DebugConfig`, GAP FAMILY F2's largest gate (6 TUs).
// t2700 + f2700/f2701/f2702 as ONE SET.  See src.cpp for the verbatim abort, the
// five-use census, the TWO-`DebugConfig`-CLASSES correction, the destructor test and
// the g3062 aggregate-init check.
// ===========================================================================

// t2700 `mlir::tracing::DebugConfig` -> AN OPAQUE UNIT.  6 gating TUs.  Every field
//   is populated by `llvm::cl` argv parsing, which this port does not translate, so a
//   value model would hand back default-initialised flags and lie -- the t1200/t1500
//   argument verbatim.  ⭐ NO VALUE IS MODELLED.  No `~DebugConfig` exists and the
//   breakpoint-manager pointers are NON-OWNED, so there is no end-of-scope effect to
//   drop (the axis `OwningOpRef` was refused on).
//   ⛔ COST, censused: MLIR's own seven members (`enableDebuggerActionHook`,
//   `isDebuggerActionHookEnabled`, `logActionsTo`, `getLogActionsTo`,
//   `getProfileActionsTo`, `addLogActionLocFilter`, `getLogActionsLocFilters`) are
//   called at ZERO sites in the corpus and are left UNDECLARED, so any future call
//   aborts loudly rather than answering from an empty model.
// ⚠️ IDENTICAL IN BOTH MODELS and spelled in BOTH overlays (the t72/t990/t1200/t1500
// precedent).  The body is `()`, NOT empty -- an empty rule body panics at
// syntactic.rs:591.
fn t2700() -> () {
    ()
}

// f2700 -- the default constructor for t2700 (`tracing::DebugConfig debugConfig;`,
// 4 sites); the unit, per t2700.  f49's precedent: a type key without its
// constructor is rc=0 and then `error[E0433]`.
unsafe fn f2700() -> () {
    ()
}

// f2701 -- `tracing::DebugConfig::registerCLOptions()`, static and `void`, 4 sites.
// ⚠️ THE DROPPED SIDE EFFECT IS STATED IN src.cpp: it registers global `llvm::cl`
// options and this port has no `cl` registry to register into -- every `cl::opt` in
// the tree is itself an opaque unit.  Takes no argument, so nothing is dropped by
// the inlined body.
unsafe fn f2701() -> () {
    ()
}

// f2702 -- `tracing::DebugConfig::createFromCLOptions()`, the static factory, 4 sites.
// The same statement in value position: a unit config built from options that were
// never parsed.  Takes no argument, so the inlined body drops no evaluation.
unsafe fn f2702() -> () {
    ()
}

// ============================================================================================
// ROW g3073 -- GAP FAMILY F1 continuation: the three named successor dialect-op keys,
// t2617-t2619.  UNSAFE arm.
//
// Each maps to `fmt::OpInst` carrying the op's generated `DEF`, which is t161/t162's body with
// the concrete op substituted.  See `src.cpp` at t2617 for the DEF verification (positive AND
// negative control, two independent instruments, and the rmeta md5 re-measured rather than
// inherited), for the position census that is why there is NO `fN` constructor on any of the
// three, and for the MEMBER finding: no member of any of the three is mapped, and that is a
// NAMED residue (`g3067`), not a closed argument.
//
// ⭐ t2619 CARRIES `mlir_arith_ConstantOp`'s DEF ON PURPOSE, and it is t162's body verbatim, not
// an approximation: `arith::ConstantIntOp` (Arith.h:54) is a hand-written view class over the
// `arith.constant` ODS op -- no state, no ODS record, `resolveTypeID() -> TypeID::get<ConstantOp>()`
// -- so `dataflowir_gen::ops` carrying `mlir_arith_ConstantOp` and NO `mlir_arith_ConstantIntOp`
// is the model agreeing with the header.  Identical situation, identical body, as t162's
// `ConstantIndexOp`.
//
// The `init` is t25's/t152's/t157's/t158's/t161's/t162's/t2610-t2616's, and for their reason: a
// default-constructed ODS op handle is the NULL handle, `fmt::OpInst` has no null, and this
// expression exists only to type-check the type key.
//
// ⛔ t25's PROHIBITION APPLIES UNCHANGED: no `operator==`, no `operator!=`, no identity test.
// ⚠️ THIS ARM IS NOT THE OTHER ARM'S `sed`: it comes out byte-identical to the other, for the
// reason t157's and t2610's arms record -- `fmt::OpInst` is a plain generated value type, it is
// not behind a `Value<T>`/`Ptr<T>` wrapper in either model, and NO `ptr_bindings_` is registered
// here (registering one on the refcount arm is `E0614`).  Each body was reasoned for its own
// model and the identity is the conclusion, not the method.
fn t2617() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_AndIOp as dataflowir_gen::MlirOp>::DEF,
    )
}
fn t2618() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_affine_AffineApplyOp as dataflowir_gen::MlirOp>::DEF,
    )
}
fn t2619() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_ConstantOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// ===========================================================================
// GAP FAMILY F4 -- g3077, 2026-09-29.  See src.cpp for the member censuses, the
// fidelity restriction on t2630's unset state, and the three spellings left loud.

// t2630 -- `mlir::OpBuilder::InsertPoint` -> `(usize, usize)`, the (block index,
// op index) pair `dataflowir_gen::OpBuilder` already keeps its insertion point in
// (`insertion_block` build.rs:614, `insertion_index` :619, `set_insertion_point`
// :628).  MLIR's own InsertPoint is `{Block *, Block::iterator}` (Builders.h:327),
// the same pair in MLIR's coordinates, so this is the model's representation of the
// type and not a stand-in for it.
// ⛔ THE INIT VALUE IS NOT A SENTINEL.  `(0, 0)` is the start of the first block --
// exactly where `OpBuilder::new(new_block_list_with_entry())` (t440's own init) puts
// the insertion point -- so a default-initialised InsertPoint and a
// default-initialised builder agree, which is the only consistent pair available.
fn t2630() -> (usize, usize) {
    (0_usize, 0_usize)
}

// f2630 -- `saveInsertionPoint() const`, 25 sites.  The `let` binds the receiver ONCE:
// the body is INLINED, and `(a0.insertion_block(), a0.insertion_index())` would name
// the receiver expression twice -- the hazard f403's comment records.
fn f2630(a0: &dataflowir_gen::OpBuilder) -> (usize, usize) {
    let __b = a0;
    (__b.insertion_block(), __b.insertion_index())
}

// f2631 -- `restoreInsertionPoint(InsertPoint)`, 22 sites.  `&mut` because the header's
// receiver is non-const and `set_insertion_point` takes `&mut self`.  The `let` binds
// the argument ONCE for the same inlining reason as f2630 -- `insPts.top()` is the one
// non-variable argument in the corpus (ddl_conversion.cpp:2958) and naming it twice
// would evaluate it twice.
fn f2631(a0: &mut dataflowir_gen::OpBuilder, a1: (usize, usize)) {
    let __ip = a1;
    a0.set_insertion_point(__ip.0, __ip.1)
}

// t2631 -- `mlir::ConversionPatternRewriter` -> `dataflowir_gen::OpBuilder`.  t542's
// body VERBATIM: DialectConversion.h:839 derives it from `PatternRewriter`, which
// t542 already maps to this type, so the type carrier is identical.  No rewrite verb
// is keyed, for t541-t543's recorded `Operation *`/`OpHandle` reason.
fn t2631() -> dataflowir_gen::OpBuilder {
    dataflowir_gen::OpBuilder::new(dataflowir_gen::new_block_list_with_entry())
}

// t2800 / f2800 -- `mlir::Pass::Statistic` as a `u64` counter.  IDENTICAL on both
// arms, for the same reason `thread_id`'s t1 is: u64 is neither a pointer nor a
// container, so there is no ownership or sharing to model differently.  See
// rules/mlir/src.cpp for why a unit model is forbidden (ported code branches on
// the value at SetSendDestinationRE.cpp:148).
fn t2800() -> u64 {
    0
}

fn f2800(a0: &mut u64, a1: u32) -> &mut u64 {
    *a0 = a1 as u64;
    a0
}

// f2801 -- the three-argument Statistic constructor.  Arguments dropped (owner /
// name / description are for the -stats printout only); 0 is the real initial
// value of an llvm::TrackingStatistic, not a placeholder.  See src.cpp.
fn f2801(a0: *mut (), a1: *const i8, a2: *const i8) -> u64 {
    let _ = (a0, a1, a2);
    0
}

// t2750 -- `llvm::iterator_range<std::reverse_iterator<mlir::Operation **>>`.
// A `Vec` OF POINTERS: `mlir::Operation *` is t243`s `*mut fmt::OpInst`, so the
// snapshot copies the pointers and every `op->moveBefore(...)` in the loop body
// still lands on the real op.  `init` is the empty range, which is the only thing
// a default-constructed `iterator_range` can be.
fn t2750() -> Vec<*mut dataflowir_gen::fmt::OpInst> {
    Vec::new()
}

// f2750 -- `llvm::make_range(rbegin, rend)`.  `a0` is `rbegin()` = the END
// pointer, `a1` is `rend()` = the BEGIN pointer (rules/reverse_iterator t1 models
// a reverse_iterator as its underlying `current`, which points ONE PAST the
// element it designates).  So the element count is `a0 - a1` and the elements are
// read DOWNWARD from `a0`, `*(a0 - 1)` first -- reverse order, as C++ has it.
unsafe fn f2750(
    a0: *mut *mut dataflowir_gen::fmt::OpInst,
    a1: *mut *mut dataflowir_gen::fmt::OpInst,
) -> Vec<*mut dataflowir_gen::fmt::OpInst> {
    unsafe {
        (0..a0.offset_from(a1) as usize)
            .map(|i| *a0.sub(i + 1))
            .collect()
    }
}

// t2900 -- `llvm::iterator_range<mlir::Region::OpIterator>`.  t2750's body and
// t2750's model: a `Vec` OF POINTERS.  The element of this range is
// `mlir::Operation &` (`Operation &operator*() const`, mlir/IR/Region.h:143), and
// `mlir::Operation &` is ALREADY carried as `*mut fmt::OpInst` on this arm --
// f2505/f2506 (`simple_ilist<Operation>::front()`/`back()`) take exactly that
// reference to exactly this pointer.  So the snapshot copies POINTERS, the loop
// body's mutations land on the real ops, and t2750's aliasing licence transfers
// unchanged.  ⛔ See src.cpp at t2900 for why this contradicts t2750's own comment
// and the row brief, both of which listed this spelling as the forbidden case.
// `init` is the empty range, the only thing a default-constructed `iterator_range`
// can be.
fn t2900() -> Vec<*mut dataflowir_gen::fmt::OpInst> {
    Vec::new()
}

// f2900 -- `mlir::Region::getOps()`.  The receiver lowers to `&mut fmt::Region`,
// f460's formal verbatim.
// ⭐ INTERIOR POINTERS INTO THE LIVE BUFFERS, not copies: `b.ops` is the OWNING
// `Vec<OpInst>` (fmt.rs:480) and each element address is taken in place, which is
// f461's / rules/vector f13's model -- `Vec::push`-ing clones here would be the
// silent-miscompile this key exists to avoid.
// ⚠️ THE FLATTENING IS THE SEMANTICS, not a convenience.  C++'s `OpIterator` walks
// the region's blocks in order and, within each, that block's operations in program
// order, skipping blocks with no ops (`skipOverBlocksWithNoOps`, Region.h:152);
// `flat_map` over `blocks` then `ops` reproduces exactly that sequence, and an
// empty `ops` contributes nothing, which is the skip.
unsafe fn f2900(
    a0: &mut dataflowir_gen::fmt::Region,
) -> Vec<*mut dataflowir_gen::fmt::OpInst> {
    a0.get_blocks_mut()
        .iter_mut()
        .flat_map(|b| {
            b.ops
                .iter_mut()
                .map(|o| o as *mut dataflowir_gen::fmt::OpInst)
        })
        .collect()
}

// ============================================================================================
// ROW g3081 -- GAP FAMILY F1 continuation: the three MEASURED-GATE dialect-op keys, t2650-t2652.
//
// Each maps to `fmt::OpInst` carrying the op's own generated `DEF` -- t161/t162/t2610-t2619's body
// with the concrete op substituted, and no new model claim.  See `src.cpp` at t2650 for: the DEF
// verification (positive AND negative control, two independent instruments, rmeta md5 re-measured
// rather than inherited); the gate-TU re-derivation, which CORRECTED the brief's attribution of
// `vector::StoreOp` from `VectorChainToSentientPESFP.cpp` (still blocked on
// `mlir::ConversionPatternRewriter`) to `VectorChainToSentientPT.cpp`; the position census that is
// why there is NO `fN` constructor on any of the three; and the MEMBER finding -- no member of any
// of the three is mapped, which is a NAMED residue (`g3067`), not a closed argument.
//
// All three are case 1 of the trichotomy: ordinary ODS ops with their own generated `DEF`, so none
// needs t162/t2619's hand-written-view-class redirection and each carries its own marker.
//
// The `init` is t25's/t152's/t157's/t158's/t161's/t162's/t2610-t2619's, and for their reason: a
// default-constructed ODS op handle is the NULL handle, `fmt::OpInst` has no null, and this
// expression exists only to type-check the type key.
//
// ⛔ t25's PROHIBITION APPLIES UNCHANGED: no `operator==`, no `operator!=`, no identity test.
// ⚠️ THIS ARM IS NOT THE OTHER ARM'S `sed`: it comes out byte-identical to the other, for the
// reason t157's, t2610's and t2617's arms record -- `fmt::OpInst` is a plain generated value type,
// it is not behind a `Value<T>`/`Ptr<T>` wrapper in either model, and NO `ptr_bindings_` is
// registered here (registering one on the refcount arm is `E0614`).  Each body was reasoned for
// its own model and the identity is the conclusion, not the method.
fn t2650() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_arith_OrIOp as dataflowir_gen::MlirOp>::DEF,
    )
}
fn t2651() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_affine_AffineVectorLoadOp as dataflowir_gen::MlirOp>::DEF,
    )
}
fn t2652() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_vector_StoreOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// ===========================================================================
// GAP FAMILY F5 -- the two waiting `mlir::OwningOpRef` instantiations (g3082).
// Both reuse t980's landed model, `libcc2rs::OwningOpRef<OpTy>` -- an `Option<OpTy>`
// whose `Drop` panics if it still holds an op.  See src.cpp at t2680 for the
// character-exact abort the keys were read from and for why no member is keyed.

// t2680 -- `mlir::OwningOpRef<mlir::Operation *>` ->
// `libcc2rs::OwningOpRef<*mut dataflowir_gen::fmt::OpInst>`.
// The `OpTy` argument is t36's model for `mlir::Operation *` (:507), i.e.
// `*mut dataflowir_gen::fmt::OpInst`, spelled CONCRETELY because the refcount arm
// disagrees (`libcc2rs::Ptr<..>` there) and a type parameter in type position is not
// an inference variable.
//
// The default value is the NULL HANDLE, which is what C++'s
// `OwningOpRef(std::nullptr_t = nullptr)` gives a declaration the converter cannot
// initialise -- and it is exactly what `ddc/ddl/ddl.h:22`'s member holds until
// something assigns it.  ⚠️ Like t980 and unlike t800 this default must NOT be a
// panic: a null handle is a real, valid, observable C++ state whose destructor is a
// no-op (`if (op)` is false), so inventing a failure here would be wrong.
//
// ⚠️ THE ONE FIDELITY GAP, STATED: the tripwire keys on `Option::is_some()`, while
// C++ keys on the POINTER being non-null, so an adopted NULL `Operation *` would
// panic in Rust where C++ is silent.  That divergence is UNREACHABLE from this key
// alone -- `adopt()` is only callable from a constructor key, and no constructor is
// keyed for any `OwningOpRef` instantiation, so `null()` is the only handle this
// rule tree can produce.  A future ctor key must take `Option::None` for a null
// argument rather than `adopt(null_mut())`.
fn t2680() -> libcc2rs::OwningOpRef<*mut dataflowir_gen::fmt::OpInst> {
    libcc2rs::OwningOpRef::null()
}

// t2681 -- `mlir::OwningOpRef<mlir::ktdf_arch::DeviceOp>` ->
// `libcc2rs::OwningOpRef<dataflowir_gen::fmt::OpInst>`.
// `DeviceOp` is an ODS-generated op class, so its value model is t161's
// (`dataflowir_gen::fmt::OpInst`), the same model t400 gives the interface trait
// over this very op.  ⭐ NO `MlirOp::DEF` IS NAMED HERE and none is needed: the
// handle is constructed EMPTY, so no `OpInst` value is built and this key does not
// depend on the `mlir_ktdf_arch_DeviceOp` marker existing.
fn t2681() -> libcc2rs::OwningOpRef<dataflowir_gen::fmt::OpInst> {
    libcc2rs::OwningOpRef::null()
}

// f2901 `llvm::hash_code mlir::hash_value(mlir::Value)` -> a hash of the value,
// consistent with its `PartialEq`.  src.cpp carries the full argument; in one line:
// a hash function's only obligation is that EQUAL values hash EQUALLY, and
// `ir::Value` derives `Hash` and `PartialEq` over the same fields (ir.rs:20), so the
// obligation holds by construction.  The numeric result is NOT upstream's (upstream
// hashes the opaque pointer, which does not exist in this model) and nothing in the
// corpus compares a `hash_code` across the boundary.
//
// BYTE-IDENTICAL ON BOTH ARMS ON PURPOSE.  `mlir::Value` is t4 on both arms
// (`dataflowir_gen::ir::Value`) and `llvm::hash_code` is rules/support t2 (`u64`) on
// both arms, so neither the parameter nor the return changes model -- unlike the
// (Iter, bool) family, this key names no type that one model boxes and the other
// does not.  The converter passes the receiver by value at both sites (measured:
// `((*x).clone())` unsafe, `((*x.upgrade().deref()).clone())` refcount).
//
// ONE BLOCK EXPRESSION, because a rule body is inlined into the caller: the `use`
// items and the `let` are inside it, and the block's value is `h.finish()`.  The
// borrow of `h` ends inside the block, so there is no borrow-of-temporary hazard of
// the kind a refcount `(*x.borrow())` receiver has.
unsafe fn f2901(a0: dataflowir_gen::ir::Value) -> u64 {
    {
        use std::hash::Hash as _;
        use std::hash::Hasher as _;
        let mut h: std::collections::hash_map::DefaultHasher = Default::default();
        a0.hash(&mut h);
        h.finish()
    }
}
