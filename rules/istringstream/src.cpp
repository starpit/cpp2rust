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
// EXTRACTION (`>>`) IS NOT COVERED, AND IT IS A REFUSAL, NOT AN OMISSION.  Two
// independent reasons, either one sufficient:
//   1. A Vec<u8> HAS NO READ CURSOR.  Modelling `>>` needs a position that
//      advances, which is a different representation (std::io::Cursor<Vec<u8>>),
//      and Cursor is not an option for this family: in the refcount model
//      Ptr<T>'s write_all/write_fmt exist only for `T: std::io::Write +
//      ByteRepr` (libcc2rs/src/rc.rs:571) and Cursor<Vec<u8>> is not ByteRepr.
//      Changing the representation would also desynchronise this module from
//      rules/sstream and rules/basic_stringstream.
//   2. A rule body is INLINED and every `aN` RE-EXPANDS.  A receiver-consuming
//      read would therefore CONSUME THE INPUT TWICE wherever the receiver
//      appears more than once in the body -- silently, with no diagnostic.
// So the rest of the istream API (getline/get/peek/eof/fail/clear/seekg) is out
// for the same reason, as are the std::ios_base formatting manipulators and
// `str(const std::string &)` (a setter, which needs a &mut receiver the rule ABI
// cannot express -- see "AN LVALUE-REFERENCE TARGET PARAMETER IS NOT
// ENFORCEABLE").

#include <sstream>
#include <string>

using t1 = std::istringstream;

std::istringstream f1() { return std::istringstream(); }

std::istringstream f2(const std::string &a0) { return std::istringstream(a0); }

std::string f3(const std::istringstream &a0) { return a0.str(); }
