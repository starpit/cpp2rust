// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::plus<T> -- the function object.
//
// IT IS NOT A TEMPLATE ARGUMENT AT ANY SITE IN dt_src.  Every one of the 20+
// recorded sites has the same shape: a TEMPORARY, constructed and passed BY
// VALUE as a runtime argument to a PROJECT-DEFINED method, e.g.
//     dsm/dsm.cpp:6038
//       lDs.HbmStartAddress().apply({}, std::plus<int64_t>(), offset);
//     dsc-based-utils/PCFGToDataflowIR/PCFG2ToDataflowIR.cpp:345
//       fm.apply({}, std::plus<int64_t>(), offset);
// `apply` is user code that gets PORTED, and its body invokes `op(a, b)`.  So
// three entries are needed and an opaque handle is not enough:
//   * the TYPE (t1), or the use site has no Rust type at all;
//   * its DEFAULT CONSTRUCTOR -- a type key with no constructor translates rc=0
//     and then fails to compile with error[E0433] "cannot find module or crate
//     std_plus_..."; that has been measured three separate times in this port;
//   * operator(), or the ported `op(a, b)` inside apply() has no callee.
// The target is therefore a ZERO-SIZED STRUCT WITH A CALL METHOD, not an opaque
// handle.
//
// operator() is spelled as an EXPLICIT MEMBER call, `o.operator()(a, b)`.  The
// bare call form `o(a, b)` does NOT work: the callee is then a DeclRefExpr to a
// parameter and the preprocessor aborts with
// `cpp_rule_preprocessor.cpp:83: Assertion 0 && "Unsupported lookup expression"`.
// MEASURED 2026-09-27, this exact module.
//
// NOT COVERED: the transparent specialisation std::plus<void> (no site uses it),
// and the other arithmetic/comparison functors (minus, multiplies, less, ...),
// each of which needs its own module or its own keys.

#include <functional>

template <typename T1> using t1 = std::plus<T1>;

template <typename T1> std::plus<T1> f1() { return std::plus<T1>(); }

template <typename T1> T1 f2(const std::plus<T1> &o, const T1 &a, const T1 &b) {
  return o.operator()(a, b);
}

// ---------------------------------------------------------------------------
// t2/f3/f4 -- std::multiplies<T1>; t3/f5/f6 -- std::divides<T1>.
//
// These are the SAME GENERIC SHAPE as t1/f1/f2 above: libcxx
// __functional/operations.h:96 `struct multiplies : __binary_function<_Tp,_Tp,_Tp>`
// and :122 `struct divides : __binary_function<_Tp,_Tp,_Tp>`, i.e. stateless
// empty types with a default ctor and `operator()(const _Tp&, const _Tp&) const`.
// So every reason recorded above for needing THREE entries per functor (type,
// default ctor, operator()) applies unchanged, and the explicit-member call form
// `o.operator()(a, b)` is used for the same measured reason.
//
// The rows are recorded at CONCRETE instantiations -- std::multiplies<double>
// (g2908), std::multiplies<long> (g2909, g2910), std::divides<long> (g472) --
// but they are keyed GENERICALLY at <T1>, exactly as t1 is: the existing
// std::plus<T1> key already serves std::plus<int64_t> sites, which is the
// precedent that a generic functor key matches a concrete instantiation.  This
// is NOT a collapsed-template-argument instance: _Tp is a real type parameter
// deduced from the declaration, not an explicit template argument on a member.
//
// NOT COVERED, deliberately: minus, negate, modulus, and the comparison
// functors.  No queue row asks for them and there is no recorded site, so a key
// for them could not be justified -- they would be speculative surface.
// std::multiplies<void>/std::divides<void> (the transparent specialisations) are
// likewise unused.

template <typename T1> using t2 = std::multiplies<T1>;

template <typename T1> std::multiplies<T1> f3() { return std::multiplies<T1>(); }

template <typename T1>
T1 f4(const std::multiplies<T1> &o, const T1 &a, const T1 &b) {
  return o.operator()(a, b);
}

template <typename T1> using t3 = std::divides<T1>;

template <typename T1> std::divides<T1> f5() { return std::divides<T1>(); }

template <typename T1>
T1 f6(const std::divides<T1> &o, const T1 &a, const T1 &b) {
  return o.operator()(a, b);
}
