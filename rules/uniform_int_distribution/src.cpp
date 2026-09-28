// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// std::uniform_int_distribution<> (== <int>).
//
// WHAT IS SPECIFIED AND MUST BE PRESERVED: the closed RANGE [a, b] and
// UNIFORMITY over it.  The bit-to-range mapping is implementation-defined, so an
// exact libc++ word-consumption match is NOT observable -- and at every corpus
// site the engine is seeded from std::random_device anyway (dip/dip.cpp:4809 and
// :4907 feed Dip::GetStAddrForSeg, which picks a bin with
// uniform_int_distribution<>(0, numBins-1)).
//
// MODEL: (i32, i32) == (a, b), exactly as rules/pair models std::pair.
//
// operator() is keyed ONLY for a std::mt19937 engine -- the one engine the
// corpus uses.  A generic URBG parameter would need the rule body to invoke an
// unconstrained generic, which is not expressible here; leaving it unkeyed makes
// any other engine FAIL LOUDLY at translate time rather than silently draw from
// the wrong source.
#include <random>

using t1 = std::uniform_int_distribution<>;

std::uniform_int_distribution<> f1() { return std::uniform_int_distribution<>(); }

std::uniform_int_distribution<> f2(int a0, int a1) {
  return std::uniform_int_distribution<>(a0, a1);
}

int f3(std::uniform_int_distribution<> &a0, std::mt19937 &a1) { return a0(a1); }

int f4(std::uniform_int_distribution<> &a0) { return a0.a(); }

int f5(std::uniform_int_distribution<> &a0) { return a0.b(); }

int f6(std::uniform_int_distribution<> &a0) { return a0.min(); }

int f7(std::uniform_int_distribution<> &a0) { return a0.max(); }

// The `unsigned long` result_type spelling of the engine -- the one the corpus's
// real cxxflags produce.  See rules/mersenne_twister_engine/src.cpp.
using MT64_ = std::mersenne_twister_engine<unsigned long, 32, 624, 397, 31,
                                           0x9908b0dfUL, 11, 0xffffffffUL, 7,
                                           0x9d2c5680UL, 15, 0xefc60000UL, 18,
                                           1812433253UL>;

int f8(std::uniform_int_distribution<> &a0, MT64_ &a1) { return a0(a1); }
