// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::mapped_iterator -- the POSITION HALF ONLY.
//
// WHAT IS MAPPED, AND WHAT DELIBERATELY IS NOT
// --------------------------------------------
// llvm/ADT/STLExtras.h defines
//
//     template <typename ItTy, typename FuncTy,
//               typename ReferenceTy =
//                   decltype(std::declval<FuncTy>()(*std::declval<ItTy>()))>
//     class mapped_iterator
//         : public iterator_adaptor_base<mapped_iterator<ItTy, FuncTy>, ItTy,
//               typename std::iterator_traits<ItTy>::iterator_category,
//               std::remove_reference_t<ReferenceTy>,
//               typename std::iterator_traits<ItTy>::difference_type,
//               std::remove_reference_t<ReferenceTy> *, ReferenceTy> {
//       ...
//       ReferenceTy operator*() const { return F(*this->I); }
//     private:
//       callable_detail::Callable<FuncTy> F{};
//     };
//
// so the object is a PAIR: the wrapped iterator `I` (inherited from
// iterator_adaptor_base) and the callable `F`.  This module maps the POSITION
// half and nothing else:
//
//   MAPPED   the type, for the instantiation whose wrapped iterator is a RAW
//            POINTER (`const T *`); prefix operator++ (iterator.h:242, which is
//            `++I; return *static_cast<DerivedT*>(this);` -- it touches ONLY I
//            and leaves F alone, which is why the step is expressible without a
//            representation for F); the friend operator== (iterator.h:257,
//            `LHS.I == RHS.I`, i.e. POSITION IDENTITY); and the inherited
//            operator!= from iterator_facade_base.
//
//   MAPPED BUT LOSSY   the two-argument constructor
//            `mapped_iterator(ItTy I, FuncTy F)`.  The target representation has
//            room for the POSITION ONLY, so the constructor KEEPS I AND THROWS F
//            AWAY.  That is stated here rather than hidden because it is the one
//            place in this module where information is destroyed, and it has a
//            direct consequence: operator* must STAY LOUD FOREVER as long as
//            this representation stands, because applying a function you did not
//            keep is not something you can do.  The constructor is mapped
//            anyway, and only for this reason: without it NOTHING can build a
//            mapped_iterator through the mapped surface, so the ++ body could
//            never be RUN, and an unrunnable body is an unverifiable body.  With
//            it, the walk (length, termination, ==/!= flip point) becomes
//            measurable; the transform does not.
//
//   NOT MAPPED, DELIBERATELY LOUD   operator* / operator-> / getCurrent() /
//            getFunction(), and every random-access operator (+=, -=, -, <, []).
//            operator* is `F(*this->I)` and CANNOT be expressed while the Rust
//            representation is a bare pointer: F was dropped by the constructor,
//            so there is nothing to apply.  A PAIR representation
//            `(*const T, fn(A)->R)` would be needed to change that -- see the
//            note at the end of this file for what that would cost.
//
// WHY THAT SPLIT IS THE DELIVERABLE AND NOT A SHORTCUT
// ----------------------------------------------------
// The measured gap is `ddc__ddl__ddl_conversion.cpp` aborting at :3743 on PREFIX
// ++ over `llvm::mapped_iterator<const mlir::Attribute *,
// mlir::StringAttr (*)(mlir::Attribute)>`, reached through
// iterator_adaptor_base.  That is the blocker this module closes.  Inventing a
// pair representation whose operator* could not be RUN -- because nothing here
// can construct a mapped_iterator in the first place -- would be a body with no
// evidence behind it, so operator* is left with NO RULE: it aborts, loudly, with
// the converter's own unmapped diagnostic rather than silently dropping the
// transform.
//
// WHY THE KEY NAMES iterator_adaptor_base AND NOT mapped_iterator
// ---------------------------------------------------------------
// mapped_iterator declares NEITHER ++ nor == nor !=.  Prefix ++ and the friend
// == are members of `llvm::iterator_adaptor_base<...>` (iterator.h:242, :257)
// and != is a member of `llvm::iterator_facade_base<...>` (iterator.h:183).  A
// rule keyed on the DERIVED class is a different entity and matches NOTHING --
// rules/smallset paid for exactly this with its operator!= (commit 6d9a856), and
// rules/ilist records the same shape.  So both CRTP bases are restated here with
// their full defaulted parameter lists, and the rules below are keyed through
// them.
//
// THE THIRD TEMPLATE PARAMETER IS NOT SPELLED, ON PURPOSE
// -------------------------------------------------------
// `ReferenceTy` is DEFAULTED, and clang's default PrintingPolicy has
// SuppressDefaultTemplateArgs = true, so the use site asks for the TWO-argument
// text `llvm::mapped_iterator<const mlir::Attribute *,
// mlir::StringAttr (*)(mlir::Attribute)>` -- which is exactly what the census
// reports as unmapped.  A restatement that took ReferenceTy as a rule template
// parameter and wrote it explicitly would record THREE arguments and match
// nothing (rules/smallset's SmallSet<T1, _, std::less<T1>> failure, same
// mechanism).  So the restatement below takes TWO parameters only and recovers
// the mapped-to type from the callable's type with a small return-type trait, so
// that the base-class arguments still come out as the real ones.

namespace std {
// Restated: the tag a raw pointer's iterator_traits reports, which is what
// mapped_iterator passes as IteratorCategoryT for a pointer ItTy.
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

// Restated from llvm/ADT/iterator.h:200.  Only the two members this module maps
// are declared: prefix operator++ (:242) and the friend operator== (:257).
template <typename DerivedT, typename WrappedIteratorT,
          typename IteratorCategoryT, typename T,
          typename DifferenceTypeT = long, typename PointerT = T *,
          typename ReferenceT = T &>
class iterator_adaptor_base
    : public iterator_facade_base<DerivedT, IteratorCategoryT, T,
                                  DifferenceTypeT, PointerT, ReferenceT> {
protected:
  WrappedIteratorT I;

  iterator_adaptor_base() = default;
  explicit iterator_adaptor_base(WrappedIteratorT u);

public:
  using difference_type = DifferenceTypeT;

  // iterator.h:242 -- PREFIX.  Postfix ++ is NOT declared: it lives on
  // iterator_facade_base and returns the OLD position, a different rule with
  // different semantics, and it is not mapped here.
  DerivedT &operator++();

  // iterator.h:257 -- a FRIEND taking the BASE by const reference, not the
  // derived class.
  friend bool operator==(const iterator_adaptor_base &LHS,
                         const iterator_adaptor_base &RHS);
};

// The callable's return type, recovered from a plain function-pointer FuncTy.
// This exists so mapped_iterator can be restated with TWO template parameters
// (see the header note) while still instantiating its base with the real
// value_type / pointer / reference arguments.
template <typename F> struct mapped_iterator_fnret;
template <typename R, typename A> struct mapped_iterator_fnret<R (*)(A)> {
  using type = R;
};

// Restated from llvm/ADT/STLExtras.h.  TWO parameters; ReferenceTy is not a
// parameter at all.
template <typename ItTy, typename FuncTy>
class mapped_iterator
    : public iterator_adaptor_base<
          mapped_iterator<ItTy, FuncTy>, ItTy,
          std::random_access_iterator_tag,
          typename mapped_iterator_fnret<FuncTy>::type, long,
          typename mapped_iterator_fnret<FuncTy>::type *,
          typename mapped_iterator_fnret<FuncTy>::type> {
public:
  mapped_iterator() = default;
  // STLExtras.h: `mapped_iterator(ItTy U, FuncTy F) : BaseT(std::move(U)),
  // F(std::move(F)) {}`.  Mapped, LOSSILY -- see the header note.
  mapped_iterator(ItTy I, FuncTy F);
  // operator*, getCurrent and getFunction are NOT declared: F was dropped, so
  // they stay unmapped and loud.
};

} // namespace llvm

// The TYPE.  `const T1 *` rather than a bare `T1 *`: the measured instantiation
// wraps `const mlir::Attribute *`, and T1 binds to the pointee so the const is
// part of the key's spelling rather than folded into T1.
template <typename T1, typename T2, typename T3>
using t1 = llvm::mapped_iterator<const T1 *, T3 (*)(T2)>;

// PREFIX operator++ -- the INHERITED member, keyed on iterator_adaptor_base.
//
// SPELLED `a.operator++()`, never `++a`: a bare operator spelling records NO src
// entry, and the target-side rule then aborts EVERY translation that loads this
// module with `LLVM ERROR: Expr rule loaded from IR but has no src`.
//
// POSITION ONLY.  The body advances the wrapped pointer; the callable is
// untouched, which is what LLVM's `++I` does.
template <typename T1, typename T2, typename T3>
llvm::mapped_iterator<const T1 *, T3 (*)(T2)> &
f1(llvm::mapped_iterator<const T1 *, T3 (*)(T2)> &a) {
  return a.operator++();
}

// operator== -- the FRIEND on iterator_adaptor_base (iterator.h:257), whose body
// is `LHS.I == RHS.I`: POSITION IDENTITY, not element identity.
template <typename T1, typename T2, typename T3>
bool f2(const llvm::mapped_iterator<const T1 *, T3 (*)(T2)> &a,
        const llvm::mapped_iterator<const T1 *, T3 (*)(T2)> &b) {
  return operator==(a, b);
}

// operator!= -- the INHERITED member on iterator_facade_base (iterator.h:183).
template <typename T1, typename T2, typename T3>
bool f3(const llvm::mapped_iterator<const T1 *, T3 (*)(T2)> &a,
        const llvm::mapped_iterator<const T1 *, T3 (*)(T2)> &b) {
  return a.operator!=(b);
}

// The TWO-ARGUMENT CONSTRUCTOR -- `mapped_iterator(ItTy I, FuncTy F)`.
//
// LOSSY BY CONSTRUCTION.  The target bodies are the IDENTITY ON THE FIRST
// ARGUMENT: the wrapped iterator becomes the whole object and the callable is
// DISCARDED, because the representation (`*const T1` / `Ptr<T1>`) has no second
// field to put it in.  The second parameter is therefore present in the target
// signature -- the key names it, so it must be -- and unread in the body.
//
// This is mapped despite being lossy because it is the only way the position
// half can be EXERCISED: with no constructor, nothing outside LLVM's own headers
// can produce a mapped_iterator, and f1/f2/f3 could never run.  It is NOT a step
// towards mapping operator*: it is the reason operator* cannot be mapped, since
// after this runs the callable no longer exists anywhere in the program.
template <typename T1, typename T2, typename T3>
llvm::mapped_iterator<const T1 *, T3 (*)(T2)> f4(const T1 *i, T3 (*f)(T2)) {
  return llvm::mapped_iterator<const T1 *, T3 (*)(T2)>(i, f);
}

// COULD operator* BE MAPPED BY KEEPING THE CALLABLE?  WHAT IT WOULD TAKE.
// ---------------------------------------------------------------------------
// Yes, in principle, and the shape is known: represent the type as the PAIR
// `(*const T1, unsafe fn(T2) -> T3)` (refcount: `(Ptr<T1>, fn(T2) -> T3)`) --
// i.e. exactly the two fields LLVM's object has -- make f4 build the pair
// instead of dropping its second argument, make f1/f2/f3 project field .0, and
// then operator* is `(a.1)(*a.0)`.  A Rust tuple is a legal rule
// representation (rules/pair and rules/tuple both use one), and the fn-pointer
// spelling is already exercised by rules/sstream's f33
// (`a1: unsafe fn(*mut u32) -> *mut u32`), so neither half is speculative.
//
// It is NOT done here, and not because it is hard: because it is a DIFFERENT
// representation, and switching to it silently invalidates the only evidence
// this module has.  Every rule below would change shape at once (f1's receiver
// becomes `&mut (…, …)` and `prefix_inc` no longer applies to it directly), and
// the one thing measured so far -- that the walk steps exactly one element and
// terminates exactly at the end -- would have to be re-measured from scratch,
// against a representation whose `==` must be made to compare .0 ALONE.  Two
// mapped_iterators over the same position with different callables ARE equal in
// LLVM (`LHS.I == RHS.I`), so a derived `PartialEq` on the pair would be WRONG,
// and function pointers do not compare meaningfully in Rust anyway.  That is a
// real trap, it is invisible in a diff, and paying for it belongs in a change
// whose whole purpose is operator*, not in the change that first makes the
// position half runnable.
