// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::stack is a LIFO adaptor.  The container template argument is DEFAULTED
// (std::deque) and SuppressDefaultTemplateArgs elides it from the rule key, so
// the key the converter searches for is one-argument: `std::stack<T1>`.
// Verified against queue/samples/g2920.txt, whose row reads
//   searched as: std::stack<long>
//   from decl (NOT a key -- canonicalised, defaulted args kept):
//     std::stack<long, std::deque<long, std::allocator<long>>>
// so a key written to the two-argument decl spelling would be DEAD.
//
// WHY THIS IS NOT A DUPLICATE OF rules/queue.  `std::stack` and `std::queue`
// are DISTINCT types with distinct keys, so nothing here shadows rules/queue's
// `std::queue<T1>`.  (Contrast the refused `rules/function`, which would have
// keyed the very types `rules/functional` already keys.)  This module is a
// deliberate transliteration of rules/queue with one member renamed and one
// member's ORDER inverted; see the next paragraph, which is the whole point.
//
// ⚠️ THE TWO WAYS A STACK MODEL IS SILENTLY WRONG, both handled below:
//  1. ORDER.  A stack is LIFO.  A model shaped like rules/queue's -- push at
//     the back, take from the FRONT -- compiles, runs, and returns the
//     elements in EXACTLY REVERSE order.  Here push() appends and top()/pop()
//     take the LAST element, so the order is LIFO.  The probe asserts the
//     order, because that is the only falsifiable part of this module.
//  2. ARITY OF REMOVAL.  `std::stack::pop()` returns VOID and `std::stack::
//     top()` returns a REFERENCE without removing.  Rust's `Vec::pop()`
//     returns `Option<T>` AND REMOVES.  Conflating them removes one element
//     too many (`x = s.top(); s.pop();` would pop twice).  f6 (top) therefore
//     does NOT remove and f7 (pop) discards the value.
//
// NOT KEYED: the container-aware constructors (`stack(const Container &)`,
// `stack(Container &&)`), `emplace`, `swap` and the relational operators.  No
// row reached them; leaving them out makes them fail loudly.

#include <stack>

template <typename T1> using t1 = std::stack<T1>;

template <typename T1> std::stack<T1> f1() { return std::stack<T1>(); }

template <typename T1> bool f2(const std::stack<T1> &o) { return o.empty(); }

template <typename T1> std::size_t f3(const std::stack<T1> &o) {
  return o.size();
}

template <typename T1> void f4(std::stack<T1> &o, const T1 &value) {
  return o.push(value);
}

template <typename T1> void f5(std::stack<T1> &o, T1 &&value) {
  return o.push(std::move(value));
}

// top() -- the LAST element pushed, and it does NOT remove.
template <typename T1> T1 &f6(std::stack<T1> &o) { return o.top(); }

// pop() -- removes the LAST element pushed and returns VOID.
template <typename T1> void f7(std::stack<T1> &o) { return o.pop(); }
