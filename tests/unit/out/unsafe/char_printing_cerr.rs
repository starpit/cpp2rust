extern crate libc;
use libc::*;
extern crate libcc2rs;
use libcc2rs::*;
use std::collections::BTreeMap;
use std::io::{Read, Seek, Write};
use std::os::fd::{AsFd, FromRawFd, IntoRawFd};
use std::rc::Rc;
pub fn main() {
    unsafe {
        __cpp2rust_init_globals();
        std::process::exit(main_0() as i32);
    }
}
unsafe fn main_0() -> i32 {
    let mut vec_: Vec<u8> = vec![195_u8, 167_u8];
    let mut i: i32 = 27;
    let mut str: Vec<libc::c_char> = {
        let s = c"bar.".as_ptr();
        std::slice::from_raw_parts(s, (0..).take_while(|&i| *s.add(i) != 0).count() + 1).to_vec()
    };
    libcc2rs::cc2_insert_int(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        (i) as i128,
        4,
        true,
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        b" a",
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[(vec_[(0_usize)]) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[(vec_[(1_usize)]) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('o' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &(str)
            .iter()
            .take((str).len() - 1)
            .map(|&c| c as u8)
            .collect::<Vec<u8>>()[..],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        b"\n",
    );
    libcc2rs::cc2_manip_unsafe(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        (libcc2rs::hex_unsafe as unsafe fn(*mut u32) -> *mut u32),
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        b"0x",
    );
    libcc2rs::cc2_insert_int(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        (27) as i128,
        4,
        true,
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        b" a\xc3\xa7ordas?",
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('\n' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        b"Sim, 0x",
    );
    libcc2rs::cc2_insert_int(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        (i) as i128,
        4,
        true,
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('.' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        b"\n",
    );
    libcc2rs::cc2_manip_unsafe(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        (libcc2rs::dec_unsafe as unsafe fn(*mut u32) -> *mut u32),
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('H' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('e' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('l' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('l' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('o' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[(',' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[(' ' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('W' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('o' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('r' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('l' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('d' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('!' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('\n' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[(vec_[(0_usize)]) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('\n' as libc::c_char) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[(vec_[(1_usize)]) as u8],
    );
    libcc2rs::cc2_insert_bytes(
        &mut std::fs::File::from_raw_fd(
            std::io::stderr()
                .as_fd()
                .try_clone_to_owned()
                .unwrap()
                .into_raw_fd(),
        ),
        &[('\n' as libc::c_char) as u8],
    );
    return 0;
}
pub unsafe fn __cpp2rust_init_globals() {}
