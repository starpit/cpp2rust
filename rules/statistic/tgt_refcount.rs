// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::TrackingStatistic (= mlir::Pass::Statistic) -> the u64 counter itself,
// which the refcount model then boxes in the Value<u64> every scalar gets.
// See src.cpp for the representation's justification.
//
// NOTE these bodies do NOT use libcc2rs's PostfixInc/PrefixInc even though
// `impl<T> PostfixInc for Ptr<T>` exists.  That impl increments the POINTER (it
// advances Ptr::offset by one element), which is right for an iterator and
// exactly wrong here: the C++ increments the pointed-to counter, not the
// handle.  Calling it would compile and leave the counter at 0 forever.  Same
// trap, same resolution, as rules/atomic/tgt_refcount.rs f2 -- the increment is
// spelled out on the pointee via read/write.

use libcc2rs::Ptr;

fn t1() -> u64 {
    0
}

fn f1(a0: Ptr<u64>) -> Ptr<u64> {
    ({
        a0.write(a0.read().wrapping_add(1));
        a0
    })
}

fn f2(a0: Ptr<u64>) -> u64 {
    ({
        let __old: u64 = a0.read();
        a0.write(__old.wrapping_add(1));
        __old
    })
}
