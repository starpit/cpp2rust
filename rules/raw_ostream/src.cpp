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
// a real stream, not a buffer someone later reads back.
// ⚠️ THIS PARAGRAPH USED TO SAY raw_string_ostream AND raw_svector_ostream ARE BOTH
// UNCOVERED.  raw_string_ostream IS NOW COVERED -- t560/f560/f561 at the bottom of
// this file, with its own (NOT-a-File) model and the measurement behind it.  Only
// raw_svector_ostream is still out, and it needs the SMALLVECTOR model rather than
// this one, because it holds a `SmallVectorImpl<char> &`.  Corrected 2026-09-28; a
// stale "NOT covered" line here is exactly the kind of thing a later slot reads as a
// measurement and acts on.
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

#include <optional>   // f3065's parameter (rules/optional t1 -> Option<T1>)
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

// ---------------------------------------------------------------------------
// TWO ROWS EXAMINED 2026-09-28 AND DELIBERATELY NOT WRITTEN HERE.  Recorded so
// the next slot does not re-derive them.  NEITHER changed a key, so this comment
// leaves ir_src/ir_unsafe/ir_refcount byte-identical.
//
// g821 -- `llvm::raw_ostream & operator shl(llvm::raw_ostream &,
//         mlir::AffineMap)`, 1 TU (AffineMinCanonicalization.cpp:120
//         `LDBG(1) << "  Map: " << applyOp.getAffineMap()`).
//   NOT STALE: grepped the whole rule tree, NO module declares an operator<< for
//   mlir::AffineMap.  But it is NOT THIS MODULE'S ROW either.  The three sibling
//   rows for mlir::Type / mlir::Attribute / mlir::Location were closed as
//   satisfied by rules/mlir f126/f127/f128, which declare the FREE
//   `operator<<(llvm::raw_ostream &, <mlir type>)` in rules/mlir against a
//   deliberately INCOMPLETE `namespace llvm { class raw_ostream; }` whose own
//   comment reads "rules/raw_ostream owns this type".  Writing the AffineMap
//   overload here instead would need a second declaration of mlir::AffineMap,
//   whose type rule is rules/mlir t8 -- a duplicate type rule for one spelling in
//   two modules, i.e. load-order-dependent.  And the fidelity question is
//   rules/mlir's to answer, not this module's: the rendered text has to be
//   `AffineMap::print`'s, i.e. the Display of dataflowir-gen's
//   `AffineMap { n_dims, n_symbols, results }` (ir.rs:313-315).  Row left OPEN with
//   owner rules/mlir.
//
// g822 -- `llvm::raw_ostream & operator shl(llvm::raw_ostream &,
//         const std::optional<long> &)`, 1 TU, 2 sites
//         (CFGSimplificationSentientLevel.cpp:326 and :332, both printing
//         `interval_marker_`).
//   This one IS this module's row: the overload is
//   llvm/Support/raw_ostream.h:846, a template inside `namespace llvm`, which is
//   why the key prints UNQUALIFIED (the shift family prints the WRITTEN
//   nested-name-specifier, empty for a function declared in its namespace's own
//   body) -- matching the sample exactly.  Its body is
//       if (O) OS << *O; else OS << std::nullopt;
//   ⭐⭐ ROW CLOSED 2026-09-29 by slot g3063 -- f3065 at the bottom of this file.
//   THE BLOCK BELOW IS LIFTED, BY THE SECOND OF THE TWO PROBES IT ASKS FOR.  THE
//   DISENGAGED RENDERING IS THE LITERAL FOUR CHARACTERS `None`.  Measured, not
//   inferred:
//       // /home/agent/work/g3063/nullopt.cpp
//       #include <llvm/Support/raw_ostream.h>
//       #include <optional>
//       int main() {
//         std::optional<long> a = 7, b;
//         llvm::outs() << "[" << a << "]\n";
//         llvm::outs() << "[" << b << "]\n";
//         llvm::outs().flush();
//         return 0;
//       }
//       $TC/shim4/clang++ -std=c++17 -I$LLVM_ROOT/include nullopt.cpp \
//           -L$LLVM_ROOT/lib -lLLVMSupport -lLLVMDemangle -o nullopt
//   prints
//       [7]
//       [None]
//   i.e. the engaged case is f13's `operator<<(long)` and the disengaged case is
//   `None` -- NOT "nullopt", NOT "(null)", NOT empty.  This is the REAL LLVM
//   22.1.3 in this toolchain answering for itself, which is strictly better
//   evidence than the `objdump -s -j .rodata` route the entry proposes (and that
//   route was unavailable anyway: `objdump`/`nm`/`readelf` are NOT on PATH here,
//   only `llvm-objdump`/`llvm-nm`/`llvm-readobj` under $LLVM_ROOT/bin).
//   THE ORIGINAL BLOCKER TEXT FOLLOWS, kept because its facts are true and a
//   reader must be able to see what was actually in the way:
//   The engaged branch is f13's body verbatim.  ⛔ THE DISENGAGED BRANCH WAS THE
//   BLOCKER, AND IT IS A ONE-FACT BLOCK: it calls
//   `operator<<(raw_ostream &, std::nullopt_t)` (raw_ostream.h:842), whose
//   DEFINITION is in raw_ostream.cpp -- NOT shipped in this toolchain, which has
//   headers only.  The exact text it writes is therefore UNKNOWN here, and it is
//   observable: `interval_marker_` is empty on precisely the path these two debug
//   lines exist to report, so guessing the literal would silently emit the wrong
//   bytes for the disengaged case while the engaged case looked right.
//   ⚠️ AND DO NOT TRUST THE OBVIOUS PROBE: `strings` IS NOT INSTALLED on this VM,
//   so `strings libLLVMSupport.a | grep nullopt` returns "command not found" and
//   an unwary reader records the empty result as absence.  `grep -a` over the
//   archive does run, and finds only the four MANGLED SYMBOL names
//   (`...11raw_ostreamESt9nullopt_t`) -- never the literal -- so the text lives in
//   a `.L.str` this probe cannot attribute.  What WOULD settle it: `objdump -s -j
//   .rodata` on that archive's raw_ostream.cpp.o, or a 3-line program linked
//   against this LLVM that prints a disengaged `std::optional<long>`.
//   Row BLOCKED as blocked:refused-unknown-nullopt-rendering.
//   ⭐ END OF THE ORIGINAL ENTRY.  The second proposal is what settled it; the row
//   is CLOSED, key f3065.  ⚠️ NOTE FOR THE NEXT READER: the entry's own last
//   sentence named the experiment that would lift it, and the row still sat blocked
//   -- "a probe is proposed" is not "a probe was run".

// ---------------------------------------------------------------------------
// t540 -- `llvm::impl::raw_ldbg_ostream`, 62 asks over the freshest full sweep
// (fresh34, 84 logs), 59 emitted placeholder sites across 11 A-bucket `.rs` files.
// Recorded key, taken from the log's own `searched as:` line and not from the
// `from decl` text the recorder labels "NOT a key":
//     searched as: llvm::impl::raw_ldbg_ostream
// and NOTHING else -- no `&`/`*` spelling is asked, so ONE value key is the whole
// row (unlike t1/t2/t3, whose ref and pointer spellings ARE asked separately).
//
// ⭐ IT IS JUST ANOTHER raw_ostream SPELLING, checked in the header rather than
// assumed: llvm/Support/DebugLog.h:233 reads
//     class LLVM_ABI raw_ldbg_ostream final : public raw_ostream
// and its only job is `write_impl` splitting on '\n' to re-emit a prefix before
// each line, forwarding everything to an underlying `raw_ostream &Os`.  So it is
// the SAME relationship `raw_fd_ostream` (t4/t5) already has to `raw_ostream`, and
// it gets the same model -- a `std::fs::File`.
//
// ⚠️ IT IS A DEBUG STREAM, AND HERE IS WHERE ITS BYTES GO, stated rather than
// invented.  `LDBG()` builds one of these over `llvm::dbgs()`; this module already
// decided (f3) that `dbgs()` is the process stderr, unconditionally, because the
// refcount/unsafe models have no `-debug-only` gate.  So a raw_ldbg_ostream's bytes
// land on stderr, which is observable, and this key does not create a sink that
// was not already there.  ⛔ WHAT IS NOT MODELLED, named so no one reads this as
// fidelity: the PREFIX (`[file:line]`) that is the class's entire reason to exist
// is dropped, because the prefix is assembled from `__FILE__`/`__LINE__` of the
// C++ TU, which a Rust rule body cannot see.  Debug-only text, no program can
// depend on it, and the alternative was 59 loud undefined names.
//
// ⛔ NO MEMBER IS KEYED, and the census says none is needed.  BOTH reads:
//   (1) the ask logs -- 62 TYPE asks, zero member asks for this type;
//   (2) the emitted corpus, all 58 `.rs` --
//       `Cpp2RustUnmapped_llvm_impl_raw_ldbg_ostream::[A-Za-z_0-9]*` is ZERO, so no
//       `::` member is reached.  The insertions written against these streams are
//       `operator<<` on the raw_ostream BASE and are already f5-f18: a member
//       operator is keyed on its return type plus parameters, not on the receiver's
//       class, so they match through the base decl with no new rule.
namespace llvm {
namespace impl {
// DebugLog.h:233.  `final` is dropped: nothing derives from it here and the
// keyword does not enter a type key.  Only the name and the base matter.
class raw_ldbg_ostream : public raw_ostream {};
} // namespace impl
} // namespace llvm

// ⚠️ INDEX t540, not the next free t6, DELIBERATELY: several slots are live in the
// rule tree today and t6 is the index a concurrent slot would also pick.  Indices
// are per-module and need not be dense.
using t540 = llvm::impl::raw_ldbg_ostream;

// ---------------------------------------------------------------------------
// t560 / f560 / f561 -- `llvm::raw_string_ostream`, WHICH THE MODULE DOC ABOVE
// EXPLICITLY LEFT OUT ("raw_string_ostream and raw_svector_ostream -- the
// buffer-backed subclasses -- are NOT covered here").  That line is now stale for
// raw_string_ostream; raw_svector_ostream is still out.
//
// ⭐ MEASURED, NOT INFERRED.  fresh36 (58 emitted `.rs`, pin binary
// afe3a6463ffe3e221aa47f583c7d8d52, rules pin/ir.v35) carries 4 live
// `Cpp2RustUnmapped_llvm_raw_string_ostream` sites, in 2 files:
//     MemoryTracker.cpp:3727 and :3753, UnitMaterializer.cpp:12225 and :12856,
// every one of them a `let mut <v>: Cpp2RustUnmapped_... = llvm_raw_string_ostream
// :: new ( { & mut <str> } )` -- the fabricated-ctor shape.
//
// ⭐ THE ZERO-HIT ACCESSOR GREP (`--verbose` + `grep -A1` on the search line) over
// MemoryTracker.cpp says the converter asks for EXACTLY THREE spellings for this
// type and nothing else -- no `&` and no `*` spelling is ever asked, unlike
// t1/t2/t3:
//     search type llvm::raw_string_ostream, result: None                        (6)
//     search expr void llvm::raw_string_ostream::raw_string_ostream(std::string &),
//                                             result: None                      (2)
//     search expr std::string & llvm::raw_string_ostream::str(), result: None    (8)
// So this row is one type key plus two member keys.  The recorder's own
// `searched as:` line agrees with the type spelling; the adjacent
// `from decl (NOT a key ...)` line names raw_ostream.h:662:16, which is the
// DECLARATION SITE and not a key.
//
// ⭐ THE MODEL IS A *REFERENCE TO THE STRING*, NOT A COPY OF IT, and that is forced
// by the header and by the corpus, not chosen.  raw_ostream.h:662 is
//     class raw_string_ostream : public raw_ostream { std::string &OS; ... };
// i.e. it holds the caller's string BY REFERENCE and its doc says "the std::string
// is always up-to-date, may be used directly and there is no need to call flush()".
// And BOTH read patterns occur in the corpus: MemoryTracker reads the buffer back
// through `ss.str()`, while UnitMaterializer:12230 reads the ORIGINAL local
// (`return printed.lower();`) after writing through `os`.  A by-value buffer -- the
// rules/sstream choice for std::ostringstream, which OWNS its string -- would
// silently lose UnitMaterializer's writes.  So the target type is the model's
// spelling of `std::string &`: a raw pointer to rules/string's t1 in the unsafe
// model, a `Ptr` to it in refcount.  `str()` is then the IDENTITY -- it returns the
// very reference the object stores (raw_ostream.h:681 `std::string &str() { return
// OS; }`) -- and the constructor is the identity too.
//
// ⛔ THE INSERTION SITES STAY LOUD, AND THAT IS THE SAME PRE-EXISTING CONVERTER
// DEFECT rules/sstream DOCUMENTS AT LENGTH, NOT SOMETHING THIS ROW INTRODUCES.
// `ss << x` on one of these DOES already match f5-f18 (verified in the verbose log:
// `Matching: llvm::raw_ostream & operator shl(const char *)`), and the converter
// inserts a DerivedToBase cast on the receiver to this module's raw_ostream target
// type, emitting `(( & mut ss as std::fs::File ) as *mut std::fs::File)`.  That is
// `error[E0605]: non-primitive cast` -- BEFORE this change (`Cpp2RustUnmapped_...`
// as File) and after it (`*mut Vec<c_char>` as File) alike, so nothing goes from
// loud to silent here; a cast the converter writes at the use site cannot be removed
// from inside a target body, and cpp2rust/converter/* is a different owner.
// rules/sstream reached the identical conclusion for the identical cast and shipped
// the model-correct bodies anyway; this follows it.
//
// ⛔ NOT COVERED, each because it is not asked anywhere in the corpus:
// `reserveExtraSpace`, and `raw_svector_ostream` (a DIFFERENT model -- it holds a
// `SmallVectorImpl<char> &`, i.e. rules/smallvector's type, not rules/string's).
// `flush()` needs no key of its own: it is f4 on the raw_ostream base, and on one of
// these streams it is a NO-OP in C++ anyway (the class is SetUnbuffered).

namespace llvm {
// raw_ostream.h:662.  ⛔ RESTATED EMPTY AND *UNRELATED* to raw_ostream, exactly as
// rules/mlir restates AsmPrinter/OpAsmPrinter unrelated and for the same reason: if
// the base relation were spelled here, `o.operator<<(...)` on one of these would be
// a derived-to-base conversion and f5-f18's keys -- 18 live keys -- would be at risk
// of being re-recorded against the derived spelling.  The real TU's AST carries the
// true inheritance regardless, which is why the `<<` sites match f5-f18 today.
class raw_string_ostream {
public:
  explicit raw_string_ostream(std::string &O);
  std::string &str();
};
} // namespace llvm

// ⚠️ INDICES t560/f560/f561, not the next free t6/f19, DELIBERATELY: several slots
// are live in the rule tree today and t6/f19 is what a concurrent slot would also
// pick.  Indices are per-module and need not be dense.
using t560 = llvm::raw_string_ostream;

// f560 -- the constructor.  The parameter is the string the stream writes THROUGH;
// the body is the identity, because the model type IS that reference.
llvm::raw_string_ostream f560(std::string &s) {
  return llvm::raw_string_ostream(s);
}

// f561 -- `str()`.  raw_ostream.h:681 is `{ return OS; }`, i.e. hand back the very
// reference the object holds, so the body is the identity here too.  NOT `const` in
// the header, and the key spelling above confirms it.
std::string &f561(llvm::raw_string_ostream &o) { return o.str(); }

// ---------------------------------------------------------------------------
// PASS 2026-09-29, slot `ostreamshl`: THREE MORE OF THE FREE `llvm::raw_ostream
// <<` MLIR FAMILY.  f600/f601/f602.
//
// ⭐ WHY THESE ARE HERE AND NOT IN rules/mlir, WHERE f126-f128 ARE.  f126
// (mlir::Type) / f127 (mlir::Attribute) / f128 (const mlir::Location &) landed in
// rules/mlir/src.cpp:4096-4125, and these three are their exact siblings.  They
// are in THIS module only because two other slots hold rules/mlir today and a
// concurrent edit to one 10k-line src.cpp is how a wave loses a module.  A KEY IS
// GLOBAL, NOT PER-MODULE -- the mapper searches one flat key space and `types_`
// is tree-wide -- so placement is free and the bodies resolve their arguments
// against rules/mlir's t18 / t83 / t8 exactly as if they sat next to f126.
// ⚠️ IF rules/mlir LATER ADOPTS THEM, MOVE, DO NOT COPY: two modules recording the
// same key is a cross-module disagreement, which is what check-ir.sh's "all rule
// modules agree" leg is for.
//
// THE CENSUS THAT SIZED THIS ROW (survey-v10, 403 TUs, all 45 `<<`-with-an-MLIR-
// operand shapes; 100 TUs contain at least one, 1,892 sites).  The
// `llvm::raw_ostream` LHS half is 63 TUs / 279 sites over 9 shapes:
//     mlir::Operation         24 TU  58 site   f3063 <-- landed 2026-09-29 (g3063)
//     mlir::Value             20 TU  80 site   REFUSED, see below -- STILL REFUSED
//     mlir::OpState           17 TU  56 site   f3064 <-- landed 2026-09-29 (g3063)
//     mlir::OperationName     10 TU  18 site   f600  <-- landed here
//     mlir::Attribute          8 TU  20 site   f127, rules/mlir
//     mlir::Location           8 TU  20 site   f128, rules/mlir
//     mlir::Type               4 TU  23 site   f126, rules/mlir
//     mlir::AffineExpr         1 TU   3 site   f601  <-- landed here
//     mlir::AffineMap          1 TU   1 site   f602  <-- landed here
//
// ⭐⭐ f600 EXISTS BECAUSE rules/mlir's OWN REFUSAL OF IT WENT STALE TODAY, and
// this is the second time that has happened to a refusal in that block (the
// AsmPrinter refusal at :4180 carries the same correction).  rules/mlir/src.cpp:
// 4085-4088 refuses `mlir::OperationName` on the ground that
//     "t18's model is `Option<&'static TdOpDef>` and TdOpDef's fields
//      (td.rs:446-457) carry no full-name field and no dialect prefix."
// That was TRUE when written and is FALSE since 91b3c13a (2026-09-29), which
// landed f2100 `OperationName::getStringRef()` in that same module.  f2100's
// target body (rules/mlir/tgt_unsafe.rs:4699) is
//     format!("{}.{}", dataflowir_gen::row_dialect(__d), __d.mnemonic)
// i.e. the FULL dialect-qualified name, reached through `row_dialect`
// (dataflowir-gen/src/lib.rs:154).  So the prefix IS derivable, the premise is
// gone, and the same string is what `<<` must write.
//
// ⛔ WHAT `raw_ostream << OperationName` PRINTS, and the standard used.
// OperationSupport.h:507 is `{ info.print(os); return os; }`, and
// `OperationName::print` (OperationSupport.h:478) is OUT-OF-LINE -- this toolchain
// ships MLIR HEADERS ONLY (no libMLIR .so or .a under
// toolchain/llvm/LLVM-22.1.3-Linux-X64/lib), so its body cannot be read or
// disassembled here.  ⭐ The fidelity argument is therefore the SAME ONE f126-f128
// were landed on, and it is an argument from the MLIR ASSEMBLY GRAMMAR rather than
// from a .cpp: an operation name's one textual spelling is `dialect.mnemonic`, and
// `getStringRef()` (OperationSupport.h:473 -> `getIdentifier()` :476 -> the
// registry's StringAttr) IS that spelling.  There is no second candidate text.
// ⚠️ THE HONEST RESIDUAL, stated so nobody has to re-derive it: `print` could in
// principle decorate an UNREGISTERED name.  It cannot matter here -- an
// unregistered name has no TdOpDef row, t18 models that as `None`, and f2100
// already panics on `None` rather than inventing text.  f600 panics identically
// and for the identity of reason, so the two keys cannot disagree.
//
// ⛔⛔ THE THREE THAT STAY REFUSED.  rules/mlir:4069-4084 already refuses them;
// this re-states the reasons ONLY because a reader who finds f600 here will ask
// why its siblings are absent, and a missing answer reads as an oversight.
//   * `mlir::Value`, 20 TU / 80 sites, THE BIGGEST SINGLE ROW IN THIS FAMILY AND
//     THE ONE THAT MUST STAY LOUD.  Value.h:246 is `{ value.print(os); return
//     os; }` and `Value::print` prints THE DEFINING OPERATION'S FULL FORM, not the
//     SSA name.  `dataflowir_gen::ir::Value` is `{ name: String, ty: Ty }` and its
//     own doc-comment (ir.rs:17-19) says why: "In real MLIR a Value is a pointer
//     into a live Operation.  For EMITTING IR none of that is needed."  There is NO
//     back-pointer to a defining op, so the information the C++ prints is
//     STRUCTURALLY ABSENT from the model, not merely unimplemented.
//     ⭐ dataflowir-gen ALREADY RECORDS THIS REFUSAL ITSELF, at asm.rs above
//     `print_operand`: "`llvm::raw_ostream << Value` is a DIFFERENT function and
//     prints the DEFINING OPERATION's full form (Value.h:246) ... NOT expressible
//     here and is deliberately absent ... a method claiming to be it could only
//     print the name -- a plausible wrong answer.  A raw-stream key for `Value`
//     therefore fails to resolve, which is the intended direction."  ⛔ SO THERE IS
//     NOTHING TO SPECIFY IN dataflowir-gen EITHER: the crate has already adjudicated
//     it, on the same evidence, against adding a printer.  Widening `ir::Value` to
//     carry a defining-op handle is a MODEL change (it makes an SSA handle own the
//     op graph) and is out of scope for a rules slot.
//     ⚠️ NOTE THE OPPOSITE ANSWER ON A PRINTER RECEIVER: rules/mlir f363
//     (`OpAsmPrinter << Value`) IS landed and IS correct, because
//     OpImplementation.h's body there is `p.printOperand(value)` -- the SSA name.
//     Same operator token, same argument type, two different right answers.
//   * `const mlir::Operation &` (24 TU / 58 site, Operation.h:1100) and
//     `mlir::OpState` (17 TU / 56 site, OpDefinition.h:315).  ⭐⭐ THESE TWO ARE NO
//     LONGER REFUSED -- f3063 / f3064 below, slot g3063, 2026-09-29.  THE REFUSAL
//     TEXT IS KEPT because its facts are all still true and a reader must see what
//     was weighed; only its CONCLUSION is reversed, and by a measurement it did
//     not have.  It read:
//       "Both are `op.print(os, OpPrintingFlags().useLocalScope())`.  t1/t25 both
//        model these as `fmt::OpInst`, and `OpInst::print()` (fmt.rs:1454) DOES
//        exist -- so unlike `Value` this is not structurally absent, and
//        rules/mlir's 'has no Display' wording understates what is actually in the
//        way.  ⭐ THE REAL BLOCKER, measured: `print()` returns
//        `Result<String, PrintError>` and returns
//        `Err(PrintError::CustomAssembly(mnemonic))` for every op whose `.td` sets
//        `custom_asm` (fmt.rs:1470).  MLIR prints such an op fine -- it calls the
//        hand-written C++ printer.  So a body would have to choose, on the Err arm,
//        between a panic (turning a debug dump into a crash at RUNTIME, long past
//        any translate-time signal) and a marker string (silent wrongness in
//        exactly the diagnostics a compiler is debugged with).  Neither is
//        admissible, so the key stays out and the site stays loud."
//     ⭐ WHAT THAT ARGUMENT IS MISSING, AND IT IS MEASURABLE FROM THE CALL SITES:
//     EVERY site is inside a runtime-guarded debug path, so the Err arm is not on
//     any ordinary execution path at all.  All seven first-abort sites of these two
//     spellings in the 218-TU gate corpus were read:
//       Conversion/AgenToSentient/AgenToSentient.cpp:50                LLVM_DEBUG(
//       Transform/Sentient/AddressRegisterPrecisionAssignment.cpp:149   LLVM_DEBUG(
//       Transform/Sentient/ReadOnlyRegisterRenumbering.cpp:149-153      LLVM_DEBUG({
//       Transform/Sentient/ScalarOpMergingAndHoisting.cpp:245 et al     LLVM_DEBUG(
//       Transform/Sentient/SinkScalarCopy.cpp:145                       `dump(raw_ostream&,int)`,
//                                                whose ONLY caller is :209 LLVM_DEBUG({
//       Transform/Dataflow/UniformQueryMapsCanonicalization.cpp:70,84   LLVM_DEBUG(
//       Transform/Sentient/ReuseLoopIteratorArguments.cpp:189-191,272   LLVM_DEBUG({
//     `LLVM_DEBUG(X)` is `DEBUG_WITH_TYPE(DEBUG_TYPE, X)`, i.e. (Debug.h:70-77)
//         do { if (::llvm::DebugFlag && ::llvm::isCurrentDebugType(TYPE)) { X; } }
//         while (false)
//     -- X is COMPILED (which is why the converter sees these sites and aborts on
//     them even though this project's release build defines NDEBUG only for the
//     C++ compiler's own asserts) but is EXECUTED only when the ported program is
//     run with `-debug` / `-debug-only=<type>`.  The sibling macro is the same
//     shape: `LDBG(...)` (DebugLog.h:106-115) is a `for` loop whose condition is
//     `::llvm::DebugFlag && ldbgIsCurrentDebugType(...)`, and under NDEBUG
//     (DebugLog.h:336-338) it degrades to `for (bool _c = false; _c; _c = false)
//     ::llvm::nulls()` -- args still compiled, never evaluated.
//     So the choice is NOT "crash a working compiler vs. print a wrong dump".  It
//     is "a named panic in a dump the user explicitly asked for, vs. a whole TU
//     that does not translate at all" -- and a TU that does not translate cannot be
//     part of a built program, which is strictly worse for every caller.
//     ⭐ AND THE PANIC ARM IS THIS MODULE'S AND rules/mlir's ESTABLISHED PRACTICE,
//     on arms that are MORE reachable than this one, not less:
//       * f600 just above panics on `None` (unregistered op name) rather than
//         inventing text.
//       * rules/mlir f2102/f2103 (`mlir::operator==/!=(OpState, OpState)`,
//         tgt_unsafe.rs:4365) panic when "both handles carry OpId::NONE".  Op
//         EQUALITY is not a debug-only operation, so that landed panic sits on an
//         ordinary path.  If it is admissible there it is admissible a fortiori
//         here.
//     ⛔ THE HONEST RESIDUAL, and it is the one the old text names correctly: on an
//     op whose `.td` sets `custom_asm` and for which `custom::print_custom` has no
//     transliteration, C++ prints and the Rust panics.  That fraction is NOT small
//     -- `dataflowir-gen/tests/asm.rs` prints 244 rows byte-identically and refuses
//     154 with the identical `PrintError`.  The panic names the mnemonic (PrintError
//     is Display, fmt.rs:841) so the failure is attributable to the exact missing
//     row.  "Make `print` total" REMAINS A REAL dataflowir-gen ROW; these keys do
//     not close it and do not hide it -- they move it from translate-time-total to
//     debug-time-partial.
//     ⚠️ FIDELITY OF THE Ok ARM: `OpInst::print()` is `print_in(&PrintCtx::top())`
//     (fmt.rs:1454-1456) and `PrintCtx::top()` is `{ indent: 0, default_dialect:
//     "builtin" }` (fmt.rs:379-381) -- one op, standalone, at column 0, with no
//     alias state and no enclosing-module context.  That is exactly what
//     `OpPrintingFlags().useLocalScope()` asks MLIR for, and it is why `print()`
//     rather than `print_in` is the right call here.
// ---------------------------------------------------------------------------

namespace mlir {
// ⛔ RESTATED EMPTY AND WITH NO `using` OF THEIR OWN, so this module records NO
// type key for any of them.  rules/mlir owns all three (t18 mlir::OperationName,
// t83 mlir::AffineExpr, t8 mlir::AffineMap) and `types_` is tree-wide.  The
// in-tree precedent for an inert restatement that maps nothing is rules/mlir's
// own `class AsmPrinter {};` / `class StringRef {}` (src.cpp:4270, :1242), and the
// precedent for the mirror case -- an incomplete type owned by another module --
// is rules/mlir's `class raw_ostream;` at :4093, which points back at t1/t2 here.
class OperationName {};
class AffineExpr {};
class AffineMap {};
// ⛔ SAME RULE FOR THESE TWO, ADDED BY g3063: NO `using` OF THEIR OWN.  rules/mlir
// owns them (t1 mlir::Operation, t25 mlir::OpState) and both already map to
// `dataflowir_gen::fmt::OpInst`; f3063/f3064 only need the SPELLINGS to record the
// right key.  `Operation` is reached by reference so an empty class is enough;
// `OpState` is taken BY VALUE (OpDefinition.h:315 is `operator<<(raw_ostream &,
// OpState op)`) and an empty class is complete, which is all by-value needs.
// ⚠️ DO NOT give either of these a `using tN`: a second type rule for one spelling
// in two modules is load-order-dependent, the hazard the OperationName/AffineExpr/
// AffineMap note above was written for.
class Operation {};
class OpState {};
} // namespace mlir

// ⭐ DECLARED AT GLOBAL SCOPE, UNQUALIFIED, exactly as rules/mlir:4096-4098 does.
// A free two-parameter operator records UNQUALIFIED in this tree even though the
// real functions live in namespace mlir (OperationSupport.h:507, AffineExpr.h:276,
// AffineMap.h:664); the baseline precedent is rules/cstddef:10's `std::byte
// operator shr(std::byte, unsigned int)`.
// ⚠️ AND THE KEY SAYS `operator shl`, NOT `operator<<`: Mapper::ToString
// (mapper.cpp:2330-2360) rewrites all four shift spellings so matchTemplate's
// bracket-depth tracker never sees the `>`.  Read the recorded key out of
// ir_src.json, never off this line.
llvm::raw_ostream &operator<<(llvm::raw_ostream &os, mlir::OperationName n);
llvm::raw_ostream &operator<<(llvm::raw_ostream &os, mlir::AffineExpr e);
llvm::raw_ostream &operator<<(llvm::raw_ostream &os, mlir::AffineMap m);
// g3063.  Operation.h:1100 is `operator<<(raw_ostream &os, const Operation &op)`
// and OpDefinition.h:315 is `operator<<(raw_ostream &os, OpState op)` -- reference
// and by-value respectively, and the recorded keys differ accordingly, so the two
// spellings are written exactly as MLIR declares them.
llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const mlir::Operation &op);
llvm::raw_ostream &operator<<(llvm::raw_ostream &os, mlir::OpState op);
// g3063.  raw_ostream.h:846 is a TEMPLATE inside namespace llvm; see the g822 entry
// above for why it records unqualified and why the extent is written CONCRETELY as
// `long` rather than as a template parameter.
llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                              const std::optional<long> &o);

// ⚠️ INDICES f600-f602, not the next free f19, for the reason t560/f560/f561
// above give: several slots are live in the rule tree today and f19 is what a
// concurrent slot would also pick.  Indices are per-module and need not be dense.

// f600 -- `mlir::OperationName` BY VALUE (OperationSupport.h:507 takes
// `OperationName info`).  10 TUs, 18 sites.
llvm::raw_ostream &f600(llvm::raw_ostream &a0, mlir::OperationName a1) {
  return operator<<(a0, a1);
}

// f601 -- `mlir::AffineExpr` BY VALUE (AffineExpr.h:276).  1 TU, 3 sites.
// The rendered text is `ir::AffineExpr`'s Display (ir.rs:359), whose
// `print_affine` helper carries MLIR's affine-expression precedence and its
// "addition of a negative constant prints as subtraction" special case (ir.rs:341)
// -- i.e. it was written against the printed grammar, not invented.
llvm::raw_ostream &f601(llvm::raw_ostream &a0, mlir::AffineExpr a1) {
  return operator<<(a0, a1);
}

// f602 -- `mlir::AffineMap` BY VALUE.  1 TU, 1 site.  AffineMap.h:664 is inline
// and visible: `{ map.print(os); return os; }`.  The rendered text is
// `ir::AffineMap`'s Display (ir.rs:402), which emits
// `affine_map<(d0, d1)[s0] -> (...)>` -- MLIR's affine-map attribute syntax,
// dim/symbol list included.  ⚠️ f601/f602 rest on the same grammar argument f126
// does, and for the same unavoidable reason: `AffineExpr::print` (AffineExpr.h:89)
// and `AffineMap::print` (AffineMap.h:200) are both out-of-line and this toolchain
// has no MLIR library to read them from.
llvm::raw_ostream &f602(llvm::raw_ostream &a0, mlir::AffineMap a1) {
  return operator<<(a0, a1);
}

// ---------------------------------------------------------------------------
// PASS 2026-09-29, slot g3063: THE THREE KEYS THAT CLOSE THE `<<` GATE FAMILY
// THAT WAS TIED FOR #1 IN THE 218-TU GATE CORPUS.  f3063 / f3064 / f3065.
//
// The gate, verbatim from converter.cpp:7467, is
//     unsupported CXXOperatorCallExpr: << on (llvm::raw_ostream, X)
// and the four X that lead a TU's abort list aggregate to ONE 13-TU family:
//     const mlir::Operation &      5 TU first-abort   f3063  <-- keyed
//     mlir::Value                  5 TU first-abort   NO KEY, deliberately
//     mlir::OpState                2 TU first-abort   f3064  <-- keyed
//     const std::optional<long> &  1 TU first-abort   f3065  <-- keyed
// (First-abort TUs, not site presence; the survey-v10 site census above counts the
// same spellings over 403 TUs and is the larger number.  Both are reported.)
//
// ⚠️ INDICES f3063-f3065, not f603: five slots were live in the rule tree on
// 2026-09-29 and a dense next index is what a concurrent slot also picks.  Indices
// are per-module and need not be dense -- the t560/f560, f600 precedents above.
// The indices are the slot id so a later reader can find the row.
//
// f3063 -- `const mlir::Operation &`.  Operation.h:1100:
//     inline raw_ostream &operator<<(raw_ostream &os, const Operation &op) {
//       const_cast<Operation &>(op).print(os, OpPrintingFlags().useLocalScope());
//       return os;
//     }
// rules/mlir t1 models `mlir::Operation` as `dataflowir_gen::fmt::OpInst`; the
// rendered text is `OpInst::print()`.  See the long note above this block for the
// reversed refusal, the LLVM_DEBUG guard measurement that reverses it, and the
// honest residual on the `Err(CustomAssembly)` arm.
llvm::raw_ostream &f3063(llvm::raw_ostream &a0, const mlir::Operation &a1) {
  return operator<<(a0, a1);
}

// f3064 -- `mlir::OpState` BY VALUE.  OpDefinition.h:315:
//     inline raw_ostream &operator<<(raw_ostream &os, OpState op) {
//       op.print(os, OpPrintingFlags().useLocalScope());
//       return os;
//     }
// ⭐ THE SAME PRINTED TEXT AS f3063 AND THAT IS CORRECT, NOT A COPY-PASTE: an
// OpState IS a `Operation *` wrapper (OpDefinition.h:110 `Operation *state;`) and
// its `print` forwards to the operation's.  rules/mlir t25 maps it to the same
// `fmt::OpInst`, and rules/mlir f2102 is the in-tree precedent for spelling an
// `mlir::OpState` parameter BY VALUE in an f-key.
// ⚠️ IT IS STILL A SEPARATE KEY: `const mlir::Operation &` and `mlir::OpState` are
// different recorded strings and the mapper matches by string, so f3063 can never
// serve an OpState site -- it would be a dead half of the pair, exactly as
// rules/mlir f2103's note records for `!=` versus `==`.
llvm::raw_ostream &f3064(llvm::raw_ostream &a0, mlir::OpState a1) {
  return operator<<(a0, a1);
}

// f3065 -- `const std::optional<long> &`, THE ROW RECORDED ABOVE AS
// `blocked:refused-unknown-nullopt-rendering` (g822).  ⭐ THE BLOCK IS LIFTED BY
// THE EXACT PROBE THAT ENTRY ASKED FOR; see the corrected g822 text above for the
// program, the link line and the output.  The disengaged rendering is the literal
// four characters `None`.
// raw_ostream.h:842-852:
//     LLVM_ABI raw_ostream &operator<<(raw_ostream &OS, std::nullopt_t);
//     template <typename T, typename = decltype(std::declval<raw_ostream &>()
//                                               << std::declval<const T &>())>
//     raw_ostream &operator<<(raw_ostream &OS, const std::optional<T> &O) {
//       if (O) OS << *O; else OS << std::nullopt;
//       return OS;
//     }
// The engaged branch is f13 (`operator<<(long)`) verbatim.
// ⚠️ WRITTEN CONCRETELY AT `long`, NOT AS A TEMPLATE, ON PURPOSE.  A templated
// `const std::optional<T1> &` key would record one string for every element type
// and would therefore also claim `std::optional<SomeUnkeyedStruct>`, whose Rust
// body could only be `format!("{}", v)` behind a `T1: Display` bound -- i.e. it
// would turn a translate-time abort into a rustc trait error for every element type
// this corpus has not measured.  `long` is the ONLY element type the 218-TU corpus
// asks for; every other instantiation stays a loud translate-time abort, which is
// where an unmeasured rendering belongs.
llvm::raw_ostream &f3065(llvm::raw_ostream &a0, const std::optional<long> &a1) {
  return operator<<(a0, a1);
}
