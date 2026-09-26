// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::Twine -- LLVM's lightweight string-concatenation rope.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/ADT/Twine.h>
// -----------------------------------------------------------------------
// Same reason as rules/raw_ostream: cpp-rule-preprocessor compiles this file
// with a fixed flag set plus whatever a `cxxflags` file next to src.cpp adds,
// and the only flags that would reach LLVM's headers are absolute -I paths
// into whatever LLVM tree the target project happens to have built.  That
// would make `ninja` fail for anyone without that tree.  So the signatures
// LLVM declares are restated here.
//
// A rule matches on a SIGNATURE STRING, so a restatement matches iff it agrees
// with LLVM exactly.  CHECKED: running cpp-rule-preprocessor over a copy of
// this file that `#include <llvm/ADT/Twine.h>`s instead (with
// -I<llvm>/include -I<llvm-build>/include -std=c++17) produces a byte-
// identical ir_src.json -- all five entries, t1 included.  If LLVM ever
// changes one of these signatures the corresponding rule silently stops
// matching, so re-run that check when upgrading LLVM.
//
// MODEL
// -----
// A Twine is the NUL-terminated byte string it denotes, i.e. exactly the
// representation rules/string gives std::string (Vec<u8> in the refcount
// model, Vec<libc::c_char> in the unsafe one).  Concatenation materialises
// eagerly instead of building a rope.
//
// Why that is faithful rather than merely convenient: Twine exists only as a
// temporary.  LLVM's own documentation forbids storing one -- a Twine holds
// BORROWED pointers into the operands of the expression that built it, so it
// is valid only until the end of the full-expression, and the only supported
// thing to do with one is hand it to an API that consumes it immediately or
// call .str()/.toStringRef() to materialise it.  A model that materialises at
// every step therefore cannot be observed to differ from the rope: there is no
// legal program that can look at an unmaterialised Twine.  It differs only in
// cost, and the operand-lifetime bug class the rope invites (a dangling Twine
// outliving its operands) is gone by construction.
//
// operator+ is a FREE function in namespace llvm, not a member, so its rule
// (f4) is keyed on `llvm::Twine llvm::operator+(const llvm::Twine &, const
// llvm::Twine &)`.  The implicit converting constructors f1 and f2 are what
// let `llvm::Twine("a") + ".mlir"` and `... + std::to_string(i)` resolve to
// that one overload: the `const char *` and `const std::string &` operands
// each become a Twine first.  Both were observed as real
// CXXConstructExpr conversions at the call sites in the target codebase, not
// assumed.
//
// THE StringRef OPERANDS -- ADDED once rules/stringref landed (31a1cdc)
// ---------------------------------------------------------------------
// This comment block previously listed `Twine(StringRef)` and the
// `operator+(StringRef, const char *)` fast path among the NOT COVERED cases,
// on the stated grounds that each "would need a type rule for its own argument
// type first".  That was exactly right, and rules/stringref now supplies it:
// llvm::StringRef is modelled as the bytes it denotes, which is the SAME
// representation this module gives a Twine.  So the two rules are f5 and f6
// below and their bodies are the identity and a concatenation respectively --
// no conversion at the boundary, because there is no boundary.
//
// WHY BOTH, when the gap is only the operator.  The survey records one site,
// dbo/src/Transforms/Autopilot.cpp:167 --
//
//     (kAutopilotName + "_" + std::to_string(index)).str()
//
// where `kAutopilotName` is a `const llvm::StringRef` (Autopilot.cpp:42).
// That is TWO operator+ calls, and they resolve to DIFFERENT overloads: the
// inner `StringRef + "_"` picks LLVM's fast path
// `Twine operator+(StringRef, const char *)` (Twine.h:540) because a StringRef
// operand makes it a better match than converting to Twine, while the outer
// `Twine + std::string` picks the general `operator+(const Twine &, const
// Twine &)` that f4 already covers, with f2 converting the std::string.  So
// f5 is what the gap needs.
//
// f6 (`Twine(StringRef)`, Twine.h:288) is NOT reachable from that site -- the
// fast path takes its StringRef directly and constructs no intermediate Twine.
// It is included because it is the conversion that makes a StringRef usable in
// EVERY other Twine position (`Twine + StringRef`, an API taking `const Twine &`
// handed a StringRef), it is one line in each model, and leaving it out would
// make a StringRef work on the left of `+ "literal"` and abort anywhere else --
// a boundary with no principle behind it.  It is verified by the probe on its
// own, not assumed to be exercised by the operator.
//
// Note f5 takes its left operand BY VALUE (`llvm::StringRef`, not `const
// llvm::StringRef &`), matching LLVM, and that is what the resolved signature
// says: `llvm::Twine llvm::operator+(llvm::StringRef, const char *)`.
//
// THE MIRROR-IMAGE FAST PATH -- f7, added for SentientToTrace.cpp
// --------------------------------------------------------------
// The NOT COVERED list below used to end with `operator+(const char *,
// StringRef)` "which has zero sites".  It has sites now.
// dcc/src/Conversion/SentientToTrace/SentientToTrace.cpp reaches it four
// times over, always with the literal on the LEFT and an MLIR name on the
// right:
//
//     annotate(op_a, "attr `" + op_b_attr.getName().strref() + "` mismatch");
//                                                          // :822, :823
//     annotate(op_b, "missing attr `" + I_a->getName().strref() + "`");
//                                                          // :829, :830
//     setTraceFail(op_a, "expected op type " + op_b.getName().getStringRef());
//                                                          // :844, :845
//     setTraceFail(op_a, "mismatch on operand `" + operandName + "`");
//                                                          // :873, :874
//
// `strref()`, `getStringRef()` and the lambda parameter `operandName` (:866)
// are each an `llvm::StringRef`, so a StringRef operand makes Twine.h:533
// `Twine operator+(const char *, StringRef)` a better match than converting
// both sides to Twine, exactly as it does for f5 in the other order -- which
// is why f4 plus f1 does NOT answer these and the translation aborted on the
// resolved signature `llvm::Twine llvm::operator+(const char *,
// llvm::StringRef)`.
//
// Three of the four sites then feed the result to a further `+ "literal"`,
// which is `Twine + const char *` and resolves to f4 with f1 converting the
// right operand -- already covered, and the reason the probe exercises a
// three-term chain: getting f7 right in isolation but leaving a stale
// terminator in the middle would only show up once something is appended
// after it.
//
// ORDER IS THE WHOLE CONTENT of this rule -- f5 and f7 have byte-identical
// argument SETS and differ only in which side the literal is on, so a body
// that concatenated its operands the wrong way round would still typecheck
// and still produce the right LENGTH.  The probe pins it with operands that
// are not each other's reverse.
//
// NOT COVERED, deliberately -- nothing in the target scope reaches them, and
// each would need a type rule for its own argument type first:
//   Twine(const StringLiteral &),
//   Twine(const SmallVectorImpl<char> &), Twine(const formatv_object_base &),
//   Twine(const std::string_view &), the numeric utostr/itostr helpers,
//   toStringRef/toVector/toNullTerminatedStringRef, isTriviallyEmpty,
//   print/dump, the `Twine(StringRef, const char *)` two-operand CONSTRUCTOR
//   (LLVM's operator+ fast path is implemented in terms of it, but a rule for
//   the operator answers the call site directly, so the constructor is never
//   reached through a translated program).
//   Twine(std::nullptr_t) is `= delete` in LLVM and so can never be called.
//
// The default constructor is NOT covered either.  LLVM's is
// `/*implicit*/ Twine()` producing the null Twine, and it is only reachable
// through APIs that take `const Twine & = Twine()` as a defaulted argument.
// A rule for it would have to agree with how the converter materialises a
// defaulted argument (see the `Default::default()` trap), and no call site in
// scope needs it, so it is left out rather than guessed at.

#include <string>

namespace llvm {

// Restated from llvm/ADT/StringRef.h, for the same reason and in the same words
// as rules/stringref: the only flags that would reach LLVM's real headers are
// absolute -I paths into whatever LLVM tree the target project happens to have
// built.  This restatement must agree with rules/stringref's, because a rule
// matches on a signature STRING and the two modules name the same type -- the
// members below are the subset the two signatures here mention.  Only `Data`
// and `Length` are declared for the same reason rules/stringref gives: the
// layout is irrelevant to matching, but the class must be complete because f5
// takes one by value.
class StringRef {
  const char *Data = nullptr;
  unsigned long Length = 0;

public:
  // llvm/ADT/StringRef.h:83 -- /*implicit*/ StringRef() = default
  StringRef() = default;
  // llvm/ADT/StringRef.h:90 -- /*implicit*/ constexpr StringRef(const char *)
  StringRef(const char *Str);
  // llvm/ADT/StringRef.h:101 -- /*implicit*/ StringRef(const std::string &)
  StringRef(const std::string &Str);
};

// Restated from llvm/ADT/Twine.h.  The member layout is irrelevant to
// signature matching but the class has to be complete, because f4 returns one
// by value.  LLVM's real members are two `Child` unions and two kind bytes;
// the shape below is only a stand-in of no consequence to any rule.
class Twine {
  const void *LHS;
  const void *RHS;
  unsigned char LHSKind;
  unsigned char RHSKind;

public:
  // llvm/ADT/Twine.h:257 -- /*implicit*/ Twine(const char *Str)
  Twine(const char *Str);
  // llvm/ADT/Twine.h:272 -- /*implicit*/ Twine(const std::string &Str)
  Twine(const std::string &Str);
  // llvm/ADT/Twine.h:288 -- /*implicit*/ Twine(StringRef Str)
  Twine(StringRef Str);
  // llvm/ADT/Twine.h:434 -- LLVM_ABI std::string str() const
  std::string str() const;
};

// llvm/ADT/Twine.h:526 -- inline Twine operator+(const Twine &, const Twine &)
Twine operator+(const Twine &LHS, const Twine &RHS);
// llvm/ADT/Twine.h:540 -- inline Twine operator+(StringRef, const char *)
Twine operator+(StringRef LHS, const char *RHS);
// llvm/ADT/Twine.h:533 -- inline Twine operator+(const char *, StringRef)
Twine operator+(const char *LHS, StringRef RHS);

} // namespace llvm

using t1 = llvm::Twine;

llvm::Twine f1(const char *s) { return llvm::Twine(s); }

llvm::Twine f2(const std::string &s) { return llvm::Twine(s); }

std::string f3(const llvm::Twine &t) { return t.str(); }

llvm::Twine f4(const llvm::Twine &a, const llvm::Twine &b) {
  return llvm::operator+(a, b);
}

llvm::Twine f5(llvm::StringRef a, const char *b) {
  return llvm::operator+(a, b);
}

llvm::Twine f6(llvm::StringRef s) { return llvm::Twine(s); }

llvm::Twine f7(const char *a, llvm::StringRef b) {
  return llvm::operator+(a, b);
}
