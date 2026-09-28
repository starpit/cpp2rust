// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::allocator<T> -- queue rows g2815/g2816 (`std::allocator<senulator_prog>`,
// decl site toolchain/libcxx/__memory/allocator.h:62).
//
// ⛔ WHY THIS NEEDS ITS OWN MODULE INSTEAD OF rules/vector.  rules/vector/src.cpp
// mentions `std::allocator<T1>` ~60 times, but ONLY as the DEFAULTED template
// argument of a container (`std::vector<T1, std::allocator<T1>>`).  Defaulted
// template arguments are ELIDED by the rule printer, so not one of those sites can
// ever record a key.  The rows' spelling is the BARE `std::allocator<senulator_prog>`
// -- the converter reached the allocator as a TYPE IN ITS OWN RIGHT.  It therefore
// has to be spelled STANDALONE here (an alias and a parameter position), never
// nested inside a container.
//
// Keyed GENERICALLY at <T1>: precedent is rules/plus, whose `std::plus<T1>` key
// serves concrete `std::plus<int64_t>` sites.
//
// std::allocator is STATELESS and EMPTY (allocator.h: no data members,
// `allocator() noexcept = default`, and all specialisations compare equal), so two
// entries suffice for a type that only ever appears as a type and a default-
// constructed value: the TYPE (t1) and the DEFAULT CONSTRUCTOR (f1).  A type key
// with no constructor translates rc=0 and then fails to compile with error[E0433]
// (measured three times; see rules/plus/src.cpp).
//
// NOT COVERED, deliberately: allocate/deallocate/construct/destroy,
// allocator_traits, operator==, and the rebind machinery.  No queue row and no
// recorded site asks for them, and a container's storage is modelled by the
// container rules themselves, not by an allocator handle.  A site that really did
// call `a.allocate(n)` must FAIL LOUDLY at translate time rather than be given a
// guessed body -- so the keys are left out on purpose.

#include <memory>

template <typename T1> using t1 = std::allocator<T1>;

template <typename T1> std::allocator<T1> f1() { return std::allocator<T1>(); }
