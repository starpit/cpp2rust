// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

//! A holder for `std::lock_guard<std::mutex>` / `std::unique_lock<std::mutex>`.
//!
//! WHY THIS TYPE EXISTS: `std::sync::MutexGuard<'a, T>` is lifetime-parametric,
//! and a rule TYPE target has to be a single spellable type used as the
//! declared type of a converter-emitted local -- there is no lifetime in scope
//! there to name.  `LockGuard<T>` erases that lifetime behind the invariant
//! C++ already guarantees: a scoped guard cannot outlive the mutex it locks,
//! because the mutex outlives the block the guard is declared in.
//!
//! Scope-exit release is Drop on the inner MutexGuard, which fires where the
//! C++ guard's destructor fires (the converter already emits the local at the
//! declaration site, and Converter::EmitScopedDestructor lowers scope exit).
pub struct LockGuard<T: 'static> {
    guard: Option<std::sync::MutexGuard<'static, T>>,
}

impl<T: 'static> LockGuard<T> {
    /// `std::lock_guard<std::mutex>::lock_guard(std::mutex&)` /
    /// `std::unique_lock<std::mutex>::unique_lock(std::mutex&)`: locks now.
    pub fn new(m: &std::sync::Mutex<T>) -> Self {
        LockGuard {
            guard: Some(Self::lock_erased(m)),
        }
    }

    /// An unlocked holder: `std::unique_lock<std::mutex> l;`, and the
    /// initializer a TYPE rule target has to yield.
    pub fn empty() -> Self {
        LockGuard { guard: None }
    }

    /// `std::unique_lock::unlock()`.  Releasing twice is UB in C++; here it is
    /// simply a no-op on an already-released holder.
    pub fn unlock(&mut self) {
        self.guard = None;
    }

    /// `std::unique_lock::owns_lock()`.
    pub fn owns_lock(&self) -> bool {
        self.guard.is_some()
    }

    /// `std::unique_lock::lock(&mutex)`.  C++ spells this `l.lock()` with the
    /// mutex remembered inside the unique_lock; the mutex is passed back in
    /// here because this holder keeps no borrow of it.
    pub fn relock(&mut self, m: &std::sync::Mutex<T>) {
        self.guard = None;
        self.guard = Some(Self::lock_erased(m));
    }

    fn lock_erased(m: &std::sync::Mutex<T>) -> std::sync::MutexGuard<'static, T> {
        // A poisoned mutex is not a C++ concept: std::mutex has no poison
        // state, so recover the data rather than invent a panic C++ cannot
        // produce.
        let g = m.lock().unwrap_or_else(|e| e.into_inner());
        // SAFETY: lifetime erasure only.  Layout is identical; the guard is
        // owned by a converter-emitted local whose scope is nested inside the
        // scope of the mutex it locks, which is what the C++ guarantees.
        unsafe {
            std::mem::transmute::<std::sync::MutexGuard<'_, T>, std::sync::MutexGuard<'static, T>>(g)
        }
    }
}
