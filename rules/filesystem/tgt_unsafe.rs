// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model and for the measured C++-vs-Rust divergences these
// bodies exist to avoid.  A path is the NUL-terminated byte string it denotes,
// the same representation rules/string gives std::string, so .string() (f1) and
// the std::string constructor (f3) are the identity.
//
// Every body slices off the trailing terminator before looking at the bytes and
// pushes a fresh one at the end, so the representation's invariant is preserved
// and the terminator is never mistaken for a character -- an off-by-one there
// would land inside the separator logic.
//
// f2 is PathBuf::push's rule transcribed onto the bytes rather than a call to
// it, because the receiver is a Vec, not a PathBuf: an absolute right operand
// REPLACES, an empty left operand does not contribute a separator, and a left
// operand already ending in a separator is appended to without adding another
// (which is what keeps "a//" / "b" == "a//b" rather than normalising it).
//
// f5/f6/f7 implement C++'s rule and NOT Rust's file_stem/extension/file_name,
// which disagree with it on six of twelve measured inputs: C++'s extension()
// keeps the leading dot, C++ does not skip a trailing separator when finding
// the file name, and C++ answers "." and ".." from stem() where Rust answers
// None.  The two special cases below -- a name made only of one or two dots,
// and a dot in first position not starting an extension -- are exactly C++'s.

fn t1() -> Vec<libc::c_char> {
    vec![0]
}

unsafe fn f1(a0: Vec<libc::c_char>) -> Vec<libc::c_char> {
    a0.clone()
}

unsafe fn f2(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let __sep = b'/' as libc::c_char;
    let __l = &a0[..a0.len().saturating_sub(1)];
    let __r = &a1[..a1.len().saturating_sub(1)];
    let mut __o: Vec<libc::c_char> = Vec::new();
    if __r.first() == Some(&__sep) || __l.is_empty() {
        __o.extend_from_slice(__r);
    } else {
        __o.extend_from_slice(__l);
        if __l.last() != Some(&__sep) {
            __o.push(__sep);
        }
        __o.extend_from_slice(__r);
    }
    __o.push(0);
    __o
}

unsafe fn f3(a0: Vec<libc::c_char>, a1: Option<()>) -> Vec<libc::c_char> {
    a0.clone()
}

unsafe fn f8(a0: Vec<libc::c_char>, a1: Option<()>) -> Vec<libc::c_char> {
    a0.clone()
}

unsafe fn f4(a0: &[libc::c_char], a1: Option<()>) -> Vec<libc::c_char> {
    ({
        trait __Cc2PathLit {
            fn __cc2_path_bytes(&self) -> Vec<libc::c_char>;
        }
        impl __Cc2PathLit for [libc::c_char] {
            fn __cc2_path_bytes(&self) -> Vec<libc::c_char> {
                let mut __o: Vec<libc::c_char> =
                    self.iter().copied().take_while(|&c| c != 0).collect();
                __o.push(0);
                __o
            }
        }
        impl __Cc2PathLit for ::std::ffi::CStr {
            fn __cc2_path_bytes(&self) -> Vec<libc::c_char> {
                let mut __o: Vec<libc::c_char> =
                    self.to_bytes().iter().map(|&b| b as libc::c_char).collect();
                __o.push(0);
                __o
            }
        }
        a0.__cc2_path_bytes()
    })
}

unsafe fn f5(a0: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let __sep = b'/' as libc::c_char;
    let __dot = b'.' as libc::c_char;
    let __c = &a0[..a0.len().saturating_sub(1)];
    let __n = match __c.iter().rposition(|&b| b == __sep) {
        Some(i) => &__c[i + 1..],
        None => __c,
    };
    let __all_dots = (__n.len() == 1 || __n.len() == 2) && __n.iter().all(|&b| b == __dot);
    let __k = if __all_dots {
        __n.len()
    } else {
        match __n.iter().rposition(|&b| b == __dot) {
            Some(i) if i > 0 => i,
            _ => __n.len(),
        }
    };
    let mut __o = __n[..__k].to_vec();
    __o.push(0);
    __o
}

unsafe fn f6(a0: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let __sep = b'/' as libc::c_char;
    let __dot = b'.' as libc::c_char;
    let __c = &a0[..a0.len().saturating_sub(1)];
    let __n = match __c.iter().rposition(|&b| b == __sep) {
        Some(i) => &__c[i + 1..],
        None => __c,
    };
    let __all_dots = (__n.len() == 1 || __n.len() == 2) && __n.iter().all(|&b| b == __dot);
    let __k = if __all_dots {
        __n.len()
    } else {
        match __n.iter().rposition(|&b| b == __dot) {
            Some(i) if i > 0 => i,
            _ => __n.len(),
        }
    };
    let mut __o = __n[__k..].to_vec();
    __o.push(0);
    __o
}

unsafe fn f7(a0: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let __sep = b'/' as libc::c_char;
    let __c = &a0[..a0.len().saturating_sub(1)];
    let __n = match __c.iter().rposition(|&b| b == __sep) {
        Some(i) => &__c[i + 1..],
        None => __c,
    };
    let mut __o = __n.to_vec();
    __o.push(0);
    __o
}
