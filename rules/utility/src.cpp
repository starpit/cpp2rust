// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <utility>

// std::forward is a cast and nothing else: it hands back the reference it was
// given.  cpp2rust handles std::move natively but not std::forward, so every
// perfectly-forwarding project template (FoldManager::apply, make_unique
// wrappers) emits a call to a function that does not exist.
//
// Both spellings are needed: `std::forward<T &>(x)` resolves to the
// lvalue-returning overload and `std::forward<T>(x)` to the rvalue-returning
// one, and the return type is part of the signature cpp2rust looks up.

template <typename T1> T1 &f1(T1 &a0) { return std::forward<T1 &>(a0); }

template <typename T1> T1 &&f2(T1 &a0) { return std::forward<T1>(a0); }
