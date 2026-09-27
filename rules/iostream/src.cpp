// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <iostream>

using t1 = std::ostream;
using t2 = std::ostream &;
using t3 = std::ostream *;

std::ostream &f1() { return std::cout; }

std::ostream &f2() { return std::cerr; }

std::ostream *f3() { return &std::cout; }

std::ostream *f4() { return &std::cerr; }

// f5/f6 -- `std::ios_base::in` and `std::ios_base::out`, STATIC DATA MEMBERS of
// std::ios_base (libcxx/ios:287-288, `static const openmode in = 0x08;`,
// `out = 0x10;`, openmode = unsigned int).  MEASURED, not guessed.
//
// WHY THEY ARE HERE AND WHY THEY MATTER FAR BEYOND this module.  The 35-TU
// stringstream row (rules/basic_stringstream f3, the
// `stringstream(const std::string &, unsigned int)` ctor) translated rc=0 and
// then failed to compile in BOTH models with
//     cannot find value `in_0` in this scope
//     cannot find value `out_1` in this scope
// because the DEFAULTED openmode argument `std::ios_base::in |
// std::ios_base::out` is materialised at the CONSTRUCT SITE and lowered by
// Converter::ConvertDeclRefExpr (converter.cpp:3977-3982) through the
// GLOBAL-VARIABLE path -- IsGlobalVar (converter_lib.cpp:80) is
// `isFileVarDecl() || isStaticLocal()` and a class-static data member IS a file
// var decl -- so it emits
//     (*std::cell::LazyCell::force_mut(&mut *&raw mut in_0))
// against a `pub static mut in_0` DECLARATION THAT IS NEVER EMITTED, because the
// declaration is only written when the VarDecl itself is traversed and a
// system-header decl never is.  (Measured against a PORTED static member: a
// `struct K { static const unsigned int A; }` does get
// `pub static mut A_0: std::cell::LazyCell<u32> = ...` plus a force in
// __cpp2rust_init_globals.  So the lowering is complete for project statics and
// dangling for system ones.)
//
// THE FIX IS A RULE, NOT A CONVERTER CHANGE, and the shape is exactly f1/f2's:
// ConvertDeclRefExpr consults ShouldReplaceWithMappedBody/GetMappedAsString
// BEFORE the IsGlobalVar branch, and the -verbose line is
//     search expr std::ios_base::in, result: None
// -- i.e. the search key is the plain qualified name, with no signature and no
// parentheses, the same shape `std::cout` records as (f1).  The rule
// preprocessor's `declref` matcher (cpp_rule_preprocessor.cpp:982-984) binds
// `declRefExpr(to(decl(unless(parmVarDecl()))))` and records
// Mapper::ToString(VarDecl), which is printQualifiedName -- so a class-static
// data member records identically to a namespace-scope global.  There was never
// a mechanism gap; there was only no user.
//
// NO rule body can dodge this by ignoring the parameter: the default expression
// is converted at the construct site and textually substituted into the inlined
// body (`[VisitCXXConstructExpr:4743] {let _ = ( ( (*...in_0)) | ((*...out_1)) );`),
// so basic_stringstream f3's `let _ = a1;` receives the already-emitted
// reference.  Keying the two members is the only available fix.

unsigned int f5() { return std::ios_base::in; }

unsigned int f6() { return std::ios_base::out; }
