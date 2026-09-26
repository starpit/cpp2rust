extern crate libcc2rs;
use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::io::prelude::*;
use std::io::{Read, Seek, Write};
use std::os::fd::AsFd;
use std::rc::{Rc, Weak};
#[derive(Default)]
pub struct Val {
    pub x: Value<i32>,
}
impl Clone for Val {
    fn clone(&self) -> Self {
        let __this: Value<Val> = Rc::new(RefCell::new(Self {
            x: Rc::new(RefCell::new((*self.x.borrow()))),
        }));
        let this: Ptr<Val> = __this.as_pointer();
        Rc::try_unwrap(__this).ok().unwrap().into_inner()
    }
}
impl_deep_clone_leaf!(Val);
impl ByteRepr for Val {
    fn byte_size() -> usize {
        4
    }
    fn to_bytes(&self, buf: &mut [u8]) {
        (*self.x.borrow()).to_bytes(&mut buf[0..4]);
    }
    fn from_bytes(buf: &[u8]) -> Self {
        Self {
            x: Rc::new(RefCell::new(<i32>::from_bytes(&buf[0..4]))),
        }
    }
}
pub fn sum_0(a: Val, b: Val) -> i32 {
    let a: Value<Val> = Rc::new(RefCell::new(a));
    let b: Value<Val> = Rc::new(RefCell::new(b));
    return ((*(*a.borrow()).x.borrow()) + (*(*b.borrow()).x.borrow()));
}
pub fn main() {
    __cpp2rust_init_globals();
    std::process::exit(main_0());
}
fn main_0() -> i32 {
    let total: Value<i32> = Rc::new(RefCell::new(0));
    ({
        (|| {
            {
                let rhs_0 = (((*total.borrow()) as usize).wrapping_add(
                    ((::std::mem::size_of::<u8>() as usize)
                        .wrapping_add((::std::mem::size_of::<u8>() as usize))
                        as usize),
                )) as i32;
                (*total.borrow_mut()) = rhs_0
            };
        })
        .operator_call_char_char_const()
    });
    ({
        (|| {
            {
                let rhs_0 = (((*total.borrow()) as usize).wrapping_add(
                    ((::std::mem::size_of::<i32>() as usize)
                        .wrapping_add((::std::mem::size_of::<u8>() as usize))
                        as usize),
                )) as i32;
                (*total.borrow_mut()) = rhs_0
            };
        })
        .operator_call_int_char_const()
    });
    assert!(((*total.borrow()) == 7));
    let v: Value<Val> = Rc::new(RefCell::new(Val {
        x: Rc::new(RefCell::new(5)),
    }));
    let acc: Value<i32> = Rc::new(RefCell::new(0));
    ({
        (|| {
            (*acc.borrow_mut()) += ({
                let _a: Val = (*v.borrow()).clone();
                let _b: Val = (*v.borrow()).clone();
                sum_0(_a, _b)
            });
        })
        .operator_call_struct_Val_ref_const()
    });
    ({
        (|| {
            (*acc.borrow_mut()) += ({
                let _a: Val = (*v.borrow()).clone();
                let _b: Val = (*v.borrow()).clone();
                sum_0(_a, _b)
            });
        })
        .operator_call_const_struct_Val_ref_const()
    });
    ({
        (|| {
            (*acc.borrow_mut()) += ({
                let _a: Val = (*v.borrow()).clone();
                let _b: Val = (*v.borrow()).clone();
                sum_0(_a, _b)
            });
        })
        .operator_call_struct_Val_refref_const()
    });
    assert!(((*acc.borrow()) == 30));
    assert!(
        (({
            (|x: i32| {
                let x: Value<i32> = Rc::new(RefCell::new(x));
                return (((*x.borrow()) as i32) / 2);
            })
            .operator_call_i32__int_const(5)
        }) == 2)
    );
    assert!(
        (({
            (|x: i32| {
                let x: Value<i32> = Rc::new(RefCell::new(x));
                return (((*x.borrow()) as f64) / 2_f64);
            })
            .operator_call_i32__double_const(5)
        }) == 2.5E+0)
    );
    return 0;
}
pub fn __cpp2rust_init_globals() {}
