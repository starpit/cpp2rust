// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// std::mt19937 (std::mersenne_twister_engine<unsigned long, 32, 624, ...>).
//
// ⭐ THE BODY IS A FAITHFUL MT19937, NOT A SUBSTITUTE PRNG.  The brief allowed a
// nondeterministic stand-in because 8 of the 9 corpus sites seed from
// std::random_device and are already non-reproducible -- but a stand-in would
// then have to make a DETERMINISTIC seed fail loudly, and a rule cannot tell the
// two apart at translate time (the seed is a runtime value).  Implementing the
// real algorithm removes the dilemma: it is correct for an entropy seed AND
// bit-exact for a deterministic one, so nothing has to fail.
//
// This matters for senverify/senverify.cpp:104-122, where rngGen() does
// std::mt19937 mt(seed) with a seed derived from the dataset name (:269) and
// writes the words out via DumpToFile as REFERENCE TENSOR DATA for verification.
// (That site is still blocked on std::uniform_real_distribution<float>, whose
// generate_canonical word-to-float mapping is implementation-defined -- a
// DIFFERENT owner; see the report.)
//
// MODEL: Vec<u64> of length 625 -- x[0..624) is the state, x[624] is the index.
// The seeding recurrence is libc++'s seed(result_type):
//   x[0] = sd & 0xFFFFFFFF;  x[i] = 1812433253*(x[i-1] ^ (x[i-1]>>30)) + i  (mod 2^32)
// and the output is the standard tempering, so the word sequence is identical.
#include <random>

using t1 = std::mt19937;

std::mt19937 f1() { return std::mt19937(); }

std::mt19937 f2(unsigned long a0) { return std::mt19937(a0); }

unsigned long f3(std::mt19937 &a0) { return a0(); }

// ⛔ TWO TYPE KEYS, AND THE SECOND IS THE ONE THE CORPUS NEEDS.  `std::mt19937`
// is mersenne_twister_engine<uint_fast32_t, ...>, and uint_fast32_t is
// `unsigned int` under the rule preprocessor's flags but `unsigned long` under
// the corpus's real per-TU cxxflags -- the spelling the queue samples record
// (g2902/g2903: `<unsigned long, _, _, ...>`).  A single key recorded from
// `std::mt19937` therefore MISSES every corpus site.  Spell both explicitly.
using MT64_ = std::mersenne_twister_engine<unsigned long, 32, 624, 397, 31,
                                           0x9908b0dfUL, 11, 0xffffffffUL, 7,
                                           0x9d2c5680UL, 15, 0xefc60000UL, 18,
                                           1812433253UL>;

using t2 = MT64_;

MT64_ f4() { return MT64_(); }

MT64_ f5(unsigned long a0) { return MT64_(a0); }

unsigned long f6(MT64_ &a0) { return a0(); }
