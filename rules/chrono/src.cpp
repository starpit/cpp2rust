// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::chrono, exactly as much of it as dcc/src/Driver/dcc.cpp:279-288 needs:
//
//     auto start = std::chrono::high_resolution_clock::now();
//     ...
//     auto end   = std::chrono::high_resolution_clock::now();
//     uint64_t t = std::chrono::duration_cast<std::chrono::milliseconds>(
//                      end - start).count();
//
// THE UNITS PROBLEM AND WHY THIS FILE IS SPELLED CONCRETELY.
//
// A duration is a (rep, period) pair where the period is a compile-time
// std::ratio.  The obvious rule shape wildcards it --
//
//     template <typename R, typename P> R f(const std::chrono::duration<R,P>&);
//
// -- and normalizeTranslationRule is happy to do so, which is exactly the trap:
// duration_cast<To>'s To is a NON-DEDUCED template argument, so a rule whose
// input period is a wildcard cannot know the ratio it is dividing by.  A body
// that returns the count unchanged COMPILES and reports nanoseconds as
// milliseconds -- a 10^6 error, silent.
//
// The representation chosen here removes the need for any rule to know a ratio
// at all: a duration's Rust value is ITS OWN COUNT, in its own units, which is
// what C++ itself stores.  Consequences:
//
//   * count() is the IDENTITY, for every period.  It returns the stored rep.
//     So it may be, and is, wildcard-free-but-concrete here only for the four
//     periods that occur; a wildcarded identity would also have been correct.
//   * time_point is likewise its clock's native count (libc++ steady_clock:
//     nanoseconds), and operator-(time_point, time_point) is plain subtraction
//     -- both operands necessarily share one period, so no ratio is involved.
//   * duration_cast is THE ONLY operation that needs ratios, and it needs BOTH
//     of them.  So it is written once per CONCRETE (From, To) pair, with both
//     ratios spelled out in the rule source itself.  The scale factor is then a
//     literal in the Rust body and cannot be wrong by omission: a pair that is
//     not written here has NO rule and aborts loudly, which is the point.
//
// That is why nothing below is a template.  A missing (From,To) pair is a loud
// abort, never a silent 10^6.
//
// high_resolution_clock needs no rules of its own: in libc++ it is a typedef
// for steady_clock, so ::now() on it resolves to steady_clock::now() and the
// recorded signature is steady_clock's.
//
// WHY duration_cast IS NOT HERE -- MEASURED, NOT ASSUMED.
//
// normalizeTranslationRule wildcards a std::ratio UNCONDITIONALLY, even when the
// rule source spells it out.  Twelve concrete (From,To) casts were written and
// generated; read back out of ir_src.json they were:
//
//   duration_cast<milliseconds>(nanoseconds)  ->
//     duration<long long, ratio<_,_>> duration_cast(const duration<long long, ratio<_,_>> &)
//   duration_cast<microseconds>(nanoseconds)  ->  ... the IDENTICAL string.
//
// Nine of the twelve collapsed onto one key.  Opting the module into
// `explicit-template-args` does surface To into the key --
//
//     duration_cast<std::chrono::duration<long long, std::ratio<_, _>>>(...)
//
// -- but To's OWN ratio is wildcarded by the same pass, so it distinguishes
// nothing.  There is exactly one accidental exception: std::ratio<1,1> is the
// DEFAULT template argument, so std::chrono::seconds prints as
// `duration<long long>` and is elided rather than wildcarded -- which pins the
// seconds END of a cast but never the other end, so ns->s still ties with ms->s.
//
// So the period of a duration_cast is not recoverable from the key in either
// direction, and a duration_cast rule of ANY shape would apply its literal scale
// factor to inputs of the wrong period: nanoseconds reported as milliseconds,
// 10^6 silent.  There is no faithful body, so there is no rule, and
// dcc/src/Driver/dcc.cpp:285 aborts loudly at duration_cast<milliseconds>.
// That abort is the correct outcome and must not be "fixed" by adding a body.
//
// Note this is NOT a limitation of the representation: count(), the time_point
// difference and the duration constructor are all period-INDEPENDENT under
// native-count (they are the identity on the stored rep), which is why they are
// mappable and duration_cast alone is not.
//
// NOT MODELLED, deliberately: system_clock (zero sites in dt_src), to_time_t,
// duration arithmetic other than time_point difference (no sites), operator<,
// duration_cast between periods not listed, and floating-point reps.  Each
// stays a loud abort rather than getting a guessed body.

#include <chrono>
#include <cstdint>

// nanoseconds / microseconds / milliseconds all normalize to the SAME key
// (duration<long long, ratio<_,_>>), so ONE alias covers the three; seconds is
// separate only because ratio<1,1> is elided as a default argument.
using t1 = std::chrono::nanoseconds;
using t4 = std::chrono::seconds;
using t5 = std::chrono::steady_clock::time_point;

// --- the clock -------------------------------------------------------------

std::chrono::steady_clock::time_point f1() {
  return std::chrono::steady_clock::now();
}

// --- time_point difference -------------------------------------------------
//
// Both operands are the same time_point type, so the result period is the
// clock's period and no conversion happens.  Plain subtraction.
std::chrono::steady_clock::duration f2(
    const std::chrono::steady_clock::time_point &a,
    const std::chrono::steady_clock::time_point &b) {
  return a - b;
}

// --- count(), the identity on the stored rep -------------------------------

// ONE rule, not one per period: the four spellings all normalize to the SAME
// key (see below), and under this representation they all want the same body.
long long f3(const std::chrono::nanoseconds &d) { return d.count(); }
long long f6(const std::chrono::seconds &d) { return d.count(); }

// --- constructing a duration from a count ----------------------------------
//
// Needed by the probe (a duration of a KNOWN number of milliseconds is the only
// reproducible way to test a cast; elapsed wall-clock time is not).  The rep is
// the count, so these are the identity too.

std::chrono::nanoseconds f30(long long n) {
  return std::chrono::nanoseconds(n);
}
std::chrono::seconds f33(long long n) { return std::chrono::seconds(n); }

std::chrono::nanoseconds f40(int n) { return std::chrono::nanoseconds(n); }
std::chrono::seconds f43(int n) { return std::chrono::seconds(n); }
