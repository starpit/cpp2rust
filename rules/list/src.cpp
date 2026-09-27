// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::list -- a doubly-linked sequence container.
//
// The allocator template argument is DEFAULTED and SuppressDefaultTemplateArgs
// elides it from the rule key, so the searched key is ONE-argument:
// `std::list<T1>`.  Verified against queue/samples/g024.txt, which prints
//   searched as: std::list<DsTrackInMem::Entry>
//   from decl (NOT a key): std::list<DsTrackInMem::Entry,
//                                    std::allocator<DsTrackInMem::Entry>>
//
// MODEL: Vec<T1>, exactly as rules/deque and rules/queue do -- Ptr<> has no
// linked-list or VecDeque infrastructure, and every operation keyed here has a
// Vec equivalent with IDENTICAL observable semantics.  The cost is complexity,
// not behaviour: push_front/pop_front are O(n) on a Vec and O(1) on a list.
// That is a deliberate, documented trade, not a correctness gap.
//
// THE ITERATOR IS DELIBERATELY NOT MODELLED HERE.  std::list's iterator is a
// STABLE CURSOR into a node chain; Rust's LinkedList has no stable-cursor public
// API and a Vec index is not equivalent (an insert invalidates it in a way a
// list iterator survives).  Inventing one would be silently wrong, so
// std::__list_iterator is left LOUD.  The split is already visible in the work
// queue: rows g023/g594/g1296 are `std::__list_iterator<X, void *>` and are
// owned by rules/list_iterator, not by this module.  Keying the container alone
// is what unblocks the unit-free operations.
//
// NOT COVERED, each because it needs its own harvested key and/or a cursor:
// begin/end/rbegin/rend and everything taking an iterator (insert, erase,
// splice), emplace_back/emplace_front, resize, remove/remove_if, sort, unique,
// reverse, merge, assign, the sized/filled/range/initializer_list constructors,
// operator== / operator< , and swap.

#include <cstddef>
#include <list>
#include <utility>

template <typename T1> using t1 = std::list<T1>;

template <typename T1> std::list<T1> f1() { return std::list<T1>(); }

template <typename T1> std::size_t f2(const std::list<T1> &o) {
  return o.size();
}

template <typename T1> bool f3(const std::list<T1> &o) { return o.empty(); }

template <typename T1> void f4(std::list<T1> &o) { return o.clear(); }

// BOTH push_back overloads are required.  A literal argument
// (`l.push_back(33)`) binds `void push_back(T1 &&)`, not `const T1 &`, and a
// module carrying only the const-ref form loses that row and emits the mangled
// fallback instead -- measured in rules/set.  Same for push_front.
template <typename T1> void f5(std::list<T1> &o, const T1 &value) {
  return o.push_back(value);
}

template <typename T1> void f6(std::list<T1> &o, T1 &&value) {
  return o.push_back(std::move(value));
}

template <typename T1> void f7(std::list<T1> &o, const T1 &value) {
  return o.push_front(value);
}

template <typename T1> void f8(std::list<T1> &o, T1 &&value) {
  return o.push_front(std::move(value));
}

template <typename T1> void f9(std::list<T1> &o) { return o.pop_back(); }

template <typename T1> void f10(std::list<T1> &o) { return o.pop_front(); }

template <typename T1> T1 &f11(std::list<T1> &o) { return o.front(); }

template <typename T1> T1 &f12(std::list<T1> &o) { return o.back(); }

template <typename T1> const T1 &f13(const std::list<T1> &o) {
  return o.front();
}

template <typename T1> const T1 &f14(const std::list<T1> &o) {
  return o.back();
}

template <typename T1> std::list<T1> f15(const std::list<T1> &o) {
  return std::list<T1>(o);
}

template <typename T1> std::list<T1> f16(std::list<T1> &&o) {
  return std::list<T1>(std::move(o));
}

template <typename T1>
std::list<T1> &f17(std::list<T1> &dst, const std::list<T1> &src) {
  return dst.operator=(src);
}

template <typename T1>
std::list<T1> &f18(std::list<T1> &dst, std::list<T1> &&src) {
  return dst.operator=(std::move(src));
}
