// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::ilist_iterator -- the iterator over LLVM's INTRUSIVE doubly-linked
// list, which is what mlir::Block::iterator is. `Block::getOperations()` is an
// `iplist<Operation>` and every walk over the operations in a block goes
// through this type.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/ADT/ilist_iterator.h>
// ---------------------------------------------------------------------------
// The reason rules/raw_ostream, rules/stringref, rules/twine and
// rules/statistic all give at length: cpp-rule-preprocessor compiles this file
// with a fixed flag set, and reaching LLVM's headers would need an absolute -I
// into whatever LLVM tree the target project happens to have built, which would
// make `ninja` in this repo fail for anyone without that tree.
//
// Each signature below was read back out of a real translation with
// `cpp2rust --verbose`, which prints `search expr <exact resolved signature>`,
// and then checked to print the SAME string from this restatement.  Two details
// are load-bearing and were NOT guessable:
//
//   * `node_options` must be restated with all SIX parameters and the rule must
//     spell the defaults out (`<T1, false, false, void, false, void>`), because
//     that is how the converter asks for it.
//   * operator== / operator!= are FRIEND declarations INSIDE the class in real
//     LLVM (ilist_iterator.h:174-179), not namespace-scope templates.  Declared
//     as a namespace-scope template this file fails to resolve at all
//     ("No viable function"); declared as a friend it prints
//     `bool llvm::operator!=(...)`, which is what the converter searches.
//
// MODEL: THE ITERATOR IS THE NODE POINTER, and that is the whole of it.
// ---------------------------------------------------------------------------
// ilist_iterator's entire state is one pointer (ilist_iterator.h:108,
// `node_pointer NodePtr = nullptr;`).  So `*mut T1` is not an approximation of
// this iterator, it IS this iterator, and three consequences follow exactly:
//
//   * operator*() is the IDENTITY -- the node and the value are the same
//     object.  Spelled as the pointer because a reference-out rule must be
//     (the playbook's rule, and `&*it` at LexicalOrdering.cpp:175 relies on it).
//   * operator== / operator!= are POINTER COMPARISON.  Not an approximation
//     either: LLVM's own bodies are `return LHS.NodePtr == RHS.NodePtr;`.
//   * constructing an iterator from a `T1 *`, and `ilist_node_impl::getIterator()`
//     going the other way, are both the identity.  The real code uses both
//     directions (`Operation::getIterator()` at LexicalOrdering.cpp:174,
//     `Block::reverse_iterator(anchor_for_op_)` at LoopAbsorption.cpp:147),
//     which is itself evidence that iterator and node pointer are interchangeable
//     here.
//
// WHAT IS DELIBERATELY LEFT LOUD, AND WHY IT CANNOT BE OTHERWISE.
// ---------------------------------------------------------------------------
// operator++ and operator-- get NO RULE.  Their bodies are
// `NodePtr = IsReverse ? NodePtr->getPrev() : NodePtr->getNext()`
// (ilist_iterator.h:182-188), i.e. they READ THE INTRUSIVE LINK, and that link
// lives in mlir::Operation's ilist_node base -- on the far side of the
// `--opaque-namespace=mlir` boundary, where the port's mlir_Operation is a
// `#[repr(transparent)] struct(u64)` with no fields at all.  There is nothing
// to follow.  A rule here could only invent a step, and the two candidate
// inventions are both silently wrong:
//
//   * pointer arithmetic (`p.add(1)`) treats an intrusive list as an array.
//     Operations in a Block are separately allocated; this reads unrelated
//     memory.
//   * snapshotting the list into a Vec and stepping an index was considered and
//     REFUSED on measured evidence: LoopRolling.cpp:93 stores three
//     `Block::iterator` members in a long-lived Window, and
//     updateEndOpsOfMatchedOps (:240-245) walks a STORED one backwards with
//     std::prev and dereferences it, while 21 sites across these TUs splice or
//     erase the list.  An index into a snapshot is stale after any splice and
//     std::prev on it reads the wrong element with no diagnostic.
//
// So ++/-- stay a loud translation failure naming the operator, which is the
// correct outcome per "a loud abort is better than a wrong value".  That is a
// deliberate ASYMMETRY in this module: comparison and dereference are exact,
// stepping is refused.  It is still worth having, because `!=`/`==` on this type
// was measured as the largest open row of that class -- 18 occurrences across 6
// TUs -- and those become correct here.
//
// Likewise no rule for `splice`, `Block::begin/end`, or
// `OpBuilder::createBlock`: those hand an iterator BACK INTO MLIR, which the
// port does not have.  They stay undefined externs, i.e. loud at rustc naming
// exactly what the port still owes.
//
// t1 is the forward iterator and t2 the reverse one.  They are separate rules
// because IsReverse is a template ARGUMENT, so one pattern cannot cover both,
// and they must not be merged: their `++` steps in opposite directions.  Both
// map to the same representation because a reverse ilist_iterator also stores
// exactly one node pointer -- only its stepping differs, and stepping is the
// part with no rule.
// ---------------------------------------------------------------------------

namespace llvm {
namespace ilist_detail {
template <class T, bool EnableSentinelTracking, bool IsSentinelTrackingExplicit,
          class TagT, bool HasIteratorBits, class ParentTy>
struct node_options {
  using value_type = T;
  using pointer = T *;
  using reference = T &;
};
} // namespace ilist_detail

template <class OptionsT, bool IsReverse, bool IsConst> class ilist_iterator {
public:
  using value_type = typename OptionsT::value_type;
  using pointer = typename OptionsT::pointer;
  using reference = typename OptionsT::reference;
  ilist_iterator() = default;
  explicit ilist_iterator(pointer NP);
  reference operator*() const;
  ilist_iterator &operator++();
  ilist_iterator operator++(int);
  friend bool operator==(const ilist_iterator &LHS, const ilist_iterator &RHS);
  friend bool operator!=(const ilist_iterator &LHS, const ilist_iterator &RHS);
};
} // namespace llvm

template <typename T1>
using t1 = llvm::ilist_iterator<
    llvm::ilist_detail::node_options<T1, false, false, void, false, void>, false,
    false>;

template <typename T1>
using t2 = llvm::ilist_iterator<
    llvm::ilist_detail::node_options<T1, false, false, void, false, void>, true,
    false>;

template <typename T1> bool f1(const t1<T1> &a, const t1<T1> &b) {
  return operator!=(a, b);
}

template <typename T1> bool f2(const t1<T1> &a, const t1<T1> &b) {
  return operator==(a, b);
}

template <typename T1> T1 &f3(const t1<T1> &it) { return it.operator*(); }

template <typename T1> bool f4(const t2<T1> &a, const t2<T1> &b) {
  return operator!=(a, b);
}

template <typename T1> bool f5(const t2<T1> &a, const t2<T1> &b) {
  return operator==(a, b);
}

template <typename T1> T1 &f6(const t2<T1> &it) { return it.operator*(); }
