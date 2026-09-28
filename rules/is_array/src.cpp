// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::is_array<T> -- queue rows g2895..g2899:
//   std::is_array<g3::FatalMessage>, <g3::FileSink>, <g3::LogMessage>,
//   <g3::SinkHandle<g3::FileSink>>, <g3::SinkHandle<mock_rt::ColorCoutSink>>.
//
// THE SITE IS TAG DISPATCH, NOT `::value`.  external/g3log/g3log/std2_make_unique.hpp:44
//     return impl_fut_stl::make_unique_helper<T>(std::is_array<T>(), std::forward<Args>(args)...);
// with the selected overload at :29 taking `std::false_type` BY VALUE.  So
// `std::is_array<T>` is DEFAULT-CONSTRUCTED AS A TEMPORARY and passed as a tag; no
// recorded site reads `std::is_array<T>::value`, which would be dead surface.
// Hence two entries: the TYPE (t1) and the DEFAULT CONSTRUCTOR (f1) -- a type key
// with no constructor translates rc=0 and then fails to compile with error[E0433]
// (measured three times in this port; see rules/plus/src.cpp).
//
// ⭐ KEYED GENERICALLY at <T1>, ONE key for all five rows, not five monomorphic
// ones.  Precedent: rules/plus keys `std::plus<T1>` and that key serves the
// concrete `std::plus<int64_t>` sites.  Nothing in the five rows' signatures
// differs, so a monomorphic key per row would be five copies of one rule.
//
// ⭐ THE REPRESENTATION MUST MATCH rules/integral_constant, because
// `std::is_array<T>()` is precisely the argument that binds to the
// `std::false_type` parameter.  Both are the unit type; see tgt_unsafe.rs.
//
// NOT COVERED, deliberately: `::value`, the `is_array_v` variable template, and the
// true (array) specialisation -- no row and no recorded site, and a missing key
// fails loudly at translate time.

#include <type_traits>

template <typename T1> using t1 = std::is_array<T1>;

template <typename T1> std::is_array<T1> f1() { return std::is_array<T1>(); }
