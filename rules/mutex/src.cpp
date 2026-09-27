// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::mutex and its SCOPED GUARDS.
//
// The corpus shape is 86 std::lock_guard + 34 std::unique_lock + 7 bare
// .lock().  Bare std::mutex::lock()/unlock() are NOT keyed: the lock they
// acquire has no expression-level place to keep a guard, so a rule for them
// could only produce a lock that never locks.  They stay a LOUD failure.
//
// A guard's LIFETIME is not a problem for a one-expression rule body: the one
// expression is the `let` INITIALIZER at the declaration site, and the
// converter already emits a scope-lifetime local there (the same shape as
// `std::atomic<bool> a(true)` / `std::ofstream f(path)`), so the Rust guard's
// Drop fires where the C++ destructor fires.
//
// std::mutex is neither copyable nor movable; returning a prvalue is
// nevertheless well formed under C++17 mandatory copy elision, which is how
// f1/f2 are spelled (same device as rules/atomic f10/f11).

#include <mutex>

using t1 = std::mutex;
using t2 = std::lock_guard<std::mutex>;
using t3 = std::unique_lock<std::mutex>;

std::mutex f1() { return std::mutex(); }

std::lock_guard<std::mutex> f2(std::mutex &a0) {
  return std::lock_guard<std::mutex>(a0);
}

std::unique_lock<std::mutex> f3(std::mutex &a0) {
  return std::unique_lock<std::mutex>(a0);
}

void f4(std::unique_lock<std::mutex> &a0) { return a0.unlock(); }

bool f5(const std::unique_lock<std::mutex> &a0) { return a0.owns_lock(); }
