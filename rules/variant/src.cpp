// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::variant is a tagged union.  A Rust enum is a tagged union.  So the
// representation is a Rust enum -- libcc2rs::Variant2..Variant8, one per arity
// -- and nothing here approximates anything: the discriminant is the
// discriminant and the payload is the payload.
//
// WHY THIS FILE IS GENERATED (cpp2rust-port/../iso/variant/gen.py).  A rule is
// keyed by the RESOLVED C++ signature, and for std::variant the signature names
// the alternative:
//
//     void std::variant<long, std::string>::variant(const long &)
//     void std::variant<long, std::string>::variant(const std::string &)
//
// are two different signatures, so they are two different rules.  Same for
// std::get<T> and std::holds_alternative<T>.  At the arity progir.h uses (8)
// that is 9 x 8 + 8 = 80 rules, and by arity 2..8 it is 371.  Generated.
//
// THE TIE THAT ISN'T.  `variant(const T1 &)` and `variant(const T2 &)` have
// EQUAL length and EQUAL distinct-template-parameter count, so neither of
// Mapper::search's tie-breakers separates them, and if both matched a call the
// converter would print "ambiguous translation rule ... Refusing to guess".
// They never both match.  matchTemplate captures each Tn once and then requires
// every later occurrence to match the SAME text, so for
// `variant(const long &)` on a `variant<long, std::string>` the T1 rule
// captures T1=long from the class template argument list and then matches
// `long` in the parameter, while the T2 rule captures T2=std::string and then
// has to match `std::string` against `long` and fails.  Exactly one rule
// matches, per alternative, which is what makes this expressible at all.  It
// relies on the alternatives being DISTINCT types -- which C++ itself requires
// for std::get<T>/std::holds_alternative<T> to be well-formed at all, so it is
// not an extra assumption.
//
// ARITY is enforced by the same mechanism: an arity-4 rule cannot match an
// arity-8 call because after capturing T1..T4 the template still wants `, T5`
// where the instantiation has `>`.  matchTemplate's spansSeveralArguments guard
// additionally stops a single Tn from swallowing `long, std::string`.
//
// <vector> is pulled in deliberately, for the same reason rules/pair does it:
// with only <variant>, libc++ prints the free comparison operators as
// std::operator== rather than std::__1::operator==, and the rule then silently
// never matches in a real TU.
//
// NOT MODELLED, deliberately: std::visit (zero sites in dt_src -- it would
// have to be a rule per (arity, callable) pair and there is nothing to key on),
// std::monostate (zero sites), variant_alternative/variant_size (zero sites),
// get_if (zero sites), emplace (zero sites), and std::get<I> BY INDEX on a
// variant (zero sites; every dt_src site selects by type).  A site that uses
// one stays a loud abort rather than getting a guessed body.

#include <map>
#include <string>
#include <variant>
#include <vector>

template <typename T1, typename T2> using t1 = std::variant<T1, T2>;
template <typename T1, typename T2, typename T3> using t2 = std::variant<T1, T2, T3>;
template <typename T1, typename T2, typename T3, typename T4> using t3 = std::variant<T1, T2, T3, T4>;
template <typename T1, typename T2, typename T3, typename T4, typename T5> using t4 = std::variant<T1, T2, T3, T4, T5>;
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> using t5 = std::variant<T1, T2, T3, T4, T5, T6>;
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> using t6 = std::variant<T1, T2, T3, T4, T5, T6, T7>;
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> using t7 = std::variant<T1, T2, T3, T4, T5, T6, T7, T8>;

// ---- arity 2 -----------------------------------------------------------

template <typename T1, typename T2> std::variant<T1, T2> f1() { return std::variant<T1, T2>(); }

template <typename T1, typename T2>
std::variant<T1, T2> f2(const std::variant<T1, T2> &a0) { return std::variant<T1, T2>(a0); }

template <typename T1, typename T2>
std::variant<T1, T2> f3(std::variant<T1, T2> &&a0) { return std::variant<T1, T2>(std::move(a0)); }

template <typename T1, typename T2>
std::variant<T1, T2> &f4(std::variant<T1, T2> &d, const std::variant<T1, T2> &s) { return d.operator=(s); }

template <typename T1, typename T2>
std::variant<T1, T2> &f5(std::variant<T1, T2> &d, std::variant<T1, T2> &&s) { return d.operator=(std::move(s)); }

template <typename T1, typename T2>
unsigned long f6(const std::variant<T1, T2> &a0) { return a0.index(); }

template <typename T1, typename T2>
bool f7(const std::variant<T1, T2> &a0, const std::variant<T1, T2> &a1) { return operator==(a0, a1); }

template <typename T1, typename T2>
bool f8(const std::variant<T1, T2> &a0, const std::variant<T1, T2> &a1) { return operator!=(a0, a1); }

template <typename T1, typename T2>
std::variant<T1, T2> f9(const T1 &a0) { return std::variant<T1, T2>(a0); }
template <typename T1, typename T2>
std::variant<T1, T2> f10(T1 &a0) { return std::variant<T1, T2>(a0); }
template <typename T1, typename T2>
std::variant<T1, T2> f11(T1 &&a0) { return std::variant<T1, T2>(std::move(a0)); }
template <typename T1, typename T2>
std::variant<T1, T2> &f12(std::variant<T1, T2> &d, const T1 &s) { return d.operator=(s); }
template <typename T1, typename T2>
std::variant<T1, T2> &f13(std::variant<T1, T2> &d, T1 &s) { return d.operator=(s); }
template <typename T1, typename T2>
std::variant<T1, T2> &f14(std::variant<T1, T2> &d, T1 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2>
bool f15(const std::variant<T1, T2> &a0) { return std::holds_alternative<T1>(a0); }
template <typename T1, typename T2>
T1 &f16(std::variant<T1, T2> &a0) { return std::get<T1>(a0); }
template <typename T1, typename T2>
const T1 &f17(const std::variant<T1, T2> &a0) { return std::get<T1>(a0); }

template <typename T1, typename T2>
std::variant<T1, T2> f18(const T2 &a0) { return std::variant<T1, T2>(a0); }
template <typename T1, typename T2>
std::variant<T1, T2> f19(T2 &a0) { return std::variant<T1, T2>(a0); }
template <typename T1, typename T2>
std::variant<T1, T2> f20(T2 &&a0) { return std::variant<T1, T2>(std::move(a0)); }
template <typename T1, typename T2>
std::variant<T1, T2> &f21(std::variant<T1, T2> &d, const T2 &s) { return d.operator=(s); }
template <typename T1, typename T2>
std::variant<T1, T2> &f22(std::variant<T1, T2> &d, T2 &s) { return d.operator=(s); }
template <typename T1, typename T2>
std::variant<T1, T2> &f23(std::variant<T1, T2> &d, T2 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2>
bool f24(const std::variant<T1, T2> &a0) { return std::holds_alternative<T2>(a0); }
template <typename T1, typename T2>
T2 &f25(std::variant<T1, T2> &a0) { return std::get<T2>(a0); }
template <typename T1, typename T2>
const T2 &f26(const std::variant<T1, T2> &a0) { return std::get<T2>(a0); }

// ---- arity 3 -----------------------------------------------------------

template <typename T1, typename T2, typename T3> std::variant<T1, T2, T3> f27() { return std::variant<T1, T2, T3>(); }

template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f28(const std::variant<T1, T2, T3> &a0) { return std::variant<T1, T2, T3>(a0); }

template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f29(std::variant<T1, T2, T3> &&a0) { return std::variant<T1, T2, T3>(std::move(a0)); }

template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> &f30(std::variant<T1, T2, T3> &d, const std::variant<T1, T2, T3> &s) { return d.operator=(s); }

template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> &f31(std::variant<T1, T2, T3> &d, std::variant<T1, T2, T3> &&s) { return d.operator=(std::move(s)); }

template <typename T1, typename T2, typename T3>
unsigned long f32(const std::variant<T1, T2, T3> &a0) { return a0.index(); }

template <typename T1, typename T2, typename T3>
bool f33(const std::variant<T1, T2, T3> &a0, const std::variant<T1, T2, T3> &a1) { return operator==(a0, a1); }

template <typename T1, typename T2, typename T3>
bool f34(const std::variant<T1, T2, T3> &a0, const std::variant<T1, T2, T3> &a1) { return operator!=(a0, a1); }

template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f35(const T1 &a0) { return std::variant<T1, T2, T3>(a0); }
template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f36(T1 &a0) { return std::variant<T1, T2, T3>(a0); }
template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f37(T1 &&a0) { return std::variant<T1, T2, T3>(std::move(a0)); }
template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> &f38(std::variant<T1, T2, T3> &d, const T1 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> &f39(std::variant<T1, T2, T3> &d, T1 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> &f40(std::variant<T1, T2, T3> &d, T1 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3>
bool f41(const std::variant<T1, T2, T3> &a0) { return std::holds_alternative<T1>(a0); }
template <typename T1, typename T2, typename T3>
T1 &f42(std::variant<T1, T2, T3> &a0) { return std::get<T1>(a0); }
template <typename T1, typename T2, typename T3>
const T1 &f43(const std::variant<T1, T2, T3> &a0) { return std::get<T1>(a0); }

template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f44(const T2 &a0) { return std::variant<T1, T2, T3>(a0); }
template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f45(T2 &a0) { return std::variant<T1, T2, T3>(a0); }
template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f46(T2 &&a0) { return std::variant<T1, T2, T3>(std::move(a0)); }
template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> &f47(std::variant<T1, T2, T3> &d, const T2 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> &f48(std::variant<T1, T2, T3> &d, T2 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> &f49(std::variant<T1, T2, T3> &d, T2 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3>
bool f50(const std::variant<T1, T2, T3> &a0) { return std::holds_alternative<T2>(a0); }
template <typename T1, typename T2, typename T3>
T2 &f51(std::variant<T1, T2, T3> &a0) { return std::get<T2>(a0); }
template <typename T1, typename T2, typename T3>
const T2 &f52(const std::variant<T1, T2, T3> &a0) { return std::get<T2>(a0); }

template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f53(const T3 &a0) { return std::variant<T1, T2, T3>(a0); }
template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f54(T3 &a0) { return std::variant<T1, T2, T3>(a0); }
template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f55(T3 &&a0) { return std::variant<T1, T2, T3>(std::move(a0)); }
template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> &f56(std::variant<T1, T2, T3> &d, const T3 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> &f57(std::variant<T1, T2, T3> &d, T3 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> &f58(std::variant<T1, T2, T3> &d, T3 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3>
bool f59(const std::variant<T1, T2, T3> &a0) { return std::holds_alternative<T3>(a0); }
template <typename T1, typename T2, typename T3>
T3 &f60(std::variant<T1, T2, T3> &a0) { return std::get<T3>(a0); }
template <typename T1, typename T2, typename T3>
const T3 &f61(const std::variant<T1, T2, T3> &a0) { return std::get<T3>(a0); }

// ---- arity 4 -----------------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4> std::variant<T1, T2, T3, T4> f62() { return std::variant<T1, T2, T3, T4>(); }

template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> f63(const std::variant<T1, T2, T3, T4> &a0) { return std::variant<T1, T2, T3, T4>(a0); }

template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> f64(std::variant<T1, T2, T3, T4> &&a0) { return std::variant<T1, T2, T3, T4>(std::move(a0)); }

template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> &f65(std::variant<T1, T2, T3, T4> &d, const std::variant<T1, T2, T3, T4> &s) { return d.operator=(s); }

template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> &f66(std::variant<T1, T2, T3, T4> &d, std::variant<T1, T2, T3, T4> &&s) { return d.operator=(std::move(s)); }

template <typename T1, typename T2, typename T3, typename T4>
unsigned long f67(const std::variant<T1, T2, T3, T4> &a0) { return a0.index(); }

template <typename T1, typename T2, typename T3, typename T4>
bool f68(const std::variant<T1, T2, T3, T4> &a0, const std::variant<T1, T2, T3, T4> &a1) { return operator==(a0, a1); }

template <typename T1, typename T2, typename T3, typename T4>
bool f69(const std::variant<T1, T2, T3, T4> &a0, const std::variant<T1, T2, T3, T4> &a1) { return operator!=(a0, a1); }

template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> f70(const T1 &a0) { return std::variant<T1, T2, T3, T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> f71(T1 &a0) { return std::variant<T1, T2, T3, T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> f72(T1 &&a0) { return std::variant<T1, T2, T3, T4>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> &f73(std::variant<T1, T2, T3, T4> &d, const T1 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> &f74(std::variant<T1, T2, T3, T4> &d, T1 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> &f75(std::variant<T1, T2, T3, T4> &d, T1 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4>
bool f76(const std::variant<T1, T2, T3, T4> &a0) { return std::holds_alternative<T1>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
T1 &f77(std::variant<T1, T2, T3, T4> &a0) { return std::get<T1>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
const T1 &f78(const std::variant<T1, T2, T3, T4> &a0) { return std::get<T1>(a0); }

template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> f79(const T2 &a0) { return std::variant<T1, T2, T3, T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> f80(T2 &a0) { return std::variant<T1, T2, T3, T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> f81(T2 &&a0) { return std::variant<T1, T2, T3, T4>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> &f82(std::variant<T1, T2, T3, T4> &d, const T2 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> &f83(std::variant<T1, T2, T3, T4> &d, T2 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> &f84(std::variant<T1, T2, T3, T4> &d, T2 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4>
bool f85(const std::variant<T1, T2, T3, T4> &a0) { return std::holds_alternative<T2>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
T2 &f86(std::variant<T1, T2, T3, T4> &a0) { return std::get<T2>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
const T2 &f87(const std::variant<T1, T2, T3, T4> &a0) { return std::get<T2>(a0); }

template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> f88(const T3 &a0) { return std::variant<T1, T2, T3, T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> f89(T3 &a0) { return std::variant<T1, T2, T3, T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> f90(T3 &&a0) { return std::variant<T1, T2, T3, T4>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> &f91(std::variant<T1, T2, T3, T4> &d, const T3 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> &f92(std::variant<T1, T2, T3, T4> &d, T3 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> &f93(std::variant<T1, T2, T3, T4> &d, T3 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4>
bool f94(const std::variant<T1, T2, T3, T4> &a0) { return std::holds_alternative<T3>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
T3 &f95(std::variant<T1, T2, T3, T4> &a0) { return std::get<T3>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
const T3 &f96(const std::variant<T1, T2, T3, T4> &a0) { return std::get<T3>(a0); }

template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> f97(const T4 &a0) { return std::variant<T1, T2, T3, T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> f98(T4 &a0) { return std::variant<T1, T2, T3, T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> f99(T4 &&a0) { return std::variant<T1, T2, T3, T4>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> &f100(std::variant<T1, T2, T3, T4> &d, const T4 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> &f101(std::variant<T1, T2, T3, T4> &d, T4 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4>
std::variant<T1, T2, T3, T4> &f102(std::variant<T1, T2, T3, T4> &d, T4 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4>
bool f103(const std::variant<T1, T2, T3, T4> &a0) { return std::holds_alternative<T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
T4 &f104(std::variant<T1, T2, T3, T4> &a0) { return std::get<T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4>
const T4 &f105(const std::variant<T1, T2, T3, T4> &a0) { return std::get<T4>(a0); }

// ---- arity 5 -----------------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5> std::variant<T1, T2, T3, T4, T5> f106() { return std::variant<T1, T2, T3, T4, T5>(); }

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f107(const std::variant<T1, T2, T3, T4, T5> &a0) { return std::variant<T1, T2, T3, T4, T5>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f108(std::variant<T1, T2, T3, T4, T5> &&a0) { return std::variant<T1, T2, T3, T4, T5>(std::move(a0)); }

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f109(std::variant<T1, T2, T3, T4, T5> &d, const std::variant<T1, T2, T3, T4, T5> &s) { return d.operator=(s); }

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f110(std::variant<T1, T2, T3, T4, T5> &d, std::variant<T1, T2, T3, T4, T5> &&s) { return d.operator=(std::move(s)); }

template <typename T1, typename T2, typename T3, typename T4, typename T5>
unsigned long f111(const std::variant<T1, T2, T3, T4, T5> &a0) { return a0.index(); }

template <typename T1, typename T2, typename T3, typename T4, typename T5>
bool f112(const std::variant<T1, T2, T3, T4, T5> &a0, const std::variant<T1, T2, T3, T4, T5> &a1) { return operator==(a0, a1); }

template <typename T1, typename T2, typename T3, typename T4, typename T5>
bool f113(const std::variant<T1, T2, T3, T4, T5> &a0, const std::variant<T1, T2, T3, T4, T5> &a1) { return operator!=(a0, a1); }

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f114(const T1 &a0) { return std::variant<T1, T2, T3, T4, T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f115(T1 &a0) { return std::variant<T1, T2, T3, T4, T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f116(T1 &&a0) { return std::variant<T1, T2, T3, T4, T5>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f117(std::variant<T1, T2, T3, T4, T5> &d, const T1 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f118(std::variant<T1, T2, T3, T4, T5> &d, T1 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f119(std::variant<T1, T2, T3, T4, T5> &d, T1 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
bool f120(const std::variant<T1, T2, T3, T4, T5> &a0) { return std::holds_alternative<T1>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
T1 &f121(std::variant<T1, T2, T3, T4, T5> &a0) { return std::get<T1>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
const T1 &f122(const std::variant<T1, T2, T3, T4, T5> &a0) { return std::get<T1>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f123(const T2 &a0) { return std::variant<T1, T2, T3, T4, T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f124(T2 &a0) { return std::variant<T1, T2, T3, T4, T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f125(T2 &&a0) { return std::variant<T1, T2, T3, T4, T5>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f126(std::variant<T1, T2, T3, T4, T5> &d, const T2 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f127(std::variant<T1, T2, T3, T4, T5> &d, T2 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f128(std::variant<T1, T2, T3, T4, T5> &d, T2 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
bool f129(const std::variant<T1, T2, T3, T4, T5> &a0) { return std::holds_alternative<T2>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
T2 &f130(std::variant<T1, T2, T3, T4, T5> &a0) { return std::get<T2>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
const T2 &f131(const std::variant<T1, T2, T3, T4, T5> &a0) { return std::get<T2>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f132(const T3 &a0) { return std::variant<T1, T2, T3, T4, T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f133(T3 &a0) { return std::variant<T1, T2, T3, T4, T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f134(T3 &&a0) { return std::variant<T1, T2, T3, T4, T5>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f135(std::variant<T1, T2, T3, T4, T5> &d, const T3 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f136(std::variant<T1, T2, T3, T4, T5> &d, T3 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f137(std::variant<T1, T2, T3, T4, T5> &d, T3 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
bool f138(const std::variant<T1, T2, T3, T4, T5> &a0) { return std::holds_alternative<T3>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
T3 &f139(std::variant<T1, T2, T3, T4, T5> &a0) { return std::get<T3>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
const T3 &f140(const std::variant<T1, T2, T3, T4, T5> &a0) { return std::get<T3>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f141(const T4 &a0) { return std::variant<T1, T2, T3, T4, T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f142(T4 &a0) { return std::variant<T1, T2, T3, T4, T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f143(T4 &&a0) { return std::variant<T1, T2, T3, T4, T5>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f144(std::variant<T1, T2, T3, T4, T5> &d, const T4 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f145(std::variant<T1, T2, T3, T4, T5> &d, T4 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f146(std::variant<T1, T2, T3, T4, T5> &d, T4 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
bool f147(const std::variant<T1, T2, T3, T4, T5> &a0) { return std::holds_alternative<T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
T4 &f148(std::variant<T1, T2, T3, T4, T5> &a0) { return std::get<T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
const T4 &f149(const std::variant<T1, T2, T3, T4, T5> &a0) { return std::get<T4>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f150(const T5 &a0) { return std::variant<T1, T2, T3, T4, T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f151(T5 &a0) { return std::variant<T1, T2, T3, T4, T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> f152(T5 &&a0) { return std::variant<T1, T2, T3, T4, T5>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f153(std::variant<T1, T2, T3, T4, T5> &d, const T5 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f154(std::variant<T1, T2, T3, T4, T5> &d, T5 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::variant<T1, T2, T3, T4, T5> &f155(std::variant<T1, T2, T3, T4, T5> &d, T5 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
bool f156(const std::variant<T1, T2, T3, T4, T5> &a0) { return std::holds_alternative<T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
T5 &f157(std::variant<T1, T2, T3, T4, T5> &a0) { return std::get<T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5>
const T5 &f158(const std::variant<T1, T2, T3, T4, T5> &a0) { return std::get<T5>(a0); }

// ---- arity 6 -----------------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> std::variant<T1, T2, T3, T4, T5, T6> f159() { return std::variant<T1, T2, T3, T4, T5, T6>(); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f160(const std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::variant<T1, T2, T3, T4, T5, T6>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f161(std::variant<T1, T2, T3, T4, T5, T6> &&a0) { return std::variant<T1, T2, T3, T4, T5, T6>(std::move(a0)); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f162(std::variant<T1, T2, T3, T4, T5, T6> &d, const std::variant<T1, T2, T3, T4, T5, T6> &s) { return d.operator=(s); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f163(std::variant<T1, T2, T3, T4, T5, T6> &d, std::variant<T1, T2, T3, T4, T5, T6> &&s) { return d.operator=(std::move(s)); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
unsigned long f164(const std::variant<T1, T2, T3, T4, T5, T6> &a0) { return a0.index(); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
bool f165(const std::variant<T1, T2, T3, T4, T5, T6> &a0, const std::variant<T1, T2, T3, T4, T5, T6> &a1) { return operator==(a0, a1); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
bool f166(const std::variant<T1, T2, T3, T4, T5, T6> &a0, const std::variant<T1, T2, T3, T4, T5, T6> &a1) { return operator!=(a0, a1); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f167(const T1 &a0) { return std::variant<T1, T2, T3, T4, T5, T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f168(T1 &a0) { return std::variant<T1, T2, T3, T4, T5, T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f169(T1 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f170(std::variant<T1, T2, T3, T4, T5, T6> &d, const T1 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f171(std::variant<T1, T2, T3, T4, T5, T6> &d, T1 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f172(std::variant<T1, T2, T3, T4, T5, T6> &d, T1 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
bool f173(const std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::holds_alternative<T1>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
T1 &f174(std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::get<T1>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
const T1 &f175(const std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::get<T1>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f176(const T2 &a0) { return std::variant<T1, T2, T3, T4, T5, T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f177(T2 &a0) { return std::variant<T1, T2, T3, T4, T5, T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f178(T2 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f179(std::variant<T1, T2, T3, T4, T5, T6> &d, const T2 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f180(std::variant<T1, T2, T3, T4, T5, T6> &d, T2 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f181(std::variant<T1, T2, T3, T4, T5, T6> &d, T2 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
bool f182(const std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::holds_alternative<T2>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
T2 &f183(std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::get<T2>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
const T2 &f184(const std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::get<T2>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f185(const T3 &a0) { return std::variant<T1, T2, T3, T4, T5, T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f186(T3 &a0) { return std::variant<T1, T2, T3, T4, T5, T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f187(T3 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f188(std::variant<T1, T2, T3, T4, T5, T6> &d, const T3 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f189(std::variant<T1, T2, T3, T4, T5, T6> &d, T3 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f190(std::variant<T1, T2, T3, T4, T5, T6> &d, T3 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
bool f191(const std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::holds_alternative<T3>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
T3 &f192(std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::get<T3>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
const T3 &f193(const std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::get<T3>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f194(const T4 &a0) { return std::variant<T1, T2, T3, T4, T5, T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f195(T4 &a0) { return std::variant<T1, T2, T3, T4, T5, T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f196(T4 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f197(std::variant<T1, T2, T3, T4, T5, T6> &d, const T4 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f198(std::variant<T1, T2, T3, T4, T5, T6> &d, T4 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f199(std::variant<T1, T2, T3, T4, T5, T6> &d, T4 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
bool f200(const std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::holds_alternative<T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
T4 &f201(std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::get<T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
const T4 &f202(const std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::get<T4>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f203(const T5 &a0) { return std::variant<T1, T2, T3, T4, T5, T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f204(T5 &a0) { return std::variant<T1, T2, T3, T4, T5, T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f205(T5 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f206(std::variant<T1, T2, T3, T4, T5, T6> &d, const T5 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f207(std::variant<T1, T2, T3, T4, T5, T6> &d, T5 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f208(std::variant<T1, T2, T3, T4, T5, T6> &d, T5 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
bool f209(const std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::holds_alternative<T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
T5 &f210(std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::get<T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
const T5 &f211(const std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::get<T5>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f212(const T6 &a0) { return std::variant<T1, T2, T3, T4, T5, T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f213(T6 &a0) { return std::variant<T1, T2, T3, T4, T5, T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> f214(T6 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f215(std::variant<T1, T2, T3, T4, T5, T6> &d, const T6 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f216(std::variant<T1, T2, T3, T4, T5, T6> &d, T6 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::variant<T1, T2, T3, T4, T5, T6> &f217(std::variant<T1, T2, T3, T4, T5, T6> &d, T6 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
bool f218(const std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::holds_alternative<T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
T6 &f219(std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::get<T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
const T6 &f220(const std::variant<T1, T2, T3, T4, T5, T6> &a0) { return std::get<T6>(a0); }

// ---- arity 7 -----------------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> std::variant<T1, T2, T3, T4, T5, T6, T7> f221() { return std::variant<T1, T2, T3, T4, T5, T6, T7>(); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f222(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f223(std::variant<T1, T2, T3, T4, T5, T6, T7> &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(std::move(a0)); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f224(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, const std::variant<T1, T2, T3, T4, T5, T6, T7> &s) { return d.operator=(s); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f225(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, std::variant<T1, T2, T3, T4, T5, T6, T7> &&s) { return d.operator=(std::move(s)); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
unsigned long f226(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return a0.index(); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
bool f227(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0, const std::variant<T1, T2, T3, T4, T5, T6, T7> &a1) { return operator==(a0, a1); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
bool f228(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0, const std::variant<T1, T2, T3, T4, T5, T6, T7> &a1) { return operator!=(a0, a1); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f229(const T1 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f230(T1 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f231(T1 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f232(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, const T1 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f233(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, T1 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f234(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, T1 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
bool f235(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::holds_alternative<T1>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
T1 &f236(std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::get<T1>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
const T1 &f237(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::get<T1>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f238(const T2 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f239(T2 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f240(T2 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f241(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, const T2 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f242(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, T2 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f243(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, T2 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
bool f244(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::holds_alternative<T2>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
T2 &f245(std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::get<T2>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
const T2 &f246(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::get<T2>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f247(const T3 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f248(T3 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f249(T3 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f250(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, const T3 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f251(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, T3 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f252(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, T3 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
bool f253(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::holds_alternative<T3>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
T3 &f254(std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::get<T3>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
const T3 &f255(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::get<T3>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f256(const T4 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f257(T4 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f258(T4 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f259(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, const T4 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f260(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, T4 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f261(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, T4 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
bool f262(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::holds_alternative<T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
T4 &f263(std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::get<T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
const T4 &f264(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::get<T4>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f265(const T5 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f266(T5 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f267(T5 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f268(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, const T5 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f269(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, T5 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f270(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, T5 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
bool f271(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::holds_alternative<T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
T5 &f272(std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::get<T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
const T5 &f273(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::get<T5>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f274(const T6 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f275(T6 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f276(T6 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f277(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, const T6 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f278(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, T6 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f279(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, T6 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
bool f280(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::holds_alternative<T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
T6 &f281(std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::get<T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
const T6 &f282(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::get<T6>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f283(const T7 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f284(T7 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> f285(T7 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f286(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, const T7 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f287(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, T7 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::variant<T1, T2, T3, T4, T5, T6, T7> &f288(std::variant<T1, T2, T3, T4, T5, T6, T7> &d, T7 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
bool f289(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::holds_alternative<T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
T7 &f290(std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::get<T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
const T7 &f291(const std::variant<T1, T2, T3, T4, T5, T6, T7> &a0) { return std::get<T7>(a0); }

// ---- arity 8 -----------------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f292() { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f293(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f294(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(std::move(a0)); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f295(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &s) { return d.operator=(s); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f296(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &&s) { return d.operator=(std::move(s)); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
unsigned long f297(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return a0.index(); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
bool f298(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0, const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a1) { return operator==(a0, a1); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
bool f299(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0, const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a1) { return operator!=(a0, a1); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f300(const T1 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f301(T1 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f302(T1 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f303(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, const T1 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f304(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T1 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f305(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T1 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
bool f306(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::holds_alternative<T1>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
T1 &f307(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T1>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
const T1 &f308(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T1>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f309(const T2 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f310(T2 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f311(T2 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f312(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, const T2 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f313(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T2 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f314(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T2 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
bool f315(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::holds_alternative<T2>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
T2 &f316(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T2>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
const T2 &f317(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T2>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f318(const T3 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f319(T3 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f320(T3 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f321(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, const T3 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f322(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T3 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f323(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T3 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
bool f324(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::holds_alternative<T3>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
T3 &f325(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T3>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
const T3 &f326(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T3>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f327(const T4 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f328(T4 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f329(T4 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f330(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, const T4 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f331(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T4 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f332(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T4 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
bool f333(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::holds_alternative<T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
T4 &f334(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T4>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
const T4 &f335(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T4>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f336(const T5 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f337(T5 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f338(T5 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f339(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, const T5 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f340(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T5 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f341(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T5 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
bool f342(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::holds_alternative<T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
T5 &f343(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T5>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
const T5 &f344(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T5>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f345(const T6 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f346(T6 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f347(T6 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f348(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, const T6 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f349(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T6 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f350(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T6 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
bool f351(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::holds_alternative<T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
T6 &f352(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T6>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
const T6 &f353(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T6>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f354(const T7 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f355(T7 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f356(T7 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f357(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, const T7 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f358(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T7 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f359(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T7 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
bool f360(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::holds_alternative<T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
T7 &f361(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T7>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
const T7 &f362(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T7>(a0); }

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f363(const T8 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f364(T8 &a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f365(T8 &&a0) { return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(std::move(a0)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f366(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, const T8 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f367(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T8 &s) { return d.operator=(s); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &f368(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &d, T8 &&s) { return d.operator=(std::move(s)); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
bool f369(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::holds_alternative<T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
T8 &f370(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T8>(a0); }
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
const T8 &f371(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) { return std::get<T8>(a0); }

