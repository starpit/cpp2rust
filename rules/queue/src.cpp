// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::queue is a FIFO adaptor.  The container template argument is DEFAULTED
// (std::deque) and SuppressDefaultTemplateArgs elides it from the rule key, so
// the key the converter searches for is one-argument: `std::queue<T1>`.
// Verified against queue/samples/g369.txt ("searched as:
// std::queue<std::pair<mlir::Operation *, std::optional<int>>>").
//
// std::priority_queue is deliberately NOT keyed here: every instantiation
// reached in dt_src (g1989) carries a CUSTOM comparator, see the module note in
// the report.

#include <queue>

template <typename T1> using t1 = std::queue<T1>;

template <typename T1> std::queue<T1> f1() { return std::queue<T1>(); }

template <typename T1> bool f2(const std::queue<T1> &o) { return o.empty(); }

template <typename T1> std::size_t f3(const std::queue<T1> &o) {
  return o.size();
}

template <typename T1> void f4(std::queue<T1> &o, const T1 &value) {
  return o.push(value);
}

template <typename T1> void f5(std::queue<T1> &o, T1 &&value) {
  return o.push(std::move(value));
}

template <typename T1> T1 &f6(std::queue<T1> &o) { return o.front(); }

template <typename T1> T1 &f7(std::queue<T1> &o) { return o.back(); }

template <typename T1> void f8(std::queue<T1> &o) { return o.pop(); }
