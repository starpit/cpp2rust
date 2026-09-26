// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn t1() -> Vec<u8> {
    Vec::new()
}

fn t2() -> Ptr<u8> {
    Ptr::null()
}

// `count` defaults to std::string::npos == usize::MAX, which the converter
// now folds to its real value, so a plain `a1 + a2` is a compile-time
// overflow that rustc rejects outright.  Saturate instead.
fn f1(a0: Vec<u8>, a1: usize, a2: usize) -> Vec<u8> {
    let mut __tmp1 = a0
        [(a1) as usize..::std::cmp::min(a1.saturating_add(a2), a0.len().saturating_sub(1))]
        .to_vec();
    __tmp1.push(0);
    __tmp1
}

fn f3(a0: Vec<u8>, a1: Ptr<u8>) -> Vec<u8> {
    let mut r = a0;
    r.pop();
    a1.with_c_str(|__s| r.extend_from_slice(__s));
    r.push(0);
    r
}

fn f4(a0: Ptr<Vec<u8>>, a1: Ptr<u8>, a2: usize) -> Ptr<Vec<u8>> {
    a0.with_mut(|__v: &mut Vec<u8>| {
        __v.pop();
        __v.extend(a1.map(|c| c.read()).take((a2) as usize));
        __v.push(0);
    });
    a0
}

fn f5(a0: Ptr<u8>) -> Ptr<u8> {
    a0
}

fn f6(a0: Ptr<u8>) -> Ptr<u8> {
    a0
}

fn f7(a0: Ptr<u8>, a1: usize) -> Vec<u8> {
    a0.map(|c| c.read())
        .take(a1 as usize)
        .chain(std::iter::once(0))
        .collect::<Vec<u8>>()
}

fn f10(a0: Ptr<u8>) -> Vec<u8> {
    let mut __bytes = a0.to_c_bytes();
    __bytes.push(0);
    __bytes
}

fn f11(a0: Ptr<u8>) -> Ptr<u8> {
    a0
}

fn f12(a0: Ptr<u8>) -> Ptr<u8> {
    a0
}

fn f14(a0: Ptr<Vec<u8>>, a1: usize, a2: usize, a3: Ptr<u8>, a4: usize) -> Ptr<Vec<u8>> {
    let pos = a1 as usize;
    let end = std::cmp::min(
        pos + a2 as usize,
        (*a0.upgrade().deref()).len().saturating_sub(1),
    );
    a0.with_mut(|__v: &mut Vec<u8>| {
        __v.splice(pos..end, a3.map(|c| c.read()).take((a4) as usize));
    });
    a0
}

fn f15(a0: Ptr<u8>) -> Ptr<u8> {
    a0.to_last()
}

fn f16(a0: Vec<u8>, a1: Ptr<u8>) -> usize {
    a1.with_c_str(|__lookup| {
        a0.iter()
            .take(a0.len().saturating_sub(1))
            .rposition(|&x| __lookup.contains(&x))
            .unwrap_or(usize::MAX)
    })
}

// TODO: This should modify a0 in place
fn f17(a0: Vec<u8>, a1: Ptr<u8>) -> Vec<u8> {
    let mut __tmp2 = a0;
    __tmp2.pop();
    a1.with_c_str(|__s| __tmp2.extend_from_slice(__s));
    __tmp2.push(0);
    __tmp2
}

fn f18(a0: Vec<u8>, a1: Ptr<u8>) -> bool {
    a0.iter()
        .copied()
        .take(a0.len().saturating_sub(1))
        .eq(a1.to_c_string_iterator())
}

fn f20(a0: Ptr<u8>, a1: usize) -> Ptr<u8> {
    a0.offset(a1 as isize)
}

// TODO: This should return a0
fn f21(a0: &mut Vec<u8>, a1: usize, a2: u8) -> Vec<u8> {
    a0.pop();
    a0.resize(a0.len() + (a1) as usize, a2);
    a0.push(0);
    a0.clone()
}

fn f26(a0: Ptr<Vec<u8>>, a1: usize) -> Ptr<u8> {
    if a1 as usize >= (*a0.upgrade().deref()).len().saturating_sub(1) {
        panic!("out of bounds access")
    } else {
        a0.decay().offset(a1 as isize)
    }
}

fn f2(a0: Vec<u8>) -> usize {
    (a0.len() - 1)
}

fn f8(a0: std::fs::File, a1: std::fs::File) -> Vec<u8> {
    use std::io::Read;
    let mut __bytes: Vec<u8> = Vec::new();
    let mut __f = &a0;
    __f.read_to_end(&mut __bytes)
        .expect("couldn't read the file");
    __bytes.push(0);
    __bytes
}

fn f9(a0: usize, a1: u8) -> Vec<u8> {
    vec![a1; (a0) as usize]
        .iter()
        .cloned()
        .chain(std::iter::once(0))
        .collect()
}

fn f13(a0: &mut Vec<u8>, a1: usize) {
    a0.pop();
    a0.resize((a1) as usize, 0);
    a0.push(0)
}

fn f19(a0: Vec<u8>) -> usize {
    (a0.len() - 1)
}

fn f22(a0: Vec<u8>) -> bool {
    a0.len() <= 1
}

fn f23() -> Vec<u8> {
    vec![0]
}

fn f24(a0: &mut Vec<u8>) {
    a0.clear();
    a0.push(0)
}

fn f25(a0: &mut Vec<u8>) {
    a0.shrink_to_fit()
}

fn f27(a0: Vec<u8>) -> Vec<u8> {
    a0
}

fn f28(a0: &mut Vec<u8>) -> Vec<u8> {
    std::mem::take(&mut *a0)
}

fn f29(a0: Ptr<Vec<u8>>, a1: Vec<u8>) {
    a0.write(a1)
}

fn f30(a0: Ptr<Vec<u8>>, a1: &mut Vec<u8>) {
    a0.write(std::mem::take(&mut *a1))
}

// operator+(const std::string&, const std::string&)
fn f31(a0: Vec<u8>, a1: Vec<u8>) -> Vec<u8> {
    let mut r = a0;
    r.pop();
    r.extend(a1.iter().copied().take_while(|&c| c != 0));
    r.push(0);
    r
}

// operator+(std::string&&, std::string&&)
fn f32(a0: Vec<u8>, a1: Vec<u8>) -> Vec<u8> {
    let mut r = a0;
    r.pop();
    r.extend(a1.iter().copied().take_while(|&c| c != 0));
    r.push(0);
    r
}

// operator+(std::string&&, char)
fn f33(a0: Vec<u8>, a1: u8) -> Vec<u8> {
    let mut r = a0;
    r.pop();
    r.push(a1);
    r.push(0);
    r
}

// operator+(const std::string&, char)
fn f34(a0: Vec<u8>, a1: u8) -> Vec<u8> {
    let mut r = a0;
    r.pop();
    r.push(a1);
    r.push(0);
    r
}

// std::string::operator+=  --  all three overloads RETURN THE RECEIVER.
//
// C++ declares `std::string &operator+=(...)`, and dropping that reference is
// not free: `a += b += c` is legal C++ (operator+= is a member, so it binds to
// the rvalue `b`) and the target codebase uses it -- dsm/dsmds.h:529, :543 and
// :558 all read `jsonStr += indent(nestedIndentLevel) += "},\n";`.  The
// converter emits `(* <inner> .upgrade().deref())` around the inner call
// because the signature says it yields a reference, so a unit-returning body
// produced `(* <with_mut block> ; .upgrade().deref())` -- not Rust.  rustfmt
// rejected the whole crate and three progtailor TUs died there
// ("ERROR: failed to run rustfmt"), long after the converter thought it was
// done.  Returning the receiver is the same shape rules/raw_ostream uses for
// operator<< and for the same reason.
//
// The receiver placeholder is bound ONCE (`let __o = a0;`) before it is used
// twice: a placeholder is raw text, so naming it twice would re-evaluate --
// and so re-run the side effects of -- the receiver expression.
//
// In statement position the returned Ptr is simply dropped, which is what
// `s += x;` means.

fn f35(a0: Ptr<Vec<u8>>, a1: Vec<u8>) -> Ptr<Vec<u8>> {
    let __o = a0;
    __o.with_mut(|__v: &mut Vec<u8>| {
        __v.pop();
        __v.extend(a1.iter().copied().take_while(|&c| c != 0));
        __v.push(0);
    });
    __o
}

fn f36(a0: Ptr<Vec<u8>>, a1: Ptr<u8>) -> Ptr<Vec<u8>> {
    let __o = a0;
    __o.with_mut(|__v: &mut Vec<u8>| {
        __v.pop();
        __v.extend(a1.to_c_string_iterator());
        __v.push(0);
    });
    __o
}

fn f37(a0: Ptr<Vec<u8>>, a1: u8) -> Ptr<Vec<u8>> {
    let __o = a0;
    __o.with_mut(|__v: &mut Vec<u8>| {
        __v.pop();
        __v.push(a1);
        __v.push(0);
    });
    __o
}

// operator+(const char*, const std::string&)
fn f38(a0: Ptr<u8>, a1: Vec<u8>) -> Vec<u8> {
    let mut r: Vec<u8> = a0.to_c_string_iterator().collect();
    r.extend(a1.iter().copied().take_while(|&c| c != 0));
    r.push(0);
    r
}

// operator+(const char*, std::string&&)
fn f39(a0: Ptr<u8>, a1: Vec<u8>) -> Vec<u8> {
    let mut r: Vec<u8> = a0.to_c_string_iterator().collect();
    r.extend(a1.iter().copied().take_while(|&c| c != 0));
    r.push(0);
    r
}

// operator+(std::string&&, const std::string&)
fn f40(a0: Vec<u8>, a1: Vec<u8>) -> Vec<u8> {
    let mut r = a0;
    r.pop();
    r.extend(a1.iter().copied().take_while(|&c| c != 0));
    r.push(0);
    r
}

// operator+(const std::string&, std::string&&)
fn f41(a0: Vec<u8>, a1: Vec<u8>) -> Vec<u8> {
    let mut r = a0;
    r.pop();
    r.extend(a1.iter().copied().take_while(|&c| c != 0));
    r.push(0);
    r
}

fn f42(a0: Vec<u8>, a1: Vec<u8>) -> bool {
    a0 == a1
}

fn f43(a0: Vec<u8>, a1: Vec<u8>) -> bool {
    a0 != a1
}

fn f44(a0: Vec<u8>, a1: Ptr<u8>) -> bool {
    !a0.iter().copied().take_while(|&c| c != 0).eq(a1.to_c_string_iterator())
}

fn f45(a0: Ptr<u8>, a1: Vec<u8>) -> bool {
    a0.to_c_string_iterator().eq(a1.iter().copied().take_while(|&c| c != 0))
}

fn f46(a0: Ptr<u8>, a1: Vec<u8>) -> bool {
    !a0.to_c_string_iterator().eq(a1.iter().copied().take_while(|&c| c != 0))
}

// A placeholder is substituted as raw text, so a method call on one has to go
// through a binding: `-7i64.to_string()` parses as `-(7i64.to_string())`.

fn f47(a0: i32) -> Vec<u8> {
    let __value = a0;
    __value
        .to_string()
        .into_bytes()
        .into_iter()
        .chain(::std::iter::once(0))
        .collect::<Vec<u8>>()
}

fn f48(a0: i64) -> Vec<u8> {
    let __value = a0;
    __value
        .to_string()
        .into_bytes()
        .into_iter()
        .chain(::std::iter::once(0))
        .collect::<Vec<u8>>()
}

fn f49(a0: i64) -> Vec<u8> {
    let __value = a0;
    __value
        .to_string()
        .into_bytes()
        .into_iter()
        .chain(::std::iter::once(0))
        .collect::<Vec<u8>>()
}

fn f50(a0: u32) -> Vec<u8> {
    let __value = a0;
    __value
        .to_string()
        .into_bytes()
        .into_iter()
        .chain(::std::iter::once(0))
        .collect::<Vec<u8>>()
}

fn f51(a0: u64) -> Vec<u8> {
    let __value = a0;
    __value
        .to_string()
        .into_bytes()
        .into_iter()
        .chain(::std::iter::once(0))
        .collect::<Vec<u8>>()
}

fn f52(a0: u64) -> Vec<u8> {
    let __value = a0;
    __value
        .to_string()
        .into_bytes()
        .into_iter()
        .chain(::std::iter::once(0))
        .collect::<Vec<u8>>()
}

fn f53(a0: f32) -> Vec<u8> {
    let __value = a0;
    format!("{:.6}", __value)
        .into_bytes()
        .into_iter()
        .chain(::std::iter::once(0))
        .collect::<Vec<u8>>()
}

fn f54(a0: f64) -> Vec<u8> {
    let __value = a0;
    format!("{:.6}", __value)
        .into_bytes()
        .into_iter()
        .chain(::std::iter::once(0))
        .collect::<Vec<u8>>()
}

fn f55(a0: Vec<u8>, a1: Ptr<usize>, a2: i32) -> i32 {
    let __pos: Ptr<usize> = a1;
    let __text: String = a0
        .iter()
        .copied()
        .take_while(|&__c| __c != 0)
        .map(|__c| __c as char)
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
        __pos.write(__lead + __sign + __skip + __n);
    }
    __value as i32
}

fn f56(a0: Vec<u8>, a1: Ptr<usize>, a2: i32) -> i64 {
    let __pos: Ptr<usize> = a1;
    let __text: String = a0
        .iter()
        .copied()
        .take_while(|&__c| __c != 0)
        .map(|__c| __c as char)
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
        __pos.write(__lead + __sign + __skip + __n);
    }
    __value
}

fn f57(a0: Vec<u8>, a1: Ptr<usize>) -> f64 {
    let __pos: Ptr<usize> = a1;
    let __text: String = a0
        .iter()
        .copied()
        .take_while(|&__c| __c != 0)
        .map(|__c| __c as char)
        .collect();
    let __lead = __text.len() - __text.trim_start().len();
    let __body = &__text[__lead..];
    let __n = (1..=__body.len())
        .rev()
        .find(|&__i| __body[..__i].parse::<f64>().is_ok())
        .expect("stod: no conversion");
    if !__pos.is_null() {
        __pos.write(__lead + __n);
    }
    __body[..__n].parse::<f64>().unwrap()
}

// operator+(char, const std::string &)
fn f58(a0: u8, a1: Vec<u8>) -> Vec<u8> {
    let mut r: Vec<u8> = ::std::vec![a0];
    r.extend(a1.iter().copied().take_while(|&c| c != 0));
    r.push(0);
    r
}

fn f59(a0: Ptr<Vec<u8>>, a1: Ptr<u8>) -> Ptr<Vec<u8>> {
    let __o = a0;
    __o.with_mut(|__v: &mut Vec<u8>| {
        __v.clear();
        __v.extend(a1.to_c_string_iterator());
        __v.push(0);
    });
    __o
}
