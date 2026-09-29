// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include "converter/translation_rule.h"

#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/JSON.h>
#include <llvm/Support/MemoryBuffer.h>

#include <algorithm>
#include <string>
#include <vector>

#include "logging.h"

namespace cpp2rust::TranslationRule {

namespace {

// The rule-module name a JSON path belongs to, i.e. the directory holding it:
// `<tree>/vector/ir_src.json` -> `vector`. Every load-time refusal below names
// it, because the failure mode these refusals replace is ANONYMOUS: a bare
// `return` (an `assert(0)` compiled out by -DNDEBUG) silently discards a whole
// module's rules, and the author of a freshly committed key sees only that the
// key is dead.
std::string ModuleOf(const std::filesystem::path &json_path) {
  auto parent = json_path.parent_path().filename().string();
  return parent.empty() ? json_path.string() : parent;
}

TypeInfo ParseTypeInfoJSON(const llvm::json::Object &obj) {
  TypeInfo info;
  if (auto ty = obj.getString("type"))
    info.type = ty->str();
  if (auto v = obj.getBoolean("is_refcount_pointer"))
    info.is_refcount_pointer = *v;
  if (auto v = obj.getBoolean("is_unsafe_pointer"))
    info.is_unsafe_pointer = *v;
  assert(!(info.is_refcount_pointer && info.is_unsafe_pointer));
  if (auto *arr = obj.getArray("derives")) {
    for (const auto &elem : *arr) {
      if (auto s = elem.getAsString())
        info.derives.emplace_back(s->str());
    }
  }
  return info;
}

Access ParseAccessJSON(llvm::StringRef value, llvm::StringRef ctx) {
  if (value == "borrow") {
    return Access::kBorrow;
  } else if (value == "borrow_mut") {
    return Access::kBorrowMut;
  } else if (value == "move") {
    return Access::kMove;
  } else if (value == "take") {
    return Access::kTake;
  } else {
    // Falling back to kBorrow here would leave a LIVE rule with a silently
    // wrong access mode, so refuse instead of guessing.
    llvm::report_fatal_error(
        llvm::Twine("cpp2rust: rule module '") + ctx +
            "': invalid access value '" + value +
            "' in rule body (expected borrow|borrow_mut|move|take); "
            "re-run cpp-rule-preprocessor for this module",
        /*gen_crash_diag=*/false);
  }
}

PlaceholderFragment
ParsePlaceholderFragmentJSON(const llvm::json::Object &obj,
                             llvm::StringRef ctx) {
  auto access = obj.getString("access");
  return {
      (unsigned)*obj.getInteger("arg"),
      ParseAccessJSON(*access, ctx),
      obj.getBoolean("is_index_base").value_or(false),
  };
}

std::vector<BodyFragment> ParseBodyFragmentsJSON(const llvm::json::Array &arr,
                                                 llvm::StringRef ctx);

MethodCallFragment ParseMethodCallFragmentJSON(const llvm::json::Object &obj,
                                               llvm::StringRef ctx) {
  MethodCallFragment mc;
  if (auto *receiver = obj.getArray("receiver")) {
    mc.receiver = ParseBodyFragmentsJSON(*receiver, ctx);
  }
  if (auto *body = obj.getArray("body")) {
    mc.body = ParseBodyFragmentsJSON(*body, ctx);
  }
  return mc;
}

std::vector<BodyFragment> ParseBodyFragmentsJSON(const llvm::json::Array &arr,
                                                 llvm::StringRef ctx) {
  std::vector<BodyFragment> result;
  for (auto &frag : arr) {
    auto *frag_obj = frag.getAsObject();
    if (!frag_obj)
      continue;
    if (auto str = frag_obj->getString("text")) {
      result.push_back(TextFragment{str->str()});
    } else if (auto n = frag_obj->getInteger("generic")) {
      result.push_back(GenericFragment{(unsigned)*n});
    } else if (auto *ph = frag_obj->getObject("placeholder")) {
      result.push_back(ParsePlaceholderFragmentJSON(*ph, ctx));
    } else if (auto *mc = frag_obj->getObject("method_call")) {
      result.push_back(std::make_unique<MethodCallFragment>(
          ParseMethodCallFragmentJSON(*mc, ctx)));
    } else if (frag_obj->get("va_args")) {
      result.push_back(VaArgsFragment{});
    } else if (frag_obj->get("init")) {
      result.push_back(InitFragment{});
    }
  }
  return result;
}

ExprRule ParseExprRuleJSON(const llvm::json::Object &obj,
                           llvm::StringRef ctx) {
  ExprRule ir;

  if (auto *params = obj.getObject("params")) {
    for (auto &[key, val] : *params) {
      if (auto *param_obj = val.getAsObject()) {
        size_t id = atoi(llvm::StringRef(key).data() + 1);
        ir.params.resize(std::max(ir.params.size(), id + 1));
        ir.params[id] = ParseTypeInfoJSON(*param_obj);
      }
    }
  }

  if (auto *rt = obj.getObject("return_type")) {
    ir.return_type = ParseTypeInfoJSON(*rt);
  }

  if (auto ms = obj.getBoolean("multi_statement")) {
    ir.multi_statement = *ms;
  }

  if (auto v = obj.getBoolean("is_extern")) {
    ir.is_extern = *v;
  }

  if (auto *generics = obj.getObject("generics")) {
    for (auto &[key, val] : *generics) {
      if (auto *arr = val.getAsArray()) {
        std::vector<std::string> bounds;
        for (auto &b : *arr) {
          if (auto s = b.getAsString())
            bounds.push_back(s->str());
        }
        size_t id = atoi(llvm::StringRef(key).data() + 1) - 1; // starts in T1
        ir.generics.resize(std::max(ir.generics.size(), id + 1));
        ir.generics[id] = std::move(bounds);
      }
    }
  }

  if (auto *body = obj.getArray("body")) {
    ir.body = ParseBodyFragmentsJSON(*body, ctx);
  }

  return ir;
}

TypeRule ParseTypeRuleJSON(const llvm::json::Object &obj) {
  TypeRule rule;
  if (auto init = obj.getString("init"))
    rule.initializer = init->str();
  rule.type_info = ParseTypeInfoJSON(obj);
  return rule;
}

void LoadTgtFromIR(ExprRules &exprs, TypeRules &types,
                   const std::filesystem::path &json_path) {
  auto buf = llvm::MemoryBuffer::getFile(json_path.string());
  if (!buf)
    return;

  auto parsed = llvm::json::parse((*buf)->getBuffer());
  if (!parsed) {
    // A bare return here would drop EVERY rule of this module, anonymously.
    llvm::report_fatal_error(llvm::Twine("cpp2rust: rule module '") +
                                 ModuleOf(json_path) + "': malformed " +
                                 json_path.filename().string() + " (" +
                                 llvm::toString(parsed.takeError()) +
                                 "); all of this module's rules would be "
                                 "silently discarded -- re-run "
                                 "cpp-rule-preprocessor for this module",
                             /*gen_crash_diag=*/false);
  }

  auto *root = parsed->getAsObject();
  if (!root)
    return;

  for (auto &[entry_name, entry_val] : *root) {
    auto *obj = entry_val.getAsObject();
    if (!obj)
      continue;

    auto name = entry_name.str();
    if (name[0] == 'f') {
      exprs[name] = ParseExprRuleJSON(*obj, ModuleOf(json_path) + "/" + name);
    } else if (name[0] == 't') {
      types[std::move(name)] = ParseTypeRuleJSON(*obj);
    }
  }
}

void LoadIrSrc(ExprRules &exprs, TypeRules &types,
               const std::filesystem::path &json_path) {
  auto buf = llvm::MemoryBuffer::getFile(json_path.string());
  if (!buf) {
    // THE "my committed key is dead" TRAP: without ir_src.json no rule of this
    // module can ever match, and the old bare return said nothing.
    llvm::report_fatal_error(
        llvm::Twine("cpp2rust: rule module '") + ModuleOf(json_path) +
            "': missing " + json_path.string() +
            "; none of this module's rules can match -- run "
            "cpp-rule-preprocessor for this module",
        /*gen_crash_diag=*/false);
  }

  auto parsed = llvm::json::parse((*buf)->getBuffer());
  if (!parsed) {
    llvm::report_fatal_error(llvm::Twine("cpp2rust: rule module '") +
                                 ModuleOf(json_path) + "': malformed " +
                                 json_path.filename().string() + " (" +
                                 llvm::toString(parsed.takeError()) +
                                 "); none of this module's rules can match -- "
                                 "re-run cpp-rule-preprocessor for this module",
                             /*gen_crash_diag=*/false);
  }

  auto *root = parsed->getAsObject();
  if (!root) {
    return;
  }

  for (auto &[entry_name, entry_val] : *root) {
    auto name = entry_name.str();
    auto val = entry_val.getAsString();
    if (name[0] == 'f') {
      auto it = exprs.find(name);
      if (it == exprs.end()) {
        // Writing through the end iterator below is UNDEFINED BEHAVIOUR; this
        // is the documented "rc=139 with no message".
        llvm::report_fatal_error(
            llvm::Twine("cpp2rust: rule module '") + ModuleOf(json_path) +
                "': expr key '" + name + "' is in ir_src.json but in no IR "
                "target (ir_unsafe.json/ir_refcount.json); the rule tree is "
                "inconsistent -- re-run cpp-rule-preprocessor for this module",
            /*gen_crash_diag=*/false);
      }
      if (auto *obj = entry_val.getAsObject()) {
        it->second.src = obj->getString("key")->str();
        auto *init_type = obj->getObject("init_type");
        assert(init_type && "ir_src.json expr entry object without init_type");
        it->second.init_type = InitTypeLocation{
            (unsigned)*init_type->getInteger("depth"),
            (unsigned)*init_type->getInteger("index"),
            {},
        };
        // Absent for an ir_src.json written before nested init types existed;
        // an absent path is the empty path, i.e. the old meaning exactly.
        if (auto *path = init_type->getArray("path")) {
          for (const auto &step : *path) {
            auto n = step.getAsInteger();
            if (!n) {
              llvm::report_fatal_error(
                  llvm::Twine("cpp2rust: rule module '") +
                      ModuleOf(json_path) + "': expr key '" + name +
                      "' has a non-integer step in its init_type path -- "
                      "re-run cpp-rule-preprocessor for this module",
                  /*gen_crash_diag=*/false);
            }
            it->second.init_type.path.push_back((unsigned)*n);
          }
        }
        continue;
      }
      it->second.src = val->str();
    } else if (name[0] == 't') {
      auto it = types.find(name);
      if (it == types.end()) {
        llvm::report_fatal_error(
            llvm::Twine("cpp2rust: rule module '") + ModuleOf(json_path) +
                "': type key '" + name + "' is in ir_src.json but in no IR "
                "target (ir_unsafe.json/ir_refcount.json); the rule tree is "
                "inconsistent -- re-run cpp-rule-preprocessor for this module",
            /*gen_crash_diag=*/false);
      }
      it->second.src = val->str();
    }
  }
}

void BodyFragmentDump(const BodyFragment &frag) {
  if (auto *t = std::get_if<TextFragment>(&frag)) {
    t->dump();
  } else if (auto *p = std::get_if<PlaceholderFragment>(&frag)) {
    p->dump();
  } else if (auto *g = std::get_if<GenericFragment>(&frag)) {
    g->dump();
  } else if (auto *v = std::get_if<VaArgsFragment>(&frag)) {
    v->dump();
  } else if (auto *i = std::get_if<InitFragment>(&frag)) {
    i->dump();
  } else if (auto *mc =
                 std::get_if<std::unique_ptr<MethodCallFragment>>(&frag)) {
    (*mc)->dump();
  }
}

bool HasInitFragment(const std::vector<BodyFragment> &body) {
  return std::any_of(body.begin(), body.end(), [](const BodyFragment &frag) {
    if (auto *mc = std::get_if<std::unique_ptr<MethodCallFragment>>(&frag)) {
      return HasInitFragment((*mc)->receiver) || HasInitFragment((*mc)->body);
    }
    return std::holds_alternative<InitFragment>(frag);
  });
}

} // namespace

void TextFragment::dump() const { log() << "  text: \"" << text << "\"\n"; }

void VaArgsFragment::dump() const { log() << "  va_args\n"; }

void InitFragment::dump() const { log() << "  init\n"; }

void PlaceholderFragment::dump() const {
  log() << "  placeholder: " << n;
  switch (access) {
  case Access::kBorrow:
    log() << " (borrow)\n";
    break;
  case Access::kBorrowMut:
    log() << " (borrow_mut)\n";
    break;
  case Access::kMove:
    log() << " (move)\n";
    break;
  case Access::kTake:
    log() << " (take)\n";
    break;
  }
}

const PlaceholderFragment *MethodCallFragment::getReceiverPlaceholder() const {
  for (auto &frag : receiver) {
    if (auto *ph = std::get_if<PlaceholderFragment>(&frag)) {
      return ph;
    }
  }
  return nullptr;
}

void MethodCallFragment::dump() const {
  log() << "  method_call:\n"
           "    receiver:\n";
  for (const auto &frag : receiver) {
    BodyFragmentDump(frag);
  }
  log() << "    body:\n";
  for (const auto &frag : body) {
    BodyFragmentDump(frag);
  }
}

void ExprRule::dump() const {
  log() << "Matching: " << src << '\n';
  if (init_type.valid()) {
    log() << "  init type: depth " << init_type.depth << ", index "
          << init_type.index;
    for (unsigned step : init_type.path) {
      log() << ", nested arg " << step;
    }
    log() << '\n';
  }
  unsigned i = 0;
  for (auto &info : params) {
    log() << "  param a" << i++ << ": ";
    info.dump();
    log() << '\n';
  }
  if (!return_type.type.empty()) {
    log() << "  return: ";
    return_type.dump();
    log() << '\n';
  }
  i = 0;
  for (auto &bounds : generics) {
    log() << "  generic T" << ++i << ':';
    for (auto &b : bounds) {
      log() << ' ' << b;
    }
    log() << '\n';
  }
  for (const auto &frag : body) {
    BodyFragmentDump(frag);
  }
}

void ExprRule::validate(const std::string &name) const {
  if (src.empty()) {
    llvm::errs() << name << '\n';
    dump();
    llvm::report_fatal_error("Expr rule loaded from IR but has no src");
  }

  if (HasInitFragment(body) && !init_type.valid()) {
    llvm::errs() << name << '\n';
    dump();
    llvm::report_fatal_error(
        "Expr rule uses init but its src pack is not declared as Init<T, "
        "Args>");
  }

  if (generics.empty())
    return;

  // Sized off the rule's own generic count, never off a fixed bound: the
  // validation loop below indexes this by `i < generics.size()`, and a rule
  // with more generics than a fixed array's extent read out of bounds.
  std::vector<bool> has_generic(generics.size(), false);
  for (size_t i = 0, e = src.size(); i < e; ++i) {
    auto pos = src.find('T', i);
    if (pos == std::string::npos)
      break;
    // Parse the FULL multi-digit index. A single-digit scan misreads `T27` as
    // `T2` and then never marks T27 as present, which makes every rule with
    // more than 9 generics fail the "absent generic from src" check below.
    size_t digits = pos + 1;
    while (digits < e && src[digits] >= '0' && src[digits] <= '9')
      ++digits;
    if (digits == pos + 1 || src[pos + 1] == '0') {
      i = pos;
      continue;
    }
    size_t n = std::stoul(src.substr(pos + 1, digits - (pos + 1)));
    if (n >= 1 && n <= has_generic.size())
      has_generic[n - 1] = true;
    i = digits - 1;
  }

  for (size_t i = 0, e = generics.size(); i < e; ++i) {
    if (!has_generic[i]) {
      llvm::errs() << name << '\n';
      dump();
      llvm::errs() << "generic T" << (i + 1)
                   << " declared but missing from src: " << src << '\n';
      llvm::report_fatal_error("Absent generic from src");
    }
  }
}

void GenericFragment::dump() const { log() << "  generic: " << n << '\n'; }

void TypeInfo::dump() const {
  log() << type;
  if (is_refcount_pointer)
    log() << " [rc_ptr]";
  if (is_unsafe_pointer)
    log() << " [unsafe_ptr]";
  for (const auto &d : derives)
    log() << " +" << d;
}

void TypeRule::dump() const {
  log() << "name: " << src << "\n  Rust type: ";
  type_info.dump();
  log() << '\n';
  if (!initializer.empty()) {
    log() << "  init: " << initializer << '\n';
  }
}

std::pair<ExprRules, TypeRules> Load(const std::filesystem::path &dir,
                                     Model model) {
  ExprRules exprs;
  TypeRules types;
  LoadTgtFromIR(exprs, types, dir / "ir_unsafe.json");

  if (model == Model::kRefCount) {
    auto refcount_ir_path = dir / "ir_refcount.json";
    if (std::filesystem::exists(refcount_ir_path)) {
      LoadTgtFromIR(exprs, types, refcount_ir_path);
    }
  }

  LoadIrSrc(exprs, types, dir / "ir_src.json");

  for (auto &[name, rule] : exprs) {
    rule.validate(name);
  }
  for (auto &[name, rule] : types) {
    if (rule.src.empty()) {
      rule.dump();
      // An empty src MATCHES THE EMPTY TYPE STRING, so carrying on here makes
      // this rule fire on unrelated types. Refuse. (A type key present only in
      // ir_unsafe.json and absent from ir_refcount.json is NOT this case: it is
      // normal, and ir_src.json still gives it a src.)
      llvm::report_fatal_error(
          llvm::Twine("cpp2rust: rule module '") + dir.filename().string() +
              "': type key '" + name +
              "' has an IR target but no src in ir_src.json; an empty src "
              "would match the empty type string -- re-run "
              "cpp-rule-preprocessor for this module",
          /*gen_crash_diag=*/false);
    }
  }
  return {std::move(exprs), std::move(types)};
}

} // namespace cpp2rust::TranslationRule
