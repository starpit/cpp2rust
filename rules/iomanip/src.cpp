// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <iomanip>

auto f1(int n) { return std::setw(n); }

// ============================================================================
// f1 (above) / f2 -- THE OUTPUT FIELD MANIPULATORS THAT CARRY A VALUE.
// Row g3098.  Read this before concluding either body is a placeholder: THE
// IDENTITY IS THE CORRECT BODY HERE, and that is measured, not assumed.
//
// `Converter::ConvertCallToOstream` does not dispatch a `std::ostream` `<<`
// chain through rules per operand -- it flattens the chain into ONE
// `write!(os, "<fmt>", <args>)` and asks `GetFmtArg` to classify each operand.
// MEASURED on the 99-module tree ir/g3090A with converter 086c2ff4
// (verif/g3098/osrow.unsafe.vlog): the only `<<` key the converter ever asks
// for on a `std::ostream` is
//     std::ostream & operator shl(std::ostream &, const char *)   (10 asks, None)
// and it asks NEVER for `operator shl(std::ostream &, std::__iom_t6)` or
// `(..., std::__iom_t4<char>)`.  ⭐ So there is no operator key to write, and a
// `libcc2rs::OStream` holding sticky width/fill/adjustfield would be
// unreachable: the emitted lowering never calls a method on the stream for a
// manipulator operand.  What the manipulator rule has to produce is therefore
// not an effect but a VALUE -- the width, the fill character -- which
// `GetFmtArg` then folds into the format spec of the field that follows.
// The identity is exactly that value.
//
// SEARCHED SPELLINGS, read off the converter rather than guessed:
//     search expr std::__iom_t6 std::setw(int), result:
//     Matching: std::__iom_t6 std::setw(int)            (21 asks, f1 -- landed)
//     search expr std::__iom_t4<char> std::setfill(char), result:
//     None                                             (8 asks  -- f2, new here)
// Note `std::__iom_t6` / `std::__iom_t4<char>`: libc++'s own opaque manipulator
// carrier types, printed by the typedef-preferring printer.  They are the KEY's
// return type only; no type rule for them is needed or wanted, because the value
// never survives `GetFmtArg`.
//
// ⛔⛔ WHAT WAS ACTUALLY BROKEN, SO THE NEXT READER DOES NOT "FIX" THE RULE.  f1
// has been landed and correct all along; the defect was in the converter, which
// tested `arg_str.contains("Setw")` -- a spelling `Mapper::ToString` never
// produces for `std::setw`, and one that DOES match the unrelated
// `PrintUtil::printSetwithQuotes`.  So every `std::setw` site printed its width
// as a data field (`write!(.., "{:}", width)`, which type-checks -- silent).
// Fixed in converter.cpp `IsOstreamManip`; it was not fixable here.
// ⛔ CORRECTED by row g3113: this note also claimed "three `printSetwithQuotes`
// sites had their argument swallowed".  The MECHANISM reproduces on a probe
// (byte diff vs clang: `[]` for `[14]`) but NO corpus site is affected -- all 8
// `printSetwithQuotes(` calls return `std::string` and take a different arm.
//
// ⛔⛔ AND THE BIGGER CORRECTION, ALSO FROM g3113: f2 BELOW MADE A LOUD SITE
// SILENT, and that had to be repaired in the converter before it could land.
// Before f2 existed, `std::setfill` lowered to the undefined
// `Cpp2RustUnmappedFn_setfill_N` -- an `E0425`.  With f2 it is the identity, so
// at any site where `GetFmtArg` cannot SPEND the fill (an unpaddable datum, or a
// chain with no `setw`) the fill CHARACTER was printed as a DATA FIELD and the
// TU compiled.  Measured, one factor (same binary, ir/goalHEAD vs ir/g3113):
//     clang                        `     abc|`
//     with f2, compiles and runs   `832abc|`      <- the width 8 and the fill 32
// This is HARNESS-COMMON's "a type key with no method key is strictly worse than
// no key at all", one layer over: f2 is only safe BECAUSE converter.cpp now
// emits `Cpp2RustUnmappedManip_<name>` for a manipulator it cannot honour.
// ⭐ DO NOT LAND f2 WITHOUT THAT CONVERTER HALF.
//
// ⛔ STILL DELIBERATELY NOT KEYED, and they keep failing detectably:
//   * `std::setprecision(int)` / `std::fixed` / `std::scientific` (11 + 11 sites)
//     -- the floatfield is a SECOND piece of state and `fixed` changes the
//     DEFAULT precision as well as the notation, so an identity would print a
//     different number of digits.  Named residual, 22 sites over 7 files.
//   * `std::boolalpha` (2 sites) -- an identity prints `1` where C++ prints
//     `true`.  It reaches `<<` as a function POINTER, so today it lowers to
//     `Some(boolalpha_N)` and is `error[E0277]: Option<fn..> doesn't implement
//     Display` (verified with the project rustc 1.98.0, verif/g3098/disc.rs):
//     DETECTABLE at compile time, which is strictly better than a keyed no-op.
//   * `std::internal`, `std::showbase`, `std::noboolalpha` and the rest of the
//     restoring manipulators -- a rule body is a pure function of its arguments
//     and cannot clear state a LATER insertion reads.
// ============================================================================
auto f2(char c) { return std::setfill(c); }
