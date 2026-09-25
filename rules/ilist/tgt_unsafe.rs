// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::ilist_iterator -> the NODE POINTER, which is its entire state
// (ilist_iterator.h:108).  Comparison is pointer comparison and dereference is
// the identity, both exactly as LLVM spells them.  operator++/-- deliberately
// have no rule because they read an intrusive link that lives behind the
// --opaque-namespace=mlir boundary; src.cpp explains why, and why the two
// candidate inventions are both silently wrong.
//
// t1/t2 return a null pointer rather than `unimplemented!()`: a
// default-constructed ilist_iterator IS a null NodePtr in LLVM, so this records
// the C++ value rather than inventing one.

fn t1<T1>() -> *mut T1 {
    ::std::ptr::null_mut()
}

fn t2<T1>() -> *mut T1 {
    ::std::ptr::null_mut()
}

unsafe fn f1<T1>(a0: *mut T1, a1: *mut T1) -> bool {
    a0 != a1
}

unsafe fn f2<T1>(a0: *mut T1, a1: *mut T1) -> bool {
    a0 == a1
}

unsafe fn f3<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

unsafe fn f4<T1>(a0: *mut T1, a1: *mut T1) -> bool {
    a0 != a1
}

unsafe fn f5<T1>(a0: *mut T1, a1: *mut T1) -> bool {
    a0 == a1
}

unsafe fn f6<T1>(a0: *mut T1) -> *mut T1 {
    a0
}
