// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::DenseMap<K, V> / llvm::DenseSet<K> -- LLVM's open-addressing hash
// containers.
//
// MODEL
//   llvm::DenseMap<K, V> -> HashMap<K, V>
//   llvm::DenseSet<K>    -> HashSet<K>
//
// WHY THE DECLARATIONS ARE RESTATED AND NOT #included: same reason as
// rules/smallvector, rules/support, rules/stringref and rules/twine --
// cpp-rule-preprocessor compiles this file with a fixed flag set and the only
// flags that would reach LLVM's headers are absolute -I paths into whatever
// LLVM tree the target project happens to have built.
//
// ARITY -- MEASURED, and it is NOT what the diagnostic prints.  The converter's
// unsupported-type diagnostic renders the FULL four-argument spelling:
//   `llvm::DenseMap<mlir::Attribute, unsigned long,
//                   llvm::DenseMapInfo<mlir::Attribute, void>,
//                   llvm::detail::DenseMapPair<mlir::Attribute, unsigned long>>`
// but that is a DIFFERENT printer from the one `search()` uses.  With -verbose
// the actual lookup is:
//   search type llvm::DenseMap<mlir::Attribute, unsigned long>, result: None
//   search type llvm::DenseSet<mlir::Operation *>, result: None
// i.e. SuppressDefaultTemplateArgs elides the defaulted KeyInfoT/BucketT, so the
// key has TWO parameters for DenseMap and ONE for DenseSet.  A four-argument key
// is a DEAD rule: it matches nothing.  (An earlier revision of this module was
// written to the diagnostic's spelling and was measured to move neither TU.)
//
// A pleasant consequence: because the traits and bucket parameters are NOT part
// of the key, a GENERIC rule here does not force the converter to find rules for
// `llvm::DenseMapInfo<K, void>` or `llvm::detail::DenseMapPair<K, V>` -- the
// generic-rule regression that bites when a defaulted argument survives into the
// key does not apply.  Measured: the generic two-arg form matches the concrete
// `<mlir::Attribute, unsigned long>` instantiation.
//
// DenseMapInfo IS A TRAITS CLASS AND IS DELIBERATELY NOT MODELLED.  Its
// getEmptyKey / getTombstoneKey / getHashValue are implementation details of the
// open-addressing table (the sentinel keys that mark empty and deleted
// buckets).  HashMap has no analogue -- it brings its own hasher and has no
// caller-visible sentinels -- so inventing values for them would be silently
// wrong.  Mapping DenseMap to HashMap is what makes them unreachable, and that
// was CHECKED: neither TU searches for any DenseMapInfo member after this
// module loads.
//
// NOT COVERED, deliberately: the operation surface (find/lookup/count/
// try_emplace/erase/begin/end and the iterator's ==/!=/++/deref).  Those
// methods live on the CRTP base `llvm::DenseMapBase<DerivedT, K, V, KeyInfoT,
// BucketT>`, whose key therefore carries FIVE template arguments, the first of
// which is the derived DenseMap itself.  Adding them requires harvesting each
// overload's exact base-class key with `-verbose`, which the two measured rows
// do not yet reach, and a guessed key records a DEAD rule.  The two measured
// first-abort rows are TYPE failures ("unsupported system type has no rule"),
// so the type rules are what those rows need; the operations are the next
// blocker.
//
// UPDATE 2026-09-27, MEASURED -- the comment above is out of date about WHY the
// operation surface is blocked, and the correction matters.  It is NOT the
// missing f-rules: all four begin()/end() return spellings already have type
// rules (t4/t5) and the receiver has t3, so the f-rules are directly writable.
// The real gate is t3's ARGUMENT MAPPING.  t3 matches the concrete
// instantiation and then the mapper maps each of its five arguments in turn, and
// arguments 4 and 5 had no model at all, so the run ABORTED rc=134 BEFORE ANY
// SEARCH COULD MATCH -- every `search expr` line read `result: None`:
//   unsupported unmapped type `llvm::DenseMapInfo<unsigned int>` has no model in
//   types_, while mapping `llvm::DenseMapBase<llvm::DenseMap<unsigned int,
//   unsigned int>, unsigned int, unsigned int, llvm::DenseMapInfo<unsigned int>,
//   llvm::detail::DenseMapPair<unsigned int, unsigned int>>`
// t6/f5 below close argument 4.  With them in place the SAME probe advances to
// argument 5 and names it a `LEAF record`:
//   unsupported unmapped type `llvm::detail::DenseMapPair<unsigned int, unsigned
//   int>` has no model in types_, while mapping `llvm::DenseMapBase<...>`
// So the sequence is measured, not inferred, and `llvm::detail::DenseMapPair<K,
// V>` is the LAST remaining gate on the whole operation surface.  Unlike
// DenseMapInfo it is not a refusal case -- it derives from `std::pair<KeyT,
// ValueT>` (DenseMap.h:45) and is the map's value_type, so rules/pair's
// `(T1, T2)` / `<(T1, T2)>::default()` shape models it faithfully.  ADDED as
// t7/f6 below.

// <utility> for std::pair (DenseMapPair's BASE, see below) and std::move.
#include <utility>

namespace llvm {

// DEFINED (not merely forward-declared) so that an EXPLICIT default constructor
// can be declared for it -- the t72/t79/t80 precedent in rules/mlir: an opaque
// TYPE key is useless without its constructor.  The template PARAMETER LIST is
// byte-identical to the forward declaration it replaces, so SuppressDefaultTemplate-
// Args behaves exactly as before and t1/t2/t3's keys are unchanged (verified
// character by character against the frozen BEFORE tree).
template <typename KeyT, typename ValueT = void> struct DenseMapInfo {
  DenseMapInfo();
};

namespace detail {
// DEFINED, not merely forward-declared, for the same reason DenseMapInfo above
// is: the type needs an explicit default constructor to be usable (t7/f6 below).
// The template PARAMETER LIST is byte-identical to the forward declaration it
// replaces, so the DEFAULT-ARGUMENT spellings in DenseMap/DenseMapIterator that
// mention it are unchanged and t1/t3/t4/t5's keys are unaffected.
// DERIVES FROM std::pair, exactly as real LLVM does (llvm/ADT/DenseMap.h:45).
// This is NOT cosmetic and it is NOT about the type key (which stays
// `llvm::detail::DenseMapPair<T1, T2>`): it is what makes `it->first` record as
//   llvm::DenseMapIterator<T1, T2>->std::pair<T1, T2>::first
// -- MEASURED, that is the key string the converter searches (dmv2 trace), because
// the member `first` is DECLARED in the std::pair base, not in DenseMapPair.  With
// DenseMapPair as a standalone struct the rule would record `...->llvm::detail::
// DenseMapPair<T1, T2>::first` and match NOTHING.
template <typename KeyT, typename ValueT>
struct DenseMapPair : public std::pair<KeyT, ValueT> {
  DenseMapPair();
};
} // namespace detail

template <typename KeyT, typename ValueT,
          typename KeyInfoT = DenseMapInfo<KeyT, void>,
          typename BucketT = detail::DenseMapPair<KeyT, ValueT>>
class DenseMap {
public:
  // `unsigned InitialReserve = 0`, NOT a nullary ctor -- MEASURED.  A plain
  // `llvm::DenseMap<unsigned, unsigned> m;` searches for
  //   void llvm::DenseMap<unsigned int, unsigned int>::DenseMap(unsigned int)
  // (dmv2 trace, `search expr` line), because real LLVM spells the default
  // constructor with a defaulted reserve argument.  A nullary key here would be a
  // DEAD rule and the TU would translate rc=0 and then fail to compile with
  // E0433 on `llvm_DenseMap::new()`.
  explicit DenseMap(unsigned InitialReserve = 0);
};

template <typename ValueT, typename ValueInfoT = DenseMapInfo<ValueT, void>>
class DenseSet {
public:
  DenseSet();
};


// Forward-declared HERE, with its default arguments, because DenseMapBase's
// begin()/end() return it.  The definition below therefore must NOT repeat the
// defaults (a default template argument may be given only once).
template <typename KeyT, typename ValueT,
          typename KeyInfoT = DenseMapInfo<KeyT>,
          typename BucketT = detail::DenseMapPair<KeyT, ValueT>,
          bool IsConst = false>
class DenseMapIterator;

// The CRTP base that carries the OPERATION surface (find/lookup/count/
// try_emplace/erase/begin/end).  Its key has FIVE arguments -- the derived
// DenseMap FIRST -- and, unlike DenseMap's, NONE of them are defaulted here, so
// SuppressDefaultTemplateArgs elides nothing and all five survive into the key.
// Measured spelling (survey-v3, 236 occurrences for the largest instantiation):
//   llvm::DenseMapBase<llvm::DenseMap<mlir::StringAttr, mlir::ktdf_arch::Device>,
//                      mlir::StringAttr, mlir::ktdf_arch::Device,
//                      llvm::DenseMapInfo<mlir::StringAttr>,
//                      llvm::detail::DenseMapPair<mlir::StringAttr, mlir::ktdf_arch::Device>>
// Note the FIRST argument is printed in DenseMap's SUGARED two-argument form.
template <typename DerivedT, typename KeyT, typename ValueT, typename KeyInfoT,
          typename BucketT>
class DenseMapBase {
public:
  DenseMapBase();
  // operator[] lives on the CRTP BASE, so its key carries all five of
  // DenseMapBase's arguments.  MEASURED spelling for the rvalue overload:
  //   unsigned int & llvm::DenseMapBase<llvm::DenseMap<unsigned int, unsigned int>,
  //     unsigned int, unsigned int, llvm::DenseMapInfo<unsigned int>,
  //     llvm::detail::DenseMapPair<unsigned int, unsigned int>>::operator[](unsigned int &&)
  // Both overloads are declared: `m[3u]` binds KeyT&& while `m[k]` on an lvalue
  // binds const KeyT&, and a missing overload emits the mangled fallback name
  // instead of the rule (the rules/set lesson).
  ValueT &operator[](const KeyT &Key);
  ValueT &operator[](KeyT &&Key);
  // MUTABLE begin/end: return the TWO-argument sugared iterator spelling (t4).
  DenseMapIterator<KeyT, ValueT> begin();
  DenseMapIterator<KeyT, ValueT> end();
  // CONST begin/end: IsConst differs from its default so nothing is elided and
  // the FIVE-argument spelling survives (t5).  The arity split is real -- the two
  // key strings differ, so one rule cannot cover both.
  DenseMapIterator<KeyT, ValueT, DenseMapInfo<KeyT>,
                   detail::DenseMapPair<KeyT, ValueT>, true>
  begin() const;
  DenseMapIterator<KeyT, ValueT, DenseMapInfo<KeyT>,
                   detail::DenseMapPair<KeyT, ValueT>, true>
  end() const;
};


// ---------------------------------------------------------------------------
// THE ITERATOR.  ARITY MEASURED, NOT GUESSED -- and there are TWO spellings.
//
// `llvm::DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, bool IsConst = false>`.
// SuppressDefaultTemplateArgs elides a TRAILING run of default-valued arguments,
// so:
//   * a MUTABLE iterator (IsConst = false, the default) has all three trailing
//     arguments at their defaults and prints as the TWO-argument sugared form
//         llvm::DenseMapIterator<SentientRegType, GraphColoring>
//   * a CONST iterator (IsConst = true) differs from its default in the LAST
//     position, so nothing can be elided and all FIVE arguments survive
//         llvm::DenseMapIterator<mlir::Operation *, scheduler::PipelineTreeNode *,
//                                llvm::DenseMapInfo<mlir::Operation *>,
//                                llvm::detail::DenseMapPair<mlir::Operation *,
//                                                           scheduler::PipelineTreeNode *>,
//                                true>
//     -- note KeyInfoT prints with ONE argument (its own `Enable = void` is
//     elided) while DenseMap's diagnostic spelling shows `<K, void>`; the two
//     printers disagree and the ONE-argument form is the one the key uses.
// Measured over the 49 queue rows: 31 rows are the short form, 18 the long one,
// and ALL 18 long rows carry `true` -- there is no fourth-argument-only variant.
// So the long key can be written with the non-type argument FIXED to `true` and
// only T1/T2 generic: the bucket and traits arguments are functions of T1/T2.
//
// Both forms are keyed.  A single generic key does NOT collapse them, because the
// key is a STRING and the two strings differ.
template <typename KeyT, typename ValueT, typename KeyInfoT,
          typename BucketT, bool IsConst>
class DenseMapIterator {
public:
  DenseMapIterator();
  // ++ is a MEMBER operator, so the rule bodies below must use the MEMBER call
  // form (`a0.operator++()`); an infix spelling records nothing at all, silently,
  // and a qualified `llvm::operator++(a0)` aborts at
  // cpp_rule_preprocessor.cpp:888.  Pre- and post-increment are DIFFERENT KEYS
  // with different return types.
  DenseMapIterator &operator++();
  DenseMapIterator operator++(int);
  // Declared so that `it->first` parses.  No rule is written FOR it -- see the
  // note on f15-f18.
  detail::DenseMapPair<KeyT, ValueT> *operator->() const;
};

// Free comparison operators, found by ADL in namespace llvm -- which is why the
// measured key is spelled `bool llvm::operator==(...)` and not a member.
template <typename KeyT, typename ValueT, typename KeyInfoT, typename BucketT,
          bool IsConst>
bool operator==(
    const DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, IsConst> &lhs,
    const DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, IsConst> &rhs);

template <typename KeyT, typename ValueT, typename KeyInfoT, typename BucketT,
          bool IsConst>
bool operator!=(
    const DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, IsConst> &lhs,
    const DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, IsConst> &rhs);

} // namespace llvm

template <typename T1, typename T2> using t1 = llvm::DenseMap<T1, T2>;

template <typename T1> using t2 = llvm::DenseSet<T1>;

template <typename T1, typename T2, typename T3, typename T4, typename T5>
using t3 = llvm::DenseMapBase<T1, T2, T3, T4, T5>;

// The MUTABLE iterator: two arguments.
template <typename T1, typename T2>
using t4 = llvm::DenseMapIterator<T1, T2>;

// The CONST iterator: five arguments, the last fixed to `true`.
template <typename T1, typename T2>
using t5 = llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                                  llvm::detail::DenseMapPair<T1, T2>, true>;

// ==/!= on the MUTABLE iterator.  Written in CALL form: an INFIX operator in a
// rule body records nothing at all, silently.
template <typename T1, typename T2>
bool f1(const llvm::DenseMapIterator<T1, T2> &a0,
        const llvm::DenseMapIterator<T1, T2> &a1) {
  return operator==(a0, a1);
}

template <typename T1, typename T2>
bool f2(const llvm::DenseMapIterator<T1, T2> &a0,
        const llvm::DenseMapIterator<T1, T2> &a1) {
  return operator!=(a0, a1);
}

// ==/!= on the CONST iterator.
template <typename T1, typename T2>
bool f3(const llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                                     llvm::detail::DenseMapPair<T1, T2>, true> &a0,
        const llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                                     llvm::detail::DenseMapPair<T1, T2>, true> &a1) {
  return operator==(a0, a1);
}

template <typename T1, typename T2>
bool f4(const llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                                     llvm::detail::DenseMapPair<T1, T2>, true> &a0,
        const llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                                     llvm::detail::DenseMapPair<T1, T2>, true> &a1) {
  return operator!=(a0, a1);
}

// ---------------------------------------------------------------------------
// t6 -- `llvm::DenseMapInfo<T1>`, AN OPAQUE TYPE KEY WITH NO MEMBERS MAPPED.
//
// WHY THIS IS NOT A REVERSAL OF THE STANDING REFUSAL.  The refusal is about
// synthesising DenseMapInfo's BEHAVIOUR -- getEmptyKey / getTombstoneKey /
// getHashValue / isEqual -- and it stands: NONE of them is mapped here, so any
// call to one still aborts LOUDLY rather than silently inventing a sentinel.
// What is added is only the TYPE, which the mapper needs for a different reason:
// t3 (`llvm::DenseMapBase<T1..T5>`) matches the concrete instantiation and then
// MAPS EACH OF ITS FIVE ARGUMENTS, and argument 4 is `llvm::DenseMapInfo<K>`.
// Without a model for it, mapping t3 aborts rc=134 with
//   unsupported unmapped type `llvm::DenseMapInfo<unsigned int>` has no model in
//   types_, while mapping `llvm::DenseMapBase<...>`
// BEFORE ANY SEARCH CAN MATCH, which blocks the module's ENTIRE operation
// surface (begin/end/find/lookup/count/try_emplace/erase and the iterator's
// ++/deref) behind a type nobody ever names in the source.
//
// Precedent, three times over in rules/mlir: t72 `mlir::TypeID` -> `()` with
// ==/!= DELIBERATELY ABSENT; t79 `llvm::BitVector` -> `Vec<bool>` with every
// member unmapped; t80 `mlir::detail::PreservedAnalyses` -> `()`.
//
// DESTRUCTOR TEST: `grep -n '~DenseMapInfo' llvm/ADT/DenseMapInfo.h` -> ZERO
// hits.  It is a stateless traits class with no members at all, so a unit loses
// nothing that could be observed.
template <typename T1> using t6 = llvm::DenseMapInfo<T1>;

// f5 -- t6's default constructor.  A type key without one gives rc=0 and then
// E0433 on `<mangled-type>::new()`; measured seven times.  Nothing in the corpus
// constructs a DenseMapInfo (it appears only as a template argument), but the
// key costs nothing and its absence is invisible until it is not.
template <typename T1> llvm::DenseMapInfo<T1> f5() {
  return llvm::DenseMapInfo<T1>();
}


// ---------------------------------------------------------------------------
// t7 -- `llvm::detail::DenseMapPair<T1, T2>`, the map's `value_type`.
//
// NOT a refusal case and not opaque: in real LLVM it is
// `struct DenseMapPair : public std::pair<KeyT, ValueT>` (llvm/ADT/DenseMap.h:45),
// i.e. literally a std::pair, so rules/pair's committed `t1` shape models it
// faithfully -- a Rust 2-tuple.  The REFCOUNT arm differs from pair's by one
// wrapper and the difference is load-bearing: this module's t1 maps DenseMap to
// `HashMap<T1, Value<T2>>`, so the bucket's second component must be `Value<T2>`
// for the two to agree.
//
// WHY IT IS NEEDED: t3 (`llvm::DenseMapBase<T1..T5>`) maps each of its five
// arguments and argument 5 is `llvm::detail::DenseMapPair<K, V>`.  With t6
// (DenseMapInfo) in place the abort moved forward to exactly this argument:
//   unsupported unmapped type `llvm::detail::DenseMapPair<unsigned int, unsigned int>`
//   has no model in types_, while mapping `llvm::DenseMapBase<...>`
// It is the LAST gate before any search against the DenseMapBase receiver can run.
template <typename T1, typename T2>
using t7 = llvm::detail::DenseMapPair<T1, T2>;

// f6 -- t7's default constructor.  Same reason as f5: a type key without one
// gives rc=0 and then E0433 on `<mangled-type>::new()`.
template <typename T1, typename T2>
llvm::detail::DenseMapPair<T1, T2> f6() {
  return llvm::detail::DenseMapPair<T1, T2>();
}

// ---------------------------------------------------------------------------
// f7-f10 -- begin()/end() on the CRTP base.  Written ONLY because t7 above
// opened the gate: MEASURED, the same dmgate.cpp probe that previously died in
// t3's argument mapping now reaches
//   search expr llvm::DenseMapIterator<unsigned int, unsigned int>
//     llvm::DenseMapBase<llvm::DenseMap<unsigned int, unsigned int>, unsigned int,
//     unsigned int, llvm::DenseMapInfo<unsigned int>,
//     llvm::detail::DenseMapPair<unsigned int, unsigned int>>::begin(), result:
// i.e. the search RUNS.  Four keys, not two: the MUTABLE overload returns t4's
// two-argument spelling and the CONST overload returns t5's five-argument
// `, true>` spelling, and the receiver is const-qualified in the const pair.
template <typename T1, typename T2>
llvm::DenseMapIterator<T1, T2>
f7(llvm::DenseMapBase<llvm::DenseMap<T1, T2>, T1, T2, llvm::DenseMapInfo<T1>,
                      llvm::detail::DenseMapPair<T1, T2>> &o) {
  return o.begin();
}

template <typename T1, typename T2>
llvm::DenseMapIterator<T1, T2>
f8(llvm::DenseMapBase<llvm::DenseMap<T1, T2>, T1, T2, llvm::DenseMapInfo<T1>,
                      llvm::detail::DenseMapPair<T1, T2>> &o) {
  return o.end();
}

template <typename T1, typename T2>
llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                       llvm::detail::DenseMapPair<T1, T2>, true>
f9(const llvm::DenseMapBase<llvm::DenseMap<T1, T2>, T1, T2,
                            llvm::DenseMapInfo<T1>,
                            llvm::detail::DenseMapPair<T1, T2>> &o) {
  return o.begin();
}

template <typename T1, typename T2>
llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                       llvm::detail::DenseMapPair<T1, T2>, true>
f10(const llvm::DenseMapBase<llvm::DenseMap<T1, T2>, T1, T2,
                             llvm::DenseMapInfo<T1>,
                             llvm::detail::DenseMapPair<T1, T2>> &o) {
  return o.end();
}


// ---------------------------------------------------------------------------
// f11-f14 -- the iterator's ++ .  MEASURED first-abort at 7b3de8f was exactly
//   unsupported CXXOperatorCallExpr: ++ on (llvm::DenseMapIterator<unsigned int,
//   unsigned int>) rule key: llvm::DenseMapIterator<unsigned int, unsigned int> &
//   llvm::DenseMapIterator<unsigned int, unsigned int>::operator++()
// Shapes copied from rules/unordered_map's committed f32/f33 (mutable) and
// f34/f35 (const): the PRE-increment rule takes its receiver by C++ reference and
// the POST-increment rule takes it BY VALUE plus an `int`, while BOTH targets
// declare a single `&mut` receiver parameter and return the iterator BY VALUE.
template <typename T1, typename T2>
llvm::DenseMapIterator<T1, T2> &f11(llvm::DenseMapIterator<T1, T2> &a0) {
  return a0.operator++();
}

template <typename T1, typename T2>
llvm::DenseMapIterator<T1, T2> f12(llvm::DenseMapIterator<T1, T2> a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1, typename T2>
llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                       llvm::detail::DenseMapPair<T1, T2>, true> &
f13(llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                           llvm::detail::DenseMapPair<T1, T2>, true> &a0) {
  return a0.operator++();
}

template <typename T1, typename T2>
llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                       llvm::detail::DenseMapPair<T1, T2>, true>
f14(llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                           llvm::detail::DenseMapPair<T1, T2>, true> a0,
    int a1) {
  return a0.operator++(a1);
}

// ---------------------------------------------------------------------------
// f15-f18 -- `it->first` / `it->second`.  These are FUSED keys: the whole
// arrow-member-access is one rule, spelled
//   llvm::DenseMapIterator<T1, T2>->std::pair<T1, T2>::first
// exactly as rules/unordered_map's committed f36-f39 are.  NO RULE IS WRITTEN FOR
// `operator->` ITSELF, and that is deliberate: the dmv2 trace shows the converter
// searches the fused key FIRST and only falls back to `operator->` + a Rust deref
// when the fused key misses -- and that fallback cannot work here, because the
// iterator maps to a libcc2rs `HashMapIter`, which is not a pointer to a tuple, so
// `(*it).first` would be E0614.  A rule returning `*const (T1, T2)` for operator->
// would additionally have to fabricate a pointer to a bucket that does not exist in
// the Rust model.  With f15-f18 FOUND the fallback is never taken.
template <typename T1, typename T2>
const T1 &f15(llvm::DenseMapIterator<T1, T2> a0) {
  return a0->first;
}

template <typename T1, typename T2>
T2 &f16(llvm::DenseMapIterator<T1, T2> a0) {
  return a0->second;
}

template <typename T1, typename T2>
const T1 &f17(llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                                     llvm::detail::DenseMapPair<T1, T2>, true>
                  a0) {
  return a0->first;
}

template <typename T1, typename T2>
const T2 &f18(llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                                     llvm::detail::DenseMapPair<T1, T2>, true>
                  a0) {
  return a0->second;
}

// ---------------------------------------------------------------------------
// f19/f20 -- DenseMapBase::operator[].  Receiver is the FIVE-argument CRTP base
// (t3), whose first argument is the derived DenseMap in its SUGARED two-argument
// spelling -- the same receiver shape f7-f10 already use and which is MEASURED to
// match.  Body in MEMBER call form for the same reason ++ is.
template <typename T1, typename T2>
T2 &f19(llvm::DenseMapBase<llvm::DenseMap<T1, T2>, T1, T2, llvm::DenseMapInfo<T1>,
                           llvm::detail::DenseMapPair<T1, T2>> &a0,
        T1 &&a1) {
  return a0.operator[](std::move(a1));
}

template <typename T1, typename T2>
T2 &f20(llvm::DenseMapBase<llvm::DenseMap<T1, T2>, T1, T2, llvm::DenseMapInfo<T1>,
                           llvm::detail::DenseMapPair<T1, T2>> &a0,
        const T1 &a1) {
  return a0.operator[](a1);
}

// ---------------------------------------------------------------------------
// f21 -- t1's CONSTRUCTOR, and the RIGHT OVERLOAD: `DenseMap(unsigned)`, not a
// nullary one.  See the note on the declaration above.  A type key without its
// constructor gives rc=0 and then E0433; measured seven times.
template <typename T1, typename T2>
llvm::DenseMap<T1, T2> f21(unsigned a0) {
  return llvm::DenseMap<T1, T2>(a0);
}
