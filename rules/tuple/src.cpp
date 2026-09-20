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

// ==== high-arity extension (generated by /tmp/gen_tuple_rules_highN.py) ====

// Arities 2-4 above cover the full rule set.  Everything below is the
// narrow high-arity slice: the tuple-of-references type map, std::tie
// itself, and the comparison operators on those tuples.  Constructors,
// make_tuple, assignment and get<I> are intentionally absent -- the
// ctor/make_tuple rule set is a 3^N cross product, and get<I> is ambiguous
// whenever two element types print the same (a tie over 21 doubles is the
// worst case), which the matcher now refuses rather than guesses.
//
// ARITY STOPS AT 9.  The matching engine has exactly nine generic slots:
// kMaxGenerics == 9 (converter/translation_rule.h:18); ExprRule::validate
// (converter/translation_rule.cpp:324-341) scans only single-digit T1..T9
// and report_fatal_error()s on anything past T9 at rule LOAD time; and
// instantiateTgt (converter/mapper.cpp:431-432) substitutes a fixed two
// characters per slot, so "T10" would expand to <T1's binding> + "0".
// See docs/src/rules/ir.md:118-119.  A 27-element std::tie (dsc/dims.h:220)
// is therefore not expressible here without an engine change.
//
// The operator==/operator!= entries for arities 2-4 are NEW, not a rewrite:
// f28/f95/f272 compare tuples of VALUES, and nothing compared a tuple of
// REFERENCES, so `a.tie() == b.tie()` fell through to a native Rust `==` on
// a tuple of Ptr<T>, i.e. pointer identity -- equal structs compared
// unequal, silently.

// --- std::tie comparison, 2 elements ----------------------------------------

template <typename T1, typename T2>
bool f289(const std::tuple<T1 &, T2 &> &a, const std::tuple<T1 &, T2 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
bool f290(const std::tuple<T1 &, T2 &> &a, const std::tuple<T1 &, T2 &> &b) {
  return operator!=(a, b);
}

// --- std::tie comparison, 3 elements ----------------------------------------

template <typename T1, typename T2, typename T3>
bool f291(const std::tuple<T1 &, T2 &, T3 &> &a, const std::tuple<T1 &, T2 &, T3 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3>
bool f292(const std::tuple<T1 &, T2 &, T3 &> &a, const std::tuple<T1 &, T2 &, T3 &> &b) {
  return operator!=(a, b);
}

// --- std::tie comparison, 4 elements ----------------------------------------

template <typename T1, typename T2, typename T3, typename T4>
bool f293(const std::tuple<T1 &, T2 &, T3 &, T4 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4>
bool f294(const std::tuple<T1 &, T2 &, T3 &, T4 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 5 elements ---------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5>
using t7 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &> f295(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4) {
  return std::tie(a0, a1, a2, a3, a4);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
bool f296(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
bool f297(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 6 elements ---------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
using t8 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &> f298(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5) {
  return std::tie(a0, a1, a2, a3, a4, a5);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
bool f299(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
bool f300(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 7 elements ---------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
using t9 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &> f301(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
bool f302(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
bool f303(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 8 elements ---------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
using t10 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &> f304(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
bool f305(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
bool f306(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 9 elements ---------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
using t11 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &> f307(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
bool f308(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
bool f309(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 10 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
using t12 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &> f310(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
bool f311(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
bool f312(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 11 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
using t13 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &> f313(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
bool f314(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
bool f315(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 12 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
using t14 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &> f316(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
bool f317(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
bool f318(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 13 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
using t15 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &> f319(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
bool f320(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
bool f321(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 14 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14>
using t16 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &> f322(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14>
bool f323(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14>
bool f324(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 15 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15>
using t17 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &> f325(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15>
bool f326(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15>
bool f327(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 16 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16>
using t18 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &> f328(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16>
bool f329(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16>
bool f330(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 17 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17>
using t19 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &> f331(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17>
bool f332(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17>
bool f333(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 18 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18>
using t20 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &> f334(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16, T18 &a17) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18>
bool f335(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18>
bool f336(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 19 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19>
using t21 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &> f337(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16, T18 &a17, T19 &a18) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19>
bool f338(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19>
bool f339(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 20 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20>
using t22 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &> f340(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16, T18 &a17, T19 &a18, T20 &a19) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20>
bool f341(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20>
bool f342(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 21 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21>
using t23 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &> f343(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16, T18 &a17, T19 &a18, T20 &a19, T21 &a20) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21>
bool f344(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21>
bool f345(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 22 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22>
using t24 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &> f346(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16, T18 &a17, T19 &a18, T20 &a19, T21 &a20, T22 &a21) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22>
bool f347(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22>
bool f348(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 23 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23>
using t25 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &> f349(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16, T18 &a17, T19 &a18, T20 &a19, T21 &a20, T22 &a21, T23 &a22) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23>
bool f350(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23>
bool f351(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 24 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24>
using t26 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &> f352(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16, T18 &a17, T19 &a18, T20 &a19, T21 &a20, T22 &a21, T23 &a22, T24 &a23) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24>
bool f353(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24>
bool f354(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 25 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25>
using t27 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &> f355(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16, T18 &a17, T19 &a18, T20 &a19, T21 &a20, T22 &a21, T23 &a22, T24 &a23, T25 &a24) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25>
bool f356(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25>
bool f357(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 26 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26>
using t28 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &> f358(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16, T18 &a17, T19 &a18, T20 &a19, T21 &a20, T22 &a21, T23 &a22, T24 &a23, T25 &a24, T26 &a25) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26>
bool f359(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26>
bool f360(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 27 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27>
using t29 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &> f361(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16, T18 &a17, T19 &a18, T20 &a19, T21 &a20, T22 &a21, T23 &a22, T24 &a23, T25 &a24, T26 &a25, T27 &a26) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27>
bool f362(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27>
bool f363(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 28 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28>
using t30 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &> f364(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16, T18 &a17, T19 &a18, T20 &a19, T21 &a20, T22 &a21, T23 &a22, T24 &a23, T25 &a24, T26 &a25, T27 &a26, T28 &a27) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28>
bool f365(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28>
bool f366(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 29 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29>
using t31 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &> f367(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16, T18 &a17, T19 &a18, T20 &a19, T21 &a20, T22 &a21, T23 &a22, T24 &a23, T25 &a24, T26 &a25, T27 &a26, T28 &a27, T29 &a28) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29>
bool f368(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29>
bool f369(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 30 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30>
using t32 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &> f370(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16, T18 &a17, T19 &a18, T20 &a19, T21 &a20, T22 &a21, T23 &a22, T24 &a23, T25 &a24, T26 &a25, T27 &a26, T28 &a27, T29 &a28, T30 &a29) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30>
bool f371(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30>
bool f372(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 31 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30, typename T31>
using t33 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &, T31 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30, typename T31>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &, T31 &> f373(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16, T18 &a17, T19 &a18, T20 &a19, T21 &a20, T22 &a21, T23 &a22, T24 &a23, T25 &a24, T26 &a25, T27 &a26, T28 &a27, T29 &a28, T30 &a29, T31 &a30) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30, typename T31>
bool f374(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &, T31 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &, T31 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30, typename T31>
bool f375(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &, T31 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &, T31 &> &b) {
  return operator!=(a, b);
}

// --- std::tie, 32 elements --------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30, typename T31, typename T32>
using t34 = std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &, T31 &, T32 &>;

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30, typename T31, typename T32>
std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &, T31 &, T32 &> f376(T1 &a0, T2 &a1, T3 &a2, T4 &a3, T5 &a4, T6 &a5, T7 &a6, T8 &a7, T9 &a8, T10 &a9, T11 &a10, T12 &a11, T13 &a12, T14 &a13, T15 &a14, T16 &a15, T17 &a16, T18 &a17, T19 &a18, T20 &a19, T21 &a20, T22 &a21, T23 &a22, T24 &a23, T25 &a24, T26 &a25, T27 &a26, T28 &a27, T29 &a28, T30 &a29, T31 &a30, T32 &a31) {
  return std::tie(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30, typename T31, typename T32>
bool f377(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &, T31 &, T32 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &, T31 &, T32 &> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30, typename T31, typename T32>
bool f378(const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &, T31 &, T32 &> &a, const std::tuple<T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &, T10 &, T11 &, T12 &, T13 &, T14 &, T15 &, T16 &, T17 &, T18 &, T19 &, T20 &, T21 &, T22 &, T23 &, T24 &, T25 &, T26 &, T27 &, T28 &, T29 &, T30 &, T31 &, T32 &> &b) {
  return operator!=(a, b);
}
