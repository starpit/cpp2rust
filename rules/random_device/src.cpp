// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// std::random_device -- THE EASIEST KEY IN THIS WAVE, NOT A REFUSAL.
//
// 8 of the 9 corpus sites that reach <random> seed from std::random_device
// (senulator/r5emulator.hpp:134-138, ddb CompareTensorsResultsWriter.cpp:100-103,
// dcc SentientToTrace.cpp:187-189, dr5 dataconv.cpp:202-243, dip/dip.cpp:4809
// and :4907).  The C++ program is therefore ALREADY non-reproducible run to run,
// so there is no C++ sequence to reproduce: any nondeterministic Rust source is
// observationally faithful *because* the C++ is nondeterministic.
//
// MODEL: an opaque handle (u8).  libc++'s random_device default token is
// "/dev/urandom" (__random/random_device.h), so operator() reads 4 bytes from it
// -- the SAME kernel entropy source, not a lookalike.  A read failure PANICS; it
// must never silently degrade to a fixed value.
#include <random>

using t1 = std::random_device;

std::random_device f1() { return std::random_device(); }

unsigned f2(std::random_device &a0) { return a0(); }
