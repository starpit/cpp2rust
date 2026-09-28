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
// For `std::initializer_list`, which appears in the ArrayRef initializer-list
// constructor key (f55) that row g325's `ArrayRef({num_ports, num_ports})`
// call site reaches.
#include <optional>   // std::nullopt_t, f133`s parameter (rules/optional t4 -> `()`)
#include <initializer_list>
// For `std::unique_ptr` / `std::default_delete`, which appear inside the
// fully-spelled `RegionRange` range-base key below.
#include <memory>
// For `std::string`, which appears inside the ListOption key (t67).  The key
// printer renders it `std::string`, not `std::__1::basic_string<...>`: verified,
// 55 recorded keys across the published IR tree spell it that way.
#include <string>
// For `std::vector`, which appears in f141's ArrayRef(const std::vector<T> &)
// constructor key.  The readback spells it `const std::vector<llvm::StringRef> &`,
// with the allocator argument default-suppressed on BOTH sides.
#include <vector>

// FORWARD DECLARATION ONLY, and only so that `mlir::RegionRange`'s own
// constructor (f142) can be SPELLED inside `namespace mlir` below -- the real
// definition and the `t46` mapping of this template are further down, in
// `namespace llvm`.  A forward declaration maps nothing and records nothing on
// its own (same inert status as the `class StringRef {}` declaration that exists
// purely so the InFlightDiagnostic `<<` keys can be spelled).
namespace llvm {
template <typename T> class MutableArrayRef;
// ⭐ AND `ArrayRef`, for the SAME reason and a second consumer: `mlir::ValueRange`
// (:270) declares `ValueRange(llvm::ArrayRef<Value>)` -- key f145's sibling f144 --
// and the real `llvm::ArrayRef` definition in this file does not open until :1117,
// i.e. AFTER namespace mlir.  Without this line the rule source does not compile
// (`no template named 'ArrayRef' in namespace 'llvm'`), which is a regen failure,
// not a converter gap.
template <typename T> class ArrayRef;
// ⭐ AND `APInt`, COMPLETE-BUT-EMPTY, purely so `mlir::IntegerAttr::getValue()`
// (f146) can be SPELLED with its real BY-VALUE return type.  A by-value return
// needs a COMPLETE type in the DEFINITION of f146, so unlike `raw_ostream` at
// :3418 this one cannot be left incomplete.  It declares no member and no `using
// tN =`, so it maps nothing and records nothing in THIS module -- `rules/apint`
// owns `llvm::APInt` (its t2, the width-carrying `libcc2rs::APInt { bit_width,
// value }`), and the converter resolves the type key by NAME across modules,
// exactly as the `llvm::StringRef` parameters of f22-f24 resolve to
// rules/stringref.  ⛔ Do NOT add `using tN = llvm::APInt` here: that would be a
// DUPLICATE key against rules/apint t2.
class APInt {};
// ⭐ AND `StringRef` / `Twine`, FORWARD ONLY, for the f400-f406 row: the
// `mlir::Builder` attribute factory (`class Builder` at :978, inside `namespace
// mlir`) declares `getNamedAttr(StringRef, Attribute)` and
// `getStringAttr(const Twine &)` with the REAL parameter types from
// `mlir/IR/Builders.h`, and the real `class StringRef {}` / `class Twine {}`
// definitions in this file do not open until :1242/:1255 -- i.e. AFTER
// `namespace mlir` closes.  A function DECLARATION may name an incomplete
// parameter type, so forward declarations are enough here; the f400-f406
// DEFINITIONS sit at the end of the file where both are complete.  ⛔ The
// parameter types must be the header's own (`StringRef`, `const Twine &`) and
// not a convenient substitute: the recorded src key carries them, so keying
// `getStringAttr(StringRef)` against a corpus call that resolves to
// `getStringAttr(const Twine &)` is a DEAD key -- silently, at rc=0.
// They declare no member and no `using tN =` here, so they map nothing;
// rules/stringref t1 and rules/twine t1 own them (both `Vec<libc::c_char>` in
// the unsafe overlay, `Vec<u8>` in the refcount one).
class StringRef;
class Twine;
} // namespace llvm

namespace llvm {
// Forward declarations only, so that `mlir::Region::getBlocks()`'s RETURN TYPE
// (`llvm::iplist<mlir::Block> &`, Region.h:44-45) can be SPELLED on the class
// below.  Real LLVM declares both as `template <typename T, class... Options>`;
// the empty pack prints nothing, so a single-parameter declaration renders the
// identical spelling `llvm::simple_ilist<mlir::Block>` that the queue records
// (g074 `searched as:`) -- and unlike a defaulted parameter a pack cannot be
// dropped by `SuppressDefaultTemplateArgs`.  The keys and the two member
// deductions are at the BOTTOM of this file (t560/t561, f460/f461); they cannot
// live here because `llvm::ilist_iterator` is not declared until t243.
template <typename T> class simple_ilist;
template <typename T> class iplist;
} // namespace llvm

namespace mlir {

class Operation;

class Block {};

// ⚠️ `getBlocks()` IS DECLARED HERE AND NOT LATER, because a C++ class cannot be
// reopened -- see f460 at the bottom of this file for the whole deduction.
// mlir/IR/Region.h:45 `BlockListType &getBlocks() { return blocks; }`, with
// `using BlockListType = llvm::iplist<Block>;` at :44.  There is NO const
// overload in Region.h, so this one declaration is the entire `getBlocks`
// surface and the corpus's non-const asks can only resolve to it.
class Region {
public:
  llvm::iplist<Block> &getBlocks();
};

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
//
// ⭐ `getValue()` IS NOW DECLARED -- the ONE member of this class that is, and the
// note further down that says "sound only because NO ACCESSOR IS MAPPED" is
// narrowed accordingly (see f146).  It became keyable only when `rules/apint`
// landed a WIDTH-CARRYING `llvm::APInt` model (t2, commit 78a49f21); before that
// the return type had no key and a rule here could not have been written without
// inventing a width.
class IntegerAttr {
public:
  // mlir/include/mlir/IR/BuiltinAttributes.h.inc -- `::llvm::APInt getValue()
  // const`.  BY VALUE, which is what makes it keyable at all: a member returning
  // a REFERENCE out of a by-value receiver is silently dangling under the
  // refcount model (the reason `std::string_view::front()` and
  // `llvm::SMLoc::getPointer()` were both refused).  Confirmed by readback --
  // the recorded key is `llvm::APInt mlir::IntegerAttr::getValue() const`, with
  // no `&`.
  llvm::APInt getValue() const;
};

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
// ⭐ THE THREE CONSTRUCTORS ARE DECLARED HERE, not just keyed at f143-f145: this
// is a STUB class, so an undeclared constructor is a compile error in the rule
// source itself (the f142 / `RegionRange` shape at :303).  The COPY constructor is
// deliberately NOT declared -- see f143's paragraph for why (written, measured
// 7 -> 7 sites, deleted).  `ValueRange(ArrayRef<Value> = {})`'s default argument is
// omitted on purpose: the recorder writes a default out at the call site, so a
// nullary variant records as the SAME key.
class ValueRange {
public:
  ValueRange(OperandRange values);
  ValueRange(llvm::ArrayRef<Value> values);
  ValueRange(std::vector<Value> &values);
};

// ⛔ REFUSED, RECORDED SO THE NEXT SLOT DOES NOT RE-DERIVE IT:
// `mlir::MutableOperandRange` IS NOT MAPPED, AND THE OWNING-`Vec` PRECEDENT THAT
// t14 (`mlir::OperandRange`, immediately above) SETS DOES NOT EXTEND TO IT.
// `OperandRange` is a READ-ONLY borrowed view, so copying it into an owning Vec
// loses only aliasing.  `MutableOperandRange` (ValueRange.h:118) is a
// WRITE-THROUGH handle over the OWNER OP's operand list -- `assign()`/`append()`/
// `erase()` on it MUTATE THE OP.  The corpus stores two of them BY VALUE as
// long-lived members and then writes through them:
//   dcc/src/Transform/Sentient/AddressPinningAndToggle.cpp:1043-1044
//     `MutableOperandRange mutable_addr_`, `MutableOperandRange immutable_addr_`
//   ... then `mutable_addr_.assign(...)`   at :1790 and :1877
//   ... and  `immutable_addr_.assign(...)` at :1903
// Under an owning-`Vec` model every one of those four writes mutates a PRIVATE
// COPY and the operand rewrite NEVER REACHES THE OP: rc=0, it compiles, and the
// transform silently does nothing.  That is strictly worse than today's loud
// abort, which is why this is a refusal and not a cheap key.
// A FAITHFUL model is `(owner op, start, length)` indexing into the op's operand
// storage, i.e. it needs OPERAND-LIST SURFACE in the IR model that
// `dataflowir_gen` does not have today -> this is a dataflowir-gen row, not a
// rules row.  Same shape of argument as the t25/OpState and OwningOpRef
// refusals: the C++ type is a HANDLE and the available Rust model is a VALUE.

// mlir/include/mlir/IR/Region.h -- a view over an op's regions.
//
// Region.h:342-356 gives `RegionRange` ITS OWN constructor,
// `RegionRange(MutableArrayRef<Region> regions = {})`, which is why this one is
// writable where its sibling is not: `mlir::OperandRange` declares NO
// constructor at all (`using RangeBaseT::RangeBaseT;`) and its live fabricated
// spelling turned out to be a hybrid -- the derived receiver scope with the BASE
// constructor name -- so it is deliberately LEFT FABRICATING (its iterator
// parameter type has no model, so the argument would arrive unmodelled).
//
// The default argument is NOT restated here: a defaulted function argument is
// not part of the recorded signature, and the recorder writes the default out
// explicitly at the call site, so `RegionRange()` and `RegionRange(marr)` record
// as the SAME single key.  Writing a second, nullary entry would be silently
// redundant (the rules/string `substr` precedent).  No other member is declared,
// because no other member is mapped.
class RegionRange {
public:
  RegionRange(llvm::MutableArrayRef<Region> regions);
};

// mlir/include/mlir/IR/OperationName.h -- a HANDLE to the registered operation
// info (`Impl *`, nullable, uniqued per name).  dataflowir_gen's
// `TdOpDef` (generated, re-exported at lib.rs:72) is exactly that record: it
// carries def_name/base/mnemonic/traits and the ODS `arguments` -- which is what
// `OperationName::getAttributeNames()` reads.  The handle is therefore
// `Option<&'static TdOpDef>`, and `None` is the null handle a
// default-constructed `OperationName` is.  No MEMBER is mapped, so
// `getAttributeNames()` still aborts loudly rather than lying.
class OperationName {};

// mlir/include/mlir/IR/OperationSupport.h:525 --
// `class RegisteredOperationName : public OperationName`.  A REGISTERED op name
// IS the .td-parsed op def, so the faithful model for the derived type is THE
// SAME Rust type as t18 with an EMPTY member set.
// WHY IT IS NEEDED: in the three TUs it gates (VectorChain.cpp, KTDFOps.cpp,
// Uniform.cpp) the type is never NAMED in source at all -- it is reached as a
// LEAF record while mapping `std::optional<mlir::RegisteredOperationName>`,
// i.e. purely the declared return type of `getRegisteredInfo()`
// (OperationSupport.h:186-189), with NOTHING read from it.
// DELIBERATELY NOT KEYED, each for a stated reason:
//  * `==`/`!=` -- the t72 TypeID precedent, and `operator==` is declared on
//    OperationName, so a derived-class key would be DEAD anyway.
//  * `getCanonicalizationPatterns` -- INHERITED, and a key on a derived class
//    cannot relocate an inherited member (measured on llvm::FailureOr: six keys
//    FOUND, all six DEAD).  Its one call site still fails at rustc, loudly.
//  * `getDialect`, `lookup`, `insert`, `getFromOpaquePointer` -- never called
//    anywhere in the corpus.
// No destructor exists (no `~RegisteredOperationName`, no `~OperationName`), so
// the handle model is permitted.  NO CONSTRUCTOR KEY: t18 itself is committed
// without one and the type is not default-constructible in C++ -- every value
// comes from `lookup()` / `getRegisteredInfo()` / `getRegisteredOperations()`.
class RegisteredOperationName : public OperationName {};

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
// ⭐ UPDATE 2026-09-27 (fourth slot): `MemRefType` IS NOW A KEY IN ITS OWN RIGHT
// -- t73, 16 TUs (queue g053, `searched as: mlir::MemRefType`).  Its DEFAULT
// CONSTRUCTOR is declared here and keyed as f42; without the ctor the type key
// alone gives rc=0 and then E0433.  The paragraph above still holds for the
// OTHER names in this block.
class MemRefType {
public:
  MemRefType();
};
// ⭐ UPDATE 2026-09-28 (mlir slot): `RankedTensorType` IS NOW A KEY IN ITS OWN
// RIGHT -- t164, 15 queue rows (g2375 etc., `searched as: mlir::RankedTensorType`).
// EXACTLY the t73/MemRefType move one paragraph up: the empty tag existed only so
// t32's `TypedValue<mlir::RankedTensorType>` could be SPELLED, and a spelling is
// not a key.  Its DEFAULT CONSTRUCTOR is declared here and keyed as f137, for the
// f42 reason: a `using tN =` alone gives rc=0 and then E0433.
class RankedTensorType {
public:
  RankedTensorType();
};
namespace ktdf {
class TokenType {};
class FifoSlotType {};
// The four ODS-generated `mlir::ktdf` op classes named by t300-t304.  Each is
// `Operation *` through its `OpState` base, declared for the t161 reason and nothing more.
// ⭐ THE REFUSAL AT :4507 THAT KEPT THESE LOUD IS FALSIFIED, and its own test is the proof:
// it recorded `grep -cE "struct mlir_ktdf_<Op>\b"` against
// dataflowir-gen-654e676bccb4a05f/out/dataflow_ods.rs as **0 for all three** and concluded
// `mlir::ktdf` was a WHOLE-DIALECT `.td` coverage gap.  Re-run 2026-09-28, that identical
// grep returns **1 for all four**: `dataflow_ods.rs:4826` reads `pub struct
// mlir_ktdf_PipelineOp;` followed by `impl MlirOp for mlir_ktdf_PipelineOp { const DEF:
// &'static TdOpDef = &super::ods_more::TD_OPS_MORE[93]; }`, inside `pub mod ops` (:4608), and
// the op is registered by name at :5190.  StageOp / PrivateOp / PrivateYieldOp are the same
// shape.  The DEFs are carried by the `ods_more` / `TD_OPS_MORE` table, which POSTDATES that
// refusal -- so the `.td` gap was real when it was written and is CLOSED now, and these
// spellings are ordinary keyable cast targets rather than a coverage gap to preserve.
// ⛔ The refusal's OTHER half STILL STANDS and is NOT touched here: the four
// `MultiOperandTraitBase` spellings whose second argument is `NOperands<N>::Impl` /
// `AtLeastNOperands<N>::Impl` stay LOUD, because the mangler ERASES the non-type `N`
// (`..._NOperands____Impl_`) and a guessed arity is a different type that silently never
// matches.  That reason is mechanical and unaffected by the ODS table growing.
class PipelineOp {};
class StageOp {};
class PrivateOp {};
class PrivateYieldOp {};
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

// mlir/Pass/AnalysisManager.h:30 -- `class PreservedAnalyses`, whose ONLY member
// is `SmallPtrSet<TypeID, 2> preservedIDs`.  Declared with ONLY the default
// constructor (implicit: AnalysisManager.h declares none, so `PreservedAnalyses()`
// is the one ported code can reach); preserveAll/isAll/isNone/preserve/isPreserved
// are deliberately NOT declared because none is mapped (see t80).
class PreservedAnalyses {
public:
  PreservedAnalyses();
};

} // namespace detail

namespace scf {
// mlir/include/mlir/Dialect/SCF/IR/SCF.h -- `scf::ForOp`, an ODS-generated op
// class, i.e. an `OpState` subclass wrapping one `Operation *`.  Maps where
// `mlir::Operation`/`OpState` map, `fmt::OpInst`, and carries the SAME
// prohibition: no equality, for the handle-vs-value reason given on OpState.
class ForOp {};

// mlir/include/mlir/Dialect/SCF/IR/SCF.h -- `scf::IfOp`, an ODS-generated op
// class exactly like `scf::ForOp` above, i.e. an `OpState` subclass wrapping one
// `Operation *`.  Maps where ForOp maps, `fmt::OpInst`, and carries the SAME
// prohibition: no equality, no member.  UNLIKE ForOp it DOES need a default
// constructor -- see f129 and the grep recorded at t159.  Declared `{}` exactly
// like func::CallOp (t152), whose IMPLICIT default constructor is what f121 keys.
class IfOp {};
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
// ⭐ UPDATE 2026-09-27 (fourth slot): `VectorType` IS NOW A KEY IN ITS OWN RIGHT
// -- t75, 16 TUs (queue g055, `searched as: mlir::VectorType`).  Its DEFAULT
// CONSTRUCTOR is declared here and keyed as f44.
class VectorType {
public:
  VectorType();
};
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
// `mlir::Location` / `mlir::FileLineColLoc` -- mlir/IR/Location.h:76 and :174.
// Declared here ONLY so the keys below can be spelled; the model is argued at
// each `using tN =`.  `FileLineColLoc` is a C++-only VIEW over a
// `FileLineColRange` of exactly one line and one column, which is why it is a
// SEPARATE type here and not a synonym for Location.
class Location {};

class FileLineColLoc {
public:
  // `unsigned getLine() const` / `unsigned getColumn() const` (Location.h:180).
  // ⛔ `getFilename()` IS DELIBERATELY NOT DECLARED -- see the absence note at
  // f124/t155: in C++ it returns `mlir::StringAttr`, and the Rust model returns
  // `&str`, so the key's RETURN types do not correspond and I will not guess a
  // StringAttr route for it.
  unsigned getLine() const;
  unsigned getColumn() const;
};

// `mlir::Builder` is declared INCOMPLETE apart from ONE member, and the exception
// is argued at f124: `getUnknownLoc()` is declared by `Builder` in the real header
// (mlir/IR/Builders.h), NOT by `OpBuilder`, so an `OpBuilder` receiver keys as
// `mlir::Builder::getUnknownLoc()` -- the inherited-member rule means declaring it
// on OpBuilder would produce a DEAD key.  Everything t59's note says about why no
// OTHER Builder member is declared still holds: all 102 TUs reach the type through
// the bare forward declaration at mlir/IR/AffineMap.h:37:7 and no TU names Builder
// itself.
//
// ✅ f124 IS ALIVE AND REACHED -- SETTLED BY PROBE 2026-09-28, so do not re-litigate
// the keyed-on-`Builder` choice.  An `OpBuilder` LVALUE receiver really does key as
// `mlir::Builder::getUnknownLoc()`; the declaring-class reasoning above is correct and
// the `llvm::FailureOr` hazard (six keys FOUND, all six DEAD) does not bite here.
// Readback, four identical occurrences:
//     search expr mlir::Location mlir::Builder::getUnknownLoc(), result:
//     Matching: mlir::Location mlir::Builder::getUnknownLoc()
//       param a0: &()
//       return: dataflowir_gen::ir::Location
//       text: "dataflowir_gen::ir::Location::Unknown"
// The probe is `probe/mlirf124/` -- `class Builder { Location getUnknownLoc(); };
// class OpBuilder : public Builder {};` restated in a `CC2_INC` include dir, NEVER in
// the main file (which would make it `IsUserDefinedDecl` and measure nothing), with
// the builder taken as a REFERENCE parameter so `OpBuilder` need not be constructible
// or mapped -- it renders as `Cpp2RustUnmapped_mlir_OpBuilder`, which is recoverable
// and does not block the call site.  Two earlier attempts had failed for reasons
// unrelated to the key: `Splat.cpp` timed out at 200 s under `--verbose`, and
// `EnsureDeviceDeclaration.cpp` aborts earlier on `std::unique_ptr<llvm::MemoryBuffer>`
// so its call site is never visited.
//
// ✅ t155 IS ALSO REACHED, with f122/f123 alongside it -- converted result, both
// models rc=0, from the same probe extended with
// `fl.getLine() * 1000u + fl.getColumn()`:
//     unsafe:   (((*fl).line()).wrapping_mul(1000_u32)).wrapping_add((*fl).column())
//     refcount: same, through `(*fl.upgrade().deref())`
// And `getLine()`/`getColumn()` are declared by `FileLineColLoc` ITSELF
// (mlir/IR/Location.h:183-184), not by the `FileLineColRange` base, so the
// inherited-member trap does not apply to f122/f123 either.
// ⛔ RETRACTED 2026-09-28.  An earlier version of this note said the four follow-ons --
// `as_file_line_col` / `is_strict_file_line_col` / `find_file_line_col` /
// `find_file_line_col_or_unknown` -- were now "LICENSED" because t155/f122/f123 had been
// proven reached.  THAT INFERENCE WAS WRONG, and two slots measured why.  "The model
// provides it" and "t155 is reached" do not add up to "a key would be reached":
//
//  * `is_strict_file_line_col` -- `grep -rn isStrictFileLineColLoc` over the corpus is
//    ZERO hits.  A key would be DEAD.
//  * `find_file_line_col_or_unknown` -- `findInstanceOfOrUnknown` is ZERO hits.  DEAD.
//  * `find_file_line_col` -- ONE site, dataflow-scheduler/lib/Analysis/Utils.cpp:36,
//    `op->getLoc()->findInstanceOf<mlir::FileLineColLoc>()`.  ⛔ IT IS UNKEYABLE, for the
//    SAME reason `hasEffect<Effect>()` is ruled out at t156: `findInstanceOf` is declared
//    `template <typename T> T findInstanceOf()` on `LocationAttr` (Location.h:41-54) with
//    NO function parameters, so the location type asked about appears ONLY as an explicit
//    template argument, which is not part of the recorded signature.
//    `findInstanceOf<FileLineColLoc>`, `<NameLoc>`, `<CallSiteLoc>` and `<FusedLoc>` would
//    all collapse into ONE key, and any single body would answer for kinds the C++
//    distinguishes -- silent wrongness.  (Same defect as rules/variant's `get<0>`/`get<1>`,
//    queue row c004.)  Independently, the returns do not correspond: the model returns
//    `Option<FileLineColLoc>` while the C++ returns a nullable `FileLineColLoc` that the
//    site converts to bool and then calls `.getLine()` on.
//  * `as_file_line_col` -- 3 sites, all `mlir::dyn_cast<FileLineColLoc>(loc)`, and NO rules
//    module anywhere keys a `dyn_cast` free template function.  No precedent for the spelling.
//
// ⛔ AND `mlir::Location::operator->` IS NOT AN UNLOCK, though it looks like one.  Its real
// signature is `LocationAttr *operator->() const` (Location.h:88); the model has NO
// `LocationAttr` at all -- `dataflowir_gen::ir::Location` IS the attribute, with `walk`,
// `find_file_line_col`, `find_file_line_col_or_unknown` and `as_file_line_col` as inherent
// methods (ir.rs:757, 776, 790, 725) -- so `operator->` would be the IDENTITY.  But its
// entire corpus consumer set is four sites and NONE is keyable: Utils.cpp:36
// (`findInstanceOf`, above), dcc/src/Utils/Utils.cpp:48 (`loc->walk([&](Location){...})`,
// needing LocWalkResult plus a closure, out of scope below), and
// ExpressionEvaluatorUtils.cpp:467,469 (`getLoc()->dump()`, no model).  So keying it would
// record a DEAD key.
//
// ⚠️ AND Utils.cpp:36 IS NOT EVEN BLOCKED -- it is SILENTLY MISTRANSLATED, which is a
// CONVERTER row, not ours.  That TU reaches rc=0 at 58 lines, and the emitted Rust uses
// `file_loc` twice with NO `let file_loc = ...` anywhere: the `if`-with-init-statement's
// declaration is DROPPED ENTIRELY because its initializer is unmapped, and the condition
// becomes a cast-and-call of an undeclared name.  The log is ZERO BYTES.  There are no
// `Cpp2RustUnmapped` / `todo!` / `UNSUPPORTED` tokens either, so NEITHER the placeholder
// census NOR a bucket census can see it.  No rules/mlir key can repair it.
//
// Still out of scope for their own reasons: `getFilename()` (C++ returns
// `mlir::StringAttr`, the model returns `&str`; the return types do not correspond, so
// it needs a DECISION not a key) and `Location::walk` + `LocWalkResult`.
//
// A trap for whoever writes them: `pin/cpp2rust` invoked DIRECTLY gives
// `rc=127 libclang-cpp.so.22.1: cannot open shared object file` unless you
// `source /home/agent/work/env.sh` first.  That is a missing `LD_LIBRARY_PATH`, not a
// result about your key.
class Builder {
public:
  Location getUnknownLoc();
  // ⭐ THE ATTRIBUTE/TYPE FACTORY HALF OF `mlir::Builder` (Builders.h:53-220),
  // added 2026-09-28 for f400-f406.  `dataflowir-gen` commit 2f78cb6 built the
  // model these name (`src/build.rs:488-561`, `pub struct Builder` with
  // `get_named_attr` / `get_dictionary_attr` / `get_bool_attr` /
  // `get_string_attr` / `get_integer_attr` / `get_integer_type` /
  // `get_str_array_attr` / `get_i64_array_attr`, all `&self`), which VOIDS the
  // refusal recorded at `using t440 =` below -- that refusal's stated reason was
  // "build.rs models only the INSERTION half", and it now models both.
  //
  // ⭐ WHY THEY GO HERE AND NOT ON `OpBuilder`.  Every one of the 148 corpus
  // sites is `builder.getX(...)` / `odsBuilder.getX(...)` on an `OpBuilder` or
  // `ImplicitLocOpBuilder` receiver, and in real MLIR all of them RESOLVE to
  // `mlir::Builder::getX` through `class OpBuilder : public Builder`.  Declaring
  // them on the BASE reproduces that resolution exactly, so whether the
  // converter keys on the receiver's static type or on the declaring class, this
  // rule and the corpus site compute the SAME key -- and ONE set of keys covers
  // BOTH receivers instead of two near-duplicate sets.  (`OpBuilder` is given
  // its `: public Builder` base at :1019 for this reason; before this row it had
  // no base at all, which is why the member census found these 148 unmapped.)
  //
  // ⛔ `getIntegerType` IS DELIBERATELY ABSENT.  The model has
  // `get_integer_type(width: u32) -> ir::Ty`, but C++ returns
  // `mlir::IntegerType`, which is declared at :790 with NO `using tN =` -- it is
  // UNMAPPED.  A key would have to claim a target type the module does not
  // define.  1 site, LEFT LOUD.  Its two-argument sibling
  // `getIntegerType(width, isSigned)` is absent from the model on purpose
  // (`ir::Ty` cannot carry signedness, so `si32` would print `i32`).
  NamedAttribute getNamedAttr(llvm::StringRef name, Attribute val);
  DictionaryAttr getDictionaryAttr(llvm::ArrayRef<NamedAttribute> value);
  BoolAttr getBoolAttr(bool value);
  StringAttr getStringAttr(const llvm::Twine &bytes);
  IntegerAttr getIntegerAttr(Type type, int64_t value);
  ArrayAttr getI64ArrayAttr(llvm::ArrayRef<int64_t> values);
  ArrayAttr getStrArrayAttr(llvm::ArrayRef<llvm::StringRef> values);
};

// `mlir::ModuleOp` -- BuiltinOps.h.inc:199, reached as the RETURN type of
// `runOnOperation`.  An OP HANDLE (a pointer-sized wrapper over Operation*).
class ModuleOp {};

// `mlir::UnrealizedConversionCastOp` -- BuiltinOps.h.inc, the SAME generated
// header as `ModuleOp` above, and the same shape: an ODS-generated op class, i.e.
// one `Operation *` through its `OpState` base.  Declared here ONLY so t160's key
// can be spelled, exactly as `affine::AffineForOp` and `scf::IfOp` are; the model
// is argued at `using t160 =`, including why it makes NO new claim (its `DEF` is
// already the one every op-handle target in this module names).
//
// SAME MIS-SERVICE CHECK AS CallOp/FuncOp/AffineForOp, re-run rather than
// assumed: `grep -n 'OpState::' src.cpp` is still ZERO HITS, so no existing key
// can serve an inherited member call on an UnrealizedConversionCastOp receiver.
// No member and no constructor are keyed, so every corpus read stays LOUD -- see
// t160 for the enumerated sites and the refusals.
class UnrealizedConversionCastOp {};

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
// ⭐ `: public Builder` ADDED 2026-09-28 for the f400-f406 row.  Real MLIR is
// `class OpBuilder : public Builder` (Builders.h:212), so this makes the rule
// source's inheritance match the header it is keying against; without it the
// f400-f406 bodies do not compile (`no member named 'getNamedAttr'`), which is a
// regen failure rather than a converter gap.  t440's key is unaffected: it is
// ARITY 0, FULLY CONCRETE, so the key is the bare name `mlir::OpBuilder` and a
// base class does not enter it.
class OpBuilder : public Builder {
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

// ---------------------------------------------------------------------------
// PASS 2026-09-27 (fourth slot).  SIX type rows, t71-t76, EACH WITH ITS DEFAULT
// CONSTRUCTOR (f40-f45).  Keys read off `searched as:` in the sweep samples
// (/home/agent/work/queue/samples/g0{31,36,53,54,55,69}.txt), and each type's
// SPELLING read from the OWNING TOOLCHAIN HEADER, never from a sibling:
//
//   t71 mlir::UnitAttr          mlir/IR/BuiltinAttributes.h.inc:412  `class UnitAttr;`
//   t72 mlir::TypeID            mlir/Support/TypeID.h:107            `class TypeID {`
//   t73 mlir::MemRefType        mlir/IR/BuiltinTypes.h.inc:1009      `class MemRefType : public ::mlir::Type::TypeBase<...>`
//   t74 mlir::TypedAttr         mlir/IR/BuiltinAttributeInterfaces.h.inc:14 `class TypedAttr;`
//   t75 mlir::VectorType        mlir/IR/BuiltinTypes.h.inc:1283      `class VectorType : public ::mlir::Type::TypeBase<...>`
//   t76 mlir::FlatSymbolRefAttr mlir/IR/BuiltinAttributes.h:26       `class FlatSymbolRefAttr;`
//
// ⭐ THE DESTRUCTOR TEST WAS APPLIED TO ALL SIX.  `grep -rn '~UnitAttr|~TypeID|
// ~MemRefType|~TypedAttr|~VectorType|~FlatSymbolRefAttr' mlir/IR/` over this
// toolchain's headers returns ZERO HITS: every one of the six is a trivially
// destructible handle (a uniquer pointer, or in TypeID's case an
// aligned-storage pointer), so none of them is the `mlir::OwningOpRef` /
// `mlir::InFlightDiagnostic` case where a destructor with observable effect
// FORBIDS an opaque model.
//
// ⭐ EVERY ONE IS CONCRETE, NOT GENERIC.  MemRefType and VectorType are already
// DECLARED above (at the IndexType/RankedTensorType block and next to
// IntegerType respectively) purely so the concrete TypedValue instantiations
// could be spelled; they now get `using tN =` of their own, which is what
// actually registers a rule.  Their declarations are NOT re-stated here.
//
// ⭐ EACH ONE DEFAULT-CONSTRUCTS IN PORTED CODE (`MemRefType t;`, a null
// return, `TypeID id;`), so each gets an EXPLICIT default constructor declared
// here and an fN rule at the bottom.  A `using tN =` ALONE gives rc=0 and then
// `error[E0433]: cannot find module or crate <mangled>` -- measured FOUR times
// in one day (mlir::Attribute, std::__thread_id, std::plus,
// mlir::InFlightDiagnostic/t70, the last of which is in THIS module and was
// fixed by f39).  The `-verbose` line is `search expr void T::T(), result:
// None`.
class UnitAttr {
public:
  UnitAttr();
};
class TypedAttr {
public:
  TypedAttr();
};
class FlatSymbolRefAttr {
public:
  FlatSymbolRefAttr();
};
// `mlir::TypeID` is NOT an attribute or a type -- it is MLIR's RTTI token, one
// pointer into a per-type static storage object (TypeID.h:112-115).  Declared
// here rather than with the Attr/Type families for that reason.
class TypeID {
public:
  TypeID();
};

// ---------------------------------------------------------------------------
// `mlir::SideEffects::EffectInstance<mlir::MemoryEffects::Effect>` -- one entry of
// a `getEffects` list.  Declared here ONLY so t156's key can be spelled; the model
// is argued at `using t156 =`.  `MemoryEffects::Effect` is left INCOMPLETE on
// purpose: it is only ever a template ARGUMENT here, never a value, and no site in
// the corpus names it as a type of its own.
// ---------------------------------------------------------------------------
namespace SideEffects {
template <typename EffectT>
class EffectInstance {};
} // namespace SideEffects
namespace MemoryEffects {
class Effect;
} // namespace MemoryEffects

} // namespace mlir

// ---- type rules, and nothing else ----------------------------------------
namespace llvm {
template <typename T>
class ArrayRef {
public:
  // The initializer-list constructor, ArrayRef.h:110-111.  Declared BY VALUE --
  // `constexpr /*implicit*/ ArrayRef(std::initializer_list<T> Vec
  // LLVM_LIFETIME_BOUND)` -- because the f18/f19 lesson is that a `const &`
  // spelling would record a key no call site searches.  Needed by row g325,
  // whose site literally constructs one: `ArrayRef({num_ports, num_ports})`.
  ArrayRef(std::initializer_list<T> array);
  // f139/f140/f141 -- THREE MORE OVERLOADS, each one PROVEN ABSENT by a
  // `-verbose` readback (/home/agent/work/mlirslot/aref.vlog): the mapper asked
  // for each of these spellings at an `ArrayRef<StringRef>` site and got
  // `result: None`, while the init-list key above came back `Matching:`.  So the
  // 6,332-site `llvm_ArrayRef_llvm_StringRef_::new_N` fabrication is a MISSING
  // OVERLOAD problem, not a dead key.  ArrayRef.h:53 / :64 / :100.
  ArrayRef();
  ArrayRef(const T &element);
  ArrayRef(const std::vector<T> &vec);
  // ⛔ NOT DECLARED, deliberately -- the two remaining `result: None` spellings:
  //   ArrayRef(const T *data, size_t length)        ArrayRef.h:74
  //   ArrayRef(const SmallVectorImpl<T> &vec)       ArrayRef.h:84
  // The first is raw-pointer -> OWNING `Vec`.  The view->owning-Vec collapse this
  // module settled at src.cpp:245-249 was settled for VIEWS of storage the model
  // already owns; RECONSTITUTING one from a bare pointer + length is a different
  // thing.  The refcount model cannot express it AT ALL (`Ptr<T>` is a Weak-based
  // struct, rc.rs:143 -- there is no `from_raw_parts` to reach), and the unsafe
  // model only via `slice::from_raw_parts(...).to_vec()` plus a `Clone` bound, i.e.
  // a body that is correct in one model and impossible in the other.  Left FAILING
  // LOUDLY as `new_N`.
  // The second names `llvm::SmallVectorImpl<T>`, which is rules/smallvector's
  // type, not this module's (THE RECEIVER DECIDES THE MODULE --
  // rules/smallvector/src.cpp:134-136 -- but the PARAMETER type has an owner too,
  // and writing a key here would fix rules/mlir's model of somebody else's type).
  // Re-routed, not refused on the merits.
};

// The FREE comparison operators on two ArrayRefs, ArrayRef.h (`template<typename
// T> inline bool operator==(ArrayRef<T> LHS, ArrayRef<T> RHS)` and its `!=`
// twin), BY VALUE on both sides.  Queue rows g325/g667/g668/g669.  They belong
// to rules/mlir and not rules/smallvector because THE RECEIVER DECIDES THE
// MODULE (rules/smallvector/src.cpp:134-136) and these are free functions with
// no SmallVector-family operand at all; the ArrayRef model is claimed here
// (t19, and the comment above f20).
template <typename T> bool operator==(ArrayRef<T> LHS, ArrayRef<T> RHS);
template <typename T> bool operator!=(ArrayRef<T> LHS, ArrayRef<T> RHS);

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

// llvm/ADT/StringRef.h -- `class StringRef`, and llvm/ADT/StringRef.h's
// `class StringLiteral : public StringRef`.  Declared here ONLY so the
// InFlightDiagnostic `<<` keys that take them can be SPELLED; neither gets a
// `using tN =` in this module, because rules/stringref already owns the type
// rule (t1 -> Vec<libc::c_char> / Vec<u8>, t2 the same for StringLiteral) and a
// second mapping of the same C++ type from a second module is a key collision.
// A declared-but-unmapped class records nothing on its own -- the module header
// says so -- so this is inert apart from making the parameter type nameable.
class StringRef {};
class StringLiteral {};

// llvm/ADT/Twine.h -- `class Twine`.  Declared here for EXACTLY the reason
// StringRef above is, and with exactly the same inert status: only so the two
// f150/f151 InFlightDiagnostic `<<` keys that take a Twine can be SPELLED.  It
// gets NO `using tN =` in this module -- rules/twine already owns the type rule
// (t1 -> Vec<libc::c_char> / Vec<u8>, the NUL-terminated bytes the rope denotes)
// and a second mapping of the same C++ type from a second module is a key
// collision.  Complete rather than forward-declared because f150 takes one BY
// VALUE; the member layout is irrelevant to signature matching, which is why
// rules/twine's own restatement (which this must and does agree with on the
// spelling `llvm::Twine`) can differ in its private members.
class Twine {};

// llvm/ADT/PointerUnion.h -- declared ONLY so the RegionRange base key can be
// spelled.  NOT mapped (no `using tN =`).
template <typename... PTs>
class PointerUnion {};

// llvm/ADT/BitVector.h:101 -- `class BitVector { Storage Bits; unsigned Size = 0;
// ... BitVector() = default; }`.  A plain non-template class, so the key is the
// bare name.  Declared with ONLY the default constructor, read off
// BitVector.h:164 (`BitVector() = default;`); no member is declared because no
// member is mapped (see t79).
class BitVector {
public:
  BitVector();
};

namespace sys {
// llvm/Support/Mutex.h:27-28 -- `template<bool mt_only> class SmartMutex`.
// Declared ONLY so `SmartMutex<true>` is nameable; the real body is
// `std::recursive_mutex impl; unsigned acquired = 0;` plus lock/unlock/try_lock,
// and NONE of those members is declared here because none is mapped (see t77).
template <bool mt_only> class SmartMutex {};
} // namespace sys

namespace cl {
// llvm/Support/CommandLine.h:410-415 -- `struct desc { StringRef Desc;
// desc(StringRef Str); void apply(Option &O) const; }`.  The CONSTRUCTOR
// parameter is `llvm::StringRef` BY VALUE (CommandLine.h:413
// `desc(StringRef Str) : Desc(Str) {}`) -- read off that line, not copied from a
// sibling.  `apply` is deliberately NOT declared and NOT mapped.
struct desc {
  desc(StringRef Str);
};
} // namespace cl

namespace detail {
// llvm/ADT/STLExtras.h -- `indexed_accessor_range_base<DerivedT, BaseT, T,
// PointerT, ReferenceT>`, the CRTP base of MLIR's range families.
template <typename DerivedT, typename BaseT, typename T, typename PointerT,
          typename ReferenceT>
class indexed_accessor_range_base {};
} // namespace detail
} // namespace llvm

// ---- t81/t82 declarations -------------------------------------------------
// Reopened AFTER `namespace llvm` closes, because `CopyOnWriteArrayRef`'s only
// constructor takes `llvm::ArrayRef<T>` and that class is declared at line 874
// above -- inside namespace mlir at the top of this file it is not yet visible.
namespace mlir {

// `mlir::CopyOnWriteArrayRef<T>` -- t81, queue row g110 (13 TUs).  A wrapper
// around `ArrayRef<T>` that copies into a `SmallVector<T>` on modification
// (ADTExtras.h:25-79).  In the corpus it appears exactly once, as a MEMBER:
// `CopyOnWriteArrayRef<int64_t> shape;` in
// `dataflow-scheduler/external/ktir-mlir-frontend/include/Ktdp/KtdpTypes.hpp:63`,
// i.e. the leaf-in-a-member-shape case that aborts before any call site.
//
// THE CONSTRUCTOR PARAMETER IS `llvm::ArrayRef<T>` BY VALUE -- no `const`, no
// `&`.  `ADTExtras.h:27` reads `CopyOnWriteArrayRef(ArrayRef<T> array) :
// nonOwning(array){};`, and that unqualified `ArrayRef` is LLVM's, pulled into
// namespace mlir by `using llvm::ArrayRef;` at `mlir/Support/LLVM.h:119`.
// Spelling it `const ArrayRef<T> &` here would record a DIFFERENT key that no
// call site ever searches -- the f18/f19 dead-key failure mode.
//
// DESTRUCTOR TEST: `~CopyOnWriteArrayRef` appears NOWHERE in the include tree.
// The members are `ArrayRef<T> nonOwning; SmallVector<T> owningStorage;`
// (ADTExtras.h:77-78) and destruction is only the implicit SmallVector teardown.
// So an ordinary mapped value is permitted, and unusually for this module the
// honest model is a real OWNING one: `Vec<T>`, exactly as t19 models
// `llvm::ArrayRef<T>`.  The copy-on-write split between the two storages is an
// allocation strategy, not observable through any operation.
//
// There is NO default constructor and no copy/move constructor declared, so the
// by-value ArrayRef constructor (f50) is the ONLY key ported code can reach.
template <typename T>
class CopyOnWriteArrayRef {
public:
  CopyOnWriteArrayRef(llvm::ArrayRef<T> array);

  // The FIVE members `Ktdp::AccessTileType::Builder` actually calls
  // (KtdpTypes.hpp:31-59).  Spellings taken CHARACTER-FOR-CHARACTER off
  // ADTExtras.h:29-59, not from a sibling: `insert`/`erase`/`set` take
  // `size_t index`, `size()` is `const`, `operator=` takes its ArrayRef BY VALUE
  // and returns `CopyOnWriteArrayRef &`, and the conversion operator is `const`.
  CopyOnWriteArrayRef &operator=(llvm::ArrayRef<T> array);
  void insert(std::size_t index, T value);
  void erase(std::size_t index);
  std::size_t size() const;
  operator llvm::ArrayRef<T>() const;
  // `set(size_t, T)` and `empty()` are declared in ADTExtras.h:52-56 but are NOT
  // called anywhere in the corpus -- LEFT OUT on purpose, so they abort loudly
  // if that ever changes, rather than becoming coverage nobody measured.
};

// `mlir::DominanceInfo` -- t82, queue rows g094 (16 TUs) and g1372 (1 TU, the
// same type reached while converting `dynamicSizesDominate`); ONE key closes
// BOTH.  `searched as: mlir::DominanceInfo`, a plain non-template class reached
// in member/parameter position (Dominance.h:140).
//
// DESTRUCTOR VERDICT: cache teardown only, so an opaque unit is PERMITTED.
// `Dominance.cpp` is not on this filesystem (headers+libs only), so the upstream
// release/22.x body was used: `~DominanceInfoBase() { for (auto entry :
// dominanceInfos) delete entry.second.getPointer(); }`.  The header corroborates
// it more strongly than the body does: `DominanceInfoBase` has EXACTLY ONE data
// member, `mutable DenseMap<Region *, llvm::PointerIntPair<DomTree *, 1, bool>>
// dominanceInfos;` (Dominance.h:129-130), and `invalidate()` (:47) is that same
// loop plus `.clear()` -- which is only sound if the map is a pure cache.  No
// I/O, no IR mutation, no diagnostic.
//
// BUT NO MEMBER IS MAPPED, DELIBERATELY.  The whole purpose of the class is its
// queries -- `properlyDominates(Operation *, Operation *, bool)`,
// `dominates(Operation *, Operation *)`, `properlyDominates(Value, Operation *)`,
// `dominates(Block *, Block *)` and the `Block::iterator` overloads
// (Dominance.h:153-200) -- and each returns a real boolean DERIVED FROM THE IR.
// A `()` model with nothing mapped is correct and safe: a dominance query aborts
// LOUDLY.  Mapping any of them with a hardcoded true/false would be silent
// wrongness that changes which transformations fire.  Same rule as t72 (TypeID),
// t79 (BitVector) and t80 (PreservedAnalyses).
//
// TWO CONSTRUCTORS ARE DECLARED because the corpus writes BOTH forms.  The real
// one is `DominanceInfoBase(Operation *op = nullptr)` (Dominance.h:39), inherited
// into `DominanceInfo` via `using super::super;`, so a defaulted argument could
// have made one of the two arities unreachable -- that is exactly the trap that
// left rules/atomic's t5 a dead key.  Measured, not assumed: see the
// `search expr` evidence in the report.  Corpus sites for the 0-ary form:
// `dcc/src/Transform/Sentient/Utils.cpp:85` (`DominanceInfo dom_info;`) and the
// member declarations at `dcc/src/Analysis/ConditionalTree.hpp:183`,
// `.../OperandReuse.hpp:53`, `.../RedundantDefinitionEliminationTree.hpp:286`.
// For the 1-ary form: `dr5/src/Passes/Scheduler/NestedLoops.cpp:355`,
// `.../LiveRangeReduction.cpp:888`, `.../SpecializedCanonicalization.cpp:172`,
// `dataflow-scheduler/lib/Transforms/DoubleBuffering.cpp:531` and the three
// `new DominanceInfo(unit_op)` sites.  The COPY constructor is `= delete`d
// (Dominance.h:44), so there is no clone key to write.
// THE 1-ARY KEY IS NAMED AFTER THE *BASE*, and this was MEASURED, not inferred.
// With `DominanceInfo` declared as a flat class carrying `DominanceInfo(Operation
// *)`, the recorded key was `void mlir::DominanceInfo::DominanceInfo(mlir::
// Operation *)` while `-verbose` on `DominanceInfo d(op);` showed
//   search expr void mlir::DominanceInfo::DominanceInfoBase(mlir::Operation *),
//   result: None
// followed by the `mlir_DominanceInfo::new_1` fallback -- a textbook DEAD KEY
// (rc=0 then E0433).  The ctor is INHERITED via `using super::super;`
// (Dominance.h:146), so it keeps the BASE's name `DominanceInfoBase` while being
// qualified by the DERIVED class, and with NO template argument list even though
// the real base is `detail::DominanceInfoBase</*IsPostDom=*/false>`.  The
// declaration below reproduces that shape exactly so the key matches.  The 0-ary
// form is a different key, `void mlir::DominanceInfo::DominanceInfo()` -- the
// derived class's own implicit default constructor -- and it MATCHED as a flat
// declaration, so both are declared.
namespace detail {
class DominanceInfoBase {
public:
  DominanceInfoBase(Operation *op);
};
} // namespace detail

class DominanceInfo : public detail::DominanceInfoBase {
public:
  using detail::DominanceInfoBase::DominanceInfoBase;
  DominanceInfo();
};

} // namespace mlir

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
//
// ⛔⛔ THE LAST SENTENCE ABOVE IS EMPIRICALLY FALSE, AND THAT IS WHY THE
// CONSTRUCTOR KEY IS REFUSED.  MEASURED 2026-09-28 (pin/cpp2rust
// md5 e2d09f4562c470f813b0fa142d373bfb, tree cloned from pin/ir.v22, witness TU
// `dialects/ExPlan/ExPlanOps.cpp`, bucket A rc=0, 4,250 emitted lines; the
// `-verbose` leg EXITED 0 at 90,747 log lines / 5,125 asks, so its silence is
// admissible evidence -- log `/home/agent/work/mlirOpState0928/vb.rs.log`):
//
//   search expr void mlir::OperationState::OperationState(mlir::Location, llvm::StringRef), result:
//   None                                                          (x15, ONE overload)
//   search expr ... & mlir::OperationState::getOrAddProperties(), result: None   (x82)
//   search expr ... mlir::OperationState::addTypes(...),          result: None   (x48)
//   ... addOperands x44, addAttributes x24, useProperties x12, getContext x10
//
// An unmapped MEMBER on this receiver DOES NOT ABORT.  All 220 member asks MISS
// and the converter emits them TEXTUALLY against the unit target -- from
// before.rs:1711 and :1778, on a parameter declared `odsState: *mut ()`:
//     (*(unsafe { (*odsState).getOrAddProperties() })).address = (address).clone();
//     (unsafe { let _newTypes: *mut ... = &mut resultTypes; (*odsState).addTypes(_newTypes) });
// 104 such lines in one 4,250-line TU.  That is loud at RUSTC time, not at
// translate time, but it is not this module's key to fix and it is NOT a licence
// to add the constructor.
//
// ⛔ WHY f143 IS REFUSED -- THE OBSERVER, NAMED.  The single reached overload is
// `OperationState(Location location, StringRef name)`, i.e. the two fields that
// ARE the op's identity.  The target type is `()`, so ANY body writable here must
// DISCARD both arguments (both are modelled -- `mlir::Location` -> t?
// `dataflowir_gen::ir::Location`, `llvm::StringRef` -> String -- so this is not a
// missing-model refusal; it is a LOST-SEMANTICS refusal).  Those fields are read
// DOWNSTREAM IN THE SAME EMITTED BODY: `__state__` is handed to
// `(*builder).create_pconst(_state)` and the result is `dyn_cast_47`-ed to the
// concrete Op type, so the NAME decides whether that cast can succeed, and the
// LOCATION is the op's diagnostic anchor.  A `()`-returning key would therefore
// build a NAMELESS, LOCATIONLESS op and COMPILE.
//
// ⭐ AND IT WOULD BE A NET REGRESSION, not a neutral one.  Today those 15 sites
// call `mlir_OperationState::new_1` -- defined NOWHERE -- so they fail at compile
// time.  f143 would replace 15 compile-time failures with 15 silent semantic
// losses while `getOrAddProperties`/`addTypes` continue to fail anyway, so the TU
// does not get closer to compiling and the class gets quieter.  Per the standing
// rule: refuse rather than lose semantics.  ⭐ THE REAL FIX FOR THIS ROW IS NOT A
// CONSTRUCTOR KEY -- it is either a REAL model for the construction bag (a struct
// carrying name/location/operands/types, which t63's own paragraph above argues
// `dataflowir-gen` does not have) or a converter-side change that makes an
// unmapped member on an opaque-unit receiver ABORT instead of emitting.  Both are
// out of this module's scope.  DO NOT WRITE f143 FOR THIS ROW.
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

// ---------------------------------------------------------------------------
// t71-t76: THE SIX ROWS ADDED 2026-09-27 (fourth slot).  See the declaration
// block at the end of `namespace mlir` for the per-type header line each
// spelling was read from and for the zero-hit destructor grep.  EACH ONE ALSO
// HAS ITS DEFAULT CONSTRUCTOR, f40-f45, at the bottom of this file.
// ---------------------------------------------------------------------------

// t71: `mlir::UnitAttr` -> `dataflowir_gen::ir::Attr` (ir.rs:466).  34 TUs, the
// largest open rules/mlir row.  ⭐ THIS ONE IS WELL-GROUNDED, NOT A WIDENING:
// `Attr::Unit` (ir.rs:473-474) is documented IN THE CRATE as `mlir::UnitAttr`
// -- "present with no value; the dictionary prints only the key" -- and its
// Display prints the empty string (ir.rs:539) with the key-only form handled at
// ir.rs:569.  So unlike t54-t57 the target enum really does model this
// attribute.
// ⛔ WHAT IS STILL LOST: the `init` is NOT `Attr::Unit`.  A default-constructed
// `mlir::UnitAttr` is a NULL HANDLE, not a present unit attribute, and
// `Attr::Unit` would print as a REAL unit attr.  The init is therefore t5's
// EMPTY-SPELLING `Attr::Raw("")` null sentinel, the same sentinel t54-t57 use,
// which no real attribute can print as.  `UnitAttr::get(ctx)` -- the factory
// that would actually yield `Attr::Unit` -- is NOT mapped and still aborts
// loudly; mapping it would need the per-overload parameter form read off
// `-verbose`, which this slot did not measure (same refusal as
// `mlir::IndexType::get`).
using t71 = mlir::UnitAttr;

// t72: `mlir::TypeID` -> AN OPAQUE UNIT.  28 TUs.  The SAME model and the same
// bargain as t58 (`mlir::detail::InterfaceMap`), t59 (`mlir::Builder`), t69
// (`mlir::OpPrintingFlags`).
// ⭐ WHY A UNIT IS ADMISSIBLE HERE: TypeID is MLIR's RTTI token -- one pointer
// into a per-type static `Storage` (TypeID.h:112-115) -- and the crate has NO
// RTTI concept at all.  Its destructor test is clean: there is no `~TypeID` in
// TypeID.h, it owns nothing (`TypeIDAllocator`/`SelfOwningTypeID` are separate
// classes), so dropping it has no observable effect.  The 28-TU row is
// DECLARATION sites and `const TypeID &` parameters, which need the type alone.
// ⛔ WHAT IS LOST, AND IT IS THE WHOLE POINT OF THE TYPE: IDENTITY.  Every
// translated TypeID is the same `()`, so if `operator==` / `operator!=`
// (TypeID.h:118-123) were ever mapped, every type would compare EQUAL to every
// other -- silently wrong, which is worse than an abort.  They are therefore
// DELIBERATELY NOT MAPPED, and neither is `TypeID::get<T>()` (TypeID.h:127-130),
// so every comparison and every factory call through one still aborts loudly.
// The unit buys the declarations and NOTHING ELSE.
using t72 = mlir::TypeID;

// t73: `mlir::MemRefType` -> `dataflowir_gen::ir::Ty` (ir.rs:37).  16 TUs.  A
// WIDENING, the same one t41 (`ShapedType`), t42 (`TensorType`) and t60
// (`IndexType`) make: every MemRefType IS a `mlir::Type` and t5 already maps
// `mlir::Type -> ir::Ty`.
// ⭐ BETTER GROUNDED THAN t41/t42: `Ty::MemRef(Vec<i64>, Box<Ty>)` (ir.rs:44)
// EXISTS and carries exactly the dimension list and element type, negative dim =
// dynamic `?` (ir.rs:56-60), so the shape SURVIVES the mapping -- there is no
// reparse-from-spelling loss of the kind `RankedTensorType` takes.
// ⛔ The `init` is still the EMPTY-SPELLING `Ty::Opaque("")` null sentinel, not
// `Ty::MemRef(vec![], ...)`: a default-constructed MemRefType is a NULL handle
// and an empty-shaped memref is a REAL type that prints `memref<f32>`.  NO
// accessor is mapped -- `getShape`, `getElementType`, `getRank`, `getLayout`,
// `getMemorySpace`, `MemRefType::get` are ALL absent and still abort loudly.
using t73 = mlir::MemRefType;

// t74: `mlir::TypedAttr` -> `dataflowir_gen::ir::Attr` (ir.rs:466).  16 TUs.
// The TWELFTH widening of an attribute class to that enum, and it has the same
// interface-over-subclasses shape as t41/t42/t55: `TypedAttr` is the ODS
// attribute INTERFACE for "an attribute that carries a type"
// (BuiltinAttributeInterfaces.h.inc:14), over IntegerAttr / FloatAttr /
// DenseElementsAttr and the rest.
// ⛔ WHAT IS LOST: the CONSTRAINT "this attribute has a type".  `Attr` has no
// variant that pairs an arbitrary payload with a `Ty` except `Attr::Int(i64,
// Ty)` (ir.rs:469), which is IntegerAttr-specific, so a TypedAttr that is not
// an integer lands in `Attr::Raw`/`Attr::Aliasable` BY SPELLING.  `getType()`
// -- the ONE member the interface exists for -- is NOT mapped and still aborts.
using t74 = mlir::TypedAttr;

// t75: `mlir::VectorType` -> `dataflowir_gen::ir::Ty` (ir.rs:37).  16 TUs.  Same
// widening as t73, and it is the BEST-GROUNDED of the type rows: `Ty::Vector(
// Vec<i64>, Box<Ty>)` is documented at ir.rs:41-42 as `vector<64xf16> --
// mlir::VectorType`, i.e. the crate models this exact class by name.
// ⛔ Same two losses as t73: the `init` is the `Ty::Opaque("")` null sentinel
// rather than a real empty vector type, and NO accessor is mapped
// (`getShape`/`getElementType`/`getNumScalableDims`/`VectorType::get` all still
// abort).  ⛔ NOTE the pre-existing declaration of `class VectorType` in this
// file is ALSO the template argument of t47 (`TypedValue<VectorType>`); adding
// `using t75` registers a rule for the TYPE ITSELF and does not change t47,
// which stays CONCRETE.
using t75 = mlir::VectorType;

// t76: `mlir::FlatSymbolRefAttr` -> `dataflowir_gen::ir::Attr` (ir.rs:466).  12
// TUs.  The THIRTEENTH widening to that enum.
// ⛔ WHAT IS LOST: `Attr` HAS NO SYMBOL-REFERENCE VARIANT (`grep -n
// 'SymbolRef\|Flat' ir.rs` finds only the two lines of `Attr::Unit` doc prose
// and `I32Array`'s, i.e. ZERO symbol-ref hits), so the referenced symbol NAME
// lands in `Attr::Raw` BY SPELLING rather than as a structured reference, and
// `getValue()`/`getAttr()`/`FlatSymbolRefAttr::get` are NOT mapped and still
// abort loudly.  A Raw spelling round-trips the printed `@name` form, which is
// what the fmt layer needs, and loses the ability to RESOLVE the symbol -- a
// capability the crate does not have for any attribute.
using t76 = mlir::FlatSymbolRefAttr;

// t77: `llvm::sys::SmartMutex<true>` -> `::std::sync::Mutex<()>`, the SAME model
// rules/mutex gives `std::mutex` (t1).  Queue row g038, 28 TUs, and its
// `searched as:` line is `llvm::sys::SmartMutex<true>` -- CONCRETE, because the
// template parameter is a bool NON-TYPE argument that is fixed at every use site
// (`SmartMutex<false>` is the `sys::Mutex` alias; only `<true>` appears in the
// corpus rows).  A concrete key keeps this off mapper.cpp:722/:835.
//
// ⭐ DESTRUCTOR TEST: `grep -rn '~SmartMutex' llvm/Support/` finds ZERO hits, so
// the only destruction effect is the implicit one of the contained
// `std::recursive_mutex` -- no observable program effect, which is what permits
// an ordinary mapped local here.
//
// ⛔ WHAT IS LOST: the contained mutex is `std::recursive_mutex`, and
// `::std::sync::Mutex` is NOT re-entrant.  That difference is UNOBSERVABLE here
// because `lock()`, `unlock()` and `try_lock()` are DELIBERATELY NOT KEYED --
// the same decision, for the same reason, that rules/mutex's header records for
// bare `std::mutex::lock()`: a one-expression rule body has no place to keep a
// guard alive, so a rule for bare lock() could only produce a lock that never
// locks.  Those three stay a LOUD abort.  Also lost: `acquired`, the
// single-threaded-mode assertion counter, which no ported code reads.
using t77 = llvm::sys::SmartMutex<true>;

// t78: `llvm::cl::desc` -> the StringRef model, `Vec<libc::c_char>` /  `Vec<u8>`
// (rules/stringref t1/t2 give StringRef and StringLiteral exactly that).  Queue
// row g041, 26 TUs, `searched as: llvm::cl::desc` -- a plain non-template
// struct, so nothing generic is involved at all.
//
// ⭐ DESTRUCTOR TEST: `grep -rn '~desc' CommandLine.h` finds ZERO hits; the
// struct is a one-field `StringRef Desc` modifier with no destructor, so it is
// pure data and an ordinary mapped value is faithful.
//
// ⛔ WHAT IS LOST: `apply(Option &O) const` (CommandLine.h:415), which calls
// `O.setDescription(Desc)`.  It is NOT declared above and NOT mapped, so a call
// to it ABORTS LOUDLY rather than silently failing to register a description --
// the t72 TypeID precedent: map the type, omit the member that would lie.  The
// PAYLOAD (the description text) is preserved exactly, which is the part a
// `-help` renderer would read.
using t78 = llvm::cl::desc;

// t79: `llvm::BitVector` -> `Vec<bool>`, one Rust bool per bit.  Queue row g105,
// 13 TUs; the row is a LEAF inside `std::array<std::vector<llvm::BitVector>, _>`,
// i.e. the type is needed for a MEMBER's shape, which is exactly the case where
// an unmapped leaf aborts before any call site is reached.  Non-template, so the
// key is the bare name and nothing generic is involved.
//
// DESTRUCTOR TEST: `grep -rn '~BitVector' llvm/ADT/BitVector.h` finds ZERO hits
// -- the class is `SmallVector<uintptr_t> Bits; unsigned Size = 0;` and its only
// destruction effect is the implicit one of the contained SmallVector.  No
// observable program effect, so an ordinary mapped value is permitted.
//
// WHY `Vec<bool>` AND NOT A UNIT: the payload IS read (`operator[]`, `test`,
// `count`, `any`), so a unit would be the kind of model that silently loses what
// the program reads.  `Vec<bool>` keeps the bits exactly; the difference from
// LLVM's packed words is not observable through any mapped operation.
//
// WHAT IS LOST: EVERY member.  `set`/`reset`/`operator[]`/`test`/`count`/`any`/
// `all`/`none`/`resize`/`flip` and the bitwise operators are NOT declared above
// and NOT mapped, so each call ABORTS LOUDLY -- the t72 TypeID precedent: map the
// type, omit the members that would lie.  In particular `BitVector::reference`,
// the proxy `operator[]` returns, is absent, so no rule can pretend a bit write
// happened.
using t79 = llvm::BitVector;

// t80: `mlir::detail::PreservedAnalyses` -> `()`.  Queue row g062, 32 TUs, and
// its `searched as:` line is `mlir::detail::PreservedAnalyses` -- a plain
// non-template class reached while converting
// `mlir::ktdf_arch::DeviceManager::isInvalidated(const PreservedAnalyses &)`,
// i.e. in PARAMETER position, which is why the abort happens before any member
// call.
//
// DESTRUCTOR TEST: `grep -rn '~PreservedAnalyses' mlir/` finds ZERO hits; the
// class holds one `SmallPtrSet<TypeID, 2>` and destroys it implicitly.  No
// observable program effect.
//
// WHY A UNIT IS THE HONEST MODEL HERE, unlike t79: the payload is a set of
// `mlir::TypeID`, and t72 maps `mlir::TypeID` ITSELF to `()` with its `==`/`!=`
// DELIBERATELY ABSENT, because mapping them would make every type compare equal.
// A set keyed on a type that cannot be compared has no representable contents, so
// `Vec<()>`/`HashSet` would be a fiction with a size that means nothing.  `()`
// says exactly as much as t72 already says.
//
// WHAT IS LOST: every member -- `preserveAll`, `isAll`, `isNone`, `preserve(TypeID)`,
// `isPreserved(TypeID)` and the template forms.  None is declared above and none
// is mapped, so a call ABORTS LOUDLY instead of answering a preservation query
// from an empty model.  This is the t72 rule applied one level up.
using t80 = mlir::detail::PreservedAnalyses;

// t81: `mlir::CopyOnWriteArrayRef<T1>` -> `Vec<T1>`.  Queue row g110, 13 TUs.
// Generic in the element type; the corpus instantiates it only at `long`
// (`CopyOnWriteArrayRef<int64_t>`), but the key is written generic exactly as t19
// is, because the class is a template and one key covers every instantiation.
// See the declaration note above for the destructor test and for why an OWNING
// `Vec` is faithful here rather than the unit this module usually reaches for.
//
// WHAT IS LOST: every member.  `insert(size_t, T)`, `erase(size_t)`,
// `set(size_t, T)`, `size()`, `empty()`, `operator=(ArrayRef<T>)` and the
// conversion `operator ArrayRef<T>() const` are NOT declared above and NOT
// mapped, so each call ABORTS LOUDLY.  All of them WOULD be faithfully
// representable over `Vec<T1>` -- unusual for this module -- and the corpus does
// call `insert`, `erase`, `size`, `operator=` and the conversion operator inside
// `Ktdp::AccessTileType::Builder` (KtdpTypes.hpp:33-59).  They are left out of
// THIS pass only because the type key plus its constructor is what was measured;
// an abort is the correct state for an unmapped member, and adding them is the
// obvious next increment.
template <typename T1> using t81 = mlir::CopyOnWriteArrayRef<T1>;

// t82: `mlir::DominanceInfo` -> `()`.  Queue rows g094 (16 TUs) + g1372 (1 TU).
// Non-template, so the key is the bare name.  See the declaration note above:
// the payload is a pure per-Region cache of dominator trees, and every query
// that would read it is deliberately UNMAPPED so it aborts rather than answering
// a dominance question from an empty model.
using t82 = mlir::DominanceInfo;

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

// ---------------------------------------------------------------------------
// f22-f38: THE REST OF THE InFlightDiagnostic `<<` FAMILY.
//
// Every one of these is the SAME member template as f21
// (`Diagnostics.h:344-349  template <typename Arg> InFlightDiagnostic
// &&operator<<(Arg &&arg) &&`) at a DIFFERENT deduced `Arg`, and the converter
// keys on the INSTANTIATED signature -- so each deduction needs its own rule
// even though most share a target body.  The work-queue row each one closes is
// named on its line.  They are spelled with the receiver as an EXPLICIT rvalue
// (`std::move(d).operator<<(x)`) for the same reason f21 is: the recorded callee
// must unambiguously be this `&&`-qualified member and not a free ADL candidate.
//
// WHICH REFERENCE KIND EACH ARGUMENT IS, AND WHY IT IS NOT COSMETIC.  `Arg&&` is
// a forwarding parameter, so an LVALUE argument deduces `Arg = T&` and the key
// reads `T &`, while an RVALUE deduces `Arg = T` and the key reads `T &&`.  The
// work queue records BOTH for StringRef (g346 / g320), for std::string (g396 /
// g1159 / g1158) and for unsigned int (g553 / g1163 / g1162), and they are
// DIFFERENT KEYS: a rule for one does not match the other.  The target bodies
// follow the same split -- an lvalue-reference argument takes a Rust SHARED
// REFERENCE, because copying the callee's `Vec` by value would MOVE the caller's
// variable and a second use of it would then be E0382.
// ---------------------------------------------------------------------------

// ---- the string payloads --------------------------------------------------
// row g320 (10 TUs), the largest remaining: `llvm::StringRef &&`.
mlir::InFlightDiagnostic &&f22(mlir::InFlightDiagnostic &&d, llvm::StringRef s) {
  return std::move(d).operator<<(std::move(s));
}

// row g346 (8 TUs): the same type as an LVALUE, so `Arg` deduces `StringRef&`.
mlir::InFlightDiagnostic &&f23(mlir::InFlightDiagnostic &&d, llvm::StringRef &s) {
  return std::move(d).operator<<(s);
}

// row g396 (5 TUs): `std::string &`.
mlir::InFlightDiagnostic &&f24(mlir::InFlightDiagnostic &&d, std::string &s) {
  return std::move(d).operator<<(s);
}

// ---- row g491 (3 TUs): `mlir::Type &` -- STREAMING A TYPE MEANS PRINTING IT.
// This one was flagged as needing a fidelity check rather than a mechanical
// body, because the message TEXT is observable and `ir::Ty`'s Rust rendering has
// to be MLIR's.  It is (dataflowir-gen/src/ir.rs:50):
//     Ty::Index     -> "index"          Ty::Int(w)  -> "i{w}"
//     Ty::Float(w)  -> "f{w}"           Ty::Vector  -> "vector<64x...xELT>"
//     Ty::MemRef    -> "memref<...>"    Ty::Opaque(s) -> the exact spelling
// which is `mlir::Type::print`'s builtin-type syntax, and `Opaque` exists
// precisely so a dialect type round-trips by its own spelling.  So `Display` is
// the faithful renderer and the diagnostic text matches.  (The one place the two
// could drift is a NEGATIVE vector dimension, printed `?x`; MLIR spells a
// dynamic dim `?` in a memref and does not admit one in a plain vector, so no
// site in this row can reach it.)
mlir::InFlightDiagnostic &&f25(mlir::InFlightDiagnostic &&d, mlir::Type &t) {
  return std::move(d).operator<<(t);
}

// ---- row g553 (2 TUs): `unsigned int &`.
mlir::InFlightDiagnostic &&f26(mlir::InFlightDiagnostic &&d, unsigned int &v) {
  return std::move(d).operator<<(v);
}

// ---- the 1-TU tail, g1150-g1164.  Note that the 15 rows collapse to 13 keys:
// g1161's key is g1151's (`int &`, reached once through a typedef named `type`)
// and g1164's is g1152's (`long &`, through `value_type`).  A key is a key -- the
// spelling the mapper prints is what matches, so ONE rule closes both rows.
mlir::InFlightDiagnostic &&f27(mlir::InFlightDiagnostic &&d, int &v) {   // g1151, g1161
  return std::move(d).operator<<(v);
}

mlir::InFlightDiagnostic &&f28(mlir::InFlightDiagnostic &&d, long &v) {  // g1152, g1164
  return std::move(d).operator<<(v);
}

mlir::InFlightDiagnostic &&f29(mlir::InFlightDiagnostic &&d, const long &v) {  // g1150
  return std::move(d).operator<<(v);
}

mlir::InFlightDiagnostic &&f30(mlir::InFlightDiagnostic &&d, const int &v) {   // g1160
  return std::move(d).operator<<(v);
}

mlir::InFlightDiagnostic &&f31(mlir::InFlightDiagnostic &&d,
                               const unsigned int &v) {                        // g1162
  return std::move(d).operator<<(v);
}

mlir::InFlightDiagnostic &&f32(mlir::InFlightDiagnostic &&d, unsigned int v) { // g1163
  return std::move(d).operator<<(std::move(v));
}

mlir::InFlightDiagnostic &&f33(mlir::InFlightDiagnostic &&d, unsigned long v) { // g1157
  return std::move(d).operator<<(std::move(v));
}

mlir::InFlightDiagnostic &&f34(mlir::InFlightDiagnostic &&d,
                               const std::string &s) {                         // g1158
  return std::move(d).operator<<(s);
}

mlir::InFlightDiagnostic &&f35(mlir::InFlightDiagnostic &&d, std::string s) {   // g1159
  return std::move(d).operator<<(std::move(s));
}

// g1155 / g1156: streaming an attribute PRINTS it, and the same fidelity test as
// f25 applies.  `ir::Attr`'s Display (ir.rs:521) is MLIR's attribute syntax --
// `Str` quotes and escapes, `Int(v,t)` prints `v : t`, `Unit` prints nothing,
// `I32Array` prints `[a : i32, b : i32]`, `Raw`/`Aliasable` the exact spelling --
// so it is the faithful renderer.  `mlir::StringAttr` maps to the same `Attr`
// (t7), so its body is the same.
mlir::InFlightDiagnostic &&f36(mlir::InFlightDiagnostic &&d, mlir::Attribute &a) {
  return std::move(d).operator<<(a);
}

mlir::InFlightDiagnostic &&f37(mlir::InFlightDiagnostic &&d, mlir::StringAttr a) {
  return std::move(d).operator<<(std::move(a));
}

// g1154: `llvm::StringLiteral &&`, the same byte payload as a StringRef
// (rules/stringref t2 gives it the identical model).
mlir::InFlightDiagnostic &&f38(mlir::InFlightDiagnostic &&d, llvm::StringLiteral s) {
  return std::move(d).operator<<(std::move(s));
}

// ---- f39: THE DEFAULT CONSTRUCTOR, defect (5) of queue row c020 ------------
// MEASURED, this tree, `pin/cpp2rust -verbose` on
// /home/agent/work/probe/mlirdiag/probe.cpp:
//     search expr void mlir::InFlightDiagnostic::InFlightDiagnostic(), result:
//     None
// so `mlir::InFlightDiagnostic d;` lowered to the undefined name
// `mlir_InFlightDiagnostic::new()` -- `error[E0433]: cannot find module or crate
// mlir_InFlightDiagnostic` x3 in BOTH models, which stops rustc before borrowck
// and therefore before any of the SHAPE defects can be measured.
//
// ⭐ A TYPE RULE IS NOT A CONSTRUCTOR.  t70 maps the TYPE; the converter looks
// the default ctor up as an ORDINARY EXPR RULE under the key above, and with no
// such rule it falls back to `<mangled type name>::new()`.  Third measured
// instance of this trap today after `mlir::Attribute` (t*/f* note at :1370) and
// `std::__thread_id` (rules/thread_id/src.cpp:63) -- the pattern is: every
// mapped type that ported code DEFAULT-CONSTRUCTS needs its ctor keyed too.
mlir::InFlightDiagnostic f39() { return mlir::InFlightDiagnostic(); }

// ---- f40-f45: THE DEFAULT CONSTRUCTORS FOR t71-t76 -------------------------
// ⭐ ONE PER TYPE KEY, NO EXCEPTIONS.  f39's note above is the whole argument
// and it applies verbatim to all six: a `using tN =` maps the TYPE ONLY, and the
// converter looks the default constructor up as an ORDINARY EXPR RULE under the
// key `void <T>::<T>()`.  On a miss it falls back to `<mangled type>::new()`,
// which does not exist, so the translation gets rc=0 and rustc then gets
// `error[E0433]: cannot find module or crate ...`.  Adding a type key WITHOUT
// its constructor is how four separate rows shipped DEAD today.
//
// Each body is the type's NULL-HANDLE sentinel, chosen per type in the `using`
// comments above, NOT a valid value: `Attr::Raw("")` for the three attribute
// rows (t5's empty-spelling sentinel), `Ty::Opaque("")` for the two type rows,
// `()` for the TypeID unit.  Appended AFTER f39, which renumbers nothing.
mlir::UnitAttr f40() { return mlir::UnitAttr(); }
mlir::TypeID f41() { return mlir::TypeID(); }
mlir::MemRefType f42() { return mlir::MemRefType(); }
mlir::TypedAttr f43() { return mlir::TypedAttr(); }
mlir::VectorType f44() { return mlir::VectorType(); }
mlir::FlatSymbolRefAttr f45() { return mlir::FlatSymbolRefAttr(); }

// ---- f46/f47: THE CONSTRUCTORS FOR t77/t78 --------------------------------
// ⭐ f39's note above is the whole argument and applies verbatim: a `using tN =`
// maps the TYPE ONLY, and the converter looks the constructor up as an ORDINARY
// EXPR RULE.  On a miss it emits `<mangled type>::new()`, so the TU gets rc=0 and
// then `error[E0433]`.  Appended AFTER f45, which renumbers nothing.
//
// f46 -- `llvm::sys::SmartMutex<true> m;`, the DEFAULT constructor (implicit;
// Mutex.h declares none, the class has only the two members).  Body is a fresh
// unlocked mutex, the same body as rules/mutex f1.
llvm::sys::SmartMutex<true> f46() { return llvm::sys::SmartMutex<true>(); }

// f47 -- `llvm::cl::desc("...")`, the ONLY constructor the struct has
// (CommandLine.h:413, `desc(StringRef Str)`, BY VALUE).  There is no default
// constructor, so this is the one key ported code can reach.
llvm::cl::desc f47(llvm::StringRef s) { return llvm::cl::desc(s); }

// ---- f48/f49: THE CONSTRUCTORS FOR t79/t80 ---------------------------------
// f39's note above is the whole argument and applies verbatim: a `using tN =`
// maps the TYPE ONLY, and the converter looks the default constructor up as an
// ORDINARY EXPR RULE (`search expr void T::T()`).  On a miss it emits
// `<mangled type>::new()`, so the TU gets rc=0 and then `error[E0433]`.
// Appended AFTER f47, which renumbers nothing.
//
// f48 -- `llvm::BitVector v;`, the DEFAULT constructor (BitVector.h:164,
// `BitVector() = default;`).  Body is an EMPTY bit vector, which is what that
// default does: `Bits` empty, `Size = 0`.
llvm::BitVector f48() { return llvm::BitVector(); }

// f49 -- `mlir::detail::PreservedAnalyses pa;`, the implicit default constructor
// (AnalysisManager.h declares none).  Body is the unit, per t80.
mlir::detail::PreservedAnalyses f49() { return mlir::detail::PreservedAnalyses(); }

// ---- f50/f51/f52: THE CONSTRUCTORS FOR t81/t82 -----------------------------
// Same argument as f48/f49: a `using tN =` maps the TYPE ONLY, and a type key
// without a constructor gives rc=0 and then `error[E0433]`.  Appended AFTER f49,
// which renumbers nothing.
//
// f50 -- `CopyOnWriteArrayRef(ArrayRef<T> array)` (ADTExtras.h:27), the class's
// ONLY constructor, taking its ArrayRef BY VALUE.  The body is the identity on
// the elements: the C++ stores the view in `nonOwning` and copies into
// `owningStorage` lazily, and `Vec<T1>` collapses both storages into one.
template <typename T1>
mlir::CopyOnWriteArrayRef<T1> f50(llvm::ArrayRef<T1> a) {
  return mlir::CopyOnWriteArrayRef<T1>(a);
}

// f51 -- `DominanceInfo di;`, the 0-ary form (the inherited
// `DominanceInfoBase(Operation *op = nullptr)` with its default argument taken).
// Body is the unit, per t82.
mlir::DominanceInfo f51() { return mlir::DominanceInfo(); }

// f52 -- `DominanceInfo di(op);` / `new DominanceInfo(op)`, the 1-ary form.  The
// body MUST still evaluate `a0`: dropping it would drop the caller's expression
// (commonly `unit_op`, `func`, `module_op`), and dropping an evaluation is the
// mistake the IndexType::get note at the bottom of the type section refuses.  So
// the target takes `a0` and yields a unit built from it, rather than ignoring it.
mlir::DominanceInfo f52(mlir::Operation *op) { return mlir::DominanceInfo(op); }

// ---- f53/f54: the FREE `llvm::operator==` / `operator!=` on two ArrayRefs ----
// Queue rows g325 (`!=`, 2 TUs), g667/g668/g669 (`==`, 1 TU each).  The four rows
// are FOUR INSTANTIATIONS of TWO keys: the converter searched
//   bool llvm::operator==(llvm::ArrayRef<long>, llvm::ArrayRef<long>)
//   bool llvm::operator==(llvm::ArrayRef<mlir::NamedAttribute>, ...)
//   bool llvm::operator==(llvm::ArrayRef<std::pair<mlir::Attribute, ...>>, ...)
//   bool llvm::operator!=(llvm::ArrayRef<long>, llvm::ArrayRef<long>)
// and a generic T1 key covers all of them the way t19 already covers every
// ArrayRef element type.
//
// CALL FORM: these are FREE functions, so the body is the UNQUALIFIED
// `operator==(a, b)`.  Writing `llvm::operator==(a, b)` aborts at
// cpp_rule_preprocessor.cpp:888; writing `a == b` records NOTHING, silently.
// The shape is copied verbatim from rules/vector f115-f118, the four committed
// probe-verified `std::vector` comparison bodies -- including the `T1: PartialEq`
// bound the Rust side needs.
template <typename T1>
bool f53(llvm::ArrayRef<T1> a, llvm::ArrayRef<T1> b) {
  return operator==(a, b);
}

template <typename T1>
bool f54(llvm::ArrayRef<T1> a, llvm::ArrayRef<T1> b) {
  return operator!=(a, b);
}

// ---- f55: the ArrayRef initializer-list constructor -------------------------
// Row g325's site is `if (connectivity_shape != ArrayRef({num_ports,
// num_ports}))`, i.e. it CONSTRUCTS an ArrayRef.  t19 had NO constructor of any
// shape before this (checked: no `ArrayRef::ArrayRef` key anywhere in the
// published IR tree), so f53/f54 alone would have given rc=0 and then E0433 on
// `llvm_ArrayRef::new()` -- the type-key-without-its-ctor failure, seven times
// paid for.  `std::initializer_list<T>` is already modelled
// (rules/initializer_list t1), so the body is the identity on the elements: the
// C++ ArrayRef borrows the list's storage and `Vec<T1>` owns it, the same
// owning/borrowing collapse t19 itself makes.
template <typename T1>
llvm::ArrayRef<T1> f55(std::initializer_list<T1> a) {
  return llvm::ArrayRef<T1>(a);
}

// ---- f56-f60: CopyOnWriteArrayRef's five called members ---------------------
// t81 mapped the TYPE and f50 its ctor; these are the five members the corpus
// reaches, all inside `Ktdp::AccessTileType::Builder` (KtdpTypes.hpp:31-59).
// Over `Vec<T1>` all five are faithful, and the reason is that the C++ class's
// two storages are an ALLOCATION STRATEGY: every one of these operations is
// defined in ADTExtras.h in terms of the logical element sequence, never in
// terms of which storage currently holds it.
//
// f56 -- `size_t size() const` (ADTExtras.h:54), `ArrayRef<T>(*this).size()`,
// i.e. the length of the logical sequence -> `Vec::len`.
template <typename T1> std::size_t f56(const mlir::CopyOnWriteArrayRef<T1> &o) {
  return o.size();
}

// f57 -- `void erase(size_t index)` (ADTExtras.h:41-50).  The three branches
// (drop_front, drop_back, copy-then-vector::erase) are the SAME logical result:
// remove the element at `index`.  `Vec::remove(index)` is exactly that.  It
// RETURNS the removed element where C++ returns void, so the body discards it
// with `drop(..)` -- one expression, yielding `()`.  Corpus site: `dropDim`,
// `shape.erase(pos)` with `unsigned pos`.
template <typename T1> void f57(mlir::CopyOnWriteArrayRef<T1> &o, std::size_t index) {
  return o.erase(index);
}

// f58 -- `void insert(size_t index, T value)` (ADTExtras.h:36-39), whose body is
// `vector.insert(vector.begin() + index, value)`.  VERIFIED, not assumed, against
// the argument shapes the corpus passes (`insertDim`: `shape.insert(pos, val)`,
// `unsigned pos`, `int64_t val`): C++ `vector::insert(begin()+index, value)`
// places `value` so that it becomes the element AT `index`, shifting the rest
// right, and `Vec::insert(index, element)` is documented as exactly that,
// "Inserts an element at position index within the vector, shifting all elements
// after it to the right".  So the unmapped fallback's accidental
// `(*c.borrow_mut()).insert(0_usize, 1_i64)` was right for the right reason, and
// this key makes it a DESIGN rather than a coincidence: argument ORDER matches
// (index first), index BASE matches (0-based, offset from begin()), and the
// out-of-range behaviour matches (C++ asserts `pos <= shape.size()` at the call
// site in KtdpTypes.hpp:55, Rust panics on `index > len`).
template <typename T1>
void f58(mlir::CopyOnWriteArrayRef<T1> &o, std::size_t index, T1 value) {
  return o.insert(index, value);
}

// f59 -- `CopyOnWriteArrayRef &operator=(ArrayRef<T> array)` (ADTExtras.h:29-33):
// it overwrites `nonOwning` and CLEARS `owningStorage`, so afterwards the logical
// sequence is exactly `array`.  Over `Vec` that is a whole-value assignment.
// Corpus site: `setShape`, `shape = newShape`.  The target returns nothing even
// though C++ returns `*this`, exactly as rules/vector f55/f58 do for
// `std::vector::operator=` -- every corpus site discards the result.
template <typename T1>
mlir::CopyOnWriteArrayRef<T1> &f59(mlir::CopyOnWriteArrayRef<T1> &dst,
                                   llvm::ArrayRef<T1> src) {
  return dst.operator=(src);
}

// f60 -- `operator ArrayRef<T>() const` (ADTExtras.h:58-60), the conversion the
// `operator AccessTileType()` site reaches when it passes `shape` to
// `AccessTileType::get`.  It selects whichever storage is live and hands out a
// view; both sides of that choice are `Vec<T1>` in the model, so the body is the
// value itself.  MEMBER CONVERSION OPERATORS are written in member form with the
// destination type named.
template <typename T1>
llvm::ArrayRef<T1> f60(const mlir::CopyOnWriteArrayRef<T1> &o) {
  return o.operator llvm::ArrayRef<T1>();
}

// ---- t83/t84: the two MEASURED terminating gates of 2026-09-27 -------------
// Both arrive through `llvm::SmallVector<...>`; the abort is
//   LLVM ERROR: unsupported unmapped type `X` has no model in types_,
//               while mapping `llvm::SmallVector<X, _>`
// i.e. rules/smallvector already maps the CONTAINER and dies on the ELEMENT, so
// the key that is missing is the ELEMENT TYPE ITSELF, not a SmallVector arity.
// (Confirmed from the LAST `LLVM ERROR` line of four first-abort logs:
// dcc/src/Conversion/AgenToSentient/AccessDetails.cpp and
// dcc/src/Transform/Dataflow/TransformPagedMemView/TransformPagedMemViewImpl.cpp
// for AffineExpr; dcc/src/Dialect/Trace/TraceOps.cpp and
// dataflow-scheduler/.../Dialect/KTDFLowering/KTDFLoweringOps.cpp for
// UnresolvedOperand.)  Neither type has a destructor anywhere in
// mlir/include (`grep -rn '~AffineExpr|~UnresolvedOperand'` = 0 hits), so both
// pass the OwningOpRef/InFlightDiagnostic destructor test.
namespace mlir {

// t83 -- `mlir::AffineExpr`, mlir/IR/AffineExpr.h:69.  A uniquer handle
// (`ImplType *expr`, null-initialised by `constexpr AffineExpr() {}`).
// ⭐ THIS IS NOT A NEW MODELLING DECISION AND IT IS NOT A UNIT.  `dataflowir-gen`
// ALREADY has the real tree at `ir.rs:124` (`enum AffineExpr { Dim, Symbol,
// Constant, Add, Mul, Mod, FloorDiv, CeilDiv }`), and t8 already maps
// `mlir::AffineMap` to `ir::AffineMap` whose field is `results: Vec<AffineExpr>`
// (ir.rs:315).  So `mlir::AffineExpr` -> `ir::AffineExpr` is FORCED by t8: any
// other choice would make `AffineMap::get(..., results, ...)` untypeable.
// A UNIT WOULD HAVE BEEN SILENTLY WRONG HERE, and the corpus proves it: the two
// gating TUs INSPECT and BUILD these expressions --
//   AccessDetails.cpp:229-235  `expr.getKind() == AffineExprKind::Mod` / `::Constant` / `::DimId`
//   TransformPagedMemViewImpl.cpp:94  `getAffineSymbolExpr(sym_idx, context_)`
//   PropagationAnalysis.cpp:468-703   `getAffineDimExpr` + `getAffineBinaryOpExpr(Add/Mul, ...)`
// -- which is exactly the "inspects or builds them" case the AffineMap note at
// t8 and ir.rs:119 both warn about.  The tree carries kind and operands, so
// `getKind` and the builders are all EXPRESSIBLE against it (they are separately
// keyed elsewhere or still mangled; none is claimed here).
// ⛔ WHAT IS STILL LOST, restated from ir.rs:121 because it is the one real
// limit: MLIR CANONICALISES ON CONSTRUCTION (uniqued, flattened, constant-folded)
// and this tree does not.  So the SIMPLIFIERS must stay unmapped --
// `getFlattenedAffineExpr` (TransformPagedMemViewImpl.cpp:429,
// PropagationAnalysis.cpp:1362,1509), `replaceDimsAndSymbols`, `isPureAffine`,
// `getLargestKnownDivisor`, `isMultipleOf`, `walk`.  Same refusal t8 already
// records for `simplifyAffineMap`: a simplifier that returned its input unchanged
// would be silently wrong rather than loudly missing.  NOTHING BELOW MAPS ONE.
// ⭐ ON `==`/`!=` -- and this is the one place this type DIFFERS from t72
// `mlir::TypeID`.  C++ `operator==` compares the uniquer POINTER
// (AffineExpr.h:77, `expr == other.expr`), which for an INTERNED type means
// pointer equality IS structural equality, and `ir::AffineExpr` derives
// `PartialEq`/`Eq` structurally (ir.rs:123).  So mapping them WOULD be faithful,
// unlike TypeID (where the model is `()` and every value would compare equal) and
// unlike OpState (a handle mapped to printed content).  They are nevertheless
// LEFT OUT of this slot: no site in the two gating TUs compares two AffineExprs,
// and `operator==(int64_t)` (AffineExpr.h:79) is a DIFFERENT, non-structural
// overload -- it asks "is this the constant v" -- so keying the handle form
// without it would leave a near-identical spelling silently taking the wrong
// body.  Both are a separate row with its own evidence.
class AffineExpr {
public:
  AffineExpr();
};

// t84 -- `mlir::OpAsmParser::UnresolvedOperand`, mlir/IR/OpImplementation.h:1567:
//     struct UnresolvedOperand { SMLoc location; StringRef name; unsigned number; };
// A PARSER-SIDE TOKEN: the three fields are the source location and the textual
// name (`%42`, `%abc#12`) of a use before it has been resolved to a `Value`.
// ⭐ OPAQUE UNIT, and here the corpus really does only STORE AND FORWARD them --
// every one of the eleven carrying files has the same two-line shape:
//     llvm::SmallVector<OpAsmParser::UnresolvedOperand, N> ops;   // declared
//     parser.parseOperandList(ops, ...)                          // filled
//     parser.resolveOperands(ops, type, loc, result.operands)     // consumed
// (KTDFLoweringOps.cpp:56-61,99-103; TraceOps.cpp:66-107; also SentientOps,
// KtdpOps, Agen, DataflowOps, VectorChain, Symbol, Uniform, KTDFOps, DdlOps.)
// NO field is ever read, no element is ever indexed, and no `.size()` is taken --
// I checked, because `Vec<()>` still has a LENGTH and that is the only property a
// unit element would preserve.  `mlir::OpAsmParser` itself has NO model, so
// `parseOperandList`/`resolveOperands` stay unmapped and every call through them
// still fails LOUDLY -- the t40 `mlir::Pass` / t43 `mlir::OpOperand` precedent:
// map the type so the CONTAINER becomes expressible, map no member.
// ⭐ ON `==`/`!=`: they MUST BE LEFT OUT, and in this case C++ declares NEITHER
// (there is no `operator==` on UnresolvedOperand anywhere in OpImplementation.h).
// Mapping one would be the t72 TypeID error in its purest form -- against a `()`
// model every token would compare equal, so two distinct `%a`/`%b` uses would
// read as the same operand.  Nothing here maps one.
// ⛔ WHAT IS LOST: `location`, `name` and `number` are unreachable.  That is the
// deliberate content of the decision, not an oversight -- the moment a TU reads
// one of them, this key must be replaced by a real three-field struct rather than
// extended, and the read will fail loudly (unmapped member) rather than silently.
class OpAsmParser {
public:
  struct UnresolvedOperand {
    UnresolvedOperand();
  };
};

} // namespace mlir

using t83 = mlir::AffineExpr;
using t84 = mlir::OpAsmParser::UnresolvedOperand;

// f61/f62 -- THE CONSTRUCTORS FOR t83/t84.  A type key without one is rc=0 then
// `E0433: cannot find module or crate mlir_AffineExpr`, measured eight times in
// this tree; the `-verbose` tell is `search expr void T::T(), result: None`.
// Both are the 0-ary form, which is the only form either type has that a
// translated program can write (`constexpr AffineExpr() {}` at AffineExpr.h:73;
// UnresolvedOperand is an aggregate whose implicit default constructor is what
// `SmallVector::resize`/value-init reaches).
mlir::AffineExpr f61() { return mlir::AffineExpr(); }
mlir::OpAsmParser::UnresolvedOperand f62() {
  return mlir::OpAsmParser::UnresolvedOperand();
}

// ---- t85: `mlir::IntegerSet`, the MEASURED terminating gate of
// dcc/src/Conversion/AgenToSentient/Helper.cpp at c9b6ffc --------------------
// Re-measured in this slot with the live binary (NOT pin/cpp2rust), last
// `LLVM ERROR` line of /home/agent/work/fa-out/dcc__src__Conversion__AgenToSentient__Helper.cpp.falog:
//   LLVM ERROR: unsupported unmapped type `mlir::IntegerSet` has no model in
//               types_, while mapping `llvm::SmallVector<mlir::IntegerSet, _>`
// So this is the t83/t84 shape again: rules/smallvector already maps the
// CONTAINER and dies on the ELEMENT, and the `, _` is mapper.cpp:1208's
// `\b\d+\b -> _` erasure of the C++ `2` in `SmallVector<IntegerSet, 2>`
// (Helper.cpp:61).  The missing key is the BARE ELEMENT TYPE; no SmallVector
// arity key is authored here.
namespace mlir {

// t85 -- `mlir::IntegerSet`, mlir/IR/IntegerSet.h.  Like `mlir::AffineExpr`
// (t83) it is an INTERNED IMMUTABLE HANDLE into the MLIRContext uniquer
// (`ImplType *set`, null-initialised by `IntegerSet() : set(nullptr) {}`) whose
// value is a list of affine constraints.
// ⭐ THIS IS NOT A NEW MODELLING DECISION.  `dataflowir-gen` ALREADY has the real
// model at `ir.rs:425`:
//     pub struct IntegerSet { n_dims: u32, n_symbols: u32, constraints: Vec<Constraint> }
//     pub struct Constraint { expr: AffineExpr, is_eq: bool }        // ir.rs:418
// with `IntegerSet::get(n_dims, n_symbols, exprs, eq_flags)` (ir.rs:431) and a
// `Display` that prints `affine_set<(d0)[s0] : (... >= 0, ... == 0)>`.  That
// model is built out of t83's `ir::AffineExpr`, so t85 -> `ir::IntegerSet` is
// FORCED by t83 exactly as t83 was forced by t8's `AffineMap { results:
// Vec<AffineExpr> }`.  A UNIT would have been available here and is REFUSED: the
// `.td` parser's own comment at ir.rs:119 names `IntegerSet::get` as one of the
// two constructors the bridge actually writes, so the content is live data.
// ⭐ THE MLIR BOUNDARY IS NOT CROSSED.  `ir.rs` is GENERATED from the `.td`
// parser and must never be hand-ported; this slot ADDS NOTHING to it and only
// names the model that is already there.  Had `ir.rs` lacked an `IntegerSet`,
// this would have been a dataflowir-gen row, not a rules row.
// ⭐ WHAT THE GATING TU DOES WITH THEM, which is what makes a handle model
// acceptable (Helper.cpp:59-135, 154):
//     IntegerSet time_set;                                   // :59   default ctor -> f119
//     SmallVector<IntegerSet, 2> data_sets;                  // :61   the container
//     data_sets.push_back(load_op.getLoadSet().getValue());  // :64+  stored by value
//     time_set = comp_ind_load_op.getTimeSet().getValue();   // :94+  assigned
//     for (auto& data_set : data_sets)                       // :154  ITERATED
//       affine::FlatAffineValueConstraints constraints(data_set);
// -- so it STORES, COPIES, ASSIGNS and ITERATES them and reads NO member.  The
// one consumer, `affine::FlatAffineValueConstraints`, has NO model, so
// `isHyperRectangular`/`getNumCols` stay unmapped and still fail LOUDLY: the t40
// `mlir::Pass` / t43 `mlir::OpOperand` / t84 precedent -- map the type so the
// CONTAINER becomes expressible, map no member.  NO member of IntegerSet is
// claimed here: not `getNumDims`, `getNumSymbols`, `getNumConstraints`,
// `getConstraint`, `isEq`, `getConstraints`, `getEqFlags`, `getContext`,
// `replaceDimsAndSymbols`, and NOT `IntegerSet::get` itself.
// ⭐ ON `==`/`!=`: LEFT OUT.  C++ `operator==` compares the uniquer POINTER
// (IntegerSet.h, `set == other.set`), and for an interned type that IS structural
// equality, which `ir::IntegerSet` would give faithfully (it derives
// `PartialEq`/`Eq`, ir.rs:424) -- so unlike t72 `mlir::TypeID` it would not be a
// lie.  It is still left out because NO site in the gating TU compares two
// IntegerSets, and the NULL sentinel below would make `IntegerSet() ==
// IntegerSet()` true in both languages but `IntegerSet() == get(0,0,{},{})`
// differ.  That is a separate row with its own evidence.
// ⭐ DESTRUCTOR: NONE.  `grep -rn '~IntegerSet'` over the whole of
// $LLVM_ROOT/include/mlir is ZERO hits, so this passes the
// OwningOpRef/InFlightDiagnostic test that forbids an opaque model for a type
// whose destructor does something.
// ⛔ WHAT IS LOST, and it is the same limit t83 records: MLIR CANONICALISES ON
// CONSTRUCTION and this model does not, so no simplifier is mapped.  Also, a
// default-constructed C++ IntegerSet is NULL, not the EMPTY set; f119 encodes
// that with an `u32::MAX` dim/symbol count (t83's `Symbol(u32::MAX)` sentinel
// idiom) rather than `0/0/vec![]`, so a null handle stays distinguishable from a
// genuinely empty constraint system.  Nothing reads it back today.
class IntegerSet {
public:
  IntegerSet();
};

} // namespace mlir

using t85 = mlir::IntegerSet;

// f119 -- THE CONSTRUCTOR FOR t85.  A type key without one is rc=0 and then
// `E0433: cannot find module or crate mlir_IntegerSet`, measured eight times in
// this tree; the `-verbose` tell is `search expr void T::T(), result: None`.
// 0-ary is the only form a translated program can write here, and it is the form
// Helper.cpp:59 writes.
mlir::IntegerSet f119() { return mlir::IntegerSet(); }

// ---- t86: `llvm::SetVector<T>`, the MEASURED terminating gate of
// dataflow-scheduler/.../KTDFLowToDFIR/OperationLowerings.cpp -----------------
// The abort names the CONTAINER, not an element:
//   LLVM ERROR: unsupported unmapped type `llvm::SetVector<mlir::Attribute>` has
//               no model in types_, while mapping
//               `std::map<mlir::Operation *, llvm::SetVector<mlir::Attribute>>`
// `mlir::Attribute` is ALREADY keyed (t1), so unlike t83/t84/t85 the missing key
// is the container itself.
// ⭐ WHERE `<mlir::Attribute>` COMES FROM, which the earlier triage got wrong: it
// is NOT an MLIR header type.  dt_src spells `SetVector<mlir::Attribute>` zero
// times because it spells it `SetVector<ResourceType>`, and `ResourceType` is a
// bare alias for `mlir::Attribute` declared SIX times in dt_src
// (ApplicableUnits.h:32, SchedulerExtContext.h:28, ComponentClassifier.h:33,
// MemoryTracker.h:37, RoutingGraph.h:48, MemoryTree.h:45).  The gating `std::map`
// is ComponentClassifier.h:38,
// `std::map<mlir::Operation*, llvm::SetVector<ResourceType>>`.
// ⭐ OWNERSHIP.  `rules/smallvector/src.cpp:134-136` says THE RECEIVER DECIDES THE
// MODULE and this file's 1681-1686 note claims `llvm::` types only the MLIR corpus
// reaches.  Every SetVector site in the corpus is under `dataflow-scheduler/` and
// every element type is an MLIR type (`mlir::Attribute` via ResourceType,
// `mlir::Operation*`, `mlir::ktdf::StageOp`, `mlir::ktdf_arch::GroupOp`), so the
// receiver is the MLIR corpus and this is the module -- the SAME rule that put
// t79 `llvm::BitVector` and t19 `llvm::ArrayRef` here.  A separate
// `rules/setvector` module was NOT created: `GetTypeMapKey` (mapper.cpp:94) strips
// at `<`, so the bare name `SetVector` may live in exactly ONE module, and a grep
// of all 80 modules' `ir_src.json` for `SetVector` found ZERO hits, so there is no
// existing claim to respect and no reason to add an 81st module.
// ⭐ WHY `Vec<T1>` AND NOT A HASH SET -- this is the load-bearing measurement.
// `llvm::SetVector` is INSERTION-ORDERED (a vector plus a set used only for
// membership).  THE CORPUS ITERATES THEM, at six sites:
//     LogicalMemoryViewBuilder.cpp:133   for (auto ms : needed_spaces)
//     ComponentClassifier.cpp:68         for (auto comp : temp_non_parallel_components)
//     UnitMaterializer.cpp:62,92,128     for (auto component : ...)
//     UniformInfra.cpp:47,87,153         for (auto component : ...)
// and those loops BUILD MLIR OPS, so the iteration order is the order of the
// emitted IR.  A `HashSet` model would silently permute the emitted output; a
// `Vec` keeps the order exactly, which is why `Vec<T1>` is the only model here
// that cannot lie about what the program prints.  (The same argument t81 makes
// for `CopyOnWriteArrayRef`: the LLVM side's extra membership set is an
// asymptotic optimisation, not something a mapped operation observes.)
// ⭐ WHY EVERY MEMBER IS LEFT UNMAPPED -- the t79 `BitVector` shape, and the
// SECOND measurement is what forces it.  `SetVector::insert` returns `bool`
// (`false` if the element was already present) and THE CORPUS USES THAT RETURN
// VALUE: `DoubleBuffering.cpp:268`, `if (visited.insert(succ))` over the
// `llvm::SmallSetVector<mlir::ktdf::StageOp, 8>` declared at :243 -- the BFS
// visited-set guard, where dropping the bool turns a terminating search into an
// infinite loop.  A `Vec` CAN answer that bool (contains-then-push), but only with
// a body that also performs the dedup, and no such body is measured in this slot.
// So NOTHING is claimed: not `insert`, not `contains`, not `empty`, not `size`,
// not `count`, not `remove`, not `clear`, not `begin`/`end`, not
// `operator[]`, and not the `SmallSetVector` arity form.  Every one of them
// ABORTS LOUDLY, exactly as t79 omits `operator[]` so a bit WRITE cannot silently
// no-op.  In particular a bare `v.insert(x)` must NOT become `v.push(x)`: that
// would drop the dedup and duplicate emitted ops.
// ⭐ DESTRUCTOR TEST: `grep -rn '~SetVector' $LLVM_ROOT/include/llvm/ADT/SetVector.h`
// is ZERO hits -- the class is `set_type set; vector_type vector;` and destruction
// has no observable program effect, so an ordinary mapped value is permitted
// (the OwningOpRef/InFlightDiagnostic test).
// ⛔ WHAT IS LOST: the O(1) membership test, and -- until the members above are
// authored with real dedup bodies -- every operation on the container.  The type
// key exists so that `std::map<mlir::Operation*, SetVector<Attribute>>` and
// `SetVector<ResourceType>`-by-value returns become EXPRESSIBLE; the first call
// still stops the translation, which is the correct state for an unmapped member.
namespace llvm {

// llvm/ADT/SetVector.h -- the real declaration is
// `template <typename T, typename Vector, typename Set, unsigned N> class SetVector`
// with all but T defaulted.  Declared here with ONE parameter, which is all the
// key needs (`GetTypeMapKey` strips at `<`), and with ONLY the default
// constructor, because no member is mapped -- the t79 BitVector / sys::SmartMutex
// declaration style in this file.
template <typename T> class SetVector {
public:
  SetVector();
};

} // namespace llvm

template <typename T1> using t86 = llvm::SetVector<T1>;

// f120 -- THE CONSTRUCTOR FOR t86.  A type key without one is rc=0 and then
// `E0433: cannot find module or crate llvm_SetVector`; the `-verbose` tell is
// `search expr void T::T(), result: None`.  `SetVector() = default;` leaves both
// the vector and the set empty, i.e. an empty Vec.  0-ary is the form every
// corpus site writes (LogicalMemoryViewBuilder.cpp:69,132;
// ComponentClassifier.cpp:33-34).
template <typename T1> llvm::SetVector<T1> f120() {
  return llvm::SetVector<T1>();
}

// t151 -- see the restatement of `mlir::RegisteredOperationName` above for the
// full argument.  Arity 0, so no normalization or swallow hazard.
using t151 = mlir::RegisteredOperationName;

// ---------------------------------------------------------------------------
// PASS 2026-09-28: the two remaining 1-each SmallVector-element gates.
// Both arrive EXACTLY as t83/t84/t85/t151 did -- never named as a type a rule is
// looked up on, but as the bare ELEMENT of a container rules/smallvector already
// maps, so the mapper RECURSES into the element and an element with no model is a
// hard `mapper.cpp:722` abort that emits nothing:
//   `unsupported unmapped type X has no model in types_, while mapping
//    llvm::SmallVector<X>`
// ---------------------------------------------------------------------------
namespace mlir {
namespace func {
// mlir/Dialect/Func/IR/FuncOps.h.inc:574 -- `class CallOp : public ::mlir::Op<
// CallOp, ZeroRegions, VariadicResults, ..., SymbolUserOpInterface::Trait>`, i.e.
// an ORDINARY ODS-GENERATED OP CLASS: one `Operation *` through its `OpState`
// base.  So it maps where t25 (`mlir::OpState`) and t27 (`mlir::scf::ForOp`) map,
// `fmt::OpInst`, and it carries t25's PROHIBITION VERBATIM: ⛔ NO `==`, NO `!=`,
// NO identity test, because a C++ op handle compares `Operation *` while
// `fmt::OpInst` is an op's printed CONTENT -- see the g045 refusal.
//
// NO MEMBER IS KEYED, and the MIS-SERVICE HAZARD WAS CHECKED, not assumed.  A key
// on a DERIVED class cannot relocate an INHERITED member (measured on
// `llvm::FailureOr`: six keys FOUND, all six DEAD), so the question that matters
// is the reverse one -- can an EXISTING key on a BASE serve a member call on
// CallOp and thereby make a unit silently wrong?  It cannot: `grep -n 'OpState::'
// src.cpp` is ZERO HITS, so rules/mlir keys NO OpState member at all.  The
// corpus's member calls therefore all stay LOUD:
//   `call.getLoc()`, `body.getCallee()`   (dbo/src/InitBin.cpp:105)
//   `func::CallOp::create(...)`           (InitBin.cpp:72,105 -- a STATIC member
//                                          on CallOp itself, not inherited)
// No destructor exists (`grep -n '~CallOp' FuncOps.h.inc` = 0 hits, and neither
// `Op` nor `OpState` declares one), so the handle model is permitted -- the
// OwningOpRef test.
class CallOp {};

// mlir/Dialect/Func/IR/FuncOps.h.inc -- `class FuncOp : public ::mlir::Op<FuncOp,
// ...>`, an ORDINARY ODS-GENERATED OP CLASS exactly like CallOp above, i.e. one
// `Operation *` through its OpState base.  Declared here ONLY so t157's key can be
// spelled; the model is argued at `using t157 =`.
//
// SAME MIS-SERVICE CHECK AS CallOp, re-run rather than assumed: `grep -n 'OpState::'
// src.cpp` is still ZERO HITS, so no existing key can serve an inherited member call
// on a FuncOp receiver.  Every corpus read therefore stays LOUD -- `func.getName()`,
// `func.getBody()`, `func.getFunctionType()`, `func.setPrivate()`, `FuncOp::create`.
// No destructor exists (neither `Op` nor `OpState` declares one), so the handle model
// is permitted -- the OwningOpRef test.
class FuncOp {};
} // namespace func

namespace arith {
// mlir/Dialect/Arith/IR/ArithOps.h.inc -- `class ConstantOp : public ::mlir::Op<
// ConstantOp, ...>`, an ORDINARY ODS-GENERATED OP CLASS exactly like func::CallOp,
// func::FuncOp and affine::AffineForOp above, i.e. one `Operation *` through its
// OpState base.  Declared here ONLY so t161's key can be spelled; the model is argued
// at `using t161 =`.
//
// SAME MIS-SERVICE CHECK as the ops above, re-expressed because the `grep -n
// 'OpState::' src.cpp` form the earlier rows cite is now SELF-REFERENTIAL (it returns
// 6 hits, all comment lines quoting the grep).  Re-run as a search for a KEY, not for
// the string: `grep -nE '^(using t[0-9]+ = .*OpState|.*OpState[a-zA-Z_]* [a-z_]+\()'
// src.cpp` is ZERO HITS, and no `tN` in this file names `mlir::OpState` or
// `mlir::Op<...>`.  So no existing key can serve an inherited member call on an
// arith::ConstantOp receiver, and every corpus read stays LOUD --
// `arith::ConstantOp::create` (a STATIC member on ConstantOp itself, 147 sites),
// `constOp.getValue()`, `dyn_cast<arith::ConstantOp>`.
// No destructor exists (`grep -n '~ConstantOp' ArithOps.h.inc` = 0 hits, and neither
// `Op` nor `OpState` declares one), so the handle model is permitted -- the
// OwningOpRef test.
class ConstantOp {};

// mlir/Dialect/Arith/IR/Arith.h:113 -- `class ConstantIndexOp : public
// arith::ConstantOp`.  A HAND-WRITTEN C++ CONVENIENCE SUBCLASS of the ODS op above,
// adding no state (`using arith::ConstantOp::ConstantOp;` plus static `create` and
// `classof`), so it is the SAME `Operation *` handle and, at runtime, the same
// `arith.constant` op.  Declared here ONLY so t162's key can be spelled.
//
// ⚠️ IT SHARES ConstantOp's `DEF` DELIBERATELY, and that is not an approximation: a
// `ConstantIndexOp` IS an `arith.constant` whose result type is `index` -- there is no
// separate ODS `def` for it, and `dataflowir_gen::ops` (read off the pinned rmeta)
// carries `mlir_arith_ConstantOp` and NO `mlir_arith_ConstantIndexOp`, which is the
// model agreeing with the header.
//
// ⛔ BECAUSE IT DERIVES, THIS FILE CANNOT RELOCATE ANY INHERITED MEMBER'S KEY (the
// measured `llvm::FailureOr` lesson): a member declared by `ConstantOp` keys as
// `mlir::arith::ConstantOp::...` whatever is written here.  Nothing is claimed for one.
// No destructor exists (`grep -n '~ConstantIndexOp' Arith.h` = 0 hits), so the handle
// model is permitted.
class ConstantIndexOp : public ConstantOp {};
} // namespace arith

namespace affine {
// mlir/Dialect/Affine/IR/AffineOps.h.inc -- `class AffineForOp : public ::mlir::Op<
// AffineForOp, ...>`, an ORDINARY ODS-GENERATED OP CLASS exactly like func::CallOp and
// func::FuncOp above, i.e. one `Operation *` through its OpState base.  Declared here
// ONLY so t158's key can be spelled; the model is argued at `using t158 =`, including
// why the concept comes from `dataflowir-gen/src/custom.rs` rather than from a `.td`
// (the Affine dialect's include dir is absent from this toolchain).
//
// SAME MIS-SERVICE CHECK AS CallOp/FuncOp, re-run rather than assumed: `grep -n
// 'OpState::' src.cpp` still keys no OpState member, so no existing key can serve an
// inherited member call on an AffineForOp receiver.  No member and no constructor are
// keyed, so every corpus read stays LOUD -- see t158 for the enumerated sites.
class AffineForOp {};
} // namespace affine

namespace linalg {
// mlir/Dialect/Linalg/IR/LinalgInterfaces.h.inc:477 --
// `class LinalgOp : public ::mlir::OpInterface<LinalgOp,
//  detail::LinalgOpInterfaceTraits>`.
// ⭐ MEASURED, NOT PREDICTED: IT IS AN OP INTERFACE, NOT AN OP CLASS.  It does
// NOT derive from OpState; it derives from `mlir::detail::Interface<...,
// Operation *, ...>` (InterfaceSupport.h:94), whose state is the pair
// (`Operation *`, `const Concept *conceptImpl`).
//
// DOES THE t58 / `InterfaceMap` REFUSAL GOVERN IT?  NO -- and that is the point
// worth recording, because it reads as though it should.  t58 refuses to model
// INTERFACE DISPATCH (the sorted TypeID -> concept table); the crate has zero
// notion of it, and t58 is nevertheless MAPPED, as an opaque unit, with every
// member absent.  `LinalgOp` is the interface's VALUE side, and its PAYLOAD IS AN
// OP: the only way a corpus site obtains one is `dyn_cast<LinalgOp>(op)` /
// `walk([](LinalgOp){})`.  So the faithful widening is t27's, `fmt::OpInst`, and
// what is LOST is exactly the `conceptImpl` pointer -- i.e. dispatch, which is
// precisely what t58 already declines to model and what stays LOUD here because
// ⛔ NO MEMBER IS KEYED.  Every read the corpus performs aborts:
//   `linalg_op.emitError(...)`              (ConstructThreeStagePipeline.cpp:258,
//                                            457, 485 -- inherited, and no
//                                            OpState member is keyed either)
//   `linalg_op.getMatchingIndexingMap(...)` (:749, :778)
//   `linalg_op.getDpsInitOperand(...)`      (:777, :1034)
//   `linalg_op.getNumDpsInits()`            (:1033)
//   `compute_op.getOperation()`             (:1032)
// so NO unit is silently wrong -- the `OpAsmParser::Argument` test.  Same
// PROHIBITION as t25/t27: ⛔ no `==`, no `!=`.  No destructor exists (`grep
// '~LinalgOp'` = 0 hits; `detail::Interface` declares none), so the handle model
// is permitted.
class LinalgOp {};
} // namespace linalg
} // namespace mlir

// t152 -- `mlir::func::CallOp`.  Arity 0, so no `\b\d+\b` normalization and no
// last-placeholder swallow hazard.  Gates dbo/src/InitBin.cpp.
using t152 = mlir::func::CallOp;

// f121 -- THE DEFAULT CONSTRUCTOR FOR t152, AND IT IS REQUIRED.  Unlike t151
// (whose type is not default-constructible in C++, so no site can ask for one),
// the GATING TU ITSELF default-constructs one: `func::CallOp found;`
// (dbo/src/InitBin.cpp:41), as do dbo/src/Transforms/Autopilot.cpp:61,:82,:103,
// :229 and PlacePrograms.cpp:182.  Without this key the TU is rc=0 and then
// `E0433: cannot find module or crate mlir_func_CallOp` -- the tell being
// `search expr void T::T(), result: None`.  A default-constructed ODS op handle
// is the NULL handle; `fmt::OpInst` has no null, so the target spells t25's
// unreachable-placeholder init and says so there.
mlir::func::CallOp f121() { return mlir::func::CallOp(); }

// t153 -- `mlir::linalg::LinalgOp`.  Arity 0.  Gates
// dataflow-scheduler/lib/Conversion/frontend/KTIRToScheduleIR/
// ConstructThreeStagePipeline.cpp.
// NO CONSTRUCTOR KEY, and that is checked rather than assumed: `grep -rn
// 'LinalgOp [a-z_]*;' dataflow-scheduler` is ZERO HITS -- every value comes from
// `dyn_cast`, a `walk` lambda parameter, a function parameter, or
// `compute_ops_[0]`, and `SmallVector<LinalgOp> v;` constructs no element.  This
// is t151's and t27's precedent, both committed without a constructor.
using t153 = mlir::linalg::LinalgOp;

// t157 -- `mlir::func::FuncOp`.  Arity 0, so no `\b\d+\b` normalization and no
// last-placeholder swallow hazard.  ⭐ FIRST-ABORT for dcc/src/Transform/Sentient/
// LexicalOrdering.cpp (n=150 census row 70):
//     LLVM ERROR: unsupported unmapped type `mlir::func::FuncOp` has no model in
//     types_, while mapping `llvm::SmallVector<mlir::func::FuncOp>`
// -- the key the mapper searched for is the BARE type, taken verbatim from the TSV;
// the `SmallVector` around it is already modelled, so the element type is the gap.
// It is t152's shape, argued in full there and at t25/t27: an ODS op handle widens to
// `fmt::OpInst`, ⛔ with NO `==`, NO `!=` and NO member keyed.
using t157 = mlir::func::FuncOp;

// f125 -- THE DEFAULT CONSTRUCTOR FOR t157, on t152/f121's precedent and for the same
// measured reason: the corpus really does default-construct one, so without this key a
// TU reaching such a site is rc=0 and then `E0433: cannot find module or crate
// mlir_func_FuncOp`.  Sites, checked not assumed (`grep -rn 'FuncOp [a-z_]*;'`):
//   `func::FuncOp program;`             dbo/src/Transforms/WrapProgramDfir.cpp:88
//   `func::FuncOp first_declaration;`   dbo/src/Transforms/Autopilot.cpp:645
//   `func::FuncOp curr_func_;`          dcc/src/Transform/Sentient/
//                                       SpecializedCanonicalization.cpp:224 (and
//                                       three more as class members under
//                                       dataflow-scheduler/include/.../KTDFToKTDFLow)
// ⚠️ NOTE FOR THE BEFORE/AFTER READER: the GATING TU above does NOT need it --
// `llvm::SmallVector<func::FuncOp> funcs;` (LexicalOrdering.cpp:204) constructs no
// element -- so f125 is licensed by those OTHER sites, not by the row it ships with.
// A default-constructed ODS op handle is the NULL handle and `fmt::OpInst` has no
// null, so the target is t152/f121's unreachable placeholder and says so there.
mlir::func::FuncOp f125() { return mlir::func::FuncOp(); }

// ---------------------------------------------------------------------------
// t158 -- `mlir::affine::AffineForOp`.  Arity 0, so no `\b\d+\b` normalization and
// no last-placeholder swallow hazard.  ⭐ FIRST-ABORT for FOUR n=150 census rows --
// the only repeat offender in the `unsupported unmapped type` tail (4 of 49):
//     LLVM ERROR: unsupported unmapped type `mlir::affine::AffineForOp` has no model
//     in types_, while mapping
//       `std::map<const dsc2::BlockNode *, mlir::affine::AffineForOp>`
//         dsc-based-utils/DSC2ToDataflowIR/V3/SNTransferLowering.cpp
//         dsc-based-utils/DSC2ToDataflowIR/V3/SNComputeLowering.cpp
//         dsc-based-utils/DSC2ToDataflowIR/V3/SNControlFlowLowering.cpp
//     and `llvm::SmallVector<mlir::affine::AffineForOp>`
//         dbo/src/Transforms/sdsc_bundle/LoopUnroll.cpp:76
// Every row aborts on the BARE type: `std::map` and `llvm::SmallVector` are already
// modelled, so the mapper RECURSES into the ELEMENT and the element is the whole gap
// -- t151/t157's situation exactly, and the key spelled here is the bare type taken
// verbatim from the abort text, not the container.
//
// THE MODEL, CHECKED RATHER THAN ASSUMED.  `$LLVM_ROOT/include/mlir/Dialect/Affine/IR/`
// is ABSENT here, so no `.td` describes this op and the concept does NOT come from the
// dataflowir-gen `.td` parser.  It comes from the HAND-WRITTEN custom-printer table:
// `cpp2rust-port/dataflowir-gen/src/custom.rs:10,47,68` registers the op
// `("affine","for")` with its own printer `affine_for(op, ctx)`, citing MLIR's
// `mlir/lib/Dialect/Affine/IR/AffineOps.cpp::AffineForOp::print`.  Its parameter type
// is `&OpInst` (`src/fmt.rs:391  pub struct OpInst`).  So the model's notion of an
// `affine.for` IS an `OpInst`, and NO `repos/dt_src` edit is needed or permitted --
// this stays on the sanctioned side of the do-not-port-MLIR boundary, the same side
// t83/t85/t154 sit on.
//
// REPRESENTATION.  `AffineForOp` is an ORDINARY ODS-GENERATED OP CLASS
// (`::mlir::Op<AffineForOp, ...>`), i.e. one `Operation *` through its `OpState` base,
// so this is t25's representation and t152/t157's immediate precedent: widen to
// `dataflowir_gen::fmt::OpInst`.  ⛔ t25's PROHIBITION APPLIES UNCHANGED: no `==`,
// no `!=`, no identity test -- a C++ op handle compares `Operation *` while an
// `OpInst` is an op's printed CONTENT (the g045 refusal).
//
// ⛔ NO CONSTRUCTOR KEY, AND THAT IS CHECKED, NOT ASSUMED -- check (1) of the two this
// row was held back on.  `grep -nE '(affine::)?AffineForOp[[:space:]]+[A-Za-z_][A-Za-z0-9_]*[[:space:]]*(;|=|\()'`
// over all four gating TUs is ZERO HITS.  Nothing default-constructs one: every value
// comes from `llvm::dyn_cast<...>(loop)`, from the static factory
// `mlir::affine::AffineForOp::create(builder, ...)`, or from a function parameter
// (`performFullUnroll(affine::AffineForOp for_op, ...)` LoopUnroll.cpp:135,144,174).
// `SmallVector<affine::AffineForOp> affine_loops;` (LoopUnroll.cpp:76) and
// `std::map<const dsc2::BlockNode *, AffineForOp>` construct no ELEMENT.  So an `fN`
// here would be a key nobody reaches -- exactly the dead-key trap that retracted four
// `mlir::Location` follow-ons -- and it is deliberately absent.  t151/t153/t27 are the
// precedent (all committed without a constructor); t152/f121 and t157/f125 needed one
// only because a TU really writes `func::CallOp found;` / `func::FuncOp curr_func_;`.
//
// ⛔ NO MEMBER IS KEYED, AND THE MIS-SERVICE HAZARD WAS RE-CHECKED -- check (2).  The
// four TUs DO reach members, so this matters:
//   `affine_for.affine::AffineForOp::hasConstantBounds()`      SNControlFlowLowering.cpp:807
//   `affine_for.affine::AffineForOp::getConstantUpperBound()`  :808
//   `affine_for.affine::AffineForOp::getInductionVar()`        :809
//   `dyn_cast<AffineForOp>(loop).getBody()`                    :780,:788,:847,:882
//   `for_loop.getRegionIterArgs()`                             :742
//   `synthetic_root.getBody(0)`                                :1199
//   `for_op.getOperation()`, `.getLowerBound()`, `.getUpperBound()`, `.getStep()`,
//   `.hasConstantBounds()`, `.getConstantLowerBound()`,
//   `.getConstantUpperBound()`, `.getStepAsInt()`  LoopUnroll.cpp:110,120,151,153,155,175,177,178,179
//   `mlir::affine::AffineForOp::create(...)`  a STATIC member on AffineForOp itself,
//                                            SNTransferLowering.cpp:776,1444 and
//                                            SNControlFlowLowering.cpp:777,785,844,879,1170,1196
// None of those is keyed, and none CAN be mis-served: a key on a DERIVED class cannot
// relocate an INHERITED member (measured on `llvm::FailureOr`: six keys FOUND, all six
// DEAD), and the reverse direction is closed too because `grep -n 'OpState::' src.cpp`
// keys NO OpState member at all (its only two hits are the two comment lines that cite
// this very grep, at t152 and t157).  So EVERY corpus read stays LOUD in the mapper
// rather than returning a plausible lie -- the `OpAsmParser::Argument` test.  The
// EXPECTED consequence, stated up front: these four TUs' first abort MOVES to one of
// those member calls; it does not clear.  That is the intended outcome, not a failure.
//
// No destructor exists (neither `Op` nor `OpState` declares one, and there is no
// `~AffineForOp`), so the handle model is permitted -- the OwningOpRef test.
using t158 = mlir::affine::AffineForOp;

// ---------------------------------------------------------------------------
// t159 -- `mlir::scf::IfOp`.  Arity 0, so no `\b\d+\b` normalization and no
// last-placeholder swallow hazard.  NEWLY EXPOSED BY t158: landing t158 cleared the
// `affine::AffineForOp` type gate on `dsc-based-utils/DSC2ToDataflowIR/V3/
// SNControlFlowLowering.cpp`, whose first abort then became
//     LLVM ERROR: unsupported unmapped type `mlir::scf::IfOp` has no model in types_,
//                 while mapping `std::vector<mlir::scf::IfOp>`
// `std::vector` is already modelled, so the mapper RECURSES into the ELEMENT and the
// element is the whole gap -- t151/t157/t158's situation exactly.  The key spelled
// here is the BARE type taken verbatim from the abort text, not the container.
// The `std::vector<scf::IfOp>` is `std::vector<mlir::scf::IfOp> cmp_list;`
// (SNControlFlowLowering.cpp:73).
//
// THE MODEL IS t27's, ALREADY COMMITTED AND UNCHANGED.  `mlir::scf::ForOp` (t27) is
// the SAME SHAPE -- an ODS-generated op class in the SAME header
// (mlir/include/mlir/Dialect/SCF/IR/SCF.h), i.e. one `Operation *` through its
// `OpState` base -- and it is already mapped to `dataflowir_gen::fmt::OpInst`.  So
// this row adds NO new claim about the model; it spells a second key against the
// representation t25/t27/t152/t153/t157/t158 already share.  No `repos/dt_src` edit
// is needed or permitted: this stays on the sanctioned side of the
// do-not-port-MLIR boundary.
//
// ⛔ t25's PROHIBITION APPLIES UNCHANGED: no `==`, no `!=`, no identity test -- a C++
// op handle compares `Operation *` while an `OpInst` is an op's printed CONTENT (the
// g045 refusal).
//
// ✅ A CONSTRUCTOR KEY *IS* LICENSED HERE, AND THAT IS THE ONE PLACE THIS ROW
// DIFFERS FROM t158 -- check (1), run rather than assumed.
// `grep -nE '(scf::)?IfOp[[:space:]]+[A-Za-z_][A-Za-z0-9_]*[[:space:]]*(;|=|\()'`
// over the gating TU is FIVE default constructions, not zero:
//     mlir::scf::IfOp scf_ifop_tmp;   SNControlFlowLowering.cpp:86, :204, :307
//     mlir::scf::IfOp scf_ifop;       SNControlFlowLowering.cpp:179
//     mlir::scf::IfOp if_op;          SNControlFlowLowering.cpp:1055
// and one more outside it, `mlir::scf::IfOp ifop;`
// (dsc-based-utils/PCFGToDataflowIR/PCFG2ToDataflowIR.cpp:1197).  (The sixth grep hit,
// :180 `mlir::scf::IfOp scf_ifop_cmp_top = cmp_list[...]`, is COPY-initialization from
// a vector element, not a default construction, and licenses nothing.)  So f129 is NOT
// a dead key -- this is t152/f121's and t157/f125's situation, where a TU really
// writes `func::CallOp found;` / `func::FuncOp curr_func_;`.  t158/t151/t153/t27
// remain without a constructor precisely because their greps were ZERO.
//
// ⛔ NO MEMBER IS KEYED, AND THE MIS-SERVICE HAZARD WAS RE-CHECKED -- check (2).  The
// gating TU DOES reach members, so this matters:
//   `scf_ifop_tmp.getResults()[0]`          SNControlFlowLowering.cpp:155, :352
//   `scf_ifop_tmp.getThenBodyBuilder()`     :159, :270, :359
//   `scf_ifop_tmp.getElseBodyBuilder()`     :164, :366
//   `scf_ifop_cmp_top.getResults()[0]`      :185, :190
//   `scf_ifop.getThenBodyBuilder()`         :195
//   `scf_ifop.getElseBodyBuilder()`         :197
//   `mlir::scf::IfOp::create(...)`  a STATIC member on IfOp itself, :149, :186, :189,
//                                  :259, :346
//   `if_op.getLoc()`                        SNTransferLowering.cpp:1980
// NONE of those is keyed and NONE can be mis-served: a key on a DERIVED class cannot
// relocate an INHERITED member (measured on `llvm::FailureOr`: six keys FOUND, all six
// DEAD), and the reverse direction is closed too because `grep -n 'OpState::' src.cpp`
// keys NO OpState member at all -- its only hits are the FOUR comment lines that cite
// this very grep, at t152, t157, t158 and here.  So every corpus read stays LOUD in the
// mapper rather than returning a plausible lie.  ⛔ THE EXPECTED CONSEQUENCE, STATED UP
// FRONT: the gating TU's first abort MOVES to one of those member calls, or the TU
// reaches bucket A carrying `Cpp2RustUnmappedExpr_MemberExpr` for each.  IT DOES NOT
// CLEAR.  An unmapped-TYPE gate clearing is not the compile frontier moving.
//
// No destructor exists (neither `Op` nor `OpState` declares one, and there is no
// `~IfOp`), so the handle model is permitted -- the OwningOpRef test.
using t159 = mlir::scf::IfOp;

// f129 -- THE DEFAULT CONSTRUCTOR FOR t159, on t152/f121's and t157/f125's precedent
// and for the same reason: the gating TU really writes `mlir::scf::IfOp scf_ifop;`
// (five sites listed at t159, plus PCFG2ToDataflowIR.cpp:1197), so this is a key the
// corpus REACHES rather than the dead ctor t158 deliberately omitted.  A
// default-constructed ODS op handle is the NULL handle and `fmt::OpInst` has no null,
// so the target is f121/f125's unreachable placeholder and says so there -- the value
// cannot be read, because no IfOp member is mapped.
mlir::scf::IfOp f129() { return mlir::scf::IfOp(); }

// ---------------------------------------------------------------------------
// t160 -- `mlir::UnrealizedConversionCastOp`.  Arity 0, so no `\b\d+\b`
// normalization and no last-placeholder-swallow hazard.
//
// EXACTLY t158's SHAPE: a TYPE-ONLY key with NO `fN`, because the corpus never
// default-constructs one.  The declaration grep
//   grep -rnE "(mlir::)?UnrealizedConversionCastOp[[:space:]]+[A-Za-z_]\w*[[:space:]]*(;|=|\()"
// is ZERO hits over `repos/dt_src`, and -- because that shape is known to be
// incomplete, having missed `mlir::scf::IfOp if_op, samv_if_op;` for t159 -- the
// multi-declarator grep
//   grep -rnE "UnrealizedConversionCastOp[[:space:]]+[A-Za-z_]\w*[[:space:]]*,"
// was ALSO run: one hit, `PrecisionConversionLowering.cpp:267`, and it is a
// FUNCTION PARAMETER (`matchAndRewrite(mlir::UnrealizedConversionCastOp castOp,`),
// not a declaration.  A loose sweep of every textual occurrence enumerates all
// five name-shaped sites and each is a parameter or an element position:
//   PrecisionConversionLowering.cpp:267  matchAndRewrite parameter
//   PrecisionConversionLowering.cpp:409  `[](mlir::UnrealizedConversionCastOp op)` lambda parameter
//   DataconvElision.cpp:53               `module.walk([&](... castOp)` walk lambda parameter
//   LogicalMemoryViewBuilder.cpp:297     `llvm::SmallVector<...> casts;` -- element position
//   LogicalMemoryViewBuilder.cpp:298     `walk([&](... ucc)` walk lambda parameter
// None needs a default constructor, so unlike t159/f129 there is NO `f` key here,
// and unlike t159 this row deliberately omits the dead ctor exactly as t158 did.
//
// THE MODEL IS ALREADY COMMITTED AND THIS ROW MAKES NO NEW CLAIM.
// `dataflowir_gen::ops::mlir_UnrealizedConversionCastOp` is the very `DEF` that
// EVERY op-handle target in this module already names -- t25, t27, t152, t157,
// t158 and t159 all spell
// `fmt::OpInst::new(<dataflowir_gen::ops::mlir_UnrealizedConversionCastOp as
// dataflowir_gen::MlirOp>::DEF)`.  So the type this key maps is the one op whose
// `DEF` was already load-bearing; it is an ODS-generated op in the Builtin
// dialect with the same single-`Operation *`-through-`OpState` representation as
// t27's `scf::ForOp`.  No destructor exists (neither `Op` nor `OpState` declares
// one, and there is no `~UnrealizedConversionCastOp`), so the handle model is
// permitted -- the OwningOpRef test.
//
// ⛔ MUST STAY REFUSED, and these are the corpus's ONLY other uses of the name:
//   * `addLegalOp<UnrealizedConversionCastOp>()` -- VectorChainLowering.cpp:1952,
//     AgenLowering.cpp:705, ArithmeticLegalization.cpp:105;
//   * `getDefiningOp<mlir::UnrealizedConversionCastOp>()` -- DataconvElision.cpp:57, :71;
//   * `mlir::UnrealizedConversionCastOp::Adaptor` -- PrecisionConversionLowering.cpp:268.
// The first two are the COLLAPSED-TEMPLATE-ARGUMENT trap, now on its EIGHTH
// instance: a member whose only distinguishing argument is an explicit template
// argument records as ONE key, so any body written here would mis-serve every
// other instantiation (`addLegalOp<arith::AddIOp>`, `getDefiningOp<scf::ForOp>`,
// ...) silently.  `::Adaptor` is a nested class the model does not have.  The
// seven `UnrealizedConversionCastOp::create` sites stay unkeyed too, so every
// corpus read of one remains LOUD in the mapper.
//
// ⛔ t25's PROHIBITION CARRIES UNCHANGED: no `operator==`, no `operator!=`, no
// identity test and NO MEMBER is mapped -- a C++ op handle compares
// `Operation *`, whereas an `OpInst` is an op's printed CONTENT, so an equality
// would be a plausible lie rather than a translation.
using t160 = mlir::UnrealizedConversionCastOp;

// ---------------------------------------------------------------------------
// PASS 2026-09-28: the `mlir::Location` row, keyed against the model that landed
// in dataflowir-gen as dt_src `1e03497` (`src/ir.rs:607-800`, re-exported at the
// crate root by `lib.rs:72-73`).  `ir.rs` is HAND-WRITTEN BY DESIGN -- its own
// module doc says so -- and is the sanctioned home for the MLIR builtins the `.td`
// does not describe, the same place `Value`, `Ty`, `Attr`, `AffineExpr` and
// `IntegerSet` live (which is why t83/t85 were FORCED by it).  So these keys do
// not cross the do-not-port-MLIR boundary.
// ---------------------------------------------------------------------------

// t154 -- `mlir::Location` -> `dataflowir_gen::ir::Location` (ir.rs:681).  Arity 0.
// NOT a widening and NOT an opaque unit: the model is the CLOSED hierarchy of
// `Builtin_LocationAttr` defs as a tree (Unknown / FileLineColRange / Name / Fused
// / CallSite), so this is the first mlir type in this module that maps onto a
// structure rather than onto `OpInst`/`Ty::Opaque`.
// ⛔ THERE IS NO NULL VARIANT and none is invented here: `mlir::Location` is
// documented as "a non-nullable wrapper around a LocationAttr" (Location.h:76),
// and every in-band candidate collides with reachable corpus data -- `Unknown` is
// CONSTRUCTED at six sites via `builder.getUnknownLoc()`, and the `-1,-1` that
// `dcc/src/Utils/Utils.cpp:44-47` passes becomes a real `u32::MAX` line.
// NO DEFAULT-CONSTRUCTOR KEY, and this was MEASURED rather than assumed: `grep -rn
// 'Location [a-zA-Z_]*;'` over dcc/dbo/dataflow-scheduler/dxp is ZERO HITS, so no
// site can ask for `mlir_Location::new()`.  What IS needed instead is the FACTORY,
// f124 -- see there.
using t154 = mlir::Location;

// t155 -- `mlir::FileLineColLoc` -> `dataflowir_gen::ir::FileLineColLoc`
// (ir.rs:621).  Arity 0.  A SEPARATE type from t154 on purpose: in C++ it is a
// VIEW over a `FileLineColRange` whose range is degenerate (`class FileLineColLoc
// : public FileLineColRange`, Location.h:174), so `dyn_cast<FileLineColLoc>` is
// NOT "is this a file location" -- a model that let the cast succeed for
// `"f":10:8 to 12:18` would hand back a line the C++ never yields.
// NO DEFAULT-CONSTRUCTOR KEY, same measurement as t154 (`FileLineColLoc [a-z_]*;`
// is ZERO HITS): every corpus value comes from `dcc::utils::getLocation(op)`,
// which is the PROJECT's own function and is ported, not keyed.
using t155 = mlir::FileLineColLoc;

// f122 -- `FileLineColLoc::getLine()` -> `a0.line()`.  ⭐ THIS IS THE KEY THE ROW
// EXISTS FOR: the nine TUs that read location structure all go through
// `dcc::utils::getLocation(op).getLine()` (Liveness.cpp:939,944,
// CFGSSentientLevelConditionalTree.cpp:159, RedundantDefinitionEliminationTree.cpp
// :197, AddressPinningAndToggle.cpp:1479,2525, LoopMerging.cpp:293,294,
// VectorRegisterInitialization.cpp:116, ScalarCopyInsertionForSymbols.cpp:532,550,
// SinkScalarCopy.cpp:103, Utils.cpp:227,230, LoopMaskTree.cpp:113).
// `unsigned` -> `u32`, exact.
unsigned f122(const mlir::FileLineColLoc &a0) { return a0.getLine(); }

// f123 -- `FileLineColLoc::getColumn()` -> `a0.column()`.  Same shape as f122;
// read at AddressPinningAndToggle.cpp:2525, which prints `line:column`.
unsigned f123(const mlir::FileLineColLoc &a0) { return a0.getColumn(); }

// f124 -- `Builder::getUnknownLoc()` -> `Location::Unknown`.  ⭐ THE CONSTRUCTOR
// KEY FOR t154, AND IT IS A FACTORY RATHER THAN A DEFAULT CTOR, which is the whole
// point: `mlir::Location` has no meaningful default construction, so the
// rc=0-then-E0433 trap cannot be closed by a `void T::T()` key here -- there is no
// such expression in the corpus to close.  The expression the corpus DOES write is
// `builder.getUnknownLoc()`, at six sites (Deuniform.cpp:238,241,
// LoopRolling.cpp:831, OldRegisterInitialization.cpp:929,932,941,
// SetSendDestinationRE.cpp:157, MultiDimLoopPeeling.cpp:640, BurstUtils.cpp:139,
// Transformer.cpp:203,207,214, SCFToSentient.cpp:205, StandardToSentient.cpp:215+,
// TransformLoopToLegalizeForSentientLowering.cpp:107, LitAutoTestGen.cpp:1144).
// ⭐ IT IS KEYED ON `Builder`, NOT `OpBuilder`, DELIBERATELY: `getUnknownLoc` is
// declared by `Builder` in mlir/IR/Builders.h and INHERITED by `OpBuilder`, and a
// rule file CANNOT relocate an inherited member's key (measured on
// `llvm::FailureOr`: six keys FOUND, all six DEAD).  So this ONE key serves both
// the `Builder` and the `OpBuilder` receivers the brief lists.
// The receiver is t59, which maps to the OPAQUE UNIT `()` -- and that is exactly
// right for this member and for no other: an unknown location carries no
// information from the builder, so nothing is lost by having no builder state.
// Every OTHER Builder member (`getIndexType()`, `getI32IntegerAttr(...)`) remains
// UNKEYED and still aborts loudly.
mlir::Location f124(mlir::Builder &a0) { return a0.getUnknownLoc(); }

// ---------------------------------------------------------------------------
// PASS 2026-09-28: the `EffectInstance` row.  The MODEL landed in dataflowir-gen
// as dt_src `c7e551a` (`pub mod ods_effects`, emitted by `build.rs:600-720`, with
// `EffectKind` DERIVED from MLIR's own `Interfaces/SideEffectInterfaces.td` rather
// than tabled, and re-exported at the crate root by `lib.rs:78`).  Only the TYPE
// key is written here; the reasons the accessor/producer keys are NOT are recorded
// below, because each is a DECISION and not an oversight.
// ---------------------------------------------------------------------------

// t156 -- `mlir::SideEffects::EffectInstance<mlir::MemoryEffects::Effect>` ->
// `dataflowir_gen::EffectInstance` (= `ods_effects::EffectInstance`).  ⭐ THIS IS
// THE GATE: `dialects/VarExpr/VarExprOps.cpp` dies at
//   `LLVM ERROR: unsupported unmapped type
//    mlir::SideEffects::EffectInstance<mlir::MemoryEffects::Effect> has no model in
//    types_, while mapping llvm::SmallVectorImpl<...>`
// i.e. the abort is the ELEMENT type of the SmallVectorImpl a `getEffects` override
// takes by reference, reached before any member call.
//
// THE CONCRETE INSTANTIATION IS KEYED, NOT A PLACEHOLDER FAMILY, and deliberately:
// the corpus has EXACTLY ONE instantiation (98 generated
// `void <Op>::getEffects(SmallVectorImpl<EffectInstance<MemoryEffects::Effect>> &)`
// definitions across 16 `*.cpp.inc` files, all with the same argument), and a
// 1-placeholder key would bucket under `mlir::SideEffects::EffectInstance` and its
// placeholder would swallow whatever follows to the final depth-0 `>`.
//
// WHY THE MODEL IS FAITHFUL HERE: the list is HETEROGENEOUS and the kind is
// PER-INSTANCE.  `SentientOps.cpp.inc:10350 LoadAndStoreOp::getEffects` emplaces an
// op-level `Write` AND a loop of per-operand `Read`s into the SAME
// `SmallVectorImpl` -- five entries, kinds `[Write, Read, Read, Write, Write]`,
// operand indices `[None, 2, 3, 5, 6]` -- so `EffectKind` sits on the instance and
// not on the container, which is what `ods_effects` does.
//
// NO DEFAULT-CONSTRUCTOR KEY, and this one is settled by the header rather than by
// a grep: `EffectInstance` (Interfaces/SideEffectInterfaces.h:139-199) declares
// TWELVE constructors and NOT ONE of them is a default constructor, so no C++ site
// can write `EffectInstance x;` and the rc=0-then-E0433 trap has nothing to bind
// to here.
//
// ⛔ WHAT IS DELIBERATELY LEFT OUT, so it keeps failing loudly:
//
//  1. THE PER-OPERAND `emplace_back` ARITY -- 16x `Read::get(), &getOperation()->
//     getOpOperand(idx), 0, false, DefaultResource::get()` and 14x the same with
//     `Write::get()`.  The model's `new_on_operand` takes an INDEX; the C++ passes
//     an `OpOperand *` formed from a LIVE `Operation`, which a ported body does not
//     have.  There is no honest key for `&getOperation()->getOpOperand(idx)` and I
//     will NOT invent an `OpOperand` model to make it typecheck, so this arity is
//     OUT and the 30 sites that use it still abort.  Consequence, and it is why
//     `Read::get()` is also unkeyed: `Read` appears ONLY in this arity, so a
//     `Read::get()` key would be a DEAD key.  Same for `Free`, which no producer
//     emplaces at all.
//
//  2. `hasEffect<Effect>()` -- THE TEMPLATE ARGUMENT COLLAPSES THE KEY, so ONE key
//     would have to answer for all four queries.  It is declared
//     `template <typename Effect> bool hasEffect()`, a member of
//     `mlir::MemoryEffectOpInterface` (`SideEffectInterfaces.h.inc:80), WITH NO
//     FUNCTION PARAMETERS -- the effect being asked about appears only as an
//     EXPLICIT template argument, which is not part of the recorded signature.  So
//     `hasEffect<MemoryEffects::Write>()`, `<Allocate>()`, `<Free>()` and
//     `<Read>()` -- the four the consumer at
//     `dr5/src/Passes/SPMDizer/SPMDizer.cpp:239-246` writes -- ALL record as the
//     same `bool mlir::MemoryEffectOpInterface::hasEffect()`, and any single body
//     would answer identically for effects the C++ distinguishes.  The only
//     mechanism that could spell the argument is `regen-rule.sh:68`'s
//     `explicit-template-args` marker, which changes how the WHOLE module records
//     and has NO user today; rules/mlir is the most-depended-on module and is not
//     the place to find out what that does.  So: OUT, and it needs that decision
//     rather than a key.
//
//  3. `isMemoryEffectFree` -- THE ARGUMENTS DO NOT CORRESPOND.  C++ is
//     `bool isMemoryEffectFree(Operation *op)` (SideEffectInterfaces.h:472); the
//     model is `is_memory_effect_free(&[EffectInstance])`.  One takes an op and
//     walks it (including nested regions for `HasRecursiveMemoryEffects`, and
//     answering FALSE for an op carrying no interface at all); the other asks only
//     "did this list come out empty".  That is a different question, not a
//     narrowing, so there is no key -- same reason `getFilename()` is out at t155.
//     The model function stays for the ported `getEffects` bodies to use once the
//     producer arities exist.
//
//  4. THE OP-LEVEL `emplace_back` ARITY (22x `Write::get(), 0, false,
//     DefaultResource::get()`, 1x `Allocate::get(), ...`) IS ALSO OUT, and this is
//     the one I would most like to have written.  It cannot be keyed alone: the
//     receiver is `llvm::SmallVectorImpl<T>::emplace_back(ArgTypes &&...)`, a
//     VARIADIC FORWARDING template, so there is no in-place `EffectInstance(...)`
//     constructor expression in the AST for a ctor key to match, and the argument
//     `Write::get()` needs its own key -- which in turn needs a mapping for
//     `mlir::MemoryEffects::Write *`, whose `get()` is declared by
//     `mlir::SideEffects::Effect::Base<DerivedEffect, BaseEffect = Effect>`
//     (SideEffectInterfaces.h:42) and therefore carries a DEFAULTED TEMPLATE
//     ARGUMENT into the key, plus a derived-to-base pointer conversion
//     (`DefaultResource *` -> `Resource *`) at the last parameter, which is exactly
//     the shape that makes the converter emit a base cast between two spellings
//     that are the same Rust type.  Three unmeasured hazards stacked in one key is
//     not something to land in this module on a guess.  t156 alone clears the
//     types_ abort; the producer bodies remain unported and loud.
using t156 = mlir::SideEffects::EffectInstance<mlir::MemoryEffects::Effect>;

// ---------------------------------------------------------------------------
// PASS 2026-09-28: THE FREE `llvm::raw_ostream <<` FAMILY, three keys.
//
// The `unsupported CXXOperatorCallExpr:` diagnostic (a718367) names the exact
// spellings the mapper searched for, over the 98 bucket-A census TUs:
//     llvm::raw_ostream & operator shl(llvm::raw_ostream &, mlir::Type)             16
//     llvm::raw_ostream & operator shl(llvm::raw_ostream &, mlir::Attribute)         3
//     llvm::raw_ostream & operator shl(llvm::raw_ostream &, const mlir::Location &)  1
// The 16 are only FOUR distinct source locations
// (dcc/src/Transform/Sentient/LexicalOrdering.cpp:132,133,152,153), each reported
// by four different logs; the ceiling of this row is 20 sites, not 498.
//
// WHY THE DECLARATIONS ARE AT GLOBAL SCOPE, UNQUALIFIED.  A free two-parameter
// operator records UNQUALIFIED in this tree.  The precedent is the one existing
// free two-parameter key in the baseline, `std::byte operator shr(std::byte,
// unsigned int)`: rules/cstddef/src.cpp:10 writes `operator>>(a0, a1)` at global
// scope, the real function lives in namespace std, and NO `std::` is recorded.
// The real MLIR operators are in namespace mlir (mlir/IR/Types.h:145,
// Attributes.h:107, Location.h:110) and record the same way.
//
// `llvm::raw_ostream` is INCOMPLETE here on purpose: a reference parameter and a
// reference return need no definition, and reaching real LLVM headers is not
// possible from cpp-rule-preprocessor's fixed flag set (see the header of this
// file).  `llvm::raw_ostream &` is already mapped tree-wide by rules/raw_ostream's
// t2, so the receiver type needs nothing from this module.
//
// ⛔ THE OTHER FOUR KEYS OF THIS FAMILY ARE REFUSED, and must stay refused.
// Each was settled against the MLIR header, and each would print something
// structurally different from the C++:
//   * `mlir::Value` (8 sites) -- Value.h:246 is `{ value.print(os); return os; }`
//     and `Value::print` (Value.h:223) prints the DEFINING OPERATION's full
//     printed form, not the SSA name.  `ir::Value` is `{ name, ty }` and has NO
//     Display impl at all, so any body would print `%7` where C++ prints a whole
//     op line.  Silent wrongness.
//   * `const mlir::Operation &` (17) and `mlir::OpState` (8) -- Operation.h:1100
//     and OpDefinition.h:315 are both
//     `op.print(os, OpPrintingFlags().useLocalScope())`.  `fmt::OpInst` has no
//     Display, and this file already refuses the flags themselves (see the
//     OpPrintingFlags note above): dataflowir-gen has ONE printer and it is NOT
//     configurable.
//   * `mlir::OperationName` (8) -- OperationSupport.h:507 is `info.print(os)`,
//     the full registered `dialect.mnemonic`.  t18's model is
//     `Option<&'static TdOpDef>` and TdOpDef's fields (td.rs:446-457) carry no
//     full-name field and no dialect prefix.
// Nor is the deliberate `mlir::OpState ==` refusal above reopened here.
// ---------------------------------------------------------------------------

namespace llvm {
// Incomplete on purpose -- see above.  rules/raw_ostream owns this type.
class raw_ostream;
} // namespace llvm

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, mlir::Type t);
llvm::raw_ostream &operator<<(llvm::raw_ostream &os, mlir::Attribute a);
llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const mlir::Location &l);

// f126 -- `mlir::Type` BY VALUE (Types.h:145 takes `Type type`), 16 counts /
// 4 distinct sites.  The rendered text is `ir::Ty`'s Display (ir.rs:50), which is
// exactly the argument f25 makes for the InFlightDiagnostic `mlir::Type &` key in
// this same module: Display IS `mlir::Type::print`'s builtin-type syntax, and
// `Ty::Opaque` exists precisely so a dialect type round-trips by its own spelling.
llvm::raw_ostream &f126(llvm::raw_ostream &a0, mlir::Type a1) {
  return operator<<(a0, a1);
}

// f127 -- `mlir::Attribute` BY VALUE (Attributes.h:107), 3 sites in 3 TUs.
// f36's argument, unchanged: `ir::Attr`'s Display (ir.rs:521) is MLIR's attribute
// syntax.
llvm::raw_ostream &f127(llvm::raw_ostream &a0, mlir::Attribute a1) {
  return operator<<(a0, a1);
}

// f128 -- `const mlir::Location &` (Location.h:110), 1 site.  `Location`'s Display
// (ir.rs:798) is documented as "The BARE location spelling, as `LocationAttr`'s
// printer produces it" -- which is what the C++ reaches, because Location.h:110 is
// `loc.print(os)` on the LocationAttr, NOT the `loc(...)` trailing-location syntax
// of an enclosing op.
llvm::raw_ostream &f128(llvm::raw_ostream &a0, const mlir::Location &a1) {
  return operator<<(a0, a1);
}

// ---------------------------------------------------------------------------
// ⭐ SUPERSEDED 2026-09-28 by t460-t463 / f360-f366 BELOW.  The refusal that
// follows is kept verbatim because its ONE load-bearing premise -- "there is no
// sink, writer or stream type anywhere in dataflowir-gen" -- was TRUE when it was
// written and is now FALSE: `dataflowir-gen` commit 8dee457 added `src/asm.rs`
// `pub struct AsmPrinter`, re-exported at the crate root (lib.rs:86), and
// `cc2.rs:44` `impl ByteRepr for AsmPrinter` is what lets a refcount rule body
// hold a `Ptr<AsmPrinter>` at all.  READ THE REFUSAL ANYWAY: every clause of it
// except that premise still constrains what may be keyed, and the ValueTypeRange
// and parser halves are still refused below for reasons it gets right.
//
// ⛔ REFUSED 2026-09-28: THE ENTIRE `mlir::OpAsmPrinter` / `mlir::AsmPrinter`
// `<<` FAMILY -- 301 sites over 8 TUs, of which the single largest key in the
// whole corpus, `mlir::OpAsmPrinter & operator shl(mlir::OpAsmPrinter &,
// const char (&)[_])`, is 170.  Recorded so the next slot does not re-derive it.
//
// WHAT `OpAsmPrinter` IS IN THE MODEL: NOTHING.  There is no sink, writer or
// stream type anywhere in dataflowir-gen.  Its printer is a PROJECTION of
// `let assemblyFormat` (fmt.rs:1), and it produces a VALUE, not a sequence of
// writes: `fmt::OpInst::print()` (fmt.rs:716) and `print_in()` (fmt.rs:725)
// return `Result<String, PrintError>`.  The only state the printer carries is
// `fmt::PrintCtx` (fmt.rs:372) -- a `Copy` struct of `{ indent: usize,
// default_dialect: &str }`.  You cannot write a byte into it.
//
// AND THE SHAPES DISAGREE, which is the real reason.  The model's counterpart of
// a tablegen custom `print()` body is `custom.rs:66`
// `print_custom(op: &OpInst, ctx: &PrintCtx) -> Option<Result<String, PrintError>>`
// -- a PURE FUNCTION OF THE OP returning the printed text.  The C++ is the
// opposite shape: a void member that mutates a sink passed in from MLIR's
// `OperationPrinter`.  There is no `a0` for a rule to bind, so even the
// char-array overload -- the one operand this file could print faithfully --
// would first need a `tN` inventing a sink that the model does not have.
// `rules/raw_ostream`'s `std::fs::File` is not that sink: a raw_ostream IS a
// file descriptor, whereas an OpAsmPrinter carries the SSA-name map, the alias
// state, the indent and the default-dialect stack, and its other members
// (`printOptionalAttrDict`, `printOperand`, `printRegion`, `getStream`) have no
// counterpart at all.  A `String` target would make `p << "lit"` type-check
// while leaving every one of those loud-or-silent.
//
// THE ROW IS ALSO ON THE WRONG SIDE OF THE MLIR BOUNDARY, and this is
// measurable: of the 170 DISTINCT source locations of the char-array key,
// 149 (88%) are in ONE generated file,
// `toolchain/gen-inc/ddc/ddl/Dialect/DdlOps.cpp.inc`, i.e. the
// assemblyFormat-generated printer that `fmt.rs` already is a projection of.
// Only 12 are in hand-written project sources (6 `ddc/ddl/Dialect/DdlOps.cpp`,
// 4 `KTDF/KTDFOps.cpp`, 2 `Symbol/Symbol.cpp`).  Modelling a sink here would
// port MLIR's asm printer a second time.
//
// The other operand overloads (`mlir::Value` 27, `const mlir::OperandRange &`
// 28, `mlir::ValueTypeRange<...>` 4, `mlir::Type` 4) are refused for exactly the
// reasons already recorded for the `llvm::raw_ostream` family above; nothing
// about an OpAsmPrinter receiver changes them.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// t460-t463 / f360-f366 -- THE MLIR PRINTER SINK AND ITS `<<` FAMILY.
//
// ⭐ WHAT CHANGED: `dataflowir_gen::AsmPrinter` (dataflowir-gen/src/asm.rs:105) is
// the addressable sink the refusal above said did not exist.  It carries exactly
// the four state items that refusal named as missing -- the text buffer (`out`),
// the SSA-name map (`set_value_name`/`ssa_name`/`name_of`), the alias state
// (`with_aliases`), the indent (`indent`/`set_indent`/`increase_indent`/
// `decrease_indent`) -- plus `default_dialect` as a REAL STACK
// (`push_default_dialect`/`pop_default_dialect`), which `fmt::PrintCtx`'s single
// `&str` could not express.  `ctx()` projects a `fmt::PrintCtx` out of it, so the
// pure printing path does the actual work and nothing is re-implemented.
//
// ⭐ AND THE SINK IS PROVEN NOT TO DROP THE BYTES, which is the one failure worse
// than the placeholder: `dataflowir-gen/tests/asm.rs` carries
// `a_known_op_through_the_sink_is_byte_identical_to_the_pure_printing_path`
// (matching `matmul_demo` lines 55 and 1477 verbatim) plus an all-rows test --
// 244 rows printed identically through the sink, 154 refused with the IDENTICAL
// `PrintError`.  The job HERE is not to undo that at the key layer, so every
// method named in a body below was checked to exist in `asm.rs`'s public surface.
//
// THE SHAPE, and why it is `rules/raw_ostream`'s and not a new invention.  These
// keys are FREE functions, two arguments, receiver first -- `OpImplementation.h`'s
//   template <typename AsmPrinterT, typename T>
//   std::enable_if_t<...> operator<<(AsmPrinterT &p, const T &value)
// at a concrete `AsmPrinterT`/`T`, exactly as `llvm::raw_ostream &operator<<(
// llvm::raw_ostream &, ...)` is keyed at :3490-3492 and f126-f128.  So the
// RECEIVER TYPE IS A SEPARATE TYPE KEY FROM THE REFERENCE TO IT, the t1/t2 split
// rules/raw_ostream already makes (`using t1 = llvm::raw_ostream;` /
// `using t2 = llvm::raw_ostream &;`), and the `&` key is the one the bodies bind:
// `Ptr<AsmPrinter>` in the refcount model, `*mut AsmPrinter` in the unsafe one.
// ⚠️ THE RETURN TYPE IS THE PRINTER BY REFERENCE, NOT A FRESH SINK -- `p << a << b`
// chains, so each body must hand THE SAME sink back.  Every body below therefore
// ends by returning its own `a0`, the f5-f18 invariant of rules/raw_ostream.
//
// ⭐ THE RECEIVERS ARE DECLARED AS TWO UNRELATED EMPTY CLASSES ON PURPOSE.  Real
// MLIR has `class OpAsmPrinter : public AsmPrinter`, and if that inheritance were
// spelled here the `AsmPrinter &` overloads would win overload resolution for an
// `OpAsmPrinter` argument by derived-to-base conversion and the recorded key would
// read `mlir::AsmPrinter &...` -- the WRONG key, silently, for 240 of the 331
// sites.  Nothing in this file needs the base relation (there is no rule that
// converts one to the other), so it is not written.
//
// ⚠️ `const char (&)[_]`: the extent below is written CONCRETELY (36) because
// `normalizeTranslationRule` rewrites every `\b\d+\b` in a key to `_`, which is
// what makes ONE key serve every literal length -- the f21 mechanism, unchanged.
// A string-literal extent has no finite key set, so that erasure must stay.
//
// ⛔ THREE SPELLINGS ARE DELIBERATELY LEFT OUT, each so it keeps FAILING LOUDLY:
//   * `mlir::ValueTypeRange<mlir::ResultRange>` (3 sites) and
//     `mlir::ValueTypeRange<mlir::OperandRange>` (1).  `AsmPrinter::print_types`
//     exists and would serve them, but the ARGUMENT TYPE HAS NO TYPE KEY: t251's
//     own note (:5225) records that `ValueTypeRange` (TypeRange.h:120-165) is a
//     DIFFERENT class from the `TypeRange` it keys and "is not keyed here",
//     because unlike `TypeRange` it DECLARES `front()` -- a reference-returning
//     accessor, which is precisely the thing the t14-t17 owning-`Vec` aliasing
//     licence does not cover.  Keying the `<<` without the type would not even
//     resolve; keying the type is a separate row that must re-derive that licence.
//   * `llvm::raw_ostream << mlir::Value`.  ABSENT FROM THE MODEL ON PURPOSE and it
//     must stay absent: `Value.h:246` is `value.print(os)`, the DEFINING
//     OPERATION's full form, which `ir::Value` ({ name, ty }, no back-pointer)
//     cannot reach.  A key here could only print the SSA name -- a plausible wrong
//     answer.  ⚠️ Note this is the exact opposite of f363 below, and the
//     difference is the receiver: `OpImplementation.h`'s
//     `operator<<(AsmPrinterT &, Value)` body IS `p.printOperand(value)`, so on a
//     PRINTER receiver the SSA name is the right answer and on a raw stream it is
//     not.
//   * `mlir::OpAsmParser` / `mlir::AsmParser` / `llvm::SourceMgr`, ~119 sites.  The
//     parser half of `asm.rs` was left absent deliberately -- a parser needs a real
//     lexer, a `SourceMgr` owning the buffer and location tracking, none of which is
//     derivable from the `.td` rows -- so any key would name a function that does
//     not exist.  Loud on purpose.
//
// ⛔ AND NO MEMBER IS KEYED, the t166 / t420-t422 discipline, for the reason the
// `DialectBytecodeReader` refusal at the end of this file states: AN UNMAPPED
// MEMBER DOES NOT ABORT.  `p.printOptionalAttrDict(...)`, `p.printRegion(...)`,
// `p.getStream()` are emitted TEXTUALLY at rc=0 with no placeholder token.  That
// is a pre-existing condition of these TUs, not something these keys introduce --
// the `<<` sites are the ones that abort today -- and each of those members is its
// own row with its own fidelity argument (`getStream()` in particular has NO
// counterpart: `asm.rs` is not a stream).
// ---------------------------------------------------------------------------

namespace mlir {

// mlir/IR/OpImplementation.h -- `class AsmPrinter` (:34) and
// `class OpAsmPrinter : public AsmPrinter` (:400).  Restated as EMPTY and
// UNRELATED; see the overload-resolution paragraph above for why the base
// relation is omitted, and note `class StringRef {}` / `class StringLiteral {}`
// at :1242 as the in-tree precedent for an inert declaration that maps nothing on
// its own.
class AsmPrinter {};
class OpAsmPrinter {};

OpAsmPrinter &operator<<(OpAsmPrinter &p, const char (&s)[36]);
OpAsmPrinter &operator<<(OpAsmPrinter &p, const char &c);
OpAsmPrinter &operator<<(OpAsmPrinter &p, const OperandRange &r);
OpAsmPrinter &operator<<(OpAsmPrinter &p, Value v);

AsmPrinter &operator<<(AsmPrinter &p, const char &c);
AsmPrinter &operator<<(AsmPrinter &p, const llvm::StringRef &s);
AsmPrinter &operator<<(AsmPrinter &p, const llvm::StringLiteral &s);

} // namespace mlir

// t460 -- `mlir::OpAsmPrinter`, the sink BY VALUE.  54 reported sites.
using t460 = mlir::OpAsmPrinter;

// t461 -- `mlir::OpAsmPrinter &`, the form every site actually holds, and the one
// f360-f363 bind.  rules/raw_ostream t2 is the precedent.
using t461 = mlir::OpAsmPrinter &;

// t462 / t463 -- the same pair for `mlir::AsmPrinter`.  40 reported sites.
using t462 = mlir::AsmPrinter;
using t463 = mlir::AsmPrinter &;

// f360 -- `mlir::OpAsmPrinter & operator shl(mlir::OpAsmPrinter &,
// const char (&)[_])`, 155 sites, the largest single key in the corpus.
// `AsmPrinter::print_str` appends VERBATIM -- no leading space, no bookkeeping --
// which is what a `p << "lit"` in a hand-written ODS printer means.
mlir::OpAsmPrinter &f360(mlir::OpAsmPrinter &a0, const char (&a1)[36]) {
  return mlir::operator<<(a0, a1);
}

// f361 -- `const char &`, 34 sites.  `AsmPrinter::print_char`.
mlir::OpAsmPrinter &f361(mlir::OpAsmPrinter &a0, const char &a1) {
  return mlir::operator<<(a0, a1);
}

// f362 -- `const mlir::OperandRange &`, 26 sites.  `AsmPrinter::print_operands`
// is `interleaveComma(values, os, [&](Value v) { printOperand(v); })`: comma-space
// separated SSA names, NO brackets, which is what `printOperands` emits.  t14
// already models `mlir::OperandRange` as an owning `Vec<ir::Value>` and this is a
// READ-ONLY use of it, so the t14-t17 aliasing licence covers it unchanged.
mlir::OpAsmPrinter &f362(mlir::OpAsmPrinter &a0, const mlir::OperandRange &a1) {
  return mlir::operator<<(a0, a1);
}

// f363 -- `mlir::Value` BY VALUE (OpImplementation.h takes `Value value`), 25
// sites.  ⭐ THIS PRINTS THE SSA NAME `%7` AND THAT IS CORRECT HERE: the header's
// body is literally `p.printOperand(value)`.  See the refusal paragraph above for
// why the raw_ostream spelling of the same argument is the opposite answer.
mlir::OpAsmPrinter &f363(mlir::OpAsmPrinter &a0, mlir::Value a1) {
  return mlir::operator<<(a0, a1);
}

// f364 -- `mlir::AsmPrinter & operator shl(mlir::AsmPrinter &, const char &)`,
// 1 site.  f361's body on the base receiver; a DISTINCT KEY, not a duplicate,
// because the receiver type is part of the signature the mapper prints.
mlir::AsmPrinter &f364(mlir::AsmPrinter &a0, const char &a1) {
  return mlir::operator<<(a0, a1);
}

// f365 -- `const llvm::StringRef &`, 1 site.  rules/stringref models a StringRef
// as a NUL-TERMINATED Vec whose `size()` is `len()-1`, so the target body drops
// the terminator -- the rules/raw_ostream f7 fix, whose absence once appended a
// stray 0x00 to every chained insertion.
mlir::AsmPrinter &f365(mlir::AsmPrinter &a0, const llvm::StringRef &a1) {
  return mlir::operator<<(a0, a1);
}

// f366 -- `const llvm::StringLiteral &`, 1 site.  Same byte payload as f365
// (rules/stringref t2 gives StringLiteral the same Vec), so the same body.
mlir::AsmPrinter &f366(mlir::AsmPrinter &a0, const llvm::StringLiteral &a1) {
  return mlir::operator<<(a0, a1);
}

// ---------------------------------------------------------------------------
// t161 -- `mlir::arith::ConstantOp`.  Arity 0, so no `\b\d+\b` normalization and no
// collapsed-argument exposure.  Queue row g110 (`system type has no rule:
// `mlir::arith::ConstantOp``, searched as `mlir::arith::ConstantOp`, which is what is
// spelled below).  Independently confirmed REACHED, not inferred: the emitted Rust of
// the n=150 census already carries the placeholder
// `Cpp2RustUnmapped_mlir_arith_ConstantOp`
// (e.g. /home/agent/work/cens0928c/out/dialect_utils__Agen__Utils.cpp.rs).
//
// NO `fN` DEFAULT CONSTRUCTOR, and this is measured rather than assumed.  The
// type-key-needs-a-constructor trap only bites where the corpus DEFAULT-constructs:
//   grep -rnE "(mlir::)?arith::ConstantOp[[:space:]]+[A-Za-z_]\w*[[:space:]]*(;|=|\()"
//     --include=*.cpp --include=*.h repos/dt_src   ->  2 hits, and NEITHER is a default
//     construction: both are LexicalOrdering.cpp:116,117
//     `mlir::arith::ConstantOp const_a = dyn_cast<mlir::arith::ConstantOp>(a);`
//     i.e. copy-initialization from a call.
//   the multi-declarator form that the simple grep missed for `scf::IfOp` --
//   grep -rnE "arith::Constant(Index)?Op[[:space:]]+[A-Za-z_]\w*[[:space:]]*,"
//     -> 1 hit, BufferizationAnalysis.cpp:103 `analyseConstantOp(arith::ConstantOp
//     constantOp,` which is a FUNCTION PARAMETER, not a declarator list.
// So there is no `arith::ConstantOp x;` anywhere and a ctor key would be DEAD.
//
// ⛔ AND NO MEMBER IS MAPPED.  All 147 `arith::ConstantOp` sites are one of two shapes
// that are both standing refusals:
//   * `arith::ConstantOp::create(builder, loc, type, attr)` -- a SINK that CREATES an
//     op, the `mlir::OpBuilder` / `RewriterBase` refusal.  There is no sink type in the
//     model; a stub would create nothing while the code proceeded as though it had.
//   * `dyn_cast<mlir::arith::ConstantOp>` / `dyn_cast_or_null<...>` -- the
//     collapsed-template-argument trap, now on its tenth instance: the explicit
//     template argument is the ONLY distinguishing argument, so every instantiation
//     would record as ONE key and any body would be a plausible lie.
// Both therefore stay LOUD, which is the intended outcome.
//
// ⛔ t25's PROHIBITION APPLIES UNCHANGED: no `operator==`, no `operator!=`, no identity
// test.  A C++ op handle compares `Operation *`; an `OpInst` is an op's printed
// CONTENT.
using t161 = mlir::arith::ConstantOp;

// t162 -- `mlir::arith::ConstantIndexOp`.  Arity 0.  Queue row g089 (`system type has
// no rule: `mlir::arith::ConstantIndexOp``, searched as
// `mlir::arith::ConstantIndexOp`).
//
// NO `fN`, measured the same way and more strongly:
//   grep -rnE "(mlir::)?arith::ConstantIndexOp[[:space:]]+[A-Za-z_]\w*[[:space:]]*(;|=|\()"
//     --include=*.cpp --include=*.h repos/dt_src  ->  ZERO hits.
//   multi-declarator form  ->  ZERO hits.
// All 344 `ConstantIndexOp` mentions in the corpus are
// `mlir::arith::ConstantIndexOp::create(builder, loc, n)` -- the op-creating SINK
// refusal above -- so no member and no constructor is keyed here either.
using t162 = mlir::arith::ConstantIndexOp;

// ============================================================================================
// REFUSED, WITH REASONS: the 9 trait-class type rows g081 g088 g096 g099 g102 g104 g107 g112
// g125.  No key is written for any of them.  The deciding measurement, run 2026-09-28:
//
// ⛔ READ THE CORRECTION AT THE END OF THIS BLOCK BEFORE ACTING ON IT.  The `OneTypedResult`
// third -- g081 / g096 / g112 -- IS OVERTURNED AND REOPENED: those rows are shipping FABRICATED
// identifiers in rc=0 output (407 occurrences, 53 distinct spellings, 8 emitted .rs files), not
// aborting, so the "dead key" conclusion below does not hold for them.  The other 6 rows
// (g088 g099 g102 g104 g107 g125) have NOT been re-measured this way and their refusal stands
// only as far as the checks below reach.
//
// (1) LOCATION CHECK over /home/agent/work/queue/samples/<id>.txt.  Every one of the 9 rows has
//     exactly ONE distinct location, and it is the trait's OWN declaration inside the prebuilt
//     LLVM include tree -- ZERO locations anywhere in repos/dt_src:
//        g081/g096/g112  OpDefinition.h:702:9    class Impl                 (35/24/17 locs)
//        g099/g102       OpDefinition.h:881:8    class SingleBlock          (23/22 locs)
//        g104            OpDefinition.h:535:8    MultiRegionTraitBase       (21 locs)
//        g107            OpDefinition.h:628:8    MultiResultTraitBase       (20 locs)
//        g088            SymbolInterfaces.h.inc:262:10    SymbolOpInterfaceTrait   (33 locs)
//        g125            FunctionInterfaces.h.inc:739:10  FunctionOpInterfaceTrait (16 locs)
//     So the type is only ever reached as a BASE-CLASS SPECIFIER of a tablegen'd Op.  No corpus
//     expression has one of these as its object type, therefore no corpus TU can read a member
//     of it.  That is the check the queue rows were waiting on and it comes back negative for
//     all nine.
//
// (2) SPELLING CHECK over repos/dt_src (`rg -c`, build dirs excluded): `OneTypedResult` 0,
//     `OpTrait::SingleBlock` 0, `SymbolOpInterfaceTrait` 0.  `MultiRegionTraitBase` 3,
//     `MultiResultTraitBase` 18, `FunctionOpInterfaceTrait` 8 and `OpTrait::` 16 hits exist, but
//     none is an object whose member is read -- they are trait mentions in op definitions / trait
//     lists, consistent with (1)'s single decl-site location.
//
// REASON PER ROW -- deliberately NOT one blanket reason:
//   g081 g096 g112 g099 g102 g104 g107  (7 rows, all `mlir::OpTrait::...`)
//     PURE TRAITS CLASS, exactly the `rules/densemap/src.cpp:33-44` precedent which refuses
//     `DenseMapInfo` as "IS A TRAITS CLASS AND IS DELIBERATELY NOT MODELLED".  Tag/CRTP bases
//     with no state; a Rust type for them would be an empty struct nothing can be done with, and
//     a too-short key there swallows placeholders (the densemap note records that hazard).
//
// ⛔⛔ CORRECTION, 2026-09-28 (LATER THAN EVERYTHING ABOVE): THE `OneTypedResult` THIRD OF THIS
// REFUSAL -- ROWS g081 / g096 / g112 -- IS WRONG, AND THOSE ROWS HAVE BEEN REOPENED.  The two
// measurements in (1) and (2) above STILL HOLD and were independently re-verified: every row has
// exactly one location (`mlir/IR/OpDefinition.h:702:9  class Impl`) and `rg -l OneTypedResult`
// over `repos/dt_src` is ZERO files.  What does NOT follow is the conclusion "a `tN` would be a
// DEAD key".  MEASURED CONTRADICTION: those rows did not abort, they FABRICATED, and the
// fabricated identifiers are sitting in rc=0 OUTPUT RIGHT NOW.  Anchored to the STANDALONE
// prefix (`(^|[^A-Za-z0-9_])Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_`, which is NOT the same
// text as the substring inside the much longer `Cpp2RustUnmapped_mlir_Op_<Op>__<traitlist>_`
// names of the separately-unkeyed `mlir::Op<...>` type -- counting them together measures two
// defects at once), over the emitted corpus `/home/agent/work/verify0928/out`:
//     8 emitted .rs files, 407 occurrences, 53 DISTINCT SPELLINGS.
//   top files: hcc/.../DataflowToCore/VectorChainLowering.cpp   267
//              dsc-based-utils/.../V3/SNTransferLowering.cpp      35
//              dcc/src/Transform/Sentient/LexicalOrdering.cpp     32
//              dsc-based-utils/PCFGToDataflowIR/PCFG2ToDataflowIR.cpp  21
//   top spellings: `..._mlir_Type__Impl_mlir_arith_ConstantOp_` 114, `..._vector_ExtractOp_` 21,
//              `..._arith_SelectOp_` 20, `..._sentient_ConstantOp_` 17, `..._arith_MulIOp_` 14.
// EVERY occurrence is in CAST-TARGET (type) POSITION -- the implicit derived-to-base upcast of an
// op to its trait base, e.g. `const_op as Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_...`.
// ⭐ SO "NOTHING READS A MEMBER" MAKES A KEY HERE TRIVIAL, NOT DEAD: the site needs a NAME for the
// cast target and NO MEMBER RULES AT ALL.  "Dead" was inferred from the member-read check alone
// and never checked against the emitted corpus, which is the mistake to avoid repeating.
// ⚠️ NOTE the outer `ResultType` varies -- `mlir::Type` (most), `mlir::VectorType`,
// `mlir::IndexType` -- and the earlier size estimate of "~19 keys / 3 outer args" UNDERCOUNTS:
// measured on the corpus above it is 53.
//
// WHAT A LANDING LOOKS LIKE, AND THE ONE TRAP THAT KILLS THE OBVIOUS VERSION.
//   * MONOMORPHIC, one `using tN = mlir::OpTrait::OneTypedResult<RT>::Impl<mlir::X::YOp>;` per
//     spelling.  SWALLOW-SAFETY: `GetTypeMapKey` (mapper.cpp:115) truncates at the first `<`, so
//     every one of these lands in the bucket `mlir::OpTrait::OneTypedResult`, which is EMPTY
//     today.  Fully concrete => placeholder arity 0 => `matchTemplate`'s capture
//     (`findNextLiteralSameDepth`, :173) never runs, so the `DenseMapInfo<T1>`/`__wrap_iter<T1 *>`
//     swallow cannot occur; and being pairwise distinguished by their own template arguments, a
//     literal match selects exactly one and `search()`'s longer-src tie-break (:430-437) is never
//     needed.  That matters, because a GENERIC `OneTypedResult<T1>::Impl<T2>` would be the SOLE
//     candidate in that bucket, where the tie-break CANNOT protect it -- this is precisely the
//     densemap failure, so DO NOT write the generic form.  Same discipline as t37-t39 / t166.
//   * TARGET MODEL: `fmt::OpInst::new(<dataflowir_gen::ops::mlir_X_YOp as MlirOp>::DEF)`, i.e.
//     the CONCRETE OP's OWN DEF -- exactly what t161/t162 already do (t162 maps
//     `arith::ConstantIndexOp` onto `mlir_arith_ConstantOp`'s DEF).  Nothing new is claimed: the
//     trait base of an op is that op.  `tgt_refcount.rs` needs an entry too or the loader aborts
//     (rules/iostream t1), written as a plain `fn`, not `unsafe fn` (the t37-t39 / t166
//     convention for a TYPE rule).
//   * ⛔ DO NOT GENERATE ALL 53 BLIND -- MEASURED, 2026-09-28: TWO of the 53 ConcreteOps HAVE NO
//     GENERATED DEF at all.  Checked with `rg "struct <op>\b"` against
//     `repos/dt_src/cpp2rust-port/dataflowir-gen/target/debug/build/dataflowir-gen-*/out/
//     dataflow_ods.rs`: `mlir_LLVM_UndefOp` -> 0 hits and `mlir_math_AbsIOp` -> 0 hits, while
//     mlir_arith_ConstantOp / sentient_ConstantOp / vector_ExtractOp / arith_SelectOp /
//     arith_MulIOp / vector_InsertOp / arith_CmpIOp / arith_AddIOp /
//     dataflow_GetLogicalMemoryViewOp / vectorchain_ShuffleOp / uniform_DefImmutableMappingOp are
//     all present.  A key naming an absent DEF records fine and passes the load smoke test, then
//     fails at rustc -- so CHECK EVERY OP'S DEF FIRST and leave the ones without one LOUD (they
//     are the `mlir::LLVM` / `mlir::math` dialects, which the .td set does not cover; that is a
//     dataflowir-gen coverage question, not a rules one).
//   * GATE: the ANCHORED standalone count above, BEFORE -> AFTER, on a witness that EMITS
//     (bucket A rc=0).  `dcc/src/Transform/Sentient/LexicalOrdering.cpp` (32) is a good size;
//     `hcc/.../VectorChainLowering.cpp` (267) is the big one.  ⚠️ `g172`'s own spelling
//     (`IndexType` / `uniform::DefImmutableMappingOp`) occurs ONCE, so g172 is a POOR gate row.
//     A recorded key that does not move that count is NOT REACHED and must be reported as such.
//   g088 `mlir::detail::SymbolOpInterfaceTrait<ktdf_arch::DeviceOp>`
//   g125 `mlir::detail::FunctionOpInterfaceTrait<func::FuncOp>`
//     HANDLED INDIVIDUALLY, because these are `mlir::detail::` INTERFACE traits: they carry
//     DEFAULT METHOD IMPLEMENTATIONS, not just tags, so the "pure traits class" argument above is
//     weaker for them.  They are still refused, on the narrower and stronger ground that the
//     member-read check in (1)/(2) is negative FOR THEM SPECIFICALLY: 0 corpus locations, 0
//     `SymbolOpInterfaceTrait` spellings, and all 8 `FunctionOpInterfaceTrait` spellings are in
//     op definitions.  Any default method these supply (`getName`/`setName`,
//     `getFunctionType`/`getArgAttrs`) is called in the corpus THROUGH THE OP
//     (`op.getName()`), so it would record against the Op or the OpInterface -- never against the
//     trait class type.  A `tN` for the trait would therefore be a DEAD key.
//
// ⚠️ CORRECTION to a check quoted at :2734 and in the t152/t157/t158/t159 notes: that comment
// says "no `tN` in this file names `mlir::OpState` or `mlir::Op<...>`".  The `mlir::OpState` half
// is NO LONGER TRUE at HEAD -- `using t25 = mlir::OpState;` is at :1295.  The `mlir::Op<...>`
// half still holds (its only occurrences are comments).  It does not change any verdict above,
// because t25 maps the OpState base, not these trait bases, and (1) shows nothing reads a trait
// member regardless.  Recorded so the next slot does not rely on the stale half.
// ============================================================================================

// ============================================================================================
// t163 / f130-f137 -- `mlir::OptionalParseResult` -> `Option<bool>`.  15 queue rows
// g2320..g2334 (one TU each), 247 placeholder lines.  Declaration:
// mlir/IR/OpDefinition.h:40 (verbatim, from the row's own location list:
// `.../include/mlir/IR/OpDefinition.h:40:7   class OptionalParseResult {`).
//
// SEARCHED AS: `mlir::OptionalParseResult` -- verbatim from g2320's
// `rule key: searched as: mlir::OptionalParseResult; from decl (NOT a key --
// canonicalised, defaulted args kept): mlir::OptionalParseResult`.  Searched form and
// from-decl form are IDENTICAL, so there is no typedef/default-arg divergence of the kind that
// killed `rules/fstream`'s t4 and `rules/chrono`'s t2.
//
// ⛔ SWALLOW-SAFETY: `GetTypeMapKey` truncates at the first `<`.  `mlir::OptionalParseResult`
// contains NO `<` and is not a template, so its key is the WHOLE spelling and the match is
// EXACT-MATCH-ONLY.  It cannot swallow any other type's placeholder -- unlike the
// `DenseMapInfo` hazard recorded at rules/densemap/src.cpp:33-44.  Swallow-safe.
//
// ⛔ IT IS **NOT** `ParseResult`-SHAPED AND IS DELIBERATELY NOT FOLDED INTO rules/support's
// t4.  OpDefinition.h:56 gives it a data member of its OWN:
//     private: std::optional<ParseResult> impl;
// so the honest model is `Option<bool>`, NOT `bool`.  Its own comment at OpDefinition.h:35-39
// says why the distinction is load-bearing: "We don't directly use Optional here, because it
// provides an implicit conversion to 'bool' which we want to avoid.  This class is used to
// implement tri-state 'parseOptional' functions that may have a failure mode when parsing that
// shouldn't be attributed to 'not present'."  THREE STATES: absent / present-success /
// present-failure.  Collapsing it to `bool` would merge two of them.
//
// ⭐ ALL FIVE MEMBERS BELOW ARE ITS OWN, NOT INHERITED -- it derives from nothing.  So the
// `llvm::FailureOr` dead-key lesson (six reader keys written on a DERIVED class, all SIX dead,
// because a key on a derived class cannot relocate an INHERITED member -- see
// rules/support/src.cpp:145-150) DOES NOT APPLY HERE.  Every one of `has_value`, `value`,
// `operator*` and the four constructors is declared in the class body at OpDefinition.h:42-54,
// so each keys against this type.
//
// ⚠️⚠️ THE POLARITY TRAP, AND WHY MY BODIES ARE IDENTITY RATHER THAN INVERTED.
// `ParseResult::operator bool()` returns `failed()`, NOT `succeeded()` -- the sibling row paid
// for that, and rules/support's f24 is `!a0` on purpose.  The inversion therefore lives
// ENTIRELY IN THAT ONE KEY.  `OptionalParseResult::value()`/`operator*()` return a
// **ParseResult**, not a bool: in C++ `if (*optRes)` is TWO calls, `operator*` then
// `ParseResult::operator bool`, so the emitted Rust is `f24(f132(x))` and the negation is
// supplied by support's f24.  Writing `!` HERE TOO would DOUBLE-INVERT and land back on the
// wrong branch -- the mirror image of the sibling's bug and just as silent.  The inner `bool` of
// my `Option<bool>` is in the SUCCESS polarity (`true` == success), exactly as rules/support's
// t1/t4 are, and the probe below pins all three states so neither error can hide.
//
// f136's `mlir::InFlightDiagnostic` ctor IS keyed, and only because the type is ALREADY
// MODELLED IN THIS MODULE as t70 -> `libcc2rs::InFlightDiagnostic` (src.cpp:1685).  Had it not
// been, this one ctor would have been left out with a scope note rather than a diagnostic type
// invented, which is `rules/error_code`'s accepted pattern.  No invention was needed.
//
// f133's `std::nullopt_t` is likewise already modelled -- rules/optional t4 -> `()`
// (rules/optional/src.cpp:45, tgt `fn t4() -> ()`), so the parameter has a real target type.
// ============================================================================================

namespace llvm {
// Declared LOCALLY for the reason this file's header gives for every other type here: the rule
// preprocessor compiles with a fixed flag set and cannot reach real LLVM/MLIR headers.  Both are
// MODELLED IN rules/support (t1 and t4, both `bool`); these declarations exist only to spell the
// parameter types of f134/f135 correctly.  No member of either is keyed here -- their members
// key against rules/support, and a declared-but-unmapped member records nothing.
class LogicalResult {
public:
  LogicalResult();
};
class ParseResult : public LogicalResult {
public:
  ParseResult(LogicalResult r);
};
} // namespace llvm

namespace mlir {
// mlir/IR/OpDefinition.h:40-57, member-for-member.  `impl` is omitted because a rule
// declaration needs no storage, but it is the reason the model is Option<bool>.
class OptionalParseResult {
public:
  OptionalParseResult();                                  // :42  = default
  OptionalParseResult(llvm::LogicalResult result);        // :43
  OptionalParseResult(llvm::ParseResult result);          // :44
  OptionalParseResult(const InFlightDiagnostic &);        // :45-46 -> OptionalParseResult(failure())
  OptionalParseResult(std::nullopt_t);                    // :47
  bool has_value() const;                                 // :50
  llvm::ParseResult value() const;                        // :53
  llvm::ParseResult operator*() const;                    // :54
};
} // namespace mlir

using t163 = mlir::OptionalParseResult;

// :50  `bool has_value() const { return impl.has_value(); }`
bool f130(const mlir::OptionalParseResult &r) { return r.has_value(); }

// :53  `ParseResult value() const { return *impl; }` -- BY VALUE, not by reference.
// ⚠️ THE DANGLING GATE: a member returning a REFERENCE into a BY-VALUE receiver is silently
// dangling under refcount (why `string_view::front()` was refused today).  Checked: both :53
// and :54 return `ParseResult` BY VALUE, which the model makes a `bool` -- a Copy scalar.
// Nothing borrows from the receiver, so there is no dangling shape here in either model, and
// tgt_refcount.rs and tgt_unsafe.rs are byte-identical for this whole block.
llvm::ParseResult f131(const mlir::OptionalParseResult &r) { return r.value(); }

// :54  `ParseResult operator*() const { return value(); }`
llvm::ParseResult f132(const mlir::OptionalParseResult &r) { return r.operator*(); }

// :47  `OptionalParseResult(std::nullopt_t) : impl(std::nullopt) {}` -- the ABSENT state.
mlir::OptionalParseResult f133(std::nullopt_t n) { return mlir::OptionalParseResult(n); }

// :43  `OptionalParseResult(LogicalResult result) : impl(result) {}`
mlir::OptionalParseResult f134(llvm::LogicalResult r) { return mlir::OptionalParseResult(r); }

// :44  `OptionalParseResult(ParseResult result) : impl(result) {}`
mlir::OptionalParseResult f135(llvm::ParseResult r) { return mlir::OptionalParseResult(r); }

// :45-46  `OptionalParseResult(const InFlightDiagnostic &) : OptionalParseResult(failure()) {}`
// The diagnostic is DISCARDED by C++ itself -- the parameter is unnamed -- and the result is
// unconditionally present-FAILURE.  t70 models the parameter, so nothing is invented.
mlir::OptionalParseResult f136(const mlir::InFlightDiagnostic &d) {
  return mlir::OptionalParseResult(d);
}

// ============================================================================================
// MEASUREMENT NOTE, 2026-09-28 -- t158 `mlir::affine::AffineForOp` AS A CONTAINER ELEMENT.
// NO KEY IS ADDED OR CHANGED BY THIS BLOCK.  It is appended AFTER the 9-row refusal comment
// block and AFTER t163 / f130-f136 (`mlir::OptionalParseResult`); NEITHER was reverted or
// edited.
//
// FIRST-ABORT-RANKING.md row #3 flagged a contradiction: the `mlir::affine::AffineForOp` type
// row was closed `done` (this file's `using t158 = mlir::affine::AffineForOp;`), yet 4 of the
// 48 real bucket-B TUs -- 8%, joint-third on the whole B frontier -- still carried
//     LLVM ERROR: unsupported unmapped type `mlir::affine::AffineForOp` has no model in
//     types_, while mapping `llvm::SmallVector<mlir::affine::AffineForOp>`
// (and the `std::map<const dsc2::BlockNode *, mlir::affine::AffineForOp>` variant).  The
// survey correctly REFUSED to reopen the row on that evidence and asked for a HEAD-level
// re-run first.  Done, and the answer is that the row was right:
//
//   binary  pin/cpp2rust (md5 33e811d189cf), rules  pin/ir.v16 with THIS module regenerated
//   from the WORKING TREE (ir.v16 predates t158-t163, which is the whole explanation):
//     dbo/src/Transforms/sdsc_bundle/LoopUnroll.cpp          B/0  ->  A rc=0  37,976 lines
//     dsc-based-utils/DSC2ToDataflowIR/V3/SNTransferLowering.cpp
//                                                            B/0  ->  A rc=0  65,706 lines
//     .../SNComputeLowering.cpp      B -> B, abort text CHANGED, no longer mentions AffineForOp:
//         `const dsc2::LoopNode *const` ... while mapping
//         `std::pair<const dsc2::LoopNode *const, std::unordered_map<PrimaryDimTypes, int>>`
//     .../SNControlFlowLowering.cpp  B -> B, abort text CHANGED, no longer mentions AffineForOp:
//         structured binding / DecompositionDecl with 2 bindings [dim, kind]
//
// So `AffineForOp` fronts 0 of 48 B TUs at HEAD, down from 4.  The rows stay `done`.
//
// ⛔ THE STANDING LESSON, because this cost a survey slot a false contradiction: an abort of
// the form "element E has no model, while mapping CONTAINER<E>" has TWO causes that read
// identically, and only one of them is a bug in this file.
//   (1) A SWALLOW BUG in a container key -- `GetTypeMapKey` (mapper.cpp:115) strips at `<`,
//       so a short key and a longer instantiation share one bucket, and `matchTemplate`'s
//       placeholder capture runs to the next SAME-DEPTH literal
//       (`findNextLiteralSameDepth`, mapper.cpp:173), so a placeholder can capture
//       `A, B` commas and all and the mapper then tries to map that comma-joined string as a
//       type.  That is `9f7657f5` / `KtdpOps.cpp` / `DenseMapInfo<StringRef, void>`.
//       ⭐ ITS SIGNATURE: the reported "type" is comma-joined or otherwise malformed, or the
//       report names the CONTAINER rather than the element.
//   (2) A STALE RULE-IR TREE.  The element key exists in `src.cpp` but not in the `ir.vN` the
//       run used.  ⭐ ITS SIGNATURE: the reported element is a clean, well-formed, singular
//       spelling that `grep` finds in this file.  That is this row.
// `mlir::affine::AffineForOp` is arity 0 -- it has no `<`, so it CANNOT be swallowed and
// cannot swallow -- which is by itself enough to rule out (1) before running anything.
// Check the arity first; it is free.
//
// READBACK, whole-spelling, against the abort's own text: `ir_src.json` records
// `"t158": "mlir::affine::AffineForOp"`, byte-identical to the abort's
// `mlir::affine::AffineForOp`.  The key is LIVE, not recorded at a divergent spelling.
// Swallow-safety: arity 0, so `GetTypeMapKey` buckets it at the whole spelling
// `mlir::affine::AffineForOp`; nothing else in this module or any other shares that bucket
// (it is the only key whose stripped form equals it), so `search()`'s longer-src tie-break at
// mapper.cpp:430-437 is never consulted, and there is no placeholder to run past a comma.
// It cannot match any other arity because it has no placeholder at all.
//
// REGRESSION LOCK: `/home/agent/work/probes/affineforop_container.cpp` exercises
// `AffineForOp` in BOTH corpus container positions (`llvm::SmallVector<...>` and
// `std::map<const BlockNode *, ...>`) with no `==`, no `!=`, no identity test and no member
// call on any op handle, per t25.  WRITTEN, NOT RUN.
// ============================================================================================

// ============================================================================================
// t164 / t165 / f137 / f138 -- `mlir::RankedTensorType` and `mlir::FunctionType`, the two
// largest remaining open mlir clusters that are NOT sinks: 15 rows (g2375 ...) and 11 rows
// (g1739 ...).  BOTH are mlir builtin TYPE VALUES, so they take the WIDENING that t41
// (`ShapedType`), t42 (`TensorType`), t60 (`IndexType`) and t73 (`MemRefType`) already took:
//   -> `dataflowir_gen::ir::Ty` (ir.rs:37), init = the empty-spelling `Ty::Opaque("")` NULL
//      SENTINEL, and NO ACCESSOR MAPPED.
//
// ⭐ WHY THIS IS THE SAME BARGAIN AND NOT A NEW CLAIM.  `RankedTensorType` was ALREADY
// widened once, one level up the interface: t42 maps `mlir::TensorType`, the type INTERFACE
// over `RankedTensorType`/`UnrankedTensorType`, and t41 maps `ShapedType` above that.  So the
// model that a ranked tensor is an `ir::Ty` is ALREADY committed to by this module; t164 only
// lets the CONCRETE spelling reach it.  Refusing t164 while keeping t42 would be incoherent.
//
// ⛔ THE COST, RESTATED SO IT IS NOT LOST: `ir::Ty` has `Ty::Vector(Vec<i64>, Box<Ty>)` and
// `Ty::MemRef(Vec<i64>, Box<Ty>)` but NO TENSOR VARIANT and NO FUNCTION VARIANT, so both land
// in `Ty::Opaque(spelling)` (ir.rs:47) and their structure is recoverable only by reparsing
// the spelling.  That is ACCEPTED here for the same reason t41/t42 accepted it, and the reason
// is checkable rather than asserted: NO accessor is mapped, so every STRUCTURE QUERY still
// aborts LOUDLY in the mapper instead of reading structure that is not there.  Specifically
// NOT claimed, for either type: `getShape`, `getElementType`, `getRank`, `hasRank`,
// `hasStaticShape`, `cloneWith`, `getNumInputs`, `getNumResults`, `getInput`, `getInputs`,
// `getResult`, `getResults`, `clone`, `getContext`, and NOT the `::get(...)` FACTORIES
// (`RankedTensorType::get`, `FunctionType::get`) -- which is what the corpus actually calls
// most often (dbo/src/InitBin.cpp:152, dr5/.../CreateAdd.cpp:42, ...), so those sites keep
// aborting loudly.  What the two keys buy is the SIGNATURE and the CONTAINER: a parameter
// (`dbo/src/HostCompute.h:42  mlir::RankedTensorType flits`), a local
// (`dbo/src/Pipeline/CorrectAtRuntime.cpp:281  const mlir::FunctionType type =`) and a
// `cast<>`/`dyn_cast<>` template argument become EXPRESSIBLE.  The t40 `mlir::Pass` /
// t43 `mlir::OpOperand` / t84 / t73 precedent: map the type, map no member.
//
// ⛔ NO `==`/`!=`, no identity test, no member -- the t25 prohibition.
//
// ⚠️ DESTRUCTOR GATE (the `mlir::OwningOpRef` / `mlir::InFlightDiagnostic` check that forbids
// an opaque model): both are trivially destructible uniquer-pointer handles in this toolchain
// -- `mlir/IR/BuiltinTypes.h` declares neither a user destructor nor any owned storage -- so
// neither is the case where a destructor with observable effect forbids the handle model.
//
// ⭐ ARITY 0, CLEAN SINGULAR SPELLINGS: neither row can be a collapsed-template-argument
// swallow, and neither spelling is served by an existing bucket (checked: `ir_src.json` at
// pin/ir.v17 contains `mlir::RankedTensorType` ONLY inside t32's
// `mlir::detail::TypedValue<mlir::RankedTensorType>`, and contains no `FunctionType` at all --
// so these are genuinely-missing models, NOT the phantom class).
namespace mlir {
class FunctionType {
public:
  FunctionType();
};
} // namespace mlir

using t164 = mlir::RankedTensorType;
using t165 = mlir::FunctionType;

// t166: the FOURTH concrete instantiation of the range CRTP base, at
// `DerivedT = mlir::ValueRange` -- queue row g090 (29 TUs).  t37/t38/t39 above
// already key this base at OperandRange / ResultRange / RegionRange; the
// ValueRange instantiation was simply missing, and it is the one the diagnostics
// print verbatim.  SPELLING READ OFF THE ABORT, not copied from a sibling:
//   system type has no rule: `llvm::detail::indexed_accessor_range_base<
//     mlir::ValueRange, llvm::PointerUnion<const mlir::Value *, mlir::OpOperand *,
//     mlir::detail::OpResultImpl *>, mlir::Value, mlir::Value, mlir::Value>`
// (ValueRange.h:390 is the base-specifier; STLExtras.h:1214 is the decl the
// converter canonicalises from).  Note the PointerT is the THREE-arm
// `PointerUnion`, one arm SHORTER than mlir::TypeRange's four-arm union -- see the
// g111 note below.
//
// MODEL: exactly t16's (`mlir::ValueRange` -> Vec<ir::Value>, src.cpp:1372),
// reached through the base-class spelling.  Nothing new is claimed and no new
// representation is invented, which is the same discipline t37-t39 were written
// under.  The return is BY VALUE, so the refcount model cannot dangle (contrast
// the refused reference-returning accessors `string_view::front()` /
// `SMLoc::getPointer()`).
//
// ⛔ STILL NO GENERIC RULE.  The t37-t39 prohibition applies unchanged: a generic
// `indexed_accessor_range_base<T1, T2, T3, T4, T5>` would have to pick ONE element
// representation for four different element types, and it would force the converter
// to map `mlir::OpOperand *` / `mlir::detail::OpResultImpl *` / the `PointerUnion`,
// turning a countable mangled name into a hard mapper.cpp:722 abort (the t26/t29
// generic-MLIR-type regression).
//
// SWALLOW-SAFETY.  `GetTypeMapKey` (mapper.cpp:115) truncates at the first `<`, so
// this key lands in the bucket `llvm::detail::indexed_accessor_range_base`, shared
// with t37, t38 and t39 -- and ONLY those.  All four spellings are FULLY CONCRETE:
// there is no `T<digits>` placeholder anywhere in any of them, so `matchTemplate`'s
// placeholder capture (`findNextLiteralSameDepth`, :173) -- the mechanism that made
// `DenseMapInfo<T1>` swallow `llvm::StringRef, void` and `__wrap_iter<T1 *>` swallow
// `DtInfo *const` -- NEVER RUNS on this bucket.  Placeholder arity 0 rules a swallow
// out by construction, which is why the nested comma-bearing `PointerUnion<...>`
// argument is harmless here: it is matched literally, not captured.  The four
// candidates are also pairwise distinguished by their FIRST template argument
// (OperandRange / ResultRange / RegionRange / ValueRange), so a literal match selects
// exactly one and `search()`'s longer-src tie-break (mapper.cpp:430-437) is never
// needed -- which matters, because that tie-break cannot protect when a short key is
// the sole candidate (how the DenseMapInfo case bit).  No defaulted template argument
// is spelled either (`indexed_accessor_range_base` has five parameters and all five
// are written), so `SuppressDefaultTemplateArgs` cannot turn this into a dead
// duplicate the way `DenseMapInfo<T1, void>` was.
//
// ⚠️ ONE KEY CANNOT ALSO COVER QUEUE ROW g111 (`mlir::TypeRange`, 16 TUs).  Measured
// from the same abort logs, that row's full spelling is
//   llvm::detail::indexed_accessor_range_base<mlir::TypeRange,
//     llvm::PointerUnion<const mlir::Value *, const mlir::Type *, mlir::OpOperand *,
//     mlir::detail::OpResultImpl *>, mlir::Type, mlir::Type, mlir::Type>
// -- a DIFFERENT DerivedT, a FOUR-arm PointerUnion, and an element type of
// `mlir::Type` rather than `mlir::Value`.  Its model would be a Vec of `ir::Ty`, not
// `ir::Value`, so sharing a key would give one of the two rows the wrong element type.
// It is left for g111, which additionally has to decide `mlir::TypeRange` itself --
// that type is NOT mapped anywhere in this module today.
using t166 = llvm::detail::indexed_accessor_range_base<
    mlir::ValueRange,
    llvm::PointerUnion<const mlir::Value *, mlir::OpOperand *,
                       mlir::detail::OpResultImpl *>,
    mlir::Value, mlir::Value, mlir::Value>;

// f137 / f138 -- the default constructors for t164 / t165, one per type key.  The f40-f45
// reason verbatim: a `using tN =` maps the TYPE ONLY, the converter looks `void <T>::<T>()` up
// as an ORDINARY EXPR RULE, and on a miss emits `<mangled type>::new()`, which does not exist
// -- rc=0 and then `error[E0433]`.  Each body is the SAME null-handle sentinel as its type's
// `init`, NOT a valid value.
mlir::RankedTensorType f137() { return mlir::RankedTensorType(); }
mlir::FunctionType f138() { return mlir::FunctionType(); }

// ---- f139/f140/f141: three more ArrayRef constructor overloads --------------
// WHY THESE THREE AND NOT THE OTHER TWO.  `llvm::ArrayRef<llvm::StringRef>` is the
// single largest fabricated-`::new_N` receiver in the corpus -- 6,332 sites across
// 207 emitted files (FABRICATED-NEWN.md §2) -- and it was reported as a DEAD KEY,
// i.e. f55 present but never reached.  That verdict is OVERTURNED by measurement:
// the `-verbose` readback in /home/agent/work/mlirslot/aref.vlog shows
//     search expr void llvm::ArrayRef<llvm::StringRef>::ArrayRef(std::initializer_list<llvm::StringRef>), result:
//     Matching: void llvm::ArrayRef<T1>::ArrayRef(std::initializer_list<T1>)
// (same for `ArrayRef<int>` and `ArrayRef<mlir::Type>`), so f55 is LIVE.  The gap is
// that FIVE OTHER OVERLOADS come back `result: None`.  Three of them are safe to
// model against `Vec<T1>` and are written here; the other two are refused at the
// class declaration above, with reasons.
//
// The `ArrayRef<T1>` -> `Vec<T1>` representation (t19) OWNS its buffer where C++
// borrows, which is this module's settled position for the whole range family
// (src.cpp:245-249).  Each body below is therefore a COPY, and each is the only
// thing `Vec` can mean: an empty ArrayRef is an empty Vec, a one-element ArrayRef
// is a one-element Vec, and an ArrayRef over a `std::vector` is that vector's
// elements.  None of the three can observe the aliasing it loses, because
// `llvm::ArrayRef` declares no mutating member at all (contrast
// MutableArrayRef/MutableOperandRange, REFUSED at src.cpp:252-271 precisely
// because they are write-through).

// f139 -- `ArrayRef()` (ArrayRef.h:53), the default constructor.  Readback:
//   search expr void llvm::ArrayRef<llvm::StringRef>::ArrayRef(), result:  None
template <typename T1> llvm::ArrayRef<T1> f139() {
  return llvm::ArrayRef<T1>();
}

// f140 -- `ArrayRef(const T &OneElt)` (ArrayRef.h:64).  Readback:
//   search expr void llvm::ArrayRef<llvm::StringRef>::ArrayRef(const llvm::StringRef &), result:  None
// Spelled `const T1 &` because that is the spelling the readback ASKED FOR -- the
// f18/f19 lesson in reverse (rules/support f19 is the same shape: a `const T1 &`
// src parameter against an `a0: &T1` target with a `T1: Clone` bound).
template <typename T1> llvm::ArrayRef<T1> f140(const T1 &a0) {
  return llvm::ArrayRef<T1>(a0);
}

// f141 -- `ArrayRef(const std::vector<T, A> &Vec)` (ArrayRef.h:100).  Readback:
//   search expr void llvm::ArrayRef<llvm::StringRef>::ArrayRef(const std::vector<llvm::StringRef> &), result:  None
// The allocator argument is default-suppressed in the searched spelling, so the
// key is written WITHOUT it; `std::vector<T1>` -> `Vec<T1>` (rules/vector t1), so
// the body is a copy of the whole vector.
template <typename T1> llvm::ArrayRef<T1> f141(const std::vector<T1> &a0) {
  return llvm::ArrayRef<T1>(a0);
}

// ---- f142: the `mlir::RegionRange` constructor -----------------------------
// WHY THIS ROW.  `mlir_RegionRange` is the largest fabricated-`::new_N` receiver
// in the corpus (FABRICATED-NEWN.md §2): the TYPE key t17 is present, the
// CONSTRUCTOR was absent, so every `RegionRange` construction lowered to a call
// to `mlir_RegionRange::new_<N>` -- a function defined NOWHERE -- while the run
// still exited rc=0 with no placeholder token.  Exactly one distinct `new_N`
// suffix appears at this receiver, which RANKS the row but does not size it
// (rules/pair saw 4 suffixes for one overload, rules/twine 2 for three).
//
// PROVEN ABSENT by a `-verbose` readback (/home/agent/work/mlirslot2/probe-regionrange.vlog):
//   search expr void mlir::RegionRange::RegionRange(llvm::MutableArrayRef<mlir::Region>), result: None
//   search expr void mlir::RegionRange::RegionRange(llvm::ArrayRef<mlir::Region *>), result: None
// The first spelling is the one written here.  The second is the OTHER Region.h
// overload and is deliberately NOT written: `llvm::ArrayRef<mlir::Region *>` has
// no model for its element (`mlir::Region *` is not keyed; t36 keys
// `mlir::Operation *` only), so a key for it would take an unmodelled argument.
//
// CONCRETE, NOT GENERIC: `RegionRange` is not a template, so the parameter type
// resolves at the single instantiation `llvm::MutableArrayRef<mlir::Region>`.
// t46 already maps `llvm::MutableArrayRef<T1>` -> `Vec<T1>` and t3 maps
// `mlir::Region` -> `fmt::Region`, so nothing new is claimed and the target
// mentions no `Tn` (no target-only-generic risk).
//
// ⚠️ ALIASING, stated rather than assumed.  `RegionRange` is a NON-OWNING VIEW
// and `Vec` owns its buffer, so this body COPIES.  That is the settled position
// for this whole range family (src.cpp:245-249, t14-t17), and it is admissible
// HERE for the same reason it was admissible for `llvm::ArrayRef`: `RegionRange`
// exposes NO mutating member, so the only thing a copy can lose is aliasing that
// no mapped operation can observe.  Contrast `MutableOperandRange`, REFUSED at
// src.cpp:252-271 precisely because it IS write-through -- and note that the
// PARAMETER here is a `MutableArrayRef`, which is write-through; what saves this
// row is that the CONSTRUCTOR only reads it, and `RegionRange` itself cannot
// write back through it.
mlir::RegionRange f142(llvm::MutableArrayRef<mlir::Region> a0) {
  return mlir::RegionRange(a0);
}

// ---- f143/f144/f145: three of the four `mlir::ValueRange` constructor -------
// spellings.  MEASURED, not predicted: /home/agent/work/VALUERANGE-SPELLINGS.md,
// from a `-verbose` leg that EXITED ON ITS OWN at 27,901 asks whose emission is
// 31,212 lines -- the same line count the plain run gives, which is the proof it
// covered the whole TU, so its `result: None` is admissible.  (The slot-scale
// attempt died at 5,782 asks, 4.8x short, while these sites sit in the last 4% of
// the file; that log's silence was worthless.)  Witness TU
// `dcc/src/Transform/Sentient/Deuniform.cpp`; pin/cpp2rust md5
// e2d09f4562c470f813b0fa142d373bfb.  The four asks, all `result: None`:
//   2  void mlir::ValueRange::ValueRange(const mlir::ValueRange &)   <- NOT written, see below
//   2  void mlir::ValueRange::ValueRange(llvm::ArrayRef<mlir::Value>)      -> f144
//   5  void mlir::ValueRange::ValueRange(mlir::OperandRange)               -> f143
//   6  void mlir::ValueRange::ValueRange(std::vector<mlir::Value> &)       -> f145
// ⭐ THE LONG-PREDICTED KEY (`ArrayRef<mlir::Value>`) WAS RIGHT AND COVERS 2 OF 15
// ASKS.  It is not the dominant spelling.  Same lesson as `optional::value_or`
// (one apparent key, three real) and `pair` f20: a deduced/converting parameter is
// part of the key, so "the missing overload" is usually several.
//
// ALL THREE ARGUMENT TYPES ARE ALREADY MODELLED, so none of these is a
// missing-model refusal: `mlir::OperandRange` is t14 (src.cpp:1351),
// `llvm::ArrayRef<T>` is t19, `std::vector<mlir::Value>` is rules/vector t1.  All
// three, and the result t16, are the SAME Rust type `Vec<ir::Value>`, so every
// body below is an identity or a copy and nothing new is claimed.
//
// ⚠️ ALIASING, stated not assumed -- the f142 licence verbatim, re-derived on
// ValueRange.h:383-430: the entire public surface past the constructors is
// `getTypes()`/`getType()`, BOTH `const`, over a read-only
// `indexed_accessor_range_base`.  ValueRange declares NO MUTATING MEMBER, so the
// owning-`Vec` model (t16) loses only aliasing that no mapped operation can
// observe.  Contrast `mlir::MutableOperandRange`, REFUSED at src.cpp:252-271
// precisely because it IS write-through.
//
// ⛔ DO NOT CONFUSE THIS WITH THE SETTLED `mlir::OperandRange` REFUSAL.  That one
// is about OperandRange's OWN constructor (it declares none --
// `using RangeBaseT::RangeBaseT;` -- its live fabricated spelling is a hybrid, and
// its `iterator` parameter has no model).  Here OperandRange is only an ARGUMENT
// type and t14 maps it.  Different row, not blocked.
//
// ⛔ THE COPY CONSTRUCTOR (`const mlir::ValueRange &`, 2 asks) IS DELIBERATELY NOT
// WRITTEN.  A previous slot wrote exactly it as "f143", measured it, and found the
// witness TU's fabricated `mlir_ValueRange::new_` count UNCHANGED at 7 -> 7; it
// then correctly DELETED the key.  A recorded readback is not reachedness (the
// `std::replace` precedent), and an unreached key is indistinguishable from a
// missing one while making the class look handled -- which is how rules/support
// came to carry six dead `llvm::FailureOr` keys.  So it stays out until someone
// can show a DISAPPEARANCE.  (Unrelated to the OTHER "f143" in this file, at
// src.cpp:1631: that paragraph refuses a `mlir::OperationState` constructor and
// only names the index that was next free when it was written.)

// f143 -- `ValueRange(OperandRange values)` (ValueRange.h:414), BY VALUE.  Both
// sides are `Vec<ir::Value>` (t14 and t16), so the body is the identity, exactly
// like f142.  5 of the 15 asks.
mlir::ValueRange f143(mlir::OperandRange a0) {
  return mlir::ValueRange(a0);
}

// f144 -- `ValueRange(ArrayRef<Value> values = {})` (ValueRange.h:413), BY VALUE.
// ⛔ THE DEFAULT ARGUMENT IS NOT PART OF THE RECORDED SIGNATURE -- the recorder
// writes it out at the call site, so a nullary variant would record as THIS SAME
// key and a second entry would be a silent duplicate (the `substr` precedent, and
// f142's own reasoning).  Written CONCRETE, not as `llvm::ArrayRef<T1>`: the ask
// spells the instantiation, and t19 at `mlir::Value` is the same `Vec<ir::Value>`.
mlir::ValueRange f144(llvm::ArrayRef<mlir::Value> a0) {
  return mlir::ValueRange(a0);
}

// f145 -- the BIGGEST spelling (6 of 15 asks) and the only one whose argument
// needed its own licence: `std::vector<mlir::Value> &`, a NON-CONST REFERENCE.
//
// ⭐ IT IS NOT A WRITE-THROUGH PARAMETER, AND THE HEADER IS WHY.  ValueRange
// declares no `std::vector` constructor at all.  This spelling is the FORWARDING
// TEMPLATE at ValueRange.h:397-401:
//     template <typename Arg, typename = enable_if_t<
//         is_constructible<ArrayRef<Value>, Arg>::value && !is_convertible<Arg, Value>::value>>
//     ValueRange(Arg &&arg) : ValueRange(ArrayRef<Value>(std::forward<Arg>(arg))) {}
// `Arg &&` deduces to `std::vector<Value> &` for a non-const LVALUE vector, which
// is the ONLY reason the recorded key lacks a `const`.  The body forwards into
// `ArrayRef<Value>(...)`, which reads `data()`/`size()` and nothing else, and the
// constructed ValueRange then exposes only the two `const` members above.  So NO
// WRITE CAN REACH THE CALLER'S VECTOR in C++ either, and there is no observer to
// name: the copy differs from the original only in aliasing, the same loss f142
// and f139-f141 already take.
// ⚠️ THIS IS WHY IT IS NOT THE `string_view::front()` / `SMLoc::getPointer()`
// shape.  Those were refused because a by-reference C++ result let the CALLER
// write (or alias) through a by-value Rust receiver.  Here the reference is an
// INPUT that is only read, and the deduced `&` is an artefact of forwarding.
mlir::ValueRange f145(std::vector<mlir::Value> &a0) {
  return mlir::ValueRange(a0);
}

// ---- f146: `mlir::IntegerAttr::getValue()`, queue row g-largest-in-diagnosis ---
//
// THE MEASUREMENT.  `-verbose` on
// `dataflow-scheduler/external/ktir-mlir-frontend/lib/Ktdp/KtdpDialect.cpp`
// (A rc=0, 3,948 emitted lines; the verbose leg EXITED 0 at 102,598 log lines, so
// its silence is real silence and not a truncation artefact) records
//
//     search expr llvm::APInt mlir::IntegerAttr::getValue() const, result: None
//
// SIXTEEN times -- the largest single ask count in that TU's diagnosis.
//
// ⛔ WHY IT WAS NOT MERELY "MISSING": IT WAS SILENT.  An unmapped MEMBER does not
// abort.  The converter emits the call TEXTUALLY, at rc=0, with NO placeholder
// token to grep for -- `KtdpDialect.cpp:3117` came out as
// `unsafe { self.getValue() }` with no `fn getValue` anywhere in the file, i.e. a
// plain E0599 at the far end of the pipeline and nothing in the translate log.
// That is the whole reason this row is worth a key rather than a refusal.
//
// WHY THE RETURN TYPE DECIDES THE WIDTH, AND WHY THAT IS THE POINT.
// `getSExtValue()` (rules/apint f8) sign-extends FROM BitWidth.  The composite
// expression `step.getValue().getSExtValue()` therefore needs BOTH halves to be
// width-aware: a width-less model of this accessor turns a 32-bit -1 into
// +4294967295, and the named observer for exactly that is a LOOP BOUND --
// `dcc/src/Dialect/Sentient/SentientOps.cpp:1074,1080,1081`.  This key supplies
// the width; rules/apint t2 carries it.
//
// THE MODEL.  t10 WIDENS `mlir::IntegerAttr` to the whole `ir::Attr` enum (there
// is no distinct Rust type for the subclass), so the target body must recover the
// integral case from the enum.  `Attr::Int(i64, Ty)` (ir.rs:470) carries the type
// suffix as DATA -- "the type suffix is part of the value, not decoration" -- and
// `Ty::Int(u32)` (ir.rs:39) is the width, so the APInt is reconstructible exactly.
//   * `Attr::Int(v, Ty::Int(w))`  -> width `w`.
//   * `Attr::Int(v, Ty::Index)`   -> width 64.  MLIR's IndexType has
//     `kInternalStorageBitWidth = 64`, and `IntegerAttr::get(IndexType, v)`
//     stores a 64-bit APInt; this is not a guess about the model, it is what
//     BuiltinAttributes.cpp's IntegerAttr storage does for index.
//   * `Attr::Bool(b)`             -> width 1.  ⭐ NOT a widening error and NOT
//     out of scope: `mlir::BoolAttr` IS an `IntegerAttr` of `i1` in MLIR
//     (BuiltinAttributes.h -- BoolAttr is a distinct C++ class but the STORAGE and
//     `getValue()` are IntegerAttr's i1 arm).  Dropping this arm would have made
//     every boolean attribute take the panic path.
//   * every other arm PANICS WITH A NAMED MESSAGE.  In C++ the static type
//     `IntegerAttr` already guaranteed integrality, so these states are
//     unreachable in any program that type-checked -- they exist only because t10
//     widened.  ⛔ A panic here is the LOUD failure the brief demands, not a
//     `todo!()`: it names the arm it saw, so a real occurrence is diagnosable
//     rather than silently answering 0.
//
// ⚠️ NARROWING THE MODULE'S OWN ABSENCE NOTES.  Three comments in this file say a
// widening is "sound only because NO ACCESSOR IS MAPPED" and list `getValue()`
// among the absent ones (:696, :759, :988, :1909).  Those are about
// `IntegerSetAttr`, `TypeAttr`, `DenseElementsAttr`/`SparseElementsAttr`,
// `llvm::cl::opt` and `FlatSymbolRefAttr` -- DIFFERENT receivers, each with its
// own `getValue()`.  This key is on `mlir::IntegerAttr` ONLY; a key is recorded
// per fully-qualified signature, so none of those receivers is affected and none
// of those notes becomes false.
//
// THE RECEIVER IS `const` AND ARRIVES BY VALUE, exactly as for f11
// (`bool mlir::Attribute::operator!() const`) whose target likewise takes a0.
llvm::APInt f146(const mlir::IntegerAttr &a) { return a.getValue(); }

// ============================================================================================
// t167-t216 -- `mlir::OpTrait::OneTypedResult<ResultType>::Impl<ConcreteOp>`, FIFTY concrete
// instantiations.  ROWS g081 / g096 / g112, REOPENED: the refusal block at :3546 concluded
// "dead key" from the member-read check alone and that conclusion is OVERTURNED by the emitted
// corpus -- read the CORRECTION at :3613 before touching any of this.  These rows were not
// aborting, they were FABRICATING, and the fabricated identifiers were in rc=0 output.
//
// MEASUREMENT, re-run 2026-09-28 over `/home/agent/work/verify0928/out`, anchored to the
// STANDALONE prefix `(^|[^A-Za-z0-9_])Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_` (NOT the
// same text as the substring inside the much longer `Cpp2RustUnmapped_mlir_Op_<Op>__<traits>_`
// names of the separately-unkeyed `mlir::Op<...>` -- counting them together measures two
// defects at once):
//     465 occurrences, 21 files, 75 DISTINCT SPELLINGS.
// ⚠️ THE EARLIER FIGURE OF "407 / 8 files / 53 spellings" AT :3624 IS NOW STALE and undercounts
// by 1.4x on spellings: the emitted corpus has grown since it was taken.  A fourth outer
// `ResultType` also appeared that the 53-count never saw (`mlir::MemRefType`), so the outer arg
// set is `mlir::Type` / `mlir::VectorType` / `mlir::IndexType` / `mlir::MemRefType`, not three.
//
// EVERY occurrence is in CAST-TARGET (type) POSITION -- the implicit derived-to-base upcast of
// an op to its trait base.  NO MEMBER IS EVER READ, which is exactly what makes these keys
// cheap: each site needs a NAME for the cast target and NO MEMBER RULES AT ALL.
//
// ⛔ MONOMORPHIC, NEVER GENERIC, and the reason is a measured failure not a preference.
// `GetTypeMapKey` (mapper.cpp:115) truncates a type key at the first `<`, so every one of these
// lands in the bucket `mlir::OpTrait::OneTypedResult`, which was EMPTY before this block.  A
// generic `OneTypedResult<T1>::Impl<T2>` would therefore be the SOLE candidate in that bucket,
// where `search()`'s longer-src tie-break (mapper.cpp:430-437) CANNOT protect it -- that is the
// `DenseMapInfo` densemap failure verbatim (a short key ate `llvm::StringRef, void`).  Written
// fully concrete, placeholder arity is 0, so `matchTemplate`'s capture
// (`findNextLiteralSameDepth`, :173) never runs and the swallow cannot occur at all.  t166 (a
// nested comma-bearing `PointerUnion<...>`) and t37-t39 are the proven-safe precedent.
//
// TARGET MODEL: the CONCRETE OP'S OWN DEF, `fmt::OpInst::new(<ops::mlir_<d>_<Op> as
// MlirOp>::DEF)` -- exactly the t161/t162 precedent (t162 maps `arith::ConstantIndexOp` onto
// `mlir_arith_ConstantOp`'s DEF).  Nothing new is claimed: the trait base of an op IS that op.
//
// ⛔⛔ 25 OF THE 75 SPELLINGS ARE DELIBERATELY NOT KEYED, AND THIS IS THE FINDING THAT WOULD
// HAVE BROKEN A BLIND SWEEP.  A key naming an absent DEF records fine, passes the load smoke
// test, and THEN FAILS AT rustc.  Checked per op with `grep -cE "struct <op>\b"` against
//   repos/dt_src/cpp2rust-port/dataflowir-gen/target/debug/build/
//     dataflowir-gen-654e676bccb4a05f/out/dataflow_ods.rs
// -- THREE WHOLE DIALECTS have no generated DEF for any of their ops:
//     mlir::LLVM    15 spellings, 35 occurrences  (UndefOp ConstantOp ZExtOp TruncOp ShlOp
//                   LShrOp OrOp AndOp AddOp ICmpOp FCmpOp BitcastOp PtrToIntOp IntToPtrOp
//                   InsertValueOp)                        -> 0 hits each
//     mlir::math     6 spellings,  6 occurrences  (AbsIOp AbsFOp ExpOp TanhOp LogOp SqrtOp)
//     mlir::memref   4 spellings,  4 occurrences  (LoadOp AllocOp GetGlobalOp
//                   ExtractAlignedPointerAsIndexOp)
// ⭐⭐ RE-TESTED 2026-09-28 (the `mlir::ktdf` falsification prompted a sweep of EVERY def-absence
// refusal in this file, on the principle that nothing here re-tests its own refusals).  THIS ONE
// STILL STANDS.  All 25 spellings re-run through the refusal's OWN test -- `grep -cE "struct
// mlir_<dialect>_<Op>\b"` against the CURRENT dataflow_ods.rs -- return **0, every one**.  And the
// SECOND route was checked too, which the original refusal did not do: `grep -rnE
// "mlir_(LLVM|math|memref)_" dataflowir-gen/src/` (the HAND-WRITTEN side, where `fmt.rs` / `ir.rs`
// live) is ALSO 0 hits, and `grep -oE "mlir_(LLVM|math|memref)_[A-Za-z0-9_]+"` over the whole
// generated file returns NOTHING AT ALL -- the three dialect PREFIXES do not occur.  So unlike
// `mlir::ktdf`, which the `ods_more` / `TD_OPS_MORE` table silently closed, these three dialects
// were not carried in by the table's growth.  ⛔ STAYS LOUD; do not reopen on a spot check.
//
// ⚠️ An earlier note flagged only TWO absent DEFs (`mlir_LLVM_UndefOp`, `mlir_math_AbsIOp`)
// because it SPOT-CHECKED eleven ops; the real shape is whole-dialect, 25 of 75.  That is a
// dataflowir-gen `.td` COVERAGE GAP, not a rules gap, and those 45 occurrences must stay LOUD
// so the coverage question keeps its evidence.  ⭐ VERIFY THE DEF EXISTS BEFORE ADDING ANY KEY
// HERE; the grep is cheap and it is the difference between a landing and a regression.
//
// ⛔ t25's PROHIBITION CARRIES TO ALL FIFTY: no `operator==`, no `operator!=`, no identity test,
// and NO MEMBER.  A C++ op handle compares `Operation *`; an `OpInst` is an op's printed
// CONTENT.  No `fN` either -- nothing constructs a trait base, these are upcast targets only.
// ============================================================================================
namespace mlir {
// mlir/IR/OpDefinition.h:698-706 -- `template <typename ResultType> class OneTypedResult {
// public: template <typename ConcreteType> class Impl : public TraitBase<...> { ... }; };`.
// Declared here ONLY so the fifty keys below can be SPELLED; it is a pure tag base in the model
// (no state, one `Operation *` through the op it is mixed into) and no member of it is mapped.
namespace OpTrait {
template <typename ResultType> class OneTypedResult {
public:
  template <typename ConcreteType> class Impl {};
};
} // namespace OpTrait

// The ODS-generated op classes the fifty keys name, each one `Operation *` through its OpState
// base, declared for the t161 reason and nothing more.  `arith::ConstantOp` is NOT redeclared
// here -- it already exists at :2902 and a second declaration would be a duplicate.
namespace arith {
class AddFOp {};
class AddIOp {};
class AndIOp {};
class BitcastOp {};
class CmpFOp {};
class CmpIOp {};
class DivFOp {};
class DivSIOp {};
class ExtFOp {};
class ExtSIOp {};
class IndexCastOp {};
class MaxSIOp {};
class MaximumFOp {};
class MinSIOp {};
class MinimumFOp {};
class MulFOp {};
class MulIOp {};
class NegFOp {};
class OrIOp {};
class RemSIOp {};
class RemUIOp {};
class SelectOp {};
class ShRSIOp {};
class SubFOp {};
class SubIOp {};
class TruncFOp {};
class TruncIOp {};
class XOrIOp {};
} // namespace arith

namespace vector {
class BitCastOp {};
class ExtractOp {};
class ExtractStridedSliceOp {};
class FromElementsOp {};
class InsertOp {};
class InsertStridedSliceOp {};
class LoadOp {};
class ShapeCastOp {};
class ShuffleOp {};
} // namespace vector

namespace sentient {
class ConstantOp {};
} // namespace sentient

namespace dataflow {
class GetLocalUnitOp {};
class GetLogicalMemoryViewOp {};
class ReceiveOp {};
} // namespace dataflow

namespace vectorchain {
class CreateAffineMaskOp {};
class MultiplyAndAccumulateOp {};
class MultiplyOp {};
class PackOp {};
class RotateOp {};
class SelectOp {};
class ShuffleOp {};
} // namespace vectorchain

namespace uniform {
class DefImmutableMappingOp {};
} // namespace uniform

} // namespace mlir

// t167 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_ConstantOp_, 122 occurrences.  DEF `ops::mlir_arith_ConstantOp` present.
using t167 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::ConstantOp>;
// t168 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_SelectOp_, 23 occurrences.  DEF `ops::mlir_arith_SelectOp` present.
using t168 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::SelectOp>;
// t169 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_vector_ExtractOp_, 23 occurrences.  DEF `ops::mlir_vector_ExtractOp` present.
using t169 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::vector::ExtractOp>;
// t170 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_sentient_ConstantOp_, 19 occurrences.  DEF `ops::mlir_sentient_ConstantOp` present.
using t170 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::sentient::ConstantOp>;
// t171 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_CmpIOp_, 17 occurrences.  DEF `ops::mlir_arith_CmpIOp` present.
using t171 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::CmpIOp>;
// t172 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_MulIOp_, 16 occurrences.  DEF `ops::mlir_arith_MulIOp` present.
using t172 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::MulIOp>;
// t173 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_VectorType__Impl_mlir_vector_InsertOp_, 14 occurrences.  DEF `ops::mlir_vector_InsertOp` present.
using t173 = mlir::OpTrait::OneTypedResult<mlir::VectorType>::Impl<mlir::vector::InsertOp>;
// t174 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_AddIOp_, 13 occurrences.  DEF `ops::mlir_arith_AddIOp` present.
using t174 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::AddIOp>;
// t175 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_dataflow_GetLogicalMemoryViewOp_, 11 occurrences.  DEF `ops::mlir_dataflow_GetLogicalMemoryViewOp` present.
using t175 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::dataflow::GetLogicalMemoryViewOp>;
// t176 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_vectorchain_ShuffleOp_, 11 occurrences.  DEF `ops::mlir_vectorchain_ShuffleOp` present.
using t176 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::vectorchain::ShuffleOp>;
// t177 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_VectorType__Impl_mlir_vector_ShapeCastOp_, 10 occurrences.  DEF `ops::mlir_vector_ShapeCastOp` present.
using t177 = mlir::OpTrait::OneTypedResult<mlir::VectorType>::Impl<mlir::vector::ShapeCastOp>;
// t178 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_AddFOp_, 9 occurrences.  DEF `ops::mlir_arith_AddFOp` present.
using t178 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::AddFOp>;
// t179 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_MulFOp_, 9 occurrences.  DEF `ops::mlir_arith_MulFOp` present.
using t179 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::MulFOp>;
// t180 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_SubIOp_, 7 occurrences.  DEF `ops::mlir_arith_SubIOp` present.
using t180 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::SubIOp>;
// t181 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_vectorchain_RotateOp_, 7 occurrences.  DEF `ops::mlir_vectorchain_RotateOp` present.
using t181 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::vectorchain::RotateOp>;
// t182 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_TruncIOp_, 7 occurrences.  DEF `ops::mlir_arith_TruncIOp` present.
using t182 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::TruncIOp>;
// t183 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_VectorType__Impl_mlir_vector_BitCastOp_, 6 occurrences.  DEF `ops::mlir_vector_BitCastOp` present.
using t183 = mlir::OpTrait::OneTypedResult<mlir::VectorType>::Impl<mlir::vector::BitCastOp>;
// t184 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_XOrIOp_, 6 occurrences.  DEF `ops::mlir_arith_XOrIOp` present.
using t184 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::XOrIOp>;
// t185 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_IndexCastOp_, 6 occurrences.  DEF `ops::mlir_arith_IndexCastOp` present.
using t185 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::IndexCastOp>;
// t186 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_ExtSIOp_, 5 occurrences.  DEF `ops::mlir_arith_ExtSIOp` present.
using t186 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::ExtSIOp>;
// t187 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_BitcastOp_, 5 occurrences.  DEF `ops::mlir_arith_BitcastOp` present.
using t187 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::BitcastOp>;
// t188 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_DivFOp_, 5 occurrences.  DEF `ops::mlir_arith_DivFOp` present.
using t188 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::DivFOp>;
// t189 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_dataflow_ReceiveOp_, 4 occurrences.  DEF `ops::mlir_dataflow_ReceiveOp` present.
using t189 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::dataflow::ReceiveOp>;
// t190 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_CmpFOp_, 4 occurrences.  DEF `ops::mlir_arith_CmpFOp` present.
using t190 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::CmpFOp>;
// t191 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_AndIOp_, 4 occurrences.  DEF `ops::mlir_arith_AndIOp` present.
using t191 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::AndIOp>;
// t192 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_DivSIOp_, 4 occurrences.  DEF `ops::mlir_arith_DivSIOp` present.
using t192 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::DivSIOp>;
// t193 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_NegFOp_, 3 occurrences.  DEF `ops::mlir_arith_NegFOp` present.
using t193 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::NegFOp>;
// t194 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_MinimumFOp_, 3 occurrences.  DEF `ops::mlir_arith_MinimumFOp` present.
using t194 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::MinimumFOp>;
// t195 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_MinSIOp_, 3 occurrences.  DEF `ops::mlir_arith_MinSIOp` present.
using t195 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::MinSIOp>;
// t196 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_MaximumFOp_, 3 occurrences.  DEF `ops::mlir_arith_MaximumFOp` present.
using t196 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::MaximumFOp>;
// t197 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_MaxSIOp_, 3 occurrences.  DEF `ops::mlir_arith_MaxSIOp` present.
using t197 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::MaxSIOp>;
// t198 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_OrIOp_, 3 occurrences.  DEF `ops::mlir_arith_OrIOp` present.
using t198 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::OrIOp>;
// t199 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_RemUIOp_, 3 occurrences.  DEF `ops::mlir_arith_RemUIOp` present.
using t199 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::RemUIOp>;
// t200 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_VectorType__Impl_mlir_vector_ExtractStridedSliceOp_, 3 occurrences.  DEF `ops::mlir_vector_ExtractStridedSliceOp` present.
using t200 = mlir::OpTrait::OneTypedResult<mlir::VectorType>::Impl<mlir::vector::ExtractStridedSliceOp>;
// t201 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_VectorType__Impl_mlir_vector_InsertStridedSliceOp_, 3 occurrences.  DEF `ops::mlir_vector_InsertStridedSliceOp` present.
using t201 = mlir::OpTrait::OneTypedResult<mlir::VectorType>::Impl<mlir::vector::InsertStridedSliceOp>;
// t202 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_ExtFOp_, 2 occurrences.  DEF `ops::mlir_arith_ExtFOp` present.
using t202 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::ExtFOp>;
// t203 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_TruncFOp_, 2 occurrences.  DEF `ops::mlir_arith_TruncFOp` present.
using t203 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::TruncFOp>;
// t204 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_SubFOp_, 2 occurrences.  DEF `ops::mlir_arith_SubFOp` present.
using t204 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::SubFOp>;
// t205 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_ShRSIOp_, 2 occurrences.  DEF `ops::mlir_arith_ShRSIOp` present.
using t205 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::ShRSIOp>;
// t206 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_vectorchain_MultiplyOp_, 2 occurrences.  DEF `ops::mlir_vectorchain_MultiplyOp` present.
using t206 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::vectorchain::MultiplyOp>;
// t207 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_vectorchain_MultiplyAndAccumulateOp_, 2 occurrences.  DEF `ops::mlir_vectorchain_MultiplyAndAccumulateOp` present.
using t207 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::vectorchain::MultiplyAndAccumulateOp>;
// t208 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_VectorType__Impl_mlir_vectorchain_CreateAffineMaskOp_, 2 occurrences.  DEF `ops::mlir_vectorchain_CreateAffineMaskOp` present.
using t208 = mlir::OpTrait::OneTypedResult<mlir::VectorType>::Impl<mlir::vectorchain::CreateAffineMaskOp>;
// t209 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_vectorchain_SelectOp_, 2 occurrences.  DEF `ops::mlir_vectorchain_SelectOp` present.
using t209 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::vectorchain::SelectOp>;
// t210 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_vectorchain_PackOp_, 2 occurrences.  DEF `ops::mlir_vectorchain_PackOp` present.
using t210 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::vectorchain::PackOp>;
// t211 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_IndexType__Impl_mlir_uniform_DefImmutableMappingOp_, 2 occurrences.  DEF `ops::mlir_uniform_DefImmutableMappingOp` present.
using t211 = mlir::OpTrait::OneTypedResult<mlir::IndexType>::Impl<mlir::uniform::DefImmutableMappingOp>;
// t212 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_IndexType__Impl_mlir_dataflow_GetLocalUnitOp_, 2 occurrences.  DEF `ops::mlir_dataflow_GetLocalUnitOp` present.
using t212 = mlir::OpTrait::OneTypedResult<mlir::IndexType>::Impl<mlir::dataflow::GetLocalUnitOp>;
// t213 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_Type__Impl_mlir_arith_RemSIOp_, 1 occurrence.  DEF `ops::mlir_arith_RemSIOp` present.
using t213 = mlir::OpTrait::OneTypedResult<mlir::Type>::Impl<mlir::arith::RemSIOp>;
// t214 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_VectorType__Impl_mlir_vector_LoadOp_, 1 occurrence.  DEF `ops::mlir_vector_LoadOp` present.
using t214 = mlir::OpTrait::OneTypedResult<mlir::VectorType>::Impl<mlir::vector::LoadOp>;
// t215 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_VectorType__Impl_mlir_vector_FromElementsOp_, 1 occurrence.  DEF `ops::mlir_vector_FromElementsOp` present.
using t215 = mlir::OpTrait::OneTypedResult<mlir::VectorType>::Impl<mlir::vector::FromElementsOp>;
// t216 -- Cpp2RustUnmapped_mlir_OpTrait_OneTypedResult_mlir_VectorType__Impl_mlir_vector_ShuffleOp_, 1 occurrence.  DEF `ops::mlir_vector_ShuffleOp` present.
using t216 = mlir::OpTrait::OneTypedResult<mlir::VectorType>::Impl<mlir::vector::ShuffleOp>;

// ============================================================================================
// t217-t235 -- THREE MORE `mlir::OpTrait` CRTP TRAIT BASES, NINETEEN concrete instantiations.
// Same shape and same justification as t167-t216 (`OneTypedResult<RT>::Impl<Op>`): the implicit
// derived-to-base upcast of an ODS op handle to one of its trait bases.
//
// MEASUREMENT 2026-09-28 over `/home/agent/work/verify0928/out`, each family anchored to its own
// STANDALONE prefix `(^|[^A-Za-z0-9_])Cpp2RustUnmapped_mlir_OpTrait_<family>` -- NOT to the
// substring inside the much longer `Cpp2RustUnmapped_mlir_Op_<Op>__<traits>_` names of the
// separately-unkeyed `mlir::Op<...>`, because the short family names ARE substrings of those and
// counting them together measures two defects at once:
//     OpTrait::detail::MultiResultTraitBase   113 occurrences /  9 distinct spellings
//     OpTrait::SingleBlock                     77 occurrences / 10 distinct spellings
//     OpTrait::detail::MultiOperandTraitBase    16 occurrences /  7 distinct spellings
// EVERY occurrence of all three is in CAST-TARGET (type) POSITION and NO MEMBER IS EVER READ, so
// each site needs a NAME for the cast target and NO MEMBER RULES AT ALL.
//
// ⛔ MONOMORPHIC, FULLY CONCRETE, one `using` per spelling -- for the t166 / t37-t39 / t167-t216
// reason.  Each of these three buckets was EMPTY before this block, so a GENERIC key
// (`MultiResultTraitBase<T1, T2>`) would be the SOLE candidate in its bucket and `search()`'s
// longer-src tie-break (mapper.cpp:430-437) could not protect it: that is the densemap failure
// verbatim, where a short key ate `llvm::StringRef, void`.  Arity 0 rules a swallow out entirely.
//
// ⛔ THREE SPELLINGS ARE DELIBERATELY LEFT UNKEYED AND STAY LOUD, and it is WHOLE-DIALECT:
// `mlir::ktdf` has NO generated `DEF` in dataflow_ods.rs at all --
//     SingleBlock<mlir::ktdf::StageOp>                                (3 occurrences)
//     SingleBlock<mlir::ktdf::PipelineOp>                             (2 occurrences)
//     MultiOperandTraitBase<mlir::ktdf::PrivateYieldOp, VariadicOperands>  (3 occurrences)
// `grep -cE "struct mlir_ktdf_<Op>\b"` against
// dataflowir-gen-654e676bccb4a05f/out/dataflow_ods.rs returns 0 for all three, exactly as
// `mlir::LLVM` / `mlir::math` / `mlir::memref` did for t167-t216.  A key naming an ABSENT DEF
// records fine, passes the load smoke test, and FAILS AT RUSTC -- so this is a dataflowir-gen
// `.td` coverage gap, not a rules gap, and it must not be papered over here.
//
// ⛔ FOUR MORE `MultiOperandTraitBase` SPELLINGS ARE ALSO LEFT OUT, for a DIFFERENT and purely
// mechanical reason: their second template argument is `mlir::OpTrait::NOperands<N>::Impl` /
// `mlir::OpTrait::AtLeastNOperands<N>::Impl`, whose NON-TYPE argument `N` is ERASED by the
// unmapped-name mangler (`..._NOperands____Impl_`), so the corpus name does not determine which
// arity to write and a guessed `N` would be a DIFFERENT type that silently never matches:
//     vectorchain::RotateOp   / NOperands<N>::Impl         (3 occurrences)
//     vectorchain::ShuffleOp  / AtLeastNOperands<N>::Impl  (2)
//     vectorchain::CastOp     / AtLeastNOperands<N>::Impl  (2)
//     sentient::ForOp         / AtLeastNOperands<N>::Impl  (2)
// These need `N` read out of the ODS-generated C++ op definition, not out of the emitted corpus.
//
// ⛔ t25's PROHIBITION CARRIES TO ALL NINETEEN: no `operator==`, no `operator!=`, no identity
// test, and NO MEMBER.  No `fN` either -- nothing constructs a trait base, these are upcast
// targets only.
// ============================================================================================
namespace mlir {
namespace OpTrait {
// mlir/IR/OpDefinition.h -- the two `template <typename ConcreteType> class` trait tags that
// appear as the TEMPLATE-TEMPLATE second argument of the two `detail::` bases below, and
// `SingleBlock` itself (OpDefinition.h:881).  Declared here ONLY so the nineteen keys can be
// SPELLED; all three are pure tag bases in the model and no member of any of them is mapped.
template <typename ConcreteType> class VariadicResults {};
template <typename ConcreteType> class VariadicOperands {};
template <typename ConcreteType> class SingleBlock {};
// `OneRegion` (OpDefinition.h:556, `class OneRegion : public TraitBase<ConcreteType, OneRegion>`)
// -- a FOURTH trait bucket of this same family, declared for t320 and for the same reason as the
// three above: a pure tag base, no member of it mapped.
template <typename ConcreteType> class OneRegion {};
namespace detail {
// mlir/IR/OpDefinition.h:628 / :560 -- `template <typename ConcreteType,
// template <typename> class TraitType> class MultiResultTraitBase` and its operand twin.
template <typename ConcreteType, template <typename> class TraitType>
class MultiResultTraitBase {};
template <typename ConcreteType, template <typename> class TraitType>
class MultiOperandTraitBase {};
} // namespace detail
} // namespace OpTrait

// The ODS-generated op classes these keys name that are NOT already declared above.  Each one is
// `Operation *` through its `OpState` base, declared for the t161 reason and nothing more.
// ALREADY DECLARED ELSEWHERE and deliberately NOT redeclared here (a second declaration would be
// a duplicate): `mlir::ModuleOp` (:963), `mlir::UnrealizedConversionCastOp` (:977),
// `mlir::scf::ForOp` / `mlir::scf::IfOp` (:611), `mlir::func::CallOp` (:2845),
// `mlir::affine::AffineForOp` (:2924), `mlir::sentient::ConstantOp` (:4345).
namespace agen {
class CompositeLoadOp {};
class CompositeStoreOp {};
} // namespace agen

namespace dataflow {
class GetUnitOp {};
class ProgramUnitOp {};
} // namespace dataflow

namespace sentient {
class ForOp {};
class IfOp {};
class MacOp {};
class YieldOp {};
} // namespace sentient

namespace uniform {
class UniformizeRegionsOp {};
} // namespace uniform

} // namespace mlir

// t217 -- Cpp2RustUnmapped_mlir_OpTrait_detail_MultiResultTraitBase_mlir_dataflow_GetUnitOp__mlir_OpTrait_VariadicResults_, 72 occurrences.  DEF `ops::mlir_dataflow_GetUnitOp` present.
using t217 = mlir::OpTrait::detail::MultiResultTraitBase<mlir::dataflow::GetUnitOp, mlir::OpTrait::VariadicResults>;
// t218 -- Cpp2RustUnmapped_mlir_OpTrait_detail_MultiResultTraitBase_mlir_scf_ForOp__mlir_OpTrait_VariadicResults_, 14 occurrences.  DEF `ops::mlir_scf_ForOp` present.
using t218 = mlir::OpTrait::detail::MultiResultTraitBase<mlir::scf::ForOp, mlir::OpTrait::VariadicResults>;
// t219 -- Cpp2RustUnmapped_mlir_OpTrait_detail_MultiResultTraitBase_mlir_UnrealizedConversionCastOp__mlir_OpTrait_VariadicResults_, 7 occurrences.  DEF `ops::mlir_UnrealizedConversionCastOp` present.
using t219 = mlir::OpTrait::detail::MultiResultTraitBase<mlir::UnrealizedConversionCastOp, mlir::OpTrait::VariadicResults>;
// t220 -- Cpp2RustUnmapped_mlir_OpTrait_detail_MultiResultTraitBase_mlir_scf_IfOp__mlir_OpTrait_VariadicResults_, 6 occurrences.  DEF `ops::mlir_scf_IfOp` present.
using t220 = mlir::OpTrait::detail::MultiResultTraitBase<mlir::scf::IfOp, mlir::OpTrait::VariadicResults>;
// t221 -- Cpp2RustUnmapped_mlir_OpTrait_detail_MultiResultTraitBase_mlir_sentient_IfOp__mlir_OpTrait_VariadicResults_, 4 occurrences.  DEF `ops::mlir_sentient_IfOp` present.
using t221 = mlir::OpTrait::detail::MultiResultTraitBase<mlir::sentient::IfOp, mlir::OpTrait::VariadicResults>;
// t222 -- Cpp2RustUnmapped_mlir_OpTrait_detail_MultiResultTraitBase_mlir_sentient_ForOp__mlir_OpTrait_VariadicResults_, 4 occurrences.  DEF `ops::mlir_sentient_ForOp` present.
using t222 = mlir::OpTrait::detail::MultiResultTraitBase<mlir::sentient::ForOp, mlir::OpTrait::VariadicResults>;
// t223 -- Cpp2RustUnmapped_mlir_OpTrait_detail_MultiResultTraitBase_mlir_uniform_UniformizeRegionsOp__mlir_OpTrait_VariadicResults_, 3 occurrences.  DEF `ops::mlir_uniform_UniformizeRegionsOp` present.
using t223 = mlir::OpTrait::detail::MultiResultTraitBase<mlir::uniform::UniformizeRegionsOp, mlir::OpTrait::VariadicResults>;
// t224 -- Cpp2RustUnmapped_mlir_OpTrait_detail_MultiResultTraitBase_mlir_sentient_MacOp__mlir_OpTrait_VariadicResults_, 2 occurrences.  DEF `ops::mlir_sentient_MacOp` present.
using t224 = mlir::OpTrait::detail::MultiResultTraitBase<mlir::sentient::MacOp, mlir::OpTrait::VariadicResults>;
// t225 -- Cpp2RustUnmapped_mlir_OpTrait_detail_MultiResultTraitBase_mlir_func_CallOp__mlir_OpTrait_VariadicResults_, 1 occurrence.  DEF `ops::mlir_func_CallOp` present.
using t225 = mlir::OpTrait::detail::MultiResultTraitBase<mlir::func::CallOp, mlir::OpTrait::VariadicResults>;
// t226 -- Cpp2RustUnmapped_mlir_OpTrait_SingleBlock_mlir_affine_AffineForOp_, 14 occurrences.  DEF `ops::mlir_affine_AffineForOp` present.
using t226 = mlir::OpTrait::SingleBlock<mlir::affine::AffineForOp>;
// t227 -- Cpp2RustUnmapped_mlir_OpTrait_SingleBlock_mlir_sentient_IfOp_, 13 occurrences.  DEF `ops::mlir_sentient_IfOp` present.
using t227 = mlir::OpTrait::SingleBlock<mlir::sentient::IfOp>;
// t228 -- Cpp2RustUnmapped_mlir_OpTrait_SingleBlock_mlir_scf_ForOp_, 13 occurrences.  DEF `ops::mlir_scf_ForOp` present.
using t228 = mlir::OpTrait::SingleBlock<mlir::scf::ForOp>;
// t229 -- Cpp2RustUnmapped_mlir_OpTrait_SingleBlock_mlir_sentient_ForOp_, 11 occurrences.  DEF `ops::mlir_sentient_ForOp` present.
using t229 = mlir::OpTrait::SingleBlock<mlir::sentient::ForOp>;
// t230 -- Cpp2RustUnmapped_mlir_OpTrait_SingleBlock_mlir_dataflow_ProgramUnitOp_, 8 occurrences.  DEF `ops::mlir_dataflow_ProgramUnitOp` present.
using t230 = mlir::OpTrait::SingleBlock<mlir::dataflow::ProgramUnitOp>;
// t231 -- Cpp2RustUnmapped_mlir_OpTrait_SingleBlock_mlir_agen_CompositeStoreOp_, 6 occurrences.  DEF `ops::mlir_agen_CompositeStoreOp` present.
using t231 = mlir::OpTrait::SingleBlock<mlir::agen::CompositeStoreOp>;
// t232 -- Cpp2RustUnmapped_mlir_OpTrait_SingleBlock_mlir_ModuleOp_, 4 occurrences.  DEF `ops::mlir_ModuleOp` present.
using t232 = mlir::OpTrait::SingleBlock<mlir::ModuleOp>;
// t233 -- Cpp2RustUnmapped_mlir_OpTrait_SingleBlock_mlir_agen_CompositeLoadOp_, 3 occurrences.  DEF `ops::mlir_agen_CompositeLoadOp` present.
using t233 = mlir::OpTrait::SingleBlock<mlir::agen::CompositeLoadOp>;
// t234 -- Cpp2RustUnmapped_mlir_OpTrait_detail_MultiOperandTraitBase_mlir_sentient_YieldOp__mlir_OpTrait_VariadicOperands_, 2 occurrences.  DEF `ops::mlir_sentient_YieldOp` present.
using t234 = mlir::OpTrait::detail::MultiOperandTraitBase<mlir::sentient::YieldOp, mlir::OpTrait::VariadicOperands>;
// t235 -- Cpp2RustUnmapped_mlir_OpTrait_detail_MultiOperandTraitBase_mlir_UnrealizedConversionCastOp__mlir_OpTrait_VariadicOperands_, 2 occurrences.  DEF `ops::mlir_UnrealizedConversionCastOp` present.
using t235 = mlir::OpTrait::detail::MultiOperandTraitBase<mlir::UnrealizedConversionCastOp, mlir::OpTrait::VariadicOperands>;

// ---------------------------------------------------------------------------
// t300-t304 -- THE FIVE `mlir::ktdf` SPELLINGS OF THE SAME THREE TRAIT BUCKETS, which the
// block above deliberately left LOUD as a dataflowir-gen `.td` coverage gap.  That gap is
// CLOSED (the falsification, with the generated lines, is recorded at the `namespace ktdf`
// declarations above); these are the LAST survivors of this family.
//
// ⭐ MEASURED, not inferred.  Anchored `grep -oE '(^|[^A-Za-z0-9_])<name>'` over the 226-file
// v30 sweep at fresh30/out (emitted 14:19:56-14:24:09, i.e. NEWER than the pin/ir.v30/mlir
// module at 14:10:44 that produced it, so its silence is evidence) shows the three SHORT /
// arity-0 family names at **0 occurrences in 0 files** -- t217-t235 are landed AND effective --
// while exactly **8 instantiated survivors** remain, and every one is `mlir::ktdf`:
//     Cpp2RustUnmapped_mlir_OpTrait_SingleBlock_mlir_ktdf_PipelineOp_      3   -> t300
//     Cpp2RustUnmapped_mlir_OpTrait_SingleBlock_mlir_ktdf_StageOp_         1   -> t301
//     Cpp2RustUnmapped_mlir_OpTrait_SingleBlock_mlir_ktdf_PrivateOp_       1   -> t302
//     ..._MultiResultTraitBase_mlir_ktdf_PrivateOp__..._VariadicResults_   2   -> t303
//     ..._MultiOperandTraitBase_mlir_ktdf_PrivateYieldOp__..._VariadicOperands_  1 -> t304
// ⚠️ TWO OF THESE FIVE ARE NOT ON THE :4507 LEFT-OUT LIST AT ALL (`SingleBlock<PrivateOp>` and
// `MultiResultTraitBase<PrivateOp, VariadicResults>`), so that list was never the complete
// survivor set and the census had to be re-run against a corpus rather than trusted.
//
// ⛔ The 6 remaining corpus tokens that CONTAIN `SingleBlock` are a DIFFERENT DEFECT and are
// NOT addressed here: they are the long `Cpp2RustUnmapped_mlir_Op_<Op>__<traitlist>_` names of
// the separately-unkeyed `mlir::Op<...>` base (ktdf::lowering::ExecuteOnOp, ddl::*).  Counting
// them with this family measures two defects at once, which is why every count above is
// anchored on both sides.
//
// ⛔ ARITY-0, FULLY CONCRETE, one `using` per spelling -- the t166 / t37-t39 reason, and it is
// load-bearing here: `GetTypeMapKey` truncates at the first `<` so arity is NOT in the key, and
// `matchTemplate` captures to the next SAME-DEPTH literal with a comma NOT acting as a
// delimiter, so a generic `MultiResultTraitBase<T1, T2>` would let `T1` swallow the whole
// argument list.  Arity 0 rules the swallow out entirely.
// ⛔ t25's PROHIBITION CARRIES: all five are CAST-TARGET (type) position ONLY and NO MEMBER OF
// ANY OF THEM IS EVER READ anywhere in the sweep, so each needs a NAME and NO MEMBER RULES AT
// ALL -- and NO `fN` either, because nothing ever constructs a trait base.  These are upcast
// targets, which is exactly what makes them trivial.
// t300 -- Cpp2RustUnmapped_mlir_OpTrait_SingleBlock_mlir_ktdf_PipelineOp_, 3 occurrences.  DEF `ops::mlir_ktdf_PipelineOp` present (dataflow_ods.rs:4826).
using t300 = mlir::OpTrait::SingleBlock<mlir::ktdf::PipelineOp>;
// t301 -- Cpp2RustUnmapped_mlir_OpTrait_SingleBlock_mlir_ktdf_StageOp_, 1 occurrence.  DEF `ops::mlir_ktdf_StageOp` present.
using t301 = mlir::OpTrait::SingleBlock<mlir::ktdf::StageOp>;
// t302 -- Cpp2RustUnmapped_mlir_OpTrait_SingleBlock_mlir_ktdf_PrivateOp_, 1 occurrence.  DEF `ops::mlir_ktdf_PrivateOp` present.
using t302 = mlir::OpTrait::SingleBlock<mlir::ktdf::PrivateOp>;
// t303 -- Cpp2RustUnmapped_mlir_OpTrait_detail_MultiResultTraitBase_mlir_ktdf_PrivateOp__mlir_OpTrait_VariadicResults_, 2 occurrences.  DEF `ops::mlir_ktdf_PrivateOp` present.
using t303 = mlir::OpTrait::detail::MultiResultTraitBase<mlir::ktdf::PrivateOp, mlir::OpTrait::VariadicResults>;
// t304 -- Cpp2RustUnmapped_mlir_OpTrait_detail_MultiOperandTraitBase_mlir_ktdf_PrivateYieldOp__mlir_OpTrait_VariadicOperands_, 1 occurrence.  DEF `ops::mlir_ktdf_PrivateYieldOp` present.
using t304 = mlir::OpTrait::detail::MultiOperandTraitBase<mlir::ktdf::PrivateYieldOp, mlir::OpTrait::VariadicOperands>;

// ---------------------------------------------------------------------------
// t320 -- `mlir::OpTrait::OneRegion<mlir::ModuleOp>`, THE LAST SURVIVOR OF THIS WHOLE FAMILY,
// and it was found by RE-CENSUSING the corpus rather than by reading any refusal's list.
//
// ⭐ FOUND BY CENSUS, NOT BY LIST.  Anchored `grep -ohE 'Cpp2RustUnmapped_mlir_OpTrait[A-Za-z0-9_]*'`
// over all 226 files of the v30 sweep at fresh30/out returns exactly SIX distinct survivors: the
// five `mlir::ktdf` spellings t300-t304 already landed, plus this one --
//     Cpp2RustUnmapped_mlir_OpTrait_OneRegion_mlir_ModuleOp_    1 occurrence, 1 file
// in `dbo__src__Transforms__ReduceToInitBin.cpp.rs`.  `OneRegion` had ZERO mentions anywhere in
// this file before now (`grep -n OneRegion rules/mlir/src.cpp` = 0 hits): it is a FOURTH trait
// bucket that no refusal in this file ever enumerated, so no "left-out list" would have led here.
//
// ⭐ DEF VERIFIED BEFORE THE KEY WAS WRITTEN, which is the whole discipline of this row:
// `grep -nE "struct mlir_ModuleOp\b"` against
//   dataflowir-gen-654e676bccb4a05f/out/dataflow_ods.rs  ->  `:4800  pub struct mlir_ModuleOp;`
// i.e. 1 hit.  Same DEF t232 (`SingleBlock<mlir::ModuleOp>`) already names, so the target model is
// not a new claim: the trait base of an op IS that op (the t161/t162 precedent).
//
// ⛔ MONOMORPHIC, FULLY CONCRETE, arity 0 -- for the t166 / t37-t39 / t217-t235 reason.  The
// `mlir::OpTrait::OneRegion` bucket was EMPTY before this key, so a generic `OneRegion<T>` would be
// the SOLE candidate in it and `search()`'s longer-src tie-break (mapper.cpp:430-437) could not
// protect it: the densemap failure verbatim.  Arity 0 also means `matchTemplate`'s capture
// (`findNextLiteralSameDepth`, :173) never runs, so the swallow bug cannot occur here at all.
//
// ⛔ t25's PROHIBITION CARRIES: cast-target position only, NO MEMBER IS EVER READ, so a NAME and
// no member rules is the complete and correct key.  CONFIRMED for this spelling specifically --
// the single site in ReduceToInitBin.cpp is an implicit derived-to-base upcast of a `ModuleOp` to
// its `OneRegion` trait base and reads nothing off it.  No `fN`: nothing constructs a trait base.
// t320 -- Cpp2RustUnmapped_mlir_OpTrait_OneRegion_mlir_ModuleOp_, 1 occurrence.  DEF `ops::mlir_ModuleOp` present (dataflow_ods.rs:4800).
using t320 = mlir::OpTrait::OneRegion<mlir::ModuleOp>;

// ---------------------------------------------------------------------------
// t400 -- `mlir::detail::SymbolOpInterfaceTrait<mlir::ktdf_arch::DeviceOp>`, THE SINGLE BIGGEST
// REMAINING `mlir_` SURVIVOR IN THE SWEEP (18 occurrences, 9 files).  This block REPLACES the
// refusal that stood here, and records the falsification rather than deleting it.
//
// ⛔⛔ WHAT THE REFUSAL SAID, AND WHY IT WAS RIGHT WHEN WRITTEN: it recorded
// `grep -nE "struct mlir_ktdf_arch_DeviceOp\b"` against dataflow_ods.rs as **0 hits**, plus
// `grep -oE "struct mlir_ktdf[A-Za-z0-9_]*"` = 18 names of which ZERO were `ktdf::arch`, and
// concluded that `mlir::ktdf::arch` was a live dataflowir-gen `.td` coverage gap that had to stay
// LOUD.  It also warned that a key naming an absent DEF "would record fine, pass the load smoke
// test, and FAIL AT RUSTC".  Both halves were correct at the time.
//
// ⭐⭐ THE GAP IS CLOSED BY `dataflowir-gen` COMMIT `ee457cc` ("add LLVM, math, memref and
// ktdf_arch dialects"), and the refusal's OWN TEST is the proof.  Re-run 2026-09-28 against
//   dataflowir-gen-654e676bccb4a05f/out/dataflow_ods.rs   (mtime 2026-09-28 16:30:18, i.e. built
//   AFTER ee457cc -- a stale build dir would lie here, so the mtime is part of the evidence)
// the identical `grep -cE '^ *pub struct mlir_ktdf_arch_DeviceOp;'` returns **1**, i.e. 0 -> 1.
// So the DEF exists and the absent-DEF hazard does not apply to this key.
//
// ⭐ THE C++ NAMESPACE IS `mlir::ktdf_arch`, NOT `mlir::ktdf::arch` (KTDFArchDialect.td:32), which
// is why `ktdf_arch` is ONE identifier here and why the mangled marker is
// `..._mlir_ktdf_arch_DeviceOp_` with a single `_arch_` segment and no separate `arch` level.  The
// emitted placeholder name is itself the confirmation, and it is what this key is written against
// -- the ground truth, not any reconstruction from the C++ spelling.
//
// ⛔ ARITY 0, FULLY CONCRETE -- the t166 / t37-t39 / t217-t235 / t320 reason.  `GetTypeMapKey`
// truncates at the first `<` so arity is not in the key, and `matchTemplate`'s capture
// (`findNextLiteralSameDepth`, mapper.cpp:173) never runs at arity 0, so the swallow bug cannot
// occur here at all.  A generic `SymbolOpInterfaceTrait<T1>` would additionally be the SOLE
// candidate in a previously-empty bucket, where `search()`'s longer-src tie-break
// (mapper.cpp:430-437) could not protect it.
//
// ⛔ THE :3707 REFUSAL'S NARROWER GROUND IS ALSO ANSWERED, and it is the half that mattered most.
// That note refused g088 on the ground that `mlir::detail::` INTERFACE traits carry DEFAULT METHOD
// IMPLEMENTATIONS rather than being pure tags, so "no member is read" needed proving FOR THIS
// SPELLING.  It proved it: any default method the trait supplies (`getName`/`setName`) is called in
// the corpus THROUGH THE OP (`op.getName()`), so it records against the Op or the OpInterface and
// NEVER against the trait class type.  ⭐ RE-CONFIRMED HERE BY LOOKING AT THE EMITTED SITES rather
// than at the C++: all 18 occurrences are in `... as Cpp2RustUnmapped_mlir_detail_
// SymbolOpInterfaceTrait_mlir_ktdf_arch_DeviceOp_)` position -- a bare `as` CAST TARGET, 2 per file
// across 9 files.  Not one site reads a member off it, not one default-constructs it.  ⚠️ That
// check is not optional politeness: AN UNMAPPED MEMBER DOES NOT ABORT -- the converter emits the
// call TEXTUALLY at rc=0 with no placeholder token, invisible to every census and to
// no-placeholders.sh -- so "nothing reads a member" is the ONLY thing that makes a bare type key
// complete.  Here it holds, which makes the key trivial rather than dead.
//
// ⛔ NO `fN`: nothing constructs a trait base, and no site default-constructs this one.  MODEL: the
// trait base of an op IS that op (the t161/t162 precedent, as t320 and t300-t304 already apply it),
// so the target is `ops::mlir_ktdf_arch_DeviceOp`'s own DEF.
namespace mlir {
namespace detail {
// SymbolInterfaces.h.inc:262 -- `template <typename ConcreteType> class SymbolOpInterfaceTrait`.
// Declared here ONLY so t400 can be SPELLED; no member of it is mapped, per the block above.
template <typename ConcreteType> class SymbolOpInterfaceTrait {};
} // namespace detail
namespace ktdf_arch {
// The ODS-generated `mlir::ktdf_arch` op class named by t400.  `Operation *` through its `OpState`
// base, declared for the t161 reason and nothing more.  ⚠️ `namespace ktdf_arch` is ALREADY OPEN at
// :791 for `MemoryType` / `ExecutionUnitType`; this is a second block in the same namespace, not a
// redeclaration of those.  `DeviceOp` is declared nowhere else in this file.
class DeviceOp {};
} // namespace ktdf_arch
} // namespace mlir
// t400 -- Cpp2RustUnmapped_mlir_detail_SymbolOpInterfaceTrait_mlir_ktdf_arch_DeviceOp_, 18
// occurrences in 9 files.  DEF `ops::mlir_ktdf_arch_DeviceOp` present (verified 0 -> 1 above).
using t400 = mlir::detail::SymbolOpInterfaceTrait<mlir::ktdf_arch::DeviceOp>;

// ---------------------------------------------------------------------------
// ⛔ THE REST OF `ee457cc`'s 31 NEWLY-GENERATABLE SPELLINGS, AND WHY ONLY ONE MORE GROUP OF THEM
// IS EVEN A CANDIDATE.  All 31 DEFs were confirmed present by the same `grep -cE '^ *pub struct
// <name>;'` test against the same dataflow_ods.rs, all 31 returning 1 (and `pub struct mlir_llvm_`
// returning 0, so there is no lowercase variant to chase -- the generator keys the marker off the
// cppNamespace `::mlir::LLVM`, not off the printed dialect name `llvm`).  ⛔ BUT A CONFIRMED DEF IS
// NOT A REASON TO WRITE A KEY.  Censusing the emitted corpus -- all 58 bucket-A `.rs` at
// fresh30/out, tree-wide `Cpp2RustUnmapped` control total 7,127 -- with
//   grep -ohE 'Cpp2RustUnmapped_[A-Za-z0-9_]*' *.rs | sort | uniq -c
// the ONLY survivors naming any of the four new dialects are:
//     18  Cpp2RustUnmapped_mlir_detail_SymbolOpInterfaceTrait_mlir_ktdf_arch_DeviceOp_   -> t400
//      3  Cpp2RustUnmapped_mlir_memref_MemorySpaceCastOp
//      3  Cpp2RustUnmapped_mlir_memref_CastOp
//      2  Cpp2RustUnmapped_mlir_memref_ReinterpretCastOp
//      2  Cpp2RustUnmapped_llvm_function_ref_std_unique_ptr_mlir_ktdf_arch_DeviceView___const_
//         mlir_ktdf_arch_Device_ref__
// ⛔ ALL SIXTEEN `mlir::LLVM` SPELLINGS AND ALL SIX `mlir::math` SPELLINGS HAVE **ZERO** SITES, and
// that is measured twice over: not one `Cpp2RustUnmapped_` name contains them, and the broader
// unanchored `grep -ohE '[A-Za-z0-9_]*LLVM[A-Za-z0-9_]*'` over the same 58 files returns exactly
// ONE token in total (`mlir_ktdf_arch_LinkDirection_LLVM_BITMASK_LARGEST_ENUMERATOR`, an enum
// sentinel, not an op) while `'[A-Za-z0-9_]*_math_[A-Za-z0-9_]*'` returns NOTHING AT ALL.
// ⚠️ SO THE CIRCULATED "45 occurrences for LLVM/math/memref" DOES NOT REPRODUCE against this
// corpus: the true figure is **8, all of them memref**.  `mlir::LLVM::UndefOp` and
// `mlir::math::AbsIOp` -- the two spellings named all day as the exemplars of the absent-DEF hazard
// -- are no longer absent, but they are also not USED here, so a key for either could not be
// gated: the only gate that distinguishes a landed key from a dead one is an anchored count on an
// emitted `.rs` moving, and a count of 0 cannot move.  Twenty-two keys whose effect is unmeasurable
// is precisely the shape of the twelve keys (t236-t242, setvector t1/t3-t6) that recorded cleanly,
// passed check-ir.sh in both models, passed the byte-diff and moved 3->3 with the emitted files
// byte-identical.  LEFT OUT DELIBERATELY, not overlooked; re-test by re-running the census above
// against a sweep that actually emits a `mlir::LLVM` or `mlir::math` site.
//
// ⛔ THE THREE `mlir::memref` SPELLINGS THAT DO HAVE SITES ARE ALSO LEFT OUT, ON A DIFFERENT AND
// SHARPER GROUND -- read the sites, not the count.  All 8 occurrences are in ONE file,
// `dataflow-scheduler__lib__Conversion__backend__ScheduleIRToDFIR__KTDFLowToDFIR__
// LogicalMemoryViewBuilder.cpp.rs`, and they are NOT cast-target-only:
//     15258  let mut mc: Cpp2RustUnmapped_mlir_memref_CastOp =
//     15259      <Cpp2RustUnmapped_mlir_memref_CastOp>::default();
//     15617  let mut mc: Cpp2RustUnmapped_mlir_memref_CastOp = (unsafe { dyn_cast_357(user) });
// i.e. every one of the three is DEFAULT-CONSTRUCTED as a local before being overwritten by a
// `dyn_cast`.  ⛔ A type key alone therefore reproduces the f42 / `MemRefType` failure EXACTLY as
// that key's own note states it: "a `using tN =` alone gives rc=0 and then E0433" -- the placeholder
// count would drop by 8 while the file still did not compile, which is the worst possible outcome
// because it buys a green census with no green rustc.  ⛔ AND THE OBVIOUS FIX IS A SEMANTIC LIE:
// `dataflowir_gen::fmt::OpInst` derives `Clone` ONLY (fmt.rs:390) and NOT `Default`, so an `fN`
// would have to return `OpInst::new(<DEF>)` -- a REAL op instance standing for a
// DEFAULT-CONSTRUCTED MLIR op wrapper, which in MLIR is a NULL handle (`state == nullptr`).  Every
// null test on such a local would then read as a live op.  These three need a null-handle model for
// the ODS op wrappers first; ⭐ THAT MODEL IS NOW WRITTEN -- see t420-t422 immediately below, which
// LAND these three.  The paragraph above is kept because its reasoning is still the reason the
// OBVIOUS fix (an `fN` returning `OpInst::new(DEF)`) is refused; what changed is that a null handle
// turns out to need NO `fN` AT ALL.  ⛔ And the `llvm::function_ref<std::unique_ptr<ktdf_arch::DeviceView>(const
// ktdf_arch::Device &)>` row is out on the ORIGINAL ground, undisturbed by ee457cc: `DeviceView`
// and `Device` are not ODS ops, so `grep -cE '^ *pub struct mlir_ktdf_arch_Device(View)?;'` is 0
// and that IS still an absent DEF.

// ---------------------------------------------------------------------------
// t420 / t421 / t422 -- A NULL-HANDLE MODEL FOR THE ODS OP WRAPPERS, applied to the three
// `mlir::memref` cast ops the paragraph above left out.  8 sites, ALL in one file
// (dataflow-scheduler/lib/Conversion/backend/ScheduleIRToDFIR/KTDFLowToDFIR/
// LogicalMemoryViewBuilder.cpp): MemorySpaceCastOp 3, CastOp 3, ReinterpretCastOp 2.  Counted
// `grep -o` on the EMITTED `.rs`, not on a queue `searched as:` line; the three names have no
// longer spelling in the corpus, and `CastOp` is NOT a substring problem here because
// `MemorySpaceCastOp` / `ReinterpretCastOp` are matched by their own full `grep -o` and the
// `Cpp2RustUnmapped_mlir_memref_CastOp` anchor carries the `memref_` prefix.
//
// ⭐ WHAT THE SITES ACTUALLY DO, read off the emitted unsafe `.rs` (lines 15229-15291), because
// this is what picks the representation and it is NOT what "cast target" would suggest:
//     let mut msc: <T> = <T>::default();                      // DEFAULT-CONSTRUCT  (the null)
//     msc = (unsafe { dyn_cast_344(user) });                  // overwrite
//     if (unsafe { (msc as fmt::OpInst).to_bool() }) break;   // operator bool
//     if !(unsafe { (msc as fmt::OpInst).to_bool() }) { ...emitError... }   // operator!
//     (unsafe { msc.getDest() }) as ir::Value                 // MEMBER, then
//     mc.getDest().replaceAllUsesWith(...); mc.erase();       // MUTATION THROUGH THE HANDLE
//
// ⭐ THE SHAPE, DECIDED ON THAT EVIDENCE: `libcc2rs::Ptr<fmt::OpInst>` / `*mut fmt::OpInst`, i.e.
// BIT-FOR-BIT t36's representation of `mlir::Operation *` (tgt_refcount.rs:493).  That is the
// right answer for a reason stronger than analogy: an ODS op wrapper IS an `Operation *` plus a
// static type assertion -- `OpState` holds exactly one `Operation *` member and every wrapper
// derives from it -- so the wrapper's representation is `Operation *`'s representation, and
// t243-t246 already model an UNPOSITIONED iterator the same way (`Ptr::null()` / `null_mut()`).
//
// ⛔ `Option<fmt::OpInst>` WAS CONSIDERED AND IS REJECTED, and the two `mc` sites are what reject
// it: `mc.erase()` and `sel.getResult().setType(...)` MUTATE THROUGH THE HANDLE, and they must be
// visible to the block that owns the op.  An `Option<OpInst>` is a BY-VALUE copy (fmt.rs:390
// derives `Clone`), so `erase()` would erase a copy and the block would keep the op -- the same
// class of semantic lie as `OpInst::new(DEF)`, just quieter.  A non-owning handle INTO the Vec the
// block already is does not have that failure mode.  ⭐ And `Ptr::borrow_vec` / `Ptr::with_ref` /
// `with_mut_ref` (libcc2rs 34e592b3 / b8a5953b) make this shape usable for a NON-POD payload:
// there is no `T: ByteRepr` bound on the deref path, which matters because `fmt::OpInst` does not
// implement it.  Liveness, not lifetimes: an access outliving the owner PANICS
// (`weak.upgrade().expect("ub: dangling pointer")`) rather than reading freed memory.
//
// ⭐⭐ AND THE REASON THIS NEEDS **NO `fN`** -- the single fact that makes the row landable, and the
// one the f42 note could not have known.  The emitted default-construct is
// `<T>::default()`, NOT `<T>::new()` (readback above, and `grep -c '_memref_[A-Za-z]*Op>::new('`
// on the emitted file is 0), and BOTH targets implement `Default` WITH THE NULL AS THE DEFAULT:
//   * `impl<T> Default for Ptr<T>` (libcc2rs rc.rs:148) builds `offset: 0, kind: Default::default()`,
//     and `PtrKind::Null` carries `#[default]` (rc.rs:26-28) -- so `Ptr::default() == Ptr::null()`
//     EXACTLY, and the impl has NO `T` bound at all.
//   * `*mut T` implements `Default` as `null_mut()` in std -- VERIFIED by compiling
//     `let p: *mut Foo = <*mut Foo>::default(); p.is_null()`, rc=0, prints `true`.
// ⛔ THAT IS THE WHOLE DIFFERENCE FROM f42 / f137 / f138.  Those three needed an `fN` because their
// targets are `ir::Ty` / `ir::Attr`, ENUMS WITH NO `Default`, so `::default()` did not resolve and a
// body had to be supplied.  A nullable handle gets its honest default FOR FREE, which is why the
// null-handle model dissolves the rc=0-then-E0433 trap instead of merely relocating it.
//
// ⚠️ WHAT THIS DOES **NOT** BUY, stated plainly because the row was set to test rustc and not the
// census: THE WITNESS FILE STILL DOES NOT COMPILE, and it never could have from these three keys.
// Measured on the BEFORE emission, all three defects PRE-EXIST and none is introduced or removed
// by t420-t422:
//   1. `mlir::dyn_cast` IS ITSELF UNMAPPED.  The file contains **101 calls** to fabricated
//      `dyn_cast_<N>` names and **ZERO** `fn dyn_cast` definitions -- every one an E0425.  That is
//      the SETTLED absence already recorded at src.cpp:948-949 ("NO rules module anywhere keys a
//      `dyn_cast` free template function"), so it is not this row's to close.
//   2. `(msc as fmt::OpInst)` is a NON-PRIMITIVE CAST (E0605).  It is how the converter spells the
//      upcast to the `OpState` base, and it is emitted for MAPPED ODS ops in the same file too
//      (`(cmv as fmt::OpInst).emitError(...)`, where `cmv` is the generated
//      `mlir_ktdp_ConstructMemoryViewOp`), so it is converter-side and TYPE-INDEPENDENT.
//   3. `msc.getDest()` / `rc.getResult()` are UNMAPPED MEMBERS, emitted TEXTUALLY at rc=0 with no
//      placeholder token (the class recorded at rules/mlir t63).  Deliberately NOT guessed here --
//      the t166 / t243-t246 discipline: type keys only, members stay loud.
// ⭐ SO `dyn_cast` CORRECTLY FAILING ON THE NULL IS TRUE BY REPRESENTATION BUT NOT YET OBSERVABLE:
// `Ptr::null()`/`null_mut()` is the value any honest `dyn_cast` miss would return and `is_null()`
// answers it, but no `dyn_cast` rule exists to exercise it.  What the keys DO buy is that the
// default-construct is now an HONEST NULL rather than an undeclared placeholder -- and, decisively,
// that the three sites can never be "fixed" later by a live-op sentinel, which is the outcome the
// paragraph above was written to prevent.
//
// SWALLOW-SAFETY: all three keys are FULLY CONCRETE, placeholder arity 0, so `matchTemplate`'s
// same-depth capture (mapper.cpp:173) never runs.  `GetTypeMapKey` truncates at the first `<` and
// none of the three has one, so each occupies its own bucket; no defaulted template argument is
// spelled, so `SuppressDefaultTemplateArgs` cannot produce a dead duplicate.
namespace mlir {
namespace memref {
// The three ODS-generated `mlir::memref` cast ops, declared ONLY so t420-t422 can be SPELLED --
// the t161 / t400 reason and nothing more.  No member of any of them is mapped.  ⚠️ `namespace
// memref` is opened here for the first time in this file; none of these three is declared
// elsewhere in it.
class CastOp {};
class MemorySpaceCastOp {};
class ReinterpretCastOp {};
} // namespace memref
} // namespace mlir
// t420 -- Cpp2RustUnmapped_mlir_memref_MemorySpaceCastOp, 3 sites / 1 file.
using t420 = mlir::memref::MemorySpaceCastOp;
// t421 -- Cpp2RustUnmapped_mlir_memref_CastOp, 3 sites / 1 file.
using t421 = mlir::memref::CastOp;
// t422 -- Cpp2RustUnmapped_mlir_memref_ReinterpretCastOp, 2 sites / 1 file.
using t422 = mlir::memref::ReinterpretCastOp;

// ---------------------------------------------------------------------------
// t236-t242 -- `llvm::SmallSet` and `llvm::detail::DenseSetImpl`, the two
// set-shaped system types the recorder attributes to this module.  SEVEN keys,
// a DELIBERATE SUBSET of the eighteen the queue lists; the nine unlanded
// spellings and the exact reason are enumerated at the bottom of this block.
//
// ⛔ MONOMORPHIC, NEVER GENERIC, for the reason already recorded at :3650 and
// :4258 and MEASURED as `rules/setvector`'s `t1`: in a key like
// `DenseSetImpl<T1, T2, T3>` the next same-depth literal after `T1` is `, `,
// and a COMMA IS NOT A DELIMITER, so `T1` captures the whole argument list and
// `T2`/`T3` bind nothing.  Written fully concrete, placeholder arity is 0, so
// `matchTemplate`'s capture (`findNextLiteralSameDepth`, mapper.cpp:173) never
// runs and the swallow is ruled out BY CONSTRUCTION.  Same discipline as
// t37-t39 / t166 / t167-t216.
//
// ⚠️ THE `_` IN THE RECORDED KEY IS THE CONVERTER'S, NOT MINE.
// `normalizeTranslationRule` (cpp2rust/converter/mapper.cpp:1830-1838) rewrites
// `\b\d+\b` -> `_` on BOTH the key and the search side, so the literal `4`
// written below is recorded as `_` and one key covers every inline capacity.
// For these two templates that erasure is HARMLESS: `N` is `SmallSet`'s inline
// capacity (SmallSet.h:133) and `InlineBuckets` is `SmallDenseMap`'s -- neither
// is semantic.  ⛔ BUT A CONVERTER SLOT IS REMOVING THAT ERASURE RIGHT NOW for
// `chrono::duration`.  IF THAT LANDS, THESE KEYS STILL WORK (they spell a real
// digit, not a literal `_`) BUT THEY STOP COLLAPSING: `SmallSet<long, 4>` and
// `SmallSet<long, 8>` would then need one key each.  Do not "fix" these by
// writing `_` in the C++ -- a literal `_` key goes dead the moment the erasure
// is removed.
//
// MODEL: `std::collections::HashSet<T>`, exactly `rules/densemap`'s t2.  Order
// is genuinely unspecified for BOTH of these (contrast `SetVector`, whose whole
// point is insertion order), so a HashSet claims nothing the C++ does not.
//
// ⭐ NO MEMBER RULES AT ALL, and that is not an omission.  Not one of the
// twelve `DenseSetImpl` queue rows is a member-call row -- all twelve are
// "system type has no rule".  The type surfaces because the recorder emits the
// DECLARING class: the corpus declares `DenseSet`/`SmallDenseSet` and
// `insert`/`contains` are INHERITED from this CRTP base.  Same shape as t166.
//
// ⛔ `rules/densemap` OWNS `llvm::DenseSet` -- a DIFFERENT bucket (`GetTypeMapKey`
// truncates at the first `<`, so `llvm::DenseSet` and `llvm::detail::DenseSetImpl`
// never share a bucket).  Nothing here touches it.
//
// THE LOCAL DECLARATIONS BELOW, and why their DEFAULTS ARE WRITTEN THE WAY THEY
// ARE -- this is the part a blind copy of the readback gets wrong.  Only
// TRAILING defaulted arguments are suppressed when a key is printed, so:
//   * `DenseSetImpl`'s third parameter is declared WITHOUT a default.  The real
//     LLVM header defaults it to `DenseMapInfo<ValueT>`, which is exactly what
//     every one of these keys spells -- so with the default declared it would
//     be suppressed and the key would be recorded at ARITY 2, not matching the
//     arity-3 `searched as:` text.  Same reasoning for `DenseMap` /
//     `SmallDenseMap`, whose final `Bucket` argument is non-default here
//     (`DenseSetPair<K>`, not `DenseMapPair<K,V>`) and so forces every earlier
//     argument to print anyway.
//   * `DenseMapInfo`'s SFINAE parameter IS defaulted to `void`, because the
//     three element types landed here (`long`, `unsigned int`, `unsigned long`)
//     are recorded as `llvm::DenseMapInfo<long>` -- WITHOUT the `, void`.  The
//     two `llvm::StringRef` rows are recorded WITH it and are among the nine
//     left out below.
namespace llvm {
template <typename T, unsigned N> class SmallSet {};
template <typename T, typename Enable = void> struct DenseMapInfo {};
template <typename KeyT, typename ValueT, typename KeyInfoT, typename BucketT>
class DenseMap {};
template <typename KeyT, typename ValueT, unsigned InlineBuckets,
          typename KeyInfoT, typename BucketT>
class SmallDenseMap {};
namespace detail {
struct DenseSetEmpty {};
template <typename KeyT> struct DenseSetPair {};
template <typename ValueT, typename MapTy, typename ValueInfoT>
class DenseSetImpl {};
}  // namespace detail
}  // namespace llvm

// t236 -- `llvm::SmallSet<long, _>`, FOUR queue rows: g120, g121, g122, g1171.
// The search arity is 2 and the `_` is the erased `N`, so this one key covers
// every inline capacity the corpus instantiates.
using t236 = llvm::SmallSet<long, 4>;

// t237 -- `llvm::SmallSet<unsigned long, _>`, queue row g1174.
using t237 = llvm::SmallSet<unsigned long, 4>;

// t238 -- queue row g323 (3 TUs), `llvm::DenseSet<long>`'s CRTP base.
using t238 = llvm::detail::DenseSetImpl<
    long,
    llvm::DenseMap<long, llvm::detail::DenseSetEmpty, llvm::DenseMapInfo<long>,
                   llvm::detail::DenseSetPair<long>>,
    llvm::DenseMapInfo<long>>;

// t239 -- queue row g1355 (1 TU), `llvm::SmallDenseSet<long, N>`'s CRTP base.
// Distinguished from t238 by the `SmallDenseMap` MapTy and its extra
// `InlineBuckets` argument, so a literal match selects exactly one.
using t239 = llvm::detail::DenseSetImpl<
    long,
    llvm::SmallDenseMap<long, llvm::detail::DenseSetEmpty, 4,
                        llvm::DenseMapInfo<long>,
                        llvm::detail::DenseSetPair<long>>,
    llvm::DenseMapInfo<long>>;

// t240 -- queue row g1356 (1 TU).
using t240 = llvm::detail::DenseSetImpl<
    unsigned int,
    llvm::DenseMap<unsigned int, llvm::detail::DenseSetEmpty,
                   llvm::DenseMapInfo<unsigned int>,
                   llvm::detail::DenseSetPair<unsigned int>>,
    llvm::DenseMapInfo<unsigned int>>;

// t241 -- queue row g1357 (1 TU).
using t241 = llvm::detail::DenseSetImpl<
    unsigned int,
    llvm::SmallDenseMap<unsigned int, llvm::detail::DenseSetEmpty, 4,
                        llvm::DenseMapInfo<unsigned int>,
                        llvm::detail::DenseSetPair<unsigned int>>,
    llvm::DenseMapInfo<unsigned int>>;

// t242 -- queue row g1358 (1 TU).
using t242 = llvm::detail::DenseSetImpl<
    unsigned long,
    llvm::DenseMap<unsigned long, llvm::detail::DenseSetEmpty,
                   llvm::DenseMapInfo<unsigned long>,
                   llvm::detail::DenseSetPair<unsigned long>>,
    llvm::DenseMapInfo<unsigned long>>;

// ⛔⛔ THE NINE SPELLINGS DELIBERATELY NOT KEYED, AND THE ONE REASON.
// A MONOMORPHIC set key has to name a CONCRETE Rust ELEMENT type, and for these
// nine the element's Rust model is NOT owned by this module and NOT derivable
// from anything in it.  A key naming the wrong element type records cleanly,
// passes the load smoke test, and is then a SILENT LIE about set membership --
// strictly worse than the loud abort that is there now.  Enumerated:
//   llvm::SmallSet<SentientRegType, _>                  g1166 g1167 g1168
//   llvm::SmallSet<const EvaluatedValue *, _>           g1169 g1170
//     -- CORPUS types, not system types.  `SentientRegType` is named only in
//        rules/equivalenceclasses' and rules/densemap' PROSE; neither maps it,
//        and nothing in the tree says what `EvaluatedValue` becomes.
//   llvm::SmallSet<mlir::Operation *, _>                g1172 g1173
//     -- `mlir::Operation *`.  t1 maps the CLASS to `fmt::OpInst`, a VALUE
//        type; what a POINTER to it is under the refcount model (and whether
//        that spelling is `Hash + Eq`, which `HashSet` REQUIRES) is not settled
//        anywhere in this module.  Also the only row that additionally hits the
//        `SmallSet<PointeeType *, N> : SmallPtrSet` partial specialisation
//        (SmallSet.h:273) -- the search key is unchanged by that, so it is the
//        element model alone that blocks it.
//   llvm::SmallSet<std::pair<mlir::Operation *, std::optional<int>>, _>
//                                                       g128 g129
//     -- same `mlir::Operation *` blocker, inside a pair.
//   DenseSetImpl over `llvm::StringRef`                 g415 g1354
//     -- element owned by rules/stringref, and these two are ALSO the rows
//        whose `DenseMapInfo` is recorded WITH the `, void`, so they need the
//        non-defaulted declaration the seven above must not have.
//   DenseSetImpl over `mlir::Attribute`                 g416 g417
//   DenseSetImpl over `mlir::StringAttr`                g156
//   DenseSetImpl over `mlir::Value`                     g168
//   DenseSetImpl over `mlir::Operation *`               g202
//     -- t4/t6 and t1 give these types a REPRESENTATION, but none of
//        `ir::Attr` / `ir::Value` / `fmt::OpInst` is known here to implement
//        `Hash + Eq`, and `std::collections::HashSet` does not compile without
//        both.  That check is a `dataflowir-gen` read, not a rules read, and it
//        is the next step for these five.

// ---------------------------------------------------------------------------
// llvm::ilist_iterator over mlir::Operation and mlir::Block -- t243..t246.
//
// THE ROWS.  34 queue rows (g244, g324, g1433-g1464) are all TYPE rows and all
// four spellings below; taken untruncated from queue/samples/, "searched as"
// and "from decl" are IDENTICAL for every one, so there is no dead-duplicate.
// All four are fully concrete (arity 0, no T<digits>), so matchTemplate's
// placeholder capture never runs and a swallow is ruled out by construction.
//
// THE DECLARATIONS ARE LOCAL, and NO TEMPLATE PARAMETER CARRIES A DEFAULT.
// Reaching real llvm/ADT/ilist_iterator.h would need an absolute -I into the
// target's LLVM tree (the reason rules/raw_ostream, rules/stringref and
// rules/ilist all give at length).  The parameter KINDS are copied verbatim
// from LLVM-22.1.3's headers so the printer renders the same spelling:
//   ilist_node_options.h:156  template <class T, bool, bool, class TagT, bool,
//                                       class ParentTy> struct node_options
//   ilist_iterator.h:80       template <class OptionsT, bool IsReverse,
//                                       bool IsConst> class ilist_iterator
// Defaults are deliberately omitted: the samples show the printer emitting all
// three ilist_iterator arguments and all six node_options arguments, so a
// defaulted-and-elided parameter would produce a spelling that never matches.
// `IsReverse` is the rbegin/rend flag -- true and false are DISTINCT keys, not
// one key with an elided default.
//
// MODEL: a Ptr<OpInst> / Ptr<Block> INTO the Vec the container already is, i.e.
// exactly rules/vector's and rules/list_iterator's iterator representation.
// THE ERASE-DURING-WALK QUESTION IS SETTLED IN FAVOUR OF THIS: every mutating
// pass copies the ops into a SmallVector BEFORE mutating, and
// EnhancedDeadVariableElimination.cpp:441-445 says why in a comment ("some of
// the operations are going to be deleted and operating directly over
// getOperations results in segfaults"); LightweightSimplification.cpp:269-310
// is the same shape -- its rbegin/rend ilist walk only does
// ops_list.push_back(&*I) and every erase() happens in a LATER loop over
// ops_list.  That check was sampled (5 of ~8 TUs), not exhaustive: treat it as
// strong but not closed.  (LexicalOrdering.cpp:186's `(*I++)->moveBefore(...)`
// is NOT a counter-witness -- that I is a SmallVector<Operation*> reverse
// iterator, not an ilist_iterator.)
//
// NO MEMBER IS KEYED HERE, matching the rest of this module: all 34 rows are
// type rows, and a member reached on one of these iterators must keep ABORTING
// LOUDLY rather than get a body guessed against an unverified model.
namespace llvm {
namespace ilist_detail {
template <class T, bool EnableSentinelTracking, bool IsSentinelTrackingExplicit,
          class TagT, bool HasIteratorBits, class ParentTy>
struct node_options;
} // namespace ilist_detail
// ⚠️ DEFINED (empty) rather than forward-declared ONLY so that f461 can RETURN
// one BY VALUE: a function definition needs a COMPLETE return type, and
// `simple_ilist<mlir::Block>::begin()` returns the iterator by value.  The
// printed SPELLING is unchanged, so t243-t246 match exactly as before -- an empty
// body adds no member and maps nothing.
template <class OptionsT, bool IsReverse, bool IsConst> class ilist_iterator {};
} // namespace llvm

// t243 -- Cpp2RustUnmapped_llvm_ilist_iterator_..._mlir_Operation_..._false_false_, 24 rows.
using t243 = llvm::ilist_iterator<
    llvm::ilist_detail::node_options<mlir::Operation, false, false, void, false,
                                     void>,
    false, false>;
// t244 -- same, IsReverse=true (rbegin/rend), 5 rows.
using t244 = llvm::ilist_iterator<
    llvm::ilist_detail::node_options<mlir::Operation, false, false, void, false,
                                     void>,
    true, false>;
// t245 -- mlir::Block, IsReverse=false, 4 rows.
using t245 = llvm::ilist_iterator<
    llvm::ilist_detail::node_options<mlir::Block, false, false, void, false,
                                     void>,
    false, false>;
// t246 -- mlir::Block, IsReverse=true, 1 row.
using t246 = llvm::ilist_iterator<
    llvm::ilist_detail::node_options<mlir::Block, false, false, void, false,
                                     void>,
    true, false>;

// ---- t250 / t251: `mlir::TypeRange` and its CRTP base ----------------------
// Queue row g111, and the BIGGEST rules row on the board by ground-truth emitted
// counts (GROUND-TRUTH-UNMAPPED.txt lines 3 and 10, re-measured off emitted `.rs`,
// not off a queue `searched as:` line):
//     Cpp2RustUnmapped_mlir_TypeRange                             12045 sites / 38 files
//     Cpp2RustUnmapped_llvm_detail_indexed_accessor_range_base_mlir_TypeRange__llvm_PointerUnion_const_mlir_Value_ptr__const_mlir_Type_ptr__mlir_OpOperand_ptr__mlir_detail_OpResultImpl_ptr___mlir_Type__mlir_Type__mlir_Type_
//                                                                  2098 sites / 16 files
// ⭐ THE TYPE KEY COMES FIRST: `mlir::TypeRange` was UNMAPPED ANYWHERE in this
// module (grep before this edit found it only in PROSE at :3519/:3935/:3970-3978),
// which is why the short name carries six times the sites of the base name.
//
// SPELLING.  Read off mlir/IR/TypeRange.h:33-37 in the pinned toolchain
// (toolchain/llvm/LLVM-22.1.3-Linux-X64/include/mlir/IR/TypeRange.h), and it agrees
// ARGUMENT-FOR-ARGUMENT with the emitted mangled name above:
//     class TypeRange : public llvm::detail::indexed_accessor_range_base<
//                           TypeRange,
//                           llvm::PointerUnion<const Value *, const Type *,
//                                              OpOperand *, detail::OpResultImpl *>,
//                           Type, Type, Type>
// Three measured differences from t166 (`mlir::ValueRange`'s base, :3980): a
// different DerivedT, a FOUR-arm PointerUnion (t166's has three -- no
// `const mlir::Type *`), and an element type of `mlir::Type` rather than
// `mlir::Value`.  So the model is a Vec of `ir::Ty` (t5), NOT `ir::Value` -- which is
// exactly why t166's paragraph refused to share one key across both rows.
//
// MODEL.  `Vec<dataflowir_gen::ir::Ty>`, the elementwise lift of t5
// (`mlir::Type` -> `ir::Ty`) the same way t14/t16 are the elementwise lift of t4.
// Nothing new is represented; both keys get the SAME body, because the base IS the
// range (the t37-t39 / t166 discipline).  Returned BY VALUE, so nothing can dangle
// under the refcount model.
//
// ⭐ THE ALIASING LICENCE, RE-DERIVED FOR `TypeRange` RATHER THAN INHERITED.  The
// t14-t17 owning-`Vec` position holds only for a READ-ONLY view, and the
// `mlir::MutableOperandRange` refusal at :1311 is what happens when it does not.
// TypeRange.h:38-71 is the whole public surface and it declares NO MEMBER FUNCTION
// AT ALL -- only constructors (`using RangeBaseT::RangeBaseT;` plus the seven
// overloads at :40-53).  Its only other members are PRIVATE statics
// (`offset_base`, `dereference_iterator` at :65/:67) plus a `friend RangeBaseT`,
// and the inherited surface is `indexed_accessor_range_base`, already settled
// read-only for t37-t39/t166.  There is NO `assign`/`append`/`erase`, no
// non-const accessor, and no reference-returning accessor -- so unlike
// MutableOperandRange nothing writes through the handle, and unlike
// `std::string_view::front()` / `llvm::SMLoc::getPointer()` nothing hands a
// borrow out of a receiver this model owns by value.  An owning `Vec<ir::Ty>` copy
// therefore loses only UNOBSERVABLE aliasing.  (⚠️ `ValueTypeRange` at
// TypeRange.h:120-165 DOES declare `front()`, but that is a DIFFERENT class and is
// not keyed here.)
//
// ⛔ STILL NO GENERIC RULE, and the t37-t39/t166 prohibition is unchanged: a
// generic `indexed_accessor_range_base<T1,...,T5>` would have to pick one element
// representation for five different element types and would force the converter to
// map the `PointerUnion` arms, turning a countable mangled name into a hard
// mapper.cpp:722 abort.
//
// SWALLOW-SAFETY, argued for the bucket and not assumed.  `GetTypeMapKey`
// (mapper.cpp:115) truncates at the first `<`, so t251 lands in the bucket
// `llvm::detail::indexed_accessor_range_base`, shared with t37, t38, t39, t166 and
// ONLY those.  ⭐ ALL FIVE ARE FULLY CONCRETE -- PLACEHOLDER ARITY 0.  There is no
// `T<digits>` anywhere in any of the five spellings, so `matchTemplate`'s
// placeholder capture (`findNextLiteralSameDepth`, :173) -- the mechanism behind the
// `DenseMapInfo<T1>` and `__wrap_iter<T1 *>` swallows -- NEVER RUNS on this bucket.
// That is precisely why the nested comma-bearing four-arm `PointerUnion<...>` is
// harmless: with no placeholder in front of them, its commas are matched
// LITERALLY, not captured past.  The five candidates are pairwise distinguished by
// their FIRST template argument (OperandRange / ResultRange / RegionRange /
// ValueRange / TypeRange), so a literal match selects exactly one and `search()`'s
// longer-src tie-break (mapper.cpp:430-437) is never consulted -- which matters,
// because that tie-break CANNOT protect a sole candidate.  t250 is a bare
// non-template name, so it buckets as `mlir::TypeRange` alone and the same
// argument is trivial there.
// ⛔ NO DEFAULTED TEMPLATE ARGUMENT IS SPELLED: `indexed_accessor_range_base` has
// five parameters and all five are written, so `SuppressDefaultTemplateArgs`
// cannot collapse this into a silent duplicate of a shorter key (the
// `DenseMapInfo<T1, void>` failure).
//
// ⛔ NO CONSTRUCTOR KEY IS WRITTEN HERE, DELIBERATELY, and `class TypeRange` below
// is declared with NO MEMBERS so that none is recorded.  TypeRange's seven ctor
// overloads (:40-53) are five templates plus two whose spellings the recorder
// resolves at the call site; writing them blind is the f143 failure mode (written,
// measured unchanged, deleted).  The TYPE key is 12,045 of the 14,143 sites and is
// measurable on its own; the ctors are a separate, separately-measured row.
namespace mlir {
// mlir/IR/TypeRange.h:33 -- declared ONLY so t250 and t251 can be spelled.  No
// member is declared because no member is mapped (the `sys::SmartMutex` /
// `BitVector` precedent at :1229-1245).
class TypeRange {};
} // namespace mlir

using t250 = mlir::TypeRange;
using t251 = llvm::detail::indexed_accessor_range_base<
    mlir::TypeRange,
    llvm::PointerUnion<const mlir::Value *, const mlir::Type *,
                       mlir::OpOperand *, mlir::detail::OpResultImpl *>,
    mlir::Type, mlir::Type, mlir::Type>;

// ---- merged from wt/mlirF: t260/t261 ----
// The two forward declarations MUST be inside `namespace mlir` -- appending them at global
// scope is what broke the first merge attempt of this block (`no type named ... in namespace
// mlir`), because the worktree had them inside the namespace and a naive EOF append does not
// carry namespace scoping with it.
namespace mlir {
// `mlir::SelfOwningTypeID` -- mlir/Support/TypeID.h, the SEPARATE class t72's
// comment already names ("`TypeIDAllocator`/`SelfOwningTypeID` are separate
// classes").  Declared next to TypeID because it is the same RTTI-token family.
// ⛔ NO MEMBER IS DECLARED, and that absence is the whole enforcement: `getTypeID()`
// and `operator TypeID()` are deliberately absent so anything that tries to
// OBSERVE the identity still aborts loudly instead of getting a silent lie.
// See `using t260 =` for the identity argument and the emitted evidence.
class SelfOwningTypeID {};

// `mlir::NamedAttrList` -- mlir/IR/Attributes.h, an ordered list of
// NamedAttribute with a sort-on-demand `getDictionary()`.  Sibling of
// `NamedAttribute` (declared above, t21).  NO MEMBER IS DECLARED: every emitted
// site is TYPE POSITION only (see `using t261 =`), so `append`/`set`/`get`/
// `getDictionary` stay unmapped and still abort.
class NamedAttrList {};

} // namespace mlir

// ---------------------------------------------------------------------------
// t260-t261 -- two ARITY-0 FULLY-CONCRETE keys (no `<` at all, so GetTypeMapKey's
// truncate-at-first-`<` and matchTemplate's same-depth capture cannot swallow
// anything: the t166 / t167-t246 proven-safe form).
//
// ⭐ BOTH KEYS ARE TAKEN FROM THE EMITTED SPELLING, NOT FROM A QUEUE ROW.  The
// ground-truth extraction over emitted `.rs` gives exactly
// `Cpp2RustUnmapped_mlir_SelfOwningTypeID` (966 sites / 104 files) and
// `Cpp2RustUnmapped_mlir_NamedAttrList` (2,033 / 16); both are bare, undecorated,
// arity-0 names, so the `SmallSet`/`SetVector` dead-key failure (queue text that
// the corpus never emits) cannot recur here.
//
// ⛔ REFUSED IN THIS BLOCK: `mlir::OwningOpRef<mlir::ModuleOp>` (972 / 79) and
// `mlir::OwningOpRef<mlir::Operation *>` (3 / 3).  The refusal is NOT new -- it is
// the settled one this file already cites twice (at the InFlightDiagnostic block,
// "a destructor with an observable effect is the axis on which `mlir::OwningOpRef`
// was REFUSED", and again at the six-handles block, "none of them is the
// `mlir::OwningOpRef` / `mlir::InFlightDiagnostic` case where a destructor with
// observable effect FORBIDS an opaque model").  OwningOpRef's destructor calls
// `op->erase()`, which UNLINKS the op from its parent block -- observable IR
// mutation, exactly the unique_ptr criterion.
// ⭐ THE OBSERVER, NAMED, from the goal TU's own emitted output
// (`dmg-AFTER/dxp__dxp_standalone.cpp.rs`): the pair
//     pub unsafe fn getModule(&mut self) -> *mut ..._OwningOpRef_mlir_ModuleOp_
//     pub unsafe fn get_module(&mut self) -> *mut ..._OwningOpRef_mlir_ModuleOp_
// (13 occurrences each) hands out a RAW POINTER INTO the owning field `module_`
// (13 occurrences), and a sibling struct holds `sdscBundleModuleOp` initialised
// by `<...>::default()`.  Every caller of `getModule()`/`get_module()` is the
// observer: model the owner as a plain handle/copy and the erase runs once per
// copy (double-erase through a pointer the callers still hold); model it as a
// unit like t61 and the erase never runs at all (leak, and the module outlives
// the pass that owned it).  Neither is representable while t61 already maps
// `mlir::ModuleOp` itself to `()` -- there is no Rust-side op to erase, so the
// ownership semantics have nothing to attach to.  This is a dataflowir-gen row
// (it needs real op/block surface), not a rules row.
// ⛔ ALSO NOT KEYED: `Cpp2RustUnmapped_mlir_NamedAttrList__class_mlir_StringAttr`
// (3 sites / 1 file).  A decorated spelling whose written form carries a template
// argument, so it is exposed to the recorder's DEFAULTED-TRAILING-ARGUMENT
// collapse: writing it verbatim risks collapsing onto the bare `NamedAttrList`
// key above and silently duplicating it.  3 sites is not worth that risk; left
// fabricating and loud.
// ---------------------------------------------------------------------------

// t260 -- `mlir::SelfOwningTypeID` -> AN OPAQUE UNIT.  966 sites / 104 files, the
// WIDEST file spread in this slot's set.  The model and the bargain are t72's
// (`mlir::TypeID` -> `()`), and the identity question the wide spread raises is
// SETTLED BY THE EMITTED OUTPUT rather than by analogy.
//
// ⭐ THE IDENTITY CHECK, MEASURED.  Across the whole `cens0928c/out` corpus the
// name occurs in EXACTLY TWO syntactic shapes and nothing else:
//     32x  static mut id_N: std::cell::LazyCell<Cpp2RustUnmapped_mlir_SelfOwningTypeID> =
//     32x  unsafe { std::mem::zeroed::<Cpp2RustUnmapped_mlir_SelfOwningTypeID>() }
// i.e. a static declaration and its zero-initialiser.  There is NO comparison, no
// `getTypeID()`, no member read, no pass-by-value -- so this is the CAST/TYPE-
// POSITION-ONLY shape that needs a NAME and no member rules at all.  `()` is
// `std::mem::zeroed`-able and `LazyCell<()>` is well-formed, so both shapes
// typecheck.
// ⛔ WHAT IS LOST IS IDENTITY, the same loss t72 takes and for the same reason:
// the crate has no RTTI concept.  The brief's hazard -- "if the corpus compares
// TypeIDs across TUs, a per-TU-fresh value is a silent lie" -- does not bite,
// because the corpus compares them NOWHERE, and `operator==`/`getTypeID()` are
// DELIBERATELY UNDECLARED above, so if a comparison ever appears it aborts loudly
// instead of answering `true` from nothing.  A per-TU-fresh value would be the
// WORSE choice here for exactly that reason: it would make comparisons compile.
using t260 = mlir::SelfOwningTypeID;

// t261 -- `mlir::NamedAttrList` -> `dataflowir_gen::ir::AttrDict` (ir.rs:559).
// 2,033 sites / 16 files.  This is the ENTRY-TYPE completion of t21: t21 maps ONE
// `mlir::NamedAttribute` to `(String, Attr)` "the ENTRY TYPE of ir::AttrDict", so
// the LIST of them maps to the dictionary itself.  Nothing is invented.
//
// ⭐ THE ORDER HAZARD IS SETTLED THE OPPOSITE WAY FROM `SetVector`, and the crate
// says so in its own words.  ir.rs:552-558, verbatim:
//     /// A `BTreeMap` is not a convenience: MLIR sorts a dictionary by key on
//     /// construction and the printer walks it in that order, so insertion order
//     /// is not what lands in the file. Using an insertion-ordered map here
//     /// produces IR that differs from the reference on every `get_unit`.
// So for THIS family insertion order is NOT the observable -- the SORTED order is,
// which is precisely `NamedAttrList`'s sort-on-demand `getDictionary()` contract.
// `rules/setvector` models `SetVector` as `Vec<T1>` *because* insertion order is
// observable there; here the reverse fact is documented and load-bearing, so a
// `Vec` model would be the wrong one: it would print dictionaries in insertion
// order and differ from the reference on every op.  De-duplication also agrees:
// MLIR's `NamedAttrList::set(name, v)` REPLACES an existing entry, which is
// `BTreeMap::insert`.
// ⭐ EVERY EMITTED SITE IS TYPE POSITION, so no member rule is needed and none is
// given.  Across `cens0928c/out` + `dmg-AFTER` the name occurs in exactly two
// shapes:  52x  `attrs: *mut Cpp2RustUnmapped_mlir_NamedAttrList,`  (a parameter)
// and      24x  `let _attrs: *mut Cpp2RustUnmapped_mlir_NamedAttrList =
//                    &mut (*result).attributes;`
// -- a declaration and the address of a field.  The 52 parameter sites become
// well-typed `*mut ir::AttrDict` immediately.
// ⚠️ HONEST LIMIT ON THE 24: `(*result).attributes` is a field the converter
// assumes on its OperationState-alike, and a grep for a `pub attributes:` field
// over dataflowir-gen finds NONE, so those 24 lines do not compile either way --
// this key does not fix them and does not make them worse (the fabricated type
// does not exist at all today).  It is named as a limit, not claimed as a win.
// ⛔ NO DEF is involved: the model is a crate `type` alias, not a generated op
// DEF, so the missing-DEF wave that hit mlir::LLVM/ktdf/linalg cannot apply.
using t261 = mlir::NamedAttrList;


// ---------------------------------------------------------------------------
// f150-f157: EIGHT MORE DEDUCTIONS OF THE SAME `InFlightDiagnostic` `<<` MEMBER
// TEMPLATE (Diagnostics.h:344-349) THAT f21-f38 DO NOT COVER.
//
// PROVENANCE, NOT GUESSWORK.  Each key text below is copied character-for-
// character out of the PIN's OWN reporter lines in the fresh28 + fresh29 sweeps
// (`unsupported CXXOperatorCallExpr: << on (mlir::InFlightDiagnostic, ...) rule
// key: <text>`).  A reporter line for a key IS the mapper's verdict that it
// searched and found nothing, so these eight are proven ABSENT by the converter
// itself rather than by a readback.  Census over both sweeps:
//     6  mlir::InFlightDiagnostic && operator shl(llvm::Twine &&) &&           f150
//     2  mlir::InFlightDiagnostic && operator shl(const llvm::Twine &) &&      f151
//     4  mlir::InFlightDiagnostic && operator shl(unsigned long &) &&          f152
//     4  mlir::InFlightDiagnostic && operator shl(mlir::Type &&) &&            f153
//     2  mlir::InFlightDiagnostic && operator shl(long &&) &&                  f154
//     2  mlir::InFlightDiagnostic && operator shl(int &&) &&                   f155
//     2  mlir::InFlightDiagnostic && operator shl(const unsigned long &) &&    f156
//     2  mlir::InFlightDiagnostic && operator shl(const llvm::StringRef &) &&  f157
//
// DIFFED AGAINST f21-f38 BEFORE WRITING, param-list by param-list.  The existing
// eighteen are: `const char (&)[_]`, `StringRef&&`, `StringRef&`, `std::string&`,
// `mlir::Type&`, `unsigned int&`, `int&`, `long&`, `const long&`, `const int&`,
// `const unsigned int&`, `unsigned int&&`, `unsigned long&&`, `const std::string&`,
// `std::string&&`, `mlir::Attribute&`, `mlir::StringAttr&&`, `StringLiteral&&`.
// So every pairing below is the MISSING half of a reference-kind pair the family
// already has one side of: f152/f156 against f33's `unsigned long&&`, f153
// against f25's `mlir::Type&`, f154 against f28's `long&`, f155 against f27's
// `int&`, f157 against f22/f23's two StringRef kinds.  NONE duplicates an
// existing key (`Arg&&` is forwarding, so `T&` and `T` are different deductions
// and different keys -- the f22-f38 header block states this).
//
// ⚠️ A PARAMETER DECLARED BY VALUE IS AN LVALUE INSIDE THE BODY.  To deduce
// `Arg = T` (the `T &&` key) the argument at the call in the rule body must be an
// XVALUE, so every by-value parameter is passed on as `std::move(v)`; writing it
// bare would deduce `Arg = T&` and record a DEAD DUPLICATE of the `T &` key that
// f22-f38 already hold.  That trap has burned two slots.
//
// All eight use the EXPLICIT member-call form `std::move(d).operator<<(x)` for
// exactly f21's reason: the recorded callee must unambiguously be this
// `&&`-qualified member and not a free ADL `operator<<` candidate.
//
// ⛔ THE 331-SITE `OpAsmPrinter`/`AsmPrinter` `<<` FAMILY IS NOT TOUCHED HERE.
// It stands REFUSED at the dated refusal in this file (src.cpp:3481-3521) and
// this block does not reopen it; these eight are `InFlightDiagnostic` receivers
// only, which already has a real `libcc2rs` struct with a reporting `Drop` (t70).
//
// FIDELITY.  `llvm::Twine` has a real model (rules/twine): the NUL-terminated
// byte string the rope denotes, `Vec<u8>` in refcount and `Vec<libc::c_char>` in
// unsafe.  Streaming it appends exactly those bytes, and `shl_bytes` stops at the
// NUL, so the terminator the model carries is not appended -- the same asymmetry
// f21 already relies on.  `mlir::Type&&` prints via `Display`, which f25 already
// established is MLIR's builtin-type syntax (ir.rs:50); f153 differs from f25
// only in taking the value rather than a shared reference, which is correct
// because an rvalue argument cannot be a caller variable a move could damage.
// ---------------------------------------------------------------------------

// f150 -- `llvm::Twine &&`, 6 sites (witness: ddc/ddl/Dialect/DdlOps.cpp, A rc=0,
// at gen-inc/ddc/ddl/Dialect/DdlOps.cpp.inc:196:9).
mlir::InFlightDiagnostic &&f150(mlir::InFlightDiagnostic &&d, llvm::Twine s) {
  return std::move(d).operator<<(std::move(s));
}

// f151 -- `const llvm::Twine &`, 2 sites.  ⚠️ OBSERVED ONLY on
// dxp/tools/DxpOptMain.cpp:109:51, which is BUCKET B (it aborts earlier on
// `llvm::ToolOutputFile`), so this key has NO bucket-A witness today and its
// reach is NOT claimed -- it is written because the pin reported the key and the
// body is the same member as f150's at the other reference kind.
mlir::InFlightDiagnostic &&f151(mlir::InFlightDiagnostic &&d,
                                const llvm::Twine &s) {
  return std::move(d).operator<<(s);
}

// f152 -- `unsigned long &`, 4 sites.  f33's lvalue half.
mlir::InFlightDiagnostic &&f152(mlir::InFlightDiagnostic &&d,
                                unsigned long &v) {
  return std::move(d).operator<<(v);
}

// f153 -- `mlir::Type &&`, 4 sites.  f25's rvalue half; see the fidelity note.
mlir::InFlightDiagnostic &&f153(mlir::InFlightDiagnostic &&d, mlir::Type t) {
  return std::move(d).operator<<(std::move(t));
}

// f154 -- `long &&`, 2 sites.  f28's rvalue half.
mlir::InFlightDiagnostic &&f154(mlir::InFlightDiagnostic &&d, long v) {
  return std::move(d).operator<<(std::move(v));
}

// f155 -- `int &&`, 2 sites.  f27's rvalue half.
mlir::InFlightDiagnostic &&f155(mlir::InFlightDiagnostic &&d, int v) {
  return std::move(d).operator<<(std::move(v));
}

// f156 -- `const unsigned long &`, 2 sites.  The const-lvalue third kind, as f29
// is to f28 and f31 is to f26.
mlir::InFlightDiagnostic &&f156(mlir::InFlightDiagnostic &&d,
                                const unsigned long &v) {
  return std::move(d).operator<<(v);
}

// f157 -- `const llvm::StringRef &`, 2 sites.  The const-lvalue third kind next
// to f22 (`StringRef&&`) and f23 (`StringRef&`).
mlir::InFlightDiagnostic &&f157(mlir::InFlightDiagnostic &&d,
                                const llvm::StringRef &s) {
  return std::move(d).operator<<(s);
}

// ---------------------------------------------------------------------------
// t340 / f240 -- `mlir::OpaqueProperties`, mlir/IR/OperationSupport.h:69-80.
// THE WHOLE DECLARATION IS SIX LINES AND ITS MEMBER SURFACE WAS CENSUSED IN FULL
// BEFORE THIS KEY WAS WRITTEN, which is the thing the `simple_ilist<mlir::Block>`
// and `OperationState` refusals could not do:
//
//     class OpaqueProperties {
//     public:
//       OpaqueProperties(void *prop) : properties(prop) {}
//       operator bool() const { return properties != nullptr; }
//       template <typename Dest> Dest as() const {
//         return static_cast<Dest>(const_cast<void *>(properties));
//       }
//     private:
//       void *properties;
//     };
//
// ⭐ THE REPRESENTATION IS THE `void *` ITSELF, not a wrapper.  The class has one
// data member, a `void *`, a converting constructor from `void *`, and no
// destructor (`grep -rn '~OpaqueProperties' $LLVM_ROOT/include/mlir` = 0 hits, the
// same test t85 passes).  So `*mut ::libc::c_void` is not an opaque stand-in: it is
// the type's exact layout and exact semantics.  `operator bool()` would be
// `!is_null()` and `as<Dest>()` a pointer cast IF a site ever asked -- see the
// deliberate omission below.
//
// ⭐ ARITY 0, FULLY CONCRETE, SO THE SWALLOW CANNOT APPLY.  `GetTypeMapKey`
// truncates at the first `<` so arity is not in the key at all, and
// `matchTemplate`'s `findNextLiteralSameDepth` capture never runs for a key with
// no template arguments -- the t166 / t37-t39 / t320 precedent.
//
// ⭐ WHERE THE 30 SITES ARE, measured over the 58 bucket-A `.rs` of fresh30/out
// (total tree-wide `Cpp2RustUnmapped` = 7,127):
//     Cpp2RustUnmapped_mlir_OpaqueProperties   30   (the LONG spelling)
//     bare `OpaqueProperties` substring total   60   -> 60 - 30 = 30 BARE
// and the 30 bare ones are all ONE spelling, `mlir_OpaqueProperties::new` -- i.e.
// this row is a TYPE key AND a FABRICATED-CTOR key, which is why f240 exists.  A
// type key alone leaves `E0433: cannot find mlir_OpaqueProperties` behind.
// Four files: ddc/ddl/Dialect/DdlOps.cpp (46 occ), dialects/Init/InitOps.cpp (6),
// dialects/ExPlan/ExPlanOps.cpp (4), dataflow-scheduler/.../Dialect/Symbol/Symbol.cpp (2).
//
// ⭐ THE MEMBER CENSUS, and it is COMPLETE rather than sampled.  Two independent
// reads agree:
//   (1) `-verbose` on dialects/Init/InitOps.cpp, converter RC=0, log 100,589 lines,
//       emitted 4,611 lines == the non-verbose leg's 4,611 (so the log is NOT
//       truncated -- trap 5/6c).  `grep -o 'searched as: [^;]*OpaqueProperties[^;]*'`
//       returns exactly `3 searched as: mlir::OpaqueProperties` and NOTHING else:
//       three TYPE asks, ZERO member asks.
//   (2) the emitted corpus, which is where an unmapped MEMBER actually shows up
//       (it does not abort -- the converter emits the call textually at rc=0 with
//       no placeholder token).  Anchored over all 58 files:
//         `mlir_OpaqueProperties::[A-Za-z_0-9]*`  ->  30 `::new`, nothing else
//         `properties[.][A-Za-z_0-9]*`            ->  30 `properties.clone`, nothing else
//       So the ONLY things the corpus does to an OpaqueProperties value are
//       CONSTRUCT it from a `void *` and COPY it.  A raw pointer is `Copy` in Rust
//       and `.clone()` on it type-checks, so both are satisfied by the model above
//       and NEITHER is emitted textually against a member that does not exist.
//
// ⛔ DELIBERATELY LEFT OUT -- `operator bool()` and `as<Dest>()`.  Both are
// trivially expressible (`!a0.is_null()`; `a0 as *mut Dest`), and that is exactly
// why leaving them out is the right call rather than a gap: ZERO sites in the
// corpus ask for either (the two greps above are the proof, not an assumption), so
// keying them would be writing rules against no evidence, and `as<Dest>()` in
// particular would have to be keyed once per concrete `Dest` -- a set the corpus
// does not name.  Absent, they FAIL LOUDLY the moment a site does ask.
//
// ⚠️ WHAT THIS ROW DOES *NOT* FIX, stated so the next slot does not read the
// placeholder drop as "the properties block now compiles".  Each of the 30 sites
// sits inside an ODS-generated block that goes on to call
// `setOpPropertiesFromAttribute` on an `Option<dataflowir_gen::TdOpDef>` and to
// build an `llvm::function_ref<...>` via a fabricated `::new_4`.  BOTH are
// unmapped, and `grep -rn 'setOpPropertiesFromAttribute' dataflowir-gen/src/*.rs`
// is ZERO hits, so neither is in the model.  They are emitted TEXTUALLY, at rc=0,
// invisible to every census -- a SEPARATE row (the unmapped-member half), not this
// one.  t340/f240 is correct in itself; it is not sufficient for these TUs to
// build.
namespace mlir {
class OpaqueProperties {
public:
  OpaqueProperties(void *prop);
};
} // namespace mlir

using t340 = mlir::OpaqueProperties;

// f240 -- THE CONVERTING CONSTRUCTOR, `OpaqueProperties(void *prop)`.  It is the
// class's ONLY constructor (there is no default ctor), and it is the one all 30
// sites reach: the emitted argument is already `(... as *mut ::libc::c_void)`, so
// the target is the identity.
mlir::OpaqueProperties f240(void *a0) { return mlir::OpaqueProperties(a0); }

// ---------------------------------------------------------------------------
// ⛔ NEW REFUSAL -- `mlir::DialectBytecodeReader` (30 sites) and
// `mlir::DialectBytecodeWriter` (30 sites).  Scoped to this slot as "same shape as
// t340, take them if the primary lands early".  THEY ARE NOT THE SAME SHAPE, and
// the difference is the whole point of the unmapped-MEMBER trap.
//
// ⭐ THE SHAPE, measured over the 58 bucket-A `.rs` of fresh30/out.  Both types are
// 30 LONG-spelling occurrences and ZERO bare ones
// (`grep -o 'Cpp2RustUnmapped_mlir_DialectBytecodeReader'` = 30 and
// `grep -o 'DialectBytecodeReader'` = 30, so 30 - 30 = 0 bare; same for Writer) --
// i.e. NO fabricated constructor, unlike t340.  All 60 sites are in ONE position:
// a parameter, `reader: *mut Cpp2RustUnmapped_mlir_DialectBytecodeReader` /
// `writer: *mut ...Writer`, on the ODS-generated `readProperties`/`writeProperties`.
// That looks like a name-only key -- the t25/t320 cast-target shape.
//
// ⛔ IT IS NOT, BECAUSE THE MEMBERS *ARE* READ, AND AN UNMAPPED MEMBER DOES NOT
// ABORT -- it is emitted TEXTUALLY at rc=0 with no placeholder token, invisible to
// every census and to pin/no-placeholders.sh.  Anchored over the same 58 files:
//     (*reader).readAttribute            46      (*writer).writeAttribute            46
//     (*reader).readOptionalAttribute    33      (*writer).writeOptionalAttribute    33
//     (*reader).getBytecodeVersion       12      (*writer).getBytecodeVersion        12
//     (*reader).readSparseArray           6      (*writer).writeSparseArray           6
//     (*reader).emitError                 6
//                                   --- 103                                    --- 97
// and in each file the ONLY `reader:`/`writer:` declaration reaching those calls is
// the parameter above (the other `writer:` decls in the corpus are
// `mlir::IRRewriter`/`mlir::PatternRewriter`, a different row).  So a TYPE key here
// would drop 60 placeholders and leave 200 textual method calls on a Rust type that
// has none of them -- trading a LOUD failure for a SILENT one, which is the exact
// trade this tree forbids.
//
// ⛔ AND THE MEMBERS CANNOT BE KEYED, checked BOTH routes because a DEF can arrive
// by either.  For all seven method names plus `DialectBytecode` itself:
//     grep -c over dataflowir-gen/src/*.rs (HAND-WRITTEN: fmt.rs, ir.rs, td*.rs)  = 0
//     grep -c over the generated out/dataflow_ods.rs (TD_OPS_MORE route)          = 0
// There is no bytecode stream in the model at all -- no reader, no writer, no
// version.  Supplying one would mean PORTING MLIR, which is out of bounds: mlir
// types get models from the dataflowir-gen `.td` parser, never a hand-written
// stand-in.  So both rows stay LOUD, deliberately, and this is a
// `dataflowir-gen` capability question (bytecode (de)serialisation) rather than a
// rules row.  ⚠️ Re-test it with the two greps above before reopening.

// ---------------------------------------------------------------------------
// t440 / t441 -- `mlir::OpBuilder` and `mlir::ImplicitLocOpBuilder`.
//
// ⭐ THE SETTLED `OpBuilder` REFUSAL IS VOID as of `dataflowir-gen` commit 7065e77,
// which built the mutable IR builder the refusal was written for: `src/build.rs`,
// re-exported at the crate root (`src/lib.rs:74-77`, verified by reading that
// `pub use build::{ ... ImplicitLocOpBuilder, OpArgs, OpBuilder, OpHandle }`).  The
// refusal's reason was "there is no builder in the model at all"; there is one now,
// so the reason no longer holds.
//
// ⭐ THE SITES, counted MYSELF over the 58 bucket-A `.rs` of fresh30/out
// (tree-wide `Cpp2RustUnmapped` control = 7,127):
//     grep -o 'Cpp2RustUnmapped_mlir_OpBuilder'             2241
//     grep -o 'Cpp2RustUnmapped_mlir_ImplicitLocOpBuilder'   474
//                                                        ---- 2715  = 38% of 7,127
// ⛔ A CIRCULATED "1,646 / 1,172 / 474" IS WRONG AND THE ERROR IS INSTRUCTIVE.  The
// 1,172 comes from `grep -o 'Cpp2RustUnmapped_mlir_[A-Za-z_0-9]*OpBuilder[A-Za-z_0-9]*'`
// -- the GREEDY `[A-Za-z_0-9]*` before `OpBuilder` swallows the run, so a
// `create_pmutCpp2RustUnmapped_mlir_OpBuilder_...` site is consumed by a match that
// STARTS elsewhere and the token is counted once instead of once per occurrence.
// That is the same family as the documented "leading context char undercounts"
// trap, and it undercounted by 1,069.  The decomposition-by-subtraction it invites
// is ALSO wrong here, and for a reason worth stating: the long spellings
// (`..._OpBuilder_dataflowir_genirLocation_i64` and 432 siblings) are NOT
// alternative spellings of the type -- they are the SAME placeholder token embedded
// inside a synthesised method name, e.g.
//     mlir_arith_ConstantIndexOp::create_pmutCpp2RustUnmapped_mlir_OpBuilder_dataflowir_genirLocation_i64
// so subtracting them REMOVES real sites.  Verified by the identity
//     2241 + 474 == 2715 == (greedy-token census total, `grep -oh 'Cpp2RustUnmapped_[A-Za-z_0-9]*' | grep -ci opbuilder`)
// which is the arithmetic check that the anchored counts are complete.
//
// ⭐ `ImplicitLocOpBuilder` IS THE CHEAP TAIL, NOT SEPARATE WORK.  It is
// `class ImplicitLocOpBuilder : public mlir::OpBuilder` (Builders.h:630) -- one
// extra `Location` field -- and `build.rs:630` models it exactly that way
// (`struct ImplicitLocOpBuilder` with `Deref/DerefMut to OpBuilder`).
//
// ⭐ THE MEMBER CENSUS, BOTH READS, because a `-verbose` log and the emitted corpus
// see different things and only the corpus sees an unmapped MEMBER:
//   (1) `-verbose` on dialects/Init/InitOps.cpp with the harness's own `--cxxflags`
//       (verif/dxpflags.py).  Converter RC=0 -- echoed from inside the backgrounded
//       command, not read off a wrapper's status (trap 6c) -- log 100,825 lines,
//       emitted 4,596 lines == the non-verbose leg's 4,596, so the log is NOT
//       truncated (traps 5 / 6c).  `grep -o 'searched as: [^;]*OpBuilder[^;]*'`:
//           262  searched as: mlir::OpBuilder
//            90  searched as: mlir::ImplicitLocOpBuilder
//       and NOTHING else -- 352 TYPE asks, ZERO member asks.
//   (2) the emitted corpus, anchored over all 58 files.  `<recv>::[A-Za-z_0-9]*` on
//       the placeholder receiver returns ZERO for both types (no `::` member is
//       reached at all), so every member call is through a VARIABLE.  The variables
//       declared with these types are, by count of declaration sites,
//       `_builder` 395, `_odsBuilder` 136, `_arg0` 102, `_loop_builder` 4, `_ip` 1,
//       and in parameter position `builder` 512 / `odsBuilder` 135.  Censusing
//       `<var>[.][A-Za-z_0-9]*` over those:
//           builder.setInsertionPointToStart      3
//           builder.setInsertionPointAfter        1
//           odsBuilder.getNamedAttr              79
//           odsBuilder.getDictionaryAttr         51
//           odsBuilder.getBoolAttr                6
//           odsBuilder.getStringAttr              4
//           odsBuilder.getStrArrayAttr            4
//           odsBuilder.getI64ArrayAttr            2
//           odsBuilder.getIntegerType             1
//           odsBuilder.getIntegerAttr             1
//       (`_builder`/`_odsBuilder`/`_arg0` are the converter's own temporaries and
//       are only ASSIGNED and PASSED, never dotted -- zero member hits each.)
//
// ⛔ SO THE TYPE KEYS ARE WRITTEN AND THE MEMBERS ARE NOT, DELIBERATELY, and this is
// the unmapped-MEMBER rule applied rather than an omission.  An unmapped member does
// NOT abort: the converter emits the call TEXTUALLY, at rc=0, with no placeholder
// token, invisible to every census and to pin/no-placeholders.sh.  So a member may
// only be keyed if the name EXISTS on the Rust type.  Checked each against
// `build.rs`'s public surface:
//   * `getNamedAttr` / `getDictionaryAttr` / `getBoolAttr` / `getStringAttr` /
//     `getStrArrayAttr` / `getI64ArrayAttr` / `getIntegerType` / `getIntegerAttr`
//     -- 148 sites, ALL LEFT OUT.  These are `mlir::Builder` members (the BASE of
//     OpBuilder, Builders.h:53-220), i.e. the attribute/type FACTORY half, and
//     `build.rs` models only the INSERTION half.  Keying them would name functions
//     that do not exist, trading 148 loud failures for 148 silent ones.  ⭐ They are
//     a separate row on `mlir::Builder`, and the right fix is a `dataflowir-gen`
//     attribute factory, not a rules key.
//   * `setInsertionPointToStart` / `setInsertionPointAfter` -- 4 sites.  The names
//     DO exist (`build.rs:519` `set_insertion_point_to_start(&mut self, block: usize)`,
//     `build.rs:541` `set_insertion_point_after(&mut self, h: &OpHandle) -> bool`),
//     but the C++ parameters are `mlir::Block *` and `mlir::Operation *` while the
//     Rust ones are a `usize` block INDEX and an `&OpHandle`.  There is no
//     pointer->index mapping available to a rule body (the index is owned by the
//     `BlockList` the builder was constructed over), so a key here would have to
//     fabricate one.  LEFT OUT so all 4 stay loud.
//   * `createOrFold` -- DELIBERATELY ABSENT from the new model (folding calls each
//     op's own `fold()`, C++ that no `.td` describes), so a key for it would name a
//     function that does not exist.  LEFT OUT; `grep -c 'createOrFold'` over the
//     corpus confirms the corpus does not ask for it either.
//
// ⚠️ WHAT THIS ROW DOES *NOT* FIX -- stated so the next slot does not read a 2,715
// placeholder drop as "the builder now compiles".  20 corpus sites are the
// FABRICATED-CONSTRUCTOR spelling `mlir_OpBuilder::new_1` (16) and
// `mlir_OpBuilder::new_2` (4) -- bare, no `Cpp2RustUnmapped_` prefix, so invisible
// to the placeholder census.  Real `OpBuilder` has ~8 constructor overloads and the
// `new_1`/`new_2` INDICES cannot be mapped onto them from the emitted text alone;
// guessing would be writing a rule against no evidence.  NO `f` KEY IS WRITTEN, so
// those 20 stay a loud `E0433: cannot find mlir_OpBuilder`.  That is the t340/f240
// shape left half-done ON PURPOSE, and it is the next slot's row: dump the
// ctor signature the converter asks for, then key it.
//
// ⭐ ARITY 0, FULLY CONCRETE -- the t166 / t37-t39 / t320 / t340 precedent.
// `GetTypeMapKey` truncates at the first `<`, so no template argument enters the
// key and `matchTemplate`'s same-depth capture (the swallow bug) never runs.
namespace mlir {

// ⛔ NO CONSTRUCTOR IS DECLARED HERE, and that is deliberate: a declared ctor with
// no `f` key is a rule the preprocessor would carry with nothing behind it, and the
// 20 `mlir_OpBuilder::new_1`/`new_2` sites must stay LOUD (see above).  The stub
// exists only to make the TYPE NAME resolvable, exactly as `class Block {}` does.
// ⛔ `OpBuilder` IS *NOT* REDECLARED HERE -- it is ALREADY declared at line 1019,
// where a 2026-09-27 slot had to declare it COMPLETE purely to hold the nested
// `OpBuilder::Listener`, with the standing note "OpBuilder itself is NOT mapped".
// That note is what t440 changes; the declaration is reused verbatim.  Declaring it
// a second time is `error: redefinition of 'OpBuilder'` and aborts the whole module
// regen -- measured, not guessed.
class ImplicitLocOpBuilder : public OpBuilder {};

} // namespace mlir

using t440 = mlir::OpBuilder;
using t441 = mlir::ImplicitLocOpBuilder;

// ---------------------------------------------------------------------------
// llvm::SmallDenseSet<T> -- t480..t482.  Queue rows g403, g1129-g1138 (11 rows,
// all reopened from `blocked:refusal-NARROWED-remeasure`).
//
// ⭐ THE ROWS' OWN `searched as:` IS THE SHORT ARITY-1 SPELLING, not the long one
// the row TITLE shows.  Taken untruncated from queue/samples/g1136.txt:
//     searched as: llvm::SmallDenseSet<unsigned int>
//     from decl (NOT a key -- canonicalised, defaulted args kept):
//                  llvm::SmallDenseSet<unsigned int, _, llvm::DenseMapInfo<unsigned int, void>>
// The recorder suppresses TRAILING defaulted template arguments, and BOTH of
// `SmallDenseSet`'s trailing parameters are defaulted (DenseSet.h:288-290:
// `unsigned InlineBuckets = 4`, `typename ValueInfoT = DenseMapInfo<ValueT>`), so
// the key the converter actually looks up is arity 1.  All eleven samples agree.
// ⛔ Keying the long `from decl` text instead would have produced three DEAD keys
// -- that text is explicitly labelled "NOT a key" by the recorder itself.
//
// ⭐ GROUND TRUTH, not just the lead: the BEFORE leg on the witness emits
//     Cpp2RustUnmapped_llvm_SmallDenseSet_unsigned_int_        (19 sites)
// in dcc/src/Transform/Sentient/Analyses/GraphStats.cpp (bucket A rc=0), which is
// the arity-1 spelling mangled.  That is the name the gate counts.
//
// ⭐ THE DECLARATION MUST CARRY THE DEFAULTS, and each `using` below must OMIT the
// trailing arguments.  Written that way the recorded arity is 1 and the key is
// FULLY CONCRETE: placeholder arity 0, so `matchTemplate`'s same-depth capture
// (`findNextLiteralSameDepth`, mapper.cpp:173) never runs and the swallow that
// killed rules/setvector's member keys is ruled out BY CONSTRUCTION.  Same
// discipline as t236-t242 / t166 / t37-t39.
// ⛔ AND THAT IS WHY THESE ARE NOT ONE GENERIC `llvm::SmallDenseSet<T1>` KEY (the
// shape rules/densemap uses for `llvm::DenseSet<T1>`).  A single arity-1 generic
// would also match the arity-3 `from decl` spelling if the recorder ever stops
// suppressing the defaults, with `T1` capturing the WHOLE argument list because a
// comma is not a delimiter -- i.e. it would silently map the set to
// `HashSet<unsigned int, _, llvm::DenseMapInfo<...>>`-as-one-type.  Concrete keys
// cannot do that.
//
// ⛔ `llvm::SmallDenseSet` IS A DIFFERENT BUCKET FROM t239.  `GetTypeMapKey`
// truncates at the first `<`, so `llvm::detail::DenseSetImpl` (t239, the CRTP
// BASE of a `SmallDenseSet<long, N>`) and `llvm::SmallDenseSet` (the DERIVED
// class, here) never share a bucket and never compete.  t239 is not a duplicate of
// t480 and neither is dead because of the other: the recorder emits whichever
// class the corpus NAMES, and these eleven rows name the derived one.
// ⚠️ The only two prior mentions of `SmallDenseSet` in this module were both
// COMMENTS (the t239 header and the "DECLARING class" note above it) -- there was
// no pre-existing SmallDenseSet key, so nothing here is a second key beside a dead
// one.
//
// MODEL: `std::collections::HashSet<T>`, exactly `rules/densemap` t2 and
// `rules/mlir` t236-t242.  A `SmallDenseSet`'s inline-bucket capacity is an
// allocation strategy, not semantics, and its iteration order is unspecified, so a
// HashSet claims nothing the C++ does not.
//
// ⭐ ELEMENT EQUALITY IS CHECKED PER SPELLING, because `std::collections::HashSet<T>`
// is only USABLE for `T: Hash + Eq`:
//   * `long` -> `i64`, `unsigned int` -> `u32`  -- primitives, both derive both.
//     Identical to t236 (`HashSet<i64>`) and t240 (`HashSet<u32>`) in this module.
//   * `llvm::StringRef` -> `Vec<u8>` (refcount) / `Vec<libc::c_char>` (unsafe).
//     ⚠️ THE TWO MODELS SPELL IT DIFFERENTLY and this module already commits to
//     both spellings (f22/f23 in each target file), so t482 is not a new
//     cross-module claim -- it reuses the one rules/mlir already makes.  Both
//     `Vec<u8>` and `Vec<i8>` are `Hash + Eq`.
//
// ⛔ THE FOURTH ELEMENT TYPE, `mlir::Attribute`, IS DELIBERATELY LEFT OUT -- rows
// g403, g1132, g1133, g1134, g1135.  t6 maps `mlir::Attribute` to
// `dataflowir_gen::ir::Attr`, and the refusal that stood here for the DenseSetImpl
// rows said the deciding read was "does `ir::Attr` implement `Hash + Eq`" and that
// it was "a dataflowir-gen read, not a rules read".  ⭐ THAT READ IS NOW DONE AND
// IT COMES BACK NEGATIVE: `dataflowir-gen/src/ir.rs:465` is
// `#[derive(Debug, Clone, PartialEq, Eq)]` -- `Eq` YES, `Hash` NO.  A
// `HashSet<ir::Attr>` is a well-formed TYPE (the struct carries no bound) so the
// key would record cleanly and pass every gate, and then no membership operation
// on it could ever compile.  That is a container that cannot function, i.e. exactly
// the silent lie t236-t242's header refuses to write.  ⭐ THE CLOSING STEP IS
// NAMED AND SMALL: add `Hash` to that derive in `dataflowir-gen/src/ir.rs` (which
// needs `Ty` and every payload to be `Hash` too), then t483 is a one-line key.
// It is not done here because it is a dataflowir-gen rebuild under a store six
// slots are already fighting for today.
//
// ⭐ NO MEMBER RULES, and as for t236-t242 that is not an omission: all eleven rows
// are `kind=type` ("system type has no rule"); the queue holds no SmallDenseSet
// method row at all.  ⚠️ BUT THE MEMBERS ARE REAL AND THEY ARE READ -- censused
// from the witness GraphStats.cpp: `insert` (x6), `erase`, `empty` (x2), `size`,
// `begin` (dereferenced), the copy constructor, and SIX range-for loops over the
// set.  Those emit TEXTUALLY at rc=0 with no placeholder token (the unmapped-member
// hole), so they stay loud at rustc, not silent -- but they are the reason this is a
// type-row closure and not a working container.  The range-for/`begin` half needs a
// `DenseSetImpl<...>::Iterator` model, which is a separate row family.
namespace llvm {
template <typename ValueT, unsigned InlineBuckets = 4,
          typename ValueInfoT = DenseMapInfo<ValueT>>
class SmallDenseSet {};
} // namespace llvm

// t480 -- rows g1130, g1131.  Recorded key: `llvm::SmallDenseSet<long>`.
using t480 = llvm::SmallDenseSet<long>;

// t481 -- rows g1136, g1137, g1138.  Recorded key:
// `llvm::SmallDenseSet<unsigned int>`.  This is the witness spelling.
using t481 = llvm::SmallDenseSet<unsigned int>;

// t482 -- row g1129.  Recorded key: `llvm::SmallDenseSet<llvm::StringRef>`.
using t482 = llvm::SmallDenseSet<llvm::StringRef>;
// ===========================================================================
// f400-f406 -- THE `mlir::Builder` ATTRIBUTE FACTORY, 148 SITES.
//
// ⭐ THIS IS THE ROW t440/t441 NAMED AND DELIBERATELY LEFT OUT.  Its refusal was
// "`build.rs` models only the INSERTION half [...] keying them would name
// functions that do not exist, trading 148 loud failures for 148 silent ones."
// `dataflowir-gen` commit 2f78cb6 built the factory half (`src/build.rs:488-561`,
// `pub struct Builder` + eight `&self` methods, re-exported at the crate root),
// so the reason no longer holds.  Each name below was CHECKED against that file,
// not against the C++ header, because an unmapped member does NOT abort -- the
// converter emits the call TEXTUALLY at rc=0 with no placeholder token.
//
// ⭐ REACHABILITY: `OpBuilder` has a `base: Builder` field and
// `impl Deref for OpBuilder { Target = Builder }`, and `ImplicitLocOpBuilder`
// Derefs to `OpBuilder`, so all eight resolve on a t440/t441 receiver through the
// Deref chain with no delegating method.
//
// ⭐ THE RECEIVER IS `mlir::OpBuilder &`, NOT `mlir::Builder &`.  All 148 sites
// are `builder.getX(...)` / `odsBuilder.getX(...)` -- the census measured
// `<recv>::[A-Za-z_0-9]*` = 0 for both builder types, so NOTHING is reached by
// qualification -- and the declaring variables are t440/t441-typed.  `mlir::Builder`
// itself is t59 -> `()`, so a `mlir::Builder &` receiver would type `a0` as `&()`
// (see f124, which is exactly that) and no factory method exists on `()`.
//
// ⛔ ONE NAME IS LEFT OUT ON PURPOSE: `getIntegerType` (1 site).  The model has
// `get_integer_type(width: u32) -> ir::Ty`, but C++ returns `mlir::IntegerType`,
// declared at :790 with NO `using tN =` -- UNMAPPED.  A key would have to name a
// target type this module does not define.  LEFT LOUD.  `createOrFold` is absent
// from the model on purpose and the corpus does not ask for it.
//
// ⛔ VALUE AND KIND ARE PRESERVED, which is the `PassOptions::Option<bool>`
// lesson: `f402` forwards `a1` unchanged so `get_bool_attr(false)` and
// `get_bool_attr(true)` cannot collapse, and the two byte-carrying keys decode the
// WHOLE payload up to the NUL with f365/f366's own idiom (`String::from_utf8_lossy`
// is what `libcc2rs::Ptr::to_rust_string` uses, so this is the module's
// established decode, not a new one).  Each `aN` is named EXACTLY ONCE -- the
// `take_while` form is preferred over f365's `take(a1.len()-1)` precisely because
// the latter mentions `a1` twice and a rule body is inlined as one expression.
//
// ⛔ `getDictionaryAttr` RETURNS `ir::AttrDict`, NOT `ir::Attr` -- consistent with
// t9 (`mlir::DictionaryAttr` -> `ir::AttrDict`) and t261
// (`mlir::NamedAttrList` -> `ir::AttrDict`), so a nested dictionary used as an
// attribute VALUE stays a rustc type error by design.
// ⛔ `getNamedAttr` returns t21's `(String, Attr)` TUPLE, not the model's
// `ir::NamedAttribute` STRUCT; f400 destructures rather than changing t21, which
// other keys already depend on.  The pair is carried whole, so nothing is lost.
// ===========================================================================
mlir::NamedAttribute f400(mlir::OpBuilder &a0, llvm::StringRef a1,
                          mlir::Attribute a2) {
  return a0.getNamedAttr(a1, a2);
}

mlir::DictionaryAttr f401(mlir::OpBuilder &a0,
                          llvm::ArrayRef<mlir::NamedAttribute> a1) {
  return a0.getDictionaryAttr(a1);
}

mlir::BoolAttr f402(mlir::OpBuilder &a0, bool a1) { return a0.getBoolAttr(a1); }

mlir::StringAttr f403(mlir::OpBuilder &a0, const llvm::Twine &a1) {
  return a0.getStringAttr(a1);
}

mlir::IntegerAttr f404(mlir::OpBuilder &a0, mlir::Type a1, int64_t a2) {
  return a0.getIntegerAttr(a1, a2);
}

mlir::ArrayAttr f405(mlir::OpBuilder &a0, llvm::ArrayRef<int64_t> a1) {
  return a0.getI64ArrayAttr(a1);
}

mlir::ArrayAttr f406(mlir::OpBuilder &a0, llvm::ArrayRef<llvm::StringRef> a1) {
  return a0.getStrArrayAttr(a1);
}

// ---------------------------------------------------------------------------
// THE REWRITER FAMILY -- t541/t542/t543 -- AND mlir::DenseArrayAttr -- t540.
//
// ⭐ WHY THIS IS NOW WRITABLE.  The three rewriter types were refused all day for
// ONE stated reason, recorded at line 3760 ("the `mlir::OpBuilder` / `RewriterBase`
// refusal") and line 5802: they are `mlir::OpBuilder` BY INHERITANCE
// (PatternMatch.h:368 `class RewriterBase : public OpBuilder`, :780
// `class IRRewriter : public RewriterBase`, :799
// `class PatternRewriter : public RewriterBase`) and OpBuilder had no model.
// ⛔ THAT BASIS IS VOID: t440 maps `mlir::OpBuilder -> dataflowir_gen::OpBuilder`
// and t441 `mlir::ImplicitLocOpBuilder`, both landed and gated.  So a rewriter IS
// an OpBuilder plus a few rewrite verbs, and the TYPE is exactly OpBuilder.
//
// ⭐ THE MEMBER CENSUS, BOTH READS (t440's discipline, because only the emitted
// corpus can see an unmapped MEMBER -- it emits TEXTUALLY at rc=0 with no
// placeholder token, invisible to every census and to pin/no-placeholders.sh):
//   (1) the ask logs of the freshest full sweep (fresh34, 84 logs),
//       `grep -o 'searched as: mlir::(PatternRewriter|RewriterBase|IRRewriter)[^;]*'`:
//            41  searched as: mlir::PatternRewriter
//            18  searched as: mlir::IRRewriter
//             8  searched as: mlir::RewriterBase
//       and NOTHING else -- 67 TYPE asks, ZERO member asks, and no `&`/`*`
//       spelling, so ONE key per type is the whole ask (t440 measured the same).
//   (2) the emitted corpus, all 58 `.rs`.  `<placeholder>::[A-Za-z_0-9]*` returns
//       ZERO for all three (no `::` member reached), so every use is through a
//       VARIABLE.  The variables declared with these types are, by declaration
//       count, `_rewriter` 6 / `rewriter` 5 (PatternRewriter) and `rewriter` 5 /
//       `_rewriter` 5 (IRRewriter).  Censusing `<var>[.][A-Za-z_0-9]*` over the
//       whole corpus returns **ZERO HITS for both `rewriter.` and `_rewriter.`**:
//       in the emitting TUs these are only DECLARED and PASSED, never dotted.
//       ⭐ So there is no unmapped-member hole to open here at all -- the type key
//       is the complete row for this corpus.
//
// ⛔ EVERY REWRITE VERB IS DELIBERATELY LEFT OUT, and this is the reason, which is
// sharper than "no counterpart".  A repo-wide census of `rewriter.<member>` over
// dt_src C++ (which reaches non-A TUs the corpus above does not) finds the verbs
//     replaceOp 49, eraseOp 30, replaceOpWithNewOp 15, notifyMatchFailure 9,
//     modifyOpInPlace 6, inlineRegionBefore 6, replaceAllUsesWith 3,
//     inlineBlockBefore 3, eraseBlock 3, applySignatureConversion 1, clone 1
// plus the INHERITED Builder/OpBuilder half (getContext 21, setInsertionPoint 20,
// getI32IntegerAttr 16, getZeroAttr 12, getIndexType 12, ...), which is t440's
// already-recorded row and not reopened here.
// ⭐ THE BLOCKER FOR THE MUTATING VERBS IS ONE FACT: they all take
// `mlir::Operation *`, and `mlir::Operation` is t1 -> `dataflowir_gen::fmt::OpInst`
// -- a DETACHED op record, NOT an `OpHandle`.  `OpHandle` is the only thing in the
// builder model that reaches an op IN ITS BLOCK (`with_op_mut`, `erase`, identity
// by `(Rc::as_ptr(list), OpId)` rather than content), and there is no
// `OpInst*` -> `OpHandle` mapping available to a rule body.  ⛔⛔ A key for
// `eraseOp`/`replaceOp`/`modifyOpInPlace` written against `fmt::OpInst` would
// therefore mutate (or drop) a DETACHED COPY and lose every rewrite SILENTLY at
// rc=0 -- the exact failure mode a rewriter model must not have.  So all of them
// stay LOUD instead: an unmapped member emits textually and fails at rustc with
// `no method named replaceOp on dataflowir_gen::OpBuilder`.
// ⭐ THE `dataflowir-gen` METHOD THAT DOES NOT EXIST AND WOULD UNBLOCK THEM:
// a way to obtain an `OpHandle` for an op already in a `BlockList` from the op
// itself -- e.g. `OpBuilder::handle_of(&OpInst) -> Option<OpHandle>` or
// `BlockList::find_op(&OpInst) -> Option<OpHandle>` -- plus, on top of it,
// `OpHandle::replace_all_uses_with(&[Value])` for `replaceOp`.  Named precisely
// so the coordinator can hand it to one of the live dataflowir-gen slots.
//
// ⭐ ARITY 0, FULLY CONCRETE for all four -- the t440/t441 precedent.
// `GetTypeMapKey` truncates at the first `<`, so no template argument enters the
// key and matchTemplate's same-depth capture (the swallow bug) never runs.
//
// t540 `mlir::DenseArrayAttr` (BuiltinAttributes.h.inc:85) is the arity-0 BASE of
// the already-mapped `mlir::detail::DenseArrayAttrImpl<T>` (t26/t29, both ->
// `ir::Attr`).  Its 32 asks are all `IntrinsicAttr<..., DenseArrayAttrImpl<int64_t>>
// ::getAttrName`, i.e. the type appears as a TEMPLATE ARGUMENT and a return type,
// never constructed and never printed, so the row is a pure type-identity row.
// It maps to `dataflowir_gen::ir::Attr` for the same reason t6/t7/t9/t10/t11/t12 do:
// in real MLIR every *Attr is a derived HANDLE over the one uniqued Attribute
// hierarchy, and `ir::Attr` is the closed union of that hierarchy.
// ⛔ NOT `ir::Attr::Array`/`I32Array` AND THIS MATTERS: those two variants model
// `mlir::ArrayAttr` (ir.rs:495-522 says so in both doc comments) -- an array of
// *Attr elements, printed `[1 : i32]` -- whereas `DenseArrayAttr` is the dense
// inline form printed `array<i64: 1, 2>`.  They are different MLIR types with
// different rendered text, so nothing here claims one is the other; the key names
// the UNION type, exactly as t6 `mlir::Attribute` does, and no member is keyed.
namespace mlir {

// BuiltinAttributes.h.inc:85.  Declared as a bare complete class: only the NAME
// enters a type key, and no member of it is keyed.
class DenseArrayAttr {};

// PatternMatch.h:368 / :780 / :799.  The inheritance is restated as LLVM writes it
// even though a type key does not need it, so the next reader can see that these
// three ARE OpBuilders and does not re-derive the refusal.  ⛔ `OpBuilder` is NOT
// redeclared -- it is already declared complete at line 1019, and a second
// declaration is `error: redefinition` and aborts the whole module regen.
class RewriterBase : public OpBuilder {};
class PatternRewriter : public RewriterBase {};
class IRRewriter : public RewriterBase {};

} // namespace mlir

// t540 -- 32 asks; recorded key `mlir::DenseArrayAttr`.
using t540 = mlir::DenseArrayAttr;

// t541 -- 8 asks; recorded key `mlir::RewriterBase`.
using t541 = mlir::RewriterBase;

// t542 -- 41 asks; recorded key `mlir::PatternRewriter`.  Witness spelling.
using t542 = mlir::PatternRewriter;

// t543 -- 18 asks; recorded key `mlir::IRRewriter`.
using t543 = mlir::IRRewriter;

// ===========================================================================
// t560 / t561 / f460 / f461 -- the `llvm::simple_ilist<mlir::Block>` ROW.
// FOUR SLOTS BOTTOMED OUT ON THIS AND IT IS LANDED AS ONE ATOMIC SET.
//
// THE ROW.  Freshest sweep: `llvm::simple_ilist<mlir::Block>` 64 ASKS / 16
// EMITTED PLACEHOLDER SITES (asks run ~5x sites; both numbers reported).
// `llvm::iplist<mlir::Block>` 32 asks / 9 emitted placeholder sites.  Queue row
// g074 gives the type spelling verbatim:
//     searched as: llvm::simple_ilist<mlir::Block>
//     from decl (NOT a key -- canonicalised, defaulted args kept):
//                 llvm::simple_ilist<mlir::Block>
// -- identical, so there is no defaulted-argument dead duplicate to dodge.
//
// ONE SHAPE, 16 SITES, e.g. KTDFLowToDFIR/DataTransferLowering.cpp (8 of them)
// and dsc-based-utils/DSC2ToDataflowIR/V3/SNTransferLowering.cpp:
//     (*(unsafe { ((*(unsafe { (*(unsafe { ...::getRegion(self) })).getBlocks() }))
//         as Cpp2RustUnmapped_llvm_simple_ilist_mlir_Block_).begin() })).getArgument(0)
// i.e. region -> block list -> FIRST BLOCK -> block argument 0.
//
// ⛔⛔ WHY THE TYPE KEY ALONE IS FORBIDDEN, AND WHY THIS IS ALL-OR-NOTHING.
// An UNMAPPED MEMBER DOES NOT ABORT -- the converter emits it TEXTUALLY, rc=0,
// with NO placeholder token, invisible to every census and to
// `pin/no-placeholders.sh`.  So a lone `t560` would swap a LOUD placeholder for
// a SILENT call to `Vec::<fmt::Block>::getBlocks()`, which does not exist: that
// is the `OperationState -> ()` bargain, and it is why this row was refused
// twice.  Hence t560 + t561 + f460 + f461 together, or none.
//
// THE MODEL, AND IT IS NOT NEW.  `fmt::Region { pub blocks: Vec<Block> }`
// (dataflowir-gen fmt.rs:513), so the block list IS a `Vec<fmt::Block>` and both
// container spellings map to it.  `llvm::iplist<T>` derives from
// `iplist_impl<simple_ilist<T>>` which derives from `simple_ilist<T>`
// (ilist.h:110/:327), so they are the SAME container at two points of the
// hierarchy -- the t37-t39 / t166 "the base IS the range" discipline, one body
// for both keys.  This is why the emitted text casts to
// `simple_ilist<mlir::Block>` before `.begin()`: `begin()` is declared on the
// BASE (simple_ilist.h:118), so the converter upcasts first, and the `begin`
// deduction below therefore keys the BASE receiver, not `iplist`.
//
// SWALLOW-SAFETY, argued for the buckets rather than assumed.  `GetTypeMapKey`
// truncates at the first `<`, giving buckets `llvm::simple_ilist` and
// `llvm::iplist`.  `grep -rn 'simple_ilist|llvm::iplist' rules/*/src.cpp` finds
// NO other module naming either (there is no `rules/ilist`), so each bucket
// holds exactly ONE candidate -- mine.  Both are FULLY CONCRETE: no `T<digits>`
// appears in either spelling, so `matchTemplate`'s placeholder capture
// (`findNextLiteralSameDepth`) NEVER RUNS and the same-depth-comma swallow is
// ruled out by construction, the t243-t246 / t250-t251 argument.
//
// ⛔ REPORT-ONLY, DELIBERATELY NOT KEYED HERE:
// `llvm::iplist_impl<llvm::simple_ilist<mlir::Block>, llvm::ilist_traits<mlir::Block>>`
// (queue g1470, 1 TU) and the `mlir::Operation` twins (g1481, g108).  The
// Operation-element spellings need `Vec<fmt::OpInst>` and a separate member
// census; `iplist_impl` needs `llvm::ilist_traits` declared and carries 1 TU.
// Neither is on the path of these 16 sites, and mixing them in would put a
// third, differently-argued type into the measurement.

// f460 -- `llvm::iplist<mlir::Block> & mlir::Region::getBlocks()`.
// ⭐ THE RECEIVER IS ALREADY MAPPED: t3 `mlir::Region -> fmt::Region`, and the
// dataflowir-gen witness already declares `getRegion(&mut self) -> *mut
// fmt::Region`, so the whole chain up to this call is live TODAY -- this member
// is the only missing link, and it is missing SILENTLY.
// THE FORM IS f150-f157's: a free function whose FIRST parameter is the
// receiver, body calling nothing but the member.
// ⚠️ `get_blocks_mut` (fmt.rs:557) IS THE CORRECT HALF, not `get_blocks`
// (:549): the receiver in every emitted site is a `*mut fmt::Region`
// dereference, i.e. a mutable lvalue, and C++ `getBlocks()` is non-const and
// returns a MUTABLE reference.  ⚠️ AND `get_blocks_mut` IS snake_case ON
// PURPOSE: a camelCase target name would let the *other*, still-unmapped C++
// members of this class resolve BY ACCIDENT against dataflowir-gen and destroy
// the diagnostic.  Renaming is this key's job.
// ⛔ `take_block_list()` is NOT what this body wants: it leaves the region
// EMPTY.  `getBlocks()` is a pure accessor, so the aliasing `&mut` borrow is the
// faithful body and the `Rc` bridge stays unused here.
llvm::iplist<mlir::Block> &f460(mlir::Region &r) { return r.getBlocks(); }

namespace llvm {
// ⚠️ EXPLICIT SPECIALISATION, not a member on the primary template, so that
// `begin()`'s return type is WRITTEN OUT and cannot be rendered through a
// dependent `typename ...::iterator` that would never match the recorded key.
// simple_ilist.h:95 `using iterator = ilist_select_iterator_type<OptionsT,
// false, false>` and :118 `iterator begin()`, which at `T = mlir::Block`
// resolves to EXACTLY t245's spelling -- the key `begin()` must match.
// simple_ilist.h:119 `const_iterator begin() const` is the OTHER overload and is
// deliberately NOT declared: the corpus receiver is a mutable lvalue, and
// declaring only one overload makes a const ask FAIL LOUDLY instead of silently
// binding to the wrong iterator constness (t246's IsReverse=true rbegin/rend
// half is likewise absent because nothing asks for it).
template <> class simple_ilist<mlir::Block> {
public:
  ilist_iterator<ilist_detail::node_options<mlir::Block, false, false, void,
                                            false, void>,
                 false, false>
  begin();
};
} // namespace llvm

// f461 -- `llvm::simple_ilist<mlir::Block>::begin()`, returning t245.
// ⭐⭐ THE ALIASING REQUIREMENT, WHICH IS WHAT KILLED THIS ROW TWICE.  The next
// thing all 16 sites do is DEREFERENCE the result and call `getArgument(0)`, so
// `begin()` must hand back an iterator that ALIASES the live block list.  Every
// ALLOCATING `Ptr` constructor is therefore wrong: `Ptr::null()` would deref
// null, and `Ptr::alloc(a0[0].clone())` fabricates a COPY so any mutation
// through the iterator is lost.  ⭐ The settled precedent is rules/vector's f13
// (`std::vector<T1>::begin()`), whose refcount formal IS `Ptr<T1>` and whose
// body is the bare `a0`: the converter hands a rule a BORROW of the owner's
// `Value<Vec<T>>` (`PtrKind::StackVec(Rc::downgrade(owner))`, rc.rs:1047 --
// the same provenance `Ptr::borrow_vec` names at rc.rs:281), so NOTHING IS
// ALLOCATED and writes land in the owner.  libcc2rs' own test asserts that
// provenance (`assert!(matches!(begin.kind, PtrKind::StackVec(_)))`,
// rc.rs:1340).  The unsafe half is rules/vector f13's `a0.as_mut_ptr()`, an
// interior pointer into the same buffer.
// ⚠️ `delete()` on a borrow-provenance `Ptr` panics `"ub: invalid delete"` BY
// DESIGN, which is correct: deleting a block through this iterator is UB in C++
// too.
// ⛔ NO `operator++`, `operator*`, `end()`, `empty()` OR ANY OTHER MEMBER IS
// KEYED, the t243-t246 discipline.  `getArgument(0)` on the RESULT is a
// `fmt::Block` member and resolves in dataflowir-gen, not here.
llvm::ilist_iterator<
    llvm::ilist_detail::node_options<mlir::Block, false, false, void, false,
                                     void>,
    false, false>
f461(llvm::simple_ilist<mlir::Block> &l) {
  return l.begin();
}

// t560 -- `llvm::simple_ilist<mlir::Block>`, queue g074, 41 TUs / 64 asks /
// 16 emitted placeholder sites.  THE receiver of f461 and the cast target in
// every one of the 16 sites.
using t560 = llvm::simple_ilist<mlir::Block>;

// t561 -- `llvm::iplist<mlir::Block>`, 32 asks / 9 emitted placeholder sites.
// NOT optional: it is the RETURN type f460 is spelled with, so without it f460
// cannot resolve at all.  Same body as t560 -- the same container.
using t561 = llvm::iplist<mlir::Block>;
