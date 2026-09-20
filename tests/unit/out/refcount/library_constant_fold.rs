extern crate libcc2rs;
use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::io::prelude::*;
use std::io::{Read, Seek, Write};
use std::os::fd::AsFd;
use std::rc::{Rc, Weak};
pub fn found_0(pos: usize) -> bool {
    let pos: Value<usize> = Rc::new(RefCell::new(pos));
    return ((*pos.borrow()) != 18446744073709551615_usize);
}
pub fn main() {
    __cpp2rust_init_globals();
    std::process::exit(main_0());
}
fn main_0() -> i32 {
    let n: Value<usize> = Rc::new(RefCell::new(18446744073709551615_usize));
    assert!(((*n.borrow()) != 0_usize));
    assert!(((*n.borrow()) == (-1_i32 as usize)));
    assert!(!({ found_0((*n.borrow()),) }));
    assert!(({ found_0(3_usize,) }));
    let m: Value<usize> = Rc::new(RefCell::new(18446744073709551615_usize));
    let __rhs = (*m.borrow()).wrapping_sub(1_usize);
    (*m.borrow_mut()) = __rhs;
    assert!(((*m.borrow()) == (-2_i32 as usize)));
    return 0;
}
pub fn __cpp2rust_init_globals() {}
