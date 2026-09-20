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
