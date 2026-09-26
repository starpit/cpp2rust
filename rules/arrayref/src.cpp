// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::ArrayRef<T> -- a NON-OWNING (pointer, length) view.  No rule existed for
// it ANYWHERE: `grep -rn 'using t[0-9]* = .*ArrayRef' --include=src.cpp rules/`
// returns nothing, and it appears only inside rules/apint and rules/smallvector
// SIGNATURES.  rules/smallvector/src.cpp:88-89 says so explicitly and declines
// SmallVector(ArrayRef<T>) for exactly that reason.  In the emitted InitBin,
// `llvm_ArrayRef_long_` is referenced with no `pub struct` defining it, so the
// argument is an opaque name and no body can read it.
//
// SURFACE IN dt_src: 339 mentions.  By instantiation, ArrayRef<int64_t> 58,
// ArrayRef<Value>/ArrayRef<mlir::Value> 62, ArrayRef<int32_t> 15,
// ArrayRef<bool> 12, then a long tail of project and mlir element types.
// The CONSTRUCTIONS that appear are, in order: from a container
// (`ArrayRef<Value>(stride_map_operands)` -- PCFGToDataflowIR.cpp:2817, 3062,
// 3380, 3748, 4526, SNTransferLowering.cpp:1057, PCFG2ToDataflowIR.cpp:130, 132,
// 665 ...), and from an initializer_list / single value, which the emitted
// InitBin shows directly at dbo__src__InitBin.cpp.rs:5134,5142 as
// `llvm_ArrayRef_long_::new_1({ vec![(*size.borrow())] })` and
// `::new_2({ ids.as_pointer() })`.
//
// THE KEYS ARE NOT GUESSED.  `cpp2rust -verbose` (mapper.cpp:759) on the probe
// prints exactly what the converter asks for:
//   search type llvm::ArrayRef<long>
//   search expr void llvm::ArrayRef<long>::ArrayRef()
//   search expr void llvm::ArrayRef<long>::ArrayRef(const long &)
//   search expr void llvm::ArrayRef<long>::ArrayRef(const long *, unsigned long)
//   search expr void llvm::ArrayRef<long>::ArrayRef(const std::vector<long> &)
//   search expr void llvm::ArrayRef<long>::ArrayRef(std::initializer_list<long>)
//   search expr unsigned long llvm::ArrayRef<long>::size() const
//   search expr bool llvm::ArrayRef<long>::empty() const
//   search expr const long & llvm::ArrayRef<long>::operator[](unsigned long) const
//
// MODEL: `(*mut T1, i64)` -- the pointer and the length, which is literally what
// ArrayRef stores.  This is rules/mlir's `(base, i64)` range convention (t5/t6,
// tgt_unsafe.rs:120,150) reused rather than a third shape invented, and it is a
// BORROW: reading element i dereferences the ORIGINAL buffer, so a mutation of
// the underlying container after the view is taken IS visible, as C++ requires.
// A `Vec<T1>` model would copy and hide exactly that.
//
// A ONE-ELEMENT ArrayRef HAS LENGTH 1, NOT 0 AND NOT THE CONTAINER'S LENGTH.
// f2 takes the address of its single argument and pairs it with 1.  This is the
// classic silent-wrongness trap for this type and the probe prints `single
// size=1 elem=7` to pin it.
//
// std::initializer_list IS DELIBERATELY LEFT LOUD, and this is a representation
// conflict rather than an omission: the port models an initializer_list as an
// OWNED Vec (the emitted `new_1({ vec![...] })` above), while C++'s ArrayRef
// over an initializer_list is a view over a TEMPORARY whose lifetime ends at the
// full expression.  Borrowing the port's owned Vec would be a dangling
// reference, and copying it would break the borrow semantics f4 depends on.
// Deciding it needs a lifetime story the port has not taken, so it stays loud.
//
// Declarations restated locally, for the reason rules/statistic, rules/twine,
// rules/stringref and rules/sourcemgr all give: cpp-rule-preprocessor compiles
// this file with a fixed flag set and reaching llvm/ADT/ArrayRef.h would need an
// absolute -I into a built LLVM tree.

#include <initializer_list>
#include <vector>

namespace llvm {

template <typename T> class ArrayRef {
public:
  ArrayRef();
  ArrayRef(const T &One);
  ArrayRef(const T *data, unsigned long length);
  ArrayRef(const std::vector<T> &Vec);
  ArrayRef(std::initializer_list<T> IL);
  unsigned long size() const;
  bool empty() const;
  const T &operator[](unsigned long Index) const;
};

} // namespace llvm

template <typename T1> using t1 = llvm::ArrayRef<T1>;

template <typename T1> llvm::ArrayRef<T1> f1() { return llvm::ArrayRef<T1>(); }

// ONE element -> length 1.
template <typename T1> llvm::ArrayRef<T1> f2(const T1 &a0) {
  return llvm::ArrayRef<T1>(a0);
}

// The type's own shape, (pointer, length).
template <typename T1>
llvm::ArrayRef<T1> f3(const T1 *a0, unsigned long a1) {
  return llvm::ArrayRef<T1>(a0, a1);
}

// From a container -- the commonest construction in dt_src by a wide margin.
template <typename T1>
llvm::ArrayRef<T1> f4(const std::vector<T1> &a0) {
  return llvm::ArrayRef<T1>(a0);
}

template <typename T1> unsigned long f5(const llvm::ArrayRef<T1> &a0) {
  return a0.size();
}

template <typename T1> bool f6(const llvm::ArrayRef<T1> &a0) {
  return a0.empty();
}

template <typename T1>
const T1 &f7(const llvm::ArrayRef<T1> &a0, unsigned long a1) {
  return a0.operator[](a1);
}
