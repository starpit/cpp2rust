// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstddef>
#include <set>
#include <utility>

// std::set is modelled as a map to unit: it reuses BTreeMap and the existing
// RefcountMapIter, which is also what makes the converter treat a set iterator
// as bidirectional without any special casing.
template <typename T1> using t1 = std::set<T1>;

// In libc++ std::set<T>::iterator and ::const_iterator are the same type, so
// one rule covers both.
template <typename T1> using t2 = typename std::set<T1>::iterator;

template <typename T1> std::size_t f1(const std::set<T1> &o) {
  return o.size();
}

template <typename T1> bool f2(const std::set<T1> &o) { return o.empty(); }

template <typename T1> void f3(std::set<T1> &o) { return o.clear(); }

template <typename T1> std::size_t f4(const std::set<T1> &o, const T1 &k) {
  return o.count(k);
}

template <typename T1>
typename std::set<T1>::iterator f5(std::set<T1> &o, const T1 &k) {
  return o.find(k);
}

template <typename T1>
typename std::set<T1>::const_iterator f6(const std::set<T1> &o, const T1 &k) {
  return o.find(k);
}

template <typename T1> typename std::set<T1>::iterator f7(std::set<T1> &o) {
  return o.begin();
}

template <typename T1> typename std::set<T1>::iterator f8(std::set<T1> &o) {
  return o.end();
}

template <typename T1>
typename std::set<T1>::const_iterator f9(const std::set<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::set<T1>::const_iterator f10(const std::set<T1> &o) {
  return o.end();
}

template <typename T1>
std::pair<typename std::set<T1>::iterator, bool> f11(std::set<T1> &o,
                                                     const T1 &v) {
  return o.insert(v);
}

template <typename T1> std::size_t f12(std::set<T1> &o, const T1 &k) {
  return o.erase(k);
}

template <typename T1> std::set<T1> f13() { return std::set<T1>(); }

template <typename T1> std::set<T1> f14(const std::set<T1> &o) {
  return std::set<T1>(o);
}

template <typename T1>
std::set<T1> &f15(std::set<T1> &dst, const std::set<T1> &src) {
  return dst.operator=(src);
}

template <typename T1>
bool f16(typename std::set<T1>::iterator a, typename std::set<T1>::iterator b) {
  return operator!=(a, b);
}

template <typename T1>
bool f17(typename std::set<T1>::iterator a, typename std::set<T1>::iterator b) {
  return operator==(a, b);
}

template <typename T1> const T1 &f18(typename std::set<T1>::iterator it) {
  return it.operator*();
}

template <typename T1>
std::pair<typename std::set<T1>::iterator, bool> f19(std::set<T1> &o, T1 &&v) {
  return o.insert(std::move(v));
}

template <typename T1>
typename std::set<T1>::iterator &f20(typename std::set<T1>::iterator &it) {
  return it.operator++();
}

template <typename T1>
typename std::set<T1>::iterator f21(typename std::set<T1>::iterator a0,
                                    int a1) {
  return a0.operator++(a1);
}

template <typename T1>
bool f22(const std::set<T1> &a, const std::set<T1> &b) {
  return operator==(a, b);
}

template <typename T1>
bool f23(const std::set<T1> &a, const std::set<T1> &b) {
  return operator!=(a, b);
}

// The initializer-list constructor: `std::set<T> s = {a, b, c};`.  libc++
// declares it as
//     set(initializer_list<value_type>, const value_compare& = value_compare())
// so the resolved rule carries the comparator parameter too, and the converter
// passes `None` for it.
template <typename T1> std::set<T1> f24(std::initializer_list<T1> a0) {
  return std::set<T1>(a0);
}
