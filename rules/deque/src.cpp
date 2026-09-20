// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <deque>
#include <vector>

template <typename T1> using t1 = std::deque<T1>;

// The allocator-explicit spelling. Unlike std::vector, whose default argument
// is suppressed where it is looked up, std::deque reaches the mapper as
// std::deque<T, std::allocator<T>> and otherwise has no rule at all.
template <typename T1, typename T2> using t6 = std::deque<T1, T2>;

template <typename T1> T1 &f1(std::deque<T1> &o) { return o.back(); }

template <typename T1> T1 &f2(std::deque<T1> &o) { return o.front(); }

template <typename T1> bool f3(const std::deque<T1> &o) { return o.empty(); }

template <typename T1> void f4(std::deque<T1> &o, T1 &&value) {
  return o.push_back(std::move(value));
}

template <typename T1> void f5(std::deque<T1> &o) { return o.pop_front(); }

template <typename T1>
void f7(std::deque<std::vector<T1>> &o, const std::vector<T1> &value) {
  return o.push_back(value);
}

template <typename T1> std::deque<T1> f8(const std::deque<T1> &o) {
  return std::deque<T1>(o);
}

template <typename T1> std::deque<T1> f9(std::deque<T1> &&o) {
  return std::deque<T1>(std::move(o));
}

template <typename T1>
std::deque<T1> &f10(std::deque<T1> &dst, const std::deque<T1> &src) {
  return dst.operator=(src);
}

template <typename T1>
std::deque<T1> &f11(std::deque<T1> &dst, std::deque<T1> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1> std::size_t f12(const std::deque<T1> &o) {
  return o.size();
}

template <typename T1> void f13(std::deque<T1> &o, std::size_t n) {
  return o.resize(n);
}

template <typename T1> void f14(std::deque<T1> &o) { return o.clear(); }

template <typename T1> T1 &f15(std::deque<T1> &o, std::size_t idx) {
  return o.at(idx);
}

template <typename T1>
const T1 &f16(const std::deque<T1> &o, std::size_t idx) {
  return o.at(idx);
}

template <typename T1> void f17(std::deque<T1> &o, const T1 &value) {
  return o.push_back(value);
}

template <typename T1> void f18(std::deque<T1> &o, T1 &&value) {
  return o.push_front(std::move(value));
}

template <typename T1> void f19(std::deque<T1> &o, const T1 &value) {
  return o.push_front(value);
}

template <typename T1> void f20(std::deque<T1> &o) { return o.pop_back(); }

// ---------------------------------------------------------------------------
// Reverse iterators (rbegin/rend/crbegin/crend and the ops on the result).
//
// std::deque<T> shares the Vec<T> representation with std::vector<T> (see the
// TODO in tgt_unsafe.rs), so the reverse iterator uses exactly the same model
// as rules/vector: a pointer to the element it DEREFERENCES to, walked
// BACKWARDS. rbegin() addresses the last element, rend() the slot one before
// the first, operator++ decrements, operator-- increments and operator*() is
// the identity. See rules/vector/src.cpp for why that one-before-the-first
// slot is exactly representable in both models.
//
// Unlike the vector rules, the const forms have to be spelled out separately.
// libc++ prints the deque iterator as
//     std::__deque_iterator<T1, T1 *, T1 &, T1 **, long>
// and its const form as
//     std::__deque_iterator<T1, const T1 *, const T1 &, const T1 *const *, long>
// The first template argument pins T1 to the bare element type, so (unlike
// std::__wrap_iter<T1 *>, where T1 can absorb the `const`) the non-const
// pattern cannot unify with a const_reverse_iterator.
//
// t2/t3 exist so that base() has a type to return; the deque forward iterator
// has no begin()/end() rules of its own.
// ---------------------------------------------------------------------------

template <typename T1> using t2 = typename std::deque<T1>::iterator;
template <typename T1> using t3 = typename std::deque<T1>::const_iterator;
template <typename T1> using t4 = typename std::deque<T1>::reverse_iterator;
template <typename T1>
using t5 = typename std::deque<T1>::const_reverse_iterator;

template <typename T1>
typename std::deque<T1>::reverse_iterator f21(std::deque<T1> &o) {
  return o.rbegin();
}

template <typename T1>
typename std::deque<T1>::reverse_iterator f22(std::deque<T1> &o) {
  return o.rend();
}

template <typename T1>
typename std::deque<T1>::const_reverse_iterator f23(const std::deque<T1> &o) {
  return o.rbegin();
}

template <typename T1>
typename std::deque<T1>::const_reverse_iterator f24(const std::deque<T1> &o) {
  return o.rend();
}

template <typename T1>
typename std::deque<T1>::const_reverse_iterator f25(const std::deque<T1> &o) {
  return o.crbegin();
}

template <typename T1>
typename std::deque<T1>::const_reverse_iterator f26(const std::deque<T1> &o) {
  return o.crend();
}

template <typename T1>
T1 &f27(typename std::deque<T1>::reverse_iterator it) {
  return it.operator*();
}

template <typename T1>
typename std::deque<T1>::reverse_iterator &
f28(typename std::deque<T1>::reverse_iterator &it) {
  return it.operator++();
}

template <typename T1>
typename std::deque<T1>::reverse_iterator
f29(typename std::deque<T1>::reverse_iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1>
typename std::deque<T1>::reverse_iterator &
f30(typename std::deque<T1>::reverse_iterator &it) {
  return it.operator--();
}

template <typename T1>
bool f31(const typename std::deque<T1>::reverse_iterator &it1,
         const typename std::deque<T1>::reverse_iterator &it2) {
  return operator==(it1, it2);
}

template <typename T1>
bool f32(const typename std::deque<T1>::reverse_iterator &it1,
         const typename std::deque<T1>::reverse_iterator &it2) {
  return operator!=(it1, it2);
}

template <typename T1>
typename std::deque<T1>::iterator
f33(const typename std::deque<T1>::reverse_iterator &it) {
  return it.base();
}

template <typename T1>
const T1 &f34(typename std::deque<T1>::const_reverse_iterator it) {
  return it.operator*();
}

template <typename T1>
typename std::deque<T1>::const_reverse_iterator &
f35(typename std::deque<T1>::const_reverse_iterator &it) {
  return it.operator++();
}

template <typename T1>
typename std::deque<T1>::const_reverse_iterator
f36(typename std::deque<T1>::const_reverse_iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1>
typename std::deque<T1>::const_reverse_iterator &
f37(typename std::deque<T1>::const_reverse_iterator &it) {
  return it.operator--();
}

template <typename T1>
bool f38(const typename std::deque<T1>::const_reverse_iterator &it1,
         const typename std::deque<T1>::const_reverse_iterator &it2) {
  return operator==(it1, it2);
}

template <typename T1>
bool f39(const typename std::deque<T1>::const_reverse_iterator &it1,
         const typename std::deque<T1>::const_reverse_iterator &it2) {
  return operator!=(it1, it2);
}

template <typename T1>
typename std::deque<T1>::const_iterator
f40(const typename std::deque<T1>::const_reverse_iterator &it) {
  return it.base();
}

template <typename T1>
bool f41(const std::deque<T1> &a, const std::deque<T1> &b) {
  return operator==(a, b);
}

template <typename T1>
bool f42(const std::deque<T1> &a, const std::deque<T1> &b) {
  return operator!=(a, b);
}

template <typename T1> std::deque<T1> f43(std::size_t n, const T1 &value) {
  return std::deque<T1>(n, value);
}

template <typename T1> std::deque<T1> f44(std::size_t n) {
  return std::deque<T1>(n);
}

// The default constructor. std::vector and std::set have one (vector/f4,
// set/f13); std::deque did not, so every `std::deque<T> d;` in a translated TU
// fell back to the mangled `std_deque_..::std_deque_..N()` placeholder.
template <typename T1> std::deque<T1> f45() { return std::deque<T1>(); }
