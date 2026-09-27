// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <iterator>

template <typename T1> using t1 = std::reverse_iterator<T1 *>;

template <typename T1> std::reverse_iterator<T1 *> f1(T1 *p) {
  return std::reverse_iterator<T1 *>(p);
}

template <typename T1>
bool f2(const std::reverse_iterator<T1 *> &a,
        const std::reverse_iterator<T1 *> &b) {
  return operator==(a, b);
}

template <typename T1>
bool f3(const std::reverse_iterator<T1 *> &a,
        const std::reverse_iterator<T1 *> &b) {
  return operator!=(a, b);
}

template <typename T1>
std::reverse_iterator<T1 *> &f4(std::reverse_iterator<T1 *> &it) {
  return it.operator++();
}

template <typename T1>
std::reverse_iterator<T1 *> f5(std::reverse_iterator<T1 *> &it, int a1) {
  return it.operator++(a1);
}

template <typename T1> T1 &f6(const std::reverse_iterator<T1 *> &it) {
  return it.operator*();
}

template <typename T1> T1 *f7(const std::reverse_iterator<T1 *> &it) {
  return it.base();
}

// libc++ spells vector<X>::iterator as the internal std::__wrap_iter<X *>, and
// the rows g1120/g1138/g2001-g2007 are reverse_iterator OVER that wrapper.  The
// wrapper is a raw pointer, so the model is IDENTICAL to t1's -- these are the
// same bodies against the second spelling of the key.
template <typename T1> using t2 = std::reverse_iterator<std::__wrap_iter<T1 *>>;

template <typename T1>
std::reverse_iterator<std::__wrap_iter<T1 *>> f8(std::__wrap_iter<T1 *> p) {
  return std::reverse_iterator<std::__wrap_iter<T1 *>>(p);
}

template <typename T1>
bool f9(const std::reverse_iterator<std::__wrap_iter<T1 *>> &a,
        const std::reverse_iterator<std::__wrap_iter<T1 *>> &b) {
  return operator==(a, b);
}

template <typename T1>
bool f10(const std::reverse_iterator<std::__wrap_iter<T1 *>> &a,
         const std::reverse_iterator<std::__wrap_iter<T1 *>> &b) {
  return operator!=(a, b);
}

template <typename T1>
std::reverse_iterator<std::__wrap_iter<T1 *>> &
f11(std::reverse_iterator<std::__wrap_iter<T1 *>> &it) {
  return it.operator++();
}

template <typename T1>
std::reverse_iterator<std::__wrap_iter<T1 *>>
f12(std::reverse_iterator<std::__wrap_iter<T1 *>> &it, int a1) {
  return it.operator++(a1);
}

template <typename T1>
T1 &f13(const std::reverse_iterator<std::__wrap_iter<T1 *>> &it) {
  return it.operator*();
}

template <typename T1>
std::__wrap_iter<T1 *>
f14(const std::reverse_iterator<std::__wrap_iter<T1 *>> &it) {
  return it.base();
}
