// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <atomic>
#include <cstdint>

// ---------------------------------------------------------------------------
// std::atomic<T> -> T.  Why a NON-ATOMIC representation is sound here, what it
// gives up, and what would have to change if threads ever appeared.
//
// THE OBSERVATION THIS RESTS ON: there is not one thread in the scheduler.
// Measured over dcg/ ddc/ dsc/ dbo/, the four roots this port covers, with a
// sweep for std::thread, std::jthread, std::mutex, std::recursive_mutex,
// std::shared_mutex, std::lock_guard, std::unique_lock, std::scoped_lock,
// std::async, std::future, std::promise, std::condition_variable,
// std::memory_order, std::barrier, std::latch, std::counting_semaphore,
// pthread_create, pthread_mutex, tbb:: and #pragma omp: ZERO hits.  The only
// threading header reached at all is a single `#include <thread>` at
// dsc/designSpaceConfig.h:31, which names nothing from it -- a leftover
// include, not a use.  ddc/ddl/ddl.cpp talks about MLIR's thread pool, but it
// constructs its MLIRContext with Threading::DISABLED precisely to avoid
// creating one.
//
// So every std::atomic in scope is only ever touched by one thread, and the
// two sites that exist -- dcg/dcg_fe/pcfg_gen/dlOps.cpp:17 and
// dlOpsNew.cpp:122, both `static std::atomic<uint64_t> gId(0)` feeding a
// name-uniquing counter -- are plain read-modify-write on a private counter.
// Under one thread, std::atomic<T>'s specified behaviour IS a plain T's: every
// operation is a single non-interleaved read/modify/write, and the memory
// order is unobservable because there is no second thread to order against.
// The representation is therefore not an approximation; on this input it is
// exact.
//
// WHY T AND NOT Cell<T>.  Cell<T> was the other candidate: interior
// mutability, get/set, no atomics.  It buys the ability to mutate through a
// shared reference -- and C++ itself proves that is not needed, because every
// mutating member of std::atomic is NON-const-qualified (store, exchange,
// fetch_add, operator++, operator+= ... only load and operator T are const).
// A program that mutates an atomic therefore always holds a non-const path to
// it, which is exactly what `&mut` models.  Cell<T> would also be a WORSE
// representation in two concrete ways: it is opaque to every existing scalar
// rule, so `gId + 1` and `gId = 5` (both legal C++ via operator T and
// operator=) would each need a new rule instead of just working; and in the
// refcount model it would nest a Cell inside the Value<T> that already
// provides interior mutability, giving Rc<RefCell<Cell<u64>>> for a counter.
// Plain T needs no new machinery in either model: unsafe gets a `u64`,
// refcount gets the `Value<u64>` every scalar already gets.
//
// WHAT IT GIVES UP, stated plainly:
//   1. Atomicity.  Under two threads the translated counter can lose updates
//      where the C++ cannot.  Sound only because of the sweep above.
//   2. Memory ordering.  A memory_order argument has nowhere to go.  This is
//      not silent: no rule here takes one, so `load(memory_order_acquire)`,
//      `store`, `fetch_add`, `compare_exchange_*` and friends have no rule and
//      abort LOUDLY at converter.cpp.  A future caller does not quietly get a
//      relaxed operation; it gets a translation failure naming the operation.
//   3. Non-copyability.  std::atomic's copy constructor is deleted; a T is
//      Copy.  The extra capability is unreachable: any program that compiles
//      as C++ has already been checked not to copy one.
//   4. is_lock_free(), wait/notify, atomic_flag: no rule, so loud.
//
// WHAT WOULD HAVE TO CHANGE IF A THREAD EVER APPEARED.  This rule would become
// silently wrong -- the bad direction -- so it must be revisited the moment
// the sweep above stops returning zero.  The replacement is not another rule
// tweak: it is std::sync::atomic::AtomicU64 with an Ordering threaded through
// from the C++ memory_order argument, which in turn requires a type rule for
// std::memory_order and its six enumerators, a load/store/fetch_* family that
// takes an ordering, and -- the hard part -- the refcount model's Value<T> =
// Rc<RefCell<T>> to stop being the representation for a shared scalar, since
// Rc is not Send.  That is a campaign-level change, not a module-level one.
// The honest entry point is this comment: if you are here because you found a
// thread, stop and design that, do not extend these two rules.
//
// WHY THE RULES BELOW NAME `unsigned long` CONCRETELY.
// std::__atomic_base<_Tp, bool> has its integral operations -- including
// operator++ -- only on the `true` specialization, and the bool is a DEFAULTED
// argument computed as is_integral<_Tp> && !is_same<_Tp,bool>.  The rule
// preprocessor instantiates a template rule with a dummy record for T1, for
// which that predicate is false, so a templated `std::__atomic_base<T1> &`
// receiver resolves to the generic base and fails with "No viable function".
// Spelling the specialization as `std::__atomic_base<T1, true>` DOES resolve,
// but then prints with the `, true` still in it, while the converter searches
// the spelling with defaulted arguments dropped
// (`std::__atomic_base<unsigned long>::operator++(int)`) -- so that rule would
// resolve and then silently never match, the trap this file must not fall
// into.  A concrete `unsigned long` receiver resolves AND prints the spelling
// the converter asks for, because with a real integral type the default
// evaluates to true and is then suppressed as equal to the default.
//
// The cost is that each atomic width needs its own expression rule.  That is
// the right trade for now: uint64_t is the only width in scope, and adding
// another is a two-line copy of f2 in three files.  The TYPE rule stays
// templated, so std::atomic<T> maps for every T already.
//
// WHY t1's DEFAULT VALUE IS `unimplemented!()` AND NOT `Default::default()`.
// Both were written and generated; this is a deliberate choice, not an
// oversight.  dt_src is -std=c++17 (dt_src/CMakeLists.txt:42), and libc++ only
// value-initializes std::atomic's default constructor from C++20 on
// (__atomic/atomic.h:129-134 guards `: __a_(_Tp())` behind
// `_LIBCPP_STD_VER >= 20`; before that it is `= default`).  So under the
// standard this code is compiled with, `std::atomic<uint64_t> d;` leaves the
// value INDETERMINATE and reading it is UB.  `Default::default()` would answer
// 0 -- inventing a value C++ does not promise, which is the silent direction.
// `unimplemented!()` panics instead, turning a C++ UB read into a loud Rust
// failure at exactly the site that had no defined value to begin with.
//
// It is also unreachable on this input: no std::atomic in dcg/ ddc/ dsc/ dbo/
// is default-constructed (both are `gId(0)`) and none is a struct member, and
// the two translated TUs contain zero `unimplemented!()` from this rule --
// checked, not assumed.  The one thing to know is that in the refcount model a
// record with an atomic member derives Default, so an `unimplemented!()` there
// would be a latent panic of the kind this playbook refuses `std::mutex` over;
// the difference is that here the panic replaces reading an indeterminate
// value, so the loud behaviour is the CORRECT one rather than a stand-in.  If a
// C++20 site ever appears, `Default::default()` becomes the right answer for it
// and this comment is the reason it is safe to switch.
// ---------------------------------------------------------------------------

template <typename T1> using t1 = std::atomic<T1>;

template <typename T1> std::atomic<T1> f1(T1 a0) {
  return std::atomic<T1>(a0);
}

// ---------------------------------------------------------------------------
// POSTFIX ++ on std::atomic<uint64_t>: `gId++`, the sole operation the two
// call sites perform.  dlOps.cpp:526 and dlOpsNew.cpp:844 both use it in a
// VALUE-CONSUMING position -- `std::to_string(gId++)` inside a name being
// built -- so the returned value is the thing the program depends on, not a
// discarded side effect.
//
// Postfix semantics: ADVANCE the receiver, RETURN the value it had BEFORE.
// Getting that backwards would be silent and would corrupt every generated
// name by one, so the target bodies use libcc2rs's PostfixInc (inc.rs), which
// is exactly `let copy = *self; *self = self.wrapping_add(1); copy`.  The
// wrapping is right: libc++ implements this as fetch_add(1) on an unsigned
// type, which wraps by definition rather than trapping.  PrefixInc, which
// libcc2rs also provides, would compile and answer one too high.
// ---------------------------------------------------------------------------

unsigned long f2(std::__atomic_base<unsigned long> &o, int a1) {
  return o.operator++(a1);
}

// ---------------------------------------------------------------------------
// The other three corners of the ++ table: PREFIX on both widths, and the
// `int` width, which is what `static std::atomic tag_id(0)` deduces at
// sys-arch-spec/progpatch/progpatch.cpp:67 (CTAD from the `0`).  That site is
// `std::to_string(++tag_id)` at progpatch.cpp:113 -- prefix, in a
// VALUE-CONSUMING position, so the returned value is again load-bearing and
// not a discarded side effect.  It was the ONLY translation gap in that whole
// TU, measured on its survey TSV.
//
// PREFIX vs POSTFIX here is not the ordinary iterator distinction, and the
// difference is visible in libc++'s own source
// (__atomic/atomic.h:185 and :189):
//
//     _Tp operator++(int) { return fetch_add(_Tp(1)); }        // OLD value
//     _Tp operator++()    { return fetch_add(_Tp(1)) + _Tp(1); } // NEW value
//
// Note what prefix does NOT do: it does not return a reference to the atomic.
// It cannot -- an atomic's value is only observable through a load -- so both
// forms return a PRVALUE of _Tp and differ only by one.  That is why f4/f5 are
// spelled with a `T` return and not a `T &`, and why the target bodies can use
// libcc2rs's PrefixInc (inc.rs, `*self = self.wrapping_add(1); *self`)
// directly: its "return the new value" contract is exactly the C++ one.  Using
// PostfixInc for a prefix site would compile and answer one too LOW, the
// mirror of the trap documented above f2.
//
// WRAPPING IS CORRECT FOR THE SIGNED WIDTH TOO, which is the one thing here
// that is not obvious.  Signed overflow on a plain `int` is UB, so a reflex
// reading says f3/f4 should not wrap.  But [atomics.types.int] specifies the
// atomic arithmetic operations to use two's-complement representation with
// "no undefined results", and libc++ implements them as fetch_add, so
// std::atomic<int> overflow is DEFINED to wrap where `int` overflow is not.
// wrapping_add therefore records the C++ semantics rather than relaxing them;
// a checked `+= 1` would introduce a panic C++ does not have.
//
// Each width needs its own rule for the reason spelled out at length above f2
// (the defaulted `bool` argument of std::__atomic_base only prints away for a
// concrete integral type).  `int` and `unsigned long` are the two widths that
// occur in dcg/ ddc/ dsc/ dbo/ sys-arch-spec/; a third is a two-line copy.
// ---------------------------------------------------------------------------

int f3(std::__atomic_base<int> &o, int a1) { return o.operator++(a1); }

int f4(std::__atomic_base<int> &o) { return o.operator++(); }

unsigned long f5(std::__atomic_base<unsigned long> &o) {
  return o.operator++();
}
