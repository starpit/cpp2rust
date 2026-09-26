// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::atomic<T> -> a plain T.  The justification for a non-atomic
// representation, what it gives up, and what would have to change if a thread
// ever appeared are all in src.cpp; read that before extending this file.
//
// PostfixInc must be NAMED in this `use` list to be in scope: rules files use a
// narrow import list rather than a glob, and a missing trait import here is a
// build failure in the rules crate, not a translation-time diagnostic.

use libcc2rs::PostfixDec;
use libcc2rs::PostfixInc;
use libcc2rs::PrefixDec;
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

// The -- table and the `long` width.  See src.cpp: the key names
// std::__atomic_base, the width is concrete, and prefix returns the NEW value
// while postfix returns the OLD one -- swapping the two trait calls compiles
// and is off by one.

unsafe fn f6(a0: &mut i64) -> i64 {
    a0.prefix_dec()
}

unsafe fn f7(a0: &mut i64) -> i64 {
    a0.postfix_dec()
}

unsafe fn f8(a0: &mut i64) -> i64 {
    a0.prefix_inc()
}

unsafe fn f9(a0: &mut i64) -> i64 {
    a0.postfix_inc()
}

unsafe fn f10(a0: &mut i32) -> i32 {
    a0.prefix_dec()
}

unsafe fn f11(a0: &mut i32) -> i32 {
    a0.postfix_dec()
}

unsafe fn f12(a0: &mut u64) -> u64 {
    a0.prefix_dec()
}

unsafe fn f13(a0: &mut u64) -> u64 {
    a0.postfix_dec()
}
