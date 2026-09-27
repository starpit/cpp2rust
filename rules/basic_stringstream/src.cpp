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
// NOT COVERED, deliberately, each because it needs its own harvested key and
// none has one: `str(const std::string &)`, the
// basic_stringstream(const std::string &) and (openmode) constructors, EXTRACTION
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
