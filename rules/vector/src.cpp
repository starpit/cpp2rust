// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <algorithm>
#include <initializer_list>
#include <vector>

template <typename T, typename A> using Init = A;

template <typename T1> using t1 = std::vector<T1>;
template <typename T1> using t2 = typename std::vector<T1>::iterator;
template <typename T1> using t3 = std::vector<std::vector<T1>>;
template <typename T1> using t4 = typename std::vector<T1>::const_iterator;

// t8 -- `std::vector<T1 *>::const_iterator`, which libc++ spells
// `std::__wrap_iter<T1 *const *>` (a POINTER TO CONST POINTER: the iterator may not
// write the stored pointer).  THIS KEY EXISTS TO DEFEAT A SWALLOW, NOT TO ADD A MODEL.
//
// MEASURED, first abort on 4 bucket-B TUs, identical on the pinned binary (33e811d1)
// AND on the HEAD-level `cc2.tk.AFTER` (0547744e), so it is NOT the already-landed
// kConstPrefix/`*const ` derive:
//   LLVM ERROR: unsupported unmapped type `DtInfo *const` has no model in types_,
//   while mapping `std::__wrap_iter<DtInfo *const *>`
// `DtInfo *const` is not a spelling anything wrote -- it is what t2's key
// (`std::__wrap_iter<T1 *>`) CAPTURED.  matchTemplate's placeholder capture runs to
// the next SAME-DEPTH literal (mapper.cpp findNextLiteralSameDepth), and against
// `std::__wrap_iter<DtInfo *const *>` that literal is t2's trailing ` *>`, whose first
// same-depth occurrence is after `const`.  So T1 swallowed `DtInfo *const`, and the
// mapper then tried to map THAT as a type and hit the trailing-const bail.  Same defect
// family as `llvm::DenseMapInfo<T1>` swallowing `llvm::StringRef, void`.
//
// The cure is the same as there: a LONGER, MORE SPECIFIC src, which search()'s
// longer-src tie-break prefers.  SWALLOW-SAFETY, both directions:
//   * t8 cannot match a NON-const `__wrap_iter<X *>`: matchTemplate must see the
//     literal ` *const *>` after the placeholder, and `std::__wrap_iter<int *>` has no
//     `const` at all.  So the 3 open `operator>=`-on-`__wrap_iter<int *>` rows are
//     untouched, and t2/t6 keep every non-const iterator they own today.
//   * t8 cannot match `__wrap_iter<const X *>` (t4/t7): that spelling is LEADING-const,
//     `const X *`, and contains no ` *const *` either.
//   * Conversely t4/t7 cannot match `__wrap_iter<X *const *>`, because their key's
//     literal prefix is `std::__wrap_iter<const `.  So t8 is the only key that can win
//     this spelling on specificity, and it wins it against t2/t6 only.
template <typename T1> using t8 = typename std::vector<T1 *>::const_iterator;

template <typename T1, typename T2 = std::allocator<T1>>
using t5 = std::vector<T1, T2>;

#if defined(__linux__)
template <typename T1, typename T2 = std::allocator<T1>>
using t6 = typename std::vector<T1, T2>::iterator;
template <typename T1, typename T2 = std::allocator<T1>>
using t7 = typename std::vector<T1, T2>::const_iterator;
#endif

template <typename T1>
typename std::vector<T1>::iterator
f1(std::vector<T1> &o, typename std::vector<T1>::const_iterator it) {
  return o.erase(it);
}

template <typename T1> std::size_t f2(const std::vector<T1> &o) {
  return o.size();
}
template <typename T1> bool f3(const std::vector<T1> &o) { return o.empty(); }

template <typename T1> std::vector<T1> f4() { return std::vector<T1>(); }

template <typename T1> void f5(std::vector<T1> &o) { return o.pop_back(); }

template <typename T1> T1 *f6(std::vector<T1> &o) { return o.data(); }

template <typename T1> T1 &f7(std::vector<T1> &o, std::size_t idx) {
  return o.at(idx);
}

template <typename T1> std::vector<T1> f8(std::size_t n) {
  return std::vector<T1>(n);
}

template <typename T1> T1 &f9(std::vector<T1> &o) { return o.front(); }

template <typename T1> T1 &f10(std::vector<T1> &o) { return o.back(); }

template <typename T1> std::size_t f11(const std::vector<T1> &o) {
  return o.capacity();
}

template <typename T1> void f12(std::vector<T1> &o, std::size_t n) {
  return o.reserve(n);
}

template <typename T1>
typename std::vector<T1>::iterator f13(std::vector<T1> &o) {
  return o.begin();
}

template <typename T1> void f14(std::vector<T1> &o, T1 &&value) {
  return o.push_back(std::move(value));
}

template <typename T1> void f15(std::vector<T1> &o, std::size_t n) {
  return o.resize(n);
}

template <typename T1> void f16(std::vector<T1> &o) { return o.clear(); }

template <typename T1>
typename std::vector<T1>::iterator f17(std::vector<T1> &o) {
  return o.end();
}

template <typename T1>
typename std::vector<T1>::iterator
f18(std::vector<T1> &o, typename std::vector<T1>::const_iterator it,
    T1 &&value) {
  return o.insert(it, std::move(value));
}

template <typename T1> std::vector<T1> f19(std::size_t n, const T1 &value) {
  return std::vector<T1>(n, value);
}

template <typename T1>
typename std::vector<T1>::iterator
f20(std::vector<T1> &o, typename std::vector<T1>::const_iterator it,
    const T1 &value) {
  return o.insert(it, value);
}

template <typename T1> void f21(std::vector<T1> &o, const T1 &value) {
  return o.push_back(value);
}

template <typename T1>
typename std::vector<T1>::reference f22(typename std::vector<T1>::iterator it) {
  return it.operator*();
}

template <typename T1>
typename std::vector<T1>::iterator
f23(const typename std::vector<T1>::iterator &it) {
  return typename std::vector<T1>::iterator(it);
}

template <typename T1>
typename std::vector<T1>::const_iterator
f24(const typename std::vector<T1>::iterator &it) {
  return typename std::vector<T1>::const_iterator(it);
}

template <typename T1>
typename std::vector<T1>::iterator f25(typename std::vector<T1>::iterator it,
                                       std::size_t n) {
  return it.operator+(n);
}

template <typename T1>
bool f26(const typename std::vector<T1>::iterator &it1,
         const typename std::vector<T1>::iterator &it2) {
  return operator!=(it1, it2);
}

template <typename T1>
bool f27(const typename std::vector<T1>::iterator &it1,
         const typename std::vector<T1>::iterator &it2) {
  return operator==(it1, it2);
}

template <typename T1>
typename std::vector<T1>::iterator f28(typename std::vector<T1>::iterator a0,
                                       int a1) {
  return a0.operator++(a1);
}

template <typename T1>
std::vector<std::vector<T1>> f29(const std::vector<std::vector<T1>> &&o) {
  return std::vector<std::vector<T1>>(std::move(o));
}

template <typename T1> std::vector<std::vector<T1>> f30(std::size_t n) {
  return std::vector<std::vector<T1>>(n);
}

template <typename T1>
void f31(std::vector<std::vector<T1>> &o, std::vector<T1> &&value) {
  return o.push_back(std::move(value));
}

template <typename T1>
void f32(std::vector<std::vector<T1>> &o, std::size_t n) {
  return o.resize(n);
}

template <typename T1>
typename std::vector<T1>::iterator::difference_type
f33(const typename std::vector<T1>::iterator &it1,
    const typename std::vector<T1>::iterator &it2) {
  return operator-(it1, it2);
}

template <typename T1>
typename std::vector<T1>::iterator &
f34(typename std::vector<T1>::iterator &it) {
  return it.operator++();
}

template <typename T1> std::vector<T1> f35(const T1 *first, const T1 *last) {
  return std::vector<T1>(first, last);
}

template <typename T1>
std::vector<T1> f36(const std::initializer_list<T1> &a0) {
  return std::vector<T1>(a0);
}

template <typename T1, typename T2> std::vector<T1> f37(T2 *first, T2 *last) {
  return std::vector<T1>(first, last);
}

std::vector<bool> f38(std::size_t n, const bool &value) {
  return std::vector<bool>(n, value);
}

template <class T1, std::size_t T2> const T1 *f40(T1 const (&a0)[T2]) {
  return std::end(a0);
}

template <typename T1> const T1 *f41(const std::vector<T1> &o) {
  return o.data();
}

template <typename T1>
typename std::vector<T1>::const_iterator
f42(typename std::vector<T1>::const_iterator first,
    typename std::vector<T1>::const_iterator last) {
  return std::max_element(first, last);
}

template <typename T1>
typename std::vector<T1>::const_iterator f43(const std::vector<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::vector<T1>::const_iterator f44(const std::vector<T1> &o) {
  return o.end();
}

bool f47(std::vector<bool> &o) { return o[0]; }

template <typename T1> void f48(std::vector<T1> &o, std::vector<T1> &a0) {
  return o.swap(a0);
}

template <typename T1>
const T1 &f50(const std::vector<T1> &o, std::size_t idx) {
  return o.at(idx);
}

template <typename T1> const T1 &f51(const std::vector<T1> &o) {
  return o.back();
}

template <typename T1>
void f52(std::vector<std::vector<T1>> &o, const std::vector<T1> &value) {
  return o.push_back(value);
}

template <typename T1>
typename std::vector<T1>::iterator
f53(std::vector<T1> &o, typename std::vector<T1>::const_iterator pos,
    const T1 *first, const T1 *last) {
  return o.insert(pos, first, last);
}

template <typename T1>
void f54(std::vector<T1> &o, std::size_t n,
         const typename std::vector<T1>::value_type &value) {
  return o.resize(n, value);
}

template <typename T1>
std::vector<T1> &f55(std::vector<T1> &dst, std::vector<T1> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1> std::vector<T1> &f56(std::vector<std::vector<T1>> &o) {
  return o.back();
}

template <typename T1>
typename std::vector<T1>::const_iterator f57(const std::vector<T1> &o) {
  return o.cend();
}

template <typename T1>
std::vector<T1> &f58(std::vector<T1> &dst, const std::vector<T1> &src) {
  return dst.operator=(src);
}

template <typename T1> void f59(std::vector<T1> &o) {
  return o.shrink_to_fit();
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::iterator
f60(std::vector<T1, T2> &o, typename std::vector<T1, T2>::const_iterator it) {
  return o.erase(it);
}

template <typename T1, typename T2 = std::allocator<T1>>
std::size_t f61(const std::vector<T1, T2> &o) {
  return o.size();
}

template <typename T1, typename T2 = std::allocator<T1>>
bool f62(const std::vector<T1, T2> &o) {
  return o.empty();
}

template <typename T1, typename T2 = std::allocator<T1>>
std::vector<T1, T2> f63() {
  return std::vector<T1, T2>();
}

template <typename T1, typename T2 = std::allocator<T1>>
void f64(std::vector<T1, T2> &o) {
  return o.pop_back();
}

template <typename T1, typename T2 = std::allocator<T1>>
T1 *f65(std::vector<T1, T2> &o) {
  return o.data();
}

template <typename T1, typename T2 = std::allocator<T1>>
T1 &f66(std::vector<T1, T2> &o, std::size_t idx) {
  return o.at(idx);
}

template <typename T1, typename T2 = std::allocator<T1>>
std::vector<T1, T2> f67(std::size_t n) {
  return std::vector<T1, T2>(n);
}

template <typename T1, typename T2 = std::allocator<T1>>
T1 &f68(std::vector<T1, T2> &o) {
  return o.front();
}

template <typename T1, typename T2 = std::allocator<T1>>
T1 &f69(std::vector<T1, T2> &o) {
  return o.back();
}

template <typename T1, typename T2 = std::allocator<T1>>
std::size_t f70(const std::vector<T1, T2> &o) {
  return o.capacity();
}

template <typename T1, typename T2 = std::allocator<T1>>
void f71(std::vector<T1, T2> &o, std::size_t n) {
  return o.reserve(n);
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::iterator f72(std::vector<T1, T2> &o) {
  return o.begin();
}

template <typename T1, typename T2 = std::allocator<T1>>
void f73(std::vector<T1, T2> &o, T1 &&value) {
  return o.push_back(std::move(value));
}

template <typename T1, typename T2 = std::allocator<T1>>
void f74(std::vector<T1, T2> &o, std::size_t n) {
  return o.resize(n);
}

template <typename T1, typename T2 = std::allocator<T1>>
void f75(std::vector<T1, T2> &o) {
  return o.clear();
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::iterator f76(std::vector<T1, T2> &o) {
  return o.end();
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::iterator
f77(std::vector<T1, T2> &o, typename std::vector<T1, T2>::const_iterator it,
    T1 &&value) {
  return o.insert(it, std::move(value));
}

template <typename T1, typename T2 = std::allocator<T1>>
std::vector<T1, T2> f78(std::size_t n, const T1 &value) {
  return std::vector<T1, T2>(n, value);
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::iterator
f79(std::vector<T1, T2> &o, typename std::vector<T1, T2>::const_iterator it,
    const T1 &value) {
  return o.insert(it, value);
}

template <typename T1, typename T2 = std::allocator<T1>>
void f80(std::vector<T1, T2> &o, const T1 &value) {
  return o.push_back(value);
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::reference
f81(typename std::vector<T1, T2>::iterator it) {
  return it.operator*();
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::iterator
f82(const typename std::vector<T1, T2>::iterator &it) {
  return typename std::vector<T1, T2>::iterator(it);
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::const_iterator
f83(const typename std::vector<T1, T2>::iterator &it) {
  return typename std::vector<T1, T2>::const_iterator(it);
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::iterator
f84(typename std::vector<T1, T2>::iterator it, std::size_t n) {
  return it.operator+(n);
}

template <typename T1, typename T2 = std::allocator<T1>>
bool f85(const typename std::vector<T1, T2>::iterator &it1,
         const typename std::vector<T1, T2>::iterator &it2) {
  return operator!=(it1, it2);
}

template <typename T1, typename T2 = std::allocator<T1>>
bool f86(const typename std::vector<T1, T2>::iterator &it1,
         const typename std::vector<T1, T2>::iterator &it2) {
  return operator==(it1, it2);
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::iterator
f87(typename std::vector<T1, T2>::iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::iterator::difference_type
f88(const typename std::vector<T1, T2>::iterator &it1,
    const typename std::vector<T1, T2>::iterator &it2) {
  return operator-(it1, it2);
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::iterator &
f89(typename std::vector<T1, T2>::iterator &it) {
  return it.operator++();
}

template <typename T1, typename T2 = std::allocator<T1>>
std::vector<T1, T2> f90(const T1 *first, const T1 *last) {
  return std::vector<T1, T2>(first, last);
}

template <typename T1, typename T2 = std::allocator<T1>>
std::vector<T1, T2> f91(const std::initializer_list<T1> &a0) {
  return std::vector<T1, T2>(a0);
}

template <typename T1, typename T2 = std::allocator<T1>, typename T3>
std::vector<T1, T2> f92(T3 *first, T3 *last) {
  return std::vector<T1, T2>(first, last);
}

template <typename T1, typename T2 = std::allocator<T1>>
const T1 *f93(const std::vector<T1, T2> &o) {
  return o.data();
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::const_iterator
f94(typename std::vector<T1, T2>::const_iterator first,
    typename std::vector<T1, T2>::const_iterator last) {
  return std::max_element(first, last);
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::const_iterator f95(const std::vector<T1, T2> &o) {
  return o.begin();
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::const_iterator f96(const std::vector<T1, T2> &o) {
  return o.end();
}

template <typename T1, typename T2 = std::allocator<T1>>
void f97(std::vector<T1, T2> &o, std::vector<T1, T2> &a0) {
  return o.swap(a0);
}

template <typename T1, typename T2 = std::allocator<T1>>
const T1 &f98(const std::vector<T1, T2> &o, std::size_t idx) {
  return o.at(idx);
}

template <typename T1, typename T2 = std::allocator<T1>>
const T1 &f99(const std::vector<T1, T2> &o) {
  return o.back();
}

template <typename T1, typename T2 = std::allocator<T1>>
void f100(std::vector<std::vector<T1, T2>> &o,
          const std::vector<T1, T2> &value) {
  return o.push_back(value);
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::iterator
f101(std::vector<T1, T2> &o, typename std::vector<T1, T2>::const_iterator pos,
     const T1 *first, const T1 *last) {
  return o.insert(pos, first, last);
}

template <typename T1, typename T2 = std::allocator<T1>>
void f102(std::vector<T1, T2> &o, std::size_t n,
          const typename std::vector<T1, T2>::value_type &value) {
  return o.resize(n, value);
}

template <typename T1, typename T2 = std::allocator<T1>>
std::vector<T1, T2> &f103(std::vector<T1, T2> &dst, std::vector<T1, T2> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1, typename T2 = std::allocator<T1>>
typename std::vector<T1, T2>::const_iterator
f104(const std::vector<T1, T2> &o) {
  return o.cend();
}

template <typename T1, typename T2 = std::allocator<T1>>
std::vector<T1, T2> &f105(std::vector<T1, T2> &dst,
                          const std::vector<T1, T2> &src) {
  return dst.operator=(src);
}

template <typename T1, typename T2 = std::allocator<T1>>
void f106(std::vector<T1, T2> &o) {
  return o.shrink_to_fit();
}

template <typename T1> std::vector<T1> f107(std::vector<T1> &&o) {
  return std::vector<T1>(std::move(o));
}

template <typename T1, typename T2 = std::allocator<T1>>
std::vector<T1, T2> f108(std::vector<T1, T2> &&o) {
  return std::vector<T1, T2>(std::move(o));
}

template <typename T1> std::vector<T1> f109(const std::vector<T1> &o) {
  return std::vector<T1>(o);
}

template <typename T1, typename T2 = std::allocator<T1>>
std::vector<T1, T2> f110(const std::vector<T1, T2> &o) {
  return std::vector<T1, T2>(o);
}

template <typename T1>
std::vector<std::vector<T1>> &f111(std::vector<std::vector<T1>> &dst,
                                   std::vector<std::vector<T1>> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1, typename... Args>
T1 &f112(std::vector<T1> &o, Init<T1, Args> &&...args) {
  return o.emplace_back(std::forward<Args>(args)...);
}

template <typename T1, typename... Args>
std::vector<T1> &f113(std::vector<std::vector<T1>> &o,
                      Init<std::vector<T1>, Args> &&...args) {
  return o.emplace_back(std::forward<Args>(args)...);
}

template <typename T1, typename T2 = std::allocator<T1>, typename... Args>
T1 &f114(std::vector<T1, T2> &o, Init<T1, Args> &&...args) {
  return o.emplace_back(std::forward<Args>(args)...);
}

template <typename T1>
bool f115(const std::vector<T1> &a, const std::vector<T1> &b) {
  return operator==(a, b);
}

template <typename T1>
bool f116(const std::vector<T1> &a, const std::vector<T1> &b) {
  return operator!=(a, b);
}

template <typename T1, typename T2 = std::allocator<T1>>
bool f117(const std::vector<T1, T2> &a, const std::vector<T1, T2> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2 = std::allocator<T1>>
bool f118(const std::vector<T1, T2> &a, const std::vector<T1, T2> &b) {
  return operator!=(a, b);
}

// `v.rbegin()` / `v.rend()`.  Measured UNMAPPED before this (E0599 `no method named
// rbegin found for Vec<i32>` in BOTH models), which made rules/reverse_iterator
// (b23a1fa + df5012e) unreachable from idiomatic vector code.
//
// The return type is spelled EXACTLY as rules/reverse_iterator's `t2`
// (`std::reverse_iterator<std::__wrap_iter<T1 *>>`) so the two modules agree; writing
// `typename std::vector<T1>::reverse_iterator` would risk a different spelling and a
// dead key.  rules/vector itself does NOT map `__wrap_iter<T1 *>`, which is why the
// inner spelling is written out here rather than reused from a `tN`.
//
// THE OFF-BY-ONE IS DELIBERATE AND MUST BE PRESERVED: a C++ reverse_iterator holds
// `current`, the pointer ONE PAST the element `*rit` designates (proved by
// `rbegin().base() == end()`, base_delta 3 on a 3-element vector).  So rbegin() must
// yield end(), NOT end()-1, and rend() must yield begin().  Both representations are
// the same raw pointer / Ptr that rules/reverse_iterator's t2 target uses, so no
// conversion is needed.
template <typename T1>
std::reverse_iterator<std::__wrap_iter<T1 *>> f119(std::vector<T1> &o) {
  return o.rbegin();
}

template <typename T1>
std::reverse_iterator<std::__wrap_iter<T1 *>> f120(std::vector<T1> &o) {
  return o.rend();
}

// g448 / g450 -- ARITHMETIC on `std::__wrap_iter<X *>`, which IS
// `std::vector<X>::iterator` in libc++ and is ALREADY mapped here as t2, so no new
// type key is needed -- only these two member operators.  Written in MEMBER form
// (`it.operator+=(n)`), the shape rules/smallvector f20 uses; an infix spelling
// records nothing silently.  The recorded key's parameter type comes from the
// CALLEE (difference_type == long), not from what is written here.
template <typename T1>
typename std::vector<T1>::iterator &
f121(typename std::vector<T1>::iterator &it, std::ptrdiff_t n) {
  return it.operator+=(n);
}

template <typename T1>
typename std::vector<T1>::iterator
f122(typename std::vector<T1>::iterator it, std::ptrdiff_t n) {
  return it.operator-(n);
}

// g849 -- RELATIONAL comparison on `std::vector<X>::iterator`, which libc++ spells
// `std::__wrap_iter<X *>` and which is ALREADY mapped here as t2 (and t6).  NO NEW
// TYPE KEY, so there is no swallow surface to argue: this row adds only a
// free-function operator.
//
// The row's "searched as" spelling, character-for-character:
//   bool std::operator>=(const std::__wrap_iter<int *> &, const std::__wrap_iter<int *> &)
// libc++ declares these as NON-MEMBER templates on `__wrap_iter`, so this must be
// written in FREE-FUNCTION form -- found by ADL exactly as f115/f116 find
// `operator==` on `std::vector`.  A member spelling (`a.operator>=(b)`) does not
// resolve and would record nothing, silently.  The recorded parameter types come
// from the CALLEE (`const __wrap_iter<T1 *> &`), not from the by-value spelling
// written here.
//
// The three other relations are written too: they are the same one-line mechanism,
// they share the receiver, and `it < last` / `it > first` are the overwhelmingly
// more common source shapes, so covering only `>=` would leave the same receiver
// aborting on the next TU.
//
// SEMANTICS: a `__wrap_iter<X *>` comparison IS a raw-pointer comparison, and C++
// only defines the ordering for two iterators into the SAME container -- so the
// unsafe model compares the pointers directly and the refcount model compares the
// two handles' offsets.  `get_offset()` is the same accessor f121/f122 and the
// `operator-` distance rule (f36/f88) already use for exactly this reason: a
// refcount `Ptr` is not ordered as a machine address, only as a position.
template <typename T1>
bool f123(typename std::vector<T1>::iterator a,
          typename std::vector<T1>::iterator b) {
  return operator>=(a, b);
}

template <typename T1>
bool f124(typename std::vector<T1>::iterator a,
          typename std::vector<T1>::iterator b) {
  return operator<=(a, b);
}

template <typename T1>
bool f125(typename std::vector<T1>::iterator a,
          typename std::vector<T1>::iterator b) {
  return operator<(a, b);
}

template <typename T1>
bool f126(typename std::vector<T1>::iterator a,
          typename std::vector<T1>::iterator b) {
  return operator>(a, b);
}

// ============================================================================
// POINTER-MONO SIBLINGS FOR THE `const T1 *` EXPRESSION KEYS (f127..f136).
//
// ⛔ THE DEFECT: a key whose RECORDED src contains `const T1 *` can never match a
// corpus site that instantiates that `T1` with a POINTER type.  clang prints the
// const_iterator of `std::vector<X *>` as `std::__wrap_iter<X *const *>` -- const to
// the RIGHT of the star -- and `matchTemplate` is purely TEXTUAL, so the key's literal
// `, const ` / `<const ` segment has nothing to match.  The key records fine, loads
// fine, and NEVER FIRES.  `t8` above already fixed this for the TYPE spelling; these
// fix it for the OPERATIONS, which t8 does not shield.
//
// Each sibling is written the PAIR-MONO way: the starred form is NEVER typed by hand,
// it is spelled through `typename std::vector<T1 *>::const_iterator` /
// `::const_pointer` and clang prints the star form for us.  Bodies are COPIED from the
// original key, so the REPRESENTATION IS UNCHANGED -- that is what makes this cheap.
//
// SWALLOW-SAFETY, argued on the LITERALS (not on the length tie-break, which
// mapper.cpp:430-437 uses `>` and so CANNOT protect a sole candidate):
//   (a) The ORIGINAL cannot steal the pointer spelling.  Its recorded src carries the
//       literal `<const ` before the placeholder (e.g. `std::__wrap_iter<const T1 *>`).
//       The pointer-element row is `std::__wrap_iter<X *const *>`, which has no `const`
//       immediately after `<`.  So for that row the SIBLING IS THE SOLE CANDIDATE and
//       no tie-break is involved.
//   (b) The SIBLING cannot steal the original's non-pointer spellings.  Its literals
//       demand the EXTRA STAR: ` *const *>` after the placeholder.  Against
//       `std::__wrap_iter<const int *>` there is no ` *const *` anywhere, so the match
//       fails outright.  The original keeps every row it owns today.
// Same argument shape as t8's, and as rules/deque_iterator's t3/t4 (4c7c7c97).
// ============================================================================

// f127 -- f43 at element type `T1 *`.  `begin() const`.
template <typename T1>
typename std::vector<T1 *>::const_iterator f127(const std::vector<T1 *> &o) {
  return o.begin();
}

// f128 -- f44 at element type `T1 *`.  `end() const`.
template <typename T1>
typename std::vector<T1 *>::const_iterator f128(const std::vector<T1 *> &o) {
  return o.end();
}

// f129 -- f57 at element type `T1 *`.  `cend() const`.
template <typename T1>
typename std::vector<T1 *>::const_iterator f129(const std::vector<T1 *> &o) {
  return o.cend();
}

// f130 -- f41 at element type `T1 *`.  `data() const`.  The return type is spelled
// through `::const_pointer` so that `T1 *const *` is clang's word, not mine.
template <typename T1>
typename std::vector<T1 *>::const_pointer f130(const std::vector<T1 *> &o) {
  return o.data();
}

// f131 -- f24 at element type `T1 *`.  iterator -> const_iterator conversion ctor.
template <typename T1>
typename std::vector<T1 *>::const_iterator
f131(const typename std::vector<T1 *>::iterator &it) {
  return typename std::vector<T1 *>::const_iterator(it);
}

// f132 -- f95 at element type `T1 *`.  `begin() const`, explicit-allocator spelling.
template <typename T1, typename T2 = std::allocator<T1 *>>
typename std::vector<T1 *, T2>::const_iterator
f132(const std::vector<T1 *, T2> &o) {
  return o.begin();
}

// f133 -- f96 at element type `T1 *`.  `end() const`, explicit-allocator spelling.
template <typename T1, typename T2 = std::allocator<T1 *>>
typename std::vector<T1 *, T2>::const_iterator
f133(const std::vector<T1 *, T2> &o) {
  return o.end();
}

// f134 -- f104 at element type `T1 *`.  `cend() const`, explicit-allocator spelling.
template <typename T1, typename T2 = std::allocator<T1 *>>
typename std::vector<T1 *, T2>::const_iterator
f134(const std::vector<T1 *, T2> &o) {
  return o.cend();
}

// f135 -- f93 at element type `T1 *`.  `data() const`, explicit-allocator spelling.
template <typename T1, typename T2 = std::allocator<T1 *>>
typename std::vector<T1 *, T2>::const_pointer
f135(const std::vector<T1 *, T2> &o) {
  return o.data();
}

// f136 -- f83 at element type `T1 *`.  iterator -> const_iterator, explicit allocator.
template <typename T1, typename T2 = std::allocator<T1 *>>
typename std::vector<T1 *, T2>::const_iterator
f136(const typename std::vector<T1 *, T2>::iterator &it) {
  return typename std::vector<T1 *, T2>::const_iterator(it);
}
