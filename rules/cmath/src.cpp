// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cmath>

int f1(int x) { return std::abs(x); }

long f2(long x) { return std::abs(x); }

long long f3(long long x) { return std::abs(x); }

double f4(double x) { return std::log2(x); }

double f5(int x) { return std::log2(x); }

// libc++ routes the integral overloads of the <cmath> functions through
// std::__math, so the signature cpp2rust looks up is
// `double std::__math::ceil(long long)`, not `double ceil(double)`.  A rule
// written against the C spelling never matches those call sites.

double f6(long long x) { return std::ceil(x); }

double f7(int x) { return std::ceil(x); }

double f8(long long x) { return std::floor(x); }

double f9(int x) { return std::floor(x); }

double f10(int x, int y) { return std::pow(x, y); }

double f11(double x, int y) { return std::pow(x, y); }

double f12(int x) { return std::sqrt(x); }

double f13(double x, int y) { return std::fmod(x, y); }

// The float overloads return FLOAT, not double: `float std::__math::ceil(float)`.

float f14(float x) { return std::ceil(x); }

double f15(long x) { return std::ceil(x); }

double f16(unsigned int x) { return std::ceil(x); }

double f17(unsigned long x) { return std::ceil(x); }

float f18(float x) { return std::floor(x); }

double f19(long x) { return std::floor(x); }

double f20(unsigned long x) { return std::floor(x); }

double f21(long x, long y) { return std::pow(x, y); }

double f22(long long x) { return std::sqrt(x); }

float f23(float x) { return std::sqrt(x); }

double f24(long long x, long long y) { return std::fmod(x, y); }
