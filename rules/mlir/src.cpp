// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// mlir::Operation / Block / Region -> THE .td-GENERATED RUST MODEL, and with it
// the walk over a block's operations that rules/ilist has to refuse.
//
// WHY THIS MODULE EXISTS
// ---------------------------------------------------------------------------
// `--opaque-namespace=mlir` turns these three types into fieldless handles
// (`#[repr(transparent)] pub struct mlir_Operation(pub u64);`).  A handle has no
// Next/Prev link, so rules/ilist correctly gives operator++/-- NO rule and 11
// TUs abort on `unsupported CXXOperatorCallExpr: ++`.  The port does not need to
// translate MLIR to fix that: `dataflowir-gen` already carries a model of these
// IR shapes generated from the same TableGen (see its src/lib.rs -- 7,458 op
// lines re-printed byte-exactly, two whole files 1,792/1,792 and 7,283/7,283),
// and in it a block's operations are an ORDINARY `Vec`:
//
//     pub struct OpInst { ... pub regions: Vec<Region>, ... }   // fmt.rs:391
//     pub struct Block  { ... pub ops: Vec<OpInst>, }           // fmt.rs:435
//     pub struct Region { pub blocks: Vec<Block>, }             // fmt.rs:444
//
// A Vec is CONTIGUOUS, and that is what makes stepping expressible here.  The
// refusal recorded in rules/ilist is about the OPAQUE model: there, "pointer
// arithmetic treats an intrusive list as an array" is a real unsoundness because
// each mlir_Operation is separately allocated.  Against fmt::Block the ops ARE
// an array, so `p.add(1)` is not an invention -- it is the same step
// rules/vector already uses for std::vector's iterator (vector/tgt_unsafe.rs
// f34/f28), and libcc2rs' prefix_inc/postfix_inc/prefix_dec/postfix_dec are
// reused verbatim rather than restated.
//
// THE TYPE RULE WINS OVER --opaque-namespace, MEASURED
// ---------------------------------------------------------------------------
// Both mechanisms name the same type, so the order matters.  Mapper::Map
// searches the registered type rules FIRST and only falls through to
// `Opaque::OpaqueBaseName` at converter/mapper.cpp:1160, after every rule
// lookup has declined.  So a rule here takes precedence; the opaque pass may
// still EMIT an unused `mlir_Block` handle declaration (Convert() notes the
// record at converter.cpp:574 before consulting the mapper), which is inert.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL
// ---------------------------------------------------------------------------
// The reason rules/raw_ostream, rules/stringref and rules/ilist all give at
// length: cpp-rule-preprocessor compiles this file with a fixed flag set and
// reaching real MLIR headers would need an absolute -I into whatever MLIR tree
// the target project built.  `llvm::ilist_iterator` is restated EXACTLY as
// rules/ilist restates it -- all six `node_options` parameters spelled out,
// defaults written -- because that is the spelling the converter searches for,
// and a different spelling would record keys that match nothing.
//
// WHAT THIS MODULE DELIBERATELY DOES NOT DO
// ---------------------------------------------------------------------------
// It does NOT re-map `llvm::ilist_iterator` itself: rules/ilist already maps it
// to the node pointer (`*mut T1` / `Ptr<T1>`), and a second type rule for the
// same key would tie and make `search` refuse as "equally specific".  This
// module adds only the stepping operators that rules/ilist leaves without a
// rule, so the two compose: ilist gives the iterator its representation,
// comparison and dereference; this gives it its step.
//
// PREFIX AND POSTFIX ARE SEPARATE RULES, AND SO ARE FORWARD AND REVERSE.
// Postfix returns the OLD position, prefix the NEW one -- identical at a
// discarded-result call site (`iter++;`) and different at `*iter++`, which the
// probe exercises.  A REVERSE iterator's `operator++` steps BACKWARD: f8/f9 map
// it to prefix_dec/postfix_dec, exactly as rules/vector does for
// std::vector::reverse_iterator (f121/f122).  Conflating either pair compiles
// and terminates while visiting the wrong element or the wrong order, so the
// probe prints the visit ORDER.
//
// Spelled `a.operator++()` / `a.operator++(0)` throughout, never `++a`: a bare
// spelling records no src entry and aborts every translation with
// "Expr rule loaded from IR but has no src".
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
  ilist_iterator &operator--();
  ilist_iterator operator--(int);
};

// The list itself.  `Block::getOperations()` returns one of these by reference
// (`iplist<Operation> &`), and every begin/end/rbegin/rend in the real code is
// either on it or forwarded from Block.
template <class T> class iplist {
public:
  using iterator =
      ilist_iterator<ilist_detail::node_options<T, false, false, void, false,
                                                void>,
                     false, false>;
  using reverse_iterator =
      ilist_iterator<ilist_detail::node_options<T, false, false, void, false,
                                                void>,
                     true, false>;
  iterator begin();
  iterator end();
  reverse_iterator rbegin();
  reverse_iterator rend();
  // iplist takes OWNERSHIP of the node it is handed, which against the
  // `Vec<OpInst>` model is a push of the pointee by value.  Needed here because
  // it is the only way a probe can put elements into the list at all.
  void push_back(T *node);
};
} // namespace llvm

namespace mlir {
class Operation;
class Region;

class Block {
public:
  using OpListType = llvm::iplist<Operation>;
  using iterator = OpListType::iterator;
  using reverse_iterator = OpListType::reverse_iterator;
  OpListType &getOperations();
  iterator begin();
  iterator end();
  reverse_iterator rbegin();
  reverse_iterator rend();
};
} // namespace mlir

// ---- type rules -----------------------------------------------------------
using t1 = mlir::Operation;
using t2 = mlir::Block;
using t3 = mlir::Region;
template <typename T1> using t4 = llvm::iplist<T1>;

// ---- the op list ----------------------------------------------------------
inline mlir::Block::OpListType &f1(mlir::Block &b) { return b.getOperations(); }

template <typename T1>
typename llvm::iplist<T1>::iterator f2(llvm::iplist<T1> &l) {
  return l.begin();
}

template <typename T1>
typename llvm::iplist<T1>::iterator f3(llvm::iplist<T1> &l) {
  return l.end();
}

template <typename T1>
typename llvm::iplist<T1>::reverse_iterator f4(llvm::iplist<T1> &l) {
  return l.rbegin();
}

template <typename T1>
typename llvm::iplist<T1>::reverse_iterator f5(llvm::iplist<T1> &l) {
  return l.rend();
}

// ---- stepping: FORWARD ----------------------------------------------------
// t_fwd/t_rev are spelled inline rather than aliased, so that this module
// registers no type rule for the iterator (rules/ilist owns that key).
template <typename T1>
llvm::ilist_iterator<
    llvm::ilist_detail::node_options<T1, false, false, void, false, void>, false,
    false> &
f6(llvm::ilist_iterator<
    llvm::ilist_detail::node_options<T1, false, false, void, false, void>, false,
    false> &it) {
  return it.operator++();
}

template <typename T1>
llvm::ilist_iterator<
    llvm::ilist_detail::node_options<T1, false, false, void, false, void>, false,
    false>
f7(llvm::ilist_iterator<
       llvm::ilist_detail::node_options<T1, false, false, void, false, void>,
       false, false>
       a0,
   int a1) {
  return a0.operator++(a1);
}

// ---- stepping: REVERSE.  ++ GOES BACKWARD --------------------------------
template <typename T1>
llvm::ilist_iterator<
    llvm::ilist_detail::node_options<T1, false, false, void, false, void>, true,
    false> &
f8(llvm::ilist_iterator<
    llvm::ilist_detail::node_options<T1, false, false, void, false, void>, true,
    false> &it) {
  return it.operator++();
}

template <typename T1>
llvm::ilist_iterator<
    llvm::ilist_detail::node_options<T1, false, false, void, false, void>, true,
    false>
f9(llvm::ilist_iterator<
       llvm::ilist_detail::node_options<T1, false, false, void, false, void>,
       true, false>
       a0,
   int a1) {
  return a0.operator++(a1);
}

// ---- stepping: operator--, both directions -------------------------------
template <typename T1>
llvm::ilist_iterator<
    llvm::ilist_detail::node_options<T1, false, false, void, false, void>, false,
    false> &
f10(llvm::ilist_iterator<
    llvm::ilist_detail::node_options<T1, false, false, void, false, void>, false,
    false> &it) {
  return it.operator--();
}

template <typename T1>
llvm::ilist_iterator<
    llvm::ilist_detail::node_options<T1, false, false, void, false, void>, false,
    false>
f11(llvm::ilist_iterator<
        llvm::ilist_detail::node_options<T1, false, false, void, false, void>,
        false, false>
        a0,
    int a1) {
  return a0.operator--(a1);
}

template <typename T1>
llvm::ilist_iterator<
    llvm::ilist_detail::node_options<T1, false, false, void, false, void>, true,
    false> &
f12(llvm::ilist_iterator<
    llvm::ilist_detail::node_options<T1, false, false, void, false, void>, true,
    false> &it) {
  return it.operator--();
}

template <typename T1>
llvm::ilist_iterator<
    llvm::ilist_detail::node_options<T1, false, false, void, false, void>, true,
    false>
f13(llvm::ilist_iterator<
        llvm::ilist_detail::node_options<T1, false, false, void, false, void>,
        true, false>
        a0,
    int a1) {
  return a0.operator--(a1);
}

// ---- Block's own begin/end, which the real call sites use directly --------
inline mlir::Block::iterator f14(mlir::Block &b) { return b.begin(); }
inline mlir::Block::iterator f15(mlir::Block &b) { return b.end(); }
inline mlir::Block::reverse_iterator f16(mlir::Block &b) { return b.rbegin(); }
inline mlir::Block::reverse_iterator f17(mlir::Block &b) { return b.rend(); }

// ---- mutation: the only way to populate the list ---------------------------
template <typename T1> void f18(llvm::iplist<T1> &l, T1 *node) {
  return l.push_back(node);
}

// ---------------------------------------------------------------------------
// ROW 2: `!=` ON AN MLIR RANGE ITERATOR.
//
// `llvm::detail::indexed_accessor_range_base<...>::iterator` is the iterator of
// every MLIR *Range* (OperandRange, ResultRange, ...).  Two census TUs --
// Transform/Sentient/Analyses/UniformGroupAnalysis.cpp and
// Transform/Sentient/Deuniform.cpp -- abort with `unsupported
// CXXOperatorCallExpr: !=` on exactly one instantiation:
//
//   bool llvm::iterator_facade_base<
//       llvm::detail::indexed_accessor_range_base<
//           mlir::OperandRange, mlir::OpOperand *,
//           mlir::Value, mlir::Value, mlir::Value>::iterator,
//       std::random_access_iterator_tag, mlir::Value, long,
//       mlir::Value, mlir::Value>::operator!=(const ...::iterator &) const
//
// THE KEY NAMES THE CRTP BASE, NOT THE DERIVED ITERATOR.  `operator!=` is
// declared ONLY on iterator_facade_base (llvm/ADT/iterator.h:183); the nested
// `iterator` declares `operator==` and inherits `!=`.  A rule keyed on the
// derived class would match nothing, so iterator_facade_base is restated here
// with its full defaulted parameter list -- the same reason rules/smallset and
// rules/mapped_iterator restate it.
//
// PointerT AND ReferenceT ARE NOT THE DEFAULTS.  The instantiation passes
// `mlir::Value` for both (a Value is a handle, so MLIR's ranges hand it out by
// value), where the defaults are `T *` and `T &`.  Because the last argument is
// non-defaulted nothing is elided, which is why all six arguments appear in the
// key text above and all six must appear below.
//
// BaseT BINDS THE WHOLE POINTER (T3 = `mlir::OpOperand *`), not the pointee.
// The iterator's state is `{ BaseT base; ptrdiff_t index; }`, so the target
// representation is the PAIR `(T3, i64)` -- a tuple with concrete members is a
// legal representation (precedent: rules/tuple's `t4<T1,T2> -> (*mut T1, *mut
// T2)`), and it is the faithful one: MLIR's `operator==` is
// `base == rhs.base && index == rhs.index`, i.e. POSITION IDENTITY over BOTH
// fields.  Comparing the index alone would call two iterators into DIFFERENT
// ranges equal, and comparing a single fused pointer would be wrong because
// `base` for an OperandRange is an `OpOperand *` while the element type handed
// out is a `mlir::Value` -- the two are not the same stride.
//
// operator*, operator+=/-= and operator- are NOT mapped: dereferencing an
// OperandRange iterator is `base[index].get()`, which needs a model of
// mlir::OpOperand that this port does not have.  `!=` is what the two TUs stop
// on, and `!=` reads only the position.

namespace std {
// The tag the range iterator passes as IteratorCategoryT
// (llvm/ADT/STLExtras.h).  Restated rather than #include <iterator>, for the
// reason given at the top of this file: the rule preprocessor's fixed flag set.
struct random_access_iterator_tag {};
} // namespace std

namespace llvm {

// Restated from llvm/ADT/iterator.h:77.  operator!= is a member of THIS class,
// so its full defaulted parameter list is load-bearing for the key text.
template <typename DerivedT, typename IteratorCategoryT, typename T,
          typename DifferenceTypeT = long, typename PointerT = T *,
          typename ReferenceT = T &>
class iterator_facade_base {
public:
  using iterator_category = IteratorCategoryT;
  using value_type = T;
  using difference_type = DifferenceTypeT;
  using pointer = PointerT;
  using reference = ReferenceT;

  // llvm/ADT/iterator.h:183.
  bool operator!=(const DerivedT &RHS) const;
};

namespace detail {
// Restated from llvm/ADT/STLExtras.h.  Only the nested `iterator` is needed:
// the range's own begin()/end() already translate, and the two TUs stop inside
// the loop condition.
template <typename DerivedT, typename BaseT, typename T,
          typename PointerT = T *, typename ReferenceT = T &>
class indexed_accessor_range_base {
public:
  class iterator
      : public iterator_facade_base<iterator, std::random_access_iterator_tag,
                                    T, long, PointerT, ReferenceT> {
  public:
    iterator() = default;
    // `operator==` is declared, `operator!=` deliberately is NOT: `!=` must
    // resolve to the INHERITED member on iterator_facade_base so that the
    // recorded key names that base, which is what the signature above says.
    bool operator==(const iterator &rhs) const;
  };
};
} // namespace detail
} // namespace llvm

// The iterator's TYPE.  Nothing else in the rule tree names this key (grepped),
// so there is no tie.
template <typename T1, typename T2, typename T3>
using t5 = typename llvm::detail::indexed_accessor_range_base<T2, T3, T1, T1,
                                                              T1>::iterator;

// operator!= -- the INHERITED member on iterator_facade_base.  Spelled
// `a.operator!=(b)` and never `a != b`: a bare operator spelling records no src
// entry and aborts every translation that loads this module.
template <typename T1, typename T2, typename T3>
bool f19(
    const typename llvm::detail::indexed_accessor_range_base<T2, T3, T1, T1,
                                                             T1>::iterator &a,
    const typename llvm::detail::indexed_accessor_range_base<T2, T3, T1, T1,
                                                             T1>::iterator &b) {
  return a.operator!=(b);
}
