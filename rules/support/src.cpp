// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::LogicalResult and llvm::hash_code -- two tiny LLVM value wrappers from
// llvm/Support/ and llvm/ADT/ that are neither containers nor MLIR IR types.
//
// Declared in THIS toolchain (LLVM-22.1.3) at:
//   * llvm/Support/LogicalResult.h:25  struct [[nodiscard]] LogicalResult
//     (mlir/Support/LogicalResult.h is a forwarding stub that only does
//      `using llvm::LogicalResult;`, which is why the converter reports the key
//      as `llvm::LogicalResult` even at `mlir::LogicalResult` spellings)
//   * llvm/ADT/Hashing.h:76            class hash_code
//
// MODEL
//   llvm::LogicalResult -> bool   (the class holds exactly one `bool IsSuccess`
//                                  and has no state beyond it; `success()` is
//                                  true, `failure()` is false)
//   llvm::hash_code     -> u64    (the class holds exactly one `size_t value`)
// Both models are MODEL-INDEPENDENT scalars, so tgt_unsafe.rs and
// tgt_refcount.rs are identical.
//
// WHY THE DECLARATIONS ARE RESTATED AND NOT #included: same reason as
// rules/stringref, rules/raw_ostream and rules/twine -- cpp-rule-preprocessor
// compiles this file with a fixed flag set that cannot reach an LLVM tree.
//
// NOT COVERED, deliberately:
//   * llvm::FailureOr<T> (llvm/Support/LogicalResult.h:74) -- derives from
//     std::optional<T> and is a different type with a different model.
//   * hash_combine / the hash_value overload family over pairs, tuples and
//     pointers -- no site in the target corpus calls them.
//   * LogicalResult's copy-ASSIGNMENT -- the model is a scalar, so a rule
//     returning `LogicalResult&` would have to be `&mut bool`; no site in the
//     corpus needs it (`LogicalResult r = success();` is initialisation).

#include <optional>
#include <utility>

namespace llvm {

// llvm/Support/LogicalResult.h:25
struct LogicalResult {
public:
  // llvm/Support/LogicalResult.h:29 -- static LogicalResult success(bool = true)
  static LogicalResult success(bool IsSuccess = true);
  // llvm/Support/LogicalResult.h:35 -- static LogicalResult failure(bool = true)
  static LogicalResult failure(bool IsFailure = true);
  // llvm/Support/LogicalResult.h:39 -- constexpr bool succeeded() const
  bool succeeded() const;
  // llvm/Support/LogicalResult.h:42 -- constexpr bool failed() const
  bool failed() const;

private:
  LogicalResult(bool IsSuccess);
  bool IsSuccess;
};

// llvm/Support/LogicalResult.h:54 -- inline LogicalResult success(bool = true)
LogicalResult success(bool IsSuccess = true);
// llvm/Support/LogicalResult.h:61 -- inline LogicalResult failure(bool = true)
LogicalResult failure(bool IsFailure = true);
// llvm/Support/LogicalResult.h:66 -- inline bool succeeded(LogicalResult)
bool succeeded(LogicalResult Result);
// llvm/Support/LogicalResult.h:70 -- inline bool failed(LogicalResult)
bool failed(LogicalResult Result);

// llvm/ADT/Hashing.h:76
class hash_code {
  unsigned long value;

public:
  // llvm/ADT/Hashing.h:82 -- hash_code() = default
  hash_code() = default;
  // llvm/ADT/Hashing.h:85 -- hash_code(size_t value)
  hash_code(unsigned long value);
  // llvm/ADT/Hashing.h:88 -- operator size_t() const
  operator unsigned long() const;
  // llvm/ADT/Hashing.h:90 -- friend bool operator==(const hash_code &, const hash_code &)
  friend bool operator==(const hash_code &lhs, const hash_code &rhs);
  // llvm/ADT/Hashing.h:93 -- friend bool operator!=(const hash_code &, const hash_code &)
  friend bool operator!=(const hash_code &lhs, const hash_code &rhs);
  // llvm/ADT/Hashing.h:98 -- friend size_t hash_value(const hash_code &)
  friend unsigned long hash_value(const hash_code &code);
};

// The three above are hidden friends in LLVM (only findable by ADL); restated
// at namespace scope so the qualified calls in f12-f14 below name them.  The
// signature a rule matches on is identical either way.
bool operator==(const hash_code &lhs, const hash_code &rhs);
bool operator!=(const hash_code &lhs, const hash_code &rhs);
unsigned long hash_value(const hash_code &code);

// llvm/Support/LogicalResult.h:74 --
//   template <typename T> class [[nodiscard]] FailureOr : public std::optional<T>
// It PUBLICLY DERIVES FROM std::optional<T> and adds no data member of its own,
// which is the whole reason the model below is `Option<T1>` and not
// `Result<T1, ()>`: every reader the corpus uses (`operator*`, `operator->`,
// `value()`, `value_or`) is INHERITED and keys against
// `std::optional<T1>::...`, which rules/optional already maps onto `Option<T1>`
// (its t1/t2/t3 all target `Option<T1>`).  A `Result` model would type-mismatch
// every one of those inherited keys.
//
// The two `FailureOr`-specific members ARE mapped, because without them the
// abort just moves one call along:
//   * the LogicalResult constructor -- `return failure();` in a
//     FailureOr-returning function goes through it, and the class ASSERTS
//     `failed(Result)`, so `None` is the faithful body, not a lost branch;
//   * `operator LogicalResult() const { return success(has_value()); }` -- this
//     is how `succeeded(fo)`/`failed(fo)` work at all, since those free
//     functions take a LogicalResult by value.  It is t1, i.e. `bool`, so the
//     body is `is_some()`.
// `succeeded`/`failed`/`success`/`failure` themselves are ALREADY modelled here
// as f1-f8, so the failure monad is complete rather than half-mapped.
//
// NOT restated: the private `using std::optional<T>::operator bool;` /
// `has_value;` hiding (a rule cannot call an inaccessible member anyway) and
// the converting `FailureOr(const FailureOr<U> &)` template (no corpus site
// converts between two different payload types).
template <typename T> class FailureOr : public std::optional<T> {
public:
  FailureOr();
  FailureOr(LogicalResult Result);
  FailureOr(T &&Y);
  FailureOr(const T &Y);
  FailureOr(const FailureOr<T> &Other);
  operator LogicalResult() const;
};

// llvm/Support/LogicalResult.h:119 --
//   class [[nodiscard]] ParseResult : public LogicalResult {
//   public:
//     ParseResult(LogicalResult Result = success()) : LogicalResult(Result) {}
//     constexpr explicit operator bool() const { return failed(); }
//   };
//
// It holds NO state of its own: the one data member is LogicalResult's
// inherited `bool IsSuccess`, which t1 above already models as `bool`.  So
// t4 -> bool is the same honest scalar model, not a new one.
//
// WHY THE SEARCHED SPELLING IS `llvm::ParseResult` AND NOT `mlir::ParseResult`:
// exactly the mechanism lines 8-11 above document for LogicalResult --
// mlir/Support/LogicalResult.h:21 and mlir/Support/LLVM.h:163 are both
// `using llvm::ParseResult;`, so every `mlir::ParseResult` site in the corpus
// reports the `llvm::` key.
//
// ⛔ ONLY TWO MEMBERS ARE ParseResult's OWN -- the converting constructor and
// `operator bool`.  `succeeded()`/`failed()` are INHERITED from LogicalResult,
// and a key on a DERIVED class CANNOT relocate an INHERITED member: that is
// precisely why the six `llvm::FailureOr` reader keys described below were all
// DEAD.  No `succeeded`/`failed` key is written here; those sites key as
// `llvm::LogicalResult::succeeded/failed` and f1/f2 already answer them.
//
// ⭐ `operator bool` IS INVERTED.  It returns `failed()`, i.e. `!IsSuccess`,
// NOT `succeeded()`.  Under the true-means-success model of t1/t4 the body must
// therefore be `!a0`.  A body of `a0` renders the corpus's dominant shape
// `if (parser.parseFoo()) return failure();` with the branch INVERTED and still
// type-checks and still compiles -- silent wrongness of the worst kind.  There
// are hundreds of such sites (dcc/src/Dialect/Sentient/SentientOps.cpp alone has
// :868, :873, :886, :929, :934, :1220, :1232-1237, ...).
//
// NOT COVERED, deliberately: `mlir::OptionalParseResult`
// (mlir/IR/OpDefinition.h:40).  It is a DISTINCT type that holds a real member
// of its own, `std::optional<ParseResult> impl`, so its model is `Option<bool>`
// rather than `bool`, and all five of its members (4 ctors, has_value, value,
// operator*) are its OWN rather than inherited.  It is keyable in principle but
// its spelling is `mlir::`, which is rules/mlir's territory, not this module's.
// Left open on purpose.
class ParseResult : public LogicalResult {
public:
  // llvm/Support/LogicalResult.h:122
  ParseResult(LogicalResult Result = success());
  // llvm/Support/LogicalResult.h:126 -- constexpr explicit operator bool() const
  explicit operator bool() const;
};

// llvm/Support/SMLoc.h:21 --
//   class SMLoc {
//     const char *Ptr = nullptr;
//   public:
//     SMLoc() = default;
//     bool isValid() const { return Ptr != nullptr; }
//     bool operator==(const SMLoc &RHS) const { return RHS.Ptr == Ptr; }
//     bool operator!=(const SMLoc &RHS) const { return RHS.Ptr != Ptr; }
//     const char *getPointer() const { return Ptr; }
//     static SMLoc getFromPointer(const char *Ptr) { ... }
//   };
//
// MODEL: an OPAQUE POINTER-SHAPED HANDLE.  An SMLoc *is* a bare
// `const char *` into a MemoryBuffer owned by a SourceMgr, and the ONE data
// member is that pointer, so `*const u8` (unsafe) / `Ptr<u8>` (refcount) is the
// class's actual representation, not a stand-in.  Nothing here reads through
// it.
//
// WHY THE TYPE IS SAFE TO KEY WHILE EVERY MEMBER IS REFUSED -- 25 corpus sites,
// grep re-run and re-confirmed 2026-09-28, and EVERY ONE is opaque
// pass-through:
//   * 12 default-constructed nulls handed straight to
//     `SourceMgr::AddNewSourceBuffer` (dxp/util.cpp:154,
//     dxp/tools/DxpOptMain.cpp:199, ddc/ddl/ddl.cpp:67,
//     dcc/tools/dcc-standalone/dcc-standalone-main.cpp:1099 and :1284,
//     dcc/src/Driver/dcc.cpp:86, hcc/tools/hcc-standalone/...:483 and :663,
//     dataflow-scheduler/tools/.../dataflow-scheduler-main.cpp:214,
//     dr5/tools/dr5-driver-lib/dr5-opt-main.cpp:267 and :449, ...);
//   * 9 `::llvm::SMLoc xOperandsLoc;` locals in tablegen'd `*Ops.cpp` parsers
//     (ddc/ddl/Dialect/DdlOps.cpp:225/229/233, dcc/src/Dialect/Trace/
//     TraceOps.cpp:67, dcc/src/Dialect/Sentient/SentientOps.cpp:1223/1226,
//     dataflow-scheduler/.../DataflowOps.cpp:101, ...) -- stored, then handed
//     to a diagnostic;
//   * 4 UNUSED lambda parameters
//     (.../Dialect/Dataflow/DataflowTypes.cpp:68/73/78/82).
// ZERO dereferences, zero `getPointer()`, zero `isValid()`.  The only
// comparison anywhere in the corpus is the single `==` site behind
// `mlir::AsmParser`.
//
// ⛔ EVERY MEMBER IS REFUSED, AND THE REFUSAL IS WHAT MAKES THE TYPE HONEST:
//   * `operator==` / `operator!=` (queue g837) -- these are POINTER equality:
//     two SMLocs are equal iff they point at the SAME BYTE of the same
//     MemoryBuffer.  Its one call site is inside an `mlir::AsmParser` parse
//     body, squarely behind the standing `llvm::SourceMgr` refusal (the parser
//     reads the buffer back out, and we do not model the buffer).  A
//     content-comparing or owned-value model would make DISTINCT LOCATIONS
//     COMPARE EQUAL -- silent wrongness.  The pointer-shaped model above at
//     least keeps distinct locations distinct, but no `==` key is written.
//   * `getPointer()` returns THE ADDRESS -- `data()`'s and `front()`'s refusal
//     ground.  For SMLoc the address IS the value, which is exactly why the
//     type must stay opaque.
//   * `getFromPointer(const char *)` -- manufactures a location from an address
//     into a buffer we do not model; no corpus site calls it.
//   * `isValid()` -- no corpus site calls it, so any model of it would be
//     unfalsifiable.
//
// SWALLOW-SAFETY: `llvm::SMLoc` is ARITY 0 (no `<` anywhere in the spelling),
// so it can neither be swallowed nor swallow a sibling -- the
// `matchTemplate`-past-a-comma class cannot reach it.  It is exact-match-only.
class SMLoc {
  const char *Ptr = nullptr;

public:
  // llvm/Support/SMLoc.h:26 -- SMLoc() = default
  SMLoc();
};

} // namespace llvm

using t1 = llvm::LogicalResult;
using t2 = llvm::hash_code;
template <typename T1> using t3 = llvm::FailureOr<T1>;
using t4 = llvm::ParseResult;
using t5 = llvm::SMLoc;

// --- LogicalResult ---------------------------------------------------------

bool f1(llvm::LogicalResult a0) { return a0.succeeded(); }

bool f2(llvm::LogicalResult a0) { return a0.failed(); }

llvm::LogicalResult f3(bool a0) { return llvm::LogicalResult::success(a0); }

llvm::LogicalResult f4(bool a0) { return llvm::LogicalResult::failure(a0); }

llvm::LogicalResult f5(bool a0) { return llvm::success(a0); }

llvm::LogicalResult f6(bool a0) { return llvm::failure(a0); }

bool f7(llvm::LogicalResult a0) { return llvm::succeeded(a0); }

bool f8(llvm::LogicalResult a0) { return llvm::failed(a0); }

llvm::LogicalResult f9(llvm::LogicalResult a0) { return llvm::LogicalResult(a0); }

// --- hash_code -------------------------------------------------------------

llvm::hash_code f10(unsigned long a0) { return llvm::hash_code(a0); }

unsigned long f11(llvm::hash_code a0) { return a0.operator unsigned long(); }

bool f12(const llvm::hash_code &a0, const llvm::hash_code &a1) {
  return llvm::operator==(a0, a1);
}

bool f13(const llvm::hash_code &a0, const llvm::hash_code &a1) {
  return llvm::operator!=(a0, a1);
}

unsigned long f14(const llvm::hash_code &a0) { return llvm::hash_value(a0); }

llvm::hash_code f15(llvm::hash_code a0) { return llvm::hash_code(a0); }

// --- FailureOr -------------------------------------------------------------

template <typename T1> llvm::FailureOr<T1> f16() { return llvm::FailureOr<T1>(); }

template <typename T1> llvm::FailureOr<T1> f17(llvm::LogicalResult a0) {
  return llvm::FailureOr<T1>(a0);
}

template <typename T1> llvm::FailureOr<T1> f18(T1 &&a0) {
  return llvm::FailureOr<T1>(std::move(a0));
}

template <typename T1> llvm::FailureOr<T1> f19(const T1 &a0) {
  return llvm::FailureOr<T1>(a0);
}

template <typename T1>
llvm::FailureOr<T1> f20(const llvm::FailureOr<T1> &a0) {
  return llvm::FailureOr<T1>(a0);
}

template <typename T1>
llvm::LogicalResult f21(const llvm::FailureOr<T1> &a0) {
  return a0.operator llvm::LogicalResult();
}

// MEASURED, 2026-09-27: restating FailureOr WITHOUT the std::optional base and
// keying `operator*`/`operator->`/`value()` directly on `llvm::FailureOr<T1>`
// RECORDS SIX LIVE KEYS THAT CAN NEVER MATCH.  The rule file only fixes the
// SPELLING of a key; the converter reads the REAL llvm/Support/LogicalResult.h,
// where those members are declared by std::optional, so every reader keys as
// `std::optional<T1>::...` no matter what this file says.  The base must stay.
//
// What DID move the probe is directly below: the refcount model of FailureOr has
// to be the SAME Rust type rules/optional gives std::optional, i.e.
// `Option<Value<T1>>`, not `Option<T1>`.  Otherwise the derived-to-base
// conversion is between two DIFFERENT Rust types and rustc reports
// `non-primitive cast: Option<i64> as Option<Rc<RefCell<i64>>>`.

// --- llvm_unreachable ------------------------------------------------------
//
// llvm/Support/ErrorHandling.h:141 --
//   [[noreturn]] void llvm_unreachable_internal(const char *msg = nullptr,
//                                               const char *file = nullptr,
//                                               unsigned line = 0);
// This is the target of the `llvm_unreachable` MACRO, and the macro has TWO
// expansions:
//   * ErrorHandling.h:164, !NDEBUG            -> llvm_unreachable_internal(msg, __FILE__, __LINE__)
//   * ErrorHandling.h:167, NDEBUG w/o builtin -> llvm_unreachable_internal()
// MEASURED 2026-09-27: the DEFAULTED-ARGUMENT TRAP DOES **NOT** APPLY HERE, and
// this is the opposite of what it was briefed as.  Writing a second rule that
// calls the 0-arg form recorded the **IDENTICAL** key string
//   void llvm::llvm_unreachable_internal(const char *, const char *, unsigned int)
// because the recorder emits the CALLEE'S DECLARED signature, not the spelled
// argument list -- defaulted parameters are present in the declaration and so
// they are present in the key.  So there is exactly ONE key for both macro
// expansions, and adding an arity-0 sibling produces a DUPLICATE key string,
// after which `search` refuses as ambiguous and the caller emits a nonexistent
// fallback.  (Contrast `std::optional`'s copy-vs-move ctors, which are two
// DISTINCT declarations; a defaulted argument is one declaration.)
// 30+ reachable sites in PCFGToDataflowIR.cpp alone (:440, :1171, :1223-1229).
//
// `[[noreturn]]` buys NO exemption from the void-body rule -- the src body must
// still be spelled `return f(...);` -- but the RUST body may DIVERGE, i.e. a
// bare `panic!` that yields no initializer is accepted.  MEASURED.
//
// The parameters arrive as `*const u8` / `Ptr<u8>`, NOT as slices, so the
// message must be CONVERTED.  Formatting the pointer itself (`{:?}` on a0)
// would still abort and would still look like a pass, while printing an address
// instead of the diagnostic -- silent wrongness, not a style choice.
namespace llvm {
[[noreturn]] void llvm_unreachable_internal(const char *msg = nullptr,
                                            const char *file = nullptr,
                                            unsigned line = 0);
} // namespace llvm

void f22(const char *a0, const char *a1, unsigned a2) {
  return llvm::llvm_unreachable_internal(a0, a1, a2);
}

// --- ParseResult -----------------------------------------------------------
//
// The converting constructor.  `return success();` / `return failure();` /
// `return mlir::success();` inside a ParseResult-returning `parse()` all route
// through it, and both sides are the same scalar, so the body is the identity.

llvm::ParseResult f23(llvm::LogicalResult a0) { return llvm::ParseResult(a0); }

// ⭐ INVERTED: `operator bool` returns `failed()`.  See the class comment.

bool f24(llvm::ParseResult a0) { return a0.operator bool(); }

// --- llvm::SMLoc -----------------------------------------------------------
//
// The DEFAULT CONSTRUCTOR, and the ONLY member keyed.  12 of the 25 corpus
// sites are literally `SMLoc()` as an argument, and 9 more are
// `::llvm::SMLoc x;` default-init locals, so a type key with no constructor
// here is the measured rc=0-then-E0433 shape.  `SMLoc() = default` leaves
// `Ptr` at its `nullptr` NSDMI, so the honest body is a NULL pointer.
//
// Every other member is REFUSED -- see the class comment for each reason.

llvm::SMLoc f25() { return llvm::SMLoc(); }

// --- llvm::cast / dyn_cast / dyn_cast_or_null / isa -- REFUSED, AND WHY -----
//
// ⭐⭐ SUPERSEDED IN PART.  The blocker this block diagnoses -- `regularNameLookup`
// never searching the namespace written in the rule body -- IS NOW FIXED (see the
// f26/f27/f28 block at the tail of this file), and the three const-identity keys
// ARE LANDED.  Read the rest of this block for the derivation, which all still
// holds and which the keys rest on: the real `searched as:` spellings, the target
// type living in the RETURN position, `return a0;` being EXACT for `To == From`,
// and the two-mechanism swallow-safety proof.  ⛔ WHAT IS STILL TRUE AND STILL
// UNLANDED: `isa` (`To` is absent from its key -- all 1,428 sites collapse onto
// TWO key strings), non-identity `cast<A>(B)`, `dyn_cast<Op>(mlir::Operation *)`,
// and the non-const `T1 llvm::cast(T1 &)` slice.  Each reason below is unchanged.
//
// llvm/Support/Casting.h.  This four-function family is the LARGEST
// undefined-name row in the project.  A census of called-but-undefined
// `name_<N>` functions over 312 emitted .rs measured
//     cast              54,082 sites / 212 TUs / 781 distinct _N variants
//     dyn_cast_or_null  39,866 / 205 / 603
//     dyn_cast           2,773 / 185 / 607
//     isa                1,428 / 170 / 432
// ~98,000 sites, and because an undefined NAME stops rustc at NAME RESOLUTION,
// every type and borrow error behind one is unobservable -- which is why no
// whole-TU rustc run has ever been possible.
//
// NO KEY IS WRITTEN HERE.  The reason is a cpp-rule-preprocessor limitation,
// not a key-spelling problem, and it is recorded here so the next slot does not
// re-derive it.  Everything below is measured, 2026-09-29.
//
// ============================================================================
// 1. THE KEY SPELLINGS, READ OFF `-verbose` `search expr` LINES WITH grep -A1
// ============================================================================
//     mlir::StringAttr  llvm::cast(const mlir::StringAttr &)        result: None
//     mlir::IntegerAttr llvm::cast(mlir::Attribute &)               result: None
//     mlir::VectorType  llvm::dyn_cast(const mlir::VectorType &)    result: None
//     mlir::scf::ForOp  llvm::dyn_cast(mlir::Operation *)           result: None
//     mlir::StringAttr  llvm::dyn_cast_or_null(const mlir::StringAttr &)
//     bool              llvm::isa(mlir::Operation *const &)         result: None
//     bool              llvm::isa(const mlir::Attribute &)          result: None
//
// ⭐ FOR THE THREE CAST FUNCTIONS THE TARGET TYPE `To` IS IN THE RETURN
// POSITION, exactly as for rules/tuple's `std::get` (f10/f11): the explicit
// template argument does not reach the key, but the deduced return type does.
// So `cast<A>(x)` and `cast<B>(x)` record DIFFERENT keys and ARE separable.
//
// ⛔ `isa` IS NOT SEPARABLE, AND THAT IS A HARD MEASURED FACT.  `isa<To>(x)`
// returns `bool`, so `To` appears NOWHERE in its key.  All 1,428 sites across
// 170 TUs collapse onto just TWO key strings -- one for the Operation hierarchy
// and one for the Attribute hierarchy -- so a single body would have to answer
// `isa<ForOp>(op)` and `isa<WhileOp>(op)` identically.  That is silent
// wrongness, so the loud (if invisible) miss is kept deliberately.  ⭐ Closing
// `isa` requires a CONVERTER change that puts the explicit template argument
// list into the recorded key.  It cannot be closed by any rule.
//
// ============================================================================
// 2. THE SLICE THAT WOULD HAVE BEEN SAFE: `To == From`
// ============================================================================
// The same `-verbose` census shows the IDENTITY shape is DOMINANT -- it is the
// top entry of every log examined:
//     48  mlir::StringAttr llvm::dyn_cast_or_null(const mlir::StringAttr &)
//     32  mlir::StringAttr llvm::cast(const mlir::StringAttr &)
//     24  mlir::{IntegerAttr,DictionaryAttr,AffineMapAttr,IntegerSetAttr,
//             ArrayAttr} llvm::cast(const <the same type> &)
//     ... plus mlir::Value, mlir::Attribute, mlir::VectorType,
//         mlir::detail::DenseArrayAttrImpl<int64_t>
// These come from tablegen'd accessors and from `cast<T>` applied to something
// already statically a `T`.
//
// ⭐ AND IT IS THE ONE SLICE WITH NO SEMANTIC GAP AT ALL, so `return a0;` would
// be the WHOLE function rather than an approximation of a downcast:
//   * `cast<T>(x : T)` asserts `isa<T>(x)`, a TAUTOLOGY when To == From, then
//     returns x.
//   * `dyn_cast<T>(x : T)` returns x when `isa<T>(x)` and the empty value
//     otherwise.  `isa<T>(x : T)` holds, so THE FAILURE BRANCH IS UNREACHABLE
//     -- there is no failure path to preserve, because such a key cannot match
//     a cast that is able to fail.
//   * `dyn_cast_or_null<T>(x : T)` is x when x is non-null and the empty value
//     when x is null -- and when To == From THE EMPTY VALUE IS x.  Exact on
//     BOTH paths.
// A body that panicked, or that produced anything other than a0, would be the
// wrong one.  Both models agree: a `const T1 &` parameter lowers to `&T1` and a
// by-value `T1` return stays `T1` (cf. rules/functional f4), so tgt_unsafe.rs
// and tgt_refcount.rs would be byte-identical.
//
// ⭐ AND THE GENERIC SPELLING `T1 llvm::cast(const T1 &)` IS SWALLOW-SAFE.  Two
// mechanisms in matchTemplate (mapper.cpp:547) make it exact:
//   1. THE REPEATED PLACEHOLDER IS CHECKED LITERALLY -- mapper.cpp:711-717.  On
//      the SECOND occurrence of T1 `repl.has_value()` is true, so the code takes
//      the `matchLiteralAt(instantiated, si, *repl)` branch and returns nullopt
//      on mismatch.  `mlir::IntegerAttr llvm::cast(mlir::Attribute &)` therefore
//      captures T1 = `mlir::IntegerAttr` at the return position and then FAILS
//      against `mlir::Attribute`.  ⛔ A NON-IDENTITY CAST CANNOT REACH THE KEY,
//      so the over-match that rules/tuple f10 tolerates cannot happen here.
//   2. THE FIRST CAPTURE IS ANCHORED ON A UNIQUE LITERAL -- T1's `nextLit` is
//      the whole run ` llvm::cast(const `, and findNextLiteralSameDepth stops at
//      its first base-depth occurrence.  That substring occurs exactly once in
//      any instantiated signature, so the capture is the return type and cannot
//      swallow into the parameter list.
// Angle depth is safe too: every observed `To` is balanced (including
// `mlir::detail::TypedValue<mlir::IndexType>`), and there is no
// `operator<`/`operator>=` token anywhere in the spelling, so the
// MaskOperatorNameBrackets desync class cannot reach it either.
//
// ============================================================================
// 3. ⛔ WHY IT STILL CANNOT BE WRITTEN: cpp_rule_preprocessor.cpp:680-693
// ============================================================================
// `regularNameLookup` resolves the callee of a DEPENDENT call inside a rule
// TEMPLATE by looking the bare DeclarationName up in EXACTLY TWO scopes:
//     if (clang::NamespaceDecl *std_ns = sema_->getStdNamespace())
//       sema_->LookupQualifiedName(decls, std_ns);
//     if (decls.empty())
//       sema_->LookupQualifiedName(decls, sema_->Context.getTranslationUnitDecl());
// namespace `std`, then the GLOBAL namespace.  `LookupQualifiedName` on the TU
// decl does NOT descend into nested namespaces, and the nested-name-specifier
// actually written in the rule body is discarded.  ⛔ SO `namespace llvm` IS
// NEVER SEARCHED, and every `llvm::` FUNCTION TEMPLATE is unreachable from a
// rule template.  The failure is `No viable function` followed by a SEGFAULT on
// the `assert(0 && "Rule resolution failed")` path at :887 -- and note that
// regen-rule.sh core-dumps there with EXIT 0, so a caller that does not check
// for the `OK <module> -> <dir>` line reads it as success.
//
// MEASURED MATRIX (each row a separate cpp-rule-preprocessor run):
//   OK    #include <algorithm>; const T1 &f(const T1 &,const T1 &){return std::max(...);}
//                                            -> const T1 & std::max(const T1 &, const T1 &)
//   OK    #include <utility>;   void f(T1 &,T1 &){return std::swap(...);}
//   OK    global-namespace template: T1 f(const T1 &a0){return gident<T1>(a0);}
//                                            -> T1 gident(const T1 &)
//   OK    NON-template rule, llvm:: template callee:
//         int f(const int &a0){return llvm::cast<int>(a0);}
//                                            -> int llvm::cast(const int &)
//   FAIL  template rule, restated `namespace llvm` template, qualified call
//   FAIL  ... with <T1, T1> instead of <T1>
//   FAIL  ... with ::llvm::cast
//   FAIL  ... with a single-parameter `template <typename To> To cast(const To &)`
//   FAIL  ... with `using llvm::cast;` + unqualified call
//   FAIL  ... with `using llvm::cast;` + ::cast
//   FAIL  ... with an `inline namespace` inside llvm
//   FAIL  ... with the declaration moved into a local header
//   FAIL  ... with that header reached through -isystem (a SYSTEM header)
//   FAIL  ... with the REAL #include <llvm/Support/Casting.h>
// ⭐ The boundary is exactly "dependent call inside a rule TEMPLATE": a
// NON-template rule resolves `llvm::cast` fine, because clang resolves its
// non-dependent call at parse time and this lookup machinery never runs.  That
// is also why f5 (`llvm::success`) and f22 (`llvm::llvm_unreachable_internal`)
// above work -- both are non-template rules.
//
// ⛔ AND NO KEY SPELLING ESCAPES IT.  Moving `cast` to the global namespace
// makes it resolve but records `T1 cast(const T1 &)`, which does not match the
// corpus key; putting it in `std` records `T1 std::cast(const T1 &)`.  The key
// string follows the RESOLVED DECL's qualified name, so the namespace the
// preprocessor must search is the namespace the key must name.
//
// ⭐ THE CONVERTER ASK, one function, and it unblocks ~96,600 of the ~98,000
// sites at once (everything except `isa`):
//   FILE      cpp2rust/cpp_rule_preprocessor.cpp
//   FUNCTION  regularNameLookup, :680
//   CHANGE    thread the call's nested-name-specifier (the CXXScopeSpec /
//             NestedNameSpecifierLoc already present on the dependent
//             DeclRefExpr that lookupCallee is re-resolving) into a
//             sema_->LookupQualifiedName(decls, <that namespace>) BEFORE the
//             std/TU fallbacks, so a qualified dependent call resolves in the
//             namespace it names.
//   TEST      a scratch module whose src.cpp is
//               namespace llvm { template <typename To, typename From>
//                                To cast(const From &Val); }
//               template <typename T1> T1 f1(const T1 &a0) {
//                 return llvm::cast<T1>(a0); }
//             must record `T1 llvm::cast(const T1 &)` instead of printing
//             `No viable function` and core-dumping.
//   SECOND, INDEPENDENT FIX: :887 `assert(0 && "Rule resolution failed")` should
//             be a diagnostic + nonzero exit.  It currently core-dumps while
//             regen-rule.sh returns 0.
//
// ============================================================================
// 4. THE TWO NON-IDENTITY CLASSES, REFUSED FOR THEIR OWN REASONS
// ============================================================================
//   * `mlir::IntegerAttr llvm::cast(mlir::Attribute &)` and friends are real
//     DOWNCASTS in an MLIR type hierarchy.  A generic `T1 llvm::cast(const T2 &)`
//     binds T1/T2 independently and its body would have to manufacture a T1 from
//     a T2; there is no cast-free, transmute-free projection that does that.  For
//     `dyn_cast`/`dyn_cast_or_null` the body would ALSO have to be able to FAIL,
//     and a checked downcast collapsed to an unchecked one is wrong in a way no
//     compile and no probe would show.
//   * `mlir::scf::ForOp llvm::dyn_cast(mlir::Operation *)` HAS `To` in the return
//     position, and `OpInst::is_a::<T>()` (dataflowir-gen src/isa.rs) is exactly
//     the right discriminator -- but the VALUE the `true` branch must yield is a
//     ported op-wrapper struct with no Rust model.  The generated `MlirOp`
//     markers are ZERO-SIZED marker types, usable only as the type argument of
//     `is_a`, never as a returned op value.  Refused until the op wrappers have
//     a model.
//
// ⭐ ALSO NOT KEYED, once the preprocessor is fixed: the NON-CONST lvalue
// overload `T1 llvm::cast(T1 &)` (~1/3 of identity sites).  A non-const `T1 &`
// parameter lowers to `&mut T1` in the unsafe model but to `Ptr<T1>` in refcount
// (rules/algorithm f9), and reading a value back out of a `Ptr<T1>` needs a
// `ByteRepr` bound the MLIR handle types are not known to satisfy -- that would
// trade this row's E0425 for an E0277 at every site.  Start with the const
// overload, which carries the majority of the mass and is identical in both
// models.

// --- llvm::cast / dyn_cast / dyn_cast_or_null -- THE IDENTITY SLICE ---------
//
// llvm/Support/Casting.h.  The largest undefined-name row in the project:
// 54,082 `cast` sites over 212 TUs, 39,866 `dyn_cast_or_null` over 205, 2,773
// `dyn_cast` over 185 (census of 312 emitted .rs, 2026-09-29).  Only the
// `To == From` slice is keyed here; the derivation of what is and is not safe is
// in the block that follows f28.
//
// ⛔ THIS BLOCK WAS UNWRITABLE UNTIL cpp_rule_preprocessor.cpp's
// `regularNameLookup` learned to search the namespace the rule body NAMES.  It
// searched `std` and the global namespace only, so `llvm::` was discarded and a
// DEPENDENT call inside a rule TEMPLATE could not reach any `llvm::` function
// template -- `No viable function`, then a SIGSEGV.  That is why f5 and f22
// above, which are NON-template rules resolved by clang at parse time, worked
// the whole time while nothing templated in `namespace llvm` did.

namespace llvm {
template <typename To, typename From> To cast(const From &Val);
template <typename To, typename From> To dyn_cast(const From &Val);
template <typename To, typename From> To dyn_cast_or_null(const From &Val);
} // namespace llvm

template <typename T1> T1 f26(const T1 &a0) { return llvm::cast<T1>(a0); }

template <typename T1> T1 f27(const T1 &a0) { return llvm::dyn_cast<T1>(a0); }

template <typename T1> T1 f28(const T1 &a0) {
  return llvm::dyn_cast_or_null<T1>(a0);
}

// ⭐ THE TARGET TYPE `To` IS IN THE RETURN POSITION, so the three keys read
//     T1 llvm::cast(const T1 &)
//     T1 llvm::dyn_cast(const T1 &)
//     T1 llvm::dyn_cast_or_null(const T1 &)
// and a corpus site keyed `mlir::StringAttr llvm::cast(const mlir::StringAttr &)`
// matches while `mlir::IntegerAttr llvm::cast(mlir::Attribute &)` does not.  Same
// mechanism as rules/tuple f10/f11 on `std::get`: the explicit template argument
// does not reach the key, the deduced return type does.
//
// ⭐ `return a0;` IS EXACT HERE, NOT AN APPROXIMATION.  For To == From:
//   * `cast<T>(x : T)` asserts `isa<T>(x)` -- a tautology -- then returns x.
//   * `dyn_cast<T>(x : T)` returns x when `isa<T>(x)` and the empty value
//     otherwise.  `isa<T>(x : T)` holds, so THE FAILURE BRANCH IS UNREACHABLE.
//     A key of this shape cannot match a cast that is able to fail, so no
//     checked downcast is being silently collapsed into an unchecked one.
//   * `dyn_cast_or_null<T>(x : T)` is x when x is non-null and the empty value
//     when x is null -- and when To == From THE EMPTY VALUE IS x.  Exact on BOTH
//     paths.
// A body that panicked, or that produced anything but a0, would be the wrong one.
//
// ⭐ SWALLOW-SAFE, by two independent mechanisms in matchTemplate:
//   1. THE REPEATED PLACEHOLDER IS CHECKED LITERALLY (mapper.cpp:711-717).  On
//      the SECOND occurrence of T1 `repl.has_value()` is true, so the code takes
//      the `matchLiteralAt` branch and returns nullopt on mismatch.
//      `mlir::IntegerAttr llvm::cast(mlir::Attribute &)` therefore captures
//      T1 = mlir::IntegerAttr at the return position and then FAILS against
//      mlir::Attribute.  The rules/tuple f10 over-match cannot happen here.
//   2. THE FIRST CAPTURE IS ANCHORED ON A UNIQUE LITERAL: T1's nextLit is the
//      whole run ` llvm::cast(const `, which occurs exactly once in any
//      instantiated signature, so the capture is the return type and cannot
//      swallow into the parameter list.
// The `operator>=` angle-depth desync class cannot reach it either: there is no
// operator name anywhere in the spelling.
//
// Both models are IDENTICAL: `const T1 &` lowers to `&T1` and a by-value `T1`
// return stays `T1` (cf. rules/functional f4), so the two overlays agree.  The
// `T1: Clone` bound is the same one f19/f20 above already carry; every type
// observed at these sites (ir::Attr, ir::Ty, ir::Value, ir::AffineMap) derives
// Clone.
//
// ⛔ THREE THINGS DELIBERATELY NOT KEYED HERE.
//   * `isa` -- `isa<To>(x)` returns bool, so `To` is NOWHERE in its key and all
//     1,428 sites over 170 TUs collapse onto TWO key strings.  One body would
//     have to answer `isa<ForOp>` and `isa<WhileOp>` identically.  Closing it
//     needs the explicit template-argument list in the recorded key -- a
//     CONVERTER change, not a rule.
//   * The NON-IDENTITY casts.  A generic `T1 llvm::cast(const T2 &)` binds T1
//     and T2 independently and its body would have to manufacture a T1 from a
//     T2; there is no cast-free, transmute-free projection that does that.  And
//     `mlir::scf::ForOp llvm::dyn_cast(mlir::Operation *)` has the right
//     discriminator in `OpInst::is_a::<T>()` but no value for the `true` branch
//     to yield: the generated MlirOp markers are ZSTs, usable only as that type
//     argument.
//   * The NON-CONST lvalue overload `T1 llvm::cast(T1 &)` (~1/3 of identity
//     sites).  A non-const `T1 &` lowers to `&mut T1` in the unsafe model but to
//     `Ptr<T1>` in refcount (cf. rules/algorithm f9), and reading a value back
//     out of a `Ptr<T1>` needs a `ByteRepr` bound the MLIR handle types are not
//     known to satisfy -- it would trade this row's E0425 for an E0277 at every
//     site.  The const overload carries the majority of the mass and is
//     identical in both models, so it goes first.
