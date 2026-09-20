extern crate libc;
use libc::*;
extern crate libcc2rs;
use libcc2rs::*;
use std::collections::BTreeMap;
use std::io::{Read, Seek, Write};
use std::os::fd::{AsFd, FromRawFd, IntoRawFd};
use std::rc::Rc;
pub unsafe fn blocker_0(mut x: i32) -> i32 {
    let mut r: i32 = 0;
    switch!(match 0 {
        _ => {
            if ((x) > (0)) {
                r = 1;
            } else {
                r = 2;
            }
        }
    });
    return r;
}
pub unsafe fn single_case_1(mut x: i32) -> i32 {
    let mut r: i32 = 0;
    switch!(match x {
        __v if __v == 1 => {
            r = 7;
        }
        _ => {}
    });
    return r;
}
pub fn main() {
    unsafe {
        __cpp2rust_init_globals();
        std::process::exit(main_0() as i32);
    }
}
unsafe fn main_0() -> i32 {
    assert!(((unsafe { blocker_0(5,) }) == (1)));
    assert!(((unsafe { blocker_0(-5_i32,) }) == (2)));
    assert!(((unsafe { single_case_1(1,) }) == (7)));
    assert!(((unsafe { single_case_1(2,) }) == (0)));
    return 0;
}
pub unsafe fn __cpp2rust_init_globals() {}
