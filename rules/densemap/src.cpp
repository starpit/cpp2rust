// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::DenseMap<K, V> / llvm::DenseSet<K> -- LLVM's open-addressing hash
// containers.
//
// MODEL
//   llvm::DenseMap<K, V> -> HashMap<K, V>
//   llvm::DenseSet<K>    -> HashSet<K>
//
// WHY THE DECLARATIONS ARE RESTATED AND NOT #included: same reason as
// rules/smallvector, rules/support, rules/stringref and rules/twine --
// cpp-rule-preprocessor compiles this file with a fixed flag set and the only
// flags that would reach LLVM's headers are absolute -I paths into whatever
// LLVM tree the target project happens to have built.
//
// ARITY -- MEASURED, and it is NOT what the diagnostic prints.  The converter's
// unsupported-type diagnostic renders the FULL four-argument spelling:
//   `llvm::DenseMap<mlir::Attribute, unsigned long,
//                   llvm::DenseMapInfo<mlir::Attribute, void>,
//                   llvm::detail::DenseMapPair<mlir::Attribute, unsigned long>>`
// but that is a DIFFERENT printer from the one `search()` uses.  With -verbose
// the actual lookup is:
//   search type llvm::DenseMap<mlir::Attribute, unsigned long>, result: None
//   search type llvm::DenseSet<mlir::Operation *>, result: None
// i.e. SuppressDefaultTemplateArgs elides the defaulted KeyInfoT/BucketT, so the
// key has TWO parameters for DenseMap and ONE for DenseSet.  A four-argument key
// is a DEAD rule: it matches nothing.  (An earlier revision of this module was
// written to the diagnostic's spelling and was measured to move neither TU.)
//
// A pleasant consequence: because the traits and bucket parameters are NOT part
// of the key, a GENERIC rule here does not force the converter to find rules for
// `llvm::DenseMapInfo<K, void>` or `llvm::detail::DenseMapPair<K, V>` -- the
// generic-rule regression that bites when a defaulted argument survives into the
// key does not apply.  Measured: the generic two-arg form matches the concrete
// `<mlir::Attribute, unsigned long>` instantiation.
//
// DenseMapInfo IS A TRAITS CLASS AND IS DELIBERATELY NOT MODELLED.  Its
// getEmptyKey / getTombstoneKey / getHashValue are implementation details of the
// open-addressing table (the sentinel keys that mark empty and deleted
// buckets).  HashMap has no analogue -- it brings its own hasher and has no
// caller-visible sentinels -- so inventing values for them would be silently
// wrong.  Mapping DenseMap to HashMap is what makes them unreachable, and that
// was CHECKED: neither TU searches for any DenseMapInfo member after this
// module loads.
//
// NOT COVERED, deliberately: the operation surface (find/lookup/count/
// try_emplace/erase/begin/end and the iterator's ==/!=/++/deref).  Those
// methods live on the CRTP base `llvm::DenseMapBase<DerivedT, K, V, KeyInfoT,
// BucketT>`, whose key therefore carries FIVE template arguments, the first of
// which is the derived DenseMap itself.  Adding them requires harvesting each
// overload's exact base-class key with `-verbose`, which the two measured rows
// do not yet reach, and a guessed key records a DEAD rule.  The two measured
// first-abort rows are TYPE failures ("unsupported system type has no rule"),
// so the type rules are what those rows need; the operations are the next
// blocker.

namespace llvm {

template <typename KeyT, typename ValueT = void> struct DenseMapInfo;

namespace detail {
template <typename KeyT, typename ValueT> struct DenseMapPair;
} // namespace detail

template <typename KeyT, typename ValueT,
          typename KeyInfoT = DenseMapInfo<KeyT, void>,
          typename BucketT = detail::DenseMapPair<KeyT, ValueT>>
class DenseMap {
public:
  DenseMap();
};

template <typename ValueT, typename ValueInfoT = DenseMapInfo<ValueT, void>>
class DenseSet {
public:
  DenseSet();
};


// The CRTP base that carries the OPERATION surface (find/lookup/count/
// try_emplace/erase/begin/end).  Its key has FIVE arguments -- the derived
// DenseMap FIRST -- and, unlike DenseMap's, NONE of them are defaulted here, so
// SuppressDefaultTemplateArgs elides nothing and all five survive into the key.
// Measured spelling (survey-v3, 236 occurrences for the largest instantiation):
//   llvm::DenseMapBase<llvm::DenseMap<mlir::StringAttr, mlir::ktdf_arch::Device>,
//                      mlir::StringAttr, mlir::ktdf_arch::Device,
//                      llvm::DenseMapInfo<mlir::StringAttr>,
//                      llvm::detail::DenseMapPair<mlir::StringAttr, mlir::ktdf_arch::Device>>
// Note the FIRST argument is printed in DenseMap's SUGARED two-argument form.
template <typename DerivedT, typename KeyT, typename ValueT, typename KeyInfoT,
          typename BucketT>
class DenseMapBase {
public:
  DenseMapBase();
};


// ---------------------------------------------------------------------------
// THE ITERATOR.  ARITY MEASURED, NOT GUESSED -- and there are TWO spellings.
//
// `llvm::DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, bool IsConst = false>`.
// SuppressDefaultTemplateArgs elides a TRAILING run of default-valued arguments,
// so:
//   * a MUTABLE iterator (IsConst = false, the default) has all three trailing
//     arguments at their defaults and prints as the TWO-argument sugared form
//         llvm::DenseMapIterator<SentientRegType, GraphColoring>
//   * a CONST iterator (IsConst = true) differs from its default in the LAST
//     position, so nothing can be elided and all FIVE arguments survive
//         llvm::DenseMapIterator<mlir::Operation *, scheduler::PipelineTreeNode *,
//                                llvm::DenseMapInfo<mlir::Operation *>,
//                                llvm::detail::DenseMapPair<mlir::Operation *,
//                                                           scheduler::PipelineTreeNode *>,
//                                true>
//     -- note KeyInfoT prints with ONE argument (its own `Enable = void` is
//     elided) while DenseMap's diagnostic spelling shows `<K, void>`; the two
//     printers disagree and the ONE-argument form is the one the key uses.
// Measured over the 49 queue rows: 31 rows are the short form, 18 the long one,
// and ALL 18 long rows carry `true` -- there is no fourth-argument-only variant.
// So the long key can be written with the non-type argument FIXED to `true` and
// only T1/T2 generic: the bucket and traits arguments are functions of T1/T2.
//
// Both forms are keyed.  A single generic key does NOT collapse them, because the
// key is a STRING and the two strings differ.
template <typename KeyT, typename ValueT,
          typename KeyInfoT = DenseMapInfo<KeyT>,
          typename BucketT = detail::DenseMapPair<KeyT, ValueT>,
          bool IsConst = false>
class DenseMapIterator {
public:
  DenseMapIterator();
};

// Free comparison operators, found by ADL in namespace llvm -- which is why the
// measured key is spelled `bool llvm::operator==(...)` and not a member.
template <typename KeyT, typename ValueT, typename KeyInfoT, typename BucketT,
          bool IsConst>
bool operator==(
    const DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, IsConst> &lhs,
    const DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, IsConst> &rhs);

template <typename KeyT, typename ValueT, typename KeyInfoT, typename BucketT,
          bool IsConst>
bool operator!=(
    const DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, IsConst> &lhs,
    const DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, IsConst> &rhs);

} // namespace llvm

template <typename T1, typename T2> using t1 = llvm::DenseMap<T1, T2>;

template <typename T1> using t2 = llvm::DenseSet<T1>;

template <typename T1, typename T2, typename T3, typename T4, typename T5>
using t3 = llvm::DenseMapBase<T1, T2, T3, T4, T5>;

// The MUTABLE iterator: two arguments.
template <typename T1, typename T2>
using t4 = llvm::DenseMapIterator<T1, T2>;

// The CONST iterator: five arguments, the last fixed to `true`.
template <typename T1, typename T2>
using t5 = llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                                  llvm::detail::DenseMapPair<T1, T2>, true>;

// ==/!= on the MUTABLE iterator.  Written in CALL form: an INFIX operator in a
// rule body records nothing at all, silently.
template <typename T1, typename T2>
bool f1(const llvm::DenseMapIterator<T1, T2> &a0,
        const llvm::DenseMapIterator<T1, T2> &a1) {
  return operator==(a0, a1);
}

template <typename T1, typename T2>
bool f2(const llvm::DenseMapIterator<T1, T2> &a0,
        const llvm::DenseMapIterator<T1, T2> &a1) {
  return operator!=(a0, a1);
}

// ==/!= on the CONST iterator.
template <typename T1, typename T2>
bool f3(const llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                                     llvm::detail::DenseMapPair<T1, T2>, true> &a0,
        const llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                                     llvm::detail::DenseMapPair<T1, T2>, true> &a1) {
  return operator==(a0, a1);
}

template <typename T1, typename T2>
bool f4(const llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                                     llvm::detail::DenseMapPair<T1, T2>, true> &a0,
        const llvm::DenseMapIterator<T1, T2, llvm::DenseMapInfo<T1>,
                                     llvm::detail::DenseMapPair<T1, T2>, true> &a1) {
  return operator!=(a0, a1);
}
