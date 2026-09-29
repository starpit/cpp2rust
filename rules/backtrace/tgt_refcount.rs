// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// ⛔ DELIBERATELY EMPTY, AND THE REASON IS A REPRESENTATION WALL, NOT LAZINESS.
// In the refcount model `void **` is `Ptr<AnyPtr>` and `char **` is `Ptr<Ptr<u8>>`,
// and an `AnyPtr` cannot hold an arbitrary TEXT-SEGMENT RETURN ADDRESS -- which is
// exactly and only what `backtrace` writes into the caller's array and what
// `backtrace_symbols` then resolves.  There is no refcount-model value that
// represents such an address, so any body here would either fabricate addresses
// (an empty or wrong stack trace, silently) or need a `todo!()`.  Both are worse
// than absence.
// ⚠️ CONSEQUENCE, STATED PLAINLY: the two models are a UNION, so the refcount
// model falls back to the UNSAFE body for f1-f3 and that fallback does not
// type-check there (raw pointers vs `Ptr<..>`).  That surfaces at rustc in the
// refcount model only, never as a silently wrong stack trace, and the unsafe
// model -- the one the goal TU is measured in -- is exact.
