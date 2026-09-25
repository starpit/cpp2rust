// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::atomic<T> -> a plain T.  The justification for a non-atomic
// representation, what it gives up, and what would have to change if a thread
// ever appeared are all in src.cpp; read that before extending this file.
//
// PostfixInc must be NAMED in this `use` list to be in scope: rules files use a
// narrow import list rather than a glob, and a missing trait import here is a
// build failure in the rules crate, not a translation-time diagnostic.

use libcc2rs::PostfixInc;
use libcc2rs::PrefixInc;

fn t1<T1>() -> T1 {
    unimplemented!()
}

unsafe fn f1<T1>(a0: T1) -> T1 {
    a0
}

unsafe fn f2(a0: &mut u64) -> u64 {
    a0.postfix_inc()
}

unsafe fn f3(a0: &mut i32) -> i32 {
    a0.postfix_inc()
}

unsafe fn f4(a0: &mut i32) -> i32 {
    a0.prefix_inc()
}

unsafe fn f5(a0: &mut u64) -> u64 {
    a0.prefix_inc()
}
