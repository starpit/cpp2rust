// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::stringstream -- `std::basic_stringstream<char, std::char_traits<char>,
// std::allocator<char>>`.  Queue rows g057 (14 TUs), g061 (13 TUs), g145 (5
// TUs) and g1307-g1315 (1 TU each) are ALL `type` rows and the searched key is
// spelled, in every one of them, plainly `std::stringstream`.  There is no
// operator or method row for it anywhere in the queue, so the whole 41-TU row is
// this type plus its default constructor.
//
// ⭐⭐ MODEL FLIPPED 2026-09-28: Vec<u8> -> `libcc2rs::IStream`.
// The old model's own words were "a Vec<u8> has no read cursor at all, so an
// extraction row would have to change the model, not extend it".  That is
// exactly what happened: libcc2rs commit 856b99d0 added `IStream` -- buffer +
// GET POSITION + failbit/eofbit/badbit, with a private `sentry()` that makes
// every extractor a no-op once the failbit is set -- and this module now uses it.
//
// ⛔ THE PUT SIDE WAS THE HARD CONSTRAINT AND IT IS DISCHARGED IN libcc2rs, NOT
// HERE.  A stringstream is read AND written, insertion is NOT rule-driven, and
// the refcount lowering lands on `Ptr<T>::write_fmt`/`write_all`, which exist
// only for `T: std::io::Write + ByteRepr` (libcc2rs/src/rc.rs:723).  So the flip
// would have traded a read-side gap for a write-side E0599 at every `ss << x`.
// `impl std::io::Write for IStream` (APPEND) and a claim-nothing
// `impl ByteRepr for IStream` were added in the same change, with two tests --
// `writing_appends_and_does_not_disturb_the_read_cursor` and
// `a_stream_behind_a_ptr_is_writable`, the latter being the E0599 check itself.
//
// ⭐ NO BEHAVIOUR CHANGED ON THE PUT SIDE.  `IStream`'s `Write` APPENDS, which is
// precisely what `Vec<u8>`'s did, so the measured construct-then-INSERT
// divergence documented below (C++ `12cXY` vs this model's `abcXY12`) is
// UNCHANGED -- neither fixed nor worsened -- and it stays unreachable for the
// same 36-site corpus grep.  Likewise the converter's hardcoded
// `write!((ss as std::fs::File), ...)` cast on the FORMATTED path is still an
// E0605; it is a converter-side defect, was failing loudly before, and fails
// loudly still.
//
// SUPERSEDED MODEL NOTE, KEPT because its reasoning is why the flip needed a
// libcc2rs change first:
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
// f3 CARRIES A KNOWN, MEASURED DEFECT THAT IS UNREACHABLE IN THE CORPUS --
// 2026-09-28.  `std::stringstream ss(s)` opens `in|out` WITHOUT `ate`, so the
// PUT POSITION IS 0 and a subsequent insertion OVERWRITES from the front.  A
// Vec<u8> has no room for a put position, so the model APPENDS.  Measured with
// /home/agent/work/ssput/ssput.cpp (seed "abcXY", insert the SHORTER "12", print
// `.str()`), which cannot confuse the two behaviours:
//     C++    : 12cXY        (overwrite)
//     unsafe : abcXY12      MISMATCH (append)
// So f3 is correct for construct-then-`.str()` and construct-then-extract, and
// silently wrong for construct-then-INSERT.
//
// IT IS NOT FIXED, AND THE REASON IS A CORPUS MEASUREMENT, NOT AN OPINION.  Over
// all of dt_src outside cpp2rust-port there are 36 `stringstream <v>(<arg>)`
// sites with a non-empty argument (the 38-line grep also catches two function
// DECLARATIONS returning a stringstream, dxp/dxp.h:205 and
// deeprt/deeprt.h:227, which construct nothing):
//     grep -rnE '\b(std::)?stringstream\s+[A-Za-z_][A-Za-z0-9_]*\s*\(' \
//       --include=*.cpp --include=*.h --include=*.hpp . | grep -v cpp2rust-port \
//       | grep -vE '\(\s*\)'
// NOT ONE of the 36 is ever inserted into.  Checked three ways: no `<v> <<`
// within 60 lines of the ctor; no `<v> <<` ANYWHERE in the same file; and the
// only two by-reference escapes (sgr/sengraph.cpp:1149 and :1167 passing
// `stream` to `Tensor::read<size_t>`) land in pure READ functions
// (sgr/sengraph.cpp, sgr/sengraphTensor.h, sgr/symTensorShape.{h,cpp},
// sys-arch-spec/progir/progir.cpp) that contain NO `<<` on the parameter at all.
// Every real shape in the corpus is construct-then-EXTRACT: `ss >> parsed`
// (util/dtgetenv.hpp:127-134, the biggest consumer at 17 TUs) or the
// `std::getline(ss, tok, ',')` comma split (perfdsc/perfDscImportHelper.cpp:28,
// sys-arch-spec/dscglobal/dscglobal.cpp:127, initpacket/initpacket.cpp:339,
// dsc/superdsc.cpp:890, sgr/symTensorShape.cpp:48 and :67, 25 more).
//
// AND NOTE WHY "NARROW f3" IS NOT AVAILABLE AS A FIX: insertion into a
// stringstream is NOT rule-driven at all.  This module ships FOUR keys -- t1, f1,
// f2, f3 -- and ZERO insertion keys, because `ss << x` is lowered by the
// converter's BUILT-IN ostream path (converter_lib.cpp:552 IsCallToOstream,
// converter.cpp:2073 ConvertCallToOstream).  There is therefore no key whose
// absence could gate the wrong case, and no key to refuse.  A correct fix must
// change the MODEL to carry `(buffer, put_pos)`, which reshapes t1/f1/f2/f3
// together and is a much larger row than this one.  DO NOT ADD AN INSERTION KEY
// OR A `str(const std::string &)` SETTER TO THIS MODULE WITHOUT FIXING THE MODEL
// FIRST -- doing so would make the append/overwrite divergence reachable.
//
// NOT COVERED, deliberately, each because it needs its own harvested key and
// none has one: `str(const std::string &)` (a SETTER; it needs a `&mut`
// receiver the rule ABI cannot express), the (openmode)-only constructor,
// the std::ios_base formatting manipulators.
//
// ⭐ EXTRACTION IS NOW COVERED, but NOT BY A KEY IN THIS MODULE.  `ss >> tok`
// resolves to the FREE `operator>>(std::istream &, std::string &)`, whose
// receiver reaches `std::istream` through a DerivedToBase conversion -- so the
// key belongs to the std::istream owner and is `rules/iostream` f8.  THAT KEY
// ONLY TYPE-CHECKS BECAUSE OF THIS FLIP: the argument-position DerivedToBase is
// emitted with NO cast (plain `&mut ss`), so the stringstream's own Rust type
// must BE the `std::istream` type.  While t1 was Vec<u8> and iostream t4 was
// std::fs::File, that call was an E0308 no matter what the rule body said.
//
// STILL NOT COVERED: the MEMBER `operator>>` overloads and the basic_ios
// predicates (`eof`/`good`/`operator bool`/`operator!`), because a MEMBER call's
// receiver IS emitted with a cast -- `(ss as Cpp2RustUnmapped_std_ios).eof()`,
// dip/dip.cpp.rs:5478 -- which no rule body can remove; and `std::getline`,
// which the converter does not rule-search at all: it emits an undefined ported
// call `getline_99(&mut s_stream, &mut substr, ...)` (dip/dip.cpp.rs:6414, with
// no `fn getline_99` anywhere in the file).  A getline key would be DEAD.  std::istringstream is a separate owner (rules/basic_istringstream,
// rows g144/g1301-g1303).

#include <sstream>
#include <string>

using t1 = std::stringstream;

std::stringstream f1() { return std::stringstream(); }

std::string f2(const std::stringstream &o) { return o.str(); }

std::stringstream f3(const std::string &o, std::ios_base::openmode m) {
  return std::stringstream(o, m);
}
