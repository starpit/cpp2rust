// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::istringstream -- `std::basic_istringstream<char, std::char_traits<char>,
// std::allocator<char>>`.  Queue rows g144 (5 TUs), g1301, g1302 and g1303 (1 TU
// each) are ALL `type` rows and the searched key is spelled, in every one of
// them, plainly `std::istringstream`; there is no operator or method row for it
// anywhere in the queue.  (Those rows carry owner=rules/basic_istringstream;
// the module is named rules/istringstream here to match the key spelling, the
// same way rules/sstream carries std::ostringstream.)
//
// ⭐⭐ MODEL FLIPPED 2026-09-28: Vec<u8> -> `libcc2rs::IStream`, the sticky-failbit
// input stream WITH A READ CURSOR added by libcc2rs 856b99d0.  This module's own
// refusal below named the blocker exactly -- "a Vec<u8> HAS NO READ CURSOR" --
// and that blocker is gone.  rules/basic_stringstream flipped in the same change,
// so std::stringstream and std::istringstream still share ONE representation, and
// it is the same one rules/iostream t4 now gives `std::istream`, which is what
// makes the free `operator>>` key (rules/iostream f8) type-check on a receiver
// that arrived through a DerivedToBase conversion.
//
// std::ostringstream (rules/sstream) is deliberately NOT flipped: it is
// output-only, has no get side, and its Vec<u8> is already exactly right.
//
// SUPERSEDED MODEL NOTE, kept for the reasoning:
// MODEL: a byte buffer, Vec<u8> -- the same model rules/sstream gives
// std::ostringstream and rules/basic_stringstream gives std::stringstream, so a
// buffer handed between them keeps one representation.
//
// A TYPE KEY NEEDS A CONSTRUCTOR: without one, the converter looks the ctor up
// as an ordinary expr rule, misses, and falls back to
// `<mangled-type>::new()`, which gives rc=0 and then E0433.  Measured four
// separate times today on other types, so both constructors that the reached
// call sites can use are keyed here even though no queue row names them:
// the default one and the `const std::string &` one, which is the form
// `Dpc::split` / `Dip::loadInitFromFile` actually use (`istringstream iss(line)`).
// The signatures are not restated -- <sstream> is included, so whatever
// overload clang resolves here is the same one the converter resolves at the
// call site, defaulted `openmode` argument included.
//
// EXTRACTION (`>>`) -- THE OLD REFUSAL AND WHAT ANSWERED EACH HALF OF IT.
// The two recorded reasons were:
//   1. "A Vec<u8> HAS NO READ CURSOR."  ANSWERED by the model flip: IStream
//      carries `pos`.  The sub-argument that std::io::Cursor was unavailable
//      because it is not ByteRepr still stands and is why a NEW libcc2rs type
//      was the right answer -- `impl ByteRepr for IStream {}` is a
//      claim-nothing impl whose methods all panic, which is honest because the
//      bound on Ptr::with_mut is imposed by the `Reinterpreted` arm alone.
//   2. "A rule body is INLINED and every `aN` RE-EXPANDS, so a receiver-consuming
//      read would CONSUME THE INPUT TWICE."  ANSWERED by shape, not by luck:
//      every libcc2rs entry point takes the stream handle ONCE and hands the SAME
//      handle back, and rules/iostream f8 binds each `aN` to a local exactly once.
//      The re-expansion hazard is real and is the reason for that discipline.
// The KEY ITSELF LIVES IN rules/iostream (f8), not here: `iss >> tok` resolves to
// the FREE `operator>>(std::istream &, std::string &)`, so the recorded key names
// std::istream and belongs to that owner.  This module's contribution is that its
// t1 is now the SAME Rust type, without which the call is an E0308.
//
// STILL OUT, and now for a DIFFERENT reason than "no cursor": the basic_ios
// predicates (eof/good/fail/operator bool/operator!) and the member `operator>>`
// overloads, because a MEMBER call's receiver is emitted as
// `(iss as Cpp2RustUnmapped_std_ios)` -- a non-primitive cast no rule body can
// remove (dip/dip.cpp.rs:5478).  `std::getline` is out because the converter
// never rule-searches it: it emits an undefined ported call `getline_99(...)`
// (dip/dip.cpp.rs:6414), so a key would be DEAD.  Also out: seekg (needs a put/get
// position API nothing asks for), the std::ios_base formatting manipulators and
// `str(const std::string &)` (a setter, which needs a &mut receiver the rule ABI
// cannot express -- see "AN LVALUE-REFERENCE TARGET PARAMETER IS NOT
// ENFORCEABLE").

#include <sstream>
#include <string>

using t1 = std::istringstream;

std::istringstream f1() { return std::istringstream(); }

std::istringstream f2(const std::string &a0) { return std::istringstream(a0); }

std::string f3(const std::istringstream &a0) { return a0.str(); }
