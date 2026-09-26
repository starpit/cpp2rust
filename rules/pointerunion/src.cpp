// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::PointerUnion<A, B, C> -- a DISCRIMINATED union of pointer types, stored
// as one word with the alternative's index in the low bits.
//
// THE ROW.  Two of the 16 remaining aborting census TUs, joint-largest after the
// std::set cluster:
//   dcc__src__Dialect__Uniform__Utils.cpp
//     cpp_type: llvm::PointerUnion<const mlir::Value *, mlir::OpOperand *,
//                                  mlir::detail::OpResultImpl *>
//   ddc__ddl__ddl_conversion.cpp
//     cpp_type: llvm::PointerUnion<mlir::Region *,
//                                  const std::unique_ptr<mlir::Region> *,
//                                  mlir::Region **>
//
// WHY A TYPE RULE IS THE WHOLE ROW, and why that is not a fudge.
// ---------------------------------------------------------------------------
// `grep -rn 'PointerUnion' dt_src` returns ZERO hits.  dt_src never names the
// type, never constructs one, and never calls `is<>`, `get<>`, `dyn_cast<>`,
// `getAddrOfPtr1()` or `isNull()` on one.  It reaches the converter ONLY as the
// OwnerT of an MLIR range: mlir/IR/Region.h:345,351 spells
//   class RegionRange : public llvm::detail::indexed_accessor_range_base<
//       RegionRange,
//       PointerUnion<Region *, const std::unique_ptr<Region> *, Region **>,
//       Region *, Region *, Region *>
// i.e. it is a BASE-CLASS TEMPLATE ARGUMENT that must merely be describable so
// the enclosing range can be typed.  The second instantiation above is
// ValueRange's analogue.  So every accessor is UNREACHABLE, exactly as
// llvm::MemoryBuffer's accessors were in rules/sourcemgr, and the hazard that
// could be fudged here cannot be reached to fudge.
//
// THE HAZARD, NAMED so a later reader knows it was avoided and not guessed:
// WHICH alternative is stored must be OBSERVABLE, or `is<A>()` and `is<B>()`
// answer the same way -- code that compiles and takes the wrong branch, this
// port's worst failure shape.  This module does NOT erase the discriminant: the
// model is `libcc2rs::Variant3<T1, T2, T3>`, the same discriminated runtime
// rules/variant uses for std::variant (its t2 is exactly
// `Variant3<T1, T2, T3>`), so when an accessor is eventually needed it is
// written as ONE RULE PER ALTERNATIVE against a discriminant that is already
// there -- the convention rules/variant established, not a new one.
//
// THE ACCESSORS ARE DELIBERATELY ABSENT, hence LOUD.  With no rule for `is<>` /
// `get<>` / `isNull()`, the converter aborts naming the operation rather than
// answering from a guess.  Nothing in dt_src calls them, so nothing regresses;
// if a future TU does, it aborts by name instead of silently taking a branch.
//
// ARITY IS FIXED AT THREE, on purpose.  The real declaration is variadic
// (`template <typename... PTs> class PointerUnion`, llvm/ADT/PointerUnion.h:104)
// but BOTH use sites spell exactly three concrete class template arguments, so a
// fixed-arity restatement keys them.  This is not a parameter-pack case -- a
// pack marker covers FUNCTION parameter packs, and these are class template
// arguments.  A 2- or 4-alternative PointerUnion would need its own t-rule
// against Variant2/Variant4; none appears in dt_src today.
//
// Declaration restated locally, for the reason rules/sourcemgr, rules/twine,
// rules/stringref and rules/statistic all give: cpp-rule-preprocessor compiles
// this file with a fixed flag set and reaching llvm/ADT/PointerUnion.h would
// need an absolute -I into a built LLVM tree.

namespace llvm {

template <typename T1, typename T2, typename T3> class PointerUnion {
public:
  PointerUnion();
};

} // namespace llvm

template <typename T1, typename T2, typename T3>
using t1 = llvm::PointerUnion<T1, T2, T3>;

// A default-constructed PointerUnion holds the FIRST alternative, null.
// Variant3::default() is `V1(T1::default())` (libcc2rs/src/variant.rs:149), so
// the discriminant it starts in matches -- not an arbitrary choice.
template <typename T1, typename T2, typename T3>
llvm::PointerUnion<T1, T2, T3> f1() {
  return llvm::PointerUnion<T1, T2, T3>();
}
