// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::basic_ios<char>: the stream state predicates.
//
// good(), eof(), fail(), bad(), clear(), operator bool() and operator!() are
// members of std::basic_ios<char>, NOT of the concrete stream classes.  Every
// char stream -- std::stringstream, std::ostringstream, std::istringstream,
// std::ifstream, std::ofstream, std::istream, std::ostream, std::cin/cout/cerr
// -- resolves them to the one signature `... std::basic_ios<char>::good() const`
// and friends.  There is exactly ONE rule per predicate for the whole program,
// so a rule body cannot assume any particular Rust representation.
//
// The two representations in play are:
//
//   * rules/sstream maps every string stream to Box<Vec<u8>>, a drain buffer:
//     insertion appends, extraction consumes off the front, so "nothing left
//     to extract" is exactly "the buffer is empty";
//   * rules/fstream, rules/iostream and rules/raw_ostream map every file
//     stream and std::cout/cerr/cin to ::std::fs::File.
//
// The signature genuinely cannot be split -- `ofstream`, `ifstream` and
// `stringstream` all print `std::basic_ios<char>::operator!() const` -- so
// these rules used to live in rules/sstream and answered the string-stream
// buffer model for file streams too, emitting `!(*f.borrow()).is_empty()` on a
// ::std::fs::File.  That is non-compiling Rust on EVERY file stream.
//
// The fix is representation polymorphism inside the rule body: each body
// declares a private trait with a single `__cc2_iostate()` method, implements
// it for Vec<u8> and for ::std::fs::File, and calls it on the receiver.  Rust
// method resolution picks the impl from the receiver's static type -- Box
// auto-derefs to Vec<u8>, a File (or a *mut File, or a (*x.borrow()) place)
// auto-refs to &File -- so the dispatch is static and free, both models work
// unchanged, and neither rules/sstream nor rules/fstream has to give up its
// representation.  Adding a third stream representation later means adding one
// impl to each of these seven bodies and nothing else.
//
// `__cc2_iostate()` returns a std::ios_base::iostate bitmask with libc++'s
// values, the same ones rules/ios_base publishes: badbit = 1, eofbit = 2,
// failbit = 4, goodbit = 0.  Every predicate below is then the C++ definition
// projected out of that one word:
//
//     good()          state == goodbit
//     eof()           state & eofbit
//     fail()          state & (failbit | badbit)
//     bad()           state & badbit
//     operator bool() !fail()
//     operator!()     fail()
//
// so the six predicates cannot drift out of agreement with each other.
//
// The per-representation state:
//
//   * Vec<u8> (string stream): empty => eofbit | failbit, else goodbit.  This
//     is bit-for-bit the behaviour rules/sstream had, so no string-stream
//     translation changes.
//   * ::std::fs::File: the model has no error state to report -- both fstream
//     constructors are `File::open(..).expect(..)`, so a failed open panics
//     and a stream that exists was opened successfully.  badbit and failbit
//     are therefore never set, and `if (!file)` / `DT_CHECK_MSG(file, ..)`
//     after an open is always the success branch (open failure is a panic
//     instead of a diagnostic -- a pre-existing property of the fstream
//     constructors, not of these rules).  eofbit IS real: it is
//
//         stream_position() > 0 && stream_position() >= metadata().len()
//
//     read with Seek and Metadata on `&File`, both of which take &self, which
//     is what these const predicates have.
//
//     The `position > 0` conjunct is what keeps a stream that has not been
//     read yet out of eofbit, and it is load-bearing in three places: a
//     freshly opened EMPTY std::ifstream reports eof() false / good() true,
//     exactly as C++ does until a read is attempted; a freshly created
//     std::ofstream (position 0, length 0 after truncation) likewise reports
//     good() true; and `std::ifstream fp(name); return fp.good();`, the
//     "did this file open" idiom in dcg/tools/dcg_standalone.cpp and
//     dcg/tools/dcg_inpfetch_standalone.cpp, answers true for any file that
//     opened, empty or not.  Without it every output stream and every empty
//     input stream would report good() false.
//
//     A non-seekable File -- a tty or a pipe, which is what std::cout and
//     std::cerr map to when they are not redirected -- makes
//     stream_position() fail, and the stream reports goodbit.  A failure to
//     stat reports goodbit too (hence unwrap_or(u64::MAX)): the safe default
//     for an unknown stream is "not at end".
//
// One known divergence from C++: C++ sets eofbit only after a read actually
// runs off the end, so a read that consumes exactly the last byte of a
// non-empty file leaves eof() false in C++ and true here.  That is the
// direction that makes `while (!f.eof()) { getline(f, line); .. }` -- how
// dcg/tools/mda/memDumpAnalyzer.h, dsc/pcfg.cpp, dsc/superdsc.cpp,
// dsc/dataOpDsc.cpp and dsc/designSpaceConfig.cpp all drive an input file --
// terminate rather than spin.
//
// clear() resets the error state, which in both representations has no
// independent existence, so it is a no-op.  It is spelled with a by-value
// receiver parameter on purpose: that gives the rule a `borrow` placeholder
// rather than a `Ptr<Box<Vec<u8>>>` one, and a Ptr-typed parameter would make
// the converter emit `(f.as_pointer() as Ptr<Box<Vec<u8>>>)` -- a cast to the
// string-stream representation -- on a file-stream receiver.

#include <sstream>

bool f1(const std::stringstream &o) { return o.good(); }

bool f2(const std::stringstream &o) { return o.eof(); }

bool f3(const std::stringstream &o) { return o.fail(); }

bool f4(const std::stringstream &o) { return o.bad(); }

void f5(std::stringstream &o) { return o.clear(); }

bool f6(const std::stringstream &o) { return o.operator bool(); }

bool f7(const std::stringstream &o) { return o.operator!(); }

// ---------------------------------------------------------------------------
// fill(), width() and put() -- the same dispatch problem, the same answer.
//
// fill/width are STREAM STATE, not operations, and the state they need already
// exists: libcc2rs::Cc2Insert carries a per-stream pending width and fill
// character (a field on StringStream, a thread-local table keyed by descriptor
// for ::std::fs::File) because std::setw/std::setfill needed exactly that.
// These four rules are therefore only the MEMBER spelling of state the runtime
// already keeps, and they agree with the manipulator spelling by construction:
// `os.width(6)` and `os << std::setw(6)` set the same word.
//
// The bodies call the library trait directly rather than declaring a private
// one, for the reason libcc2rs/src/stream_fmt.rs gives for the extractors: the
// trait is declared once in the library with an impl per representation
// (StringStream, ::std::fs::File, and the Box/&mut/*mut/Ptr wrappers), so
// dispatch is still static and still chosen from the receiver's Rust type, and
// a representation nobody implemented is a compile error naming the trait.
//
// C++ semantics being reproduced, and the asymmetry is the point:
//   * width(n) applies to the NEXT inserted item only, then resets to 0 --
//     cc2_take_width() is what every insertion already calls;
//   * fill(c) PERSISTS until changed;
//   * both setters return the PREVIOUS value (`auto prev = os.fill('0');` at
//     util/sendefs/senulatorProg.cpp:141 depends on it);
//   * the width getter must NOT consume the pending width, so it takes and puts
//     it straight back -- the only way to read it through this trait.
//
// put(c) is UNFORMATTED output: it writes one byte and the pending width does
// NOT pad it (measured against clang). So it calls cc2_write, not
// cc2_pad_and_write, which is the one-line difference from `os << c`.
char f8(const std::stringstream &o) { return o.fill(); }

char f9(std::stringstream &o, char c) { return o.fill(c); }

std::streamsize f10(std::stringstream &o) { return o.width(); }

std::streamsize f11(std::stringstream &o, std::streamsize n) { return o.width(n); }

std::ostream &f12(std::ostream &o, char c) { return o.put(c); }
