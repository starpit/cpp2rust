// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1() -> Vec<libc::c_char> {
    Vec::new()
}

fn t2() -> *mut libc::c_char {
    ::std::ptr::null_mut()
}

unsafe fn f1(a0: Vec<libc::c_char>, a1: usize, a2: usize) -> Vec<libc::c_char> {
    let mut __tmp1 =
        a0[(a1) as usize..::std::cmp::min((a1.saturating_add(a2)) as usize, a0.len() - 1)].to_vec();
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
    let __src = a1.clone();
    a0.clear();
    a0.extend(__src);
}

unsafe fn f30(a0: &mut Vec<libc::c_char>, a1: &mut Vec<libc::c_char>) {
    let __src = std::mem::take(&mut *a1);
    a0.clear();
    a0.extend(__src);
}

unsafe fn f31(a0: Vec<libc::c_char>, a1: libc::c_char) -> Vec<libc::c_char> {
    let mut __tmp = a0.clone();
    __tmp.pop();
    __tmp.push(a1);
    __tmp.push(0);
    __tmp
}

unsafe fn f32(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let mut __tmp = a0;
    __tmp.pop();
    __tmp.extend_from_slice(&a1[..a1.len() - 1]);
    __tmp.push(0);
    __tmp
}
unsafe fn f33(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let mut __tmp = a0;
    __tmp.pop();
    __tmp.extend_from_slice(&a1[..a1.len() - 1]);
    __tmp.push(0);
    __tmp
}
unsafe fn f34(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let mut __tmp = a0;
    __tmp.pop();
    __tmp.extend_from_slice(&a1[..a1.len() - 1]);
    __tmp.push(0);
    __tmp
}
unsafe fn f35(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let mut __tmp = a0;
    __tmp.pop();
    __tmp.extend_from_slice(&a1[..a1.len() - 1]);
    __tmp.push(0);
    __tmp
}
unsafe fn f36(a0: Vec<libc::c_char>, a1: libc::c_char) -> Vec<libc::c_char> {
    let mut __tmp = a0;
    __tmp.pop();
    __tmp.push(a1);
    __tmp.push(0);
    __tmp
}
unsafe fn f37(a0: *const libc::c_char, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let __from = a0;
    let mut __tmp = ::std::slice::from_raw_parts(
        __from,
        (0..).position(|i| *__from.add(i) == 0).unwrap(),
    )
    .to_vec();
    __tmp.extend_from_slice(&a1[..a1.len() - 1]);
    __tmp.push(0);
    __tmp
}

unsafe fn f38(a0: &mut Vec<libc::c_char>, a1: Vec<libc::c_char>) {
    a0.pop();
    a0.extend_from_slice(&a1[..a1.len() - 1]);
    a0.push(0);
}
unsafe fn f39(a0: &mut Vec<libc::c_char>, a1: libc::c_char) {
    a0.pop();
    a0.push(a1);
    a0.push(0);
}
unsafe fn f40(a0: &mut Vec<libc::c_char>, a1: *const libc::c_char) {
    a0.pop();
    let __from = a1;
    a0.extend_from_slice(::std::slice::from_raw_parts(
        __from,
        (0..).position(|i| *__from.add(i) == 0).unwrap(),
    ));
    a0.push(0);
}

unsafe fn f41(a0: *const libc::c_char, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let __from = a0;
    let mut __tmp = ::std::slice::from_raw_parts(
        __from,
        (0..).position(|i| *__from.add(i) == 0).unwrap(),
    )
    .to_vec();
    __tmp.extend_from_slice(&a1[..a1.len() - 1]);
    __tmp.push(0);
    __tmp
}
unsafe fn f42(a0: libc::c_char, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let mut __tmp = vec![a0];
    __tmp.extend_from_slice(&a1[..a1.len() - 1]);
    __tmp.push(0);
    __tmp
}
unsafe fn f43(a0: libc::c_char, a1: Vec<libc::c_char>) -> Vec<libc::c_char> {
    let mut __tmp = vec![a0];
    __tmp.extend_from_slice(&a1[..a1.len() - 1]);
    __tmp.push(0);
    __tmp
}

unsafe fn f44(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> bool {
    a0 == a1
}
unsafe fn f45(a0: *const libc::c_char, a1: Vec<libc::c_char>) -> bool {
    let s = a0;
    std::slice::from_raw_parts(s, (0..).take_while(|&i| *s.add(i) != 0).count() + 1).to_vec() == a1
}
unsafe fn f46(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> bool {
    a0 != a1
}
unsafe fn f47(a0: Vec<libc::c_char>, a1: *const libc::c_char) -> bool {
    let s = a1;
    a0 != std::slice::from_raw_parts(s, (0..).take_while(|&i| *s.add(i) != 0).count() + 1).to_vec()
}
unsafe fn f48(a0: *const libc::c_char, a1: Vec<libc::c_char>) -> bool {
    let s = a0;
    std::slice::from_raw_parts(s, (0..).take_while(|&i| *s.add(i) != 0).count() + 1).to_vec() != a1
}

// f49 -- std::string::npos == size_type(-1).  size_type is `usize` in this module
// (readback: `search type size_type, result: usize`), so this is usize::MAX.
// Model-independent: a plain integral constant, hence byte-identical in tgt_refcount.rs.
unsafe fn f49() -> usize {
    usize::MAX
}

// f50..f57 -- std::to_string. `Vec<libc::c_char>`, NUL-TERMINATED (see src.cpp: every
// other constructor in this module terminates, and f46..f48 compare with `len() - 1`).
// The `\0` is inside the format string so there is exactly one allocation and no chance
// of forgetting it on one arm.
//
// f56/f57 are `{:.6}` because C++ to_string for float/double is `%f`, i.e. always six
// decimals -- `{}` would print "1.5" where C++ prints "1.500000".
unsafe fn f50(a0: i32) -> Vec<libc::c_char> {
    format!("{}\0", a0)
        .into_bytes()
        .iter()
        .map(|&b| b as libc::c_char)
        .collect::<Vec<libc::c_char>>()
}
unsafe fn f51(a0: u32) -> Vec<libc::c_char> {
    format!("{}\0", a0)
        .into_bytes()
        .iter()
        .map(|&b| b as libc::c_char)
        .collect::<Vec<libc::c_char>>()
}
unsafe fn f52(a0: i64) -> Vec<libc::c_char> {
    format!("{}\0", a0)
        .into_bytes()
        .iter()
        .map(|&b| b as libc::c_char)
        .collect::<Vec<libc::c_char>>()
}
unsafe fn f53(a0: u64) -> Vec<libc::c_char> {
    format!("{}\0", a0)
        .into_bytes()
        .iter()
        .map(|&b| b as libc::c_char)
        .collect::<Vec<libc::c_char>>()
}
unsafe fn f54(a0: i64) -> Vec<libc::c_char> {
    format!("{}\0", a0)
        .into_bytes()
        .iter()
        .map(|&b| b as libc::c_char)
        .collect::<Vec<libc::c_char>>()
}
unsafe fn f55(a0: u64) -> Vec<libc::c_char> {
    format!("{}\0", a0)
        .into_bytes()
        .iter()
        .map(|&b| b as libc::c_char)
        .collect::<Vec<libc::c_char>>()
}
unsafe fn f56(a0: f32) -> Vec<libc::c_char> {
    format!("{:.6}\0", a0)
        .into_bytes()
        .iter()
        .map(|&b| b as libc::c_char)
        .collect::<Vec<libc::c_char>>()
}
unsafe fn f57(a0: f64) -> Vec<libc::c_char> {
    format!("{:.6}\0", a0)
        .into_bytes()
        .iter()
        .map(|&b| b as libc::c_char)
        .collect::<Vec<libc::c_char>>()
}

// t3, f58..f66 -- std::string_view.  Same representation as t1: `Vec<libc::c_char>`,
// NUL-TERMINATED (so size() is len()-1 and empty() is len() <= 1).  See src.cpp for the
// measured aliasing adjudication and for the per-member refusals (data(), iterators,
// ordering, operator[]/at/back/front, find*, the (const char*, size_t) ctor).
fn t3() -> Vec<libc::c_char> {
    Vec::new()
}

unsafe fn f58() -> Vec<libc::c_char> {
    vec![0]
}
unsafe fn f59(a0: Vec<libc::c_char>) -> Vec<libc::c_char> {
    a0.clone()
}
unsafe fn f60(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> bool {
    a0 == a1
}
unsafe fn f61(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>) -> bool {
    a0 != a1
}
unsafe fn f62(a0: Vec<libc::c_char>) -> usize {
    (a0.len() - 1)
}
unsafe fn f63(a0: Vec<libc::c_char>) -> usize {
    (a0.len() - 1)
}
unsafe fn f64(a0: Vec<libc::c_char>) -> bool {
    a0.len() <= 1
}
unsafe fn f66(a0: Vec<libc::c_char>, a1: usize, a2: usize) -> Vec<libc::c_char> {
    let mut __sv2 = a0[(a1) as usize..::std::cmp::min((a1.saturating_add(a2)) as usize, a0.len() - 1)].to_vec();
    __sv2.push(0);
    __sv2
}
unsafe fn f81(a0: Vec<libc::c_char>, a1: Vec<libc::c_char>, a2: usize) -> usize {
    // Each `aN` occurs EXACTLY ONCE (rule bodies are inlined, so every occurrence
    // re-evaluates the argument), and neither buffer is MOVED -- a0 is the receiver
    // place (`jsonString`) and is live after the call.
    (|__hv: &Vec<libc::c_char>, __nv: &Vec<libc::c_char>, __pos: usize| -> usize {
        let __h = &__hv[..__hv.len().saturating_sub(1)];
        let __n = &__nv[..__nv.len().saturating_sub(1)];
        if __n.len() > __h.len() {
            usize::MAX
        } else {
            let __last = ::std::cmp::min(__pos, __h.len() - __n.len());
            (0..=__last)
                .rev()
                .find(|&__i| &__h[__i..__i + __n.len()] == __n)
                .unwrap_or(usize::MAX)
        }
    })(&a0, &a1, a2)
}
