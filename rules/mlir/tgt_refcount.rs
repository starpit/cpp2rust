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
