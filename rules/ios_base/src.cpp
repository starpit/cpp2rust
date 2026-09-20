// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::ios_base: sync_with_stdio and the openmode / fmtflags / iostate flag
// constants.  The flag values are libc++'s, so that combinations built with |
// and tested with & keep working; the types are all `unsigned int` typedefs.
// std::ios_base::seekdir is deliberately absent: it is an enum rather than an
// int typedef, and the seekg/seekp calls that consume it have no rules yet.

#include <ios>

using t1 = std::ios_base::openmode;
using t2 = std::ios_base::fmtflags;
using t3 = std::ios_base::iostate;

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
