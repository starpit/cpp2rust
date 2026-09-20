extern crate libc;
use libc::*;
extern crate libcc2rs;
use libcc2rs::*;
use std::collections::BTreeMap;
use std::io::{Read, Seek, Write};
use std::os::fd::{AsFd, FromRawFd, IntoRawFd};
use std::rc::Rc;
#[repr(C)]
#[derive(Copy, Clone, Default)]
pub struct Node {
    pub x: i32,
}
pub unsafe fn put_0(m: *mut BTreeMap<*mut Node, Box<i32>>, mut k: *mut Node, mut v: i32) {
    (*(*m).entry(k).or_default().as_mut()) = v;
}
pub fn main() {
    unsafe {
        __cpp2rust_init_globals();
        std::process::exit(main_0() as i32);
    }
}
unsafe fn main_0() -> i32 {
    let mut a: Node = Node { x: 1 };
    let mut b: Node = Node { x: 2 };
    let mut m: BTreeMap<*mut Node, Box<i32>> = BTreeMap::new();
    (unsafe { put_0(&mut m, (&mut a as *mut Node), 10) });
    (unsafe { put_0(&mut m, (&mut b as *mut Node), 20) });
    (unsafe { put_0(&mut m, (&mut a as *mut Node), 11) });
    assert!(((m.len()) == (2_usize)));
    assert!(((*m.entry((&mut a as *mut Node)).or_default().as_mut()) == (11)));
    assert!(((*m.entry((&mut b as *mut Node)).or_default().as_mut()) == (20)));
    assert!(
        ((*(m
            .get(&(&mut a as *mut Node))
            .expect("out of range!")
            .as_ref() as *const i32))
            == (11))
    );
    let mut um: BTreeMap<*const Node, Box<i32>> = BTreeMap::new();
    let mut ca: *const Node = (&mut a as *mut Node).cast_const();
    (*um.entry(ca).or_default().as_mut()) = 5;
    assert!(((um.len()) == (1_usize)));
    assert!(((*um.entry(ca).or_default().as_mut()) == (5)));
    return 0;
}
pub unsafe fn __cpp2rust_init_globals() {}
