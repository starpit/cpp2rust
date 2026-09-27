// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// REFCOUNT LIMITATION, MEASURED: in the refcount model a C++ reference arrives
// as `Ptr<T>`, and EVERY way to read through a `Ptr<T>` (`with`, `with_mut`,
// `deref`) is bounded on `T: ByteRepr` -- which neither `std::sync::Mutex<()>`
// nor `LockGuard<()>` can satisfy (a mutex is not byte-copyable).  So the
// by-reference bodies below keep the raw-pointer shape; under refcount they
// fail to COMPILE (E0605 non-primitive cast Ptr<LockGuard<()>> as *mut
// LockGuard<()>, E0614 cannot dereference Ptr<Mutex<()>>) rather than
// silently locking nothing.  The unsafe model is verified end to end.

use libcc2rs::LockGuard;

fn t1() -> ::std::sync::Mutex<()> {
    ::std::sync::Mutex::new(())
}

fn t2() -> LockGuard<()> {
    LockGuard::empty()
}

fn t3() -> LockGuard<()> {
    LockGuard::empty()
}

fn f1() -> ::std::sync::Mutex<()> {
    ::std::sync::Mutex::new(())
}

fn f2(a0: *mut ::std::sync::Mutex<()>) -> LockGuard<()> {
    unsafe { LockGuard::new(&*a0) }
}

fn f3(a0: *mut ::std::sync::Mutex<()>) -> LockGuard<()> {
    unsafe { LockGuard::new(&*a0) }
}

fn f4(a0: *mut LockGuard<()>) {
    unsafe { LockGuard::unlock(&mut *a0) }
}

fn f5(a0: *const LockGuard<()>) -> bool {
    unsafe { LockGuard::owns_lock(&*a0) }
}
