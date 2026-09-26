// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the units argument.  Every duration and time_point is an i64
// holding ITS OWN count, so count() and the time_point difference carry no
// scale factor and duration_cast carries a literal one.

use libcc2rs::*;

fn t1() -> i64 { 0 }
fn t4() -> i64 { 0 }
fn t5() -> i64 { 0 }

// steady_clock is monotonic by specification, so CLOCK_MONOTONIC, not the wall
// clock.  libc++'s steady_clock::period is nano, so the value is nanoseconds.
fn f1() -> i64 {
    {
        // A monotonic origin, initialised once: steady_clock is required to be
        // monotonic, so this is Instant (CLOCK_MONOTONIC), never the wall clock.
        static __ORIGIN: std::sync::OnceLock<std::time::Instant> = std::sync::OnceLock::new();
        __ORIGIN.get_or_init(std::time::Instant::now).elapsed().as_nanos() as i64
    }
}

// !! OPERAND ORDER IS REVERSED, AND THAT IS MEASURED, NOT A TYPO. !!
// With the obvious `a0 - a1` the probe's `b - a` (two successive now() calls)
// came out NEGATIVE; with `a1 - a0` it is positive and both models MATCH C++.
// So for this free binary operator the converter hands the rule (rhs, lhs).
// UNCONFIRMED whether that is general to std::chrono::operator- , to all free
// operators, or specific to this key -- it needs a second call site with two
// distinguishable operands to pin down.  Do not "tidy" this back to a0 - a1
// without re-running the probe: the wrong order still compiles and still
// returns 0 for a self-difference, so only a two-timestamp probe catches it.
fn f2(a0: i64, a1: i64) -> i64 { a1 - a0 }

fn f3(a0: i64) -> i64 { a0 }
fn f6(a0: i64) -> i64 { a0 }

fn f30(a0: i64) -> i64 { a0 }
fn f33(a0: i64) -> i64 { a0 }

fn f40(a0: i32) -> i64 { a0 as i64 }
fn f43(a0: i32) -> i64 { a0 as i64 }
