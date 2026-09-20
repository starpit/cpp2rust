extern crate libc;
use libc::*;
extern crate libcc2rs;
use libcc2rs::*;
use std::collections::BTreeMap;
use std::io::{Read, Seek, Write};
use std::os::fd::{AsFd, FromRawFd, IntoRawFd};
use std::rc::Rc;
pub fn main() {
    unsafe {
        __cpp2rust_init_globals();
        std::process::exit(main_0() as i32);
    }
}
unsafe fn main_0() -> i32 {
    let mut t: (i32, f64) = (1.into(), 2.5E+0.into());
    printf(
        c"%d %d\n".as_ptr() as *const i8,
        (*&raw mut (*&mut t).0),
        (((*&raw mut (*&mut t).1) * (2_f64)) as i32),
    );
    let mut owned: Option<Box<i32>> = Some(Box::new(7));
    printf(
        c"%d\n".as_ptr() as *const i8,
        (*owned.as_deref_mut().unwrap()),
    );
    let mut shared: Option<Value<i32>> = Some(Rc::new(std::cell::RefCell::new(9)));
    printf(
        c"%d\n".as_ptr() as *const i8,
        (*shared.as_ref().map_or(::std::ptr::null_mut(), |rc| {
            std::cell::RefCell::as_ptr(&**rc)
        })),
    );
    let mut empty: Option<Box<i32>> = None;
    printf(
        c"%d\n".as_ptr() as *const i8,
        (((empty
            .as_deref_mut()
            .map_or(::std::ptr::null_mut(), |v| v as *mut i32))
        .is_null()) as i32),
    );
    let mut pair: (i32, i32) = <(i32, i32)>::default();
    printf(c"%d %d\n".as_ptr() as *const i8, pair.0, pair.1);
    let mut a: i32 = 3;
    let mut b: i32 = 4;
    let mut hi: i32 = (*if *&mut a >= *&mut b {
        (&mut a) as *const _
    } else {
        (&mut b) as *const _
    });
    let mut lo: i32 = (*if *&mut a <= *&mut b {
        (&mut a) as *const _
    } else {
        (&mut b) as *const _
    });
    printf(c"%d %d\n".as_ptr() as *const i8, hi, lo);
    return 0;
}
pub unsafe fn __cpp2rust_init_globals() {}
