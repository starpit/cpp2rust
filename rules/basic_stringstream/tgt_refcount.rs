// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp.  std::string is a NUL-terminated Vec<u8> in this model.
//
// THIS FILE MUST EXIST even though the bodies look like the unsafe ones: for a
// model-DEPENDENT return type, deleting it does NOT leave the key out of the
// refcount model, the converter falls back to the UNSAFE body -- which here
// returns Vec<libc::c_char> and gives E0308 "expected Vec<u8>, found Vec<i8>"
// (measured for rules/sstream, same shape).
//
// c008 IS REFUTED FOR THIS MODULE, MEASURED 2026-09-27 with probe
// /home/agent/work/probes/ssprobe.cpp (`std::stringstream ss; ss << "abc";
// ss << 42; ss.str();`).  The refcount model emits
//     let ss: Value<Vec<u8>> = Rc::new(RefCell::new(Vec::new()));
//     write!(ss.as_pointer(), "abc",);
// and that `as_pointer()` on an `Rc<RefCell<Vec<u8>>>` does NOT produce
// error[E0283]: no ambiguity between rc.rs:850 and rc.rs:890 is reported at any
// insertion site.  rules/sstream's documented blocker therefore does not
// generalise to basic_stringstream.
//
// WHAT ACTUALLY BLOCKS IT IS A DIFFERENT, NEW DEFECT, AND IT IS IN THE
// CONVERTER'S BUILT-IN OSTREAM LOWERING, NOT IN ANY RULE BODY -- and it hits
// BOTH models identically.  An insertion of a NON-STRING value makes
// ConvertCallToOstream cast the stream operand to the hardcoded ostream target
// type `std::fs::File`:
//     write!((ss as std::fs::File), "{:}", 42,);             // unsafe
//     write!((ss.as_pointer() as std::fs::File), "{:}", 42,); // refcount
//   error[E0605]: non-primitive cast: `std::vec::Vec<u8>` as `File`
//   error[E0605]: non-primitive cast: `libcc2rs::Ptr<_>` as `File`
// The string-literal insertion on the line before it is emitted WITHOUT the cast
// (`write!(ss, "abc",)` / `write!(ss.as_pointer(), "abc",)`) and compiles, so the
// cast is attached to the DerivedToBase conversion of the receiver on the
// formatted path only.  No annotation or model change inside a target can remove
// a cast the converter writes at the use site, and cpp2rust/converter/* is a
// different owner, so the keys are shipped as the model-correct bodies and this
// is left FAILING LOUDLY rather than guessed at.

fn t1() -> Vec<u8> {
    Vec::new()
}

fn f1() -> Vec<u8> {
    Vec::new()
}

// .str() -- a COPY out of the buffer, plus exactly ONE trailing NUL, because
// rules/string's size() is len()-1.
fn f2(a0: Vec<u8>) -> Vec<u8> {
    let mut __s: Vec<u8> = a0.clone();
    __s.push(0);
    __s
}

// f3 -- see tgt_unsafe.rs.  THIS OVERRIDE IS REQUIRED, and not because of any
// raw-pointer text: the PARAMETER type is model-dependent.  std::string is
// Vec<u8> here and Vec<libc::c_char> in the unsafe model, so inheriting the
// unsafe body would give E0308 "expected Vec<u8>, found Vec<i8>" -- the same
// shape already measured for f2.  No refcount-specific body logic is involved.
fn f3(a0: Vec<u8>, a1: libc::c_uint) -> Vec<u8> {
    let _ = a1;
    let __s = a0;
    let __n = __s.len().saturating_sub(1);
    __s[..__n].to_vec()
}
