// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::ArrayRef<T> -> `(*const T1, i64)`: the pointer and the length, which is
// exactly what ArrayRef stores.  rules/mlir's `(base, i64)` range convention
// (t5/t6) reused, not a third shape.  src.cpp has the survey, the -verbose keys,
// and why initializer_list is left loud.

// NOTE, a trap paid for here: `unsafe fn` does NOT supply an unsafe context for
// its own body in this toolchain (Rust 2024 `unsafe_op_in_unsafe_fn`), and
// cpp-rule-preprocessor TYPE-CHECKS target bodies.  A raw-pointer op needs an
// explicit `unsafe { .. }` block; without it the preprocessor dies with
// `_Unwind_Resume` at semantic.rs:261 -- the SAME line as the tuple-receiver
// trap, so the panic site actively misleads.

fn t1<T1>() -> (*const T1, i64) {
    (::std::ptr::null(), 0)
}

unsafe fn f1<T1>() -> (*const T1, i64) {
    (::std::ptr::null(), 0)
}

// ONE element -> LENGTH 1.  Not 0, and not the length of whatever container the
// element happens to live in -- the classic silent-wrongness trap for this type.
unsafe fn f2<T1>(a0: *const T1) -> (*const T1, i64) {
    (a0, 1)
}

unsafe fn f3<T1>(a0: *const T1, a1: u64) -> (*const T1, i64) {
    (a0, a1 as i64)
}

// From a container.  A BORROW: the pair points INTO the container's buffer, so a
// later mutation through the container is visible through the view, as C++
// requires.  `a0` is named ONCE -- a rule body is inlined textually and every
// further occurrence would re-evaluate the argument expression.
unsafe fn f4<T1>(a0: *const Vec<T1>) -> (*const T1, i64) {
    unsafe {
        let __v = &*a0;
        (<[T1]>::as_ptr(__v), <[T1]>::len(__v) as i64)
    }
}

// Field access, not a method call: the trap about a TUPLE receiver
// (semantic.rs:261) applies to method calls, which these are not.
unsafe fn f5<T1>(a0: (*const T1, i64)) -> u64 {
    a0.1 as u64
}

unsafe fn f6<T1>(a0: (*const T1, i64)) -> bool {
    a0.1 == 0
}

// operator[] returns `const T1 &`, i.e. a pointer in this model.
unsafe fn f7<T1>(a0: (*const T1, i64), a1: u64) -> *const T1 {
    unsafe { a0.0.add(a1 as usize) }
}
