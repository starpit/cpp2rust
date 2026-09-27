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
