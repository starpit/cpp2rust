// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::SmallVector -- LLVM's vector with inline storage, and the container
// dt_src reaches for everywhere MLIR is involved.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/ADT/SmallVector.h>
// ---------------------------------------------------------------------------
// The reason rules/ilist, rules/densemap, rules/stringref, rules/raw_ostream,
// rules/twine and rules/statistic all give at length: cpp-rule-preprocessor
// compiles this file with a fixed flag set, and reaching LLVM's headers would
// need an absolute -I into whatever LLVM tree the target project happens to
// have built, which would make `ninja` in this repo fail for anyone without
// that tree.  So the signatures LLVM declares are restated here.
//
// A rule matches on a SIGNATURE STRING, so a restatement is only worth
// anything if it prints the SAME string.  Every signature below was read out of
// a real translation of a real dt_src TU -- `cpp2rust --verbose` prints
// `search expr <exact resolved signature>` -- over the eight TUs that carry an
// `UnmappedType llvm::SmallVector<...>` record, and then checked to print the
// same string from this file.  The three that are NOT guessable from the header
// are recorded where they occur:
//
//   * size()/empty() are declared on `SmallVectorBase<Size_T>`, whose parameter
//     is the SIZE TYPE and not the element type, so the converter searches for
//     `unsigned long llvm::SmallVectorBase<unsigned int>::size() const` -- one
//     signature shared by EVERY instantiation, with no element type in it at
//     all.  f1/f2 are therefore element-type-independent by construction, and
//     their target bodies must not mention T1 (they do not: `len`/`is_empty`).
//   * begin/end/front/back/operator[] are on `SmallVectorTemplateCommon<T>`,
//     push_back/pop_back on `SmallVectorTemplateBase<T>`, clear/append/
//     emplace_back/erase on `SmallVectorImpl<T>`, and only the constructors and
//     assignment operators on `SmallVector<T, N>` itself.  A rule keyed on
//     `SmallVector<T1>` for any of the first group matches nothing.
//   * `SmallVector<T>` and `SmallVector<T, N>` are two DIFFERENT keys, because
//     clang suppresses the inline-capacity argument only when the source did
//     not write it.  dt_src writes both (`SmallVector<Operation *>` and
//     `SmallVector<ProgramUnitOp, 4>`), and 35 of the 214 distinct measured
//     signatures are the `, _` spelling, so every constructor and assignment
//     rule is given twice.  Same reason rules/vector carries t1 and t5.
//
// THE REPRESENTATION: Vec<T>, and the inline capacity is DROPPED
// ---------------------------------------------------------------------------
// SmallVector<T, N> is a std::vector whose first N elements live in the object
// instead of on the heap.  That is an allocation strategy, not observable
// behaviour: the element sequence, the indices, the iteration order and the
// values are exactly a vector's, so Vec<T> is the whole of the observable
// semantics and N is dropped -- the same decision rules/array records for
// std::array<T, N>, whose non-type argument is likewise absent from the target.
//
// Two things WOULD have made N observable, and both were checked against the
// eight TUs rather than assumed:
//
//   * capacity().  LLVM returns the CURRENT capacity, which is N for a vector
//     that has not grown, while Vec::capacity() is whatever Rust's growth
//     policy picked -- the two disagree numerically, and there is no way to
//     make them agree without modelling the inline buffer.  So capacity() gets
//     NO RULE and stays a loud undefined name.  Measured: no site in the eight
//     TUs calls it, so nothing is lost by refusing it, and the refusal is what
//     keeps a wrong number from being printed.
//   * pointer stability across growth.  C++ invalidates `data()`/iterators on
//     any reallocation and so does Vec, so this is not a difference -- but a
//     site that relied on the first N elements NEVER moving would be relying on
//     the inline buffer.  Checked: the sites take a pointer, use it, and drop
//     it within the expression; none holds one across a push_back.
//
// WHAT IS DELIBERATELY LEFT OUT, so that it stays loud rather than guessed
// ---------------------------------------------------------------------------
//   * capacity()/reserve()/resize()/set_size(): capacity for the reason above,
//     the others because no measured site uses them. A rule nobody exercises is
//     a rule nobody has verified.
//   * push_back for a NON-trivially-copyable element type.  LLVM splits
//     SmallVectorTemplateBase on `is_trivially_copy_constructible &&
//     is_trivially_move_constructible && is_trivially_destructible`: the true
//     branch declares `push_back(T)` BY VALUE, the false branch
//     `push_back(const T &)` plus `push_back(T &&)`.  Those are three different
//     signature strings from one name, and a restatement cannot declare both
//     `push_back(T)` and `push_back(const T &)` in one class -- every call
//     would be ambiguous and this file would not compile.  The by-value branch
//     is the one dt_src needs: every element type in the measured set is a
//     trivially copyable handle (int, long, StringRef, Value, Attribute,
//     Operation *, ProgramUnitOp, CopyOp, ForOp, and raw pointers).  The one
//     non-trivial element type measured is `SmallVector<std::string>` in
//     dcc/tools/LitAutoTestGen, whose `push_back(const std::string &)` keeps no
//     rule and stays loud; its UnmappedType gap is closed by the type rules
//     regardless.  Splitting that branch out is a second module, the way
//     rules/vector_cxx11 is a second module for a second spelling.
//   * SmallVector(ArrayRef<T>) and ArrayRef<T>(const SmallVector<T> &):
//     llvm::ArrayRef has no representation in the port yet, so a rule here
//     would have to invent one.
//   * Anything on the far side of --opaque-namespace=mlir (llvm::all_of,
//     llvm::zip over a SmallVector): those are separate facilities with their
//     own semantics, not SmallVector operations.
// ---------------------------------------------------------------------------

#include <cstddef>
#include <initializer_list>
#include <utility>
#include <vector>

template <typename T, typename A> using Init = A;

namespace llvm {

// llvm/ADT/SmallVector.h -- the size/capacity half, templated on the SIZE type.
// `SmallVectorSizeType<T>` is uint32_t for every element type in the measured
// set, which is why the converter asks for `SmallVectorBase<unsigned int>`.
template <class Size_T> class SmallVectorBase {
protected:
  void *BeginX = nullptr;
  Size_T Size = 0, Capacity = 0;

public:
  std::size_t size() const;
  std::size_t capacity() const;
  bool empty() const;
};

// The element-typed half: everything that hands out an element or an iterator.
// Second parameter defaulted and never written, exactly as LLVM has it, so the
// printed name is `llvm::SmallVectorTemplateCommon<T>`.
template <typename T, typename = void>
class SmallVectorTemplateCommon : public SmallVectorBase<unsigned int> {
public:
  T *begin();
  T *end();
  const T *begin() const;
  const T *end() const;
  T *data();
  const T *data() const;
  T &operator[](std::size_t idx);
  const T &operator[](std::size_t idx) const;
  T &front();
  const T &front() const;
  T &back();
  const T &back() const;
};

// LLVM's trivially-copyable branch, whose push_back takes T BY VALUE. The
// default is spelled `true` so that clang suppresses it and the printed name is
// `llvm::SmallVectorTemplateBase<T>`, which is what the converter searches for.
// src.cpp's header comment records why the non-trivial branch is not here.
template <typename T, bool TakesParamByValue = true>
class SmallVectorTemplateBase : public SmallVectorTemplateCommon<T> {
public:
  void push_back(T Elt);
  void pop_back();
};

template <typename T> class SmallVectorImpl : public SmallVectorTemplateBase<T> {
public:
  void clear();
  void append(const SmallVectorImpl<T> &RHS);
  template <typename... ArgTypes> T &emplace_back(ArgTypes &&...Args);
  T *erase(const T *CS, const T *CE);
};

template <typename T, unsigned N = 0> class SmallVector : public SmallVectorImpl<T> {
public:
  SmallVector();
  SmallVector(const SmallVector &RHS);
  SmallVector(SmallVector &&RHS);
  SmallVector(std::initializer_list<T> IL);
  SmallVector &operator=(const SmallVector &RHS);
  SmallVector &operator=(SmallVector &&RHS);
  SmallVector &operator=(std::initializer_list<T> IL);
};

} // namespace llvm

// ---------------------------------------------------------------------------
// types
// ---------------------------------------------------------------------------

template <typename T1> using t1 = llvm::SmallVector<T1>;
template <typename T1, unsigned T2> using t2 = llvm::SmallVector<T1, T2>;
template <typename T1> using t3 = llvm::SmallVectorImpl<T1>;

// The two INTERMEDIATE bases are mapped as well, and that is not padding: a
// method's declaring class is what the refcount model asks about when it decides
// whether a receiver has to be DECAYED from a pointer-to-container into a
// pointer-to-element (IsBoxedType, converter_refcount.cpp:49, tests whether the
// MAPPED spelling starts with `Vec<`). back()/front()/begin()/end() are declared
// on SmallVectorTemplateCommon, so without a rule for that class the receiver was
// not recognised as a container: measured, `SmallVectorImpl<int> &v; v.back()`
// emitted `(v).clone() as Ptr<i32>` -- E0605, non-primitive cast -- where
// std::vector's identical shape emits `v.decay() as Ptr<i32>` and runs. With the
// rule the two agree. The same class is also SPELLED in the output when a rule is
// missing, which is the other way this showed up (E0425, cannot find type
// `llvm_SmallVectorTemplateCommon_int__void_`).
template <typename T1> using t4 = llvm::SmallVectorTemplateCommon<T1>;
template <typename T1> using t5 = llvm::SmallVectorTemplateBase<T1>;

// A SmallVector nested inside a std::vector needs its OWN rule in the refcount
// model, which is why rules/vector carries t3 (`std::vector<std::vector<T1>>`)
// beside t1: an element that is itself a container has to be BOXED
// (`Vec<Value<Vec<T1>>>`), because a reference to one element must stay valid
// while the outer vector is mutated. Without this rule the generic t1 match gives
// `Vec<Vec<i64>>`, and a pointer taken to an element then has no Value to upgrade
// through -- measured on XRFRegisterAnalyzer's own shape,
// `std::vector<SmallVector<int64_t, 8>>`: E0599, no method `as_pointer` on
// `Ref<'_, Vec<i64>>`. The unsafe model needs no such distinction and gets the
// same `Vec<Vec<T1>>` either way.
//
// ONLY this combination, because it is the only nested one measured in the eight
// TUs. `SmallVector<DenseMap<..>>` (UniformGrouper) is the same question for a
// different inner container and is deliberately left alone rather than guessed
// at: it stays loud in the refcount model.
template <typename T1, unsigned T2>
using t6 = std::vector<llvm::SmallVector<T1, T2>>;

// ---------------------------------------------------------------------------
// size and emptiness -- one rule each for EVERY instantiation, because the
// declaring class carries the size type and not the element type.
// ---------------------------------------------------------------------------

template <typename T1> std::size_t f1(const llvm::SmallVector<T1> &o) {
  return o.size();
}

template <typename T1> bool f2(const llvm::SmallVector<T1> &o) {
  return o.empty();
}

// ---------------------------------------------------------------------------
// element access. A reference-returning rule is spelled as a POINTER on the
// target side, as every other container module does.
// ---------------------------------------------------------------------------

template <typename T1> T1 &f3(llvm::SmallVector<T1> &o) { return o.front(); }

template <typename T1> T1 &f4(llvm::SmallVector<T1> &o) { return o.back(); }

// operator[]. rules/vector deliberately gives std::vector's none, because the
// converter's own subscript path handles it -- but that path is UNSAFE-MODEL
// ONLY: measured, the refcount model without this rule falls back to a generic
// call on the declaring base and emits `llvm_SmallVectorTemplateCommon_int__void_`,
// a type nothing declares (E0425). So the rule is here, and its target body
// spells the borrow EXPLICITLY: `&mut (a0)[i]` becomes `&mut (*v)[i]` when the
// receiver is a reference parameter, which rustc denies by default
// (`dangerous_implicit_autorefs`).
template <typename T1> T1 &f5(llvm::SmallVector<T1> &o, std::size_t idx) {
  return o.operator[](idx);
}

template <typename T1> T1 *f6(llvm::SmallVector<T1> &o) { return o.begin(); }

template <typename T1> T1 *f7(llvm::SmallVector<T1> &o) { return o.end(); }

template <typename T1> T1 *f8(llvm::SmallVector<T1> &o) { return o.data(); }

// ---------------------------------------------------------------------------
// mutation
// ---------------------------------------------------------------------------

template <typename T1> void f9(llvm::SmallVector<T1> &o, T1 value) {
  return o.push_back(value);
}

template <typename T1> void f10(llvm::SmallVector<T1> &o) {
  return o.pop_back();
}

template <typename T1> void f11(llvm::SmallVector<T1> &o) { return o.clear(); }

template <typename T1>
void f12(llvm::SmallVector<T1> &o, const llvm::SmallVectorImpl<T1> &rhs) {
  return o.append(rhs);
}

// f13 IS DELIBERATELY ABSENT -- emplace_back has no rule.
//
// The measured signature is `T1 & llvm::SmallVectorImpl<T1>::emplace_back(&&...)`
// (ScalarCopyInsertionForSymbols on `std::pair<Value, SentientRegType>`,
// UniformGrouper on a DenseMap element).  The `&&...` marker is printed by
// Mapper::ToString only when the resolved callee still carries a parameter PACK
// (mapper.cpp:1980), and written against this restatement the same rule shape
// rules/vector's f129 uses -- `Init<T1, Args> &&...args` forwarded into
// `emplace_back` -- records the key WITHOUT the marker:
// `T1 & llvm::SmallVectorImpl<T1>::emplace_back()`.  Measured both ways, with the
// receiver spelled `SmallVector<T1> &` and `SmallVectorImpl<T1> &`.
//
// That key is not merely useless, it is DANGEROUS: `v.emplace_back()` with no
// arguments is legal C++ and means DEFAULT-CONSTRUCT an element, and it is what
// prints `emplace_back()`.  Shipping this rule would make a zero-argument
// emplace_back match a body that pushes an argument it was never given.  So
// emplace_back stays an undefined name, loud at rustc, until the pack marker can
// be reproduced from a restatement.  It closes no gap: the UnmappedType records
// in both TUs are the TYPE, which t1/t2/t3 cover.

// Range erase. LLVM's iterator IS T1 *, so const_iterator is `const T1 *` and
// the signature the converter searches for on an element type that is itself a
// pointer reads `T ** ...::erase(T *const *, T *const *)`.
template <typename T1>
T1 *f14(llvm::SmallVector<T1> &o, const T1 *first, const T1 *last) {
  return o.erase(first, last);
}

// ---------------------------------------------------------------------------
// construction and assignment. Given twice -- once for `SmallVector<T>` and
// once for `SmallVector<T, N>` -- because those are two distinct keys.
// ---------------------------------------------------------------------------

template <typename T1> llvm::SmallVector<T1> f15() {
  return llvm::SmallVector<T1>();
}

template <typename T1, unsigned T2> llvm::SmallVector<T1, T2> f16() {
  return llvm::SmallVector<T1, T2>();
}

template <typename T1>
llvm::SmallVector<T1> f17(const llvm::SmallVector<T1> &o) {
  return llvm::SmallVector<T1>(o);
}

template <typename T1, unsigned T2>
llvm::SmallVector<T1, T2> f18(const llvm::SmallVector<T1, T2> &o) {
  return llvm::SmallVector<T1, T2>(o);
}

template <typename T1> llvm::SmallVector<T1> f19(llvm::SmallVector<T1> &&o) {
  return llvm::SmallVector<T1>(std::move(o));
}

template <typename T1, unsigned T2>
llvm::SmallVector<T1, T2> f20(llvm::SmallVector<T1, T2> &&o) {
  return llvm::SmallVector<T1, T2>(std::move(o));
}

template <typename T1>
llvm::SmallVector<T1> f21(const std::initializer_list<T1> &il) {
  return llvm::SmallVector<T1>(il);
}

template <typename T1, unsigned T2>
llvm::SmallVector<T1, T2> f22(const std::initializer_list<T1> &il) {
  return llvm::SmallVector<T1, T2>(il);
}

template <typename T1>
llvm::SmallVector<T1> &f23(llvm::SmallVector<T1> &dst,
                           llvm::SmallVector<T1> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1, unsigned T2>
llvm::SmallVector<T1, T2> &f24(llvm::SmallVector<T1, T2> &dst,
                               llvm::SmallVector<T1, T2> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1>
llvm::SmallVector<T1> &f25(llvm::SmallVector<T1> &dst,
                           const llvm::SmallVector<T1> &src) {
  return dst.operator=(src);
}

template <typename T1, unsigned T2>
llvm::SmallVector<T1, T2> &f26(llvm::SmallVector<T1, T2> &dst,
                               const llvm::SmallVector<T1, T2> &src) {
  return dst.operator=(src);
}

template <typename T1>
llvm::SmallVector<T1> &f27(llvm::SmallVector<T1> &dst,
                           const std::initializer_list<T1> &il) {
  return dst.operator=(il);
}

template <typename T1, unsigned T2>
llvm::SmallVector<T1, T2> &f28(llvm::SmallVector<T1, T2> &dst,
                               const std::initializer_list<T1> &il) {
  return dst.operator=(il);
}
