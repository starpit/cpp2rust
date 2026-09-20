// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstddef>
#include <unordered_map>
#include <utility>

// C++ leaves the iteration order of std::unordered_map unspecified, so reusing
// the ordered BTreeMap model already used for std::map is a valid (order
// refining) translation and needs no new runtime type: the existing
// RefcountMapIter / UnsafeMapIterator cover the iterators.
template <typename T1, typename T2> using t1 = std::unordered_map<T1, T2>;

// Unlike std::unordered_set, libc++ gives unordered_map distinct iterator and
// const_iterator types, so both need a rule.
template <typename T1, typename T2>
using t2 = typename std::unordered_map<T1, T2>::const_iterator;

template <typename T1, typename T2>
using t3 = typename std::unordered_map<T1, T2>::iterator;

template <typename T1, typename T2>
T2 &f1(std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.operator[](key);
}

template <typename T1, typename T2>
std::size_t f2(const std::unordered_map<T1, T2> &o) {
  return o.size();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator
f3(std::unordered_map<T1, T2> &o,
   typename std::unordered_map<T1, T2>::iterator it) {
  return o.erase(it);
}

template <typename T1, typename T2>
bool f4(const std::unordered_map<T1, T2> &o) {
  return o.empty();
}

template <typename T1, typename T2> std::unordered_map<T1, T2> f5() {
  return std::unordered_map<T1, T2>();
}

template <typename T1, typename T2>
std::unordered_map<T1, T2> f6(const std::unordered_map<T1, T2> &o) {
  return std::unordered_map<T1, T2>(o);
}

template <typename T1, typename T2>
T2 &f7(std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.at(key);
}

template <typename T1, typename T2>
T2 &f8(std::unordered_map<T1, T2> &o, T1 &&key) {
  return o.operator[](std::move(key));
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator
f9(const std::unordered_map<T1, T2> &o) {
  return o.end();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator
f10(std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.find(key);
}

template <typename T1, typename T2>
bool f11(typename std::unordered_map<T1, T2>::iterator a,
         typename std::unordered_map<T1, T2>::iterator b) {
  return operator!=(a, b);
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator
f12(std::unordered_map<T1, T2> &o) {
  return o.begin();
}

template <typename T1, typename T2>
bool f13(typename std::unordered_map<T1, T2>::const_iterator a,
         typename std::unordered_map<T1, T2>::const_iterator b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator
f14(std::unordered_map<T1, T2> &o) {
  return o.end();
}

template <typename T1, typename T2>
const T2 &f15(const std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.at(key);
}

template <typename T1, typename T2>
bool f16(typename std::unordered_map<T1, T2>::iterator a,
         typename std::unordered_map<T1, T2>::iterator b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator
f17(const std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.find(key);
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator
f18(const std::unordered_map<T1, T2> &o) {
  return o.begin();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator
f19(const typename std::unordered_map<T1, T2>::iterator &it) {
  return typename std::unordered_map<T1, T2>::const_iterator(it);
}

template <typename T1, typename T2>
const T1 &f20(typename std::unordered_map<T1, T2>::const_iterator it) {
  return it->first;
}

template <typename T1, typename T2>
const T2 &f21(typename std::unordered_map<T1, T2>::const_iterator it) {
  return it->second;
}

template <typename T1, typename T2>
const T1 &f22(typename std::unordered_map<T1, T2>::iterator it) {
  return it->first;
}

template <typename T1, typename T2>
T2 &f23(typename std::unordered_map<T1, T2>::iterator it) {
  return it->second;
}

template <typename T1, typename T2>
std::unordered_map<T1, T2> f24(std::unordered_map<T1, T2> &&o) {
  return std::unordered_map<T1, T2>(std::move(o));
}

template <typename T1, typename T2>
std::unordered_map<T1, T2> &f25(std::unordered_map<T1, T2> &dst,
                                std::unordered_map<T1, T2> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1, typename T2>
std::unordered_map<T1, T2> &f26(std::unordered_map<T1, T2> &dst,
                                const std::unordered_map<T1, T2> &src) {
  return dst.operator=(src);
}

template <typename T1, typename T2>
bool f27(typename std::unordered_map<T1, T2>::const_iterator a,
         typename std::unordered_map<T1, T2>::const_iterator b) {
  return operator!=(a, b);
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator &
f28(typename std::unordered_map<T1, T2>::iterator &it) {
  return it.operator++();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator &
f29(typename std::unordered_map<T1, T2>::const_iterator &it) {
  return it.operator++();
}

template <typename T1, typename T2>
void f30(std::unordered_map<T1, T2> &o) {
  return o.clear();
}

template <typename T1, typename T2>
std::size_t f31(const std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.count(key);
}

template <typename T1, typename T2>
std::pair<typename std::unordered_map<T1, T2>::iterator, bool>
f32(std::unordered_map<T1, T2> &o, const std::pair<const T1, T2> &v) {
  return o.insert(v);
}

template <typename T1, typename T2>
std::pair<typename std::unordered_map<T1, T2>::iterator, bool>
f33(std::unordered_map<T1, T2> &o, std::pair<const T1, T2> &&v) {
  return o.insert(std::move(v));
}

template <typename T1, typename T2>
std::size_t f34(std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.erase(key);
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator
f35(typename std::unordered_map<T1, T2>::iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator
f36(typename std::unordered_map<T1, T2>::const_iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator
f37(std::unordered_map<T1, T2> &o,
    typename std::unordered_map<T1, T2>::const_iterator it) {
  return o.erase(it);
}

template <typename T1, typename T2>
void f38(std::unordered_map<T1, T2> &o, std::unordered_map<T1, T2> &a0) {
  return o.swap(a0);
}

template <typename T1, typename T2>
std::pair<typename std::unordered_map<T1, T2>::iterator, bool>
f39(std::unordered_map<T1, T2> &o, T1 &&key, T2 &&value) {
  return o.emplace(std::move(key), std::move(value));
}

template <typename T1, typename T2>
std::pair<typename std::unordered_map<T1, T2>::iterator, bool>
f40(std::unordered_map<T1, T2> &o, const T1 &key, const T2 &value) {
  return o.emplace(key, value);
}

template <typename T1, typename T2>
std::pair<typename std::unordered_map<T1, T2>::iterator, bool>
f41(std::unordered_map<T1, T2> &o, std::pair<T1, T2> &&v) {
  return o.insert(std::move(v));
}

// An lvalue std::pair<K, V> (i.e. without the const on the key) binds to the
// forwarding insert() overload, not to insert(const value_type &).
template <typename T1, typename T2>
std::pair<typename std::unordered_map<T1, T2>::iterator, bool>
f42(std::unordered_map<T1, T2> &o, std::pair<T1, T2> &v) {
  return o.insert(v);
}

template <typename T1, typename T2>
std::pair<typename std::unordered_map<T1, T2>::iterator, bool>
f43(std::unordered_map<T1, T2> &o, const std::pair<T1, T2> &v) {
  return o.insert(v);
}

// A non-const lvalue of value_type also prefers the forwarding overload over
// insert(const value_type &): the reference binding is less cv-qualified.
template <typename T1, typename T2>
std::pair<typename std::unordered_map<T1, T2>::iterator, bool>
f44(std::unordered_map<T1, T2> &o, std::pair<const T1, T2> &v) {
  return o.insert(v);
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator
f45(const std::unordered_map<T1, T2> &o) {
  return o.cbegin();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator
f46(const std::unordered_map<T1, T2> &o) {
  return o.cend();
}

template <typename T1, typename T2>
bool f47(const std::unordered_map<T1, T2> &a, const std::unordered_map<T1, T2> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
bool f48(const std::unordered_map<T1, T2> &a, const std::unordered_map<T1, T2> &b) {
  return operator!=(a, b);
}
