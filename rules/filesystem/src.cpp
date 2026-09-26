// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::filesystem::path -- the second-largest operator class in the dt_src port.
//
// MODEL
// -----
// A path is the byte string it denotes, i.e. exactly the representation
// rules/string gives std::string (Vec<libc::c_char> unsafe, Vec<u8> refcount,
// NUL-terminated) and the same one rules/stringref and rules/twine use.  That
// choice is not a convenience: it is what C++'s own model is.  A
// std::filesystem::path IS a string plus lexical operations on it -- it holds a
// single `string_type __pn_` member (libc++ __filesystem/path.h) and every
// member this module covers is a pure function of those bytes.  Nothing here
// touches the filesystem.
//
// Sharing rules/string's representation is what makes the two boundaries in the
// real call sites free: `path(someStdString)` and `.string()` are both the
// IDENTITY, with no conversion and no length fixup.  Every site in scope has
// exactly that shape -- a std::string or a literal goes in, a std::string comes
// back out via .string() -- so the path never escapes as a distinct type.
//
// THE NUL BYTE.  rules/string's Vec carries a trailing 0 that is not part of
// the string (its f2 is `a0.len() - 1`, its f22 is `a0.len() <= 1`).  This
// module shares the representation so it shares the convention, and every body
// below is written to preserve it: the terminator is dropped before appending
// and re-pushed, never counted as a character.  Getting that wrong is an
// off-by-one in the separator logic, which is why the probe checks a join
// against an empty component specifically.
//
// WHY operator/ IS SPELLED AS A FREE FUNCTION
// -------------------------------------------
// libc++ declares it as a hidden friend (__filesystem/path.h:658,
// `friend path operator/(const path&, const path&)`), so the resolved
// signature -- read back out of a real translation with `cpp2rust --verbose`,
// not copied from the header -- is
//
//     std::filesystem::path std::filesystem::operator/(const std::filesystem::path &,
//                                                      const std::filesystem::path &)
//
// which is the one the survey records as the gap at all 15 sites.  Note the
// rule below writes `a / b` rather than `std::filesystem::operator/(a, b)`:
// the latter does not compile, because a hidden friend is findable only by
// argument-dependent lookup and is not a member of namespace std::filesystem.
// The resolved signature is identical either way.
//
// Joining a `const char *` or a std::string to a path is NOT a separate
// signature.  libc++ has `operator/=` templates taking any pathable _Source,
// but the non-assigning `operator/` has exactly one overload, so clang inserts
// a CXXConstructExpr converting the right operand to a path first and then
// calls that one operator.  Confirmed at the real sites: `path(dir) / "debug"`
// (dbo/src/Pipeline/Debug.cpp:127) and `path(dir) / file.str()` (:48) both
// resolve to the signature above with f3/f4 supplying the conversion.  So this
// module needs ONE operator rule, and rules/variant's per-signature
// generated-rule mechanism is not needed here.
//
// THE SEMANTIC TRAP, CONFIRMED BY RUNNING RATHER THAN ASSUMED
// ----------------------------------------------------------
// operator/ is NOT string concatenation with a separator.  Five behaviours
// matter, and all five were measured against clang-built C++ (and separately
// against Rust's std::path::PathBuf::push) rather than reasoned about:
//
//     C++  path("a")   / path("b")   == "a/b"      append with a separator
//     C++  path("a/")  / path("b")   == "a/b"      NO doubled separator
//     C++  path("a")   / path("/b")  == "/b"       ABSOLUTE RHS REPLACES
//     C++  path("a")   / path("")    == "a/"       empty RHS still separates
//     C++  path("")    / path("b")   == "b"        empty LHS does not separate
//
// The absolute-RHS case is the one a plausible wrong body (concatenate with a
// separator) gets wrong, and it is wrong SILENTLY: it would produce "a//b"
// where C++ produces "/b", i.e. a path under the wrong root.  The
// empty-component cases are the ones a "trim both sides and join" body gets
// wrong, in the other direction.
//
// Rust's PathBuf::push agrees with C++ on ALL of these, which is why the
// unsafe and refcount bodies can be a byte-level transcription of push's rule
// rather than an approximation.  Measured, same ten inputs, both languages,
// identical on all ten: relative join, trailing separator on the left,
// absolute right-hand side, empty right, empty left, multi-component operands,
// absolute left, trailing separator on the right, a doubled separator that
// must be PRESERVED ("a//" / "b" == "a//b", not "a/b"), and a "." left
// operand.  The bodies below therefore implement exactly:
//
//     if rhs is absolute (begins with '/')      -> result is rhs
//     else if lhs is empty                      -> result is rhs
//     else if lhs already ends with '/'          -> lhs ++ rhs
//     else                                       -> lhs ++ "/" ++ rhs
//
// The third clause is what preserves "a//" / "b" == "a//b": it appends without
// ADDING a separator and without removing the redundant one either.  C++ does
// not normalise (that is what lexically_normal() is for) and neither does this.
//
// WHY stem()/extension()/filename() ARE MODELLED HERE AND NOT DELEGATED
// --------------------------------------------------------------------
// This is the part where Rust's std::path is NOT the right target, and
// discovering that is the reason these three have their own bodies rather than
// a call to file_stem()/extension()/file_name().  C++ and Rust DISAGREE, and
// measured over twelve inputs they disagree on six:
//
//   input        C++ stem   C++ ext   Rust file_stem  Rust extension
//   "foo.tar.gz" "foo.tar"  ".gz"     "foo.tar"       "gz"     <- ext: no dot
//   ".foo"       ".foo"     ""        ".foo"          None     agree
//   "foo."       "foo"      "."       "foo"           ""       <- ext: no dot
//   "a/b/"       ""         ""        "b"             None     <- DISAGREE
//   ".."         ".."       ""        None            None     <- DISAGREE
//   "."          "."        ""        None            None     <- DISAGREE
//
// Three disagreements are load-bearing and none is cosmetic:
//
//   1. Rust's extension() omits the leading dot; C++'s includes it.  The real
//      site is `stem + "_" + suffix + counter + extension`
//      (dbo/src/Utils/sdsc_bundle/SdscUtils.cpp:36) -- with Rust's spelling
//      that builds "foo_copy1json" instead of "foo_copy1.json", a wrong
//      filename with no diagnostic anywhere.
//   2. Rust IGNORES a trailing separator when finding the file name, so
//      Path::new("a/b/").file_stem() is "b" where C++'s is "".  C++ treats the
//      empty final component as a real (empty) component.
//   3. Rust returns None for "." and ".." (it refuses to call them file names);
//      C++ returns them unchanged from stem() with an empty extension.
//
// So these bodies implement C++'s rule directly on the bytes, which is what the
// playbook means by "C++ semantics are the spec": take everything after the
// last '/' as the file name, then split it at the last '.', except that a file
// name consisting only of dots ("." and "..") is all stem and no extension, and
// a dot in FIRST position does not begin an extension (".gitignore" is stem
// ".gitignore", not stem "" extension ".gitignore").  Verified against
// clang-built C++ on eighteen names including several with multiple dots
// ("foo.tar.gz", "x.y.z.", "a..b", "..a", "..."), several with none ("foo",
// "..", "a/b/"), and the two the dot rules turn on.
//
// filename() is here for the same reason -- Rust's file_name() skips a
// trailing separator -- and because two real sites use it:
// dbo/src/Transforms/sdsc_bundle/GatherIndexConversion.cpp:315 and the
// stem/extension pair at SdscUtils.cpp:33.
//
// THE DEFAULTED `format` ARGUMENT
// ------------------------------
// All three constructors take a second parameter `format fmt = auto_format`, so
// the converter emits a defaulted argument as `None` and the rule parameter must
// be spelled `Option<()>` (the shape rules/map f32 and rules/gtest f7 already
// use).  The body ignores it: `auto_format` is the only value any site passes
// and on POSIX all three formats are the same parse.
//
// AND IT MUST IGNORE IT BY NOT NAMING IT AT ALL.  The playbook's "a rule body
// must not touch a receiver it does not need" applies to ARGUMENTS too, and the
// failure is not a warning.  Writing `let _ = a1;` to silence an unused
// parameter -- which is what these three bodies said first -- makes the
// converter substitute the argument, and a defaulted enum argument substitutes
// as the bare enumerator name `std_filesystem_path_format_auto_format`, which
// nothing declares.  That was 15 E0425s from three one-token statements.  An
// unmentioned parameter is simply not emitted, so the placeholder never appears.
//
// THE LITERAL ARGUMENT NEEDS TWO IMPLS, and this is the one place this module
// departs from rules/stringref f8's recipe rather than following it.  f8 gets
// away with a `*const libc::c_char` parameter because its C++ parameter is
// reached through llvm::StringLiteral's OWN constructor template, where the
// converter materialises the literal as `&std::mem::transmute(*b"..\0")` -- a
// reference to a byte array, which coerces.  Here the literal reaches
// `path(const _Source &, format)` and the converter materialises it as a
// `c"..."` CStr instead, so a pointer-typed parameter emits
// `c"debug" as *const libc::c_char` and rustc rejects the cast (E0606), while a
// slice-typed one emits `c"debug".iter()` and rustc says CStr has no `iter`
// (E0599) -- which is the very error rules/stringref's commit message records.
// BOTH forms occur, in the same TU, depending on how the literal arrives.  So
// f4's parameter is spelled as the slice and the body dispatches on the
// receiver's Rust type with a private trait declared inside the body, which is
// the playbook's tool for exactly this ("one C++ signature reached from several
// Rust representations") and what rules/basic_ios uses for the same reason.  The
// whole body is wrapped in explicit parens, `({ .. })`, because a bare block
// would be a parse error the moment this rule lands in an `if`/`while`
// condition.  The refcount model needs only the slice impl, because there the
// literal always arrives as `b"..."`, but the trait is written identically in
// both models so the two cannot drift.
//
// f4's Rust parameter is spelled as a POINTER (`*const libc::c_char` /
// `Ptr<u8>`) even though the C++ parameter is `const char (&)[N]`.  That is
// rules/stringref f8's spelling and it is not cosmetic: the converter
// materialises a string literal in this position as a `c"..."` CStr and adds
// `.as_ptr()` only when the declared parameter is a pointer.  Declared as a
// slice it emits the bare CStr and rustc reports no method `iter` on
// `&'static CStr`.  cpp2rust prints the extent as `_`, so the single rule
// matches every literal length.
//
// NOT MODELLED, deliberately.  Each is left a loud abort rather than a guessed
// body, and each is named with what it would need:
//
//   * Everything that TOUCHES THE FILESYSTEM: exists, is_regular_file,
//     file_size, create_directory, create_directories, copy, remove_all,
//     temp_directory_path, directory_iterator.  These are not lexical
//     operations on bytes, they are syscalls, and each needs a decision about
//     how a std::error_code out-parameter maps onto io::Result that this module
//     cannot make on its own.  They already translate to undefined
//     `create_directories_N` names (loud at rustc), which is the correct state
//     until that decision is made.  They are NOT the gap this module closes:
//     the survey's 15 `/` records are all operator/.
//   * operator/= .  It is the mutating form and the playbook records it as one
//     of the bridge's four blockers, but a reference-returning mutator needs
//     the returns-unit judgement made per site, and it has zero sites in the
//     six TUs this module unblocks.
//   * parent_path(), replace_filename(), lexically_normal(),
//     replace_extension(), operator==, the iterator interface, and
//     construction from a std::string_view or an iterator pair.
//     replace_filename has exactly one site (GatherIndexConversion.cpp:314)
//     and that TU records ZERO gaps today, so adding it would be unverifiable
//     against any measurement.
//   * root_name/root_directory/relative_path and the is_absolute/is_relative
//     predicates.  Zero sites in dxp/ dcg/ ddc/ dsc/ dbo/ dcc/ util/.
//   * The WIDE-character and u8 spellings (wstring(), u8string(), the wchar_t
//     constructors).  POSIX-only port; zero sites.

#include <filesystem>
#include <string>

using t1 = std::filesystem::path;

std::string f1(const std::filesystem::path &p) { return p.string(); }

// Spelled `a / b` and not `std::filesystem::operator/(a, b)`: libc++ declares
// it as a hidden friend, so it is reachable only by argument-dependent lookup
// and is not a member of the namespace.  The resolved signature is the
// free-function one either way.
std::filesystem::path f2(const std::filesystem::path &a,
                         const std::filesystem::path &b) {
  return a / b;
}

std::filesystem::path f3(const std::string &s) {
  return std::filesystem::path(s);
}

// The RVALUE std::string constructor is a SEPARATE signature, and leaving it
// out is not a missing nicety, it is a hole that shows up as an unresolved
// `std_filesystem_path::std_filesystem_path1` in the generated Rust.  libc++'s
// path has both `path(string_type &&)` and the `_Source` template, so clang
// picks the rvalue overload whenever the operand is a temporary -- which is
// what `path(std::string("root"))` is, and more importantly what
// `path(dir) / "a" / "b"` produces for the inner joins, since operator/ returns
// by value.  The two real sites with that shape are
// dbo/src/Pipeline/Pipeline.cpp:38 (a three-way chain) and
// dbo/src/Transforms/EmitSpyreCode.cpp:99.  Both f3 and f5 are the identity, so
// this costs a line, but the signatures are distinct strings and a rule matches
// on the string.
std::filesystem::path f8(std::string &&s) {
  return std::filesystem::path(static_cast<std::string &&>(s));
}

std::filesystem::path f4(const char (&s)[15]) {
  return std::filesystem::path(s);
}

std::filesystem::path f5(const std::filesystem::path &p) { return p.stem(); }

std::filesystem::path f6(const std::filesystem::path &p) {
  return p.extension();
}

std::filesystem::path f7(const std::filesystem::path &p) {
  return p.filename();
}

// operator/= -- THE MUTATING FORM, and the current blocker of
// progtailor_analyze_execlist_standalone.cpp:43-44 (converter.cpp's
// unsupported-`/=` path).  The earlier note in this file said it had "zero
// sites"; it has three, all in that TU:
//
//     auto senprog_path = build_dir;   // path copy
//     senprog_path /= line;            // std::string rhs
//     senprog_path /= "senprog.txt";   // string-literal rhs
//
// libc++ declares ONE non-template `operator/=(const path&)` plus a
// `template <class _Src> path& operator/=(const _Src&)` (__filesystem/path.h),
// so a std::string rhs and a char-array rhs are SEPARATE instantiated
// signatures from the path rhs, and each needs its own rule.  Keys were read
// back out of ir_src.json, not copied from the header.
//
// THE SEMANTICS ARE [fs.path.append], NOT string concatenation, and the
// difference is silent:
//     path("a")  /= "b"   -> "a/b"     separator inserted
//     path("a/") /= "b"   -> "a/b"     NOT "a//b" -- no doubled separator
//     path("a")  /= "/b"  -> "/b"      ABSOLUTE RHS REPLACES ENTIRELY
//     path("")   /= "b"   -> "b"       empty lhs contributes no separator
//     path("a//")/= "b"   -> "a//b"    an existing redundant separator is KEPT
// A body spelled `a0.push_str(a1)` is wrong on three of those five and
// compiles.  The bodies below are the same four-clause rule operator/ (f2)
// already implements, applied in place, so the two forms cannot drift.

std::filesystem::path &f9(std::filesystem::path &a,
                          const std::filesystem::path &b) {
  return a.operator/=(b);
}

std::filesystem::path &f10(std::filesystem::path &a, const std::string &s) {
  return a.operator/=(s);
}

std::filesystem::path &f11(std::filesystem::path &a, const char (&s)[15]) {
  return a.operator/=(s);
}

// path(const char *) and parent_path() -- MEASURED, not predicted.  With
// operator/= landed the TU translates rc=0, but rc=0 is translation success and
// NOT correctness: the emitted line 37 read
//
//   let build_dir = ({ std_filesystem_path::new_1({ execlistFilename.as_pointer() },
//                                                 None).parent_path() });
//
// i.e. TWO undefined names -- a `const char *` constructor (distinct from f4's
// `const char (&)[N]`: argv[1] is a pointer, not an array) and parent_path().
// Both are loud at rustc, which is the correct state, but neither was modelled.
std::filesystem::path f12(const char *s) { return std::filesystem::path(s); }

// parent_path is LEXICAL -- it touches no filesystem -- and it is NOT "truncate
// at the last separator", which gets two of these five wrong:
//     "a/b"  -> "a"      strips the component AND the separator
//     "a"    -> ""       no separator at all: EMPTY, not "a"
//     "/a"   -> "/"      the root separator is KEPT, so not "" either
//     "a/b/" -> "a/b"    the trailing filename is EMPTY and is what gets stripped
//     "/"    -> "/"      the root is its own parent
// So: cut at the last separator, and if that leaves nothing while the separator
// was in FIRST position, the answer is the root "/" rather than "".
//
// A SIXTH case the probe caught and a simple truncation gets wrong:
//     "a//b" -> "a"      NOT "a/" -- the redundant separators go too
// C++ finds the parent by dropping the last ELEMENT, and the separator run
// between two elements is one separator's worth of nothing, so every trailing
// separator is stripped, down to (but not past) the root.  The first body
// written here printed "a/" where clang printed "a", which is exactly the kind
// of one-byte path difference that would compile and then open the wrong name.
// ("//a" is left alone: POSIX gives "//" an implementation-defined meaning and
// there are zero sites.)
std::filesystem::path f13(const std::filesystem::path &p) {
  return p.parent_path();
}
