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

// Restated from llvm/ADT/ArrayRef.h, and ONLY as the parameter type of the free
// `operator!=` below (g797).  ⛔ NO `t` RULE IS ADDED FOR IT: `llvm::ArrayRef<T1>`
// already HAS one, rules/mlir t19 -> `Vec<T1>`, and a second type rule for the
// same spelling in a second module is exactly the kind of duplicate that makes
// which-module-wins depend on load order.  The class only has to be COMPLETE
// (f25 takes one by value); its layout is never read by a rule.
template <typename T> class ArrayRef {
  const T *Data = nullptr;
  unsigned long Length = 0;
};

template <typename T> class SmallVectorImpl : public SmallVectorTemplateBase<T> {
public:
  void clear();
  void resize(std::size_t n);
  void reserve(std::size_t n);
  bool operator==(const SmallVectorImpl &RHS) const;
  bool operator!=(const SmallVectorImpl &RHS) const;
};

template <typename T, unsigned N = 4>
class SmallVector : public SmallVectorImpl<T> {
public:
  SmallVector();
  SmallVector(SmallVector &&other);
  SmallVector(const T *first, const T *last);
};

// llvm::StringRef, RESTATED LOCALLY AND DELIBERATELY WITHOUT A TYPE RULE.
// `SmallString::operator+=(StringRef)` (g446) needs the argument type to be
// SPELLED for the key to come out as the queue recorded it, but the TYPE is
// owned by rules/stringref (`using t1 = llvm::StringRef;` there).  A second
// `using tN = llvm::StringRef;` here would record a DUPLICATE key with a
// different module's model; a BARE class declaration records nothing at all,
// which is exactly what is wanted.  Only the two fields are restated: no member
// of StringRef is called from this module.
class StringRef {
  const char *Data = nullptr;
  unsigned long Length = 0;

public:
  StringRef() = default;
  StringRef(const char *Str);
  StringRef(const char *data, unsigned long length);
};

// llvm/ADT/SmallString.h:24 --
//   template <unsigned InternalLen> class SmallString
//       : public SmallVector<char, InternalLen>
// so SmallString IS this hierarchy, and that is why g445/g446 are keyed HERE
// and not in rules/stringref: THE RECEIVER DECIDES THE MODULE, and forking the
// SmallVector model across two modules is the one outcome worth avoiding.
//
// CRITICALLY, THE MODEL IS A BARE `Vec<char>` WITH NO NUL TERMINATOR, unlike
// rules/string's std::string and rules/stringref's StringRef.  This is forced,
// not chosen: the same use site (dcc ResourceIds.cpp:82-92) calls
// `id.resize(prefix_len)`, which keys on `SmallVectorImpl<char>::resize` -> f14,
// a plain `Vec::resize_with`.  If SmallString carried a terminator, f14 would
// truncate one byte short of the C++ length on every call.  The whole
// hierarchy's model has to agree, and the hierarchy is SmallVector's.
//
// The consequence is paid in f22 instead: the StringRef ARGUMENT does carry a
// terminator (rules/stringref's model), so the append must drop its last byte.
//
// Declared as deriving from SmallVectorImpl<char> rather than
// SmallVector<char, InternalLen>: the intermediate SmallVector is irrelevant to
// these two keys, and naming it would make the measured `resize`/`size`
// receiver ambiguous for no gain.
template <unsigned InternalLen>
class SmallString : public SmallVectorImpl<char> {
public:
  // llvm/ADT/SmallString.h:27 -- SmallString() = default
  SmallString() = default;
  // llvm/ADT/SmallString.h:95 -- SmallString &operator+=(StringRef RHS)
  SmallString &operator+=(StringRef RHS);
  // llvm/ADT/SmallString.h:99 -- SmallString &operator+=(char C)
  SmallString &operator+=(char C);
};

// g797: the FREE `operator!=`, `bool llvm::operator!=(const
// llvm::SmallVectorImpl<mlir::Attribute> &, llvm::ArrayRef<mlir::Attribute>)`.
// It is declared HERE, inside `namespace llvm`, and not at global scope, because
// the key carries the `llvm::` qualifier -- unlike the shift family, whose keys
// print the WRITTEN nested-name-specifier and so come out unqualified.  LLVM
// declares it as `operator!=(const SmallVectorImpl<T> &, ArrayRef<T>)` because
// SmallVectorImpl converts to ArrayRef but the reverse comparison needs the
// mixed overload.
template <typename T> bool operator!=(const SmallVectorImpl<T> &LHS, ArrayRef<T> RHS);

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

// `llvm::SmallString<_>`.  The inline capacity is a NON-TYPE argument with no
// default, so the mapper elides it to `_` exactly as it does for t1's second
// argument, and this one key covers `SmallString<64>`, `<128>`, ... .
// A TYPE RULE WITHOUT A CONSTRUCTOR gives rc=0 and then E0433 at compile time
// (measured four times on 2026-09-27), so f21 below supplies the default ctor.
template <unsigned T1> using t10 = llvm::SmallString<T1>;

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

// MEMBER operator== on the Impl base.  Queue row g027-family sibling g223 (2 TUs)
// recorded the key as
//   bool llvm::SmallVectorImpl<mlir::Value>::operator==(
//       const llvm::SmallVectorImpl<mlir::Value> &) const
// i.e. a MEMBER, so it is written in member call form (an infix spelling records
// nothing, and a qualified free spelling aborts the preprocessor).  Element-wise
// equality, which is what LLVM's operator== does (size then std::equal).
template <typename T1>
bool f20(const llvm::SmallVectorImpl<T1> &a0,
         const llvm::SmallVectorImpl<T1> &a1) {
  return a0.operator==(a1);
}

// ---------------------------------------------------- llvm::SmallString<_>
// f21 is t10's DEFAULT CONSTRUCTOR, without which t10 is a type rule with no
// initializer and every use site gets rc=0 then E0433 on a nonexistent
// `<mangled>::new()`.
template <unsigned T1> llvm::SmallString<T1> f21() {
  return llvm::SmallString<T1>();
}

// g445: `llvm::SmallString<_> & llvm::SmallString<_>::operator+=(char)`.
// g446: `llvm::SmallString<_> & llvm::SmallString<_>::operator+=(llvm::StringRef)`.
// Both are MEMBER operators, so both are written in MEMBER CALL form: an infix
// spelling records NOTHING silently, a qualified free spelling aborts the
// preprocessor at cpp_rule_preprocessor.cpp:888, and a bare `o(a,b)` aborts
// at :83.  Both use sites (dcc ResourceIds.cpp:91 and :92) DISCARD the returned
// reference, and a declared reference return is not enforceable anyway because
// the body is inlined -- so the Rust bodies return nothing, the same shape
// rules/string's f38/f39 use for std::string's operator+=.
template <unsigned T1>
llvm::SmallString<T1> &f22(llvm::SmallString<T1> &a0, char a1) {
  return a0.operator+=(a1);
}

template <unsigned T1>
llvm::SmallString<T1> &f23(llvm::SmallString<T1> &a0, llvm::StringRef a1) {
  return a0.operator+=(a1);
}

// g796: the MEMBER `operator!=`, recorded as
//   bool llvm::SmallVectorImpl<long>::operator!=(
//       const llvm::SmallVectorImpl<long> &) const
// -- the exact sibling of f20's `operator==`, so it is written in the same
// MEMBER CALL form for the same reason (an infix `a0 != a1` records nothing).
// LLVM defines it as `!(*this == RHS)`, i.e. element-wise inequality; the site
// is DataTransferLowering.cpp:368 `src_time_dims.extents != dst_time_dims.extents`.
template <typename T1>
bool f24(const llvm::SmallVectorImpl<T1> &a0,
         const llvm::SmallVectorImpl<T1> &a1) {
  return a0.operator!=(a1);
}

// g797: the FREE `llvm::operator!=(const SmallVectorImpl<T1> &, ArrayRef<T1>)`,
// called UNQUALIFIED so ADL finds it -- a class-qualified free spelling aborts
// the preprocessor (see f22/f23 above).  The second operand's Rust model is NOT
// invented here: `llvm::ArrayRef<T1>` is rules/mlir t19 -> `Vec<T1>` in BOTH
// models, so the comparison is the same element-wise one f20/f24 use.  Site:
// Planner.cpp:1076 `full_path_no_ls != endpoint_path`.
template <typename T1>
bool f25(const llvm::SmallVectorImpl<T1> &a0, llvm::ArrayRef<T1> a1) {
  return operator!=(a0, a1);
}
