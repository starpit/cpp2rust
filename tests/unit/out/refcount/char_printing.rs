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
        let mut __bytes = Ptr::<u8>::from_string_literal(b"foo.").to_c_bytes();
        __bytes.push(0);
        __bytes
    }));
    libcc2rs::cc2_insert_int(&libcc2rs::cout(), (*i.borrow()) as i128, 4, true);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), b" a");
    libcc2rs::cc2_insert_bytes(
        &libcc2rs::cout(),
        &[((vec_.as_pointer() as Ptr<u8>).offset(0_usize).read()) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &libcc2rs::cout(),
        &[((vec_.as_pointer() as Ptr<u8>).offset(1_usize).read()) as u8],
    );
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('o' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(
        &libcc2rs::cout(),
        &(*str.borrow())
            .iter()
            .take((*str.borrow()).len() - 1)
            .map(|&c| c as u8)
            .collect::<Vec<u8>>()[..],
    );
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), b"\n");
    libcc2rs::cc2_manip_refcount(
        &libcc2rs::cout(),
        (libcc2rs::hex_refcount as fn(Ptr<u32>) -> Ptr<u32>),
    );
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), b"0x");
    libcc2rs::cc2_insert_int(&libcc2rs::cout(), (27) as i128, 4, true);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), b" a\xc3\xa7ordas?");
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('\n' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), b"Sim, 0x");
    libcc2rs::cc2_insert_int(&libcc2rs::cout(), (*i.borrow()) as i128, 4, true);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('.' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), b"\n");
    libcc2rs::cc2_manip_refcount(
        &libcc2rs::cout(),
        (libcc2rs::dec_refcount as fn(Ptr<u32>) -> Ptr<u32>),
    );
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('H' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('e' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('l' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('l' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('o' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[(',' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[(' ' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('W' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('o' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('r' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('l' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('d' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('!' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('\n' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(
        &libcc2rs::cout(),
        &[((vec_.as_pointer() as Ptr<u8>).offset(0_usize).read()) as u8],
    );
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('\n' as u8) as u8]);
    libcc2rs::cc2_insert_bytes(
        &libcc2rs::cout(),
        &[((vec_.as_pointer() as Ptr<u8>).offset(1_usize).read()) as u8],
    );
    libcc2rs::cc2_insert_bytes(&libcc2rs::cout(), &[('\n' as u8) as u8]);
    return 0;
}
pub fn __cpp2rust_init_globals() {}
