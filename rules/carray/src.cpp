// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstdlib>

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
