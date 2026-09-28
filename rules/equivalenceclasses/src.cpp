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
};

} // namespace llvm

// TYPE key.  Must be `using tN = ...` on the SRC side with a matching
// `fn tN() -> T { <initializer> }` on the target side; a bare `type tN = T;`
// target does not register.
template <typename T1> using t1 = llvm::EquivalenceClasses<T1>;
