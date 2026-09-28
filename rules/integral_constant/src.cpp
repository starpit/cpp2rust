// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::integral_constant<bool, false>  (i.e. std::false_type), queue row g355.
//
// THE SITE IS TAG DISPATCH, NOT `::value`.  From
// external/g3log/g3log/std2_make_unique.hpp:29
//     make_unique_helper(std::false_type, Args &&... args)
// and :44
//     return impl_fut_stl::make_unique_helper<T>(std::is_array<T>(), ...);
// so the type appears as a BY-VALUE PARAMETER TYPE of the selected overload, and
// the argument bound to it is a default-constructed `std::is_array<T>` prvalue.
// A `::value` key would therefore be DEAD SURFACE at every recorded site: nothing
// reads the member.  What is needed is a TYPE (the parameter has to have a Rust
// type at all) and a DEFAULT CONSTRUCTOR (a type key with no constructor
// translates rc=0 and then fails to compile with error[E0433]; measured three
// times in this port -- see rules/plus/src.cpp).
//
// ⭐ THIS MODULE AND rules/is_array MUST AGREE ON THE REPRESENTATION, because
// `std::is_array<T>()` is exactly the expression that binds to the
// `std::false_type` parameter.  Both are the unit type; see tgt_unsafe.rs.
//
// NOT COVERED, deliberately: std::integral_constant<bool, true> / std::true_type
// (no recorded row or site -- every g3log make_unique site is the non-array
// overload), the `::value` member, the `value_type`/`type` nested typedefs, and
// integral_constant over non-bool integers.  Speculative surface, and a missing
// key fails loudly at translate time, which is the intended behaviour.

#include <type_traits>

using t1 = std::integral_constant<bool, false>;

std::integral_constant<bool, false> f1() {
  return std::integral_constant<bool, false>();
}
