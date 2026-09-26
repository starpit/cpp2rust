// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::raw_ostream -- LLVM's debug/diagnostic output API.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/Support/...>
// ----------------------------------------------------------------------
// cpp-rule-preprocessor compiles this file with a fixed flag set; a module
// that needs more passes them through a `cxxflags` file next to src.cpp, and
// the only flags that would reach LLVM's headers are absolute -I paths into
// whatever LLVM tree the target project happens to have built.  That would
// make `ninja` in this repo fail for anyone without that tree.  So the
// signatures LLVM declares are restated here instead.
//
// A rule matches on a SIGNATURE STRING -- return type, qualified name and
// parameter types, e.g. "llvm::raw_ostream & operator shl(llvm::StringRef)" --
// so a restatement matches iff it agrees with LLVM exactly.  It was checked
// against the genuine header: running cpp-rule-preprocessor over a version of
// this file that #includes llvm/Support/raw_ostream.h, llvm/Support/Debug.h
// and llvm/ADT/StringRef.h produces a byte-identical ir_src.json, and the
// rules fire on real translation units that include those headers.  If LLVM
// ever changes one of these signatures the corresponding rule silently stops
// matching, so re-run that check when upgrading LLVM.
//
// MODEL
// -----
// A raw_ostream is a file descriptor, std::fs::File, exactly as
// rules/iostream models std::ostream.  errs()/dbgs() hand back the process
// stderr and outs() the process stdout through the libcc2rs thread-local
// registry (cerr()/cout()), so the handle stays valid for the life of the
// thread and successive writes append rather than race.
//
// Why File and not a byte buffer (rules/sstream's choice): raw_ostream's
// reason to exist in the target codebase is llvm::errs(), i.e. side effects on
// a real stream, not a buffer someone later reads back.  raw_string_ostream
// and raw_svector_ostream -- the buffer-backed subclasses -- are NOT covered
// here; they would need the sstream model and their own rules.
//
// operator<< is a MEMBER of raw_ostream, and Mapper keys a member operator on
// its return type plus parameters, not on the receiver's class.  That is why
// every rule below returns raw_ostream: the return type is the only thing
// keeping these rules off std::ostream's own member operator<<, which the
// converter handles with its built-in path (IsCallToOstream) and which
// Mapper::Contains would otherwise shadow.  Checked: std::cout/std::cerr
// output is byte-identical with and without this module loaded.
//
// Each operator<< rule RETURNS THE STREAM rather than unit.  The generated
// MLIR enum printers (*Enums.h.inc) end in `return p << valueStr;`, an
// operator<< in value position, and `errs() << a << b` feeds one call's result
// in as the next call's receiver.  A unit-returning rule (the shape
// rules/string uses for std::string::append) breaks both.  The receiver
// placeholder appears exactly once in every body so that a chain does not
// re-emit -- and so re-evaluate -- its left-hand side.
//
// KNOWN GAP (refcount model only): `llvm::errs() << x` with errs() as the
// DIRECT receiver of << aborts in ConverterRefCount::VisitCallExpr -- its
// isObject() branch returns without setting computed_expr_type_, and a
// zero-argument mapped call has no nested conversion to set it incidentally.
// Every other shape works in both models, including errs() reaching a stream
// parameter through a call argument.  The unsafe model has no such gap.
//
// NOT COVERED: the formatting helpers (format(), formatv(), write_hex(),
// indent, Colors), the SmallVectorImpl<char> and std::string_view overloads.
// They need rules for their argument types first.
//
// operator<<(const void *) IS covered (f18).  It was once left out on the
// grounds that the refcount model's AnyPtr has no machine address to print,
// but that is not a reason to refuse the insertion: AnyPtr already has an
// IDENTITY (`to_int`, the same number its pointer-to-integer casts use), so
// the digits are stable within a run and differ between distinct pointers,
// which is everything a correct program can depend on -- two C++ runs of the
// same program do not agree on the digits either (ASLR).  The rule reuses
// libcc2rs::cc2_addr_of, added for std::ostream's pointer insertion, so both
// models get the same notion of address.  NOTE the FORMAT differs from
// std::ostream: raw_ostream is write_hex(PrefixLower), an unconditional "0x"
// followed by unpadded lowercase hex, so a null pointer is `0x0`, whereas
// std::ostream's num_put uses hex|showbase and prints a bare `0`.
// `bool` needs no rule of its own: bool -> int is an integral promotion and
// bool -> char only a conversion, so C++ always picks operator<<(int) for it.
// llvm::StringRef IS mapped -- rules/stringref t1, to the same eager
// NUL-terminated Vec representation this module and rules/twine use.  (This line
// previously claimed it had no type rule anywhere; that was stale and TWO agents
// read it as a measurement and acted on it.), so the ARGUMENT of f7
// still translates as an opaque llvm_StringRef; the rule itself resolves,
// which is what unblocks the enum printers.

#include <string>

namespace llvm {

// Restated from llvm/ADT/StringRef.h.  Only the name matters to the rules --
// it is the parameter type of one operator<< overload -- but the class has to
// be complete, because f7 takes one by value.
class StringRef {
  const char *Data;
  unsigned long Length;
};

// Restated from llvm/Support/raw_ostream.h.  Only the members below are
// used; the rest of the class is irrelevant to signature matching.
class raw_ostream {
public:
  void flush();

  raw_ostream &operator<<(char C);
  raw_ostream &operator<<(unsigned char C);
  raw_ostream &operator<<(signed char C);
  raw_ostream &operator<<(StringRef Str);
  raw_ostream &operator<<(const char *Str);
  raw_ostream &operator<<(const std::string &Str);
  raw_ostream &operator<<(unsigned long N);
  raw_ostream &operator<<(long N);
  raw_ostream &operator<<(unsigned long long N);
  raw_ostream &operator<<(long long N);
  raw_ostream &operator<<(unsigned int N);
  raw_ostream &operator<<(int N);
  raw_ostream &operator<<(double N);
  raw_ostream &operator<<(const void *P);
};

// raw_fd_ostream derives from raw_ostream through raw_pwrite_stream; the
// intermediate class does not appear in any signature, so it is skipped.
class raw_fd_ostream : public raw_ostream {};

raw_fd_ostream &outs();
raw_fd_ostream &errs();
// llvm/Support/Debug.h.
raw_ostream &dbgs();

} // namespace llvm

using t1 = llvm::raw_ostream;
using t2 = llvm::raw_ostream &;
using t3 = llvm::raw_ostream *;
using t4 = llvm::raw_fd_ostream;
using t5 = llvm::raw_fd_ostream &;

// Stream accessors.
llvm::raw_fd_ostream &f1() { return llvm::errs(); }

llvm::raw_fd_ostream &f2() { return llvm::outs(); }

llvm::raw_ostream &f3() { return llvm::dbgs(); }

void f4(llvm::raw_ostream &o) { return o.flush(); }

// Insertion.  Overload resolution is exact, so every C++ overload the target
// codebase can reach needs its own rule: `int` and `long` are different member
// functions even where Rust would print them identically.
llvm::raw_ostream &f5(llvm::raw_ostream &o, const char *v) {
  return o.operator<<(v);
}

llvm::raw_ostream &f6(llvm::raw_ostream &o, const std::string &v) {
  return o.operator<<(v);
}

llvm::raw_ostream &f7(llvm::raw_ostream &o, llvm::StringRef v) {
  return o.operator<<(v);
}

llvm::raw_ostream &f8(llvm::raw_ostream &o, char v) {
  return o.operator<<(v);
}

llvm::raw_ostream &f9(llvm::raw_ostream &o, unsigned char v) {
  return o.operator<<(v);
}

llvm::raw_ostream &f10(llvm::raw_ostream &o, signed char v) {
  return o.operator<<(v);
}

llvm::raw_ostream &f11(llvm::raw_ostream &o, int v) {
  return o.operator<<(v);
}

llvm::raw_ostream &f12(llvm::raw_ostream &o, unsigned int v) {
  return o.operator<<(v);
}

llvm::raw_ostream &f13(llvm::raw_ostream &o, long v) {
  return o.operator<<(v);
}

llvm::raw_ostream &f14(llvm::raw_ostream &o, unsigned long v) {
  return o.operator<<(v);
}

llvm::raw_ostream &f15(llvm::raw_ostream &o, long long v) {
  return o.operator<<(v);
}

llvm::raw_ostream &f16(llvm::raw_ostream &o, unsigned long long v) {
  return o.operator<<(v);
}

llvm::raw_ostream &f17(llvm::raw_ostream &o, double v) {
  return o.operator<<(v);
}

// raw_ostream.h: `operator<<(const void *P)` is write_hex(PrefixLower), NOT
// the stream's own format state -- raw_ostream has none.
llvm::raw_ostream &f18(llvm::raw_ostream &o, const void *v) {
  return o.operator<<(v);
}
