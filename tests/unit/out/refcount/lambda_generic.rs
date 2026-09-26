extern crate libcc2rs;
use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::io::prelude::*;
use std::io::{Read, Seek, Write};
use std::os::fd::AsFd;
use std::rc::{Rc, Weak};
pub fn main() {
    __cpp2rust_init_globals();
    std::process::exit(main_0());
}
fn main_0() -> i32 {
    assert!(
        (({
            (|x: i32| {
                let x: Value<i32> = Rc::new(RefCell::new(x));
                return ((*x.borrow()) + (*x.borrow()));
            })(4)
        }) == 8)
    );
    assert!(
        (({
            (|x: f64| {
                let x: Value<f64> = Rc::new(RefCell::new(x));
                return ((*x.borrow()) + (*x.borrow()));
            })(1.5E+0)
        }) == 3.0E+0)
    );
    let base: Value<i32> = Rc::new(RefCell::new(10));
    assert!(
        (({
            (|x: i32| {
                let x: Value<i32> = Rc::new(RefCell::new(x));
                return ((*x.borrow()) + (*base.borrow()));
            })(5)
        }) == 15)
    );
    assert!(
        (({
            (|x: f64| {
                let x: Value<f64> = Rc::new(RefCell::new(x));
                return ((*x.borrow()) + ((*base.borrow()) as f64));
            })(2.5E+0)
        }) == 1.25E+1)
    );
    let total: Value<i32> = Rc::new(RefCell::new(0));
    ({
        (|x: i32, y: i32| {
            let x: Value<i32> = Rc::new(RefCell::new(x));
            let y: Value<i32> = Rc::new(RefCell::new(y));
            (*total.borrow_mut()) += ((*x.borrow()) * (*y.borrow()));
        })(2, 3)
    });
    ({
        (|x: u32, y: u32| {
            let x: Value<u32> = Rc::new(RefCell::new(x));
            let y: Value<u32> = Rc::new(RefCell::new(y));
            {
                let rhs_0 = (((*total.borrow()) as u32)
                    .wrapping_add((*x.borrow()).wrapping_mul((*y.borrow()))))
                    as i32;
                (*total.borrow_mut()) = rhs_0
            };
        })(4_u32, 5_u32)
    });
    assert!(((*total.borrow()) == 26));
    assert!(
        (({
            (|x: i32, y: i32| {
                let x: Value<i32> = Rc::new(RefCell::new(x));
                let y: Value<i32> = Rc::new(RefCell::new(y));
                return ((*x.borrow()) - (*y.borrow()));
            })(9, 4)
        }) == 5)
    );
    assert!(
        (({
            (|x: f64, y: f64| {
                let x: Value<f64> = Rc::new(RefCell::new(x));
                let y: Value<f64> = Rc::new(RefCell::new(y));
                return ((*x.borrow()) - (*y.borrow()));
            })(2.5E+0, 1.0E+0)
        }) == 1.5E+0)
    );
    assert!(
        (({
            (|x: i32, y: i32| {
                let x: Value<i32> = Rc::new(RefCell::new(x));
                let y: Value<i32> = Rc::new(RefCell::new(y));
                return (((*x.borrow()) * (*y.borrow())) + (*base.borrow()));
            })(2, 3)
        }) == 16)
    );
    assert!(
        (({
            (|x: i32, y: f64| {
                let x: Value<i32> = Rc::new(RefCell::new(x));
                let y: Value<f64> = Rc::new(RefCell::new(y));
                return ((((*x.borrow()) as f64) * (*y.borrow())) + ((*base.borrow()) as f64));
            })(2, 5.0E-1)
        }) == 1.1E+1)
    );
    return 0;
}
pub fn __cpp2rust_init_globals() {}
