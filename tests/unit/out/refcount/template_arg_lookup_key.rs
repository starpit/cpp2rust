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
    let t: Value<(Value<i32>, Value<f64>)> = Rc::new(RefCell::new((
        Rc::new(RefCell::new(1.try_into().expect("failed conversion"))),
        Rc::new(RefCell::new(2.5E+0.try_into().expect("failed conversion"))),
    )));
    println!(
        "{} {}",
        (((*t.borrow()).0.as_pointer() as Ptr<i32>).read()),
        (((((*t.borrow()).1.as_pointer() as Ptr<f64>).read()) * 2_f64) as i32)
    );
    let owned: Value<Option<Value<i32>>> = Rc::new(RefCell::new(Some(Rc::new(RefCell::new(7)))));
    println!("{}", (*(*owned.borrow()).as_ref().unwrap().borrow()));
    let shared: Value<Option<Value<i32>>> =
        Rc::new(RefCell::new(Some(Rc::new(std::cell::RefCell::new(9)))));
    println!("{}", ((*shared.borrow()).as_pointer().read()));
    let empty: Value<Option<Value<i32>>> = Rc::new(RefCell::new(None));
    println!("{}", ((((*empty.borrow()).as_pointer()).is_null()) as i32));
    let pair: Value<(Value<i32>, Value<i32>)> = Rc::new(RefCell::new((
        Rc::new(RefCell::new(i32::default())),
        Rc::new(RefCell::new(i32::default())),
    )));
    println!(
        "{} {}",
        (*(*pair.borrow()).0.borrow()),
        (*(*pair.borrow()).1.borrow())
    );
    let a: Value<i32> = Rc::new(RefCell::new(3));
    let b: Value<i32> = Rc::new(RefCell::new(4));
    let hi: Value<i32> = Rc::new(RefCell::new(
        (if a.as_pointer().read() >= b.as_pointer().read() {
            a.as_pointer()
        } else {
            b.as_pointer()
        }
        .read()),
    ));
    let lo: Value<i32> = Rc::new(RefCell::new(
        (if a.as_pointer().read() <= b.as_pointer().read() {
            a.as_pointer()
        } else {
            b.as_pointer()
        }
        .read()),
    ));
    println!("{} {}", (*hi.borrow()), (*lo.borrow()));
    return 0;
}
pub fn __cpp2rust_init_globals() {}
