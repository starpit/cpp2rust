// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::stringstream -- `std::basic_stringstream<char, std::char_traits<char>,
// std::allocator<char>>`.  Queue rows g057 (14 TUs), g061 (13 TUs), g145 (5
// TUs) and g1307-g1315 (1 TU each) are ALL `type` rows and the searched key is
// spelled, in every one of them, plainly `std::stringstream`.  There is no
// operator or method row for it anywhere in the queue, so the whole 41-TU row is
// this type plus its default constructor.
//
// MODEL: a byte buffer, Vec<u8> -- the same model rules/sstream (feef166) gives
// std::ostringstream, for the same FORCED reason.  Insertion into any ostream is
// not rule-driven (converter_lib.cpp:552 IsCallToOstream matches on the RESULT
// type being basic_ostream, and converter.cpp:2073 ConvertCallToOstream emits
// write_all / write! against ToString() of the left-most operand), so the target
// only has to implement std::io::Write; and in the refcount model Ptr<T>'s
// write_all/write_fmt exist only for `T: std::io::Write + ByteRepr`
// (libcc2rs/src/rc.rs:571), whose only inhabitants are Vec<u8> and
// std::fs::File.  std::io::Cursor<Vec<u8>> is NOT ByteRepr, so it is not an
// option even though a Cursor is the natural shape for a bidirectional stream.
// Vec<u8> is therefore the only representation available, exactly as in
// rules/sstream.
//
// A TYPE KEY NEEDS A CONSTRUCTOR: without f1, the converter looks the default
// ctor up as an ordinary expr rule, misses, and falls back to
// `<mangled-type>::new()`, which gives rc=0 then E0433.  So the default ctor is
// keyed here even though no queue row names it.
//
// c008 -- MEASURED HERE, NOT INHERITED.  rules/sstream documents refcount as
// blocked by overlapping AsPointer impls in libcc2rs
// (`impl<T> AsPointer<T> for Rc<RefCell<T>>` at rc.rs:850 vs
// `Rc<RefCell<Vec<T>>>` at rc.rs:890) giving E0283 at every insertion site.
// See tgt_refcount.rs for what a probe of THIS module actually measured.
//
// f3 -- THE CTOR OVERLOAD, LANDED 2026-09-27.  The recorded blocker for queue
// rows g087 (18 TUs) / g091 (17 TUs) / g222 -- `system type has no rule:
// std::basic_stringstream<char, std::char_traits<char>, std::allocator<char>>`
// -- IS STALE.  Re-measured in a private clone of pin/ir.v9 with
// /home/agent/work/probes/ss_remeasure.cpp (`const char* ptr; std::stringstream
// ss(ptr); ss.str();`): the TYPE matches t1, `str()` matches f2, the TU
// translates rc=0, and the only miss in the whole -verbose log is
//     search expr void std::basic_stringstream<char>::basic_stringstream(
//         const std::string &, unsigned int), result:            <-- None
// which lowers to the unmapped-ctor fallback
// `std_basic_stringstream_char__std_char_traits_char___std_allocator_char__::new_1(...)`
// and then E0433.  So the live gate was never the type: it is the
// rc=0-then-E0433 unkeyed-constructor trap, and the corpus writes the
// one-argument form everywhere (`std::stringstream ss(ptr)` dtgetenv.hpp:127,
// `ss(jsonstr)` senulator.cpp:765, `s_stream(myOption)` dip.h:297, 30+ more).
// NOTE THE SECOND PARAMETER: `openmode` is a DEFAULTED argument and it is KEPT
// in the key, canonicalised to `unsigned int`.  A one-parameter key would not
// match.  A `const char *` argument needs NO separate key -- it converts through
// std::basic_string(const char *) first, which is why one key covers both
// spellings; that was measured on the pointer form above.
//
// NOT COVERED, deliberately, each because it needs its own harvested key and
// none has one: `str(const std::string &)` (a SETTER; it needs a `&mut`
// receiver the rule ABI cannot express), the (openmode)-only constructor,
// EXTRACTION
// (`>>`) and the other istream API (getline/get/peek/eof/fail/clear/seekg) --
// a Vec<u8> has no read cursor at all, so an extraction row would have to
// change the model, not extend it -- and the std::ios_base formatting
// manipulators.  std::istringstream is a separate owner (rules/basic_istringstream,
// rows g144/g1301-g1303).

#include <sstream>
#include <string>

using t1 = std::stringstream;

std::stringstream f1() { return std::stringstream(); }

std::string f2(const std::stringstream &o) { return o.str(); }

std::stringstream f3(const std::string &o, std::ios_base::openmode m) {
  return std::stringstream(o, m);
}
