// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstddef>
#include <set>
#include <utility>

template <typename T1> using t1 = std::set<T1>;

template <typename T1> std::set<T1> f1() { return std::set<T1>(); }

template <typename T1> std::size_t f2(const std::set<T1> &o) {
  return o.size();
}

template <typename T1> bool f3(const std::set<T1> &o) { return o.empty(); }

template <typename T1> void f4(std::set<T1> &o) { return o.clear(); }

template <typename T1> std::size_t f5(const std::set<T1> &o, const T1 &k) {
  return o.count(k);
}

template <typename T1> std::size_t f6(std::set<T1> &o, const T1 &k) {
  return o.erase(k);
}

template <typename T1>
bool f7(const std::set<T1> &a, const std::set<T1> &b) {
  return operator==(a, b);
}

template <typename T1>
bool f8(const std::set<T1> &a, const std::set<T1> &b) {
  return operator!=(a, b);
}

template <typename T1> std::set<T1> f9(const std::set<T1> &o) {
  return std::set<T1>(o);
}

template <typename T1> std::set<T1> f10(std::set<T1> &&o) {
  return std::set<T1>(std::move(o));
}

template <typename T1>
std::set<T1> &f11(std::set<T1> &dst, const std::set<T1> &src) {
  return dst.operator=(src);
}

template <typename T1>
std::set<T1> &f12(std::set<T1> &dst, std::set<T1> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1> using t2 = typename std::set<T1>::const_iterator;

template <typename T1>
std::pair<typename std::set<T1>::iterator, bool> f13(std::set<T1> &o,
                                                     const T1 &k) {
  return o.insert(k);
}

template <typename T1>
typename std::set<T1>::const_iterator f14(const std::set<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::set<T1>::const_iterator f15(const std::set<T1> &o) {
  return o.end();
}

template <typename T1>
typename std::set<T1>::iterator f16(std::set<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::set<T1>::iterator f17(std::set<T1> &o) {
  return o.end();
}

template <typename T1>
bool f18(typename std::set<T1>::const_iterator a,
         typename std::set<T1>::const_iterator b) {
  return operator==(a, b);
}

template <typename T1>
bool f19(typename std::set<T1>::const_iterator a,
         typename std::set<T1>::const_iterator b) {
  return operator!=(a, b);
}

template <typename T1>
typename std::set<T1>::const_iterator &
f20(typename std::set<T1>::const_iterator &it) {
  return it.operator++();
}

template <typename T1>
typename std::set<T1>::const_iterator
f21(typename std::set<T1>::const_iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1>
const T1 &f22(typename std::set<T1>::const_iterator it) {
  return it.operator*();
}

template <typename T1>
std::pair<typename std::set<T1>::iterator, bool> f23(std::set<T1> &o, T1 &&k) {
  return o.insert(std::move(k));
}
