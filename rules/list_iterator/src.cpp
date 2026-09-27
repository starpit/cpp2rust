// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::list's ITERATORS -- `std::__list_iterator<T1, void *>` and
// `std::__list_const_iterator<T1, void *>`.  rules/list keys the CONTAINER only
// and deliberately left these loud; queue rows g023/g594/g1296 (types),
// g418/g489 (== / !=), g439/g440 (prefix/postfix ++), g207/g213 (const_iterator
// != and postfix ++) are this module.
//
// MODEL, and why it is NOT LinkedList's cursor.  rules/list models a
// std::list<T1> as `Vec<T1>` because Ptr<> has no linked-list infrastructure and
// Rust's LinkedList has no stable public cursor API.  Given the container IS a
// Vec in BOTH models, its iterator is exactly rules/vector's iterator -- a raw
// element pointer in the unsafe model (`*mut T1` / `*const T1`) and a `Ptr<T1>`
// in the refcount model.  Every body below is byte-for-byte the shape
// rules/vector f13/f17/f22/f26/f27/f34/f28/f43/f44 already uses, against the
// same `Vec<T1>` representation, so nothing new is invented here.
//
// CONSEQUENCE, stated plainly: this inherits rules/list's complexity trade and
// its ONE semantic gap -- a std::list iterator SURVIVES an insert/erase
// elsewhere in the list, a Vec element pointer does not.  Nothing keyed here
// mutates the container, so no keyed operation can observe the difference; a
// future insert()/erase() TAKING an iterator must not be added without
// revisiting this.
//
// begin()/end() live HERE rather than in rules/list because they are the
// iterator-producing API and rules/list is a separate owner; a rule module is
// data keyed by signature, so the split is free.
//
// MEASURED LIMITATION, LOUD NOT SILENT.  `operator++()` returns `iterator &`,
// and the converter consumes a reference-returning result by DEREFERENCING it,
// emitting `let s: *mut i32 = (*r.prefix_inc());` for `iterator s = ++r;`.  That
// is E0308 in the unsafe model (and the analogous E0308 in refcount), because
// the target returns the iterator by value, not a pointer to it.  The discarded
// form `++it` used by every queue row here (g207/g213/g418/g439/g440/g489 are
// all loop increments) is correct and byte-exact.  rules/vector f34 and
// rules/set f20 have the IDENTICAL shape, so this is a pre-existing
// reference-return convention question, not specific to this module; it is left
// FAILING LOUDLY rather than papered over by returning `*mut *mut T1`, which
// would be a guess.
//
// NOT COVERED: operator-- (no row), operator->, rbegin/rend/crbegin/crend,
// the iterator-taking insert/erase/splice, and the const_iterator <- iterator
// conversion (no row).

#include <cstddef>
#include <list>

template <typename T1> using t1 = typename std::list<T1>::iterator;
template <typename T1> using t2 = typename std::list<T1>::const_iterator;

template <typename T1>
typename std::list<T1>::iterator f1(std::list<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::list<T1>::iterator f2(std::list<T1> &o) {
  return o.end();
}

template <typename T1>
typename std::list<T1>::const_iterator f3(const std::list<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::list<T1>::const_iterator f4(const std::list<T1> &o) {
  return o.end();
}

// libc++ declares == / != on __list_iterator as FREE functions, so the rule
// body must be the UNQUALIFIED call form (a member form records nothing, and a
// qualified `std::operator==` aborts at cpp_rule_preprocessor.cpp:888).
template <typename T1>
bool f5(const typename std::list<T1>::iterator &it1,
        const typename std::list<T1>::iterator &it2) {
  return operator==(it1, it2);
}

template <typename T1>
bool f6(const typename std::list<T1>::iterator &it1,
        const typename std::list<T1>::iterator &it2) {
  return operator!=(it1, it2);
}

// ++ IS a member, so it uses member form.
template <typename T1>
typename std::list<T1>::iterator &
f7(typename std::list<T1>::iterator &it) {
  return it.operator++();
}

template <typename T1>
typename std::list<T1>::iterator f8(typename std::list<T1>::iterator a0,
                                    int a1) {
  return a0.operator++(a1);
}

template <typename T1>
typename std::list<T1>::reference
f9(typename std::list<T1>::iterator it) {
  return it.operator*();
}

template <typename T1>
bool f10(const typename std::list<T1>::const_iterator &it1,
         const typename std::list<T1>::const_iterator &it2) {
  return operator==(it1, it2);
}

template <typename T1>
bool f11(const typename std::list<T1>::const_iterator &it1,
         const typename std::list<T1>::const_iterator &it2) {
  return operator!=(it1, it2);
}

template <typename T1>
typename std::list<T1>::const_iterator &
f12(typename std::list<T1>::const_iterator &it) {
  return it.operator++();
}

template <typename T1>
typename std::list<T1>::const_iterator
f13(typename std::list<T1>::const_iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1>
typename std::list<T1>::const_reference
f14(typename std::list<T1>::const_iterator it) {
  return it.operator*();
}
