// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstddef>
#include <tuple>
#include <utility>

// std::tuple is modelled exactly like std::pair: a Rust tuple whose
// elements are Value<> cells (refcount) or bare values (unsafe).  The rule
// table is keyed by the resolved C++ signature, so every arity needs its own
// entries; arities 2, 3 and 4 are covered here.

template <typename T1, typename T2> using t1 = std::tuple<T1, T2>;

template <typename T1, typename T2, typename T3> using t2 = std::tuple<T1, T2, T3>;

template <typename T1, typename T2, typename T3, typename T4> using t3 = std::tuple<T1, T2, T3, T4>;

// --- 2-element tuples -------------------------------------------------------

template <typename T1, typename T2> std::tuple<T1, T2> f1() {
  return std::tuple<T1, T2>();
}

template <typename T1, typename T2, typename T3 = T1, typename T4 = T2>
std::tuple<T1, T2> f2(const T3 &a0, const T4 &a1) {
  return std::tuple<T1, T2>(a0, a1);
}

template <typename T1, typename T2, typename T3 = T1, typename T4 = T2>
std::tuple<T1, T2> f3(const T3 &a0, T4 &a1) {
  return std::tuple<T1, T2>(a0, a1);
}

template <typename T1, typename T2, typename T3 = T1, typename T4 = T2>
std::tuple<T1, T2> f4(const T3 &a0, T4 &&a1) {
  return std::tuple<T1, T2>(a0, std::move(a1));
}

template <typename T1, typename T2, typename T3 = T1, typename T4 = T2>
std::tuple<T1, T2> f5(T3 &a0, const T4 &a1) {
  return std::tuple<T1, T2>(a0, a1);
}

template <typename T1, typename T2, typename T3 = T1, typename T4 = T2>
std::tuple<T1, T2> f6(T3 &a0, T4 &a1) {
  return std::tuple<T1, T2>(a0, a1);
}

template <typename T1, typename T2, typename T3 = T1, typename T4 = T2>
std::tuple<T1, T2> f7(T3 &a0, T4 &&a1) {
  return std::tuple<T1, T2>(a0, std::move(a1));
}

template <typename T1, typename T2, typename T3 = T1, typename T4 = T2>
std::tuple<T1, T2> f8(T3 &&a0, const T4 &a1) {
  return std::tuple<T1, T2>(std::move(a0), a1);
}

template <typename T1, typename T2, typename T3 = T1, typename T4 = T2>
std::tuple<T1, T2> f9(T3 &&a0, T4 &a1) {
  return std::tuple<T1, T2>(std::move(a0), a1);
}

template <typename T1, typename T2, typename T3 = T1, typename T4 = T2>
std::tuple<T1, T2> f10(T3 &&a0, T4 &&a1) {
  return std::tuple<T1, T2>(std::move(a0), std::move(a1));
}

template <typename T1, typename T2>
std::tuple<T1, T2> f11(const std::tuple<T1, T2> &a0) {
  return std::tuple<T1, T2>(a0);
}

template <typename T1, typename T2>
std::tuple<T1, T2> f12(std::tuple<T1, T2> &&a0) {
  return std::tuple<T1, T2>(std::move(a0));
}

template <typename T1, typename T2>
std::tuple<T1, T2> &f13(std::tuple<T1, T2> &dst, const std::tuple<T1, T2> &src) {
  return dst.operator=(src);
}

template <typename T1, typename T2>
std::tuple<T1, T2> &f14(std::tuple<T1, T2> &dst, std::tuple<T1, T2> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1, typename T2> T1 &f15(std::tuple<T1, T2> &a0) {
  return std::get<0>(a0);
}

template <typename T1, typename T2> T2 &f16(std::tuple<T1, T2> &a0) {
  return std::get<1>(a0);
}

template <typename T1, typename T2>
const T1 &f17(const std::tuple<T1, T2> &a0) {
  return std::get<0>(a0);
}

template <typename T1, typename T2>
const T2 &f18(const std::tuple<T1, T2> &a0) {
  return std::get<1>(a0);
}

template <typename T1, typename T2> auto f19(const T1 &a0, const T2 &a1) {
  return std::make_tuple(a0, a1);
}

template <typename T1, typename T2> auto f20(const T1 &a0, T2 &a1) {
  return std::make_tuple(a0, a1);
}

template <typename T1, typename T2> auto f21(const T1 &a0, T2 &&a1) {
  return std::make_tuple(a0, std::move(a1));
}

template <typename T1, typename T2> auto f22(T1 &a0, const T2 &a1) {
  return std::make_tuple(a0, a1);
}

template <typename T1, typename T2> auto f23(T1 &a0, T2 &a1) {
  return std::make_tuple(a0, a1);
}

template <typename T1, typename T2> auto f24(T1 &a0, T2 &&a1) {
  return std::make_tuple(a0, std::move(a1));
}

template <typename T1, typename T2> auto f25(T1 &&a0, const T2 &a1) {
  return std::make_tuple(std::move(a0), a1);
}

template <typename T1, typename T2> auto f26(T1 &&a0, T2 &a1) {
  return std::make_tuple(std::move(a0), a1);
}

template <typename T1, typename T2> auto f27(T1 &&a0, T2 &&a1) {
  return std::make_tuple(std::move(a0), std::move(a1));
}

template <typename T1, typename T2>
bool f28(const std::tuple<T1, T2> &a, const std::tuple<T1, T2> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
bool f29(const std::tuple<T1, T2> &a, const std::tuple<T1, T2> &b) {
  return operator!=(a, b);
}

// --- 3-element tuples -------------------------------------------------------

template <typename T1, typename T2, typename T3> std::tuple<T1, T2, T3> f30() {
  return std::tuple<T1, T2, T3>();
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f31(const T4 &a0, const T5 &a1, const T6 &a2) {
  return std::tuple<T1, T2, T3>(a0, a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f32(const T4 &a0, const T5 &a1, T6 &a2) {
  return std::tuple<T1, T2, T3>(a0, a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f33(const T4 &a0, const T5 &a1, T6 &&a2) {
  return std::tuple<T1, T2, T3>(a0, a1, std::move(a2));
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f34(const T4 &a0, T5 &a1, const T6 &a2) {
  return std::tuple<T1, T2, T3>(a0, a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f35(const T4 &a0, T5 &a1, T6 &a2) {
  return std::tuple<T1, T2, T3>(a0, a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f36(const T4 &a0, T5 &a1, T6 &&a2) {
  return std::tuple<T1, T2, T3>(a0, a1, std::move(a2));
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f37(const T4 &a0, T5 &&a1, const T6 &a2) {
  return std::tuple<T1, T2, T3>(a0, std::move(a1), a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f38(const T4 &a0, T5 &&a1, T6 &a2) {
  return std::tuple<T1, T2, T3>(a0, std::move(a1), a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f39(const T4 &a0, T5 &&a1, T6 &&a2) {
  return std::tuple<T1, T2, T3>(a0, std::move(a1), std::move(a2));
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f40(T4 &a0, const T5 &a1, const T6 &a2) {
  return std::tuple<T1, T2, T3>(a0, a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f41(T4 &a0, const T5 &a1, T6 &a2) {
  return std::tuple<T1, T2, T3>(a0, a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f42(T4 &a0, const T5 &a1, T6 &&a2) {
  return std::tuple<T1, T2, T3>(a0, a1, std::move(a2));
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f43(T4 &a0, T5 &a1, const T6 &a2) {
  return std::tuple<T1, T2, T3>(a0, a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f44(T4 &a0, T5 &a1, T6 &a2) {
  return std::tuple<T1, T2, T3>(a0, a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f45(T4 &a0, T5 &a1, T6 &&a2) {
  return std::tuple<T1, T2, T3>(a0, a1, std::move(a2));
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f46(T4 &a0, T5 &&a1, const T6 &a2) {
  return std::tuple<T1, T2, T3>(a0, std::move(a1), a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f47(T4 &a0, T5 &&a1, T6 &a2) {
  return std::tuple<T1, T2, T3>(a0, std::move(a1), a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f48(T4 &a0, T5 &&a1, T6 &&a2) {
  return std::tuple<T1, T2, T3>(a0, std::move(a1), std::move(a2));
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f49(T4 &&a0, const T5 &a1, const T6 &a2) {
  return std::tuple<T1, T2, T3>(std::move(a0), a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f50(T4 &&a0, const T5 &a1, T6 &a2) {
  return std::tuple<T1, T2, T3>(std::move(a0), a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f51(T4 &&a0, const T5 &a1, T6 &&a2) {
  return std::tuple<T1, T2, T3>(std::move(a0), a1, std::move(a2));
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f52(T4 &&a0, T5 &a1, const T6 &a2) {
  return std::tuple<T1, T2, T3>(std::move(a0), a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f53(T4 &&a0, T5 &a1, T6 &a2) {
  return std::tuple<T1, T2, T3>(std::move(a0), a1, a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f54(T4 &&a0, T5 &a1, T6 &&a2) {
  return std::tuple<T1, T2, T3>(std::move(a0), a1, std::move(a2));
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f55(T4 &&a0, T5 &&a1, const T6 &a2) {
  return std::tuple<T1, T2, T3>(std::move(a0), std::move(a1), a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f56(T4 &&a0, T5 &&a1, T6 &a2) {
  return std::tuple<T1, T2, T3>(std::move(a0), std::move(a1), a2);
}

template <typename T1, typename T2, typename T3, typename T4 = T1, typename T5 = T2, typename T6 = T3>
std::tuple<T1, T2, T3> f57(T4 &&a0, T5 &&a1, T6 &&a2) {
  return std::tuple<T1, T2, T3>(std::move(a0), std::move(a1), std::move(a2));
}

template <typename T1, typename T2, typename T3>
std::tuple<T1, T2, T3> f58(const std::tuple<T1, T2, T3> &a0) {
  return std::tuple<T1, T2, T3>(a0);
}

template <typename T1, typename T2, typename T3>
std::tuple<T1, T2, T3> f59(std::tuple<T1, T2, T3> &&a0) {
  return std::tuple<T1, T2, T3>(std::move(a0));
}

template <typename T1, typename T2, typename T3>
std::tuple<T1, T2, T3> &f60(std::tuple<T1, T2, T3> &dst, const std::tuple<T1, T2, T3> &src) {
  return dst.operator=(src);
}

template <typename T1, typename T2, typename T3>
std::tuple<T1, T2, T3> &f61(std::tuple<T1, T2, T3> &dst, std::tuple<T1, T2, T3> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1, typename T2, typename T3> T1 &f62(std::tuple<T1, T2, T3> &a0) {
  return std::get<0>(a0);
}

template <typename T1, typename T2, typename T3> T2 &f63(std::tuple<T1, T2, T3> &a0) {
  return std::get<1>(a0);
}

template <typename T1, typename T2, typename T3> T3 &f64(std::tuple<T1, T2, T3> &a0) {
  return std::get<2>(a0);
}

template <typename T1, typename T2, typename T3>
const T1 &f65(const std::tuple<T1, T2, T3> &a0) {
  return std::get<0>(a0);
}

template <typename T1, typename T2, typename T3>
const T2 &f66(const std::tuple<T1, T2, T3> &a0) {
  return std::get<1>(a0);
}

template <typename T1, typename T2, typename T3>
const T3 &f67(const std::tuple<T1, T2, T3> &a0) {
  return std::get<2>(a0);
}

template <typename T1, typename T2, typename T3> auto f68(const T1 &a0, const T2 &a1, const T3 &a2) {
  return std::make_tuple(a0, a1, a2);
}

template <typename T1, typename T2, typename T3> auto f69(const T1 &a0, const T2 &a1, T3 &a2) {
  return std::make_tuple(a0, a1, a2);
}

template <typename T1, typename T2, typename T3> auto f70(const T1 &a0, const T2 &a1, T3 &&a2) {
  return std::make_tuple(a0, a1, std::move(a2));
}

template <typename T1, typename T2, typename T3> auto f71(const T1 &a0, T2 &a1, const T3 &a2) {
  return std::make_tuple(a0, a1, a2);
}

template <typename T1, typename T2, typename T3> auto f72(const T1 &a0, T2 &a1, T3 &a2) {
  return std::make_tuple(a0, a1, a2);
}

template <typename T1, typename T2, typename T3> auto f73(const T1 &a0, T2 &a1, T3 &&a2) {
  return std::make_tuple(a0, a1, std::move(a2));
}

template <typename T1, typename T2, typename T3> auto f74(const T1 &a0, T2 &&a1, const T3 &a2) {
  return std::make_tuple(a0, std::move(a1), a2);
}

template <typename T1, typename T2, typename T3> auto f75(const T1 &a0, T2 &&a1, T3 &a2) {
  return std::make_tuple(a0, std::move(a1), a2);
}

template <typename T1, typename T2, typename T3> auto f76(const T1 &a0, T2 &&a1, T3 &&a2) {
  return std::make_tuple(a0, std::move(a1), std::move(a2));
}

template <typename T1, typename T2, typename T3> auto f77(T1 &a0, const T2 &a1, const T3 &a2) {
  return std::make_tuple(a0, a1, a2);
}

template <typename T1, typename T2, typename T3> auto f78(T1 &a0, const T2 &a1, T3 &a2) {
  return std::make_tuple(a0, a1, a2);
}

template <typename T1, typename T2, typename T3> auto f79(T1 &a0, const T2 &a1, T3 &&a2) {
  return std::make_tuple(a0, a1, std::move(a2));
}

template <typename T1, typename T2, typename T3> auto f80(T1 &a0, T2 &a1, const T3 &a2) {
  return std::make_tuple(a0, a1, a2);
}

template <typename T1, typename T2, typename T3> auto f81(T1 &a0, T2 &a1, T3 &a2) {
  return std::make_tuple(a0, a1, a2);
}

template <typename T1, typename T2, typename T3> auto f82(T1 &a0, T2 &a1, T3 &&a2) {
  return std::make_tuple(a0, a1, std::move(a2));
}

template <typename T1, typename T2, typename T3> auto f83(T1 &a0, T2 &&a1, const T3 &a2) {
  return std::make_tuple(a0, std::move(a1), a2);
}

template <typename T1, typename T2, typename T3> auto f84(T1 &a0, T2 &&a1, T3 &a2) {
  return std::make_tuple(a0, std::move(a1), a2);
}

template <typename T1, typename T2, typename T3> auto f85(T1 &a0, T2 &&a1, T3 &&a2) {
  return std::make_tuple(a0, std::move(a1), std::move(a2));
}

template <typename T1, typename T2, typename T3> auto f86(T1 &&a0, const T2 &a1, const T3 &a2) {
  return std::make_tuple(std::move(a0), a1, a2);
}

template <typename T1, typename T2, typename T3> auto f87(T1 &&a0, const T2 &a1, T3 &a2) {
  return std::make_tuple(std::move(a0), a1, a2);
}

template <typename T1, typename T2, typename T3> auto f88(T1 &&a0, const T2 &a1, T3 &&a2) {
  return std::make_tuple(std::move(a0), a1, std::move(a2));
}

template <typename T1, typename T2, typename T3> auto f89(T1 &&a0, T2 &a1, const T3 &a2) {
  return std::make_tuple(std::move(a0), a1, a2);
}

template <typename T1, typename T2, typename T3> auto f90(T1 &&a0, T2 &a1, T3 &a2) {
  return std::make_tuple(std::move(a0), a1, a2);
}

template <typename T1, typename T2, typename T3> auto f91(T1 &&a0, T2 &a1, T3 &&a2) {
  return std::make_tuple(std::move(a0), a1, std::move(a2));
}

template <typename T1, typename T2, typename T3> auto f92(T1 &&a0, T2 &&a1, const T3 &a2) {
  return std::make_tuple(std::move(a0), std::move(a1), a2);
}

template <typename T1, typename T2, typename T3> auto f93(T1 &&a0, T2 &&a1, T3 &a2) {
  return std::make_tuple(std::move(a0), std::move(a1), a2);
}

template <typename T1, typename T2, typename T3> auto f94(T1 &&a0, T2 &&a1, T3 &&a2) {
  return std::make_tuple(std::move(a0), std::move(a1), std::move(a2));
}

template <typename T1, typename T2, typename T3>
bool f95(const std::tuple<T1, T2, T3> &a, const std::tuple<T1, T2, T3> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3>
bool f96(const std::tuple<T1, T2, T3> &a, const std::tuple<T1, T2, T3> &b) {
  return operator!=(a, b);
}

// --- 4-element tuples -------------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4> std::tuple<T1, T2, T3, T4> f97() {
  return std::tuple<T1, T2, T3, T4>();
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f98(const T5 &a0, const T6 &a1, const T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f99(const T5 &a0, const T6 &a1, const T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f100(const T5 &a0, const T6 &a1, const T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f101(const T5 &a0, const T6 &a1, T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f102(const T5 &a0, const T6 &a1, T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f103(const T5 &a0, const T6 &a1, T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f104(const T5 &a0, const T6 &a1, T7 &&a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f105(const T5 &a0, const T6 &a1, T7 &&a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f106(const T5 &a0, const T6 &a1, T7 &&a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f107(const T5 &a0, T6 &a1, const T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f108(const T5 &a0, T6 &a1, const T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f109(const T5 &a0, T6 &a1, const T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f110(const T5 &a0, T6 &a1, T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f111(const T5 &a0, T6 &a1, T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f112(const T5 &a0, T6 &a1, T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f113(const T5 &a0, T6 &a1, T7 &&a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f114(const T5 &a0, T6 &a1, T7 &&a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f115(const T5 &a0, T6 &a1, T7 &&a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f116(const T5 &a0, T6 &&a1, const T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f117(const T5 &a0, T6 &&a1, const T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f118(const T5 &a0, T6 &&a1, const T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f119(const T5 &a0, T6 &&a1, T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f120(const T5 &a0, T6 &&a1, T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f121(const T5 &a0, T6 &&a1, T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f122(const T5 &a0, T6 &&a1, T7 &&a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f123(const T5 &a0, T6 &&a1, T7 &&a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f124(const T5 &a0, T6 &&a1, T7 &&a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f125(T5 &a0, const T6 &a1, const T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f126(T5 &a0, const T6 &a1, const T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f127(T5 &a0, const T6 &a1, const T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f128(T5 &a0, const T6 &a1, T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f129(T5 &a0, const T6 &a1, T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f130(T5 &a0, const T6 &a1, T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f131(T5 &a0, const T6 &a1, T7 &&a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f132(T5 &a0, const T6 &a1, T7 &&a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f133(T5 &a0, const T6 &a1, T7 &&a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f134(T5 &a0, T6 &a1, const T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f135(T5 &a0, T6 &a1, const T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f136(T5 &a0, T6 &a1, const T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f137(T5 &a0, T6 &a1, T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f138(T5 &a0, T6 &a1, T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f139(T5 &a0, T6 &a1, T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f140(T5 &a0, T6 &a1, T7 &&a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f141(T5 &a0, T6 &a1, T7 &&a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f142(T5 &a0, T6 &a1, T7 &&a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, a1, std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f143(T5 &a0, T6 &&a1, const T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f144(T5 &a0, T6 &&a1, const T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f145(T5 &a0, T6 &&a1, const T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f146(T5 &a0, T6 &&a1, T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f147(T5 &a0, T6 &&a1, T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f148(T5 &a0, T6 &&a1, T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f149(T5 &a0, T6 &&a1, T7 &&a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f150(T5 &a0, T6 &&a1, T7 &&a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f151(T5 &a0, T6 &&a1, T7 &&a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(a0, std::move(a1), std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f152(T5 &&a0, const T6 &a1, const T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f153(T5 &&a0, const T6 &a1, const T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f154(T5 &&a0, const T6 &a1, const T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f155(T5 &&a0, const T6 &a1, T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f156(T5 &&a0, const T6 &a1, T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f157(T5 &&a0, const T6 &a1, T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f158(T5 &&a0, const T6 &a1, T7 &&a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f159(T5 &&a0, const T6 &a1, T7 &&a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f160(T5 &&a0, const T6 &a1, T7 &&a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f161(T5 &&a0, T6 &a1, const T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f162(T5 &&a0, T6 &a1, const T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f163(T5 &&a0, T6 &a1, const T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f164(T5 &&a0, T6 &a1, T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f165(T5 &&a0, T6 &a1, T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f166(T5 &&a0, T6 &a1, T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f167(T5 &&a0, T6 &a1, T7 &&a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f168(T5 &&a0, T6 &a1, T7 &&a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f169(T5 &&a0, T6 &a1, T7 &&a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), a1, std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f170(T5 &&a0, T6 &&a1, const T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f171(T5 &&a0, T6 &&a1, const T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f172(T5 &&a0, T6 &&a1, const T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), std::move(a1), a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f173(T5 &&a0, T6 &&a1, T7 &a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f174(T5 &&a0, T6 &&a1, T7 &a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f175(T5 &&a0, T6 &&a1, T7 &a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), std::move(a1), a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f176(T5 &&a0, T6 &&a1, T7 &&a2, const T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), std::move(a1), std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f177(T5 &&a0, T6 &&a1, T7 &&a2, T8 &a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), std::move(a1), std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5 = T1, typename T6 = T2, typename T7 = T3, typename T8 = T4>
std::tuple<T1, T2, T3, T4> f178(T5 &&a0, T6 &&a1, T7 &&a2, T8 &&a3) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0), std::move(a1), std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4>
std::tuple<T1, T2, T3, T4> f179(const std::tuple<T1, T2, T3, T4> &a0) {
  return std::tuple<T1, T2, T3, T4>(a0);
}

template <typename T1, typename T2, typename T3, typename T4>
std::tuple<T1, T2, T3, T4> f180(std::tuple<T1, T2, T3, T4> &&a0) {
  return std::tuple<T1, T2, T3, T4>(std::move(a0));
}

template <typename T1, typename T2, typename T3, typename T4>
std::tuple<T1, T2, T3, T4> &f181(std::tuple<T1, T2, T3, T4> &dst, const std::tuple<T1, T2, T3, T4> &src) {
  return dst.operator=(src);
}

template <typename T1, typename T2, typename T3, typename T4>
std::tuple<T1, T2, T3, T4> &f182(std::tuple<T1, T2, T3, T4> &dst, std::tuple<T1, T2, T3, T4> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1, typename T2, typename T3, typename T4> T1 &f183(std::tuple<T1, T2, T3, T4> &a0) {
  return std::get<0>(a0);
}

template <typename T1, typename T2, typename T3, typename T4> T2 &f184(std::tuple<T1, T2, T3, T4> &a0) {
  return std::get<1>(a0);
}

template <typename T1, typename T2, typename T3, typename T4> T3 &f185(std::tuple<T1, T2, T3, T4> &a0) {
  return std::get<2>(a0);
}

template <typename T1, typename T2, typename T3, typename T4> T4 &f186(std::tuple<T1, T2, T3, T4> &a0) {
  return std::get<3>(a0);
}

template <typename T1, typename T2, typename T3, typename T4>
const T1 &f187(const std::tuple<T1, T2, T3, T4> &a0) {
  return std::get<0>(a0);
}

template <typename T1, typename T2, typename T3, typename T4>
const T2 &f188(const std::tuple<T1, T2, T3, T4> &a0) {
  return std::get<1>(a0);
}

template <typename T1, typename T2, typename T3, typename T4>
const T3 &f189(const std::tuple<T1, T2, T3, T4> &a0) {
  return std::get<2>(a0);
}

template <typename T1, typename T2, typename T3, typename T4>
const T4 &f190(const std::tuple<T1, T2, T3, T4> &a0) {
  return std::get<3>(a0);
}

template <typename T1, typename T2, typename T3, typename T4> auto f191(const T1 &a0, const T2 &a1, const T3 &a2, const T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f192(const T1 &a0, const T2 &a1, const T3 &a2, T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f193(const T1 &a0, const T2 &a1, const T3 &a2, T4 &&a3) {
  return std::make_tuple(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f194(const T1 &a0, const T2 &a1, T3 &a2, const T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f195(const T1 &a0, const T2 &a1, T3 &a2, T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f196(const T1 &a0, const T2 &a1, T3 &a2, T4 &&a3) {
  return std::make_tuple(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f197(const T1 &a0, const T2 &a1, T3 &&a2, const T4 &a3) {
  return std::make_tuple(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f198(const T1 &a0, const T2 &a1, T3 &&a2, T4 &a3) {
  return std::make_tuple(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f199(const T1 &a0, const T2 &a1, T3 &&a2, T4 &&a3) {
  return std::make_tuple(a0, a1, std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f200(const T1 &a0, T2 &a1, const T3 &a2, const T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f201(const T1 &a0, T2 &a1, const T3 &a2, T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f202(const T1 &a0, T2 &a1, const T3 &a2, T4 &&a3) {
  return std::make_tuple(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f203(const T1 &a0, T2 &a1, T3 &a2, const T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f204(const T1 &a0, T2 &a1, T3 &a2, T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f205(const T1 &a0, T2 &a1, T3 &a2, T4 &&a3) {
  return std::make_tuple(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f206(const T1 &a0, T2 &a1, T3 &&a2, const T4 &a3) {
  return std::make_tuple(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f207(const T1 &a0, T2 &a1, T3 &&a2, T4 &a3) {
  return std::make_tuple(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f208(const T1 &a0, T2 &a1, T3 &&a2, T4 &&a3) {
  return std::make_tuple(a0, a1, std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f209(const T1 &a0, T2 &&a1, const T3 &a2, const T4 &a3) {
  return std::make_tuple(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f210(const T1 &a0, T2 &&a1, const T3 &a2, T4 &a3) {
  return std::make_tuple(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f211(const T1 &a0, T2 &&a1, const T3 &a2, T4 &&a3) {
  return std::make_tuple(a0, std::move(a1), a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f212(const T1 &a0, T2 &&a1, T3 &a2, const T4 &a3) {
  return std::make_tuple(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f213(const T1 &a0, T2 &&a1, T3 &a2, T4 &a3) {
  return std::make_tuple(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f214(const T1 &a0, T2 &&a1, T3 &a2, T4 &&a3) {
  return std::make_tuple(a0, std::move(a1), a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f215(const T1 &a0, T2 &&a1, T3 &&a2, const T4 &a3) {
  return std::make_tuple(a0, std::move(a1), std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f216(const T1 &a0, T2 &&a1, T3 &&a2, T4 &a3) {
  return std::make_tuple(a0, std::move(a1), std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f217(const T1 &a0, T2 &&a1, T3 &&a2, T4 &&a3) {
  return std::make_tuple(a0, std::move(a1), std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f218(T1 &a0, const T2 &a1, const T3 &a2, const T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f219(T1 &a0, const T2 &a1, const T3 &a2, T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f220(T1 &a0, const T2 &a1, const T3 &a2, T4 &&a3) {
  return std::make_tuple(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f221(T1 &a0, const T2 &a1, T3 &a2, const T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f222(T1 &a0, const T2 &a1, T3 &a2, T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f223(T1 &a0, const T2 &a1, T3 &a2, T4 &&a3) {
  return std::make_tuple(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f224(T1 &a0, const T2 &a1, T3 &&a2, const T4 &a3) {
  return std::make_tuple(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f225(T1 &a0, const T2 &a1, T3 &&a2, T4 &a3) {
  return std::make_tuple(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f226(T1 &a0, const T2 &a1, T3 &&a2, T4 &&a3) {
  return std::make_tuple(a0, a1, std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f227(T1 &a0, T2 &a1, const T3 &a2, const T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f228(T1 &a0, T2 &a1, const T3 &a2, T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f229(T1 &a0, T2 &a1, const T3 &a2, T4 &&a3) {
  return std::make_tuple(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f230(T1 &a0, T2 &a1, T3 &a2, const T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f231(T1 &a0, T2 &a1, T3 &a2, T4 &a3) {
  return std::make_tuple(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f232(T1 &a0, T2 &a1, T3 &a2, T4 &&a3) {
  return std::make_tuple(a0, a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f233(T1 &a0, T2 &a1, T3 &&a2, const T4 &a3) {
  return std::make_tuple(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f234(T1 &a0, T2 &a1, T3 &&a2, T4 &a3) {
  return std::make_tuple(a0, a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f235(T1 &a0, T2 &a1, T3 &&a2, T4 &&a3) {
  return std::make_tuple(a0, a1, std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f236(T1 &a0, T2 &&a1, const T3 &a2, const T4 &a3) {
  return std::make_tuple(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f237(T1 &a0, T2 &&a1, const T3 &a2, T4 &a3) {
  return std::make_tuple(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f238(T1 &a0, T2 &&a1, const T3 &a2, T4 &&a3) {
  return std::make_tuple(a0, std::move(a1), a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f239(T1 &a0, T2 &&a1, T3 &a2, const T4 &a3) {
  return std::make_tuple(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f240(T1 &a0, T2 &&a1, T3 &a2, T4 &a3) {
  return std::make_tuple(a0, std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f241(T1 &a0, T2 &&a1, T3 &a2, T4 &&a3) {
  return std::make_tuple(a0, std::move(a1), a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f242(T1 &a0, T2 &&a1, T3 &&a2, const T4 &a3) {
  return std::make_tuple(a0, std::move(a1), std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f243(T1 &a0, T2 &&a1, T3 &&a2, T4 &a3) {
  return std::make_tuple(a0, std::move(a1), std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f244(T1 &a0, T2 &&a1, T3 &&a2, T4 &&a3) {
  return std::make_tuple(a0, std::move(a1), std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f245(T1 &&a0, const T2 &a1, const T3 &a2, const T4 &a3) {
  return std::make_tuple(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f246(T1 &&a0, const T2 &a1, const T3 &a2, T4 &a3) {
  return std::make_tuple(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f247(T1 &&a0, const T2 &a1, const T3 &a2, T4 &&a3) {
  return std::make_tuple(std::move(a0), a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f248(T1 &&a0, const T2 &a1, T3 &a2, const T4 &a3) {
  return std::make_tuple(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f249(T1 &&a0, const T2 &a1, T3 &a2, T4 &a3) {
  return std::make_tuple(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f250(T1 &&a0, const T2 &a1, T3 &a2, T4 &&a3) {
  return std::make_tuple(std::move(a0), a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f251(T1 &&a0, const T2 &a1, T3 &&a2, const T4 &a3) {
  return std::make_tuple(std::move(a0), a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f252(T1 &&a0, const T2 &a1, T3 &&a2, T4 &a3) {
  return std::make_tuple(std::move(a0), a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f253(T1 &&a0, const T2 &a1, T3 &&a2, T4 &&a3) {
  return std::make_tuple(std::move(a0), a1, std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f254(T1 &&a0, T2 &a1, const T3 &a2, const T4 &a3) {
  return std::make_tuple(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f255(T1 &&a0, T2 &a1, const T3 &a2, T4 &a3) {
  return std::make_tuple(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f256(T1 &&a0, T2 &a1, const T3 &a2, T4 &&a3) {
  return std::make_tuple(std::move(a0), a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f257(T1 &&a0, T2 &a1, T3 &a2, const T4 &a3) {
  return std::make_tuple(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f258(T1 &&a0, T2 &a1, T3 &a2, T4 &a3) {
  return std::make_tuple(std::move(a0), a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f259(T1 &&a0, T2 &a1, T3 &a2, T4 &&a3) {
  return std::make_tuple(std::move(a0), a1, a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f260(T1 &&a0, T2 &a1, T3 &&a2, const T4 &a3) {
  return std::make_tuple(std::move(a0), a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f261(T1 &&a0, T2 &a1, T3 &&a2, T4 &a3) {
  return std::make_tuple(std::move(a0), a1, std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f262(T1 &&a0, T2 &a1, T3 &&a2, T4 &&a3) {
  return std::make_tuple(std::move(a0), a1, std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f263(T1 &&a0, T2 &&a1, const T3 &a2, const T4 &a3) {
  return std::make_tuple(std::move(a0), std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f264(T1 &&a0, T2 &&a1, const T3 &a2, T4 &a3) {
  return std::make_tuple(std::move(a0), std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f265(T1 &&a0, T2 &&a1, const T3 &a2, T4 &&a3) {
  return std::make_tuple(std::move(a0), std::move(a1), a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f266(T1 &&a0, T2 &&a1, T3 &a2, const T4 &a3) {
  return std::make_tuple(std::move(a0), std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f267(T1 &&a0, T2 &&a1, T3 &a2, T4 &a3) {
  return std::make_tuple(std::move(a0), std::move(a1), a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f268(T1 &&a0, T2 &&a1, T3 &a2, T4 &&a3) {
  return std::make_tuple(std::move(a0), std::move(a1), a2, std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4> auto f269(T1 &&a0, T2 &&a1, T3 &&a2, const T4 &a3) {
  return std::make_tuple(std::move(a0), std::move(a1), std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f270(T1 &&a0, T2 &&a1, T3 &&a2, T4 &a3) {
  return std::make_tuple(std::move(a0), std::move(a1), std::move(a2), a3);
}

template <typename T1, typename T2, typename T3, typename T4> auto f271(T1 &&a0, T2 &&a1, T3 &&a2, T4 &&a3) {
  return std::make_tuple(std::move(a0), std::move(a1), std::move(a2), std::move(a3));
}

template <typename T1, typename T2, typename T3, typename T4>
bool f272(const std::tuple<T1, T2, T3, T4> &a, const std::tuple<T1, T2, T3, T4> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4>
bool f273(const std::tuple<T1, T2, T3, T4> &a, const std::tuple<T1, T2, T3, T4> &b) {
  return operator!=(a, b);
}

// --- std::tie, 2 elements ---------------------------------------------------

template <typename T1, typename T2> using t4 = std::tuple<T1 &, T2 &>;

template <typename T1, typename T2>
std::tuple<T1 &, T2 &> f274(T1 &a0, T2 &a1) {
  return std::tie(a0, a1);
}

template <typename T1, typename T2>
std::tuple<T1 &, T2 &> &f275(std::tuple<T1 &, T2 &> &dst, const std::tuple<T1, T2> &src) {
  return dst.operator=(src);
}

template <typename T1, typename T2> T1 &f276(std::tuple<T1 &, T2 &> &a0) {
  return std::get<0>(a0);
}

template <typename T1, typename T2> T2 &f277(std::tuple<T1 &, T2 &> &a0) {
  return std::get<1>(a0);
}

// --- std::tie, 3 elements ---------------------------------------------------

template <typename T1, typename T2, typename T3> using t5 = std::tuple<T1 &, T2 &, T3 &>;

template <typename T1, typename T2, typename T3>
std::tuple<T1 &, T2 &, T3 &> f278(T1 &a0, T2 &a1, T3 &a2) {
  return std::tie(a0, a1, a2);
}

template <typename T1, typename T2, typename T3>
std::tuple<T1 &, T2 &, T3 &> &f279(std::tuple<T1 &, T2 &, T3 &> &dst, const std::tuple<T1, T2, T3> &src) {
  return dst.operator=(src);
}

template <typename T1, typename T2, typename T3> T1 &f280(std::tuple<T1 &, T2 &, T3 &> &a0) {
  return std::get<0>(a0);
}

template <typename T1, typename T2, typename T3> T2 &f281(std::tuple<T1 &, T2 &, T3 &> &a0) {
  return std::get<1>(a0);
}

template <typename T1, typename T2, typename T3> T3 &f282(std::tuple<T1 &, T2 &, T3 &> &a0) {
  return std::get<2>(a0);
}

// --- std::tie, 4 elements ---------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4> using t6 = std::tuple<T1 &, T2 &, T3 &, T4 &>;

template <typename T1, typename T2, typename T3, typename T4>
std::tuple<T1 &, T2 &, T3 &, T4 &> f283(T1 &a0, T2 &a1, T3 &a2, T4 &a3) {
  return std::tie(a0, a1, a2, a3);
}

template <typename T1, typename T2, typename T3, typename T4>
std::tuple<T1 &, T2 &, T3 &, T4 &> &f284(std::tuple<T1 &, T2 &, T3 &, T4 &> &dst, const std::tuple<T1, T2, T3, T4> &src) {
  return dst.operator=(src);
}

template <typename T1, typename T2, typename T3, typename T4> T1 &f285(std::tuple<T1 &, T2 &, T3 &, T4 &> &a0) {
  return std::get<0>(a0);
}

template <typename T1, typename T2, typename T3, typename T4> T2 &f286(std::tuple<T1 &, T2 &, T3 &, T4 &> &a0) {
  return std::get<1>(a0);
}

template <typename T1, typename T2, typename T3, typename T4> T3 &f287(std::tuple<T1 &, T2 &, T3 &, T4 &> &a0) {
  return std::get<2>(a0);
}

template <typename T1, typename T2, typename T3, typename T4> T4 &f288(std::tuple<T1 &, T2 &, T3 &, T4 &> &a0) {
  return std::get<3>(a0);
}

