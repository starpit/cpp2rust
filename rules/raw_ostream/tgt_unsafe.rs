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

unsafe fn f7(a0: *mut std::fs::File, a1: Vec<libc::c_char>) -> *mut std::fs::File {
    let __o = a0;
    let __b: Vec<u8> = a1.iter().map(|&c| c as u8).collect();
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
