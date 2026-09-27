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

namespace llvm {

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
