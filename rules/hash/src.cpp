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

// `size_t std::__hash_impl<T>::operator()(T) const` IS LEFT OUT, and this is a
// PREPROCESSOR LIMITATION, not a choice.  Written as
//     template <typename T1>
//     std::size_t f1(const std::__hash_impl<T1> &a0, T1 a1) {
//       return a0.operator()(a1);
//     }
// the rule preprocessor RESOLVES the call against the PRIMARY template
//     template <class _Tp, class = void> struct __hash_impl { ... = delete; };
// (hash.h:351-356), which has no `operator()` at all -- only the enum / integral /
// floating-point PARTIAL specialisations do -- and dies with
//     No viable function
//     cpp_rule_preprocessor.cpp:888 Assertion `0 && "Rule resolution failed"'
// So a generic key for this operator cannot be written until the preprocessor can be
// pointed at a partial specialisation.  Deliberately NOT replaced by a concrete key:
// naming `SenComponents` is impossible from a rule source, and a `todo!()`/
// `unimplemented!()` body would be silent wrongness.
//
// The VALUE it would have to produce is fully specified by libc++, transcribed here so
// the next attempt does not have to rederive it:
//
//   hash.h:358-365, the ENUM specialisation
//       struct __hash_impl<_Tp, __enable_if_t<is_enum<_Tp>::value && ...>>
//         size_t operator()(_Tp __v) const {
//           using type = __underlying_type_t<_Tp>;
//           return hash<type>()(static_cast<type>(__v));
//         }
//   hash.h:367-372, the INTEGRAL specialisation for sizeof(T) <= sizeof(size_t)
//       struct __hash_impl<_Tp, __enable_if_t<is_integral<_Tp>::value && ...
//                                             && (sizeof(_Tp) <= sizeof(size_t))>>
//         size_t operator()(_Tp __v) const { return static_cast<size_t>(__v); }
//
// i.e. an enum hashes as its underlying integer and an integer no wider than size_t
// hashes as the IDENTITY CAST -- `v as usize`.  For the two instantiations this program
// reaches (`enum SenComponents : int`, sys-arch-spec/arch_enums.h:13, and `long`) that
// is exactly `a1 as usize`.  This is the one hash rule with no iteration-order risk: a
// wrong hash would change unordered_map iteration order (observable), the mandated one
// cannot.

template <typename T1> using t2 = std::hash<T1>;
