#pragma once

// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Sema/Sema.h>

#include <algorithm>
#include <functional>
#include <map>
#include <optional>
#include <set>
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
  // Declares one API-boundary type as an identity-bearing handle. Only reached
  // when --opaque-namespace is in effect.
  static void EmitOpaqueHandle(const std::string &name, std::string &out);
  // Records `name` as an API-boundary type AND as a record still owed a
  // declaration. The two indexes have to agree: the second decides whether
  // anything is emitted, the first decides which shape.
  static void NoteOpaqueRecord(std::string name);
  // Records one API-boundary enum CONSTANT and the integer value the C++
  // enumerator has. Unlike a boundary type, whose value the port still owes, an
  // enumerator's value is known here, so it is carried through rather than
  // approximated.
  static void NoteOpaqueEnumConstant(std::string name, int64_t value);
  // Declares every boundary enum constant recorded so far. Without this they
  // were emitted as bare undefined identifiers.
  static void EmitOpaqueEnumConstants(std::string &out);

  // True when `decl` is googletest's TEST_F/TEST registration static -- the
  // `TestInfo *const X::test_info_` that the macro defines to hand the test to
  // gtest's runtime registry. See the definition for why the predicate cannot
  // match a static that has a real side effect.
  bool IsGTestRegistrationStatic(const clang::VarDecl *decl) const;
  // True for gtest's RUN_ALL_TESTS(), whose job cargo test does. See the
  // definition for the four conditions that keep it from matching user code.
  bool IsGTestRunAllTests(const clang::FunctionDecl *decl) const;
  // Records the test class whose registration static was suppressed, so a
  // #[test] wrapper can be emitted for it and the count is reportable.
  void NoteSuppressedGTestRegistration(const clang::VarDecl *info);
  // Emits one #[test] wrapper per suppressed registration: construct the
  // fixture, SetUp, TestBody, TearDown.
  static void EmitGTestHarness(std::string &out);
  static void EmitGlobalInits(Model model, std::string &out);

  // Prepends a rule module's top-level `use` line when, and ONLY when, the
  // emitted text actually names something that line defines.
  //
  // The IR does not carry rule modules' `use` lines (the preprocessor drops
  // them) and `Converter` never receives `rules_dir` -- factory.cpp:14 hands it
  // straight to Mapper::LoadTranslationRules and keeps no copy -- so there is
  // nothing to derive this from. Measured over all 89 modules, the union of
  // non-local, non-std imports in the whole rule tree is ONE line, so there is
  // no general mechanism to build here and this deliberately does not build one.
  //
  // Emission is CONDITIONAL because `557be22` emitted it unconditionally on the
  // reasoning that "an unused import is at worst a warning". That is false when
  // the crate is not a declared dependency of the consumer: tests/lit links
  // exactly libcc2rs, libc, nix and jiff (Cpp2RustTest.py:256-263), so every
  // emitted file -- including unit/init.cpp, which has no MLIR content at all --
  // died on `E0433: cannot find module or crate dataflowir_gen`, taking down all
  // 1024 lit tests. Conditional emission restores the property that was wrongly
  // assumed: a consumer needs the crate only when the file genuinely uses it.
  static void EmitRuleModuleImports(std::string &out);

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

  /// Resolves a defaulted REFERENCE parameter, which arrives as
  /// `Option<pointer>`, to the pointer the body reads.
  virtual void EmitDefaultedRefParam(clang::ParmVarDecl *param);

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
  void EmitNestedEnums(clang::RecordDecl *decl);
  // Nested classes/structs/unions, which in Rust live outside the enclosing
  // record. Needed on the abstract/trait path too, where there is no struct body
  // to hang them off -- and they carry their own nested enums.
  void EmitNestedRecords(clang::RecordDecl *decl);

  // Fields of `decl` preceded by those of every base lowered to a trait. A Rust
  // trait has no fields, so the derived struct is the only home for a
  // polymorphic base's data. See the definition for the trade-offs accepted.
  static std::vector<clang::FieldDecl *>
  FieldsIncludingTraitBases(const clang::RecordDecl *decl);

  // Per flattened trait-base field, the initializer this constructor gives it,
  // resolved through the base mem-initializer and its arguments.
  std::vector<std::pair<const clang::FieldDecl *, const clang::Expr *>>
  CollectTraitBaseFieldInits(clang::CXXConstructorDecl *decl);

  // Rust name of the generated accessor a trait body uses to read a field that
  // only its implementors have.
  static std::string TraitFieldAccessorName(const clang::FieldDecl *field);

  // Writes `fn __f_x(&self) -> T { self.x }` for every accessor a trait above
  // `decl` requires, into that trait's impl block for `decl`.
  void EmitTraitFieldAccessorImpls(const clang::CXXRecordDecl *decl);

  // True while converting the default method bodies of a trait, where the
  // receiver is the type parameter `Self` and therefore has no fields.
  bool in_trait_body_ = false;
  struct PushTraitBody {
    Converter &c;
    bool prev;
    explicit PushTraitBody(Converter &c) : c(c), prev(c.in_trait_body_) {
      c.in_trait_body_ = true;
    }
    ~PushTraitBody() { c.in_trait_body_ = prev; }
  };
  // Fields the trait currently being emitted reads through `self`.
  std::set<const clang::FieldDecl *> trait_field_reads_;
  // Per abstract class, the field accessors its trait declared as required.
  static std::map<std::string, std::vector<const clang::FieldDecl *>>
      trait_accessors_;

  void EmitReprC(clang::RecordDecl *decl);
  virtual void EmitRustUnion(clang::RecordDecl *decl);

  virtual bool EmitsReprCForRecords() const { return true; }

  virtual const char *CharRustType() const { return "libc::c_char"; }

  virtual bool VisitCXXMethodDecl(clang::CXXMethodDecl *decl);

  virtual bool ShouldConvertMethod(const clang::CXXMethodDecl *decl);

  virtual bool ConvertOutOfLineMethod(clang::CXXMethodDecl *decl);

  bool ConvertCXXMethodDecl(clang::CXXMethodDecl *decl);

  std::string GetMethodName(const clang::CXXMethodDecl *decl);

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
  // The record a UFCS call should name for `method` reached on `receiver`. See
  // the definition in converter.cpp: an inherited non-virtual method is copied
  // onto the derived struct, so the declaring class is the wrong name.
  const clang::CXXRecordDecl *
  GetUFCSOwner(const clang::CXXMethodDecl *method,
               const clang::CXXRecordDecl *receiver) const;

  virtual bool ThisIsRustPtr() const { return false; }

  // The function whose `this` a CXXThisExpr currently refers to.
  //
  // This is NOT always `curr_function_`. A lambda is INLINED at its use site
  // (see ConvertLambdaExpr), but its body is converted with `curr_function_`
  // pushed to the CLOSURE's `operator()` -- needed for the return type. A
  // `this`-capturing lambda's body still means the ENCLOSING function's `this`,
  // so every question of the form "what is `this` spelled as here" must be
  // asked of the enclosing function, not of the closure. Reading
  // `curr_function_` instead made a captured `this` look like a plain `self` in
  // the refcount model and emitted `self.key_` on a `Ptr<Holder>` (E0609 no
  // field `key_`), which blocked every comparator-carrying container.
  clang::FunctionDecl *ThisContextFunction() const;

  virtual void ConvertCXXConstructorBody(clang::CXXConstructorDecl *decl);
  void EmitConstructorFieldInits(clang::CXXConstructorDecl *decl);

  // A field initializer that reads `this` cannot be spelled inside the `Self
  // { .. }` literal that both models build the object with: at that point
  // `this` does not exist yet. Such an initializer is deferred -- the literal
  // gets the field's default and the initializer is re-emitted as an
  // assignment after `this` is bound. Returns the fields deferred, in
  // declaration order.
  std::vector<const clang::FieldDecl *>
  CollectThisDependentFieldInits(clang::CXXConstructorDecl *decl);
  void EmitDeferredFieldInits(clang::CXXConstructorDecl *decl,
                              const std::vector<const clang::FieldDecl *> &f);
  // The initializer of `field` in `decl`, from the definition, or null.
  static const clang::Expr *GetFieldInitExpr(clang::CXXConstructorDecl *decl,
                                             const clang::FieldDecl *field);

  virtual bool VisitCXXConstructorDecl(clang::CXXConstructorDecl *decl);

  virtual bool VisitFieldDecl(clang::FieldDecl *decl);

  virtual bool VisitNamespaceDecl(clang::NamespaceDecl *decl);

  virtual bool VisitTypedefDecl(clang::TypedefDecl *decl);
  virtual bool VisitTypeAliasDecl(clang::TypeAliasDecl *decl);
  virtual bool VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl *decl);

  virtual bool VisitStaticAssertDecl(clang::StaticAssertDecl *decl);

  virtual bool VisitCompoundStmt(clang::CompoundStmt *stmt);

  virtual bool VisitDeclStmt(clang::DeclStmt *stmt);

  virtual bool VisitReturnStmt(clang::ReturnStmt *stmt);

  virtual bool VisitGotoStmt(clang::GotoStmt *stmt);

  void ConvertCondition(clang::Expr *cond);

  virtual bool VisitIfStmt(clang::IfStmt *stmt);

  // The `if`/`else` tail of VisitIfStmt, with no enclosing block. Split out so
  // that an init-statement or a condition variable can be emitted in front of
  // it inside one block without re-entering VisitIfStmt.
  void EmitIfStmtNoScope(clang::IfStmt *stmt);

  virtual bool VisitWhileStmt(clang::WhileStmt *stmt);

  virtual bool VisitDoStmt(clang::DoStmt *stmt);

  virtual bool VisitForStmt(clang::ForStmt *stmt);

  virtual bool VisitCXXForRangeStmt(clang::CXXForRangeStmt *stmt);

  virtual bool VisitCXXForRangeStmtMap(clang::CXXForRangeStmt *stmt);

  // std::set/unordered_set iterate to their ELEMENT, not to a key/value pair,
  // even though they share the map's MapIter runtime type.
  static bool IsSetRangeFor(const clang::CXXForRangeStmt *stmt);
  // Rebinds a set's loop variable from the iterator to the element it denotes.
  void EmitSetElementShadow(clang::VarDecl *loop_var,
                            const std::string &loop_var_name);

  virtual bool VisitCXXForRangeStmtVector(clang::CXXForRangeStmt *stmt);

  virtual bool VisitCXXForRangeStmtString(clang::CXXForRangeStmt *stmt);

  bool VisitCXXForRangeStmtIndexBased(clang::CXXForRangeStmt *stmt,
                                      const char *len_suffix);

  void ConvertForRangeBody(clang::CXXForRangeStmt *stmt,
                           const clang::VarDecl *map_iter_decl = nullptr);

  // --- structured bindings (`auto &[a, b] : m`) ------------------------------
  //
  // For a tuple-like type clang does NOT make the names variables: each
  // BindingDecl carries an implicit *holding* VarDecl whose initializer
  // projects one element out of the holder, and every reference to the name is
  // a DeclRefExpr to that holding var.  Nothing ever declared them, so each use
  // came out as an undefined identifier.  These emit the missing declarations.
  //
  // `holder` names a Rust tuple (both models map std::pair and std::tuple to
  // one), either by value or through a pointer; each binding becomes a pointer
  // to one field of it.
  void EmitTupleBindings(const clang::DecompositionDecl *decomp,
                         const std::string &holder, bool holder_is_pointer);
  // The Rust loop variable of a map range-for is a MapIter, not a pair -- there
  // is no `pair<const K, V>` in the map's Rust representation at all -- so the
  // two names come off the iterator's first()/second() instead.  The two models
  // disagree on what those return, so each spells its own.
  virtual void EmitMapIterBindings(const clang::DecompositionDecl *decomp,
                                   const std::string &holder);
  // True when `decomp` is a tuple-like decomposition, i.e. one whose bindings
  // read through a holding var.  An array or plain-struct decomposition instead
  // gets an expression per binding that needs no declaration.
  static bool IsTupleLikeDecomposition(const clang::DecompositionDecl *decomp);
  // The two bindings of a map decomposition, or {nullptr, nullptr} if `decomp`
  // is not one this converter can project.
  std::array<clang::VarDecl *, 2>
  MapBindingHoldingVars(const clang::DecompositionDecl *decomp);
  // `let <name>: <pointer type> = <init>;` for one binding.
  void EmitBindingLet(clang::VarDecl *holding_var, const std::string &init);
  // One binding of a tuple decomposition, as a pointer to field `index` of the
  // Rust tuple `holder`.  `aliases` is false for `auto [..]`, which decomposes a
  // copy, so a write through the name must not reach the original.
  virtual void EmitOneTupleBinding(clang::VarDecl *holding_var,
                                   const std::string &holder,
                                   bool holder_is_pointer, unsigned index,
                                   bool aliases);

  virtual bool VisitBreakStmt(clang::BreakStmt *stmt);

  virtual bool VisitContinueStmt(clang::ContinueStmt *stmt);

  bool GetFmtArg(clang::Expr *arg, std::string &fmt, std::string &fmt_args,
                 const char *&fmt_trait, std::string &fmt_width);

  bool GetRawArg(clang::Expr *arg, std::string &raw_args);

  void ConvertCallToOstream(clang::CallExpr *expr);
  virtual std::string ConvertStream(clang::Expr *expr);

  // Stream insertion with the format state on the stream. See the long comment
  // above ConvertOstreamItem in converter.cpp for why the state cannot live in
  // the format string or in the Converter.
  bool ConvertOstreamItem(clang::Expr *arg, const std::string &stream_str,
                          clang::CXXOperatorCallExpr *call);
  // How a user-defined operator<< receives the stream, per model.
  // NOT const: the refcount override consumes `pending_deref_`, but ONLY when
  // `may_take_stash` is set. The CHAINED path must not consume -- it has its own
  // consumer downstream, and taking the stash here stole it (measured: it
  // regressed dsc__superdsc.cpp from rc=0 to the assert).
  virtual std::string StreamInserterReceiver(const std::string &stream_str,
                                             bool may_take_stash = false);
  virtual const char *StreamManipFn() const;
  virtual std::string StreamReceiver(const std::string &stream_str) const;
  // How a base manipulator reaches the helper as a value: the two models spell
  // a function pointer differently.
  virtual std::string StreamManipArg(clang::Expr *arg);
  std::string StreamManipName(clang::Expr *arg);

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
    // Emit the hoisted binding with NO type annotation, so inference supplies
    // the type. Set for the stream argument of a user-written inserter, whose
    // parameter is generic over the stream representation.
    bool infer_type = false;
  };

  struct CallInfo {
    std::vector<CallArg> args;
    std::vector<clang::Expr *> variadic_args;
    clang::CallExpr *expr;
    bool is_variadic;
    bool is_fn_ptr_call;
    bool is_libc_passthrough;
  };

  CallInfo CollectCallInfo(clang::CallExpr *expr);

  void ConvertParamTy(clang::QualType param_type, clang::Expr *expr);

  // Emits a pointer-type adjustment (const/mut fixup or reinterpret cast)
  // after `expr` has been converted, for cases where the argument's Rust
  // pointee type differs from the parameter's Rust pointee type even though
  // Clang did not insert an implicit cast node for the call argument (e.g.
  // when two C types are canonically identical, such as `size_t` and
  // `unsigned long`, but map to different Rust types).
  virtual void ConvertParamTyPointerCastIfNeeded(clang::QualType param_type,
                                                 clang::Expr *expr);

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

  std::string GetEscapedCharLiteral(char character) const;
  std::string GetCodeUnitArrayLiteral(const clang::StringLiteral *expr);
  bool IsArrayInitContext() const;

  std::string GetEscapedUTF8CharLiteral(clang::Expr *expr) const;

  std::string GetEscapedStringLiteral(clang::Expr *expr,
                                      uint64_t pad_nulls = 0) const;
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

  virtual void EmitStmtExprTail(clang::Expr *tail);

  virtual bool VisitConditionalOperator(clang::ConditionalOperator *expr);

  virtual bool VisitDeclRefExpr(clang::DeclRefExpr *expr);
  std::string ConvertDeclRefExpr(clang::DeclRefExpr *expr);
  std::optional<std::string> FoldLibraryConstant(clang::DeclRefExpr *expr);

  // The refcount model can stash an lvalue as a pending deref rather than
  // emitting a place expression. `std::mem::take(&mut <nothing>)` is not a
  // recovery, so a consumer that needs the moved-out value asks the model for
  // the take form of whatever is stashed. Returns nullopt when nothing is
  // pending, which is always the case in the unsafe model.
  virtual std::optional<std::string> TakePendingDerefAsMemTake() {
    return std::nullopt;
  }

  // Same problem on the UFCS receiver path: converting the receiver of a
  // method call in an LValue context can stash it instead of emitting it,
  // leaving `base_text` with no receiver in it. Gives the model the chance to
  // rebuild the receiver text from what it stashed. `object_type` is the
  // non-reference receiver type. The unsafe model never stashes and returns
  // `base_text` unchanged.
  virtual std::string FinishUFCSReceiverText(std::string base_text,
                                             clang::QualType object_type) {
    return base_text;
  }

  virtual bool VisitParenExpr(clang::ParenExpr *expr);

  void ConvertMemberExpr(clang::MemberExpr *expr);

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
  // See converter.cpp: names both sides of an omitted-argument/no-default
  // disagreement. True when a survey run recorded it as a gap.
  bool ReportDefaultArgMismatch(const clang::CXXConstructorDecl *ctor,
                                const clang::ParmVarDecl *param,
                                unsigned param_idx,
                                const clang::CXXConstructExpr *expr);

  virtual void ConvertArrayCXXConstructExpr(clang::CXXConstructExpr *expr);

  virtual bool
  VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr *expr);

  virtual bool VisitTypeTraitExpr(clang::TypeTraitExpr *expr);

  virtual bool VisitSizeOfPackExpr(clang::SizeOfPackExpr *expr);

  virtual bool VisitOffsetOfExpr(clang::OffsetOfExpr *expr);

  virtual bool VisitEnumDecl(clang::EnumDecl *decl);

  virtual std::string EnumeratorName(const clang::EnumConstantDecl *decl) const;

  virtual bool VisitCXXDefaultArgExpr(clang::CXXDefaultArgExpr *expr);
  virtual bool VisitConstantExpr(clang::ConstantExpr *expr);

  virtual bool VisitLambdaExpr(clang::LambdaExpr *expr);

  // Which `operator()` body to translate for `expr`. For a non-generic lambda
  // that is simply the call operator; for a generic one it is the
  // specialization the call site being converted selected, falling back to the
  // sole instantiation when there is exactly one. Null means no specialization
  // could be chosen -- the caller must report that rather than guess.
  clang::CXXMethodDecl *SelectLambdaCallOperator(clang::LambdaExpr *expr);

  // The generic-lambda `operator()` specialization named by the call currently
  // being converted, or null. Set for the duration of converting a call's
  // callee so the LambdaExpr that the callee expands to can be emitted at that
  // call's argument types.
  clang::CXXMethodDecl *pending_lambda_call_op_ = nullptr;
  // Lambdas are INLINED at every use: ConvertLambdaVarDecl emits nothing for
  // `auto f = [..]{..}` and VisitDeclRefExpr re-expands the LambdaExpr at each
  // reference. A lambda that refers to ITSELF -- the `std::function<R(A)> f =
  // [&f](A a){ .. f(..) .. }` idiom C++ uses for a recursive local function --
  // therefore inlines into itself without bound, which is unbounded recursion
  // in the converter and, with no stack guard, a bare SIGSEGV. These are the
  // lambda bodies currently being inlined, so the self-reference can be
  // recognised at the inner use and refused instead of expanded.
  std::vector<const clang::CXXRecordDecl *> lambdas_being_inlined_;

  struct PushLambdaBeingInlined {
    Converter &c;
    PushLambdaBeingInlined(Converter &c, const clang::CXXRecordDecl *lambda)
        : c(c) {
      c.lambdas_being_inlined_.push_back(lambda);
    }
    ~PushLambdaBeingInlined() { c.lambdas_being_inlined_.pop_back(); }
    PushLambdaBeingInlined(const PushLambdaBeingInlined &) = delete;
    PushLambdaBeingInlined &operator=(const PushLambdaBeingInlined &) = delete;
  };

  // True when `lambda`'s body is already on the inlining stack, i.e. expanding
  // it here would be expanding it inside itself.
  bool IsInliningLambda(const clang::CXXRecordDecl *lambda) const {
    return std::find(lambdas_being_inlined_.begin(),
                     lambdas_being_inlined_.end(),
                     lambda) != lambdas_being_inlined_.end();
  }


  struct PushPendingLambdaCallOp {
    Converter &c;
    clang::CXXMethodDecl *prev;
    PushPendingLambdaCallOp(Converter &c, clang::CXXMethodDecl *op)
        : c(c), prev(c.pending_lambda_call_op_) {
      c.pending_lambda_call_op_ = op;
    }
    ~PushPendingLambdaCallOp() { c.pending_lambda_call_op_ = prev; }
    PushPendingLambdaCallOp(const PushPendingLambdaCallOp &) = delete;
    PushPendingLambdaCallOp &
    operator=(const PushPendingLambdaCallOp &) = delete;
  };

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
  bool CommaNeedsOwnBlock(const clang::Expr *expr);

#define StrCat(...) _StrCat(__FUNCTION__, __LINE__, __VA_ARGS__)

  inline bool is_empty(char c) { return false; }
  inline bool is_empty(const char *s) { return s == nullptr || *s == '\0'; }
  template <size_t N> inline bool is_empty(const char (&s)[N]) {
    return s[0] == '\0';
  }
  template <typename T> inline bool is_empty(const T &s) { return s.empty(); }

  // StrCat puts a space after every value it appends, because most consecutive
  // values are adjacent tokens that would otherwise run together. When the next
  // thing appended opens a new line that space separates nothing -- the newline
  // already does -- and it is left behind as trailing whitespace. rustfmt
  // rejects such a file outright (`error[internal]: left behind trailing
  // whitespace`), which loses the whole TU, so drop the dead separator AT THE
  // JOIN rather than scrubbing the finished output: the bytes are then never
  // emitted in the first place.
  //
  // Popping the whole run of spaces, not just one, because two consecutive
  // values can each contribute one. Only spaces: the converter emits no tabs,
  // and no emitted string literal can span this join, since every literal is
  // escaped (`\n` as two characters) and so never contains a real newline.
  static void DropDeadSeparator(std::string &out) {
    while (!out.empty() && out.back() == ' ') {
      out.pop_back();
    }
  }
  // Whether the value about to be appended opens a fresh line. Overloaded the
  // same way `is_empty` is, and for the same reason: StrCat is called with
  // `char`, string literals, `std::string`, `std::string_view` and
  // `llvm::SmallString`, and only some of those have `.front()`.
  static bool StartsNewLine(char c) { return c == '\n'; }
  static bool StartsNewLine(const char *s) { return s != nullptr && *s == '\n'; }
  template <size_t N> static bool StartsNewLine(const char (&s)[N]) {
    return s[0] == '\n';
  }
  template <typename T> static bool StartsNewLine(const T &v) {
    return !v.empty() && v.front() == '\n';
  }

  template <typename T> static void AppendCode(std::string &out, const T &v) {
    if (StartsNewLine(v)) {
      DropDeadSeparator(out);
    }
    out += v;
  }

  template <typename... Ts>
  inline void _StrCat(const char *func, int line, const Ts &...vals) {
    log() << '[' << func << ':' << line << "] ";
    ((log() << vals << '\n', AppendCode(*rs_code_, vals),
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

  // The mangler PROPER: base name plus the Rust spelling of every parameter.
  // Two C++ overloads whose parameters have the SAME Rust spelling come out of
  // this identical -- that is a collision, and it is not hypothetical:
  // progir.h:154 and :157 declare `setOperand(const long, SdscFoldIdInput)` and
  // `setOperand(const long long, SdscFoldIdInput)`, distinct C++ types that are
  // both `i64`, and they were emitted as one name. That single collision wore
  // four different error codes -- E0592 in the unsafe model, E0201+E0428+E0046
  // in the refcount one -- across most sampled TUs.
  std::string GetOverloadedFunctionNameRaw(const clang::FunctionDecl *decl);

  // Whether some OTHER member of `decl`'s overload set mangles to `raw`.
  bool OverloadNameCollides(const clang::FunctionDecl *decl,
                            std::string_view raw);

  // What to append when it does. Overridden by the refcount model, which has a
  // cheaper answer for the axis it erases (pointee constness) and falls back to
  // this one otherwise. Empty means "cannot tell them apart", which is left
  // LOUD as a collision rather than papered over.
  virtual std::string OverloadCollisionSuffix(const clang::FunctionDecl *decl);

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

  virtual void ConvertFunctionParameters(clang::FunctionDecl *decl);
  // Whether `decl` is a user-written `std::ostream &operator<<(std::ostream &,
  // T)`, which must be emitted generic over the stream representation. See the
  // definition in converter.cpp.
  static bool IsUserStreamInserter(const clang::FunctionDecl *decl);

  virtual void ConvertFunctionQualifiers(clang::FunctionDecl *decl);

  virtual void ConvertFunctionReturnType(clang::FunctionDecl *decl);

  virtual void ConvertFunctionMain(const clang::FunctionDecl *decl,
                                   const std::string_view main_function_name);

  virtual void ConvertAbstractClass(clang::CXXRecordDecl *decl);

  // True if a `signature { .. }` item was actually emitted, i.e. at least one
  // member passed `predicate`. False means no item by that name exists in the
  // output, so nothing may name it.
  bool ConvertCXXMethodDecls(const clang::CXXRecordDecl *decl,
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

  // True when the type's Rust spelling names a trait, not a struct.
  static bool IsTraitTyped(clang::QualType qual_type);

  // `impl From<(A, B, ..)> for T` per converting constructor of arity >= 2.
  // rules/{unique_ptr,shared_ptr} spell make_unique/make_shared's
  // multi-argument forms as `<T1>::from((a0, a1))`; this supplies the impl.
  virtual void AddFromTraits(const clang::CXXRecordDecl *decl);

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

  virtual void ConvertLoopVariable(clang::VarDecl *decl,
                                   clang::Expr *range_init);

  virtual void ConvertUniquePtrDeref(clang::CXXOperatorCallExpr *expr);

  virtual bool ConvertCXXOperatorCallExpr(clang::CXXOperatorCallExpr *expr);

  // Transliterates an overloaded operator whose callee is on an opaque API
  // boundary. Returns false -- writing nothing -- when it is not one, or with
  // --opaque-namespace absent.
  bool ConvertOpaqueOperatorCall(clang::CXXOperatorCallExpr *expr);

  // An operator with no dedicated handler: opaque transliteration if it is on a
  // boundary type, otherwise a reported gap. Never writes nothing.
  bool ConvertUnhandledOperatorCall(clang::CXXOperatorCallExpr *expr);

  std::string GetMappedAsString(clang::Expr *expr, clang::Expr **args = nullptr,
                                unsigned num_args = 0,
                                TempMaterializationCtx *ctx = nullptr);

  std::string
  ConvertIRFragment(const std::vector<TranslationRule::BodyFragment> &fragments,
                    clang::Expr *expr, clang::Expr **args, unsigned num_args,
                    TempMaterializationCtx *ctx);

  std::string ConvertPlaceholder(clang::Expr *expr, clang::Expr *arg,
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

  // Whether any field has a default member initializer (`int k = 7;`), which
  // `#[derive(Default)]` cannot express -- it would zero the field instead.
  static bool RecordHasFieldInitializer(const clang::RecordDecl *decl);

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

  struct PushMethodTarget {
    Converter &c;
    MethodTarget prev;
    PushMethodTarget(Converter &c, MethodTarget k)
        : c(c), prev(c.method_target_) {
      c.method_target_ = k;
    }
    ~PushMethodTarget() { c.method_target_ = prev; }
  };

  // The record whose `impl <Record> {` item is CURRENTLY OPEN, or null at item
  // level. Rust has no nested items inside an `impl`, so a method emitter that
  // opens its own `impl` (ConvertOutOfLineMethod) must not run while one is
  // already open -- that emits `impl X { .. impl X { .. } .. }` and the whole
  // file stops parsing with "implementation is not supported in `trait`s or
  // `impl`s". Reachable because ConvertCXXMethodDecls feeds
  // ForEachTemplateInstantiatedMethod's results into VisitCXXMethodDecl from
  // INSIDE the impl it just opened, and an explicit specialization of a member
  // function template (`template <> void N::walk<0>(..) {..}` in the .cpp) is
  // out-of-line while `isTemplateInstantiation()` is false, so it took the
  // out-of-line branch.
  const clang::CXXRecordDecl *open_impl_for_ = nullptr;

  struct PushOpenImpl {
    Converter &c;
    const clang::CXXRecordDecl *prev;
    PushOpenImpl(Converter &c, const clang::CXXRecordDecl *decl)
        : c(c), prev(c.open_impl_for_) {
      c.open_impl_for_ = decl;
    }
    ~PushOpenImpl() { c.open_impl_for_ = prev; }
  };

  // Whether an `impl` for this method's own record is already open, i.e. whether
  // emitting one here would nest.
  bool IsOwnImplOpen(const clang::CXXMethodDecl *decl) const {
    return open_impl_for_ != nullptr && decl->getParent() != nullptr &&
           open_impl_for_->getCanonicalDecl() ==
               decl->getParent()->getCanonicalDecl();
  }

  std::string ufcs_receiver_;
  // The receiver's static record type, set beside ufcs_receiver_ and read by
  // GetUFCSName so an inherited method copied onto a derived struct is called by
  // that struct's name rather than its declaring class's.
  const clang::CXXRecordDecl *ufcs_receiver_record_ = nullptr;
  // Sets the above from a receiver expression, looking through the implicit
  // derived-to-base conversion. Every model's receiver path must call it; see
  // converter.cpp.
  void SetUFCSReceiverRecord(clang::Expr *base, bool is_arrow);
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
  static std::unordered_set<std::string> decl_ids_;
  static std::unordered_set<std::string> abstract_structs_;
  // Abstract classes for which ConvertAbstractClass actually emitted a `trait`
  // item. A strict subset of `abstract_structs_`: that set records "we took the
  // trait PATH for this class", which is what the `dyn` and pointer-shape
  // decisions want, while this one records "a trait by this NAME exists in the
  // output", which is the only sound basis for writing `impl <name> for T` or a
  // `: <name>` supertrait bound. They diverge because the trait's member
  // predicate can reject every member, leaving no item at all.
  static std::unordered_set<std::string> trait_records_;
  // Per emitted trait, the method NAMES its item actually declares. An override
  // may only be routed into `impl <trait> for T` if the trait declares that
  // name, or rustc says "method X is not a member of trait Y" (E0407). Knowing
  // the trait exists is not enough: ConvertAbstractClass's predicate drops
  // members, so a trait routinely exists while lacking the very method being
  // routed -- e.g. a gtest fixture's trait carries SetUp/TearDown but never
  // TestBody, which is declared only on the opaque `::testing::Test`.
  static std::unordered_map<std::string, std::unordered_set<std::string>>
      trait_method_names_;

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
  // Boundary enum constant name -> the C++ enumerator's value. Ordered so the
  // emitted declarations are in a stable order run to run.
  static std::map<std::string, int64_t> opaque_enum_constants_;
  // One entry per suppressed TEST_F/TEST registration static, in source order so
  // the emitted #[test] functions appear in the order the file declares them.
  struct GTestCase {
    std::string test_struct; // the Rust name of the per-test class
    std::string suite;       // the fixture/suite name, for the #[test] name
    std::string test;        // the test name
    bool has_set_up = false;
    bool has_tear_down = false;
  };
  static std::vector<GTestCase> gtest_cases_;
  struct DeferredBlock {
    std::string header;
    std::string body;
  };
  static std::map<std::string, DeferredBlock> virtual_methods_;

  // The `trait <X> { … }` blocks ConvertAbstractClass lowers an abstract class
  // to, keyed on trait name, held rather than emitted.
  //
  // Held because a non-virtual method of a trait-lowered class whose body is
  // defined OUT OF LINE is visited long after the class, by which time an
  // inline-emitted trait's braces are closed -- so ConvertOutOfLineMethod
  // currently emits `impl <trait-name> { … }`, an inherent impl on a name that
  // is a trait, which is E0782 at every call site (42 of them once the defining
  // TU is compiled alongside). `virtual_methods_` cannot serve: it accumulates
  // `impl Trait for X` blocks keyed per implementor, and a non-virtual method
  // has no single implementor. The refcount model already defers whole traits
  // (`MethodsOnPtrFor(...).trait.body`); this is the base model's equivalent.
  //
  // THIS CHANGE IS DESTINATION-ONLY and must stay byte-identical: the block is
  // still assembled at exactly the same point and still appears at exactly the
  // same position in the output, via a placeholder substituted at finalization.
  // Routing out-of-line methods into it is a separate, measured step.
  static std::map<std::string, DeferredBlock> trait_blocks_;

  // The inert marker left where a deferred trait block belongs. A `//` comment
  // terminated by a newline, so if substitution ever failed to fire the output
  // would lose the trait and fail LOUDLY on an undefined name, rather than
  // commenting out whatever followed it on the line.
  static std::string TraitBlockPlaceholder(const std::string &trait_name);

  // Substitutes every held trait block back into `out` at its placeholder.
  static void EmitTraitBlocks(std::string &out);
  // Turns unbounded converter recursion from a bare SIGSEGV into a diagnostic
  // naming a source location, via clang/Basic/Stack.h. A safety net, not a fix
  // for any particular cycle -- see the comment on the definition.
  void CheckStackSpace(const clang::Expr *expr);

  // Recursion tracing, off unless CPP2RUST_TRACE_DEPTH=<n> is set -- the same
  // convention as the CPP2RUST_DEBUG_* hooks.
  //
  // Unbounded recursion in the converter is otherwise a bare SIGSEGV with no
  // output at all: the stack grows monotonically past 64 MB and nothing says
  // where. With this set, once the depth of nested Convert()/VisitLambdaExpr()
  // frames passes <n> the innermost 40 frames are printed -- each naming the
  // converter function, the AST node class and the source location -- which
  // turns the crash into a printout that names the cycle. That is how the
  // self-referential-lambda cycle in VisitLambdaExpr was found, and it is kept
  // because the next such cycle would otherwise be just as invisible.
  //
  // The hot path is one load and a branch: trace_enabled_ is resolved once and
  // every frame after that returns immediately when tracing is off.
  struct TraceFrame {
    const char *site;
    const char *kind;
    std::string loc;
  };
  static std::vector<TraceFrame> trace_stack_;
  static long trace_limit_;
  // -1 until the environment has been read; then 0 (off) or 1 (on).
  static signed char trace_enabled_;
  static void TraceInit();
  static void TraceDump();
  struct PushTrace {
    bool active;
    PushTrace(Converter &c, const char *site, const clang::Expr *node) {
      if (trace_enabled_ < 0) {
        TraceInit();
      }
      active = trace_enabled_ > 0;
      if (active) {
        Push(c, site, node);
      }
    }
    ~PushTrace() {
      if (active) {
        trace_stack_.pop_back();
      }
    }
    PushTrace(const PushTrace &) = delete;
    PushTrace &operator=(const PushTrace &) = delete;

  private:
    static void Push(Converter &c, const char *site, const clang::Expr *node);
  };


  static void EmitDeferredBlock(const DeferredBlock &block, std::string &out);

  // True if this class should be lowered to a Rust TRAIT rather than a struct.
  // An abstract class only earns a trait when some pure virtual is visible to
  // the Rust side; abstract purely via a non-emitted (opaque/system) base buys
  // no dispatch and costs every field write in every inherited body.
  static bool IsTraitLowerable(const clang::CXXRecordDecl *decl);

  // Re-emits, as inherent methods of `decl`, the methods it inherits from a base
  // that was lowered to a STRUCT. Rust has no inheritance and such a base
  // delivers nothing through a trait impl, so without this every inherited call
  // is E0599. Sound because the base's fields were flattened in alongside.
  void EmitInheritedStructMethods(clang::CXXRecordDecl *decl);
  // Re-emit an inherited method in the shape a model that splits methods between
  // inherent impls and Ptr traits needs. Returns true if it handled the method,
  // in which case the caller must not also copy the body. See converter.cpp.
  virtual bool EmitInheritedMethodOnPtr(clang::CXXRecordDecl *decl,
                                        clang::CXXMethodDecl *method);

  // Nearest transitive base that ConvertAbstractClass lowered to a trait, or
  // nullptr when every base up the chain is a concrete struct.
  static const clang::CXXRecordDecl *
  GetTraitBase(const clang::CXXRecordDecl *decl);

  // Trait whose impl block this override belongs in: the highest abstract
  // ancestor declaring it. nullptr when no base became a trait at all, and also
  // when the nearest trait exists but does not declare THIS method -- routing it
  // there would be E0407. Not static: it needs GetMethodName, which is
  // model-dependent, to compare against what the trait declared.
  const clang::CXXRecordDecl *
  GetDeclaringTrait(const clang::CXXRecordDecl *impl_for,
                    const clang::CXXMethodDecl *method);

  // nullptr when `trait` is nullptr, i.e. no `impl Trait for T` to write.
  DeferredBlock *VirtualMethodsFor(const clang::CXXRecordDecl *decl,
                                   const clang::CXXRecordDecl *trait);

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
