// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::EquivalenceClasses<T> -- Tarjan union-find -- and its nested
// member_iterator, PARTIALLY mapped.  Read the "WHAT IS DELIBERATELY UNMAPPED"
// section before extending this: three quarters of this type's surface returns
// a reference or a pointer into container-owned storage, and every one of those
// is left a loud abort rather than guessed.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL, not #include <llvm/ADT/EquivalenceClasses.h>
// -------------------------------------------------------------------------------
// Same reason as rules/denseset, rules/densemap, rules/smallset: cpp-rule-
// preprocessor compiles this file with a fixed flag set and cannot reach LLVM's
// headers.  Every signature below was read out of
// llvm/ADT/EquivalenceClasses.h of LLVM-22.1.3 (the tree the port builds
// against), lines 62..376, not remembered.
//
// THE KEY IS SIMPLE, for once
// ---------------------------
// `template <class ElemTy> class EquivalenceClasses` (EquivalenceClasses.h:62)
// has ONE template parameter and NO default arguments, so the
// defaulted-argument-elision trap that cost rules/smallset and rules/denseset
// their arity does not apply here: `llvm::EquivalenceClasses<int>` is the
// string the census reports and the string a one-parameter restatement
// produces.  The census (dualpass/census/B) records it as
//     UnmappedType  llvm::EquivalenceClasses<int>  -
// twelve times across six TUs -- GraphColoring, GraphStats, PortAssignment,
// SmartRegisterAllocation, OldRegisterInitialization and
// RegisterInitialization/Selector -- and grepping the rule IR for
// EquivalenceClasses returned nothing at all before this module.  Five of those
// six TUs never CALL anything on the container: it reaches them only as the
// type of RegisterGraphs::ec_map_, declared in GraphColoring.h and included.
// For them the type rule alone is the whole fix.
//
// WHAT dt_src ACTUALLY CALLS, and it is all in one function pair
// -------------------------------------------------------------
// dcc/src/Transform/Sentient/Analyses/GraphColoring.cpp is the only caller:
//   :348  for (EquivalenceClasses<int>::iterator I = this_ec.begin(), E = ...)
//   :350  if (!(*I)->isLeader()) continue;
//   :351  auto leader = *this_ec.findLeader(**I);
//   :353  for (member_iterator MI = this_ec.member_begin(**I);
//              MI != this_ec.member_end(); ++MI)
//   :556  const EquivalenceClasses<int> &EC = ec.second;   // and EC.empty()
//   :559-:585  the same walk again, plus EC.getLeaderValue(e)
//   :88-:90    member_iterator passed by value as a parameter
// dataflow-scheduler and dr5 use <mlir::Value> and <void *>, which are
// different keys and different census rows; nothing here claims them.
//
// THE REPRESENTATION, and the one decision that matters
// ----------------------------------------------------
// LLVM's internal shape is a DenseMap<ElemTy, ECValue *> plus a bump-allocated
// intrusive linked list per class, with the leader bit STOLEN from ECValue's
// Next pointer (EquivalenceClasses.h:74..:123).  None of that is observable
// through the surface mapped here, so the Rust side is a FLAT LEADER MAP,
//     BTreeMap<T1, T1>          // element -> the leader of its class
// and unionSets REWRITES every member of one class to point at the other
// leader.  A flat leader map is transitive BY CONSTRUCTION: after
// unionSets(1,2) then unionSets(2,3), 3's entry is rewritten to 1 in the same
// pass that rewrites 2's, so findLeader(1) and findLeader(3) agree.  This is
// the case the brief warns about -- a model that stored only PAIRWISE links
// would pass a two-element probe and silently fail on three -- and the probe
// exercises it explicitly.  The cost of the flat map is that unionSets is
// O(class size) instead of O(1) amortised; performance is not a semantic the
// port claims to preserve.
//
// WHICH ELEMENT BECOMES THE LEADER is, in general, an unspecified
// implementation detail, and the brief is right that a model must not assume
// it.  But this LLVM's strategy is readable and unconditional:
// EquivalenceClasses.h:306..:322, `unionSets(L1, L2)` appends L2's list to
// L1's, clears L2's leader flag, and sets `L2LV.Leader = &L1LV` -- it then
// `return L1`.  There is no rank or size comparison, so the merged class's
// leader is always the FIRST argument's leader.  The Rust body below reproduces
// exactly that (it rewrites the l2 members to l1, and returns l1).  It is
// reproduced deliberately rather than relied upon: no rule here EXPOSES a
// leader value (see below), so the probe tests CLASS MEMBERSHIP -- which
// elements share a leader, the specified behaviour -- and not the leader's
// identity.
//
// ITERATION ORDER.  EquivalenceClasses.h:129 keeps a `SmallVector<const
// ECValue *> Members` explicitly "to provide a deterministic iteration order",
// i.e. INSERTION order over classes, and member_iterator walks the intrusive
// list, i.e. L1's members then L2's appended after them.  GraphColoring.cpp
// DOES depend on iterating both (:348 and :353), and that is precisely the part
// this module does NOT map, so no order claim is being made here.
//
// WHAT IS DELIBERATELY UNMAPPED -- each one a loud abort, not a guess
// ------------------------------------------------------------------
// Everything that hands back a REFERENCE OR POINTER INTO CONTAINER-OWNED
// STORAGE, because a flat leader map has no stable per-element address to point
// at and returning a pointer to a local would be exactly the
// silently-wrong-but-compiling output this port keeps finding:
//   - `const ECValue &insert(const ElemTy &)`             (:217)
//   - `const ElemTy &getLeaderValue(const ElemTy &)`      (:188)
//   - `const ElemTy &getOrInsertLeaderValue(const ElemTy &)` (:199)
//   - `const ElemTy &member_iterator::operator*()`        (:349)
//     and `operator->`                                    (:354)
// ECValue ITSELF is not declared here at all, and therefore neither is
// `iterator` (:158, `SmallVector<const ECValue *>::const_iterator`),
// `begin()`/`end()` (:160/:161), `member_begin()` (:167), `members()`,
// `ECValue::isLeader()`/`getData()`/`getNext()`: modelling ECValue honestly
// means a runtime type with the leader bit and the member list, in libcc2rs,
// which this module does not own.  `erase()` (:230), the copy constructor and
// `operator=` (:137..:151), the `unionSets(member_iterator, member_iterator)`
// overload (:309), postfix `operator++` (:364) and `getNumClasses()`'s
// iteration-based definition are likewise absent.
//
// CONSEQUENCE, stated plainly: GraphColoring.cpp still aborts, because its walk
// needs `iterator` and `ECValue`.  What this module fixes is the TYPE gap in all
// six TUs (the five non-calling ones outright) and the value-level union-find
// surface -- unionSets, findLeader, isEquivalent, contains, empty -- which is
// what makes the semantics above testable at all.

namespace llvm {

// EquivalenceClasses.h:62.  ONE parameter, no default -- see the header comment.
// ECValue and the Members vector are NOT restated: no rule below names them,
// and a partial restatement of ECValue would let a rule be written against a
// shape this module cannot implement.
template <class ElemTy> class EquivalenceClasses {
public:
  // EquivalenceClasses.h:334.  A nested NON-template class whose comparisons
  // are ordinary CONST MEMBER functions (:369/:372), not the hidden friends
  // rules/denseset had to deal with -- so the rules below spell
  // `a.operator==(b)`.  Spelling a plain `a == b` would record no src entry and
  // then abort every translation that loads this module
  // (`Expr rule loaded from IR but has no src`), which is the trap
  // rules/smallset paid for.
  //
  // operator* / operator-> / postfix ++ are NOT declared: see the header
  // comment.  operator++ (prefix) is not declared either, because without
  // operator* an advance is unobservable and a mapped ++ would silently make a
  // member walk look supported.
  class member_iterator {
  public:
    member_iterator(const member_iterator &RHS);
    bool operator==(const member_iterator &RHS) const;
    bool operator!=(const member_iterator &RHS) const;
  };

  EquivalenceClasses();

  bool empty() const;
  bool contains(const ElemTy &V) const;
  bool isEquivalent(const ElemTy &V1, const ElemTy &V2) const;

  member_iterator member_end() const;
  member_iterator findLeader(const ElemTy &V) const;
  member_iterator unionSets(const ElemTy &V1, const ElemTy &V2);
};

} // namespace llvm

// ---------------------------------------------------------------------------
// THE CONTAINER TYPE and THE MEMBER ITERATOR.
//
// t1 is the container itself.  rules/stringmap is the cautionary tale: mapping
// only the iterator leaves the converter porting the receiver class, and the
// output cannot compile.  t1 alone is what unblocks the five TUs that merely
// DECLARE an EquivalenceClasses member.
// ---------------------------------------------------------------------------

template <typename T1> using t1 = llvm::EquivalenceClasses<T1>;
template <typename T1>
using t2 = typename llvm::EquivalenceClasses<T1>::member_iterator;

template <typename T1> llvm::EquivalenceClasses<T1> f1() {
  return llvm::EquivalenceClasses<T1>();
}

// unionSets(ElemTy, ElemTy) -- inserts both operands if absent (:306) and
// merges.  The returned member_iterator is the merged class's leader position.
template <typename T1>
typename llvm::EquivalenceClasses<T1>::member_iterator
f2(llvm::EquivalenceClasses<T1> &o, const T1 &v1, const T1 &v2) {
  return o.unionSets(v1, v2);
}

// findLeader -- the class's leader position, or member_end() when the value was
// never inserted (:295).  Comparing two findLeader results is the SPECIFIED way
// to ask whether two elements share a class.
template <typename T1>
typename llvm::EquivalenceClasses<T1>::member_iterator
f3(const llvm::EquivalenceClasses<T1> &o, const T1 &v) {
  return o.findLeader(v);
}

template <typename T1>
typename llvm::EquivalenceClasses<T1>::member_iterator
f4(const llvm::EquivalenceClasses<T1> &o) {
  return o.member_end();
}

template <typename T1>
bool f5(const llvm::EquivalenceClasses<T1> &o, const T1 &v1, const T1 &v2) {
  return o.isEquivalent(v1, v2);
}

template <typename T1>
bool f6(const llvm::EquivalenceClasses<T1> &o, const T1 &v) {
  return o.contains(v);
}

template <typename T1> bool f7(const llvm::EquivalenceClasses<T1> &o) {
  return o.empty();
}

// The comparisons -- MEMBER calls, spelled explicitly (see the note on the
// class).  Position identity: `Node == RHS.Node` (:370), i.e. same ECValue
// address, and for a leader position that is the same relation as "same class".
template <typename T1>
bool f8(typename llvm::EquivalenceClasses<T1>::member_iterator a,
        typename llvm::EquivalenceClasses<T1>::member_iterator b) {
  return a.operator==(b);
}

template <typename T1>
bool f9(typename llvm::EquivalenceClasses<T1>::member_iterator a,
        typename llvm::EquivalenceClasses<T1>::member_iterator b) {
  return a.operator!=(b);
}
