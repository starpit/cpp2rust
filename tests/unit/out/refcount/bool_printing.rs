extern crate libcc2rs;
use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::io::prelude::*;
use std::io::{Read, Seek, Write};
use std::os::fd::AsFd;
use std::rc::{Rc, Weak};
pub fn foo_0() -> bool {
    return true;
}
pub fn bar_1() -> bool {
    return true;
}
pub fn main() {
    __cpp2rust_init_globals();
    std::process::exit(main_0());
}
fn main_0() -> i32 {
    let i1: Value<i32> = Rc::new(RefCell::new(0));
    let i2: Value<i32> = Rc::new(RefCell::new(1));
    libcc2rs::cc2_insert_bool(&libcc2rs::cout(), true);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('\n' as u8) as u8]);
    libcc2rs::cc2_insert_bool(&libcc2rs::cout(), false);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('\n' as u8) as u8]);
    libcc2rs::cc2_insert_bool(&libcc2rs::cout(), ((*i1.borrow()) != (*i2.borrow())));
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('\n' as u8) as u8]);
    libcc2rs::cc2_insert_bool(&libcc2rs::cout(), ((*i1.borrow()) == (*i2.borrow())));
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('\n' as u8) as u8]);
    libcc2rs::cc2_insert_bool(&libcc2rs::cout(), ({ foo_0() }));
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('\n' as u8) as u8]);
    libcc2rs::cc2_insert_bool(&libcc2rs::cout(), ({ bar_1() }));
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('\n' as u8) as u8]);
    return 0;
}
pub fn __cpp2rust_init_globals() {}
