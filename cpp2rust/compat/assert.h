// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include_next <assert.h>

#undef assert

#ifndef __cplusplus
#include <stdbool.h>
#endif

void cpp2rust_assert_fail(bool condition) __attribute__((noreturn));

#ifdef __cplusplus
// assert() contextually converts its operand, so a plain bool parameter
// rejects types with an *explicit* operator bool (mlir::Value, llvm::Error,
// iterators, ...). Convert explicitly to match the real assert().
#define assert(expr) cpp2rust_assert_fail(static_cast<bool>(expr))
#else
#define assert(expr) cpp2rust_assert_fail(expr)
#endif
