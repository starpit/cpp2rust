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
// good()/eof()/fail()/bad()/clear()/operator bool() are std::basic_ios<char>
// members, so these rules also fire for file streams, where the buffer model
// does not apply.  They were unmapped (and so untranslatable) before.

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

// Stream state.  These live on std::basic_ios<char>, so one rule covers every
// char stream; the bodies model the string-stream state (see the module note).
bool f13(const std::stringstream &o) { return o.good(); }

bool f14(const std::stringstream &o) { return o.eof(); }

bool f15(const std::stringstream &o) { return o.fail(); }

bool f16(const std::stringstream &o) { return o.bad(); }

void f17(std::stringstream &o) { return o.clear(); }

bool f18(const std::stringstream &o) { return o.operator bool(); }

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
std::istream &f29(std::istream &o, std::string &v) { return operator>>(o, v); }

std::istream &f30(std::istream &o, std::string &v, char d) {
  return std::getline(o, v, d);
}

std::istream &f31(std::istream &o, std::string &v) { return std::getline(o, v); }

std::istream &f32(std::istream &o, char &v) { return operator>>(o, v); }
