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

// g630 / g651: the PREFIX increment and decrement on the INTEGRAL atomics.
//
// WHICH CLASS THE KEY NAMES, AND WHY IT DIFFERS FROM load/store/exchange.
// libc++ splits __atomic_base in two:
//     template <class T, bool = is_integral<T>::value && !is_same<T,bool>::value>
//     struct __atomic_base;                                  // primary: <T, false>
//     template <class T> struct __atomic_base<T, true>        // integral extras
//         : public __atomic_base<T, false> { ... operator++() ... operator--() ... };
// load/store/exchange live in the PRIMARY, whose only written spelling in the
// tree is the base clause `__atomic_base<_Tp, false>` -- which is exactly why
// f14/f15/f16 above are recorded as `std::__atomic_base<unsigned long, false>`
// and why the elided spelling was a DEAD KEY there.  operator++/operator--
// live in the `<T, true>` partial specialization, which `std::atomic<int>`
// names through the written base clause `__atomic_base<_Tp>` -- so the key the
// converter searches for is `std::__atomic_base<int>`, WITHOUT a second
// argument, exactly as the queue rows g630/g651 print it.  The two spellings
// are therefore both real and both needed; they are NOT alternatives.
typedef std::atomic<int> t6;
typedef std::__atomic_base<int> t7;
typedef std::__atomic_base<int, false> t8;

std::atomic<int> f17() { return std::atomic<int>(); }
std::atomic<int> f18(int a0) { return std::atomic<int>(a0); }

// PREFIX, so the value returned is the NEW one.  C++ defines
// `__atomic_base<T,true>::operator++()` as `fetch_add(1) + 1` with seq_cst.
int f19(std::__atomic_base<int> &a0) { return a0.operator++(); }

typedef std::atomic<long> t9;
typedef std::__atomic_base<long> t10;
typedef std::__atomic_base<long, false> t11;

std::atomic<long> f20() { return std::atomic<long>(); }
std::atomic<long> f21(long a0) { return std::atomic<long>(a0); }

long f22(std::__atomic_base<long> &a0) { return a0.operator--(); }

// g815 / g366: the SAME `<T, true>` partial specialisation as f19/f22, now for
// `unsigned long`.
//
// ⛔ WHY BOTH t5 AND t12 ARE NEEDED AND NEITHER IS A DUPLICATE.  t5 above is
// `std::__atomic_base<unsigned long, false>` -- the PRIMARY, where load/store/
// exchange live, and whose only written spelling in the tree is the base clause
// `__atomic_base<_Tp, false>`.  operator++ lives in the `<T, true>` partial
// specialisation, named through the written base clause `__atomic_base<_Tp>`,
// so the searched key elides the second argument.  The mapper matches by
// STRING, so the two spellings are two different strings and t5 does NOT cover
// this one -- the same split already recorded at t7/t8 for `int`, where the
// elided spelling was proven necessary and the two-argument one was proven
// necessary too.  Confirmed by the samples: g815/g366 both print
// `std::__atomic_base<unsigned long>` with no second argument.
typedef std::__atomic_base<unsigned long> t12;

// PREFIX (g815, external/g3log/g3log.cpp:172 `++g_fatal_hook_recursive_counter`,
// whose own comment reads "thread safe counter" -- i.e. the site is relying on
// the atomicity, which is exactly why this is NOT lowered to `+= 1`).  Prefix
// yields the NEW value, so `fetch_add(1)`'s OLD return is corrected by
// `wrapping_add(1)`, identically to f19.
unsigned long f23(std::__atomic_base<unsigned long> &a0) {
  return a0.operator++();
}

// POSTFIX (g366, dcg dlOps.cpp:526 and dlOpsNew.cpp:844,
// `std::to_string(gId++)`).  Postfix yields the OLD value, which IS
// `fetch_add(1)`'s return -- so, unlike f19/f23, there is NO correction term,
// and that missing `wrapping_add(1)` is the whole difference between the two
// keys.  The `int` in the key `...::operator++(int)` is the dummy parameter that
// distinguishes postfix from prefix; it is spelled as a literal 0 in MEMBER CALL
// form, the only form that records a key (an infix `a0++` records nothing).
unsigned long f24(std::__atomic_base<unsigned long> &a0) {
  return a0.operator++(0);
}
