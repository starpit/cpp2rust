extern crate libc;
use libc::*;
extern crate libcc2rs;
use libcc2rs::*;
use std::collections::BTreeMap;
use std::io::{Read, Seek, Write};
use std::os::fd::{AsFd, FromRawFd, IntoRawFd};
use std::rc::Rc;
pub unsafe fn found_0(mut pos: usize) -> bool {
    return ((pos) != (18446744073709551615_usize));
}
pub fn main() {
    unsafe {
        __cpp2rust_init_globals();
        std::process::exit(main_0() as i32);
    }
}
unsafe fn main_0() -> i32 {
    let mut n: usize = 18446744073709551615_usize;
    assert!(((n) != (0_usize)));
    assert!(((n) == (-1_i32 as usize)));
    assert!(!(unsafe { found_0(n,) }));
    assert!((unsafe { found_0(3_usize,) }));
    let mut m: usize = 18446744073709551615_usize;
    m = (m).wrapping_sub(1_usize);
    assert!(((m) == (-2_i32 as usize)));
    return 0;
}
pub unsafe fn __cpp2rust_init_globals() {}
