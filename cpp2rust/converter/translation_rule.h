#pragma once

// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <clang/AST/Expr.h>
#include <llvm/ADT/SmallVector.h>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "converter/factory.h"

namespace cpp2rust::TranslationRule {

// Upper bound on the `Tn` placeholder index a rule may use. The largest index
// in the committed rule IR today is T27 (rules/tuple's 27-generic type rule
// t5); a variadic `operator==` over two N-ary tuples needs 2N generics, so 20
// -ary tuple rules can plausibly reach T40. 64 leaves headroom for that and is
// only a sanity bound -- no fixed-size storage is sized by it any more.
static inline constexpr unsigned kMaxGenerics = 64;

struct TextFragment {
  std::string text;

  void dump() const;
};

enum class Access : int8_t { kBorrow, kBorrowMut, kMove, kTake };

struct PlaceholderFragment {
  unsigned n; // "a0", "a1", ...
  Access access;
  bool is_index_base = false;

  void dump() const;
};

struct GenericFragment {
  unsigned n; // "T1", "T2", ...

  void dump() const;
};

struct VaArgsFragment {
  void dump() const;
};

struct InitFragment {
  void dump() const;
};

struct MethodCallFragment; // forward declaration

using BodyFragment = std::variant<TextFragment, PlaceholderFragment,
                                  GenericFragment, VaArgsFragment, InitFragment,
                                  std::unique_ptr<MethodCallFragment>>;

struct MethodCallFragment {
  std::vector<BodyFragment> receiver;
  std::vector<BodyFragment> body;

  const PlaceholderFragment *getReceiverPlaceholder() const;
  void dump() const;
};

struct TypeInfo {
  std::vector<std::string> derives;
  std::string type;
  bool is_refcount_pointer = false;
  bool is_unsafe_pointer = false;

  bool is_pointer() const { return is_refcount_pointer || is_unsafe_pointer; }

  // True when the rule declared this parameter (or return) as a Rust MUTABLE
  // REFERENCE. There is no structured flag for this in the IR -- the JSON
  // emitted by the rule preprocessor carries `is_refcount_pointer` /
  // `is_unsafe_pointer` and nothing else -- but `type` is the target's declared
  // Rust type verbatim (`"&mut Vec<T1>"`), so the leading `&mut` IS the record,
  // not a guess about a mapped type. A generic substitution `T1` can never
  // introduce a leading `&mut` (rule generics are bare type params), so this
  // does not need instantiation to be correct.
  // A lifetime is part of the spelling whenever the target ties its return to
  // an argument (`&'a mut T`), which a reference-returning rule MUST do, so the
  // optional `'name` has to be skipped here.
  bool is_mut_ref() const {
    std::string_view s = type;
    if (!s.starts_with("&")) {
      return false;
    }
    s.remove_prefix(1);
    while (!s.empty() && isspace((unsigned char)s.front()))
      s.remove_prefix(1);
    if (s.starts_with("'")) {
      auto end = s.find_first_of(" \t");
      if (end == std::string_view::npos) {
        return false;
      }
      s.remove_prefix(end);
      while (!s.empty() && isspace((unsigned char)s.front()))
        s.remove_prefix(1);
    }
    return s.starts_with("mut ") || s.starts_with("mut\t");
  }

  // True when the rule declared this parameter (or return) as a Rust SHARED
  // reference -- `&T`, `&'a [u8]`, but NOT `&mut T`. Same reasoning and same
  // lifetime-skipping as is_mut_ref(): `type` is the declaration verbatim, and
  // a generic substitution can never introduce a leading `&`.
  bool is_shared_ref() const {
    std::string_view s = type;
    if (!s.starts_with("&")) {
      return false;
    }
    s.remove_prefix(1);
    // `&&T` is a shared reference too, but nothing in the converter needs to
    // add a borrow for it and treating it as one would be a guess; refuse.
    if (s.starts_with("&")) {
      return false;
    }
    while (!s.empty() && isspace((unsigned char)s.front()))
      s.remove_prefix(1);
    if (s.starts_with("'")) {
      auto end = s.find_first_of(" \t");
      if (end == std::string_view::npos) {
        return false;
      }
      s.remove_prefix(end);
      while (!s.empty() && isspace((unsigned char)s.front()))
        s.remove_prefix(1);
    }
    return !s.empty() && !s.starts_with("mut ") && !s.starts_with("mut\t");
  }

  void dump() const;
};

// Where an `Init<T, Args>` pack's `T` lives, relative to the callee's own
// template-argument list, as recorded by cpp-rule-preprocessor's
// `findTemplateArgument`.  `path` is a descent into the NESTED template
// arguments of the argument named by (depth, index) -- empty for the common
// case, `{0}` for e.g. `unordered_map`'s `pair<const K, V>`, which is only
// reachable as template argument 0 of the container's ALLOCATOR argument.
// ⛔ An ir_src.json written before `path` existed simply has no "path" member;
// an absent path is the empty path and means exactly what it used to.
struct InitTypeLocation {
  unsigned depth = -1u;
  unsigned index = -1u;
  llvm::SmallVector<unsigned, 2> path;

  bool valid() const { return depth != -1u; }
};

struct ExprRule {
  std::string src;
  InitTypeLocation init_type;
  std::vector<TypeInfo> params;
  TypeInfo return_type;
  std::vector<std::vector<std::string>> generics; // "T1" -> ["Ord", "Clone"]
  std::vector<BodyFragment> body;
  bool multi_statement = false;
  bool is_extern = false;

  void dump() const;
  void validate(const std::string &name) const;
};

struct TypeRule {
  std::string src;
  std::string initializer; // Rust initializer expression
  TypeInfo type_info;

  void dump() const;

  static TypeRule Plain(std::string type) {
    return {{}, {}, {{}, std::move(type), false, false}};
  }
  static TypeRule RefcountPtr(std::string type) {
    return {{}, {}, {{}, std::move(type), true, false}};
  }
  static TypeRule UnsafePtr(std::string type) {
    return {{}, {}, {{}, std::move(type), false, true}};
  }
};

// ⭐⭐ A DELIBERATELY REFUSED MEMBER -- the third state RULE 2 asks for and that
// the converter could not previously express.
//
// RULE 2 says: "if a thing cannot be written correctly, leave the key out and
// make it FAIL LOUDLY at translate time." ⛔ For a MEMBER those two halves were
// UNREACHABLE AT THE SAME TIME. `Mapper::Contains(expr->getCallee())` returning
// false does not abort: VisitCallExpr falls through to generic
// `ConvertCallExpr`, which prints the literal C++ method name, so an absent
// member key yields rc=0, a plausible `.rs`, and a rustc E0599 one stage later
// that no census in this project can observe. Measured four independent times
// (`std::hash<int>` -> `0(v)`, 427 sites / 190 TUs; the fabricated `::new_N`
// ctor class, 31,369 sites; g3055's `.getNumDims()`; and t85 IntegerSet's own
// comment claiming it "aborts loudly", which is FALSE).
//
// A RefusedRule is a key that exists in `ir_src.json` with `"refused": true`
// and has NO target body in any model. It is NOT loaded into `exprs_`, so
// `Mapper::Contains` stays false and every existing dispatch decision is
// unchanged; it is loaded into a separate bucket that `Mapper::GetRefusedRule`
// consults, and VisitCallExpr aborts loudly when a call site matches one.
//
// ⛔ OPT-IN ONLY, and that is the whole safety argument: an unmapped member that
// no rule mentions is not refused, it is simply absent, and behaves exactly as
// before. With no rule marked refused this container is empty and the converter
// cannot behave differently from one built without it.
struct RefusedRule {
  std::string src;    // the C++ key spelling, same form as ExprRule::src
  std::string origin; // "<module>/<key name>", quoted in the abort message
};

using ExprRules = std::unordered_map<std::string, ExprRule>;
using TypeRules = std::unordered_map<std::string, TypeRule>;
using RefusedRules = std::vector<RefusedRule>;

std::pair<ExprRules, TypeRules> Load(const std::filesystem::path &dir,
                                     Model model,
                                     RefusedRules *refused = nullptr);
} // namespace cpp2rust::TranslationRule
