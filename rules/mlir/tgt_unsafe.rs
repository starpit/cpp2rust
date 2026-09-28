// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

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

// t84 -- `mlir::OpAsmParser::UnresolvedOperand` -> `()`.  An opaque unit for a
// parser token the corpus only ever stores in a SmallVector and forwards to
// `parseOperandList`/`resolveOperands`, neither of which has a model.  Its three
// fields are unreachable BY DESIGN -- see src.cpp -- and no `==` exists to lie with.
unsafe fn t84() -> () {
    ()
}

// f62 -- the default constructor for t84; the unit, per t84.  0-ary, so there is
// no argument whose evaluation could be dropped (the f52 DominanceInfo hazard).
unsafe fn f62() -> () {
    ()
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
