// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp.  In THIS model a std::string is a NUL-terminated Vec<u8> and a
// `const char *` is a Ptr<u8>, so f2, f3 and f5 have DIFFERENT bodies from the
// unsafe ones (Vec<u8> / Ptr<u8> vs Vec<libc::c_char> / *const libc::c_char).
//
// THIS FILE MUST EXIST.  For a model-DEPENDENT parameter or return type,
// deleting it does not leave the key out of the refcount model -- the converter
// falls back to the UNSAFE body, which here would take/return
// Vec<libc::c_char> and give error[E0308] "expected Vec<u8>, found Vec<i8>"
// (measured for rules/sstream, same shape).

use libcc2rs::*;

fn t1() -> Vec<u8> {
    Vec::new()
}

// path() -- the empty path.
fn f1() -> Vec<u8> {
    Vec::new()
}

// path(const std::string &) -- a COPY of the string's bytes without its NUL
// terminator, which is representation and not content.  Copying matters: the
// argument is a const reference and must not be emptied.
fn f2(a0: Vec<u8>) -> Vec<u8> {
    let mut __p: Vec<u8> = a0.clone();
    if __p.last() == Some(&0) {
        __p.pop();
    }
    __p
}

// path(const char *) -- the bytes up to, and not including, the terminator.
fn f3(a0: Ptr<u8>) -> Vec<u8> {
    let mut __p: Vec<u8> = Vec::new();
    a0.with_c_str(|__s| __p.extend_from_slice(__s));
    __p
}

// operator/(const path &, const path &) -- append with EXACTLY ONE separator.
// An absolute right operand replaces the left one; an empty right operand
// leaves the left one alone; otherwise '/' is inserted only when it is not
// already present at the join.
// Each `aN` is bound ONCE: a rule body is inlined and every mention of `aN`
// re-expands the argument expression, so repeating one would re-evaluate it.
// `return` is also unavailable (rule bodies reject it, syntactic.rs:426), hence
// the if/else-if chain as an expression.
fn f4(a0: Vec<u8>, a1: Vec<u8>) -> Vec<u8> {
    let __lhs: Vec<u8> = a0;
    let __rhs: Vec<u8> = a1;
    if __rhs.first() == Some(&b'/') {
        __rhs
    } else if __rhs.is_empty() {
        __lhs
    } else {
        let mut __out: Vec<u8> = __lhs;
        if !__out.is_empty() && __out.last() != Some(&b'/') {
            __out.push(b'/');
        }
        __out.extend_from_slice(&__rhs);
        __out
    }
}

// .string() -- a COPY of the native string, plus exactly ONE trailing NUL,
// because rules/string's size() is len()-1.
fn f5(a0: Vec<u8>) -> Vec<u8> {
    let mut __s: Vec<u8> = a0.clone();
    __s.push(0);
    __s
}

// path(std::string &&) -- MOVES the bytes and drops the NUL terminator, which is
// representation and not content.  Same result as f2, different key: see src.cpp.
fn f6(a0: Vec<u8>) -> Vec<u8> {
    let mut __p: Vec<u8> = a0;
    if __p.last() == Some(&0) {
        __p.pop();
    }
    __p
}

// path(const char (&)[N], format) -- the key a string literal selects.  ONE key
// covers every length: mapper.cpp:1251 normalises the extent to `_`.  Same body
// as f3, and the parameter must be a POINTER, not a slice -- see src.cpp.
fn f7(a0: Ptr<u8>) -> Vec<u8> {
    let mut __p: Vec<u8> = Vec::new();
    a0.with_c_str(|__s| __p.extend_from_slice(__s));
    __p
}
