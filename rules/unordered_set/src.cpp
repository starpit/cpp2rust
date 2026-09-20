// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstddef>
#include <unordered_set>
#include <utility>

// C++ leaves the iteration order of std::unordered_set unspecified, so mapping
// it onto an ordered BTreeMap, as rules/set does for std::set, is a valid
// (order refining) translation and needs no new runtime type: the existing
// RefcountMapIter / UnsafeMapIterator cover the iterators. The mapped slot
// holds a second copy of the element rather than unit, so that `*it` -- an
// lvalue of type `const T &` -- has a cell to point at; see the target files.
template <typename T1> using t1 = std::unordered_set<T1>;

// In libc++ std::unordered_set<T>::iterator and ::const_iterator are the same
// type (both __table::const_iterator), so one rule covers both.
template <typename T1> using t2 = typename std::unordered_set<T1>::iterator;

template <typename T1> std::size_t f1(const std::unordered_set<T1> &o) {
  return o.size();
}

template <typename T1> bool f2(const std::unordered_set<T1> &o) {
  return o.empty();
}

template <typename T1> void f3(std::unordered_set<T1> &o) { return o.clear(); }

template <typename T1>
std::size_t f4(const std::unordered_set<T1> &o, const T1 &k) {
  return o.count(k);
}

template <typename T1>
typename std::unordered_set<T1>::iterator f5(std::unordered_set<T1> &o,
                                             const T1 &k) {
  return o.find(k);
}

template <typename T1>
typename std::unordered_set<T1>::const_iterator
f6(const std::unordered_set<T1> &o, const T1 &k) {
  return o.find(k);
}

template <typename T1>
typename std::unordered_set<T1>::iterator f7(std::unordered_set<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::unordered_set<T1>::iterator f8(std::unordered_set<T1> &o) {
  return o.end();
}

template <typename T1>
typename std::unordered_set<T1>::const_iterator
f9(const std::unordered_set<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::unordered_set<T1>::const_iterator
f10(const std::unordered_set<T1> &o) {
  return o.end();
}

template <typename T1>
std::pair<typename std::unordered_set<T1>::iterator, bool>
f11(std::unordered_set<T1> &o, const T1 &v) {
  return o.insert(v);
}

template <typename T1>
std::size_t f12(std::unordered_set<T1> &o, const T1 &k) {
  return o.erase(k);
}

template <typename T1> std::unordered_set<T1> f13() {
  return std::unordered_set<T1>();
}

template <typename T1>
std::unordered_set<T1> f14(const std::unordered_set<T1> &o) {
  return std::unordered_set<T1>(o);
}

template <typename T1>
std::unordered_set<T1> &f15(std::unordered_set<T1> &dst,
                            const std::unordered_set<T1> &src) {
  return dst.operator=(src);
}

template <typename T1>
bool f16(typename std::unordered_set<T1>::iterator a,
         typename std::unordered_set<T1>::iterator b) {
  return operator!=(a, b);
}

template <typename T1>
bool f17(typename std::unordered_set<T1>::iterator a,
         typename std::unordered_set<T1>::iterator b) {
  return operator==(a, b);
}

template <typename T1>
const T1 &f18(typename std::unordered_set<T1>::iterator it) {
  return it.operator*();
}

template <typename T1>
std::pair<typename std::unordered_set<T1>::iterator, bool>
f19(std::unordered_set<T1> &o, T1 &&v) {
  return o.insert(std::move(v));
}

template <typename T1>
typename std::unordered_set<T1>::iterator &
f20(typename std::unordered_set<T1>::iterator &it) {
  return it.operator++();
}

template <typename T1>
typename std::unordered_set<T1>::iterator
f21(typename std::unordered_set<T1>::iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1>
typename std::unordered_set<T1>::iterator
f22(std::unordered_set<T1> &o, typename std::unordered_set<T1>::iterator it) {
  return o.erase(it);
}

template <typename T1>
std::unordered_set<T1> f23(std::unordered_set<T1> &&o) {
  return std::unordered_set<T1>(std::move(o));
}

template <typename T1>
std::unordered_set<T1> &f24(std::unordered_set<T1> &dst,
                            std::unordered_set<T1> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1>
std::pair<typename std::unordered_set<T1>::iterator, bool>
f25(std::unordered_set<T1> &o, T1 &&v) {
  return o.emplace(std::move(v));
}

template <typename T1>
std::pair<typename std::unordered_set<T1>::iterator, bool>
f26(std::unordered_set<T1> &o, const T1 &v) {
  return o.emplace(v);
}

template <typename T1>
void f27(std::unordered_set<T1> &o, std::unordered_set<T1> &a0) {
  return o.swap(a0);
}

template <typename T1>
typename std::unordered_set<T1>::const_iterator
f28(const std::unordered_set<T1> &o) {
  return o.cbegin();
}

template <typename T1>
typename std::unordered_set<T1>::const_iterator
f29(const std::unordered_set<T1> &o) {
  return o.cend();
}

template <typename T1>
bool f30(const std::unordered_set<T1> &a, const std::unordered_set<T1> &b) {
  return operator==(a, b);
}

template <typename T1>
bool f31(const std::unordered_set<T1> &a, const std::unordered_set<T1> &b) {
  return operator!=(a, b);
}
