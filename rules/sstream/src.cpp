// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::stringstream / std::ostringstream / std::istringstream.
//
// Model: a stream is a byte buffer, Box<Vec<u8>> in both models.
//   * insertion (operator<<) is the converter's built-in ostream path, which
//     emits write!()/write_all() -- io::Write for Vec<u8> appends, so the
//     buffer grows at the end and needs no separate put pointer;
//   * extraction (operator>>, std::getline) drains the consumed bytes off the
//     front, so the read cursor is "the start of the buffer" and needs no
//     separate get pointer.
//   The buffer is NOT NUL terminated; str() adds the terminator when it hands
//   the contents back as a std::string.
//   Box<Vec<u8>> rather than a bare Vec<u8> because libcc2rs has two
//   AsPointer impls for Rc<RefCell<Vec<T>>> and the refcount model's
//   write!(o.as_pointer(), ..) would then be ambiguous.
//
// The consequence of the drain model is that str() after an extraction returns
// what is left, not the whole original buffer, and that the stream state bits
// are derived from emptiness: eof()/fail() are "buffer is empty", good() is
// "buffer is not empty", bad() is always false, and clear() is a no-op (in C++
// it resets the error state, which here has no independent existence).
//
// Those state predicates USED to live here, as f13-f18.  They are members of
// std::basic_ios<char>, not of any string-stream class, so the single rule the
// signature admits also fired on std::ifstream/std::ofstream/std::cout, where
// the buffer model does not apply and `!(*f.borrow()).is_empty()` on a
// ::std::fs::File is not even compilable Rust.  They now live in
// rules/basic_ios, which dispatches on the receiver's Rust representation and
// reproduces exactly the emptiness-derived answers above for this module's
// Box<Vec<u8>>.

#include <istream>
#include <sstream>
#include <string>

using t1 = std::stringstream;
using t2 = std::ostringstream;
using t3 = std::istringstream;
using t4 = std::istream;
using t5 = std::ios;

// Default construction.
std::stringstream f1() { return std::stringstream(); }

std::ostringstream f2() { return std::ostringstream(); }

std::istringstream f3() { return std::istringstream(); }

// Construction from a std::string.
std::stringstream f4(const std::string &s) { return std::stringstream(s); }

std::ostringstream f5(const std::string &s) { return std::ostringstream(s); }

std::istringstream f6(const std::string &s) { return std::istringstream(s); }

// str() getter.
std::string f7(const std::stringstream &o) { return o.str(); }

std::string f8(const std::ostringstream &o) { return o.str(); }

std::string f9(const std::istringstream &o) { return o.str(); }

// str() setter.
void f10(std::stringstream &o, const std::string &s) { return o.str(s); }

void f11(std::ostringstream &o, const std::string &s) { return o.str(s); }

void f12(std::istringstream &o, const std::string &s) { return o.str(s); }

std::istream &f19(std::istringstream &o, int &v) {
  return o.operator>>(v);
}

std::istream &f20(std::istringstream &o, unsigned int &v) {
  return o.operator>>(v);
}

std::istream &f21(std::istringstream &o, long &v) {
  return o.operator>>(v);
}

std::istream &f22(std::istringstream &o, unsigned long &v) {
  return o.operator>>(v);
}

std::istream &f23(std::istringstream &o, long long &v) {
  return o.operator>>(v);
}

std::istream &f24(std::istringstream &o, unsigned long long &v) {
  return o.operator>>(v);
}

std::istream &f25(std::istringstream &o, short &v) {
  return o.operator>>(v);
}

std::istream &f26(std::istringstream &o, unsigned short &v) {
  return o.operator>>(v);
}

std::istream &f27(std::istringstream &o, float &v) {
  return o.operator>>(v);
}

std::istream &f28(std::istringstream &o, double &v) {
  return o.operator>>(v);
}

// Free extraction operators and std::getline.
//
// These take std::istream, not std::istringstream, so the ONE rule each
// signature admits also fires on std::ifstream and std::cin -- exactly the
// collision that moved the state predicates out to rules/basic_ios.  The
// representations differ: this module maps string streams to Box<Vec<u8>>,
// while rules/fstream and rules/iostream map file streams and std::cin to
// ::std::fs::File, which has neither .len() nor .drain() nor indexing.  A body
// written against the drain buffer therefore emitted non-compiling Rust on
// every file stream ("no method named `len` found for struct `File`").
//
// All four are fixed the same way rules/basic_ios fixes the predicates: each
// body declares a private trait with one method, implements it for Vec<u8>
// (drain the buffer) and for ::std::fs::File (read a byte at a time, which
// leaves the file position where C++ leaves it), and calls it on the receiver,
// so Rust resolves the impl statically from the receiver's type.  Verified in
// both models against clang-compiled C++ on an ifstream and an istringstream in
// the same program.
//
// One extra step was needed here that the predicates did not need: each
// refcount body's receiver had to be respelled from `Ptr<Box<Vec<u8>>>` to
// `&mut Box<Vec<u8>>`.  A Ptr-typed parameter makes the converter emit
// `f.as_pointer()` typed as `Ptr<Box<Vec<u8>>>` at the CALL SITE -- outside the
// body, where no trait can reach it -- which is `Ptr<File>` on a file stream
// and a type error.  Spelling the parameter as a borrow removes the cast.
//
// The File impls differ in exactly one respect, and it is the subtle part.
// getline (f30, f31) CONSUMES its delimiter, so its loop can simply stop on it.
// operator>> for a token (f29) does NOT: C++ skips the leading run of
// whitespace, consumes the token, and leaves the file position AT the
// terminating whitespace, so the next getline on the same stream sees it.  A
// File has no pushback buffer, so f29's impl seeks back one byte after reading
// the terminator -- without that it over-consumes and `f >> tok; getline(f, r);`
// loses a character.  operator>> for a single char (f32) consumes exactly the
// char it returns, so it needs no pushback at all.
std::istream &f29(std::istream &o, std::string &v) { return operator>>(o, v); }

std::istream &f30(std::istream &o, std::string &v, char d) {
  return std::getline(o, v, d);
}

// f31 is the one-argument getline, and it had been MISSED when f29/f30/f32 were
// converted to the private-trait dispatch: its refcount receiver was still
// spelled `Ptr<Box<..>>`, so the converter emitted `f.as_pointer()` typed as
// `Ptr<Box<StringStream>>` at the call site, which on a std::ifstream is
// `Ptr<File>` -- E0631 on the with_mut closure and E0308 on the returned
// pointer.  Now it has f30's shape exactly, so `std::getline(f, line)` as a
// STATEMENT works on a file stream in both models.
//
// `while (std::getline(f, line))` -- how dcg/tools/mda/memDumpAnalyzer.h,
// dsc/pcfg.cpp, dsc/superdsc.cpp, dsc/dataOpDsc.cpp and
// dsc/designSpaceConfig.cpp all drive an input file -- is STILL NOT SUPPORTED,
// deliberately.
//
// WARNING, and this paragraph replaces a claim that is no longer true.  This
// used to read "it fails at rustc rather than silently".  That holds only in the
// unsafe model.  MEASURED 2026-09-24 on a six-line istringstream probe, against
// the same source built with clang (which prints three lines and `total=3`):
//   unsafe   -- E0614 at rustc.  Loud, as documented.
//   refcount -- COMPILES, then panics at run time, `ub: null pointer` at
//               libcc2rs/src/rc.rs:350, having printed nothing at all.
// The refcount body below ends in `Ptr::<Box<StringStream>>::null()`, and using
// that as a loop condition emits `.upgrade().deref()` on the null.  FOUR rules
// in tgt_refcount.rs return that same null stream, so the trap is latent in all
// of them rather than specific to getline.  A refuse-to-compile gap and a
// runtime null deref are not the same risk, and the five files listed above are
// exactly the ones this would reach.
//
// Making it compile
// needs the rule to return the stream so `operator bool` can be applied to the
// result, and that was tried and reverted: rules/basic_ios answers
// `operator bool` on a ::std::fs::File from position-vs-length, which is
// "not at end" rather than "the last read succeeded", so the loop never
// terminates -- a HANG, measured (run_rc=124, the probe's timeout). C++ sets
// failbit when getline reads nothing, and neither representation carries a
// failbit that a read can set. Doing this properly means the File side growing
// real stream state, which is the same thing the format flags needed and is a
// bigger change than this one; until then a loud rustc error beats an infinite
// loop. The `while (!f.eof()) { getline(..); .. }` spelling, which is what
// memDumpAnalyzer.h actually uses, is unaffected and works.
std::istream &f31(std::istream &o, std::string &v) { return std::getline(o, v); }

std::istream &f32(std::istream &o, char &v) { return operator>>(o, v); }

// ---------------------------------------------------------------------------
// operator>>(std::ios_base &(*)(std::ios_base &)) -- applying a manipulator.
//
// This is the rule that makes `inFile >> std::hex >> lineno` mean what C++
// means by it, and the reason it has to exist at all is that the radix is
// STICKY PER STREAM.  Measured against clang-compiled C++:
//
//     std::istringstream ss("ff 10");
//     ss >> std::hex >> a;   // a == 255
//     ss >> b;               // b == 16  -- still hex, no manipulator here
//
// so a fix that inspected the extraction site and emitted a base-16 parse for
// the operand next to `std::hex` would be right for the first line and silently
// WRONG for the second.  That is the exact failure this whole change exists to
// remove: before the assertions build turned it into an abort, this construct
// translated to a decimal parse of hexadecimal input with no diagnostic at all,
// and `DT_CHECK(lineno != -1)` then passed on the wrong number.  So the state
// has to live on the stream, and this rule's only job is to put it there.
//
// `std::hex` is NOT a value that can be pattern-matched: it arrives as a
// FUNCTION POINTER, which is why the parameter here is spelled as one and why
// rules/ios_base has to map `std::ios_base` itself (to the flags word) and give
// hex/dec/oct real function bodies.  This rule then simply CALLS whatever
// manipulator it was handed on the receiving stream's flags word, so nothing
// here knows the names hex/dec/oct and a user-written manipulator with the same
// signature works unchanged.
//
// A null manipulator pointer is a no-op rather than a panic: C++ cannot produce
// one, so the Option is an artefact of how the converter spells a function
// pointer, not a case with C++ semantics to reproduce.
std::istream &f33(std::istream &o, std::ios_base &(*m)(std::ios_base &)) {
  return o.operator>>(m);
}
