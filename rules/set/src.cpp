// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstddef>
#include <set>
#include <utility>

// std::set is modelled as a map to unit: it reuses BTreeMap and the existing
// RefcountMapIter, which is also what makes the converter treat a set iterator
// as bidirectional without any special casing.
//
// There is deliberately NO rule for std::set<T, Comparator>, and the reason is
// NOT the one the port playbook gives (it says a derived Ord would change dedup
// and iteration order, which is true of a naive attempt but is fixable).  The
// blocker is mechanical and lives in the RULE FORMAT:
//
//   1. libc++ prints set<T>::iterator and set<T, C>::iterator as the SAME
//      string -- std::__tree_const_iterator<T1, std::__tree_node<T1, void *> *,
//      long> -- because the comparator is not part of the iterator type.  So a
//      `t4 = set<T1, T2>::iterator` rule collides with `t2` on the src side.
//      Two rules for one C++ spelling are accepted only when they map to the
//      SAME Rust type (mapper.cpp:781); a comparator-aware iterator is by
//      definition a different Rust type, so adding one makes the converter exit
//      with "duplicate type rule" on EVERY translation, not just the set ones.
//
//   2. Attaching the comparator's logic to the element type needs an
//      `impl Ord`-like item mentioning BOTH the project's element type and the
//      project's comparator type.  A rule target is a type expression plus
//      inlined expression bodies; the "private trait inside the rule body"
//      trick cannot carry it, because a trait impl is coherence-checked
//      globally and the body is inlined once per call site (E0119 on the second
//      site).  Emitting it is converter work, not rule work.
//
// So std::set<T, Comparator> stays a loud UnmappedType abort.  See the port
// playbook for the three dt_src TUs this blocks.
template <typename T1> using t1 = std::set<T1>;

// In libc++ std::set<T>::iterator and ::const_iterator are the same type, so
// one rule covers both.
template <typename T1> using t2 = typename std::set<T1>::iterator;

template <typename T1> std::size_t f1(const std::set<T1> &o) {
  return o.size();
}

template <typename T1> bool f2(const std::set<T1> &o) { return o.empty(); }

template <typename T1> void f3(std::set<T1> &o) { return o.clear(); }

template <typename T1> std::size_t f4(const std::set<T1> &o, const T1 &k) {
  return o.count(k);
}

template <typename T1>
typename std::set<T1>::iterator f5(std::set<T1> &o, const T1 &k) {
  return o.find(k);
}

template <typename T1>
typename std::set<T1>::const_iterator f6(const std::set<T1> &o, const T1 &k) {
  return o.find(k);
}

template <typename T1> typename std::set<T1>::iterator f7(std::set<T1> &o) {
  return o.begin();
}

template <typename T1> typename std::set<T1>::iterator f8(std::set<T1> &o) {
  return o.end();
}

template <typename T1>
typename std::set<T1>::const_iterator f9(const std::set<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::set<T1>::const_iterator f10(const std::set<T1> &o) {
  return o.end();
}

template <typename T1>
std::pair<typename std::set<T1>::iterator, bool> f11(std::set<T1> &o,
                                                     const T1 &v) {
  return o.insert(v);
}

template <typename T1> std::size_t f12(std::set<T1> &o, const T1 &k) {
  return o.erase(k);
}

template <typename T1> std::set<T1> f13() { return std::set<T1>(); }

template <typename T1> std::set<T1> f14(const std::set<T1> &o) {
  return std::set<T1>(o);
}

template <typename T1>
std::set<T1> &f15(std::set<T1> &dst, const std::set<T1> &src) {
  return dst.operator=(src);
}

template <typename T1>
bool f16(typename std::set<T1>::iterator a, typename std::set<T1>::iterator b) {
  return operator!=(a, b);
}

template <typename T1>
bool f17(typename std::set<T1>::iterator a, typename std::set<T1>::iterator b) {
  return operator==(a, b);
}

template <typename T1> const T1 &f18(typename std::set<T1>::iterator it) {
  return it.operator*();
}

template <typename T1>
std::pair<typename std::set<T1>::iterator, bool> f19(std::set<T1> &o, T1 &&v) {
  return o.insert(std::move(v));
}

template <typename T1>
typename std::set<T1>::iterator &f20(typename std::set<T1>::iterator &it) {
  return it.operator++();
}

template <typename T1>
typename std::set<T1>::iterator f21(typename std::set<T1>::iterator a0,
                                    int a1) {
  return a0.operator++(a1);
}

template <typename T1>
bool f22(const std::set<T1> &a, const std::set<T1> &b) {
  return operator==(a, b);
}

template <typename T1>
bool f23(const std::set<T1> &a, const std::set<T1> &b) {
  return operator!=(a, b);
}

// The initializer-list constructor: `std::set<T> s = {a, b, c};`.  libc++
// declares it as
//     set(initializer_list<value_type>, const value_compare& = value_compare())
// so the resolved rule carries the comparator parameter too, and the converter
// passes `None` for it.
template <typename T1> std::set<T1> f24(std::initializer_list<T1> a0) {
  return std::set<T1>(a0);
}

// emplace.  A set's element IS the single constructor argument, so the only two
// arities that occur are emplace(T1 &&) and emplace(const T1 &) -- exactly as in
// rules/unordered_set (:150, :156).  There is no `arg_idx` hazard here (the one
// that hits std::map::emplace, whose element is a pair built from two args).
//
// SEMANTICS: emplace returns pair<iterator, bool> with the bool true only when
// the element was NOT already present, and on collision the INCUMBENT is kept --
// the newcomer is destroyed, not assigned over the old element.  The targets are
// therefore probe-and-insert, like f11/f19 and like rules/denseset, NOT
// BTreeMap::insert (which would overwrite the mapped copy while keeping the old
// key, blending two elements that a subset-comparing Ord calls equal).
template <typename T1>
std::pair<typename std::set<T1>::iterator, bool> f25(std::set<T1> &o, T1 &&v) {
  return o.emplace(std::move(v));
}

template <typename T1>
std::pair<typename std::set<T1>::iterator, bool> f26(std::set<T1> &o,
                                                     const T1 &v) {
  return o.emplace(v);
}
