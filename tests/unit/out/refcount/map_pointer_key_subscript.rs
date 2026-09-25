extern crate libcc2rs;
use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::io::prelude::*;
use std::io::{Read, Seek, Write};
use std::os::fd::AsFd;
use std::rc::{Rc, Weak};
#[derive(Default)]
pub struct Node {
    pub x: Value<i32>,
}
impl Clone for Node {
    fn clone(&self) -> Self {
        let __this: Value<Node> = Rc::new(RefCell::new(Self {
            x: Rc::new(RefCell::new((*self.x.borrow()))),
        }));
        let this: Ptr<Node> = __this.as_pointer();
        Rc::try_unwrap(__this).ok().unwrap().into_inner()
    }
}
impl_deep_clone_leaf!(Node);
impl ByteRepr for Node {
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
pub fn put_0(m: Ptr<BTreeMap<Ptr<Node>, Value<i32>>>, k: Ptr<Node>, v: i32) {
    let k: Value<Ptr<Node>> = Rc::new(RefCell::new(k));
    let v: Value<i32> = Rc::new(RefCell::new(v));
    ((m).clone() as Ptr<BTreeMap<Ptr<Node>, Value<i32>>>)
        .with_mut(|__v: &mut BTreeMap<Ptr<Node>, Value<i32>>| {
            __v.entry((*k.borrow()).clone())
                .or_insert_with(|| Rc::new(RefCell::new(<i32>::default())))
                .as_pointer()
        })
        .write((*v.borrow()));
}
pub fn main() {
    __cpp2rust_init_globals();
    std::process::exit(main_0());
}
fn main_0() -> i32 {
    let a: Value<Node> = Rc::new(RefCell::new(Node {
        x: Rc::new(RefCell::new(1)),
    }));
    let b: Value<Node> = Rc::new(RefCell::new(Node {
        x: Rc::new(RefCell::new(2)),
    }));
    let m: Value<BTreeMap<Ptr<Node>, Value<i32>>> = Rc::new(RefCell::new(BTreeMap::new()));
    ({ put_0(m.as_pointer(), (a.as_pointer()), 10) });
    ({ put_0(m.as_pointer(), (b.as_pointer()), 20) });
    ({ put_0(m.as_pointer(), (a.as_pointer()), 11) });
    assert!(((*m.borrow()).len() == 2_usize));
    assert!(
        (((m.as_pointer() as Ptr<BTreeMap<Ptr<Node>, Value<i32>>>)
            .with_mut(|__v: &mut BTreeMap<Ptr<Node>, Value<i32>>| {
                __v.entry((a.as_pointer()))
                    .or_insert_with(|| Rc::new(RefCell::new(<i32>::default())))
                    .as_pointer()
            })
            .read())
            == 11)
    );
    assert!(
        (((m.as_pointer() as Ptr<BTreeMap<Ptr<Node>, Value<i32>>>)
            .with_mut(|__v: &mut BTreeMap<Ptr<Node>, Value<i32>>| {
                __v.entry((b.as_pointer()))
                    .or_insert_with(|| Rc::new(RefCell::new(<i32>::default())))
                    .as_pointer()
            })
            .read())
            == 20)
    );
    assert!(
        (((*m.borrow())
            .get(&(a.as_pointer()))
            .expect("out of range!")
            .as_pointer()
            .read())
            == 11)
    );
    let um: Value<BTreeMap<Ptr<Node>, Value<i32>>> = Rc::new(RefCell::new(BTreeMap::new()));
    let ca: Value<Ptr<Node>> = Rc::new(RefCell::new((a.as_pointer())));
    (um.as_pointer() as Ptr<BTreeMap<Ptr<Node>, Value<i32>>>)
        .with_mut(|__v: &mut BTreeMap<Ptr<Node>, Value<i32>>| {
            __v.entry((*ca.borrow()).clone())
                .or_insert_with(|| Rc::new(RefCell::new(<i32>::default())))
                .as_pointer()
        })
        .write(5);
    assert!(((*um.borrow()).len() == 1_usize));
    assert!(
        (((um.as_pointer() as Ptr<BTreeMap<Ptr<Node>, Value<i32>>>)
            .with_mut(|__v: &mut BTreeMap<Ptr<Node>, Value<i32>>| {
                __v.entry((*ca.borrow()).clone())
                    .or_insert_with(|| Rc::new(RefCell::new(<i32>::default())))
                    .as_pointer()
            })
            .read())
            == 5)
    );
    return 0;
}
pub fn __cpp2rust_init_globals() {}
