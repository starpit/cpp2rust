// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// libc++'s INTERNAL hash base, `std::__hash_impl<_Tp, class = void>`.
//
// WHY THE BASE AND NOT `std::hash`.  `std::hash<_Tp>` is
// `template <class _Tp> struct hash : public __hash_impl<_Tp> {};`
// (toolchain/libcxx/__functional/hash.h:434), so for every LIBRARY-PROVIDED
// specialisation the whole implementation lives in `__hash_impl`.  A generic
// `std::hash<T1>` key would additionally match the FOUR user specialisations this
// program writes (`std::hash<DataLocation>`, `std::hash<SdscHasher>`,
// `std::hash<mlir::Value>`, `std::hash<ddc::shuffle::DimSymbol>`), which the
// converter already ports from their user declarations -- so it risks displacing
// working output.  `std::__hash_impl` is libc++-internal and is never specialised
// anywhere in dt_src, so this key cannot displace anything.
//
// THE KEY IS THE SUGARED ONE-ARGUMENT SPELLING.  The diagnostic prints the
// canonicalised `std::__hash_impl<SenComponents, void>` (defaulted arg kept), but the
// form the matcher SEARCHES is `std::__hash_impl<SenComponents>`.  Read the recorded
// key back out of <tree>/hash/ir_src.json rather than trusting the diagnostic.
//
// THE CALLER IS USER CODE, NOT A CONTAINER.  util/utils.h:168-172
//     template <class T> void hash_combine(std::size_t &seed, const T &v) {
//       std::size_t x = seed + 0x9e3779b9 + std::hash<T>()(v);
// instantiated at T = SenComponents from dsc/superdsc.h:184 and
// ddc/.../shuffle.h:133.  Nothing on that path touches `__hash_table`.

#include <cstddef>
#include <functional>

template <typename T1> using t1 = std::__hash_impl<T1>;

// `size_t std::__hash_impl<T1>::operator()(T1) const` -- f1 below.  ⭐ IT CLOSES THE
// WHOLE `0(v)` CLASS: 427 call sites in 190 of 312 corpus TUs emitted
// `return ((unsafe { 0((*x).storage_) }) ^ ...)`, i.e. `error[E0618] expected
// function, found {integer}`, because t2 gave `std::hash<T1>` a scalar `usize` model
// with NO `operator()` key, so the functor's own DEFAULT VALUE landed in callee
// position.  ⛔ Standing lesson: A TYPE KEY WITH NO `operator()` KEY IS STRICTLY WORSE
// THAN NO KEY AT ALL -- `std::less<int>` has no type key and aborts loudly with
// `unsupported system type has no rule`, while `std::hash<int>` had one and silently
// emitted `0(v)`.
//
// ⛔⛔ WHY THE `typename T1 = HashableEnumHint` DEFAULT ARGUMENT IS LOAD-BEARING AND
// MUST NOT BE DELETED.  libc++ is
//     template <class _Tp, class = void> struct __hash_impl { ... = delete; };   :351
//     template <class _Tp> struct __hash_impl<_Tp,
//         __enable_if_t<is_enum<_Tp>::value && __is_unqualified_v<_Tp>>>          :358
//       : __unary_function<_Tp, size_t> { size_t operator()(_Tp) const; };
// -- the PRIMARY template has no `operator()` at all; only the enum / integral /
// floating-point PARTIAL SPECIALISATIONS have one.  cpp_rule_preprocessor models an
// un-hinted `typename T1` as an empty STRUCT, and no type trait holds for an empty
// struct, so resolution picked the PRIMARY and the tool died.  MEASURED, three ways,
// 2026-09-29:
//   * `f1(const std::hash<T1>&, const T1&) -> o.operator()(a1)` (the DERIVED
//     spelling, since `struct hash : public __hash_impl<_Tp>`, hash.h:434):
//         LLVM ERROR: rule resolution failed for `f1`: could not resolve the call
//         to `operator()` (no viable function)
//     -- and before f90878cb that same path was a bare SIGSEGV.
//   * naming the enum partial specialisation's own enable_if generically,
//     `std::__hash_impl<T1, std::__enable_if_t<std::is_enum<T1>::value && ...>>`:
//     rc=139 SEGFAULT with NO diagnostic -- the enable_if is a SUBSTITUTION FAILURE
//     against the struct stand-in, so the rule DECLARATION itself does not survive
//     instantiation and lookup is never reached.  That crash is now loud.
//   * concrete `std::hash<long>` / `std::hash<int>` keys: rc=0, but the 427 sites need
//     `std::__hash_impl<SenComponents>` -- a PROJECT enum, UNNAMEABLE from a rule
//     source -- so those keys would be DEAD.  Deliberately not landed.
// The default argument tells the preprocessor to build the stand-in for T1 as an
// ENUMERATION rather than a struct, which is the one thing that makes the enum partial
// specialisation selectable.  The stand-in is a FRESH empty enum NAMED `T1`, so the
// recorded key stays GENERIC -- read it back out of <tree>/hash/ir_src.json:
//     "f1": "unsigned long std::__hash_impl<T1>::operator()(T1) const"
// `HashableEnumHint` itself is never keyed and never appears in the key.
//
// THE VALUE, from libc++, not guessed:
//   hash.h:358-365 ENUM:      return hash<__underlying_type_t<_Tp>>()(static_cast<...>(v))
//                             -> which lands on the integral specialisation below
//   hash.h:367-372 INTEGRAL, sizeof(_Tp) <= sizeof(size_t):
//                             return static_cast<size_t>(v)     -- the IDENTITY CAST
// so an enum hashes as its underlying integer and a size_t-or-narrower integer hashes
// as itself: `a1 as usize`.  Both instantiations this program reaches are covered --
// `enum SenComponents : int` (sys-arch-spec/arch_enums.h:13, emitted as
// `pub type SenComponents = i32;`) and `long`.  Rust `as usize` on a signed integer
// sign-extends then reinterprets, which is exactly what `static_cast<size_t>` does, so
// `SenComponents::NO_COMPONENT == -1` hashes to 0xFFFF_FFFF_FFFF_FFFF in both.
//
// ⚠️ WHAT THIS GENERIC KEY DOES **NOT** MODEL, STATED RATHER THAN LEFT IMPLICIT.  The
// key matches `std::__hash_impl<T1>::operator()(T1)` for ANY T1, including the
// FLOATING-POINT partial specialisation (hash.h:379-387) and `__hash_impl<long double>`
// (:390), whose bodies are `__scalar_hash`, NOT the identity cast.  For those, `a1 as
// usize` would be a WRONG HASH -- the silent kind, missed HashMap lookups.  It is
// emitted anyway because dt_src instantiates NO floating-point hash: measured
//     grep -rnE "unordered_(map|set|multimap|multiset)< *(float|double|long double)
//               |hash< *(float|double|long double)" --include=*.cpp --include=*.h
// over all of dt_src outside cpp2rust-port -> ZERO hits, and the same grep restricted
// to unordered_map/set -> 0 files.  ⛔ IF A FLOATING-POINT `std::hash` SITE EVER
// APPEARS, THIS KEY MUST BE SPLIT before it is allowed to serve it; the key language
// has no way to constrain T1 to non-floating types.  Pointers are unaffected:
// `std::hash<_Tp*>` (hash.h:339) is its own specialisation of `hash`, not of
// `__hash_impl`, so it does not match this key and still aborts loudly.
//
// t1's `std::__hash_impl<T1>` type key is kept: it is what gives the receiver a Rust
// type, and this key's own parameter names it.

// The stand-in HINT.  Never keyed, never emitted, never named in any recorded key --
// its only job is to carry the KIND (enumeration) that T1's stand-in must have.
enum HashableEnumHint : int {};

template <typename T1 = HashableEnumHint>
std::size_t f1(const std::__hash_impl<T1> &o, T1 a1) {
  return o.operator()(a1);
}

template <typename T1> using t2 = std::hash<T1>;
