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

} // namespace llvm

using t1 = llvm::LogicalResult;
using t2 = llvm::hash_code;

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
