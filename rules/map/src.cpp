// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <map>
#include <utility>

template <typename T1, typename T2> using t1 = std::map<T1, T2>;

template <typename T1, typename T2>
using t2 = typename std::map<T1, T2>::const_iterator;

template <typename T1, typename T2>
using t3 = typename std::map<T1, T2>::iterator;

template <typename T1, typename T2> T2 &f1(std::map<T1, T2> &o, const T1 &key) {
  return o.operator[](key);
}

template <typename T1, typename T2> std::size_t f2(const std::map<T1, T2> &o) {
  return o.size();
}

template <typename T1, typename T2>
typename std::map<T1, T2>::iterator f3(std::map<T1, T2> &o,
                                       typename std::map<T1, T2>::iterator it) {
  return o.erase(it);
}

template <typename T1, typename T2> std::map<T1, T2> f5() {
  return std::map<T1, T2>();
}

template <typename T1, typename T2>
std::map<T1, T2> f6(const std::map<T1, T2> &&o) {
  return std::map<T1, T2>(std::move(o));
}

template <typename T1, typename T2> T2 &f7(std::map<T1, T2> &o, const T1 &key) {
  return o.at(key);
}

template <typename T1, typename T2> T2 &f8(std::map<T1, T2> &o, T1 &&key) {
  return o.operator[](std::move(key));
}

template <typename T1, typename T2>
typename std::map<T1, T2>::const_iterator f9(const std::map<T1, T2> &o) {
  return o.end();
}

template <typename T1, typename T2>
typename std::map<T1, T2>::iterator f10(std::map<T1, T2> &o, const T1 &key) {
  return o.find(key);
}

template <typename T1, typename T2>
bool f11(typename std::map<T1, T2>::iterator a,
         typename std::map<T1, T2>::iterator b) {
  return operator!=(a, b);
}

template <typename T1, typename T2>
typename std::map<T1, T2>::iterator f12(std::map<T1, T2> &o) {
  return o.begin();
}

template <typename T1, typename T2>
bool f13(typename std::map<T1, T2>::const_iterator a,
         typename std::map<T1, T2>::const_iterator b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
typename std::map<T1, T2>::iterator f14(std::map<T1, T2> &o) {
  return o.end();
}

template <typename T1, typename T2>
const T2 &f15(const std::map<T1, T2> &o, const T1 &key) {
  return o.at(key);
}

template <typename T1, typename T2>
bool f16(typename std::map<T1, T2>::iterator a,
         typename std::map<T1, T2>::iterator b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
typename std::map<T1, T2>::const_iterator f17(const std::map<T1, T2> &o,
                                              const T1 &key) {
  return o.find(key);
}

template <typename T1, typename T2>
typename std::map<T1, T2>::const_iterator
f19(const typename std::map<T1, T2>::iterator &it) {
  return typename std::map<T1, T2>::const_iterator(it);
}

template <typename T1, typename T2>
const T1 &f20(typename std::map<T1, T2>::const_iterator it) {
  return it->first;
}

template <typename T1, typename T2>
const T2 &f21(typename std::map<T1, T2>::const_iterator it) {
  return it->second;
}

template <typename T1, typename T2>
const T1 &f22(typename std::map<T1, T2>::iterator it) {
  return it->first;
}

template <typename T1, typename T2>
T2 &f23(typename std::map<T1, T2>::iterator it) {
  return it->second;
}

template <typename T1, typename T2> std::map<T1, T2> f24(std::map<T1, T2> &&o) {
  return std::map<T1, T2>(std::move(o));
}

template <typename T1, typename T2>
std::map<T1, T2> &f25(std::map<T1, T2> &dst, std::map<T1, T2> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1, typename T2>
std::map<T1, T2> &f26(std::map<T1, T2> &dst, const std::map<T1, T2> &src) {
  return dst.operator=(src);
}

template <typename T1, typename T2>
bool f27(typename std::map<T1, T2>::const_iterator a,
         typename std::map<T1, T2>::const_iterator b) {
  return operator!=(a, b);
}

template <typename T1, typename T2>
typename std::map<T1, T2>::iterator &
f28(typename std::map<T1, T2>::iterator &it) {
  return it.operator++();
}

template <typename T1, typename T2>
typename std::map<T1, T2>::const_iterator &
f29(typename std::map<T1, T2>::const_iterator &it) {
  return it.operator++();
}

template <typename T1, typename T2>
bool f30(const std::map<T1, T2> &a, const std::map<T1, T2> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
bool f31(const std::map<T1, T2> &a, const std::map<T1, T2> &b) {
  return operator!=(a, b);
}

// The initializer-list constructor: `std::map<K, V> m = {{k, v}, ...};`.
// libc++ declares it as
//     map(initializer_list<value_type>, const key_compare& = key_compare())
// so the resolved rule carries the comparator parameter too, and the converter
// passes `None` for it.  std::pair<const T1, T2> appears only INSIDE this
// signature -- no std::pair<const T1, T2> TYPE rule is introduced, so the
// `const T *` ambiguity that killed the earlier attempt at that family does
// not arise here: T1 and T2 are already pinned by std::map<T1, T2>.
template <typename T1, typename T2>
std::map<T1, T2> f32(std::initializer_list<std::pair<const T1, T2>> a0) {
  return std::map<T1, T2>(a0);
}

// ---------------------------------------------------------------------------
// The POSTFIX ++ on a map iterator: `it++`, as opposed to f28/f29's `++it`.
//
// libc++ prints the two distinctly -- the prefix returns `__map_iterator &`
// and the postfix returns `__map_iterator` and takes an int -- so f28/f29 do
// not cover the postfix form, and `it++` in dsc/dims.cpp:736 died at
// converter.cpp:3455 with "unsupported CXXOperatorCallExpr: ++".
//
// Postfix semantics are the ones the C++ standard specifies and the call site
// relies on: ADVANCE the receiver, RETURN the value it had before. Getting
// that backwards would be silent -- the loop would still terminate, on the
// wrong element. libcc2rs already implements exactly this as
// `PostfixInc for MapIter` (iterators.rs), which both models reuse; the
// prefix rules call `inc()` then `clone()`, so the two are not interchangeable.
// ---------------------------------------------------------------------------

template <typename T1, typename T2>
typename std::map<T1, T2>::iterator
f33(typename std::map<T1, T2>::iterator &it, int a1) {
  return it.operator++(a1);
}

template <typename T1, typename T2>
typename std::map<T1, T2>::const_iterator
f34(typename std::map<T1, T2>::const_iterator &it, int a1) {
  return it.operator++(a1);
}

// ---------------------------------------------------------------------------
// The four RELATIONAL operators on std::map.
//
// f30/f31 already cover == and !=; `<`, `<=`, `>` and `>=` had no rule, and
// `new_sp.bigDimToSize_ >= new_sp.dimToSize_` in
// dcg/dcg_fe/pcfg_gen/stcdpOp.cpp:90 (both std::map<std::string, double>,
// inside a DT_CHECK) died at converter.cpp:3455.
//
// This is only sound because Rust's derived Ord on BTreeMap agrees with C++'s
// std::lexicographical_compare over the (key, value) SEQUENCE, which is what
// std::map's relational operators are specified to do. That was not assumed --
// it was measured: six discriminating pairs (same keys/different values, one
// map a strict PREFIX of another, a differing KEY, and equal maps) give
// identical answers from clang-compiled std::map and from BTreeMap:
// `10 01 01 11 01 01` for both.
//
// NOTE the receiver, and why `>=` here is not the `>=` a scalar would get: the
// elements are compared through Box<T2>/Value<T2>, whose Ord forwards to T2,
// so the comparison reaches the mapped values rather than the handles.
// ---------------------------------------------------------------------------

template <typename T1, typename T2>
bool f35(const std::map<T1, T2> &a, const std::map<T1, T2> &b) {
  return operator<(a, b);
}

template <typename T1, typename T2>
bool f36(const std::map<T1, T2> &a, const std::map<T1, T2> &b) {
  return operator<=(a, b);
}

template <typename T1, typename T2>
bool f37(const std::map<T1, T2> &a, const std::map<T1, T2> &b) {
  return operator>(a, b);
}

template <typename T1, typename T2>
bool f38(const std::map<T1, T2> &a, const std::map<T1, T2> &b) {
  return operator>=(a, b);
}
