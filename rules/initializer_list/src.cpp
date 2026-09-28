// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <initializer_list>

template <typename T1> using t1 = std::initializer_list<T1>;

template <typename T1>
typename std::initializer_list<T1>::size_type f1(std::initializer_list<T1> &o) {
  return o.size();
}

// f2 -- THE NULLARY (EMPTY-BRACE) `std::initializer_list` CONSTRUCTOR.
// MEASURED 2026-09-28 on the real corpus TU
// /home/agent/work/repos/dt_src/sys-arch-spec/isa/isa.cpp (bucket A, rc=0,
// 283,526 lines, pin/cpp2rust md5 e2d09f4562c470f813b0fa142d373bfb against
// pin/ir.v21): the type key t1 was recorded but had NO matching CONSTRUCTOR key,
// so the converter emitted
//   std_initializer_list_std_pair_std_string__int__::new_1()
// -- a function defined NOWHERE -- at **99 sites**, with rc=0 and NO placeholder
// token. `grep -c 'impl std_initializer_list_std_pair_std_string__int__'` on that
// same emission is **0**, so every one of the 99 was fabricated by construction.
//
// WHY NULLARY AND NOT AN ELEMENT-TAKING OVERLOAD.  The emitted fabricated call
// takes ZERO arguments, and the source sites are literally empty braces:
//   isa.cpp:259  defineField(10, "imm/pc_target", 6, {}, 21 - 6 + 1, UNSIGNED);
// against the lambda parameter declared at isa.cpp:165 as
//   std::initializer_list<std::pair<std::string, int>> encodeList
// A NON-empty braced list does NOT come through here at all: it is a
// `CXXStdInitializerListExpr`, which converter.cpp:5885 already lowers to
// `vec![...]` directly without consulting the rule table.  Only the empty `{}`
// form reaches the rule table, as libc++'s public
// `constexpr initializer_list() noexcept : __begin_(nullptr), __size_(0) {}`.
// So this is ONE key for ONE overload; there is no sibling to deduce (contrast
// rules/pair f20, where a deduced `_U2 = const int &` split one apparent key
// into three).
//
// LIFETIME LICENCE -- the refusal criterion for this type, discharged.
// `std::initializer_list<T>` is a NON-OWNING VIEW over a temporary array whose
// lifetime ends at the end of the full-expression, and the module already models
// it as an OWNING `Vec<T>` (t1), which in general copies.  That is a real
// question for an element-bearing overload and it is NOT this key: an EMPTY list
// has NO elements, so there is nothing to copy, nothing to alias, and no
// observer -- `size()` is 0 and `begin() == end()` in both models.  The Rust
// value is returned BY VALUE, so nothing dangles under refcount either (the
// ground on which `string_view::front()` and `SMLoc::getPointer()` were refused
// is reference-returning accessors, which this is not).  The one C++ fact not
// reproduced is that `begin()` is a null pointer rather than `Vec::new`'s
// dangling-but-aligned pointer, and that is not observable through any member of
// `std::initializer_list`.
template <typename T1> std::initializer_list<T1> f2() {
  return std::initializer_list<T1>();
}
