// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Overlay on tgt_unsafe.rs: t1/t4 (the stream value itself, std::fs::File) are
// identical in both models and are not repeated here.  Everything else changes
// shape, because `raw_ostream &` is Ptr<std::fs::File> rather than a raw
// pointer and `char` is u8 rather than libc::c_char.
//
// Ptr<std::fs::File> already has write_all (libcc2rs impls it for any
// Ptr<T: std::io::Write + ByteRepr>, and std::fs::File is ByteRepr), so the
// insertion bodies need no with_mut closure.  flush() has no such shortcut and
// goes through one.

use libcc2rs::*;

fn t2() -> Ptr<std::fs::File> {
    libcc2rs::cerr()
}

fn t3() -> Ptr<std::fs::File> {
    libcc2rs::cerr()
}

fn t5() -> Ptr<std::fs::File> {
    libcc2rs::cerr()
}

fn f1() -> Ptr<std::fs::File> {
    libcc2rs::cerr()
}

fn f2() -> Ptr<std::fs::File> {
    libcc2rs::cout()
}

fn f3() -> Ptr<std::fs::File> {
    libcc2rs::cerr()
}

fn f4(a0: Ptr<std::fs::File>) {
    let __o = a0;
    __o.with_mut(|__f: &mut std::fs::File| {
        let _ = ::std::io::Write::flush(__f);
    });
}

fn f5(a0: Ptr<std::fs::File>, a1: Ptr<u8>) -> Ptr<std::fs::File> {
    let __o = a0;
    let __b: Vec<u8> = a1.to_c_string_iterator().collect();
    let _ = __o.write_all(&__b);
    __o
}

fn f6(a0: Ptr<std::fs::File>, a1: Vec<u8>) -> Ptr<std::fs::File> {
    let __o = a0;
    let __b: Vec<u8> = a1
        .iter()
        .copied()
        .take(a1.len().saturating_sub(1))
        .collect();
    let _ = __o.write_all(&__b);
    __o
}

fn f7(a0: Ptr<std::fs::File>, a1: Vec<u8>) -> Ptr<std::fs::File> {
    let __o = a0;
    let _ = __o.write_all(&a1);
    __o
}

fn f8(a0: Ptr<std::fs::File>, a1: u8) -> Ptr<std::fs::File> {
    let __o = a0;
    let _ = __o.write_all(&[a1]);
    __o
}

fn f9(a0: Ptr<std::fs::File>, a1: u8) -> Ptr<std::fs::File> {
    let __o = a0;
    let _ = __o.write_all(&[a1]);
    __o
}

fn f10(a0: Ptr<std::fs::File>, a1: i8) -> Ptr<std::fs::File> {
    let __o = a0;
    let _ = __o.write_all(&[a1 as u8]);
    __o
}

fn f11(a0: Ptr<std::fs::File>, a1: i32) -> Ptr<std::fs::File> {
    let __o = a0;
    let __b = format!("{}", a1);
    let _ = __o.write_all(__b.as_bytes());
    __o
}

fn f12(a0: Ptr<std::fs::File>, a1: u32) -> Ptr<std::fs::File> {
    let __o = a0;
    let __b = format!("{}", a1);
    let _ = __o.write_all(__b.as_bytes());
    __o
}

fn f13(a0: Ptr<std::fs::File>, a1: i64) -> Ptr<std::fs::File> {
    let __o = a0;
    let __b = format!("{}", a1);
    let _ = __o.write_all(__b.as_bytes());
    __o
}

fn f14(a0: Ptr<std::fs::File>, a1: u64) -> Ptr<std::fs::File> {
    let __o = a0;
    let __b = format!("{}", a1);
    let _ = __o.write_all(__b.as_bytes());
    __o
}

fn f15(a0: Ptr<std::fs::File>, a1: i64) -> Ptr<std::fs::File> {
    let __o = a0;
    let __b = format!("{}", a1);
    let _ = __o.write_all(__b.as_bytes());
    __o
}

fn f16(a0: Ptr<std::fs::File>, a1: u64) -> Ptr<std::fs::File> {
    let __o = a0;
    let __b = format!("{}", a1);
    let _ = __o.write_all(__b.as_bytes());
    __o
}

// See tgt_unsafe.rs f17: "%e", not Rust's {:e}.
fn f17(a0: Ptr<std::fs::File>, a1: f64) -> Ptr<std::fs::File> {
    let __o = a0;
    let __t = format!("{:.6e}", a1);
    let __b = match __t.split_once('e') {
        Some((__m, __e)) => {
            let __v: i32 = __e.parse().unwrap_or(0);
            format!(
                "{}e{}{:02}",
                __m,
                if __v < 0 { '-' } else { '+' },
                __v.unsigned_abs()
            )
        }
        None => __t,
    };
    let _ = __o.write_all(__b.as_bytes());
    __o
}

// See tgt_unsafe.rs f18.  AnyPtr is the refcount model's `const void *`; its
// cc2_addr_of is the identity `to_int` already uses for casts, so the digits
// agree with any comparison the translated program makes on the same pointer.
fn f18(a0: Ptr<std::fs::File>, a1: AnyPtr) -> Ptr<std::fs::File> {
    let __o = a0;
    let __b = format!("0x{:x}", libcc2rs::cc2_addr_of(&a1));
    let _ = __o.write_all(__b.as_bytes());
    __o
}
