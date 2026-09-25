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

// std::vector<bool>::at -- the bool SPECIALIZATION only.
//
// Why it needs a rule of its own when the generic f7 already covers
// `T1 & std::vector<T1>::at(unsigned long)`: libc++'s vector<bool> does not
// return `bool &`, it returns the proxy `std::__bit_reference<std::vector<
// bool>>`, so the printed signature is
// `std::__bit_reference<std::vector<bool>> std::vector<bool>::at(unsigned
// long)` and f7 never matches it.  Without this rule the call falls to
// ConvertGenericCallExpr, which emits a method call on a record type and
// loses the receiver -- `( { . at_usize ( ... ) } )` -- and then aborts the
// refcount model in assert_consumed ("pending_deref_ not consumed").
// Reproduced on dsc-based-utils/progtailor/progtailor_standalone.cpp and its
// three siblings, via `connectivity_.at(from).at(to)`.
//
// READ-ONLY, and that is forced, not chosen.  The declared return type is a
// CLASS (the proxy), not `bool &`, so the converter does not treat the result
// as an lvalue and never inserts the deref or the `.write()` that it inserts
// for f7.  A pointer-returning body was tried first and emits
// `(...).offset(i) = true` and `(...).offset(i) as bool`, neither of which is
// Rust.  Returning the VALUE is the only body that composes with f47
// (`__bit_reference::operator bool`, whose target is the identity), and it
// makes every READ correct.
//
// Consequence, stated plainly: `v.at(i) = x` on a vector<bool> now generates
// an assignment to a non-place expression, which rustc rejects.  It is a loud
// failure in the generated crate rather than a loud failure in the converter.
// No call site in either surveyed scope writes through vector<bool>::at --
// the one in the repo that does is sgr/sengraph.cpp:1263, and sgr/ is in
// neither scope; the sites this rule unblocks are the READS at
// sgr/sengraph.h:488 and :492, reached from the four progtailor TUs.
//
// vector<bool>::operator[] deliberately gets NO rule: the generic path
// already turns it into `(v.as_pointer() as Ptr<bool>).offset(i)`, which
// reads AND writes correctly, and a value-returning rule here would shadow
// and break that.  Same for back().

std::vector<bool>::reference f127(std::vector<bool> &o, std::size_t idx) {
  return o.at(idx);
}

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

template <typename T1>
bool f112(const std::vector<T1> &a, const std::vector<T1> &b) {
  return operator==(a, b);
}

template <typename T1>
bool f113(const std::vector<T1> &a, const std::vector<T1> &b) {
  return operator!=(a, b);
}

// ---------------------------------------------------------------------------
// Reverse iterators (rbegin/rend/crbegin/crend and the ops on the result).
//
// MODEL: a std::reverse_iterator over a contiguous container is represented by
// a pointer to the element it DEREFERENCES to, and walking it backwards.
// So rbegin() addresses the LAST element, operator++ DECREMENTS the pointer,
// operator-- increments it, and operator*() is the identity (unlike the real
// std::reverse_iterator, whose stored `base()` sits one past the element).
//
// rend() is therefore the slot one BEFORE the first element. Both models can
// name that slot exactly and reproducibly:
//   * refcount: Ptr<T> keeps a `usize` offset, so "one before index 0" is the
//     wrapped offset usize::MAX; PartialEq compares (kind, offset) so a
//     decremented iterator and a freshly built rend() compare equal.
//   * unsafe: a raw `base - 1`, built with wrapping_sub/offset(-1) so the two
//     spellings produce the identical address.
// On an EMPTY container rbegin() and rend() both land on that same slot, so a
// reverse loop terminates immediately -- which is what C++ does too.
//
// base() is the pointer + 1, i.e. the forward iterator one past the element,
// exactly as the standard specifies.
//
// A single pair of rules covers both the const and the non-const reverse
// iterator: `std::__wrap_iter<T1 *>` unifies with `std::__wrap_iter<const
// int *>` by capturing T1 = `const int`, the same way f22/f26/f27 already
// serve std::vector<T1>::const_iterator.
// ---------------------------------------------------------------------------

template <typename T1>
using t8 = typename std::vector<T1>::reverse_iterator;
template <typename T1>
using t9 = typename std::vector<T1>::const_reverse_iterator;

template <typename T1>
typename std::vector<T1>::reverse_iterator f114(std::vector<T1> &o) {
  return o.rbegin();
}

template <typename T1>
typename std::vector<T1>::reverse_iterator f115(std::vector<T1> &o) {
  return o.rend();
}

template <typename T1>
typename std::vector<T1>::const_reverse_iterator f116(const std::vector<T1> &o) {
  return o.rbegin();
}

template <typename T1>
typename std::vector<T1>::const_reverse_iterator f117(const std::vector<T1> &o) {
  return o.rend();
}

template <typename T1>
typename std::vector<T1>::const_reverse_iterator
f118(const std::vector<T1> &o) {
  return o.crbegin();
}

template <typename T1>
typename std::vector<T1>::const_reverse_iterator
f119(const std::vector<T1> &o) {
  return o.crend();
}

template <typename T1>
typename std::vector<T1>::reference
f120(typename std::vector<T1>::reverse_iterator it) {
  return it.operator*();
}

template <typename T1>
typename std::vector<T1>::reverse_iterator &
f121(typename std::vector<T1>::reverse_iterator &it) {
  return it.operator++();
}

template <typename T1>
typename std::vector<T1>::reverse_iterator
f122(typename std::vector<T1>::reverse_iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1>
typename std::vector<T1>::reverse_iterator &
f123(typename std::vector<T1>::reverse_iterator &it) {
  return it.operator--();
}

template <typename T1>
bool f124(const typename std::vector<T1>::reverse_iterator &it1,
          const typename std::vector<T1>::reverse_iterator &it2) {
  return operator==(it1, it2);
}

template <typename T1>
bool f125(const typename std::vector<T1>::reverse_iterator &it1,
          const typename std::vector<T1>::reverse_iterator &it2) {
  return operator!=(it1, it2);
}

template <typename T1>
typename std::vector<T1>::iterator
f126(const typename std::vector<T1>::reverse_iterator &it) {
  return it.base();
}

// ---------------------------------------------------------------------------
// Contiguous-iterator operator-(long): `v.begin() + n - k` and `v.end() - k`.
//
// f25/f84 already cover operator+(long) on the same std::__wrap_iter<T1 *>
// receiver; the subtracting half had no rule at all, so `foldParams.begin() +
// i - 1` in ddc/ddc_fold.cpp:424 died at converter.cpp:3455 with
// "unsupported CXXOperatorCallExpr: -".  Note the operator- SPELLING matters:
// `long std::operator-(const __wrap_iter &, const __wrap_iter &)` is the
// iterator-DIFFERENCE and is already rules/vector f33/f88; this is the member
// `__wrap_iter __wrap_iter::operator-(long) const`, a different signature.
//
// The parameter is spelled std::size_t rather than long to match f25's
// spelling, which is what makes the two sides of `begin() + i - 1` agree on
// one Rust integer type at the call site.
// ---------------------------------------------------------------------------

template <typename T1>
typename std::vector<T1>::iterator f128(typename std::vector<T1>::iterator it,
                                        std::size_t n) {
  return it.operator-(n);
}

// ---------------------------------------------------------------------------
// emplace_back through an argument pack (upstream 7896632, which also DELETED
// the emplace_back converter plugin these replace).
//
// RENUMBERED f112/f113/f114 -> f129/f130/f131. Both sides independently grew
// this module past the merge-base's f111, so upstream's three new rules and our
// seventeen (vector==/!=, reverse iterators, vector<bool>::at, iterator
// operator-) claimed the same numbers. The names are module-local -- nothing
// outside rules/vector refers to them, and rules/errno has its own unrelated
// f112 -- so renumbering the smaller set is a rename with no behavioural
// content. Ours keep their numbers because tgt_unsafe/tgt_refcount and the
// frozen IR already agree on them.
// ---------------------------------------------------------------------------

template <typename T1, typename... Args>
T1 &f129(std::vector<T1> &o, Init<T1, Args> &&...args) {
  return o.emplace_back(std::forward<Args>(args)...);
}

template <typename T1, typename... Args>
std::vector<T1> &f130(std::vector<std::vector<T1>> &o,
                      Init<std::vector<T1>, Args> &&...args) {
  return o.emplace_back(std::forward<Args>(args)...);
}

template <typename T1, typename T2 = std::allocator<T1>, typename... Args>
T1 &f131(std::vector<T1, T2> &o, Init<T1, Args> &&...args) {
  return o.emplace_back(std::forward<Args>(args)...);
}

// ---------------------------------------------------------------------------
// RENUMBERED f129-f132 -> f132-f135.  These were written against 6f21c8d, where
// f128 was the highest number in this module; the upstream merge landed an
// emplace_back argument-pack set that independently took f129/f130/f131 (and was
// itself renumbered from f112/f113/f114 for the same reason -- see the note above
// it).  The names are module-local, so this is a rename with no behavioural
// content, and t10 was free on both sides.
//
// std::reverse_iterator over a RAW POINTER -- `std::reverse_iterator<T1 *>`,
// as distinct from f114-f126's `std::reverse_iterator<std::__wrap_iter<T1 *>>`.
//
// WHY THIS IS A SEPARATE SET OF RULES AND NOT A GENERALISATION OF THOSE.
// A rule matches on the SIGNATURE STRING, and libc++ spells the two
// differently because the underlying iterator is a different type: a
// std::vector's iterator is the wrapper class `std::__wrap_iter<T1 *>`, while
// a container whose iterator IS a raw pointer reverses to
// `std::reverse_iterator<T1 *>` with no wrapper in between.  f121's spelling
// therefore cannot match the latter, no matter how the template parameter is
// captured -- `std::__wrap_iter<T1 *>` cannot unify with `mlir::sentient::IfOp
// *` because the outer template name differs.  Confirmed by the converter's
// own lookup: `cpp2rust --verbose` on
// dcc/src/Transform/Sentient/CFGSimplificationSentientLevel.cpp prints
//
//   search expr std::reverse_iterator<mlir::sentient::IfOp *> &
//               std::reverse_iterator<mlir::sentient::IfOp *>::operator++(),
//               result:
//
// -- an empty result next to an f121 that was already present and matching
// the vector spelling in the same run.
//
// WHERE THESE OCCUR.  Two shapes, both raw-pointer-iterator containers:
//   * llvm::SmallVector<T> / SmallVectorTemplateCommon<T>::rbegin(), which is
//     `std::reverse_iterator<T *>`.  Five sites over four TUs in dcc/,
//     e.g. CFGSimplificationSentientLevel.cpp:875 `for (auto it =
//     if_ops_to_clean_up.rbegin(); it != ...rend(); ++it)`, and
//     LiveRangeReduction.cpp:862 `for (auto result = list.rbegin(); ...;
//     result++)` -- note the POSTFIX form there, which is why f133 exists.
//   * std::array<T, N>::rbegin(), the same spelling from a std container, which
//     is what makes this reachable by a self-contained probe rather than only
//     through LLVM headers.
//
// MODEL: identical to f114-f126's, and deliberately so -- a reverse iterator
// is the POINTER TO THE ELEMENT IT DEREFERENCES TO, walked backwards.  So
// operator++ DECREMENTS, operator-- increments, operator*() is the identity,
// and rend() is the slot one before the first element.  The full justification,
// including why that differs from the real std::reverse_iterator (whose stored
// base() sits one past the element) and how each model names the
// one-before-the-first slot reproducibly, is in the comment above f114; read it
// there rather than trusting this paragraph.
//
// THE TRAP THAT MATTERS, stated for the postfix rule f133.  Post-increment
// yields a COPY OF THE OLD VALUE, which for an iterator means the OLD
// POSITION -- so after `auto old = it++`, `*old` must be the element `it`
// addressed BEFORE the step.  A body returning the new position compiles
// cleanly and answers the NEXT element, which on this reversed representation
// means one element further TOWARDS THE FRONT.  Verified by running, not by
// reading: on `std::array<int,4> a = {10,20,30,40}`, C++ gives `old=40 new=30`,
// and so must the translation.  PostfixDec is therefore the correct trait here
// -- "return the old value, then step" where the step is a decrement -- exactly
// as f122 already uses it for the wrapped spelling.
//
// NO operator== / operator!= RULES ARE ADDED HERE.  The comparison on this
// spelling is `bool std::__1::operator!=(const std::reverse_iterator<T1 *> &,
// const std::reverse_iterator<T1 *> &)`, which is a DIFFERENT gap class
// (`CXXOperatorCallExpr !=`, the top row of the blocker list) and is being
// worked separately.  Adding a half of it here would collide with that work
// for no gain, since every loop site needs both halves to translate.  The
// honest position: these rules clear the `++` occurrences and leave the `!=`
// on the same lines recorded, which is what the survey will show.
// ---------------------------------------------------------------------------

template <typename T1> using t10 = std::reverse_iterator<T1 *>;

template <typename T1>
std::reverse_iterator<T1 *> &f132(std::reverse_iterator<T1 *> &it) {
  return it.operator++();
}

template <typename T1>
std::reverse_iterator<T1 *> f133(std::reverse_iterator<T1 *> a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1>
std::reverse_iterator<T1 *> &f134(std::reverse_iterator<T1 *> &it) {
  return it.operator--();
}

template <typename T1> T1 &f135(std::reverse_iterator<T1 *> it) {
  return it.operator*();
}

// ---------------------------------------------------------------------------
// COMPARISON on the raw-pointer reverse iterator: `it != v.rend()`.
//
// The other half of t10/f132-f135. I left these out when the ++ rules landed on
// the grounds that ==/!= was another agent's class; it is not being worked, and
// a measurement changed the priority: with converter.cpp's materialized-temp
// abort fixed (9df5804), dcc/src/Transform/Sentient/LiveRangeReduction.cpp went
// from a hard rc=134 to exactly TWO recorded gaps, and both are this comparison
// -- :838 on reverse_iterator<mlir::BlockArgument *> and :862 on
// reverse_iterator<mlir::Value *>. So these two rules take that TU to zero.
// Five more sites across four further TUs use the same spelling
// (reverse_iterator<mlir::sentient::IfOp *>, <const EvaluatedValue **>,
// <std::pair<mlir::Operation *, unsigned> *>, <std::pair<mlir::sentient::ForOp,
// ...> *>).
//
// f124/f125 already do this for the WRAPPED spelling
// (std::reverse_iterator<std::__wrap_iter<T1 *>>) and cannot match this one, for
// the same reason f121 could not match f132 -- the outer template name inside
// differs. Verified the same way: an empty `result:` next to a matching f125 in
// one --verbose run.
//
// SEMANTICS: pointer comparison, because the representation IS the pointer to
// the element the iterator dereferences to. Two reverse iterators into the same
// container are equal exactly when they address the same element, and rend() is
// the one-before-first slot both models name reproducibly (see above f114). So
// `it != rend()` terminates a reverse walk at the right place and not one step
// early or late -- the failure mode that would matter here, and the reason the
// probe walks a container to EXHAUSTION and prints the element count rather
// than just comparing two iterators.
// ---------------------------------------------------------------------------

template <typename T1>
bool f136(const std::reverse_iterator<T1 *> &a,
          const std::reverse_iterator<T1 *> &b) {
  return operator==(a, b);
}

template <typename T1>
bool f137(const std::reverse_iterator<T1 *> &a,
          const std::reverse_iterator<T1 *> &b) {
  return operator!=(a, b);
}
