// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::ios_base: sync_with_stdio and the openmode / fmtflags / iostate flag
// constants.  The flag values are libc++'s, so that combinations built with |
// and tested with & keep working; the types are all `unsigned int` typedefs.
// std::ios_base::seekdir is deliberately absent: it is an enum rather than an
// int typedef, and the seekg/seekp calls that consume it have no rules yet.
//
// t4 maps std::ios_base ITSELF -- the class, not one of its typedefs -- to the
// same u32 the fmtflags constants above already are.  That is what turns the
// three base manipulators into ordinary callables: `std::hex` has the resolved
// signature `std::ios_base & std::hex(std::ios_base &)`, so with t4 in place it
// becomes a real Rust `fn(*mut u32) -> *mut u32` that sets the basefield of a
// flags word, and `operator>>(std::ios_base &(*)(std::ios_base &))` (see
// rules/sstream) can simply CALL it on whatever flags word the receiving stream
// carries.  Nothing anywhere special-cases the names `hex`/`dec`/`oct`, so a
// user-written manipulator with that signature works for free.
//
// Mapping the class to its own flags word is a deliberate narrowing: the real
// std::ios_base holds a width, a precision, a locale and a callback list as
// well.  Only the basefield is modelled, because only the basefield changes an
// answer anywhere in scope -- and the alternative on offer was to keep aborting.
// A site that upcast a stream to std::ios_base & for anything other than
// applying a manipulator would be mis-modelled here; no such site exists in
// dcg/ ddc/ dsc/ dbo/, and one would have to be written to be found, since the
// only way to reach this type is through a manipulator signature.

#include <ios>

using t1 = std::ios_base::openmode;
using t2 = std::ios_base::fmtflags;
using t3 = std::ios_base::iostate;
using t4 = std::ios_base;

// The three base manipulators, as plain functions on a flags word.  Each
// replaces the basefield and leaves every other flag alone, which is what
// libc++'s `setf(hex, basefield)` does.
std::ios_base &f30(std::ios_base &b) { return std::hex(b); }

std::ios_base &f31(std::ios_base &b) { return std::dec(b); }

std::ios_base &f32(std::ios_base &b) { return std::oct(b); }

// std::ios_base::sync_with_stdio has no meaning in Rust: there is no C stdio
// buffer to keep in step with, so the call becomes a no-op.
bool f1(bool sync) { return std::ios_base::sync_with_stdio(sync); }

std::ios_base::openmode f2() { return std::ios_base::app; }

std::ios_base::openmode f3() { return std::ios_base::ate; }

std::ios_base::openmode f4() { return std::ios_base::binary; }

std::ios_base::openmode f5() { return std::ios_base::in; }

std::ios_base::openmode f6() { return std::ios_base::out; }

std::ios_base::openmode f7() { return std::ios_base::trunc; }


std::ios_base::fmtflags f8() { return std::ios_base::boolalpha; }

std::ios_base::fmtflags f9() { return std::ios_base::dec; }

std::ios_base::fmtflags f10() { return std::ios_base::fixed; }

std::ios_base::fmtflags f11() { return std::ios_base::hex; }

std::ios_base::fmtflags f12() { return std::ios_base::internal; }

std::ios_base::fmtflags f13() { return std::ios_base::left; }

std::ios_base::fmtflags f14() { return std::ios_base::oct; }

std::ios_base::fmtflags f15() { return std::ios_base::right; }

std::ios_base::fmtflags f16() { return std::ios_base::scientific; }

std::ios_base::fmtflags f17() { return std::ios_base::showbase; }

std::ios_base::fmtflags f18() { return std::ios_base::showpoint; }

std::ios_base::fmtflags f19() { return std::ios_base::showpos; }

std::ios_base::fmtflags f20() { return std::ios_base::skipws; }

std::ios_base::fmtflags f21() { return std::ios_base::unitbuf; }

std::ios_base::fmtflags f22() { return std::ios_base::uppercase; }

std::ios_base::fmtflags f23() { return std::ios_base::adjustfield; }

std::ios_base::fmtflags f24() { return std::ios_base::basefield; }

std::ios_base::fmtflags f25() { return std::ios_base::floatfield; }


std::ios_base::iostate f26() { return std::ios_base::goodbit; }

std::ios_base::iostate f27() { return std::ios_base::badbit; }

std::ios_base::iostate f28() { return std::ios_base::eofbit; }

std::ios_base::iostate f29() { return std::ios_base::failbit; }
