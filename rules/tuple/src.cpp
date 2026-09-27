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

// 27-ary: the arity of DataStructDims::tie() (dsc/dims.h:283).
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27>
using t5 = std::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27>;

// g018: `operator==` on a 27-ary tuple of const-lvalue-references
// (`DataStructDims::operator==` at dsc/dims.h:283 is `tie() == A.tie()`, and
// `tie()` returns a 27-element std::tuple of `const double &` / `const std::map<...> &`).
// 27 is the ACTUAL corpus arity, counted from the depth-0 commas of the recorded
// rule key in queue/samples/g018.txt -- 21 `const double &` + 6 `const std::map<...> &`.
// Unqualified `operator==` per rules/vector f115; a qualified `std::operator==` aborts
// the rule preprocessor with "No viable function".
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27>
bool f5(const std::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27> &a0, const std::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27> &a1) {
  return operator==(a0, a1);
}
