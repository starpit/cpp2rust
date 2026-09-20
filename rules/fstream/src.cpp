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

// The std::string overloads.  f1/f5 cover only the `const char *` spelling, so
// every `std::ofstream out(fileName)` where fileName is a std::string fell
// back to the mangled `std_basic_ofstream_..::std_basic_ofstream_..N()`
// placeholder.  As with f1/f5 the openmode is dropped: std::ofstream and
// std::ifstream both map to ::std::fs::File, which is opened for write and
// read respectively.
std::ofstream f9(const std::string &filename, std::ios_base::openmode mode) {
  return std::ofstream(filename, mode);
}

std::ifstream f10(const std::string &filename, std::ios_base::openmode mode) {
  return std::ifstream(filename, mode);
}

// ---------------------------------------------------------------------------
// The two-step open: `std::ofstream out; out.open(name, mode);`
//
// f1/f5/f9/f10 above only cover the one-step form, so dcg/tools/dcg_standalone.cpp,
// dcg/tools/dcg_inpfetch_standalone.cpp, dcg/dcg_fe/pcfg_gen/stcdpOp.cpp and
// dcg/tools/mda/memDumpAnalyzer.h -- which all default-construct the stream and
// then call open() -- fell back to the mangled
// `std_basic_ofstream_..::std_basic_ofstream_..N()` placeholder for the
// constructor and to same-named method calls (`.open(..)`, `.is_open()`,
// `.close()`) that ::std::fs::File does not have.
//
// The model needs a value for "this stream is not open", and ::std::fs::File
// has no such state.  /dev/null is the closest thing that is representable and
// that behaves right: writes to it are discarded, exactly as writes to an
// unopened ofstream are, and reads from it hit end-of-file immediately, exactly
// as reads from an unopened ifstream do.  So:
//
//   * the default constructors open /dev/null;
//   * close() reopens /dev/null, which drops the previous File and so closes
//     the real descriptor;
//   * is_open() asks whether the handle is a REGULAR FILE, which distinguishes
//     a stream that has been open()ed from one that has not (or has been
//     closed).  A stream deliberately opened on a device or a fifo reports
//     false; that is the one case this test gets wrong, and no case in the
//     scheduler scope does it.
//
// open() honours std::ios_base::app (bit 1, see rules/ios_base) and otherwise
// truncates, which is C++'s `out | trunc` default.  The other openmode bits
// have no effect here, as they already have none in f1/f5/f9/f10.

std::ofstream f11() { return std::ofstream(); }

std::ifstream f12() { return std::ifstream(); }

void f13(std::ofstream &o, const std::string &filename,
         std::ios_base::openmode mode) {
  return o.open(filename, mode);
}

void f14(std::ifstream &o, const std::string &filename,
         std::ios_base::openmode mode) {
  return o.open(filename, mode);
}

bool f15(const std::ofstream &o) { return o.is_open(); }

bool f16(const std::ifstream &o) { return o.is_open(); }

void f17(std::ofstream &o) { return o.close(); }

void f18(std::ifstream &o) { return o.close(); }
