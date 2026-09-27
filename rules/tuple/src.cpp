// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <tuple>

template <typename T1, typename T2> using t1 = std::tuple<T1, T2>;

template <typename T1, typename T2, typename T3>
using t2 = std::tuple<T1, T2, T3>;

template <typename T1, typename T2, typename T3, typename T4>
using t3 = std::tuple<T1, T2, T3, T4>;

template <typename T1, typename T2, typename T3, typename T4, typename T5>
using t4 = std::tuple<T1, T2, T3, T4, T5>;

// CONSTRUCTOR probe: the converter reports the ctor key as
// `void std::tuple<int, std::string, double>::tuple(&&...)`, i.e. an
// unexpanded-pack parameter list.  Read the key the preprocessor actually
// records for this body before believing a rule can express it.
template <typename T1, typename T2>
std::tuple<T1, T2> f1(T1 a0, T2 a1) {
  return std::tuple<T1, T2>(a0, a1);
}

template <typename T1, typename T2, typename T3>
std::tuple<T1, T2, T3> f2(T1 a0, T2 a1, T3 a2) {
  return std::tuple<T1, T2, T3>(a0, a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4>
std::tuple<T1, T2, T3, T4> f3(T1 a0, T2 a1, T3 a2, T4 a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::tuple<T1, T2, T3, T4, T5> f4(T1 a0, T2 a1, T3 a2, T4 a3, T5 a4) {
  return std::tuple<T1, T2, T3, T4, T5>(a0, a1, a2, a3, a4);
}
