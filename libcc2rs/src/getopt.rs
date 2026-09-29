// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

//! `getopt(3)` / `getopt_long(3)`: the STATEFUL command-line scanner, and the
//! four globals it walks `argv` through.
//!
//! ⭐ WHY THIS IS NOT A REIMPLEMENTATION.  `getopt_long` is a *cursor* over
//! `argv`: each call advances `optind`, publishes the current option's argument
//! in `optarg`, reports an unrecognised option as `'?'` (leaving it in
//! `optopt`), and returns `-1` once options are exhausted -- at which point
//! `optind` indexes the first NON-option argument.  Every one of those four
//! facts is load-bearing for the corpus: `dxp/dxp_standalone.cpp:66` and eleven
//! sibling `*_standalone.cpp` drivers are a `while ((tmp = getopt_long(...)) !=
//! -1)` loop whose body reads `optarg`.
//!
//! ⛔ A MODEL THAT RETURNED A FIXED VALUE, OR THAT LEFT `optarg` UNSET, WOULD
//! MAKE EVERY COMMAND-LINE FLAG SILENTLY IGNORED: the translated `main` would
//! run with default options, exit 0, and look like it worked.  Nothing short of
//! a differential run would see it.  So the state is not modelled -- the real
//! glibc scanner IS the model.  Rust's std has no `getopt`, and the `libc` crate
//! binds `getopt`/`getopt_long`/`struct option` but NOT the four globals
//! (verified against libc-0.2.189: `grep -rn 'optarg\|optind\|optopt\|opterr'
//! src/unix/` is empty), which is the entire reason this file exists.
//!
//! ⭐ THE ACCESSORS RETURN POINTERS, NOT VALUES, on purpose.  `optind` is
//! ASSIGNED by the corpus (`deeprt/deeprt_standalone.cpp:147`, `optind = 1;`
//! to restart a scan), so the rule body substituted at a use site has to be a
//! PLACE expression, not a call.  `*getopt_optind()` is a place; a
//! `getopt_optind_value()` would have silently turned that reset into a
//! rustc error at best and a dropped reset at worst.

use std::ffi::{c_char, c_int};

unsafe extern "C" {
    /// The argument of the option just returned, or null.  Points INTO `argv`.
    pub static mut optarg: *mut c_char;
    /// Index of the next `argv` element to scan.  1 at start; after the scan
    /// returns -1 it indexes the first non-option argument.  Writable.
    pub static mut optind: c_int;
    /// Non-zero (the default) makes the scanner print its own diagnostic for an
    /// unrecognised option.  Set to 0 to silence it.
    pub static mut opterr: c_int;
    /// The unrecognised option character, set when the scanner returns `'?'`.
    pub static mut optopt: c_int;
}

/// `&raw mut optarg` -- see the module note on why this is a pointer.
#[inline]
pub fn getopt_optarg() -> *mut *mut c_char {
    &raw mut optarg
}

/// `&raw mut optind`.
#[inline]
pub fn getopt_optind() -> *mut c_int {
    &raw mut optind
}

/// `&raw mut opterr`.
#[inline]
pub fn getopt_opterr() -> *mut c_int {
    &raw mut opterr
}

/// `&raw mut optopt`.
#[inline]
pub fn getopt_optopt() -> *mut c_int {
    &raw mut optopt
}

#[cfg(test)]
mod getopt_tests {
    use super::*;
    use std::ffi::CString;
    use std::sync::Mutex;

    /// ⛔ THE SCANNER IS PROCESS-GLOBAL AND `cargo test` RUNS TESTS IN PARALLEL
    /// THREADS, so two scans in flight at once interleave through the same
    /// `optind`/`optarg`.  Measured: without this lock
    /// `getopt_short_only_round_trip` failed on its final `optind` assertion
    /// while the other two passed -- i.e. the flake looked like a contract error
    /// in the model.  Serialise instead of weakening the assertions.
    static SCANNER: Mutex<()> = Mutex::new(());

    /// Rebuild the scanner's state so the tests do not depend on each other or
    /// on the harness's own `argv`.  `optind = 0` is glibc's documented reset
    /// (it re-reads `argv[0]` and re-initialises the internal `nextchar`),
    /// which is what makes running several scans in one process well defined.
    unsafe fn reset() {
        unsafe {
            optind = 0;
            opterr = 0; // do not let the scanner write to the test's stderr
            optarg = std::ptr::null_mut();
            optopt = 0;
        }
    }

    fn argv_of(words: &[&str]) -> (Vec<CString>, Vec<*mut c_char>) {
        let owned: Vec<CString> = words.iter().map(|w| CString::new(*w).unwrap()).collect();
        let ptrs: Vec<*mut c_char> = owned.iter().map(|c| c.as_ptr() as *mut c_char).collect();
        (owned, ptrs)
    }

    /// ⭐⭐ THE ROUND TRIP, and it fails for a stub.  All three cases a
    /// fixed-value / `optarg`-never-set model would get wrong are asserted in
    /// one scan of one `argv`, in the shape `dxp_standalone.cpp:66` uses:
    ///   1. a short flag WITH an argument sets `optarg` to that argument;
    ///   2. an unknown flag returns `'?'` (and leaves it in `optopt`);
    ///   3. exhaustion returns `-1`, with `optind` pointing PAST the last
    ///      option, i.e. at the first non-option word.
    #[test]
    fn getopt_long_round_trip() {
        let _guard = SCANNER.lock().unwrap();
        // `-d <dir>` then an unknown `-z`, then a non-option tail.
        let words = ["prog", "-d", "outdir", "-z", "tail1", "tail2"];
        let (_owned, argv) = argv_of(&words);
        let optstring = CString::new("d:b:h").unwrap();
        let long_help = CString::new("help").unwrap();
        let longopts = [
            libc::option {
                name: long_help.as_ptr(),
                has_arg: 0,
                flag: std::ptr::null_mut(),
                val: b'h' as c_int,
            },
            libc::option {
                name: std::ptr::null(),
                has_arg: 0,
                flag: std::ptr::null_mut(),
                val: 0,
            },
        ];

        unsafe {
            reset();

            // (1) short flag with an argument.
            let c = libc::getopt_long(
                argv.len() as c_int,
                argv.as_ptr(),
                optstring.as_ptr(),
                longopts.as_ptr(),
                std::ptr::null_mut(),
            );
            assert_eq!(c, b'd' as c_int, "first option must be 'd'");
            let got = *getopt_optarg();
            assert!(!got.is_null(), "optarg must be set for `-d outdir`");
            assert_eq!(
                std::ffi::CStr::from_ptr(got).to_str().unwrap(),
                "outdir",
                "optarg must point at the option's argument"
            );

            // (2) unknown flag.
            let c = libc::getopt_long(
                argv.len() as c_int,
                argv.as_ptr(),
                optstring.as_ptr(),
                longopts.as_ptr(),
                std::ptr::null_mut(),
            );
            assert_eq!(c, '?' as c_int, "an unrecognised option must return '?'");
            assert_eq!(*getopt_optopt(), b'z' as c_int, "optopt must hold 'z'");

            // (3) exhaustion.
            let c = libc::getopt_long(
                argv.len() as c_int,
                argv.as_ptr(),
                optstring.as_ptr(),
                longopts.as_ptr(),
                std::ptr::null_mut(),
            );
            assert_eq!(c, -1, "the scan must report exhaustion as -1");
            let idx = *getopt_optind() as usize;
            assert_eq!(
                idx, 4,
                "optind must point past the last option, at the first non-option word"
            );
            assert_eq!(
                std::ffi::CStr::from_ptr(argv[idx]).to_str().unwrap(),
                "tail1",
                "argv[optind] must be the first non-option argument"
            );
        }
    }

    /// A LONG option, because `dxp_standalone` drives four of its five options
    /// through `longopts` (`--use-dxp` and friends) and a short-only model
    /// would pass the test above while ignoring all of them.
    #[test]
    fn getopt_long_matches_a_long_option_and_reports_longindex() {
        let _guard = SCANNER.lock().unwrap();
        let words = ["prog", "--help"];
        let (_owned, argv) = argv_of(&words);
        let optstring = CString::new("d:").unwrap();
        let long_help = CString::new("help").unwrap();
        let longopts = [
            libc::option {
                name: long_help.as_ptr(),
                has_arg: 0,
                flag: std::ptr::null_mut(),
                val: b'h' as c_int,
            },
            libc::option {
                name: std::ptr::null(),
                has_arg: 0,
                flag: std::ptr::null_mut(),
                val: 0,
            },
        ];
        let mut longindex: c_int = -1;
        unsafe {
            reset();
            let c = libc::getopt_long(
                argv.len() as c_int,
                argv.as_ptr(),
                optstring.as_ptr(),
                longopts.as_ptr(),
                &mut longindex,
            );
            assert_eq!(c, b'h' as c_int, "`--help` must yield its `val`");
            assert_eq!(longindex, 0, "longindex must name the matched longopts slot");
            assert_eq!(libc::getopt_long(
                argv.len() as c_int,
                argv.as_ptr(),
                optstring.as_ptr(),
                longopts.as_ptr(),
                &mut longindex,
            ), -1);
            assert_eq!(*getopt_optind() as usize, 2, "optind past the long option");
        }
    }

    /// The short-only `getopt(3)`, which ten corpus TUs use instead
    /// (`dip/dip_standalone.cpp:80`, `ddb/src/ddb_std.cpp:118`, ...).  Same
    /// globals, same `-1` contract.
    #[test]
    fn getopt_short_only_round_trip() {
        let _guard = SCANNER.lock().unwrap();
        let words = ["prog", "-i", "in.txt", "rest"];
        let (_owned, argv) = argv_of(&words);
        let optstring = CString::new("i:so").unwrap();
        unsafe {
            reset();
            let c = libc::getopt(argv.len() as c_int, argv.as_ptr(), optstring.as_ptr());
            assert_eq!(c, b'i' as c_int);
            assert_eq!(
                std::ffi::CStr::from_ptr(*getopt_optarg()).to_str().unwrap(),
                "in.txt"
            );
            assert_eq!(
                libc::getopt(argv.len() as c_int, argv.as_ptr(), optstring.as_ptr()),
                -1
            );
            assert_eq!(*getopt_optind() as usize, 3);
        }
    }
}
