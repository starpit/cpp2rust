// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::SmallSet<T, N, C> -- llvm/ADT/SmallSet.h:133.
//
// THE KEY IS 2-ARY, NOT 3-ARY, AND THE ABORT TEXT SHOWS THE 3-ARY FORM.
// ---------------------------------------------------------------------
// SmallSet.h:132-134 declares
//     template <typename T, unsigned N, typename C = std::less<T>>
//     class SmallSet;
// so every instantiation carries THREE arguments -- but `C` is DEFAULTED and the
// converter's SEARCH side prints with SuppressDefaultTemplateArgs, so the string
// the lookup is actually performed with has only TWO.  Read verbatim out of the
// census abort on dcc/src/Transform/Sentient/Analyses/PropagationAnalysis.cpp
// (9 TUs gate here, all of them on this one string):
//
//   unsupported system type has no rule:
//     `llvm::SmallSet<std::pair<mlir::Operation *, std::optional<int>>, _,
//                     std::less<std::pair<mlir::Operation *, std::optional<int>>>>`
//   rule key: searched as:
//     llvm::SmallSet<std::pair<mlir::Operation *, std::optional<int>>, _>
//   from decl (NOT a key -- canonicalised, defaulted args kept):
//     llvm::SmallSet<std::pair<mlir::Operation *, std::optional<int>>, _,
//                    std::less<std::pair<mlir::Operation *, std::optional<int>>>>
//
// ⛔ THE BACKTICKED TYPE AND THE `from decl` TAIL ARE THE SAME 3-ARY STRING AND
// NEITHER IS A KEY.  A rule written as `llvm::SmallSet<T1, _, std::less<T1>>`
// would record cleanly, pass every structural check, and be DEAD at all 9 TUs.
// The `searched as:` line is the only key-shaped text in that message.
//
// `N` RECORDS AS `_`.  Mapper::ToString ends every spelling -- rules side and
// search side alike -- with normalizeTranslationRule, whose sole rewrite is
// `\b\d+\b` -> `_` (mapper.cpp:1830).  So the `4` written below and the `8`,
// `4`, `kMaxNumLocales` and `64` the corpus writes ALL collapse onto one key,
// exactly as rules/array (`std::array<T1, _>`) and rules/smallvector
// (`llvm::SmallVector<T1, _>`) already rely on.  ONE key covers every inline
// capacity, which is correct here because N IS AN OPTIMISATION, NOT SEMANTICS
// for everything keyed below -- see the iteration note, which is the one place
// where that stops being true and is therefore the one place left unkeyed.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/ADT/SmallSet.h>
// --------------------------------------------------------------------------
// Same reason as rules/smallvector, rules/setvector, rules/stringref,
// rules/raw_ostream and rules/twine: cpp-rule-preprocessor compiles this file
// with a fixed flag set, and the only flags that would reach LLVM's headers are
// absolute -I paths into whatever LLVM tree the target project happens to have
// built.  So the signatures are restated here, faithfully, from
// LLVM-22.1.3's SmallSet.h.
//
// THE MODEL: `Vec<T1>` WITH LINEAR SCAN, NOT `BTreeSet` AND NOT `HashSet`.
// ----------------------------------------------------------------------
// SmallSet is a SMALL-SIZE-OPTIMISED set: SmallSet.h:136-138 holds a
// `SmallVector<T, N> Vector` and a `std::set<T, C> Set`, `isSmall()` is
// `Set.empty()`, and below N every lookup is a LINEAR SCAN of the vector
// (`vfind`).  A `Vec<T1>` with linear `contains` is therefore a
// STRUCTURALLY EXACT model of the mode the type exists to provide, and the
// uniqueness invariant is enforced by the rule bodies (insert checks first),
// not by the container.
//
// The two alternatives were rejected for MEASURED reasons, not aesthetic ones:
//
//  * `BTreeSet<T1>` requires `Ord` on the element and IMPOSES A TOTAL ORDER the
//    C++ side does not have below N.  The gating element type is
//    `std::pair<mlir::Operation *, std::optional<int>>` -- a pair of a POINTER
//    and an OPTIONAL -- and `mlir::Operation *`'s element model and its
//    `Hash`/`Eq` are recorded UNSETTLED in this queue, with a standing block on
//    exactly that question.  A BTreeSet model would make this module depend on
//    resolving that block; the Vec model needs `PartialEq` and nothing else, so
//    it does not.
//  * `HashSet<T1>` requires `Hash` + `Eq`, i.e. the same block, plus it throws
//    away order entirely.
//
// ⭐ SO THE ELEMENT MODEL DECISION, stated deliberately as rules/set's existing
// split demands: the element is keyed as PLAIN `T1`, the scalar/pointer arm of
// rules/set's rule, NOT the `Value<...>` arm rules/set uses for a mapped
// container element.  `std::pair` and `std::optional` are both mapped
// containers, so the `Value<...>` arm would be the reflex choice -- but it is
// the WRONG one here, because the Vec model never needs to own, hash or order
// the element; it only needs to COMPARE it.  Writing the rule generically in
// `T1` defers the element's representation to whatever rules/pair and
// rules/optional already decided for it, which is the only answer that cannot
// go stale when that block is resolved.
//
// ⛔ WHAT IS DELIBERATELY LEFT OUT: `begin()`, `end()`, and every operation on
// `SmallSetIterator` (`operator++`, `operator*`, `operator==`).  THIS IS THE ONE
// PLACE WHERE N IS SEMANTICS AND NOT AN OPTIMISATION, so it is the one place a
// single collapsed `_` key cannot be made correct:
//
//     SmallSet.h:215-225   begin()/end() return `{Vector.begin()}` when
//                          isSmall() and `{Set.begin()}` otherwise.
//
// i.e. C++ SmallSet iterates in INSERTION order while size <= N and in SORTED
// order once size > N.  A `Vec` iterates in insertion order ALWAYS.  Since N
// collapses to `_`, one body would have to serve both modes, and it would be
// silently right below N and silently wrong above it -- and the wrongness is an
// ORDER, which compiles and produces a different program.  That is the
// collapsed-non-type-argument trap in its SmallSet flavour, and the same class
// of refusal as rules/chrono's ratio.
//
// THE OBSERVERS, and they are why this is not hypothetical:
//  * dcc/src/Transform/Sentient/ScalarCopyInsertionForSymbols.cpp:302,347,517
//    `for (SentientRegType locale : symbolic_locales_)`, where
//    `symbolic_locales_` is `SmallSet<SentientRegType, kMaxNumLocales>` and
//    :77 defines `kMaxNumLocales = getMaxEnumValForSentientRegType()`.  ⭐ THIS
//    SITE IS PROVABLY SAFE: the set holds UNIQUE `SentientRegType` values and N
//    is that enum's maximum value, so its size can NEVER exceed N and it is
//    ALWAYS in small mode.  Insertion order is EXACT here.
//  * dcc/src/Transform/Sentient/OldRegisterInitialization.cpp:769
//    `for (Operation *op : to_be_erased_) op->erase();` where `to_be_erased_`
//    is `SmallSet<mlir::Operation *, 8>` populated from a pass walk.  ⛔ THIS
//    SITE IS NOT SAFE: it CAN exceed 8, in which case C++ erases in
//    POINTER-SORTED order and a Vec model would erase in insertion order.
//    `Operation::erase()` order is observable when one op is an operand or a
//    parent of another, so this is a behaviour change that COMPILES.
//
// Because `_` cannot distinguish the provably-safe site from the unsafe one,
// and because the unsafe one is reached through the SAME generic key, iteration
// is NOT keyed and every iteration site FAILS LOUDLY at translate time.  That
// is the required outcome, and it deliberately leaves
// ScalarCopyInsertionForSymbols.cpp gated -- on an HONEST gate naming the
// iterator -- rather than translating it on an order this module cannot
// guarantee.
//
// t2 (`SmallSetIterator`) IS keyed even so, because it is UNAVOIDABLE:
// `insert` returns `std::pair<const_iterator, bool>` (SmallSet.h:183), so the
// iterator needs a Rust type before `insert` -- the dominant member, 6 of the
// 8 corpus call sites -- can be expressed at all.
// ⭐ AND IT IS NOT THE `std::hash<int>` VIOLATION (a type key with no method
// key, which bypasses the loud path).  That rule bites when a member of the
// keyed type IS READ; here NOTHING is ever read off this iterator.  Measured
// over every SmallSet site in dt_src -- all 8 of them:
//     PropagationAnalysis.cpp:1168,1277   `.insert(p).second`     -> .second
//     PropagationAnalysis.cpp:1014        `.insert(p);`            -> discarded
//     OldRegisterInitialization.cpp:1031,1179,1198  `.insert(x);`  -> discarded
//     AddressPinningAndToggle.cpp:1445    `.insert(ev);`           -> discarded
//     ScalarCopyInsertionForSymbols.cpp:268 `.insert(locale);`     -> discarded
// ZERO sites read `.first`.  So the iterator is produced and dropped, it has no
// live member surface to miss, and `*const T1` is an exact model of "a
// borrowed handle on an element we never look at".  If a `.first` use ever
// appears it hits an unkeyed member and fails loudly, which is correct.

#include <cstddef>
#include <functional>
#include <set>
#include <utility>

namespace llvm {

// Restated from SmallSet.h:32-33.  THREE parameters, NONE defaulted, so unlike
// SmallSet itself the search side has no default to suppress and the key keeps
// all three.  `C` is written as a free generic `T2` rather than as
// `std::less<T1>` so the key cannot be tied to one comparator spelling.
template <typename T, unsigned N, typename C> class SmallSetIterator {
public:
  const T &operator*() const;
  SmallSetIterator &operator++();
  bool operator==(const SmallSetIterator &RHS) const;
};

// Restated from SmallSet.h:132-134.
template <typename T, unsigned N, typename C = std::less<T>> class SmallSet {
public:
  using key_type = T;
  using size_type = size_t;
  using value_type = T;
  using const_iterator = SmallSetIterator<T, N, C>;

  SmallSet();
  SmallSet(const SmallSet &);

  bool empty() const;
  size_type size() const;
  size_type count(const T &V) const;
  bool contains(const T &V) const;
  std::pair<const_iterator, bool> insert(const T &V);
  std::pair<const_iterator, bool> insert(T &&V);
  bool erase(const T &V);
  void clear();
};

} // namespace llvm

// --- TYPES -----------------------------------------------------------------
// t1 is written with TWO arguments so the recorded key is the 2-ary
// `llvm::SmallSet<T1, _>` -- the `searched as:` spelling.  `4` is arbitrary:
// it normalizes to `_`.
template <typename T1> using t1 = llvm::SmallSet<T1, 4>;

// t2 is the insert-return iterator.  Written 3-ary because SmallSetIterator
// defaults nothing.
//
// ⛔⛔ THE COMPARATOR IS PINNED TO `std::less<T1>`, NOT LEFT AS A FREE `T2`, AND
// THAT IS THE WHOLE OF ROW g120/g121/g122.  MEASURED, 2026-09-29, on
// pin/cpp2rust a0b0a70761208ff7559492f8a5fd8cca + pin/ir.v45 (98 modules),
// -model=refcount, dcc/src/Transform/Sentient/Analyses/ExpressionEvaluatorUtils.cpp:
//
//   LLVM ERROR: unsupported unmapped type `std::less<long>` has no model in
//   types_, while mapping `std::pair<llvm::SmallSetIterator<long, _,
//   std::less<long>>, bool>`
//   LEAF record `std::less` declared at
//   toolchain/libcxx/__functional/operations.h:356:8 [system type]: type
//   `std::less<long>` is not present in types_ (rule key `std::less`)
//
// A FREE `T2` MATCHES THE ITERATOR AND THEN DEMANDS A types_ MODEL FOR WHATEVER
// IS BOUND TO IT.  `SmallSet`'s `C` is DEFAULTED, so what is bound is always
// `std::less<T>` -- a type this port deliberately leaves UNKEYED (rules/hash's
// standing note: `std::less<int>` has no type key and ABORTS LOUDLY, while
// `std::hash<int>` had one with no `operator()` key and silently emitted `0(v)`
// at 427 sites in 190 TUs).  So the free `T2` converted a deliberate loud
// refusal into a GATE ON 10 TUs that has nothing to do with comparing anything.
//
// ⭐ PINNING IS CORRECT AND COMPLETE FOR THIS CORPUS, not a narrowing: all 15
// `SmallSet<` instantiations in dt_src are 2-ary (`git grep -n "SmallSet<"`,
// 15 of 15), so `C` is the default at every one of them.  A site that ever
// passes an explicit comparator no longer matches this key and FAILS LOUDLY,
// which is the required outcome -- it is exactly the site where the comparator
// would be semantics rather than the defaulted `<`.
//
// ⛔ AND THIS IS WHY IT IS NOT FIXED BY KEYING `std::less` INSTEAD.  A `()`
// model for an empty comparator functor COLLAPSES DISTINCT COMPARATORS ONTO ONE
// VALUE, and the corpus has a site where that is observable:
// senulator/pcfg2compute.cpp:463,465 store `std::less<float>()` and
// `std::less_equal<float>()` as VALUES in the same table, keyed by operator, so
// a unit model makes the two table entries identical -- silent wrongness that
// compiles.  Pinning here needs no `std::less` model at all and leaves that
// site loud.
template <typename T1>
using t2 = llvm::SmallSetIterator<T1, 4, std::less<T1>>;

// --- MEMBERS ---------------------------------------------------------------
template <typename T1> bool f1(const llvm::SmallSet<T1, 4> &o) {
  return o.empty();
}

template <typename T1> std::size_t f2(const llvm::SmallSet<T1, 4> &o) {
  return o.size();
}

template <typename T1>
std::size_t f3(const llvm::SmallSet<T1, 4> &o, const T1 &k) {
  return o.count(k);
}

template <typename T1>
bool f4(const llvm::SmallSet<T1, 4> &o, const T1 &k) {
  return o.contains(k);
}

template <typename T1>
std::pair<typename llvm::SmallSet<T1, 4>::const_iterator, bool>
f5(llvm::SmallSet<T1, 4> &o, const T1 &k) {
  return o.insert(k);
}

template <typename T1>
std::pair<typename llvm::SmallSet<T1, 4>::const_iterator, bool>
f6(llvm::SmallSet<T1, 4> &o, T1 &&k) {
  return o.insert(std::move(k));
}

template <typename T1> bool f7(llvm::SmallSet<T1, 4> &o, const T1 &k) {
  return o.erase(k);
}

template <typename T1> void f8(llvm::SmallSet<T1, 4> &o) { return o.clear(); }

template <typename T1> llvm::SmallSet<T1, 4> f9() {
  return llvm::SmallSet<T1, 4>();
}

template <typename T1>
llvm::SmallSet<T1, 4> f10(const llvm::SmallSet<T1, 4> &o) {
  return llvm::SmallSet<T1, 4>(o);
}
