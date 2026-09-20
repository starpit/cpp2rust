extern crate libcc2rs;
use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::io::prelude::*;
use std::io::{Read, Seek, Write};
use std::os::fd::AsFd;
use std::rc::{Rc, Weak};
pub fn blocker_0(x: i32) -> i32 {
    let x: Value<i32> = Rc::new(RefCell::new(x));
    let r: Value<i32> = Rc::new(RefCell::new(0));
    switch!(match 0 {
        _ => {
            if ((*x.borrow()) > 0) {
                (*r.borrow_mut()) = 1;
            } else {
                (*r.borrow_mut()) = 2;
            }
        }
    });
    return (*r.borrow());
}
pub fn single_case_1(x: i32) -> i32 {
    let x: Value<i32> = Rc::new(RefCell::new(x));
    let r: Value<i32> = Rc::new(RefCell::new(0));
    switch!(match (*x.borrow()) {
        __v if __v == 1 => {
            (*r.borrow_mut()) = 7;
        }
        _ => {}
    });
    return (*r.borrow());
}
pub fn main() {
    __cpp2rust_init_globals();
    std::process::exit(main_0());
}
fn main_0() -> i32 {
    assert!((({ blocker_0(5,) }) == 1));
    assert!((({ blocker_0(-5_i32,) }) == 2));
    assert!((({ single_case_1(1,) }) == 7));
    assert!((({ single_case_1(2,) }) == 0));
    return 0;
}
pub fn __cpp2rust_init_globals() {}
