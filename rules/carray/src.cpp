// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstdlib>

// A 1-D array of KNOWN bound.  `GetTypeMapKey` (mapper.cpp:150-158) keys an
// array type by everything AFTER the first `[`, so this keys as `_]` -- and
// until this entry existed nothing in 90 modules answered `_]`, so any bounded
// 1-D array reached as a TEMPLATE ARGUMENT (e.g. the
// `std::map<PrimaryDimTypes, bool[MetaDimKind::Count]>` at
// dt_src/ddc/ddl/ddl_conversion.cpp:3539) aborted with
// `Type is not present in types_: bool[_]`.
//
// The target MUST be `libcc2rs::IgnoreRule`, exactly like t1/t2's UNSAFE
// targets: that keeps the converter on its OWN `ConstantArrayType` traversal
// (converter.cpp:845-850, GetArrayDefaultAsString at :6899), which emits
// `[T; N]` and so the EXTENT SURVIVES.  A `Vec<T1>`/`Box<[T1]>` target would
// give one C++ type two different Rust spellings depending on which path
// spelled it, because `normalizeTranslationRule` (mapper.cpp:1290-1305) has
// already rewritten N to `_` on the string path -- the extent is simply not
// recoverable there.
//
// MEASURED LIMIT OF THIS ENTRY -- read before "fixing" it (probe
// /home/agent/work/probe/carr.cpp, both models):
//   * It DOES clear the abort.  `std::map<PrimaryDimTypes, bool[4]>` now
//     translates (0 `pub struct` in the emitted .rs, so the array type was
//     rule-mapped, not ported).
//   * But the abort is the only thing it clears.  The map's VALUE parameter is
//     instantiated with this rule's TARGET, so unsafe emits
//         let mut m: BTreeMap<PrimaryDimTypes, Box<libcc2rs::IgnoreRule>>
//     and refcount `BTreeMap<PrimaryDimTypes, Value<libcc2rs::IgnoreRule>>`.
//     `m[k][i]` is then E0608 "cannot index into a value of type
//     Box<IgnoreRule>" (22 errors unsafe, 11 refcount).  IgnoreRule keeps the
//     extent only on the DECL path -- a local `int a[4]` already emitted
//     `(0..4).map(|_| 0_i32).collect::<Box<[i32]>>()` WITHOUT this rule -- and
//     the template-argument path, which is the only path that aborted, is
//     exactly the path where the extent is already gone (normalizeTranslationRule
//     wildcarded N to `_` before the lookup).
//   * So this is a LOUD-to-LOUD trade, not a fix: converter abort -> crate that
//     does not build.  It is kept because ddl_conversion.cpp aborts again
//     immediately afterwards anyway (llvm::PointerUnion, see below), so the
//     entry buys the next blocker's exposure and costs nothing that compiles.
//     `Vec<T1>` / `Box<[T1]>` targets are NOT better: they compile and then
//     panic or silently alias, because a default-constructed one has the wrong
//     length and the real length is unrecoverable on the string path.
//   * A real fix needs a CONTAINER-side change (the same conclusion the
//     unbounded-array note below reached): the map rule would have to keep the
//     value type's extent, or the converter would have to carry N through
//     normalizeTranslationRule.  That is a converter/mapper change, not a rule.
template <typename T1, std::size_t T2> using t3 = T1[T2];

template <typename T1, std::size_t T2> using t1 = T1[T2][T2];
template <typename T1, std::size_t T2> using t2 = T1[T2][T2][T2];

// STILL NOT ADDED: an array of UNKNOWN bound,
// `template <typename T1> using t3 = T1[];`.
// It resolves cleanly (key "]", so only unbounded arrays reach it; a bounded
// `int[5]` keys as "5]") and it DOES clear the "Type is not present in types_"
// abort that `std::shared_ptr<uint8_t[]>` in dbo/src/Utils/sdsc_bundle causes
// in four TUs.  But the element type is all it fixes: `buf[0]` on a
// `shared_ptr<T[]>` still lowers to a raw `.offset(0).write(..)` on the
// shared_ptr ITSELF (Ptr<Option<Value<Box<[u8]>>>>), which does not typecheck.
// The alias alone trades a loud converter abort for a crate that does not
// build.  shared_ptr<T[]> needs a real sub-model -- element type, operator[],
// and the ctor from `new T[n]` -- before the alias is worth having.
//
// That sub-model now EXISTS, and it is in rules/shared_ptr, not here: `t3 =
// std::shared_ptr<T1[]>` plus `t4 = std::shared_ptr<void>` and the members
// f34-f43.  The diagnosis above was right about where the fault lay -- the
// container, not the element type -- so the fix had to be a CONTAINER rule,
// and adding the alias here would still be the wrong move: it would make a
// bare incomplete array type nameable in contexts nothing has verified, while
// buying nothing that shared_ptr's own t3 does not already buy.  Measured:
// with rules/shared_ptr t3/t4 in place and this file unchanged, the
// `char[]`/`unsigned char[]` bucket in the scheduler census goes from 6 TUs to
// 0 and the OK count from 29 to 31.
