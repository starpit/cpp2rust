// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Pass-through shim.
//
// `compat/` is placed on the command line with `-isystem` (cpp2rust/CMakeLists.txt),
// AHEAD of the sysroot. glibc's own headers include <features.h> unconditionally --
// e.g. compat/assert.h does `#include_next <assert.h>`, the sysroot's assert.h then
// asks for <features.h>, and the search starts at the head of the -isystem chain,
// i.e. here. Without this file that lookup fails outright:
//     fatal error: cannot open file '.../cpp2rust/compat/features.h'
// so every C and multi-file test dies before translating anything.
//
// `#include_next` resumes the search AFTER this directory, which reaches the real
// glibc features.h in the sysroot. Nothing is overridden; this only keeps the chain
// from terminating at compat/.
#include_next <features.h>
