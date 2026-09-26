// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp.  THE POSITION HALF ONLY.
//
// llvm::mapped_iterator is `{ WrappedIteratorT I; Callable<FuncTy> F; }`, and the
// representation here is the WRAPPED POINTER ALONE -- `*const T1`, the same
// representation rules/vector gives a contiguous iterator.  That is sufficient
// for prefix ++ (LLVM's body is `++I`, which does not read F) and for == / !=
// (LLVM's body is `LHS.I == RHS.I`, position identity).  It is NOT sufficient for
// operator*, which is `F(*this->I)` -- so operator* has NO RULE here and stays
// loud.  T2 and T3 (the callable's argument and return types) appear in the
// signatures because the key names them, and are unused in the bodies for
// exactly that reason: the callable is not represented.
use libcc2rs::UnsafePrefixInc;

fn t1<T1, T2, T3>() -> *const T1 {
    core::ptr::null()
}

// PREFIX ++: advance, then yield the NEW position.  `prefix_inc` is
// libcc2rs::UnsafePrefixInc for *const T (inc.rs:100) -- it writes the advanced
// pointer back through the receiver and returns the advanced value, which is the
// prefix, not the postfix, semantics.  rules/vector's f34 uses the same helper
// for std::vector's iterator.
unsafe fn f1<T1, T2, T3>(a0: &mut *const T1) -> *const T1 {
    a0.prefix_inc()
}

unsafe fn f2<T1, T2, T3>(a0: *const T1, a1: *const T1) -> bool {
    a0 == a1
}

unsafe fn f3<T1, T2, T3>(a0: *const T1, a1: *const T1) -> bool {
    a0 != a1
}

// The TWO-ARGUMENT CONSTRUCTOR -- LOSSY ON PURPOSE.  The wrapped iterator IS the
// whole object here, so the constructor is the identity on a0 and `a1`, the
// callable, is DROPPED: `*const T1` has no field to hold it.  That is why
// operator* has no rule and must not get one under this representation -- after
// this body runs the function is gone from the program.
unsafe fn f4<T1, T2, T3>(a0: *const T1, a1: unsafe fn(T2) -> T3) -> *const T1 {
    a0
}
