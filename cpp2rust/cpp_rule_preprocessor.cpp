// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <clang/AST/ASTContext.h>
#include <clang/AST/Attr.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Sema/Initialization.h>
#include <clang/Sema/Lookup.h>
#include <clang/Sema/Overload.h>
#include <clang/Sema/Sema.h>
#include <clang/Sema/Template.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/JSON.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>

#include <cstdlib>
#include <filesystem>
#include <string>

#include "compat/platform_flags.h"
#include "converter/converter_lib.h"
#include "converter/mapper.h"

namespace fs = std::filesystem;

namespace cpp2rust {

enum LookupKind { RegularName, CXXMethodName, CXXConstructorName, ADL };

struct LookupInfo {
  clang::DeclarationName name;
  LookupKind kind;
  llvm::ArrayRef<clang::TemplateArgumentLoc> explicitArgs;
  // The nested-name-specifier the rule body actually WROTE in front of the
  // callee, e.g. the `llvm::` of `llvm::cast<T1>(a0)`.  It used to be dropped
  // on the floor here, which made every QUALIFIED call to a non-`std`, non-
  // global function TEMPLATE unresolvable from inside a rule TEMPLATE: see
  // regularNameLookup, which could only search `std` and the TU.
  clang::NestedNameSpecifierLoc qualifierLoc;

  LookupInfo(const clang::Expr *expr) {
    if (const auto *ul = llvm::dyn_cast<clang::UnresolvedLookupExpr>(expr)) {
      clang::DeclarationName dname = ul->getName();
      name = dname;
      if (ul->requiresADL()) {
        kind = LookupKind::ADL;
      } else {
        kind = LookupKind::RegularName;
      }
      explicitArgs = ul->template_arguments();
      qualifierLoc = ul->getQualifierLoc();
    } else if (const auto *dm =
                   llvm::dyn_cast<clang::CXXDependentScopeMemberExpr>(expr)) {
      name = dm->getMember();
      kind = LookupKind::CXXMethodName;
      explicitArgs = dm->template_arguments();
    } else if (const auto *um =
                   llvm::dyn_cast<clang::UnresolvedMemberExpr>(expr)) {
      name = um->getMemberName();
      kind = LookupKind::CXXMethodName;
      explicitArgs = um->template_arguments();
    } else if (const auto *dref =
                   llvm::dyn_cast<clang::DependentScopeDeclRefExpr>(expr)) {
      clang::DeclarationName dname = dref->getDeclName();
      if (dname.getNameKind() ==
          clang::DeclarationName::NameKind::CXXConstructorName) {
        name = dname;
        kind = LookupKind::CXXConstructorName;
      } else {
        assert(0 && "Unsupported dref name kind");
      }
    } else if (llvm::isa<clang::CXXUnresolvedConstructExpr>(expr)) {
      kind = LookupKind::CXXConstructorName;
    } else {
      expr->dump();
      assert(0 && "Unsupported lookup expression");
    }
  }
};

class Callback : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  explicit Callback(llvm::json::Object &out) : out_(out) {}

  void init(clang::Sema &sema) {
    sema_ = &sema;
    clang::SourceManager &sm = sema.Context.getSourceManager();
    loc_ = sm.getLocForStartOfFile(sm.getMainFileID());
  }

  void run(const clang::ast_matchers::MatchFinder::MatchResult &R) override {
    assert(sema_);
    Mapper::PushASTContext scoped(*R.Context);
    if (auto func = R.Nodes.getNodeAs<clang::FunctionDecl>("validate_func")) {
      const char *err = nullptr;
      if (auto body =
              clang::dyn_cast_or_null<clang::CompoundStmt>(func->getBody())) {
        if (body->size() != 1) {
          err = "body must contain exactly one statement (a return)";
        } else if (!clang::isa<clang::ReturnStmt>(*body->body_begin())) {
          err = "body must be a return statement";
        }
      } else {
        err = "body cannot be empty";
      }

      if (err) {
        llvm::errs() << "ERROR: " << func->getQualifiedNameAsString() << ": "
                     << err << '\n';
        std::exit(EXIT_FAILURE);
      }
      return;
    }
    if (auto var = R.Nodes.getNodeAs<clang::TypedefNameDecl>("tvar")) {
      clang::QualType type = var->getUnderlyingType();
      if (auto *alias = llvm::dyn_cast<clang::TypeAliasDecl>(var)) {
        if (auto *tdecl = alias->getDescribedAliasTemplate()) {
          type = lookupType(tdecl);
        }
      }
      auto src = Mapper::ToString(type, Mapper::ScalarSugar::kPreserve);
      out_.try_emplace(var->getQualifiedNameAsString(), std::move(src));
      return;
    }

    if (auto func = R.Nodes.getNodeAs<clang::FunctionDecl>("func")) {
      auto add = [&](std::string &&src) {
        out_.try_emplace(func->getQualifiedNameAsString(), std::move(src));
      };

      if (const auto *fcall = R.Nodes.getNodeAs<clang::CallExpr>("fcall")) {
        if (fcall->getDirectCallee()) {
          add(Mapper::ToString(fcall));
          return;
        }

        LookupInfo lookup(fcall->getCallee());
        clang::FunctionDecl *rule = nullptr;
        clang::FunctionDecl *decl = lookupCalledDecl(
            func->getDescribedFunctionTemplate(), lookup, &rule);
        if (Mapper::HasFunctionParameterPack(func) &&
            Mapper::HasFunctionParameterPack(decl)) {
          addPackRule(func, rule, decl);
          return;
        }
        add(Mapper::ToString(decl));
        return;
      }
      if (const auto *ctor =
              R.Nodes.getNodeAs<clang::CXXConstructExpr>("ctor")) {
        if (ctor->getConstructor()) {
          add(Mapper::ToString(ctor));
          return;
        }
      }
      if (const auto *muse = R.Nodes.getNodeAs<clang::MemberExpr>("muse")) {
        if (llvm::isa<clang::FieldDecl>(muse->getMemberDecl())) {
          add(Mapper::ToString(muse));
          return;
        }
      }
      if (const auto *um =
              R.Nodes.getNodeAs<clang::UnresolvedMemberExpr>("umuse")) {
        add(Mapper::ToString(um));
        return;
      }
      if (R.Nodes.getNodeAs<clang::DeclRefExpr>("declref")) {
        if (const auto *enum_val =
                R.Nodes.getNodeAs<clang::EnumConstantDecl>("enum_val")) {
          add(Mapper::ToString(enum_val));
          return;
        } else if (const auto *decl =
                       R.Nodes.getNodeAs<clang::VarDecl>("decl")) {
          add(Mapper::ToString(decl));
          return;
        }
      }
      if (const auto *uop =
              R.Nodes.getNodeAs<clang::UnaryOperator>("udeclref")) {
        add(Mapper::ToString(uop));
        return;
      }
      if (const auto *dsme =
              R.Nodes.getNodeAs<clang::CXXDependentScopeMemberExpr>("dsme")) {
        if (dsme->isArrow()) {
          clang::MemberExpr *expr = lookupArrowAccess(
              func->getDescribedFunctionTemplate(), dsme->getMemberNameInfo(),
              dsme->getQualifierLoc());
          add(Mapper::ToString(expr));
          return;
        }
        clang::NamedDecl *decl = lookupMemberAccess(
            func->getDescribedFunctionTemplate(), dsme->getMember());
        add(Mapper::ToString(decl));
        return;
      }
      if (const auto *uctor =
              R.Nodes.getNodeAs<clang::CXXUnresolvedConstructExpr>("uctor")) {
        LookupInfo lookup(uctor);
        clang::NamedDecl *decl = lookupCalledDecl(
            func->getDescribedFunctionTemplate(), lookup, nullptr);
        add(Mapper::ToString(decl));
        return;
      }
      if (const auto *lit =
              R.Nodes.getNodeAs<clang::IntegerLiteral>("macro_int")) {
        if (lit->getBeginLoc().isMacroID()) {
          add(Mapper::ToString(lit));
        }
        return;
      }
    }
  }

private:
  llvm::json::Object &out_;
  clang::Sema *sema_ = nullptr;
  clang::SourceLocation loc_;

  // ⛔ EVERY `assert(... && "Rule resolution failed")` IN THIS FILE IS A NO-OP:
  // the pinned binary is built RelWithDebInfo, i.e. -DNDEBUG.  So each of them
  // used to fall through to a null dereference and the tool died with SIGSEGV,
  // printing NOTHING that names the rule.  pin/regen-rule.sh's only reliable
  // success signal is its `OK <module> -> <dir>` line, and a caller that greps
  // for anything else records that crash as a landed key -- a harness fault
  // presenting as a result, which is this project's most expensive failure
  // mode.  Route every such path through here instead.  Same pattern as
  // converter.cpp:2793 and as the resolution-failure path in lookupCalledDecl.
  [[noreturn]] static void fail(const llvm::Twine &msg) {
    llvm::report_fatal_error(msg, /*gen_crash_diag=*/false);
  }

  void addPackRule(const clang::FunctionDecl *func, clang::FunctionDecl *rule,
                   clang::FunctionDecl *callee) {
    auto key = Mapper::ToString(callee);
    auto init_type = getInitType(func, rule);
    if (init_type.isNull()) {
      out_.try_emplace(func->getQualifiedNameAsString(), std::move(key));
      return;
    }

    auto loc = findTemplateArgument(callee, init_type);
    llvm::json::Array path;
    for (unsigned step : loc.path) {
      path.push_back((int64_t)step);
    }
    out_.try_emplace(
        func->getQualifiedNameAsString(),
        llvm::json::Object{
            {"key", std::move(key)},
            {"init_type", llvm::json::Object{{"depth", loc.depth},
                                             {"index", loc.index},
                                             {"path", std::move(path)}}},
        });
  }

  clang::QualType getInitType(const clang::FunctionDecl *func,
                              clang::FunctionDecl *rule) {
    auto pattern = func->parameters()
                       .back()
                       ->getType()
                       ->castAs<clang::PackExpansionType>()
                       ->getPattern()
                       .getNonReferenceType();
    const auto *alias = pattern->getAs<clang::TemplateSpecializationType>();
    if (!alias || !alias->isTypeAlias() ||
        alias->getTemplateName().getAsTemplateDecl()->getName() != "Init") {
      return clang::QualType();
    }

    auto *tmpl = func->getDescribedFunctionTemplate();
    const clang::Sema::InstantiatingTemplate Inst(*sema_, loc_, tmpl);
    return getSubstType(Inst, alias->template_arguments()[0].getAsType(),
                        rule->getTemplateSpecializationArgs()->asArray());
  }

  // Where an `Init<T, Args>` pack's `T` lives, relative to the CALLEE's own
  // template-argument list.  `path` is a descent into the NESTED template
  // arguments of the argument named by (depth, index): empty means "the
  // argument itself", `{0}` means "template argument 0 of that argument", and
  // so on.  See the class comment on the nested search below for why the
  // nesting is needed at all.
  struct InitTypeLoc {
    unsigned depth = 0;
    unsigned index = 0;
    llvm::SmallVector<unsigned, 2> path;
  };

  // The whole point of this encoding is that the CALL SITE, not the rule, knows
  // the concrete init type: at translate time the converter re-reads the
  // callee's instantiation arguments and replays (depth, index, path).  So the
  // encoding may only name things that are recoverable from those arguments.
  //
  // ⭐ A TOP-LEVEL (depth, index) ALONE IS TOO WEAK, and `unordered_map` is the
  // case that proves it.  `unordered_map<K, V>::emplace` constructs a
  // `pair<const K, V>`, and that type is NOT one of the container's template
  // arguments -- those are K, V, hash<K>, equal_to<K> and
  // allocator<pair<const K, V>>.  It IS, however, template argument 0 of the
  // ALLOCATOR, so `{depth, index=4, path={0}}` names it exactly.  Hence the
  // nested descent.  ⛔ DELIBERATELY NOT GENERALISED: this searches only
  // argument types reachable by repeatedly taking a class template
  // specialization's own template arguments.  A `T` that is *computed* rather
  // than *contained* (`typename A::value_type` where A is not a specialization,
  // a type built by a metafunction, a pointer/array/function type wrapping a
  // parameter) is still unrepresentable, and MUST fail loudly below rather than
  // be approximated -- a wrong init type is a silent mis-construction at every
  // arrival site.
  InitTypeLoc findTemplateArgument(const clang::FunctionDecl *callee,
                                   clang::QualType type) {
    auto args = sema_->getTemplateInstantiationArgs(callee);
    // Two passes, so that a type which IS a top-level argument keeps exactly
    // the (depth, index) it had before this nesting existed -- every already
    // recorded module must re-record byte-identically.
    for (unsigned depth = 0; depth < args.getNumLevels(); ++depth) {
      for (unsigned index = 0; index < args.getNumSubsitutedArgs(depth);
           ++index) {
        const auto &arg = args(depth, index);
        if (arg.getKind() == clang::TemplateArgument::Type &&
            sema_->Context.hasSameType(arg.getAsType(), type)) {
          return {depth, index, {}};
        }
      }
    }
    for (unsigned depth = 0; depth < args.getNumLevels(); ++depth) {
      for (unsigned index = 0; index < args.getNumSubsitutedArgs(depth);
           ++index) {
        const auto &arg = args(depth, index);
        if (arg.getKind() != clang::TemplateArgument::Type) {
          continue;
        }
        llvm::SmallVector<unsigned, 2> path;
        if (findNestedTemplateArgument(arg.getAsType(), type, path)) {
          return {depth, index, std::move(path)};
        }
      }
    }
    fail(llvm::Twine("cpp-rule-preprocessor: Init type '") +
         Mapper::ToString(type) +
         "' is neither a template argument of '" + Mapper::ToString(callee) +
         "' nor a nested template argument of one of them, so it cannot be "
         "recovered at the call site.  Spell the Init<> type as a type that is "
         "reachable from the callee's template arguments, or key this call "
         "without an Init<> pack.");
  }

  // Depth-first search for `want` among `in`'s nested template arguments,
  // recording the index path taken.  `kMaxDepth` is a guard, not a policy: the
  // libc++ types involved nest two or three deep, and an unbounded walk over a
  // recursively-defined specialization would not terminate.
  bool findNestedTemplateArgument(clang::QualType in, clang::QualType want,
                                  llvm::SmallVectorImpl<unsigned> &path) {
    static constexpr unsigned kMaxDepth = 4;
    if (path.size() >= kMaxDepth) {
      return false;
    }
    const auto *spec =
        llvm::dyn_cast_or_null<clang::ClassTemplateSpecializationDecl>(
            in->getAsCXXRecordDecl());
    if (!spec) {
      return false;
    }
    auto nested = spec->getTemplateArgs().asArray();
    for (unsigned i = 0; i < nested.size(); ++i) {
      if (nested[i].getKind() != clang::TemplateArgument::Type) {
        continue;
      }
      path.push_back(i);
      if (sema_->Context.hasSameType(nested[i].getAsType(), want) ||
          findNestedTemplateArgument(nested[i].getAsType(), want, path)) {
        return true;
      }
      path.pop_back();
    }
    return false;
  }

  void forceCompleteDefinition(clang::QualType type) {
    type = type.getCanonicalType();
    if (type->isPointerType()) {
      type = type->getPointeeType();
    }

    if (!type->isIncompleteType()) {
      return;
    }

    sema_->RequireCompleteType(loc_, type,
                               clang::Sema::CompleteTypeKind::Normal,
                               clang::diag::err_incomplete_type);

    if (auto *spec =
            llvm::dyn_cast_or_null<clang::ClassTemplateSpecializationDecl>(
                type->getAsCXXRecordDecl())) {
      for (const auto *decl : spec->decls()) {
        if (const auto *tdef = llvm::dyn_cast<clang::TypedefNameDecl>(decl)) {
          clang::QualType tdef_t = tdef->getUnderlyingType();
          forceCompleteDefinition(tdef_t);
        }
      }

      for (const auto &arg : spec->getTemplateArgs().asArray()) {
        if (arg.getKind() == clang::TemplateArgument::Type) {
          forceCompleteDefinition(arg.getAsType());
        }
      }
    }
  }

  clang::FunctionDecl *deduceTemplateArguments(
      clang::FunctionTemplateDecl *decl, llvm::ArrayRef<clang::Expr *> callArgs,
      clang::QualType obj_t, clang::Expr::Classification exprClass,
      clang::TemplateArgumentListInfo *explicitArgs = nullptr) {
    clang::FunctionDecl *spec = nullptr;
    clang::sema::TemplateDeductionInfo info((loc_));
    auto check = [](llvm::ArrayRef<clang::QualType>, bool) -> bool {
      return false;
    };

    auto result = sema_->DeduceTemplateArguments(
        decl, explicitArgs, callArgs, spec, info, false, false, false, obj_t,
        exprClass, false, check);

    if (result == clang::TemplateDeductionResult::Success) {
      return spec;
    }

    if (result == clang::TemplateDeductionResult::SubstitutionFailure ||
        result == clang::TemplateDeductionResult::ConstraintsNotSatisfied) {
      if (const auto *deduced = info.takeCanonical()) {
        clang::TemplateArgumentListInfo targsInfo;
        for (const auto &arg : deduced->asArray()) {
          targsInfo.addArgument(
              sema_->getTrivialTemplateArgumentLoc(arg, {}, loc_));
        }

        clang::DefaultArguments defaultArgs;
        clang::Sema::CheckTemplateArgumentInfo ctai;
        clang::Sema::InstantiatingTemplate Inst(*sema_, loc_, decl);
        auto invalid = sema_->CheckTemplateArgumentList(
            decl, decl->getTemplateParameters(), loc_, targsInfo, defaultArgs,
            true, ctai);

        if (!invalid) {
          return sema_->InstantiateFunctionDeclaration(decl, deduced, loc_);
        }
      }
    }

    return nullptr;
  }

  clang::NamespaceDecl *createNamespaceDecl() {
    auto &ctx = sema_->getASTContext();
    auto *tu = ctx.getTranslationUnitDecl();
    auto *ns = clang::NamespaceDecl::Create(ctx, tu, false, loc_, loc_, nullptr,
                                            nullptr, false);
    tu->addDecl(ns);
    return ns;
  }

  clang::RecordDecl *
  createRecordDecl(llvm::StringRef name,
                   clang::QualType base = clang::QualType()) {
    bool owned = true;
    bool dependent = false;
    clang::CXXScopeSpec scope;
    clang::MultiTemplateParamsArg args;
    auto decl = sema_->ActOnTag(
        sema_->getCurScope(), clang::DeclSpec::TST_struct,
        clang::TagUseKind::Definition, loc_, scope,
        &sema_->Context.Idents.get(name), loc_, clang::ParsedAttributesView(),
        clang::AS_none, loc_, args, owned, dependent, loc_, false,
        clang::TypeResult(), false, false, clang::OffsetOfKind::Outside);
    assert(decl.isUsable() && "Record decl creation failed");
    auto *rdecl = decl.getAs<clang::RecordDecl>();

    rdecl->startDefinition();
    if (!base.isNull()) {
      clang::CXXBaseSpecifier baseSpec(
          clang::SourceRange(loc_, loc_), false, true, clang::AS_public,
          sema_->Context.getTrivialTypeSourceInfo(base, loc_),
          /*EllipsisLoc=*/clang::SourceLocation());
      const clang::CXXBaseSpecifier *bases[] = {&baseSpec};
      llvm::cast<clang::CXXRecordDecl>(rdecl)->setBases(bases, 1);
    }
    rdecl->completeDefinition();
    return rdecl;
  }

  static clang::QualType getNTTPType(const clang::TemplateArgument &arg) {
    switch (arg.getKind()) {
    case clang::TemplateArgument::Integral:
      return arg.getIntegralType();
    case clang::TemplateArgument::Declaration:
      return arg.getParamTypeForDecl();
    case clang::TemplateArgument::NullPtr:
      return arg.getNullPtrType();
    case clang::TemplateArgument::StructuralValue:
      return arg.getStructuralValueType();
    default:
      return clang::QualType();
    }
  }

  clang::QualType
  getTemplateIdType(clang::ClassTemplateDecl *decl,
                    llvm::ArrayRef<clang::TemplateArgument> args) {
    clang::TemplateArgumentListInfo info(loc_, loc_);
    for (const clang::TemplateArgument &arg : args) {
      info.addArgument(
          sema_->getTrivialTemplateArgumentLoc(arg, getNTTPType(arg), loc_));
    }
    return sema_->CheckTemplateIdType(clang::ElaboratedTypeKeyword::None,
                                      clang::TemplateName(decl), loc_, info,
                                      sema_->getCurScope(),
                                      /*ForNestedNameSpecifier=*/false);
  }

  using MirrorMap = llvm::SmallDenseMap<const clang::ClassTemplateDecl *,
                                        clang::ClassTemplateDecl *>;

  clang::TypeSourceInfo *findMatch(MirrorMap &substs, clang::QualType type) {
    const auto *tst = type->getAs<clang::TemplateSpecializationType>();
    if (!tst) {
      return nullptr;
    }

    const auto *tdecl = llvm::dyn_cast_or_null<clang::ClassTemplateDecl>(
        tst->getTemplateName().getAsTemplateDecl());
    if (!tdecl) {
      return nullptr;
    }

    if (auto it = substs.find(tdecl->getCanonicalDecl()); it != substs.end()) {
      auto match = getTemplateIdType(it->second, tst->template_arguments());
      assert(!match.isNull());
      return sema_->Context.getTrivialTypeSourceInfo(match, loc_);
    }
    return nullptr;
  }

  clang::ClassTemplateDecl *
  createInheritingTemplate(llvm::StringRef name, clang::ClassTemplateDecl *decl,
                           MirrorMap &substs) {
    clang::ASTContext &ctx = sema_->Context;
    auto *pattern = clang::CXXRecordDecl::Create(
        ctx, clang::TagTypeKind::Struct, sema_->CurContext, loc_, loc_,
        &ctx.Idents.get(name));

    auto *mirror = clang::ClassTemplateDecl::Create(
        ctx, sema_->CurContext, loc_,
        clang::DeclarationName(&ctx.Idents.get(name)),
        decl->getTemplateParameters(), pattern);
    pattern->setDescribedClassTemplate(mirror);
    mirror->setAccess(clang::AS_public);
    substs.try_emplace(decl->getCanonicalDecl(), mirror);

    clang::QualType base_t = getTemplateIdType(
        decl, decl->getTemplateParameters()->getInjectedTemplateArgs(ctx));
    assert(!base_t.isNull() && "Failed building mirror base");

    pattern->startDefinition();
    clang::CXXBaseSpecifier base(clang::SourceRange(loc_, loc_), false, true,
                                 clang::AS_public,
                                 ctx.getTrivialTypeSourceInfo(base_t, loc_),
                                 /*EllipsisLoc=*/clang::SourceLocation());
    const clang::CXXBaseSpecifier *bases[] = {&base};
    pattern->setBases(bases, 1);

    for (auto *member : decl->getTemplatedDecl()->decls()) {
      if (const auto *td = llvm::dyn_cast<clang::TypedefNameDecl>(member)) {
        if (auto *replacement = findMatch(substs, td->getUnderlyingType())) {
          auto *copy = clang::TypedefDecl::Create(
              ctx, pattern, loc_, loc_, td->getIdentifier(), replacement);
          copy->setAccess(clang::AS_public);
          pattern->addDecl(copy);
        }
      } else if (auto *tdecl =
                     llvm::dyn_cast<clang::ClassTemplateDecl>(member)) {
        clang::Sema::ContextRAII savedContext(*sema_, pattern);
        createInheritingTemplate(tdecl->getName(), tdecl, substs);
      }
    }

    pattern->completeDefinition();
    sema_->CurContext->addDecl(mirror);
    return mirror;
  }

  // The stand-in ENUMERATION for a rule template parameter whose hint is an
  // enumeration type.  Named after the parameter (`T1`), so the recorded key
  // prints `std::__hash_impl<T1>` and stays GENERIC; it is a fresh, empty,
  // enumerator-less enum, so it is a DISTINCT type from the hint and cannot
  // make the key name the hint.
  //
  // ⭐ WHY THIS IS NEEDED AND WHAT IT GENERALISES.  createTemplateArguments
  // builds the stand-in for an un-hinted parameter with createRecordDecl, i.e.
  // always a CLASS type.  A library template whose members live on a PARTIAL
  // SPECIALISATION selected by a type TRAIT on that parameter is then
  // unreachable by construction: no trait holds for an empty struct, so
  // resolution picks the PRIMARY template.  libc++'s hash is exactly this --
  //     template <class _Tp, class = void> struct __hash_impl { ...= delete; };
  //     template <class _Tp> struct __hash_impl<_Tp,
  //         __enable_if_t<is_enum<_Tp>::value && __is_unqualified_v<_Tp>>>
  //       : __unary_function<_Tp, size_t> { size_t operator()(_Tp) const; };
  // (toolchain/libcxx/__functional/hash.h:351-365) -- the PRIMARY has no
  // `operator()` at all, so `o.operator()(a1)` on a generic `__hash_impl<T1>`
  // could not resolve however it was spelled, and the same held through the
  // derived `std::hash<T1>` spelling.
  //
  // ⛔ NOT SPECIAL-CASED TO hash, OR TO ENUMS-IN-libc++.  The kind of stand-in
  // is chosen from the KIND OF THE HINT the rule itself wrote, which is the
  // channel that already existed for `typename T2 = std::allocator<T1>`
  // (rules/vector).  Any trait-selected partial specialisation over an
  // enumeration is reachable by the same one line of rule source, and a hint
  // kind nobody has needed yet fails LOUDLY below instead of crashing.
  clang::QualType createMirrorEnum(llvm::StringRef name,
                                   const clang::EnumDecl *hdecl) {
    clang::ASTContext &ctx = sema_->Context;
    clang::EnumDecl *edecl = clang::EnumDecl::Create(
        ctx, sema_->CurContext, loc_, loc_, &ctx.Idents.get(name),
        /*PrevDecl=*/nullptr, hdecl->isScoped(), hdecl->isScopedUsingClassTag(),
        hdecl->isFixed());
    edecl->setAccess(clang::AS_public);
    sema_->CurContext->addDecl(edecl);
    edecl->startDefinition();
    // Empty (no enumerators): the stand-in is only ever used as a TYPE, and an
    // enumerator would leak a name into the synthetic namespace.  The integer
    // and promotion types are copied from the hint so that
    // `__underlying_type_t<T1>` -- which the enum specialisation's own body
    // uses -- is the hint's, not a guess.
    edecl->completeDefinition(hdecl->getIntegerType(), hdecl->getPromotionType(),
                              /*NumPositiveBits=*/0, /*NumNegativeBits=*/0);
    return ctx.getCanonicalTagType(edecl);
  }

  clang::QualType createMirrorType(llvm::StringRef name, clang::QualType hint) {
    clang::ASTContext &ctx = sema_->Context;
    forceCompleteDefinition(hint);

    if (const auto *etype = hint->getAs<clang::EnumType>()) {
      const clang::EnumDecl *edecl = etype->getDecl()->getDefinition();
      if (!edecl) {
        fail(llvm::Twine("template parameter `") + name +
             "` has an INCOMPLETE enumeration as its default-argument hint (`" +
             Mapper::ToString(hint) +
             "`); a stand-in enum needs the hint's underlying type, so define "
             "the enum in the rule source");
      }
      return createMirrorEnum(name, edecl);
    }

    const auto *hdecl = hint->getAsCXXRecordDecl();
    if (!hdecl || !hdecl->isCompleteDefinition()) {
      // ⛔ WAS `assert(hdecl && ...)`, i.e. a no-op under -DNDEBUG followed by a
      // null dereference.  A rule that hinted a parameter with anything other
      // than a class or an enum (a builtin, a pointer, a function type) died
      // with a bare SIGSEGV and no mention of the rule or the parameter.
      fail(llvm::Twine("template parameter `") + name +
           "`'s default-argument hint `" + Mapper::ToString(hint) +
           "` is neither a complete class type nor an enumeration, and only "
           "those two kinds of stand-in can be built");
    }

    const auto *hspec =
        llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(hdecl);
    if (!hspec) {
      // if it is not a template specialization inheriting from it suffices
      clang::RecordDecl *rdecl = createRecordDecl(name, hint);
      return ctx.getTagType(clang::ElaboratedTypeKeyword::None,
                            rdecl->getQualifier(), rdecl, false);
    }

    MirrorMap substs;
    auto *mirror =
        createInheritingTemplate(name, hspec->getSpecializedTemplate(), substs);
    clang::QualType spec =
        getTemplateIdType(mirror, hspec->getTemplateArgs().asArray());
    assert(!spec.isNull() && spec->getAsCXXRecordDecl());

    // required to print Tn instead of Tn<arg1, arg2, ...>
    clang::NamespaceDecl *ns = createNamespaceDecl();
    auto *alias =
        clang::TypeAliasDecl::Create(ctx, ns, loc_, loc_, &ctx.Idents.get(name),
                                     ctx.getTrivialTypeSourceInfo(spec, loc_));
    ns->addDecl(alias);

    clang::QualType alias_t = ctx.getTypedefType(
        clang::ElaboratedTypeKeyword::None, std::nullopt, alias);
    spec->getAsCXXRecordDecl()->addAttr(
        clang::PreferredNameAttr::CreateImplicit(
            ctx, ctx.getTrivialTypeSourceInfo(alias_t, loc_)));

    return ctx.getCanonicalType(spec);
  }

  clang::QualType getSubstType(const clang::Sema::InstantiatingTemplate &Inst,
                               clang::QualType type,
                               llvm::ArrayRef<clang::TemplateArgument> args) {
    assert(!Inst.isInvalid() && "Invalid instantiation context");
    clang::MultiLevelTemplateArgumentList mtal;
    mtal.setKind(clang::TemplateSubstitutionKind::Rewrite);
    mtal.addOuterTemplateArguments(args);

    clang::TypeSourceInfo *tsi =
        sema_->SubstType(sema_->Context.getTrivialTypeSourceInfo(type), mtal,
                         loc_, clang::DeclarationName());
    assert(tsi && "Template argument type instantiation failed");
    return tsi->getType();
  }

  clang::QualType
  getDefaultArg(clang::TemplateDecl *decl,
                const clang::TemplateTypeParmDecl *parm,
                llvm::ArrayRef<clang::TemplateArgument> currentArgs) {
    clang::QualType type = parm->getDefaultArgument().getArgument().getAsType();
    if (!type->isDependentType()) {
      return type;
    }

    const clang::Sema::InstantiatingTemplate Inst(*sema_, loc_, decl);
    return getSubstType(Inst, type, currentArgs);
  }

  clang::VarDecl *createVarDecl(clang::QualType type, llvm::StringRef name,
                                clang::StorageClass sclass = clang::SC_None) {
    clang::ASTContext &ctx = sema_->Context;
    clang::VarDecl *decl = clang::VarDecl::Create(
        ctx, sema_->CurContext, loc_, loc_, &ctx.Idents.get(name),
        type.getNonReferenceType(), nullptr, sclass);
    sema_->CurContext->addDecl(decl);
    decl->markUsed(ctx);
    return decl;
  }

  clang::DeclRefExpr *createDeclRefExpr(clang::VarDecl *decl) {
    const clang::DeclarationNameInfo nameInfo(decl->getDeclName(), loc_);
    return sema_->BuildDeclRefExpr(decl, decl->getType(), clang::VK_LValue,
                                   nameInfo, decl->getQualifierLoc());
  }

  clang::DeclRefExpr *createConstexprDeclRefExpr(clang::QualType type,
                                                 llvm::StringRef name) {
    clang::VarDecl *decl = createVarDecl(type, name, clang::SC_Static);
    decl->setConstexpr(true);

    clang::Expr *init;
    clang::ASTContext &ctx = sema_->Context;
    if (type->isIntegerType()) {
      init = clang::IntegerLiteral::Create(
          ctx, llvm::APInt(ctx.getIntWidth(type), 1), type, loc_);
    } else {
      init = new (ctx) clang::ImplicitValueInitExpr(type);
    }
    decl->setInit(init);
    return createDeclRefExpr(decl);
  }

  clang::OpaqueValueExpr *createOpaqueValueExpr(clang::QualType type) {
    return new (sema_->Context) clang::OpaqueValueExpr(
        loc_, type.getNonReferenceType(),
        type->isRValueReferenceType() ? clang::VK_XValue : clang::VK_LValue);
  }

  void
  createTemplateArguments(clang::TemplateDecl *decl,
                          llvm::SmallVectorImpl<clang::TemplateArgument> &out) {
    for (clang::NamedDecl *param : *decl->getTemplateParameters()) {
      if (const auto *ttp =
              llvm::dyn_cast<clang::TemplateTypeParmDecl>(param)) {
        clang::QualType type;
        if (param->isTemplateParameterPack()) {
          out.emplace_back(clang::TemplateArgument::getEmptyPack());
          continue;
        }
        if (ttp->hasDefaultArgument()) {
          clang::QualType hint = getDefaultArg(decl, ttp, out);
          assert(!hint.isNull() && "Failed retrieving type hint");
          type = createMirrorType(param->getName(), hint);
        } else {
          clang::RecordDecl *rdecl = createRecordDecl(param->getName());
          type = sema_->Context.getTagType(clang::ElaboratedTypeKeyword::None,
                                           rdecl->getQualifier(), rdecl, false);
        }
        assert(!type.isNull() && "Template type argument creation failed");
        out.emplace_back(type);
      } else if (const auto *nttp =
                     llvm::dyn_cast<clang::NonTypeTemplateParmDecl>(param)) {
        clang::QualType type = nttp->getType();
        if (type->isDependentType()) {
          const clang::Sema::InstantiatingTemplate Inst(*sema_, loc_, decl);
          type = getSubstType(Inst, type, out);
        }
        clang::DeclRefExpr *var =
            createConstexprDeclRefExpr(type, param->getName());
        out.emplace_back(var, true);
      } else {
        assert(0 && "Unsupported template param kind");
      }
    }
  }

  clang::FunctionDecl *instantiateRuleDecl(clang::FunctionTemplateDecl *decl) {
    llvm::SmallVector<clang::TemplateArgument, 8> args;
    createTemplateArguments(decl, args);
    return sema_->InstantiateFunctionDeclaration(
        decl, clang::TemplateArgumentList::CreateCopy(sema_->Context, args),
        loc_);
  }

  clang::FunctionDecl *createCandidate(
      clang::NamedDecl *decl, llvm::ArrayRef<clang::Expr *> callArgs,
      clang::TemplateArgumentListInfo *explicitArgs = nullptr,
      clang::QualType obj_t = clang::QualType(),
      clang::Expr::Classification eclass = clang::Expr::Classification()) {
    if (auto *tdecl = llvm::dyn_cast<clang::FunctionTemplateDecl>(decl)) {
      if (auto *fdecl = deduceTemplateArguments(tdecl, callArgs, obj_t, eclass,
                                                explicitArgs)) {
        return fdecl;
      }
      return nullptr;
    }
    return llvm::dyn_cast<clang::FunctionDecl>(decl);
  }

  clang::CXXRecordDecl *resolveCXXRecordDecl(clang::QualType obj_t) {
    obj_t = obj_t.getCanonicalType();
    while (obj_t->isPointerOrReferenceType()) {
      obj_t = obj_t->getPointeeType();
    }

    forceCompleteDefinition(obj_t);
    if (auto *rdecl = obj_t->getAsCXXRecordDecl()) {
      return rdecl->getDefinition();
    }
    return nullptr;
  }

  // The DeclContext the rule body's own nested-name-specifier names, or null
  // when the call was unqualified or the qualifier is dependent (`T1::foo`).
  //
  // WHY THIS EXISTS.  regularNameLookup re-resolves the callee of a DEPENDENT
  // call written inside a rule TEMPLATE.  It used to look the bare
  // DeclarationName up in exactly two scopes -- `std`, then the translation
  // unit -- and LookupQualifiedName does NOT descend into nested namespaces.
  // So a rule template that called `llvm::cast<T1>(a0)` had its `llvm::`
  // discarded, found nothing, and died with `No viable function` followed by a
  // null-deref crash on the resolution-failure path below.  That blocked the
  // largest undefined-name row in the corpus (`llvm::cast` /
  // `llvm::dyn_cast` / `llvm::dyn_cast_or_null`, ~96,600 call sites), and it
  // could not be worked around from the rules side: the recorded key follows
  // the RESOLVED decl's qualified name, so the namespace that must be searched
  // is exactly the namespace the key must name.
  //
  // ⛔ DELIBERATELY NOT A NAMESPACE ALLOW-LIST.  The scope searched is whatever
  // the rule body wrote, so `mlir::`, `llvm::sys::`, `llvm::cl::`, `DCC::` and
  // class-qualified static calls all work by construction, and a namespace
  // nobody has tried yet cannot be the reason a future rule fails.
  clang::DeclContext *qualifiedLookupContext(clang::NestedNameSpecifierLoc nns) {
    if (!nns) {
      return nullptr;
    }
    clang::CXXScopeSpec scope;
    scope.Adopt(nns);
    // Returns null for a dependent qualifier, which is the correct answer here:
    // fall through to the std/TU searches rather than guessing.
    return sema_->computeDeclContext(scope, /*EnteringContext=*/false);
  }

  void regularNameLookup(llvm::ArrayRef<clang::Expr *> callArgs,
                         clang::TemplateArgumentListInfo *explicitTArgs,
                         clang::DeclarationName &name,
                         clang::NestedNameSpecifierLoc qualifierLoc,
                         clang::OverloadCandidateSet &candidates) {
    clang::LookupResult decls(*sema_, name, loc_,
                              clang::Sema::LookupOrdinaryName);
    // The written qualifier wins, exactly as it does in real C++ lookup.
    if (clang::DeclContext *qualified = qualifiedLookupContext(qualifierLoc)) {
      sema_->LookupQualifiedName(decls, qualified);
    }
    if (decls.empty()) {
      decls.clear();
      if (clang::NamespaceDecl *std_ns = sema_->getStdNamespace()) {
        sema_->LookupQualifiedName(decls, std_ns);
      }
    }
    if (decls.empty()) {
      decls.clear();
      sema_->LookupQualifiedName(decls,
                                 sema_->Context.getTranslationUnitDecl());
    }
    for (auto *ndecl : decls) {
      if (auto *candidate = createCandidate(ndecl, callArgs, explicitTArgs)) {
        sema_->AddOverloadCandidate(
            candidate, clang::DeclAccessPair::make(candidate, clang::AS_public),
            callArgs, candidates, false);
      }
    }

    for (const auto *arg : callArgs) {
      if (auto *rdecl = resolveCXXRecordDecl(arg->getType())) {
        for (auto *frdecl : rdecl->friends()) {
          auto *fd = frdecl->getFriendDecl();
          if (!fd) {
            continue;
          }

          if (auto *ndecl = llvm::dyn_cast<clang::NamedDecl>(fd);
              ndecl && ndecl->getDeclName() == name) {
            if (auto *candidate =
                    createCandidate(ndecl, callArgs, explicitTArgs)) {
              sema_->AddOverloadCandidate(
                  candidate,
                  clang::DeclAccessPair::make(candidate, clang::AS_public),
                  callArgs, candidates, false);
            }
          }
        }
      }
    }
  }

  void cxxMethodNameLookup(clang::QualType obj_t,
                           llvm::ArrayRef<clang::Expr *> callArgs,
                           clang::TemplateArgumentListInfo *explicitTArgs,
                           clang::DeclarationName &name,
                           clang::OverloadCandidateSet &candidates) {
    clang::CXXRecordDecl *rdecl = resolveCXXRecordDecl(obj_t);
    assert(rdecl && "Failed fetching record decl");
    clang::LookupResult members(*sema_, name, loc_,
                                clang::Sema::LookupMemberName);
    sema_->LookupQualifiedName(members, rdecl);

    auto eclass = clang::Expr::Classification::makeSimpleLValue();
    for (auto *ndecl : members) {
      if (auto *candidate =
              createCandidate(ndecl, callArgs, explicitTArgs, obj_t, eclass)) {
        sema_->AddMethodCandidate(
            clang::DeclAccessPair::make(candidate, clang::AS_public), obj_t,
            eclass, callArgs, candidates);
      }
    }
  }

  void cxxConstructorNameLookup(clang::QualType obj_t,
                                llvm::ArrayRef<clang::Expr *> callArgs,
                                clang::OverloadCandidateSet &candidates) {
    clang::CXXRecordDecl *rdecl = resolveCXXRecordDecl(obj_t);
    assert(rdecl && "Failed fetching record decl");
    clang::DeclContextLookupResult ctors = sema_->LookupConstructors(rdecl);

    for (auto *ndecl : ctors) {
      if (auto *candidate = createCandidate(ndecl, callArgs)) {
        sema_->AddOverloadCandidate(
            candidate, clang::DeclAccessPair::make(candidate, clang::AS_public),
            callArgs, candidates, false);
      }
    }
  }

  void adlLookup(llvm::ArrayRef<clang::Expr *> callArgs,
                 clang::DeclarationName &name,
                 clang::OverloadCandidateSet &candidates) {
    clang::ADLResult adl;
    sema_->ArgumentDependentLookup(name, loc_, callArgs, adl);

    for (auto *ndecl : adl) {
      if (auto *candidate = createCandidate(ndecl, callArgs)) {
        sema_->AddOverloadCandidate(
            candidate, clang::DeclAccessPair::make(candidate, clang::AS_public),
            callArgs, candidates, false);
      }
    }
  }

  clang::FunctionDecl *lookupCalledDecl(clang::FunctionTemplateDecl *decl,
                                        LookupInfo &lookup,
                                        clang::FunctionDecl **rule_out) {
    clang::NamespaceDecl *ns = createNamespaceDecl();
    clang::Sema::ContextRAII savedContext(*sema_, ns);
    clang::FunctionDecl *rule = instantiateRuleDecl(decl);
    // ⛔ WAS `assert(rule && "Rule instantiation failed")` -- a no-op under
    // -DNDEBUG, so `rule->parameters()` below dereferenced null and the tool
    // died with a bare SIGSEGV, printing only the `Preprocessing <path>` line.
    // Measured on the enum-partial-specialisation probe: a rule whose PARAMETER
    // TYPE is guarded by `__enable_if_t<is_enum<T1>::value && ...>` cannot be
    // instantiated at all while the stand-in for T1 is an empty STRUCT -- the
    // enable_if is a substitution failure, so the whole declaration is
    // discarded and this returns null BEFORE any lookup happens.  That is a
    // different failure from the resolution failure below and needs its own
    // message, because the remedy is different: hint the parameter.
    if (!rule) {
      fail(llvm::Twine("rule instantiation failed for `") +
           decl->getQualifiedNameAsString() +
           "`: no declaration survives substituting the stand-in types built "
           "for its template parameters -- a SFINAE/enable_if substitution "
           "failure in one of its PARAMETER types, before any name lookup "
           "happens.  An un-hinted `typename Tn` is modelled as an empty STRUCT, "
           "so a parameter constrained by a type trait (is_enum, is_integral, "
           "...) can never be formed; give that parameter a DEFAULT ARGUMENT of "
           "the right KIND of type (e.g. `template <typename T1 = SomeEnum>`) "
           "so the stand-in is built to match");
    }
    if (rule_out) {
      *rule_out = rule;
    }
    llvm::ArrayRef<clang::ParmVarDecl *> parms = rule->parameters();
    auto csk = lookup.name.getNameKind() ==
                       clang::DeclarationName::NameKind::CXXOperatorName
                   ? clang::OverloadCandidateSet::CSK_Operator
                   : clang::OverloadCandidateSet::CSK_Normal;

    llvm::SmallVector<clang::Expr *, 8> callArgs;
    for (const auto *parm : parms) {
      clang::QualType parm_t = parm->getType();
      forceCompleteDefinition(parm_t);
      callArgs.emplace_back(createOpaqueValueExpr(parm_t));
    }

    llvm::ArrayRef<clang::TemplateArgument> ruleTArgs =
        rule->getTemplateSpecializationArgs()->asArray();
    clang::TemplateArgumentListInfo explicitTArgs;

    {
      const clang::Sema::InstantiatingTemplate Inst(*sema_, loc_, decl);
      assert(!Inst.isInvalid() && "Invalid instantiation context");
      for (const auto &argloc : lookup.explicitArgs) {
        const auto &arg = argloc.getArgument();
        if (!arg.isDependent()) {
          explicitTArgs.addArgument(argloc);
          continue;
        }

        clang::TemplateArgument inst;
        if (arg.getKind() == clang::TemplateArgument::Type) {
          inst = clang::TemplateArgument(
              getSubstType(Inst, arg.getAsType(), ruleTArgs));
        } else if (arg.getKind() == clang::TemplateArgument::Expression) {
          if (auto *expr =
                  llvm::dyn_cast<clang::DeclRefExpr>(arg.getAsExpr())) {
            const auto *nttp =
                llvm::dyn_cast<clang::NonTypeTemplateParmDecl>(expr->getDecl());
            assert(nttp && "Unexpected decl in expr");
            inst = ruleTArgs[nttp->getIndex()];
          } else {
            assert(0 && "Unsupported explicit template argument expression");
          }
        } else {
          assert(0 && "Unsupported explicit template argument kind");
        }

        explicitTArgs.addArgument(
            sema_->getTrivialTemplateArgumentLoc(inst, {}, loc_));
      }
    }

    clang::DeclarationName name = lookup.name;
    if (clang::QualType nameType = name.getCXXNameType();
        !nameType.isNull() && nameType->isDependentType()) {
      const clang::Sema::InstantiatingTemplate Inst(*sema_, loc_, decl);
      assert(!Inst.isInvalid() && "Invalid instantiation context");
      clang::MultiLevelTemplateArgumentList mtal;
      mtal.setKind(clang::TemplateSubstitutionKind::Rewrite);
      mtal.addOuterTemplateArguments(ruleTArgs);
      name = sema_->SubstDeclarationNameInfo({name, loc_}, mtal).getName();
    }

    clang::OverloadCandidateSet candidates(loc_, csk);
    switch (lookup.kind) {
    case LookupKind::RegularName:
      regularNameLookup(callArgs, &explicitTArgs, name, lookup.qualifierLoc,
                        candidates);
      break;
    case LookupKind::CXXMethodName: {
      llvm::ArrayRef<clang::Expr *> margs = callArgs;
      cxxMethodNameLookup(margs.front()->getType().getNonReferenceType(),
                          margs.drop_front(), &explicitTArgs, name, candidates);
      break;
    }
    case LookupKind::CXXConstructorName:
      cxxConstructorNameLookup(rule->getReturnType(), callArgs, candidates);
      break;
    case LookupKind::ADL:
      // An ADL call is by definition unqualified, so qualifierLoc is empty
      // here; passed for uniformity rather than because it can be set.
      regularNameLookup(callArgs, &explicitTArgs, name, lookup.qualifierLoc,
                        candidates);
      adlLookup(callArgs, name, candidates);
      break;
    }

    clang::OverloadCandidateSet::iterator best;
    const char *reason = "no candidate selected";
    switch (candidates.BestViableFunction(*sema_, loc_, best)) {
    case clang::OverloadingResult::OR_Success:
      return best->Function;
    case clang::OverloadingResult::OR_Ambiguous:
      for (auto &candidate : candidates) {
        if (candidate.Viable) {
          return candidate.Function;
        }
      }
      reason = "ambiguous, and no candidate was viable";
      break;
    case clang::OverloadingResult::OR_No_Viable_Function:
      reason = "no viable function";
      break;
    case clang::OverloadingResult::OR_Deleted:
      reason = "the selected function is deleted";
      break;
    }

    // ⛔ THIS USED TO BE `assert(0 && "Rule resolution failed")`, AND THE PIN IS
    // BUILT -DNDEBUG, so the assert was a NO-OP: the function returned nullptr,
    // the caller dereferenced it, and the tool died with SIGSEGV after printing
    // a bare `No viable function` with no rule name and no callee name.  Worse,
    // callers of pin/regen-rule.sh recorded that crash as SUCCESS -- the only
    // reliable success signal is the `OK <module> -> <dir>` line -- so a rule
    // that never resolved read as a landed key.  A harness fault recorded as a
    // result is the most expensive failure mode in this project, so fail loudly,
    // name both ends, and exit nonzero.  Same pattern as converter.cpp:2793.
    llvm::report_fatal_error(llvm::Twine("rule resolution failed for `") +
                                 decl->getQualifiedNameAsString() +
                                 "`: could not resolve the call to `" +
                                 name.getAsString() + "` (" + reason + ")",
                             /*gen_crash_diag=*/false);
  }

  clang::NamedDecl *lookupMemberAccess(clang::FunctionTemplateDecl *decl,
                                       clang::DeclarationName name) {
    clang::NamespaceDecl *ns = createNamespaceDecl();
    clang::Sema::ContextRAII savedContext(*sema_, ns);
    clang::FunctionDecl *rule = instantiateRuleDecl(decl);
    assert(rule && "Rule instantiation failed");
    clang::CXXRecordDecl *rdecl =
        resolveCXXRecordDecl(rule->getParamDecl(0)->getType());
    assert(rdecl && "Failed fetching record decl");
    clang::LookupResult members(*sema_, name, loc_,
                                clang::Sema::LookupMemberName);
    sema_->LookupQualifiedName(members, rdecl);
    assert(!members.empty() && "Rule resolution failed");
    return members.getRepresentativeDecl();
  }

  clang::MemberExpr *
  lookupArrowAccess(clang::FunctionTemplateDecl *decl,
                    const clang::DeclarationNameInfo &nameInfo,
                    clang::NestedNameSpecifierLoc nns) {
    clang::NamespaceDecl *ns = createNamespaceDecl();
    clang::Sema::ContextRAII savedContext(*sema_, ns);
    clang::FunctionDecl *rule = instantiateRuleDecl(decl);
    assert(rule && "Rule instantiation failed");

    clang::Expr *obj = createOpaqueValueExpr(
        rule->getParamDecl(0)->getType().getNonReferenceType());
    auto arrow =
        sema_->BuildOverloadedArrowExpr(sema_->getCurScope(), obj, loc_);
    assert(arrow.isUsable() && "Overloaded arrow operator not found");

    auto *base = arrow.getAs<clang::CXXOperatorCallExpr>();
    assert(base && "Unexpected base type");

    clang::CXXRecordDecl *rdecl =
        resolveCXXRecordDecl(base->getType()->getPointeeType());
    assert(rdecl && "Failed fetching record decl");

    clang::LookupResult members(*sema_, nameInfo.getName(), loc_,
                                clang::Sema::LookupMemberName);
    sema_->LookupQualifiedName(members, rdecl);
    for (auto *ndecl : members) {
      if (auto *vdecl = llvm::dyn_cast<clang::ValueDecl>(ndecl)) {
        clang::MemberExpr *access = sema_->BuildMemberExpr(
            base, true, loc_, nns, loc_, vdecl,
            clang::DeclAccessPair::make(vdecl, clang::AS_public), false,
            nameInfo, vdecl->getType(), clang::VK_LValue, clang::OK_Ordinary);
        assert(access && "Rule resolution failed");
        return access;
      }
    }
    assert(0 && "Rule resolution failed");
    return nullptr;
  }

  clang::QualType lookupType(clang::TypeAliasTemplateDecl *decl) {
    clang::NamespaceDecl *ns = createNamespaceDecl();
    clang::Sema::ContextRAII savedContext(*sema_, ns);

    llvm::SmallVector<clang::TemplateArgument, 4> args;
    createTemplateArguments(decl, args);

    clang::MultiLevelTemplateArgumentList mtal;
    mtal.setKind(clang::TemplateSubstitutionKind::Rewrite);
    mtal.addOuterTemplateArguments(args);

    clang::Sema::InstantiatingTemplate TypeInst(*sema_, loc_, decl, args);
    assert(!TypeInst.isInvalid() && "Invalid instantiation context");

    clang::TypeSourceInfo *tsi =
        sema_->SubstType(decl->getTemplatedDecl()->getTypeSourceInfo(), mtal,
                         loc_, clang::DeclarationName());
    assert(tsi && "Rule resolution failed");
    return tsi->getType();
  }
};

class ActionFactory : public clang::tooling::FrontendActionFactory {
public:
  explicit ActionFactory(llvm::json::Object &out) : cb_(out) {
    using namespace clang::ast_matchers;
    finder_.addMatcher(
        returnStmt(
            isExpansionInMainFile(),
            hasReturnValue(ignoringImplicit(ignoringParenImpCasts(anyOf(
                callExpr().bind("fcall"), cxxConstructExpr().bind("ctor"),
                cxxFunctionalCastExpr(has(ignoringImplicit(
                    ignoringParenImpCasts(cxxConstructExpr().bind("ctor"))))),
                memberExpr(hasDeclaration(fieldDecl())).bind("muse"),
                unresolvedMemberExpr().bind("umuse"),
                declRefExpr(to(anyOf(enumConstantDecl().bind("enum_val"),
                                     decl(unless(parmVarDecl())).bind("decl"))))
                    .bind("declref"),
                unaryOperator(hasUnaryOperand(
                                  declRefExpr(to(decl(unless(parmVarDecl()))))))
                    .bind("udeclref"),
                cxxDependentScopeMemberExpr().bind("dsme"),
                cxxUnresolvedConstructExpr().bind("uctor"),
                integerLiteral().bind("macro_int"))))),
            hasAncestor(functionDecl(isDefinition(),
                                     matchesName("(^|::)f[0-9]+$"),
                                     isExpansionInMainFile())
                            .bind("func"))),
        &cb_);

    finder_.addMatcher(
        typedefNameDecl(matchesName("(^|::)t[0-9]+$"), isExpansionInMainFile())
            .bind("tvar"),
        &cb_);

    finder_.addMatcher(functionDecl(isDefinition(),
                                    matchesName("(^|::)f[0-9]+$"),
                                    isExpansionInMainFile())
                           .bind("validate_func"),
                       &cb_);
  }

  std::unique_ptr<clang::FrontendAction> create() override {
    class ASTConsumer : public clang::ASTConsumer {
    public:
      explicit ASTConsumer(std::unique_ptr<clang::ASTConsumer> AC,
                           clang::CompilerInstance &CI, Callback *CB)
          : AC_(std::move(AC)), CI_(&CI), CB_(CB) {}

      void HandleTranslationUnit(clang::ASTContext &ctx) override {
        auto &DE = CI_->getDiagnostics();
        if (DE.hasErrorOccurred()) {
          std::exit(EXIT_FAILURE);
        }
        DE.setSuppressAllDiagnostics(true);
        DE.setClient(new clang::IgnoringDiagConsumer(), true);
        CB_->init(CI_->getSema());
        AC_->HandleTranslationUnit(ctx);
      }

    private:
      std::unique_ptr<clang::ASTConsumer> AC_;
      clang::CompilerInstance *CI_;
      Callback *CB_;
    };

    class Wrapped : public clang::ASTFrontendAction {
      clang::ast_matchers::MatchFinder &F_;
      Callback *CB_;

    public:
      explicit Wrapped(clang::ast_matchers::MatchFinder &MF, Callback &CB)
          : F_(MF), CB_(&CB) {}

      std::unique_ptr<clang::ASTConsumer>
      CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
        return std::make_unique<ASTConsumer>(F_.newASTConsumer(), CI, CB_);
      }
    };
    return std::make_unique<Wrapped>(finder_, cb_);
  }

private:
  clang::ast_matchers::MatchFinder finder_;
  Callback cb_;
};

void Extract(const std::filesystem::path &src_path, llvm::json::Object &out,
             llvm::ArrayRef<llvm::StringRef> cxx_flags) {
  auto flags = getPlatformClangBeginFlags();
  flags.insert(flags.end(), cxx_flags.begin(), cxx_flags.end());
  auto end_flags = getPlatformClangEndFlags();
  flags.insert(flags.end(), end_flags.begin(), end_flags.end());
  auto code = llvm::MemoryBuffer::getFile(src_path.string());
  if (!code) {
    llvm::errs() << "ERROR: cannot read " << src_path.string() << '\n';
    std::exit(EXIT_FAILURE);
  }
  ActionFactory factory(out);
  clang::tooling::runToolOnCodeWithArgs(
      factory.create(), (*code)->getBuffer(), flags, src_path.string(),
      src_path.extension() == ".c" ? CLANG_C_COMPILER : CLANG_CXX_COMPILER);
}

} // namespace cpp2rust

namespace {

llvm::cl::OptionCategory cat("cpp-rule-preprocessor options");

llvm::cl::opt<std::string>
    SrcDir("dir",
           llvm::cl::desc("Path to a rule directory containing src.c and/or "
                          "src.cpp."),
           llvm::cl::value_desc("rule-dir"), llvm::cl::Required,
           llvm::cl::cat(cat));

llvm::cl::opt<std::string>
    OutPath("out", llvm::cl::desc("Path of the ir_src.json file to write."),
            llvm::cl::value_desc("out.json"), llvm::cl::Required,
            llvm::cl::cat(cat));

llvm::cl::list<std::string> CXXFlags("cxxflags",
                                     llvm::cl::desc("Additional CXXFLAGS"),
                                     llvm::cl::value_desc("cxxflags"),
                                     llvm::cl::ZeroOrMore, llvm::cl::cat(cat));

} // namespace

int main(int argc, char *argv[]) {
  llvm::cl::HideUnrelatedOptions(cat);
  llvm::cl::ParseCommandLineOptions(argc, argv);

  llvm::SmallVector<llvm::StringRef, 4> cxx_flags(CXXFlags.begin(),
                                                  CXXFlags.end());
  fs::path dir = SrcDir.getValue();
  llvm::json::Object root;
  for (const char *name : {"src.c", "src.cpp"}) {
    auto path = dir / name;
    if (!fs::exists(path)) {
      continue;
    }
    llvm::errs() << "Preprocessing " << path.string() << '\n';
    llvm::json::Object file_root;
    cpp2rust::Extract(path, file_root, cxx_flags);
    for (auto &[k, v] : file_root) {
      if (!root.try_emplace(k, std::move(v)).second) {
        llvm::errs() << "ERROR: rule name " << k.str()
                     << " defined in multiple files in " << dir.string()
                     << '\n';
        return EXIT_FAILURE;
      }
    }
  }

  fs::path out_path = OutPath.getValue();
  std::error_code ec;
  llvm::raw_fd_ostream out(out_path.string(), ec);
  if (ec) {
    llvm::errs() << "ERROR: failed to open " << out_path.string() << ": "
                 << ec.message() << '\n';
    return EXIT_FAILURE;
  }
  out << llvm::formatv("{0:2}", llvm::json::Value(std::move(root))) << '\n';
  return EXIT_SUCCESS;
}
