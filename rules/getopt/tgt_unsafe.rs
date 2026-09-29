// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// t1 -- `struct option`.  `::libc::option` is field-for-field the C struct
// (`name: *const c_char, has_arg: c_int, flag: *mut c_int, val: c_int`), and the
// FIELD NAMES matter: the converter emits the corpus's
// `{"help", no_argument, nullptr, 'h'}` as a named aggregate initialiser
// `::libc::option { name: ..., has_arg: ..., flag: ..., val: ... }`, so any
// representation with different field names would not compile at the use site.
fn t1() -> ::libc::option {
    unsafe { std::mem::zeroed() }
}

// ⭐ THE FOUR GLOBALS ARE DECLARED HERE, not imported from libcc2rs, and the
// reason is MEASURED rather than stylistic.  The `libc` crate binds `getopt`,
// `getopt_long` and `struct option` but NOT `optarg`/`optind`/`opterr`/`optopt`
// (libc-0.2.189: `grep -rn 'optarg|optind|optopt|opterr' src/unix/` is empty),
// so somebody has to declare them.  The natural home is `libcc2rs`, and
// `libcc2rs/src/getopt.rs` DOES declare them and carries the round-trip test --
// but the rule preprocessor's Rust stage type-checks this file against the
// PREBUILT `liblibcc2rs-*.rmeta` in `pin/target_preprocessor`, which is built
// from `repos/cpp2rust/libcc2rs`.  A worktree's addition is invisible to it:
//     error[E0425]: cannot find function `getopt_optarg` in crate `libcc2rs`
//     thread 'main' panicked at src/semantic.rs:260: unresolved access="unknown"
// (measured, and the regen then core-dumps with no `OK getopt -> ...` line).
// Building a private artifact store instead fails at `cargo build` --
// `package collision in the lockfile: libcc2rs ... /wt/cpp2rust/libcc2rs and
// .../wt/getoptrow/libcc2rs are different` -- because dataflowir-gen pins main's
// copy.  So the declaration is repeated here.  ⭐ That is SAFE, not a fork:
// these are `extern "C"` declarations of glibc symbols, so both crates resolve
// to the same objects, and `libcc2rs/src/getopt.rs` remains the documented
// model and the place the contract is TESTED.  Once this branch is merged the
// bodies below can become `*libcc2rs::getopt_optarg()` etc. verbatim.
//
// ⛔⛔ AND THE DECLARATION MUST BE *INSIDE* THE BODY, NOT A FILE-LEVEL ITEM
// HERE.  Measured: a file-level `unsafe extern "C" { static mut optarg: ... }`
// in this file type-checks and regenerates fine, and then the converter emits
// the substituted body as the BARE NAME `optarg` into the translated TU --
// because a rule target's helper items are NOT copied into the output (grepping
// the generated dxp_standalone.cpp.rs for `extern "C"` finds nothing, and
// rules/limits' `pub trait HasMinMax` likewise never appears).  The generated
// file would then carry an undefined `optarg`, i.e. the SAME E0425 as the
// placeholder it replaced -- but WITHOUT the `Cpp2RustUnmapped` marker that the
// placeholder census greps for.  That is strictly worse than no key: it hides
// the miss.  A block expression carrying its own `extern` item is self-
// contained, so whatever the converter substitutes compiles on its own.
//
// f1..f4 -- the scanner's four globals, as PLACE expressions.  See
// rules/getopt/src.cpp f2: `optind` is assigned by the corpus
// (deeprt/deeprt_standalone.cpp:147, `optind = 1;`), so the substituted body has
// to be assignable.  `*<raw pointer>` IS a place expression; a plain block
// `{ ...; optind }` is NOT (`{ .. } = 1` does not parse), which is why every one
// of these goes through `&raw mut`.
unsafe fn f1() -> *mut ::libc::c_char {
    *{
        unsafe extern "C" {
            static mut optarg: *mut ::libc::c_char;
        }
        &raw mut optarg
    }
}

unsafe fn f2() -> i32 {
    *{
        unsafe extern "C" {
            static mut optind: ::libc::c_int;
        }
        &raw mut optind
    }
}

unsafe fn f3() -> i32 {
    *{
        unsafe extern "C" {
            static mut optopt: ::libc::c_int;
        }
        &raw mut optopt
    }
}

unsafe fn f4() -> i32 {
    *{
        unsafe extern "C" {
            static mut opterr: ::libc::c_int;
        }
        &raw mut opterr
    }
}

// f5/f6 -- the REAL scanner, not a model of it.  `getopt_long` is a stateful
// cursor over `argv` (it advances `optind`, publishes `optarg`, returns '?' for
// an unknown option and -1 at exhaustion); calling glibc's own implementation is
// the only representation that cannot drift from that contract.
unsafe fn f5(a0: i32, a1: *const *mut ::libc::c_char, a2: *const ::libc::c_char) -> i32 {
    ::libc::getopt(a0, a1, a2)
}

unsafe fn f6(
    a0: i32,
    a1: *const *mut ::libc::c_char,
    a2: *const ::libc::c_char,
    a3: *const ::libc::option,
    a4: *mut i32,
) -> i32 {
    ::libc::getopt_long(a0, a1, a2, a3, a4)
}
