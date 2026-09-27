#pragma once

// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Type.h>

#include <string>

#include "converter/factory.h"
#include "converter/translation_rule.h"

namespace cpp2rust::Mapper {
class PushASTContext {
public:
  explicit PushASTContext(clang::ASTContext &ctx);
  ~PushASTContext();
  PushASTContext(const PushASTContext &) = delete;
  PushASTContext &operator=(const PushASTContext &) = delete;

private:
  clang::ASTContext *prev_;
};

bool Contains(clang::QualType qual_type);
bool Contains(const clang::Expr *expr);

std::string Map(clang::QualType qual_type);
std::string MapInitializer(clang::QualType qual_type);
const TranslationRule::ExprRule *GetExprRule(const clang::Expr *expr);
bool IsLibcPassthrough(const clang::Expr *expr);
std::string MapFunctionName(const clang::FunctionDecl *decl);
std::string InstantiateTemplate(const clang::Expr *expr, unsigned n);
bool ReturnsPointer(const clang::Expr *expr);
std::string GetParamType(const clang::Expr *expr, unsigned index);
bool ParamIsPointer(const clang::Expr *expr, unsigned index);
bool MapsToPointer(clang::QualType qual_type);
bool MapsToRefcountPointer(clang::QualType qual_type);
const std::vector<std::string> *MappedDerives(clang::QualType qual_type);
void SetDerives(clang::QualType qual_type, std::vector<std::string> derives);

enum class ScalarSugar {
  kDesugar,
  kPreserve,
};

bool HasFunctionParameterPack(const clang::FunctionDecl *decl);

clang::QualType GetTypeForDecl(const clang::NamedDecl *decl);
std::string ToString(clang::QualType qual_type,
                     ScalarSugar sugar = ScalarSugar::kDesugar);
std::string ToString(const clang::Expr *expr);
std::string ToString(const clang::NamedDecl *decl);
std::string ToRustName(std::string name);

// Describes the spelling(s) the LAST failed `search(QualType)` actually looked
// up, as `searched as: X` (plus `, canonical: Y` when the canonical fallback
// differs and was therefore also tried). The diagnostics must quote THESE, not
// a spelling reconstructed from the RecordDecl: `search()` looks up the SUGARED
// spelling first and only falls back to the canonical one IF THE TWO STRINGS
// DIFFER, while `ToString(GetTypeForDecl(decl))` canonicalises and does not
// elide defaulted template args. Three separate rule authors wrote a DEAD rule
// straight off the old message.
std::string DescribeLastTypeSearch();

// True when `cpp_type` -- a bare leaf SPELLING, with no decl and no QualType in
// hand -- names a PROJECT (user-defined) tag decl in this TU, i.e. one the
// converter itself ports and emits. Out-parameter `decl` receives it.
bool LooksLikeUserDefinedTypeName(const std::string &cpp_type,
                                  const clang::TagDecl **decl = nullptr);

void LoadTranslationRules(Model model, clang::ASTContext &ctx,
                          const std::string &rules_dir);
void AddRuleForUserDefinedType(clang::NamedDecl *decl);
} // namespace cpp2rust::Mapper
