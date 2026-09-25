// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::TrackingStatistic -- LLVM's STATISTIC counter, which is also what
// mlir::Pass::Statistic is (MLIR typedefs it straight through).  In dt_src it
// arrives via TableGen: dcc/src/Transform/Sentient/Passes.td declares
// `Statistic<"loops_merged_count", ...>` and mlir-tblgen emits
//
//   ::mlir::Pass::Statistic loops_merged_count{this, "num-loop-pairs-merged", ...};
//
// as a protected member of the generated pass base (gen-inc/.../Passes.h.inc:1170).
//
// WHY THE DECLARATION BELOW IS LOCAL AND NOT #include <llvm/ADT/Statistic.h>
// ---------------------------------------------------------------------------
// The same reason as rules/raw_ostream, rules/stringref and rules/twine, whose
// comments state it at length: cpp-rule-preprocessor compiles this file with a
// fixed flag set, and reaching LLVM's headers would need an absolute -I into
// whatever LLVM tree the target project happens to have built, which would make
// `ninja` in this repo fail for anyone without that tree.  So the signatures
// LLVM declares are restated here.
//
// A rule matches on the SIGNATURE STRING, so a restatement matches iff it
// agrees with LLVM exactly.  Both signatures below were read back out of a real
// translation with `cpp2rust --verbose`, which prints `search expr <exact
// resolved signature>` -- not copied out of the header and hoped over:
//
//     const llvm::TrackingStatistic & llvm::TrackingStatistic::operator++()
//     unsigned long llvm::TrackingStatistic::operator++(int)
//
// The field list matters even though no rule names a field: the class layout is
// what makes the two operator++ overloads resolve at all.  It is restated from
// llvm/ADT/Statistic.h as of LLVM 22.1.3.  If LLVM ever changes one of these
// signatures the rules silently stop matching, so re-check on an upgrade.
//
// MODEL: the counter IS its u64 value.
// ------------------------------------
// TrackingStatistic is a counter plus three static description strings plus an
// `Initialized` flag.  Everything except the value exists to support
// `-stats` reporting at process exit: DebugType/Name/Desc are the labels
// printed in that report, and Initialized is how llvm registers the counter
// with the global statistics list the first time it is touched (that is what
// `init()` does, and it is why BOTH operator++ overloads call it).
//
// The port does not have that reporting machinery -- there is no
// PrintStatistics, no global registry, and nothing reads a Statistic back.
// Measured: across dcc/ ddc/ dsc/ dbo/ sys-arch-spec/ the ONLY operations
// performed on any Statistic are these four increments
// (LoopMerging.cpp:278 `++loops_merged_count`,
//  LoopSplittingAndUnrolling.cpp:1001/1011 `loops_split_count++` /
//  `loops_unroll_count++`, VectorRegisterInitialization.cpp:163
//  `vector_register_init_count++`).  Zero reads, zero compares, zero prints,
// zero `-stats` plumbing.  So the labels have no observable consumer and
// carrying them would be representing state nothing can look at.
//
// That makes u64 the exact representation rather than an approximation, on the
// same footing as rules/atomic's std::atomic<T> -> T: every operation in scope
// is a read-modify-write on a private counter, and the parts being dropped are
// unreachable.  If a `-stats` path is ever ported the model must grow to a
// record with the three labels, and THIS PARAGRAPH is the trigger for that:
// the moment anything reads a counter's Name or Desc, u64 stops being enough.
//
// WHAT STAYS LOUD.  No rule is written for operator=, operator+=, operator-=,
// operator--, getValue() or the implicit `operator uint64_t()`.  None occurs in
// scope, and each would need its own judgement call (operator+= has a `if (V ==
// 0) return *this` early-out; the implicit conversion is how a Statistic gets
// READ, which is exactly the case this model cannot serve).  They abort loudly
// at converter.cpp naming the operation rather than answering from a
// representation that was chosen for increments only.
//
// PREFIX vs POSTFIX, and why f1 cannot return the receiver.
// ---------------------------------------------------------
// LLVM's own bodies (Statistic.h:78-86):
//
//     const TrackingStatistic &operator++() { Value.fetch_add(1, relaxed); return init(); }
//     uint64_t operator++(int)             { init(); return Value.fetch_add(1, relaxed); }
//
// so prefix returns the OBJECT (by const reference) and postfix returns the
// OLD VALUE.  Getting postfix backwards would be silent -- it compiles and
// answers one too high -- so f2 uses libcc2rs's PostfixInc, which is exactly
// `let copy = *self; *self = self.wrapping_add(1); copy`.  wrapping is right:
// fetch_add on an unsigned type wraps by definition.
//
// f1 is spelled to return the POINTER to the counter, not the counter's value.
// The playbook's rule for a reference-in/reference-out rule is that it must be
// spelled as the pointer (`*mut T1` / `Ptr<T1>`), and that is what the C++
// `const TrackingStatistic &` is.  All four real sites DISCARD the result --
// `++loops_merged_count;` as a statement -- so the returned handle is never
// consumed; spelling it as the pointer keeps the rule honest for a site that
// did consume it rather than quietly narrowing prefix to "return unit".
// ---------------------------------------------------------------------------

#include <cstdint>

namespace llvm {

class TrackingStatistic {
public:
  const char *const DebugType;
  const char *const Name;
  const char *const Desc;
  std::uint64_t Value;
  bool Initialized;

  const TrackingStatistic &operator++();
  std::uint64_t operator++(int);
};

} // namespace llvm

using t1 = llvm::TrackingStatistic;

const llvm::TrackingStatistic &f1(llvm::TrackingStatistic &o) {
  return o.operator++();
}

std::uint64_t f2(llvm::TrackingStatistic &o, int a1) {
  return o.operator++(a1);
}
