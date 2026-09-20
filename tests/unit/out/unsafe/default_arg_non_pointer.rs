extern crate libc;
use libc::*;
extern crate libcc2rs;
use libcc2rs::*;
use std::collections::BTreeMap;
use std::io::{Read, Seek, Write};
use std::os::fd::{AsFd, FromRawFd, IntoRawFd};
use std::rc::Rc;
pub unsafe fn tag_0(mut v: i32, mut suffix: Option<Vec<libc::c_char>>) -> i32 {
    let mut suffix: Vec<libc::c_char> = suffix.unwrap_or({
        let s = c"".as_ptr();
        std::slice::from_raw_parts(s, (0..).take_while(|&i| *s.add(i) != 0).count() + 1).to_vec()
    });
    return ((v) + ((suffix.len() - 1) as i32));
}
#[repr(C)]
#[derive(Copy, Clone, Default)]
pub struct Holder {
    pub n: i32,
}
impl Holder {
    pub unsafe fn Holder(mut n: i32, mut name: Option<Vec<libc::c_char>>) -> Self {
        let mut name: Vec<libc::c_char> = name.unwrap_or({
            let s = c"anon".as_ptr();
            std::slice::from_raw_parts(s, (0..).take_while(|&i| *s.add(i) != 0).count() + 1)
                .to_vec()
        });
        let mut this = Self {
            n: ((n) + ((name.len() - 1) as i32)),
        };
        this
    }
}
pub fn main() {
    unsafe {
        __cpp2rust_init_globals();
        std::process::exit(main_0() as i32);
    }
}
unsafe fn main_0() -> i32 {
    assert!(((unsafe { tag_0(1, None,) }) == (1)));
    assert!(
        ((unsafe {
            tag_0(
                1,
                Some({
                    let s = c"abc".as_ptr();
                    std::slice::from_raw_parts(s, (0..).take_while(|&i| *s.add(i) != 0).count() + 1)
                        .to_vec()
                }),
            )
        }) == (4))
    );
    let mut a: Holder = Holder::Holder({ 0 }, None);
    assert!(((a.n) == (4)));
    let mut b: Holder = Holder::Holder({ 0 }, {
        Some({
            let s = c"xy".as_ptr();
            std::slice::from_raw_parts(s, (0..).take_while(|&i| *s.add(i) != 0).count() + 1)
                .to_vec()
        })
    });
    assert!(((b.n) == (2)));
    return 0;
}
pub unsafe fn __cpp2rust_init_globals() {}
