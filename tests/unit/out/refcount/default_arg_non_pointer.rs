extern crate libcc2rs;
use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::io::prelude::*;
use std::io::{Read, Seek, Write};
use std::os::fd::AsFd;
use std::rc::{Rc, Weak};
pub fn tag_0(v: i32, suffix: Option<Vec<u8>>) -> i32 {
    let v: Value<i32> = Rc::new(RefCell::new(v));
    let suffix: Value<Vec<u8>> = Rc::new(RefCell::new(
        suffix.unwrap_or(
            Ptr::<u8>::from_string_literal(b"")
                .to_c_string_iterator()
                .chain(std::iter::once(0))
                .collect::<Vec<u8>>(),
        ),
    ));
    return ((*v.borrow()) + (((*suffix.borrow()).len() - 1) as i32));
}
#[derive(Default)]
pub struct Holder {
    pub n: Value<i32>,
}
impl Holder {
    pub fn Holder(n: i32, name: Option<Vec<u8>>) -> Self {
        let n: Value<i32> = Rc::new(RefCell::new(n));
        let name: Value<Vec<u8>> = Rc::new(RefCell::new(
            name.unwrap_or(
                Ptr::<u8>::from_string_literal(b"anon")
                    .to_c_string_iterator()
                    .chain(std::iter::once(0))
                    .collect::<Vec<u8>>(),
            ),
        ));
        let __this: Value<Holder> = Rc::new(RefCell::new(Self {
            n: Rc::new(RefCell::new(
                ((*n.borrow()) + (((*name.borrow()).len() - 1) as i32)),
            )),
        }));
        let this: Ptr<Holder> = __this.as_pointer();
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
impl ByteRepr for Holder {
    fn byte_size() -> usize {
        4
    }
    fn to_bytes(&self, buf: &mut [u8]) {
        (*self.n.borrow()).to_bytes(&mut buf[0..4]);
    }
    fn from_bytes(buf: &[u8]) -> Self {
        Self {
            n: Rc::new(RefCell::new(<i32>::from_bytes(&buf[0..4]))),
        }
    }
}
pub fn main() {
    __cpp2rust_init_globals();
    std::process::exit(main_0());
}
fn main_0() -> i32 {
    assert!((({ tag_0(1, None,) }) == 1));
    assert!(
        (({
            tag_0(
                1,
                Some(
                    Ptr::<u8>::from_string_literal(b"abc")
                        .to_c_string_iterator()
                        .chain(std::iter::once(0))
                        .collect::<Vec<u8>>(),
                ),
            )
        }) == 4)
    );
    let a: Value<Holder> = Rc::new(RefCell::new(Holder::Holder({ 0 }, None)));
    assert!(((*(*a.borrow()).n.borrow()) == 4));
    let b: Value<Holder> = Rc::new(RefCell::new(Holder::Holder({ 0 }, {
        Some(
            Ptr::<u8>::from_string_literal(b"xy")
                .to_c_string_iterator()
                .chain(std::iter::once(0))
                .collect::<Vec<u8>>(),
        )
    })));
    assert!(((*(*b.borrow()).n.borrow()) == 2));
    return 0;
}
pub fn __cpp2rust_init_globals() {}
