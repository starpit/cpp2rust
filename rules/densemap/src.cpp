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
