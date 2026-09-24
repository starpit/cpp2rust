// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <iterator>
#include <streambuf>
#include <string>

using t1 = std::string;
using t2 = std::string::iterator;

std::string f1(const std::string &s, std::size_t pos, std::size_t count) {
  return s.substr(pos, count);
}

std::size_t f2(const std::string &s) { return s.size(); }

std::string f3(const std::string &a, const char *b) { return a + b; }

std::string &f4(std::string &s, const char *p, std::size_t n) {
  return s.append(p, n);
}

const char *f5(const std::string &s) { return s.c_str(); }

char *f6(std::string &s) { return s.data(); }

std::string f7(const char *s, std::size_t n) { return std::string(s, n); }

std::string f8(std::istreambuf_iterator<char> first,
               std::istreambuf_iterator<char> last) {
  return std::string(first, last);
}

std::string f9(std::size_t n, char ch) { return std::string(n, ch); }

std::string f10(const char *s) { return std::string(s); }

const char *f11(const std::string &o) { return o.data(); }

std::string::iterator f12(std::string &s) { return s.begin(); }

void f13(std::string &s, std::size_t n) { return s.resize(n); }

std::string &f14(std::string &s, std::size_t pos, std::size_t count,
                 const char *p, std::size_t n) {
  return s.replace(pos, count, p, n);
}

std::string::iterator f15(std::string &s) { return s.end(); }

std::size_t f16(const std::string &s, const char *chars) {
  return s.find_last_of(chars);
}

std::string f17(std::string &&a0, const char *a1) { return std::move(a0) + a1; }

bool f18(const std::string &a, const char *b) { return a == b; }

std::size_t f19(const std::string &s) { return s.length(); }

std::string::iterator f20(std::string::iterator it, std::size_t n) {
  return it + n;
}

std::string &f21(std::string &s, std::size_t n, char c) {
  return s.append(n, c);
}

bool f22(std::string &s) { return s.empty(); }

std::string f23() { return std::string(); }

void f24(std::string &o) { return o.clear(); }

void f25(std::string &o) { return o.shrink_to_fit(); }

char &f26(std::string &o, std::size_t idx) { return o.at(idx); }

std::string f27(const std::string &o) { return std::string(o); }

std::string f28(std::string &&o) { return std::string(std::move(o)); }

std::string &f29(std::string &dst, const std::string &src) {
  return dst.operator=(src);
}

std::string &f30(std::string &dst, std::string &&src) {
  return dst.operator=(std::move(src));
}

std::string f31(const std::string &a, const std::string &b) { return a + b; }

std::string f32(std::string &&a, std::string &&b) {
  return std::move(a) + std::move(b);
}

std::string f33(std::string &&a, char b) { return std::move(a) + b; }

std::string f34(const std::string &a, char b) { return a + b; }

std::string &f35(std::string &s, const std::string &o) {
  return s.operator+=(o);
}

std::string &f36(std::string &s, const char *o) { return s.operator+=(o); }

std::string &f37(std::string &s, char c) { return s.operator+=(c); }

std::string f38(const char *a, const std::string &b) { return a + b; }

std::string f39(const char *a, std::string &&b) { return a + std::move(b); }

std::string f40(std::string &&a, const std::string &b) {
  return std::move(a) + b;
}

std::string f41(const std::string &a, std::string &&b) {
  return a + std::move(b);
}

bool f42(const std::string &a, const std::string &b) { return a == b; }

bool f43(const std::string &a, const std::string &b) { return a != b; }

bool f44(const std::string &a, const char *b) { return a != b; }

bool f45(const char *a, const std::string &b) { return a == b; }

bool f46(const char *a, const std::string &b) { return a != b; }

// ---------------------------------------------------------------------------
// std::to_string
//
// One rule per arithmetic overload: overload resolution is exact, so the
// int rule never matches a `long long` call site.
//
// The float and double forms are NOT Rust's `to_string`.  libc++ implements
// them with sprintf("%f"), i.e. always six digits after the decimal point,
// where Rust prints the shortest round-tripping form -- so `1.5` becomes
// "1.500000" in C++ and "1.5" in Rust.  The targets format explicitly.
// ---------------------------------------------------------------------------

std::string f47(int a0) { return std::to_string(a0); }

std::string f48(long a0) { return std::to_string(a0); }

std::string f49(long long a0) { return std::to_string(a0); }

std::string f50(unsigned int a0) { return std::to_string(a0); }

std::string f51(unsigned long a0) { return std::to_string(a0); }

std::string f52(unsigned long long a0) { return std::to_string(a0); }

std::string f53(float a0) { return std::to_string(a0); }

std::string f54(double a0) { return std::to_string(a0); }

// ---------------------------------------------------------------------------
// std::stoi / std::stoll / std::stod
//
// The defaulted arguments are materialised by the caller, so the signature
// cpp2rust looks up carries all of them: `int std::stoi(const std::string &,
// unsigned long *, int)`.  A rule with fewer parameters never matches.
//
// `pos` is an out-parameter receiving the index one past the last character
// consumed; it is usually null.  C++ throws std::invalid_argument when no
// conversion is possible -- these panic instead, which is the loud failure,
// not a silent zero.
// ---------------------------------------------------------------------------

int f55(const std::string &a0, std::size_t *a1, int a2) {
  return std::stoi(a0, a1, a2);
}

long long f56(const std::string &a0, std::size_t *a1, int a2) {
  return std::stoll(a0, a1, a2);
}

double f57(const std::string &a0, std::size_t *a1) {
  return std::stod(a0, a1);
}

// operator+(char, const std::string &) -- a character on the LEFT.
//
// The mirror of f34 (`string + char`) and the char analogue of f38
// (`const char * + string`), and the one concatenation form the module was
// missing. It is not hypothetical: `sys-arch-spec/progir/progir.cpp:32` is
// `out = '"' + out + '"'`, and without this rule that TU aborts at
// `unsupported CXXOperatorCallExpr: +`. progir.cpp is the TU that DEFINES
// `InstrInfo::empty`, so this rule is on the path to the gtest tests linking.
std::string f58(char a0, const std::string &a1) { return a0 + a1; }
