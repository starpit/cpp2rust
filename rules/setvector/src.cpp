// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::SetVector -- a set with INSERTION-ORDER iteration
// (llvm/ADT/SetVector.h:57).
//
// THE KEY IS 4-ARY, AND THE CORPUS NEVER SPELLS `llvm::SetVector` AT ALL.
// ---------------------------------------------------------------------
// SetVector.h:55-57 declares
//   template <typename T, typename Vector = SmallVector<T, 0>,
//             typename Set = DenseSet<T>, unsigned N = 0> class SetVector;
// so every instantiation carries FOUR arguments and the key must be
// `llvm::SetVector<T1, T2, T3, _>`.  A 1-ary `llvm::SetVector<T1>` would
// SWALLOW THE TAIL: GetTypeMapKey (mapper.cpp:94) truncates the bucket key at
// `<`, so arity is not part of the bucket, and matchTemplate scans to the FINAL
// depth-0 `>` -- T1 would bind to the whole `T, Vector, Set, N` list and the
// model would be `Vec<that mess>`.  The `unsigned N` prints as `_` because
// mapper.cpp:1208 rewrites `\b\d+\b` -> `_` on both sides of the match, the
// same elision rules/smallvector and rules/array rely on.
//
// Every SetVector site in this corpus (enumerated with `grep -rn SetVector`
// over all of dt_src -- seven, all of them SmallSetVector, zero bare
// SetVector) is:
//   dataflow-scheduler/lib/Transforms/DoubleBuffering.cpp
//     :196,:197  llvm::SmallSetVector<mlir::ktdf::StageOp, 4> producers/consumers
//     :243       llvm::SmallSetVector<mlir::ktdf::StageOp, 8> visited
//   dataflow-scheduler/lib/Transforms/ParallelizeLoopsAcrossInstances.cpp
//     :94        llvm::SmallSetVector<mlir::ktdf_arch::GroupOp, 4> applicable_groups
//     :172,:173  the same type, returned by value from getEquivalenceClass
// and SetVector.h:339 is
//   template <typename T, unsigned N>
//   class SmallSetVector : public SetVector<T, SmallVector<T, N>, DenseSet<T>, N>
// -- exactly the 4-ary argument list the census recorded.  So `insert`,
// `contains`, `begin` and `end` are all INHERITED: the callee's DECLARING class
// is SetVector, the recorder emits the callee's declared signature, and a rule
// keyed on SmallSetVector::insert would read back FOUND from the rule tree and
// be DEAD at every call site -- the measured `llvm::FailureOr` shape, six keys
// FOUND all six DEAD.  THE MEMBER RULES BELOW ARE KEYED ON THE BASE.
//
// The TWO type keys, by contrast, are both needed and are not redundant:
//   * t1 `llvm::SetVector<T1, T2, T3, _>` is the receiver type of every member
//     rule and is the spelling the census names as the first-abort;
//   * t2 `llvm::SmallSetVector<T1, _>` is the type the corpus VARIABLES are
//     DECLARED with.  Without it the declarations themselves are unmapped.
// t2's class below deliberately does NOT restate the member functions: a
// derived-class member rule is precisely the dead key described above.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/ADT/SetVector.h>
// --------------------------------------------------------------------------
// Same reason as rules/twine, rules/apint, rules/smallvector and
// rules/raw_ostream: cpp-rule-preprocessor compiles this file with a fixed flag
// set, and the only flags that would reach LLVM's real headers are absolute -I
// paths into whatever LLVM tree the target project happens to have built.  A
// rule matches on a signature STRING, so the restatement is faithful iff it
// agrees with LLVM exactly; each member carries the header line it came from.
//
// Because matching is on the signature STRING and not on the class body, the
// base-class argument list of the local SmallSetVector is irrelevant to keying
// (only `llvm::SmallSetVector<T1, _>` is recorded); it is written as
// `SetVector<T, T, T, N>` rather than dragging local SmallVector/DenseSet
// restatements in, and that substitution cannot change any key.
//
// MODEL: `Vec<T1>` -- A VEC, NOT A HashSet, BECAUSE THE ORDER IS OBSERVABLE
// ------------------------------------------------------------------------
// A SetVector is unique elements in INSERTION ORDER; the vector_ member is the
// iteration order and set_ is only a membership index (header:12-13, "a set
// that has insertion order iteration characteristics ... useful for keeping a
// set of things that need to be visited later but in a deterministic order").
// Both corpus uses depend on that order: DoubleBuffering.cpp:219 copies
// `producers.begin(), producers.end()` straight into a SmallVector that is
// returned to the caller, and :243's `visited` drives a BFS.  A HashSet model
// would be silently wrong in exactly the way the header exists to prevent, so
// the model is `Vec<T1>` and `insert` is a MEMBERSHIP-TESTED push, which is
// also what LLVM itself does on the small path (header:157-161,
// `if (!llvm::is_contained(vector_, X)) vector_.push_back(X)`).
//
// The membership test is `contains`/`==`, i.e. `T1: PartialEq`, not `Hash`.
// That is the honest bound: SetVector's own small path is a LINEAR SCAN with
// `is_contained`, so equality is the only operation the C++ requires, and both
// element types here (mlir::ktdf::StageOp, mlir::ktdf_arch::GroupOp) are
// MLIR op handles with value equality.  O(n) insert instead of O(1) is a
// performance difference, never an observable one.
//
// MEMBERS KEYED -- ONLY THE FOUR THE SIX SITES REACH, read off those lines:
//   f1  SmallSetVector's implicit DEFAULT CTOR (the five `... producers;`
//       declarations).  Note this is the ONE key that must name the DERIVED
//       class: SetVector.h:341's `using SetVector<...>::SetVector;` inherits
//       the converting constructors but an inheriting-constructor declaration
//       never inherits a default constructor, so `SmallSetVector<T,4> v;` calls
//       SmallSetVector's own implicitly-declared default ctor, not SetVector's.
//       A type rule maps the TYPE ONLY -- without this the converter looks the
//       default ctor up as an ordinary expr rule, misses, and emits
//       `<mangled>::new()`, which does not exist: rc=0 then E0433.
//   f2  insert            DoubleBuffering.cpp:203,206,244; PLAI.cpp:173,177
//   f3  contains          DoubleBuffering.cpp:212
//   f4  begin             DoubleBuffering.cpp:219,221 (and every range-for)
//   f5  end               DoubleBuffering.cpp:219,221 (and every range-for)
//
// DELIBERATELY NOT KEYED, because no site in this corpus reaches them and a key
// nobody reaches is a key nobody has checked -- each would abort loudly rather
// than be silently wrong: the iterator-pair and `from_range_t` constructors
// (header:80-88), getArrayRef/takeVector (header:91-96), empty/size
// (header:100-104), the const begin/end overloads and all four reverse
// iterators (header:112-130), front/back/operator[] (header:133-147), the
// range and `insert_range` overloads of insert (header:169-176), remove /
// remove_if / erase / set_union / set_subtract (header:180ff), count, clear,
// pop_back, pop_back_val, operator==/!=, swap, and the whole of
// std::swap (header:350-364).  The const begin()/end() pair is the one most
// likely to be needed next: it is a DISTINCT key from f4/f5 (same name,
// `const` receiver) and this corpus only ever iterates a mutable SetVector.

// The four ELEMENT types the census actually searches with, restated as empty
// tags for the same reason rules/mlir restates its element types that way: only
// the fully-qualified NAME reaches the key, never the body.  No key is recorded
// for any of them here -- `mlir::Attribute` and `mlir::Operation` are keyed by
// rules/mlir (t6 and t1) and the two op classes are keyed by dataflowir-gen.
namespace mlir {
class Attribute {};
class Operation {};
namespace ktdf {
class StageOp {};
} // namespace ktdf
namespace ktdf_arch {
class GroupOp {};
} // namespace ktdf_arch
} // namespace mlir

namespace llvm {

// Restated ONLY so the concrete SetVector keys below can be spelled with the
// exact argument text the converter searches with.  No key is recorded for any
// of these three (a key comes from a `using tN` / `fN`, never from a class
// declaration), and each is the minimum shape that makes the spelling legal.
template <typename T> struct DenseMapInfo;                 // llvm/ADT/DenseMap.h
template <typename T, unsigned N = 1> class SmallVector {}; // SmallVector.h:1204
// (N=1 is the same deliberate SENTINEL default as SetVector's: the searched text
// spells the small-size, so writing 0 must not be elided as "the default".)
template <typename T, typename ValueInfoT = DenseMapInfo<T>>
class DenseSet {};                                          // DenseSet.h:279

// Restated from llvm/ADT/SetVector.h:55-57.  The member layout is irrelevant
// to signature matching; the two real members (set_, vector_) are omitted.
// THE DEFAULTS ARE NOW CARRIED (they were dropped before, which forced every
// spelling to be 4-ary); see the arity section of the header comment.
// MEASURED 2026-09-28: the rule-side recorder DROPS DEFAULTED ARGUMENTS, so a
// spelling that matches the declared defaults collapses to 1-ary in the key.
// The census searches this type BOTH 1-ary and 4-ary, so the two spellings must
// be two distinct recorded keys -- which is only possible if the 4-ary spelling
// is NOT the default.  Hence the defaults below are deliberate SENTINELS that no
// searched spelling ever uses: a bare `llvm::SetVector<T>` still records 1-ary
// (defaults dropped), and the explicit
// `<T, SmallVector<T, 0>, DenseSet<T>, 0>` differs from them and is KEPT.
// Changing these defaults cannot change any model -- only which text is elided.
template <typename T, typename Vector = void, typename Set = void,
          unsigned N = 1>
class SetVector {
public:
  // llvm/ADT/SetVector.h:169 -- bool insert(const value_type &X).  `value_type`
  // is `Vector::value_type`, which for every corpus instantiation is T.
  bool insert(const T &X);

  // llvm/ADT/SetVector.h:253 -- bool contains(const_arg_type key) const, where
  // const_arg_type is const_pointer_or_const_ref<Set::key_type>::type.  Both
  // corpus element types are class types (MLIR op handles), so that resolves to
  // `const T &`.  A POINTER element type would resolve to `const T` by value
  // and key differently; there is no such site here.
  bool contains(const T &key) const;

  // llvm/ADT/SetVector.h:106,112 -- iterator begin() / iterator end(), where
  // `iterator` is `vector_type::const_iterator` (header:72).  Note that this is
  // a CONST iterator even on the non-const overload -- a SetVector never hands
  // out a mutable element, because mutating one through the vector would
  // desynchronise the set.  For SmallVector<T, N> that type is `const T *`.
  const T *begin();
  const T *end();
};

// Restated from llvm/ADT/SetVector.h:339.  See the header comment for why the
// base-class argument list is written with dummies and why this class
// deliberately restates NO members.
template <typename T, unsigned N>
class SmallSetVector : public SetVector<T, T, T, N> {
public:
  // Implicitly declared in LLVM; spelled out here so it can carry a rule.
  SmallSetVector();
};

} // namespace llvm

// ARITY REPAIR, 2026-09-28.  The generic 4-ary
//     template <typename T1, typename T2, typename T3, unsigned T4>
//     using t1 = llvm::SetVector<T1, T2, T3, T4>;
// that used to sit here is DELIBERATELY GONE (one key VANISHES).  It was dead in
// both directions, measured over all 12 census rows' `searched as:` text:
//   * 9 rows search 1-ARY (`llvm::SetVector<mlir::Attribute>` x8,
//     `llvm::SetVector<mlir::Operation *>` x1) -- the defaulted args survive only
//     in the `from decl (NOT a key ...)` display, which the previous header read
//     as the key.  A 4-ary key cannot match a 1-ary search at all.
//   * 3 rows search 4-ARY -- and those failed on the SWALLOW: the next same-depth
//     literal after `T1` is `, `, a comma is not a delimiter, so `T1` captured to
//     the final depth-0 `>` and bound the whole argument list, leaving T2/T3/T4
//     with nothing.  (The `DenseMapInfo<T1>` ate `llvm::StringRef, void` failure.)
// The replacements are ARITY-0: with no template parameters there is nothing to
// capture, so a swallow is ruled out structurally (the t167-t216 position in
// rules/mlir).  Every spelling below is VERBATIM from the `searched as:` line of
// queue/samples/g{223,225,320,1120..1128}.txt -- note in particular that the
// 4-ary third argument is the 1-ARY `llvm::DenseSet<T>`, not the
// `llvm::DenseSet<T, llvm::DenseMapInfo<T, void>>` the decl display shows.
// The MODEL IS UNCHANGED: still `Vec<element>`, insertion order observable.

// g223 g225 g1120 g1121 g1122 g1123 g1124 g1125 -- searched 1-ary.
using t1 = llvm::SetVector<mlir::Attribute>;

// g1126 -- searched 1-ary, pointer element.
using t3 = llvm::SetVector<mlir::Operation *>;

// g320 -- the SAME C++ type as t1, but searched with all four arguments spelled
// out.  It is a DISTINCT key because the search text is distinct; the mapper
// elides the `unsigned` literals to `_`, which is why `0` is written here and
// `_` appears in the recorded key.
using t4 = llvm::SetVector<mlir::Attribute, llvm::SmallVector<mlir::Attribute, 0>,
                           llvm::DenseSet<mlir::Attribute>, 0>;

// g1127 -- searched 4-ary.
using t5 =
    llvm::SetVector<mlir::ktdf::StageOp, llvm::SmallVector<mlir::ktdf::StageOp, 0>,
                    llvm::DenseSet<mlir::ktdf::StageOp>, 0>;

// g1128 -- searched 4-ary.
using t6 = llvm::SetVector<mlir::ktdf_arch::GroupOp,
                           llvm::SmallVector<mlir::ktdf_arch::GroupOp, 0>,
                           llvm::DenseSet<mlir::ktdf_arch::GroupOp>, 0>;

template <typename T1, unsigned T2> using t2 = llvm::SmallSetVector<T1, T2>;

template <typename T1, unsigned T2> llvm::SmallSetVector<T1, T2> f1() {
  return llvm::SmallSetVector<T1, T2>();
}

template <typename T1, typename T2, typename T3, unsigned T4>
bool f2(llvm::SetVector<T1, T2, T3, T4> &o, const T1 &x) {
  return o.insert(x);
}

template <typename T1, typename T2, typename T3, unsigned T4>
bool f3(const llvm::SetVector<T1, T2, T3, T4> &o, const T1 &x) {
  return o.contains(x);
}

template <typename T1, typename T2, typename T3, unsigned T4>
const T1 *f4(llvm::SetVector<T1, T2, T3, T4> &o) {
  return o.begin();
}

template <typename T1, typename T2, typename T3, unsigned T4>
const T1 *f5(llvm::SetVector<T1, T2, T3, T4> &o) {
  return o.end();
}
