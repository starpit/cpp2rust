// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::SmallVector<T, N> -- a std::vector with N elements of inline storage.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/ADT/SmallVector.h>
// ---------------------------------------------------------------------------
// Same reason as rules/stringref, rules/raw_ostream and rules/twine:
// cpp-rule-preprocessor compiles this file with a fixed flag set and the only
// flags that would reach LLVM's headers are absolute -I paths into whatever
// LLVM tree the target project happens to have built.  So the signatures LLVM
// declares are restated here.
//
// N IS AN OPTIMISATION, NOT SEMANTICS.  The mapper elides non-type template
// arguments as `_` (exactly as rules/array relies on: `std::array<T1, _>` is
// one key covering every extent), so ONE key `llvm::SmallVector<T1, _>`
// covers every inline-capacity instantiation.
//
// THE METHODS LIVE ON THE CRTP BASES, NOT ON SmallVector.  The hierarchy is
//   SmallVector -> SmallVectorImpl -> SmallVectorTemplateBase
//                                  -> SmallVectorTemplateCommon
// and a call to `v.push_back(x)` keys on
//   void llvm::SmallVectorTemplateBase<mlir::OpFoldResult>::push_back(mlir::OpFoldResult)
// read back out of a real translation with `cpp2rust -verbose` (grep -A1
// 'search expr') -- so each base needs its own TYPE rule as well, or the
// mangled-name fallback just moves from the derived type to the base.
//
// Note the printers DISAGREE, as they do for DenseMapInfo: `-verbose`
// suppresses the defaulted second template argument (`SmallVectorTemplateBase
// <mlir::OpFoldResult>`) while the mangled fallback name keeps it
// (`SmallVectorTemplateBase<mlir::OpFoldResult, true>`).  The type rules below
// are written with the parameter so the mapper records `<T1, _>`, which is what
// matches the concrete `true`.
//
// push_back's parameter is BY VALUE here because that is what was measured:
// LLVM passes `ValueParamT`, which is `T` for small trivially-copyable T and
// `const T &` otherwise.  All three element types this corpus pushes
// (mlir::OpFoldResult, mlir::ktdf::StageOp, mlir::Operation *) take the
// by-value form.  A large element type would key on `const T1 &` and is
// deliberately NOT added here -- unmeasured.

#include <cstddef>
#include <cstdint>

namespace llvm {

// The SIZE/CAPACITY base of the whole hierarchy, templated on the SIZE TYPE
// (uint32_t normally, uint64_t for vectors whose inline capacity cannot be
// described in 32 bits) -- NOT on the element type.  `size()`, `capacity()`
// and `empty()` really live here, which is why a real translation searches
//   search expr unsigned long llvm::SmallVectorBase<unsigned int>::size() const
// and NOT the SmallVectorTemplateCommon form f5 provides.  Harvested from
// LoopTiling.cpp / TileSCFForLoops.cpp / StripMineSCFForLoops.cpp.
//
// NOTE THE TWO PRINTERS DISAGREE, a fifth instance of a trap measured on four
// other axes today: the TYPE search spells the argument SUGARED
// (`llvm::SmallVectorBase<uint32_t>`) while the EXPR search and the
// mangled-name fallback spell it CANONICALISED (`<unsigned int>`).  A key
// written as the diagnostic's fallback name asks would be DEAD; a GENERIC
// `llvm::SmallVectorBase<T1>` covers both spellings and both size types.
//
// It is declared as a STANDALONE class, not as a base of
// SmallVectorTemplateCommon, deliberately: LLVM's real base is
// `SmallVectorBase<SmallVectorSizeType<T>>`, a computed type, and restating
// that machinery here would only risk recording the wrong receiver for f5/f6.
template <typename Size_T> class SmallVectorBase {
public:
  std::size_t size() const;
  std::size_t capacity() const;
  bool empty() const;
};

template <typename T, typename = void> class SmallVectorTemplateCommon {
public:
  using size_type = std::size_t;
  using iterator = T *;
  using const_iterator = const T *;
  size_type size() const;
  bool empty() const;
  iterator begin();
  iterator end();
  T *data();
  T &operator[](size_type idx);
  T &front();
  T &back();
};

template <typename T, bool TriviallyCopyable = true>
class SmallVectorTemplateBase : public SmallVectorTemplateCommon<T> {
public:
  void push_back(T elt);
  void pop_back();
};

template <typename T> class SmallVectorImpl : public SmallVectorTemplateBase<T> {
public:
  void clear();
  void resize(std::size_t n);
  void reserve(std::size_t n);
};

template <typename T, unsigned N = 4>
class SmallVector : public SmallVectorImpl<T> {
public:
  SmallVector();
  SmallVector(SmallVector &&other);
  SmallVector(const T *first, const T *last);
};

} // namespace llvm

// ---------------------------------------------------------------- type rules

template <typename T1, unsigned T2> using t1 = llvm::SmallVector<T1, T2>;
template <typename T1> using t2 = llvm::SmallVectorImpl<T1>;
template <typename T1, bool T2>
using t3 = llvm::SmallVectorTemplateBase<T1, T2>;
template <typename T1, typename T2>
using t4 = llvm::SmallVectorTemplateCommon<T1, T2>;

// ONE-ARGUMENT spelling.  SmallVector's second template argument has a COMPUTED
// default (`CalculateSmallVectorDefaultInlinedElements<T>::value`), so a use site
// written `SmallVector<long>` is printed by the mapper with the defaulted argument
// ELIDED ENTIRELY -- `search type llvm::SmallVector<long>, result: None` --
// which the two-argument key `llvm::SmallVector<T1, _>` above cannot match even
// though the DIAGNOSTIC's fallback name prints `<long, _>`.  Harvested verbatim
// from AffineMinCanonicalization.cpp and StageCoarsening/Materializer.cpp.
// std::array does not need this because its extent has NO default.
template <typename T1> using t5 = llvm::SmallVector<T1>;

// The size/capacity base.  Its argument is the SIZE type, so there is no element
// type available in this key; the family's model is `Vec<T1>` and the target is
// written to match, which is sound only because every rule whose receiver is this
// base is INLINED and therefore never emits the type in a declaration position.
template <typename T1> using t6 = llvm::SmallVectorBase<T1>;

// MEASURED, AND THE REMAINING BLOCKER ON THIS ROW: the generic t6 above MATCHES,
// but mapping it then requires a model for its ARGUMENT, and the argument arrives
// SUGARED:
//   unsupported unmapped type `uint32_t` has no model in types_,
//     while mapping `llvm::SmallVectorBase<uint32_t>`
// A concrete key cannot rescue this: `using t7 = llvm::SmallVectorBase<uint32_t>;`
// was written, generated, and read back out of ir_src.json as
// `llvm::SmallVectorBase<unsigned int>` -- the rule preprocessor CANONICALISES a
// concrete template argument, while the converter's type search spells it SUGARED,
// so the two can never meet and the key is DEAD.  It was therefore REMOVED rather
// than left in place looking like coverage.  The fix belongs in the converter's
// types_ (a builtin typedef such as uint32_t should resolve to its canonical
// builtin model), NOT here; no smallvector key can express it.

// MEASURED: the DEFAULTED `void` second argument of SmallVectorTemplateCommon is
// spelled EXPLICITLY at the search, `llvm::SmallVectorTemplateCommon<
// scheduler::StageNode *, void>`, which the generic `<T1, T2>` key of t4 does not
// satisfy (T2 would have to bind to `void`).  Same disagreement as DenseMapInfo,
// in the opposite direction, so the `void` is spelled concretely here.
template <typename T1> using t9 = llvm::SmallVectorTemplateCommon<T1, void>;

// ------------------------------------------------------------ function rules

template <typename T1>
void f1(llvm::SmallVectorTemplateBase<T1> &o, T1 elt) {
  return o.push_back(elt);
}

template <typename T1> llvm::SmallVector<T1> f2() {
  return llvm::SmallVector<T1>();
}

template <typename T1> llvm::SmallVector<T1> f3(llvm::SmallVector<T1> &&o) {
  return llvm::SmallVector<T1>(static_cast<llvm::SmallVector<T1> &&>(o));
}

template <typename T1>
llvm::SmallVector<T1> f4(const T1 *first, const T1 *last) {
  return llvm::SmallVector<T1>(first, last);
}

template <typename T1>
std::size_t f5(const llvm::SmallVectorTemplateCommon<T1> &o) {
  return o.size();
}

template <typename T1>
bool f6(const llvm::SmallVectorTemplateCommon<T1> &o) {
  return o.empty();
}

template <typename T1>
T1 &f7(llvm::SmallVectorTemplateCommon<T1> &o, std::size_t idx) {
  return o.operator[](idx);
}

template <typename T1> T1 *f8(llvm::SmallVectorTemplateCommon<T1> &o) {
  return o.begin();
}

template <typename T1> T1 *f9(llvm::SmallVectorTemplateCommon<T1> &o) {
  return o.end();
}

template <typename T1> T1 *f10(llvm::SmallVectorTemplateCommon<T1> &o) {
  return o.data();
}

template <typename T1> T1 &f11(llvm::SmallVectorTemplateCommon<T1> &o) {
  return o.front();
}

template <typename T1> T1 &f12(llvm::SmallVectorTemplateCommon<T1> &o) {
  return o.back();
}

template <typename T1> void f13(llvm::SmallVectorImpl<T1> &o) {
  return o.clear();
}

template <typename T1> void f14(llvm::SmallVectorImpl<T1> &o, std::size_t n) {
  return o.resize(n);
}

template <typename T1> void f15(llvm::SmallVectorImpl<T1> &o, std::size_t n) {
  return o.reserve(n);
}

template <typename T1> void f16(llvm::SmallVectorTemplateBase<T1> &o) {
  return o.pop_back();
}

// size()/capacity()/empty() on the SIZE base.  These do NOT duplicate f5/f6:
// those key on `llvm::SmallVectorTemplateCommon<T1>` and a real translation was
// measured searching the `llvm::SmallVectorBase<...>` form and getting None.
// f5/f6 are kept because the mapper may reach either receiver depending on how
// the call is spelled; the keys are distinct so neither shadows the other.
template <typename T1> std::size_t f17(const llvm::SmallVectorBase<T1> &o) {
  return o.size();
}

template <typename T1> std::size_t f18(const llvm::SmallVectorBase<T1> &o) {
  return o.capacity();
}

template <typename T1> bool f19(const llvm::SmallVectorBase<T1> &o) {
  return o.empty();
}
