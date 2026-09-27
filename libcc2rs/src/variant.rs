// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

//! std::variant<...> modelled as a REAL Rust enum, per arity.
//!
//! A rule module cannot declare an enum, but a rule target CAN NAME a
//! libcc2rs type (rules/map's `UnsafeMapIterator`, rules/rustls' external
//! types), so the enum lives here and rules/variant's per-arity type keys
//! merely name it -- the same per-arity pattern that carried std::tuple to
//! 27-ary.
//!
//! WHY AN ENUM AND NOT A TUPLE/STRUCT: a tuple representation makes reading
//! an INACTIVE alternative succeed, which is exactly the C++ semantics a
//! variant exists to deny (std::get on an inactive alternative throws
//! std::bad_variant_access).  With a real enum there is no inactive field to
//! read at all.

/// `std::variant<A, B>`.
#[derive(Clone, Debug, PartialEq)]
pub enum Variant2<A, B> {
    V0(A),
    V1(B),
}

/// `std::variant<A, B, C>`.
#[derive(Clone, Debug, PartialEq)]
pub enum Variant3<A, B, C> {
    V0(A),
    V1(B),
    V2(C),
}

/// `std::variant<A, B, C, D, E, F, G, H>`.
///
/// THE ONLY ARITY THE dxp_standalone CORPUS ACTUALLY USES: `OperandAttr::data_`
/// (sys-arch-spec/progir/progir.h:256) is 8-ary, and there is no 2-ary or 3-ary
/// instantiation anywhere in it.  Before this arity existed, the mapper's
/// placeholder resolution gave the 3-ary key's `T3` the JOINED text of
/// alternatives 3..8 (`findNextLiteralSameDepth` walks to the final depth-0
/// `>`), and mapper.cpp:1233 then asserted `Type is not present in types_` on
/// that non-type -- a segfault under NDEBUG on three TUs.
#[derive(Clone, Debug, PartialEq)]
pub enum Variant8<A, B, C, D, E, F, G, H> {
    V0(A),
    V1(B),
    V2(C),
    V3(D),
    V4(E),
    V5(F),
    V6(G),
    V7(H),
}

impl<A, B> Variant2<A, B> {
    /// `std::variant::index()`
    pub fn index(&self) -> usize {
        match self {
            Variant2::V0(_) => 0,
            Variant2::V1(_) => 1,
        }
    }
}

impl<A, B, C> Variant3<A, B, C> {
    /// `std::variant::index()`
    pub fn index(&self) -> usize {
        match self {
            Variant3::V0(_) => 0,
            Variant3::V1(_) => 1,
            Variant3::V2(_) => 2,
        }
    }
}

impl<A, B, C, D, E, F, G, H> Variant8<A, B, C, D, E, F, G, H> {
    /// `std::variant::index()`
    pub fn index(&self) -> usize {
        match self {
            Variant8::V0(_) => 0,
            Variant8::V1(_) => 1,
            Variant8::V2(_) => 2,
            Variant8::V3(_) => 3,
            Variant8::V4(_) => 4,
            Variant8::V5(_) => 5,
            Variant8::V6(_) => 6,
            Variant8::V7(_) => 7,
        }
    }
}

// A default-constructed std::variant value-initialises its FIRST alternative,
// so Default follows alternative 0 and only alternative 0 needs Default.
impl<A: Default, B> Default for Variant2<A, B> {
    fn default() -> Self {
        Variant2::V0(A::default())
    }
}

impl<A: Default, B, C> Default for Variant3<A, B, C> {
    fn default() -> Self {
        Variant3::V0(A::default())
    }
}

impl<A: Default, B, C, D, E, F, G, H> Default for Variant8<A, B, C, D, E, F, G, H> {
    fn default() -> Self {
        Variant8::V0(A::default())
    }
}
