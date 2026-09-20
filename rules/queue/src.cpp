// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::queue<T>.
//
// Model: the same Vec<T1> that rules/deque uses, with push() appending and
// pop()/front() working the front.  std::queue is a container ADAPTOR over
// std::deque, so sharing the representation is not just convenient -- it is
// what keeps `std::queue<T>` and its underlying `std::deque<T>` the same Rust
// type if a TU ever names both.
//
// Only the members the scheduler actually uses are here: dcgbeCodegen.cpp and
// dcgbe_elements.h (std::queue<InstrInfo>), perfmodel.cpp
// (std::queue<PerfPerComponent *>) and SdscTree.cpp (std::queue<SdscTreeNode *>)
// between them call queue(), push() (both the const-lvalue and the rvalue
// overload), pop(), front(), empty() and size() and nothing else.  back(),
// emplace() and swap() are deliberately absent rather than guessed at.
//
// As with std::deque, the allocator/container-explicit spelling needs its own
// alias: the default argument is not suppressed where the type is looked up,
// so `std::queue<T, std::deque<T>>` reaches the mapper spelled out in full and
// would otherwise have no rule at all.  That is rules/deque's t6 trap.

#include <queue>

template <typename T1> using t1 = std::queue<T1>;

template <typename T1, typename T2> using t6 = std::queue<T1, T2>;

template <typename T1> std::queue<T1> f1() { return std::queue<T1>(); }

template <typename T1> void f2(std::queue<T1> &o, const T1 &value) {
  return o.push(value);
}

// The rvalue overload.  `q.push(3)` and `q.push(makeNode())` bind to
// push(T1 &&), not to push(const T1 &), so without this a literal push falls
// back to the mangled `push_pmuti32_rv` placeholder.
template <typename T1> void f8(std::queue<T1> &o, T1 &&value) {
  return o.push(std::move(value));
}

template <typename T1> void f3(std::queue<T1> &o) { return o.pop(); }

template <typename T1> T1 &f4(std::queue<T1> &o) { return o.front(); }

template <typename T1> const T1 &f5(const std::queue<T1> &o) {
  return o.front();
}

template <typename T1> bool f6(const std::queue<T1> &o) { return o.empty(); }

template <typename T1> std::size_t f7(const std::queue<T1> &o) {
  return o.size();
}
