// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn f4(a0: Vec<u8>) -> Box<Vec<u8>> {
    Box::new(a0.iter().copied().take_while(|&c| c != 0).collect())
}

fn f5(a0: Vec<u8>) -> Box<Vec<u8>> {
    Box::new(a0.iter().copied().take_while(|&c| c != 0).collect())
}

fn f6(a0: Vec<u8>) -> Box<Vec<u8>> {
    Box::new(a0.iter().copied().take_while(|&c| c != 0).collect())
}

fn f7(a0: Box<Vec<u8>>) -> Vec<u8> {
    let mut __s: Vec<u8> = a0.to_vec();
    __s.push(0);
    __s
}

fn f8(a0: Box<Vec<u8>>) -> Vec<u8> {
    let mut __s: Vec<u8> = a0.to_vec();
    __s.push(0);
    __s
}

fn f9(a0: Box<Vec<u8>>) -> Vec<u8> {
    let mut __s: Vec<u8> = a0.to_vec();
    __s.push(0);
    __s
}

fn f10(a0: Ptr<Box<Vec<u8>>>, a1: Vec<u8>) {
    a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        __v.clear();
        __v.extend(a1.iter().copied().take_while(|&c| c != 0));
    });
}

fn f11(a0: Ptr<Box<Vec<u8>>>, a1: Vec<u8>) {
    a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        __v.clear();
        __v.extend(a1.iter().copied().take_while(|&c| c != 0));
    });
}

fn f12(a0: Ptr<Box<Vec<u8>>>, a1: Vec<u8>) {
    a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        __v.clear();
        __v.extend(a1.iter().copied().take_while(|&c| c != 0));
    });
}

fn f19(a0: Ptr<Box<Vec<u8>>>, a1: Ptr<i32>) -> Ptr<Box<Vec<u8>>> {
    let __tok: String = a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        let mut __i = 0usize;
        while __i < __v.len() && __v[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < __v.len() && (__v[__i] == b'-' || __v[__i] == b'+') {
            __i += 1;
        }
        while __i < __v.len() && (__v[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&__v[__b..__i]).into_owned();
        __v.drain(..__i);
        __t
    });
    a1.write(__tok.parse::<i32>().unwrap_or(Default::default()));
    let __r: Ptr<Box<Vec<u8>>> = a0;
    __r
}

fn f20(a0: Ptr<Box<Vec<u8>>>, a1: Ptr<u32>) -> Ptr<Box<Vec<u8>>> {
    let __tok: String = a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        let mut __i = 0usize;
        while __i < __v.len() && __v[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < __v.len() && (__v[__i] == b'-' || __v[__i] == b'+') {
            __i += 1;
        }
        while __i < __v.len() && (__v[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&__v[__b..__i]).into_owned();
        __v.drain(..__i);
        __t
    });
    a1.write(__tok.parse::<u32>().unwrap_or(Default::default()));
    let __r: Ptr<Box<Vec<u8>>> = a0;
    __r
}

fn f21(a0: Ptr<Box<Vec<u8>>>, a1: Ptr<i64>) -> Ptr<Box<Vec<u8>>> {
    let __tok: String = a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        let mut __i = 0usize;
        while __i < __v.len() && __v[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < __v.len() && (__v[__i] == b'-' || __v[__i] == b'+') {
            __i += 1;
        }
        while __i < __v.len() && (__v[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&__v[__b..__i]).into_owned();
        __v.drain(..__i);
        __t
    });
    a1.write(__tok.parse::<i64>().unwrap_or(Default::default()));
    let __r: Ptr<Box<Vec<u8>>> = a0;
    __r
}

fn f22(a0: Ptr<Box<Vec<u8>>>, a1: Ptr<u64>) -> Ptr<Box<Vec<u8>>> {
    let __tok: String = a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        let mut __i = 0usize;
        while __i < __v.len() && __v[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < __v.len() && (__v[__i] == b'-' || __v[__i] == b'+') {
            __i += 1;
        }
        while __i < __v.len() && (__v[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&__v[__b..__i]).into_owned();
        __v.drain(..__i);
        __t
    });
    a1.write(__tok.parse::<u64>().unwrap_or(Default::default()));
    let __r: Ptr<Box<Vec<u8>>> = a0;
    __r
}

fn f23(a0: Ptr<Box<Vec<u8>>>, a1: Ptr<i64>) -> Ptr<Box<Vec<u8>>> {
    let __tok: String = a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        let mut __i = 0usize;
        while __i < __v.len() && __v[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < __v.len() && (__v[__i] == b'-' || __v[__i] == b'+') {
            __i += 1;
        }
        while __i < __v.len() && (__v[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&__v[__b..__i]).into_owned();
        __v.drain(..__i);
        __t
    });
    a1.write(__tok.parse::<i64>().unwrap_or(Default::default()));
    let __r: Ptr<Box<Vec<u8>>> = a0;
    __r
}

fn f24(a0: Ptr<Box<Vec<u8>>>, a1: Ptr<u64>) -> Ptr<Box<Vec<u8>>> {
    let __tok: String = a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        let mut __i = 0usize;
        while __i < __v.len() && __v[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < __v.len() && (__v[__i] == b'-' || __v[__i] == b'+') {
            __i += 1;
        }
        while __i < __v.len() && (__v[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&__v[__b..__i]).into_owned();
        __v.drain(..__i);
        __t
    });
    a1.write(__tok.parse::<u64>().unwrap_or(Default::default()));
    let __r: Ptr<Box<Vec<u8>>> = a0;
    __r
}

fn f25(a0: Ptr<Box<Vec<u8>>>, a1: Ptr<i16>) -> Ptr<Box<Vec<u8>>> {
    let __tok: String = a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        let mut __i = 0usize;
        while __i < __v.len() && __v[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < __v.len() && (__v[__i] == b'-' || __v[__i] == b'+') {
            __i += 1;
        }
        while __i < __v.len() && (__v[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&__v[__b..__i]).into_owned();
        __v.drain(..__i);
        __t
    });
    a1.write(__tok.parse::<i16>().unwrap_or(Default::default()));
    let __r: Ptr<Box<Vec<u8>>> = a0;
    __r
}

fn f26(a0: Ptr<Box<Vec<u8>>>, a1: Ptr<u16>) -> Ptr<Box<Vec<u8>>> {
    let __tok: String = a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        let mut __i = 0usize;
        while __i < __v.len() && __v[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < __v.len() && (__v[__i] == b'-' || __v[__i] == b'+') {
            __i += 1;
        }
        while __i < __v.len() && (__v[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&__v[__b..__i]).into_owned();
        __v.drain(..__i);
        __t
    });
    a1.write(__tok.parse::<u16>().unwrap_or(Default::default()));
    let __r: Ptr<Box<Vec<u8>>> = a0;
    __r
}

fn f27(a0: Ptr<Box<Vec<u8>>>, a1: Ptr<f32>) -> Ptr<Box<Vec<u8>>> {
    let __tok: String = a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        let mut __i = 0usize;
        while __i < __v.len() && __v[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < __v.len() && (__v[__i] == b'-' || __v[__i] == b'+') {
            __i += 1;
        }
        while __i < __v.len() && (__v[__i].is_ascii_digit()
                || __v[__i] == b'.'
                || __v[__i] == b'e'
                || __v[__i] == b'E'
                || ((__v[__i] == b'-' || __v[__i] == b'+')
                    && (__v[__i - 1] == b'e' || __v[__i - 1] == b'E'))) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&__v[__b..__i]).into_owned();
        __v.drain(..__i);
        __t
    });
    a1.write(__tok.parse::<f32>().unwrap_or(Default::default()));
    let __r: Ptr<Box<Vec<u8>>> = a0;
    __r
}

fn f28(a0: Ptr<Box<Vec<u8>>>, a1: Ptr<f64>) -> Ptr<Box<Vec<u8>>> {
    let __tok: String = a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        let mut __i = 0usize;
        while __i < __v.len() && __v[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < __v.len() && (__v[__i] == b'-' || __v[__i] == b'+') {
            __i += 1;
        }
        while __i < __v.len() && (__v[__i].is_ascii_digit()
                || __v[__i] == b'.'
                || __v[__i] == b'e'
                || __v[__i] == b'E'
                || ((__v[__i] == b'-' || __v[__i] == b'+')
                    && (__v[__i - 1] == b'e' || __v[__i - 1] == b'E'))) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&__v[__b..__i]).into_owned();
        __v.drain(..__i);
        __t
    });
    a1.write(__tok.parse::<f64>().unwrap_or(Default::default()));
    let __r: Ptr<Box<Vec<u8>>> = a0;
    __r
}

fn f29(a0: Ptr<Box<Vec<u8>>>, a1: Ptr<Vec<u8>>) -> Ptr<Box<Vec<u8>>> {
    let __tok: Vec<u8> = a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        let mut __i = 0usize;
        while __i < __v.len() && __v[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        while __i < __v.len() && !__v[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __t = __v[__b..__i].to_vec();
        __v.drain(..__i);
        __t
    });
    let mut __out: Vec<u8> = __tok;
    __out.push(0);
    a1.write(__out);
    let __r: Ptr<Box<Vec<u8>>> = a0;
    __r
}

fn f30(a0: Ptr<Box<Vec<u8>>>, a1: Ptr<Vec<u8>>, a2: u8) -> Ptr<Box<Vec<u8>>> {
    let __line: Vec<u8> = a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        let __pos = __v.iter().position(|&c| c == a2);
        let __keep = __pos.unwrap_or(__v.len());
        let __end = __pos.map(|__p| __p + 1).unwrap_or(__v.len());
        __v.drain(..__end).take(__keep).collect()
    });
    let mut __out: Vec<u8> = __line;
    __out.push(0);
    a1.write(__out);
    let __r: Ptr<Box<Vec<u8>>> = a0;
    __r
}

fn f31(a0: Ptr<Box<Vec<u8>>>, a1: Ptr<Vec<u8>>) -> Ptr<Box<Vec<u8>>> {
    let __line: Vec<u8> = a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        let __pos = __v.iter().position(|&c| c == b'\n');
        let __keep = __pos.unwrap_or(__v.len());
        let __end = __pos.map(|__p| __p + 1).unwrap_or(__v.len());
        __v.drain(..__end).take(__keep).collect()
    });
    let mut __out: Vec<u8> = __line;
    __out.push(0);
    a1.write(__out);
    let __r: Ptr<Box<Vec<u8>>> = a0;
    __r
}

fn f32(a0: Ptr<Box<Vec<u8>>>, a1: Ptr<u8>) -> Ptr<Box<Vec<u8>>> {
    let __c: u8 = a0.with_mut(|__v: &mut Box<Vec<u8>>| {
        let mut __i = 0usize;
        while __i < __v.len() && __v[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __ch = if __i < __v.len() { __v[__i] } else { 0 };
        let __end = ::std::cmp::min(__i + 1, __v.len());
        __v.drain(..__end);
        __ch
    });
    a1.write(__c);
    let __r: Ptr<Box<Vec<u8>>> = a0;
    __r
}
