// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::thread::id (libc++ spells it std::__thread_id) -- opaque thread identity.
//
// MODEL: u64, NOT std::thread::ThreadId.  This is a deliberate, measured choice.
//
// WHAT THE CALL SITES ACTUALLY NEED.  The only uses in the port are in g3log:
//   external/g3log/logmessage.cpp:129   _call_thread_id(std::this_thread::get_id())
//   external/g3log/logmessage.cpp:149   _call_thread_id(other._call_thread_id)   // copy
//   external/g3log/logmessage.cpp:175   oss << _call_thread_id;                  // ostream
//   external/g3log/g3log/logmessage.hpp:105  swap(first._call_thread_id, ...)
// i.e. construct, copy, OSTREAM-INSERT.  No `<`, no std::map<thread::id, ...>,
// no std::hash use anywhere in the port.
//
// WHY NOT std::thread::ThreadId, the obvious counterpart.  TWO reasons, and the
// second is fatal for the one site that matters:
//   1. ORDERING.  C++ requires a total order on std::thread::id (operator< is
//      mandated, which is why it works as a std::map key).  Rust's ThreadId has
//      Eq + Hash but NOT Ord, so it could not serve a `<` or a map key if one
//      appeared.  Measured above: none appears, so this alone would not decide it.
//   2. DISPLAY.  `oss << id` is NOT rule-driven -- converter_lib.cpp:552
//      IsCallToOstream matches any operator<< returning basic_ostream, and
//      ConvertCallToOstream (converter.cpp:2073) emits a `write!(os, "{}", id)`
//      with a Display format.  ThreadId implements Debug but NOT Display, so a
//      ThreadId model would translate rc=0 and then fail to compile with E0277 at
//      every insertion site -- exactly the rc=0-is-not-compiles class.  There is
//      no rule key to fix it with, because the insertion never consults the rule
//      table.  u64 is Display.
// u64 is a SUPERSET of C++'s semantics (it is also ordered and arithmetic), so it
// can never make a correct program misbehave; it only fails to REJECT C++ that
// the standard rejects (e.g. `id1 + id2`).  That laxity is compile-time only.
//
// f1 -- std::this_thread::get_id().  Must be (a) stable across calls on one
// thread, (b) distinct between threads, (c) never 0, because a
// DEFAULT-CONSTRUCTED std::thread::id means "not any thread" and must compare
// unequal to every live thread's id (t1 below yields 0, matching libc++, whose
// __thread_id holds a zero __libcpp_thread_id for the not-a-thread state).
// std::thread::ThreadId's inner u64 is not stably observable on stable Rust
// (ThreadId::as_u64 is unstable), so f1 hashes the ThreadId with DefaultHasher --
// deterministic within a process (SipHash with zero keys), so (a) holds -- and
// ORs in the low bit for (c).
//
// NOT COVERED, deliberately: operator< / operator> / operator<= / operator>= on
// ids (no site uses them; a u64 model could serve them, but a key must be
// harvested from a real use, and inventing one would assert an ordering
// convention nothing measures), std::hash<std::thread::id>, and std::thread
// itself (a different row).

#include <thread>

using t1 = std::thread::id;

std::thread::id f1() { return std::this_thread::get_id(); }

// CALL FORM, UNQUALIFIED: an infix `a == b` records nothing silently, and a
// qualified std::operator==(a,b) aborts the preprocessor (No viable function).
bool f2(std::thread::id a, std::thread::id b) { return operator==(a, b); }
bool f3(std::thread::id a, std::thread::id b) { return operator!=(a, b); }

// The DEFAULT CONSTRUCTOR needs its own key: a type rule alone is not enough.
// Measured: with t1 only, `std::thread::id none;` lowers to
// `std___thread_id::new()` and the TU translates rc=0 then fails to compile with
// error[E0433] "cannot find module or crate std___thread_id".
std::thread::id f4() { return std::thread::id(); }
