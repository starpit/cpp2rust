// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::atomic<T> -- modelled as REAL Rust atomics, not as T.
//
// WHY REAL ATOMICS AND NOT Cell<T>/plain T
// ----------------------------------------
// The first corpus site that forced this module is common/logging.cpp, whose
// `std::atomic<bool>` arrives through g3log:
//     external/g3log/g3log/loglevels.hpp:103   atomicbool status;
//     external/g3log/g3log/atomicbool.hpp:18   std::atomic<bool> value_;
// i.e. the per-log-level "is this level enabled" flag.  g3log is an ASYNC
// logger: a background LogWorker thread drains the queue while arbitrary
// application threads call LOG(), and atomicbool.hpp itself spells out
// acquire/release pairs (load(memory_order_acquire) / store(.., release)) --
// that is a deliberate cross-thread handshake, not an accident.  The wider
// corpus is concurrent too: mock_rt/backend/shm_queue.h has
// std::atomic<uint64_t> rd_tail_/wr_head_ (a lock-free SPSC ring), and
// senulator/fifo.cpp uses bare std::atomic_thread_fence.  Modelling
// std::atomic<T> as Cell<T> or plain T would silently DELETE the only
// synchronisation those sites have, which is exactly the "silent wrongness"
// this port refuses.  So: std::atomic<bool> -> std::sync::atomic::AtomicBool,
// and std::memory_order -> std::sync::atomic::Ordering, with every operation
// carrying the order the C++ wrote.
//
// Only the bool instantiation is modelled here.  std::atomic<T> is NOT one
// Rust type: each width is a distinct type (AtomicI32/AtomicU64/...), so each
// instantiation needs its own type rule, and there is no generic spelling that
// covers them.  The integral ones are the next rows, not a generalisation of
// this one.

#include <atomic>

typedef std::atomic<bool> t1;
typedef std::memory_order t2;

std::memory_order f1() { return std::memory_order_relaxed; }
std::memory_order f2() { return std::memory_order_consume; }
std::memory_order f3() { return std::memory_order_acquire; }
std::memory_order f4() { return std::memory_order_release; }
std::memory_order f5() { return std::memory_order_acq_rel; }
std::memory_order f6() { return std::memory_order_seq_cst; }

bool f7(const std::atomic<bool> &a0, std::memory_order a1) {
  return a0.load(a1);
}

void f8(std::atomic<bool> &a0, bool a1, std::memory_order a2) {
  return a0.store(a1, a2);
}

bool f9(std::atomic<bool> &a0, bool a1, std::memory_order a2) {
  return a0.exchange(a1, a2);
}

// std::atomic<bool> DERIVES from std::__atomic_base<bool>, and every operation
// key above is recorded against the BASE (see ir_src.json), so the base needs a
// type rule of its own or translation aborts on it -- measured on logging.cpp,
// where adding only `std::atomic<bool>` moved the abort to
// `std::__atomic_base<bool, false>`.  Both names denote the same Rust type.
typedef std::__atomic_base<bool> t3;

// A TYPE RULE ALONE IS NOT ENOUGH (rules/mlir's `mlir::Attribute` paid for this):
// the construction path needs its own rule or the TU translates rc=0 and then
// fails to compile on an undefined `std_atomic_bool_::new`.  std::atomic is
// neither copyable nor movable; returning a prvalue is nevertheless well formed
// under C++17's mandatory copy elision, which is how these are spelled.
std::atomic<bool> f10() { return std::atomic<bool>(); }
std::atomic<bool> f11(bool a0) { return std::atomic<bool>(a0); }

// g1283/g1297: the SAME model for `unsigned long`.  MEASURED (probe, atomic.h:42:8):
// the converter searches the base as `std::__atomic_base<unsigned long, false>`
// -- WITH the second argument spelled -- unlike bool, which is searched as
// `std::__atomic_base<bool>`.  The elided spelling gave rc=0 and then an
// UnmappedSystemType abort, so the second argument is written out here.
// -- exactly as t3 is for bool.  Both the derived type and the base need a rule or
// the member calls resolve against an unmapped base.
typedef std::atomic<unsigned long> t4;
typedef std::__atomic_base<unsigned long, false> t5;

std::atomic<unsigned long> f12() { return std::atomic<unsigned long>(); }
std::atomic<unsigned long> f13(unsigned long a0) {
  return std::atomic<unsigned long>(a0);
}

unsigned long f14(const std::atomic<unsigned long> &a0, std::memory_order a1) {
  return a0.load(a1);
}

void f15(std::atomic<unsigned long> &a0, unsigned long a1,
         std::memory_order a2) {
  return a0.store(a1, a2);
}

unsigned long f16(std::atomic<unsigned long> &a0, unsigned long a1,
                 std::memory_order a2) {
  return a0.exchange(a1, a2);
}
