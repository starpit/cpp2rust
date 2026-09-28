// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// REFCOUNT half.  Byte-for-byte the same bodies as tgt_unsafe.rs -- see that
// file for every argument.  The six MLIR types -> the .td-generated model in `dataflowir-gen`.  src.cpp
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
fn f1(a0: dataflowir_gen::ir::Attr, a1: dataflowir_gen::ir::Attr) -> bool {
    a0 == a1
}

// f2 -- `bool mlir::operator!=(mlir::StringAttr, std::nullptr_t)`.
// The null-handle test.  `a1` is the `nullptr` literal, which the converter emits
// as `Default::default()` (converter.cpp:3651); it carries no information, so it
// is bound to `()` and deliberately UNUSED -- the answer depends only on a0.
// Null is `Attr::Raw("")`, the same sentinel t6/t7 hand back for a default-
// constructed handle, and it is unreachable as a real attribute spelling.
fn f2(a0: dataflowir_gen::ir::Attr, a1: ()) -> bool {
    a0 != dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

// ---------------------------------------------------------------------------
// f3/f5 -- the DEFAULT CONSTRUCTORS of the two handles.  Same body as t6/t7's
// `init` and that identity is load-bearing: `mlir::Attribute a;` and a defaulted
// member of a ported struct must produce the same null handle, or a null test
// answers differently depending on how the handle came into being.
fn f3() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

// f4/f6 -- the COPY CONSTRUCTORS.  A C++ Attribute is a single pointer into the
// uniquer, so a copy is a second handle to the SAME uniqued instance and compares
// equal to its source.  `Attr::clone()` is that: an equal value, independently
// owned, which is what the by-value handle semantics need.
fn f4(a0: dataflowir_gen::ir::Attr) -> dataflowir_gen::ir::Attr {
    a0.clone()
}

fn f5() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

fn f6(a0: dataflowir_gen::ir::Attr) -> dataflowir_gen::ir::Attr {
    a0.clone()
}

// ---------------------------------------------------------------------------
// f7-f11 -- byte-for-byte the same bodies as tgt_unsafe.rs; see that file for
// every argument.  All five are pure value comparisons over `ir::Attr`, so
// neither model's pointer representation appears and the two halves must agree.
fn f7(a0: dataflowir_gen::ir::Attr, a1: dataflowir_gen::ir::Attr) -> bool {
    a0 == a1
}

fn f8(a0: dataflowir_gen::ir::Attr, a1: dataflowir_gen::ir::Attr) -> bool {
    a0 != a1
}

fn f9(a0: dataflowir_gen::ir::Attr, a1: ()) -> bool {
    a0 == dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

fn f10(a0: dataflowir_gen::ir::Attr, a1: dataflowir_gen::ir::Attr) -> bool {
    a0 != a1
}

fn f11(a0: dataflowir_gen::ir::Attr) -> bool {
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
fn f12() -> dataflowir_gen::ir::AffineMap {
    dataflowir_gen::ir::AffineMap::get(0, 0, ::std::vec::Vec::new())
}

fn f13(a0: dataflowir_gen::ir::AffineMap) -> dataflowir_gen::ir::AffineMap {
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
fn f14(a0: dataflowir_gen::ir::AffineMap, a1: dataflowir_gen::ir::AffineMap) -> bool {
    a0 == a1
}

fn f15(a0: dataflowir_gen::ir::AffineMap, a1: dataflowir_gen::ir::AffineMap) -> bool {
    a0 != a1
}

// f16 -- `bool mlir::Value::operator==(mlir::Value) const`, `impl == other.impl`.
// `ir::Value` (ir.rs:21) carries the printed name INCLUDING its `%` sigil plus the
// type; MLIR's printer gives every distinct SSA value in a region a distinct name,
// so name equality is SSA identity and the derived `PartialEq` is the C++
// semantics.  Note this is NOT value-content equality: two structurally identical
// but distinct SSA values have different names and correctly compare unequal.
fn f16(a0: dataflowir_gen::ir::Value, a1: dataflowir_gen::ir::Value) -> bool {
    a0 == a1
}

// f17-f19 -- the `!=` partners and the mlir::Type pair. Each is a SEPARATE rule
// key: the converter keys on the resolved callee signature and C++17 does not
// rewrite `!=` into `==`, so a TU spelling `a != b` consults f17/f19 and nothing
// else. Both handles compare by the identity their model carries: ir::Value is
// the %-sigilled SSA name (unique per region in MLIR's printer) and ir::Type is
// the printed type, so derived PartialEq is the right relation.
fn f17(a0: dataflowir_gen::ir::Value, a1: dataflowir_gen::ir::Value) -> bool {
    a0 != a1
}

fn f18(a0: dataflowir_gen::ir::Ty, a1: dataflowir_gen::ir::Ty) -> bool {
    a0 == a1
}

fn f19(a0: dataflowir_gen::ir::Ty, a1: dataflowir_gen::ir::Ty) -> bool {
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
// IDENTICAL to tgt_unsafe.rs, deliberately: all of these are VALUE types in this
// model, so neither model's pointer representation appears.  The justification for
// each one lives in src.cpp at its declaration.
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
fn t36() -> libcc2rs::Ptr<dataflowir_gen::fmt::OpInst> {
    libcc2rs::Ptr::<dataflowir_gen::fmt::OpInst>::null()
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

// t28 `mlir::OpFoldResult` -> `Result<ir::Attr, ir::Value>`, the SUM of the two
// alternatives `PointerUnion<Attribute, Value>` holds.  IDENTICAL to the unsafe
// target because both alternatives are VALUE types in this model, so no pointer
// representation appears.  tgt_unsafe.rs' t28 comment carries the full
// justification: the crate models nothing for OpFoldResult, this rule INTRODUCES
// the sum out of t6's and t4's existing models, and collapsing it to one
// alternative would be silently wrong.  `init` is the null union, carried as the
// same `Attr::Raw("")` null-handle sentinel the other Attr rules use.
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
fn f20(a0: dataflowir_gen::ir::Ty) -> bool {
    a0 == dataflowir_gen::ir::Ty::Opaque(::std::string::String::new())
}

// t70 `mlir::InFlightDiagnostic` -> `libcc2rs::InFlightDiagnostic`, A REAL
//   ACCUMULATING BUFFER THAT PRINTS ON `Drop` -- NOT an opaque unit.  Identical
//   to the unsafe model: the type is a VALUE with an owned `String`, so neither
//   model's pointer representation appears.  The full argument (Diagnostics.h:
//   325-328's reporting destructor, :319-324's abandoning move ctor, and why a
//   unit would silently delete every diagnostic) is in tgt_unsafe.rs and in
//   libcc2rs/src/diag.rs.
fn t70() -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::new()
}

// ---------------------------------------------------------------------------
// t71-t76 -- THE SIX ROWS ADDED 2026-09-27 (fourth slot).  src.cpp carries the
// full argument for each, including the OWNING HEADER LINE each spelling was
// read from and the zero-hit `~TypeName` destructor grep that clears all six for
// a non-Drop model.  IDENTICAL MODELS to tgt_unsafe.rs -- none of these six is
// ownership-sensitive, so the two models do not diverge here.
//
// t71 `mlir::UnitAttr`   -> `ir::Attr` (ir.rs:466).  ⭐ WELL-GROUNDED, not a
//   widening: `Attr::Unit` (ir.rs:473) is documented in the crate as
//   `mlir::UnitAttr` itself.  ⛔ But the INIT IS NOT `Attr::Unit` -- a
//   default-constructed UnitAttr is a NULL HANDLE and `Attr::Unit` is a REAL
//   present attribute, so the init is t5's empty-spelling `Attr::Raw("")`
//   sentinel.  `UnitAttr::get(ctx)` is NOT mapped and still aborts loudly.
fn t71() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}

// t72 `mlir::TypeID` -> AN OPAQUE UNIT, the same model as t58/t59/t69.  MLIR's
//   RTTI token, one pointer into a per-type static `Storage` (TypeID.h:112-115);
//   no `~TypeID` exists and it owns nothing, so the drop is effect-free.
//   ⛔ WHAT IS LOST IS THE ENTIRE POINT OF THE TYPE -- IDENTITY.  Every
//   translated TypeID is the same `()`, so `operator==`/`operator!=`
//   (TypeID.h:118-123) are DELIBERATELY NOT MAPPED: mapping them would make
//   every type compare EQUAL to every other, silently wrong, which ranks BELOW
//   an abort.  `TypeID::get<T>()` is likewise absent.  The body is `()`, not
//   empty: an empty body panics at syntactic.rs:591.
fn t72() -> () {
    ()
}

// t73 `mlir::MemRefType` -> `ir::Ty` (ir.rs:37).  A WIDENING, the same one t41,
//   t42 and t60 make.  ⭐ `Ty::MemRef(Vec<i64>, Box<Ty>)` (ir.rs:44) carries the
//   dimension list and element type, negative dim = dynamic `?`, so the SHAPE
//   SURVIVES.  ⛔ The init is still the empty-spelling `Ty::Opaque("")` null
//   sentinel, because an empty-shaped memref is a REAL type (`memref<f32>`).  NO
//   accessor is mapped.
fn t73() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(String::new())
}

// t74 `mlir::TypedAttr` -> `ir::Attr` (ir.rs:466).  A WIDENING, the 12th, with
//   the interface-over-subclasses shape of t41/t42/t55.  ⛔ `Attr` has no
//   variant pairing an arbitrary payload with a `Ty` except the
//   IntegerAttr-specific `Attr::Int(i64, Ty)` (ir.rs:469), so a non-integer
//   TypedAttr lands in `Attr::Raw` BY SPELLING and `getType()` is NOT mapped.
fn t74() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}

// t75 `mlir::VectorType` -> `ir::Ty` (ir.rs:37).  The BEST-GROUNDED of the two
//   type rows: `Ty::Vector(Vec<i64>, Box<Ty>)` is documented at ir.rs:41-42 as
//   `vector<64xf16> -- mlir::VectorType`, this exact class by name.  ⛔ Same two
//   losses as t73.
fn t75() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(String::new())
}

// t76 `mlir::FlatSymbolRefAttr` -> `ir::Attr` (ir.rs:466).  A WIDENING, the
//   13th.  ⛔ `Attr` HAS NO SYMBOL-REFERENCE VARIANT (zero symbol-ref hits in
//   ir.rs), so the referenced symbol NAME lands in `Attr::Raw` BY SPELLING --
//   the printed `@name` form round-trips, the ability to RESOLVE the symbol does
//   not.  getValue()/getAttr()/get are NOT mapped.
fn t76() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}

// f21 -- `mlir::InFlightDiagnostic && operator shl(const char (&)[_]) &&`, row
// g286, 21 TUs.  a0 is the receiver, a1 the streamed literal, each mentioned
// EXACTLY ONCE (a second mention would re-evaluate and emit a second message).
// In the REFCOUNT model a `const char (&)[N]` argument arrives as `Ptr<u8>`
// (rules/stringref f8's spelling), whose `to_c_bytes()` gives the bytes up to the
// NUL; `shl_bytes` stops at a NUL anyway, so the two models append the same text.
fn f21(a0: libcc2rs::InFlightDiagnostic, a1: &[u8]) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_bytes(a0, a1)
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
fn f22(a0: libcc2rs::InFlightDiagnostic, a1: Vec<u8>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_bytes(a0, &a1)
}

// f23 -- row g346, `llvm::StringRef &`, 8 TUs
fn f23(a0: libcc2rs::InFlightDiagnostic, a1: &Vec<u8>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_bytes(a0, a1)
}

// f24 -- row g396, `std::string &`, 5 TUs
fn f24(a0: libcc2rs::InFlightDiagnostic, a1: &Vec<u8>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_bytes(a0, a1)
}

// f25 -- row g491, `mlir::Type &`, 3 TUs -- Display is MLIR's type syntax (ir.rs:50)
fn f25(a0: libcc2rs::InFlightDiagnostic, a1: &dataflowir_gen::ir::Ty) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f26 -- row g553, `unsigned int &`, 2 TUs
fn f26(a0: libcc2rs::InFlightDiagnostic, a1: &u32) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f27 -- row g1151 + g1161, `int &`
fn f27(a0: libcc2rs::InFlightDiagnostic, a1: &i32) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f28 -- row g1152 + g1164, `long &`
fn f28(a0: libcc2rs::InFlightDiagnostic, a1: &i64) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f29 -- row g1150, `const long &`
fn f29(a0: libcc2rs::InFlightDiagnostic, a1: &i64) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f30 -- row g1160, `const int &`
fn f30(a0: libcc2rs::InFlightDiagnostic, a1: &i32) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f31 -- row g1162, `const unsigned int &`
fn f31(a0: libcc2rs::InFlightDiagnostic, a1: &u32) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f32 -- row g1163, `unsigned int &&`
fn f32(a0: libcc2rs::InFlightDiagnostic, a1: u32) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f33 -- row g1157, `unsigned long &&`
fn f33(a0: libcc2rs::InFlightDiagnostic, a1: u64) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f34 -- row g1158, `const std::string &`
fn f34(a0: libcc2rs::InFlightDiagnostic, a1: &Vec<u8>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_bytes(a0, a1)
}

// f35 -- row g1159, `std::string &&`
fn f35(a0: libcc2rs::InFlightDiagnostic, a1: Vec<u8>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_bytes(a0, &a1)
}

// f36 -- row g1155, `mlir::Attribute &` -- Display is MLIR's attr syntax (ir.rs:521)
fn f36(a0: libcc2rs::InFlightDiagnostic, a1: &dataflowir_gen::ir::Attr) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f37 -- row g1156, `mlir::StringAttr &&` (t7 -> the same Attr)
fn f37(a0: libcc2rs::InFlightDiagnostic, a1: dataflowir_gen::ir::Attr) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f38 -- row g1154, `llvm::StringLiteral &&`
fn f38(a0: libcc2rs::InFlightDiagnostic, a1: Vec<u8>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_bytes(a0, &a1)
}

// f39 -- the default constructor, `mlir::InFlightDiagnostic d;`.  A fresh,
// in-flight, empty diagnostic; `live` is true so Drop reports it (diag.rs).
fn f39() -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::new()
}

// f40-f45 -- THE DEFAULT CONSTRUCTORS FOR t71-t76, one per type key.  A `using
// tN =` maps the TYPE ONLY; the converter looks `void <T>::<T>()` up as an
// ORDINARY EXPR RULE and on a miss emits `<mangled type>::new()`, which does not
// exist -- rc=0 and then `error[E0433]`.  Each body is the SAME null-handle
// sentinel as its type's `init` above, NOT a valid value.
fn f40() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}
fn f41() -> () {
    ()
}
fn f42() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(String::new())
}
fn f43() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}
fn f44() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(String::new())
}
fn f45() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(String::new())
}

// t77 -- `llvm::sys::SmartMutex<true>`, the same model rules/mutex gives
// std::mutex.  lock/unlock/try_lock are NOT keyed, so the recursive-vs-plain
// difference is unobservable; see the src.cpp note.  The refcount limitation
// rules/mutex/tgt_refcount.rs records applies identically: a BY-REFERENCE member
// on this type could not be read through a `Ptr<T>` (Mutex is not ByteRepr) --
// which costs nothing here, because no member is keyed.
fn t77() -> ::std::sync::Mutex<()> {
    ::std::sync::Mutex::new(())
}

// t78 -- `llvm::cl::desc`, a one-field StringRef modifier; the StringRef model
// from rules/stringref t1.
fn t78() -> Vec<u8> {
    vec![0]
}

// f46 -- the default constructor for t77, a fresh unlocked mutex.
fn f46() -> ::std::sync::Mutex<()> {
    ::std::sync::Mutex::new(())
}

// f47 -- `cl::desc(StringRef)`: the modifier IS its payload, so the constructor
// is the identity on the StringRef bytes.
fn f47(a0: Vec<u8>) -> Vec<u8> {
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
fn f48() -> Vec<bool> {
    Vec::new()
}

// f49 -- the implicit default constructor for t80; the unit, per t80.
fn f49() -> () {
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
fn f50<T1>(a0: Vec<T1>) -> Vec<T1> {
    a0
}

// f51 -- `DominanceInfo di;`, the 0-ary form.  The unit, per t82.
fn f51() -> () {
    ()
}

// f52 -- `DominanceInfo di(op);`.  The unit per t82, but the body CONSUMES `a0`
// rather than ignoring it: a rule body is inlined as one expression, so dropping
// `a0` would drop the caller's expression (`unit_op`, `func`, `module_op`) and
// change C++ evaluation.  `drop` evaluates it and yields `()`.
fn f52(a0: libcc2rs::Ptr<dataflowir_gen::fmt::OpInst>) -> () {
    drop(a0)
}

// f53/f54 -- the free ArrayRef comparisons.  Shape copied verbatim from
// rules/vector f115-f118, including the `&Vec<T1>` parameters and the
// `T1: PartialEq` bound.
fn f53<T1: PartialEq>(a0: &Vec<T1>, a1: &Vec<T1>) -> bool {
    a0 == a1
}

fn f54<T1: PartialEq>(a0: &Vec<T1>, a1: &Vec<T1>) -> bool {
    a0 != a1
}

// f55 -- ArrayRef(std::initializer_list<T1>).  rules/initializer_list maps
// `std::initializer_list<T1>` to `Vec<T1>`, so this is the identity, exactly as
// rules/vector f36 is for `std::vector`'s initializer-list constructor.
fn f55<T1>(a0: Vec<T1>) -> Vec<T1> {
    a0
}

fn f56<T1>(a0: &Vec<T1>) -> usize {
    a0.len()
}

// f57 -- `erase(index)` is void in C++ and `Vec::remove` yields the element, so
// the result is discarded.  `drop(..)` keeps the body ONE expression yielding ().
fn f57<T1>(a0: &mut Vec<T1>, a1: usize) {
    drop(a0.remove(a1))
}

fn f58<T1>(a0: &mut Vec<T1>, a1: usize, a2: T1) {
    a0.insert(a1, a2)
}

fn f59<T1>(a0: &mut Vec<T1>, a1: Vec<T1>) {
    let __src = a1;
    a0.clear();
    a0.extend(__src);
}

fn f60<T1: Clone>(a0: &Vec<T1>) -> Vec<T1> {
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
fn t83() -> dataflowir_gen::ir::AffineExpr {
    dataflowir_gen::ir::AffineExpr::Symbol(u32::MAX)
}

// f61 -- the default constructor for t83.  Byte-identical to t83's `init` on
// purpose: they describe the same value by two routes (the type's `init` and the
// expression a written `AffineExpr e;` becomes), the same way f12 mirrors t8.
fn f61() -> dataflowir_gen::ir::AffineExpr {
    dataflowir_gen::ir::AffineExpr::Symbol(u32::MAX)
}

// t84 -- `mlir::OpAsmParser::UnresolvedOperand` -> `()`.  An opaque unit for a
// parser token the corpus only ever stores in a SmallVector and forwards to
// `parseOperandList`/`resolveOperands`, neither of which has a model.  Its three
// fields are unreachable BY DESIGN -- see src.cpp -- and no `==` exists to lie with.
fn t84() -> () {
    ()
}

// f62 -- the default constructor for t84; the unit, per t84.  0-ary, so there is
// no argument whose evaluation could be dropped (the f52 DominanceInfo hazard).
fn f62() -> () {
    ()
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
fn f120<T1>() -> Vec<T1> {
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
//   `dataflowir_gen::EffectInstance`.  IDENTICAL to the unsafe target: the model is
//   a plain `Copy` value struct of a `Copy` enum, an `Option<usize>`, an `i32`, a
//   `bool` and a `&'static str`, so there is nothing for the refcount model to own
//   differently.  It is restated here and not omitted because a module that HAS a
//   `tgt_refcount.rs` MUST carry every TYPE key in it -- omitting one aborts at LOAD
//   time (`translation_rule.cpp:233`) and under NDEBUG presents as rc=139 with no
//   message (the `rules/iostream` `t1` defect, 68b29ef7).
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
// f126-f128 -- the free `llvm::raw_ostream <<` family; see src.cpp and
// tgt_unsafe.rs.  Overlay differences only: `raw_ostream &` is
// `libcc2rs::Ptr<std::fs::File>` rather than a raw pointer, and libcc2rs impls
// `write_all` directly on `Ptr<T: std::io::Write + ByteRepr>` (std::fs::File is
// ByteRepr), so no `with_mut` closure is needed -- exactly as
// rules/raw_ostream/tgt_refcount.rs does it.  The receiver is still RETURNED.
// ---------------------------------------------------------------------------

fn f126(
    a0: libcc2rs::Ptr<std::fs::File>,
    a1: dataflowir_gen::ir::Ty,
) -> libcc2rs::Ptr<std::fs::File> {
    let __o = a0;
    let __b = ::std::string::ToString::to_string(&a1).into_bytes();
    let _ = __o.write_all(&__b);
    __o
}

fn f127(
    a0: libcc2rs::Ptr<std::fs::File>,
    a1: dataflowir_gen::ir::Attr,
) -> libcc2rs::Ptr<std::fs::File> {
    let __o = a0;
    let __b = ::std::string::ToString::to_string(&a1).into_bytes();
    let _ = __o.write_all(&__b);
    __o
}

fn f128(
    a0: libcc2rs::Ptr<std::fs::File>,
    a1: &dataflowir_gen::ir::Location,
) -> libcc2rs::Ptr<std::fs::File> {
    let __o = a0;
    let __b = ::std::string::ToString::to_string(a1).into_bytes();
    let _ = __o.write_all(&__b);
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
fn f137() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(String::new())
}
fn f138() -> dataflowir_gen::ir::Ty {
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
fn f139<T1>() -> Vec<T1> {
    Vec::new()
}

// f140 -- the one-element constructor.  `T1: Clone` and `a0.clone()` follow
// rules/support f19 exactly: at `a0: &T1` the by-value probe step picks
// `<T1 as Clone>::clone`, so this yields `T1`, not `&T1`.
fn f140<T1: Clone>(a0: &T1) -> Vec<T1> {
    // PATH FORM, not `a0.clone()`: the converter substitutes `&(*s)` for a `const &`
    // argument and appends the method textually, so `a0.clone()` emitted
    // `vec![&(*s).clone()]` -- `&((*s).clone())`, a reference to a temporary and the
    // WRONG element type.  MEASURED, then fixed.  Same lesson as AGENT-COMMON's
    // `Vec::len(&a0)` / `Ptr::decay(&a0)` note.
    vec![Clone::clone(a0)]
}

// f141 -- from a `std::vector<T1>`, which rules/vector also models as `Vec<T1>`,
// so the copy is `Vec::clone`.
fn f141<T1: Clone>(a0: &Vec<T1>) -> Vec<T1> {
    a0.clone()
}

// f142 -- `mlir::RegionRange`'s own constructor (Region.h:342-356).  Identical to
// the unsafe body, and deliberately so: both sides are VALUE-LIKE (`Vec` of an
// owned `fmt::Region`), there is no raw-pointer text and no `as_pointer()`, so the
// two models expand the same text identically (the `rules/cstddef` precedent).
// It is restated here only because this module HAS a `tgt_refcount.rs`, and the
// `rules/iostream` t1 incident established that a module which has one must carry
// every key the loader looks for in it.  See src.cpp at f142 for the readback and
// the view/aliasing argument.
fn f142(a0: Vec<dataflowir_gen::fmt::Region>) -> Vec<dataflowir_gen::fmt::Region> {
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
fn f143(a0: Vec<dataflowir_gen::ir::Value>) -> Vec<dataflowir_gen::ir::Value> {
    a0
}

fn f144(a0: Vec<dataflowir_gen::ir::Value>) -> Vec<dataflowir_gen::ir::Value> {
    a0
}

// f145 -- `std::vector<mlir::Value> &` is the forwarding template's DEDUCED
// parameter (ValueRange.h:397-401), read-only in fact, so `&mut Vec` here (the
// rules/array f3-f5 convention for a non-const reference) is only borrowed, never
// written.  PATH FORM with an explicit reborrow, NOT `a0.clone()`: the converter
// substitutes the argument textually and appends methods textually, so a bare
// `a0.clone()` risks emitting `&mut (*s).clone()` = `&mut ((*s).clone())`, a
// reference to a temporary -- the bug measured and fixed in f140.
fn f145(a0: &mut Vec<dataflowir_gen::ir::Value>) -> Vec<dataflowir_gen::ir::Value> {
    Clone::clone(&*a0)
}

// t166 -- the range CRTP base at its FOURTH concrete instantiation,
// `DerivedT = mlir::ValueRange` (queue row g090).  Body IDENTICAL to t16
// (`mlir::ValueRange`) and to t37, because the base IS the range: no new
// representation is introduced.  Returned BY VALUE, so nothing can dangle under
// the refcount model.  See src.cpp for the full spelling read off the abort, the
// swallow-safety argument, and why g111 (`mlir::TypeRange`) needs its own key.
fn t166() -> Vec<dataflowir_gen::ir::Value> {
    Default::default()
}

// f146 -- `llvm::APInt mlir::IntegerAttr::getValue() const`, 16 asks on
// KtdpDialect.cpp and the largest single count in that TU's diagnosis.  Body
// IDENTICAL to the unsafe model's, and it is identical for a reason that matters
// HERE specifically: the C++ member returns `llvm::APInt` BY VALUE, so there is no
// borrow taken out of the by-value receiver and nothing can dangle under refcount.
// That is precisely the test `std::string_view::front()` and
// `llvm::SMLoc::getPointer()` FAILED -- both return a REFERENCE into a receiver
// this model owns by value -- and it is why the by-value return was confirmed off
// the readback spelling (no `&`) before the key was written.
//
// See src.cpp and tgt_unsafe.rs for the width argument (t10 widens IntegerAttr to
// `ir::Attr`; `Attr::Int(i64, Ty)` + `Ty::Int(u32)` make the APInt exactly
// reconstructible) and for why the fallback arm PANICS with a named message rather
// than answering 0.
fn f146(a0: dataflowir_gen::ir::Attr) -> libcc2rs::APInt {
    match a0 {
        dataflowir_gen::ir::Attr::Int(v, dataflowir_gen::ir::Ty::Int(w)) => {
            libcc2rs::APInt::new(w, v as u64, true, true)
        }
        dataflowir_gen::ir::Attr::Int(v, dataflowir_gen::ir::Ty::Index) => {
            libcc2rs::APInt::new(64, v as u64, true, true)
        }
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
// The representation is a `Ptr<T>` INTO the Vec the container already is --
// identical to t36 (`mlir::Operation *`) and to rules/list_iterator's
// tgt_refcount t1/t2 against the same Vec representation, so nothing new is
// introduced here.  The init is the NULL POINTER, the same faithful default t36
// gives: an iterator with no position traps on use instead of pretending to
// point at an operation.  IsReverse does not change the REPRESENTATION, only
// the direction a (deliberately unkeyed) operator++ would step.
fn t243() -> libcc2rs::Ptr<dataflowir_gen::fmt::OpInst> {
    libcc2rs::Ptr::<dataflowir_gen::fmt::OpInst>::null()
}

fn t244() -> libcc2rs::Ptr<dataflowir_gen::fmt::OpInst> {
    libcc2rs::Ptr::<dataflowir_gen::fmt::OpInst>::null()
}

fn t245() -> libcc2rs::Ptr<dataflowir_gen::fmt::Block> {
    libcc2rs::Ptr::<dataflowir_gen::fmt::Block>::null()
}

fn t246() -> libcc2rs::Ptr<dataflowir_gen::fmt::Block> {
    libcc2rs::Ptr::<dataflowir_gen::fmt::Block>::null()
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
// argument.  The two invariants of every body in this family hold here too:
//   * `a0` AND `a1` ARE EACH MENTIONED EXACTLY ONCE (a rule body is inlined as
//     one expression, so a second mention re-evaluates and would emit a second
//     diagnostic / re-run whatever produced the streamed value).
//   * exactly-once reporting comes from `self` BY VALUE.
// An lvalue-reference argument takes a Rust SHARED REFERENCE so inlining cannot
// move the caller's variable (E0382).
// ---------------------------------------------------------------------------

// f150 -- `llvm::Twine &&`.  A Twine is the NUL-terminated byte string it denotes
// (rules/twine), `Vec<u8>` in this model; `shl_bytes` stops at the NUL, so the
// terminator is not appended.
fn f150(a0: libcc2rs::InFlightDiagnostic, a1: Vec<u8>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_bytes(a0, &a1)
}

// f151 -- `const llvm::Twine &`
fn f151(a0: libcc2rs::InFlightDiagnostic, a1: &Vec<u8>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_bytes(a0, a1)
}

// f152 -- `unsigned long &`
fn f152(a0: libcc2rs::InFlightDiagnostic, a1: &u64) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f153 -- `mlir::Type &&` -- Display is MLIR's type syntax (ir.rs:50), as f25
fn f153(a0: libcc2rs::InFlightDiagnostic, a1: dataflowir_gen::ir::Ty) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f154 -- `long &&`
fn f154(a0: libcc2rs::InFlightDiagnostic, a1: i64) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f155 -- `int &&`
fn f155(a0: libcc2rs::InFlightDiagnostic, a1: i32) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f156 -- `const unsigned long &`
fn f156(a0: libcc2rs::InFlightDiagnostic, a1: &u64) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_display(a0, a1)
}

// f157 -- `const llvm::StringRef &`
fn f157(a0: libcc2rs::InFlightDiagnostic, a1: &Vec<u8>) -> libcc2rs::InFlightDiagnostic {
    libcc2rs::InFlightDiagnostic::shl_bytes(a0, a1)
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

// t340 -- `mlir::OpaqueProperties` -> `*mut ::libc::c_void`.  See tgt_unsafe.rs and
// src.cpp.  BYTE-IDENTICAL to the unsafe overlay because a `void *` has no
// refcountable pointee: the refcount model spells an opaque `void *` as a raw
// pointer as well (rules/brotli f6, `*mut c_void` in BOTH overlays).
fn t340() -> *mut ::libc::c_void {
    ::std::ptr::null_mut()
}

// f240 -- `OpaqueProperties(void *prop)`.  Identity, as in the unsafe overlay.
fn f240(a0: *mut ::libc::c_void) -> *mut ::libc::c_void {
    a0
}

// t420-t422: a NULL-HANDLE model for the three `mlir::memref` ODS op wrappers.  See src.cpp for
// what the 8 sites do after the default-construct, why `Option<fmt::OpInst>` is rejected (the two
// `mc` sites MUTATE THROUGH THE HANDLE: `mc.erase()`, `sel.getResult().setType(...)`), and why
// this needs NO `fN` (`impl<T> Default for Ptr<T>`, rc.rs:148, with `#[default] PtrKind::Null`,
// makes `Ptr::default()` EXACTLY `Ptr::null()` -- unlike f42/f137/f138, whose `ir::Ty`/`ir::Attr`
// enum targets have no `Default` at all, which is the whole rc=0-then-E0433 trap).
// The representation is BIT-FOR-BIT t36's (`mlir::Operation *`, :493), because an ODS op wrapper IS
// an `Operation *` plus a static type assertion -- `OpState` holds exactly one `Operation *`.  The
// init is the NULL POINTER: a default-constructed MLIR op wrapper is `state == nullptr`, so this is
// the faithful default and a `dyn_cast` against it is the one that must FAIL.
// ⚠️ NO MEMBER IS KEYED, the t166 / t243-t246 discipline: `getDest()` / `getResult()` / `erase()`
// on these stay unmapped rather than get a body guessed against an unverified model.
fn t420() -> libcc2rs::Ptr<dataflowir_gen::fmt::OpInst> {
    libcc2rs::Ptr::<dataflowir_gen::fmt::OpInst>::null()
}

fn t421() -> libcc2rs::Ptr<dataflowir_gen::fmt::OpInst> {
    libcc2rs::Ptr::<dataflowir_gen::fmt::OpInst>::null()
}

fn t422() -> libcc2rs::Ptr<dataflowir_gen::fmt::OpInst> {
    libcc2rs::Ptr::<dataflowir_gen::fmt::OpInst>::null()
}

// t440 -- `mlir::OpBuilder` -> `dataflowir_gen::OpBuilder` (build.rs:461).  See
// tgt_unsafe.rs for the full reasoning; the two overlays agree because the builder is
// modelled on an `Rc<RefCell<Vec<Block>>>` BlockList, i.e. the refcount overlay's own
// representation, so no `Ptr`/`StrongPtr` wrapper is needed at the TYPE level.
// ⭐ `cc2.rs` already carries the `ByteRepr` markers for `OpBuilder` /
// `ImplicitLocOpBuilder` / `OpHandle` / `OpArgs`, which is what lets a refcount rule
// hold `Ptr<OpBuilder>` for the C++ `OpBuilder &` parameters -- that bound is a
// prerequisite for this key and it is already in place.
fn t440() -> dataflowir_gen::OpBuilder {
    dataflowir_gen::OpBuilder::new(dataflowir_gen::new_block_list_with_entry())
}

// t441 -- `mlir::ImplicitLocOpBuilder` -> `dataflowir_gen::ImplicitLocOpBuilder`
// (build.rs:630).  Identical to the unsafe overlay, for the same reason.
fn t441() -> dataflowir_gen::ImplicitLocOpBuilder {
    dataflowir_gen::ImplicitLocOpBuilder::new(
        dataflowir_gen::ir::Location::Unknown,
        dataflowir_gen::new_block_list_with_entry(),
    )
}

// t460-t463 / f360-f366 -- THE MLIR PRINTER SINK AND ITS `<<` FAMILY.  src.cpp
// carries the model and the three spellings deliberately left out; tgt_unsafe.rs
// carries the shared invariants.  What is DIFFERENT in this overlay:
//
// ⭐ `Ptr<AsmPrinter>` IS ONLY LEGAL BECAUSE `dataflowir-gen/src/cc2.rs:44` ADDS
// `impl ByteRepr for AsmPrinter`.  `Ptr::with_mut` is bounded on `T: ByteRepr`
// (libcc2rs/src/rc.rs:505-507) and `StrongPtr::deref` likewise, so without that
// impl every body here fails with "the method exists but its trait bounds were not
// satisfied" -- which is what kept this family unkeyable until 8dee457.
// ⚠️ IT IS A MARKER AND CLAIMS NO LAYOUT: `ByteRepr`'s `byte_size`/`to_bytes`/
// `from_bytes` are the trait's panicking defaults, so reinterpreting a printer as
// bytes still aborts loudly rather than producing a plausible value.  That is the
// intended direction -- an `AsmPrinter` owns a `String`, a `Vec<String>` and a
// `BTreeMap`.
//
// The `&`-typed keys t461/t463 init to `Ptr::null()` for the t420-t422 reason: a
// reference cannot be default-constructed from any well-formed C++, so the init is
// unreachable, and the null handle is the faithful "no printer" value.
//
// ⚠️ Writes go through `with_mut`, the rules/raw_ostream f4 spelling, and every
// body returns its own `a0` so `p << a << b` chains.
fn t460() -> dataflowir_gen::AsmPrinter {
    dataflowir_gen::AsmPrinter::new()
}

fn t461() -> libcc2rs::Ptr<dataflowir_gen::AsmPrinter> {
    libcc2rs::Ptr::<dataflowir_gen::AsmPrinter>::null()
}

fn t462() -> dataflowir_gen::AsmPrinter {
    dataflowir_gen::AsmPrinter::new()
}

fn t463() -> libcc2rs::Ptr<dataflowir_gen::AsmPrinter> {
    libcc2rs::Ptr::<dataflowir_gen::AsmPrinter>::null()
}

// f360 -- `const char (&)[_]`, 155 sites.  A char array reaches this model as
// `&[u8]` (the f21 spelling) and INCLUDES the literal's NUL terminator, so the
// bytes are cut at the first NUL before printing -- `print_str` appends verbatim
// and would otherwise plant a 0x00 in the middle of the round-trip text.
fn f360(a0: libcc2rs::Ptr<dataflowir_gen::AsmPrinter>, a1: &[u8]) -> libcc2rs::Ptr<dataflowir_gen::AsmPrinter> {
    let __p = a0;
    let __n = a1.iter().position(|&c| c == 0u8).unwrap_or(a1.len());
    let __s = String::from_utf8_lossy(&a1[..__n]).into_owned();
    __p.with_mut(|__q: &mut dataflowir_gen::AsmPrinter| {
        __q.print_str(&__s);
    });
    __p
}

// f361 -- `const char &`, 34 sites.
fn f361(a0: libcc2rs::Ptr<dataflowir_gen::AsmPrinter>, a1: &u8) -> libcc2rs::Ptr<dataflowir_gen::AsmPrinter> {
    let __p = a0;
    let __c = *a1 as char;
    __p.with_mut(|__q: &mut dataflowir_gen::AsmPrinter| {
        __q.print_char(__c);
    });
    __p
}

// f362 -- `const mlir::OperandRange &`, 26 sites.  t14 is `Vec<ir::Value>`.
fn f362(a0: libcc2rs::Ptr<dataflowir_gen::AsmPrinter>, a1: &Vec<dataflowir_gen::ir::Value>) -> libcc2rs::Ptr<dataflowir_gen::AsmPrinter> {
    let __p = a0;
    __p.with_mut(|__q: &mut dataflowir_gen::AsmPrinter| {
        __q.print_operands(a1.iter());
    });
    __p
}

// f363 -- `mlir::Value` by value, 25 sites.  Prints the SSA NAME; see src.cpp.
fn f363(a0: libcc2rs::Ptr<dataflowir_gen::AsmPrinter>, a1: dataflowir_gen::ir::Value) -> libcc2rs::Ptr<dataflowir_gen::AsmPrinter> {
    let __p = a0;
    __p.with_mut(|__q: &mut dataflowir_gen::AsmPrinter| {
        __q.print_operand(&a1);
    });
    __p
}

// f364 -- `mlir::AsmPrinter &` + `const char &`, 1 site.  f361's body, distinct key.
fn f364(a0: libcc2rs::Ptr<dataflowir_gen::AsmPrinter>, a1: &u8) -> libcc2rs::Ptr<dataflowir_gen::AsmPrinter> {
    let __p = a0;
    let __c = *a1 as char;
    __p.with_mut(|__q: &mut dataflowir_gen::AsmPrinter| {
        __q.print_char(__c);
    });
    __p
}

// f365 -- `const llvm::StringRef &`, 1 site.  The trailing NUL is not part of the
// string (rules/stringref), so it is dropped -- the rules/raw_ostream f7 fix.
fn f365(a0: libcc2rs::Ptr<dataflowir_gen::AsmPrinter>, a1: &Vec<u8>) -> libcc2rs::Ptr<dataflowir_gen::AsmPrinter> {
    let __p = a0;
    let __b: Vec<u8> = a1
        .iter()
        .copied()
        .take(a1.len().saturating_sub(1))
        .collect();
    let __s = String::from_utf8_lossy(&__b).into_owned();
    __p.with_mut(|__q: &mut dataflowir_gen::AsmPrinter| {
        __q.print_str(&__s);
    });
    __p
}

// f366 -- `const llvm::StringLiteral &`, 1 site.  Same payload as f365.
fn f366(a0: libcc2rs::Ptr<dataflowir_gen::AsmPrinter>, a1: &Vec<u8>) -> libcc2rs::Ptr<dataflowir_gen::AsmPrinter> {
    let __p = a0;
    let __b: Vec<u8> = a1
        .iter()
        .copied()
        .take(a1.len().saturating_sub(1))
        .collect();
    let __s = String::from_utf8_lossy(&__b).into_owned();
    __p.with_mut(|__q: &mut dataflowir_gen::AsmPrinter| {
        __q.print_str(&__s);
    });
    __p
}

// t480..t482 -- llvm::SmallDenseSet<T>.  Rows g1129, g1130, g1131, g1136, g1137,
// g1138.  Plain `std::collections::HashSet`, identical to rules/densemap t2 and to
// t236-t242 in this file; a default-constructed SmallDenseSet IS the empty set, so
// the init is an exact match and not a sentinel.  TYPE rules, so `fn`, not
// `unsafe fn` (the t166 / t236 convention).
fn t480() -> std::collections::HashSet<i64> {
    std::collections::HashSet::new()
}

fn t481() -> std::collections::HashSet<u32> {
    std::collections::HashSet::new()
}

// `llvm::StringRef` is `Vec<u8>` in THIS model (f22/f23 above) and
// `Vec<libc::c_char>` in the unsafe one -- see tgt_unsafe.rs.  Both are Hash + Eq.
fn t482() -> std::collections::HashSet<Vec<u8>> {
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
    a1: Vec<u8>,
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
fn f403(a0: &dataflowir_gen::OpBuilder, a1: &Vec<u8>) -> dataflowir_gen::ir::Attr {
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
fn f406(a0: &dataflowir_gen::OpBuilder, a1: Vec<Vec<u8>>) -> dataflowir_gen::ir::Attr {
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

// t540 -- `mlir::DenseArrayAttr` -> `dataflowir_gen::ir::Attr`.  Identical to the
// unsafe overlay: `ir::Attr` is a plain value union with no pointer in it, so the
// two models agree.  See src.cpp for the model and for why no member is keyed.
fn t540() -> dataflowir_gen::ir::Attr {
    dataflowir_gen::ir::Attr::Raw(::std::string::String::new())
}

// t541/t542/t543 -- `mlir::RewriterBase` / `mlir::PatternRewriter` /
// `mlir::IRRewriter` -> `dataflowir_gen::OpBuilder`, the SAME type t440 lands
// `mlir::OpBuilder` on (PatternMatch.h:368/780/799 make all three OpBuilders by
// inheritance).  Identical to the unsafe overlay for t440's own reason: the builder
// is modelled on an `Rc<RefCell<Vec<Block>>>` BlockList, i.e. this overlay's own
// representation, so no `Ptr`/`StrongPtr` wrapper is needed at the TYPE level, and
// `cc2.rs` already carries the `ByteRepr` marker for `OpBuilder`.
// ⛔ NO rewrite verb is keyed -- see src.cpp.
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
// argument (an unmapped MEMBER is emitted TEXTUALLY and never aborts), and the
// bucket/swallow safety.  `fmt::Region { pub blocks: Vec<Block> }` (fmt.rs:513),
// so the block list IS a `Vec<fmt::Block>`; `iplist<T>` derives from
// `simple_ilist<T>` so BOTH keys get the SAME body -- the t37-t39/t166 "the base
// IS the range" discipline.
fn t560() -> Vec<dataflowir_gen::fmt::Block> {
    Vec::new()
}

fn t561() -> Vec<dataflowir_gen::fmt::Block> {
    Vec::new()
}

// f460 -- `mlir::Region::getBlocks()` -> `fmt::Region::get_blocks_mut()`
// (fmt.rs:557).  The MUT half, because the C++ member is non-const and returns a
// mutable reference, and every emitted receiver is a `*mut fmt::Region` deref.
// snake_case is the point: it stops the class's OTHER unmapped members from
// resolving by accident.  ⚠️ `a0` is named EXACTLY ONCE and carries nothing but a
// method call, because a `&mut` formal's `aN` re-expands to the bare lvalue.
fn f460(a0: &mut dataflowir_gen::fmt::Region) -> &mut Vec<dataflowir_gen::fmt::Block> {
    a0.get_blocks_mut()
}

// f461 -- `llvm::simple_ilist<mlir::Block>::begin()` -> t245's `Ptr<fmt::Block>`.
// ⭐ IT ALIASES, IT DOES NOT COPY -- the whole reason this row died twice.  The
// formal and body are rules/vector f13's verbatim: the converter passes a BORROW
// of the owner's `Value<Vec<T>>` (`PtrKind::StackVec(Rc::downgrade(owner))`,
// rc.rs:1047; the provenance `Ptr::borrow_vec` names at rc.rs:281), so returning
// `a0` unchanged allocates NOTHING and the 16 sites' `(*it).getArgument(0)` reads
// the live first block.  `Ptr::null()` (t245's init) would deref null here and
// `Ptr::alloc(..clone())` would fabricate a copy; neither is admissible.
fn f461(a0: libcc2rs::Ptr<dataflowir_gen::fmt::Block>) -> libcc2rs::Ptr<dataflowir_gen::fmt::Block> {
    a0
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
fn t580() -> dataflowir_gen::DialectBytecodeReader {
    dataflowir_gen::DialectBytecodeReader::new(&[1u8]).unwrap()
}

// t581 -- `mlir::DialectBytecodeWriter` -> `dataflowir_gen::DialectBytecodeWriter`.
// An empty sink with an empty attribute table.
fn t581() -> dataflowir_gen::DialectBytecodeWriter {
    dataflowir_gen::DialectBytecodeWriter::new()
}

// f480 -- `LogicalResult readAttribute(mlir::Attribute &)`, 46 sites.
// rules/support t1 models LogicalResult as `bool`, true == success.
// ⭐ `a1` IS NAMED EXACTLY ONCE, in one arm, because a `&mut` parameter's `aN`
// re-expands to the bare lvalue at the call site.
fn f480(
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
fn f481(
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
fn f482(a0: &dataflowir_gen::DialectBytecodeReader) -> u64 {
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
fn f483(
    a0: &dataflowir_gen::DialectBytecodeReader,
    a1: &Vec<u8>,
) -> libcc2rs::InFlightDiagnostic {
    a0.emit_error(&::std::string::String::from_utf8_lossy(
        &a1.iter().map(|&c| c as u8).take_while(|b| *b != 0).collect::<Vec<u8>>(),
    ))
}

// f484 -- `void writeAttribute(mlir::Attribute)`, 46 sites.  C++ takes the
// attribute BY VALUE; the model borrows, so the owned parameter is lent.
fn f484(a0: &mut dataflowir_gen::DialectBytecodeWriter, a1: dataflowir_gen::ir::Attr) {
    a0.write_attribute(&a1)
}

// f485 -- `void writeOptionalAttribute(mlir::Attribute)`, 33 sites.
// ⭐ A DIFFERENT BODY FROM f484, and it has to be: the optional form emits
// `index + 1` with 0 reserved as the null sentinel, while the bare form emits the
// index itself.  The two are DIFFERENT ON THE WIRE, so one body cannot serve both.
// `Some(&a1)` is unconditional because `ir::Attr` has no null state -- the same
// asymmetry f481 records from the reading side.
fn f485(a0: &mut dataflowir_gen::DialectBytecodeWriter, a1: dataflowir_gen::ir::Attr) {
    a0.write_optional_attribute(Some(&a1))
}

// f486 -- `int64_t getBytecodeVersion() const` on the WRITER, 12 sites.
// ⚠️ `i64`, against f482's `u64`.  See f482.
fn f486(a0: &dataflowir_gen::DialectBytecodeWriter) -> i64 {
    a0.get_bytecode_version()
}

// f487 -- `void writeSparseArray(llvm::ArrayRef<int>)`, 6 sites, keyed at the ONE
// corpus instantiation `T = int`.  ⭐ THE WRITE SIDE IS KEYABLE WHERE THE READ SIDE
// IS NOT: the writer only READS the array, so t19's by-value `ArrayRef<int> ->
// Vec<i32>` copy loses nothing, whereas readSparseArray's out-param writes would
// have been dropped into a temporary (see the refusal at t580).
fn f487(a0: &mut dataflowir_gen::DialectBytecodeWriter, a1: Vec<i32>) {
    a0.write_sparse_array(&a1)
}

// ---------------------------------------------------------------------------
// t520 / f420 / f421 -- `mlir::MutableOperandRange` AS A WRITE-THROUGH VIEW.
// See tgt_unsafe.rs at t520 for the aliasing argument and the measured
// demonstration, and src.cpp at t520 for the member census and the two
// forward-looking hazards (the dropped segment list, and the flat ODS index).
//
// ⭐ THE ONLY DIFFERENCE FROM THE UNSAFE MODEL IS THE OWNER'S SPELLING: t36 models
// `mlir::Operation *` as `libcc2rs::Ptr<fmt::OpInst>` here, so the owner slot is a
// `Ptr` and the deref in f421 goes through `Ptr::with_ref` rather than `*`.
// ⭐ `with_ref`, NOT `with`: `fmt::OpInst` is not `ByteRepr` and lives in another
// crate, so the orphan rule makes the byte-decoding accessors unusable on it.
// `with_ref`/`with_mut_ref` (libcc2rs rc.rs:593,625) are the bound-free path, and
// they mutate the OWNER'S STORAGE, which is exactly the aliasing this row needs.
// ⚠️ SAFETY IN THIS MODEL IS LIVENESS, NOT LIFETIMES: a view that outlives its op
// panics with `ub: dangling pointer` on the next access rather than reading freed
// memory.  That is the loud outcome, not a silent one.
fn t520() -> (libcc2rs::Ptr<dataflowir_gen::fmt::OpInst>, u32, u32) {
    (libcc2rs::Ptr::<dataflowir_gen::fmt::OpInst>::null(), 0, 0)
}

fn f420(
    a0: libcc2rs::Ptr<dataflowir_gen::fmt::OpInst>,
    a1: u32,
    a2: u32,
    a3: Vec<(u32, (::std::string::String, dataflowir_gen::ir::Attr))>,
) -> (libcc2rs::Ptr<dataflowir_gen::fmt::OpInst>, u32, u32) {
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

fn f421(
    a0: libcc2rs::Ptr<dataflowir_gen::fmt::OpInst>,
) -> (libcc2rs::Ptr<dataflowir_gen::fmt::OpInst>, u32, u32) {
    let __owner = a0;
    let __n: usize = __owner.with_ref(|op| op.operands.values().map(|g| g.len()).sum());
    (__owner, 0u32, __n as u32)
}

// f500 -- `mlir::BlockArgument mlir::Block::getArgument(unsigned)` (Block.h:139)
// -> `fmt::Block::get_argument()`.  THE LAST LINK in the chain t560/t561/f460/f461
// built, and the one that was still SILENT: an unmapped MEMBER is emitted
// TEXTUALLY, rc=0, with no placeholder token, so the six sites on
// `DataTransferLowering.cpp` survived the placeholder going 8 -> 0.  See src.cpp for
// the sites and the signature argument.
//
// ⭐ IDENTICAL TO THE UNSAFE MODEL HERE, and that is expected rather than sloppy:
// the receiver is a C++ `mlir::Block &`, which is `&mut fmt::Block` in BOTH models
// (f460's receiver is the same shape), and the result is a by-value `ir::Value` in
// both.  Only the ITERATOR that PRODUCES the receiver differs between the models
// (t245 / f461: `*mut fmt::Block` here, `libcc2rs::Ptr<fmt::Block>` there), and that
// difference is already absorbed before this call.
//
// ⚠️ THE `clone()` IS THE C++ SIGNATURE: Block.h:139 returns `BlockArgument` BY
// VALUE.  `fmt::Block::get_argument` returns `&Value` into the live `args` on purpose
// -- an accessor that copied internally would silently drop a write -- so the copy
// happens here, at the boundary where C++ copies the handle too.  ⛔ The widening it
// costs is t35's pre-existing one (`ir::Value` is compared by content, an MLIR
// `BlockArgument` by identity); all six sites only READ, so nothing is lost today,
// and `getArgNumber`/`getOwner` stay unmapped and loud.
// ⚠️ `a0`/`a1` each named EXACTLY ONCE; `a0` carries nothing but a method call.
// ⛔ Out of range PANICS in the accessor rather than fabricating a `Value`.
fn f500(a0: &mut dataflowir_gen::fmt::Block, a1: u32) -> dataflowir_gen::ir::Value {
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
fn f520(
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
fn f521(
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
fn f522(
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
fn f523(
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
fn f524(
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
fn f525(
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
fn f526(
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
fn f527(
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
fn f528(
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
fn f529(
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
fn f530(
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
fn f531(
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
// and tgt_unsafe.rs at f540.  Identical arithmetic in this model; only the owner
// handle's type differs (`Ptr` rather than `*mut`), and neither body derefs it.
fn f540(a0: (libcc2rs::Ptr<dataflowir_gen::fmt::OpInst>, u32, u32)) -> u32 {
    a0.2
}

fn f541(
    a0: (libcc2rs::Ptr<dataflowir_gen::fmt::OpInst>, u32, u32),
    a1: u32,
    a2: u32,
    a3: Option<(u32, (::std::string::String, dataflowir_gen::ir::Attr))>,
) -> (libcc2rs::Ptr<dataflowir_gen::fmt::OpInst>, u32, u32) {
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
    let __view: &(libcc2rs::Ptr<dataflowir_gen::fmt::OpInst>, u32, u32) = &a0;
    (__view.0.clone(), __view.1 + a1, a2)
}

// ===========================================================================
// t700/t701 + f600/f601 -- `llvm::cl::initializer<bool>` / `<int>` and
// `llvm::cl::init<Ty>`.  ⭐ IDENTICAL TO THE UNSAFE MODEL, and for a reason that
// is worth stating rather than leaving to the reader: the two models differ only
// where a REPRESENTATION differs (a raw `*mut` vs a `Ptr`, a `Vec<libc::c_char>`
// vs a `Vec<u8>`), and `initializer<Ty>`'s payload here is a PRIMITIVE -- `bool`
// and `i32` are the same type in both models, hold no pointer and need no
// refcount.  t78 differs across the models only because its payload is a
// `StringRef`; this row's is not.  The full argument is at `using t700 =` in
// src.cpp.
fn t700() -> bool {
    false
}

fn t701() -> i32 {
    0
}

// f600 -- `llvm::cl::init<bool>(const bool &)`.  IDENTITY, by value.
fn f600(a0: &bool) -> bool {
    *a0
}

// f601 -- `llvm::cl::init<int>(const int &)`.
fn f601(a0: &i32) -> i32 {
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
fn f620(a0: &mut ()) {
    ()
}

// f560 -- GENERIC detail::DenseArrayAttrImpl<T1>, i.e. DenseI32ArrayAttr (12 asks
// on Ktdp/KtdpOps.cpp) and DenseI64ArrayAttr (8).  `T1` is unused: both map to
// `ir::Attr`.  See the src-side comment for why a concrete key cannot work.
fn f560<T1>(
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
// t730 `mlir::IntegerType` -> `ir::Ty` (ir.rs:37).  ⭐ THE WIDTH SURVIVES:
//   `Ty::Int(u32)` (ir.rs:39) carries a width and prints `i{w}` (ir.rs:54), so
//   this is the t42/t75 WIDENING and NOT a width erasure.
//   ⛔ Losses: (1) NO SIGNEDNESS (`Ty::Int` has no such field) -- hence f630 keys
//   only the 1-arg overload and `IntegerType::get` stays unkeyed; (2) NULL-HANDLE
//   init rather than a real empty integer type, deliberately NOT `Ty::Int(0)`,
//   which would print `i0`; (3) NO accessor mapped.
fn t730() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(String::new())
}

// f630 -- `Builder::getIntegerType(unsigned width)`, the 1-arg overload only.
// `build.rs:546 pub fn get_integer_type(&self, width: u32) -> Ty`.  `a0` is
// BORROWED, not moved: the refcount model's receiver is not `Copy`, and a moved
// receiver is E0382 at rc=0 with no placeholder token to show it.
fn f630(a0: &dataflowir_gen::OpBuilder, a1: u32) -> dataflowir_gen::ir::Ty {
    a0.get_integer_type(a1)
}

// t590 / t591 -- `mlir::DialectAsmPrinter` and `mlir::DialectAsmPrinter &`.  See the
// t590 block in `src.cpp`; same model as t462/t463, and the null handle is the
// faithful "no printer" value here for the same reason it is there.
fn t590() -> dataflowir_gen::AsmPrinter {
    dataflowir_gen::AsmPrinter::new()
}

fn t591() -> libcc2rs::Ptr<dataflowir_gen::AsmPrinter> {
    libcc2rs::Ptr::<dataflowir_gen::AsmPrinter>::null()
}

// t750 `mlir::detail::ShapedTypeTrait<mlir::MemRefType>` -> `ir::Ty`, 4 sites.
//   THE CRTP BASE of the same object t73 maps, so it maps to what t73 maps to --
//   the t560/t561 "the base IS the same container" discipline.  No new model
//   claim.  Init is t73's own `Ty::Opaque("")` null sentinel.
fn t750() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(String::new())
}

// t751 `mlir::detail::ShapedTypeTrait<mlir::VectorType>` -> `ir::Ty`, 3 sites.
//   Same, over t75.
fn t751() -> dataflowir_gen::ir::Ty {
    dataflowir_gen::ir::Ty::Opaque(String::new())
}

// f650 `ShapedTypeTrait<MemRefType>::getRank() const` -> THE SHAPE LENGTH.
//   BuiltinTypeInterfaces.h.inc:604-608 is `assert(hasRank()); return
//   getShape().size();` and `Ty::MemRef(Vec<i64>, Box<Ty>)` (ir.rs:44) IS that
//   shape.  ⛔ The fallback arm is the C++ `assert(hasRank())`, not a placeholder.
//   ⚠️ `a0` is a BORROW, not a move: `ir::Ty` is a plain enum here but the
//   receiver is read-only and naming it once keeps the f122 read shape.
fn f650(a0: &dataflowir_gen::ir::Ty) -> i64 {
    match a0 {
        dataflowir_gen::ir::Ty::MemRef(shape, _) => shape.len() as i64,
        _ => panic!("ub: cannot query rank of unranked shaped type"),
    }
}

// f651 `ShapedTypeTrait<VectorType>::getNumElements() const` -> THE SHAPE PRODUCT.
//   BuiltinTypeInterfaces.h.inc:609-612, the product of the dimensions, with a
//   NEGATIVE dim = dynamic `?` (ir.rs:56-60) panicking as the C++ assert aborts.
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

// t770 `mlir::detail::SymbolOpInterfaceTrait<mlir::func::FuncOp>` -> `fmt::OpInst`.
//   See tgt_unsafe.rs.  Byte-identical to the unsafe target for t157's reason: an
//   `fmt::OpInst` is a VALUE in both targets, so there is no reference shape to differ on.
fn t770() -> dataflowir_gen::fmt::OpInst {
    dataflowir_gen::fmt::OpInst::new(
        <dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as dataflowir_gen::MlirOp>::DEF,
    )
}

// f670 `SymbolOpInterfaceTrait<func::FuncOp>::getName()` -> THE SYMBOL NAME
//   (`attrs["sym_name"]`).  See tgt_unsafe.rs for the header chain, the NUL-terminated
//   `Vec<libc::c_char>` StringRef model and why the missing-attribute arm panics.
//   ⭐ `a0` is BORROWED, not moved: in refcount a receiver must never be moved out of
//   (`libcc2rs::Ptr` is not `Copy`), and a read-only accessor has no reason to.
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
// Writes go through `with_mut` (the rules/raw_ostream f4 spelling) and every body
// returns its own `a0` so `p << a << b` chains.

// f750 -- `mlir::AsmPrinter &` + `const char (&)[_]`, 23 sites.  f360's body on the
// base receiver: a char array reaches this model as `&[u8]` INCLUDING the literal's
// NUL terminator, so the bytes are cut at the first NUL -- `print_str` appends
// verbatim and would otherwise plant a 0x00 mid-text.
fn f750(a0: libcc2rs::Ptr<dataflowir_gen::AsmPrinter>, a1: &[u8]) -> libcc2rs::Ptr<dataflowir_gen::AsmPrinter> {
    let __p = a0;
    let __n = a1.iter().position(|&c| c == 0u8).unwrap_or(a1.len());
    let __s = String::from_utf8_lossy(&a1[..__n]).into_owned();
    __p.with_mut(|__q: &mut dataflowir_gen::AsmPrinter| {
        __q.print_str(&__s);
    });
    __p
}

// f751 -- `mlir::AsmPrinter &` + `const long &`, 3 sites.  f29's `&i64` spelling;
// `print_i64` is `v.to_string()`, i.e. plain decimal, which is what MLIR's
// `getStream() << value` produces.
fn f751(a0: libcc2rs::Ptr<dataflowir_gen::AsmPrinter>, a1: &i64) -> libcc2rs::Ptr<dataflowir_gen::AsmPrinter> {
    let __p = a0;
    let __v = *a1;
    __p.with_mut(|__q: &mut dataflowir_gen::AsmPrinter| {
        __q.print_i64(__v);
    });
    __p
}

// f752 -- `mlir::AsmPrinter &` + `mlir::Type` by value, 2 sites.  t5 is
// `dataflowir_gen::ir::Ty`; `print_type` takes it by reference, so the by-value
// parameter is borrowed in place and nothing is cloned.
fn f752(a0: libcc2rs::Ptr<dataflowir_gen::AsmPrinter>, a1: dataflowir_gen::ir::Ty) -> libcc2rs::Ptr<dataflowir_gen::AsmPrinter> {
    let __p = a0;
    __p.with_mut(|__q: &mut dataflowir_gen::AsmPrinter| {
        __q.print_type(&a1);
    });
    __p
}

// f753 -- `mlir::OpAsmPrinter &` + `mlir::Type` by value, 17 sites.  f752's body on
// the derived receiver; both receivers are the same `AsmPrinter` sink in this model.
fn f753(a0: libcc2rs::Ptr<dataflowir_gen::AsmPrinter>, a1: dataflowir_gen::ir::Ty) -> libcc2rs::Ptr<dataflowir_gen::AsmPrinter> {
    let __p = a0;
    __p.with_mut(|__q: &mut dataflowir_gen::AsmPrinter| {
        __q.print_type(&a1);
    });
    __p
}
