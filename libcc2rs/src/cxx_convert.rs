// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

//! A C++ IMPLICIT ARITHMETIC CONVERSION, SPELLABLE FROM A RULE BODY.
//!
//! WHY THIS EXISTS.  The pipeline's convention for an ordinary C++ implicit
//! narrowing conversion is a CONVERTER-EMITTED Rust `as` cast -- measured on an
//! 8-line probe (`double d; int i = d;`): the unsafe model emits
//! `let mut i: i32 = (d as i32);` and the refcount model emits
//! `Rc::new(RefCell::new(((*d.borrow()) as i32)))`.  That path runs off
//! `NeedsImplicitScalarCast` + `ConvertCast` in converter.cpp, driven by
//! `PlaceholderCtx::implicit_convert_to`, and it is WHOLE-ARGUMENT: it can cast
//! the argument a rule body substitutes, but it cannot reach INSIDE it.
//!
//! ⛔ AND A RULE BODY CANNOT WRITE THE CAST ITSELF.  A rule target is a GENERIC
//! Rust fn that the rule preprocessor type-checks before it extracts the body
//! fragments, so `a0.0 as T1` is rejected there with
//!   error[E0605]: an `as` expression can only be used to convert between
//!                 primitive types or to coerce to a specific trait object
//! (measured; the regen then aborts at semantic.rs:260).  There is no escape via
//! the `Tn` generic fragment either: substitution is textual and happens in the
//! CONVERTER, long after that type-check.
//!
//! So the only thing a rule body can name is a TRAIT METHOD, and the conversion
//! it needs -- `f64 -> i32` -- is one std deliberately does not provide: there is
//! NEITHER `From<f64> for i32` NOR `TryFrom<f64> for i32`, because it is lossy.
//! Both types are foreign, so the orphan rule forbids adding those impls here.
//! A NEW LOCAL TRAIT IS THEREFORE FORCED.  `rules/pair`'s f23 (the converting
//! copy ctor `pair<T1,T2>::pair(const pair<T3,T4>&)`) is the first caller: 14
//! sites in fresh38, e.g. dcg/dcg_fe/pcfg_gen/inputNeighFetchOp.cpp:1735, where
//! `make_pair(coreId, coord_A.at("i"))` builds a `pair<int, double>` that is used
//! as a `map<pair<int,int>, int>` key.
//!
//! ⚠️⚠️ SEMANTICS, AND THE ONE PLACE THIS IS NOT C++.  A C++ implicit arithmetic
//! conversion TRUNCATES and never panics, which is why this is `as` and NOT
//! `TryFrom` + `expect()`: a panicking body would be a behaviour change, not a
//! translation.  ⛔ BUT `as` IS NOT BIT-FOR-BIT C++ IN ONE CASE.  A
//! float -> integer conversion whose truncated value does not fit the integer is
//! UNDEFINED BEHAVIOUR in C++ ([conv.fpint]/1), while Rust's `as` is defined and
//! SATURATES (since 1.45: it clamps to the integer's min/max, and NaN gives 0).
//! So this is C++'s semantics on every in-range value and a DEFINED, saturating
//! answer where C++ has none.  That is deliberate -- the alternative spellings
//! are a panic (wrong: C++ does not trap) or `unreachable_unchecked` (wrong:
//! reproducing UB as UB buys nothing and is unauditable) -- but it IS a
//! divergence and is recorded here rather than papered over.  Integer -> integer,
//! integer -> float and float -> float `as` are all fully defined in both
//! languages and agree (modulo C++20's mandated two's-complement wraparound,
//! which `as` also does).
//!
//! ⛔ WHAT IS DELIBERATELY *NOT* HERE, so the remainder stays LOUD.  Only the 14
//! primitive ARITHMETIC types get impls.  `bool` and `char` are absent: C++ does
//! convert them implicitly, but the reverse direction is not expressible as `as`
//! in Rust at all (`i32 as bool` and `i32 as char` are both rejected), so a
//! one-directional half-family would be a trap, and no measured site asks for
//! one.  A pair whose element conversion is not covered here fails at rustc with
//!   error[E0277]: the trait bound `f64: CxxConvert<String>` is not satisfied
//! which NAMES BOTH TYPES -- the same loudness as the `i32: From<f64>` it
//! replaces, and never a silent wrong value.

/// A C++ implicit arithmetic conversion from `Self` to `To`.
///
/// The reflexive impl below makes `T: CxxConvert<T>` hold for EVERY `T`, which is
/// what lets a rule body call this with no extra bound: `rules/pair` f23's target
/// declares its argument as the receiver's own `(T1, T2)`, so the generic
/// type-check only ever sees the identity case, and the cross-type impls are
/// reached only after the converter has substituted the two CONCRETE element
/// types at the call site.
pub trait CxxConvert<To> {
    fn cxx_convert(self) -> To;
}

/// The identity conversion.  Coexists with the concrete impls below for the same
/// reason `impl<T> From<T> for T` coexists with `impl From<u8> for u32`: the
/// blanket impl's `Self` and `To` are the SAME type, and every impl below has
/// them different, so coherence can prove the two never overlap.
impl<T> CxxConvert<T> for T {
    #[inline]
    fn cxx_convert(self) -> T {
        self
    }
}

/// `$from as $to` for each listed target type.  ⚠️ Identical pairs must never
/// appear here -- that would overlap the reflexive impl above (E0119).
macro_rules! cxx_convert_as {
    ($from:ty => $($to:ty),+ $(,)?) => {
        $(impl CxxConvert<$to> for $from {
            #[inline]
            fn cxx_convert(self) -> $to {
                self as $to
            }
        })+
    };
}

cxx_convert_as!(i8 => i16, i32, i64, i128, isize, u8, u16, u32, u64, u128, usize, f32, f64);
cxx_convert_as!(i16 => i8, i32, i64, i128, isize, u8, u16, u32, u64, u128, usize, f32, f64);
cxx_convert_as!(i32 => i8, i16, i64, i128, isize, u8, u16, u32, u64, u128, usize, f32, f64);
cxx_convert_as!(i64 => i8, i16, i32, i128, isize, u8, u16, u32, u64, u128, usize, f32, f64);
cxx_convert_as!(i128 => i8, i16, i32, i64, isize, u8, u16, u32, u64, u128, usize, f32, f64);
cxx_convert_as!(isize => i8, i16, i32, i64, i128, u8, u16, u32, u64, u128, usize, f32, f64);
cxx_convert_as!(u8 => i8, i16, i32, i64, i128, isize, u16, u32, u64, u128, usize, f32, f64);
cxx_convert_as!(u16 => i8, i16, i32, i64, i128, isize, u8, u32, u64, u128, usize, f32, f64);
cxx_convert_as!(u32 => i8, i16, i32, i64, i128, isize, u8, u16, u64, u128, usize, f32, f64);
cxx_convert_as!(u64 => i8, i16, i32, i64, i128, isize, u8, u16, u32, u128, usize, f32, f64);
cxx_convert_as!(u128 => i8, i16, i32, i64, i128, isize, u8, u16, u32, u64, usize, f32, f64);
cxx_convert_as!(usize => i8, i16, i32, i64, i128, isize, u8, u16, u32, u64, u128, f32, f64);
cxx_convert_as!(f32 => i8, i16, i32, i64, i128, isize, u8, u16, u32, u64, u128, usize, f64);
cxx_convert_as!(f64 => i8, i16, i32, i64, i128, isize, u8, u16, u32, u64, u128, usize, f32);
