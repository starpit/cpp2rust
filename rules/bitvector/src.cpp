// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::BitVector.
//
// WHY THE DECLARATION BELOW IS LOCAL AND NOT #include <llvm/ADT/BitVector.h>
// ---------------------------------------------------------------------------
// Same reason as rules/densemap, rules/stringref, rules/raw_ostream and
// rules/twine: cpp-rule-preprocessor compiles this file with a fixed flag set,
// and the only flags that would reach LLVM's headers are absolute -I paths into
// whatever LLVM tree the target project happens to have built.  So the
// signatures LLVM declares are restated here instead.  A rule matches on a
// SIGNATURE STRING, so a restatement matches iff it agrees with LLVM exactly;
// everything below was read out of
// llvm/ADT/BitVector.h (LLVM 22.1.3) lines 112-620.
//
// ===========================================================================
// REPRESENTATION: llvm::BitVector -> Vec<bool>, index i == bit i.
// ===========================================================================
// The alternative considered was C++'s own layout, a packed Vec<u64> plus a
// length.  It was rejected: the packed form buys only the memory footprint,
// which no probe can observe and which this port does not preserve anywhere
// else, and it pays for that by routing every single method through shifting
// and masking -- twenty chances to be silently wrong instead of zero.  It
// would only be FORCED if the target scope reached the words directly
// (getData(), the BitWord-taking ctor, operator>>=/<<= on whole words); a
// sweep of dt_src finds none of those -- the entire surface in use is
// ctor(n[,v]), copy-ctor, set()/set(i), test(i), count(), size(), resize(n),
// find_first()/find_next(i) -- so the simple representation is exact for it.
//
// Vec<bool> matches BitVector's semantics exactly on the used surface, with
// one consequence worth stating: Vec<bool>'s length IS Size, so unlike
// rules/bitset there is no "materialized prefix" approximation here.  size(),
// resize() and all() are therefore FAITHFUL, which they cannot be for
// std::bitset<N>.
//
// SEMANTICS THAT ARE EASY TO CONFLATE, each checked against the header:
//   * set()/reset()/flip() with NO argument act on ALL bits; the one-argument
//     forms act on one bit.  These are distinct overloads in LLVM (not a
//     defaulted argument), so they are distinct rules here: f7/f8, f9/f10,
//     f11/f12.  Conflating them compiles and is silently wrong.
//   * set(Idx)/reset(Idx)/flip(Idx)/test(Idx) ASSERT Idx < Size; they do NOT
//     grow (BitVector.h:376-381, 480-483).  The targets index directly, so an
//     out-of-range index panics rather than silently extending -- deliberately
//     unlike rules/bitset, whose lazy representation has to grow.
//   * count() is a POPCOUNT (BitVector.h:181), not a length.
//   * on an EMPTY vector, C++ gives any()==false (any_of over zero words),
//     none()==true, all()==true (BitVector.h:189-208: the loop and the
//     remainder test are both skipped, so it falls through to `return true`).
//     Rust's iterator any()/all() on an empty slice give exactly false/true,
//     so no special case is needed -- but the probe pins it anyway.
//   * resize(N, t) fills a GROWING region with t and defaults to false
//     (BitVector.h:360); it truncates when shrinking.  Vec::resize is the same
//     operation.
//
// WHAT IS DELIBERATELY LEFT UNMAPPED, so it stays a loud abort:
//   * the write proxy.  operator[] is TWO functions: a const one returning
//     bool (BitVector.h:468, mapped as f6) and a non-const one returning
//     `BitVector::reference` (BitVector.h:463), a proxy class whose
//     operator=(bool) does the write.  Only the const/read one is mapped;
//     `bv[i] = v` needs the proxy type modelled and is not attempted.  No
//     dt_src call site writes through an index (they all use set/reset).
//   * the set-bits iterators (set_bits_begin/set_bits_end/set_bits), used at
//     dr5/src/Analysis/DPSConversionHelper.cpp:84.  An iterator needs the
//     iterator-protocol modelling rules/densemap documents, and find_first/
//     find_next already cover the same traversal for the other call sites.
//   * range set(I,E)/reset(I,E), back(), pop_back(), anyCommon, subsetOf,
//     test(BitVector), reset(BitVector), the bitwise compound operators and
//     the shifts: nothing in the target scope calls them, and breadth nobody
//     exercises is how a broad wrong module gets built.
//
// ENGINE NOTE -- the two defaulted arguments.  `BitVector(unsigned s, bool t =
// false)` and `resize(unsigned N, bool t = false)` are single declarations with
// a default, so `BitVector v(n)` and `v.resize(n)` reach the rule with a
// CXXDefaultArgExpr for the bool.  rules/bitset/src.cpp records that a rule
// body referencing a NON-POINTER CXXDefaultArgExpr aborts the converter at
// converter/converter.cpp:1634 ("computed_expr_type_ not set").  Both spellings
// are exercised by the probe; if the one-argument spelling aborts, that is the
// pre-existing engine bug and not a gap in this module.

#include <cstddef>
// <vector>/<map>/<string> are pulled in for the same reason rules/bitset does:
// a real TU always has a larger header set, and libc++ spells free operators
// differently depending on it.
#include <map>
#include <string>
#include <vector>

namespace llvm {

class BitVector {
public:
  typedef unsigned size_type;

  // --- construction --------------------------------------------------------
  BitVector();
  explicit BitVector(unsigned s, bool t = false);
  BitVector(const BitVector &RHS);
  BitVector &operator=(const BitVector &RHS);

  // --- observers -----------------------------------------------------------
  bool empty() const;
  size_type size() const;
  size_type count() const;
  bool any() const;
  bool all() const;
  bool none() const;
  bool test(unsigned Idx) const;
  bool operator[](unsigned Idx) const;

  // --- search --------------------------------------------------------------
  int find_first() const;
  int find_next(unsigned Prev) const;

  // --- mutation ------------------------------------------------------------
  void clear();
  void resize(unsigned N, bool t = false);
  BitVector &set();
  BitVector &set(unsigned Idx);
  BitVector &reset();
  BitVector &reset(unsigned Idx);
  BitVector &flip();
  BitVector &flip(unsigned Idx);
  void push_back(bool Val);

  // --- comparison ----------------------------------------------------------
  bool operator==(const BitVector &RHS) const;
  bool operator!=(const BitVector &RHS) const;
};

} // namespace llvm

using t1 = llvm::BitVector;

// --- construction ----------------------------------------------------------

llvm::BitVector f1() { return llvm::BitVector(); }

llvm::BitVector f2(unsigned s, bool t) { return llvm::BitVector(s, t); }

llvm::BitVector f3(const llvm::BitVector &a0) { return llvm::BitVector(a0); }

llvm::BitVector &f4(llvm::BitVector &dst, const llvm::BitVector &src) {
  return dst.operator=(src);
}

// --- observers -------------------------------------------------------------

bool f5(const llvm::BitVector &o) { return o.empty(); }

unsigned f6(const llvm::BitVector &o) { return o.size(); }

// POPCOUNT, not a length.
unsigned f7(const llvm::BitVector &o) { return o.count(); }

bool f8(const llvm::BitVector &o) { return o.any(); }

bool f9(const llvm::BitVector &o) { return o.all(); }

bool f10(const llvm::BitVector &o) { return o.none(); }

bool f11(const llvm::BitVector &o, unsigned a1) { return o.test(a1); }

// The READ (const) operator[] only; the write proxy is not modelled.
bool f12(const llvm::BitVector &o, unsigned a1) { return o.operator[](a1); }

// --- search ----------------------------------------------------------------
// Both return -1 when there is no such bit.

int f13(const llvm::BitVector &o) { return o.find_first(); }

int f14(const llvm::BitVector &o, unsigned a1) { return o.find_next(a1); }

// --- mutation --------------------------------------------------------------
// The `BitVector &` these return is NOT modelled (the target bodies evaluate
// to unit), exactly as rules/bitset f4/f20: they are supported as STATEMENTS,
// and `x = v.set(i)` is not.  Every dt_src call site is a statement.

void f15(llvm::BitVector &o) { return o.clear(); }

void f16(llvm::BitVector &o, unsigned a1, bool a2) {
  return o.resize(a1, a2);
}

// NO ARGUMENT: sets ALL bits.
llvm::BitVector &f17(llvm::BitVector &o) { return o.set(); }

// One argument: sets ONE bit.  Asserts a1 < size() in C++.
llvm::BitVector &f18(llvm::BitVector &o, unsigned a1) { return o.set(a1); }

llvm::BitVector &f19(llvm::BitVector &o) { return o.reset(); }

llvm::BitVector &f20(llvm::BitVector &o, unsigned a1) { return o.reset(a1); }

llvm::BitVector &f21(llvm::BitVector &o) { return o.flip(); }

llvm::BitVector &f22(llvm::BitVector &o, unsigned a1) { return o.flip(a1); }

void f23(llvm::BitVector &o, bool a1) { return o.push_back(a1); }

// --- comparison ------------------------------------------------------------

bool f24(const llvm::BitVector &a, const llvm::BitVector &b) {
  return a.operator==(b);
}

bool f25(const llvm::BitVector &a, const llvm::BitVector &b) {
  return a.operator!=(b);
}
