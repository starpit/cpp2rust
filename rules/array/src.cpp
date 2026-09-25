// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <array>

template <typename T1, std::size_t T2>
using t1 = std::array<T1, T2>;

template <typename T1, std::size_t T2>
const T1 *f1(const std::array<T1, T2> &o) {
  return o.data();
}

template <typename T1, std::size_t T2>
std::size_t f2(const std::array<T1, T2> &o) {
  return o.size();
}

template <typename T1, std::size_t T2> T1 *f3(std::array<T1, T2> &o) {
  return o.data();
}

template <typename T1, std::size_t T2>
std::array<T1, T2> f4(std::array<T1, T2> &&o) {
  return std::array<T1, T2>(std::move(o));
}

template <typename T1, std::size_t T2>
std::array<T1, T2> &f5(std::array<T1, T2> &dst, std::array<T1, T2> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1, std::size_t T2>
std::array<T1, T2> f6(const std::array<T1, T2> &o) {
  return std::array<T1, T2>(o);
}

template <typename T1, std::size_t T2>
std::array<T1, T2> &f7(std::array<T1, T2> &dst, const std::array<T1, T2> &src) {
  return dst.operator=(src);
}

template <typename T1, std::size_t T2>
bool f8(const std::array<T1, T2> &a, const std::array<T1, T2> &b) {
  return operator==(a, b);
}

template <typename T1, std::size_t T2>
bool f9(const std::array<T1, T2> &a, const std::array<T1, T2> &b) {
  return operator!=(a, b);
}


// ---------------------------------------------------------------------------
// element access / bulk assignment
// ---------------------------------------------------------------------------

template <typename T1, std::size_t T2>
T1 &f10(std::array<T1, T2> &o, std::size_t idx) {
  return o.at(idx);
}

template <typename T1, std::size_t T2>
const T1 &f11(const std::array<T1, T2> &o, std::size_t idx) {
  return o.at(idx);
}

// std::array<T, N>::fill maps to an element-wise store, NOT to Vec::fill with a
// shared handle: in the refcount model that would make every slot alias one
// allocation.  t1 maps std::array<T, N> to Vec<T> with PLAIN elements (no
// Value<> wrapper), so a per-element clone is both correct and cheap.
template <typename T1, std::size_t T2>
void f12(std::array<T1, T2> &o, const T1 &value) {
  return o.fill(value);
}

// ---------------------------------------------------------------------------
// rbegin / rend.
//
// std::array's iterator IS a raw pointer, so these return
// `std::reverse_iterator<T1 *>` -- the spelling rules/vector t10/f129-f132
// owns, NOT vector's `std::reverse_iterator<std::__wrap_iter<T1 *>>`.  That is
// why these two rules belong here and the operations on their RESULT do not:
// once the type matches t10, ++/--/* are already covered.
//
// No type rule is declared for the result.  vector's t10 already maps that
// exact spelling, and two modules mapping one C++ type to the identical Rust
// type is accepted (addRulesFromDirectory treats it as the idempotent case);
// but declaring it twice buys nothing and adds a spelling to keep in sync, so
// the mapping is left in one place.
//
// The representation, and the reason rbegin() is `data() + len - 1` rather
// than something one-past-the-end, is documented above rules/vector f114 --
// the reverse iterator is the pointer to the element it dereferences to.
// rend() is the slot one BEFORE the first element, which for an EMPTY array
// coincides with rbegin(), so a reverse walk terminates immediately, as in
// C++.
// ---------------------------------------------------------------------------

template <typename T1, std::size_t T2>
typename std::array<T1, T2>::reverse_iterator f13(std::array<T1, T2> &o) {
  return o.rbegin();
}

template <typename T1, std::size_t T2>
typename std::array<T1, T2>::reverse_iterator f14(std::array<T1, T2> &o) {
  return o.rend();
}
