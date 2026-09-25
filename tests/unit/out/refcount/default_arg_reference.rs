extern crate libcc2rs;
use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::io::prelude::*;
use std::io::{Read, Seek, Write};
use std::os::fd::AsFd;
use std::rc::{Rc, Weak};
thread_local!(
    pub static called_0: Value<i32> = Rc::new(RefCell::new(0));
);
pub fn make_1() -> Vec<i64> {
    (*called_0.with(Value::clone).borrow_mut()).prefix_inc();
    return vec![1_i64, 2_i64];
}
pub fn sum_2(values: Option<Ptr<Vec<i64>>>) -> i64 {
    let __dflt_values: Value<Vec<i64>>;
    let values: Ptr<Vec<i64>> = match values {
        Some(__p) => __p,
        None => {
            __dflt_values = Rc::new(RefCell::new(Vec::new()));
            __dflt_values.as_pointer()
        }
    };
    let acc: Value<i64> = Rc::new(RefCell::new(
        (((*values.upgrade().deref()).len() as i64) * 1000_i64),
    ));
    let i: Value<u32> = Rc::new(RefCell::new(0_u32));
    'loop_: while {
        let _lhs = ((*i.borrow()) as usize);
        _lhs < (*values.upgrade().deref()).len()
    } {
        (*acc.borrow_mut()) += ((values.decay() as Ptr<i64>)
            .offset(((*i.borrow()) as usize))
            .read());
        (*i.borrow_mut()).prefix_inc();
    }
    return (*acc.borrow());
}
pub fn len_3(name: Option<Ptr<Vec<u8>>>) -> i32 {
    let __dflt_name: Value<Vec<u8>>;
    let name: Ptr<Vec<u8>> = match name {
        Some(__p) => __p,
        None => {
            __dflt_name = Rc::new(RefCell::new({
                let mut __bytes = Ptr::<u8>::from_string_literal(b"abc").to_c_bytes();
                __bytes.push(0);
                __bytes
            }));
            __dflt_name.as_pointer()
        }
    };
    return (((*name.upgrade().deref()).len() - 1) as i32);
}
pub fn scalar_4(k: Option<Ptr<i64>>) -> i64 {
    let __dflt_k: Value<i64>;
    let k: Ptr<i64> = match k {
        Some(__p) => __p,
        None => {
            __dflt_k = Rc::new(RefCell::new(7_i64));
            __dflt_k.as_pointer()
        }
    };
    return (k.read());
}
pub fn lazy_5(values: Option<Ptr<Vec<i64>>>) -> i64 {
    let __dflt_values: Value<Vec<i64>>;
    let values: Ptr<Vec<i64>> = match values {
        Some(__p) => __p,
        None => {
            __dflt_values = Rc::new(RefCell::new(({ make_1() })));
            __dflt_values.as_pointer()
        }
    };
    return ((*values.upgrade().deref()).len() as i64);
}
thread_local!(
    pub static shared_6: Value<Vec<i64>> = Rc::new(RefCell::new(vec![9_i64]));
);
pub fn grow_7(values: Option<Ptr<Vec<i64>>>) {
    let values: Ptr<Vec<i64>> = match values {
        Some(__p) => __p,
        None => shared_6.with(|v| v.as_pointer()),
    };
    values.with_mut(|__v: &mut Vec<i64>| __v.push(1_i64));
}
#[derive()]
pub struct Holder {
    pub n: Value<i64>,
}
impl Holder {
    pub fn Holder(values: Option<Ptr<Vec<i64>>>) -> Self {
        let __dflt_values: Value<Vec<i64>>;
        let values: Ptr<Vec<i64>> = match values {
            Some(__p) => __p,
            None => {
                __dflt_values = Rc::new(RefCell::new(Vec::new()));
                __dflt_values.as_pointer()
            }
        };
        let __this: Value<Holder> = Rc::new(RefCell::new(Self {
            n: <Value<i64>>::default(),
        }));
        let this: Ptr<Holder> = __this.as_pointer();
        (*(*this.upgrade().deref()).n.borrow_mut()) = ((*values.upgrade().deref()).len() as i64);
        Rc::try_unwrap(__this).ok().unwrap().into_inner()
    }
}
impl Clone for Holder {
    fn clone(&self) -> Self {
        let __this: Value<Holder> = Rc::new(RefCell::new(Self {
            n: Rc::new(RefCell::new((*self.n.borrow()))),
        }));
        let this: Ptr<Holder> = __this.as_pointer();
        Rc::try_unwrap(__this).ok().unwrap().into_inner()
    }
}
impl Default for Holder {
    fn default() -> Self {
        { Holder::Holder(None) }
    }
}
impl ByteRepr for Holder {
    fn byte_size() -> usize {
        8
    }
    fn to_bytes(&self, buf: &mut [u8]) {
        (*self.n.borrow()).to_bytes(&mut buf[0..8]);
    }
    fn from_bytes(buf: &[u8]) -> Self {
        Self {
            n: Rc::new(RefCell::new(<i64>::from_bytes(&buf[0..8]))),
        }
    }
}
pub fn main() {
    __cpp2rust_init_globals();
    std::process::exit(main_0());
}
fn main_0() -> i32 {
    assert!((({ sum_2(None,) }) == 0_i64));
    let x: Value<Vec<i64>> = Rc::new(RefCell::new(vec![4_i64, 5_i64, 6_i64]));
    assert!((({ sum_2(Some(x.as_pointer()),) }) == 3015_i64));
    assert!((({ len_3(None,) }) == 3));
    let y: Value<Vec<u8>> = Rc::new(RefCell::new({
        let mut __bytes = Ptr::<u8>::from_string_literal(b"hi").to_c_bytes();
        __bytes.push(0);
        __bytes
    }));
    assert!((({ len_3(Some(y.as_pointer()),) }) == 2));
    assert!((({ scalar_4(None,) }) == 7_i64));
    let k: Value<i64> = Rc::new(RefCell::new(42_i64));
    assert!((({ scalar_4(Some(k.as_pointer()),) }) == 42_i64));
    assert!((called_0.with(|rc| *rc.borrow()) == 0));
    assert!((({ lazy_5(Some(x.as_pointer()),) }) == 3_i64));
    assert!((called_0.with(|rc| *rc.borrow()) == 0));
    assert!((({ lazy_5(None,) }) == 2_i64));
    assert!((called_0.with(|rc| *rc.borrow()) == 1));
    ({ grow_7(None) });
    assert!((shared_6.with(|rc| rc.borrow().clone()).len() == 2_usize));
    let own: Value<Vec<i64>> = Rc::new(RefCell::new(Vec::new()));
    ({ grow_7(Some(own.as_pointer())) });
    assert!(((*own.borrow()).len() == 1_usize));
    assert!((shared_6.with(|rc| rc.borrow().clone()).len() == 2_usize));
    let a: Value<Holder> = Rc::new(RefCell::new(Holder::Holder(None)));
    assert!(((*(*a.borrow()).n.borrow()) == 0_i64));
    let b: Value<Holder> = Rc::new(RefCell::new(Holder::Holder({ Some(x.as_pointer()) })));
    assert!(((*(*b.borrow()).n.borrow()) == 3_i64));
    return 0;
}
pub fn __cpp2rust_init_globals() {
    let _ = called_0.with(|_| ());
    let _ = shared_6.with(|_| ());
}
