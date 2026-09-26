// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstddef>

using t1 = std::byte;

std::byte f1(const std::byte &a0, unsigned a1) { return operator<<(a0, a1); }

std::byte f2(const std::byte &a0, unsigned a1) { return operator>>(a0, a1); }

std::byte f3(std::byte &a0, unsigned a1) { return operator<<=(a0, a1); }

std::byte f4(std::byte &a0, unsigned a1) { return operator>>=(a0, a1); }

// ---------------------------------------------------------------------------
// std::nullptr_t -- `<cstddef>` is where the standard declares it ([support.types]),
// which is why it lands in this module rather than rules/builtin (which maps
// only __builtin_* FUNCTIONS and no types at all).
//
// WHAT THE GAP ACTUALLY IS.  32 `UnmappedType std::nullptr_t` records across 3
// TUs (dcg/dcg_fe/pcfg_gen/dlOps.cpp, dcc/src/Transform/Sentient/Utils.cpp,
// dcc/src/Transform/Sentient/Analyses/Utils.cpp).  Every site is the SAME shape:
// `std::make_pair(nullptr, nullptr)` / `std::make_tuple(nullptr, 0, 0, 0, 0)`,
// where template argument deduction picks T = std::nullptr_t and so the type
// appears as a template ARGUMENT that must be nameable.  It is NOT the
// `p != nullptr` shape -- that is already covered, as a PARAMETER type, by
// rules/unique_ptr f18-f21 and rules/shared_ptr f23/f24/f40/f41, which take
// `std::nullptr_t` and drop the argument.  Those rules never needed a type rule
// because the parameter is consumed by the rule; a deduced template argument is
// not.
//
// WHY A UNIT-LIKE TYPE AND NOT A NULL POINTER.  std::nullptr_t is not a pointer
// type in C++: it has no pointee, cannot be dereferenced or offset, and has
// EXACTLY ONE value (the null pointer literal).  `()` also has exactly one
// inhabitant, so the mapping is a bijection on values.  Mapping it to `*const ()`
// or to a null `Ptr` instead would hand the translation a Rust type that admits
// values C++ cannot form and operations C++ forbids, and would silently pick ONE
// pointer representation for a type that in C++ converts to any of them.  The
// conversion `std::pair<std::nullptr_t, std::nullptr_t>` ->
// `std::pair<Operation *, int>` is a SEPARATE rule in the pair/tuple modules;
// with `()` here, a site that needs it fails LOUDLY on the missing conversion
// instead of compiling against a guessed pointer width.
