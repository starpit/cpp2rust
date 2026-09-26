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
pub struct Val {
    pub x: i32,
}
pub unsafe fn sum_0(mut a: Val, mut b: Val) -> i32 {
    return ((a.x) + (b.x));
}
pub fn main() {
    unsafe {
        __cpp2rust_init_globals();
        std::process::exit(main_0() as i32);
    }
}
unsafe fn main_0() -> i32 {
    let mut total: i32 = 0;
    (unsafe {
        (|| {
            total = ((total as usize).wrapping_add(
                ((::std::mem::size_of::<libc::c_char>() as usize)
                    .wrapping_add((::std::mem::size_of::<libc::c_char>() as usize))
                    as usize),
            )) as i32;
        })
        .operator_call_char_char_const()
    });
    (unsafe {
        (|| {
            total = ((total as usize).wrapping_add(
                ((::std::mem::size_of::<i32>() as usize)
                    .wrapping_add((::std::mem::size_of::<libc::c_char>() as usize))
                    as usize),
            )) as i32;
        })
        .operator_call_int_char_const()
    });
    assert!(((total) == (7)));
    let mut v: Val = Val { x: 5 };
    let mut acc: i32 = 0;
    (unsafe {
        (|| {
            acc += (unsafe {
                let _a: Val = v;
                let _b: Val = v;
                sum_0(_a, _b)
            });
        })
        .operator_call_struct_Val_ref_const()
    });
    (unsafe {
        (|| {
            acc += (unsafe {
                let _a: Val = v;
                let _b: Val = v;
                sum_0(_a, _b)
            });
        })
        .operator_call_const_struct_Val_ref_const()
    });
    (unsafe {
        (|| {
            acc += (unsafe {
                let _a: Val = v;
                let _b: Val = v;
                sum_0(_a, _b)
            });
        })
        .operator_call_struct_Val_refref_const()
    });
    assert!(((acc) == (30)));
    assert!(
        ((unsafe {
            (|x: i32| {
                return ((x as i32) / (2));
            })
            .operator_call_i32__int_const(5)
        }) == (2))
    );
    assert!(
        ((unsafe {
            (|x: i32| {
                return ((x as f64) / (2_f64));
            })
            .operator_call_i32__double_const(5)
        }) == (2.5E+0))
    );
    return 0;
}
pub unsafe fn __cpp2rust_init_globals() {}
