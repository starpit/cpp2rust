extern crate libc;
use libc::*;
extern crate libcc2rs;
use libcc2rs::*;
use std::collections::BTreeMap;
use std::io::{Read, Seek, Write};
use std::os::fd::{AsFd, FromRawFd, IntoRawFd};
use std::rc::Rc;
pub static mut log__0: std::cell::LazyCell<i32> = std::cell::LazyCell::new(|| unsafe { 0 });
pub unsafe fn run_1(mut f: impl Fn(i32), mut x: i32) -> i32 {
    (unsafe { f(x) });
    return ((x) + (1));
}
pub unsafe fn run_2(mut f: impl Fn(i32), mut x: i32) -> i32 {
    (unsafe { f(x) });
    return ((x) + (1));
}
pub unsafe fn run_3(mut f: impl Fn(i32), mut x: i32) -> i32 {
    (unsafe { f(x) });
    return ((x) + (1));
}
pub fn main() {
    unsafe {
        __cpp2rust_init_globals();
        std::process::exit(main_0() as i32);
    }
}
unsafe fn main_0() -> i32 {
    assert!(((unsafe { run_1((|_a0: i32| {}), 10,) }) == (11)));
    assert!(((*std::cell::LazyCell::force_mut(&mut *&raw mut log__0)) == (0)));
    assert!(
        ((unsafe {
            run_2(
                (|v: i32| {
                    (*std::cell::LazyCell::force_mut(&mut *&raw mut log__0)) += v;
                }),
                20,
            )
        }) == (21))
    );
    assert!(((*std::cell::LazyCell::force_mut(&mut *&raw mut log__0)) == (20)));
    assert!(
        ((unsafe {
            run_3(
                (|v: i32| {
                    if ((v) > (100)) {
                        return;
                    }
                    (*std::cell::LazyCell::force_mut(&mut *&raw mut log__0)) += ((v) * (2));
                }),
                30,
            )
        }) == (31))
    );
    assert!(((*std::cell::LazyCell::force_mut(&mut *&raw mut log__0)) == (80)));
    return 0;
}
pub unsafe fn __cpp2rust_init_globals() {
    std::cell::LazyCell::force(&*&raw const log__0);
}
