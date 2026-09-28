// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::unordered_map<K,V> -> HashMap<K,V>, std::unordered_set<K> -> HashSet<K>.
//
// SEMANTIC DIFFERENCE, NOT PAPERED OVER: std::map is ordered and BTreeMap matches it,
// but std::unordered_map / std::unordered_set have UNSPECIFIED iteration order and
// Rust's HashMap/HashSet order differs from libc++'s (and, being randomly seeded,
// differs between runs). Lookup, insertion, erase, size, count, contains and
// container equality (which is order-independent for unordered containers) are
// faithful. Any code that ITERATES an unordered container and depends on the order
// it observes is behaviour this mapping does NOT reproduce -- nor does C++ portably.
//
// std::hash<T> is deliberately LEFT UNMAPPED: the converter already ports it
// successfully, and HashMap brings its own hasher, so no hash function is modelled here.
//
// Iterator-returning operations (find/begin/end/insert/emplace) are NOT mapped: the
// only iterator helpers in libcc2rs (UnsafeMapIterator/RefcountMapIter) are keyed on
// BTreeMap and require Ord, so a HashMap iterator would be a libcc2rs change outside
// this module. Those keys are omitted rather than approximated.
//
// contains() is NOT mapped either: the rule preprocessor parses at C++17, where
// unordered_map::contains / unordered_set::contains do not exist ("No viable
// function"). count() covers the same corpus uses. Likewise operator!= is absent
// from libc++'s C++20 headers (rewritten from ==), so only operator== is mapped.

#include <unordered_map>
#include <unordered_set>
#include <utility>

template <typename T1, typename T2> using t1 = std::unordered_map<T1, T2>;

template <typename T1> using t2 = std::unordered_set<T1>;

template <typename T1, typename T2>
T2 &f1(std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.operator[](key);
}

template <typename T1, typename T2>
std::size_t f2(const std::unordered_map<T1, T2> &o) {
  return o.size();
}

template <typename T1, typename T2> std::unordered_map<T1, T2> f3() {
  return std::unordered_map<T1, T2>();
}

template <typename T1, typename T2>
T2 &f4(std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.at(key);
}

template <typename T1, typename T2>
const T2 &f5(const std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.at(key);
}

template <typename T1, typename T2>
std::size_t f6(const std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.count(key);
}

template <typename T1, typename T2>
std::size_t f8(std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.erase(key);
}

template <typename T1, typename T2>
bool f9(const std::unordered_map<T1, T2> &o) {
  return o.empty();
}

template <typename T1, typename T2> void f10(std::unordered_map<T1, T2> &o) {
  return o.clear();
}

template <typename T1, typename T2>
bool f11(const std::unordered_map<T1, T2> &a, const std::unordered_map<T1, T2> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
std::unordered_map<T1, T2> &f13(std::unordered_map<T1, T2> &dst,
                                const std::unordered_map<T1, T2> &src) {
  return dst.operator=(src);
}

template <typename T1> std::unordered_set<T1> f14() {
  return std::unordered_set<T1>();
}

template <typename T1> std::size_t f15(const std::unordered_set<T1> &o) {
  return o.size();
}

template <typename T1>
std::size_t f16(const std::unordered_set<T1> &o, const T1 &key) {
  return o.count(key);
}

template <typename T1>
std::size_t f18(std::unordered_set<T1> &o, const T1 &key) {
  return o.erase(key);
}

template <typename T1> bool f19(const std::unordered_set<T1> &o) {
  return o.empty();
}

template <typename T1> void f20(std::unordered_set<T1> &o) {
  return o.clear();
}

template <typename T1>
bool f21(const std::unordered_set<T1> &a, const std::unordered_set<T1> &b) {
  return operator==(a, b);
}


// ---------------------------------------------------------------------------
// Iterator-returning operations. Backed by libcc2rs's HashMapIter
// (UnsafeHashMapIterator / RefcountHashMapIter), added alongside the BTreeMap
// MapIter and following the same protocol, so the converter's for-range lowering
// and the it->first / it->second routing to MapIterator::first/second work unchanged.
// ORDER CAVEAT: see libcc2rs/src/iterators.rs -- traversal visits every element
// exactly once but in an order that differs from libc++'s.
//
// insert / emplace are still NOT mapped: they return std::pair<iterator, bool>, whose
// lowering needs a pair whose first element is a mapped iterator type, and that is not
// modelled here. Left out rather than approximated.
// ---------------------------------------------------------------------------

template <typename T1, typename T2>
using t3 = typename std::unordered_map<T1, T2>::iterator;

template <typename T1, typename T2>
using t4 = typename std::unordered_map<T1, T2>::const_iterator;

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator f22(std::unordered_map<T1, T2> &o) {
  return o.begin();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator f23(std::unordered_map<T1, T2> &o) {
  return o.end();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator f24(std::unordered_map<T1, T2> &o,
                                                  const T1 &key) {
  return o.find(key);
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator
f25(const std::unordered_map<T1, T2> &o) {
  return o.begin();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator
f26(const std::unordered_map<T1, T2> &o) {
  return o.end();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator
f27(const std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.find(key);
}

template <typename T1, typename T2>
bool f28(typename std::unordered_map<T1, T2>::iterator a,
         typename std::unordered_map<T1, T2>::iterator b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
bool f29(typename std::unordered_map<T1, T2>::iterator a,
         typename std::unordered_map<T1, T2>::iterator b) {
  return operator!=(a, b);
}

template <typename T1, typename T2>
bool f30(typename std::unordered_map<T1, T2>::const_iterator a,
         typename std::unordered_map<T1, T2>::const_iterator b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
bool f31(typename std::unordered_map<T1, T2>::const_iterator a,
         typename std::unordered_map<T1, T2>::const_iterator b) {
  return operator!=(a, b);
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator &
f32(typename std::unordered_map<T1, T2>::iterator &it) {
  return it.operator++();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator
f33(typename std::unordered_map<T1, T2>::iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator &
f34(typename std::unordered_map<T1, T2>::const_iterator &it) {
  return it.operator++();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator
f35(typename std::unordered_map<T1, T2>::const_iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1, typename T2>
const T1 &f36(typename std::unordered_map<T1, T2>::iterator it) {
  return it->first;
}

template <typename T1, typename T2>
T2 &f37(typename std::unordered_map<T1, T2>::iterator it) {
  return it->second;
}

template <typename T1, typename T2>
const T1 &f38(typename std::unordered_map<T1, T2>::const_iterator it) {
  return it->first;
}

template <typename T1, typename T2>
const T2 &f39(typename std::unordered_map<T1, T2>::const_iterator it) {
  return it->second;
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator
f40(std::unordered_map<T1, T2> &o,
    typename std::unordered_map<T1, T2>::const_iterator it) {
  return o.erase(it);
}


// ---------------------------------------------------------------------------
// std::unordered_set<T1>'s iterator.  MEASURED DISTINCTION, do NOT reuse t3/t4:
// libc++ spells it `std::__hash_const_iterator<std::__hash_node<T1, void *> *>`,
// with the element type T1 DIRECTLY -- there is NO `std::__hash_value_type<K, V>`
// wrapper, which is exactly what every unordered_MAP iterator key above carries.
// In libc++ `unordered_set<T1>::iterator` and `::const_iterator` are the SAME
// type, so ONE type key (t5) serves both; only the RECEIVER's constness
// distinguishes begin()/end()/find() below, and those are separate keys.
//
// ORDER CAVEAT (not a defect, and not papered over): traversal visits every
// element exactly once, but the ORDER differs from libc++'s and is unspecified in
// C++ either way.  See the header comment and libcc2rs/src/iterators.rs.
// ---------------------------------------------------------------------------

template <typename T1> using t5 = typename std::unordered_set<T1>::const_iterator;

// Both insert overloads are required: `s.insert(33)` on a literal binds
// insert(T1 &&), not insert(const T1 &), and a missing overload emits the mangled
// fallback name instead of the rule (measured on rules/set).
template <typename T1>
std::pair<typename std::unordered_set<T1>::iterator, bool>
f41(std::unordered_set<T1> &o, const T1 &k) {
  return o.insert(k);
}

template <typename T1>
std::pair<typename std::unordered_set<T1>::iterator, bool>
f42(std::unordered_set<T1> &o, T1 &&k) {
  return o.insert(std::move(k));
}

template <typename T1>
typename std::unordered_set<T1>::const_iterator f43(const std::unordered_set<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::unordered_set<T1>::const_iterator f44(const std::unordered_set<T1> &o) {
  return o.end();
}

template <typename T1>
typename std::unordered_set<T1>::iterator f45(std::unordered_set<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::unordered_set<T1>::iterator f46(std::unordered_set<T1> &o) {
  return o.end();
}

template <typename T1>
typename std::unordered_set<T1>::const_iterator
f47(const std::unordered_set<T1> &o, const T1 &k) {
  return o.find(k);
}

template <typename T1>
typename std::unordered_set<T1>::iterator f48(std::unordered_set<T1> &o,
                                              const T1 &k) {
  return o.find(k);
}

template <typename T1>
bool f49(typename std::unordered_set<T1>::const_iterator a,
         typename std::unordered_set<T1>::const_iterator b) {
  return operator==(a, b);
}

template <typename T1>
bool f50(typename std::unordered_set<T1>::const_iterator a,
         typename std::unordered_set<T1>::const_iterator b) {
  return operator!=(a, b);
}

template <typename T1>
typename std::unordered_set<T1>::const_iterator &
f51(typename std::unordered_set<T1>::const_iterator &it) {
  return it.operator++();
}

template <typename T1>
typename std::unordered_set<T1>::const_iterator
f52(typename std::unordered_set<T1>::const_iterator a0, int a1) {
  return a0.operator++(a1);
}

// f53 (operator* on an unordered_set iterator) is DELIBERATELY ABSENT, loudly.
// It needs libcc2rs::SetIterator implemented for HashSetIter (added in
// libcc2rs/src/iterators.rs), but the rule preprocessor type-checks targets against a
// PINNED libcc2rs artifact, and its staleness guard is keyed on new ITEM NAMES -- a new
// trait IMPL adds no name, so the guard passes and rustc then rejects the target with
// E0277 "the trait bound HashSetIter<..>: SetIterator is not satisfied".  Re-add f53 once
// the pinned libcc2rs is refreshed; do not approximate it.

template <typename T1>
typename std::unordered_set<T1>::iterator
f54(std::unordered_set<T1> &o, typename std::unordered_set<T1>::const_iterator it) {
  return o.erase(it);
}

// f55 -- operator[] TAKING AN RVALUE KEY.  MEASURED 2026-09-28 from
// `probe/umapdecomp/p.cpp` (`std::unordered_map<int,long> um; um[7] = 100;`):
//     search expr long & std::unordered_map<int, long>::operator[](int &&), result:
//     None
// f1 keys only the `const T1 &` overload, so a SUBSCRIPT WITH A PRVALUE KEY -- which
// is what `um[7]`, `um[i + 1]` and `um[some_call()]` all are, since libc++ declares
// both `mapped_type &operator[](const key_type &)` and `mapped_type &operator[](key_type &&)`
// and overload resolution picks the && one for any rvalue -- found NO key and fell
// through to the converter's generic array-subscript lowering, emitting
// `um[(7) as usize] = 100_i64` against a `HashMap<i32, Box<i64>>` (two E0308s: the
// index is not a `&i32`, and the value is not a `Box<i64>`).  `rules/map` has had
// this pair since the start (f1 + f8, byte-identical bodies); this is the missing
// unordered twin.  The body is `entry(k).or_default()`, which INSERTS a
// default-constructed value for a missing key -- exactly what C++ `operator[]` does,
// and why it must not be a `get`/panic form.
template <typename T1, typename T2>
T2 &f55(std::unordered_map<T1, T2> &o, T1 &&key) {
  return o.operator[](std::move(key));
}

// f56 -- the INITIALIZER-LIST CONSTRUCTOR, i.e. `std::unordered_map<K,V> m = {{k,v}, ...}`.
// MEASURED 2026-09-28 from the fallback readback of `probe/umilist/p.cpp`:
//     std_unordered_map_std_string__int__std_hash_std_string___std_equal_to_std_string___
//     std_allocator_std_pair_const_std_string__int___ :: new_1 ( { vec! [ (..) , .. ] } , )
// Two facts that differ from `rules/map`'s f35 and were read off the emission rather than
// guessed: (a) the braced init list already lowers to a `vec![...]`, same as map; but
// (b) there is NO extra argument -- the call site carries the init list ALONE, with a
// single trailing comma and no `None`.  map's f35 needs the `const std::less<T1> &`
// parameter because libc++ declares `map(initializer_list, const key_compare& = ...)`
// as ONE constructor with a DEFAULTED comparator, which the recorder collapses to a
// literal `None` argument; libc++'s `unordered_map(initializer_list<value_type>)` is by
// contrast its own 1-parameter OVERLOAD (the bucket-count/hasher/equal/allocator forms
// are separate overloads), so Hash, KeyEqual and Allocator appear only in the mangled
// TYPE name and never as arguments.  Hence a 1-arg key, not a 4-arg one.
// The body uses `.rev()` before `collect()` because HashMap's FromIterator inserts in
// iteration order and a later duplicate REPLACES the value (last-wins), while
// std::unordered_map's init-list construction keeps the FIRST of equivalent keys;
// reversing makes the two agree.  `a0` occurs EXACTLY ONCE -- a rule body is inlined as
// one expression, so every `aN` occurrence re-evaluates that argument.
template <typename T1, typename T2>
std::unordered_map<T1, T2>
f56(const std::initializer_list<std::pair<const T1, T2>> &a0) {
  return std::unordered_map<T1, T2>(a0);
}
