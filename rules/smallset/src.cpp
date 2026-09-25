// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::SmallSetIterator's comparison.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/ADT/SmallSet.h>
// ---------------------------------------------------------------------------
// Same reason as rules/densemap, rules/stringmap and rules/stringref:
// cpp-rule-preprocessor compiles this file with a fixed flag set, and the only
// flags that would reach LLVM's headers are absolute -I paths into whatever
// LLVM tree the target project happens to have built.  So the signatures are
// restated here, and a restatement matches iff it agrees with LLVM exactly.
//
// WHAT THIS ITERATOR ACTUALLY IS -- a tagged union, not one representation
// -----------------------------------------------------------------------
// llvm/ADT/SmallSet.h:32 is
//
//     template <typename T, unsigned N, typename C>
//     class SmallSetIterator
//         : public iterator_facade_base<SmallSetIterator<T, N, C>,
//                                       std::forward_iterator_tag, T> {
//       union { SetIterTy SetIter; VecIterTy VecIter; };
//       bool IsSmall;
//
// i.e. SmallSet<T, N, C> holds BOTH a SmallVector<T, N> and a std::set<T, C>
// (SmallSet.h:133-134) and the iterator wraps whichever half is live, tagged by
// IsSmall.  `isSmall()` is `Set.empty()` (SmallSet.h:231): below N elements the
// SmallVector is the store; the N+1'th insert MOVES every element into the
// std::set and CLEARS the vector (insertImpl, SmallSet.h:243-253), and the set
// half is used from then on.
//
// SO WHICH REPRESENTATION IS MODELLED: ONE -- an ordered key set -- and the
// small/large split is NOT OBSERVABLE through the surface mapped here.  That is
// a claim about the C++, not a convenience:
//
//   * isSmall() is a property of the CONTAINER, not of a position in it.  Every
//     iterator handed out by one SmallSet at one time has the same IsSmall, so
//     the `if (IsSmall != RHS.IsSmall) return false;` arm of operator==
//     (SmallSet.h:106) is unreachable for two iterators into the SAME set.
//   * It is reachable only for iterators into DIFFERENT sets -- and comparing
//     iterators from two different containers is UNDEFINED, not merely
//     unspecified, in C++: [iterator.requirements.general]/8 makes == only
//     defined for iterators into the same sequence.  For the small half those
//     are two unrelated SmallVector element POINTERS; for the large half two
//     std::set node pointers.  So no conforming program can see that arm, and a
//     translation is not obliged to reproduce it.
//   * The two halves also differ in ORDER: the SmallVector half iterates in
//     INSERTION order, the std::set half in C-order (std::less by default).
//     That IS observable -- but through operator*/operator++, which this module
//     deliberately does not map (see NOT MODELLED).  The comparison itself
//     cannot see it.
//
// The probe therefore crosses the boundary anyway rather than assuming the
// above: it runs the identical checks on a SmallSet<long, 4> holding 3 elements
// (small half, SmallVector) and on one holding 9 (large half, std::set), and
// both must agree.
//
// WHAT AN ITERATOR COMPARISON MEANS -- the same argument as rules/densemap
// ----------------------------------------------------------------------
// C++ is the specification.  LLVM's own body at SmallSet.h:104-110 reduces, for
// the same-container case that is the only defined one, to
//
//     return VecIter == RHS.VecIter;   // small: element pointers
//     return SetIter == RHS.SetIter;   // large: std::set node identity
//
// i.e. IDENTITY OF POSITION and NOT equality of the pointed-to values, exactly
// as DenseMap.h:1225 (`LHS.Ptr == RHS.Ptr`), DenseSet.h:145, StringMap.h:488
// and ilist_iterator.h:175 are.
//
// FOR A DEDUPLICATING SET THOSE TWO COINCIDE, and saying so is the honest
// version of the argument rules/densemap and rules/stringmap make.  Their
// probes distinguish position from value by putting the SAME value at two
// DIFFERENT keys -- a map can hold duplicate values.  A SmallSet cannot: insert
// is a no-op when the element is already present (SmallSet.h:182), so within one
// container two distinct positions NECESSARILY hold distinct elements, and
// end() holds none.  Position identity and element identity are therefore the
// same relation on every pair a defined program can compare.  That is precisely
// what makes the existing key-based iterator PartialEq sound here rather than
// approximately right -- and it is also why the probe below cannot catch a
// value-comparing body the way rules/stringmap's could: there is no such body to
// distinguish.  What the probe does catch is the failure modes that remain
// reachable -- an always-true or always-false or inverted comparison, and
// find()/end() disagreeing.
//
// So NO NEW RUNTIME TYPE IS NEEDED.  libcc2rs's map iterators
// (libcc2rs/src/iterators.rs:60) already have exactly this semantics --
// PartialEq is position by key, with end() being `key: None` -- and a set is a
// map whose value is (), which is the representation rules/set already gives
// std::set.  rules/stringmap reused them rather than adding a runtime type and
// this module does the same.  SmallSet is ORDERED (its large half is
// std::set<T, C>, its comparator defaulting to std::less<T>), so unlike DenseMap
// and StringMap the ordered BTreeMap the iterators walk is not even an
// order-refinement here -- it is the same order, for the large half.
//
// operator== IS AN IN-CLASS MEMBER, operator!= IS INHERITED, and conflating
// them costs a silent miss
// ------------------------------------------------------------------------
// This is the trap rules/ilist and rules/stringmap both hit, in a THIRD shape.
// StringMapIterBase declares both operators as HIDDEN FRIENDS, so restating
// them at namespace scope reports "No viable function" and the rule silently
// never resolves.  SmallSetIterator is different and has to be restated
// differently:
//
//   * operator== is declared and defined INSIDE the class as a MEMBER
//     (SmallSet.h:104) -- `bool operator==(const SmallSetIterator &RHS) const`.
//     Not a friend, not a namespace-scope template.  So it is restated as a
//     member below.
//   * operator!= is NOT DECLARED BY SmallSetIterator AT ALL.  It comes from the
//     CRTP base, iterator.h:183 --
//         bool operator!=(const DerivedT &RHS) const {
//           return !(static_cast<const DerivedT &>(*this) == RHS);
//         }
//     so the entity the call resolves to is a member of
//     iterator_facade_base<SmallSetIterator<T, N, C>, std::forward_iterator_tag,
//     T>, and the signature string names THAT class, not SmallSetIterator.
//     Declaring an own operator!= on the restated SmallSetIterator would
//     produce a different string and would never match -- the same class of
//     silent non-match rules/densemap records at f18/f19 for DenseMapPair's
//     base.  So iterator_facade_base is restated too, with its full defaulted
//     parameter list (iterator.h:77-79), and the restated SmallSetIterator
//     derives from it exactly as LLVM's does.
//
// Note iterator.h:182 guards operator!= with `#ifndef
// __cpp_impl_three_way_comparison`: under C++20 the rewritten `!(a == b)` is used
// instead and no operator!= entity exists.  f4 below is therefore the rule for
// the pre-C++20 spelling; if a target TU is compiled as C++20 the `!=` site
// rewrites to operator== and f2/f3 cover it.  Both are mapped, so the coverage
// does not depend on which standard the target project uses -- which is the
// point of mapping both rather than picking one.
//
// THE COMPARATOR IS A DEFAULTED THIRD PARAMETER, AND THE TWO TYPES DIFFER ON IT.
// SmallSet.h:127 is `template <typename T, unsigned N, typename C =
// std::less<T>>` and SmallSetIterator (SmallSet.h:32) is `template <typename T,
// unsigned N, typename C>` with NO default.  Mapper::ToString prints through
// clang's default PrintingPolicy, where SuppressDefaultTemplateArgs is TRUE, so
// the polarity is:
//
//   * the USE SITE ELIDES a defaulted argument.  Measured from inside
//     Mapper::search(), the container lookup arrives as
//         key='llvm::SmallSet'  txt='llvm::SmallSet<long, _>'
//     -- TWO arguments; the defaulted std::less is suppressed because the use
//     site never wrote it.
//   * a RULE that WRITES the argument explicitly KEEPS it, because an
//     explicitly-written template argument is never suppressed.  The earlier
//     three-argument restatement below therefore offered
//         cand='llvm::SmallSet<T1, _, std::less<T1>>'
//     and never matched the two-argument text the use site asked for.  The
//     module built, validated and covered nothing on the container.
//
// SmallSetIterator is the control that proved this: its three arguments ARE
// written at the use site (it has no defaults to suppress), the lookup arrives as
//     txt='llvm::SmallSetIterator<long, _, std::less<long>>'
// and the three-argument iterator rules in this file MATCH.  Hence the shapes
// below: llvm::SmallSet is restated with TWO parameters, SmallSetIterator with
// THREE.  (An earlier revision of this comment asserted the opposite polarity --
// that the rule elides and the use site keeps.  It is wrong; this is the measured
// direction.)  N stays a genuine non-type parameter -- the same
// `template <typename T1, unsigned T2>` form rules/smallvector uses for
// SmallVector<T1, T2> -- because the census shows the sites instantiate it at
// more than one value.
//
// NOT MODELLED, deliberately -- each is a loud abort rather than a guessed body:
// SmallSet itself (insert, erase, count, contains, size, empty, clear,
// begin/end, its constructors and insert_range), the iterator's operator++ and
// operator* (which is where the small/large ORDER difference discussed above
// would have to be faced, and it is not faceable without choosing one of the two
// orders -- so it stays loud), the rest of iterator_facade_base's operators, and
// SmallPtrSet/SmallPtrSetIterator, which are a separate type with a separate
// small/large story.  Only the iterator comparison is the survey gap; everything
// else stays loud.

// NO #include HERE EITHER, for the same reason the llvm declarations are local:
// cpp-rule-preprocessor's fixed flag set does not reach a C++ standard library
// (`#include <cstddef>` is a `fatal error: 'cstddef' file not found`), so the
// two std entities the signatures name are restated as well.  Both are named
// only to make the canonical type string agree:
//   * std::less<T> is SmallSet's DEFAULT comparator argument (SmallSet.h:127),
//     and the census shows the converter spells it out rather than eliding it.
//   * std::forward_iterator_tag is the category SmallSetIterator passes to
//     iterator_facade_base (SmallSet.h:34).
// iterator_facade_base's DifferenceTypeT defaults to std::ptrdiff_t, which
// canonicalises to `long` on this target, so it is spelled `long` below --
// restating a std::ptrdiff_t typedef would canonicalise to the same string.
namespace std {
template <typename T> struct less {};
struct forward_iterator_tag {};
// SmallSet::insert returns std::pair<const_iterator, bool> (SmallSet.h:183).
// Restated rather than #include <utility>: libc++'s <utility> drags in the real
// std::less and would clash with the restatement above.  The canonical string is
// the same either way -- rules/set, which DOES include <utility>, records its
// insert key as `std::pair<..., bool> std::set<T1>::insert(const T1 &)`, i.e.
// clang suppresses libc++'s inline namespace, so `std::pair` is what both spell.
template <typename T1, typename T2> struct pair {
  T1 first;
  T2 second;
};
} // namespace std

namespace llvm {

// Restated from llvm/ADT/iterator.h:77.  The FULL defaulted parameter list
// matters: operator!= is a member of THIS class, so the signature string the
// converter searches for spells this class's canonical arguments -- including
// the three defaults.  Only operator!= is declared; the others are not mapped.
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

  // llvm/ADT/iterator.h:183 -- a MEMBER of the base, not of SmallSetIterator.
  // See the header comment: an operator!= declared on the derived class is a
  // different entity and the rule would silently never resolve.
  bool operator!=(const DerivedT &RHS) const;
};

// Restated from llvm/ADT/SmallSet.h:32.  Derives from iterator_facade_base
// exactly as LLVM's does so that `a != b` resolves into the base.
template <typename T, unsigned N, typename C>
class SmallSetIterator
    : public iterator_facade_base<SmallSetIterator<T, N, C>,
                                  std::forward_iterator_tag, T> {
public:
  SmallSetIterator();

  // llvm/ADT/SmallSet.h:104 -- an in-class MEMBER, taking the iterator by
  // const reference.  Restated as a member for the reason in the header comment.
  bool operator==(const SmallSetIterator &RHS) const;
};

// Restated from llvm/ADT/SmallSet.h:128.  The only part of SmallSet this module
// needs is the nested typedef at :150, which is how the target code names the
// iterator type -- the same route rules/stringmap and rules/densemap take.  The
// NOTE: the comparator is NOT a parameter of this restatement at all -- SmallSet
// is declared with TWO parameters and the std::less<T> is baked into the
// const_iterator typedef below.  That is what makes every recorded SmallSet key
// come out as `llvm::SmallSet<T1, _>`, which is exactly the two-argument text the
// use site asks for once SuppressDefaultTemplateArgs has elided its default
// comparator (see the polarity note in the header).  Declaring
// `typename C = std::less<T>` and writing it explicitly at the use sites below
// would instead record `llvm::SmallSet<T1, _, std::less<T1>>` and match nothing.
// The typedef still instantiates SmallSetIterator with all THREE arguments, so
// the iterator keys are unchanged and the working iterator rules do not regress.
template <typename T, unsigned N> class SmallSet {
public:
  using key_type = T;
  using value_type = T;
  // llvm/ADT/SmallSet.h:150 -- using const_iterator = SmallSetIterator<T, N, C>,
  // with C pinned to the real default std::less<T> so the iterator key keeps all
  // three arguments while SmallSet's own key stays two.
  using const_iterator = SmallSetIterator<T, N, std::less<T>>;

  // The MEMBERS THE TYPE RULE ALONE WAS NOT ENOUGH FOR.  See the note above t3:
  // a type rule for the container did not stop the converter emitting
  // `llvm_SmallSet_long_____std_less_long__`, because with every member call on
  // the receiver unmapped the converter takes the port-the-class path (it emits
  // `<llvm_SmallSet_...>::default()` and `insert_pconsti64`) and never consults
  // the type rule at all.  rules/stringmap records the same finding in a
  // different shape.  Only the members below are declared; the rest of SmallSet
  // stays out and stays loud.
  //
  // size_type is size_t (SmallSet.h:147), which canonicalises to `unsigned long`
  // on this target -- the string rules/set's f1 key also carries -- so it is
  // spelled directly rather than restating a std::size_t typedef.
  SmallSet();
  unsigned long size() const;
  // SmallSet.h:183/185 -- BOTH insert overloads, the lvalue and the rvalue one.
  // The probe reaches the rvalue one for `small.insert(10)` and the lvalue one
  // for `s.insert(a)`, and they are two different signature strings.
  std::pair<const_iterator, bool> insert(const T &V);
  std::pair<const_iterator, bool> insert(T &&V);
  // SmallSet.h:215/221 -- both are const; SmallSet has no non-const begin/end.
  const_iterator begin() const;
  const_iterator end() const;
};

} // namespace llvm

template <typename T1, unsigned T2>
using t1 = typename llvm::SmallSet<T1, T2>::const_iterator;

// The same type reached by its own name rather than through SmallSet's typedef,
// for sites that spell `llvm::SmallSetIterator<T, N>` directly.  It
// canonicalises to the same string; both spellings are written out so neither
// route depends on the other resolving.
template <typename T1, unsigned T2>
using t2 = llvm::SmallSetIterator<T1, T2, std::less<T1>>;

// operator== -- the in-class member (SmallSet.h:104).
//
// SPELLED AS AN EXPLICIT MEMBER CALL, and that is not cosmetic.  The plain
// `a == b` / `a != b` spellings were tried first, as two extra rules alongside
// these, and cpp-rule-preprocessor emitted NO src entry for them: ir_src.json
// came back with f1 and f4 only.  The rule then exists on the target side with
// nothing to match against, and the converter does not warn -- it ABORTS every
// translation that loads the module:
//     LLVM ERROR: Expr rule loaded from IR but has no src
// (rc=134, both models, before any output is written).  So an operator-call site
// must be restated in the `a.operator==(b)` form; the resolved signatures below
// are what the two rules here actually key on, read back out of
// /home/agent/work/pin/ir/smallset/ir_src.json:
//     f1: bool llvm::SmallSetIterator<T1, _, std::less<T1>>::operator==(
//             const llvm::SmallSetIterator<T1, _, std::less<T1>> &) const
//     f2: bool llvm::iterator_facade_base<
//             llvm::SmallSetIterator<T1, _, std::less<T1>>,
//             std::forward_iterator_tag, T1>::operator!=(
//             const llvm::SmallSetIterator<T1, _, std::less<T1>> &) const
// f1's string is exactly the census's unmapped type applied to ==, and f2's
// confirms the header comment above: `!=` really does key on
// iterator_facade_base, so restating the CRTP base was load-bearing and an
// operator!= declared on SmallSetIterator would have matched nothing.
template <typename T1, unsigned T2>
bool f1(typename llvm::SmallSet<T1, T2>::const_iterator a,
        typename llvm::SmallSet<T1, T2>::const_iterator b) {
  return a.operator==(b);
}

// operator!= -- the INHERITED member (iterator.h:183).
template <typename T1, unsigned T2>
bool f2(typename llvm::SmallSet<T1, T2>::const_iterator a,
        typename llvm::SmallSet<T1, T2>::const_iterator b) {
  return a.operator!=(b);
}

// The CONTAINER TYPE, and only the type.
//
// The probe showed the iterator comparisons above translating cleanly while the
// output still would not compile, with seven copies of
//     error[E0425]: cannot find type `llvm_SmallSet_long_____std_less_long__`
// i.e. the container SmallSet<long, 4, std::less<long>> had no Rust name.  This
// is the same shape rules/stringmap records for StringMap itself: mapping an
// iterator without naming the container it comes from covers the survey gap and
// still cannot be RUN, so there is no runtime evidence for the comparison
// bodies.  The mangled name in that error spells the comparator
// (`_____std_less_long__` is `<long, _, std::less<long>>`) because it is derived
// from the ported class, not from the lookup text -- the lookup itself asks for
// the two-argument `llvm::SmallSet<long, _>`, which is what the alias below now
// records.
//
// Mapping the type does NOT mean the container is modelled: insert / erase /
// count / contains / begin / end / size stay unmapped and loud.  What this gives
// is the ordered-key-set representation rules/set already uses for std::set, so
// the iterator rules above and the container agree on one Rust type.
// The comparator is NOT spelled out.  Written as `llvm::SmallSet<T1, T2>` the
// preprocessor reports the key as `llvm::SmallSet<T1, _>` -- TWO arguments --
// which is the text the use site asks for.  The earlier three-argument spelling
// `llvm::SmallSet<T1, T2, std::less<T1>>` recorded
// `llvm::SmallSet<T1, _, std::less<T1>>` and silently matched nothing, which is
// the whole reason this container rule used to be dead.
template <typename T1, unsigned T2>
using t3 = llvm::SmallSet<T1, T2>;

// SmallSet's own members -- the default constructor, insert, size, begin, end.
//
// WHY THESE ARE HERE AND WHAT THEY ARE NOT.  The header comment's NOT MODELLED
// list said SmallSet's surface stays loud, and that was not sustainable: t3
// alone left the container emitted as a ported C++ class (see the note above
// t3), so nothing could be RUN and the iterator comparison bodies above had no
// runtime evidence at all.  These five are exactly the members the verification
// probe reaches, and each has the SAME body rules/set gives the corresponding
// std::set member -- which is the point: SmallSet IS an ordered set of keys
// (its large half is literally std::set<T, C>), so the ordered BTreeMap
// representation is not an approximation for these.
//
// erase / count / contains / clear / insert_range / the initializer_list and
// iterator-range constructors are still NOT mapped and still abort loudly.  They
// would be more of the same bodies, but nothing measures them here.
//
// ORDER IS NOT AT STAKE in begin()/end() the way it would be for operator++ or
// operator*: those two remain unmapped, so an iterator obtained here can only be
// COMPARED, never walked or dereferenced.  That is why mapping begin/end does not
// have to choose between the SmallVector half's insertion order and the std::set
// half's C-order (src.cpp's header comment discusses that split).
template <typename T1, unsigned T2>
llvm::SmallSet<T1, T2> f3() {
  return llvm::SmallSet<T1, T2>();
}

template <typename T1, unsigned T2>
unsigned long f4(const llvm::SmallSet<T1, T2> &o) {
  return o.size();
}

// insert(const T &) -- SmallSet.h:183.  The pair's first is an iterator to the
// element that is IN the set afterwards, whether or not this call put it there,
// and second says whether it did (SmallSet.h:180-182).  Reinsertion of a
// present element is a NO-OP that still hands back an iterator to the
// incumbent -- which is what the probe uses in place of the find() SmallSet does
// not have.  rules/set's f11 body is exactly this contract.
template <typename T1, unsigned T2>
std::pair<typename llvm::SmallSet<T1, T2>::const_iterator, bool>
f5(llvm::SmallSet<T1, T2> &o, const T1 &v) {
  return o.insert(v);
}

// insert(T &&) -- SmallSet.h:185.  A separate entity and a separate key; the
// probe's `small.insert(10)` binds to this one, so mapping only the lvalue form
// would have left the probe untranslatable.
template <typename T1, unsigned T2>
std::pair<typename llvm::SmallSet<T1, T2>::const_iterator, bool>
f6(llvm::SmallSet<T1, T2> &o, T1 &&v) {
  return o.insert(static_cast<T1 &&>(v));
}

template <typename T1, unsigned T2>
typename llvm::SmallSet<T1, T2>::const_iterator
f7(const llvm::SmallSet<T1, T2> &o) {
  return o.begin();
}

template <typename T1, unsigned T2>
typename llvm::SmallSet<T1, T2>::const_iterator
f8(const llvm::SmallSet<T1, T2> &o) {
  return o.end();
}
