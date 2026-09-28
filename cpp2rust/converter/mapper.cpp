// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include "converter/mapper.h"

#include <clang/AST/ExprCXX.h>
#include <clang/Basic/OperatorKinds.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <llvm/Support/ErrorHandling.h>
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

// SECOND index, keyed by the AS-WRITTEN (sugared) spelling of every tag
// reachable from the OUTER QualType currently being mapped. `user_tags_` above
// cannot serve a template specialisation: it keys on
// `ToString(GetTypeForDecl(tag))`, which DESUGARS
// (`FoldFunction<std::vector<long>>` becomes
// `FoldFunction<std::vector<long, std::allocator<long>>>`), while the leaf
// string that arrives at mapTypeStringRecursive is the sugar the source wrote.
// Indexing from the outer QualType makes BOTH sides of the comparison come out
// of the same printer on the same sugar, so the match is by construction --
// no guessed PrintingPolicy anywhere. Filled on demand, cleared with the ctx.
// ACCUMULATING, never keyed on one outer type: MEASURED, the five gating dbo/ddc/dcg
// TUs reach this leaf with `(no outer QualType in hand)` -- the leaf arrives from an
// EXPRESSION context, not from a QualType entry point -- so an index built only from
// map_ctx_.outer_type fires on the probe and buys NOTHING on the corpus. It is
// therefore also fed from `search(clang::QualType)`, i.e. from every type the mapper
// ever looks up, whose sugared spelling is the very string it looked up.
std::unordered_map<std::string, const clang::TagDecl *> sugared_tags_;

void CollectSugaredTags(clang::QualType ty, int depth);

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

      // A placeholder that captured NO TEXT is not a match. Without this the
      // substitution above happily binds a placeholder to the empty string, so
      // a key spelled `T1 (T2)` unified with a ZERO-argument instantiation
      // (T2 = ""), and since GetTypeMapKey strips at `<` the arity is not in
      // the bucket key either -- the two together made arrow arities
      // indistinguishable. Measured on dataflow-scheduler/lib/Pipeline.cpp: a
      // unary `std::function<T1 (T2)>` key stole the nullary instantiation and
      // aborted with `unmapped type `` has no model in types_, while mapping
      // std::function<std::unique_ptr<mlir::Pass> ()>` -- the empty backticks
      // are this empty binding showing through.
      if (repl->empty()) {
        return std::nullopt;
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
    // Parse the FULL multi-digit index. A single-digit scan consumed only `T1`
    // of `T10` and left the trailing `0` glued onto the substituted type, which
    // emitted struct field types spelled `*const f640` / `*const f641` for the
    // 10th..27th element of a 27-ary std::tie (measured on probe/tup27:
    // `error[E0425]: cannot find type f640`). Same defect shape as the src-side
    // scanner fixed in translation_rule.cpp:366.
    std::string::size_type digits = pos + 1;
    while (digits < instantiated_template.size() &&
           std::isdigit(instantiated_template[digits])) {
      ++digits;
    }
    // `T0` is not a placeholder (indices are 1-based); skip it rather than
    // indexing types.at(-1).
    if (instantiated_template[pos + 1] == '0') {
      ++pos;
      continue;
    }
    const size_t idx =
        std::stoul(instantiated_template.substr(pos + 1, digits - (pos + 1))) -
        1;
    const auto &repl = types.at(idx).value();
    instantiated_template.replace(pos, digits - pos, repl);
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
  // Record the AS-WRITTEN spellings of the project tags inside this type while we
  // still hold the QualType. mapTypeStringRecursive gets only a STRING, and for a
  // leaf reached from an expression there is no outer QualType at all.
  CollectSugaredTags(qual_type, 0);
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

// PRECEDENCE SAFETY for an INLINED rule body.
//
// A rule body is substituted TEXTUALLY into the surrounding emission
// (Converter::GetMappedAsString -> ConvertIRFragment), and the caller then
// freely appends/prepends operators -- most visibly a cast: `<body> as u8`.
// Rust's `as` binds TIGHTER than `==`, so rules/set's free `operator==` body
// `a0 == a1` came out as
//     (a == b as u8)                 // parses as a == (b as u8)
// instead of
//     ((a == b) as u8)
// and rustc reported `error[E0605]: non-primitive cast: BTreeSet<i32> as u8`
// plus an E0308. Any rule whose body's TOP LEVEL is a binary operator is one
// cast away from the same mis-parse, in BOTH models -- and a mis-parse that
// still COMPILES is silently wrong rather than loudly broken.
//
// We wrap at LOAD time, once per rule, rather than at every substitution, and
// only when the body's top level really is a binary operator. That keeps the
// emission byte-identical for every other rule (the substitution path is shared
// by all of them) and, crucially, leaves PLACE expressions alone: `operator[]`
// and `operator*` bodies are used as ASSIGNMENT TARGETS, and a body like `*a0`
// must stay `*a0` -- wrapping it to `(*a0)` changes what a following `.field`
// binds to.
//
// Depth is computed from the TEXT fragments only. Every other fragment kind
// (placeholder, generic, va-args, init, nested method call) expands to a
// balanced expression, so treating it as an opaque atom is sound. `<`/`>` are
// deliberately NOT treated as brackets: a generic spelling is written without
// spaces (`Vec<T1>`), so it cannot match a spaced binary operator.
bool bodyTopLevelIsBinaryOperator(
    const std::vector<TranslationRule::BodyFragment> &body) {
  // Spaced spellings only. The rule preprocessor emits Rust from rustc's own
  // pretty printer, which always puts a space on both sides of a binary
  // operator -- so requiring the spaces is what separates a binary `*` from a
  // prefix deref and a binary `&` from a borrow.
  static const std::array<const char *, 18> kBinOps{
      " == ", " != ", " <= ", " >= ", " && ", " || ", " << ", " >> ",
      " + ",  " - ",  " * ",  " / ",  " % ",  " & ",  " | ",  " ^ ",
      " < ",  " > "};
  int depth = 0;
  for (const auto &frag : body) {
    const auto *t = std::get_if<TranslationRule::TextFragment>(&frag);
    if (t == nullptr) {
      continue;
    }
    const std::string &s = t->text;
    for (std::string::size_type i = 0; i < s.size(); ++i) {
      const char c = s[i];
      if (c == '(' || c == '[' || c == '{') {
        ++depth;
        continue;
      }
      if (c == ')' || c == ']' || c == '}') {
        --depth;
        continue;
      }
      if (depth != 0) {
        continue;
      }
      for (const char *op : kBinOps) {
        const std::string_view sv(op);
        if (s.compare(i, sv.size(), sv) == 0) {
          return true;
        }
      }
    }
  }
  return false;
}

void parenthesizeBodyIfNeeded(TranslationRule::ExprRule &rule) {
  // A multi-statement body is emitted as `{ ... }` by GetMappedAsString, which
  // is already a single primary expression -- and `{(a; b)}` would not even
  // parse. Leave it alone.
  if (rule.multi_statement || rule.body.empty()) {
    return;
  }
  if (!bodyTopLevelIsBinaryOperator(rule.body)) {
    return;
  }
  // Insert-at-front/back rather than rebuilding: BodyFragment holds a
  // unique_ptr alternative, so the vector is move-only.
  rule.body.insert(rule.body.begin(),
                   TranslationRule::BodyFragment(
                       std::in_place_type<TranslationRule::TextFragment>,
                       TranslationRule::TextFragment{"("}));
  rule.body.emplace_back(std::in_place_type<TranslationRule::TextFragment>,
                         TranslationRule::TextFragment{")"});
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
      parenthesizeBodyIfNeeded(rule);
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

  // `void` ITSELF, mapped to Rust's unit type `()`.
  //
  // Only `void *` was registered (just below), so a BARE `void` reaching
  // mapTypeStringRecursive had no model. That was the single largest gap in the
  // 403-TU survey-v5 (263 TUs record it), and it is reached in exactly two
  // shapes, both of which are TYPE ARGUMENTS of an already-mapped family:
  //   * `std::function<void ()>` -- matched by the committed arrow-shape key
  //     `std::function<T1 ()>`, which binds T1 to the RETURN type `void`;
  //   * `std::shared_ptr<void>` / `std::unique_ptr<void>` -- the type-erased
  //     element.
  // Both want `()` in Rust, which is what the QualType path already produces
  // for a void return (`fn tN() -> () { () }`).
  //
  // NO GUARD RESTRICTING IT TO TEMPLATE-ARGUMENT POSITION, and that is a
  // deliberate, checkable decision rather than an omission: C++ itself forbids
  // `void` in every position where a value model would be wrong. [basic.types]
  // makes void an incomplete type that can never be completed, so a variable,
  // a non-static data member, an array element and a by-value parameter of type
  // `void` are all ill-formed and cannot reach the mapper -- clang rejects them
  // before the converter runs. The only remaining positions are a function
  // RETURN type (where `()` is correct and is what the existing
  // Converter::VisitFunctionDecl path emits), a template argument (the two
  // shapes above) and a cast-to-void discard (which never consults types_).
  // So a position-sensitive guard would have no position left to reject; it
  // would only be dead code claiming a safety property it does not provide.
  //
  // Registered with a BARE AddTypeRule, NOT through add_scalar_rule: that
  // helper also synthesises `void *` / `const void *` as `*mut ()` / `*const
  // ()`, which CONFLICTS with the `::libc::c_void` (unsafe) and `AnyPtr`
  // (refcount) entries added immediately below -- AddTypeRule treats a
  // disagreeing duplicate as fatal and would `exit(EXIT_FAILURE)` during rule
  // loading, i.e. every TU would stop translating. The pointer spellings stay
  // as they are; only the bare leaf is new.
  {
    auto unit = TranslationRule::TypeRule::Plain("()");
    unit.initializer = "()";
    unit.type_info.derives = {"Copy",  "Clone",     "Default",  "Debug",
                              "PartialEq", "PartialOrd", "Eq", "Ord", "Hash"};
    AddTypeRule(ToString(ctx_->VoidTy), TranslationRule::TypeRule(unit));
    AddTypeRule("const " + ToString(ctx_->VoidTy), std::move(unit));
  }

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

  // THE <cstdint> FIXED-WIDTH ALIASES. Only size_t/ssize_t were registered, so a
  // type spelled `int64_t` or `uint32_t` had no model even though `long` and
  // `unsigned int` did.
  //
  // This is not hypothetical: the printer that Mapper::search() uses keeps the
  // SUGARED spelling, so these aliases are exactly what it looks up, while the
  // diagnostic's printer canonicalises them away. Two rows today were nothing but
  // this gap:
  //   `unmapped type 'int64_t' ... while mapping mlir::detail::DenseArrayAttrImpl<int64_t>`
  //     -- 3 TUs, reached because that rule had to be made generic precisely because
  //     its canonicalised `<long>` spelling never matched;
  //   `llvm::SmallVectorBase<uint32_t>` searched while the diagnostic printed
  //     `<unsigned int>`.
  // Each alias maps to the same Rust type its underlying builtin already maps to, so
  // the two spellings cannot disagree: on this target `long` -> i64 and
  // `unsigned int` -> u32, matching int64_t and uint32_t below.
  add_scalar_rule("int8_t", "i8", "0_i8");
  add_scalar_rule("uint8_t", "u8", "0_u8");
  add_scalar_rule("int16_t", "i16", "0_i16");
  add_scalar_rule("uint16_t", "u16", "0_u16");
  add_scalar_rule("int32_t", "i32", "0_i32");
  add_scalar_rule("uint32_t", "u32", "0_u32");
  add_scalar_rule("int64_t", "i64", "0_i64");
  add_scalar_rule("uint64_t", "u64", "0_u64");
  add_scalar_rule("intptr_t", "isize", "0_isize");
  add_scalar_rule("uintptr_t", "usize", "0_usize");
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

// Index the tags reachable from an AS-WRITTEN type under their AS-WRITTEN
// spelling. This is CollectTagDecls' twin with the one difference that matters:
// it must NOT canonicalise. CollectTagDecls opens with `ty.getCanonicalType()`
// because it only wants a decl to name a location; here the STRING is the
// product, and canonicalising it reintroduces exactly the desugared spelling
// (`std::vector<long, std::allocator<long>>`) that no leaf ever carries.
// Template arguments are therefore read off the sugared
// TemplateSpecializationType when there is one, and only fall back to the
// specialisation decl's (already-canonical) argument list when there is not.
void CollectSugaredTags(clang::QualType ty, int depth) {
  if (ty.isNull() || depth > 8 || sugared_tags_.size() > 4096 ||
      ctx_ == nullptr) {
    return;
  }
  const clang::QualType unq = ty.getUnqualifiedType();
  const clang::TagDecl *tag = unq->getAsTagDecl();
  if (tag != nullptr && tag->getIdentifier() != nullptr &&
      IsUserDefinedDecl(tag)) {
    sugared_tags_.emplace(ToString(unq), tag);
  }
  if (const auto *tst = unq->getAs<clang::TemplateSpecializationType>()) {
    for (const auto &arg : tst->template_arguments()) {
      if (arg.getKind() == clang::TemplateArgument::Type) {
        CollectSugaredTags(arg.getAsType(), depth + 1);
      }
    }
  } else if (const auto *spec =
                 llvm::dyn_cast_or_null<clang::ClassTemplateSpecializationDecl>(
                     tag)) {
    for (const auto &arg : spec->getTemplateArgs().asArray()) {
      if (arg.getKind() == clang::TemplateArgument::Type) {
        CollectSugaredTags(arg.getAsType(), depth + 1);
      }
    }
  }
  if (!unq->getPointeeType().isNull()) {
    CollectSugaredTags(unq->getPointeeType(), depth + 1);
  }
  if (const auto *arr = ctx_->getAsArrayType(unq)) {
    CollectSugaredTags(arr->getElementType(), depth + 1);
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

std::string mapTypeStringRecursive(const std::string &cpp_type);

// A C++ FUNCTION-POINTER TYPE HAS NO `types_` ENTRY UNDER ANY SPELLING, and it
// never can: the signature is part of the type, so `R (*)(A, B)` would need one
// rule per signature, naming dt_src user types that no system-rule module can
// reach. 26 of 403 dxp TUs aborted here on exactly one spelling reached as the
// VALUE type of a mapped `llvm::DenseMap`:
//   unsupported unmapped type `llvm::LogicalResult (*)(mlir::Operation *,
//     const mlir::NamedAttribute &)` has no model in types_
// So the model is DERIVED STRUCTURALLY, the way pointers and references already
// are (and the way addBuiltinTypes' add_scalar_rule already synthesises `T *`
// per scalar -- this is the same idea one level up).
//
// Shape and calling convention deliberately MATCH the QualType path, which has
// emitted `Option<unsafe fn(..) -> R>` for a pointer-to-function since before
// this change (Converter::VisitPointerType / ConvertFunctionPointerType,
// converter.cpp:359,375). Two paths for one C++ type must not disagree.
//   * `Option<..>`: a function pointer is NULLABLE in C++ and the corpus does
//     assign/compare against null, so the Rust type must be able to represent
//     it. `Option<fn(..)>` is niche-optimised, so this costs no space.
//   * `unsafe fn`: the corpus pointers are stored in tables and called from
//     translated code, so `extern "Rust"` would be sound here; `unsafe fn` is
//     chosen only for agreement with the existing path, and a safe fn item
//     coerces to it implicitly. It is NOT `extern "C"`: no corpus instance was
//     found crossing an FFI boundary, and claiming a C ABI we have not verified
//     would be silently wrong in the ABI.
//
// Splits at TOP LEVEL only (paren, angle-bracket and square-bracket depth), so
// a nested function pointer or a template argument containing a comma survives.
// Every component is mapped through mapTypeStringRecursive, so `void` -> the
// unit return, `llvm::LogicalResult` -> `bool` (rules/support) and
// `mlir::Operation *` -> `*mut fmt::OpInst` (rules/mlir) come from the rules --
// and an UNMAPPED component asserts naming THE COMPONENT, not the signature.
std::optional<std::string> tryDeriveFunctionPointerType(
    const std::string &cpp_type) {
  const std::string kMarker = "(*)(";
  const size_t marker = cpp_type.find(kMarker);
  if (marker == std::string::npos) {
    return std::nullopt;
  }
  // The parameter list opens at the marker's last '(' and must close at the
  // very end of the spelling. Anything trailing (`const`, a member-pointer
  // form, an array of function pointers) is NOT this shape; return nullopt and
  // let the existing loud path report it rather than guessing.
  const size_t args_open = marker + kMarker.size() - 1;
  int depth = 0;
  size_t args_close = std::string::npos;
  for (size_t i = args_open; i < cpp_type.size(); ++i) {
    if (cpp_type[i] == '(') {
      ++depth;
    } else if (cpp_type[i] == ')') {
      if (--depth == 0) {
        args_close = i;
        break;
      }
    }
  }
  if (args_close == std::string::npos) {
    return std::nullopt;
  }
  auto trim = [](std::string s) {
    while (!s.empty() && isspace(static_cast<unsigned char>(s.front()))) {
      s.erase(s.begin());
    }
    while (!s.empty() && isspace(static_cast<unsigned char>(s.back()))) {
      s.pop_back();
    }
    return s;
  };
  if (!trim(cpp_type.substr(args_close + 1)).empty()) {
    return std::nullopt;
  }
  const std::string ret = trim(cpp_type.substr(0, marker));
  if (ret.empty()) {
    return std::nullopt;
  }
  const std::string args_str =
      cpp_type.substr(args_open + 1, args_close - args_open - 1);

  std::vector<std::string> params;
  {
    int d = 0;
    std::string cur;
    for (char c : args_str) {
      if (c == '(' || c == '<' || c == '[') {
        ++d;
      } else if (c == ')' || c == '>' || c == ']') {
        --d;
      }
      if (c == ',' && d == 0) {
        params.push_back(trim(cur));
        cur.clear();
        continue;
      }
      cur += c;
    }
    cur = trim(cur);
    if (!cur.empty()) {
      params.push_back(cur);
    }
  }
  // `R (*)(void)` and `R (*)()` are both zero-parameter.
  if (params.size() == 1 && params.front() == "void") {
    params.clear();
  }

  PushMapContext ctx(cpp_type, map_ctx_.outer_type);
  std::string out = "Option<unsafe fn(";
  for (size_t i = 0; i < params.size(); ++i) {
    if (i != 0) {
      out += ", ";
    }
    out += mapTypeStringRecursive(params[i]);
  }
  out += ")";
  if (ret != "void") {
    out += " -> " + mapTypeStringRecursive(ret);
  }
  out += ">";
  return out;
}

// A REFERENCE SPELLING has no `types_` entry either, and per-type entries are
// the wrong fix: the reference is STRUCTURAL, so `const T &` would need a second
// key beside every `T` in every module. Measured: across all of
// /home/agent/work/pin/ir only the BARE `"mlir::NamedAttribute"` exists, no
// `const mlir::NamedAttribute &` form in any module.
//
// This is the follow-on to tryDeriveFunctionPointerType above: with the
// signature derived, the walk reaches its SECOND parameter and stopped there on
//   unsupported unmapped type `const mlir::NamedAttribute &` ... while mapping
//   `llvm::LogicalResult (*)(mlir::Operation *, const mlir::NamedAttribute &)`
// (26 dxp TUs, all on that one spelling).
//
// The model deliberately MATCHES the QualType path, Converter::VisitReferenceType
// (converter.cpp:352-356), which is exactly:
//     StrCat(pointee.isConstQualified() ? "*const" : "*mut"); Convert(pointee);
// so `const T &` -> `*const <T>` and `T &` -> `*mut <T>`. Three consequences of
// matching it, none of them assumptions:
//   * It is MODEL-BLIND -- there is no `switch (model_)` in it, unlike
//     VisitPointerType's callers and addBuiltinTypes' `T *` rules (which give
//     refcount `Ptr<T>`). A C++ REFERENCE is a borrow the refcount model does
//     not own, so both models get the same raw pointer here. Doing otherwise
//     would make the two paths disagree for one C++ type.
//   * An RVALUE reference gets the same treatment: VisitReferenceType is on the
//     ReferenceType base and never distinguishes, so `T &&` -> `*mut <T>`.
//   * NO `dyn`: the abstract-struct `dyn` insertion lives in VisitPointerType
//     only (converter.cpp:388-391), not in VisitReferenceType.
//
// The `&` must be the LAST character after trimming, and anything that cannot be
// parsed with certainty returns nullopt and falls through to the existing loud
// path rather than guessing -- a wrong strip would silently produce a wrong type.
// In particular `T *const &` (reference to a const pointer) BAILS: its pointee
// spelling `T *const` is not a key any rule carries, and deciding what to do
// with the trailing `const` by guessing is exactly the silent wrongness this
// project exists to stop.
std::optional<std::string>
tryDeriveReferenceType(const std::string &cpp_type) {
  auto trim = [](std::string s) {
    while (!s.empty() && isspace(static_cast<unsigned char>(s.front()))) {
      s.erase(s.begin());
    }
    while (!s.empty() && isspace(static_cast<unsigned char>(s.back()))) {
      s.pop_back();
    }
    return s;
  };

  std::string s = trim(cpp_type);
  if (s.empty() || s.back() != '&') {
    return std::nullopt;
  }
  s.pop_back();
  if (!s.empty() && s.back() == '&') { // `T &&`
    s.pop_back();
  }
  s = trim(s);
  // A third `&`, or nothing left, is a shape we do not understand.
  if (s.empty() || s.back() == '&') {
    return std::nullopt;
  }

  // Is there a `*` (or `[`) at the TOP level? If so a leading `const ` binds to
  // the POINTEE of that pointer, not to the reference's own referent:
  // `const X *&` is a reference to a MUTABLE pointer-to-const, so it is `*mut`
  // and the whole `const X *` is what must be mapped. A `*` inside template
  // arguments -- `const X<Y *> &` -- is NOT top level and must not fool us.
  bool top_level_ptr = false;
  {
    int depth = 0;
    for (char c : s) {
      if (c == '<' || c == '(' || c == '[') {
        ++depth;
      } else if (c == '>' || c == ')' || c == ']') {
        --depth;
      } else if ((c == '*' || c == '[') && depth == 0) {
        top_level_ptr = true;
      }
    }
  }

  // `T *const` / `T const`: a TRAILING const. Bail loudly rather than guess.
  {
    const std::string kConst = "const";
    if (s.size() >= kConst.size() &&
        s.compare(s.size() - kConst.size(), kConst.size(), kConst) == 0 &&
        (s.size() == kConst.size() ||
         !(isalnum(static_cast<unsigned char>(s[s.size() - kConst.size() - 1])) ||
           s[s.size() - kConst.size() - 1] == '_'))) {
      return std::nullopt;
    }
  }

  const std::string kConstPrefix = "const ";
  bool is_const = false;
  std::string pointee = s;
  if (!top_level_ptr && s.compare(0, kConstPrefix.size(), kConstPrefix) == 0) {
    is_const = true;
    pointee = trim(s.substr(kConstPrefix.size()));
  }
  if (pointee.empty()) {
    return std::nullopt;
  }

  PushMapContext ctx(cpp_type, map_ctx_.outer_type);
  return (is_const ? "*const " : "*mut ") + mapTypeStringRecursive(pointee);
}

// A TRAILING `*` IS DECORATION, AND THE ONLY MISSING MEMBER OF THIS FAMILY.
// The QualType path already decomposes a pointer (VisitPointerType,
// mapper.cpp:1618 `t->getAs<clang::PointerType>()`), and addBuiltinTypes
// synthesises `T *`/`const T *` per SCALAR (:644-646) -- but a template
// ARGUMENT arrives here as a bare STRING with no decl and no QualType, so
// `const mlir::ktdf_arch::Device *` was searched in types_ as one opaque key,
// missed, and then could not even reach the project-leaf branch below: that
// branch keys on `user_tags_`, whose entries are BARE spellings
// (`mlir::ktdf_arch::Device`), so the decorated spelling never matches and a
// PORTED project type aborted the TU.
//
// MEASURED: this was the largest single wall in the corpus -- 9 of an 18-TU
// first-abort sample died on exactly
//   LLVM ERROR: unsupported unmapped type `const mlir::ktdf_arch::Device *`
//   has no model in types_, while mapping
//   `llvm::DenseMap<std::pair<const mlir::ktdf_arch::Device *, mlir::TypeID>,
//   std::unique_ptr<...>>`
// and four separately-filed queue rows (g724 `const std::pair<const long,
// VariableDefinition::ExprType> *`, g807 `std::map<int, ProgramAndStateInfo>
// *`, g809 `std::pair<long,long> *`, g811 `std::vector<InstrInfo> *`) are the
// SAME defect with a system pointee instead of a project one.
//
// Same three caveats as tryDeriveReferenceType directly above, for the same
// reasons: no `dyn` for an abstract pointee (that lives in VisitPointerType),
// and a TRAILING const (`T *const`) BAILS rather than guess. Ordered AFTER the
// types_ search, so every `T *` a rule already models (`mlir::Operation *` ->
// `*mut fmt::OpInst`, every scalar) still wins and nothing existing moves.
std::optional<std::string> tryDerivePointerType(const std::string &cpp_type) {
  auto trim = [](std::string s) {
    while (!s.empty() && isspace(static_cast<unsigned char>(s.front()))) {
      s.erase(s.begin());
    }
    while (!s.empty() && isspace(static_cast<unsigned char>(s.back()))) {
      s.pop_back();
    }
    return s;
  };

  std::string s = trim(cpp_type);
  if (s.empty() || s.back() != '*') {
    return std::nullopt;
  }
  s.pop_back(); // strip exactly ONE level; recursion handles `T **`.
  s = trim(s);
  if (s.empty()) {
    return std::nullopt;
  }

  // Does a `*` remain at the TOP level? If so a leading `const ` binds to the
  // pointee of THAT inner pointer, not to this one: `const X **` is a mutable
  // pointer to a `const X *`, i.e. `*mut *const X`. A `*` inside template
  // arguments (`std::vector<X *> *`) is NOT top level and must not fool us --
  // that is the g811 shape and getting it wrong would silently drop a const.
  bool inner_top_level_ptr = false;
  {
    int depth = 0;
    for (char c : s) {
      if (c == '<' || c == '(' || c == '[') {
        ++depth;
      } else if (c == '>' || c == ')' || c == ']') {
        --depth;
      } else if (c == '*' && depth == 0) {
        inner_top_level_ptr = true;
      }
    }
  }

  // `T *const` / `T const`: a TRAILING const. Bail loudly rather than guess,
  // exactly as tryDeriveReferenceType does -- `T *const` is not a key any rule
  // carries and inventing a stripping order here is the silent wrongness this
  // project exists to stop.
  {
    const std::string kConst = "const";
    if (s.size() >= kConst.size() &&
        s.compare(s.size() - kConst.size(), kConst.size(), kConst) == 0 &&
        (s.size() == kConst.size() ||
         !(isalnum(static_cast<unsigned char>(s[s.size() - kConst.size() - 1])) ||
           s[s.size() - kConst.size() - 1] == '_'))) {
      return std::nullopt;
    }
  }

  const std::string kConstPrefix = "const ";
  bool is_const = false;
  std::string pointee = s;
  if (!inner_top_level_ptr && s.compare(0, kConstPrefix.size(), kConstPrefix) == 0) {
    is_const = true;
    pointee = trim(s.substr(kConstPrefix.size()));
  }
  if (pointee.empty()) {
    return std::nullopt;
  }

  // A POINTER TO AN ABSTRACT PROJECT RECORD MUST CARRY `dyn`, and this is the
  // one place on the derive path that can know it: we hold the pointee's own
  // decl. AddRuleForUserDefinedType already does exactly this for every project
  // record it registers (`*mut dyn N` in the unsafe model, `PtrDyn<dyn N>` in
  // refcount -- see the isAbstract() branch there); a derived pointer that
  // returned a plain `*mut N` for an abstract `N` would be a dyn-less pointer
  // to a trait, i.e. silently wrong Rust instead of a loud abort.
  // `FoldFunction<Dtype>` (pure-virtual getData/insertData) is exactly that
  // shape, and it is why this is REQUIRED and not a refinement.
  if (const clang::TagDecl *tag = nullptr;
      LooksLikeUserDefinedTypeName(pointee, &tag)) {
    const auto *cxx = llvm::dyn_cast<clang::CXXRecordDecl>(tag);
    const clang::CXXRecordDecl *def =
        cxx != nullptr ? cxx->getDefinition() : nullptr;
    if (def != nullptr && def->isAbstract()) {
      const std::string rs_name = ToRustName(ToString(GetTypeForDecl(tag)));
      switch (model_) {
      case Model::kUnsafe:
        return (is_const ? "*const dyn " : "*mut dyn ") + rs_name;
      case Model::kRefCount:
        return "PtrDyn<dyn " + rs_name + '>';
      }
    }
  }
  PushMapContext ctx(cpp_type, map_ctx_.outer_type);
  return (is_const ? "*const " : "*mut ") + mapTypeStringRecursive(pointee);
}

// A TOP-LEVEL `const ` IS DECORATION WITH NO RUST SPELLING AT ALL.
// THE PROOF THAT NOTHING STRIPS IT TODAY is addBuiltinTypes (:639): it registers
// `AddTypeRule("const " + cxx, ...)` as a LITERAL DUPLICATE beside every scalar,
// which is only necessary because a `const ` spelling never reaches the
// unqualified key. A rule-authored key gets no such twin, so every rule-mapped
// type was UNREACHABLE under a `const` spelling even though the type is modelled.
//
// MEASURED: `mlir::Value` is mapped (rules/mlir t4 -> `ir::Value`), yet
//   LLVM ERROR: unsupported unmapped type `const mlir::Value` has no model in
//   types_
// terminated dcc/src/Transform/Sentient/ScalarCopyInsertionForSymbols.cpp,
// reached as `std::optional<const mlir::Value>`. A scan of the census logs'
// TERMINATING aborts found 5 distinct `const ...` spellings across 6 TUs
// (`const mlir::Value` x2, `const std::string`, `const PrimaryDimTypes`,
// `const StringKey<DsKey>`, `const std::pair<std::vector<PcfgLccrCond2>, long>`)
// and EVERY ONE of them is this shape -- a top-level const, no decoration.
//
// THE DERIVED TYPE IS THE UNQUALIFIED ONE, UNCHANGED, AND THAT IS DELIBERATE.
// In Rust immutability is a property of the BINDING (`let` vs `let mut`) and of
// the reference (`&T` vs `&mut T`), never of the value type: there is no `const
// i32` distinct from `i32`. So a `const X` VALUE and an `X` value are the same
// Rust type and the honest derivation is "map the unqualified spelling and return
// it verbatim". The two shapes where the const IS observable in Rust are `const X
// &` and `const X *`, and those are NOT ours: tryDeriveReferenceType and
// tryDerivePointerType above already read the same leading `const ` and turn it
// into `*const` rather than `*mut`. This function must therefore never see them.
//
// Two ways to get that wrong, each of which cost a measured bug in the pointer
// case (c9b6ffc):
//  * `const X *` is a pointer-to-CONST, not a const pointer -- the const binds to
//    the pointee, so stripping it here would hand `*mut X` to the pointer derive
//    and SILENTLY DROP the const. We bail whenever a `*`, `&` or `[` remains at
//    the TOP level.
//  * the scan must be DEPTH-AWARE, so a `const`/`*` inside template arguments
//    cannot fool it: `const std::vector<const X *>` IS a const vector and must
//    strip (the inner `*` is at depth 1), while `std::vector<const X *>` does not
//    begin with `const ` at all and is never a candidate.
// Ordered AFTER the types_ search, so every `const T` a rule models explicitly
// still wins -- including addBuiltinTypes' deliberate `const `+scalar twins, which
// keep resolving to themselves and are untouched by this.
std::optional<std::string> tryDeriveTopLevelConstType(const std::string &cpp_type) {
  auto trim = [](std::string s) {
    while (!s.empty() && isspace(static_cast<unsigned char>(s.front()))) {
      s.erase(s.begin());
    }
    while (!s.empty() && isspace(static_cast<unsigned char>(s.back()))) {
      s.pop_back();
    }
    return s;
  };

  std::string s = trim(cpp_type);
  const std::string kConstPrefix = "const ";
  if (s.compare(0, kConstPrefix.size(), kConstPrefix) != 0) {
    return std::nullopt;
  }
  std::string unqualified = trim(s.substr(kConstPrefix.size()));
  if (unqualified.empty()) {
    return std::nullopt;
  }

  // Any top-level `*`/`&`/`[` means the const we just stripped was NOT ours: it
  // binds to a pointee or referent and the pointer/reference derive owns it.
  // Depth counting is what keeps `std::vector<const X *>` out of the decision.
  {
    int depth = 0;
    for (char c : unqualified) {
      if (c == '<' || c == '(' || c == '[') {
        if (c == '[' && depth == 0) {
          return std::nullopt;
        }
        ++depth;
      } else if (c == '>' || c == ')' || c == ']') {
        --depth;
      } else if ((c == '*' || c == '&') && depth == 0) {
        return std::nullopt;
      }
    }
  }

  PushMapContext ctx(cpp_type, map_ctx_.outer_type);
  return mapTypeStringRecursive(unqualified);
}

std::string mapTypeStringRecursive(const std::string &cpp_type) {
  auto [rule, subs] = search(types_, cpp_type, GetTypeMapKey(cpp_type));
  if (!rule) {
    // Before ANY fallback or failure: a function-pointer spelling gets a
    // structurally derived model. Ahead of the survey branch too, because a
    // derivable type is not a gap and recording it as one would keep a
    // now-solved row on the work queue.
    if (auto derived = tryDeriveFunctionPointerType(cpp_type)) {
      return *derived;
    }
    // Same reasoning, one shape up: a trailing `&`/`&&` is structural.
    if (auto derived = tryDeriveReferenceType(cpp_type)) {
      return *derived;
    }
    // Same reasoning, and the last shape in the family: a trailing `*`.
    if (auto derived = tryDerivePointerType(cpp_type)) {
      return *derived;
    }
    // And the one member of the family that was missing: a LEADING `const ` with
    // nothing decorated left of it. Last in the family so the three decoration
    // derives above keep first refusal on `const X *` / `const X &`.
    if (auto derived = tryDeriveTopLevelConstType(cpp_type)) {
      return *derived;
    }
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
      // THE NAME MUST COME FROM THE DECL, NEVER FROM `cpp_type`. What this TU
      // will actually DEFINE is GetRecordName's `ToRustName(ToString(
      // GetTypeForDecl(decl)))` (converter.cpp:4755-4760), and for a template
      // specialisation that DESUGARS: the leaf string
      // `FoldFunction<std::vector<long>>` mangles to
      // `FoldFunction_std_vector_long__`, but VisitRecordDecl emits
      // `FoldFunction_std_vector_long__std_allocator_long___`. Returning
      // ToRustName(cpp_type) there would turn a LOUD abort into rc=0 plus
      // E0412 -- strictly worse. The two strings are equal for a
      // non-template tag, which is why this is a no-op for the enum and
      // plain-record cases that already worked.
      const std::string ported = ToRustName(ToString(GetTypeForDecl(tag)));
      static std::set<std::string> ported_leaves;
      if (ported_leaves.insert(cpp_type).second) {
        llvm::errs() << "note: project leaf type `" << cpp_type
                     << "` has no types_ entry yet; emitting its PORTED name `"
                     << ported << "` (declared at "
                     << (ctx_ != nullptr
                             ? tag->getLocation().printToString(
                                   ctx_->getSourceManager())
                             : std::string("<no ASTContext>"))
                     << ")\n";
      }
      return ported;
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
    // NDEBUG: the release build compiles `assert(0 && ...)` to NOTHING, so this
    // used to FALL THROUGH to the `rule->type_info.type` below with
    // `rule == nullptr` -- forming a null `const std::string&` that
    // instantiateTgt copies, i.e. the rc=139 segfault this assert existed to
    // prevent. It must be a failure that SURVIVES NDEBUG.
    llvm::report_fatal_error(
        llvm::Twine("unsupported unmapped ") + DescribeUnmappedLeaf(cpp_type) +
            ": type `" + cpp_type + "` is not present in types_ (rule key `" +
            GetTypeMapKey(cpp_type) + "`)",
        /*gen_crash_diag=*/false);
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
  if (auto it = user_tags_.find(cpp_type); it != user_tags_.end()) {
    if (decl != nullptr) {
      *decl = it->second;
    }
    return true;
  }
  // SECOND CHANCE, for a template specialisation. `user_tags_` deliberately
  // skips ClassTemplateSpecializationDecl because its key desugars and would
  // never match; the sugared index below is built from the OUTER QualType being
  // mapped, so its keys are the same spellings a leaf carries. Rebuilt whenever
  // the outer type changes -- the walk is bounded and only runs after a miss.
  if (!map_ctx_.outer_type.isNull()) {
    CollectSugaredTags(map_ctx_.outer_type, 0);
  }
  if (auto it = sugared_tags_.find(cpp_type); it != sugared_tags_.end()) {
    if (decl != nullptr) {
      *decl = it->second;
    }
    return true;
  }
  return false;
}

PushASTContext::PushASTContext(clang::ASTContext &ctx) : prev_(ctx_) {
  ctx_ = &ctx;
  // The tag index holds decls owned by the OLD context; never let it outlive it.
  user_tags_.clear();
  user_tags_built_ = false;
  sugared_tags_.clear();
}
PushASTContext::~PushASTContext() {
  ctx_ = prev_;
  user_tags_.clear();
  user_tags_built_ = false;
  sugared_tags_.clear();
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

bool ParamIsMutRef(const clang::Expr *expr, unsigned index) {
  auto rule = search(expr);
  if (!rule || index >= rule->params.size()) {
    return false;
  }
  return rule->params[index].is_mut_ref();
}

bool ParamIsSharedRef(const clang::Expr *expr, unsigned index) {
  auto rule = search(expr);
  if (!rule || index >= rule->params.size()) {
    return false;
  }
  return rule->params[index].is_shared_ref();
}

bool ReturnsMutRef(const clang::Expr *expr) {
  auto rule = search(expr);
  return rule && rule->return_type.is_mut_ref();
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
    // NDEBUG: `assert(0 && "!expr")` compiles to NOTHING in the release build,
    // so this used to fall straight into `expr->IgnoreParenImpCasts()` -- a
    // NULL DEREFERENCE, i.e. the rc=139 segfault the assert existed to prevent.
    // Deliberately NOT a survey::Record: a null Expr is an internal invariant
    // violation, not a translation gap -- there is no C++ construct that "is" a
    // null expression, and there is no spelling or location to record.
    llvm::report_fatal_error(
        "internal: Mapper::ToString(const clang::Expr *) called with a null "
        "expression");
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
