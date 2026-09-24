extern crate libcc2rs;
use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::io::prelude::*;
use std::io::{Read, Seek, Write};
use std::os::fd::AsFd;
use std::rc::{Rc, Weak};
pub fn main() {
    __cpp2rust_init_globals();
    std::process::exit(main_0());
}
fn main_0() -> i32 {
    let vec_: Value<Vec<u8>> = Rc::new(RefCell::new(vec![195_u8, 167_u8]));
    let i: Value<i32> = Rc::new(RefCell::new(27));
    let str: Value<Vec<u8>> = Rc::new(RefCell::new({
        let mut __bytes = Ptr::<u8>::from_string_literal(b"bar.").to_c_bytes();
        __bytes.push(0);
        __bytes
    }));
    libcc2rs::cc2_insert_int(&libcc2rs::cerr(), (*i.borrow()) as i128, 4, true);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), b" a");
    libcc2rs::cc2_insert_bytes(
        &libcc2rs::cerr(),
        &[((vec_.as_pointer() as Ptr<u8>).offset(0_usize).read()) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &libcc2rs::cerr(),
        &[((vec_.as_pointer() as Ptr<u8>).offset(1_usize).read()) as u8],
    );
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('o' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(
        &libcc2rs::cerr(),
        &(*str.borrow())
            .iter()
            .take((*str.borrow()).len() - 1)
            .map(|&c| c as u8)
            .collect::<Vec<u8>>()[..],
    );
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), b"\n");
    libcc2rs::cc2_manip_refcount(
        &libcc2rs::cerr(),
        (libcc2rs::hex_refcount as fn(Ptr<u32>) -> Ptr<u32>),
    );
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), b"0x");
    libcc2rs::cc2_insert_int(&libcc2rs::cerr(), (27) as i128, 4, true);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), b" a\xc3\xa7ordas?");
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('\n' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), b"Sim, 0x");
    libcc2rs::cc2_insert_int(&libcc2rs::cerr(), (*i.borrow()) as i128, 4, true);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('.' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), b"\n");
    libcc2rs::cc2_manip_refcount(
        &libcc2rs::cerr(),
        (libcc2rs::dec_refcount as fn(Ptr<u32>) -> Ptr<u32>),
    );
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('H' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('e' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('l' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('l' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('o' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[(',' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[(' ' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('W' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('o' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('r' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('l' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('d' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('!' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('\n' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(
        &libcc2rs::cerr(),
        &[((vec_.as_pointer() as Ptr<u8>).offset(0_usize).read()) as u8],
    );
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('\n' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(
        &libcc2rs::cerr(),
        &[((vec_.as_pointer() as Ptr<u8>).offset(1_usize).read()) as u8],
    );
    libcc2rs::cc2_insert_bytes(&libcc2rs::cerr(), &[('\n' as u8) as u8]);
    return 0;
}
pub fn __cpp2rust_init_globals() {}
