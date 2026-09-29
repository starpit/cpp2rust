// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model.  A raw_ostream is a std::fs::File, a
// `raw_ostream &` is a raw pointer to one, and every insertion returns the
// receiver so that `a << b << c` and `return o << x;` both have a value.
//
// Writes go through the fully qualified `::std::io::Write::write_all` rather
// than method syntax: the rule body is inlined into the translated crate, and
// spelling the trait out means the body does not depend on `Write` happening
// to be in scope there.  Write errors are dropped, matching raw_ostream, which
// records an error code on the stream instead of reporting it at the call.

fn t1() -> std::fs::File {
    std::fs::File::open("").unwrap()
}

unsafe fn t2() -> *mut std::fs::File {
    libcc2rs::cerr_unsafe()
}

unsafe fn t3() -> *mut std::fs::File {
    libcc2rs::cerr_unsafe()
}

fn t4() -> std::fs::File {
    std::fs::File::open("").unwrap()
}

unsafe fn t5() -> *mut std::fs::File {
    libcc2rs::cerr_unsafe()
}

unsafe fn f1() -> *mut std::fs::File {
    libcc2rs::cerr_unsafe()
}

unsafe fn f2() -> *mut std::fs::File {
    libcc2rs::cout_unsafe()
}

unsafe fn f3() -> *mut std::fs::File {
    libcc2rs::cerr_unsafe()
}

unsafe fn f4(a0: *mut std::fs::File) {
    let __o = a0;
    let _ = ::std::io::Write::flush(&mut *__o);
}

unsafe fn f5(a0: *mut std::fs::File, a1: *const libc::c_char) -> *mut std::fs::File {
    let __o = a0;
    let __b = ::std::ffi::CStr::from_ptr(a1).to_bytes().to_vec();
    let _ = ::std::io::Write::write_all(&mut *__o, &__b);
    __o
}

unsafe fn f6(a0: *mut std::fs::File, a1: Vec<libc::c_char>) -> *mut std::fs::File {
    let __o = a0;
    let __b: Vec<u8> = a1
        .iter()
        .take(a1.len().saturating_sub(1))
        .map(|&c| c as u8)
        .collect();
    let _ = ::std::io::Write::write_all(&mut *__o, &__b);
    __o
}

// operator<<(StringRef).  rules/stringref models a StringRef as the same
// NUL-TERMINATED Vec rules/string gives std::string (its size() is len()-1), so
// the terminator is NOT part of the string and must be dropped here -- exactly
// as f6 does for const std::string &.  Writing the whole Vec appended a stray
// 0x00 byte to every diagnostic and broke chained insertions (`os << a << b`
// produced "a\0b\0"); measured against the C++ build with probe.sh.
unsafe fn f7(a0: *mut std::fs::File, a1: Vec<libc::c_char>) -> *mut std::fs::File {
    let __o = a0;
    let __b: Vec<u8> = a1
        .iter()
        .take(a1.len().saturating_sub(1))
        .map(|&c| c as u8)
        .collect();
    let _ = ::std::io::Write::write_all(&mut *__o, &__b);
    __o
}

unsafe fn f8(a0: *mut std::fs::File, a1: libc::c_char) -> *mut std::fs::File {
    let __o = a0;
    let _ = ::std::io::Write::write_all(&mut *__o, &[a1 as u8]);
    __o
}

unsafe fn f9(a0: *mut std::fs::File, a1: u8) -> *mut std::fs::File {
    let __o = a0;
    let _ = ::std::io::Write::write_all(&mut *__o, &[a1 as u8]);
    __o
}

unsafe fn f10(a0: *mut std::fs::File, a1: i8) -> *mut std::fs::File {
    let __o = a0;
    let _ = ::std::io::Write::write_all(&mut *__o, &[a1 as u8]);
    __o
}

unsafe fn f11(a0: *mut std::fs::File, a1: i32) -> *mut std::fs::File {
    let __o = a0;
    let __b = format!("{}", a1);
    let _ = ::std::io::Write::write_all(&mut *__o, __b.as_bytes());
    __o
}

unsafe fn f12(a0: *mut std::fs::File, a1: u32) -> *mut std::fs::File {
    let __o = a0;
    let __b = format!("{}", a1);
    let _ = ::std::io::Write::write_all(&mut *__o, __b.as_bytes());
    __o
}

unsafe fn f13(a0: *mut std::fs::File, a1: i64) -> *mut std::fs::File {
    let __o = a0;
    let __b = format!("{}", a1);
    let _ = ::std::io::Write::write_all(&mut *__o, __b.as_bytes());
    __o
}

unsafe fn f14(a0: *mut std::fs::File, a1: u64) -> *mut std::fs::File {
    let __o = a0;
    let __b = format!("{}", a1);
    let _ = ::std::io::Write::write_all(&mut *__o, __b.as_bytes());
    __o
}

unsafe fn f15(a0: *mut std::fs::File, a1: i64) -> *mut std::fs::File {
    let __o = a0;
    let __b = format!("{}", a1);
    let _ = ::std::io::Write::write_all(&mut *__o, __b.as_bytes());
    __o
}

unsafe fn f16(a0: *mut std::fs::File, a1: u64) -> *mut std::fs::File {
    let __o = a0;
    let __b = format!("{}", a1);
    let _ = ::std::io::Write::write_all(&mut *__o, __b.as_bytes());
    __o
}

// raw_ostream::operator<<(double) is llvm::write_double(FloatStyle::Exponent),
// i.e. C's "%e": six fraction digits and an at-least-two-digit signed
// exponent.  Rust's {:e} prints neither, so the exponent is rebuilt here.
unsafe fn f17(a0: *mut std::fs::File, a1: f64) -> *mut std::fs::File {
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
    let _ = ::std::io::Write::write_all(&mut *__o, __b.as_bytes());
    __o
}

// LLVM prints a pointer as write_hex(HexPrintStyle::PrefixLower): the literal
// "0x", then lowercase hex with no zero padding and no width, so a null
// pointer is "0x0" and not the bare "0" std::ostream's showbase produces.
// The address is the pointer value itself. The ported version of this rule
// called `libcc2rs::cc2_addr_of`, which lives in `libcc2rs/src/stream_fmt.rs`
// -- a file that exists only on the `dt-src-port` branch. Depending on it made
// this module fail to generate its targets, leaving an INCOMPLETE MODULE DIR
// (ir_src.json with no ir_unsafe/ir_refcount), which aborts EVERY translation
// in the project. Computed directly instead, so the rule needs nothing that is
// not already on this branch.
unsafe fn f18(a0: *mut std::fs::File, a1: *const ::libc::c_void) -> *mut std::fs::File {
    let __o = a0;
    let __b = format!("0x{:x}", a1 as usize);
    let _ = ::std::io::Write::write_all(&mut *__o, __b.as_bytes());
    __o
}

// t540 -- `llvm::impl::raw_ldbg_ostream` -> `std::fs::File`, t1/t4's body verbatim.
// It is a `raw_ostream` subclass (DebugLog.h:233), so it gets this module's
// raw_ostream model unchanged.  See src.cpp for where its bytes go and for the
// prefix that is deliberately not modelled.
// ⚠️ NOT REPEATED IN tgt_refcount.rs, and that is this module's convention rather
// than an omission: the refcount overlay's own header says the VALUE types t1/t4
// (`std::fs::File`) "are identical in both models and are not repeated here" --
// only the `&`/`*` spellings change shape.  t540 is a value type, and no `&`/`*`
// spelling of it is asked.
fn t540() -> std::fs::File {
    std::fs::File::open("").unwrap()
}

// t560 / f560 / f561 -- `llvm::raw_string_ostream`.  The measurement, the header
// check, the three asked key spellings and the loud-insertion note are all in
// `src.cpp` at the t560 block.
//
// ⚠️ THE TARGET TYPE IS NOT `std::fs::File` -- this is the one spelling in this
// module that is NOT a file descriptor, because a raw_string_ostream's whole
// purpose is that its bytes land in a `std::string` the caller reads back.  It is
// rules/string's t1 BY REFERENCE, i.e. a raw pointer to the NUL-terminated
// `Vec<libc::c_char>`, which is also what a `std::string &` parameter is spelled as
// everywhere else in this model.
//
// ⚠️ `null_mut()` is the t560 sentinel for the same reason rules/mlir t461 uses it:
// a type rule's body is only ever the DEFAULT VALUE for a declaration the converter
// cannot initialise, and "no string to write into" is the faithful such value.
unsafe fn t560() -> *mut Vec<libc::c_char> {
    ::std::ptr::null_mut()
}

// f560 -- the constructor.  The C++ stores `std::string &OS`, so the body hands back
// the address of the string it is given and is otherwise the identity.
//
// ⛔⛔ `::core::ptr::from_mut` RATHER THAN A CAST, AND THE REASON IS MEASURED, NOT
// STYLISTIC.  THREE bodies were tried and the emitted text read back out of the four
// live sites each time; ONLY the third is correct, and the first two fail with NO
// placeholder token to make them visible:
//   (1) `a0 as *mut Vec<libc::c_char>`  records `access: "borrow"` and the converter
//       emits a borrow placeholder as a BARE PLACE EXPRESSION, so the site read
//       `resource_str as *mut Vec<libc::c_char>` on a `Vec<libc::c_char>` local:
//       `error[E0605]: non-primitive cast`.
//   (2) `&mut *a0 as *mut Vec<libc::c_char>`  records text `"&mut "` PLUS
//       `access: "borrow_mut"` -- i.e. an explicit `&mut` in a rule body is recorded
//       as text AND upgrades the placeholder, and the converter then adds its own, so
//       the site read `&mut &mut resource_str as *mut Vec<libc::c_char>`: a
//       `&mut &mut Vec`, E0605 again.
//   (3) `::core::ptr::from_mut(a0)`  records text `"::core::ptr::from_mut("` +
//       `access: "borrow_mut"` + `")"` with NO `&mut` of my own, and the site reads
//       `::core::ptr::from_mut(&mut resource_str)`.  ⭐ So the way to get a MUTABLE
//       borrow of an argument into a rule body is to put it in ARGUMENT POSITION of a
//       function that takes `&mut T`, never to write the `&mut` yourself.
//       (`from_mut` is stable since Rust 1.76 and is in `core`, so the inlined body
//       depends on nothing being in scope in the translated crate -- the same reason
//       f5 spells `::std::io::Write::write_all` out in full.)
unsafe fn f560(a0: &mut Vec<libc::c_char>) -> *mut Vec<libc::c_char> {
    ::core::ptr::from_mut(a0)
}

// f561 -- `str()`, `{ return OS; }` in the header, so the identity here.  The
// receiver is taken BY VALUE (a raw pointer is Copy) rather than as a borrow,
// because a borrow placeholder would make the converter emit `&ss`, i.e. a
// `&*mut Vec<..>`, and the call sites immediately deref the result
// (`(*(ss.str())).as_ptr()`).
unsafe fn f561(a0: *mut Vec<libc::c_char>) -> *mut Vec<libc::c_char> {
    a0
}

// ---------------------------------------------------------------------------
// f600 / f601 / f602 -- the free `llvm::raw_ostream <<` MLIR keys.  See the
// PASS 2026-09-29 block in src.cpp for the census, the fidelity argument, and the
// three shapes of this family that STAY REFUSED.
//
// ⚠️ EVERY BODY BELOW IS SELF-CONTAINED AND CARRIES NO FILE-LEVEL ITEM OF ITS OWN.
// A file-level helper in a tgt_*.rs is NOT copied into the emitted output (measured
// on rules/getopt, 2026-09-29: the converter emits the bare name, i.e. the same
// E0425 MINUS the Cpp2RustUnmapped marker the placeholder census greps for -- which
// is strictly worse than having no key).  Everything these need is either in `core`
// or reached by an absolute `dataflowir_gen::` path, so nothing has to be in scope
// in the translated crate.
// ---------------------------------------------------------------------------

// f600 -- `raw_ostream << mlir::OperationName`.  `mlir::OperationName` is
// rules/mlir t18, `Option<dataflowir_gen::TdOpDef>` (an OWNING copy of the
// immutable registry row -- see t18's note there for why it may not carry a
// lifetime).  The string is byte-identical to rules/mlir f2100
// (`OperationName::getStringRef`) ON PURPOSE: `<<` is `info.print(os)` and
// `print` writes `getStringRef()`, so the two keys must never be able to disagree.
//
// ⛔ `None` PANICS RATHER THAN PRINTING ANYTHING.  `None` is the null / unregistered
// handle, and in C++ `OperationName::print` on one dereferences a null `Impl *` --
// UB.  f2100 panics on the same arm with the same wording; inventing a placeholder
// text here would be the silent-wrongness this whole row exists to avoid.
unsafe fn f600(
    a0: *mut std::fs::File,
    a1: Option<dataflowir_gen::TdOpDef>,
) -> *mut std::fs::File {
    let __o = a0;
    let __s = match a1 {
        Some(ref __d) => {
            format!("{}.{}", dataflowir_gen::row_dialect(__d), __d.mnemonic)
        }
        None => panic!(
            "ub: llvm::raw_ostream << mlir::OperationName on a null op-name handle"
        ),
    };
    let _ = ::std::io::Write::write_all(&mut *__o, __s.as_bytes());
    __o
}

// f601 -- `raw_ostream << mlir::AffineExpr`.  rules/mlir t83 is
// `dataflowir_gen::ir::AffineExpr`.  `Display` (ir.rs:359) is the printed affine
// grammar; `ToString` is in the prelude, so `to_string()` needs nothing imported.
unsafe fn f601(
    a0: *mut std::fs::File,
    a1: dataflowir_gen::ir::AffineExpr,
) -> *mut std::fs::File {
    let __o = a0;
    let __s = ::std::string::ToString::to_string(&a1);
    let _ = ::std::io::Write::write_all(&mut *__o, __s.as_bytes());
    __o
}

// f602 -- `raw_ostream << mlir::AffineMap`.  rules/mlir t8 is
// `dataflowir_gen::ir::AffineMap`; Display is ir.rs:402.
unsafe fn f602(
    a0: *mut std::fs::File,
    a1: dataflowir_gen::ir::AffineMap,
) -> *mut std::fs::File {
    let __o = a0;
    let __s = ::std::string::ToString::to_string(&a1);
    let _ = ::std::io::Write::write_all(&mut *__o, __s.as_bytes());
    __o
}
