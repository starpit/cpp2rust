// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// The parameter and the result are the POINTER, not the pointee: declaring
// them `&mut T1` makes the converter deref the placeholder, and the rule then
// returns a value where the caller wants a reference.

unsafe fn f1<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

unsafe fn f2<T1>(a0: *mut T1) -> *mut T1 {
    a0
}
