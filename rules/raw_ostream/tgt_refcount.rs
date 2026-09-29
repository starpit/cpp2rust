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

// See tgt_unsafe.rs f7: a StringRef is a NUL-TERMINATED Vec in this port, so the
// terminator is dropped, as in f6.  Writing the whole Vec appended a stray NUL to
// every diagnostic.
fn f7(a0: Ptr<std::fs::File>, a1: Vec<u8>) -> Ptr<std::fs::File> {
    let __o = a0;
    let __b: Vec<u8> = a1
        .iter()
        .copied()
        .take(a1.len().saturating_sub(1))
        .collect();
    let _ = __o.write_all(&__b);
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

// See tgt_unsafe.rs f18.  `AnyPtr` is the refcount model's `const void *`, and
// `AnyPtr::to_int` (libcc2rs/src/void.rs:106) is the same address a translated
// cast produces, so the printed digits agree with any comparison the program
// makes on that pointer.
//
// The ported version called `libcc2rs::cc2_addr_of`, which lives in
// `libcc2rs/src/stream_fmt.rs` -- a file present only on the `dt-src-port`
// branch. That dependency made this module fail to generate its targets and
// left an INCOMPLETE MODULE DIR, which aborts every translation in the project.
fn f18(a0: Ptr<std::fs::File>, a1: AnyPtr) -> Ptr<std::fs::File> {
    let __o = a0;
    let __b = format!("0x{:x}", a1.to_int());
    let _ = __o.write_all(__b.as_bytes());
    __o
}

// t560 / f560 / f561 -- `llvm::raw_string_ostream`.  See the t560 block in
// `src.cpp`.  Not an overlay omission: these three DIFFER from the unsafe bodies
// (`Ptr<Vec<u8>>` vs `*mut Vec<libc::c_char>`), so all three are spelled out.
//
// ⚠️ NOT `Ptr<std::fs::File>`: this is the one spelling in this module whose bytes
// go into a `std::string` the caller reads back, so the target is rules/string's
// t1 BY REFERENCE -- `Ptr<Vec<u8>>`, the same shape rules/string f4/f14/f26 give a
// `std::string &`.
fn t560() -> Ptr<Vec<u8>> {
    Ptr::null()
}

// f560 -- the constructor.  `Ptr` is NOT `Copy`, so the body must not bind `a0` to a
// `let` first (that is E0382 at rc=0 with no placeholder token); moving it straight
// out is a single move and is fine.
fn f560(a0: Ptr<Vec<u8>>) -> Ptr<Vec<u8>> {
    a0
}

// f561 -- `str()`, the identity.  Same single-move rule as f560.
fn f561(a0: Ptr<Vec<u8>>) -> Ptr<Vec<u8>> {
    a0
}

// ---------------------------------------------------------------------------
// f600 / f601 / f602 -- SPELLED OUT HERE, NOT LEFT TO THE OVERLAY.
// ⛔ AN ABSENT REFCOUNT TARGET IS NOT A LOUD REFUSAL, IT IS A SILENT FALLBACK:
// the two models are a UNION, so a key present only in tgt_unsafe.rs makes the
// refcount model reuse the UNSAFE body, and the mismatch (`*mut std::fs::File` vs
// `Ptr<std::fs::File>`) then surfaces at rustc rather than at translate time.
// check-ir.sh reports that union explicitly; these three must not add to it.
// Only the STREAM argument changes shape (`Ptr<std::fs::File>`, which already has
// `write_all`); the rendered text is identical to tgt_unsafe.rs by construction.
// ---------------------------------------------------------------------------

fn f600(a0: Ptr<std::fs::File>, a1: Option<dataflowir_gen::TdOpDef>) -> Ptr<std::fs::File> {
    let __o = a0;
    let __s = match a1 {
        Some(ref __d) => {
            format!("{}.{}", dataflowir_gen::row_dialect(__d), __d.mnemonic)
        }
        None => panic!(
            "ub: llvm::raw_ostream << mlir::OperationName on a null op-name handle"
        ),
    };
    let _ = __o.write_all(__s.as_bytes());
    __o
}

fn f601(a0: Ptr<std::fs::File>, a1: dataflowir_gen::ir::AffineExpr) -> Ptr<std::fs::File> {
    let __o = a0;
    let __s = ::std::string::ToString::to_string(&a1);
    let _ = __o.write_all(__s.as_bytes());
    __o
}

fn f602(a0: Ptr<std::fs::File>, a1: dataflowir_gen::ir::AffineMap) -> Ptr<std::fs::File> {
    let __o = a0;
    let __s = ::std::string::ToString::to_string(&a1);
    let _ = __o.write_all(__s.as_bytes());
    __o
}
