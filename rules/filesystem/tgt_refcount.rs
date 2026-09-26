// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Overlay on tgt_unsafe.rs.  Only the byte type and the `const char (&)[N]`
// argument change shape: a path is Vec<u8> rather than Vec<libc::c_char>, and a
// string literal arrives as Ptr<u8>, which walks itself to the terminator.  The
// lexical rules are identical -- see src.cpp for what they are and why they are
// C++'s and not Rust's std::path's.  t1 is repeated because the element type
// differs from the unsafe model's.

use libcc2rs::*;

fn t1() -> Vec<u8> {
    vec![0]
}

fn f1(a0: Vec<u8>) -> Vec<u8> {
    a0
}

fn f2(a0: Vec<u8>, a1: Vec<u8>) -> Vec<u8> {
    let __l = &a0[..a0.len().saturating_sub(1)];
    let __r = &a1[..a1.len().saturating_sub(1)];
    let mut __o: Vec<u8> = Vec::new();
    if __r.first() == Some(&b'/') || __l.is_empty() {
        __o.extend_from_slice(__r);
    } else {
        __o.extend_from_slice(__l);
        if __l.last() != Some(&b'/') {
            __o.push(b'/');
        }
        __o.extend_from_slice(__r);
    }
    __o.push(0);
    __o
}

fn f3(a0: Vec<u8>, a1: Option<()>) -> Vec<u8> {
    a0
}

fn f8(a0: Vec<u8>, a1: Option<()>) -> Vec<u8> {
    a0
}

fn f4(a0: &[u8], a1: Option<()>) -> Vec<u8> {
    ({
        trait __Cc2PathLit {
            fn __cc2_path_bytes(&self) -> Vec<u8>;
        }
        impl __Cc2PathLit for [u8] {
            fn __cc2_path_bytes(&self) -> Vec<u8> {
                let mut __o: Vec<u8> = self.iter().copied().take_while(|&c| c != 0).collect();
                __o.push(0);
                __o
            }
        }
        impl __Cc2PathLit for ::std::ffi::CStr {
            fn __cc2_path_bytes(&self) -> Vec<u8> {
                let mut __o: Vec<u8> = self.to_bytes().to_vec();
                __o.push(0);
                __o
            }
        }
        a0.__cc2_path_bytes()
    })
}

fn f5(a0: Vec<u8>) -> Vec<u8> {
    let __c = &a0[..a0.len().saturating_sub(1)];
    let __n = match __c.iter().rposition(|&b| b == b'/') {
        Some(i) => &__c[i + 1..],
        None => __c,
    };
    let __all_dots = (__n.len() == 1 || __n.len() == 2) && __n.iter().all(|&b| b == b'.');
    let __k = if __all_dots {
        __n.len()
    } else {
        match __n.iter().rposition(|&b| b == b'.') {
            Some(i) if i > 0 => i,
            _ => __n.len(),
        }
    };
    let mut __o = __n[..__k].to_vec();
    __o.push(0);
    __o
}

fn f6(a0: Vec<u8>) -> Vec<u8> {
    let __c = &a0[..a0.len().saturating_sub(1)];
    let __n = match __c.iter().rposition(|&b| b == b'/') {
        Some(i) => &__c[i + 1..],
        None => __c,
    };
    let __all_dots = (__n.len() == 1 || __n.len() == 2) && __n.iter().all(|&b| b == b'.');
    let __k = if __all_dots {
        __n.len()
    } else {
        match __n.iter().rposition(|&b| b == b'.') {
            Some(i) if i > 0 => i,
            _ => __n.len(),
        }
    };
    let mut __o = __n[__k..].to_vec();
    __o.push(0);
    __o
}

fn f7(a0: Vec<u8>) -> Vec<u8> {
    let __c = &a0[..a0.len().saturating_sub(1)];
    let __n = match __c.iter().rposition(|&b| b == b'/') {
        Some(i) => &__c[i + 1..],
        None => __c,
    };
    let mut __o = __n.to_vec();
    __o.push(0);
    __o
}

// operator/= overlay.  Same four-clause rule as f2/f9; the receiver of a
// reference-returning mutator is a `Ptr<Vec<u8>>` mutated through `with_mut`,
// as rules/string's f35 does.  ONE `with_mut` per body: the rhs bytes are
// copied out into `__r` FIRST, before the mutable borrow is taken, so no
// statement ever holds two guards on the same receiver.
fn f9(a0: Ptr<Vec<u8>>, a1: Vec<u8>) -> Ptr<Vec<u8>> {
    let __o = a0;
    let __r: Vec<u8> = a1[..a1.len().saturating_sub(1)].to_vec();
    __o.with_mut(|__v: &mut Vec<u8>| {
        __v.pop();
        if __r.first() == Some(&b'/') || __v.is_empty() {
            __v.clear();
        } else if __v.last() != Some(&b'/') {
            __v.push(b'/');
        }
        __v.extend_from_slice(&__r);
        __v.push(0);
    });
    __o
}

fn f10(a0: Ptr<Vec<u8>>, a1: Vec<u8>) -> Ptr<Vec<u8>> {
    let __o = a0;
    let __r: Vec<u8> = a1[..a1.len().saturating_sub(1)].to_vec();
    __o.with_mut(|__v: &mut Vec<u8>| {
        __v.pop();
        if __r.first() == Some(&b'/') || __v.is_empty() {
            __v.clear();
        } else if __v.last() != Some(&b'/') {
            __v.push(b'/');
        }
        __v.extend_from_slice(&__r);
        __v.push(0);
    });
    __o
}

fn f11(a0: Ptr<Vec<u8>>, a1: &[u8]) -> Ptr<Vec<u8>> {
    trait __Cc2PathAppLit {
        fn __cc2_path_app_bytes(&self) -> Vec<u8>;
    }
    impl __Cc2PathAppLit for [u8] {
        fn __cc2_path_app_bytes(&self) -> Vec<u8> {
            self.iter().copied().take_while(|&c| c != 0).collect()
        }
    }
    impl __Cc2PathAppLit for ::std::ffi::CStr {
        fn __cc2_path_app_bytes(&self) -> Vec<u8> {
            self.to_bytes().to_vec()
        }
    }
    let __o = a0;
    let __r = a1.__cc2_path_app_bytes();
    __o.with_mut(|__v: &mut Vec<u8>| {
        __v.pop();
        if __r.first() == Some(&b'/') || __v.is_empty() {
            __v.clear();
        } else if __v.last() != Some(&b'/') {
            __v.push(b'/');
        }
        __v.extend_from_slice(&__r);
        __v.push(0);
    });
    __o
}

fn f12(a0: Ptr<u8>) -> Vec<u8> {
    let mut __o: Vec<u8> = a0.to_c_string_iterator().collect();
    __o.push(0);
    __o
}

fn f13(a0: Vec<u8>) -> Vec<u8> {
    let __c = &a0[..a0.len().saturating_sub(1)];
    let mut __o: Vec<u8> = match __c.iter().rposition(|&b| b == b'/') {
        Some(i) => {
            let mut __j = i;
            while __j > 0 && __c[__j - 1] == b'/' {
                __j -= 1;
            }
            if __j == 0 {
                vec![b'/']
            } else {
                __c[..__j].to_vec()
            }
        }
        None => Vec::new(),
    };
    __o.push(0);
    __o
}
