// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <iterator>
#include <streambuf>
#include <string_view>
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

// ---------------------------------------------------------------------------
// t3, f58..f68 -- `std::string_view`, landed on the measured adjudication in
// /home/agent/work/BORROW-ADJUDICATION.md (queue rows g471 g941 g847 g2833 g2835 g2836).
//
// REPRESENTATION: the same as t1 and as rules/stringref -- `Vec<libc::c_char>` /
// `Vec<u8>`, NUL-TERMINATED, so `size()` is `len() - 1` and `empty()` is `len() <= 1`.
//
// AN OWNING STAND-IN FOR A NON-OWNING VIEW IS ACCEPTED HERE, AND ONLY HERE, BECAUSE THE
// CORPUS CANNOT OBSERVE THE DIFFERENCE.  Measured over the 7 real first-party
// `std::string_view` sites (14 hits in 5 files; 4 are #include/comment, 3 are vendored):
//   * mutation through the owner while a view is alive: 0 of 7.  And the reverse
//     direction is IMPOSSIBLE for this type -- `std::string_view` has NO mutating
//     member and `data()` returns `const char *`.
//   * dangling: 1 of 7 stores a long-lived view (VariableDefinition.cpp:177), and it is
//     SAFE -- a process-lifetime static whose referents are another static's strings,
//     which the source comment at :183 states outright.
//   * buffer identity: 1 of 7 (VariableDefinition.cpp:166, `ptr == sv.data() + sv.size()`),
//     and corpus-wide ZERO sites compare two DIFFERENT views' `.data()`, so the failure
//     mode an owning copy would hide does not occur.
//   * cost only: 5 of 7, including `hash`/`equal_to`, which are CONTENT-based in C++ too,
//     so a content model is faithful rather than approximate.
// Cat-1 aliasing is therefore UNMODELLED and said so: mutating the owner and reading the
// new bytes through the view prints `ZAAQ` in C++ and `AAAA` here.  That is the documented
// boundary, kept as commented case (1) of probe/svalias/p.cpp.  Do NOT try to model it with
// a borrowing representation -- that needs a lifetime a rule target cannot carry.
//
// DELIBERATELY NOT KEYED, each with its own reason (every one stays a LOUD abort; none
// emits todo!()/unimplemented!()/UNSUPPORTED):
//   * `data()` -- exposes the buffer ADDRESS.  Under refcount the receiver is a by-value
//     Vec dropped at end of body, so any pointer made from it DANGLES SILENTLY.  Identical
//     ground to rules/stringref src.cpp:146-158, which already refuses it in writing.  A
//     rule correct in unsafe and dangling in refcount is worse than an abort.
//   * `begin/end/cbegin/cend/rbegin/rend` -- same address exposure via iterators, and this
//     representation's trailing NUL would be traversed: an off-by-one in every loop.  Zero
//     first-party sites.
//   * the ordering operators `< <= > >=` -- lexicographic order would have to agree with
//     C++ across the trailing NUL this representation carries, and C++'s does not.  Zero
//     sites, so no body could even be checked.  Same reason rules/stringref refuses them.
//   * `operator[]`, `at()`, `back()` -- no first-party sites, and indexing to len()-1 would
//     read the terminator.
//   * `front()` -- REFUSED although the adjudication listed it safe, and the adjudication's own
//     argument is why.  `front()` returns `const char &`, and the receiver of a `string_view`
//     member is a BY-VALUE Vec (the type is a value type, unlike `std::string &` on f26 `at()`),
//     so the reference the target must return points into a Vec dropped at end of body --
//     SILENTLY DANGLING under refcount, exactly the `data()` failure with a different name.
//     The one site is VariableDefinition.cpp:230; it stays a loud abort.
//   * `remove_prefix`, `remove_suffix`, `copy`, `compare`, `find*` -- zero first-party
//     sites.  "The model provides it" plus "the type is reached" is NOT "a key would be
//     reached"; do not key what nothing calls.
//   * `string_view(const char *, size_t)` -- would silently TRUNCATE at an interior NUL,
//     exactly as rules/stringref records for its own two-argument constructor.  Zero
//     first-party sites construct one.
//   * `std::from_chars` -- not a member of this type and keyed by NO module.  Row g2832
//     (`ParseInt64`) stays BLOCKED on it and on `.data()`; row g2834 (`toStringView`) stays
//     BLOCKED on `llvm::StringRef::data()`.  Six of the eight rows are served, not eight.
//   * `std::hash<std::string_view>` / `std::equal_to<std::string_view>` -- content-based and
//     therefore SAFE to model, but the RECEIVER type belongs to rules/hash / rules/functional,
//     not to this module.  Left for its owner rather than keyed across module boundaries.
using t3 = std::string_view;

std::string_view f58() { return std::string_view(); }

// READBACK CORRECTION: `std::string_view(a0)` from a `std::string` recorded NOTHING AT ALL
// (f59 was absent from ir_src.json).  It is not a string_view constructor -- libc++ reaches it
// through `std::basic_string`'s implicit CONVERSION OPERATOR, so the key must be spelled on
// basic_string, not on the view.
std::string_view f59(const std::string &a0) { return a0.operator std::string_view(); }

bool f60(std::string_view a0, std::string_view a1) { return a0 == a1; }

bool f61(std::string_view a0, std::string_view a1) { return a0 != a1; }

std::size_t f62(std::string_view a0) { return a0.size(); }

std::size_t f63(std::string_view a0) { return a0.length(); }

bool f64(std::string_view a0) { return a0.empty(); }

// READBACK CORRECTION -- the brief said the 1-arg and 2-arg `substr` forms are DIFFERENT KEYS.
// MEASURED: THEY ARE THE SAME KEY.  Both recorded as
//     std::string_view std::basic_string_view<char>::substr(unsigned long, unsigned long) const
// i.e. the recorder writes the DEFAULTED argument out explicitly, so two src entries collided in
// one bucket and the search would have picked between them.  Keeping both is silent wrongness, so
// there is ONE key, spelled with both arguments, and its body is `saturating_add` rather than
// `a1 + a2` precisely because the 1-arg form arrives with a2 == npos == usize::MAX: saturating
// there yields min(MAX, len-1) == len-1, which is exactly `substr(pos)`.  A plain `a1 + a2` would
// overflow on every 1-arg call.
std::string_view f66(std::string_view a0, std::size_t a1, std::size_t a2) {
  return a0.substr(a1, a2);
}
