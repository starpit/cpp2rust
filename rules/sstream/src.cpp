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
// REFCOUNT IS KNOWN-BLOCKED but tgt_refcount.rs is still SHIPPED -- deleting it
// does NOT leave the key out (the converter falls back to the UNSAFE body, which
// is the wrong model: two E0308 Vec<i8>/Vec<u8>).  The blocker is in libcc2rs,
// not in the key set.  A refcount local of type Vec<_> becomes
// Value<Vec<u8>> = Rc<RefCell<Vec<u8>>>, and the built-in ostream lowering
// writes through it as `os.as_pointer()`.  Two AsPointer impls satisfy that
// receiver -- the blanket `impl<T> AsPointer<T> for Rc<RefCell<T>>`
// (libcc2rs/src/rc.rs:850, T = Vec<u8>) and
// `impl<T> AsPointer<T> for Rc<RefCell<Vec<T>>>` (rc.rs:890, T = u8) -- so every
// insertion site is error[E0283] "type annotations needed".  The ambiguity is at
// the USE site the converter generates, not in any rule body, so no annotation
// inside a target can fix it.  Nor can a different model type: Ptr<T>'s
// write_all/write_fmt exist only for `T: std::io::Write + ByteRepr`
// (rc.rs:571), and the only such types are Vec<u8> (ByteRepr via
// reinterpret.rs:91) and std::fs::File -- std::io::Cursor<Vec<u8>> is not
// ByteRepr.  So Vec is forced and Vec is exactly what collides.  Fixing this
// needs the overlapping rc.rs:890 impl narrowed or removed in libcc2rs, which
// this module is not permitted to touch; until then the unsafe model is the
// supported one and no key is shipped whose target cannot compile.
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
