// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include "converter/mapper.h"

#include <clang/AST/ExprCXX.h>
#include <clang/Basic/OperatorKinds.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <llvm/Support/ThreadPool.h>

#include <cctype>
#include <cstdlib>
#include <format>
#include <optional>
#include <regex>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "converter/converter_lib.h"
#include "converter/survey.h"
#include "converter/translation_rule.h"

namespace cpp2rust::Mapper {

namespace {

clang::ASTContext *ctx_ = nullptr;
Model model_ = Model::kUnsafe;
bool translation_rules_loaded_ = false;

std::unordered_multimap<std::string, TranslationRule::ExprRule>
    exprs_; // src -> ExprRule
std::unordered_multimap<std::string, TranslationRule::TypeRule>
    types_; // src -> TypeRule

// The spelling(s) the last `search(clang::QualType)` actually looked up. See
// Mapper::DescribeLastTypeSearch in mapper.h for why the diagnostics must quote
// these and not a spelling rebuilt from the RecordDecl.
std::string last_type_search_sugared_;
std::string last_type_search_canonical_; // empty when it equalled the sugared

// Index of PROJECT (user-defined) tag decls in this TU, keyed by the MAPPER's
// own spelling of the type -- the same string a leaf reaching
// mapTypeStringRecursive carries. Built lazily, once, only on a lookup miss.
std::unordered_map<std::string, const clang::TagDecl *> user_tags_;
bool user_tags_built_ = false;

clang::PrintingPolicy getPrintPolicy() {
  assert(ctx_);
  clang::PrintingPolicy policy(ctx_->getLangOpts());
  policy.Bool = true;
  policy.SuppressTagKeyword = true;
  policy.SuppressScope = false;
  policy.FullyQualifiedName = true;
  policy.SuppressUnwrittenScope = true;
  policy.UsePreferredNames = true;
  return policy;
}

std::string GetExprMapKey(const std::string &str) {
  // Extract the function name from something like
  // const T1 & std::foo<T1, T2>::fn_name(args)
  auto n = str.find_first_of('(');
  if (n == std::string::npos) {
    n = str.size();
  }

  // Walk backwards from '(' tracking <> depth:
  // - skip characters inside template arguments (depth > 0)
  // - stop at the first space outside all angle brackets
  std::string result;
  int depth = 0;
  for (int i = (int)n - 1; i >= 0; --i) {
    char c = str[i];
    if (c == '>')
      ++depth;
    else if (c == '<')
      --depth;
    else if (c == ' ' && depth == 0)
      break;
    else if (depth == 0)
      result += c;
  }
  std::reverse(result.begin(), result.end());
  return result;
}

constexpr const char kPackMarker[] = "&&...";

std::string GetTypeMapKey(const std::string &str) {
  auto n = str.find_first_of("<[");
  if (n == std::string::npos || str[n] == '<') {
    return str.substr(0, n);
  }
  // something like int[][] or T1[] -> []
  return str.substr(n + 1);
}

void AddTypeRule(std::string src, TranslationRule::TypeRule &&rule) {
  auto key = GetTypeMapKey(src);
  rule.src = std::move(src);
  types_.emplace(std::move(key), std::move(rule));
}

// Attempts to unify an instantiated C++ type or function signature with a
// corresponding template pattern. If the two match structurally, it returns
// a mapping from template parameter names (e.g., "T1") to their concrete
// instantiated types (e.g., "int"). If no match is possible, returns nullopt.
//
// Example:
//   template_str   = "std::vector<T1>::vector()"
//   instantiated   = "std::vector<int>::vector()"
//   result         = { "int" }
std::optional<std::vector<std::optional<std::string>>>
matchTemplate(const std::string &template_str,
              const std::string &instantiated) {
  auto matchLiteralAt = [&](const std::string &input_str, size_t pos,
                            std::string_view literal, size_t &end_pos) -> bool {
    size_t i = pos;
    size_t j = 0;

    while (true) {
      while (i < input_str.size() && std::isspace(input_str[i])) {
        i++;
      }

      while (j < literal.size() && std::isspace(literal[j])) {
        j++;
      }

      if (j == literal.size()) {
        end_pos = i;
        return true;
      }

      if (i >= input_str.size()) {
        return false;
      }

      if (input_str[i] != literal[j]) {
        return false;
      }

      i++;
      j++;
    }
  };

  auto findNextLiteralSameDepth = [&](const std::string &s, size_t start,
                                      std::string_view lit) -> size_t {
    int ang = 0;
    int par = 0;
    int sq = 0;

    for (size_t i = 0; i < s.size() && i < start; i++) {
      switch (s[i]) {
      case '<': {
        ang++;
        break;
      }
      case '>': {
        ang--;
        break;
      }
      case '(': {
        par++;
        break;
      }
      case ')': {
        par--;
        break;
      }
      case '[': {
        sq++;
        break;
      }
      case ']': {
        sq--;
        break;
      }
      default:
        break;
      }
      assert(ang >= 0 && par >= 0 && sq >= 0 && "Unbalanced ang, par or sq");
    }

    int base_ang = ang;
    int base_par = par;
    int base_sq = sq;

    for (size_t i = start; i <= s.size(); i++) {
      if (ang == base_ang && par == base_par && sq == base_sq) {
        size_t end_i = 0;
        if (matchLiteralAt(s, i, lit, end_i)) {
          return i;
        }
      }

      if (i == s.size()) {
        break;
      }

      char c = s[i];
      switch (c) {
      case '<': {
        ang++;
        break;
      }
      case '>': {
        ang--;
        break;
      }
      case '(': {
        par++;
        break;
      }
      case ')': {
        par--;
        break;
      }
      case '[': {
        sq++;
        break;
      }
      case ']': {
        sq--;
        break;
      }
      default:
        break;
      }

      if (ang < 0 || par < 0 || sq < 0) {
        return std::string::npos;
      }
    }

    return std::string::npos;
  };

  std::vector<std::optional<std::string>> captured;

  size_t ti = 0;
  size_t si = 0;

  while (ti < template_str.size()) {
    if (template_str[ti] == 'T' && ti + 1 < template_str.size() &&
        std::isdigit(template_str[ti + 1])) {
      size_t tj = ti + 2;
      while (tj < template_str.size() && std::isdigit(template_str[tj])) {
        tj++;
      }

      size_t type_idx = std::stoi(&template_str[ti + 1]) - 1;
      assert(type_idx < TranslationRule::kMaxGenerics &&
             "template placeholder exceeds kMaxGenerics");
      ti = tj;

      std::string_view nextLit;
      size_t scan = ti;
      while (scan < template_str.size()) {
        if (template_str[scan] == 'T' && scan + 1 < template_str.size() &&
            std::isdigit(template_str[scan + 1])) {
          break;
        }
        scan++;
      }
      nextLit = std::string_view(template_str).substr(ti, scan - ti);

      captured.resize(std::max(captured.size(), type_idx + 1));
      auto &repl = captured[type_idx];
      if (repl.has_value()) {
        size_t end_pos = 0;
        if (!matchLiteralAt(instantiated, si, *repl, end_pos)) {
          return std::nullopt;
        }
        si = end_pos;
      } else {
        if (!nextLit.empty()) {
          size_t k = findNextLiteralSameDepth(instantiated, si, nextLit);
          if (k == std::string::npos) {
            return std::nullopt;
          }

          size_t a = si;
          size_t b = k;

          while (a < b && std::isspace(instantiated[a])) {
            a++;
          }
          while (b > a && std::isspace(instantiated[b - 1])) {
            b--;
          }

          repl = instantiated.substr(a, b - a);
          si = k;
        } else {
          size_t a = si;
          size_t b = instantiated.size();

          while (a < b && std::isspace(instantiated[a])) {
            a++;
          }
          while (b > a && std::isspace(instantiated[b - 1])) {
            b--;
          }

          repl = instantiated.substr(a, b - a);
          si = instantiated.size();
        }
      }

      if (!nextLit.empty()) {
        size_t end_pos = 0;
        if (!matchLiteralAt(instantiated, si, nextLit, end_pos)) {
          return std::nullopt;
        }
        si = end_pos;
        ti += nextLit.size();
      }
    } else {
      size_t tj = ti;
      while (tj < template_str.size()) {
        if (template_str[tj] == 'T' && tj + 1 < template_str.size() &&
            std::isdigit(template_str[tj + 1])) {
          break;
        }
        ++tj;
      }

      auto lit = std::string_view(template_str).substr(ti, tj - ti);
      size_t end_pos = 0;
      if (!matchLiteralAt(instantiated, si, lit, end_pos)) {
        return std::nullopt;
      }
      si = end_pos;
      ti = tj;
    }
  }

  while (si < instantiated.size() && std::isspace(instantiated[si])) {
    si++;
  }

  if (si != instantiated.size()) {
    return std::nullopt;
  }

  return captured;
}

// Substitutes concrete types into a target template string using the provided
// type mapping. Each template parameter in `tgt_template` is replaced with its
// corresponding instantiated type from `types`.
//
// Example:
//   types        = { {"i32"} }
//   tgt_template = "Vec<T1>"
//   result       = "Vec<i32>"
std::string instantiateTgt(const std::vector<std::optional<std::string>> &types,
                           const std::string &tgt_template) {
  assert(types.size() <= TranslationRule::kMaxGenerics &&
         "template placeholder exceeds kMaxGenerics");
  std::string instantiated_template = tgt_template;
  std::string::size_type pos = 0;
  while ((pos = instantiated_template.find('T', pos)) != std::string::npos) {
    if (pos + 1 >= instantiated_template.size()) {
      break;
    }
    if (!std::isdigit(instantiated_template[pos + 1])) {
      ++pos;
      continue;
    }
    const auto &repl = types.at(instantiated_template[pos + 1] - '1').value();
    instantiated_template.replace(pos, 2, repl);
    pos += repl.length();
  }
  return instantiated_template;
}

template <typename T>
std::pair<T *, std::vector<std::optional<std::string>>>
search(std::unordered_multimap<std::string, T> &map, const std::string &txt,
       const std::string &key) {
  auto [it, end] = map.equal_range(key);
  T *rule = nullptr;
  std::vector<std::optional<std::string>> subs;

  for (; it != end; ++it) {
    auto &this_rule = it->second;
    auto this_subs = matchTemplate(this_rule.src, txt);
    if (!this_subs) {
      continue;
    }
    // tie breaker: prefer more specific rules (usually the longer ones)
    if (!rule || this_rule.src.size() > rule->src.size()) {
      rule = &this_rule;
      subs = *std::move(this_subs);
    }
  }
  return {rule, std::move(subs)};
}

TranslationRule::ExprRule *search(const clang::Expr *expr) {
  if (RefersToUserDefinedDecl(expr)) {
    return nullptr;
  }
  auto qualified_name = ToString(expr);
  auto [rule, subs] =
      search(exprs_, qualified_name, GetExprMapKey(qualified_name));
  log() << "search expr " << qualified_name << ", result:\n";
  if (rule) {
    rule->dump();
  } else {
    log() << "None\n";
  }
  return rule;
}

std::pair<TranslationRule::TypeRule *, std::vector<std::optional<std::string>>>
search(clang::QualType qual_type) {
  auto sugared = ToString(qual_type, ScalarSugar::kPreserve);
  last_type_search_sugared_ = sugared;
  last_type_search_canonical_.clear();
  if (auto res = search(types_, sugared, GetTypeMapKey(sugared)); res.first) {
    log() << "search type " << sugared
          << ", result: " << res.first->type_info.type << '\n';
    return res;
  }
  auto type = ToString(qual_type);
  if (type == sugared) {
    log() << "search type " << type << ", result: None\n";
    return {};
  }
  last_type_search_canonical_ = type;
  auto res = search(types_, type, GetTypeMapKey(type));
  log() << "search type " << type
        << ", result: " << (res.first ? res.first->type_info.type : "None")
        << '\n';
  return res;
}

void addRulesFromDirectory(const std::filesystem::path &dir, Model model) {
  namespace fs = std::filesystem;
  for (const auto &entry : fs::directory_iterator(dir)) {
    const auto &path = entry.path();
    assert(fs::exists(path / "ir_src.json") &&
           (fs::exists(path / "ir_unsafe.json") ||
            fs::exists(path / "ir_refcount.json")));
    auto [expr_rules, type_rules] = TranslationRule::Load(path, model);
    if (expr_rules.empty() && type_rules.empty()) {
      log() << "No rules found in " << path << '\n';
      continue;
    }
    for (auto &[_, rule] : expr_rules) {
      exprs_.emplace(GetExprMapKey(rule.src), std::move(rule));
    }
    for (auto &[_, rule] : type_rules) {
      auto key = GetTypeMapKey(rule.src);
      auto [begin, end] = types_.equal_range(key);
      bool already_present = false;
      for (auto it = begin; it != end; ++it) {
        if (it->second.src != rule.src) {
          continue;
        }
        // An AGREEING duplicate is not a conflict. Several modules legitimately
        // restate the same library type -- with this toolchain's libc++,
        // rules/vector, rules/string and rules/algorithm each declare
        // `std::__wrap_iter<const T1 *>` and all three map it to the same Rust
        // type. Rejecting that made the converter unable to translate a single
        // file: it exited during rule loading, before parsing any C++, with
        // "maps to both 'Ptr<T1>' and 'Ptr<T1>'" -- naming two identical
        // mappings as if they disagreed.
        //
        // Only a DISAGREEMENT is a real defect, because type lookup must be
        // single-valued: `types_` is keyed on the printed C++ type string and
        // every consultation takes the first match, so two different Rust types
        // under one key would silently resolve per load order.
        if (it->second.type_info.type == rule.type_info.type) {
          already_present = true;
          break;
        }
        llvm::errs() << "ERROR: conflicting type rules for C++ type '"
                     << rule.src << "': maps to both '"
                     << it->second.type_info.type << "' and '"
                     << rule.type_info.type << "'\n";
        std::exit(EXIT_FAILURE);
      }
      if (already_present) {
        continue;
      }
      types_.emplace(std::move(key), std::move(rule));
    }
  }
}

void addBuiltinTypes(Model model) {
  assert(ctx_);

  auto add_scalar_rule = [&](const std::string &cxx, const std::string &rust,
                             const std::string &initializer = {}) {
    auto plain = TranslationRule::TypeRule::Plain(rust);
    plain.initializer = initializer;
    std::vector<std::string> derives = {"Copy",  "Clone",     "Default",
                                        "Debug", "PartialEq", "PartialOrd"};
    if (!(rust == "f32" || rust == "f64")) {
      derives.insert(derives.end(), {"Eq", "Ord", "Hash"});
    }
    plain.type_info.derives = std::move(derives);
    AddTypeRule(cxx, TranslationRule::TypeRule(plain));
    AddTypeRule("const " + cxx, std::move(plain));

    switch (model) {
    case Model::kUnsafe:
      AddTypeRule(cxx + " *",
                  TranslationRule::TypeRule::UnsafePtr("*mut " + rust));
      AddTypeRule("const " + cxx + " *",
                  TranslationRule::TypeRule::UnsafePtr("*const " + rust));
      break;
    case Model::kRefCount:
      AddTypeRule(cxx + " *", TranslationRule::TypeRule::RefcountPtr(
                                  "Ptr::<" + rust + ">"));
      AddTypeRule("const " + cxx + " *", TranslationRule::TypeRule::RefcountPtr(
                                             "Ptr::<" + rust + ">"));
      break;
    }
  };

  auto add_builtin_rule = [&](clang::QualType qt, const std::string &rust) {
    add_scalar_rule(ToString(qt), rust);
  };

  auto add_size_rules = [&](clang::QualType size_type,
                            std::initializer_list<const char *> aliases,
                            const std::string &rust) {
    auto initializer = "0_" + rust;
    for (const char *alias : aliases) {
      add_scalar_rule(alias, rust, initializer);
    }
    if (const auto *predef = clang::dyn_cast<clang::PredefinedSugarType>(
            size_type.getTypePtr())) {
      add_scalar_rule(predef->getIdentifier()->getName().str(), rust,
                      initializer);
    }
  };

  auto build_rust_type = [&](clang::QualType qt) {
    unsigned bits = ctx_->getTypeSize(qt);
    char sign = qt->isSignedIntegerType() ? 'i' : 'u';
    return std::format("{}{}", sign, bits);
  };

  // Misc
  add_builtin_rule(ctx_->BoolTy, "bool");
  add_builtin_rule(ctx_->FloatTy, "f32");
  add_builtin_rule(ctx_->DoubleTy, "f64");

  switch (model) {
  case Model::kUnsafe:
    AddTypeRule(ToString(ctx_->VoidTy) + " *",
                TranslationRule::TypeRule::UnsafePtr("*mut ::libc::c_void"));
    AddTypeRule("const " + ToString(ctx_->VoidTy) + " *",
                TranslationRule::TypeRule::UnsafePtr("*const ::libc::c_void"));
    break;
  case Model::kRefCount:
    AddTypeRule(ToString(ctx_->VoidTy) + " *",
                TranslationRule::TypeRule::RefcountPtr("AnyPtr"));
    AddTypeRule("const " + ToString(ctx_->VoidTy) + " *",
                TranslationRule::TypeRule::RefcountPtr("AnyPtr"));
    break;
  }

  // Char
  switch (model) {
  case Model::kUnsafe:
    add_builtin_rule(ctx_->CharTy, "libc::c_char");
    break;
  case Model::kRefCount:
    add_builtin_rule(ctx_->CharTy, "u8");
    break;
  }
  add_builtin_rule(ctx_->SignedCharTy, "i8");
  add_builtin_rule(ctx_->UnsignedCharTy, "u8");

  // Integers
  add_builtin_rule(ctx_->ShortTy, build_rust_type(ctx_->ShortTy));
  add_builtin_rule(ctx_->UnsignedShortTy,
                   build_rust_type(ctx_->UnsignedShortTy));
  add_builtin_rule(ctx_->IntTy, build_rust_type(ctx_->IntTy));
  add_builtin_rule(ctx_->UnsignedIntTy, build_rust_type(ctx_->UnsignedIntTy));
  add_builtin_rule(ctx_->LongTy, build_rust_type(ctx_->LongTy));
  add_builtin_rule(ctx_->UnsignedLongTy, build_rust_type(ctx_->UnsignedLongTy));
  add_builtin_rule(ctx_->LongLongTy, build_rust_type(ctx_->LongLongTy));
  add_builtin_rule(ctx_->UnsignedLongLongTy,
                   build_rust_type(ctx_->UnsignedLongLongTy));

  add_size_rules(ctx_->getSizeType(), {"size_t", "size_type"}, "usize");
  add_size_rules(ctx_->getSignedSizeType(), {"ssize_t"}, "isize");
}

clang::QualType normalizeQualType(clang::QualType qual_type) {
  assert(ctx_);

  bool isLRef = qual_type->isLValueReferenceType();
  bool isRRef = qual_type->isRValueReferenceType();
  qual_type = qual_type.getNonReferenceType();

  clang::Qualifiers qualifiers = qual_type.getQualifiers();

  while (true) {
    if (const auto *attributed =
            llvm::dyn_cast<clang::AttributedType>(qual_type)) {
      qual_type = attributed->getModifiedType();
      continue;
    }
    if (const auto *dcltype = llvm::dyn_cast<clang::DecltypeType>(qual_type)) {
      qual_type = dcltype->getUnderlyingType();
      continue;
    }
    break;
  }

  if (llvm::isa<clang::InjectedClassNameType>(qual_type)) {
    qual_type = qual_type.getCanonicalType();
  }

  qual_type = qual_type.withFastQualifiers(qualifiers.getFastQualifiers());
  if (qualifiers.hasNonFastQualifiers()) {
    qual_type = ctx_->getQualifiedType(qual_type, qualifiers);
  }

  if (isLRef) {
    qual_type = ctx_->getLValueReferenceType(qual_type);
  }

  if (isRRef) {
    qual_type = ctx_->getRValueReferenceType(qual_type);
  }

  return qual_type.getCanonicalType().getUnqualifiedType().getDesugaredType(
      *ctx_);
}

// mapTypeStringRecursive works on STRINGS and has no clang::Decl in hand, so a
// bare leaf name is unactionable: it names neither the outer type it came from
// nor where the leaf is declared. These entry-point-set contexts thread that in.
struct MapContext {
  std::string outer;
  clang::QualType outer_type;
};
MapContext map_ctx_;

struct PushMapContext {
  MapContext prev;
  PushMapContext(std::string outer, clang::QualType ty = clang::QualType())
      : prev(map_ctx_) {
    map_ctx_ = MapContext{std::move(outer), ty};
  }
  PushMapContext(const PushMapContext &) = delete;
  PushMapContext &operator=(const PushMapContext &) = delete;
  ~PushMapContext() { map_ctx_ = prev; }
};

// Collects every tag (enum/class/struct/union) decl reachable from `ty` through
// pointers, references and template arguments, so an unmapped leaf spelling can
// be matched against a real declaration with a location.
void CollectTagDecls(clang::QualType ty,
                     std::vector<const clang::TagDecl *> &out, int depth = 0) {
  if (ty.isNull() || depth > 8 || out.size() > 32) {
    return;
  }
  ty = ty.getCanonicalType();
  if (const auto *tag = ty->getAsTagDecl()) {
    out.push_back(tag);
    if (const auto *spec =
            llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(tag)) {
      for (const auto &arg : spec->getTemplateArgs().asArray()) {
        if (arg.getKind() == clang::TemplateArgument::Type) {
          CollectTagDecls(arg.getAsType(), out, depth + 1);
        }
      }
    }
  }
  if (!ty->getPointeeType().isNull()) {
    CollectTagDecls(ty->getPointeeType(), out, depth + 1);
  }
  if (const auto *arr = ctx_ ? ctx_->getAsArrayType(ty) : nullptr) {
    CollectTagDecls(arr->getElementType(), out, depth + 1);
  }
}

// Walk every DeclContext in the TU collecting PROJECT tag decls, keyed by the
// mapper's spelling of their type. This is the only way to answer
// "is this SPELLING a project type?" on the mapTypeStringRecursive path, which
// by construction holds neither a clang::Decl nor a QualType: the leaf arrives
// as a template argument of a mapped library type (e.g. the enum inside
// `std::optional<mlir::ktdp::SpyreMemorySpaceKind>`) and is a bare string.
// Deliberately skips unnamed tags (they have no stable spelling to key on) and
// template specialisations (GetTypeForDecl rebuilds a specialisation type whose
// spelling is not what a leaf string carries -- a wrong hit there would emit a
// name nothing defines, so we prefer the loud assert).
void CollectUserTags(const clang::DeclContext *dc, int depth = 0) {
  if (depth > 32) {
    return;
  }
  for (const auto *d : dc->decls()) {
    if (const auto *tag = llvm::dyn_cast<clang::TagDecl>(d)) {
      if (tag->getIdentifier() != nullptr &&
          !llvm::isa<clang::ClassTemplateSpecializationDecl>(tag) &&
          IsUserDefinedDecl(tag)) {
        auto ty = GetTypeForDecl(tag);
        if (!ty.isNull()) {
          user_tags_.emplace(ToString(ty), tag);
        }
      }
    }
    if (llvm::isa<clang::NamespaceDecl>(d) || llvm::isa<clang::RecordDecl>(d) ||
        llvm::isa<clang::LinkageSpecDecl>(d)) {
      CollectUserTags(llvm::cast<clang::DeclContext>(d), depth + 1);
    }
  }
}

std::string DescribeUnmappedLeaf(const std::string &cpp_type) {
  std::string msg = "type `" + cpp_type + "` has no model in types_";
  if (!map_ctx_.outer.empty()) {
    msg += ", while mapping `" + map_ctx_.outer + "`";
  }
  if (!map_ctx_.outer_type.isNull() && ctx_ != nullptr) {
    std::vector<const clang::TagDecl *> tags;
    CollectTagDecls(map_ctx_.outer_type, tags);
    for (const auto *tag : tags) {
      const std::string name = tag->getQualifiedNameAsString();
      const bool is_leaf = cpp_type.find(name) != std::string::npos;
      msg += std::string("\n    ") + (is_leaf ? "LEAF " : "") +
             (llvm::isa<clang::EnumDecl>(tag) ? "enum" : "record") + " `" +
             name + "` declared at " +
             tag->getLocation().printToString(ctx_->getSourceManager()) +
             (IsUserDefinedDecl(tag) ? " [project type]" : " [system type]");
    }
    if (tags.empty()) {
      msg += "\n    (no tag decl reachable from the outer type)";
    }
  } else {
    msg += "\n    (no outer QualType in hand -- mapped from an expression or "
           "rule-setup context, so VisitEnumDecl/VisitRecordDecl may not have "
           "run for it yet)";
  }
  return msg;
}

std::string mapTypeStringRecursive(const std::string &cpp_type) {
  auto [rule, subs] = search(types_, cpp_type, GetTypeMapKey(cpp_type));
  if (!rule) {
    if (survey::Enabled()) {
      // Keep the DETAIL exactly the bare spelling: it is the survey's grouping
      // key and the work queue's row label. The new context goes in the
      // location field, which is free-form.
      survey::Record(survey::GapKind::kUnmappedType, cpp_type,
                     DescribeUnmappedLeaf(cpp_type));
      // No Rust is emitted in survey mode, so the returned spelling is never
      // written anywhere; it only keeps the walk going.
      return cpp_type;
    }
    // A PROJECT leaf is NOT a missing model: the converter ports the type and
    // emits a `pub struct`/`pub type` for it in this same TU, under exactly the
    // name GetRecordName computes -- `ToRustName(ToString(GetTypeForDecl))`,
    // converter.cpp:4755-4760. It has no `types_` entry here only because
    // AddRuleForUserDefinedType has not run for it YET (the leaf is reached
    // through a mapped library template before VisitEnumDecl/VisitRecordDecl
    // gets to the decl). Asserting made a CORRECT port of the enum abort the
    // whole TU. Measured on mlir::ktdp::SpyreMemorySpaceKind, an I32EnumAttr in
    // a GENERATED header reached via a plain -I (so isInSystemHeader is false
    // and IsUserDefinedDecl is true), arriving as the argument of
    // `std::optional<...>`.
    //
    // We deliberately do NOT AddTypeRule here: VisitEnumDecl bails on
    // `Mapper::Contains(...)` (converter.cpp:4215), so registering the rule
    // would SUPPRESS the `pub type` emission and turn the name undefined.
    //
    // SYSTEM types with no rule keep asserting. That loud failure is
    // load-bearing -- it is what stops a missing model becoming an undefined
    // Rust name at rc=0 -- and is not weakened by this branch.
    if (const clang::TagDecl *tag = nullptr;
        LooksLikeUserDefinedTypeName(cpp_type, &tag)) {
      static std::set<std::string> ported_leaves;
      if (ported_leaves.insert(cpp_type).second) {
        llvm::errs() << "note: project leaf type `" << cpp_type
                     << "` has no types_ entry yet; emitting its PORTED name `"
                     << ToRustName(cpp_type) << "` (declared at "
                     << (ctx_ != nullptr
                             ? tag->getLocation().printToString(
                                   ctx_->getSourceManager())
                             : std::string("<no ASTContext>"))
                     << ")\n";
      }
      return ToRustName(cpp_type);
    }
    if (survey::MangleUnmapped()) {
      // --mangle-unmapped (TRIAGE ONLY): the twin of the fallback in
      // Converter::ReportUnmappedSystemType (converter.cpp:3408). That one
      // covers an unmapped system record reached with a DECL in hand; this one
      // covers the RECURSIVE LEAF of a mapped family, which arrives here as a
      // bare STRING. Without it, mapping a CRTP family correctly made things
      // measurably WORSE: `rules/smallvector` maps SmallVector<T1,_> and its
      // three bases, mapping recurses into the element type, and
      // `mlir::OpFoldResult` -- which has no model -- aborted the whole TU at
      // rc=134 with NOTHING emitted, where before the rule the TU emitted and
      // could be counted. A correct rule must never be punished by the
      // instrument.
      //
      // We have NO decl and NO QualType here, so there is deliberately no
      // file:line in this message: DescribeUnmappedLeaf already says so
      // ("no outer QualType in hand") and inventing a location would be worse
      // than admitting we do not have one.
      static std::set<std::string> reported;
      if (reported.insert(cpp_type).second) {
        llvm::errs() << "MANGLED (triage): unmapped leaf type has no rule: `"
                     << cpp_type << "` (would be emitted as the undefined name `"
                     << ToRustName(cpp_type)
                     << "`) rule key: " << cpp_type << " -- "
                     << DescribeUnmappedLeaf(cpp_type) << '\n';
      }
      // Returning the MANGLED spelling, not the C++ spelling: this value is
      // substituted into emitted Rust, so it must at least be an identifier.
      // It is an UNDEFINED one -- that is the point, the emission does not
      // compile and rc=0 here means even less than usual.
      return ToRustName(cpp_type);
    }
    llvm::errs() << "unsupported unmapped " << DescribeUnmappedLeaf(cpp_type)
                 << '\n';
    assert(0 && "Type is not present in types_");
  }
  for (auto &ty : subs) {
    if (ty) {
      ty = mapTypeStringRecursive(*ty);
    }
  }
  return instantiateTgt(subs, rule->type_info.type);
}

std::string normalizeTranslationRule(std::string rule) {
  // Detach pointer from double reference. Useful for matching translation
  // rules.
  ReplaceAll(rule, "*&&", "* &&");

  static const std::array<std::pair<std::regex, std::string>, 1>
      normalization_rules{{
          // Ignore constant template parameters, i.e. replace them with _.
          {std::regex(R"(\b\d+\b)"), "_"},
      }};

  for (const auto &r : normalization_rules) {
    rule = std::regex_replace(rule, r.first, r.second);
  }

  return rule;
}

} // namespace

std::string DescribeLastTypeSearch() {
  if (last_type_search_sugared_.empty()) {
    return "searched as: <no type search recorded>";
  }
  std::string out = "searched as: " + last_type_search_sugared_;
  if (!last_type_search_canonical_.empty()) {
    out += ", canonical (searched only because it differs): " +
           last_type_search_canonical_;
  }
  return out;
}

bool LooksLikeUserDefinedTypeName(const std::string &cpp_type,
                                  const clang::TagDecl **decl) {
  if (ctx_ == nullptr) {
    return false;
  }
  if (!user_tags_built_) {
    user_tags_built_ = true;
    CollectUserTags(ctx_->getTranslationUnitDecl());
  }
  auto it = user_tags_.find(cpp_type);
  if (it == user_tags_.end()) {
    return false;
  }
  if (decl != nullptr) {
    *decl = it->second;
  }
  return true;
}

PushASTContext::PushASTContext(clang::ASTContext &ctx) : prev_(ctx_) {
  ctx_ = &ctx;
  // The tag index holds decls owned by the OLD context; never let it outlive it.
  user_tags_.clear();
  user_tags_built_ = false;
}
PushASTContext::~PushASTContext() {
  ctx_ = prev_;
  user_tags_.clear();
  user_tags_built_ = false;
}

bool Contains(clang::QualType qual_type) {
  return search(qual_type).first != nullptr;
}

bool Contains(const clang::Expr *expr) { return search(expr) != nullptr; }

const TranslationRule::ExprRule *GetExprRule(const clang::Expr *expr) {
  return search(expr);
}

bool IsLibcPassthrough(const clang::Expr *expr) {
  const auto *tgt_ir = GetExprRule(expr);
  if (tgt_ir == nullptr || !tgt_ir->body.empty() || !tgt_ir->is_extern) {
    return false;
  }
  const auto *ref =
      clang::dyn_cast<clang::DeclRefExpr>(expr->IgnoreParenImpCasts());
  const auto *decl = ref != nullptr ? ref->getDecl() : nullptr;
  return decl != nullptr &&
         decl->getASTContext().getSourceManager().isInSystemHeader(
             decl->getLocation());
}

std::string MapFunctionName(const clang::FunctionDecl *decl) {
  assert(decl);
  if (!IsUserDefinedDecl(decl) &&
      exprs_.contains(GetExprMapKey(ToString(decl)))) {
    return std::format("libcc2rs::{}_{}", decl->getNameAsString(),
                       model_ == Model::kRefCount ? "refcount" : "unsafe");
  }
  // SECOND mangled-name fallback, the function twin of the type one in
  // Converter::VisitRecordType. A SYSTEM function with no exprs_ rule has no
  // body to emit in this TU, so GetNamedDeclAsString's disambiguating decl-id
  // suffix (converter_lib.cpp:803-819) invents a callee -- `std::next` becomes
  // `next_19` / `next_20` -- that is called and never defined. Measured: 140 of
  // the 1589 rustc errors on KTDF/Utils/Utils.cpp are exactly these two names.
  // Same defect class as the mangled TYPE name: a missing model turns into an
  // undefined symbol at rc=0 instead of a translate-time failure.
  //
  // Project functions are fine: their definition is emitted in this TU (or, for
  // an out-of-line sibling-TU definition, is a known cross-TU limitation --
  // AGENT-COMMON, "A MISSING METHOD IS OFTEN NOT A DEFECT AT ALL"), so only the
  // system case is reported.
  if (!IsUserDefinedDecl(decl)) {
    const std::string key = ToString(decl);
    const std::string loc =
        ctx_ != nullptr
            ? decl->getLocation().printToString(ctx_->getSourceManager())
            : std::string("<no ASTContext>");
    const std::string mangled = GetNamedDeclAsString(decl->getCanonicalDecl());
    std::string detail = "system function has no rule: `" + key +
                         "` (would be called as the undefined name `" + mangled +
                         "`) rule key: " + key;
    if (survey::Enabled()) {
      survey::Record(survey::GapKind::kUnsupportedExpr, detail, loc);
      return mangled;
    }
    llvm::errs() << "unsupported " << detail << " at " << loc << '\n';
    assert(0 && "unsupported system function: no rule in exprs_");
  }
  return GetNamedDeclAsString(decl->getCanonicalDecl());
}

std::string InstantiateTemplate(const clang::Expr *expr, unsigned n) {
  auto expr_str = ToString(expr);
  PushMapContext ctx("expression " + expr_str);
  auto [rule, subs] = search(exprs_, expr_str, GetExprMapKey(expr_str));
  auto text = std::format("T{}", n);
  if (!rule) {
    return text;
  }
  auto &ty = subs.at(n - 1);
  if (ty) {
    ty = mapTypeStringRecursive(*ty);
  }
  return instantiateTgt(subs, text);
}

std::string Map(clang::QualType qual_type) {
  PushMapContext ctx(ToString(qual_type), qual_type);
  auto [rule, subs] = search(qual_type);
  if (rule) {
    for (auto &ty : subs) {
      if (ty) {
        ty = mapTypeStringRecursive(*ty);
      }
    }
    return instantiateTgt(subs, rule->type_info.type);
  }
  return {};
}

std::string MapInitializer(clang::QualType qual_type) {
  PushMapContext ctx(ToString(qual_type), qual_type);
  auto [rule, subs] = search(qual_type);
  if (rule && !rule->initializer.empty()) {
    for (auto &ty : subs) {
      if (ty) {
        ty = mapTypeStringRecursive(*ty);
      }
    }
    return instantiateTgt(subs, rule->initializer);
  }
  return {};
}

bool MapsToPointer(clang::QualType qual_type) {
  auto rule = search(qual_type).first;
  return rule && rule->type_info.is_pointer();
}

bool MapsToRefcountPointer(clang::QualType qual_type) {
  auto rule = search(qual_type).first;
  return rule && rule->type_info.is_refcount_pointer;
}

const std::vector<std::string> *MappedDerives(clang::QualType qual_type) {
  auto rule = search(qual_type).first;
  return rule ? &rule->type_info.derives : nullptr;
}

void SetDerives(clang::QualType qual_type, std::vector<std::string> derives) {
  if (auto *rule = search(qual_type).first) {
    rule->type_info.derives = std::move(derives);
  }
}
bool ReturnsPointer(const clang::Expr *expr) {
  auto rule = search(expr);
  return rule && rule->return_type.is_pointer();
}

const TranslationRule::TypeInfo &GetParamInfo(const clang::Expr *expr,
                                              unsigned index) {
  auto rule = search(expr);
  assert(rule && "expression must have a translation rule");
  return rule->params.at(index);
}

std::string GetParamType(const clang::Expr *expr, unsigned index) {
  auto expr_str = ToString(expr);
  PushMapContext ctx("parameter " + std::to_string(index) + " of " + expr_str);
  auto [rule, subs] = search(exprs_, expr_str, GetExprMapKey(expr_str));
  for (auto &ty : subs) {
    if (ty) {
      ty = mapTypeStringRecursive(*ty);
    }
  }
  return instantiateTgt(subs, rule->params.at(index).type);
}

bool ParamIsPointer(const clang::Expr *expr, unsigned index) {
  return GetParamInfo(expr, index).is_pointer();
}

clang::QualType GetTypeForDecl(const clang::NamedDecl *decl) {
  if (const auto *spec =
          llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl)) {
    llvm::ArrayRef<clang::TemplateArgument> args =
        spec->getTemplateArgs().asArray();
    llvm::SmallVector<clang::TemplateArgument, 4> canon(args.begin(),
                                                        args.end());
    ctx_->canonicalizeTemplateArguments(canon);

    return ctx_->getTemplateSpecializationType(
        clang::ElaboratedTypeKeyword::None,
        clang::TemplateName(spec->getSpecializedTemplate()), args, canon);
  }

  const auto *rdecl = llvm::dyn_cast<clang::TagDecl>(decl);
  assert(rdecl && "Unsupported decl type");

  return ctx_->getTagType(clang::ElaboratedTypeKeyword::None,
                          rdecl->getQualifier(), rdecl, /*OwnsTag*/ false);
}

void AddRuleForUserDefinedType(clang::NamedDecl *decl) {
  auto cpp_name = ToString(GetTypeForDecl(decl));
  auto rs_name = ToRustName(cpp_name);

  AddTypeRule(cpp_name, TranslationRule::TypeRule::Plain(rs_name));

  if (auto record_decl = llvm::dyn_cast<clang::RecordDecl>(decl)) {
    // Forward declaration
    if (!record_decl->isThisDeclarationADefinition()) {
      return;
    }

    if (auto cxx_decl = llvm::dyn_cast<clang::CXXRecordDecl>(record_decl)) {
      if (cxx_decl->isAbstract()) {
        switch (model_) {
        case Model::kUnsafe:
          AddTypeRule(cpp_name + " *", TranslationRule::TypeRule::UnsafePtr(
                                           "*mut dyn " + rs_name));
          break;
        case Model::kRefCount:
          AddTypeRule(cpp_name + " *", TranslationRule::TypeRule::RefcountPtr(
                                           "PtrDyn<dyn " + rs_name + '>'));
          break;
        }
      } else {
        switch (model_) {
        case Model::kUnsafe:
          AddTypeRule(cpp_name + " *",
                      TranslationRule::TypeRule::UnsafePtr("*mut " + rs_name));
          break;
        case Model::kRefCount:
          AddTypeRule(cpp_name + " *", TranslationRule::TypeRule::RefcountPtr(
                                           "Ptr<" + rs_name + '>'));
          break;
        }
      }

      for (auto *nested : GetNestedStructs(cxx_decl)) {
        AddRuleForUserDefinedType(nested);
      }
    }
  }
}

std::string ToRustName(std::string name) {
  ReplaceAll(name, "::", "_");
  ReplaceAll(name, "*", "ptr");
  ReplaceAll(name, "&", "ref");
  ReplaceAll(name, "[", "arr");
  ReplaceAll(name, "]", "arr");
  ReplaceAll(name, "-", "neg");
  for (auto &c : name) {
    if (!std::isalnum(c) && c != '_') {
      c = '_';
    }
  }
  return name;
}

std::string ToString(clang::QualType qual_type, ScalarSugar sugar) {
  assert(ctx_);

  if (sugar == ScalarSugar::kPreserve) {
    clang::QualType t = qual_type;
    if (const auto *decltype_type =
            clang::dyn_cast<clang::DecltypeType>(t.getTypePtr())) {
      t = decltype_type->getUnderlyingType();
    }
    if (const auto *typedef_type = t->getAs<clang::TypedefType>()) {
      if (t.getCanonicalType()->isBuiltinType()) {
        return typedef_type->getDecl()->getNameAsString();
      }
    } else if (const auto *predef = t->getAs<clang::PredefinedSugarType>()) {
      return predef->getIdentifier()->getName().str();
    } else if (const auto *ptr = t->getAs<clang::PointerType>()) {
      auto pointee = ptr->getPointeeType();
      auto canonical = pointee.getCanonicalType().getDesugaredType(*ctx_);
      if (Map(pointee) == Map(canonical)) {
        pointee = canonical;
      }
      std::string out;
      llvm::raw_string_ostream os(out);
      ctx_->getPointerType(pointee).print(os, getPrintPolicy());
      return normalizeTranslationRule(std::move(out));
    }
  }

  if (auto cxx_record_decl = qual_type->getAsCXXRecordDecl()) {
    if (cxx_record_decl->isLambda()) {
      return ToString(cxx_record_decl->getLambdaCallOperator());
    }
  }

  if (auto *tag = qual_type->getAsTagDecl();
      tag && !tag->getIdentifier() && !tag->getTypedefNameForAnonDecl()) {
    return ToString(clang::cast<clang::NamedDecl>(tag));
  }

  if (auto *tag = qual_type->getAsTagDecl();
      tag && tag->getIdentifier() &&
      tag->getDeclContext()->isFunctionOrMethod()) {
    return GetNamedDeclAsString(tag);
  }

  if (auto renamed = DisambiguateAnonymousTag(qual_type->getAsTagDecl());
      !renamed.empty()) {
    return renamed;
  }

  std::string type;
  llvm::raw_string_ostream os(type);
  normalizeQualType(qual_type).print(os, getPrintPolicy());
  return normalizeTranslationRule(std::move(type));
}

bool HasFunctionParameterPack(const clang::FunctionDecl *decl) {
  if (auto *primary = decl->getPrimaryTemplate()) {
    decl = primary->getTemplatedDecl();
  }
  return decl->getNumParams() && decl->parameters().back()->isParameterPack();
}

std::string ToString(const clang::NamedDecl *decl) {
  if (auto *record = clang::dyn_cast<clang::RecordDecl>(decl);
      record && !record->getIdentifier()) {
    if (auto renamed = DisambiguateAnonymousTag(record); !renamed.empty()) {
      return renamed;
    }
    if (auto *typedef_decl = record->getTypedefNameForAnonDecl()) {
      return ToString(clang::cast<clang::NamedDecl>(typedef_decl));
    }
    return GetNamedDeclAsString(record);
  }

  if (auto *enum_decl = clang::dyn_cast<clang::EnumDecl>(decl)) {
    if (auto renamed = DisambiguateAnonymousTag(enum_decl); !renamed.empty()) {
      return renamed;
    }
    if (!enum_decl->getIdentifier() &&
        !enum_decl->getTypedefNameForAnonDecl()) {
      return GetNamedDeclAsString(enum_decl);
    }
  }

  std::string out;
  llvm::raw_string_ostream os(out);

  const clang::FunctionDecl *func_decl = nullptr;
  if (auto *template_decl = llvm::dyn_cast<clang::FunctionTemplateDecl>(decl)) {
    func_decl = template_decl->getTemplatedDecl();
  } else {
    func_decl = llvm::dyn_cast_or_null<clang::FunctionDecl>(decl);
  }

  if (!func_decl) {
    decl->printQualifiedName(os, getPrintPolicy());
    return normalizeTranslationRule(std::move(out));
  }

  os << ToString(func_decl->getReturnType()) << ' ';
  if (const auto op = func_decl->getOverloadedOperator();
      op >= clang::OverloadedOperatorKind::OO_LessLess &&
      op <= clang::OverloadedOperatorKind::OO_GreaterGreaterEqual) {
    // ensure matchTemplate does not consider these operator names when matching
    func_decl->getQualifier().print(os, getPrintPolicy());
    os << "operator ";
    switch (op) {
    case clang::OverloadedOperatorKind::OO_LessLess:
      os << "shl";
      break;
    case clang::OverloadedOperatorKind::OO_GreaterGreater:
      os << "shr";
      break;
    case clang::OverloadedOperatorKind::OO_LessLessEqual:
      os << "shleq";
      break;
    case clang::OverloadedOperatorKind::OO_GreaterGreaterEqual:
      os << "shreq";
      break;
    default:
      if (survey::Enabled()) {
        survey::Record(survey::GapKind::kUnsupportedExpr,
                       std::string("mapped operator name: ") +
                           clang::getOperatorSpelling(op),
                       {});
        os << "unmapped_op";
        break;
      }
      assert(0 && "Unexpected overloaded operator kind");
    }
  } else if (const auto *method_decl =
                 llvm::dyn_cast<clang::CXXMethodDecl>(func_decl)) {
    if (method_decl->getParent()->isLambda() &&
        method_decl->getOverloadedOperator() == clang::OO_Call) {
      func_decl->printName(os, getPrintPolicy());
    } else {
      func_decl->printQualifiedName(os, getPrintPolicy());
    }
  } else {
    func_decl->printQualifiedName(os, getPrintPolicy());
  }

  bool has_pack = HasFunctionParameterPack(func_decl);
  unsigned num_params = func_decl->getNumParams();
  if (has_pack) {
    const auto *primary = func_decl->getPrimaryTemplate();
    num_params =
        (primary ? primary->getTemplatedDecl() : func_decl)->getNumParams() - 1;
  }

  os << '(';
  for (unsigned i = 0; i < num_params; ++i) {
    if (i) {
      os << ", ";
    }
    os << ToString(func_decl->getParamDecl(i)->getType());
  }
  if (has_pack) {
    if (num_params) {
      os << ", ";
    }
    os << kPackMarker;
  }
  if (func_decl->isVariadic()) {
    if (func_decl->getNumParams()) {
      os << ", ";
    }
    os << "...";
  }
  os << ')';

  if (const auto *method_decl =
          llvm::dyn_cast<clang::CXXMethodDecl>(func_decl)) {
    if (method_decl->isConst()) {
      os << " const";
    }
    if (method_decl->isVolatile()) {
      os << " volatile";
    }
    switch (method_decl->getRefQualifier()) {
    case clang::RQ_LValue:
      os << " &";
      break;
    case clang::RQ_RValue:
      os << " &&";
      break;
    default:
      break;
    }
  }

  return normalizeTranslationRule(std::move(out));
}

std::string ToString(const clang::Expr *expr) {
  if (!expr) {
    assert(0 && "!expr");
  }

  expr = expr->IgnoreParenImpCasts();

  if (llvm::isa<clang::IntegerLiteral>(expr) &&
      expr->getBeginLoc().isMacroID()) {
    auto &sm = ctx_->getSourceManager();
    auto name = clang::Lexer::getImmediateMacroName(expr->getBeginLoc(), sm,
                                                    ctx_->getLangOpts());
    if (!name.empty()) {
      return name.str();
    }
  }

  if (const auto *CE = llvm::dyn_cast<clang::CallExpr>(expr)) {
    if (const auto *decl = CE->getDirectCallee()) {
      return ToString(decl);
    }
  }

  if (const auto *ctor = llvm::dyn_cast<clang::CXXConstructExpr>(expr)) {
    if (const auto *ctor_decl = ctor->getConstructor()) {
      return ToString(ctor_decl);
    }
    assert(0 && "expr is a CXXConstructExpr but could not get constructor");
  }

  if (const auto *ME = llvm::dyn_cast<clang::MemberExpr>(expr)) {
    if (const auto *member_decl =
            llvm::dyn_cast<clang::NamedDecl>(ME->getMemberDecl())) {
      if (const auto *method_decl =
              llvm::dyn_cast<clang::CXXMethodDecl>(member_decl)) {
        return ToString(method_decl);
      }
      if (ME->isArrow()) {
        auto *base = ME->getBase()->IgnoreParenImpCasts();
        if (auto *op = llvm::dyn_cast<clang::CXXOperatorCallExpr>(base)) {
          if (op->getOperator() == clang::OO_Arrow) {
            return ToString(op->getArg(0)->getType()) + "->" +
                   ToString(member_decl);
          }
        }
      } else if (auto for_range = GetParentForRange(*ctx_, ME)) {
        if (ToString(for_range->getRangeInit()->getType())
                .starts_with("std::map<")) {
          auto iter_type = GetForRangeIteratorType(for_range);
          if (!iter_type.isNull()) {
            return ToString(iter_type) + "->" + ToString(member_decl);
          }
        }
      }
      return ToString(member_decl);
    }
    assert(0 && "expr is a MemberExpr but could not get named decl");
  }

  if (const auto *decl_ref = llvm::dyn_cast<clang::DeclRefExpr>(expr)) {
    if (const auto *named_decl =
            llvm::dyn_cast<clang::NamedDecl>(decl_ref->getDecl())) {
      if (const auto *tmpl_decl =
              llvm::dyn_cast<clang::FunctionTemplateDecl>(named_decl)) {
        return ToString(tmpl_decl->getTemplatedDecl());
      }
      return ToString(named_decl);
    }
    return "";
  }

  if (const auto *uop = llvm::dyn_cast<clang::UnaryOperator>(expr)) {
    auto sub = ToString(uop->getSubExpr());
    std::string_view opcode =
        clang::UnaryOperator::getOpcodeStr(uop->getOpcode());
    return uop->isPostfix() ? std::format("{}{}", sub, opcode)
                            : std::format("{}{}", opcode, sub);
  }

  return "Unhandled case in ToString";
}

void LoadTranslationRules(Model model, clang::ASTContext &ctx,
                          const std::string &rules_dir) {
  ctx_ = &ctx;
  model_ = model;

  if (translation_rules_loaded_) {
    return;
  }
  translation_rules_loaded_ = true;

  addBuiltinTypes(model);
  addRulesFromDirectory(rules_dir, model);

#if 0
  for (auto &[src, rule] : exprs_) {
    log() << "Expr key: " << src << '\n';
    rule.dump();
  }
  for (auto &[src, rule] : types_) {
    log() << "Type key: " << src << '\n';
    rule.dump();
  }
#endif
}

} // namespace cpp2rust::Mapper
