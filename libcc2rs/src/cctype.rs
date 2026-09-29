// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// C <ctype.h> scalar classification/conversion functions, as NAMED functions.
//
// `rules/cctype` already models `tolower`/`toupper`/`isspace` as rule BODIES,
// which the converter INLINES at each call site. That is the right lowering for
// an ordinary call. It is not available when the function is named without being
// called -- `std::transform(b, e, b, ::tolower)` passes an `int (*)(int)
// noexcept` as an operand -- because an inlined body has no address. For that
// site the converter lowers the operand to the CALLABLE form
// (`converter.cpp ConvertFnPtrCallee`) and emits `libcc2rs::tolower_unsafe`,
// i.e. it expects a real function item here, not a rule body.
//
// ⚠️ THESE BODIES MUST STAY SEMANTICALLY IDENTICAL TO `rules/cctype/tgt_*.rs`.
// The same C++ symbol reaches Rust by two routes -- inlined rule body when
// called, this function item when named -- and the two must not disagree. The
// comment block below is therefore the one from `rules/cctype`, kept in sync.
//
// These take and return `int`, so the targets are i32 -> i32: no narrowing to
// u8, which would turn EOF (-1) into 0xFF and mangle a sign-extended `char`.
//
// LOCALE: the C functions are locale-dependent; these bodies implement the "C"
// locale (ASCII) only, which is what dt_src needs -- every call site is
// identifier/keyword/option-name folding in a compiler toolchain. Bytes >= 0x80
// and EOF (-1) therefore pass through UNCHANGED rather than being remapped.
//
// A negative argument other than EOF is UB in C, but callers routinely pass a
// sign-extended `char`, so the `-128..=-2` arm reproduces the MEASURED glibc
// behaviour: glibc's table is indexed from -128, so tolower(-23) yields 233 --
// the value for the corresponding unsigned char (never a fold, since 0x80..0xFF
// hold no ASCII letters). Arguments below -128 are outside glibc's table
// entirely (a genuine out-of-bounds read) and are not modelled.
//
// Unlike the rule bodies, these are ordinary functions rather than inlined text,
// so the "mention `a0` exactly once" constraint does not apply -- an argument is
// evaluated by the caller exactly once regardless. The bodies are kept in the
// rules' `match` shape anyway, so that a divergence shows up as a textual diff.

pub fn tolower_refcount(a0: i32) -> i32 {
    match a0 {
        -1 => -1,
        __c @ 0x41..=0x5A => __c + 32,
        __c @ -128..=-2 => __c & 0xFF,
        __c => __c,
    }
}

pub fn toupper_refcount(a0: i32) -> i32 {
    match a0 {
        -1 => -1,
        __c @ 0x61..=0x7A => __c - 32,
        __c @ -128..=-2 => __c & 0xFF,
        __c => __c,
    }
}

pub fn isspace_refcount(a0: i32) -> i32 {
    match a0 {
        0x20 | 0x09..=0x0D => 1,
        _ => 0,
    }
}

/// # Safety
///
/// None -- this is a pure scalar function and has no safety contract. It is
/// `unsafe` only because the converter's callable form for the unsafe model
/// coerces every mapped function to `unsafe fn(..) -> ..`, so the emitted
/// `Some(libcc2rs::tolower_unsafe)` must have that type.
pub unsafe fn tolower_unsafe(a0: i32) -> i32 {
    tolower_refcount(a0)
}

/// # Safety
///
/// None; see `tolower_unsafe`.
pub unsafe fn toupper_unsafe(a0: i32) -> i32 {
    toupper_refcount(a0)
}

/// # Safety
///
/// None; see `tolower_unsafe`.
pub unsafe fn isspace_unsafe(a0: i32) -> i32 {
    isspace_refcount(a0)
}

#[cfg(test)]
mod tests {
    use super::*;

    // Every assertion below is the value glibc actually returns, not a
    // re-derivation of the body: the arms exist to match a measured table.
    #[test]
    fn tolower_folds_only_ascii_upper() {
        assert_eq!(tolower_refcount(b'A' as i32), b'a' as i32);
        assert_eq!(tolower_refcount(b'Z' as i32), b'z' as i32);
        assert_eq!(tolower_refcount(b'a' as i32), b'a' as i32);
        assert_eq!(tolower_refcount(b'_' as i32), b'_' as i32);
        assert_eq!(tolower_refcount(b'0' as i32), b'0' as i32);
        // EOF passes through; it must NOT become 0xFF.
        assert_eq!(tolower_refcount(-1), -1);
        // A sign-extended `char` 0xE9: glibc indexes from -128 and yields 233.
        assert_eq!(tolower_refcount(-23), 233);
        // 0x80..=0xFF hold no ASCII letters, so they are identities.
        assert_eq!(tolower_refcount(0xE9), 0xE9);
    }

    #[test]
    fn toupper_folds_only_ascii_lower() {
        assert_eq!(toupper_refcount(b'a' as i32), b'A' as i32);
        assert_eq!(toupper_refcount(b'z' as i32), b'Z' as i32);
        assert_eq!(toupper_refcount(b'A' as i32), b'A' as i32);
        assert_eq!(toupper_refcount(-1), -1);
        assert_eq!(toupper_refcount(-23), 233);
    }

    #[test]
    fn isspace_matches_the_c_locale_set() {
        for c in [b' ', b'\t', b'\n', 0x0B, 0x0C, b'\r'] {
            assert_ne!(isspace_refcount(c as i32), 0, "{c:#x} is a C-locale space");
        }
        for c in [b'a', b'0', 0x00, 0x1F, 0x7F] {
            assert_eq!(isspace_refcount(c as i32), 0, "{c:#x} is not a space");
        }
        assert_eq!(isspace_refcount(-1), 0);
    }

    // The converter emits these three as `unsafe fn(i32) -> i32` VALUES, not as
    // calls (`Some(libcc2rs::tolower_unsafe)` inside a `std::mem::transmute` to
    // `Option<unsafe fn(i32) -> i32>`). This test pins that coercion, which a
    // plain call would not: a signature change to `u8` or a non-`unsafe` fn
    // would still compile as a call and would break every emitted site.
    #[test]
    fn the_unsafe_forms_coerce_to_the_fn_pointer_the_converter_emits() {
        let fns: [unsafe fn(i32) -> i32; 3] =
            [tolower_unsafe, toupper_unsafe, isspace_unsafe];
        let opt: Option<unsafe fn(i32) -> i32> = Some(tolower_unsafe);
        unsafe {
            assert_eq!(fns[0](b'Q' as i32), b'q' as i32);
            assert_eq!(fns[1](b'q' as i32), b'Q' as i32);
            assert_eq!(fns[2](b' ' as i32), 1);
            assert_eq!(opt.unwrap()(b'Q' as i32), b'q' as i32);
        }
    }

    // The unsafe form must not diverge from the refcount form, and neither may
    // diverge from `rules/cctype`. Sweep the whole modelled domain.
    #[test]
    fn the_two_models_agree_over_the_whole_modelled_domain() {
        for c in -128..=0xFFi32 {
            unsafe {
                assert_eq!(tolower_unsafe(c), tolower_refcount(c), "tolower({c})");
                assert_eq!(toupper_unsafe(c), toupper_refcount(c), "toupper({c})");
                assert_eq!(isspace_unsafe(c), isspace_refcount(c), "isspace({c})");
            }
        }
    }

    // `free_unsafe` is `libc::free`, so it is only correct on C-allocated
    // memory. Round-trip a `libc::malloc`'d block to pin that, and to pin that
    // the two are the same allocator (a Rust-allocator `free` would abort here
    // under a checking allocator and is UB regardless).
    #[test]
    fn free_unsafe_round_trips_a_libc_malloc() {
        unsafe {
            let p = crate::malloc_unsafe(64);
            assert!(!p.is_null());
            std::ptr::write_bytes(p as *mut u8, 0xAB, 64);
            assert_eq!(*(p as *const u8).add(63), 0xAB);
            crate::free_unsafe(p);
            // free(NULL) is a documented no-op and emitted sites do reach it.
            crate::free_unsafe(std::ptr::null_mut());
        }
    }
}
