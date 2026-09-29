// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

//! `llvm::cl` carrier types that need a REAL Rust struct, not a value model.
//!
//! `rules/cl` maps most of `llvm::cl` with a VALUE model -- a `cl::opt<T>` IS its
//! `T` -- because every one of those types is only ever READ THROUGH.  This module
//! exists for the one shape where that is not available: a type the corpus
//! CONSTRUCTS with an aggregate initialiser.
//!
//! ⛔ WHY A `()` MODEL IS NOT AN OPTION HERE, AND WHY THIS FILE EXISTS.
//! `llvm::cl::OptionEnumValue` (CommandLine.h:679) is
//!     struct OptionEnumValue { StringRef Name; int Value; StringRef Description; };
//! and the corpus only ever builds temporaries of it, via the `clEnumValN` /
//! `clEnumVal` macros (CommandLine.h:686/691), which expand to an AGGREGATE
//! INITIALISER.  `Converter::VisitInitListExpr` (converter.cpp:7964) and
//! `ConverterRefCount::VisitInitListExpr` (converter_refcount.cpp:1844) both emit
//!     <mapped-type> { <field> : <init> , ... }
//! i.e. a Rust STRUCT LITERAL whose PATH is the mapped type's own name and whose
//! field names are the C++ field names verbatim (`GetNamedDeclAsString(field)` over
//! the REAL `RecordDecl`'s `fields()`, so `Name` / `Value` / `Description`).
//! With the type keyed to the unit that emits `() { Name : ... , }` -- a struct
//! literal body with `()` as its path, which is not Rust at all.  Construction is
//! the one syntactic position where a mapped type's NAME is printed, so
//! "the corpus never reads a member" does NOT make an opaque model safe: it makes
//! it emit unparseable text.
//!
//! ⭐ THE THREE TYPE PARAMETERS ARE NOT GENERALITY, THEY ARE THE TWO MODELS.
//! The struct-literal FIELD VALUES are converted by `ConvertVarInit(field->getType(), ...)`,
//! i.e. per MODEL, while the struct-literal PATH comes from `GetUnsafeTypeAsString`.
//! So one single concrete field type cannot serve both arms:
//!     C++ field         unsafe model              refcount model
//!     StringRef Name    Vec<libc::c_char>         Value<Vec<u8>>
//!     int Value         i32                       Value<i32>
//! The defaults below are the UNSAFE shapes, matching the module that names this
//! type (`rules/cl` ships a `tgt_unsafe.rs` only).
//!
//! ⛔⛔ AND THE DEFAULTS ARE LOAD-BEARING, NOT COSMETIC -- DO NOT ASSUME INFERENCE
//! RESCUES THE REFCOUNT ARM.  The converter annotates the temporary it builds:
//!     let mut __tmp_68: Vec<ClOptionEnumValue> = ...          (unsafe)
//!     let __tmp_61: Value<Vec<ClOptionEnumValue>> = ...       (refcount)
//! A DEFAULTED type parameter in TYPE position is not an inference variable, so the
//! bare path means `<Vec<c_char>, i32, Vec<c_char>>` on BOTH legs and inference
//! never runs over the field expressions.  Checked with rustc directly, because the
//! harness cannot see it (rustfmt is its only Rust parser):
//!   unsafe arm   -- type-checks; the defaults match the field expressions exactly.
//!   refcount arm -- 3x E0308 per site: `expected Vec<i8>, found Rc<RefCell<Vec<u8>>>`.
//! ⚠️ So this struct fixes the UNSAFE arm completely and leaves the REFCOUNT arm
//! ill-typed rather than unparseable.  ⭐ It regresses nothing reachable: every site
//! is an argument to `Cpp2RustUnmappedFn_values_N` (`cl::values` is deliberately
//! refused), so an intentional E0425 already sits upstream of the E0308.  Closing
//! the refcount residual needs the path emitted WITH arguments, or a per-model
//! target spelling -- a converter change, rowed separately, NOT a wider default.
//!
//! ⛔ NO METHOD IS PROVIDED, DELIBERATELY.  `rules/cl` keys no member of
//! `OptionEnumValue` and no member of `ValuesClass`; the only reader of the three
//! fields anywhere is `llvm::cl::ValuesClass::apply` (CommandLine.h:702), which sits
//! behind `llvm::cl::values(...)` -- a variadic that no rule signature can spell and
//! that is deliberately left as a loud `Cpp2RustUnmappedFn_values_N` placeholder.
//! The fields are `pub` so that a keyed read would be a read of the real value; any
//! method call on the carrier still fails loudly (`E0599`).

#![allow(non_snake_case)]

/// `llvm::cl::OptionEnumValue` -- CommandLine.h:679, a three-field plain
/// aggregate carrying one `--flag=<name>` choice of an enum-valued option.
///
/// The field NAMES must stay exactly `Name` / `Value` / `Description`: the
/// converter emits the C++ field names verbatim and never renames them, so a
/// Rust-style `name` / `value` / `description` would be `E0560`.
#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct ClOptionEnumValue<N = Vec<libc::c_char>, V = i32, D = Vec<libc::c_char>>
{
    /// `StringRef Name` -- the spelling that appears after `=` on the command line.
    pub Name: N,
    /// `int Value` -- the enumerator, narrowed to `int` by `clEnumValN`'s own
    /// `int(ENUMVAL)` cast (CommandLine.h:686).
    pub Value: V,
    /// `StringRef Description` -- the help text.
    pub Description: D,
}
