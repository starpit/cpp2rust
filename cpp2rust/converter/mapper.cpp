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
#include <unordered_map>
#include <utility>
#include <vector>

#include "converter/converter_lib.h"
#include "converter/translation_rule.h"
#include "survey.h"

namespace cpp2rust::Mapper {

namespace {

clang::ASTContext *ctx_ = nullptr;
Model model_ = Model::kUnsafe;
bool translation_rules_loaded_ = false;

std::unordered_multimap<std::string, TranslationRule::ExprRule>
    exprs_; // src -> ExprRule
std::unordered_multimap<std::string, TranslationRule::TypeRule>
    types_; // src -> TypeRule

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
  // Idempotent: registering a type twice is normal (a decl can be reached by
  // more than one route) and must not depend on the caller guarding first.
  // Guarding on "is the plain name known" used to skip the whole call and
  // leave the POINTER forms unregistered.
  auto [it, end] = types_.equal_range(key);
  for (; it != end; ++it) {
    if (it->second.src == src) {
      return;
    }
  }
  // A top-level const pointer (`DtInfo *const`) is the same Rust type as the
  // plain one -- constness is dropped on that side -- but it is a distinct
  // spelling that lookups use and nothing registered it.
  if (src.ends_with(" *")) {
    auto const_src = src + "const";
    bool have = false;
    auto [cb, ce] = types_.equal_range(key);
    for (; cb != ce; ++cb) {
      if (cb->second.src == const_src) {
        have = true;
        break;
      }
    }
    if (!have) {
      auto copy = rule;
      copy.src = const_src;
      types_.emplace(key, std::move(copy));
    }
  }

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
// True if `text` contains a comma outside any bracket -- i.e. it spans more
// than one template argument.
//
// A single template parameter must never capture one. std::tuple<T1, T2>
// otherwise matches a 26-element std::tie tuple by binding T1 to "const
// double &" and T2 to the remaining 25 arguments, and that bogus "type" then
// fails to map with the whole parameter list quoted as if it were a type.
bool spansSeveralArguments(std::string_view text) {
  int depth = 0;
  for (char c : text) {
    switch (c) {
    case '<':
    case '(':
    case '[':
      ++depth;
      break;
    case '>':
    case ')':
    case ']':
      --depth;
      break;
    case ',':
      if (depth == 0) {
        return true;
      }
      break;
    default:
      break;
    }
  }
  return false;
}

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
          if (spansSeveralArguments(*repl)) {
            return std::nullopt;
          }
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
          if (spansSeveralArguments(*repl)) {
            return std::nullopt;
          }
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

// `const T1` is spelled west of the type name, but when T1 binds to a POINTER
// clang prints the very same type east of the `*`: `const T1 &` with
// T1 = `Node *` is `Node *const &`, not `const Node * &`. A rule source keeps
// the west spelling it was written with, so the literal `const ` in front of
// the placeholder never lines up and the rule silently does not match. That is
// how `std::map<K *, V>::operator[]` fell through to the generic subscript
// path and came out as pointer arithmetic (`(m as Ptr<..>).offset(k)`).
//
// Produce the east-const spellings of `src` so the caller can retry with them.
// Only the placeholders are moved: a `const` in front of a concrete name is
// part of that name and means something else (`const Node *` is a pointer to
// const, a different type from `Node *const`).
std::vector<std::string> eastConstVariants(const std::string &src,
                                           const std::string &instantiated) {
  // The east spelling only ever shows up as `*` followed by `const`; without
  // one in the text there is nothing a variant could match.
  if (instantiated.find("*const") == std::string::npos &&
      instantiated.find("* const") == std::string::npos) {
    return {};
  }

  // Offsets of every `const T<n>` in `src`, plus the length of the run.
  struct Site {
    size_t pos;  // index of 'c' in "const"
    size_t len;  // length of "const<ws>T<digits>"
    size_t name; // index of 'T'
  };
  std::vector<Site> sites;
  for (size_t i = src.find("const"); i != std::string::npos;
       i = src.find("const", i + 1)) {
    // "const" must be a whole word.
    if (i > 0 && (std::isalnum((unsigned char)src[i - 1]) || src[i - 1] == '_')) {
      continue;
    }
    size_t j = i + 5;
    if (j >= src.size() || !std::isspace((unsigned char)src[j])) {
      continue;
    }
    while (j < src.size() && std::isspace((unsigned char)src[j])) {
      j++;
    }
    if (j + 1 >= src.size() || src[j] != 'T' ||
        !std::isdigit((unsigned char)src[j + 1])) {
      continue;
    }
    size_t k = j + 1;
    while (k < src.size() && std::isdigit((unsigned char)src[k])) {
      k++;
    }
    sites.push_back({i, k - i, j});
  }

  // A signature with more than a handful of them is not worth enumerating.
  constexpr size_t kMaxSites = 4;
  if (sites.empty() || sites.size() > kMaxSites) {
    return {};
  }

  std::vector<std::string> variants;
  for (unsigned mask = 1; mask < (1u << sites.size()); ++mask) {
    std::string out;
    size_t copied = 0;
    for (size_t s = 0; s < sites.size(); ++s) {
      if (!(mask & (1u << s))) {
        continue;
      }
      const Site &site = sites[s];
      out.append(src, copied, site.pos - copied);
      // "const  T1" -> "T1 const"
      out.append(src, site.name, site.pos + site.len - site.name);
      out.append(" const");
      copied = site.pos + site.len;
    }
    out.append(src, copied, std::string::npos);
    variants.push_back(std::move(out));
  }
  return variants;
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
    // The index can be more than one digit, and the token to replace is
    // `T` plus that whole run -- replacing a fixed two characters turns
    // T10 into <binding of T1> followed by a stray "0".
    size_t end = pos + 1;
    while (end < instantiated_template.size() &&
           std::isdigit(instantiated_template[end])) {
      ++end;
    }
    unsigned idx = std::stoul(instantiated_template.substr(pos + 1,
                                                           end - pos - 1));
    const auto &repl = types.at(idx - 1).value();
    instantiated_template.replace(pos, end - pos, repl);
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

  const std::string *ambiguous_with = nullptr;
  for (; it != end; ++it) {
    auto &this_rule = it->second;
    auto this_subs = matchTemplate(this_rule.src, txt);
    if (!this_subs) {
      // Retry with `const Tn` respelled east of the pointer, which is how
      // clang prints it when Tn binds to a pointer type.
      for (const auto &variant : eastConstVariants(this_rule.src, txt)) {
        this_subs = matchTemplate(variant, txt);
        if (this_subs) {
          break;
        }
      }
    }
    if (!this_subs) {
      continue;
    }
    // tie breaker: prefer more specific rules (usually the longer ones)
    if (!rule || this_rule.src.size() > rule->src.size()) {
      rule = &this_rule;
      subs = *std::move(this_subs);
      ambiguous_with = nullptr;
    } else if (this_rule.src.size() == rule->src.size() &&
               this_rule.src != rule->src) {
      // Two DIFFERENT rules of equal specificity both match, so "longest
      // wins" is a coin flip and the multimap order decides. std::get<0> and
      // std::get<1> on a std::tuple<int, int> do exactly this: the rules are
      // `T1 &get(tuple<T1,T2>&)` and `T2 &get(tuple<T1,T2>&)`, equal length,
      // both matching. Picking one silently returns the wrong element.
      ambiguous_with = &this_rule.src;
    }
  }
  if (ambiguous_with) {
    llvm::errs() << "ERROR: ambiguous translation rule for '" << txt
                 << "': '" << rule->src << "' and '" << *ambiguous_with
                 << "' are equally specific. Refusing to guess.\n";
    ReportUnsupported("AmbiguousRule", txt);
    return {};
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

// Registers a project type the first time it is actually needed.
//
// The up-front pass cannot see everything: clang instantiates class templates
// lazily, and cpp2rust drives Sema during conversion, so a specialization can
// come into existence after that pass has already run. Rather than fail on a
// type whose declaration we can plainly reach, register it here and retry.
std::pair<TranslationRule::TypeRule *, std::vector<std::optional<std::string>>>
lazyRegisterAndRetry(clang::QualType qual_type, const std::string &type) {
  static bool registering = false;
  if (registering) {
    return {};
  }

  auto peeled = qual_type.getNonReferenceType();
  while (peeled->isPointerType()) {
    peeled = peeled->getPointeeType();
  }
  auto *tag = peeled.getUnqualifiedType()->getAsTagDecl();
  if (!tag || !IsUserDefinedDecl(tag)) {
    return {};
  }
  if (auto *def = tag->getDefinition()) {
    tag = def;
  }

  registering = true;
  AddRuleForUserDefinedType(tag);
  registering = false;

  return search(types_, type, GetTypeMapKey(type));
}

std::pair<TranslationRule::TypeRule *, std::vector<std::optional<std::string>>>
search(clang::QualType qual_type) {
  auto sugared = ToString(qual_type, ScalarSugar::kPreserve);
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
  auto res = search(types_, type, GetTypeMapKey(type));
  if (!res.first) {
    res = lazyRegisterAndRetry(qual_type, type);
  }
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
      for (auto it = begin; it != end; ++it) {
        if (it->second.src == rule.src) {
          llvm::errs() << "ERROR: duplicate type rule for C++ type '"
                       << rule.src << "': maps to both '"
                       << it->second.type_info.type << "' and '"
                       << rule.type_info.type << "'\n";
          std::exit(EXIT_FAILURE);
        }
      }
      types_.emplace(std::move(key), std::move(rule));
    }
  }
}

// Gives every loaded library type its `const T`, `T *` and `const T *` forms.
//
// A module declares only the plain spelling (`t1 = std::vector<T1>`), so a
// `std::vector<regInfo> *` parameter or a `const std::string` had no rule at
// all. Scalars get these for free in add_scalar_rule; this is the same thing
// for rule-provided types, with constness dropped on the Rust side exactly as
// scalars drop it.
//
// Runs AFTER every module is loaded and goes through AddTypeRule, which is
// idempotent -- doing it inside the per-module loop trips that loop's
// duplicate-rule check, which exits the process.
void addDerivedTypeForms(Model model) {
  std::vector<std::pair<std::string, std::string>> base;
  base.reserve(types_.size());
  for (const auto &[key, rule] : types_) {
    if (!rule.src.starts_with("const ") && !rule.src.contains('*') &&
        !rule.src.contains('[')) {
      base.emplace_back(rule.src, rule.type_info.type);
    }
  }

  // Only the `const T` form. Adding `T *` as well makes a rule match where
  // the converter previously handled the pointer decay itself, and it emits
  // an offset against the container instead of its elements.
  (void)model;
  for (const auto &[src, rust] : base) {
    AddTypeRule("const " + src, TranslationRule::TypeRule::Plain(rust));
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

  // Bare `void`. Only the pointer forms were registered, so a rule whose
  // generic binds to void (std::shared_ptr<void>, a void-returning callable)
  // had nothing to map to.
  // The unsafe model spells a void pointee ::libc::c_void, so bare `void`
  // must match that there; the refcount model has no libc types and uses ().
  const char *void_rust =
      model == Model::kUnsafe ? "::libc::c_void" : "()";
  AddTypeRule(ToString(ctx_->VoidTy),
              TranslationRule::TypeRule::Plain(void_rust));
  AddTypeRule("const " + ToString(ctx_->VoidTy),
              TranslationRule::TypeRule::Plain(void_rust));

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

std::string mapTypeStringRecursive(const std::string &cpp_type) {
  auto [rule, subs] = search(types_, cpp_type, GetTypeMapKey(cpp_type));
  if (!rule) {
    // Only a type STRING is available here, so the decl cannot be reached
    // directly. Clang instantiates class templates lazily and cpp2rust drives
    // Sema while converting, so specializations keep appearing after the
    // up-front pass ran. Re-walk the TU once per unseen type and retry.
    static std::unordered_set<std::string> refreshed;
    if (ctx_ && refreshed.insert(cpp_type).second) {
      PreRegisterUserDefinedTypes(ctx_->getTranslationUnitDecl());
      std::tie(rule, subs) =
          search(types_, cpp_type, GetTypeMapKey(cpp_type));
    }
  }
  if (!rule) {
    // Pointer-to-a-library-type, reached as a container's element type
    // (std::map<K, std::vector<regInfo>*>). Deliberately handled HERE and not
    // by registering `std::vector<T> *` as a real type rule: the converter's
    // pointer-decay path keys on that type NOT having a rule, and giving it
    // one makes `p[0]` index the vector handle instead of its elements.
    // This string-level fallback only affects recursive type spelling.
    {
      std::string_view view = cpp_type;
      while (view.ends_with("const")) {
        view.remove_suffix(5);
        while (view.ends_with(' ')) {
          view.remove_suffix(1);
        }
      }
      // Never a FUNCTION pointer: `Ret (*)(args)` would have its '*' stripped
      // and the bare function type looked up, which cannot match and reports
      // the parameter list as if it were a type. Those are handled by
      // VisitPointerType's FunctionProtoType branch instead.
      if (view.ends_with('*') && cpp_type.find('(') == std::string::npos) {
        view.remove_suffix(1);
        while (view.ends_with(' ')) {
          view.remove_suffix(1);
        }
        std::string pointee(view);
        if (auto [prule, psubs] =
                search(types_, pointee, GetTypeMapKey(pointee));
            prule) {
          for (auto &ty : psubs) {
            if (ty) {
              ty = mapTypeStringRecursive(*ty);
            }
          }
          return "Ptr<" + instantiateTgt(psubs, prule->type_info.type) + '>';
        }
      }
    }

    // NDEBUG builds compile the assert out, so guard the null deref below
    // explicitly rather than letting it fall through.
    if (ReportUnsupported("UnmappedType", cpp_type)) {
      if (getenv("CPP2RUST_DEBUG_UNMAPPED")) {
        auto key = GetTypeMapKey(cpp_type);
        llvm::errs() << "UNMAPPED '" << cpp_type << "' key='" << key
                     << "' registered=" << types_.size() << " near:";
        auto [b, e] = types_.equal_range(key);
        for (; b != e; ++b) {
          llvm::errs() << " {" << b->second.src << "}";
        }
        llvm::errs() << '\n';
      }
      return UnsupportedPlaceholder("UnmappedType", cpp_type);
    }
    llvm::errs() << "cpp_type: " << cpp_type << '\n';
    if (ctx_) {
      llvm::errs() << "  (key='" << GetTypeMapKey(cpp_type) << "')\n";
    }
    assert(0 && "Type is not present in types_");
    llvm::report_fatal_error("Type is not present in types_");
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

PushASTContext::PushASTContext(clang::ASTContext &ctx) : prev_(ctx_) {
  ctx_ = &ctx;
}
PushASTContext::~PushASTContext() { ctx_ = prev_; }

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
  return GetNamedDeclAsString(decl->getCanonicalDecl());
}

std::string InstantiateTemplate(const clang::Expr *expr, unsigned n) {
  auto expr_str = ToString(expr);
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

void PreRegisterUserDefinedTypes(clang::DeclContext *dc) {
  for (auto *d : dc->decls()) {
    if (auto *tag = llvm::dyn_cast<clang::TagDecl>(d)) {
      // Only project types: a system type without a rule must keep failing
      // loudly rather than be silently renamed into a plausible-looking one.
      // Anonymous tags are skipped: nothing can name one to look it up, and
      // naming one here would consume a disambiguating id out of the order
      // the converter itself assigns them in.
      const bool has_name =
          tag->getIdentifier() || tag->getTypedefNameForAnonDecl();
      if (has_name && IsUserDefinedDecl(tag) && tag->isCompleteDefinition()) {
        AddRuleForUserDefinedType(tag);
      }
    }
    // A class template's instantiations are not TU children, so reach them
    // through the template: types nested in one (an enum inside
    // FoldFunction<int>, say) are concrete and do need a rule. The uninstan-
    // tiated pattern is skipped -- it is dependent and has nothing to map.
    if (auto *tmpl = llvm::dyn_cast<clang::ClassTemplateDecl>(d)) {
      for (auto *spec : tmpl->specializations()) {
        // The specialization is itself a type that needs a rule, not just a
        // scope holding some.
        if (IsUserDefinedDecl(spec)) {
          AddRuleForUserDefinedType(spec);
        }
        if (spec->isCompleteDefinition()) {
          PreRegisterUserDefinedTypes(spec);
        }
      }
      continue;
    }
    // Namespaces and records only: recursing into function bodies would cost
    // a full extra walk of the TU for the rare function-local type.
    if (llvm::isa<clang::NamespaceDecl, clang::RecordDecl,
                  clang::LinkageSpecDecl>(d)) {
      PreRegisterUserDefinedTypes(llvm::cast<clang::DeclContext>(d));
    }
  }
}

void AddRuleForUserDefinedType(clang::NamedDecl *decl) {
  auto cpp_name = ToString(GetTypeForDecl(decl));
  auto rs_name = ToRustName(cpp_name);

  // A class template specialization registers under the spelling that keeps
  // defaulted template arguments -- FoldFunction<std::vector<long long,
  // std::allocator<long long>>> -- while lookups use the spelling that drops
  // them. Register the dropped form under the same Rust name as well, or the
  // type is unreachable by the name every caller actually uses.
  if (auto *spec =
          llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl)) {
    auto policy = getPrintPolicy();
    policy.SuppressDefaultTemplateArgs = true;
    std::string terse;
    llvm::raw_string_ostream os(terse);
    clang::QualType(ctx_->getCanonicalTagType(spec)).print(os, policy);
    if (terse != cpp_name && !terse.empty()) {
      AddTypeRule(terse, TranslationRule::TypeRule::Plain(rs_name));
      AddTypeRule("const " + terse, TranslationRule::TypeRule::Plain(rs_name));
      switch (model_) {
      case Model::kUnsafe:
        AddTypeRule(terse + " *",
                    TranslationRule::TypeRule::UnsafePtr("*mut " + rs_name));
        AddTypeRule("const " + terse + " *",
                    TranslationRule::TypeRule::UnsafePtr("*const " + rs_name));
        break;
      case Model::kRefCount:
        AddTypeRule(terse + " *", TranslationRule::TypeRule::RefcountPtr(
                                      "Ptr<" + rs_name + '>'));
        AddTypeRule("const " + terse + " *",
                    TranslationRule::TypeRule::RefcountPtr("Ptr<" + rs_name +
                                                           '>'));
        break;
      }
    }
  }

  AddTypeRule(cpp_name, TranslationRule::TypeRule::Plain(rs_name));
  // Scalars register their const forms too (see add_scalar_rule); without the
  // same here, a `const Foo *` parameter has no rule even though `Foo *` does.
  // Constness is dropped on the Rust side, as it is for scalars.
  AddTypeRule("const " + cpp_name, TranslationRule::TypeRule::Plain(rs_name));

  if (auto record_decl = llvm::dyn_cast<clang::RecordDecl>(decl)) {
    // Forward declaration. A pointer to an incomplete type is still valid
    // C++ and still needs a rule; only the abstract/dyn distinction needs
    // the definition, so assume the non-abstract form here.
    if (!record_decl->isThisDeclarationADefinition()) {
      switch (model_) {
      case Model::kUnsafe:
        AddTypeRule(cpp_name + " *",
                    TranslationRule::TypeRule::UnsafePtr("*mut " + rs_name));
        AddTypeRule("const " + cpp_name + " *",
                    TranslationRule::TypeRule::UnsafePtr("*const " + rs_name));
        break;
      case Model::kRefCount:
        AddTypeRule(cpp_name + " *", TranslationRule::TypeRule::RefcountPtr(
                                         "Ptr<" + rs_name + '>'));
        AddTypeRule("const " + cpp_name + " *",
                    TranslationRule::TypeRule::RefcountPtr("Ptr<" + rs_name +
                                                           '>'));
        break;
      }
      return;
    }

    if (auto cxx_decl = llvm::dyn_cast<clang::CXXRecordDecl>(record_decl)) {
      if (cxx_decl->isAbstract()) {
        switch (model_) {
        case Model::kUnsafe:
          AddTypeRule(cpp_name + " *", TranslationRule::TypeRule::UnsafePtr(
                                           "*mut dyn " + rs_name));
          AddTypeRule("const " + cpp_name + " *",
                      TranslationRule::TypeRule::UnsafePtr("*const dyn " +
                                                           rs_name));
          break;
        case Model::kRefCount:
          AddTypeRule(cpp_name + " *", TranslationRule::TypeRule::RefcountPtr(
                                           "PtrDyn<dyn " + rs_name + '>'));
          AddTypeRule("const " + cpp_name + " *",
                      TranslationRule::TypeRule::RefcountPtr("PtrDyn<dyn " +
                                                             rs_name + '>'));
          break;
        }
      } else {
        switch (model_) {
        case Model::kUnsafe:
          AddTypeRule(cpp_name + " *",
                      TranslationRule::TypeRule::UnsafePtr("*mut " + rs_name));
          AddTypeRule("const " + cpp_name + " *",
                      TranslationRule::TypeRule::UnsafePtr("*const " +
                                                           rs_name));
          break;
        case Model::kRefCount:
          AddTypeRule(cpp_name + " *", TranslationRule::TypeRule::RefcountPtr(
                                           "Ptr<" + rs_name + '>'));
          AddTypeRule("const " + cpp_name + " *",
                      TranslationRule::TypeRule::RefcountPtr("Ptr<" + rs_name +
                                                             '>'));
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
      if (!ReportUnsupported("OverloadedOperatorKind",
                             clang::getOperatorSpelling(op))) {
        assert(0 && "Unexpected overloaded operator kind");
      }
      os << "unsupported";
      break;
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

  os << '(';
  for (unsigned i = 0, n = func_decl->getNumParams(); i < n; ++i) {
    if (i) {
      os << ", ";
    }
    os << ToString(func_decl->getParamDecl(i)->getType());
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
        auto range_type = ToString(for_range->getRangeInit()->getType());
        if (range_type.starts_with("std::map<") ||
            range_type.starts_with("std::set<") ||
            range_type.starts_with("std::unordered_map<") ||
            range_type.starts_with("std::unordered_set<")) {
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
  addDerivedTypeForms(model);

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
