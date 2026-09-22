extern crate libc;
use libc::*;
extern crate libcc2rs;
use libcc2rs::*;
use std::collections::BTreeMap;
use std::io::{Read, Seek, Write};
use std::os::fd::{AsFd, FromRawFd, IntoRawFd};
use std::rc::Rc;
pub unsafe fn sum_vec_0(v: *const Vec<i32>) -> i32 {
    let mut n: i32 = 0;
    'loop_: for x in 0..((*v).len()) {
        let mut x = (&(*v))[x].clone();
        n += x;
    }
    return n;
}
pub unsafe fn sum_vec_rvref_1(v: *mut Vec<i32>) -> i32 {
    let mut n: i32 = 0;
    'loop_: for x in 0..((*v).len()) {
        let mut x = (&(*v))[x].clone();
        n += x;
    }
    return n;
}
pub unsafe fn sum_arr_2(a: *const Vec<i32>) -> i32 {
    return ((((&(*a))[(0_usize)]) + ((&(*a))[(1_usize)])) + ((&(*a))[(2_usize)]));
}
#[repr(C)]
#[derive(Copy, Clone, Default)]
pub struct Two {
    pub a: i32,
    pub b: i32,
}
pub unsafe fn sum_two_3(t: *const Two) -> i32 {
    return (((*t).a) + ((*t).b));
}
#[repr(C)]
#[derive(Copy, Clone, Default)]
pub struct One {
    pub x: i32,
}
pub unsafe fn get_one_4(o: *const One) -> i32 {
    return (*o).x;
}
pub fn main() {
    unsafe {
        __cpp2rust_init_globals();
        std::process::exit(main_0() as i32);
    }
}
unsafe fn main_0() -> i32 {
    let mut v: Vec<i32> = vec![1, 2, 3];
    assert!(((unsafe { sum_vec_0(&v,) }) == (6)));
    assert!(((v.len()) == (3_usize)));
    assert!(((unsafe { sum_vec_0(&v,) }) == (6)));
    assert!(((v.len()) == (3_usize)));
    assert!(((unsafe { sum_vec_rvref_1(&mut v,) }) == (6)));
    assert!(((v.len()) == (3_usize)));
    let mut a: Vec<i32> = vec![10, 20, 30];
    assert!(((unsafe { sum_arr_2(&a,) }) == (60)));
    assert!(((unsafe { sum_arr_2(&a,) }) == (60)));
    let mut t: Two = Two { a: 4, b: 5 };
    assert!(((unsafe { sum_two_3(&t,) }) == (9)));
    assert!(
        ((unsafe {
            let mut _t: Two = Two { a: 6, b: 7 };
            sum_two_3(&mut _t)
        }) == (13))
    );
    let mut i: i32 = 8;
    assert!(
        ((unsafe {
            let mut _o: One = One { x: i };
            get_one_4(&mut _o)
        }) == (8))
    );
    let mut o: One = One { x: 9 };
    assert!(((unsafe { get_one_4(&o,) }) == (9)));
    return 0;
}
pub unsafe fn __cpp2rust_init_globals() {}
