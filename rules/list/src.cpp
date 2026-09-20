// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// ---------------------------------------------------------------------------
// std::list
//
// MODEL: std::list<T> is represented by the SAME Vec<T> that std::vector<T>
// and std::deque<T> use, and its iterators by the same pointer-to-element
// model (Ptr<T> in refcount, *mut T / *const T in unsafe). Iteration order,
// begin/end, front/back, push/pop at either end and erase/insert at a position
// are then already-solved problems, shared bit for bit with rules/vector and
// rules/deque.
//
// SEMANTICS NOT PRESERVED by that choice -- read this before trusting a
// translation that relies on them:
//   * ITERATOR / REFERENCE STABILITY. A real std::list never invalidates an
//     iterator or reference except to the erased element. Over a Vec:
//       - unsafe model: push_back/push_front/insert/resize may REALLOCATE, and
//         every previously handed out raw pointer dangles.
//       - refcount model: Ptr<T> is an (Rc, element offset) pair, so it
//         survives reallocation, but it names a POSITION, not a node: an
//         insert or erase anywhere at or before it silently re-points it at a
//         different element.
//     Code that stashes list iterators across mutations (the dt_src
//     DsTrackInMem::epsToListIter pattern, `std::unordered_map<int, ListIter>`)
//     is only safe here because it appends with push_back and then never
//     inserts or erases in the middle.
//   * COMPLEXITY. push_front/pop_front/insert/erase are O(1) on a list and
//     O(n) on a Vec. Semantics are unchanged; cost is not.
//   * SPLICE / MERGE / SORT / REMOVE are deliberately NOT translated. splice()
//     in particular is O(1) node relinking that preserves iterators into the
//     spliced range, and nothing in the Vec model reproduces that; leaving it
//     unmapped surfaces as an explicit translation failure rather than as
//     silently wrong code.
//
// libc++ spells the two iterators as separate class templates (unlike
// std::vector, where __wrap_iter<T1 *> also unifies with the const form by
// capturing T1 = `const int`), so each operation needs a const and a non-const
// rule:
//     std::__list_iterator<T1, void *>
//     std::__list_const_iterator<T1, void *>
// ---------------------------------------------------------------------------

#include <cstddef>
#include <list>
#include <map>
#include <string>
#include <vector>

template <typename T1> using t1 = std::list<T1>;
template <typename T1> using t2 = typename std::list<T1>::iterator;
template <typename T1> using t3 = typename std::list<T1>::const_iterator;

// --- capacity / modifiers on the container ---------------------------------

template <typename T1> std::size_t f1(const std::list<T1> &o) {
  return o.size();
}

template <typename T1> bool f2(const std::list<T1> &o) { return o.empty(); }

template <typename T1> void f3(std::list<T1> &o) { return o.clear(); }

template <typename T1> void f4(std::list<T1> &o, const T1 &value) {
  return o.push_back(value);
}

template <typename T1> void f5(std::list<T1> &o, T1 &&value) {
  return o.push_back(std::move(value));
}

template <typename T1> void f6(std::list<T1> &o, const T1 &value) {
  return o.push_front(value);
}

template <typename T1> void f7(std::list<T1> &o, T1 &&value) {
  return o.push_front(std::move(value));
}

template <typename T1> void f8(std::list<T1> &o) { return o.pop_back(); }

template <typename T1> void f9(std::list<T1> &o) { return o.pop_front(); }

// --- element access --------------------------------------------------------

template <typename T1> T1 &f10(std::list<T1> &o) { return o.front(); }

template <typename T1> const T1 &f11(const std::list<T1> &o) {
  return o.front();
}

template <typename T1> T1 &f12(std::list<T1> &o) { return o.back(); }

template <typename T1> const T1 &f13(const std::list<T1> &o) {
  return o.back();
}

// --- begin / end -----------------------------------------------------------

template <typename T1>
typename std::list<T1>::iterator f14(std::list<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::list<T1>::iterator f15(std::list<T1> &o) {
  return o.end();
}

template <typename T1>
typename std::list<T1>::const_iterator f16(const std::list<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::list<T1>::const_iterator f17(const std::list<T1> &o) {
  return o.end();
}

template <typename T1>
typename std::list<T1>::const_iterator f18(const std::list<T1> &o) {
  return o.cbegin();
}

template <typename T1>
typename std::list<T1>::const_iterator f19(const std::list<T1> &o) {
  return o.cend();
}

// --- construction / assignment ---------------------------------------------

template <typename T1> std::list<T1> f20() { return std::list<T1>(); }

template <typename T1> std::list<T1> f21(const std::list<T1> &o) {
  return std::list<T1>(o);
}

template <typename T1> std::list<T1> f22(std::list<T1> &&o) {
  return std::list<T1>(std::move(o));
}

template <typename T1>
std::list<T1> &f23(std::list<T1> &dst, const std::list<T1> &src) {
  return dst.operator=(src);
}

template <typename T1>
std::list<T1> &f24(std::list<T1> &dst, std::list<T1> &&src) {
  return dst.operator=(std::move(src));
}

// --- erase / insert --------------------------------------------------------

template <typename T1>
typename std::list<T1>::iterator
f25(std::list<T1> &o, typename std::list<T1>::const_iterator it) {
  return o.erase(it);
}

template <typename T1>
typename std::list<T1>::iterator
f26(std::list<T1> &o, typename std::list<T1>::const_iterator it,
    const T1 &value) {
  return o.insert(it, value);
}

template <typename T1>
typename std::list<T1>::iterator
f27(std::list<T1> &o, typename std::list<T1>::const_iterator it, T1 &&value) {
  return o.insert(it, std::move(value));
}

// --- iterator --------------------------------------------------------------

template <typename T1>
T1 &f28(typename std::list<T1>::iterator it) {
  return it.operator*();
}

template <typename T1>
typename std::list<T1>::iterator &
f29(typename std::list<T1>::iterator &it) {
  return it.operator++();
}

template <typename T1>
typename std::list<T1>::iterator f30(typename std::list<T1>::iterator a0,
                                     int a1) {
  return a0.operator++(a1);
}

template <typename T1>
typename std::list<T1>::iterator &
f31(typename std::list<T1>::iterator &it) {
  return it.operator--();
}

template <typename T1>
bool f32(const typename std::list<T1>::iterator &it1,
         const typename std::list<T1>::iterator &it2) {
  return operator==(it1, it2);
}

template <typename T1>
bool f33(const typename std::list<T1>::iterator &it1,
         const typename std::list<T1>::iterator &it2) {
  return operator!=(it1, it2);
}

// --- const_iterator --------------------------------------------------------

template <typename T1>
const T1 &f34(typename std::list<T1>::const_iterator it) {
  return it.operator*();
}

template <typename T1>
typename std::list<T1>::const_iterator &
f35(typename std::list<T1>::const_iterator &it) {
  return it.operator++();
}

template <typename T1>
typename std::list<T1>::const_iterator
f36(typename std::list<T1>::const_iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1>
typename std::list<T1>::const_iterator &
f37(typename std::list<T1>::const_iterator &it) {
  return it.operator--();
}

template <typename T1>
bool f38(const typename std::list<T1>::const_iterator &it1,
         const typename std::list<T1>::const_iterator &it2) {
  return operator==(it1, it2);
}

template <typename T1>
bool f39(const typename std::list<T1>::const_iterator &it1,
         const typename std::list<T1>::const_iterator &it2) {
  return operator!=(it1, it2);
}

// --- iterator copy / conversion --------------------------------------------
//
// erase() and insert() take a const_iterator, so every call that passes a
// plain iterator goes through this converting constructor first. Without f41
// those calls resolve to nothing and the whole TU fails to load.

template <typename T1>
typename std::list<T1>::iterator
f40(const typename std::list<T1>::iterator &it) {
  return typename std::list<T1>::iterator(it);
}

template <typename T1>
typename std::list<T1>::const_iterator
f41(const typename std::list<T1>::iterator &it) {
  return typename std::list<T1>::const_iterator(it);
}
