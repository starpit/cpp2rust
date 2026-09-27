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

