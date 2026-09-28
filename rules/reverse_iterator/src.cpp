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

// ---------------------------------------------------------------------------
// SWALLOW FIX -- /home/agent/work/SWALLOW-AUDIT.md rows #4 and #5, the same bug
// and the same fix shape as rules/vector t8.  t1's trailing literal is ` *>`;
// against `std::reverse_iterator<mlir::Operation *const *>` the FIRST same-depth
// ` *>` is the SECOND star, so T1 swallows the decoration:
//   LLVM ERROR: unsupported unmapped type `mlir::Operation *const`,
//     while mapping `std::reverse_iterator<mlir::Operation *const *>`
//   LLVM ERROR: unsupported unmapped type `dsc2::ScheduleNode *const`,
//     while mapping `std::reverse_iterator<std::__wrap_iter<dsc2::ScheduleNode *const *>>`
// SWALLOW-SAFETY: the new keys' literal run is ` *const *>`, and the text
// `*const` CANNOT occur in `std::reverse_iterator<Foo *>` or
// `std::reverse_iterator<std::__wrap_iter<Foo *>>`, so every instantiation that
// resolves via t1/t2 today keeps resolving there.  Where both match (a
// `<Foo *const *>` element) the new src is strictly longer, so search()'s
// tie-break (mapper.cpp:430-437) picks it.  `T1 *const *` is a pointer to const
// pointer, exactly how clang prints it; no default argument is involved, so
// nothing is elided.  Model: identical to t1/t2 -- the element is itself a
// pointer, so the iterator is a pointer to a pointer (rules/vector t8's shape).
template <typename T1> using t3 = std::reverse_iterator<T1 *const *>;
template <typename T1>
using t4 = std::reverse_iterator<std::__wrap_iter<T1 *const *>>;
