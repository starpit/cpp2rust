// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <optional>

template <typename T1> using t1 = std::optional<T1>;

// ---------------------------------------------------------------------------
// construction
// ---------------------------------------------------------------------------

template <typename T1> std::optional<T1> f1() { return std::optional<T1>(); }

template <typename T1> std::optional<T1> f2(std::nullopt_t a0) {
  return std::optional<T1>(a0);
}

// The converting constructor is `template <class U> optional(U&&)`, so each
// argument value category deduces a different signature.
template <typename T1, typename T2> std::optional<T1> f3(T2 &&a0) {
  return std::optional<T1>(std::move(a0));
}

template <typename T1, typename T2> std::optional<T1> f4(T2 &a0) {
  return std::optional<T1>(a0);
}

template <typename T1, typename T2> std::optional<T1> f5(const T2 &a0) {
  return std::optional<T1>(a0);
}

template <typename T1> std::optional<T1> f6(const std::optional<T1> &a0) {
  return std::optional<T1>(a0);
}

template <typename T1> std::optional<T1> f7(std::optional<T1> &&a0) {
  return std::optional<T1>(std::move(a0));
}

// ---------------------------------------------------------------------------
// assignment
// ---------------------------------------------------------------------------

template <typename T1>
std::optional<T1> &f8(std::optional<T1> &dst, const std::optional<T1> &src) {
  return dst.operator=(src);
}

template <typename T1>
std::optional<T1> &f9(std::optional<T1> &dst, std::optional<T1> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1>
std::optional<T1> &f10(std::optional<T1> &dst, std::nullopt_t src) {
  return dst.operator=(src);
}

template <typename T1, typename T2>
std::optional<T1> &f11(std::optional<T1> &dst, T2 &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1, typename T2>
std::optional<T1> &f12(std::optional<T1> &dst, T2 &src) {
  return dst.operator=(src);
}

template <typename T1, typename T2>
std::optional<T1> &f13(std::optional<T1> &dst, const T2 &src) {
  return dst.operator=(src);
}

// ---------------------------------------------------------------------------
// observers
// ---------------------------------------------------------------------------

template <typename T1> bool f15(const std::optional<T1> &a0) {
  return a0.operator bool();
}

template <typename T1> T1 &f16(std::optional<T1> &a0) { return a0.value(); }

template <typename T1> const T1 &f17(const std::optional<T1> &a0) {
  return a0.value();
}

template <typename T1> T1 &f18(std::optional<T1> &a0) {
  return a0.operator*();
}

template <typename T1> const T1 &f19(const std::optional<T1> &a0) {
  return a0.operator*();
}

template <typename T1> T1 *f20(std::optional<T1> &a0) {
  return a0.operator->();
}

template <typename T1> const T1 *f21(const std::optional<T1> &a0) {
  return a0.operator->();
}

template <typename T1, typename T2>
T1 f22(const std::optional<T1> &a0, T2 &&a1) {
  return a0.value_or(std::move(a1));
}

template <typename T1, typename T2>
T1 f23(const std::optional<T1> &a0, T2 &a1) {
  return a0.value_or(a1);
}

template <typename T1, typename T2>
T1 f24(const std::optional<T1> &a0, const T2 &a1) {
  return a0.value_or(a1);
}

// ---------------------------------------------------------------------------
// comparison
// ---------------------------------------------------------------------------

template <typename T1>
bool f26(const std::optional<T1> &a0, const std::optional<T1> &a1) {
  return operator==(a0, a1);
}

template <typename T1>
bool f27(const std::optional<T1> &a0, const std::optional<T1> &a1) {
  return operator!=(a0, a1);
}

template <typename T1>
bool f28(const std::optional<T1> &a0, std::nullopt_t a1) {
  return operator==(a0, a1);
}

template <typename T1>
bool f29(const std::optional<T1> &a0, std::nullopt_t a1) {
  return operator!=(a0, a1);
}

// The MIXED-VALUE overloads: libc++ declares
//   template <class _Tp, class _Up> bool operator==(const optional<_Tp>&, const _Up&)
// (and the != twin) as free function TEMPLATES with a SECOND, independent
// template parameter -- the optional is compared against a BARE VALUE, not
// against another optional and not against nullopt_t.  That is the shape at
// dcc/src/Transform/Sentient/Analyses/PropagationAnalysis.cpp:316,
// `common_value != const_op.getValue()`, where common_value is a
// std::optional<int64_t> (declared :291) and getValue() yields an int64_t.
// Neither f26/f27 (optional vs optional) nor f28/f29 (optional vs nullopt_t)
// covers it, so those four resolving is not evidence that this one does.
//
// SEMANTICS, stated because two wrong bodies pass a careless test.  The
// standard says ([optional.comparewitht]) the result is
// `bool(x) ? *x == v : false` for == and `bool(x) ? *x != v : true` for !=.
// So a DISENGAGED optional is NEVER equal to a bare value -- it is not "equal
// because there is nothing to disagree with", and it is not an unwrap of an
// empty optional either.  A body that only tests engagement, or one that
// unwraps unconditionally, is silently wrong on exactly one of the two cases,
// which is why the probe exercises engaged-and-equal, engaged-and-unequal AND
// disengaged.
//
// T2 is DROPPED on the target side, exactly as f5 already drops it for the
// converting constructor: the model represents the bare value at the same Rust
// type as the contained one, which is what the sites mean (int64_t vs
// int64_t).  A genuinely heterogeneous comparison would need
// T1: PartialEq<T2>; no site in scope has one.
template <typename T1, typename T2>
bool f30(const std::optional<T1> &a0, const T2 &a1) {
  return operator==(a0, a1);
}

template <typename T1, typename T2>
bool f31(const std::optional<T1> &a0, const T2 &a1) {
  return operator!=(a0, a1);
}

// The MIXED-VALUE RELATIONAL overloads, the <,>,<=,>= twins of f30/f31.  libc++
// declares each as a free function TEMPLATE with a second, independent template
// parameter, e.g.
//   template <class _Tp, class _Up> bool operator>=(const optional<_Tp>&, const _Up&)
// so they are spelled FREE here, exactly like f30/f31, and T2 is DROPPED on the
// target side for the same reason (the model represents the bare value at the
// same Rust type as the contained one; a genuinely heterogeneous comparison
// would need T1: PartialOrd<T2> and no site in scope has one).
//
// NOTE ON THE MANGLED NAME, because it has already cost one agent a cycle: the
// IR spells `>=` as `operator ge` (and `<=` as `operator le`, `<<` as
// `operator shl`).  Seeing `operator ge` in a --verbose dump is the NORMAL
// spelling, not evidence that a rule failed to bind.
//
// THE SITE: dcc/src/Transform/Sentient/AnnotateMacXRFWtRange.cpp:104 and :106,
// `min_val < 64` and `max_val >= 64`, where min_val/max_val are
// std::optional<int64_t> (declared :97/:100) and 64 is a bare int.  So this TU
// needs BOTH `<` and `>=`; `>` and `<=` are here because they are the same
// one-liner and a hole in the table aborts a whole TU at the next site.
//
// SEMANTICS, AND THEY ARE NOT ==' s.  [optional.comparewitht] gives
//     x <  v  ==  bool(x) ? *x <  v : true
//     x >  v  ==  bool(x) ? *x >  v : false
//     x <= v  ==  bool(x) ? *x <= v : true
//     x >= v  ==  bool(x) ? *x >= v : false
// i.e. a DISENGAGED optional is strictly LESS THAN every bare value -- so
// `nullopt >= x` is FALSE and `nullopt < x` is TRUE.  That is a different rule
// from ==, where disengaged is merely unequal in both directions, and it is the
// case a plausible-looking body (test engagement, or unwrap and compare) gets
// wrong.  It also means `min_val < 64` at the site above is TRUE when the
// analysis found no constant, which is load-bearing for that pass.
//
// ORDER MATTERS here in a way it did not for ==/!=: these operators are not
// commutative, so the probe checks engaged-and-greater against
// engaged-and-less and not merely that the thing compiles.
template <typename T1, typename T2>
bool f32(const std::optional<T1> &a0, const T2 &a1) {
  return operator<(a0, a1);
}

template <typename T1, typename T2>
bool f33(const std::optional<T1> &a0, const T2 &a1) {
  return operator>(a0, a1);
}

template <typename T1, typename T2>
bool f34(const std::optional<T1> &a0, const T2 &a1) {
  return operator<=(a0, a1);
}

template <typename T1, typename T2>
bool f35(const std::optional<T1> &a0, const T2 &a1) {
  return operator>=(a0, a1);
}

// ---------------------------------------------------------------------------
// modifiers / has_value()
//
// libc++ declares has_value() and reset() in *private base classes* of
// std::optional and re-exports has_value() with a using-declaration, so the
// rule preprocessor cannot reach either one through std::optional itself.
// Naming the base class directly does work, and the converter looks the call
// up under the base class name too, so that is what these rules do.
//
// The guard is on _LIBCPP_VERSION -- the STANDARD LIBRARY -- and not on
// __APPLE__, the host OS. Those are not the same condition and conflating them
// cost 10 rustc errors on dsc/test/operandattr_unit_test.cpp: this pod is
// Linux, but the rule preprocessor parses with libc++ (CPLUS_INCLUDE_PATH
// points at it, because the rules key on its std::__1:: spellings), so
// __APPLE__ was false, f14/f25 were dropped from ir_src.json entirely, and
// every `opt.has_value()` fell through to a verbatim `.has_value()` on a Rust
// Option -- E0599, since Option spells it `is_some`. Keying on the library
// makes the rule track what is actually being parsed. Verified directly:
// naming std::optional rather than the base class still fails rule resolution
// ("No viable function"), so the base-class spelling remains necessary.
// ---------------------------------------------------------------------------
#if defined(_LIBCPP_VERSION)
template <typename T1> using t2 = std::__optional_storage_base<T1>;
template <typename T1> using t3 = std::__optional_destruct_base<T1>;

template <typename T1>
bool f14(const std::__optional_storage_base<T1> &a0) {
  return a0.has_value();
}

template <typename T1> void f25(std::__optional_destruct_base<T1> &a0) {
  return a0.reset();
}
#endif
