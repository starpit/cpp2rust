// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cmath>

int f1(int x) { return std::abs(x); }

long f2(long x) { return std::abs(x); }

long long f3(long long x) { return std::abs(x); }

double f4(double x) { return std::log2(x); }

double f5(int x) { return std::log2(x); }

// std::pow -- 10 of the goal TU's undefined-free-function errors are ONE key,
// `std::pow(2, 20)` at util/foldManager/foldInfrastructure.h:1763/1832/1866
// (int, int).  libc++'s two-argument pow over integral arguments is the
// __promote template, which yields DOUBLE, not int: a body that stayed in
// integer arithmetic would be silent truncation, and `2^20 = 1048576` does not
// even fit a 16-bit int.  The other three overloads are keyed because the
// RETURN TYPE is part of the key, so (double,double), (double,int) and
// (float,float) are DISTINCT keys -- none of them collapses, because the
// distinguishing arguments are ordinary function parameters, not explicit
// template arguments.
double f6(double a0, double a1) { return std::pow(a0, a1); }

double f7(int a0, int a1) { return std::pow(a0, a1); }

double f8(double a0, int a1) { return std::pow(a0, a1); }

float f9(float a0, float a1) { return std::pow(a0, a1); }
