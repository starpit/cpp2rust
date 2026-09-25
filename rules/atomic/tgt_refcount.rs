// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::atomic<T> -> a plain T, which the refcount model then boxes in the
// Value<T> every scalar already gets.  The justification for a non-atomic
// representation, what it gives up, and what would have to change if a thread
// ever appeared are all in src.cpp; read that before extending this file.
//
// NOTE the body of f2 does NOT use libcc2rs's PostfixInc, even though the
// unsafe overlay does and `impl<T> PostfixInc for Ptr<T>` exists.  That impl
// increments the POINTER (it advances Ptr::offset by one element), which is the
// right thing for an iterator and exactly the wrong thing here: the C++
// increments the pointed-to counter, not the handle.  Calling it would compile
// and leave the counter at 0 forever while handing back aliases to it.  So the
// increment is spelled out on the pointee via read/write.

use libcc2rs::Ptr;

fn t1<T1>() -> T1 {
    unimplemented!()
}

fn f1<T1>(a0: T1) -> T1 {
    a0
}

fn f2(a0: Ptr<u64>) -> u64 {
    ({
        let __old: u64 = a0.read();
        a0.write(__old.wrapping_add(1));
        __old
    })
}

fn f3(a0: Ptr<i32>) -> i32 {
    ({
        let __old: i32 = a0.read();
        a0.write(__old.wrapping_add(1));
        __old
    })
}

fn f4(a0: Ptr<i32>) -> i32 {
    ({
        let __new: i32 = a0.read().wrapping_add(1);
        a0.write(__new);
        __new
    })
}

fn f5(a0: Ptr<u64>) -> u64 {
    ({
        let __new: u64 = a0.read().wrapping_add(1);
        a0.write(__new);
        __new
    })
}
