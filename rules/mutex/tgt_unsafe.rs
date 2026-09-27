// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

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

unsafe fn f1() -> ::std::sync::Mutex<()> {
    ::std::sync::Mutex::new(())
}

unsafe fn f2(a0: *mut ::std::sync::Mutex<()>) -> LockGuard<()> {
    unsafe { LockGuard::new(&*a0) }
}

unsafe fn f3(a0: *mut ::std::sync::Mutex<()>) -> LockGuard<()> {
    unsafe { LockGuard::new(&*a0) }
}

unsafe fn f4(a0: *mut LockGuard<()>) {
    unsafe { LockGuard::unlock(&mut *a0) }
}

unsafe fn f5(a0: *const LockGuard<()>) -> bool {
    unsafe { LockGuard::owns_lock(&*a0) }
}
