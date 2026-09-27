// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::ostringstream -- an in-memory output stream.
//
// MODEL: a byte buffer, Vec<u8>.
//
// WHY THIS AND NOT A SINK ENUM SHARED WITH std::ostream.  Insertion into any
// ostream is NOT rule-driven: converter_lib.cpp:552 IsCallToOstream matches any
// operator<< whose RESULT type is basic_ostream (which an ostringstream
// insertion is, because the free ADL operator<< returns basic_ostream&), and
// converter.cpp:1761 ConvertCallToOstream then emits, textually,
//     <stream>.write_all(&([...].concat()));      and
//     write!(<stream>, "<fmt>", args);
// where <stream> is just ToString() of the left-most operand.  There is no
// type-based dispatch and no rule lookup on the stream at all, so the
// DerivedToBase cast on the receiver costs nothing and the target type only has
// to implement std::io::Write.  Vec<u8> does (std's `impl Write for Vec<u8>`,
// which APPENDS), so the buffer needs no new libcc2rs type.
//
// .str() copies the buffer out as a std::string, i.e. the NUL-TERMINATED Vec
// that rules/string uses (Vec<libc::c_char> in the unsafe model, Vec<u8> in
// refcount); the terminator is added here and is not part of size().
//
// c008 IS REFUTED FOR THIS MODULE TOO -- CORRECTED 2026-09-27 BY MEASUREMENT.
// This file previously said the refcount model was blocked by overlapping
// AsPointer impls in libcc2rs (the blanket `impl<T> AsPointer<T> for
// Rc<RefCell<T>>` at rc.rs:850 vs `Rc<RefCell<Vec<T>>>` at rc.rs:890) giving
// error[E0283] at every insertion site.  THAT CLAIM WAS WRONG, and it had
// already misdirected three slots into deferring the row.  Probe
// /home/agent/work/probes/oss-slot/oss.cpp (`std::ostringstream os; os << "abc";
// os << 42; os.str();`) emits, in the refcount model,
//     let os: Value<Vec<u8>> = Rc::new(RefCell::new(Vec::new()));
//     write!(os.as_pointer(), "abc",);
// and that `as_pointer()` on an `Rc<RefCell<Vec<u8>>>` produces NO ambiguity
// error anywhere -- E0283 does not appear in either model.  (Same result the
// rules/basic_stringstream slot got for std::stringstream, which is the same
// lowering over the same model.)
//
// WHAT ACTUALLY BLOCKS BOTH MODELS is a converter defect in the built-in ostream
// lowering, and it hits the UNSAFE model identically -- so this module is
// partial in BOTH models, not just refcount.  An insertion of a NON-STRING value
// makes ConvertCallToOstream cast the stream operand to the hardcoded ostream
// target type `std::fs::File`:
//     write!((os as std::fs::File), "{:}", 42,);              // unsafe
//     write!((os.as_pointer() as std::fs::File), "{:}", 42,); // refcount
//   error[E0605]: non-primitive cast: `std::vec::Vec<u8>` as `File`
//   error[E0605]: non-primitive cast: `libcc2rs::Ptr<_>` as `File`
// exactly one error per model, and nothing else.  The STRING-LITERAL insertion on
// the line before is emitted WITHOUT the cast (`write!(os, "abc",)` /
// `write!(os.as_pointer(), "abc",)`) and compiles, so the cast rides the
// DerivedToBase conversion of the receiver on the FORMATTED path only.  No
// annotation or model change inside a target body can remove a cast the converter
// writes at the use site, and no other model type is available anyway: Ptr<T>'s
// write_all/write_fmt exist only for `T: std::io::Write + ByteRepr` (rc.rs:571),
// whose only inhabitants are Vec<u8> (ByteRepr via reinterpret.rs:91) and
// std::fs::File -- std::io::Cursor<Vec<u8>> is not ByteRepr.  cpp2rust/converter/*
// is a different owner, so the keys are shipped as the model-correct bodies and
// the formatted path is left FAILING LOUDLY.
//
// tgt_refcount.rs is still SHIPPED regardless -- deleting it does NOT leave the
// key out (the converter falls back to the UNSAFE body, which is the wrong
// model: two E0308 Vec<i8>/Vec<u8>).
//
// NOT COVERED, deliberately, each because it needs its own harvested key:
// istringstream/stringstream, str(const std::string&), the
// basic_ostringstream(std::string) constructor, and the std::ios_base
// formatting manipulators.

#include <sstream>
#include <string>

using t1 = std::ostringstream;

std::ostringstream f1() { return std::ostringstream(); }

std::string f2(const std::ostringstream &o) { return o.str(); }
