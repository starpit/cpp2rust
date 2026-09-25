// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::DenseMap and its iterator.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/ADT/DenseMap.h>
// ---------------------------------------------------------------------------
// Same reason as rules/stringref, rules/raw_ostream and rules/twine:
// cpp-rule-preprocessor compiles this file with a fixed flag set, and the only
// flags that would reach LLVM's headers are absolute -I paths into whatever
// LLVM tree the target project happens to have built.  That would make `ninja`
// in this repo fail for anyone without that tree.  So the signatures LLVM
// declares are restated here instead.
//
// A rule matches on a SIGNATURE STRING, so a restatement matches iff it agrees
// with LLVM exactly.  Every signature below was read back out of a real
// translation of a real dt_src TU with `cpp2rust --verbose`, which prints
// `search expr <exact resolved signature>` -- not copied out of the header and
// hoped over.  The two that the restatement got wrong on the first attempt,
// and how, are recorded at f14 and f18/f19.
//
// WHAT AN ITERATOR COMPARISON MEANS, and why this is not a guess
// -------------------------------------------------------------
// C++ is the specification.  LLVM's own body at llvm/ADT/DenseMap.h:1225 is
//
//     return LHS.Ptr == RHS.Ptr;
//
// i.e. IDENTITY OF POSITION -- the bucket address -- and NOT equality of the
// pointed-to values.  The same is true of every sibling: DenseSet.h:145 is
// `LHS.I == RHS.I`, StringMap.h:488 is `LHS.Ptr == RHS.Ptr`, and
// ilist_iterator.h:175 is `LHS.NodePtr == RHS.NodePtr`.
//
// A value-comparing body would be SILENTLY WRONG for a container holding
// duplicate values, and that is measured here rather than argued: the
// verification probe puts 7 at two different keys and checks that iterators to
// them are UNEQUAL.  Against clang-built C++ that is 0; a value-comparing body
// gives 1.
//
// The existing MapIter (libcc2rs/src/iterators.rs:60) already has exactly this
// semantics -- `fn eq(&self) { self.key == other.key }`, i.e. position by key,
// end() being `key: None` -- so no new runtime type is needed and the LLVM
// containers reuse the BTreeMap model std::map and std::unordered_map already
// use.  DenseMap leaves its iteration order unspecified (it is a hash table),
// so the ordered BTreeMap is an order-REFINING translation, which is the same
// argument rules/unordered_map records for std::unordered_map.

#include <utility>

namespace llvm {

template <typename KeyT> struct DenseMapInfo;

namespace detail {
// llvm/ADT/DenseMap.h:45 -- `struct DenseMapPair : std::pair<KeyT, ValueT>`.
// It is the BUCKET type the iterator's operator-> yields, and it must be
// COMPLETE here or the arrow-member rules (f18/f19) cannot resolve at all:
// cpp-rule-preprocessor's lookupArrowAccess looks the member up on the
// pointee's record decl and asserts `rdecl` otherwise.
//
// DERIVING FROM std::pair IS LOAD-BEARING, and getting it wrong is the kind of
// silent non-match this file's header warns about.  `it->second` resolves
// through the BASE, so the signature clang produces is
//     llvm::DenseMapIterator<T1, T2>->std::pair<T1, T2>::second
// Restating DenseMapPair with its own `first`/`second` fields instead resolves
// to `->llvm::detail::DenseMapPair<T1, T2>::second`, which is a different
// string and never matches -- the rule generates and validates cleanly and then
// simply does nothing, which is exactly how this was found: the first attempt
// left `it->second` reaching an unmapped type and rustc said
// `type MapIter<...> cannot be dereferenced`.
template <typename KeyT, typename ValueT>
struct DenseMapPair : std::pair<KeyT, ValueT> {
  KeyT &getFirst();
  const KeyT &getFirst() const;
  ValueT &getSecond();
  const ValueT &getSecond() const;
};
}

template <typename KeyT, typename ValueT,
          typename KeyInfoT = DenseMapInfo<KeyT>,
          typename Bucket = detail::DenseMapPair<KeyT, ValueT>,
          bool IsConst = false>
class DenseMapIterator {
public:
  Bucket *operator->() const;
  DenseMapIterator &operator++();
  DenseMapIterator operator++(int);

  // llvm/ADT/DenseMap.h:1217 / :1229 -- these are HIDDEN FRIENDS, not
  // namespace-scope templates. Declared at namespace scope they are not
  // found: cpp-rule-preprocessor's namespace lookup reports "No viable
  // function" because the deduction has no way to reach the defaulted
  // template arguments. As friends they are found by ADL on the operand,
  // which is what the real call site does.
  friend bool operator==(const DenseMapIterator &LHS,
                         const DenseMapIterator &RHS);
  friend bool operator!=(const DenseMapIterator &LHS,
                         const DenseMapIterator &RHS);
};

template <typename DerivedT, typename KeyT, typename ValueT,
          typename KeyInfoT, typename BucketT>
class DenseMapBase {
public:
  using iterator = DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT>;
  using const_iterator =
      DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, true>;
  iterator begin();
  iterator end();
  iterator find(const KeyT &Val);
  const_iterator begin() const;
  const_iterator end() const;
  const_iterator find(const KeyT &Val) const;
  ValueT &operator[](const KeyT &Key);
  ValueT &operator[](KeyT &&Key);
  unsigned int count(const KeyT &Val) const;
  bool contains(const KeyT &Val) const;
  bool empty() const;
  unsigned int size() const;
  void clear();

  // The lookup/insert/erase family. Every return type here is LLVM's, not
  // std::map's, and three of them DIFFER -- see the block above f28.
  ValueT lookup(const KeyT &Val) const;
  ValueT &at(const KeyT &Val);
  const ValueT &at(const KeyT &Val) const;
  std::pair<iterator, bool> insert(const std::pair<KeyT, ValueT> &KV);
  std::pair<iterator, bool> insert(std::pair<KeyT, ValueT> &&KV);
  template <typename... Ts>
  std::pair<iterator, bool> try_emplace(KeyT &&Key, Ts &&...Args);
  template <typename... Ts>
  std::pair<iterator, bool> try_emplace(const KeyT &Key, Ts &&...Args);
  bool erase(const KeyT &Val);
  void erase(iterator I);
};

template <typename KeyT, typename ValueT,
          typename KeyInfoT = DenseMapInfo<KeyT>,
          typename BucketT = detail::DenseMapPair<KeyT, ValueT>>
class DenseMap : public DenseMapBase<DenseMap<KeyT, ValueT, KeyInfoT, BucketT>,
                                     KeyT, ValueT, KeyInfoT, BucketT> {
public:
  DenseMap();
  explicit DenseMap(unsigned InitialReserve);
};

} // namespace llvm

template <typename T1, typename T2> using t1 = llvm::DenseMap<T1, T2>;
template <typename T1, typename T2>
using t2 = typename llvm::DenseMap<T1, T2>::iterator;

template <typename T1, typename T2>
bool f1(typename llvm::DenseMap<T1, T2>::iterator a,
        typename llvm::DenseMap<T1, T2>::iterator b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
bool f2(typename llvm::DenseMap<T1, T2>::iterator a,
        typename llvm::DenseMap<T1, T2>::iterator b) {
  return operator!=(a, b);
}

template <typename T1, typename T2>
typename llvm::DenseMap<T1, T2>::iterator f3(llvm::DenseMap<T1, T2> &o,
                                             const T1 &k) {
  return o.find(k);
}

template <typename T1, typename T2>
typename llvm::DenseMap<T1, T2>::iterator f4(llvm::DenseMap<T1, T2> &o) {
  return o.end();
}

template <typename T1, typename T2>
typename llvm::DenseMap<T1, T2>::iterator f5(llvm::DenseMap<T1, T2> &o) {
  return o.begin();
}

template <typename T1, typename T2> llvm::DenseMap<T1, T2> f6() {
  return llvm::DenseMap<T1, T2>();
}

template <typename T1, typename T2>
T2 &f7(llvm::DenseMap<T1, T2> &o, const T1 &k) {
  return o.operator[](k);
}

template <typename T1, typename T2>
unsigned int f8(const llvm::DenseMap<T1, T2> &o, const T1 &k) {
  return o.count(k);
}

template <typename T1, typename T2>
bool f9(const llvm::DenseMap<T1, T2> &o, const T1 &k) {
  return o.contains(k);
}

template <typename T1, typename T2>
bool f10(const llvm::DenseMap<T1, T2> &o) {
  return o.empty();
}

template <typename T1, typename T2>
unsigned int f11(const llvm::DenseMap<T1, T2> &o) {
  return o.size();
}

template <typename T1, typename T2> void f12(llvm::DenseMap<T1, T2> &o) {
  return o.clear();
}

// The ctor as clang actually resolves it at a call site: LLVM's is
// `explicit DenseMap(unsigned InitialReserve = 0)`, so a default-constructed
// DenseMap resolves to `DenseMap(unsigned int)`, NOT `DenseMap()`. Observed on
// a real translation as
//   void llvm::DenseMap<long, unsigned int>::DenseMap(unsigned int)
template <typename T1, typename T2> llvm::DenseMap<T1, T2> f14(unsigned n) {
  return llvm::DenseMap<T1, T2>(n);
}

// operator[] on an RVALUE key. LLVM declares both `operator[](const KeyT &)`
// and `operator[](KeyT &&)`; `m[1] = x` with a literal picks the second, and
// the first (f7) does not match it.
template <typename T1, typename T2> T2 &f15(llvm::DenseMap<T1, T2> &o, T1 &&k) {
  return o.operator[](static_cast<T1 &&>(k));
}

// PREFIX ++ on the iterator. A begin()/end() loop is unusable without it, and
// 11 of the 33 dt_src TUs whose == / != this module fixes also have a `++` on
// the same iterator, so the two travel together.
template <typename T1, typename T2>
typename llvm::DenseMap<T1, T2>::iterator &
f16(typename llvm::DenseMap<T1, T2>::iterator &it) {
  return it.operator++();
}

// POSTFIX ++. libc++/LLVM print the two distinctly -- the prefix returns
// `DenseMapIterator &` and the postfix returns `DenseMapIterator` and takes an
// int -- so f16 does not cover this one. Same split rules/map f33 records.
template <typename T1, typename T2>
typename llvm::DenseMap<T1, T2>::iterator
f17(typename llvm::DenseMap<T1, T2>::iterator &it, int a1) {
  return it.operator++(a1);
}

// it->first and it->second on a DenseMap iterator.  rules/map's f22/f23 do NOT
// cover these even though both end up spelled over std::pair: an arrow rule is
// keyed on the WHOLE string including the receiver, and the receiver here is a
// DenseMapIterator, not a std::__map_iterator.
//
// Note `first` is NOT const-qualified the way std::map's pair<const K, V> is:
// LLVM's bucket is a plain pair of KeyT and ValueT.  The rule reflects that.
template <typename T1, typename T2>
T1 &f18(typename llvm::DenseMap<T1, T2>::iterator it) {
  return it->first;
}

template <typename T1, typename T2>
T2 &f19(typename llvm::DenseMap<T1, T2>::iterator it) {
  return it->second;
}

// THE CONST ITERATOR IS A DIFFERENT TYPE AND A DIFFERENT SIGNATURE.
//
// cpp2rust prints DenseMap<K,V>::iterator as `llvm::DenseMapIterator<K, V>`
// (the defaulted template arguments elided) but const_iterator as
// `llvm::DenseMapIterator<K, V, llvm::DenseMapInfo<K>,
//  llvm::detail::DenseMapPair<K, V>, true>` -- the trailing `true` forces the
// rest to be spelled. So the two are DISTINCT strings and each needs its own
// rule; there is NO duplicate-type-rule collision of the kind rules/set records
// for std::set<T, Comparator> (verified: the two resolve to different strings on
// a probe that uses both in one TU).
//
// This is not hypothetical: dcc/.../AgenToSentient/Helper.cpp:772 reaches
// `indices_coeff_dict.find(nullptr) != indices_coeff_dict.end()` through a
// const receiver, and its only recorded gap was the const form.
//
// Both map to the SAME Rust type, which mapper.cpp:781 accepts (two C++ types
// may map to one identical Rust type -- the rules/vector iterator idempotence).
template <typename T1, typename T2>
using t3 = typename llvm::DenseMap<T1, T2>::const_iterator;

template <typename T1, typename T2>
bool f20(typename llvm::DenseMap<T1, T2>::const_iterator a,
         typename llvm::DenseMap<T1, T2>::const_iterator b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
bool f21(typename llvm::DenseMap<T1, T2>::const_iterator a,
         typename llvm::DenseMap<T1, T2>::const_iterator b) {
  return operator!=(a, b);
}

template <typename T1, typename T2>
typename llvm::DenseMap<T1, T2>::const_iterator
f22(const llvm::DenseMap<T1, T2> &o, const T1 &k) {
  return o.find(k);
}

template <typename T1, typename T2>
typename llvm::DenseMap<T1, T2>::const_iterator
f23(const llvm::DenseMap<T1, T2> &o) {
  return o.end();
}

template <typename T1, typename T2>
typename llvm::DenseMap<T1, T2>::const_iterator
f24(const llvm::DenseMap<T1, T2> &o) {
  return o.begin();
}

template <typename T1, typename T2>
typename llvm::DenseMap<T1, T2>::const_iterator &
f25(typename llvm::DenseMap<T1, T2>::const_iterator &it) {
  return it.operator++();
}

template <typename T1, typename T2>
const T2 &f26(typename llvm::DenseMap<T1, T2>::const_iterator it) {
  return it->second;
}

template <typename T1, typename T2>
const T1 &f27(typename llvm::DenseMap<T1, T2>::const_iterator it) {
  return it->first;
}

// ---------------------------------------------------------------------------
// lookup / at / insert / try_emplace / erase.  Added ON EVIDENCE: a census of
// llvm::DenseMap receivers across dt_src counted insert 86 calls, at 71,
// lookup 31, erase 10 and try_emplace 7, and before this every one of them
// emitted a nonexistent BTreeMap method -- measured, in both models:
//   no method named `insert_pmuti32_i64_rv` / `lookup` / `at_pconsti32`
//   / `erase_pconsti32` / `erase_UnsafeMapIteratori32_i64` / `try_emplace`
//
// THREE OF THESE DISAGREE WITH std::map, so a body copied from rules/map
// would be silently wrong rather than absent.  LLVM's own bodies are the
// specification and each was read out of llvm/ADT/DenseMap.h:
//
//   * erase(key) returns BOOL (DenseMap.h:330 `return false; // not in map.`).
//     std::map::erase(key) returns the NUMBER erased, and rules/map's f43
//     accordingly answers 0/1 as a size_t.  Returning a count here would be
//     wrong at every `if (m.erase(k))` and every arithmetic use.
//   * erase(iterator) returns VOID (DenseMap.h:341).  std::map's returns the
//     FOLLOWING iterator, which is rules/map's f3.  A body that returned one
//     would typecheck in Rust and mean something C++ never said.
//   * lookup(key) returns ValueT BY VALUE, default-constructed when the key is
//     absent, and DOES NOT INSERT (DenseMap.h:205).  This is the whole point of
//     lookup existing beside operator[], which DOES insert -- so reusing f7's
//     entry().or_insert_with() body would silently grow the map.  The probe
//     reads size() back after looking up an absent key for exactly this.
//
// The other three agree with std::map and reuse its shape:
//   * insert(pair&&) and try_emplace KEEP THE INCUMBENT on a duplicate key and
//     report false (DenseMap.h:241, :256), whereas BTreeMap::insert OVERWRITES.
//     Same trap rules/map's f45/f50 record; the probe inserts 99 over an
//     existing 10 and asserts it is still 10.
//   * at(key) returns ValueT& and ABORTS on a missing key (DenseMap.h:224 is an
//     assert).  `.expect(...)` keeps that loud instead of inventing a value.
//
// Measured against $TC/shim4/clang++ before any rule was written:
//   1 1 10 1 / 2 0 10 / 3 0 10 / 4 1 20 2 / 5 0 2 / 6 21 / 7 1 0 1 / 8 1 1
// A BTreeMap::insert body gives `2 0 99`; an operator[] body for lookup gives
// `5 0 3`; a count-returning erase gives a different line 7.
//
// Signatures were read back out of a real translation with --verbose, as this
// file's header requires, NOT copied from the header and hoped over:
//   bool ...DenseMapBase<...>::erase(const int &)
//   void ...DenseMapBase<...>::erase(llvm::DenseMapIterator<int, long>)
//   long ...DenseMapBase<...>::lookup(const int &) const
//   long & ...DenseMapBase<...>::at(const int &)
//   std::pair<llvm::DenseMapIterator<int, long>, bool> ...::insert(std::pair<int, long> &&)
//   std::pair<llvm::DenseMapIterator<int, long>, bool> ...::try_emplace(int &&, &&...)
// Note try_emplace's parameter pack prints as `&&...`, so it has to be RESTATED
// as a pack: a non-variadic `try_emplace(KeyT &&, ValueT &&)` resolves to a
// different string and would never match.
//
// NOTE the LLVM include form matters for whether this module is reached at all.
// The dt_src database passes `-isystem <llvm>/include`, which makes DenseMap a
// system type and lets these rules apply.  Under a plain `-I` the same headers
// are USER code, Mapper::AddRuleForUserDefinedType claims the instantiation and
// every rule here is silently bypassed in favour of translating LLVM's own
// bodies.  Measured both ways; `-isystem` is what the real build uses.
// ---------------------------------------------------------------------------

template <typename T1, typename T2>
std::pair<typename llvm::DenseMap<T1, T2>::iterator, bool>
f28(llvm::DenseMap<T1, T2> &o, std::pair<T1, T2> &&v) {
  return o.insert(static_cast<std::pair<T1, T2> &&>(v));
}

template <typename T1, typename T2>
std::pair<typename llvm::DenseMap<T1, T2>::iterator, bool>
f29(llvm::DenseMap<T1, T2> &o, T1 &&key, T2 &&value) {
  return o.try_emplace(static_cast<T1 &&>(key), static_cast<T2 &&>(value));
}

template <typename T1, typename T2>
T2 f30(const llvm::DenseMap<T1, T2> &o, const T1 &key) {
  return o.lookup(key);
}

template <typename T1, typename T2>
T2 &f31(llvm::DenseMap<T1, T2> &o, const T1 &key) {
  return o.at(key);
}

template <typename T1, typename T2>
bool f32(llvm::DenseMap<T1, T2> &o, const T1 &key) {
  return o.erase(key);
}

template <typename T1, typename T2>
void f33(llvm::DenseMap<T1, T2> &o,
         typename llvm::DenseMap<T1, T2>::iterator it) {
  return o.erase(it);
}

// THE CONST-RECEIVER AND LVALUE-ARGUMENT OVERLOADS ARE DIFFERENT SIGNATURES.
// Same lesson this file already records for find/begin/end (f22..f24) and for
// operator[] on an rvalue key (f15): a rule is keyed on the WHOLE resolved
// string, so an overload the port actually reaches and this module does not
// restate is a rule that validates cleanly and then silently does nothing.
//
// Each of the three below was produced by a PLAIN C++ shape, read off with
// --verbose, not guessed:
//   static long readback(const llvm::DenseMap<int,long> &m) { return m.at(1); }
//     -> const long & ...::at(const int &) const        (f34, distinct from f31)
//   std::pair<int,long> kv(1, 10L); m.insert(kv);
//     -> ...::insert(const std::pair<int, long> &)      (f35, distinct from f28)
//   int k = 3; m.try_emplace(k, 30L);
//     -> ...::try_emplace(const int &, &&...)           (f36, distinct from f29)
//
// The const `at` is the one most likely to be load-bearing: reading a map
// through a `const DenseMap &` parameter is the ordinary way to pass one, and
// src.cpp already records dcc/.../AgenToSentient/Helper.cpp:772 needing the
// const form of `find` for exactly that reason.

template <typename T1, typename T2>
const T2 &f34(const llvm::DenseMap<T1, T2> &o, const T1 &key) {
  return o.at(key);
}

template <typename T1, typename T2>
std::pair<typename llvm::DenseMap<T1, T2>::iterator, bool>
f35(llvm::DenseMap<T1, T2> &o, const std::pair<T1, T2> &v) {
  return o.insert(v);
}

template <typename T1, typename T2>
std::pair<typename llvm::DenseMap<T1, T2>::iterator, bool>
f36(llvm::DenseMap<T1, T2> &o, const T1 &key, T2 &&value) {
  return o.try_emplace(key, static_cast<T2 &&>(value));
}
