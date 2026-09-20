// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstdlib>

template <typename T1, std::size_t T2> using t1 = T1[T2][T2];
template <typename T1, std::size_t T2> using t2 = T1[T2][T2][T2];

// NOT ADDED: an array of UNKNOWN bound, `template <typename T1> using t3 = T1[];`.
// It resolves cleanly (key "]", so only unbounded arrays reach it; a bounded
// `int[5]` keys as "5]") and it DOES clear the "Type is not present in types_"
// abort that `std::shared_ptr<uint8_t[]>` in dbo/src/Utils/sdsc_bundle causes
// in four TUs.  But the element type is all it fixes: `buf[0]` on a
// `shared_ptr<T[]>` still lowers to a raw `.offset(0).write(..)` on the
// shared_ptr ITSELF (Ptr<Option<Value<Box<[u8]>>>>), which does not typecheck.
// The alias alone trades a loud converter abort for a crate that does not
// build.  shared_ptr<T[]> needs a real sub-model -- element type, operator[],
// and the ctor from `new T[n]` -- before the alias is worth having.
