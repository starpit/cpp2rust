// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <fstream>
#include <iostream>
#include <iterator>

using t1 = std::ifstream;
using t2 = std::ofstream;
using t3 = std::ostream_iterator<char>;

std::ofstream f1(const char *filename, std::ios_base::openmode mode) {
  return std::ofstream(filename, mode);
}

template <typename T1>
std::ostream_iterator<T1> f2(const std::ostream_iterator<T1> &a0) {
  return std::ostream_iterator<T1>(a0);
}

template <typename T1> std::ostream_iterator<T1> f3(std::ostream &a0) {
  return std::ostream_iterator<T1>(a0);
}

std::filebuf *f4(const std::ifstream &o) { return o.rdbuf(); }

std::ifstream f5(const char *filename, std::ios_base::openmode mode) {
  return std::ifstream(filename, mode);
}

template <typename T1>
std::istream_iterator<T1> f6(const std::istream_iterator<T1> &a0) {
  return std::istream_iterator<T1>(a0);
}

template <typename T1>
std::istreambuf_iterator<T1> f7(std::istreambuf_iterator<T1> &a0) {
  return std::istreambuf_iterator<T1>(a0);
}

std::istreambuf_iterator<char> f8(std::basic_streambuf<char> *p) {
  return std::istreambuf_iterator<char>(p);
}

// ---------------------------------------------------------------------------
// t4 -- std::istreambuf_iterator<char>.
//
// This is an `f`-KEY-WITHOUT-`t`-KEY GAP, not a new family.  This module ALREADY
// carries a method on this exact type:
//     void std::basic_ofstream<char>::basic_ofstream(const char *, unsigned int)   (f1)
//     std::istreambuf_iterator<char>::istreambuf_iterator(std::streambuf *)        (f8)
// but ir_src.json had no TYPE entry at either spelling, so a use site that needs
// the type itself aborts with `has no model in types_` even though its
// constructor is keyed.  build.py named the bogus owners rules/basic_ofstream and
// rules/istreambuf_iterator after the TYPE, and could not see that the type was
// already half-modelled here.
//
// ⛔ MEASURED NEGATIVE RESULT, recorded here so nobody pays for it twice.
// g318/g353 searched `std::basic_ofstream<char>` and it is NOT REACHABLE as a
// type key from this file.  I wrote `using tN = std::basic_ofstream<char>;`,
// regenerated, and READ IT BACK from ir_src.json:
//     "t4": "std::ofstream"
// -- i.e. the TYPE-key printer resolves the resolved spelling back DOWN to the
// typedef, producing a byte-for-byte DUPLICATE of t2 and NOT the spelling the
// rows searched.  The entry was therefore removed rather than shipped dead.
//
// That is the real shape of the asymmetry the owner audit spotted: f1 above
// records at `std::basic_ofstream<char>` because a FUNCTION key is built from
// the CXXRecordDecl's qualified name, whereas a TYPE key goes through a printer
// that prefers the typedef.  So an `f`-key at a resolved spelling does NOT imply
// a `t`-key is obtainable at that same spelling, and g318/g353 cannot be closed
// by adding a type entry here.  They need a converter-side change (make the type
// search try the typedef spelling, or make it record the resolved one), which is
// not this slot's file.
//
// The MODELS are forced, not chosen: f1 already returns ::std::fs::File for the
// basic_ofstream ctor and f8 already returns ::std::fs::File for the
// istreambuf_iterator ctor, so the type entries must agree with the committed
// function entries or the module would contradict itself.
//
// NOT COVERED: the default-constructed END iterator `std::istreambuf_iterator<char>()`
// (no keyed ctor here, and no row asks for it), and `std::basic_ifstream<char>`
// (no row searched the resolved spelling for the input side).

using t4 = std::istreambuf_iterator<char>;

// ---------------------------------------------------------------------------
// f9 -- `std::ofstream(const std::string &, std::ios_base::openmode)`.
//
// MEASURED ABSENT, verbatim from a `-verbose` run of
// sys-arch-spec/initpacket/initpacket.cpp against pin/ir.v20:
//     search expr void std::basic_ofstream<char>::basic_ofstream(const std::string &, unsigned int), result:
//     None
// This is the FABRICATED-`::new_<N>` class: with no ctor key the converter emits
// `std_basic_ofstream_char_...::new_N` at rc=0 with no placeholder token, so
// pin/no-placeholders.sh cannot see it.
//
// ONE key, not two: the corpus overwhelmingly writes `std::ofstream ofs(name);`
// with the openmode DEFAULTED, and the recorder writes a defaulted argument out
// at the call site, so `ofs(name)` and `ofs(name, std::ios::binary)` record as
// this SAME key.  A nullary/one-arg second entry would be silently redundant
// (the `substr` precedent).
//
// ⭐ THE OPENMODE IS NOW HONOURED, and f1 was fixed in the SAME change (its key
// string is untouched; only its BODY changed, and it gained the `a1` parameter the
// key always had).  The earlier version of this rule DISCARDED `mode` and used
// `File::create`, which TRUNCATES.  That was not a theoretical loss: the corpus has
// `ios::app`/`ios_base::app` at 10 sites, three of them exactly this
// ctor-with-mode shape, two of them append-mode LOG files --
//     ddb/src/Standardization/DDBStandardizationMgr.h:179
//         std::ofstream log_file(erroFileName, std::ios_base::app);
//     ddb/src/Standardization/DDBStandardizationMgr.cpp:152
//         std::ofstream file(fileName, std::ios::app);
//     spyrecode-host-functions/processSpyreCodeArtifacts_standalone.cpp:205
//         std::ofstream out(name, std::ios::binary);
// -- so the discarding body would have emitted a program that RUNS and silently
// wipes two log files.  That is the SILENT WRONGNESS class, worse than a loud
// failure, and it is why this rule was not committed as written.
//
// The mapping, the measured bit values, and the disposition of
// ate/binary/in/trunc are documented once, on f1 in tgt_unsafe.rs.
//
// ⚠️ STILL LOSSY, named: f5 (`std::ifstream(const char *, openmode)`) also
// discards its mode.  That is NOT the same exposure -- `File::open` is read-only
// and destroys nothing -- so the worst case there is a missing `in|out`/`ate`
// position, not data loss.  Left alone deliberately: no corpus site asks for it,
// and changing it is not needed to remove the truncation bug.
std::ofstream f9(const std::string &filename, std::ios_base::openmode mode) {
  return std::ofstream(filename, mode);
}
