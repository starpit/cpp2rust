// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// ============================================================================
// rules/getopt -- `<getopt.h>`: `struct option`, `getopt`, `getopt_long`, and
// the four globals the scanner walks `argv` through.
//
// ⭐⭐ t1 IS THE CURRENT FIRST ABORT OF `dxp/dxp_standalone.cpp`, THE STATED
// PORT GOAL.  Verbatim, against pin/ir.v39 + snap/coord43/cpp2rust:
//     LLVM ERROR: unsupported system type has no rule: `option`
//       (would be emitted as the undefined name `option`), reached while
//       converting `main`
//       rule key: searched as: option; from decl (NOT a key -- canonicalised,
//       defaulted args kept): option
//       at .../lib/clang/22/include/bits/getopt_ext.h:50:8
// Note the SPELLING: the key is the bare `option`, no `std::` and no `::` --
// it is a C struct at global scope.  Read back from ir_src.json as `"t1":
// "option"`.
//
// WHY A NEW MODULE.  `getopt` is declared in `<unistd.h>` as well as
// `<getopt.h>`, so rules/unistd was the candidate owner -- but rules/unistd is
// eleven one-line fd/path syscalls with no type key and no state, and
// `getopt_long`/`struct option` live only in `<getopt.h>`.  Verified there is no
// duplicate owner: `grep -rn -i 'optarg|optind|getopt' rules/` over the whole
// tree returns NOTHING, and no module directory is named for libc's option
// scanner.  ⚠️ This module takes the tree from 95 to 96, so check-ir.sh reports
// 96 modules -- that is this row, not a fault.
//
// ⭐ THE SURFACE IS MEASURED, not guessed.  Census over repos/dt_src
// (*.cpp/*.h/*.cc/*.hpp, word-boundary):
//     struct option   12 sites / 12 files   (always `struct option longopts[]`)
//     getopt_long     12 sites / 12 files
//     getopt          10 sites / 10 files   (excluding `#include <getopt.h>`)
//     optarg         105 sites / 21 files
//     optind           3 sites /  2 files   (ONE of them a WRITE, see f3)
//     optopt           0 sites /  0 files
//     opterr           0 sites /  0 files
// optopt/opterr are keyed anyway: they are the same three lines, they are part
// of the same scanner state, and a caller that sets `opterr = 0` to silence the
// scanner's own stderr would otherwise be a silent behaviour change rather than
// a miss.
//
// ⛔⛔ WHAT WOULD HAVE BEEN SILENTLY WRONG, and why the whole family lands
// together rather than the type alone.  With ONLY t1 present the goal TU
// translates rc=0 at 38,343 lines -- and the loop body reads
//     tmp = Cpp2RustUnmappedFn_getopt_long_225(...)
//     sdscDir = (Cpp2RustUnmappedVar_optarg_226).cast_const();
// i.e. `getopt_long` and `optarg` do NOT abort; they become marked placeholder
// NAMES.  Those are loud at rustc (E0425) rather than at translate time, so a
// type-only landing would have moved the goal TU from "aborts" to "rc=0 and
// does not compile", which reads like progress and is not.  Keying the type
// without the functions is therefore refused here on the same grounds as the
// standing rule about a type key with no method key.
//
// ⛔ AND THE BEHAVIOURAL TRAP: `getopt_long` is a CURSOR.  Each call mutates
// `optind`, publishes the current argument in `optarg`, returns `'?'` for an
// unrecognised option, and returns -1 once options are exhausted.  A body that
// returned a fixed value, or that left `optarg` unset, would make EVERY
// command-line flag silently ignored -- the translated `main` would run with
// default options and exit 0.  So the target does not reimplement the scanner:
// it calls the REAL one (`libc::getopt_long`), and the globals are bound as the
// real `extern "C"` statics in libcc2rs/src/getopt.rs, which carries the
// round-trip test over exactly the three cases a stub would pass.
//
// ⭐ REFCOUNT IS DELIBERATELY LEFT WITHOUT THESE KEYS, and it is a refusal with
// a measured cause rather than an omission.  In the refcount model the converter
// emits the aggregate initialiser with EVERY FIELD Value-WRAPPED:
//     ::libc::option { name: Rc::new(RefCell::new(Ptr::<u8>::from_string_literal(b"help"))),
//                      has_arg: Rc::new(RefCell::new(0)), flag: ..., val: ... }
// (measured on a 19-line probe TU, snap/coord43/cpp2rust --model=refcount), so
// `::libc::option` -- whose fields are `*const c_char`/`c_int` -- cannot be the
// refcount representation, and the honest refcount model is a `libcc2rs` struct
// of `Value<Ptr<u8>>`/`Value<i32>` in the shape of rules/dirent's `Dirent`, fed
// by a scanner that walks `Ptr<Ptr<u8>>` instead of `*const *mut c_char`.  That
// is a second model, not a second spelling, and it did not fit this slot's
// budget.  Leaving the refcount side EMPTY keeps it failing LOUDLY at translate
// time with the abort quoted at the top, which is the required outcome; writing
// `::libc::option` there would have produced a type-mismatch wall in the
// caller's own file instead.  rules/errno is the precedent for a legitimately
// partial refcount target (1 key of 153).  The goal TU does not reach this row
// in refcount anyway: it aborts 660 lines earlier on
// `unsupported structured binding / DecompositionDecl with 2 bindings [k, v]`
// at util/utils.h:148.
// ============================================================================

#include <getopt.h>

using t1 = struct option;

// f1 -- `optarg`, the argument of the option just returned (null if none).
// Recorded key is the BARE NAME `optarg`, with no signature and no parentheses:
// a system global is lowered through ConvertDeclRefExpr, and the rule
// preprocessor's `declref` matcher records printQualifiedName.  rules/iostream
// f1 (`std::cout`) is the precedent for the shape.
// ⭐ The wrapper returns the global by value only to fix the RUST TYPE
// (`*mut c_char`); the target BODY is a place expression, see tgt_unsafe.rs.
char *f1(void) { return optarg; }

// f2 -- `optind`.  ⭐ THIS ONE IS ASSIGNED BY THE CORPUS:
// `deeprt/deeprt_standalone.cpp:147` is `optind = 1;  // initialization of
// global val for getopt`, restarting the scan before a second pass.  That is
// why the target body must be a PLACE expression (`*libcc2rs::getopt_optind()`)
// and not a call returning a value -- a call would make the reset a rustc error
// at best, and if it ever compiled it would DROP the reset and make the second
// scan see no options at all.
int f2(void) { return optind; }

// f3/f4 -- `optopt` (the unrecognised option character, set when the scan
// returns '?') and `opterr` (non-zero, the default, makes the scanner print its
// own diagnostic; a caller sets it to 0 to silence that).  No corpus site today;
// keyed because they are the same state and `opterr = 0` is a WRITE whose loss
// would change what the program prints.
int f3(void) { return optopt; }
int f4(void) { return opterr; }

// f5 -- `getopt(3)`, the short-only scanner.  10 corpus sites
// (dip/dip_standalone.cpp:80, ddb/src/ddb_std.cpp:118, ...).  Recorded key is
// the resolved libc signature, printed by the typedef-preferring printer.
int f5(int argc, char *const argv[], const char *optstring) {
  return getopt(argc, argv, optstring);
}

// f6 -- `getopt_long(3)`.  ⭐ THE ROW.  The emitted call site in the goal TU is
//     Cpp2RustUnmappedFn_getopt_long_225(argc, (argv).cast_const(),
//         c"d:b:h".as_ptr(), (longopts.as_mut_ptr()).cast_const(),
//         (&mut longindex as *mut i32))
// i.e. exactly `libc::getopt_long`'s own signature
// (c_int, *const *mut c_char, *const c_char, *const option, *mut c_int) -> c_int,
// which is why the target is a one-line forward rather than a shim.
int f6(int argc, char *const argv[], const char *optstring,
       const struct option *longopts, int *longindex) {
  return getopt_long(argc, argv, optstring, longopts, longindex);
}
