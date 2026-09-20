// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1() -> Vec<libc::c_char> {
    Vec::new()
}

fn t2() -> *mut libc::c_char {
    ::std::ptr::null_mut()
}

unsafe // `count` defaults to std::string::npos == usize::MAX, which the converter
// now folds to its real value, so a plain `a1 + a2` is a compile-time
// overflow that rustc rejects outright.  Saturate instead.
fn f1(a0: Vec<libc::c_char>, a1: usize, a2: usize) -> Vec<libc::c_char> {
    let mut __tmp1 =
        a0[(a1) as usize..::std::cmp::min(a1.saturating_add(a2), a0.len() - 1)].to_vec();
    __tmp1.push(0);
    __tmp1
}
unsafe fn f2(a0: Vec<libc::c_char>) -> usize {
    (a0.len() - 1)
}
unsafe fn f3(a0: Vec<libc::c_char>, a1: *const libc::c_char) -> Vec<libc::c_char> {
    let mut __tmp2 = a0.clone();
    __tmp2.pop();
    let __from = a1;
    __tmp2.extend_from_slice(::std::slice::from_raw_parts(
        __from,
        (0..).position(|i| *__from.add(i) == 0).unwrap(),
    ));
    __tmp2.push(0);
    __tmp2
}
unsafe fn f4(a0: &mut Vec<libc::c_char>, a1: *mut libc::c_char, a2: usize) {
    a0.splice(a0.len().saturating_sub(1)..a0.len(), {
        let mut v = ::std::slice::from_raw_parts(a1, a2 as usize).to_vec();
        v.push(0);
        v
    });
}
unsafe fn f5(a0: Vec<libc::c_char>) -> *const libc::c_char {
    a0.as_ptr()
}
unsafe fn f6(a0: &mut Vec<libc::c_char>) -> *const libc::c_char {
    a0.as_mut_ptr()
}
unsafe fn f7(a0: *const libc::c_char, a1: usize) -> Vec<libc::c_char> {
    std::slice::from_raw_parts(a0, a1 as usize)
        .to_vec()
        .iter()
        .copied()
        .chain(std::iter::once(0))
        .collect()
}
// TODO: this does not care for a1
use std::io::Read;

unsafe fn f8(a0: std::fs::File, a1: std::fs::File) -> Vec<libc::c_char> {
    let mut __bytes: Vec<u8> = Vec::new();
    let mut __f = &a0;
    __f.read_to_end(&mut __bytes)
        .expect("couldn't read the file");
    let mut __buf: Vec<libc::c_char> = __bytes.iter().map(|&b| b as libc::c_char).collect();
    __buf.push(0);
    __buf
}

unsafe fn f9(a0: usize, a1: libc::c_char) -> Vec<libc::c_char> {
    vec![a1; (a0) as usize]
        .iter()
        .cloned()
        .chain(std::iter::once(0))
        .collect()
}
unsafe fn f10(a0: *const libc::c_char) -> Vec<libc::c_char> {
    let s = a0;
    std::slice::from_raw_parts(s, (0..).take_while(|&i| *s.add(i) != 0).count() + 1).to_vec()
}
unsafe fn f11(a0: Vec<libc::c_char>) -> *const libc::c_char {
    a0.as_ptr()
}
unsafe fn f12(a0: &mut Vec<libc::c_char>) -> *mut libc::c_char {
    a0.as_mut_ptr()
}
unsafe fn f13(a0: &mut Vec<libc::c_char>, a1: usize) {
    a0.pop();
    a0.resize((a1) as usize, 0);
    a0.push(0)
}
unsafe fn f14(
    a0: &mut Vec<libc::c_char>,
    a1: usize,
    a2: usize,
    a3: *const libc::c_char,
    a4: usize,
) {
    a0.splice(
        a1 as usize..a1 as usize + a2 as usize,
        ::std::slice::from_raw_parts(a3, a4 as usize).to_vec(),
    );
}
unsafe fn f15(a0: &mut Vec<libc::c_char>) -> *mut libc::c_char {
    a0.as_mut_ptr().add(a0.len() - 1)
}
unsafe fn f16(a0: Vec<libc::c_char>, a1: *const libc::c_char) -> usize {
    match a0.iter().rposition(|&c| {
        ::std::ffi::CStr::from_ptr(a1)
            .to_str()
            .unwrap()
            .contains(c as u8 as char)
    }) {
        Some(idx) => idx,
        None => usize::MAX,
    }
}
unsafe fn f17(a0: Vec<libc::c_char>, a1: *const libc::c_char) -> Vec<libc::c_char> {
    let mut __tmp2 = a0.clone();
    __tmp2.pop();
    let __from = a1;
    __tmp2.extend_from_slice(::std::slice::from_raw_parts(
        __from,
        (0..).position(|i| *__from.add(i) == 0).unwrap(),
    ));
    __tmp2.push(0);
    __tmp2
}
unsafe fn f18(a0: Vec<libc::c_char>, a1: *const libc::c_char) -> bool {
    a0 == {
        let s = a1;
        std::slice::from_raw_parts(s, (0..).take_while(|&i| *s.add(i) != 0).count() + 1).to_vec()
    }
}
unsafe fn f19(a0: Vec<libc::c_char>) -> usize {
    (a0.len() - 1)
}
unsafe fn f20(a0: *mut libc::c_char, a1: usize) -> *mut libc::c_char {
    a0.add(a1 as usize)
}
unsafe fn f21(a0: &mut Vec<libc::c_char>, a1: usize, a2: libc::c_char) {
    a0.splice(
        a0.len() - 1..a0.len() - 1,
        ::std::vec::from_elem(a2, a1 as usize),
    );
}

unsafe fn f22(a0: Vec<libc::c_char>) -> bool {
    a0.len() <= 1
}

unsafe fn f23() -> Vec<libc::c_char> {
    vec![0]
}

unsafe fn f24(a0: &mut Vec<libc::c_char>) {
    a0.clear();
    a0.push(0)
}

unsafe fn f25(a0: &mut Vec<libc::c_char>) {
    a0.shrink_to_fit()
}

unsafe fn f26(a0: &mut Vec<libc::c_char>, a1: usize) -> *mut libc::c_char {
    if a1 as usize >= a0.len() - 1 {
        panic!("out of bounds access")
    } else {
        &mut a0[a1 as usize]
    }
}

unsafe fn f27(a0: Vec<libc::c_char>) -> Vec<libc::c_char> {
    a0.clone()
}

unsafe fn f28(a0: &mut Vec<libc::c_char>) -> Vec<libc::c_char> {
    std::mem::take(&mut *a0)
}

unsafe fn f29(a0: &mut Vec<libc::c_char>, a1: Vec<libc::c_char>) {
    *a0 = a1.clone()
}

unsafe fn f30(a0: &mut Vec<libc::c_char>, a1: &mut Vec<libc::c_char>) {
    *a0 = std::mem::take(&mut *a1)
}

unsafe fn f31(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let mut r = a0;
    r.pop();
    r.extend(a1.iter().copied().take_while(|&c| c != 0));
    r.push(0);
    r
}

unsafe fn f32(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let mut r = a0;
    r.pop();
    r.extend(a1.iter().copied().take_while(|&c| c != 0));
    r.push(0);
    r
}

unsafe fn f33(a0: Vec<libc::c_char>, a1: libc::c_char) -> Vec<libc::c_char> {
    let mut r = a0;
    r.pop();
    r.push(a1);
    r.push(0);
    r
}

unsafe fn f34(a0: Vec<libc::c_char>, a1: libc::c_char) -> Vec<libc::c_char> {
    let mut r = a0;
    r.pop();
    r.push(a1);
    r.push(0);
    r
}

// operator+= returns the RECEIVER in both models -- see the long note in
// tgt_refcount.rs.  The reference in/reference out is spelled as the POINTER,
// `*mut Vec<libc::c_char>`, not `&mut`, and the receiver placeholder is bound
// once before being used twice.
unsafe fn f35(a0: *mut Vec<libc::c_char>, a1: Vec<libc::c_char>) -> *mut Vec<libc::c_char> {
    let __o = a0;
    (*__o).pop();
    (*__o).extend(a1.iter().copied().take_while(|&c| c != 0));
    (*__o).push(0);
    __o
}

unsafe fn f36(a0: *mut Vec<libc::c_char>, a1: *const libc::c_char) -> *mut Vec<libc::c_char> {
    let __o = a0;
    (*__o).pop();
    let __from = a1;
    (*__o).extend_from_slice(::std::slice::from_raw_parts(
        __from,
        (0..).position(|i| *__from.add(i) == 0).unwrap(),
    ));
    (*__o).push(0);
    __o
}

unsafe fn f37(a0: *mut Vec<libc::c_char>, a1: libc::c_char) -> *mut Vec<libc::c_char> {
    let __o = a0;
    (*__o).pop();
    (*__o).push(a1);
    (*__o).push(0);
    __o
}

unsafe fn f38(a0: *const libc::c_char, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let __from = a0;
    let mut r = ::std::slice::from_raw_parts(
        __from,
        (0..).position(|i| *__from.add(i) == 0).unwrap(),
    )
    .to_vec();
    r.extend(a1.iter().copied().take_while(|&c| c != 0));
    r.push(0);
    r
}

unsafe fn f39(a0: *const libc::c_char, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let __from = a0;
    let mut r = ::std::slice::from_raw_parts(
        __from,
        (0..).position(|i| *__from.add(i) == 0).unwrap(),
    )
    .to_vec();
    r.extend(a1.iter().copied().take_while(|&c| c != 0));
    r.push(0);
    r
}

unsafe fn f40(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let mut r = a0;
    r.pop();
    r.extend(a1.iter().copied().take_while(|&c| c != 0));
    r.push(0);
    r
}

unsafe fn f41(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let mut r = a0;
    r.pop();
    r.extend(a1.iter().copied().take_while(|&c| c != 0));
    r.push(0);
    r
}

unsafe fn f42(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> bool {
    a0 == a1
}

unsafe fn f43(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> bool {
    a0 != a1
}

unsafe fn f44(a0: Vec<libc::c_char>, a1: *const libc::c_char) -> bool {
    let __from = a1;
    let __b = ::std::slice::from_raw_parts(
        __from,
        (0..).position(|i| *__from.add(i) == 0).unwrap(),
    );
    !a0.iter().copied().take_while(|&c| c != 0).eq(__b.iter().copied())
}

unsafe fn f45(a0: *const libc::c_char, a1: Vec<libc::c_char>) -> bool {
    let __from = a0;
    let __a = ::std::slice::from_raw_parts(
        __from,
        (0..).position(|i| *__from.add(i) == 0).unwrap(),
    );
    __a.iter().copied().eq(a1.iter().copied().take_while(|&c| c != 0))
}

unsafe fn f46(a0: *const libc::c_char, a1: Vec<libc::c_char>) -> bool {
    let __from = a0;
    let __a = ::std::slice::from_raw_parts(
        __from,
        (0..).position(|i| *__from.add(i) == 0).unwrap(),
    );
    !__a.iter().copied().eq(a1.iter().copied().take_while(|&c| c != 0))
}

// A placeholder is substituted as raw text, so a method call on one has to go
// through a binding: `-7i64.to_string()` parses as `-(7i64.to_string())`.

unsafe fn f47(a0: i32) -> Vec<libc::c_char> {
    let __value = a0;
    __value
        .to_string()
        .into_bytes()
        .into_iter()
        .map(|__b| __b as libc::c_char)
        .chain(::std::iter::once(0))
        .collect::<Vec<libc::c_char>>()
}

unsafe fn f48(a0: i64) -> Vec<libc::c_char> {
    let __value = a0;
    __value
        .to_string()
        .into_bytes()
        .into_iter()
        .map(|__b| __b as libc::c_char)
        .chain(::std::iter::once(0))
        .collect::<Vec<libc::c_char>>()
}

unsafe fn f49(a0: i64) -> Vec<libc::c_char> {
    let __value = a0;
    __value
        .to_string()
        .into_bytes()
        .into_iter()
        .map(|__b| __b as libc::c_char)
        .chain(::std::iter::once(0))
        .collect::<Vec<libc::c_char>>()
}

unsafe fn f50(a0: u32) -> Vec<libc::c_char> {
    let __value = a0;
    __value
        .to_string()
        .into_bytes()
        .into_iter()
        .map(|__b| __b as libc::c_char)
        .chain(::std::iter::once(0))
        .collect::<Vec<libc::c_char>>()
}

unsafe fn f51(a0: u64) -> Vec<libc::c_char> {
    let __value = a0;
    __value
        .to_string()
        .into_bytes()
        .into_iter()
        .map(|__b| __b as libc::c_char)
        .chain(::std::iter::once(0))
        .collect::<Vec<libc::c_char>>()
}

unsafe fn f52(a0: u64) -> Vec<libc::c_char> {
    let __value = a0;
    __value
        .to_string()
        .into_bytes()
        .into_iter()
        .map(|__b| __b as libc::c_char)
        .chain(::std::iter::once(0))
        .collect::<Vec<libc::c_char>>()
}

unsafe fn f53(a0: f32) -> Vec<libc::c_char> {
    let __value = a0;
    format!("{:.6}", __value)
        .into_bytes()
        .into_iter()
        .map(|__b| __b as libc::c_char)
        .chain(::std::iter::once(0))
        .collect::<Vec<libc::c_char>>()
}

unsafe fn f54(a0: f64) -> Vec<libc::c_char> {
    let __value = a0;
    format!("{:.6}", __value)
        .into_bytes()
        .into_iter()
        .map(|__b| __b as libc::c_char)
        .chain(::std::iter::once(0))
        .collect::<Vec<libc::c_char>>()
}

unsafe fn f55(a0: Vec<libc::c_char>, a1: *mut usize, a2: i32) -> i32 {
    let __pos: *mut usize = a1;
    let __text: String = a0
        .iter()
        .copied()
        .take_while(|&__c| __c != 0)
        .map(|__c| __c as u8 as char)
        .collect();
    let __lead = __text.len() - __text.trim_start().len();
    let __body = &__text[__lead..];
    let __sign = usize::from(__body.starts_with('+') || __body.starts_with('-'));
    let __rest = &__body[__sign..];
    let __radix: u32 = if a2 != 0 {
        a2 as u32
    } else if __rest.starts_with("0x") || __rest.starts_with("0X") {
        16
    } else if __rest.len() > 1 && __rest.starts_with('0') {
        8
    } else {
        10
    };
    let __skip =
        usize::from(__radix == 16 && (__rest.starts_with("0x") || __rest.starts_with("0X"))) * 2;
    let __n = __rest[__skip..]
        .chars()
        .take_while(|__c| __c.is_digit(__radix))
        .count();
    assert!(__n > 0, "stoi: no conversion");
    let __digits: String = [&__body[..__sign], &__rest[__skip..__skip + __n]].concat();
    let __value = i64::from_str_radix(&__digits, __radix).expect("stoi: out of range");
    if !__pos.is_null() {
        *__pos = __lead + __sign + __skip + __n;
    }
    __value as i32
}

unsafe fn f56(a0: Vec<libc::c_char>, a1: *mut usize, a2: i32) -> i64 {
    let __pos: *mut usize = a1;
    let __text: String = a0
        .iter()
        .copied()
        .take_while(|&__c| __c != 0)
        .map(|__c| __c as u8 as char)
        .collect();
    let __lead = __text.len() - __text.trim_start().len();
    let __body = &__text[__lead..];
    let __sign = usize::from(__body.starts_with('+') || __body.starts_with('-'));
    let __rest = &__body[__sign..];
    let __radix: u32 = if a2 != 0 {
        a2 as u32
    } else if __rest.starts_with("0x") || __rest.starts_with("0X") {
        16
    } else if __rest.len() > 1 && __rest.starts_with('0') {
        8
    } else {
        10
    };
    let __skip =
        usize::from(__radix == 16 && (__rest.starts_with("0x") || __rest.starts_with("0X"))) * 2;
    let __n = __rest[__skip..]
        .chars()
        .take_while(|__c| __c.is_digit(__radix))
        .count();
    assert!(__n > 0, "stoll: no conversion");
    let __digits: String = [&__body[..__sign], &__rest[__skip..__skip + __n]].concat();
    let __value = i64::from_str_radix(&__digits, __radix).expect("stoll: out of range");
    if !__pos.is_null() {
        *__pos = __lead + __sign + __skip + __n;
    }
    __value
}

unsafe fn f57(a0: Vec<libc::c_char>, a1: *mut usize) -> f64 {
    let __pos: *mut usize = a1;
    let __text: String = a0
        .iter()
        .copied()
        .take_while(|&__c| __c != 0)
        .map(|__c| __c as u8 as char)
        .collect();
    let __lead = __text.len() - __text.trim_start().len();
    let __body = &__text[__lead..];
    let __n = (1..=__body.len())
        .rev()
        .find(|&__i| __body[..__i].parse::<f64>().is_ok())
        .expect("stod: no conversion");
    if !__pos.is_null() {
        *__pos = __lead + __n;
    }
    __body[..__n].parse::<f64>().unwrap()
}
