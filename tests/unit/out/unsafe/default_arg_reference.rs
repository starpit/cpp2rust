extern crate libc;
use libc::*;
extern crate libcc2rs;
use libcc2rs::*;
use std::collections::BTreeMap;
use std::io::{Read, Seek, Write};
use std::os::fd::{AsFd, FromRawFd, IntoRawFd};
use std::rc::Rc;
pub static mut called_0: std::cell::LazyCell<i32> = std::cell::LazyCell::new(|| unsafe { 0 });
pub unsafe fn make_1() -> Vec<i64> {
    (*std::cell::LazyCell::force_mut(&mut *&raw mut called_0)).prefix_inc();
    return vec![1_i64, 2_i64];
}
pub unsafe fn sum_2(values: Option<*const Vec<i64>>) -> i64 {
    let mut __dflt_values: Vec<i64>;
    let mut values: *const Vec<i64> = match values {
        Some(__p) => __p,
        None => {
            __dflt_values = Vec::new();
            &mut __dflt_values
        }
    };
    let mut acc: i64 = (((*values).len() as i64) * (1000_i64));
    let mut i: u32 = 0_u32;
    'loop_: while ((i as usize) < ((*values).len())) {
        acc += (&(*values))[(i as usize)];
        i.prefix_inc();
    }
    return acc;
}
pub unsafe fn len_3(name: Option<*const Vec<libc::c_char>>) -> i32 {
    let mut __dflt_name: Vec<libc::c_char>;
    let mut name: *const Vec<libc::c_char> = match name {
        Some(__p) => __p,
        None => {
            __dflt_name = {
                let s = c"abc".as_ptr();
                std::slice::from_raw_parts(s, (0..).take_while(|&i| *s.add(i) != 0).count() + 1)
                    .to_vec()
            };
            &mut __dflt_name
        }
    };
    return (((*name).len() - 1) as i32);
}
pub unsafe fn scalar_4(k: Option<*const i64>) -> i64 {
    let mut __dflt_k: i64;
    let mut k: *const i64 = match k {
        Some(__p) => __p,
        None => {
            __dflt_k = 7_i64;
            &mut __dflt_k
        }
    };
    return (*k);
}
pub unsafe fn lazy_5(values: Option<*const Vec<i64>>) -> i64 {
    let mut __dflt_values: Vec<i64>;
    let mut values: *const Vec<i64> = match values {
        Some(__p) => __p,
        None => {
            __dflt_values = (unsafe { make_1() });
            &mut __dflt_values
        }
    };
    return ((*values).len() as i64);
}
pub static mut shared_6: std::cell::LazyCell<Vec<i64>> =
    std::cell::LazyCell::new(|| unsafe { vec![9_i64] });
pub unsafe fn grow_7(values: Option<*mut Vec<i64>>) {
    let mut values: *mut Vec<i64> = match values {
        Some(__p) => __p,
        None => &mut (*std::cell::LazyCell::force_mut(&mut *&raw mut shared_6)),
    };
    (*values).push(1_i64);
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct Holder {
    pub n: i64,
}
impl Holder {
    pub unsafe fn new(values: Option<*const Vec<i64>>) -> Self {
        let mut __dflt_values: Vec<i64>;
        let mut values: *const Vec<i64> = match values {
            Some(__p) => __p,
            None => {
                __dflt_values = Vec::new();
                &mut __dflt_values
            }
        };
        let mut this = Self { n: 0_i64 };
        this.n = ((*values).len() as i64);
        this
    }
}
impl Default for Holder {
    fn default() -> Self {
        unsafe { Holder::new(None) }
    }
}
pub fn main() {
    unsafe {
        __cpp2rust_init_globals();
        std::process::exit(main_0() as i32);
    }
}
unsafe fn main_0() -> i32 {
    assert!(((unsafe { sum_2(None,) }) == (0_i64)));
    let mut x: Vec<i64> = vec![4_i64, 5_i64, 6_i64];
    assert!(((unsafe { sum_2(Some(&x),) }) == (3015_i64)));
    assert!(((unsafe { len_3(None,) }) == (3)));
    let mut y: Vec<libc::c_char> = {
        let s = c"hi".as_ptr();
        std::slice::from_raw_parts(s, (0..).take_while(|&i| *s.add(i) != 0).count() + 1).to_vec()
    };
    assert!(((unsafe { len_3(Some(&y),) }) == (2)));
    assert!(((unsafe { scalar_4(None,) }) == (7_i64)));
    let mut k: i64 = 42_i64;
    assert!(((unsafe { scalar_4(Some(&k),) }) == (42_i64)));
    assert!(((*std::cell::LazyCell::force_mut(&mut *&raw mut called_0)) == (0)));
    assert!(((unsafe { lazy_5(Some(&x),) }) == (3_i64)));
    assert!(((*std::cell::LazyCell::force_mut(&mut *&raw mut called_0)) == (0)));
    assert!(((unsafe { lazy_5(None,) }) == (2_i64)));
    assert!(((*std::cell::LazyCell::force_mut(&mut *&raw mut called_0)) == (1)));
    (unsafe { grow_7(None) });
    assert!((((*std::cell::LazyCell::force_mut(&mut *&raw mut shared_6)).len()) == (2_usize)));
    let mut own: Vec<i64> = Vec::new();
    (unsafe { grow_7(Some(&mut own)) });
    assert!(((own.len()) == (1_usize)));
    assert!((((*std::cell::LazyCell::force_mut(&mut *&raw mut shared_6)).len()) == (2_usize)));
    let mut a: Holder = Holder::new(None);
    assert!(((a.n) == (0_i64)));
    let mut b: Holder = Holder::new({ Some(&x) });
    assert!(((b.n) == (3_i64)));
    return 0;
}
pub unsafe fn __cpp2rust_init_globals() {
    std::cell::LazyCell::force(&*&raw const called_0);
    std::cell::LazyCell::force(&*&raw const shared_6);
}
