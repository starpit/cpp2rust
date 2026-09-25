// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::TrackingStatistic (= mlir::Pass::Statistic) -> the u64 counter itself.
// The justification for dropping the three description strings and the
// Initialized flag, the measurement that says nothing in scope reads them, and
// what would have to change if `-stats` were ever ported are all in src.cpp;
// read that before extending this file.
//
// PostfixInc must be NAMED in this `use` list to be in scope: rules files use a
// narrow import list rather than a glob, and a missing trait import here is a
// build failure in the rules crate, not a translation-time diagnostic.

use libcc2rs::PostfixInc;
use libcc2rs::PrefixInc;

fn t1() -> u64 {
    0
}

// Prefix: step, then hand back the POINTER to the counter, because the C++
// returns `const TrackingStatistic &` and a reference-out rule is spelled as
// the pointer.  The `({ .. })` parens are mandatory -- a bare block body is a
// parse error the moment the rule lands in an `if`/`while` condition.
unsafe fn f1(a0: &mut u64) -> *mut u64 {
    ({
        a0.prefix_inc();
        a0 as *mut u64
    })
}

// Postfix: hand back the OLD value.  PrefixInc here would compile and answer
// one too high.
unsafe fn f2(a0: &mut u64) -> u64 {
    a0.postfix_inc()
}
