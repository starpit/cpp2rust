// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::DenseSet, its base llvm::detail::DenseSetImpl, and the nested iterator.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/ADT/DenseSet.h>
// ---------------------------------------------------------------------------
// Same reason as rules/densemap, rules/smallset, rules/stringmap and
// rules/stringref: cpp-rule-preprocessor compiles this file with a fixed flag
// set, and the only flags that would reach LLVM's headers are absolute -I paths
// into whatever LLVM tree the target project happens to have built.  So the
// signatures LLVM declares are restated here, and a restatement matches iff it
// agrees with LLVM exactly.  Every declaration below was read out of
// llvm/ADT/DenseSet.h of the LLVM the port actually builds against
// (LLVM-22.1.3, DenseSet.h:31..285), not remembered.
//
// THE SET IS SPELLED THROUGH ITS BASE, and that is the whole gap
// -------------------------------------------------------------
// `llvm::DenseSet<ValueT, ValueInfoT>` (DenseSet.h:278) is a thin derived class
//     class DenseSet : public detail::DenseSet<ValueT, ValueInfoT>
// where `detail::DenseSet` is an ALIAS TEMPLATE (DenseSet.h:263) for
//     detail::DenseSetImpl<ValueT,
//                          DenseMap<ValueT, DenseSetEmpty, ValueInfoT,
//                                   DenseSetPair<ValueT>>,
//                          ValueInfoT>
// Every member -- insert, contains, count, size, begin, end, find -- is a member
// of DenseSetImpl, and the iterator is DenseSetImpl's NESTED template
// `DenseSetIterator<bool IsConst>` (DenseSet.h:104).  So the strings the
// converter asks for name DenseSetImpl and its full argument list, never
// `llvm::DenseSet<T>`.  The census records exactly that, e.g.
//   llvm::detail::DenseSetImpl<mlir::Value, llvm::DenseMap<mlir::Value,
//     llvm::detail::DenseSetEmpty, llvm::DenseMapInfo<mlir::Value>,
//     llvm::detail::DenseSetPair<mlir::Value>>,
//     llvm::DenseMapInfo<mlir::Value>>::DenseSetIterator<false>
// rules/densemap declares only `llvm::DenseMap<T1,T2>` and the two
// DenseMapIterator forms, so it covers none of this: the set was a total gap and
// grepping the rule IR for DenseSetIterator/DenseSetImpl returned nothing.
//
// The restatement below therefore reproduces the whole chain -- DenseSetEmpty,
// DenseSetPair, a 4-parameter DenseMap, DenseSetImpl with its nested iterator,
// and DenseSet deriving from it -- so that `llvm::DenseSet<T1>` spelled in a
// rule resolves to the same canonical string.
//
// NO DEFAULT ARGUMENTS ON THE INNER TEMPLATES, deliberately.  rules/smallset
// records that cpp-rule-preprocessor ELIDES an argument that equals its
// template's default, which silently changes a key's arity and makes the rule
// match nothing.  The inner DenseMap here is restated with FOUR parameters and
// no defaults, because the string the converter asks for spells all four; only
// DenseSet's own ValueInfoT keeps its default, since the census's strings show
// the outer name elided down to one argument the same way rules/densemap's
// `llvm::DenseMap<T1,T2>` is.
//
// WHAT AN ITERATOR COMPARISON MEANS -- rules/densemap's argument, unchanged
// -----------------------------------------------------------------------
// LLVM's own body at DenseSet.h:144 is
//     friend bool operator==(const DenseSetIterator &LHS,
//                            const DenseSetIterator &RHS) { return LHS.I == RHS.I; }
// i.e. IDENTITY OF POSITION (the wrapped DenseMap iterator, hence the bucket
// address), not equality of the pointed-to values.  For a DEDUPLICATING set
// those two relations coincide on every pair a defined program can compare --
// the honest version rules/smallset states: insert is a no-op when the element
// is present (DenseSet.h:198 forwards to try_emplace, which keeps the
// incumbent), so within one container two distinct positions necessarily hold
// distinct elements, and end() holds none.  So the existing key-based iterator
// PartialEq in libcc2rs (iterators.rs:60 -- position by key, end() = None) is
// what this comparison MEANS here, and no new runtime type is needed.
//
// AND THEY ARE HIDDEN FRIENDS.  DenseSet.h:144/:148 declare == and != INSIDE
// DenseSetIterator as friends, exactly as DenseMap.h:1217/:1229 does.  Declared
// at namespace scope in a restatement they are not found at all ("No viable
// function"), which is the silent miss rules/densemap and rules/smallset both
// record.  So they are restated as friends and the rules spell `operator==(a,b)`
// -- the free-function call form the friend is found by, which is what
// rules/densemap's f1/f2 use.
//
// REPRESENTATION: the ordered key set rules/set and rules/smallset already use
// ---------------------------------------------------------------------------
// BTreeMap<T1, Box<T1>> (unsafe) / BTreeMap<T1, Value<T1>> (refcount), the
// set-as-map-keyed-on-its-own-element shape, with UnsafeMapIterator<T1,T1> /
// RefcountMapIter<T1,T1> as the iterator.  DenseSet is a HASH table and leaves
// its iteration order unspecified, so walking it in key order is an
// order-REFINING translation -- the same argument rules/densemap records for
// DenseMap and rules/unordered_map for std::unordered_map.  The verification
// probe prints a SORTED signature of the elements so it does not depend on the
// refinement.
//
// MAP THE CONTAINER, NOT JUST THE ITERATOR.  rules/stringmap is the cautionary
// tale: it mapped the iterator and left StringMap itself unnamed, so the
// converter took the port-the-class path on the receiver and the output could
// not compile at all.  rules/smallset hit the same thing (`cannot find type
// llvm_SmallSet_long_____std_less_long__`).  So the type rule for DenseSet AND
// the members the port actually calls are both here.
//
// WHAT THE PORT ACTUALLY CALLS.  A census of llvm::DenseSet receivers across
// dt_src (grep over dbo/, dcc/, dataflow-scheduler/) finds exactly two shapes in
// the three aborting TUs:
//   dbo/src/Pipeline/CorrectAtRuntime.cpp:462        seen.insert(v).second
//   dbo/src/Pipeline/PrepareRuntimeCorrection.cpp:302  taken.insert(pos).second
//   dbo/src/Transforms/Autopilot.cpp:573            written.insert(sym).second
//   dbo/src/Transforms/Autopilot.cpp:634/647/668    combined_names.insert/contains
// plus the default construction of each set.  insert and contains are therefore
// the load-bearing members; count/size/empty/find/begin/end are mapped because
// the verification probe reaches them (a set that did not DEDUP would otherwise
// be indistinguishable), and everything else stays loud.
//
// NOT MODELLED, deliberately -- each is a loud abort rather than a guessed body:
// erase (both the key and the iterator overload), clear, resize/reserve,
// getMemorySize, swap, insert_as/find_as, the range and initializer_list
// constructors, insert_range, postfix operator++, operator->, the
// iterator-to-const_iterator converting constructor, DenseSetImpl's own
// namespace-scope operator==/!= (DenseSet.h:241/:256 -- SET equality, a
// different entity from the ITERATOR comparison mapped here), and SmallDenseSet,
// whose MapTy is SmallDenseMap and whose canonical string is therefore a
// different one entirely (dcc/.../GraphStats.cpp uses it; it stays a gap).
//
// TWO SPELLINGS OF THE KEY-INFO ARGUMENT, TWO RULE FAMILIES.  The census shows
//     llvm::DenseMapInfo<long>                  (and <mlir::Value>, <unsigned long>)
//     llvm::DenseMapInfo<llvm::StringRef, void>
// i.e. for StringRef the `Enable` parameter of DenseMapInfo is spelled out and
// for the others it is elided.  Both are part of the canonical string, so one
// family cannot serve both.  f1..f22 carry the ONE-argument spelling and
// f23..f44 the TWO-argument one; the FAMILY TWO banner at the bottom of this
// file records the mechanism and why a sentinel default on DenseMapInfo is what
// lets one translation unit produce both keys.
//
// STILL A GAP -- POINTER element types.  `const_arg_type_t<T *>` is `const T *`
// BY VALUE, not `const T * &`, so llvm::DenseSet<mlir::Operation *> keys find /
// contains / count / insert on a different string than the by-reference forms
// declared below.  Left unmapped in both families, so it stays a loud abort
// rather than a guess.

#include <utility>

namespace llvm {

// DenseMapInfo, restated with TWO parameters and a SENTINEL default.  LLVM
// declares `template <typename T, typename Enable = void> struct DenseMapInfo;`
// (DenseMapInfo.h:51), and the census shows both a one- and a two-argument
// spelling reaching the converter.  The default here is deliberately NOT `void`
// so that BOTH keys are expressible from one translation unit: a defaulted
// argument is elided from the key, an explicitly-different one is kept.  See the
// FAMILY TWO banner below for the full argument.  `DenseMapInfoEnableDefault` is
// never spelled by any rule, so it can appear in no key; it is incomplete
// because nothing needs it defined.
struct DenseMapInfoEnableDefault;

template <typename KeyT, typename Enable = DenseMapInfoEnableDefault>
struct DenseMapInfo;

namespace detail {

// DenseSet.h:32 / :35.  DenseSetPair derives from DenseSetEmpty (the empty base
// class trick that makes a set bucket hold one item), and both names appear
// INSIDE the canonical string of every DenseSetImpl, so both must exist here
// under these exact names.  Neither is otherwise used.
struct DenseSetEmpty {};

template <typename KeyT> class DenseSetPair : public DenseSetEmpty {
public:
  KeyT &getFirst();
  const KeyT &getFirst() const;
  DenseSetEmpty &getSecond();
  const DenseSetEmpty &getSecond() const;
};

} // namespace detail

// The bucket map.  FOUR parameters and NO DEFAULTS: the string the converter
// asks for spells all four arguments, and a default that an argument equals
// would be elided (rules/smallset records that failure).  Only the name is
// needed -- no member of this DenseMap is reachable through the set, and
// rules/densemap already owns DenseMap's own surface.  Restating it here does
// not collide: this declaration exists only so DenseSetImpl's canonical
// argument list can be spelled, and the rules below key on DenseSetImpl.
template <typename KeyT, typename ValueT, typename KeyInfoT, typename BucketT>
class DenseMap {};

namespace detail {

// DenseSet.h:55.  Three parameters, no defaults, exactly as LLVM declares it.
template <typename ValueT, typename MapTy, typename ValueInfoT>
class DenseSetImpl {
public:
  using key_type = ValueT;
  using value_type = ValueT;
  // DenseSet.h:66 -- `using size_type = unsigned;`, NOT size_t.  size() and
  // count() therefore return `unsigned int`, which is what rules/densemap's f8
  // and f11 also carry; a size_t return would be a different string.
  using size_type = unsigned int;

  explicit DenseSetImpl(unsigned InitialReserve);

  // DenseSet.h:104 -- a PRIVATE nested template in LLVM, published through the
  // iterator/const_iterator typedefs at :152/:153.  It is restated PUBLIC here
  // because a rule has to name the type; the canonical string is the same, and
  // it is the string the census reports as unmapped.
  //
  // The comparisons are HIDDEN FRIENDS (DenseSet.h:144/:148), like
  // DenseMapIterator's.  See the header comment: restating them at namespace
  // scope makes them unresolvable and the rule silently dead.
  template <bool IsConst> class DenseSetIterator {
  public:
    DenseSetIterator();
    ValueT &operator*() const;
    DenseSetIterator &operator++();
    friend bool operator==(const DenseSetIterator &LHS,
                           const DenseSetIterator &RHS);
    friend bool operator!=(const DenseSetIterator &LHS,
                           const DenseSetIterator &RHS);
  };

  using iterator = DenseSetIterator<false>;
  using const_iterator = DenseSetIterator<true>;

  bool empty() const;
  size_type size() const;

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;

  // DenseSet.h:161..:176.  LLVM spells the parameter
  // `const_arg_type_t<ValueT>`, i.e. const_pointer_or_const_ref<T>::type, which
  // for a NON-POINTER element is `const ValueT &` -- the form restated here, and
  // the form all three element types the port needs (mlir::Value, long,
  // unsigned long) resolve to.  For a POINTER element it is `const ValueT` BY
  // VALUE, a different string, so llvm::DenseSet<Operation *> (used in
  // dataflow-scheduler) is NOT covered by these and stays loud.
  iterator find(const ValueT &V);
  const_iterator find(const ValueT &V) const;
  bool contains(const ValueT &V) const;
  size_type count(const ValueT &V) const;

  // DenseSet.h:198/:202.  BOTH overloads: `s.insert(x)` on an lvalue binds the
  // first and `s.insert(f())` the second, and they are two different signature
  // strings -- the split rules/smallset records at f5/f6 and rules/densemap at
  // f7/f15.  insert KEEPS THE INCUMBENT and reports false on a duplicate (it
  // forwards to DenseMap::try_emplace), which is the contract `insert(x).second`
  // at all three abort sites depends on.
  std::pair<iterator, bool> insert(const ValueT &V);
  std::pair<iterator, bool> insert(ValueT &&V);
};

// DenseSet.h:263 -- an ALIAS TEMPLATE, not a class.  It is what llvm::DenseSet
// derives from, and restating it as the alias (rather than flattening it into
// the base clause) keeps the canonical argument list identical.
template <typename ValueT, typename ValueInfoT>
using DenseSet = DenseSetImpl<
    ValueT,
    llvm::DenseMap<ValueT, DenseSetEmpty, ValueInfoT, DenseSetPair<ValueT>>,
    ValueInfoT>;

} // namespace detail

// DenseSet.h:278.  ValueInfoT KEEPS its default here -- unlike the inner
// templates -- because the census's strings show the outer name elided to one
// argument, the same way rules/densemap's key is `llvm::DenseMap<T1, T2>` with
// two of its four parameters defaulted away.
//
// LLVM writes `using BaseT::BaseT;`.  The default constructor is restated
// EXPLICITLY instead of inherited: `llvm::DenseSet<T> s;` is what all three
// aborting TUs write, and going through an inherited constructor would make
// which entity clang resolves to (the implicit DenseSet() or the inherited
// DenseSetImpl(unsigned)) depend on inherited-constructor subtleties.  Both
// forms are declared and both are mapped (f1 and f2), so neither route depends
// on the other resolving.
template <typename ValueT, typename ValueInfoT = DenseMapInfo<ValueT>>
class DenseSet : public detail::DenseSet<ValueT, ValueInfoT> {
public:
  DenseSet();
  explicit DenseSet(unsigned InitialReserve);
};

} // namespace llvm

// ---------------------------------------------------------------------------
// THE CONTAINER TYPE and THE TWO ITERATOR TYPES.
//
// t1 is DenseSet itself -- the rules/stringmap lesson: an iterator rule without
// a named container cannot work, because the converter ports the receiver class
// instead of consulting the rules.
//
// t2/t3 are the nested iterators, reached through the typedefs the way target
// code names them.  They are DISTINCT strings (`DenseSetIterator<false>` vs
// `<true>`) and each needs its own rule, exactly as rules/densemap records for
// DenseMapIterator's const form; both map to the SAME Rust type, which
// mapper.cpp:781 accepts.
// ---------------------------------------------------------------------------

template <typename T1> using t1 = llvm::DenseSet<T1>;
template <typename T1> using t2 = typename llvm::DenseSet<T1>::iterator;
template <typename T1> using t3 = typename llvm::DenseSet<T1>::const_iterator;

// Construction.  See the note on the class: both the nullary form and the
// reserve-taking form are mapped so the mapping does not depend on which entity
// `llvm::DenseSet<T> s;` resolves to.
template <typename T1> llvm::DenseSet<T1> f1() { return llvm::DenseSet<T1>(); }

template <typename T1> llvm::DenseSet<T1> f2(unsigned n) {
  return llvm::DenseSet<T1>(n);
}

// insert -- the load-bearing member.  Keeps the incumbent, reports whether it
// inserted; `insert(x).second` at the three abort sites is exactly this bool.
template <typename T1>
std::pair<typename llvm::DenseSet<T1>::iterator, bool>
f3(llvm::DenseSet<T1> &o, const T1 &v) {
  return o.insert(v);
}

template <typename T1>
std::pair<typename llvm::DenseSet<T1>::iterator, bool>
f4(llvm::DenseSet<T1> &o, T1 &&v) {
  return o.insert(static_cast<T1 &&>(v));
}

// contains / count -- Autopilot.cpp:647 and :668 call contains; count is what a
// probe can use to tell a deduplicating body from a non-deduplicating one.
template <typename T1>
bool f5(const llvm::DenseSet<T1> &o, const T1 &v) {
  return o.contains(v);
}

template <typename T1>
unsigned int f6(const llvm::DenseSet<T1> &o, const T1 &v) {
  return o.count(v);
}

template <typename T1> unsigned int f7(const llvm::DenseSet<T1> &o) {
  return o.size();
}

template <typename T1> bool f8(const llvm::DenseSet<T1> &o) {
  return o.empty();
}

// begin/end/find, in both const-nesses.  The const receiver hands back
// const_iterator, a different type AND a different signature -- rules/densemap
// f22..f24.
template <typename T1>
typename llvm::DenseSet<T1>::iterator f9(llvm::DenseSet<T1> &o) {
  return o.begin();
}

template <typename T1>
typename llvm::DenseSet<T1>::iterator f10(llvm::DenseSet<T1> &o) {
  return o.end();
}

template <typename T1>
typename llvm::DenseSet<T1>::const_iterator f11(const llvm::DenseSet<T1> &o) {
  return o.begin();
}

template <typename T1>
typename llvm::DenseSet<T1>::const_iterator f12(const llvm::DenseSet<T1> &o) {
  return o.end();
}

template <typename T1>
typename llvm::DenseSet<T1>::iterator f13(llvm::DenseSet<T1> &o, const T1 &v) {
  return o.find(v);
}

template <typename T1>
typename llvm::DenseSet<T1>::const_iterator f14(const llvm::DenseSet<T1> &o,
                                                const T1 &v) {
  return o.find(v);
}

// The iterator comparisons.  SPELLED AS FREE-FUNCTION CALLS, never `a == b`:
// these are hidden friends (found by ADL on the operand), and rules/smallset
// records that a plain `a == b` spelling produces NO src entry, after which the
// converter aborts EVERY translation that loads the module with
// `LLVM ERROR: Expr rule loaded from IR but has no src`.
template <typename T1>
bool f15(typename llvm::DenseSet<T1>::iterator a,
         typename llvm::DenseSet<T1>::iterator b) {
  return operator==(a, b);
}

template <typename T1>
bool f16(typename llvm::DenseSet<T1>::iterator a,
         typename llvm::DenseSet<T1>::iterator b) {
  return operator!=(a, b);
}

template <typename T1>
bool f17(typename llvm::DenseSet<T1>::const_iterator a,
         typename llvm::DenseSet<T1>::const_iterator b) {
  return operator==(a, b);
}

template <typename T1>
bool f18(typename llvm::DenseSet<T1>::const_iterator a,
         typename llvm::DenseSet<T1>::const_iterator b) {
  return operator!=(a, b);
}

// operator* and PREFIX operator++.  A begin()/end() walk is unusable without
// both, and the probe's sorted signature is how the order-refinement discussed
// in the header comment is kept out of the evidence.  DenseSet.h:131 returns
// `reference`, i.e. ValueT & for the mutable iterator and const ValueT & for the
// const one -- two different strings.  POSTFIX ++ is NOT mapped and stays loud
// (rules/densemap f17 records that it is a separate entity).
template <typename T1> T1 &f19(typename llvm::DenseSet<T1>::iterator it) {
  return it.operator*();
}

template <typename T1>
const T1 &f20(typename llvm::DenseSet<T1>::const_iterator it) {
  return it.operator*();
}

template <typename T1>
typename llvm::DenseSet<T1>::iterator &
f21(typename llvm::DenseSet<T1>::iterator &it) {
  return it.operator++();
}

template <typename T1>
typename llvm::DenseSet<T1>::const_iterator &
f22(typename llvm::DenseSet<T1>::const_iterator &it) {
  return it.operator++();
}


// ---------------------------------------------------------------------------
// FAMILY TWO -- the same surface keyed on the TWO-ARGUMENT DenseMapInfo.
//
// The gap the header comment above left open, now closed and MEASURED rather
// than argued.  The census shows the key-info argument arriving in two
// spellings, and dbo/src/Transforms/Autopilot.cpp asks for the second one:
//   llvm::detail::DenseSetImpl<llvm::StringRef, llvm::DenseMap<llvm::StringRef,
//     llvm::detail::DenseSetEmpty, llvm::DenseMapInfo<llvm::StringRef, void>,
//     llvm::detail::DenseSetPair<llvm::StringRef>>,
//     llvm::DenseMapInfo<llvm::StringRef, void>>::DenseSetIterator<false>
// -- `<llvm::StringRef, void>`, two arguments, where <long>/<mlir::Value>/
// <unsigned long> arrive with ONE.  A single rule family cannot serve both:
// the argument is part of the canonical string and the strings differ.
//
// WHY the two spellings differ, and why the sentinel default below is what makes
// both families expressible in ONE translation unit.  `Mapper::ToString` prints
// through clang's default PrintingPolicy, where SuppressDefaultTemplateArgs is
// TRUE, and cpp-rule-preprocessor elides defaulted arguments the same way.  So
// an argument that EQUALS its template's default vanishes from the key and an
// explicitly-different one stays -- rules/smallset records the identical
// mechanism from the other side (a written `std::less<T1>` kept a third argument
// the use site had dropped).  DenseMapInfo therefore gets TWO parameters here
// with a default that is deliberately NOT `void`:
//   * family one writes `llvm::DenseSet<T1>`, whose defaulted ValueInfoT is
//     `DenseMapInfo<T1>`; the second argument equals the default, is elided, and
//     the key keeps the ONE-argument spelling the two already-closed TUs match.
//   * family two writes the argument out through the `dsv` alias as
//     `DenseMapInfo<T1, void>`; `void` differs from the sentinel default, is
//     never suppressed, and the key comes out two-argument.
// The sentinel itself is an incomplete type that no rule ever spells, so it
// cannot appear in any key.  Verified by grepping the produced IR for both
// spellings, not by trusting the argument.
//
// REPRESENTATION IS UNCHANGED.  ValueInfoT is hashing policy: it is not
// observable through any member mapped here, and tgt_*.rs already drops it.  So
// f23..f44 are f1..f22's bodies verbatim -- the two families differ only in the
// C++ string they are keyed on.
// ---------------------------------------------------------------------------

template <typename T1> using dsv = llvm::DenseSet<T1, llvm::DenseMapInfo<T1, void>>;
template <typename T1> using t4 = dsv<T1>;
template <typename T1> using t5 = typename dsv<T1>::iterator;
template <typename T1> using t6 = typename dsv<T1>::const_iterator;

// Construction.  See the note on the class: both the nullary form and the
// reserve-taking form are mapped so the mapping does not depend on which entity
// `llvm::DenseSet<T> s;` resolves to.
template <typename T1> dsv<T1> f23() { return dsv<T1>(); }

template <typename T1> dsv<T1> f24(unsigned n) {
  return dsv<T1>(n);
}

// insert -- the load-bearing member.  Keeps the incumbent, reports whether it
// inserted; `insert(x).second` at the three abort sites is exactly this bool.
template <typename T1>
std::pair<typename dsv<T1>::iterator, bool>
f25(dsv<T1> &o, const T1 &v) {
  return o.insert(v);
}

template <typename T1>
std::pair<typename dsv<T1>::iterator, bool>
f26(dsv<T1> &o, T1 &&v) {
  return o.insert(static_cast<T1 &&>(v));
}

// contains / count -- Autopilot.cpp:647 and :668 call contains; count is what a
// probe can use to tell a deduplicating body from a non-deduplicating one.
template <typename T1>
bool f27(const dsv<T1> &o, const T1 &v) {
  return o.contains(v);
}

template <typename T1>
unsigned int f28(const dsv<T1> &o, const T1 &v) {
  return o.count(v);
}

template <typename T1> unsigned int f29(const dsv<T1> &o) {
  return o.size();
}

template <typename T1> bool f30(const dsv<T1> &o) {
  return o.empty();
}

// begin/end/find, in both const-nesses.  The const receiver hands back
// const_iterator, a different type AND a different signature -- rules/densemap
// f44..f46.
template <typename T1>
typename dsv<T1>::iterator f31(dsv<T1> &o) {
  return o.begin();
}

template <typename T1>
typename dsv<T1>::iterator f32(dsv<T1> &o) {
  return o.end();
}

template <typename T1>
typename dsv<T1>::const_iterator f33(const dsv<T1> &o) {
  return o.begin();
}

template <typename T1>
typename dsv<T1>::const_iterator f34(const dsv<T1> &o) {
  return o.end();
}

template <typename T1>
typename dsv<T1>::iterator f35(dsv<T1> &o, const T1 &v) {
  return o.find(v);
}

template <typename T1>
typename dsv<T1>::const_iterator f36(const dsv<T1> &o,
                                                const T1 &v) {
  return o.find(v);
}

// The iterator comparisons.  SPELLED AS FREE-FUNCTION CALLS, never `a == b`:
// these are hidden friends (found by ADL on the operand), and rules/smallset
// records that a plain `a == b` spelling produces NO src entry, after which the
// converter aborts EVERY translation that loads the module with
// `LLVM ERROR: Expr rule loaded from IR but has no src`.
template <typename T1>
bool f37(typename dsv<T1>::iterator a,
         typename dsv<T1>::iterator b) {
  return operator==(a, b);
}

template <typename T1>
bool f38(typename dsv<T1>::iterator a,
         typename dsv<T1>::iterator b) {
  return operator!=(a, b);
}

template <typename T1>
bool f39(typename dsv<T1>::const_iterator a,
         typename dsv<T1>::const_iterator b) {
  return operator==(a, b);
}

template <typename T1>
bool f40(typename dsv<T1>::const_iterator a,
         typename dsv<T1>::const_iterator b) {
  return operator!=(a, b);
}

// operator* and PREFIX operator++.  A begin()/end() walk is unusable without
// both, and the probe's sorted signature is how the order-refinement discussed
// in the header comment is kept out of the evidence.  DenseSet.h:131 returns
// `reference`, i.e. ValueT & for the mutable iterator and const ValueT & for the
// const one -- two different strings.  POSTFIX ++ is NOT mapped and stays loud
// (rules/densemap f39 records that it is a separate entity).
template <typename T1> T1 &f41(typename dsv<T1>::iterator it) {
  return it.operator*();
}

template <typename T1>
const T1 &f42(typename dsv<T1>::const_iterator it) {
  return it.operator*();
}

template <typename T1>
typename dsv<T1>::iterator &
f43(typename dsv<T1>::iterator &it) {
  return it.operator++();
}

template <typename T1>
typename dsv<T1>::const_iterator &
f44(typename dsv<T1>::const_iterator &it) {
  return it.operator++();
}
