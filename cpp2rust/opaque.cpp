// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include "opaque.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/Basic/SourceManager.h>
#include <llvm/Support/raw_ostream.h>

#include <cctype>
#include <cstdlib>
#include <set>
#include <unordered_set>

namespace cpp2rust::Opaque {
namespace {

std::unordered_set<std::string> namespaces_;
std::set<std::string> referenced_;

bool IsIdentChar(char c) {
  return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

} // namespace

void SetNamespaces(const std::vector<std::string> &names) {
  namespaces_.clear();
  for (const auto &name : names) {
    if (!name.empty()) {
      namespaces_.insert(name);
    }
  }
}

bool Enabled() { return !namespaces_.empty(); }

bool IsOpaqueNamespaceName(std::string_view name) {
  return !name.empty() && namespaces_.contains(std::string(name));
}

bool IsOpaqueDecl(const clang::Decl *decl) {
  if (!Enabled() || decl == nullptr) {
    return false;
  }
  // The main file is always translation input, whatever namespace it opens.
  // Checked on the spelling location so that a declaration produced by a macro
  // defined in a generated header, but expanded in the main file, is still
  // treated as the main file's own.
  const auto &src_mgr = decl->getASTContext().getSourceManager();
  const auto loc = decl->getLocation();
  if (loc.isValid() && src_mgr.isInMainFile(src_mgr.getExpansionLoc(loc))) {
    return false;
  }

  // A namespace declaration is itself opaque when it names one.
  if (const auto *ns = llvm::dyn_cast<clang::NamespaceDecl>(decl);
      ns != nullptr && ns->getIdentifier() != nullptr &&
      IsOpaqueNamespaceName(ns->getName())) {
    return true;
  }

  // Any enclosing namespace, not just the outermost: `mlir::detail::Foo` and
  // `mlir::LLVM::LLVMPointerType` are both reached through `mlir`, and an
  // inline or anonymous namespace in between must not hide that.
  for (const auto *dc = decl->getDeclContext(); dc != nullptr;
       dc = dc->getParent()) {
    const auto *ns = llvm::dyn_cast<clang::NamespaceDecl>(dc);
    if (ns == nullptr || ns->getIdentifier() == nullptr) {
      continue;
    }
    if (IsOpaqueNamespaceName(ns->getName())) {
      if (getenv("CPP2RUST_DEBUG_OPAQUE") != nullptr) {
        if (const auto *named = llvm::dyn_cast<clang::NamedDecl>(decl)) {
          llvm::errs() << "OPAQUE " << decl->getDeclKindName() << ' '
                       << named->getQualifiedNameAsString() << '\n';
        }
      }
      return true;
    }
  }
  return false;
}

std::string OpaqueBaseName(std::string_view cpp_type) {
  if (!Enabled()) {
    return {};
  }
  // Strip what decorates the outermost type: leading `const`/`volatile`, and
  // trailing `*`, `&`, `&&`, `const`, `volatile`.
  auto view = cpp_type;
  auto strip_leading = [&](std::string_view word) {
    if (view.starts_with(word) && view.size() > word.size() &&
        !IsIdentChar(view[word.size()])) {
      view.remove_prefix(word.size());
      while (view.starts_with(' ')) {
        view.remove_prefix(1);
      }
      return true;
    }
    return false;
  };
  auto strip_trailing = [&](std::string_view word) {
    if (view.ends_with(word) &&
        (view.size() == word.size() ||
         !IsIdentChar(view[view.size() - word.size() - 1]))) {
      view.remove_suffix(word.size());
      while (view.ends_with(' ')) {
        view.remove_suffix(1);
      }
      return true;
    }
    return false;
  };
  bool changed = true;
  while (changed) {
    changed = strip_leading("const") || strip_leading("volatile") ||
              strip_trailing("*") || strip_trailing("&&") ||
              strip_trailing("&") || strip_trailing("const") ||
              strip_trailing("volatile");
  }
  // A function type or a function pointer is never the boundary itself; its
  // parameter list would be mistaken for a qualified name.
  if (view.find('(') != std::string_view::npos) {
    return {};
  }
  const auto sep = view.find("::");
  if (sep == std::string_view::npos ||
      !IsOpaqueNamespaceName(view.substr(0, sep))) {
    return {};
  }
  return std::string(view);
}

void NoteReferenced(std::string rust_name) {
  if (!rust_name.empty()) {
    referenced_.insert(std::move(rust_name));
  }
}

const std::set<std::string> &Referenced() { return referenced_; }

bool IsReferenced(const std::string &rust_name) {
  return referenced_.contains(rust_name);
}

} // namespace cpp2rust::Opaque
