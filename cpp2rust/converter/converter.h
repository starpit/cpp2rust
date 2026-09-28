#pragma once

// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Sema/Sema.h>

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "converter/converter_lib.h"
#include "converter/factory.h"
#include "converter/lex.h"
#include "converter/translation_rule.h"
#include "logging.h"

namespace cpp2rust {
inline constexpr const char kDestructorName[] = "destructor";
class Converter : public clang::RecursiveASTVisitor<Converter> {

public:
  explicit Converter(std::string &rs_code, clang::ASTContext &ctx,
                     const char *keyword_unsafe = "unsafe",
                     const char *keyword_mut = keyword::kMut)
      : rs_code_(&rs_code), ctx_(ctx), keyword_unsafe_(keyword_unsafe),
        keyword_mut_(keyword_mut) {}

  virtual ~Converter() = default;

  Converter(const Converter &) = delete;
  Converter &operator=(const Converter &) = delete;
  Converter(Converter &&) = delete;
  Converter &operator=(Converter &&) = delete;

  void SetSema(clang::Sema &sema) { sema_ = &sema; }

  auto &GetSema() {
    assert(sema_ && "sema_ should already be set");
    return *sema_;
  }

  bool VisitRecoveryExpr(clang::RecoveryExpr *expr);

  virtual void EmitFilePreamble();

  static void EmitOpaqueRecords(std::string &out);
  static void EmitGlobalInits(Model model, std::string &out);

  static void EmitVirtualMethods(std::string &out);

  virtual bool VisitBuiltinType(clang::BuiltinType *type);

  virtual bool VisitRecordType(clang::RecordType *type);

  virtual bool VisitConstantArrayType(clang::ConstantArrayType *type);

  virtual bool VisitIncompleteArrayType(clang::IncompleteArrayType *type);

  virtual bool VisitReferenceType(clang::ReferenceType *type);

  virtual bool VisitPointerType(clang::PointerType *type);

  enum class FnProtoType { LambdaCallOperator, FnPtr };

  virtual std::string
  ConvertFunctionPointerType(const clang::FunctionProtoType *proto,
                             FnProtoType kind = FnProtoType::FnPtr);

  virtual bool VisitDecayedType(clang::DecayedType *type);

  virtual bool VisitTypedefType(clang::TypedefType *type);

  virtual bool VisitUsingType(clang::UsingType *type);

  virtual bool VisitTranslationUnitDecl(clang::TranslationUnitDecl *decl);

  virtual bool VisitFunctionDecl(clang::FunctionDecl *decl);

  virtual void EmitFunctionPreamble(clang::FunctionDecl *decl);

  virtual void ConvertFunctionBody(clang::FunctionDecl *decl);

  void ConvertGotoBlock(clang::CompoundStmt *body);

  void ConvertBody(clang::Stmt *body);

  void ConvertBodyStmts(clang::Stmt *body);

  void EmitHoistedDecls(clang::CompoundStmt *body);

  virtual bool VisitFunctionTemplateDecl(clang::FunctionTemplateDecl *decl);
  bool VisitVarTemplateDecl(clang::VarTemplateDecl *decl);

  virtual bool VisitVarDecl(clang::VarDecl *decl);
  virtual bool LazyStaticInit() const { return true; }
  virtual std::string ForceGlobalInit(const clang::VarDecl *decl);

  void ConvertVarDecl(clang::VarDecl *decl);

  virtual void EmitHoistedInArmAssignment(clang::VarDecl *decl);

  void ConvertVarDeclInitializer(clang::VarDecl *decl);

  virtual void ConvertGlobalVarDecl(clang::VarDecl *decl);

  virtual void ConvertVaListVarDecl(clang::VarDecl *decl);

  virtual bool ConvertVarDeclSkipInit(clang::VarDecl *decl);

  virtual bool ConvertLambdaVarDecl(clang::VarDecl *decl);

  bool VisitRecordDecl(clang::RecordDecl *decl);

  virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *decl);

  virtual void EmitRustStructOrUnion(clang::RecordDecl *decl);

  void EmitReprC(clang::RecordDecl *decl);
  virtual void EmitRustUnion(clang::RecordDecl *decl);

  virtual bool EmitsReprCForRecords() const { return true; }

  virtual const char *CharRustType() const { return "libc::c_char"; }

  virtual bool VisitCXXMethodDecl(clang::CXXMethodDecl *decl);

  virtual bool ShouldConvertMethod(const clang::CXXMethodDecl *decl);

  virtual bool ConvertOutOfLineMethod(clang::CXXMethodDecl *decl);

  bool ConvertCXXMethodDecl(clang::CXXMethodDecl *decl);

  std::string GetMethodName(const clang::CXXMethodDecl *decl);
  std::string EmittedMethodKey(const clang::CXXMethodDecl *decl);

  virtual std::string GetSelfMaybeWithMut(const clang::CXXMethodDecl *decl);

  std::string GetCtorName(clang::CXXConstructorDecl *decl);

  virtual void ConvertCXXRecordMethods(clang::CXXRecordDecl *decl);

  virtual void ConvertLateInstantiatedMethods(clang::CXXRecordDecl *decl);

  virtual std::string DestroyMembers(const clang::CXXRecordDecl *decl);

  virtual void EmitScopedDestructor(const clang::VarDecl *decl);

  void EmitDeallocation(clang::CXXDeleteExpr *expr,
                        const std::string &argument_as_string);

  virtual void SetUFCSReceiver(clang::Expr *base, bool is_arrow,
                               const clang::CXXMethodDecl *method);

  void ConvertUserOperatorCall(clang::CXXOperatorCallExpr *expr);

  virtual std::string GetUFCSName(const clang::CXXMethodDecl *method) const;

  virtual bool ThisIsRustPtr() const { return false; }

  virtual void ConvertCXXConstructorBody(clang::CXXConstructorDecl *decl);
  void EmitConstructorFieldInits(clang::CXXConstructorDecl *decl);
  // Emits `{ctor_name}_at(__cc2_this: *mut Self, ..)` plus a `{ctor_name}(..) ->
  // Self` wrapper delegating to it -- the only shape in which a field
  // initializer that reads through `this` can be lowered.
  void EmitInPlaceConstructor(clang::CXXConstructorDecl *decl,
                             const std::string &ctor_name);

  virtual bool VisitCXXConstructorDecl(clang::CXXConstructorDecl *decl);

  virtual bool VisitFieldDecl(clang::FieldDecl *decl);

  virtual bool VisitNamespaceDecl(clang::NamespaceDecl *decl);

  virtual bool VisitTypedefDecl(clang::TypedefDecl *decl);
  virtual bool VisitTypeAliasDecl(clang::TypeAliasDecl *decl);
  virtual bool VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl *decl);

  bool VisitStaticAssertDecl(clang::StaticAssertDecl *decl);
  bool VisitConceptDecl(clang::ConceptDecl *decl);

  virtual bool VisitCompoundStmt(clang::CompoundStmt *stmt);

  virtual bool VisitDeclStmt(clang::DeclStmt *stmt);

  virtual bool VisitReturnStmt(clang::ReturnStmt *stmt);

  virtual bool VisitGotoStmt(clang::GotoStmt *stmt);

  void ConvertCondition(clang::Expr *cond);

  virtual bool VisitIfStmt(clang::IfStmt *stmt);

  virtual bool VisitWhileStmt(clang::WhileStmt *stmt);

  virtual bool VisitDoStmt(clang::DoStmt *stmt);

  virtual bool VisitForStmt(clang::ForStmt *stmt);

  virtual bool VisitCXXForRangeStmt(clang::CXXForRangeStmt *stmt);

  virtual bool VisitCXXForRangeStmtMap(clang::CXXForRangeStmt *stmt);

  virtual bool VisitCXXForRangeStmtVector(clang::CXXForRangeStmt *stmt);

  virtual bool VisitCXXForRangeStmtString(clang::CXXForRangeStmt *stmt);

  bool VisitCXXForRangeStmtIndexBased(clang::CXXForRangeStmt *stmt,
                                      const char *len_suffix);

  void ConvertForRangeBody(clang::CXXForRangeStmt *stmt,
                           const clang::VarDecl *map_iter_decl = nullptr);

  virtual bool VisitBreakStmt(clang::BreakStmt *stmt);

  virtual bool VisitContinueStmt(clang::ContinueStmt *stmt);

  bool GetFmtArg(clang::Expr *arg, std::string &fmt, std::string &fmt_args,
                 const char *&fmt_trait, std::string &fmt_width);

  bool GetRawArg(clang::Expr *arg, std::string &raw_args);

  void ConvertCallToOstream(clang::CallExpr *expr);
  virtual std::string ConvertStream(clang::Expr *expr);
  // `std::flush` on an already-converted stream expression. Model-specific:
  // the refcount model's stream is a `libcc2rs::Ptr<File>`, which is not
  // itself a `Write` and does not autoderef to one.
  virtual std::string FlushStream(const std::string &stream);

  struct TempMaterializationCtx {
    std::vector<std::optional<clang::QualType>> materialized_args;
    std::string temporary_bindings;

    TempMaterializationCtx(size_t num_args)
        : materialized_args(num_args), materialized_refs_(num_args) {}

    const std::string &GetOrMaterialize(
        unsigned argument_num,
        std::function<std::pair<std::string, std::string>(const std::string &,
                                                          clang::QualType)>
            materialize_fn);

  private:
    std::vector<std::string> materialized_refs_;
  };

  struct PlaceholderCtx {
    unsigned arg_idx;
    std::optional<clang::QualType> implicit_convert_to;
    TempMaterializationCtx *materialize_ctx;
    int materialize_idx; // <0 = no idx, >=0 idx valid
    TranslationRule::Access access;
    bool is_receiver;
    bool is_cpp_ptr;
    bool maps_to_rust_ptr;
    bool declared_in_rule_as_rust_ptr;
    bool is_index_base;
    // The rule declared this parameter as `&mut T`, and the placeholder is NOT
    // the receiver of a method call in the rule body (where Rust's autoref
    // supplies the `&mut` for us). Such a placeholder sits in argument
    // position of an inlined call, so the emitted text must be an explicit
    // `&mut <place>` reborrow.
    bool needs_explicit_mut_borrow = false;
    // Same, for a `&T` (shared) declaration. Distinct from the `&mut` case
    // because it is NOT sufficient on its own: unlike `&mut`, whose emission is
    // always a place (ConvertLValue), the emission for a shared-`&` parameter is
    // SOMETIMES ALREADY A RUST REFERENCE -- a C string literal comes out as
    // `c"x"`, which is `&CStr` -- and blind prefixing would give `&&CStr`. So
    // this flag only says "the declaration wants a borrow"; whether one is
    // actually added is decided by `emitted_a_reference_` after the emission.
    bool needs_explicit_shared_borrow = false;

    bool needs_materialization() const {
      return materialize_ctx && materialize_idx >= 0 &&
             declared_in_rule_as_rust_ptr && !is_cpp_ptr && !maps_to_rust_ptr;
    }

    bool needs_pointer_receiver() const {
      return is_receiver && !maps_to_rust_ptr && declared_in_rule_as_rust_ptr;
    }

    bool needs_object_receiver() const {
      return is_receiver && is_cpp_ptr && !declared_in_rule_as_rust_ptr;
    }

    bool needs_ptr_wrap() const {
      return !is_receiver && !is_cpp_ptr && !maps_to_rust_ptr &&
             declared_in_rule_as_rust_ptr;
    }

    bool needs_lvalue() const {
      return access == TranslationRule::Access::kBorrowMut;
    }

    bool needs_mut_borrow() const {
      return needs_explicit_mut_borrow &&
             access == TranslationRule::Access::kBorrowMut;
    }

    void dump() const;
  };

  std::optional<TempMaterializationCtx> ConvertCallExpr(clang::CallExpr *expr);

  struct CallArg {
    enum class Kind : int8_t {
      Hoisted,
      Inline,
      Materialized,
    };

    std::string param_name;
    std::string ref_temp_name;
    clang::QualType param_type;
    clang::Expr *expr;
    bool has_default;
    Kind kind;
  };

  struct CallInfo {
    std::vector<CallArg> args;
    std::vector<clang::Expr *> variadic_args;
    clang::CallExpr *expr;
    // Non-null when the callee survived instantiation as an unresolved
    // OverloadExpr and CollectCallInfo picked the unique exact-match
    // candidate. EmitCall must then spell that decl instead of converting the
    // callee subexpression, which would hit VisitUnresolvedLookupExpr.
    const clang::FunctionDecl *resolved_overload = nullptr;
    // Non-null when the callee is a call THROUGH a pointer-to-member whose
    // member pointer is a compile-time constant (a non-type template argument
    // or an `&`-of-member literal), so the call is a DIRECT call and the callee
    // is statically known. EmitCall then spells the member function instead of
    // converting the `->*`, which is what reaches VisitBinaryOperator's
    // pointer-to-member abort. Stays null for a member pointer that is a
    // runtime value -- that case must keep aborting loudly.
    const clang::CXXMethodDecl *ptr_mem_callee = nullptr;
    // The object expression, i.e. the LHS of the `->*`/`.*`.
    clang::Expr *ptr_mem_object = nullptr;
    // True for `->*` (BO_PtrMemI), false for `.*` (BO_PtrMemD).
    bool ptr_mem_is_arrow = false;
    bool is_variadic;
    bool is_fn_ptr_call;
    bool is_libc_passthrough;
  };

  CallInfo CollectCallInfo(clang::CallExpr *expr);

  // Resolves a call whose callee is still an OverloadExpr
  // (UnresolvedLookupExpr / UnresolvedMemberExpr, type `<overloaded function
  // type>`) by exact argument-type match. Returns nullptr unless exactly one
  // non-instance candidate matches, so ambiguity stays LOUD.
  const clang::FunctionDecl *ResolveOverloadedCallee(clang::CallExpr *expr);

  // LOUD failure for a call with no callee decl, no prototype and no
  // resolvable overload set. Names the callee, every candidate with its
  // parameter types in the mapper's spelling, the supplied argument types and
  // the location. Mirrors ReportUnsupportedOperatorCall.
  void ReportUnresolvedCall(clang::CallExpr *expr, clang::Expr *callee);

  void ConvertParamTy(clang::QualType param_type, clang::Expr *expr);

  // Emits a pointer-type adjustment (const/mut fixup or reinterpret cast)
  // after `expr` has been converted, for cases where the argument's Rust
  // pointee type differs from the parameter's Rust pointee type even though
  // Clang did not insert an implicit cast node for the call argument (e.g.
  // when two C types are canonically identical, such as `size_t` and
  // `unsigned long`, but map to different Rust types).
  virtual void ConvertParamTyPointerCastIfNeeded(clang::QualType param_type,
                                                 clang::Expr *expr);

  virtual bool FunctionPointerCastNeedsTransmute() const { return true; }

  void ConvertFunctionPointerTransmute(clang::Expr *expr, clang::QualType type);

  void EmitHoistedArgs(CallInfo &info);

  void EmitArgList(const CallInfo &info);

  void EmitCall(CallInfo &&info);

  void ConvertGenericCallExpr(clang::CallExpr *expr);

  virtual void EmitFnPtrCall(clang::Expr *callee);

  virtual void
  ConvertFunctionToFunctionPointer(const clang::FunctionDecl *fn_decl);

  std::string GetFunctionRefName(const clang::FunctionDecl *fn_decl);

  std::string ConvertFnPtrCallee(clang::Expr *arg);
  virtual std::string ConvertFnPtrPlaceholder(clang::Expr *arg);

  // Option<fn> implements Copy
  virtual bool FunctionPointerImplementsCopy() const { return true; }

  bool TypeIsCopyable(clang::QualType ty) const {
    if (ty->isFunctionPointerType() || ty->isFunctionType()) {
      return FunctionPointerImplementsCopy();
    }
    if (ty->isBuiltinType() || ty->isEnumeralType()) {
      return true;
    }
    if (auto *record = ty->getAsRecordDecl()) {
      return RecordDerivesCopy(record);
    }
    return false;
  }

  virtual void ConvertPrintf(clang::CallExpr *expr);

  void ConvertVAArgCall(clang::CallExpr *expr);

  virtual void ConvertVariadicArg(clang::Expr *arg);

  void DefineImplicitMembers(clang::CXXRecordDecl *decl);

  virtual bool VisitCallExpr(clang::CallExpr *expr);

  virtual bool VisitIntegerLiteral(clang::IntegerLiteral *expr);

  virtual bool VisitFloatingLiteral(clang::FloatingLiteral *expr);

  virtual bool VisitCharacterLiteral(clang::CharacterLiteral *expr);

  std::string GetCodeUnitArrayLiteral(const clang::StringLiteral *expr);
  bool IsArrayInitContext() const;

  std::string GetEscapedUTF8CharLiteral(clang::Expr *expr) const;

  std::string GetEscapedStringLiteral(clang::Expr *expr,
                                      uint64_t pad_nulls = 0) const;

  // Doubles `{` and `}` so that LITERAL C++ text can be spliced into a Rust
  // format literal (`write!`/`format!`/`println!`). In a Rust format string
  // both braces are metacharacters, so an unescaped one is either a hard error
  // ("invalid format string: unmatched `}`") or -- when the surrounding text
  // happens to parse as a placeholder, e.g. a literal `{0}` next to a real
  // argument -- SILENTLY DIFFERENT OUTPUT.
  //
  // CALL THIS ONLY ON LITERAL TEXT, never on a placeholder this converter
  // itself synthesised: doubling a real `{}` prints `{}` instead of the value,
  // which is silent wrongness in the opposite direction. Where a caller both
  // escapes literals and inserts placeholders (printf2fmt), escape FIRST and
  // insert placeholders AFTER, so the inserted ones are never re-escaped.
  static std::string EscapeFmtBraces(std::string_view text);
  virtual bool VisitStringLiteral(clang::StringLiteral *expr);

  virtual bool VisitCXXBoolLiteralExpr(clang::CXXBoolLiteralExpr *expr);

  void ConvertIntegerToEnumeralCast(clang::Expr *to, clang::Expr *from);

  void ConvertIntegralToBooleanCast(clang::ImplicitCastExpr *expr);

  virtual bool VisitImplicitCastExpr(clang::ImplicitCastExpr *expr);

  virtual bool VisitExplicitCastExpr(clang::ExplicitCastExpr *expr);

  virtual bool VisitBinaryOperator(clang::BinaryOperator *expr);
  bool VisitCXXRewrittenBinaryOperator(clang::CXXRewrittenBinaryOperator *expr);

  virtual void ConvertBinaryOperator(clang::BinaryOperator *expr);

  virtual bool ConvertIncAndDec(clang::UnaryOperator *expr);

  virtual bool VisitUnaryOperator(clang::UnaryOperator *expr);

  virtual bool VisitStmtExpr(clang::StmtExpr *expr);

  // C++ exceptions have NO lowering. These three exist only so the failure is
  // LOUD. Before they were added, RecursiveASTVisitor's default Visit for each
  // returned true, so `Convert(stmt)` (converter.cpp:1384) traversed straight
  // INTO the children: a `throw MyExc("boom");` emitted only the CONSTRUCTION
  // of the exception object as a discarded statement expression and control
  // fell through, so a TU with a `throw` translated rc=0, with a zero-byte log
  // and zero placeholders, while being semantically wrong -- C++ terminates
  // (rc=134), the Rust printed the statement after the call and exited 0. A
  // silently-not-thrown throw and a silently-not-catching catch are the worst
  // failure mode this project has: every metric scores them as success.
  virtual bool VisitCXXThrowExpr(clang::CXXThrowExpr *expr);
  virtual bool VisitCXXTryStmt(clang::CXXTryStmt *stmt);
  virtual bool VisitCXXCatchStmt(clang::CXXCatchStmt *stmt);
  // Shared reporter for the three: names the construct, the type and the
  // location; records-and-continues under --survey, aborts otherwise.
  void ReportUnsupportedException(const clang::Stmt *stmt,
                                  const std::string &detail);

  virtual void EmitStmtExprTail(clang::Expr *tail);

  virtual bool VisitConditionalOperator(clang::ConditionalOperator *expr);

  virtual bool VisitDeclRefExpr(clang::DeclRefExpr *expr);
  std::string ConvertDeclRefExpr(clang::DeclRefExpr *expr);

  // An ADL name clang could not resolve. There is no lowering; this exists only
  // so the failure is LOUD and names the construct instead of traversing to
  // nothing and tripping the sentinel assert in Convert(Expr*, ...).
  virtual bool VisitUnresolvedLookupExpr(clang::UnresolvedLookupExpr *expr);

  virtual bool VisitParenExpr(clang::ParenExpr *expr);

  void ConvertMemberExpr(clang::MemberExpr *expr);

  // A `this`-bearing expression reached with `curr_function_ == nullptr`.
  // Loud, named refusal: the `this` -> `this`/`self` choice is not derivable
  // here, and guessing it is the NSDMI-reads-0-instead-of-7 failure class.
  void ReportThisWithoutEnclosingFunction(const clang::Expr *expr,
                                         const std::string &what);

  // Loud, named refusal for the terminal `else` of ConvertMemberExpr: a member
  // whose DeclName kind has no name lowering. Emits no token and is fatal.
  void ReportUnsupportedMemberName(const clang::MemberExpr *expr,
                                   const clang::NamedDecl *member);

  virtual bool VisitMemberExpr(clang::MemberExpr *expr);

  virtual bool VisitCXXThisExpr(clang::CXXThisExpr *expr);

  virtual bool VisitInitListExpr(clang::InitListExpr *expr);
  bool VisitOpaqueValueExpr(clang::OpaqueValueExpr *expr);
  bool VisitArrayInitIndexExpr(clang::ArrayInitIndexExpr *expr);
  virtual bool VisitArrayInitLoopExpr(clang::ArrayInitLoopExpr *expr);

  virtual bool VisitCompoundLiteralExpr(clang::CompoundLiteralExpr *expr);

  virtual bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr *expr);

  virtual bool VisitCXXNullPtrLiteralExpr(clang::CXXNullPtrLiteralExpr *expr);

  virtual bool VisitGNUNullExpr(clang::GNUNullExpr *expr);

  virtual bool VisitCXXNewExpr(clang::CXXNewExpr *expr);

  virtual bool VisitCXXDeleteExpr(clang::CXXDeleteExpr *expr);

  virtual bool VisitCXXConstructExpr(clang::CXXConstructExpr *expr);

  void ConvertCXXConstructExprArgs(clang::CXXConstructExpr *expr);

  virtual void ConvertArrayCXXConstructExpr(clang::CXXConstructExpr *expr);

  virtual bool
  VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr *expr);

  virtual bool VisitTypeTraitExpr(clang::TypeTraitExpr *expr);

  virtual bool VisitSizeOfPackExpr(clang::SizeOfPackExpr *expr);

  virtual bool
  VisitConceptSpecializationExpr(clang::ConceptSpecializationExpr *expr);
  virtual bool VisitRequiresExpr(clang::RequiresExpr *expr);

  virtual bool VisitOffsetOfExpr(clang::OffsetOfExpr *expr);

  virtual bool VisitEnumDecl(clang::EnumDecl *decl);

  virtual std::string EnumeratorName(const clang::EnumConstantDecl *decl) const;

  virtual bool VisitCXXDefaultArgExpr(clang::CXXDefaultArgExpr *expr);
  virtual bool VisitConstantExpr(clang::ConstantExpr *expr);

  static void CollectLambdaCallOperatorInstantiations(
      const clang::CXXRecordDecl *lambda_class,
      llvm::SmallVectorImpl<clang::CXXMethodDecl *> &out);
  clang::CXXMethodDecl *SelectLambdaCallOperator(clang::LambdaExpr *expr);
  virtual bool VisitLambdaExpr(clang::LambdaExpr *expr);

  virtual bool VisitImplicitValueInitExpr(clang::ImplicitValueInitExpr *expr);
  virtual bool VisitCXXScalarValueInitExpr(clang::CXXScalarValueInitExpr *expr);

  virtual bool VisitSwitchStmt(clang::SwitchStmt *stmt);

  void EmitSwitchArm(const SwitchArm &arm, bool is_default);

  bool ConvertSwitchCaseCondition(clang::SwitchCase *stmt);

  virtual bool VisitVAArgExpr(clang::VAArgExpr *expr);

  virtual bool VisitCXXDefaultInitExpr(clang::CXXDefaultInitExpr *expr);

  virtual bool VisitPredefinedExpr(clang::PredefinedExpr *expr);

  virtual bool VisitClassTemplateDecl(clang::ClassTemplateDecl *decl);

  virtual bool
  VisitCXXStdInitializerListExpr(clang::CXXStdInitializerListExpr *expr);

protected:
  const clang::Expr *GetParentExpr(const clang::Expr *expr);

#define StrCat(...) _StrCat(__FUNCTION__, __LINE__, __VA_ARGS__)

  inline bool is_empty(char c) { return false; }
  inline bool is_empty(const char *s) { return s == nullptr || *s == '\0'; }
  template <size_t N> inline bool is_empty(const char (&s)[N]) {
    return s[0] == '\0';
  }
  template <typename T> inline bool is_empty(const T &s) { return s.empty(); }

  template <typename... Ts>
  inline void _StrCat(const char *func, int line, const Ts &...vals) {
    log() << '[' << func << ':' << line << "] ";
    ((log() << vals << '\n', *rs_code_ += vals,
      (is_empty(vals) ? void() : void(*rs_code_ += ' '))),
     ...);
  }

  class Buffer {
    std::string partial_code;
    std::string *full_code;
    Converter &c;

  public:
    Buffer(Converter &c) : full_code(c.rs_code_), c(c) {
      c.rs_code_ = &partial_code;
    }
    ~Buffer() { c.rs_code_ = full_code; }
    std::string str() && { return std::move(partial_code); }
  };

  template <auto kOpen, auto kClose> class PushDelim {
    Converter &c;
    bool enabled;

  public:
    PushDelim(Converter &c, bool enabled = true) : c(c), enabled(enabled) {
      if (enabled) {
        c.StrCat(kOpen);
      }
    }
    ~PushDelim() {
      if (enabled) {
        c.StrCat(kClose);
      }
    }
    PushDelim(const PushDelim &) = delete;
    PushDelim(PushDelim &&) = delete;
    PushDelim &operator=(const PushDelim &) = delete;
    PushDelim &operator=(PushDelim &&) = delete;
  };

  using PushBrace =
      PushDelim<token::kOpenCurlyBracket, token::kCloseCurlyBracket>;
  using PushParen = PushDelim<token::kOpenParen, token::kCloseParen>;
  using PushBracket = PushDelim<token::kOpenBracket, token::kCloseBracket>;
  using PushLazyType = PushDelim<token::kLazyCellType, token::kGt>;
  using PushLazyInit = PushDelim<token::kLazyCellNew, token::kCloseParen>;

  template <typename T>
  inline std::string
  ToString(T node, std::optional<clang::QualType> implicit_convert_to = {}) {
    Buffer buf(*this);
    if constexpr (std::is_convertible_v<T, clang::Expr *>) {
      Convert(node, implicit_convert_to);
    } else {
      Convert(node);
    }
    return std::move(buf).str();
  }

  template <typename T> inline std::string ToStringBase(T node) {
    Buffer buf(*this);
    Converter::Convert(node);
    return std::move(buf).str();
  }

  virtual bool Convert(clang::QualType qual_type);
  virtual bool ConvertMappedType(clang::QualType qual_type);

  virtual std::string ConvertPointeeType(clang::QualType ptr_type);

  virtual bool Convert(clang::Decl *decl);
  virtual bool Convert(clang::Stmt *stmt);
  virtual bool Convert(clang::Expr *expr,
                       std::optional<clang::QualType> implicit_convert_to = {});

  virtual std::string GetDefaultAsString(clang::QualType qual_type);

  virtual std::string GetArrayDefaultAsString(clang::QualType qual_type);

  virtual std::string GetDefaultAsStringFallback(clang::QualType qual_type);

  virtual std::string ConvertVarDefaultInit(clang::QualType qual_type);

  virtual std::string
  GetOverloadedFunctionName(const clang::FunctionDecl *decl);

  // Folds anything ToIdentifier() left behind into [A-Za-z0-9_] so a rule
  // target type carrying Rust-only punctuation (`&`, a lifetime's `'`) cannot
  // become a function name rustc refuses to parse; refuses loudly, naming the
  // offending type, if a name still is not a valid identifier.
  void ForceRustIdentifier(std::string &name, const clang::FunctionDecl *decl);

  virtual std::string GetRecordName(const clang::NamedDecl *decl) const;

  virtual std::vector<const char *>
  GetStructAttributes(const clang::RecordDecl *decl);

  virtual std::string GetUnsafeTypeAsString(clang::QualType qual_type) const;

  virtual bool NeedsMut(const clang::VarDecl *decl, clang::QualType type,
                        llvm::StringRef name) const;

  virtual void ConvertVarInit(clang::QualType qual_type, clang::Expr *expr);

  virtual void ConvertUnsignedArithOperand(clang::Expr *expr,
                                           clang::QualType type);

  virtual void ConvertEqualsNullPtr(clang::Expr *expr);

  virtual void ConvertPointerSubscript(clang::ArraySubscriptExpr *expr);

  virtual void ConvertPointerOffset(clang::Expr *base, clang::Expr *idx,
                                    bool is_addition = true);

  virtual void ConvertArraySubscript(clang::Expr *base, clang::Expr *idx,
                                     clang::QualType type);

  void EmitFlexibleArrayElementPtr(clang::Expr *array, clang::Expr *idx,
                                   bool is_mut);

  virtual void ConvertAssignment(clang::Expr *lhs, clang::Expr *rhs,
                                 std::string_view assign_operator);

  // Collects the Rust lifetime binders appearing in this function's rendered
  // parameter and return types and returns them as a generic parameter list
  // (`"<'a>"`, or `""` when there are none) to be emitted directly after the
  // function name. Without this, a lifetime coming out of a rule TARGET -- the
  // faithful shape of a non-owning callable reference such as
  // `llvm::function_ref` -- reached the output undeclared. See the definition.
  std::string GetLifetimeBinders(clang::FunctionDecl *decl);

  virtual void ConvertFunctionParameters(clang::FunctionDecl *decl);

  virtual void ConvertFunctionQualifiers(clang::FunctionDecl *decl);

  virtual void ConvertFunctionReturnType(clang::FunctionDecl *decl);

  virtual void ConvertFunctionMain(const clang::FunctionDecl *decl,
                                   const std::string_view main_function_name);

  virtual void ConvertAbstractClass(clang::CXXRecordDecl *decl);

  void ConvertCXXMethodDecls(const clang::CXXRecordDecl *decl,
                             const std::string_view signature,
                             bool (*predicate)(clang::CXXMethodDecl *));

  void ConvertVirtualMethods(clang::CXXRecordDecl *decl);

  bool ConvertOutOfLineVirtualMethod(clang::CXXMethodDecl *decl);

  void AddOrdTrait(const clang::CXXRecordDecl *decl);

  void ConvertOrdAndPartialOrdTraits(const clang::CXXRecordDecl *decl,
                                     const clang::FunctionDecl *eq,
                                     const clang::FunctionDecl *lt,
                                     const clang::FunctionDecl *cmp);

  void ConvertOrdAndPartialOrdTraitsBase(std::string_view cmp_body,
                                         std::string_view eq_body,
                                         std::string_view record_name);

  std::string GetComparisonCall(const clang::FunctionDecl *op,
                                const clang::CXXRecordDecl *decl,
                                std::string_view lhs, std::string_view rhs);

  virtual std::string
  GetComparisonReferenceArg(const clang::CXXRecordDecl *decl,
                            std::string_view value);

  virtual std::string GetComparisonReceiver(const clang::CXXMethodDecl *method,
                                            const clang::CXXRecordDecl *decl,
                                            std::string_view lhs);

  virtual void AddCloneTrait(const clang::RecordDecl *decl);

  virtual void AddDefaultTrait(const clang::RecordDecl *decl);

  virtual void AddDefaultTraitForUnion(const clang::RecordDecl *decl);

  void EmitDefaultStructLiteral(const clang::RecordDecl *decl);

  virtual void AddByteReprTrait(const clang::RecordDecl *decl);

  virtual void
  ConvertUnsignedArithBinaryOperator(clang::BinaryOperator *binary_operator,
                                     clang::Expr *expr);

  virtual void ConvertAddrOf(clang::Expr *expr, clang::QualType pointer_type);

  virtual void ConvertDeref(clang::Expr *expr);

  void EmitDeref(std::string inner, clang::QualType pointee_type);

  virtual void ConvertArrow(clang::Expr *expr);

  virtual void ConvertCast(clang::QualType qual_type,
                           int line = __builtin_LINE());

  // ⛔ RUST'S `as` REFUSES A BLOCK EXPRESSION AS ITS LEFT OPERAND.
  // `if c { &mut *x } else { &mut *y } as *mut ()` is a PARSE error --
  //   error: expected expression, found `as`
  //   help: parentheses are required to parse this as an expression
  // -- because a block-form expression in statement-ish position terminates at
  // its closing `}` and `as` then has nothing to its left. The converter emits
  // EVERY C++ conditional operator as an `if`/`else` BLOCK
  // (VisitConditionalOperator), so any path that appends a cast to a converted
  // sub-expression can produce this, and it is invisible to a placeholder
  // census: the file emits, rc=1 comes only from `failed to run rustfmt`, and
  // the line count looks healthy.
  //
  // `operand_start` is the offset in the CURRENT `rs_code_` buffer at which the
  // cast's operand began. If the text emitted from there starts a block, wrap
  // it in parentheses retroactively -- the operand text is already in a plain
  // `std::string`, so the insert is exact and needs no re-emission.
  //
  // ⛔ THE FIX IS THE PARENTHESES, NEVER DROPPING THE CAST: a pointer coercion
  // that disappears is a SILENT TYPE CHANGE, which is strictly worse than a
  // parse error because nothing downstream reports it.
  void ParenthesizeBlockCastOperand(size_t operand_start);

  // `hoisted_range_name`, when non-empty, names a local the caller has already
  // bound the range init to; the range init is then NOT re-emitted here. See
  // the hoist comment on VisitCXXForRangeStmtIndexBased.
  virtual void ConvertLoopVariable(clang::VarDecl *decl,
                                   clang::Expr *range_init,
                                   const std::string &index_name = {},
                                   const std::string &hoisted_range_name = {});

  bool EmitVectorDecompositionBindings(const clang::DecompositionDecl *decl,
                                       const std::string &holder_name);

  bool IsHoistFreeDecompositionRange(clang::CXXForRangeStmt *stmt);

  virtual void ConvertUniquePtrDeref(clang::CXXOperatorCallExpr *expr);

  virtual bool ConvertCXXOperatorCallExpr(clang::CXXOperatorCallExpr *expr);

  // Loud, actionable report for an overloaded-operator call with no lowering.
  // Callable (not a `default:` body) so the OO_LessLess arm can reach it.
  void ReportUnsupportedOperatorCall(clang::CXXOperatorCallExpr *expr);

  // Loud, actionable report for a SYSTEM record type with no types_ rule, which
  // would otherwise be mangled into an identifier nothing ever defines.
  void ReportUnmappedSystemType(const clang::RecordDecl *decl);
  // C++17 structured bindings are not lowered yet; name the construct loudly
  // instead of emitting an undefined Rust name for each binding.
  void ReportUnsupportedStructuredBinding(const clang::DecompositionDecl *decl);
  bool ConvertTupleDecompositionDecl(clang::DecompositionDecl *decl);
  // Name of the synthetic iterator variable a decomposing map for-range binds.
  // Carries line and column so two loops nested in one another -- e.g.
  // RegDefTracker.cpp:127 and :128 -- get DISTINCT names.
  std::string GetDecompositionIterName(const clang::DecompositionDecl *decl);
  // Emits `let <b0> = <iter>.first(); let <b1> = <iter>.second();`.
  // Returns false (emitting nothing) if the shape is not a 2-binding
  // decomposition, so the caller keeps the loud diagnostic.
  static bool IsMapLikeRangeClass(const std::string &class_name);
  static const char *MapRangeIteratorName(const std::string &class_name);
  // `true` for the range classes whose modelled iterator exposes the key/value
  // accessors as the INHERENT `key_ptr()` / `value_ptr()` rather than as the
  // `MapIterator` trait's `first()` / `second()`. See
  // MapDecompositionUsesPtrAccessors's definition for why the two families
  // cannot be unified in libcc2rs.
  static bool MapDecompositionUsesPtrAccessors(const std::string &class_name);
  bool EmitMapDecompositionBindings(const clang::DecompositionDecl *decl,
                                    const std::string &iter_name,
                                    bool ptr_accessors = false);

  std::string GetMappedAsString(clang::Expr *expr, clang::Expr **args = nullptr,
                                unsigned num_args = 0,
                                TempMaterializationCtx *ctx = nullptr);

  std::string
  ConvertIRFragment(const std::vector<TranslationRule::BodyFragment> &fragments,
                    clang::Expr *expr, clang::Expr **args, unsigned num_args,
                    TempMaterializationCtx *ctx,
                    bool is_method_call_receiver = false);

  std::string ConvertPlaceholder(clang::Expr *expr, clang::Expr *arg,
                                 const PlaceholderCtx &ph_ctx);
  std::string ConvertPlaceholderImpl(clang::Expr *expr, clang::Expr *arg,
                                     const PlaceholderCtx &ph_ctx);

  std::string ConvertVariadicTail(clang::Expr *expr,
                                  const std::vector<clang::Expr *> &all_args);

  std::string ConvertInitFragment(clang::Expr *expr,
                                  const std::vector<clang::Expr *> &all_args);

  void ConvertConstructFromArgs(clang::QualType type,
                                llvm::ArrayRef<clang::Expr *> args,
                                clang::SourceLocation loc);

  virtual void ConvertConstructedValue(clang::QualType type,
                                       clang::CXXConstructExpr *ctor);

  virtual std::string ConvertMappedMethodCall(
      clang::Expr *expr, const TranslationRule::MethodCallFragment &mc,
      clang::Expr **args, unsigned num_args, TempMaterializationCtx *ctx);

  virtual std::string AccessLValueObject(clang::MemberExpr *member);

  virtual void ConvertGenericBinaryOperator(clang::BinaryOperator *expr);

  virtual bool IsReferenceType(const clang::Expr *expr) const;

  virtual bool RecordDerivesDefault(const clang::RecordDecl *decl);

  bool RecordDerivesCopy(const clang::RecordDecl *decl) const;

  bool IsPassThroughRule(clang::Expr *expr) const;

  bool RecordHasCopyableFields(const clang::RecordDecl *decl);

  bool ShouldReplaceWithMappedBody(clang::DeclRefExpr *expr) const;

  std::string *rs_code_;
  clang::ASTContext &ctx_;
  clang::FunctionDecl *curr_function_ = nullptr;
  bool in_function_formals_ = false;
  enum class MethodTarget : uint8_t {
    ValueImpl,
    TraitDecl,
    TraitDefault,
    PtrImpl,
  };
  MethodTarget method_target_ = MethodTarget::ValueImpl;

  // True while emitting members INSIDE a `trait { ... }` body (see
  // ConvertAbstractClass). Rust forbids a visibility qualifier on a trait item
  // -- `pub unsafe fn ...` inside a trait is error[E0449] -- so every
  // visibility emission must be suppressed there. `method_target_` does not
  // answer this question in the unsafe model, which never pushes TraitDecl.
  bool in_trait_body_ = false;

  struct PushInTraitBody {
    Converter &c;
    bool prev;
    PushInTraitBody(Converter &c, bool v) : c(c), prev(c.in_trait_body_) {
      c.in_trait_body_ = v;
    }
    ~PushInTraitBody() { c.in_trait_body_ = prev; }
  };

  struct PushMethodTarget {
    Converter &c;
    MethodTarget prev;
    PushMethodTarget(Converter &c, MethodTarget k)
        : c(c), prev(c.method_target_) {
      c.method_target_ = k;
    }
    ~PushMethodTarget() { c.method_target_ = prev; }
  };

  std::string ufcs_receiver_;
  bool in_const_initializer_ = false;
  std::optional<bool> autoref_mut_;
  bool suppress_iterator_clone_ = false;

  struct PushExplicitAutoref {
    Converter &c;
    std::optional<bool> prev;
    PushExplicitAutoref(Converter &c, std::optional<bool> v)
        : c(c), prev(c.autoref_mut_) {
      c.autoref_mut_ = v;
    }
    ~PushExplicitAutoref() { c.autoref_mut_ = prev; }
  };

  struct PushSuppressIteratorClone {
    Converter &c;
    bool prev;
    PushSuppressIteratorClone(Converter &c, clang::CXXConstructExpr *expr)
        : c(c), prev(c.suppress_iterator_clone_) {
      auto *ctor = expr->getConstructor();
      if (!ctor->isCopyOrMoveConstructor() &&
          ctor->isConvertingConstructor(/*AllowExplicit=*/false) &&
          ctor->getNumParams() == 1 && IsIteratorType(expr->getType())) {
        c.suppress_iterator_clone_ = true;
      }
    }
    ~PushSuppressIteratorClone() { c.suppress_iterator_clone_ = prev; }
    PushSuppressIteratorClone(const PushSuppressIteratorClone &) = delete;
    PushSuppressIteratorClone &
    operator=(const PushSuppressIteratorClone &) = delete;

    static bool take(Converter &c) {
      return std::exchange(c.suppress_iterator_clone_, false);
    }

  private:
    static bool IsIteratorType(clang::QualType qt) {
      if (auto *record = qt->getAsCXXRecordDecl()) {
        for (auto *d : record->decls()) {
          if (auto *tnd = llvm::dyn_cast<clang::TypedefNameDecl>(d)) {
            if (tnd->getName() == "iterator_category")
              return true;
          }
        }
      }
      return false;
    }
  };

  struct PushConstInitializer {
    Converter &c;
    bool prev;
    bool enabled;
    PushConstInitializer(Converter &c, bool enabled)
        : c(c), prev(c.in_const_initializer_), enabled(enabled) {
      if (enabled) {
        c.in_const_initializer_ = true;
      }
    }
    ~PushConstInitializer() {
      if (enabled) {
        c.in_const_initializer_ = prev;
      }
    }
  };
  std::vector<clang::Expr *> curr_for_inc_;
  std::vector<clang::QualType> curr_init_type_;

  enum class BreakTarget : int8_t { Loop, FallthroughSwitch, Switch };
  std::vector<BreakTarget> break_target_;

  bool isSwitchBreak() const {
    return !break_target_.empty() &&
           break_target_.back() == BreakTarget::Switch;
  }

  class PushBreakTarget {
  public:
    PushBreakTarget(std::vector<BreakTarget> &stack, BreakTarget target)
        : stack_(stack) {
      stack_.push_back(target);
    }
    ~PushBreakTarget() { stack_.pop_back(); }
    PushBreakTarget(const PushBreakTarget &) = delete;
    PushBreakTarget &operator=(const PushBreakTarget &) = delete;

  private:
    std::vector<BreakTarget> &stack_;
  };

  class PushInitType {
  public:
    PushInitType(Converter &c, clang::QualType type) : c_(c) {
      c_.curr_init_type_.emplace_back(type);
    }
    ~PushInitType() { c_.curr_init_type_.pop_back(); }
    PushInitType(const PushInitType &) = delete;
    PushInitType &operator=(const PushInitType &) = delete;

  private:
    Converter &c_;
  };

  std::unordered_set<const clang::VarDecl *> map_iter_decls_;

  // BindingDecls of a structured binding that we lowered to a RAW POINTER
  // `let` (see EmitMapDecompositionBindings). A C++ binding over a map element
  // is an lvalue reference, and this converter models a reference as a raw
  // pointer dereferenced at every use -- exactly what it already does for
  // reference-typed VarDecls. But a BindingDecl's own type is NOT a reference
  // type in the AST (clang strips it; the reference lives on the hidden holding
  // VarDecl), so the generic reference test in VisitDeclRefExpr cannot see it
  // and we must remember which bindings are pointers ourselves.
  std::unordered_set<const clang::BindingDecl *> ptr_bindings_;

  // Local variables hoisted outside a goto_block so that all labels can see and
  // use the variables.
  std::unordered_set<const clang::VarDecl *> hoisted_decls_;
  class PushHoistedDecls {
  public:
    PushHoistedDecls(std::unordered_set<const clang::VarDecl *> &field)
        : field_(field), saved_(std::move(field)) {
      field_.clear();
    }
    ~PushHoistedDecls() { field_ = std::move(saved_); }
    PushHoistedDecls(const PushHoistedDecls &) = delete;
    PushHoistedDecls &operator=(const PushHoistedDecls &) = delete;

  private:
    std::unordered_set<const clang::VarDecl *> &field_;
    std::unordered_set<const clang::VarDecl *> saved_;
  };

  unsigned materialized_temp_id_ = 0;
  std::string *materialized_temp_bindings_ = nullptr;
  class HoistMaterializedTempBindings {
    Converter &c;
    std::string *prev;
    std::string bindings;
    std::optional<Buffer> buf;
    bool as_block;

  public:
    explicit HoistMaterializedTempBindings(Converter &c, bool as_block = false)
        : c(c), prev(c.materialized_temp_bindings_), buf(c),
          as_block(as_block) {
      c.materialized_temp_bindings_ = &bindings;
    }
    ~HoistMaterializedTempBindings() {
      c.materialized_temp_bindings_ = prev;
      std::string body = std::move(*buf).str();
      buf.reset();

      if (as_block && !bindings.empty()) {
        c.StrCat('{', bindings, body, '}');
        return;
      }
      c.StrCat(bindings, body);
    }
    HoistMaterializedTempBindings(const HoistMaterializedTempBindings &) =
        delete;
    HoistMaterializedTempBindings &
    operator=(const HoistMaterializedTempBindings &) = delete;
  };

  struct ScopedMapIterDecl {
    Converter &c;
    const clang::VarDecl *decl;
    ScopedMapIterDecl(Converter &c, const clang::VarDecl *decl)
        : c(c), decl(decl) {
      c.map_iter_decls_.insert(decl);
    }
    ~ScopedMapIterDecl() { c.map_iter_decls_.erase(decl); }
  };

  struct ScopedPtrBindings {
    Converter &c;
    const clang::DecompositionDecl *decl;
    ScopedPtrBindings(Converter &c, const clang::DecompositionDecl *decl)
        : c(c), decl(decl) {
      for (const auto *b : decl->bindings()) {
        c.ptr_bindings_.insert(b);
      }
    }
    ~ScopedPtrBindings() {
      for (const auto *b : decl->bindings()) {
        c.ptr_bindings_.erase(b);
      }
    }
    ScopedPtrBindings(const ScopedPtrBindings &) = delete;
    ScopedPtrBindings &operator=(const ScopedPtrBindings &) = delete;
  };
  static std::unordered_set<std::string> decl_ids_;
  // Keyed on the EMITTED pair `<RecordName>::<GetMethodName>`, which is exactly
  // the identity Rust uses for E0201 ("duplicate definitions with name ..."),
  // unlike `decl_ids_`/`GetMethodID` whose key embeds `GetLocationID` and so
  // treats a header in-class definition and its out-of-line `.cpp` twin as two
  // distinct entities. Overload-safe: `GetMethodName` routes overloads through
  // `GetOverloadedFunctionName`, which mangles the parameter types
  // (`setOperand_i64_Optioni32`), so two genuinely different overloads get two
  // different keys and BOTH survive. The only pairs this can collapse are ones
  // that would emit the same Rust name in the same impl, i.e. E0201 already.
  static std::unordered_set<std::string> emitted_impl_methods_;
  static std::unordered_set<std::string> abstract_structs_;

  class RecordIndex {
  public:
    void MarkReferenced(std::string name) {
      entries_.try_emplace(std::move(name), false);
    }
    // Returns false if `name` is already defined; otherwise marks it and
    // returns true.
    bool MarkDefined(const std::string &name) {
      bool &defined = entries_[name];
      if (defined) {
        return false;
      }
      defined = true;
      return true;
    }
    template <typename F> void ForEachUndefined(F &&f) const {
      for (const auto &[name, defined] : entries_) {
        if (!defined) {
          f(name);
        }
      }
    }

  private:
    // record name -> true if a definition has been emitted, false if only
    // referenced.
    std::unordered_map<std::string, bool> entries_;
  };
  static RecordIndex record_decls_;
  struct DeferredBlock {
    std::string header;
    std::string body;
  };
  static std::map<std::string, DeferredBlock> virtual_methods_;

  static void EmitDeferredBlock(const DeferredBlock &block, std::string &out);

  // True if the Rust spelling `base_target` of a base class names a TRAIT, so
  // that `impl <base_target> for <derived>` is legal. False for a base whose
  // mapped target is a type (notably `()`), in which case the base's virtual
  // methods must be emitted as inherent methods on the derived type.
  bool BaseTargetNamesTrait(clang::QualType base_type,
                            std::string_view base_target) const;

  DeferredBlock &VirtualMethodsFor(const clang::CXXRecordDecl *decl);

  std::string hoisted_records_;

  enum class ExprKind : uint8_t {
    Callee,
    LValue,
    RValue,
    XValue,
    AddrOf,
    Object,
    Void,
  };

  static const char *expr_kind_to_string(ExprKind kind) {
    switch (kind) {
    case ExprKind::Callee:
      return "Callee";
    case ExprKind::LValue:
      return "LValue";
    case ExprKind::RValue:
      return "RValue";
    case ExprKind::XValue:
      return "XValue";
    case ExprKind::AddrOf:
      return "AddrOf";
    case ExprKind::Object:
      return "Object";
    case ExprKind::Void:
      return "Void";
    default:
      return "Unknown";
    }
  }

  bool isLValue() const;
  bool isRValue() const;
  bool isXValue() const;
  bool isAddrOf() const;
  bool isObject() const;
  bool isVoid() const;
  bool isCallee() const;

  void dump_expr_kinds();

  struct PushCurrFunction {
    Converter &c;
    clang::FunctionDecl *prev;
    PushCurrFunction(Converter &c, clang::FunctionDecl *decl)
        : c(c), prev(c.curr_function_) {
      c.curr_function_ = decl;
    }
    ~PushCurrFunction() { c.curr_function_ = prev; }
  };

  struct PushExprKind {
    Converter &c;
    PushExprKind(Converter &c, ExprKind k, const char *file = __builtin_FILE(),
                 int line = __builtin_LINE())
        : c(c) {
      c.curr_expr_kind_.push_back(k);
      log() << "PushExprKind " << file << ':' << line << ' ';
      c.dump_expr_kinds();
      log() << '[';
      for (const auto k : c.curr_expr_kind_) {
        log() << c.expr_kind_to_string(k) << ", ";
      }
      log() << "]\n";
    }
    ~PushExprKind() { c.curr_expr_kind_.pop_back(); }
  };

  enum class ComputedExprType : uint8_t {
    Value,
    FreshValue,
    Pointer,
    FreshPointer,
    Unknown,
    Pending,
  };
  ComputedExprType computed_expr_type_ = ComputedExprType::Unknown;

  // THE REFERENCE-NESS BIT, and why it is a separate bool rather than a new
  // `ComputedExprType::Reference`.
  //
  // `computed_expr_type_` answers "is this a value or a pointer, and is it
  // fresh". Reference-ness is ORTHOGONAL to all three: `c"x"` is a FreshValue
  // AND already a Rust reference. Folding it into the enum would force an
  // either-or, and -- worse -- the enum is assigned unconditionally at ~70
  // sites, every one of which stores FreshValue/FreshPointer, so a new
  // enumerator would be silently overwritten by whichever Visit ran last and
  // `isFresh()`'s two asserts would have to grow a third case. A separate bool
  // defaults to false (the safe answer: "assume it is a place") and is set by
  // only the handful of emissions that really do produce a `&`.
  //
  // Scope: valid only immediately after one placeholder conversion inside
  // ConvertPlaceholder, which clears it before converting. Nothing else reads
  // it, so no other code path has to maintain it.
  bool emitted_a_reference_ = false;

  bool isFresh() const {
    assert(computed_expr_type_ != ComputedExprType::Unknown);
    assert(computed_expr_type_ != ComputedExprType::Pending);
    return computed_expr_type_ == ComputedExprType::FreshValue ||
           computed_expr_type_ == ComputedExprType::FreshPointer;
  }

  void SetFresh();
  void SetValueFreshness(clang::QualType type);
  void SetFreshType(clang::QualType type);

  std::string ConvertLValue(clang::Expr *expr);
  std::string
  ConvertRValue(clang::Expr *expr,
                std::optional<clang::QualType> implicit_convert_to = {},
                int line = __builtin_LINE());
  virtual std::string
  ConvertFreshRValue(clang::Expr *expr,
                     std::optional<clang::QualType> implicit_convert_to = {});
  virtual std::string ConvertFreshPointer(clang::Expr *expr);
  // target_ptr_type, when known (e.g. a translation rule's parameter type),
  // is the Rust pointer type the result will be used as.
  virtual std::string ConvertFreshObject(clang::Expr *expr,
                                         std::string_view target_ptr_type = {});
  std::string ConvertPointer(clang::Expr *expr, int line = __builtin_LINE());

  /// Materialize a temporary for a prvalue bound to a reference parameter.
  /// Returns (binding_code, ref_expression).
  virtual std::pair<std::string, std::string>
  MaterializeTemp(const std::string &binding_name, clang::QualType param_type,
                  clang::Expr *expr);

  /// Emits binding_code to materialized_temp_bindings_.
  /// Returns ref_expression.
  std::string EmitMaterializedTempBinding(clang::QualType param_type,
                                          clang::Expr *expr);

  virtual const char *GetPointerDerefPrefix(clang::QualType pointee_type);

  TempMaterializationCtx CollectRefBindingTempArgs(clang::CallExpr *expr);

  bool IsCastRedundantInRust(clang::Expr *expr, clang::QualType target_type);

private:
  std::string getIntegerLiteral(clang::IntegerLiteral *expr, bool incl_type,
                                const clang::QualType *type = nullptr);
  const char *keyword_unsafe_;
  const char *keyword_mut_;
  std::vector<ExprKind> curr_expr_kind_;
  static std::unordered_map<std::string, std::string> inner_structs_;
  static std::unordered_set<std::string> globals_;
  static std::vector<std::string> global_inits_;
  clang::Sema *sema_ = nullptr;
};
} // namespace cpp2rust
