// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::EquivalenceClasses<T> -- Tarjan union-find, llvm/ADT/EquivalenceClasses.h:62.
//
// WHY THE DECLARATION BELOW IS LOCAL AND NOT #include <llvm/ADT/EquivalenceClasses.h>
// -----------------------------------------------------------------------------
// Same reason as rules/apint, rules/twine and rules/raw_ostream:
// cpp-rule-preprocessor compiles this file with a fixed flag set and the only
// flags that would reach LLVM's real headers are absolute -I paths into whatever
// LLVM tree the target project happens to have built.  A rule matches on a
// signature STRING, so the restatement is faithful iff it agrees with LLVM
// exactly.
//
// THE CONTAINER IS NOT DEFINED HERE.  libcc2rs/src/iterators.rs:1432-1610
// already implements the whole of EquivalenceClasses<T> (insert / find_leader /
// union_sets / member_begin / ... , with MemberIter<T> and ECValue<T>), with a
// `T: Ord + Clone` bound (NOT Copy) and FIRST-INSERTION leader order (NOT Ord
// order).  This module only KEYS it.
//
// WHAT THIS MODULE COVERS AND WHY IT IS EXACTLY ONE KEY
// ----------------------------------------------------
// The census first-abort is
//   `unsupported unmapped type llvm::EquivalenceClasses<int> has no model in
//    types_, while mapping llvm::DenseMap<SentientRegType,
//    llvm::EquivalenceClasses<int>>`
// in three TUs: Analyses/GraphColoring.cpp, Analyses/GraphStats.cpp and
// RegisterInitialization/Selector.cpp.
//
// The spelling does NOT come from those .cpp files -- two of the three do not
// mention EquivalenceClasses at all.  It comes from the HEADER
// dcc/src/Transform/Sentient/Analyses/GraphColoring.hpp:187
//     DenseMap<SentientRegType, llvm::EquivalenceClasses<int>> ec_map_;
// i.e. the type appears as a DenseMap VALUE type.  A DenseMap<K,V>
// instantiation needs V mapped and default-initialisable, which is why the type
// aborts before any member is called.  So the gate for all three TUs is the
// TYPE, and the type target's initialiser is the default construction.
//
// NOT COVERED, DELIBERATELY.  GraphColoring.cpp (and only that one of the three)
// additionally reaches, on `ec_map_[locale]`:
//   :624  this_ec.insert(baseNodeIdx)
//   :676  this_ec.unionSets(it_A, it_B)
//   :348/:559  begin()/end() and llvm::EquivalenceClasses<int>::iterator
//   :349/:560  (*I)->isLeader()
//   :350  findLeader(**I) and :565 findLeader(*EC.member_begin(**I))
//   :353/:577  member_begin(**I) / member_end()
//   :571  getLeaderValue(e)
//   :553  ec.second.empty()
// and the free function GraphColoring::doesEdgeExist (GraphColoring.hpp:71-73,
// GraphColoring.cpp:87-90) takes three
// `llvm::EquivalenceClasses<int>::member_iterator` parameters and derefs them.
// Those are MEMBERS OF A NESTED TYPE plus nine expr keys; none of them is the
// first abort for any of the three TUs, and this slot could not measure each one
// as REACHED-and-correct.  A key nobody reaches is a key nobody has checked, so
// they are left out and will abort loudly at the next gate rather than be
// guessed here.

namespace llvm {

// Restated from llvm/ADT/EquivalenceClasses.h:62 --
//   template <class ElemTy> class EquivalenceClasses
// Arity 1, no defaulted parameter, so one key covers <int>, <unsigned>,
// <mlir::Value>, <mlir::Operation *> and <void *>.  Member layout is irrelevant
// to signature matching; the class must merely be complete.
template <class ElemTy> class EquivalenceClasses {
public:
  EquivalenceClasses();

  // Restated from EquivalenceClasses.h:66 -- `class ECValue`, a NESTED class (not
  // a typedef). It is keyed because the converter does NOT abort on an unmapped
  // nested type: it emits `Cpp2RustUnmapped_llvm_EquivalenceClasses_int__ECValue`
  // (converter.cpp:162) and translates to rc=0, so GraphColoring.cpp reached
  // bucket A carrying 8 undefined type names. The searched spelling is read off
  // the converter's own note, verbatim:
  //   note: no Rust type text for `llvm::EquivalenceClasses<int>::ECValue`
  //   (Record); emitting the undefined placeholder
  //   `Cpp2RustUnmapped_llvm_EquivalenceClasses_int__ECValue`
  class ECValue {
  public:
    bool isLeader() const;  // header:88
  };

  // Restated from EquivalenceClasses.h:171 -- `class member_iterator`, also a
  // nested CLASS. Searched spelling, verbatim from the same note stream:
  //   note: no Rust type text for
  //   `llvm::EquivalenceClasses<int>::member_iterator` (Record)
  // GraphColoring.hpp:71-73 names it in three PARAMETERS of doesEdgeExist, which
  // is why it cannot be an anonymous `impl Iterator` on the Rust side.
  class member_iterator {
  public:
    // Restated from EquivalenceClasses.h:171-200.  These are MEMBER operators (not
    // hidden friends as on libc++'s deque iterator), so the rule bodies below use
    // the MEMBER call form `it.operator==(rhs)`.  An infix `==`/`!=`/`++` in a rule
    // body records NOTHING -- that is the readback-verified reason the previous slot
    // left these rows -- and a qualified `llvm::operator==` aborts at
    // cpp_rule_preprocessor.cpp:888.
    bool operator==(const member_iterator &RHS) const;   // header:186
    bool operator!=(const member_iterator &RHS) const;   // header:190
    const ElemTy &operator*() const;                     // header:180
    member_iterator &operator++();                       // header:193
    member_iterator operator++(int);                     // header:198
  };

  // MEMBERS THE CORPUS CALLS, restated from the header so this file typechecks and
  // so each signature STRING matches LLVM exactly.  See the f-key block below.
  const ECValue &insert(const ElemTy &Data);          // header:201
  bool empty() const;                                 // header:150
  const ElemTy &getLeaderValue(const ElemTy &V) const;// header:212
  member_iterator member_begin(const ECValue &ECV) const; // header:225
  member_iterator member_end() const;                 // header:231
  member_iterator findLeader(const ECValue &ECV) const;   // header:239
  member_iterator findLeader(const ElemTy &V) const;      // header:243
  member_iterator unionSets(member_iterator L1, member_iterator L2); // header:290
};

} // namespace llvm

// TYPE key.  Must be `using tN = ...` on the SRC side with a matching
// `fn tN() -> T { <initializer> }` on the target side; a bare `type tN = T;`
// target does not register.
template <typename T1> using t1 = llvm::EquivalenceClasses<T1>;

// NESTED-TYPE keys. GetTypeMapKey (mapper.cpp:114) strips at the first `<`, so all
// three keys land in the SAME bucket `llvm::EquivalenceClasses` and matchTemplate
// picks between them on the literal tail after `<T1>`.
template <typename T1>
using t2 = typename llvm::EquivalenceClasses<T1>::member_iterator;
template <typename T1>
using t3 = typename llvm::EquivalenceClasses<T1>::ECValue;

// ============================================================================
// MEMBER (FUNCTION) KEYS -- ADDED 2026-09-28.  THIS MODULE HAD 3 TYPE KEYS AND
// ZERO FUNCTION KEYS, WHICH IS A SILENT-WRONGNESS SHAPE, AND IT WAS MEASURED AS
// SUCH RATHER THAN ASSUMED.
//
// Witness: dcc/src/Transform/Sentient/Analyses/GraphColoring.cpp --
// bucket A, rc=0, 28,911 emitted lines (pin e2d09f45, rule tree cloned from
// pin/ir.v27).  The emitted .rs contained CALLS TO METHODS THAT EXIST NOWHERE,
// at rc=0, with NO placeholder token, so pin/no-placeholders.sh stayed clean and
// no bucket census could see them:
//   .findLeader_pconsti32_const(...)                                     x3
//   .findLeader_pconstlibcc2rsECValuei32_const(...)                      x1
//   .unionSets_libcc2rsMemberIteri32_libcc2rsMemberIteri32(_L1, _L2)     x1
//   .member_begin(...)  x3   .member_end()  x1   .getLeaderValue(...) x1
//   .isLeader()         x2   .insert(...)   x1
// The `_pconsti32_const` suffixes are the converter's overload mangling: with no
// rule it emits the C++ name TEXTUALLY against the mapped receiver type, i.e.
// against `libcc2rs::EquivalenceClasses<i32>`, which has `find_leader` and
// `union_sets_iters` and has never had any of the names above.
//
// WHY THESE ARE LANDABLE AND NOT A REFERENCE/ITERATOR-IDENTITY REFUSAL.
// The refusal criterion for this type is that a member handing out a REFERENCE
// or an ITERATOR under a by-value receiver dangles silently (the ground on which
// string_view::front() and SMLoc::getPointer() were both refused).  Every member
// keyed below returns BY VALUE:
//   * findLeader / member_begin / member_end / unionSets return
//     `libcc2rs::MemberIter<T>`, which is a SNAPSHOT (`chain: Vec<T>, idx`),
//     not a pointer into the forest (iterators.rs:1348-1352);
//   * getLeaderValue returns `T` by value (a `.clone()` out of the BTreeMap);
//   * isLeader returns `bool`;
//   * insert returns `ECValue<T>` by value.
// LEADER IDENTITY is also preserved, which is the other refusal ground: LLVM's
// unionSets keeps L1 as leader and splices L2's chain onto its end
// (EquivalenceClasses.h:295-312), and `union_leaders` reproduces exactly that
// (iterators.rs:1596-1611), so `updated_leader == it_A` at GraphColoring.cpp:679
// -- the test by which the caller learns which class survived -- reads the same
// answer.  MemberIter's PartialEq compares the CURRENT ELEMENT, and elements are
// unique in the forest, so element equality == C++ node-address equality.
//
// ONE HONEST CAVEAT ON insert (f8).  C++ returns `const ECValue &`, a reference
// INTO the structure whose isLeader() bit changes on a later unionSets; libcc2rs
// returns a snapshot, so a caller that held the result across a mutation would
// read a STALE is_leader().  The corpus's only call site discards the result
// (GraphColoring.cpp:624 `this_ec.insert(baseNodeIdx);`), and a by-value return
// cannot dangle -- which is the property that makes a key safe here.  The
// observer to watch for is `const auto &E = EC.insert(x); EC.unionSets(...);
// E.isLeader()`.  It does not exist in this corpus.
//
// DELIBERATELY NOT KEYED, WITH THE OBSERVER NAMED IN EACH CASE:
//  * begin() / end().  REFUSED, and this is a TYPE mismatch, not a naming gap.
//    LLVM's `iterator` is `SmallVector<const ECValue *>::const_iterator`, i.e. a
//    RAW POINTER, so the converter maps the loop variable as
//    `*const *const libcc2rs::ECValue<i32>` (emitted at GraphColoring .rs:27660,
//    28390).  libcc2rs's begin()/end() return `RangeIter<ECValue<T>>`, which is
//    not that type, so keying them would swap a compile error for a type error
//    at the ASSIGNMENT and change what `*I` means.  Honest fixes are a model for
//    `EquivalenceClasses<T>::iterator` or a pointer-yielding begin(); both are
//    libcc2rs changes and libcc2rs is not this slot's to touch.  OBSERVER: the
//    `for (I = EC.begin(), E = EC.end(); I != E; ++I)` walks at :348 and :559.
//  * ++MI / *MI on member_iterator.  Not keyed here: these are EXPR keys on the
//    nested iterator type (prefix_inc / at), the converter emits infix forms, and
//    a `++` written infix in a rule body records NOTHING.  They belong with the
//    begin()/end() work, not ahead of it.
//  * erase(), getNumClasses(), isEquivalent(), getOrInsertLeaderValue(): no
//    corpus caller measured, and a key nobody reaches is a key nobody checked.

// f1 -- EquivalenceClasses.h:243 `member_iterator findLeader(const ElemTy &V) const`.
// GraphColoring.cpp:350, :565, and the emitted mangling `findLeader_pconsti32_const`.
template <typename T1>
typename llvm::EquivalenceClasses<T1>::member_iterator
f1(const llvm::EquivalenceClasses<T1> &ec, const T1 &v) {
  return ec.findLeader(v);
}

// f2 -- EquivalenceClasses.h:239 `member_iterator findLeader(const ECValue &ECV) const`,
// the ECValue overload.  GraphColoring.cpp:562; emitted mangling
// `findLeader_pconstlibcc2rsECValuei32_const`.  Rust cannot overload, so this is a
// SEPARATE key onto the separately-named `find_leader_of`.
template <typename T1>
typename llvm::EquivalenceClasses<T1>::member_iterator
f2(const llvm::EquivalenceClasses<T1> &ec,
   const typename llvm::EquivalenceClasses<T1>::ECValue &ecv) {
  return ec.findLeader(ecv);
}

// f3 -- EquivalenceClasses.h:290 `member_iterator unionSets(member_iterator L1,
// member_iterator L2)`, the iterator overload GraphColoring.cpp:676 calls.
// NON-CONST receiver: it mutates the forest.
template <typename T1>
typename llvm::EquivalenceClasses<T1>::member_iterator
f3(llvm::EquivalenceClasses<T1> &ec,
   typename llvm::EquivalenceClasses<T1>::member_iterator l1,
   typename llvm::EquivalenceClasses<T1>::member_iterator l2) {
  return ec.unionSets(l1, l2);
}

// f4 -- EquivalenceClasses.h:225 `member_iterator member_begin(const ECValue &ECV) const`.
// GraphColoring.cpp:353, :565, :577.
template <typename T1>
typename llvm::EquivalenceClasses<T1>::member_iterator
f4(const llvm::EquivalenceClasses<T1> &ec,
   const typename llvm::EquivalenceClasses<T1>::ECValue &ecv) {
  return ec.member_begin(ecv);
}

// f5 -- EquivalenceClasses.h:231 `member_iterator member_end() const`.  The C++
// body is `member_iterator(nullptr)`, i.e. it needs no receiver state at all, so
// the target may equally spell it as the Default; it is keyed on the receiver
// form because that is how the corpus writes it (GraphColoring.cpp:577).
template <typename T1>
typename llvm::EquivalenceClasses<T1>::member_iterator
f5(const llvm::EquivalenceClasses<T1> &ec) {
  return ec.member_end();
}

// f6 -- EquivalenceClasses.h:212 `const ElemTy &getLeaderValue(const ElemTy &V) const`.
// GraphColoring.cpp:571.  C++ returns a const reference; the libcc2rs member returns
// T BY VALUE (a clone out of the BTreeMap), which is what makes this safe under
// refcount -- a reference into a union-find forest is invalidated by unionSets.
template <typename T1>
T1 f6(const llvm::EquivalenceClasses<T1> &ec, const T1 &v) {
  return ec.getLeaderValue(v);
}

// f7 -- EquivalenceClasses.h:88 `bool isLeader() const`, a member of the NESTED
// ECValue (keyed as t3).  GraphColoring.cpp:349, :560 spell it `(*I)->isLeader()`;
// it is the filter that turns all-entries iteration into leader iteration.
template <typename T1>
bool f7(const typename llvm::EquivalenceClasses<T1>::ECValue &ecv) {
  return ecv.isLeader();
}

// f8 -- EquivalenceClasses.h:201 `const ECValue &insert(const ElemTy &Data)`.
// GraphColoring.cpp:624.  See the insert caveat above: snapshot, not a reference.
template <typename T1>
typename llvm::EquivalenceClasses<T1>::ECValue
f8(llvm::EquivalenceClasses<T1> &ec, const T1 &v) {
  return ec.insert(v);
}

// f9 -- EquivalenceClasses.h:150 `bool empty() const`.  GraphColoring.cpp:553
// `ec.second.empty()`.
template <typename T1>
bool f9(const llvm::EquivalenceClasses<T1> &ec) {
  return ec.empty();
}

// ============================================================================
// OPERATOR KEYS ON member_iterator (t2) -- ADDED 2026-09-28.
//
// WHY THESE ARE VISIBLE WHERE f1-f9 WERE NOT.  f1-f9 closed calls to methods that
// existed nowhere, emitted textually at rc=0 with NO placeholder token, so
// pin/no-placeholders.sh stayed clean.  These rows are different: the converter
// cannot emit an operator textually, so it emits
// `Cpp2RustUnmappedExpr_CXXOperatorCallExpr`, and the witness
// dcc/src/Transform/Sentient/Analyses/GraphColoring.cpp carries those placeholders.
// The gate for this block is therefore the PLACEHOLDER COUNT FALLING, measured on
// the emitted .rs before and after.
//
// NOTHING IS ADDED TO libcc2rs FOR THESE.  `MemberIter<T>` already carries
// `PartialEq` (iterators.rs:1366, comparing the CURRENT ELEMENT, which is C++ node
// identity because elements are unique in the forest), `PrefixInc`
// (iterators.rs:1407) and `PostfixInc` (iterators.rs:1413), and `at()`
// (iterators.rs:1393) is the `*MI` form.  All four return BY VALUE, which is the
// property that makes a key safe here.
//
// ⛔ THESE ARE NOT the `I != E` / `++I` of the begin()/end() walks at :348 and :559.
// Those run over LLVM's `iterator`, which is
// `SmallVector<const ECValue *>::const_iterator` -- a RAW POINTER -- and are
// refused below with their observer.  member_iterator is a real class and is the
// only one of the two these keys can reach.

// f10 -- EquivalenceClasses.h:186 `bool operator==(const member_iterator &) const`.
// The `updated_leader == it_A` test at GraphColoring.cpp:679, by which the caller
// learns which class survived unionSets.
template <typename T1>
bool f10(const typename llvm::EquivalenceClasses<T1>::member_iterator &it1,
        const typename llvm::EquivalenceClasses<T1>::member_iterator &it2) {
  return it1.operator==(it2);
}

// f11 -- EquivalenceClasses.h:190 `bool operator!=(const member_iterator &) const`.
// The loop test of every `for (MI = EC.member_begin(..); MI != EC.member_end(); ++MI)`.
template <typename T1>
bool f11(const typename llvm::EquivalenceClasses<T1>::member_iterator &it1,
        const typename llvm::EquivalenceClasses<T1>::member_iterator &it2) {
  return it1.operator!=(it2);
}

// f12 -- EquivalenceClasses.h:193 `member_iterator &operator++()`.  C++ returns a
// reference; the target returns the new state BY VALUE, exactly as
// rules/deque_iterator f7 does for `std::deque<T>::iterator &`.
template <typename T1>
typename llvm::EquivalenceClasses<T1>::member_iterator &
f12(typename llvm::EquivalenceClasses<T1>::member_iterator &it) {
  return it.operator++();
}

// f13 -- EquivalenceClasses.h:198 `member_iterator operator++(int)`, postfix.
// Yields the OLD position, which PostfixInc reproduces.
template <typename T1>
typename llvm::EquivalenceClasses<T1>::member_iterator
f13(typename llvm::EquivalenceClasses<T1>::member_iterator a0, int a1) {
  return a0.operator++(a1);
}

// f14 -- EquivalenceClasses.h:180 `const ElemTy &operator*() const`.  Keyed with the
// ++/!= rows because a walk that cannot dereference is useless: GraphColoring uses
// `*MI` AS A GRAPH NODE INDEX (:353-:360, and doesEdgeExist at
// GraphColoring.cpp:87-90 derefs all three of its member_iterator parameters).
// C++ hands out `const ElemTy &`; libcc2rs's `at()` returns T BY VALUE (a clone out
// of the snapshot chain), which is the honest return here -- a reference into a
// union-find forest is invalidated by unionSets, and a by-value return cannot
// dangle under refcount.
template <typename T1>
T1 f14(const typename llvm::EquivalenceClasses<T1>::member_iterator &it) {
  return it.operator*();
}
