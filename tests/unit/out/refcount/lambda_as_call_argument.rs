extern crate libcc2rs;
use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::io::prelude::*;
use std::io::{Read, Seek, Write};
use std::os::fd::AsFd;
use std::rc::{Rc, Weak};
thread_local!(
    pub static log__0: Value<i32> = Rc::new(RefCell::new(0));
);
pub fn run_1(f: impl Fn(i32), x: i32) -> i32 {
    let f: Value<_> = Rc::new(RefCell::new(f));
    let x: Value<i32> = Rc::new(RefCell::new(x));
    ({ (*f.borrow_mut())((*x.borrow())) });
    return ((*x.borrow()) + 1);
}
pub fn run_2(f: impl Fn(i32), x: i32) -> i32 {
    let f: Value<_> = Rc::new(RefCell::new(f));
    let x: Value<i32> = Rc::new(RefCell::new(x));
    ({ (*f.borrow_mut())((*x.borrow())) });
    return ((*x.borrow()) + 1);
}
pub fn run_3(f: impl Fn(i32), x: i32) -> i32 {
    let f: Value<_> = Rc::new(RefCell::new(f));
    let x: Value<i32> = Rc::new(RefCell::new(x));
    ({ (*f.borrow_mut())((*x.borrow())) });
    return ((*x.borrow()) + 1);
}
pub fn main() {
    __cpp2rust_init_globals();
    std::process::exit(main_0());
}
fn main_0() -> i32 {
    assert!(
        (({
            run_1(
                (|_a0: i32| {
                    let _a0: Value<i32> = Rc::new(RefCell::new(_a0));
                }),
                10,
            )
        }) == 11)
    );
    assert!((log__0.with(|rc| *rc.borrow()) == 0));
    assert!(
        (({
            run_2(
                (|v: i32| {
                    let v: Value<i32> = Rc::new(RefCell::new(v));
                    log__0.with(|rc| *rc.borrow_mut() += (*v.borrow()));
                }),
                20,
            )
        }) == 21)
    );
    assert!((log__0.with(|rc| *rc.borrow()) == 20));
    assert!(
        (({
            run_3(
                (|v: i32| {
                    let v: Value<i32> = Rc::new(RefCell::new(v));
                    if ((*v.borrow()) > 100) {
                        return;
                    }
                    log__0.with(|rc| *rc.borrow_mut() += ((*v.borrow()) * 2));
                }),
                30,
            )
        }) == 31)
    );
    assert!((log__0.with(|rc| *rc.borrow()) == 80));
    return 0;
}
pub fn __cpp2rust_init_globals() {
    let _ = log__0.with(|_| ());
}
