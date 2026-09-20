extern crate libc;
use libc::*;
extern crate libcc2rs;
use libcc2rs::*;
use std::collections::BTreeMap;
use std::io::{Read, Seek, Write};
use std::os::fd::{AsFd, FromRawFd, IntoRawFd};
use std::rc::Rc;
pub unsafe trait Base {
    unsafe fn id(&self) -> i32;
}
#[repr(C)]
#[derive(Copy, Clone, Default)]
pub struct Derived {}
unsafe impl Base for Derived {
    unsafe fn id(&self) -> i32 {
        return 7;
    }
}
pub fn main() {
    unsafe {
        __cpp2rust_init_globals();
        std::process::exit(main_0() as i32);
    }
}
unsafe fn main_0() -> i32 {
    let mut x: Derived = <Derived>::default();
    let mut b: *mut dyn Base = (&mut x as *mut Derived);
    assert!(((unsafe { (*(b).cast_const()).id() }) == (7)));
    let mut d: Vec<*mut dyn Base> = Vec::new();
    d.insert(0, (&mut x as *mut Derived));
    d.push((&mut x as *mut Derived));
    assert!(((d.len()) == (2_usize)));
    assert!(!(d.is_empty()));
    return 0;
}
pub unsafe fn __cpp2rust_init_globals() {}
