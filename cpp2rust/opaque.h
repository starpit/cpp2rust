// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#pragma once

#include <clang/AST/DeclBase.h>

#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace cpp2rust::Opaque {

// Opaque API boundaries.
//
// A port does not always want to translate everything the compiler can see. A
// large external C++ API -- MLIR and LLVM here -- is reachable from the code
// under translation but is not itself translation input: the port replaces it,
// it does not carry it over. Left alone, cpp2rust treats those declarations as
// ordinary user code and tries to translate them, which is both wrong and
// fatal (it flattens `mlir::StringAttr` to `mlir_StringAttr`, fails to resolve
// `mlir::OpState`, and aborts long before it reaches the code that matters).
//
// The honest predicate is not "which include flag carried this header": the
// same MLIR API arrives both as real headers under `-isystem` (already skipped
// as system headers) and as TableGen-generated dialect headers in the build
// tree under a plain `-I`, and no path test distinguishes those two from the
// project's own build outputs in a way that survives a different checkout. The
// predicate that matches the intent is the NAMESPACE: `mlir`, `llvm`, and the
// project's own generated dialect namespaces name an API, not a location.
//
// Measured on dsc-based-utils: `mlir` alone accounts for the whole effect,
// because the project's dialects are nested (`mlir::dataflow`, `mlir::uniform`
// ...) and so are reached through it. `llvm` is still worth naming: it is where
// the leaf types are spelled (`llvm::StringRef`, `llvm::SmallVector<mlir::
// Value>`), and naming it makes those boundary types by declaration rather
// than by whatever order the AST walk happened to reach them in.
//
// Declarations inside an opaque namespace are not translated and do not abort
// the run. A reference to one from translated code becomes an opaque Rust type
// -- a nameable unit struct with no definition -- so translation continues and
// the output names exactly what the port still owes. This builds the boundary;
// mapping those names onto a hand-written Rust layer is a translation rule's
// job, not this file's.
//
// Off unless `--opaque-namespace` is given, so default behaviour is unchanged.

// Sets the namespaces to treat as an opaque boundary. An empty list disables
// the feature.
void SetNamespaces(const std::vector<std::string> &names);

// True when at least one opaque namespace is configured.
bool Enabled();

// True when `name` is one of the configured opaque namespaces.
bool IsOpaqueNamespaceName(std::string_view name);

// True when `decl` is part of an opaque namespace and is therefore an API
// boundary rather than translation input.
//
// Declarations in the translation unit's own main file are never opaque: a
// bridge .cpp that reopens `namespace mlir` to define its own operations is
// still the code we were asked to translate. Only what it *reaches* through
// that namespace is a boundary.
bool IsOpaqueDecl(const clang::Decl *decl);

// If the C++ type spelling `cpp_type` names a type in an opaque namespace,
// returns that type stripped of cv-qualifiers, pointers and references;
// otherwise returns an empty string. Used where only a type string is
// available.
//
// Only the OUTERMOST type is tested: `llvm::ArrayRef<mlir::Value>` is opaque
// as a whole, but `std::vector<mlir::Value>` is not -- that one still maps to
// `Vec<T>` with an opaque element type, which is strictly more information.
std::string OpaqueBaseName(std::string_view cpp_type);

// Records that translated code named the opaque Rust type `rust_name`, so a
// declaration for it is emitted. Only what the translated code actually
// reaches is emitted: an opaque API has far more types than any one port
// touches.
void NoteReferenced(std::string rust_name);

// Every opaque Rust type named so far, sorted.
const std::set<std::string> &Referenced();

// True when `rust_name` is one of those -- i.e. a boundary type rather than a
// record the run declared but never defined for some other reason. The two
// share one emission pass but not one shape, so the pass has to tell them
// apart. Always false with the feature off.
bool IsReferenced(const std::string &rust_name);

} // namespace cpp2rust::Opaque
