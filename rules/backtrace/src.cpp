// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// THE STACK-TRACE TRIPLE.  These three live in ONE module deliberately: they are
// only ever used together (util/dt_exception.hpp:91-92 + :65 and
// external/g3log/crashhandler_unix.cpp:159-160 + :186 are the only sites in the
// corpus), and they are the three names behind 3 of dxp_standalone.cpp's
// placeholders.  `backtrace`/`backtrace_symbols` are <execinfo.h> and
// `__cxa_demangle` is <cxxabi.h>; naming the module after the family rather than
// after either header is the honest description of what it covers.
// ⚠️ THIS IS A NEW MODULE, so check-ir.sh reads 97 modules, not 96.
//
// ⛔ NONE OF THE THREE IS MODELLED -- each body CALLS THE REAL GLIBC SYMBOL,
// because each one's contract is observable and a model would be a lie:
//   * backtrace(void **, int) WRITES INTO THE CALLER'S ARRAY and returns the
//     count.  A body returning 0 yields a silently EMPTY stack trace, and
//     std::backtrace is not available on the pinned toolchain.
//   * backtrace_symbols ALLOCATES an array the caller must free() -- the caller
//     at dt_exception.hpp does exactly that, so the allocation must be a real
//     malloc'd block, not a Rust Vec.
//   * __cxa_demangle ALLOCATES (caller frees) AND writes through an in/out
//     `status` pointer.
// ⭐ Each body therefore carries its OWN `unsafe extern "C"` declaration inside
// the block expression.  A FILE-LEVEL item in a tgt_*.rs is NOT copied into the
// emitted output (measured on rules/getopt), so a file-level extern block would
// emit the bare name and lose the Cpp2RustUnmapped marker -- strictly worse than
// no key.  These can each become `libcc2rs::...` verbatim once the prebuilt
// liblibcc2rs rmeta in pin/target_preprocessor is rebuilt (queue g2985).

#include <cstddef>
#include <cxxabi.h>
#include <execinfo.h>

int f1(void **a0, int a1) { return backtrace(a0, a1); }

char **f2(void *const *a0, int a1) { return backtrace_symbols(a0, a1); }

char *f3(const char *a0, char *a1, std::size_t *a2, int *a3) {
  return abi::__cxa_demangle(a0, a1, a2, a3);
}
