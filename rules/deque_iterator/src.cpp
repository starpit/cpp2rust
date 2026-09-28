// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::deque's ITERATORS -- libc++'s `std::__deque_iterator<T, Ptr, Ref, MapPtr,
// DiffType>`.  rules/deque keys the CONTAINER only and left these loud.  Queue
// rows: g592/g593/g279/g1284-g1292 (types), g211 (`+`),
// g212/g436/g438 (postfix `++`), g417 (`!=`), g447 (`+=`).
//
// WHICH MODULE.  Every one of those keys carries the FIVE-ARGUMENT libc++
// internal spelling with NO `__cxx11` segment anywhere, so they belong here
// (a sibling of rules/deque) and need no per-module cxxflags file.
//
// MODEL, and why it is NOT a block-map cursor.  rules/deque models a
// std::deque<T1> as `Vec<T1>` in BOTH models -- tgt_unsafe.rs t1 is
// `Vec<T1>` with an explicit TODO saying VecDeque is impossible because Ptr<>
// has no VecDeque infrastructure, and tgt_refcount.rs operates on `Ptr<T1>` /
// `Ptr<Vec<T1>>` over that same Vec.  Given the container IS a Vec in both
// models, its iterator is exactly rules/vector's iterator: a raw element
// pointer in the unsafe model (`*mut T1` / `*const T1`) and a `Ptr<T1>` in the
// refcount model.  Every body below is the shape rules/vector
// f22/f25/f26/f27/f28/f34/f43/f44/f121 and rules/list_iterator f1..f14 already
// use against the same `Vec<T1>` representation; nothing new is invented, and
// the deque's real two-level `__m_iter_` / `__ptr_` pair is deliberately NOT
// modelled because there is no deque to have blocks of.
//
// CONSEQUENCE, stated plainly: this inherits rules/deque's complexity trade and
// its ONE semantic gap -- a real std::deque iterator survives a push_front /
// push_back that does not reallocate the block map, a Vec element pointer does
// not survive any reallocation.  Nothing keyed HERE mutates the container, so no
// keyed operation can observe the difference; an iterator-taking insert()/erase()
// must not be added without revisiting this.  (rules/deque's own push_back and
// pop_front do mutate, and that gap is already rules/deque's.)
//
// begin()/end() live HERE rather than in rules/deque because they are the
// iterator-producing API and rules/deque is a separate owner; a rule module is
// data keyed by signature, so the split is free.
//
// MEASURED LIMITATION, LOUD NOT SILENT.  `operator++()` and `operator+=()`
// return `iterator &`, and the converter consumes a reference-returning result
// by DEREFERENCING it, so `iterator s = ++r;` emits
// `let s: *mut T1 = (*r.prefix_inc());` -- E0308.  rules/vector f34, rules/set
// f20 and rules/list_iterator f7 all have the IDENTICAL shape, so this is a
// pre-existing reference-return convention question, not specific to this
// module.  Every queue row here is a discarded loop increment (g212/g436/g438
// are `it++` in for-statements, g447 is a discarded `+=`), which is correct and
// byte-exact.  It is left failing loudly rather than papered over by returning
// `*mut *mut T1`, which would be a guess.
//
// NOT COVERED (no queue row): operator--, operator->, operator[], the relational
// operators < > <= >= <=>, operator-(iterator, iterator) and
// operator-(iterator, n), the free `operator+(n, iterator)`, rbegin/rend, the
// iterator-taking insert/erase, and the const_iterator <- iterator converting
// constructor.

#include <cstddef>
#include <deque>

template <typename T1> using t1 = typename std::deque<T1>::iterator;
template <typename T1> using t2 = typename std::deque<T1>::const_iterator;

// t3/t4 -- POINTER-MONOMORPHISED SIBLINGS.  These add NO model; they exist
// because t1/t2's keys cannot TEXTUALLY match an instantiation whose element
// type is itself a pointer.
//
// MEASURED, queue rows g2811/g2812, first abort on
// `L3DlOpsScheduler::getInsertionNode`:
//   system type has no rule: `std::__deque_iterator<dsc2::ScheduleNode *,
//   dsc2::ScheduleNode *const *, dsc2::ScheduleNode *const &,
//   dsc2::ScheduleNode *const *const *, long>`
// t2 records as `std::__deque_iterator<T1, const T1 *, const T1 &,
// const T1 *const *, long>`.  When T1 is a pointer, clang prints the
// const-qualified pointer with const to the RIGHT of the star
// (`dsc2::ScheduleNode *const *`, not `const dsc2::ScheduleNode **`), so
// matchTemplate -- which is purely textual -- has nothing to match t2's literal
// `, const ` segment against and the key misses.  It looks exactly like a
// MISSING key in a census; it is not.  Same defect family as rules/vector t8.
//
// The cure is the same as rules/vector t8's: state the pointer element type in
// the C++ source so clang emits the `T1 *const *` spelling directly.
//
// SWALLOW-SAFETY.  GetTypeMapKey truncates at the first `<`, so t1/t2/t3/t4 all
// share the one `std::__deque_iterator` bucket and search() picks among them by
// the longer-src tie-break (mapper.cpp:430-437).  The tie-break cannot rescue a
// SOLE candidate, so each direction is checked on the literals themselves:
//   * t3 cannot match a non-const iterator (`<X *, X **, ...>`): t3's literal
//     after the first placeholder is ` *, ` then ` *const *, `, and a non-const
//     spelling has no `const` anywhere.
//   * t3 cannot match t2's own spelling (`<X, const X *, ...>`): that one is
//     LEADING-const and contains no ` *const *, ` at argument 2.
//   * Conversely t2 cannot match t3's spelling, because t2's literal prefix
//     after the first placeholder is `, const ` -- the miss this row is about.
//     So t3 is the only candidate for the pointer-element const spelling, and it
//     does not need the tie-break to win.
//   * t4 (`<T1 *, T1 **, T1 *&, T1 ***, long>`) DOES overlap t1
//     (`<T1, T1 *, T1 &, T1 **, long>`) on a pointer-element instantiation --
//     both can match `<X *, X **, X *&, X ***, long>`.  t4's src is strictly
//     LONGER than t1's (every argument gains a star), so the longer-src
//     tie-break selects t4, and t1 keeps every non-pointer element type because
//     t4's literals demand the extra star.  Both map to the same
//     representation, so either choice is representation-identical anyway.
template <typename T1> using t3 = typename std::deque<T1 *>::const_iterator;
template <typename T1> using t4 = typename std::deque<T1 *>::iterator;

template <typename T1>
typename std::deque<T1>::iterator f1(std::deque<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::deque<T1>::iterator f2(std::deque<T1> &o) {
  return o.end();
}

template <typename T1>
typename std::deque<T1>::const_iterator f3(const std::deque<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::deque<T1>::const_iterator f4(const std::deque<T1> &o) {
  return o.end();
}

// libc++ declares == / != on __deque_iterator as hidden FRIEND functions
// (deque:392/397), so the rule body must be the UNQUALIFIED call form: a member
// form records nothing, and a qualified `std::operator==` aborts at
// cpp_rule_preprocessor.cpp:888.
template <typename T1>
bool f5(const typename std::deque<T1>::iterator &it1,
        const typename std::deque<T1>::iterator &it2) {
  return operator==(it1, it2);
}

template <typename T1>
bool f6(const typename std::deque<T1>::iterator &it1,
        const typename std::deque<T1>::iterator &it2) {
  return operator!=(it1, it2);
}

// ++ IS a member (deque:320/328), so it uses member form.
template <typename T1>
typename std::deque<T1>::iterator &f7(typename std::deque<T1>::iterator &it) {
  return it.operator++();
}

template <typename T1>
typename std::deque<T1>::iterator f8(typename std::deque<T1>::iterator a0,
                                     int a1) {
  return a0.operator++(a1);
}

template <typename T1>
typename std::deque<T1>::reference f9(typename std::deque<T1>::iterator it) {
  return it.operator*();
}

// g211 -- `operator+(difference_type) const`.  difference_type is `long` here,
// which arrives on the Rust side as i64, NOT isize.
template <typename T1>
typename std::deque<T1>::iterator f10(typename std::deque<T1>::iterator it,
                                      std::ptrdiff_t n) {
  return it.operator+(n);
}

// g447 -- `operator+=(difference_type)`, returning `iterator &`.
template <typename T1>
typename std::deque<T1>::iterator &f11(typename std::deque<T1>::iterator &it,
                                       std::ptrdiff_t n) {
  return it.operator+=(n);
}

template <typename T1>
bool f12(const typename std::deque<T1>::const_iterator &it1,
         const typename std::deque<T1>::const_iterator &it2) {
  return operator==(it1, it2);
}

template <typename T1>
bool f13(const typename std::deque<T1>::const_iterator &it1,
         const typename std::deque<T1>::const_iterator &it2) {
  return operator!=(it1, it2);
}

template <typename T1>
typename std::deque<T1>::const_iterator &
f14(typename std::deque<T1>::const_iterator &it) {
  return it.operator++();
}

template <typename T1>
typename std::deque<T1>::const_iterator
f15(typename std::deque<T1>::const_iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1>
typename std::deque<T1>::const_reference
f16(typename std::deque<T1>::const_iterator it) {
  return it.operator*();
}

template <typename T1>
typename std::deque<T1>::const_iterator
f17(typename std::deque<T1>::const_iterator it, std::ptrdiff_t n) {
  return it.operator+(n);
}

template <typename T1>
typename std::deque<T1>::const_iterator &
f18(typename std::deque<T1>::const_iterator &it, std::ptrdiff_t n) {
  return it.operator+=(n);
}
