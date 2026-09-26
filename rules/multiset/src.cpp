// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstddef>
#include <set>
#include <utility>

// std::multiset.  A DISTINCT canonical spelling from std::set (libc++ prints the
// container as std::multiset<T, std::less<T>, std::allocator<T>>), so it gets its
// own module rather than an extra rule in rules/set.
//
// MODEL: BTreeMap<T1, usize> -- element -> MULTIPLICITY.  NOT a BTreeSet and not
// rules/set's BTreeMap<T1, Box<T1>>, because a multiset ALLOWS DUPLICATES and the
// only observable this TU asks for is `==`:
//     {1,1,2} != {1,2}
// A set-shaped model answers that wrong while passing every test whose elements
// happen to be distinct.  `size()` is therefore the SUM of the multiplicities,
// not the number of map entries, and `count(k)` is the multiplicity itself.
//
// THE ITERATOR IS DELIBERATELY NOT MAPPED, and this bounds the module.
// libc++ prints std::multiset<T>::iterator and std::set<T>::iterator as the SAME
// string -- std::__tree_const_iterator<T1, std::__tree_node<T1, void *> *, long>
// -- because neither the comparator nor the multi-ness is part of the iterator
// type.  Two rules for one C++ spelling are accepted only when they map to the
// SAME Rust type (mapper.cpp:781), and rules/set already claims that string for
// UnsafeMapIterator<T1,T1> / RefcountMapIter<T1,T1>, whose container is
// BTreeMap<T1, Box<T1>> / BTreeMap<T1, Value<T1>>.  The two escapes are both
// unacceptable:
//   * map multiset's iterator to a multiplicity-aware iterator type -> different
//     Rust type for an identical src key -> the converter exits with "duplicate
//     type rule" on EVERY translation, breaking the whole tree, not just ours;
//   * reuse rules/set's iterator type exactly -> it typechecks only over
//     BTreeMap<T1, Box<T1>>, i.e. only if the container drops multiplicity, which
//     is precisely the bug this module exists to avoid.
// So insert/find/begin/end/erase(iterator)/operator* stay a LOUD unmapped abort,
// and what is mapped here is the container, ==/!=, and the multiplicity-correct
// observers.  The failing site in SdscRelayoutInsertion.cpp:47 is `==`; that
// TU's `insert` at :42 stays loud, so this closes the equality gap only.
template <typename T1> using t1 = std::multiset<T1>;

template <typename T1> std::size_t f1(const std::multiset<T1> &o) {
  return o.size();
}

template <typename T1> bool f2(const std::multiset<T1> &o) { return o.empty(); }

template <typename T1> void f3(std::multiset<T1> &o) { return o.clear(); }

// The multiplicity, not a 0/1 membership flag.
template <typename T1>
std::size_t f4(const std::multiset<T1> &o, const T1 &k) {
  return o.count(k);
}

template <typename T1> std::multiset<T1> f5() { return std::multiset<T1>(); }

template <typename T1> std::multiset<T1> f6(const std::multiset<T1> &o) {
  return std::multiset<T1>(o);
}

template <typename T1>
std::multiset<T1> &f7(std::multiset<T1> &dst, const std::multiset<T1> &src) {
  return dst.operator=(src);
}

// operator==/!= are hidden friends / free functions for the ordered containers,
// so they are spelled operator==(a, b), not a.operator==(b).
template <typename T1>
bool f8(const std::multiset<T1> &a, const std::multiset<T1> &b) {
  return operator==(a, b);
}

template <typename T1>
bool f9(const std::multiset<T1> &a, const std::multiset<T1> &b) {
  return operator!=(a, b);
}

// erase(const key_type &) returns the NUMBER OF ELEMENTS REMOVED, which for a
// multiset is the whole multiplicity of the key -- not 0/1 as for std::set.
// Mappable without naming the iterator type.
template <typename T1> std::size_t f10(std::multiset<T1> &o, const T1 &k) {
  return o.erase(k);
}

template <typename T1>
std::multiset<T1> f11(std::multiset<T1> &&o) {
  return std::multiset<T1>(std::move(o));
}

template <typename T1>
std::multiset<T1> &f12(std::multiset<T1> &dst, std::multiset<T1> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1>
void f13(std::multiset<T1> &o, std::multiset<T1> &a0) {
  return o.swap(a0);
}

// The initializer-list constructor.  It is here for a second reason besides
// coverage: with no mapped insert, this is the ONLY way a probe can build a
// multiset that HAS a duplicate, and an unprobeable multiplicity claim is not a
// claim.  Like std::set's (rules/set:148), libc++ declares it as
//     multiset(initializer_list<value_type>, const value_compare& = value_compare())
// so the resolved rule carries the comparator parameter and the converter passes
// `None` for it.
template <typename T1> std::multiset<T1> f14(std::initializer_list<T1> a0) {
  return std::multiset<T1>(a0);
}
