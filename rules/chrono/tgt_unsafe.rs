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
// ONE process-wide origin, in libcc2rs -- NOT a `static` in this body.  A rule body is
// INLINED AT EVERY CALL SITE, so a static here becomes a separate origin per site and
// every now() returns ~0 ("nanos since this site's first call").  Measured: that made
// `end` systematically smaller than `start`, and it MASKED an inverted f2 below.
fn f1() -> i64 {
    libcc2rs::cc2_steady_now_nanos()
}

// a0 is the LHS, a1 the RHS.  An earlier revision had `a1 - a0` with a long comment
// claiming the converter hands a free operator (rhs, lhs).  THAT WAS WRONG, and the
// audit that disproved it is worth keeping: `BuildUnifiedArgs`
// (converter_lib.cpp:1465) prepends a receiver ONLY for a CXXMemberCallExpr/MemberExpr,
// and a CXXOperatorCallExpr is neither -- so for a FREE operator `all_args` is exactly
// `expr->getArgs()`, a0 = LHS, a1 = RHS, never swapped.  Confirmed by probing a mapped
// free non-commutative operator of the identical shape (`std::operator-` on two
// `__wrap_iter<int*>`): `itdiff-fwd:3 itdiff-rev:-3`, both models.
// The original "measurement" was reading the per-call-site-origin artifact above, not
// operand order.
fn f2(a0: i64, a1: i64) -> i64 { a0 - a1 }

fn f3(a0: i64) -> i64 { a0 }
fn f6(a0: i64) -> i64 { a0 }

fn f30(a0: i64) -> i64 { a0 }
fn f33(a0: i64) -> i64 { a0 }

fn f40(a0: i32) -> i64 { a0 as i64 }
fn f43(a0: i32) -> i64 { a0 as i64 }
