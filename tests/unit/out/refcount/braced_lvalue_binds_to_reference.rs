extern crate libcc2rs;
use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::io::prelude::*;
use std::io::{Read, Seek, Write};
use std::os::fd::AsFd;
use std::rc::{Rc, Weak};
pub fn sum_vec_0(v: Ptr<Vec<i32>>) -> i32 {
    let n: Value<i32> = Rc::new(RefCell::new(0));
    'loop_: for mut x in v.decay() as Ptr<i32> {
        let x: Value<i32> = Rc::new(RefCell::new(x.read()));
        (*n.borrow_mut()) += (*x.borrow());
    }
    return (*n.borrow());
}
pub fn sum_vec_rvref_1(v: Ptr<Vec<i32>>) -> i32 {
    let n: Value<i32> = Rc::new(RefCell::new(0));
    'loop_: for mut x in v.decay() as Ptr<i32> {
        let x: Value<i32> = Rc::new(RefCell::new(x.read()));
        (*n.borrow_mut()) += (*x.borrow());
    }
    return (*n.borrow());
}
pub fn sum_arr_2(a: Ptr<Vec<i32>>) -> i32 {
    return ((((a.decay() as Ptr<i32>).offset(0_usize).read())
        + ((a.decay() as Ptr<i32>).offset(1_usize).read()))
        + ((a.decay() as Ptr<i32>).offset(2_usize).read()));
}
#[derive(Default)]
pub struct Two {
    pub a: Value<i32>,
    pub b: Value<i32>,
}
impl Clone for Two {
    fn clone(&self) -> Self {
        let __this: Value<Two> = Rc::new(RefCell::new(Self {
            a: Rc::new(RefCell::new((*self.a.borrow()))),
            b: Rc::new(RefCell::new((*self.b.borrow()))),
        }));
        let this: Ptr<Two> = __this.as_pointer();
        Rc::try_unwrap(__this).ok().unwrap().into_inner()
    }
}
impl ByteRepr for Two {
    fn byte_size() -> usize {
        8
    }
    fn to_bytes(&self, buf: &mut [u8]) {
        (*self.a.borrow()).to_bytes(&mut buf[0..4]);
        (*self.b.borrow()).to_bytes(&mut buf[4..8]);
    }
    fn from_bytes(buf: &[u8]) -> Self {
        Self {
            a: Rc::new(RefCell::new(<i32>::from_bytes(&buf[0..4]))),
            b: Rc::new(RefCell::new(<i32>::from_bytes(&buf[4..8]))),
        }
    }
}
pub fn sum_two_3(t: Ptr<Two>) -> i32 {
    return {
        let _lhs = (*(*t.upgrade().deref()).a.borrow());
        _lhs + (*(*t.upgrade().deref()).b.borrow())
    };
}
#[derive(Default)]
pub struct One {
    pub x: Value<i32>,
}
impl Clone for One {
    fn clone(&self) -> Self {
        let __this: Value<One> = Rc::new(RefCell::new(Self {
            x: Rc::new(RefCell::new((*self.x.borrow()))),
        }));
        let this: Ptr<One> = __this.as_pointer();
        Rc::try_unwrap(__this).ok().unwrap().into_inner()
    }
}
impl ByteRepr for One {
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
pub fn get_one_4(o: Ptr<One>) -> i32 {
    return (*(*o.upgrade().deref()).x.borrow());
}
pub fn main() {
    __cpp2rust_init_globals();
    std::process::exit(main_0());
}
fn main_0() -> i32 {
    let v: Value<Vec<i32>> = Rc::new(RefCell::new(vec![1, 2, 3]));
    assert!((({ sum_vec_0(v.as_pointer(),) }) == 6));
    assert!(((*v.borrow()).len() == 3_usize));
    assert!((({ sum_vec_0(v.as_pointer(),) }) == 6));
    assert!(((*v.borrow()).len() == 3_usize));
    assert!((({ sum_vec_rvref_1(v.as_pointer(),) }) == 6));
    assert!(((*v.borrow()).len() == 3_usize));
    let a: Value<Vec<i32>> = Rc::new(RefCell::new(vec![10, 20, 30]));
    assert!((({ sum_arr_2(a.as_pointer(),) }) == 60));
    assert!((({ sum_arr_2(a.as_pointer(),) }) == 60));
    let t: Value<Two> = Rc::new(RefCell::new(Two {
        a: Rc::new(RefCell::new(4)),
        b: Rc::new(RefCell::new(5)),
    }));
    assert!((({ sum_two_3(t.as_pointer(),) }) == 9));
    assert!(
        (({
            let _t: Value<Two> = Rc::new(RefCell::new(Two {
                a: Rc::new(RefCell::new(6)),
                b: Rc::new(RefCell::new(7)),
            }));
            sum_two_3(_t.as_pointer())
        }) == 13)
    );
    let i: Value<i32> = Rc::new(RefCell::new(8));
    assert!(
        (({
            let _o: Value<One> = Rc::new(RefCell::new(One {
                x: Rc::new(RefCell::new((*i.borrow()))),
            }));
            get_one_4(_o.as_pointer())
        }) == 8)
    );
    let o: Value<One> = Rc::new(RefCell::new(One {
        x: Rc::new(RefCell::new(9)),
    }));
    assert!((({ get_one_4(o.as_pointer(),) }) == 9));
    return 0;
}
pub fn __cpp2rust_init_globals() {}
