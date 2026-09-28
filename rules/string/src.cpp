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

std::string f31(std::string &&a0, char a1) { return std::move(a0) + a1; }

std::string f32(std::string &&a0, std::string &&a1) {
  return std::move(a0) + std::move(a1);
}

std::string f33(const std::string &a0, const std::string &a1) { return a0 + a1; }

std::string f34(std::string &&a0, const std::string &a1) {
  return std::move(a0) + a1;
}

std::string f35(const std::string &a0, std::string &&a1) {
  return a0 + std::move(a1);
}

std::string f36(const std::string &a0, char a1) { return a0 + a1; }

std::string f37(const char *a0, const std::string &a1) { return a0 + a1; }

std::string &f38(std::string &a0, const std::string &a1) {
  return a0.operator+=(a1);
}

std::string &f39(std::string &a0, char a1) { return a0.operator+=(a1); }

std::string &f40(std::string &a0, const char *a1) { return a0.operator+=(a1); }

std::string f41(const char *a0, std::string &&a1) { return a0 + std::move(a1); }

std::string f42(char a0, const std::string &a1) { return a0 + a1; }

std::string f43(char a0, std::string &&a1) { return a0 + std::move(a1); }

bool f44(const std::string &a0, const std::string &a1) { return a0 == a1; }

bool f45(const char *a0, const std::string &a1) { return a0 == a1; }

bool f46(const std::string &a0, const std::string &a1) { return a0 != a1; }

bool f47(const std::string &a0, const char *a1) { return a0 != a1; }

bool f48(const char *a0, const std::string &a1) { return a0 != a1; }

// f49 -- `std::basic_string<char>::npos`.  A DECLARATION-ONLY in-class static data
// member (libcxx/string:797, `static const size_type npos = -1;` -- the definition
// lives out-of-line in the dylib), so the converter never traverses a VarDecl that
// carries an initializer and never emits the `pub static mut npos_<n>` it then
// references.  MEASURED, not inferred: with no key, a TU using it emits
//     (*std::cell::LazyCell::force_mut(&mut *&raw mut npos_1))
// against nothing at all -- an instant E0425 that no placeholder grep and no
// bucket census can see (there is no `Cpp2RustUnmapped` token and rc=0).
// 6 of 78 bucket-A census TUs carry it.
//
// THE SEARCHED SPELLING IS THE TEMPLATE-SPECIALISED QUALIFIED NAME, read back from
// the converter's own -verbose log, not guessed:
//     search expr std::basic_string<char>::npos, result:
//     None
// Same mechanism and same key shape as rules/iostream f5/f6 (std::ios_base::in /
// ::out): Converter::ConvertDeclRefExpr consults the mapper BEFORE the IsGlobalVar
// branch, and the rule preprocessor's `declref` matcher records
// Mapper::ToString(VarDecl) == printQualifiedName, so the two spellings agree.
//
// WIDTH IS MEASURED, NOT ASSUMED.  The same log shows `search type size_type,
// result: usize` for this module, so `npos == size_type(-1)` is `usize::MAX`.
// A body of `0`, or `-1` in a narrower width, would be silently wrong; the probe
// asserts a REAL find() index in one case and npos in another so neither passes.
std::size_t f49() { return std::string::npos; }

// ---------------------------------------------------------------------------
// f50..f57 -- `std::to_string`, the #2 cause of compile errors in the port-goal TU:
// MEASURED by the first `cargo check` on `dxp/dxp_standalone.cpp`'s emitted Rust
// (36,772 lines, bucket A) -- 166 of 1,245 errors, 13.3%, all `E0425 cannot find
// function`, across FIVE distinct mangled fallback spellings (`to_string_6` x88,
// `to_string_80` x46, `to_string_84` x4, `to_string_94` x12, `to_string_97` x16).
// Five spellings means the recorder saw FIVE DIFFERENT OVERLOADS, so one key
// cannot answer for them; each arithmetic overload is keyed separately below.
//
// SEARCHED SPELLINGS, read back from the converter's own -verbose log (each printed
// `result:` / `None` against ir.v13), NOT guessed -- note `unsigned` prints as
// `unsigned int`, and the return type IS part of the key for a free function:
//     search expr std::string std::to_string(int), result:                None
//     search expr std::string std::to_string(unsigned int), result:       None
//     search expr std::string std::to_string(long), result:               None
//     search expr std::string std::to_string(unsigned long), result:      None
//     search expr std::string std::to_string(long long), result:          None
//     search expr std::string std::to_string(unsigned long long), result: None
//     search expr std::string std::to_string(float), result:              None
//     search expr std::string std::to_string(double), result:             None
//
// THE FLOAT OVERLOADS ARE WHERE A PLAUSIBLE BODY IS SILENTLY WRONG.  [string.conversions]
// defines to_string(float/double) as `sprintf(buf, "%f", val)` -- `%f` is ALWAYS SIX
// decimal places, so `to_string(1.5)` is "1.500000" and `to_string(2.0)` is "2.000000",
// whereas Rust's `{}` yields "1.5" and "2".  The bodies therefore use `{:.6}`, and the
// probe asserts the exact strings so a `{}` body fails rather than passing every integer
// test and corrupting every float.  `long double` (`%Lf`) is DELIBERATELY LEFT OUT: this
// toolchain has no faithful Rust f80, so any body would be a silent precision change.
//
// REPRESENTATION: t1 is `Vec<libc::c_char>` (unsafe) / `Vec<u8>` (refcount), and it is
// NUL-TERMINATED -- f7/f9/f10 all `chain(std::iter::once(0))` or `.push(0)`, and f46..f48
// compare with `len() - 1`.  The bodies below append the NUL for exactly that reason; a
// body without it would make every `.len() - 1` slice in the goal TU drop a real digit.
std::string f50(int a0) { return std::to_string(a0); }

std::string f51(unsigned int a0) { return std::to_string(a0); }

std::string f52(long a0) { return std::to_string(a0); }

std::string f53(unsigned long a0) { return std::to_string(a0); }

std::string f54(long long a0) { return std::to_string(a0); }

std::string f55(unsigned long long a0) { return std::to_string(a0); }

std::string f56(float a0) { return std::to_string(a0); }

std::string f57(double a0) { return std::to_string(a0); }
