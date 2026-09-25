extern crate libcc2rs;
use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::io::prelude::*;
use std::io::{Read, Seek, Write};
use std::os::fd::AsFd;
use std::rc::{Rc, Weak};
pub trait Base {
    fn id(&self) -> i32;
}
#[derive(Clone, ByteRepr, Default)]
pub struct Derived {}
impl_deep_clone_leaf!(Derived);
pub fn main() {
    __cpp2rust_init_globals();
    std::process::exit(main_0());
}
fn main_0() -> i32 {
    let x: Value<Derived> = Rc::new(RefCell::new(<Derived>::default()));
    let b: Value<PtrDyn<dyn Base>> =
        Rc::new(RefCell::new((x.as_pointer()).to_dyn::<dyn Base>(|w| w)));
    assert!((({ (*(*b.borrow()).upgrade().deref()).id() }) == 7));
    let d: Value<Vec<PtrDyn<dyn Base>>> = Rc::new(RefCell::new(Vec::new()));
    (*d.borrow_mut()).insert(0, (x.as_pointer()).to_dyn::<dyn Base>(|w| w));
    (*d.borrow_mut()).push((x.as_pointer()).to_dyn::<dyn Base>(|w| w));
    assert!(((*d.borrow()).len() == 2_usize));
    assert!(!((*d.borrow()).is_empty()));
    return 0;
}
impl Base for Derived {
    fn id(&self) -> i32 {
        return 7;
    }
}
pub fn __cpp2rust_init_globals() {}
