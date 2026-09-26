extern crate libcc2rs;
use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::io::prelude::*;
use std::io::{Read, Seek, Write};
use std::os::fd::AsFd;
use std::rc::{Rc, Weak};
thread_local!(
    pub static a_1: Value<i32> = Rc::new(RefCell::new(0_i32));
);
pub fn main() {
    __cpp2rust_init_globals();
    std::process::exit(main_0());
}
fn main_0() -> i32 {
    let next: Value<_> = Rc::new(RefCell::new(
        (|| {
            thread_local!(
                static n_2: Value<i32> = Rc::new(RefCell::new(0));
            );
            return (*n_2.with(Value::clone).borrow_mut()).prefix_inc();
        }),
    ));
    assert!((({ (*next.borrow_mut())() }) == 1));
    assert!((({ (*next.borrow_mut())() }) == 2));
    let copy: Value<_> = Rc::new(RefCell::new((*next.borrow()).clone()));
    assert!((({ (*copy.borrow_mut())() }) == 3));
    assert!((({ (*next.borrow_mut())() }) == 4));
    assert!(
        (({
            (|x: i32| {
                let x: Value<i32> = Rc::new(RefCell::new(x));
                thread_local!(
                    static calls_3: Value<i32> = Rc::new(RefCell::new(0));
                );
                (*calls_3.with(Value::clone).borrow_mut()).postfix_inc();
                return calls_3.with(|rc| *rc.borrow());
            })(1)
        }) == 1)
    );
    assert!(
        (({
            (|x: i32| {
                let x: Value<i32> = Rc::new(RefCell::new(x));
                thread_local!(
                    static calls_3: Value<i32> = Rc::new(RefCell::new(0));
                );
                (*calls_3.with(Value::clone).borrow_mut()).postfix_inc();
                return calls_3.with(|rc| *rc.borrow());
            })(2)
        }) == 2)
    );
    assert!(
        (({
            (|x: f64| {
                let x: Value<f64> = Rc::new(RefCell::new(x));
                thread_local!(
                    static calls_4: Value<i32> = Rc::new(RefCell::new(0));
                );
                (*calls_4.with(Value::clone).borrow_mut()).postfix_inc();
                return calls_4.with(|rc| *rc.borrow());
            })(1.0E+0)
        }) == 1)
    );
    assert!(
        (({
            (|x: i32| {
                let x: Value<i32> = Rc::new(RefCell::new(x));
                thread_local!(
                    static calls_3: Value<i32> = Rc::new(RefCell::new(0));
                );
                (*calls_3.with(Value::clone).borrow_mut()).postfix_inc();
                return calls_3.with(|rc| *rc.borrow());
            })(3)
        }) == 3)
    );
    let read: Value<_> = Rc::new(RefCell::new(
        (|_a0: i32| {
            let _a0: Value<i32> = Rc::new(RefCell::new(_a0));
            return a_1.with(|rc| *rc.borrow());
        }),
    ));
    assert!((({ (*read.borrow_mut())(0,) }) == 0));
    a_1.with(|rc| *rc.borrow_mut() = 1);
    assert!((({ (*read.borrow_mut())(0,) }) == 1));
    let local: Value<_> = Rc::new(RefCell::new(
        (|| {
            let p: Value<P_0> = Rc::new(RefCell::new(P_0 {
                x: Rc::new(RefCell::new(2)),
                y: Rc::new(RefCell::new(3)),
            }));
            return ((*(*p.borrow()).x.borrow()) * (*(*p.borrow()).y.borrow()));
        }),
    ));
    assert!((({ (*local.borrow_mut())() }) == 6));
    return 0;
}
#[derive(Default)]
pub struct P_0 {
    pub x: Value<i32>,
    pub y: Value<i32>,
}
impl Clone for P_0 {
    fn clone(&self) -> Self {
        let __this: Value<P_0> = Rc::new(RefCell::new(Self {
            x: Rc::new(RefCell::new((*self.x.borrow()))),
            y: Rc::new(RefCell::new((*self.y.borrow()))),
        }));
        let this: Ptr<P_0> = __this.as_pointer();
        Rc::try_unwrap(__this).ok().unwrap().into_inner()
    }
}
impl_deep_clone_leaf!(P_0);
impl ByteRepr for P_0 {
    fn byte_size() -> usize {
        8
    }
    fn to_bytes(&self, buf: &mut [u8]) {
        (*self.x.borrow()).to_bytes(&mut buf[0..4]);
        (*self.y.borrow()).to_bytes(&mut buf[4..8]);
    }
    fn from_bytes(buf: &[u8]) -> Self {
        Self {
            x: Rc::new(RefCell::new(<i32>::from_bytes(&buf[0..4]))),
            y: Rc::new(RefCell::new(<i32>::from_bytes(&buf[4..8]))),
        }
    }
}
pub fn __cpp2rust_init_globals() {
    let _ = a_1.with(|_| ());
}
