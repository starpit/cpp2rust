// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp: rules/deque models std::deque<T1> as Vec<T1> (its tgt_unsafe t1
// is `Vec<T1>` with the VecDeque TODO), so its iterator is rules/vector's
// iterator -- a raw element pointer.  `iterator` -> *mut T1,
// `const_iterator` -> *const T1.

use libcc2rs::*;

fn t1<T1>() -> *mut T1 {
    Default::default()
}

fn t2<T1>() -> *const T1 {
    Default::default()
}

unsafe fn f1<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr()
}

unsafe fn f2<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr().add(a0.len())
}

// A begin()/end() target receives the container BY VALUE even for a `const &`
// parameter, so `a0 as *const _` would be E0605; take it by value, exactly as
// rules/vector f43/f44 and rules/list_iterator f3/f4 do.
unsafe fn f3<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr()
}

unsafe fn f4<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr().add(a0.len())
}

// POSITION comparison, not value comparison: two iterators at different
// positions holding EQUAL elements must compare UNEQUAL.
unsafe fn f5<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 == a1
}

unsafe fn f6<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 != a1
}

unsafe fn f7<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.prefix_inc()
}

// postfix ++ yields the OLD position; UnsafePostfixInc does exactly that.
unsafe fn f8<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.postfix_inc()
}

unsafe fn f9<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

// difference_type is `long` -> i64, not isize.
unsafe fn f10<T1>(a0: *mut T1, a1: i64) -> *mut T1 {
    a0.offset(a1 as isize)
}

unsafe fn f11<T1>(a0: &mut *mut T1, a1: i64) -> *mut T1 {
    *a0 = (*a0).offset(a1 as isize);
    *a0
}

unsafe fn f12<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 == a1
}

unsafe fn f13<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 != a1
}

unsafe fn f14<T1>(a0: &mut *const T1) -> *const T1 {
    a0.prefix_inc()
}

unsafe fn f15<T1>(a0: &mut *const T1) -> *const T1 {
    a0.postfix_inc()
}

unsafe fn f16<T1>(a0: *const T1) -> *const T1 {
    a0
}

unsafe fn f17<T1>(a0: *const T1, a1: i64) -> *const T1 {
    a0.offset(a1 as isize)
}

unsafe fn f18<T1>(a0: &mut *const T1, a1: i64) -> *const T1 {
    *a0 = (*a0).offset(a1 as isize);
    *a0
}
