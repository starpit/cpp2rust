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

// Does the rule table hold ANY row for `op` on `record`, regardless of the
// overload's signature?
//
// `Contains(const Expr *)` cannot answer this: it asks about the ONE overload
// the call site resolved to, so a miss conflates two cases the converter must
// separate. When an operator's lookup misses, the converter falls back to a
// native Rust operator and records NO gap -- correct by construction for
// `std::vector`'s `[]`, `std::string`'s `=` and `std::ostream`'s `<<`, which are
// handled by deliberate shape-based handlers, but SILENTLY WRONG when the type
// does have operator rules and merely none that matched this signature (a
// `std::tie` assignment needs `T1 = int &` and `T1 = int` at once, so
// rules/tuple's f13/f14 cannot match and the native `=` is emitted instead).
// `obj_mapped && !callee_has_rule` is therefore not a usable discriminator;
// "rows exist for this operator on this type" is.
bool HasAnyRuleForOperator(const clang::CXXRecordDecl *record,
                           clang::OverloadedOperatorKind op);

std::string Map(clang::QualType qual_type);
std::string MapInitializer(clang::QualType qual_type);
const TranslationRule::ExprRule *GetExprRule(const clang::Expr *expr);
bool IsLibcPassthrough(const clang::Expr *expr);
std::string MapFunctionName(const clang::FunctionDecl *decl);
std::string InstantiateTemplate(const clang::Expr *expr, unsigned n);
// Spells a Rust type so it can be the base of a `::` path: bare where that
// parses (`i32`, `Outer`), qualified where it does not (`<Vec<i32>>`,
// `<Ptr<N>>`, `<(A, B)>`). A rule body writes `T1::default()` bare, which is
// only legal while T1 binds to a plain path.
std::string AsPathBase(const std::string &rust_type);
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

// Whether a printed function signature carries the template arguments that
// its own text cannot otherwise show.
//
// `std::holds_alternative<float>(v)` and `std::holds_alternative<int>(v)`
// print the SAME signature -- `bool std::holds_alternative(const
// std::variant<...> &)` -- because the parameter it is deduced from does not
// mention the argument. kInclude spells those arguments out, so the two
// calls become distinguishable; kOmit is what every existing caller wants and
// is byte-for-byte what the printer produced before this option existed.
enum class TemplateArgs {
  kOmit,
  kInclude,
};

clang::QualType GetTypeForDecl(const clang::NamedDecl *decl);
std::string ToString(clang::QualType qual_type,
                     ScalarSugar sugar = ScalarSugar::kDesugar);
std::string ToString(const clang::Expr *expr,
                     TemplateArgs targs = TemplateArgs::kOmit);
std::string ToString(const clang::NamedDecl *decl,
                     TemplateArgs targs = TemplateArgs::kOmit);
std::string ToRustName(std::string name);

void LoadTranslationRules(Model model, clang::ASTContext &ctx,
                          const std::string &rules_dir);
void AddRuleForUserDefinedType(clang::NamedDecl *decl);

// Registers a type rule for every user-defined tag reachable from `dc`.
// Rules are otherwise added only as the converter walks onto each decl, so a
// function body converted earlier than a type's declaration cannot map it.
void PreRegisterUserDefinedTypes(clang::DeclContext *dc);
} // namespace cpp2rust::Mapper
