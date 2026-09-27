// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// `std::bitset<N>` -> `Vec<bool>`, one Rust bool per bit.  Queue row g003.
//
// THE SIZE IS UNRECOVERABLE.  `mapper.cpp`'s normalizeTranslationRule rewrites
// `\b\d+\b` -> `_` on BOTH the recording and the search side, so every
// instantiation of `std::bitset<N>` collapses to the single key
// `std::bitset<_>`; N is erased and a model cannot see it.  That is why the
// numbers below are spelled `64`: the literal normalises to `_` and one key
// therefore covers every N (same mechanism as rules/stringref's
// `StringLiteral(const char (&)[_])`).  NO per-N family is possible.
//
// `rules/array` faces the same problem and does NOT solve it: its `t1` drops
// the extent entirely and maps to `Vec<T1>` whose default is EMPTY, so
// `array::size()` in that model answers 0.  There is no AST-derived extent
// anywhere in this tree.  The model here is instead a GROW-ON-DEMAND
// `Vec<bool>`: bits above the vector's length are absent, which reads
// identically to a zero bit, and a write grows the vector first.  Every member
// whose answer depends on N is therefore OMITTED rather than guessed:
//   size(), all(), the no-argument set()/reset()/flip(), to_string(),
//   operator<<  (needs exactly N characters; the senulator call sites print
//   `std::bitset<32>(x)`, so they stay unsupported), and the bitwise
//   operators and ==/!= (two Vec<bool> of different grown lengths).
// `operator[]` is OMITTED DELIBERATELY: it returns the assignable proxy
// `std::bitset<N>::reference`, so `bs[3] = true` is a WRITE THROUGH A PROXY.
// Mapping it to a plain `bool` read would make that assignment silently do
// nothing, so it is left out to abort loudly instead -- exactly the choice
// made for `llvm::BitVector::reference` at rules/mlir t79.
//
// DESTRUCTOR TEST: `std::bitset` has no user-declared destructor (it is a
// trivial aggregate over `__storage`), so a plain Vec payload owes no Drop.
//
// Documented precondition on f2/f6: the integer value fits in N bits.  C++
// truncates a wider value to N on construction and `to_ulong` THROWS when the
// value does not fit `unsigned long`; neither can be expressed without N.
// Every corpus site satisfies it -- `std::bitset<32>(uint32)`,
// `collBitset b(rank)`, `collBitset bits = comm_size`.

#include <bitset>
#include <cstddef>

using t1 = std::bitset<64>;

// `collBitset everyone;` -- the default constructor, all bits zero.
std::bitset<64> f1() { return std::bitset<64>(); }

// `collBitset b(rank);` / `std::bitset<32>(z_int)` / `collBitset bits = n;`
std::bitset<64> f2(unsigned long long o) { return std::bitset<64>(o); }

// `collBitset dest = b;` -- the copy constructor.
std::bitset<64> f3(const std::bitset<64> &o) { return std::bitset<64>(o); }

// `everyone.set(i)` -- set(size_t, bool = true); the bool is defaulted at
// every corpus call site, which resolves to this same FunctionDecl.
std::bitset<64> &f4(std::bitset<64> &o, std::size_t p, bool v) {
  return o.set(p, v);
}

// `bits.count()`
std::size_t f5(const std::bitset<64> &o) { return o.count(); }

// `dest.to_ulong()`
unsigned long f6(const std::bitset<64> &o) { return o.to_ulong(); }

// `o.test(p)` -- the bounds-checked READ.  This is the read path; the proxy
// `operator[]` is not modelled.
bool f7(const std::bitset<64> &o, std::size_t p) { return o.test(p); }

// `o.reset(p)` -- reset(size_t).
std::bitset<64> &f8(std::bitset<64> &o, std::size_t p) { return o.reset(p); }

bool f9(const std::bitset<64> &o) { return o.any(); }

bool f10(const std::bitset<64> &o) { return o.none(); }
