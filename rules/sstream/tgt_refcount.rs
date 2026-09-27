// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp.  std::string is a NUL-terminated Vec<u8> in this model.
//
// THIS FILE MUST EXIST EVEN THOUGH THE REFCOUNT MODEL IS KNOWN-BLOCKED.  Measured
// 2026-09-27: deleting it does NOT leave the key out of the refcount model -- the
// converter falls back to the UNSAFE target body, so an ostringstream local in a
// refcount TU got `let mut __s: Vec<libc::c_char> = ...` assigned into a
// `Value<Vec<u8>>` and failed with two error[E0308] "expected Vec<u8>, found
// Vec<i8>".  A model-DEPENDENT body therefore cannot be dropped by deleting the
// file; the fallback aliases it to the wrong model.  The bodies below are the
// model-correct ones.
//
// What is still blocked, and why it is not fixable from here: every insertion
// site the built-in ostream lowering generates is `os.as_pointer()` on a
// `Value<Vec<u8>>` = `Rc<RefCell<Vec<u8>>>`, and TWO libcc2rs impls satisfy that
// receiver -- the blanket `impl<T> AsPointer<T> for Rc<RefCell<T>>`
// (libcc2rs/src/rc.rs:850, T = Vec<u8>) and
// `impl<T> AsPointer<T> for Rc<RefCell<Vec<T>>>` (rc.rs:890, T = u8) -- giving
// error[E0283] "type annotations needed" at each one.  That ambiguity is in
// converter-generated code at the USE site, so no annotation in a target body can
// resolve it, and no other model type avoids it: Ptr<T>::write_all/write_fmt
// exist only for `T: std::io::Write + ByteRepr` (rc.rs:571), and the only such
// types are Vec<u8> and std::fs::File (std::io::Cursor<Vec<u8>> is not ByteRepr).
// The fix is to narrow or remove the overlapping rc.rs:890 impl in libcc2rs,
// which this module is not permitted to touch.

fn t1() -> Vec<u8> {
    Vec::new()
}

fn f1() -> Vec<u8> {
    Vec::new()
}

// .str() -- a COPY out of the buffer, plus exactly ONE trailing NUL, because
// rules/string's size() is len()-1.  Copying (rather than moving a0 through)
// matters: `os.str()` must not empty the stream.
fn f2(a0: Vec<u8>) -> Vec<u8> {
    let mut __s: Vec<u8> = a0.clone();
    __s.push(0);
    __s
}
