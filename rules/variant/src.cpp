// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::variant<...> -- modelled as a REAL Rust enum (libcc2rs::Variant2 /
// Variant3), one type key per ARITY, exactly as rules/tuple does.
//
// WHY THIS IS EXPRESSIBLE AT ALL: a rule module cannot declare an enum, but a
// rule TARGET CAN NAME A libcc2rs TYPE (rules/map/tgt_unsafe.rs names
// UnsafeMapIterator), so the enum lives in libcc2rs and these keys only name
// it.  The per-arity shape is the one std::tuple proved up to 27-ary.
//
// WHY AN ENUM AND NOT A TUPLE: a tuple lets a read of an INACTIVE alternative
// succeed, which is the one thing a variant exists to forbid.
//
// ⛔ THE PARAGRAPH THAT STOOD HERE -- "std::get IS DELIBERATELY NOT KEYED,
// `get<0>` and `get<1>` record BYTE-IDENTICAL keys" -- IS MEASURABLY FALSE FOR
// std::get AND IS RETRACTED.  The explicit INDEX does not reach the key, but the
// RETURN TYPE does, and a variant's alternatives are distinct types, so the eight
// keys are pairwise distinct.  std::get IS NOW KEYED: f21-f36, and the twelve
// search keys it was verified against are quoted verbatim there.
// ⭐ holds_alternative<T> IS STILL NOT KEYED, and now for a MEASURED reason
// rather than this one: its whole key is `_Bool (const variant<..8..> &)` --
// bool return, no alternative anywhere in it -- so all four call sites in
// dxp_standalone.cpp record ONE key and any rule would answer three of them
// wrongly and silently.  See the note above f21.
// index() is keyable because it carries no explicit template argument.

#include <cstddef>
#include <variant>

template <typename T1, typename T2> using t1 = std::variant<T1, T2>;

template <typename T1, typename T2, typename T3>
using t2 = std::variant<T1, T2, T3>;

// ARITY 8 -- THE ONLY ARITY THE CORPUS ACTUALLY INSTANTIATES.  Measured on
// dxp_standalone: exactly one std::variant instantiation exists,
// `OperandAttr::data_` at sys-arch-spec/progir/progir.h:256, and it is 8-ary;
// arities 2 and 3 have ZERO uses.  t1/t2 above were therefore not merely
// unused, they were HARMFUL: GetTypeMapKey (mapper.cpp:94) truncates the bucket
// key at the first `<`, so ARITY IS NOT PART OF THE KEY and t1/t2 were
// candidates for the 8-ary type.  matchTemplate (mapper.cpp:250ff) resolves each
// Tn by scanning to the literal text that follows it in the rule src, and for
// `std::variant<T1, T2, T3>` the text after T3 is `>`, which
// findNextLiteralSameDepth finds at the FINAL depth-0 position -- so T3 captured
// alternatives 3..8 JOINED INTO ONE STRING, mapper.cpp:1242 recursed on that
// non-type, and the assert at :1233 (a no-op under the release build's NDEBUG)
// fell through to a null deref: rc=139 on Pipeline.cpp, StageCoarsening.cpp and
// RunProgramPipelines.cpp.  An 8-placeholder src beats t2 DETERMINISTICALLY
// rather than by luck: `search` (mapper.cpp:383) tie-breaks on src.size() and
// prefers the LONGER src.  kMaxGenerics is 64 (translation_rule.h:23), so 8 is
// well inside the limit.
template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
using t3 = std::variant<T1, T2, T3, T4, T5, T6, T7, T8>;

// Default construction value-initialises ALTERNATIVE 0.
template <typename T1, typename T2> std::variant<T1, T2> f1() {
  return std::variant<T1, T2>();
}

template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f2() {
  return std::variant<T1, T2, T3>();
}

// Construction from a value.  The ALTERNATIVE is identified by the PARAMETER
// TYPE, not by an explicit template argument, so each of these records a
// distinct key -- which is what makes them safe to model where get<> is not.
template <typename T1, typename T2> std::variant<T1, T2> f3(T1 a0) {
  return std::variant<T1, T2>(a0);
}

template <typename T1, typename T2> std::variant<T1, T2> f4(T2 a0) {
  return std::variant<T1, T2>(a0);
}

template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f5(T1 a0) {
  return std::variant<T1, T2, T3>(a0);
}

template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f6(T2 a0) {
  return std::variant<T1, T2, T3>(a0);
}

template <typename T1, typename T2, typename T3>
std::variant<T1, T2, T3> f7(T3 a0) {
  return std::variant<T1, T2, T3>(a0);
}

template <typename T1, typename T2>
std::size_t f8(const std::variant<T1, T2> &a0) {
  return a0.index();
}

template <typename T1, typename T2, typename T3>
std::size_t f9(const std::variant<T1, T2, T3> &a0) {
  return a0.index();
}

// ---- arity 8 ------------------------------------------------------------
// A TYPE KEY WITHOUT ITS CONSTRUCTORS GIVES rc=0 AND THEN E0433: the converter
// looks the default ctor up as an ordinary expr rule and, on a miss, falls back
// to `<mangled-type>::new()`, which does not exist.  So t3 is accompanied by its
// default ctor and ALL EIGHT value ctors.  The alternative is identified by the
// PARAMETER TYPE, never by an explicit template argument, so each of the eight
// records a DISTINCT key -- which is exactly why these are safe to model where
// std::get<I> is not.
template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f10() {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>();
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f11(T1 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f12(T2 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f13(T3 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f14(T4 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f15(T5 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f16(T6 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f17(T7 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::variant<T1, T2, T3, T4, T5, T6, T7, T8> f18(T8 a0) {
  return std::variant<T1, T2, T3, T4, T5, T6, T7, T8>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
std::size_t f19(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return a0.index();
}

// ---- f20 -- operator== on two 8-ary variants (row g848) ---------------------
//
// Site: sys-arch-spec/progir/progir.cpp:69 `return attr1.data_ == attr2.data_;`
// on the 8-ary `OperandAttr::data_`, the only std::variant the corpus
// instantiates.
//
// ⛔ WHAT MAKES THIS CORRECT RATHER THAN A PAYLOAD COMPARISON.  std::variant
// equality compares the ACTIVE INDEX FIRST and only then the held value: two
// variants holding equal-looking data in DIFFERENT alternatives are NOT equal.
// A model that unwrapped and compared payloads would answer `true` for
// `variant<long, float>(1L)` vs `variant<long, float>(1.0f)`.
//
// This module models std::variant as a REAL Rust enum (libcc2rs::Variant8), and
// that enum is `#[derive(Clone, Debug, PartialEq)]` (libcc2rs/src/variant.rs:42).
// A derived PartialEq on an enum compares the DISCRIMINANT first and the payload
// only within the matching arm -- byte-for-byte the C++ rule, for free. This is
// the payoff of the enum decision recorded at the top of this file; had variant
// been modelled as a tuple there would be no faithful body to write here.
//
// Eight placeholders, not two or three, for the reason t3 documents: arity is not
// part of the bucket key and `search` (mapper.cpp:474) prefers the LONGER src, so
// the 8-placeholder form beats a shorter one deterministically instead of by luck,
// and it cannot swallow alternatives 3..8 into T3 the way `t2` did.
template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
bool f20(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0,
         const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a1) {
  return operator==(a0, a1);
}

// ---- f21-f36 -- std::get<I> on the 8-ary variant (the 18-site `get` class on
// ---- the port goal, dxp/dxp_standalone.cpp) ---------------------------------
//
// ⭐ THIS RETRACTS THIS FILE'S OWN REFUSAL NOTE AT THE TOP, WHICH IS MEASURABLY
// FALSE FOR std::get.  That note says `get<0>` and `get<1>` "record
// BYTE-IDENTICAL keys (the explicit non-type template argument does not reach the
// key)".  The INDEX indeed does not reach the key -- but THE RETURN TYPE DOES,
// and for a variant every alternative has a different type, so the eight keys are
// pairwise distinct.  Read back verbatim out of the converter's own search lines
// on dxp_standalone.cpp (all twelve, with the allocator/comparator arguments
// elided here only for width):
//     const long &                     (const variant<..8..> &)
//     const std::map<int, long> &      (const variant<..8..> &)
//     const float &                    (const variant<..8..> &)
//     const std::map<int, float> &     (const variant<..8..> &)
//     const std::array<unsigned, 4> &  (const variant<..8..> &)
//     const std::map<int, std::array<unsigned,4> > & (const variant<..8..> &)
//     const std::string &              (const variant<..8..> &)
//     const std::map<int, std::string> & (const variant<..8..> &)
//     std::map<int, long> &            (variant<..8..> &)
//     std::map<int, float> &           (variant<..8..> &)
//     std::map<int, std::array<unsigned,4> > & (variant<..8..> &)
//     std::map<int, std::string> &     (variant<..8..> &)
// -- i.e. the CONST slice uses all eight alternatives and the NON-CONST slice
// four of them.  A search key binds T1..T8 from the variant's alternative list
// and then the return type selects exactly one of the sixteen srcs; binding
// `const T2 &` from a `get<0>` call would contradict the T2 bound from the
// variant's second alternative, so there is no tie.  Same mechanism rules/tuple
// f10/f11 landed on.
//
// ⛔ THE ONE CASE THIS IS WRONG FOR, stated so it is not rediscovered:
// `std::variant<X, ..., X>` with two EQUAL alternatives -- both keys then match
// and `search()` would have to tie-break between equal-length srcs.  The corpus
// has exactly ONE variant instantiation (`OperandAttr::data_`,
// sys-arch-spec/progir/progir.h:256) and its eight alternatives are pairwise
// distinct, so this is not exercised; if such a variant appears it must abort,
// not pick.
//
// ⛔ WHY ALL SIXTEEN AND NOT THE TWELVE THE GOAL TU USES: a key set that covers
// some alternatives and not others is the worst outcome, because the covered
// sites go quiet while the uncovered ones stay loud and the file then reads as
// "std::get is handled".  Both const-ness slices, all eight alternatives.
//
// ⛔ WHY holds_alternative<T> IS STILL NOT KEYED, now measured rather than
// assumed: its recorded key is `_Bool (const variant<..8..> &) noexcept` -- the
// alternative appears ONLY as the explicit template argument, and the converter's
// search key does NOT carry explicit template arguments (all four call sites in
// dxp_standalone record the ONE key above, which is why they became four
// distinct `Cpp2RustUnmappedFn_holds_alternative_*` names for a single key).  A
// rule for it could only ever test one fixed alternative and would answer three
// of the four sites WRONGLY AND SILENTLY.  `--explicit-template-args` does not
// rescue it: that flag changes what the PREPROCESSOR records for the rule, not
// what the CONVERTER searches for, so it would only make the key dead.  It stays
// a loud placeholder.
//
// BODIES.  `Variant8` is a real Rust enum, so the wrong-alternative path has no
// field to read: the body matches and PANICS, which is the faithful model of
// `throw std::bad_variant_access`.  That is a real behaviour, not a stub.
// tgt_refcount.rs deliberately omits all sixteen for exactly the reason it omits
// f8/f9/f19/f20 -- refcount passes a variant reference as `Ptr<Variant8<..>>`
// whose accessors are bounded on `ByteRepr`, which an enum over arbitrary
// alternatives cannot satisfy -- so refcount inherits the unsafe body and fails
// to COMPILE rather than returning a wrong alternative.

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
const T1 &f21(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<0>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
const T2 &f22(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<1>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
const T3 &f23(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<2>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
const T4 &f24(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<3>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
const T5 &f25(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<4>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
const T6 &f26(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<5>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
const T7 &f27(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<6>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
const T8 &f28(const std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<7>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
T1 &f29(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<0>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
T2 &f30(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<1>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
T3 &f31(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<2>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
T4 &f32(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<3>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
T5 &f33(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<4>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
T6 &f34(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<5>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
T7 &f35(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<6>(a0);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
T8 &f36(std::variant<T1, T2, T3, T4, T5, T6, T7, T8> &a0) {
  return std::get<7>(a0);
}
