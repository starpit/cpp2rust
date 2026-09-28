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
