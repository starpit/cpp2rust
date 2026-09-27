// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// C <ctype.h> scalar classification/conversion functions.
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
// Each body mentions `a0` EXACTLY ONCE (match scrutinee + binding pattern),
// because a rule body is inlined into the caller and every occurrence of `aN`
// re-evaluates the argument -- `tolower(*p++)` must advance the pointer once.

unsafe fn f1(a0: i32) -> i32 {
    match a0 {
        -1 => -1,
        __c @ 0x41..=0x5A => __c + 32,
        __c @ -128..=-2 => __c & 0xFF,
        __c => __c,
    }
}

unsafe fn f2(a0: i32) -> i32 {
    match a0 {
        -1 => -1,
        __c @ 0x61..=0x7A => __c - 32,
        __c @ -128..=-2 => __c & 0xFF,
        __c => __c,
    }
}

unsafe fn f3(a0: i32) -> i32 {
    match a0 {
        0x20 | 0x09..=0x0D => 1,
        _ => 0,
    }
}
