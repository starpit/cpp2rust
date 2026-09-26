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
    assert!(
        ((unsafe {
            (|x: i32| {
                return ((x) + (x));
            })(4)
        }) == (8))
    );
    assert!(
        ((unsafe {
            (|x: f64| {
                return ((x) + (x));
            })(1.5E+0)
        }) == (3.0E+0))
    );
    let mut base: i32 = 10;
    assert!(
        ((unsafe {
            (|x: i32| {
                return ((x) + (base));
            })(5)
        }) == (15))
    );
    assert!(
        ((unsafe {
            (|x: f64| {
                return ((x) + (base as f64));
            })(2.5E+0)
        }) == (1.25E+1))
    );
    let mut total: i32 = 0;
    (unsafe {
        (|x: i32, y: i32| {
            total += ((x) * (y));
        })(2, 3)
    });
    (unsafe {
        (|x: u32, y: u32| {
            total = ((total as u32).wrapping_add((x).wrapping_mul(y))) as i32;
        })(4_u32, 5_u32)
    });
    assert!(((total) == (26)));
    assert!(
        ((unsafe {
            (|x: i32, y: i32| {
                return ((x) - (y));
            })(9, 4)
        }) == (5))
    );
    assert!(
        ((unsafe {
            (|x: f64, y: f64| {
                return ((x) - (y));
            })(2.5E+0, 1.0E+0)
        }) == (1.5E+0))
    );
    assert!(
        ((unsafe {
            (|x: i32, y: i32| {
                return (((x) * (y)) + (base));
            })(2, 3)
        }) == (16))
    );
    assert!(
        ((unsafe {
            (|x: i32, y: f64| {
                return (((x as f64) * (y)) + (base as f64));
            })(2, 5.0E-1)
        }) == (1.1E+1))
    );
    return 0;
}
pub unsafe fn __cpp2rust_init_globals() {}
