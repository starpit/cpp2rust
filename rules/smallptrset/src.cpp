// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::SmallPtrSet<T, N> -- llvm/ADT/SmallPtrSet.h:527.
//
// ⭐⭐ THE MEMBERS ARE KEYED BY THE *DECLARING CLASS*, NOT BY THE RECEIVER'S TYPE.
// -----------------------------------------------------------------------------
// This is the fact that makes a flat `rules/smallset`-style restatement DEAD ON
// ARRIVAL for this type, and it is MEASURED, not inferred.  Mapper::ToString(
// const clang::Expr *) (mapper.cpp:3088) resolves a CallExpr through
// `CE->getDirectCallee()` and then ToString(NamedDecl) prints
// `func_decl->printQualifiedName(...)` -- the qualified name of the *method
// decl*, i.e. its DECLARING class.  SmallSet declares all of its own members, so
// rules/smallset never had to care; SmallPtrSet declares almost none of them.
//
// Read off a `-verbose` run of a probe whose receiver is a plain
// `llvm::SmallPtrSet<proj::Node *, 4>` (`grep -A1 'search expr'`, converter
// rc=0, so the log is complete -- a truncated -verbose log's silence is not
// evidence):
//
//   search expr std::pair<llvm::SmallPtrSetIterator<proj::Node *>, bool>
//               llvm::SmallPtrSetImpl<proj::Node *>::insert(proj::Node *)
//   search expr bool     llvm::SmallPtrSetImpl<proj::Node *>::erase(proj::Node *)
//   search expr bool     llvm::SmallPtrSetImpl<proj::Node *>::contains(const proj::Node *) const
//   search expr unsigned int llvm::SmallPtrSetImpl<proj::Node *>::count(const proj::Node *) const
//   search expr void     llvm::SmallPtrSet<proj::Node *, _>::SmallPtrSet()
//   search expr void     llvm::SmallPtrSet<proj::Node *, _>::SmallPtrSet(std::initializer_list<proj::Node *>)
//
// ⭐ Note every one of the four data members names `SmallPtrSetImpl` even though
// NO source line anywhere in dt_src ever spells that class.  Only the two
// CONSTRUCTORS carry the `SmallPtrSet<T1, _>` spelling, because a constructor is
// declared by the most-derived class.  So this module needs type keys for the
// WHOLE CHAIN it touches, exactly as rules/smallvector does for
// SmallVectorImpl / SmallVectorTemplateBase / SmallVectorTemplateCommon /
// SmallVectorBase.
//
// ⛔ NO CENSUS LOG CAN ANSWER THIS QUESTION: the *type* ask fires when the
// variable is declared and aborts the run, so a member ask is never reached and
// `SmallPtrSetImpl` appears in zero census logs.  It has to be probed.
//
// `N` RECORDS AS `_`.  Same mechanism as rules/smallset and rules/smallvector:
// normalizeTranslationRule rewrites `\b\d+\b` -> `_` (mapper.cpp:1830), so the
// `4` written below and the corpus's 1, 2, 4, 8, 16 and 32 all collapse onto the
// single key `llvm::SmallPtrSet<T1, _>` -- verified by readback and by the
// `-verbose` ask above, which is already `<proj::Node *, _>`.  Both template
// parameters are UN-DEFAULTED, so unlike rules/smallset there is no
// defaulted-argument suppression subtlety here and the backticked type in the
// abort, the `searched as:` line and the key all agree.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL.  Same reason as rules/smallset,
// rules/smallvector, rules/setvector, rules/stringref: cpp-rule-preprocessor
// compiles this file with a fixed flag set that cannot reach an LLVM tree, so
// the signatures are restated here, faithfully, from LLVM-22.1.3's
// SmallPtrSet.h (:56 SmallPtrSetImplBase, :333 SmallPtrSetIterator,
// :368 SmallPtrSetImpl, :527 SmallPtrSet).
//
// ⭐ `contains` AND `count` TAKE `ConstPtrType`, AND THAT FORCES A DIFFERENT
// PARAMETRISATION FROM EVERY OTHER MEMBER.  SmallPtrSetImpl declares
//     using ConstPtrType = typename add_const_past_pointer<PtrType>::type;
// which for `PtrType = mlir::Operation *` resolves to `const mlir::Operation *`
// -- confirmed in the ask strings above.  A rule written `contains(const T1)`
// with `T1 = mlir::Operation *` would record `contains(mlir::Operation *const)`
// (top-level const on the POINTER) and be DEAD, and a rule spelling
// `typename add_const_past_pointer<T1>::type` would record that DEPENDENT string
// literally and also be dead.  The only form that can produce the measured
// spelling is to parametrise on the POINTEE: the receiver is written
// `llvm::SmallPtrSetImpl<T1 *>` and the parameter `const T1 *`, so `T1` binds to
// `mlir::Operation` rather than to `mlir::Operation *`.  `insert` and `erase`
// take plain `PtrType`, so they keep the whole-element `T1` form.
//
// THE MODEL: `Vec<T1>` WITH LINEAR SCAN -- the same model, and the same reasons,
// as rules/smallset.  SmallPtrSet's small mode IS a linear scan over an inline
// array (SmallPtrSet.h:529-531 says so in terms: "In small mode SmallPtrSet uses
// linear search for the elements").  A `Vec` needs only `PartialEq`, never `Ord`
// or `Hash`, so this module places no requirement on `mlir::Operation *`'s
// still-unsettled element model -- and because the element is keyed as plain
// `T1`, its representation is deferred to whatever rules/mlir decides for it.
//
// ⛔ WHAT IS DELIBERATELY LEFT OUT, AND WHY LEAVING IT OUT IS SAFE HERE.
// ---------------------------------------------------------------------
// `begin()`, `end()`, `find()`, every operation on SmallPtrSetIterator, and the
// SmallPtrSetImplBase members `size()`/`empty()`/`capacity()`/`clear()`/
// `reserve()`.
//
// ITERATION IS THE LOAD-BEARING REFUSAL, and it is STRICTLY WORSE here than the
// one rules/smallset already makes.  SmallSet is at least insertion-ordered
// below N; SmallPtrSet has NO ordered mode at all -- it is a
// quadratically-probed hash table over pointer VALUES, so its iteration order is
// derived from the pointer bits and is unspecified in BOTH modes.  A `Vec`
// iterates in insertion order always.  Since `N` collapses to `_` one body would
// have to serve every site, and the wrongness would be an ORDER, which compiles.
// THE OBSERVER, and it is why this is not hypothetical:
//
//   dcc/src/Transform/Sentient/RegisterInitialization/Transformer.cpp:57
//     for (Operation *op : ops_to_erase_) op->erase();
//
// `ops_to_erase_` is `llvm::SmallPtrSet<mlir::Operation *, 32>` (Transformer.h
// is not where it lives -- Transformer.cpp:83).  `Operation::erase()` order is
// OBSERVABLE whenever one erased op is an operand or a parent of another, so a
// Vec model would be a behaviour change that compiles.  This is byte-for-byte
// the site rules/smallset already refused for at
// OldRegisterInitialization.cpp:769.  Iteration therefore stays UNKEYED and
// Transformer.cpp stays gated -- on an HONEST gate naming the iterator instead
// of the container.  ⭐ That is the required outcome, not a failure.
//
// ⛔⛔ AND THE HAZARD THAT MAKES THAT CHOICE SAFE ONLY BECAUSE IT WAS CENSUSED:
// A TYPE KEY WITH NO MEMBER KEY IS STRICTLY WORSE THAN NO KEY AT ALL, because an
// unmapped MEMBER does not abort -- the converter emits the call textually, the
// `std::hash<int>` shape that silently emitted `0(v)` at 427 sites.  So the
// member set had to be sized against the corpus, not guessed.  Census over ALL
// 22 `SmallPtrSet<...>` declarations in dt_src (`git grep`, whole-file
// multi-line-aware scan for `<var>.`/`<var>->` and for a range-`for` over
// `<var>`, plus the one cross-file case -- `visited_` is declared in
// RedundantDefinitionEliminationTree.hpp:370 and used only in
// RedundantDefinitionEliminationTreeImpl.cpp, which a per-file scan misses):
//
//   insert    23   KEYED (f1)        14 discarded, 9 read as `.insert(p).second`
//   contains   9   KEYED (f6)
//   count      2   KEYED (f7)
//   erase      2   KEYED (f5)
//   range-for  6   UNKEYED -> LOUD at translate time (the refusal above)
//   find       2   UNKEYED -> `Vec` has no `find`  -> E0599 at rustc
//   end        2   UNKEYED -> `Vec` has no `end`   -> E0599 at rustc
//   begin      1   UNKEYED -> `Vec` has no `begin` -> E0599 at rustc
//   empty      1   UNKEYED -> `Vec` has no `empty` -> E0599 at rustc
//   size/clear/capacity/reserve   ZERO SITES
//
// ⭐ So there is NO silent-wrongness case in the residue: every unkeyed member
// that the corpus actually calls is spelled differently in Rust than its `Vec`
// counterpart, so it fails LOUDLY -- at translate time for iteration, at rustc
// for the other four.  `Vec::insert` and `Vec::contains` DO exist, which is
// exactly why `insert` and `contains` had to be keyed rather than left to the
// textual path: `insert` is the one member a Vec would silently accept with a
// completely different meaning (`Vec::insert(index, value)`).
//
// ⭐ AND THIS IS WHY NO `llvm::SmallPtrSetImplBase` TYPE KEY IS ADDED.  Its five
// members have ZERO call sites in the corpus, and it is the one class in the
// chain that is NOT a template -- so its key would be the bare
// `llvm::SmallPtrSetImplBase`, with no argument from which to build `Vec<T1>`,
// and its model would have to be a fabricated concrete Rust type.  Adding a
// fabricated model to buy coverage of zero sites is the wrong trade; if a
// `size()`/`empty()` site ever appears it fails loudly in rustc first.
//
// t3 (`SmallPtrSetIterator`) IS keyed even though no member of it is, and that
// is NOT the `std::hash<int>` violation -- for the same reason rules/smallset's
// t2 is not.  `insert` returns `std::pair<iterator, bool>`
// (SmallPtrSet.h:388), so the iterator needs a Rust type before `insert` -- the
// dominant member -- can be expressed at all.  The rule bites when a member of
// the keyed type IS READ; measured over all 23 `insert` sites in dt_src, NINE
// read `.second` and ZERO read `.first`, so the iterator is produced and
// dropped and `*const T1` is an exact model of "a borrowed handle on an element
// we never look at".  A future `.first` use hits an unkeyed member and fails
// loudly, which is correct.

#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace llvm {

// Restated from SmallPtrSet.h:333.  ONE parameter.  No member of it is keyed --
// see the header comment.
template <typename PtrTy> class SmallPtrSetIterator {
public:
  const PtrTy operator*() const;
  SmallPtrSetIterator &operator++();
  bool operator==(const SmallPtrSetIterator &RHS) const;
};

// Restated from SmallPtrSet.h:368.  The `add_const_past_pointer` indirection is
// NOT restated: it cannot be, because a dependent `typename
// add_const_past_pointer<T1>::type` would be recorded literally and be dead.
// `contains`/`count` are therefore declared on the POINTEE-parametrised
// specialisation below instead, and only the members taking plain `PtrType` are
// declared here.
template <typename PtrType> class SmallPtrSetImpl {
public:
  using iterator = SmallPtrSetIterator<PtrType>;
  using const_iterator = SmallPtrSetIterator<PtrType>;
  using value_type = PtrType;
  using size_type = unsigned;

  std::pair<iterator, bool> insert(PtrType Ptr);
  bool erase(PtrType Ptr);

  // ConstPtrType members.  Declared with the parameter spelled `const T *`
  // where the class argument is `T *`, which is the ONLY form that records the
  // measured `contains(const mlir::Operation *) const` spelling.  Valid C++
  // here because the rule bodies below instantiate this class with a pointer.
  bool contains(const typename std::remove_pointer<PtrType>::type *Ptr) const;
  size_type
  count(const typename std::remove_pointer<PtrType>::type *Ptr) const;
};

// Restated from SmallPtrSet.h:527.
template <class PtrType, unsigned SmallSize>
class SmallPtrSet : public SmallPtrSetImpl<PtrType> {
public:
  SmallPtrSet();
  SmallPtrSet(const SmallPtrSet &that);
  SmallPtrSet(std::initializer_list<PtrType> IL);
};

} // namespace llvm

// --- TYPES -----------------------------------------------------------------
// t1 is written 2-ary so the recorded key is `llvm::SmallPtrSet<T1, _>`, the
// `searched as:` spelling.  `4` is arbitrary: it normalizes to `_`.
template <typename T1> using t1 = llvm::SmallPtrSet<T1, 4>;

// t2 is the class that DECLARES every data member -- the key every member ask
// actually names.  Same model as t1: an upcast is the identity on a Vec.
template <typename T1> using t2 = llvm::SmallPtrSetImpl<T1>;

// t3 is the insert-return iterator.
template <typename T1> using t3 = llvm::SmallPtrSetIterator<T1>;

// --- MEMBERS ---------------------------------------------------------------
template <typename T1>
std::pair<typename llvm::SmallPtrSetImpl<T1>::iterator, bool>
f1(llvm::SmallPtrSetImpl<T1> &o, T1 p) {
  return o.insert(p);
}

template <typename T1> llvm::SmallPtrSet<T1, 4> f2() {
  return llvm::SmallPtrSet<T1, 4>();
}

template <typename T1>
llvm::SmallPtrSet<T1, 4> f3(const llvm::SmallPtrSet<T1, 4> &o) {
  return llvm::SmallPtrSet<T1, 4>(o);
}

template <typename T1>
llvm::SmallPtrSet<T1, 4> f4(std::initializer_list<T1> il) {
  return llvm::SmallPtrSet<T1, 4>(il);
}

template <typename T1> bool f5(llvm::SmallPtrSetImpl<T1> &o, T1 p) {
  return o.erase(p);
}

// f6/f7 -- the two ConstPtrType members.  POINTEE-parametrised: `T1` binds to
// `mlir::Operation`, not to `mlir::Operation *`.
template <typename T1>
bool f6(const llvm::SmallPtrSetImpl<T1 *> &o, const T1 *p) {
  return o.contains(p);
}

template <typename T1>
unsigned f7(const llvm::SmallPtrSetImpl<T1 *> &o, const T1 *p) {
  return o.count(p);
}

// ⭐⭐ f8/f9 -- contains/count ON A CONST-ELEMENT SET, i.e. `SmallPtrSet<const T *,
// N>`.  f6/f7 DO NOT COVER THIS, MEASURED (probe/g272c.vlog, converter rc=0):
//
//   search expr bool llvm::SmallPtrSetImpl<const proj::Node *>::contains(const proj::Node *) const
//     -> None
//   search expr unsigned int llvm::SmallPtrSetImpl<const proj::Node *>::count(const proj::Node *) const
//     -> None
//
// because `add_const_past_pointer<const T *>` is IDEMPOTENT: the parameter is
// `const T *` while the class argument is ALSO `const T *`, so f6's pattern
// (receiver `T1 *`, parameter `const T1 *`) would need T1 = `const T` and would
// then demand the parameter `const const T *`, which is not the recorded string.
//
// ⛔ AND THAT `None` DID NOT ABORT -- it emitted the call TEXTUALLY, the exact
// hazard that makes a type key with a missing member worse than no key:
//
//   (({ (*cs.borrow()).contains((a.as_pointer()),) }) as u8),
//   ({ (*cs.borrow()).count((b.as_pointer()),) }),
//
// The corpus has 2 const-element declarations out of 23 -- UniformGrouper.cpp:72
// (`SmallPtrSet<const LocalRegInitCandidate *, 32>`, insert only) and
// Planner.cpp:583 (`SmallPtrSet<const TransferMaterializationInfo *, 4>`, whose
// :981 IS a `contains`) -- so this is a live site, not a hypothetical.
//
// No ambiguity with f6/f7: their receiver pattern `T1 *` cannot bind a
// `const T *` ask and produce a matching parameter, and f8/f9's `const T1 *`
// receiver cannot bind a non-const `T *` ask at all.  Verified by re-probing
// BOTH shapes after adding these (see the header note).
template <typename T1>
bool f8(const llvm::SmallPtrSetImpl<const T1 *> &o, const T1 *p) {
  return o.contains(p);
}

template <typename T1>
unsigned f9(const llvm::SmallPtrSetImpl<const T1 *> &o, const T1 *p) {
  return o.count(p);
}
