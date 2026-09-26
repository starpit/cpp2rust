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

// The -- table and the `long` width.  As with f2-f5 the increment/decrement is
// spelled out on the POINTEE via read/write rather than through libcc2rs's
// PrefixDec/PostfixDec: the Ptr impls of those move the HANDLE by one element,
// which would compile, leave the counter untouched and hand back aliases to it.
// Prefix answers the NEW value, postfix the OLD one (see src.cpp).

fn f6(a0: Ptr<i64>) -> i64 {
    ({
        let __new: i64 = a0.read().wrapping_sub(1);
        a0.write(__new);
        __new
    })
}

fn f7(a0: Ptr<i64>) -> i64 {
    ({
        let __old: i64 = a0.read();
        a0.write(__old.wrapping_sub(1));
        __old
    })
}

fn f8(a0: Ptr<i64>) -> i64 {
    ({
        let __new: i64 = a0.read().wrapping_add(1);
        a0.write(__new);
        __new
    })
}

fn f9(a0: Ptr<i64>) -> i64 {
    ({
        let __old: i64 = a0.read();
        a0.write(__old.wrapping_add(1));
        __old
    })
}

fn f10(a0: Ptr<i32>) -> i32 {
    ({
        let __new: i32 = a0.read().wrapping_sub(1);
        a0.write(__new);
        __new
    })
}

fn f11(a0: Ptr<i32>) -> i32 {
    ({
        let __old: i32 = a0.read();
        a0.write(__old.wrapping_sub(1));
        __old
    })
}

fn f12(a0: Ptr<u64>) -> u64 {
    ({
        let __new: u64 = a0.read().wrapping_sub(1);
        a0.write(__new);
        __new
    })
}

fn f13(a0: Ptr<u64>) -> u64 {
    ({
        let __old: u64 = a0.read();
        a0.write(__old.wrapping_sub(1));
        __old
    })
}
