// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model: a std::filesystem::path is its native string as a
// NON-NUL-TERMINATED Vec<u8>.  In THIS model a std::string is a NUL-terminated
// Vec<libc::c_char> whose size() is len()-1, so f2 strips the terminator on the
// way in and f5 adds exactly one on the way out.

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
fn f2(a0: Vec<libc::c_char>) -> Vec<u8> {
    let mut __p: Vec<u8> = a0.iter().map(|&__b| __b as u8).collect();
    if __p.last() == Some(&0) {
        __p.pop();
    }
    __p
}

// path(const char *) -- read up to, and not including, the terminator.
unsafe fn f3(a0: *const libc::c_char) -> Vec<u8> {
    ::std::ffi::CStr::from_ptr(a0).to_bytes().to_vec()
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
fn f5(a0: Vec<u8>) -> Vec<libc::c_char> {
    let mut __s: Vec<libc::c_char> = a0.iter().map(|&__b| __b as libc::c_char).collect();
    __s.push(0);
    __s
}

// path(std::string &&) -- MOVES the bytes and drops the NUL terminator, which is
// representation and not content.  Same result as f2, different key: see src.cpp.
fn f6(a0: Vec<libc::c_char>) -> Vec<u8> {
    let mut __p: Vec<u8> = a0.iter().map(|&__b| __b as u8).collect();
    if __p.last() == Some(&0) {
        __p.pop();
    }
    __p
}
