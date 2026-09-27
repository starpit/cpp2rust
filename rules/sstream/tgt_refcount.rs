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
// c008 IS REFUTED FOR THIS MODULE -- CORRECTED 2026-09-27 BY MEASUREMENT.  The
// text that used to stand here blamed overlapping AsPointer impls in libcc2rs
// (blanket `impl<T> AsPointer<T> for Rc<RefCell<T>>`, rc.rs:850, vs
// `Rc<RefCell<Vec<T>>>`, rc.rs:890) for an error[E0283] at every insertion site,
// and THREE SLOTS DEFERRED THIS ROW ON THAT CLAIM.  It is wrong.  Probe
// /home/agent/work/probes/oss-slot/oss.cpp (`std::ostringstream os; os << "abc";
// os << 42; os.str();`) emits
//     let os: Value<Vec<u8>> = Rc::new(RefCell::new(Vec::new()));
//     write!(os.as_pointer(), "abc",);
// and there is NO E0283 in either model; the `as_pointer()` call is unambiguous.
//
// THE REAL BLOCKER IS THE CONVERTER'S OSTREAM LOWERING, AND IT HITS BOTH MODELS.
// A NON-STRING insertion casts the stream operand to the hardcoded ostream
// target type `std::fs::File`:
//     write!((os as std::fs::File), "{:}", 42,);              // unsafe
//     write!((os.as_pointer() as std::fs::File), "{:}", 42,); // refcount
//   error[E0605]: non-primitive cast: `std::vec::Vec<u8>` as `File`
//   error[E0605]: non-primitive cast: `libcc2rs::Ptr<_>` as `File`
// one error per model and nothing else.  The string-literal insertion on the
// preceding line has NO cast and compiles, so the cast rides the DerivedToBase
// conversion of the receiver on the formatted path only.  It is written at the
// USE site, so no target body can annotate it away, and no other model type is
// reachable: Ptr<T>::write_all/write_fmt exist only for `T: std::io::Write +
// ByteRepr` (rc.rs:571), whose only inhabitants are Vec<u8> and std::fs::File
// (std::io::Cursor<Vec<u8>> is not ByteRepr).  cpp2rust/converter/* is a
// different owner, so this is left FAILING LOUDLY rather than guessed at.

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
