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

} // namespace llvm

using t1 = llvm::LogicalResult;
using t2 = llvm::hash_code;
template <typename T1> using t3 = llvm::FailureOr<T1>;

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
