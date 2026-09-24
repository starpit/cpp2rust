// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::StringRef -- LLVM's non-owning string view (pointer + length).
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/ADT/StringRef.h>
// ---------------------------------------------------------------------------
// Same reason as rules/raw_ostream and rules/twine: cpp-rule-preprocessor
// compiles this file with a fixed flag set, and the only flags that would
// reach LLVM's headers are absolute -I paths into whatever LLVM tree the
// target project happens to have built.  That would make `ninja` in this repo
// fail for anyone without that tree.  So the signatures LLVM declares are
// restated here instead.
//
// A rule matches on a SIGNATURE STRING, so a restatement matches iff it agrees
// with LLVM exactly.  Each signature below was read back out of a real
// translation of a real TU with `cpp2rust --verbose`, which prints
// `search expr <exact resolved signature>` -- not copied out of the header and
// hoped over.  The ones observed, verbatim:
//
//     bool llvm::operator==(llvm::StringRef, llvm::StringRef)
//     bool llvm::operator!=(llvm::StringRef, llvm::StringRef)
//     std::string llvm::StringRef::str() const
//     bool llvm::StringRef::empty() const
//     unsigned long llvm::StringRef::size() const
//     void llvm::StringRef::StringRef()
//     void llvm::StringRef::StringRef(const char *)
//     void llvm::StringRef::StringRef(const char *, unsigned long)
//     void llvm::StringRef::StringRef(const std::string &)
//     void llvm::StringRef::StringRef(const llvm::StringRef &)
//     void llvm::StringRef::StringRef(llvm::StringRef &&)
//     llvm::StringRef & llvm::StringRef::operator=(const llvm::StringRef &)
//     void llvm::StringLiteral::StringLiteral(const char (&)[_])
//
// Note `unsigned long`, not `usize`: the rule's Rust signature must be spelled
// the way src.cpp spells the C++ one (rules/string f55-f57 paid for that as an
// E0308), so size() returns u64 and the length parameter of the two-argument
// constructor is u64.
//
// MODEL -- and why this cannot be an opaque handle
// -----------------------------------------------
// A StringRef is the byte string it denotes, i.e. the SAME representation
// rules/string gives std::string: Vec<libc::c_char> in the unsafe model,
// Vec<u8> in the refcount model, NUL-terminated.  It is a VALUE, not a view.
//
// Deciding this against the alternative (a `#[repr(transparent)] u64` handle,
// which is what --opaque-namespace=llvm would produce) is not a matter of
// taste, because the target codebase COMPUTES WITH THE CONTENTS:
//
//   * dbo/src/Utils/sdsc_bundle/SdscUtils.cpp:55 is
//     `llvm::DenseSet<StringRef> uniqueFilenames`, deduplicating filenames BY
//     CONTENT.  Under identity-only traits two StringRefs over different
//     buffers holding the same text would not collide, and the uniquifier
//     would silently stop uniquifying -- exactly the silent-wrongness class
//     the playbook ranks above a loud abort.
//   * The same file's :29-:35 calls .str() on two StringRefs and builds a
//     std::filesystem::path out of them.  A handle has no bytes to hand over.
//
// The verification probe is built around that first case on purpose: two
// StringRefs over DIFFERENT buffers holding the same text must compare EQUAL.
//
// WHAT "NON-OWNING VIEW" COSTS, SAID OUT LOUD
// -------------------------------------------
// C++'s StringRef borrows; this model copies.  Three consequences, none of
// which any site in scope can observe, and all of which are recorded here
// rather than discovered later:
//
//   1. A StringRef does NOT alias its buffer.  Mutating the std::string a
//      StringRef was built from is undefined behaviour in C++ (the docs say it
//      is "not in general safe to store a StringRef"), so no correct program
//      can depend on seeing the mutation.  A program that DID would diverge
//      here -- but it would already be UB, and the C++ build is the arbiter.
//   2. A sub-view (rsplit, substr, take_front, drop_front) has to COPY.  Only
//      rsplit is reachable in scope and it is not modelled here (see below).
//
// THE NUL BYTE, and why size() is len()-1
// ---------------------------------------
// rules/string's Vec carries a trailing 0 that is not part of the string --
// its f2 (`size()`) is `a0.len() - 1` and its f22 (`empty()`) is
// `a0.len() <= 1`.  StringRef shares the representation, so it shares the
// convention: this module's size() and empty() are spelled identically.  That
// is what lets a std::string flow into a StringRef parameter (f4) and a
// StringRef into .str() (f1) as the IDENTITY, with no length fixup at the
// boundary -- and getting it wrong in either direction is an off-by-one in
// every comparison, which is why the probe checks size() on a
// non-NUL-terminated slice specifically.
//
// A StringRef CAN hold interior NULs (it has an explicit length) where this
// model cannot.  No site in scope constructs one: the only two-argument
// construction in dt_src is LLVM's own StringLiteral, whose length is
// strlen(literal) by construction.  A site that did would silently truncate,
// so it is named here rather than left implicit.
//
// == AND != ARE FREE FUNCTIONS IN NAMESPACE llvm, not members, and they take
// their operands BY VALUE.  So the resolved signature is
// `bool llvm::operator==(llvm::StringRef, llvm::StringRef)` -- one signature,
// which is the whole reason this module is small.  Comparison against a
// `const char *`, against a std::string and against a StringLiteral are NOT
// separate signatures: clang inserts a CXXConstructExpr converting the operand
// to StringRef FIRST (observed as `ImplicitCastExpr <ConstructorConversion>`
// in the AST dump of all three shapes), and then calls the one operator.  Each
// of those conversions is a constructor that needs its own rule -- f3, f4, f6
// -- but the operator itself needs exactly one, so rules/variant's
// per-signature generated-rule mechanism is NOT needed here.
//
// StringLiteral is a SUBCLASS of StringRef, and its constructor binds the
// literal by reference with the extent in the type.  cpp2rust prints the
// extent as `_`, so the single rule f8 matches every literal length -- the
// same mechanism rules/gtest f15 and rules/pair f18 already rely on, and the
// reason there is no per-length family here.
//
// StringLiteral DOES need a type rule of its own (t2), which is not what I
// first assumed -- I assumed every value of it would reach a rule through the
// StringRef slot its base class occupies.  Running the probe disproved that:
// `constexpr llvm::StringLiteral k = "spyrecode.json"` is a VARIABLE whose
// DECLARED type is StringLiteral, and the converter emits a declared type
// verbatim, so without t2 the output is `let k: llvm_StringLiteral = ...` and
// rustc says `cannot find type llvm_StringLiteral`.  t2 maps it to the same
// Rust type as t1, which is sound precisely because it IS a StringRef with a
// narrower constructor -- and `addRulesFromDirectory` accepts two C++ types
// mapping to one identical Rust type (that is the rules/vector iterator
// idempotence the playbook records), so this is not a duplicate-rule abort.
//
// f8's Rust parameter is spelled `*const libc::c_char` / `Ptr<u8>`, NOT
// `&'static [u8]`, even though the C++ parameter is `const char (&)[N]`.  That
// is rules/gtest f15's spelling and it is not cosmetic: the converter
// materialises a string literal in this position as a `c"..."` CStr and adds
// `.as_ptr()` only when the declared parameter is a pointer.  Declared as a
// slice it emits the bare CStr and rustc says `no method named iter found for
// reference &'static CStr` -- which is what the probe reported before this was
// fixed.  Walking to the NUL gives the same length LLVM's `N - 1` does for
// every literal without an interior NUL, and StringLiteral::withInnerNUL (the
// only way to get one) has zero sites in dt_src.
//
// NOT MODELLED, deliberately.  Each is a loud abort rather than a guessed body:
//
//   * rsplit/split -- `dbo/.../ExtractPrograms.cpp:52` is
//     `filename.rsplit('.').first`.  Its resolved signature is
//     `std::pair<llvm::StringRef, llvm::StringRef> llvm::StringRef::rsplit(char)
//     const`, i.e. it needs the pair-of-StringRefs return, and LLVM's rsplit
//     has a subtlety a one-line body would get wrong: with no separator
//     present it returns (whole, EMPTY) -- which is why the call site's very
//     next line is `if (stem.empty()) stem = filename;`.  Worth doing, but it
//     is a second decision (pair construction in both models) and one site, so
//     it stays loud.
//   * .data().  This one is NOT an "and nothing reaches it" omission, it is a
//     refusal: it resolves fine (`const char * llvm::StringRef::data() const`)
//     and the unsafe body would be rules/string f11's `a0.as_ptr()`, but the
//     REFCOUNT model cannot express it.  A Ptr<u8> has to point at something
//     that outlives the call, and the receiver arrives BY VALUE as a Vec<u8>
//     that is dropped at the end of the body -- so any Ptr made from it
//     dangles, which is a silent read of freed memory rather than a loud
//     failure.  rules/string gets away with f11 because its receiver is a
//     std::string the caller still owns; a StringRef's is a temporary of this
//     model's own making.  Confirmed there is nothing to check a body against:
//     ZERO .data() sites on a StringRef across dxp/ dcg/ ddc/ dsc/ dbo/ dcc/
//     util/ dsc-based-utils/.  A rule correct in one model and dangling in the
//     other is worse than an abort, so both models leave it loud.
//   * front/back/operator[], starts_with/ends_with, find/rfind/count,
//     substr/take_front/drop_front, lower/upper, compare/compare_insensitive,
//     getAsInteger, begin/end/bytes, the std::string_view constructor, and
//     `operator+=(std::string &, StringRef)`.  Zero sites in
//     dcg/ ddc/ dsc/ dbo/ dsc-based-utils/ reach any of them.
//   * The ORDERING operators < <= > >=.  Zero sites, and each would need the
//     lexicographic comparison to agree with LLVM's `compare` on the NUL byte
//     this representation carries and C++'s does not -- a real question with no
//     site to check it against, so it is not guessed at.
//   * StringRef(std::nullptr_t) is `= delete` in LLVM and can never be called.

#include <string>

namespace llvm {

// Restated from llvm/ADT/StringRef.h.  The member layout is irrelevant to
// signature matching, but it has to be exactly this pair because
// StringLiteral's constructor below delegates to a (const char *, size_t) one,
// and the class has to be complete since f1 and the constructors take/return
// one by value.
class StringRef {
  const char *Data = nullptr;
  unsigned long Length = 0;

public:
  // llvm/ADT/StringRef.h:83 -- /*implicit*/ StringRef() = default
  StringRef() = default;
  // llvm/ADT/StringRef.h:90 -- /*implicit*/ constexpr StringRef(const char *)
  StringRef(const char *Str);
  // llvm/ADT/StringRef.h:96 -- constexpr StringRef(const char *, size_t)
  StringRef(const char *data, unsigned long length);
  // llvm/ADT/StringRef.h:101 -- /*implicit*/ StringRef(const std::string &)
  StringRef(const std::string &Str);

  // llvm/ADT/StringRef.h:143 -- [[nodiscard]] constexpr bool empty() const
  bool empty() const;
  // llvm/ADT/StringRef.h:146 -- [[nodiscard]] constexpr size_t size() const
  unsigned long size() const;
  // llvm/ADT/StringRef.h:568 -- [[nodiscard]] std::string str() const
  std::string str() const;
};

// llvm/ADT/StringRef.h:860 -- class StringLiteral : public StringRef
class StringLiteral : public StringRef {
public:
  // llvm/ADT/StringRef.h:868 -- template <size_t N> constexpr
  // StringLiteral(const char (&Str)[N]) : StringRef(Str, N - 1)
  template <unsigned long N> StringLiteral(const char (&Str)[N]);
};

// llvm/ADT/StringRef.h:892 -- inline bool operator==(StringRef, StringRef)
bool operator==(StringRef LHS, StringRef RHS);
// llvm/ADT/StringRef.h:900 -- inline bool operator!=(StringRef, StringRef)
bool operator!=(StringRef LHS, StringRef RHS);

} // namespace llvm

using t1 = llvm::StringRef;
using t2 = llvm::StringLiteral;

std::string f1(llvm::StringRef s) { return s.str(); }

bool f2(llvm::StringRef a, llvm::StringRef b) { return llvm::operator==(a, b); }

llvm::StringRef f3(const char *s) { return llvm::StringRef(s); }

llvm::StringRef f4(const std::string &s) { return llvm::StringRef(s); }

bool f5(llvm::StringRef s) { return s.empty(); }

llvm::StringRef f6(const char *s, unsigned long n) {
  return llvm::StringRef(s, n);
}

unsigned long f7(llvm::StringRef s) { return s.size(); }

llvm::StringLiteral f8(const char (&s)[15]) { return llvm::StringLiteral(s); }

llvm::StringRef f9() { return llvm::StringRef(); }

llvm::StringRef f10(const llvm::StringRef &s) { return llvm::StringRef(s); }

llvm::StringRef f11(llvm::StringRef &&s) {
  return llvm::StringRef(static_cast<llvm::StringRef &&>(s));
}

// The assignment operator.  It RETURNS UNIT, not the reference: the playbook
// allows that only where the result is always discarded, and it is -- the two
// StringRef assignments in scope are `stem = filename`
// (dbo/.../ExtractPrograms.cpp:53) and `precision = current_unit_precision`
// (dsc-based-utils/Stitcher/ModuleStitcher.cpp:656), both plain statements.
// This is the same shape rules/string's f29 uses for std::string's.
llvm::StringRef &f12(llvm::StringRef &d, const llvm::StringRef &s) {
  return d.operator=(s);
}

bool f13(llvm::StringRef a, llvm::StringRef b) { return llvm::operator!=(a, b); }
