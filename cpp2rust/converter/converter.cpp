// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include "converter/converter.h"

#include "tu_guard.h"

#include <clang/AST/APValue.h>
#include <clang/AST/ParentMapContext.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/Version.h>
#include <clang/Sema/Template.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/Support/ConvertUTF.h>
#include <llvm/Support/ErrorHandling.h>

#include <algorithm>
#include <format>
#include <ranges>
#include <utility>

#include "compiler.h"
#include "converter/converter_lib.h"
#include "converter/lex.h"
#include "converter/mapper.h"
#include "converter/survey.h"

namespace cpp2rust {
std::unordered_map<std::string, std::string> Converter::inner_structs_;
std::unordered_set<std::string> Converter::decl_ids_;
std::unordered_set<std::string> Converter::globals_;
std::vector<std::string> Converter::global_inits_;
std::unordered_set<std::string> Converter::abstract_structs_;
Converter::RecordIndex Converter::record_decls_;
std::map<std::string, Converter::DeferredBlock> Converter::virtual_methods_;

void Converter::ConvertUniquePtrDeref(clang::CXXOperatorCallExpr *expr) {
  bool is_star = expr->getOperator() == clang::OverloadedOperatorKind::OO_Star;
  PushParen paren(*this, is_star);
  if (is_star) {
    StrCat(token::kStar);
  }
  if (expr->getArg(0)->IgnoreImplicit()->getType().isConstQualified()) {
    StrCat("(*(std::ptr::addr_of!(");
    Convert(expr->getArg(0));
    StrCat(").cast_mut())).as_deref_mut().unwrap()");
  } else {
    Convert(expr->getArg(0));
    StrCat(".as_deref_mut().unwrap()");
  }
}

void Converter::EmitFilePreamble() {
  StrCat(R"(
extern crate libc;
use libc::*;
extern crate libcc2rs;
use libcc2rs::*;
use std::collections::{BTreeMap, HashMap, HashSet};
use std::io::{Read, Write, Seek};
use std::os::fd::{AsFd, FromRawFd, IntoRawFd};
use std::rc::Rc;
)");
}

void Converter::EmitDeferredBlock(const DeferredBlock &block,
                                  std::string &out) {
  out += block.header;
  out += " {\n";
  out += block.body;
  out += "}\n";
}

void Converter::EmitVirtualMethods(std::string &out) {
  for (const auto &[name, impl] : virtual_methods_) {
    EmitDeferredBlock(impl, out);
  }
}

std::string Converter::ForceGlobalInit(const clang::VarDecl *decl) {
  return std::format("std::cell::LazyCell::force(&*&raw const {});",
                     GetNamedDeclAsString(decl));
}

void Converter::EmitGlobalInits(Model model, std::string &out) {
  out += model == Model::kUnsafe ? "pub unsafe fn __cpp2rust_init_globals() {\n"
                                 : "pub fn __cpp2rust_init_globals() {\n";
  for (const auto &line : global_inits_) {
    out += line;
    out += '\n';
  }
  out += "}\n";
}

void Converter::EmitOpaqueRecords(std::string &out) {
  record_decls_.ForEachUndefined([&](const std::string &name) {
    out += "#[derive(Clone, Copy, Default, ByteRepr)]";
    out += "pub struct ";
    out += name;
    out += ";\n";
  });
}

bool Converter::VisitRecoveryExpr(clang::RecoveryExpr *expr) {
  llvm::errs() << "RecoveryExpr: ";
  expr->dump();
  // Contained on the --dir path (see tu_guard.h); plain exit(1) on --file, so
  // the single-TU behaviour is unchanged.
  cpp2rust::tu_guard::BailOut("converter.cpp VisitRecoveryExpr");
  return false;
}

bool Converter::Convert(clang::QualType qual_type) {
  // Catch va_list before desugaring
  if (IsVaListType(qual_type)) {
    StrCat("VaList");
    return false;
  }

  if (auto decl = qual_type->getAsRecordDecl();
      decl && IsUserDefinedDecl(decl)) {
    record_decls_.MarkReferenced(GetRecordName(decl));
  }

  auto mapped = Mapper::Map(qual_type);
  if (!mapped.empty() && mapped != token::kIgnoreRule) {
    StrCat(mapped);
    return false;
  }

  qual_type = qual_type.getUnqualifiedType().getDesugaredType(ctx_);
  // MEASURED: a type that reaches TraverseType and matches no Visit*Type emits
  // ZERO TOKENS, and every caller splices that nothing into a position that
  // syntactically requires a type. A 39-TU random-sample census found this in 6
  // of the 10 translate-to-completion TUs it parse-checked, in six shapes that
  // are all THIS one site (`Convert(QualType)` is the only path type text comes
  // from -- `ToString(QualType)` just buffers it):
  //     pub struct S { pub f : , }            field
  //     let mut loop_ : = ...                  `auto` local
  //     let mut m : = <>::default() ;          default-init of the same local
  //     fn f ( p : *const , )                  pointee (variadic pack parm)
  //     fn f ( ... ) -> { ...                  return type
  //     ( ( loop_ as ) ) . getBody ( )         cast target
  // rustfmt cannot parse any of them, so the WHOLE file is unparseable and the
  // one metric that tracks convergence -- rustc errors on the emission -- cannot
  // be taken at all. Emitting a named, undefined placeholder turns each site
  // into a local `E0412 cannot find type`, i.e. a PARSEABLE file with a
  // diagnosable gap. Deliberately NOT the bare mangled name that
  // `--mangle-unmapped` uses (survey.h:100): that spelling is exactly what a
  // PORTED type would be called, so it can silently resolve to an unrelated
  // `pub struct` emitted in the same TU and compile. The `Cpp2RustUnmapped_`
  // prefix is emitted by nothing else, so it can only ever fail, and it carries
  // the C++ spelling so the missing model can be named from the rustc error
  // alone.
  const size_t before = rs_code_->size();
  bool res = TraverseType(qual_type);
  if (rs_code_->size() == before) {
    const std::string cpp = Mapper::ToString(qual_type);
    // BEFORE the placeholder: a PROJECT tag decl that this very TU also PORTS
    // and EMITS. This is a CONVERSION-ORDER miss, not a missing model: the
    // biggest placeholder names measured over the corpus
    // (`Isa::InstOperand` 40 A-TUs, `SenTargets` 35, `DataConvertOpFuncs` 35,
    // `DataFormats` 31, `OperandAttr::Type` 26 -- all first-party enums) reach
    // here only because `VisitEnumDecl` (which emits `pub type <name> = <int>;`
    // plus a `pub const <name>_<enumerator>` each) has not run yet, so
    // `Mapper::Map` misses; and there is NO `VisitEnumType` in this converter,
    // so `TraverseType` emits zero tokens. Measured: 165 of the 167 (TU, name)
    // pairs carrying one of those placeholders ALSO carry the matching
    // `pub type` IN THE SAME FILE (Rust items are order-independent, so the
    // name resolves) -- e.g. `util__sendefs__numeric_convert.cpp.rs` has
    // `Cpp2RustUnmapped_DataFormats` at line 737 and `pub type DataFormats =
    // i32;` 198 lines below it.
    //
    // This is byte-for-byte the branch at mapper.cpp:1604-1631, which is why
    // the `Sentient*` I32EnumAttrs in the same logs already get a PORTED name
    // and no placeholder. Two things it deliberately does NOT do:
    //   * It does NOT register anything in `types_`. `VisitEnumDecl` bails on
    //     `Mapper::Contains(getCanonicalTagType(decl))` (converter.cpp:5222,
    //     same trap at mapper.cpp:1597); a key would SUPPRESS the `pub type`
    //     and every `pub const <name>_*`, leaving each enumerator reference
    //     undefined -- fewer placeholder tokens, strictly WORSE Rust.
    //   * It does NOT guess a width. The measured emissions are
    //     `Isa_InstOperand = u8` (`enum class InstOperand : uint8_t`,
    //     sys-arch-spec/isa/isa.hpp:48) while the other four are `i32`, so any
    //     uniform width would silently corrupt the #1 name. Emitting only the
    //     NAME sidesteps the question.
    // The name comes from the DECL, never from `cpp`: see mapper.cpp:1606-1616
    // for why (a template specialisation's leaf spelling mangles differently
    // from what VisitRecordDecl actually defines).
    //
    // A SYSTEM type still falls through to the placeholder below, and
    // ReportUnmappedSystemType's loud abort is untouched.
    // NARROWED TO ENUMS, and that is load-bearing. Measured: the unrestricted
    // form also rewrote `Cpp2RustUnmapped_mlir_AsmParser` to `mlir_AsmParser`
    // in KtdpAttrs.cpp -- an MLIR CLASS reached through a plain `-I` (so
    // `IsUserDefinedDecl` is true) that this TU does NOT define, so the rename
    // only threw away the `Cpp2RustUnmapped_` prefix's one guarantee: that the
    // spelling is emitted by nothing else and can therefore ONLY ever fail,
    // never silently resolve to an unrelated `pub struct` of the same name
    // (converter.cpp:145). An enum is the case where the definition provably
    // IS emitted alongside, by `VisitEnumDecl`, so restricting to `isEnum()`
    // keeps every one of the 167 measured enum sites and gives up nothing.
    if (const clang::TagDecl *tag = nullptr;
        Mapper::LooksLikeUserDefinedTypeName(cpp, &tag) && tag->isEnum()) {
      const std::string ported =
          Mapper::ToRustName(Mapper::ToString(Mapper::GetTypeForDecl(tag)));
      if (!ported.empty()) {
        StrCat(ported);
        static std::set<std::string> ported_leaves;
        if (ported_leaves.insert(cpp).second) {
          llvm::errs() << "note: no Rust type text for `" << cpp << "` ("
                       << qual_type->getTypeClassName()
                       << "), but it is a project leaf type this TU ports; "
                          "emitting its PORTED name `"
                       << ported << "` (declared at "
                       << tag->getLocation().printToString(
                              ctx_.getSourceManager())
                       << ")\n";
        }
        return res;
      }
    }
    // A dependent/deduced type can print as nothing at all; fall back to the
    // AST type class so the placeholder still says WHAT was dropped.
    std::string tail = Mapper::ToRustName(cpp);
    if (tail.empty()) {
      tail = qual_type->getTypeClassName();
    }
    StrCat("Cpp2RustUnmapped_" + tail);
    static std::set<std::string> reported;
    if (reported.insert(tail).second) {
      llvm::errs() << "note: no Rust type text for `" << cpp << "` ("
                   << qual_type->getTypeClassName()
                   << "); emitting the undefined placeholder "
                      "`Cpp2RustUnmapped_"
                   << tail << "` so the file parses\n";
    }
  }
  return res;
}

bool Converter::ConvertMappedType(clang::QualType qual_type) {
  std::string type_as_string = Mapper::Map(qual_type);
  if (type_as_string == token::kIgnoreRule) {
    return false;
  }
  StrCat(type_as_string);
  return true;
}

std::string Converter::ConvertPointeeType(clang::QualType ptr_type) {
  assert(!ptr_type.isNull() && ptr_type->isPointerType());
  auto pointee = ptr_type->getPointeeType();
  if (!pointee->isRecordType()) {
    return std::string(Trim(ToString(pointee)));
  }

  auto str = ToString(ptr_type);
  Unwrap(str, "*mut ", "");
  Unwrap(str, "*const ", "");
  return std::string(Trim(str));
}

bool Converter::VisitBuiltinType(clang::BuiltinType *type) {
  switch (type->getKind()) {
  case clang::BuiltinType::Bool:
    StrCat("bool");
    break;
  case clang::BuiltinType::Float:
    StrCat("f32");
    break;
  case clang::BuiltinType::Double:
  case clang::BuiltinType::LongDouble:
    StrCat("f64");
    break;
  case clang::BuiltinType::Char_S:
  case clang::BuiltinType::Char_U:
    StrCat(CharRustType());
    break;
  case clang::BuiltinType::SChar:
    StrCat("i8");
    break;
  case clang::BuiltinType::UChar:
    StrCat("u8");
    break;
  case clang::BuiltinType::UShort:
  case clang::BuiltinType::UInt:
  case clang::BuiltinType::ULong:
  case clang::BuiltinType::ULongLong:
  case clang::BuiltinType::Short:
  case clang::BuiltinType::Int:
  case clang::BuiltinType::Long:
  case clang::BuiltinType::LongLong:
  case clang::BuiltinType::WChar_S:
  case clang::BuiltinType::WChar_U:
  case clang::BuiltinType::Char8:
  case clang::BuiltinType::Char16:
  case clang::BuiltinType::Char32:
    StrCat(std::format("{}{}", type->isSignedInteger() ? 'i' : 'u',
                       ctx_.getTypeSize(type)));
    break;
  case clang::BuiltinType::Void:
    StrCat("::libc::c_void");
    break;
  case clang::BuiltinType::UInt128:
    StrCat("u128");
    break;
  case clang::BuiltinType::Int128:
    StrCat("i128");
    break;
  case clang::BuiltinType::NullPtr:
    Convert(ctx_.VoidPtrTy);
    break;
  default:
    if (survey::Enabled()) {
      survey::Record(survey::GapKind::kUnmappedType,
                     std::string("builtin: ") +
                         type->getName(ctx_.getPrintingPolicy()).str(),
                     {});
      break;
    }
    llvm::errs() << "unsupported builtin type: "
                 << type->getName(ctx_.getPrintingPolicy()) << '\n';
    assert(0 && "unsupported builtin type\n");
    break;
  }
  return false;
}

bool Converter::VisitRecordType(clang::RecordType *type) {
  auto *decl = type->getDecl();
  if (auto lambda = clang::dyn_cast<clang::CXXRecordDecl>(decl)) {
    if (lambda->isLambda()) {
      if (in_function_formals_) {
        StrCat(
            ConvertFunctionPointerType(lambda->getLambdaCallOperator()
                                           ->getType()
                                           ->getAs<clang::FunctionProtoType>(),
                                       FnProtoType::LambdaCallOperator));
      } else {
        StrCat('_');
      }
      return false;
    }
  }

  // A SYSTEM record with no type rule must NOT be mangled into a name.
  //
  // Reaching here means Mapper::Map() found no rule (Convert(QualType) only
  // falls through to TraverseType when the lookup came back empty). For a
  // PROJECT type that is correct and expected: VisitRecordDecl emits a
  // `pub struct <mangled>` for it in this same TU, so the mangled name resolves.
  // For a SYSTEM type nothing is ever emitted, so `GetRecordName` invents an
  // identifier (`mlir::DictionaryAttr` -> `mlir_DictionaryAttr`) that is
  // referenced and never defined -- and `AddRuleForUserDefinedType` then
  // REGISTERS that invention as a type rule, so every later lookup "succeeds"
  // and the mapper's own loud `Type is not present in types_` path
  // (mapper.cpp:722) is never reached. The TU reports rc=0 and then fails
  // rustc with `cannot find type`. Measured on
  // dataflow-scheduler/lib/Dialect/KTDF/Utils/Utils.cpp: rc=0, 0 placeholders,
  // 1589 rustc errors, ALL of them this class.
  //
  // This is the same defect the `unimplemented!()`/`todo!()` ban was written to
  // stop -- a missing model deferred past translate time -- by a different
  // mechanism, so it gets the same treatment: loud here, recorded in --survey.
  if (!IsUserDefinedDecl(decl)) {
    ReportUnmappedSystemType(decl);
    return false;
  }

  StrCat(GetRecordName(decl));
  Mapper::AddRuleForUserDefinedType(decl);
  return false;
}

std::string Converter::ConvertPointer(clang::Expr *expr, int line) {
  log() << "ConvertPointer called from line " << line << '\n';
  PushExprKind push(*this, ExprKind::AddrOf);
  return ToString(expr);
}

std::string Converter::ConvertFreshPointer(clang::Expr *expr) {
  auto str = ConvertPointer(expr);
  if (isFresh()) {
    return str;
  }
  SetFresh();
  return str;
}

std::string Converter::ConvertFreshObject(clang::Expr *expr, std::string_view) {
  return ConvertFreshPointer(expr);
}

std::string Converter::ConvertLValue(clang::Expr *expr) {
  PushExprKind push(*this, ExprKind::LValue);
  return ToString(expr);
}

std::string
Converter::ConvertRValue(clang::Expr *expr,
                         std::optional<clang::QualType> implicit_convert_to,
                         int line) {
  log() << "ConvertRValue called from line " << line << '\n';
  PushExprKind push(*this, ExprKind::RValue);
  return ToString(expr, implicit_convert_to);
}

std::string Converter::ConvertFreshRValue(
    clang::Expr *expr, std::optional<clang::QualType> implicit_convert_to) {
  auto str = ConvertRValue(expr, implicit_convert_to);
  if (!isFresh() && !expr->getType()->isVoidType() &&
      !expr->getType()->isPointerType()) {
    SetFresh();
    return std::format("({}).clone()", std::move(str));
  }
  SetFresh();
  return str;
}

std::pair<std::string, std::string>
Converter::MaterializeTemp(const std::string &binding_name,
                           clang::QualType param_type, clang::Expr *expr) {
  auto pointee = param_type.getNonReferenceType();
  auto value = ConvertRValue(expr, pointee);
  auto type_str = ToStringBase(pointee);
  const auto *decl = in_const_initializer_ ? keyword::kStatic : keyword::kLet;

  auto binding =
      std::format("{} mut {} : {} = {};", decl, binding_name, type_str, value);
  auto ref = in_const_initializer_
                 ? std::format("& mut *& raw mut {}", binding_name)
                 : std::format("& mut {}", binding_name);
  return {binding, ref};
}

std::string Converter::EmitMaterializedTempBinding(clang::QualType param_type,
                                                   clang::Expr *expr) {
  assert(materialized_temp_bindings_ && "materialized temp emitted outside a "
                                        "HoistMaterializedTempBindings scope");
  auto [binding, ref] = MaterializeTemp(
      std::format("__tmp_{}", materialized_temp_id_++), param_type, expr);
  *materialized_temp_bindings_ += std::move(binding);
  return ref;
}

bool Converter::VisitConstantArrayType(clang::ConstantArrayType *type) {
  StrCat('[');
  Convert(type->getElementType());
  auto size = GetNumAsString(type->getSize());
  StrCat(std::format("; {}]", size.c_str()));
  return false;
}

bool Converter::VisitIncompleteArrayType(clang::IncompleteArrayType *type) {
  StrCat('[');
  Convert(type->getElementType());
  StrCat(']');
  return false;
}

bool Converter::VisitReferenceType(clang::ReferenceType *type) {
  auto pointee_type = type->getPointeeType();
  StrCat(pointee_type.isConstQualified() ? "*const" : "*mut");
  return Convert(pointee_type);
}

std::string
Converter::ConvertFunctionPointerType(const clang::FunctionProtoType *proto,
                                      FnProtoType kind) {
  std::string result =
      (kind == FnProtoType::LambdaCallOperator ? "impl Fn(" : "fn(");
  for (auto p_ty : proto->param_types()) {
    result += ToString(p_ty);
    result += ',';
  }
  result += ')';
  if (!proto->getReturnType()->isVoidType()) {
    result += std::format(" -> {}", ToString(proto->getReturnType()));
  }
  return result;
}

bool Converter::VisitPointerType(clang::PointerType *type) {
  if (auto proto = type->getPointeeType()->getAs<clang::FunctionProtoType>()) {
    StrCat(std::format("Option<{} {}>", keyword_unsafe_,
                       ConvertFunctionPointerType(proto)));
    return false;
  }

  if (IsVaListType(clang::QualType(type, 0))) {
    StrCat("VaList");
    return false;
  }

  auto pointee_type = type->getPointeeType();
  StrCat(pointee_type.isConstQualified() ? "*const" : "*mut");
  if (pointee_type->isRecordType() &&
      abstract_structs_.contains(GetID(pointee_type->getAsRecordDecl()))) {
    StrCat(keyword::kDyn);
  }
  return Convert(pointee_type);
}

bool Converter::VisitDecayedType(clang::DecayedType *type) {
  return Convert(type->getDecayedType());
}

bool Converter::VisitTypedefType(clang::TypedefType *type) {
  return Convert(type->desugar());
}

bool Converter::VisitUsingType(clang::UsingType *type) {
  return Convert(type->desugar());
}

bool Converter::Convert(clang::Decl *decl) { return TraverseDecl(decl); }

bool Converter::VisitTranslationUnitDecl(clang::TranslationUnitDecl *decl) {
  for (auto *child : decl->decls()) {
    if (IsUserDefinedDecl(child) &&
        (IsInMainFile(child) || !decl_ids_.contains(GetID(child)))) {
      Convert(child);
      if (!hoisted_records_.empty()) {
        StrCat(hoisted_records_);
        hoisted_records_.clear();
      }
    }
  }
  return false;
}

bool Converter::VisitFunctionDecl(clang::FunctionDecl *decl) {
  if (survey::Enabled()) {
    survey::SetScope(decl->getQualifiedNameAsString());
  }
  if (auto method = clang::dyn_cast<clang::CXXMethodDecl>(decl)) {
    return VisitCXXMethodDecl(method);
  }
  if (!IsConvertibleFunctionDecl(decl)) {
    return false;
  }
  if (!IsInMainFile(decl) && !decl_ids_.insert(GetID(decl)).second) {
    return false;
  }
  decl->dump(log());
  PushCurrFunction push_fn(*this, decl);
  std::string function_name;
  if (decl->isMain()) {
    function_name = "main_0";
    ConvertFunctionMain(decl, function_name);
  } else {
    function_name = GetNamedDeclAsString(decl->getCanonicalDecl());
  }
  // main_0 should be static
  if (!decl->isMain())
    ConvertFunctionQualifiers(decl);
  StrCat(keyword_unsafe_, keyword::kFn, std::move(function_name));
  {
    PushParen paren(*this);
    ConvertFunctionParameters(decl);
  }
  ConvertFunctionReturnType(decl);
  {
    PushBrace brace(*this);
    EmitFunctionPreamble(decl);
    ConvertFunctionBody(decl);
  }
  return false;
}

void Converter::EmitHoistedDecls(clang::CompoundStmt *body) {
  for (auto *child : body->body()) {
    if (auto *decl_stmt = clang::dyn_cast<clang::DeclStmt>(child)) {
      for (auto *decl : decl_stmt->decls()) {
        if (auto *var = clang::dyn_cast<clang::VarDecl>(decl);
            var && var->isLocalVarDecl() && !IsGlobalVar(var)) {
          hoisted_decls_.insert(var);
          if (ConvertVarDeclSkipInit(var)) {
            StrCat(token::kAssign, ConvertVarDefaultInit(var->getType()),
                   token::kSemiColon);
          }
        }
      }
    }
  }
}

void Converter::ConvertGotoBlock(clang::CompoundStmt *body) {
  HoistMaterializedTempBindings hoist_temps(*this);
  PushHoistedDecls push(hoisted_decls_);
  EmitHoistedDecls(body);

  StrCat("goto_block!");
  {
    PushParen paren(*this);
    PushBrace outer(*this);
    StrCat("'__entry: ");
    std::optional<PushBrace> arm;
    arm.emplace(*this);
    for (auto *child : body->body()) {
      if (auto *label = clang::dyn_cast<clang::LabelStmt>(child)) {
        arm.reset();
        StrCat(std::format("'{}: ", label->getDecl()->getName().str()));
        arm.emplace(*this);
        Convert(label->getSubStmt());
      } else {
        Convert(child);
      }
    }
  }
  StrCat(token::kSemiColon);
}

void Converter::ConvertFunctionBody(clang::FunctionDecl *decl) {
  ConvertBodyStmts(decl->getBody());
  if (auto *dtor = clang::dyn_cast<clang::CXXDestructorDecl>(decl)) {
    StrCat(DestroyMembers(dtor->getParent()));
  }
  if (decl->getReturnType()->isVoidType()) {
    return;
  }
  auto compound = clang::dyn_cast<clang::CompoundStmt>(decl->getBody());
  if (!compound || compound->body_empty()) {
    return;
  }
  if (CompoundHasTopLevelLabel(compound) ||
      !clang::isa<clang::ReturnStmt>(compound->body_back())) {
    StrCat(R"(panic!("ub: non-void function does not return a value"))");
  }
}

bool Converter::VisitFunctionTemplateDecl(clang::FunctionTemplateDecl *decl) {
  for (auto *function_decl : decl->specializations()) {
    VisitFunctionDecl(function_decl);
  }
  return false;
}

bool Converter::VisitVarTemplateDecl(clang::VarTemplateDecl *decl) {
  for (auto *var_decl : decl->specializations()) {
    VisitVarDecl(var_decl);
  }
  return false;
}

void Converter::ConvertVaListVarDecl(clang::VarDecl *decl) {
  if (clang::isa<clang::ParmVarDecl>(decl)) {
    // va_list parameter (decayed to __va_list_tag *)
  } else {
    // va_list local variable
    StrCat(keyword::kLet);
  }
  StrCat(keyword_mut_, GetNamedDeclAsString(decl), token::kColon, "VaList");
}

bool Converter::NeedsMut(const clang::VarDecl *decl, clang::QualType type,
                         llvm::StringRef name) const {
  auto *method_or_null =
      curr_function_ ? clang::dyn_cast<clang::CXXMethodDecl>(curr_function_)
                     : nullptr;
  return ((hoisted_decls_.contains(decl) ||
           (!type.isConstQualified() && !type->isReferenceType())) &&
          ((method_or_null == nullptr) || !method_or_null->isVirtual()) &&
          !IsGlobalVar(decl) && name != "_");
}

bool Converter::ConvertVarDeclSkipInit(clang::VarDecl *decl) {
  auto qual_type = decl->getType();
  if (IsVaListType(qual_type) && decl->isLocalVarDecl()) {
    ConvertVaListVarDecl(decl);
    return true;
  }

  auto name = GetNamedDeclAsString(decl);
  if (decl->isFileVarDecl()) {
    if ((decl->isThisDeclarationADefinition() ==
             clang::VarDecl::DeclarationOnly &&
         !decl->hasInit()) ||
        !globals_.insert(name).second) {
      return false;
    }
    StrCat(AccessSpecifierAsString(decl->getAccess()), keyword::kStatic,
           keyword_mut_);
    ENSURE(decl_ids_.insert(GetID(decl)).second);
    global_inits_.push_back(ForceGlobalInit(decl));
  } else if (decl->isStaticLocal()) {
    StrCat(keyword::kStatic, keyword_mut_);
  } else if (decl->isLocalVarDecl()) {
    StrCat(keyword::kLet);
  }

  if (NeedsMut(decl, qual_type, name)) {
    // If this decl requires 'mut', print it irregardless of the model
    StrCat(keyword::kMut);
  }
  StrCat(name, token::kColon);

  bool is_parm_with_default_value = false;
  if (auto parm = clang::dyn_cast<clang::ParmVarDecl>(decl)) {
    is_parm_with_default_value = HasUsableDefaultArg(parm);
  }

  if (is_parm_with_default_value) {
    StrCat("Option<");
  }
  {
    PushLazyType lazy(*this, IsGlobalVar(decl) && LazyStaticInit());
    Convert(qual_type);
  }
  if (is_parm_with_default_value) {
    StrCat('>');
  }
  return true;
}

// A two-binding structured binding whose Rust model is a 2-tuple.
//
// Measured as the largest remaining terminating abort after c9b6ffc cleared the
// previous one: on 5 of 18 re-run TUs the LAST abort was `unsupported structured
// binding / DecompositionDecl with 2 bindings`, every measured shape two-binding
// over `std::pair` (`auto [it, inserted] = map.try_emplace(...)`) or
// `llvm::detail::DenseMapPair` (`auto [child_val, parent_val] = ...`).
//
// This deliberately IGNORES clang's tuple-like desugaring -- a hidden holding
// VarDecl per binding initialised to `std::get<I>(__d)`. `std::get` has no rule,
// so converting the holding vars would only move the refusal to an unmapped
// call. Instead the initialiser is bound ONCE to a synthetic name and each
// binding reads a tuple index off it, which is exactly the Rust model: both
// `rules/pair` t1 and `rules/densemap` t7 emit a bare Rust 2-tuple. The gate is
// therefore the MAPPED type text, not the C++ class name: any type whose model
// is parenthesised is indexable as `.0` / `.1`.
//
// REFERENCE bindings are REFUSED, not copied. `std::tuple<const mlir::Attribute
// &, const mlir::Attribute &>` (RegDefTracker.cpp), `std::tuple<mlir::Operation
// *&, mlir::Operation *&>` (TransformPagedMemViewImpl.cpp) and `auto &[a, b]`
// all ALIAS in C++, and indexing a by-value temporary would silently duplicate
// what the source shares -- a write through one binding would not be seen
// through the other. Those stay on the loud refusal path.
bool Converter::ConvertTupleDecompositionDecl(clang::DecompositionDecl *decl) {
  // Only a local `let` is lowered: a file-scope or static-local decomposition
  // would need one `static mut` per binding plus the init hoisting that goes
  // with it, and no measured shape is one.
  if (!decl->isLocalVarDecl() || decl->isStaticLocal() || !decl->hasInit()) {
    return false;
  }
  auto bindings = decl->bindings();
  if (bindings.size() != 2) {
    return false;
  }
  auto type = decl->getType();
  // A CONST LVALUE REFERENCE holder is accepted, a mutable or rvalue one is
  // not. Measured shape (5 of 18 first-abort-sampled TUs, all of them the SAME
  // single source site, foldInfrastructure.h:1992:23):
  //     const auto& [fp_dim, func] = dim_prop_.at(i);
  //   DecompositionDecl        'const value_type &'  (= const std::pair<const
  //                                                    FoldDimProp *, BaseFuncType> &)
  //   BindingDecl fp_dim      'const FoldDimProp *const'   <- NOT a reference
  //   BindingDecl func        'const BaseFuncType'         <- NOT a reference
  // so the reference-ness is entirely in the HOLDER; the bindings clang builds
  // for a tuple-like `const pair &` are const NON-reference tuple_element_t.
  // That is why a copy of the holder is sound HERE and only here: every
  // binding is const, so no write can travel back through the alias, and each
  // binding's type is a scalar (pointer / enum / arithmetic), so the element
  // copy is bit-for-bit and needs no clone. A NON-const reference holder, or
  // any binding that is itself a reference or a class type, still ALIASES
  // observably and stays on the loud refusal path -- see the REFERENCE
  // paragraph above.
  if (type->isReferenceType()) {
    if (!type->isLValueReferenceType()) {
      return false;
    }
    auto pointee = type.getNonReferenceType();
    if (!pointee.isConstQualified()) {
      return false;
    }
    for (const auto *binding : bindings) {
      auto bt = binding->getType();
      if (bt->isReferenceType() || !bt.isConstQualified()) {
        return false;
      }
      // Scalars only: a class-typed element would need a clone whose semantics
      // this function cannot establish, and a pointer-to-non-const element
      // would let a write reach the aliased pair.
      if (!bt->isPointerType() && !bt->isEnumeralType() &&
          !bt->isIntegralType(ctx_) && !bt->isFloatingType()) {
        return false;
      }
      if (bt->isPointerType() &&
          !bt->getPointeeType().isConstQualified()) {
        return false;
      }
    }
    // Everything below is written against the VALUE type.
    type = pointee;
  } else {
    for (const auto *binding : bindings) {
      if (binding->getType()->isReferenceType()) {
        return false;
      }
    }
  }
  if (!Mapper::Contains(type.getUnqualifiedType())) {
    return false;
  }
  // The model must be a Rust tuple for `.0` / `.1` to mean the C++ elements.
  // With exactly two bindings a parenthesised model can only be a 2-tuple --
  // any other arity would not have type-checked in C++.
  const std::string model = Mapper::Map(type.getUnqualifiedType());
  if (model.size() < 2 || model.front() != '(' || model.back() != ')') {
    return false;
  }

  // REFCOUNT IS REFUSED, measured: that model wraps every local in
  // `Rc<RefCell<..>>`, so the holder comes out as
  // `Rc<RefCell<(Rc<RefCell<i32>>, Rc<RefCell<bool>>)>>` and the index gives
  // `E0609: no field `0``. Reaching the elements needs a borrow of the cell
  // before the index, which is not expressible from here without duplicating
  // that model's access-mode expansion. `keyword_unsafe_` is the only model
  // discriminator the base class has -- ConverterRefCount passes "" for it
  // (converter_refcount.cpp:33) and ConverterUnsafe passes "unsafe".
  if (keyword_unsafe_ == nullptr || *keyword_unsafe_ == '\0') {
    return false;
  }

  HoistMaterializedTempBindings hoist_temps(*this);
  // A DecompositionDecl has no name of its own.
  const std::string holder = GetDecompositionIterName(decl);
  StrCat(keyword::kLet, holder, token::kColon);
  // Annotate: without the type, `let t = <init>;` left one measured case at
  // `E0282: type annotations needed` because the tuple element types are only
  // pinned by the (separate) binding statements.
  Convert(type);
  StrCat(token::kAssign);
  ConvertVarInit(type, decl->getInit());
  StrCat(token::kSemiColon);

  unsigned index = 0;
  for (const auto *binding : bindings) {
    StrCat(keyword::kLet, keyword::kMut, GetNamedDeclAsString(binding),
           token::kAssign);
    StrCat(std::format("{}.{}", holder, index));
    StrCat(token::kSemiColon);
    ++index;
  }
  return true;
}

bool Converter::ConvertLambdaVarDecl(clang::VarDecl *decl) {
  if (decl->getType()->isFunctionPointerType()) {
    return false;
  }
  if (decl->hasInit()) {
    if (clang::isa<clang::LambdaExpr>(
            decl->getInit()->IgnoreUnlessSpelledInSource())) {
      // Lambdas are inlined at the call site.
      return true;
    }
  }
  return false;
}

void Converter::ConvertVarDeclInitializer(clang::VarDecl *decl) {
  if (decl->hasInit()) {
    ConvertVarInit(decl->getType(), decl->getInit());
  } else if (!clang::isa<clang::ParmVarDecl>(decl)) {
    StrCat(ConvertVarDefaultInit(decl->getType()));
  }
}

void Converter::EmitHoistedInArmAssignment(clang::VarDecl *decl) {
  if (!decl->hasInit()) {
    return;
  }
  StrCat(GetNamedDeclAsString(decl), token::kAssign);
  ConvertVarInit(decl->getType(), decl->getInit());
  StrCat(token::kSemiColon);
}

void Converter::ConvertVarDecl(clang::VarDecl *decl) {
  if (hoisted_decls_.contains(decl)) {
    EmitHoistedInArmAssignment(decl);
    return;
  }

  HoistMaterializedTempBindings hoist_temps(*this);
  if (!ConvertVarDeclSkipInit(decl)) {
    // Skip global variables declared extern
    return;
  }
  PushConstInitializer static_init(*this, decl->isFileVarDecl() ||
                                              decl->isStaticLocal());
  StrCat(token::kAssign);
  ConvertVarDeclInitializer(decl);
  StrCat(token::kSemiColon);
}

void Converter::ConvertGlobalVarDecl(clang::VarDecl *decl) {
  HoistMaterializedTempBindings hoist_temps(*this);
  if (!ConvertVarDeclSkipInit(decl)) {
    // Skip global variables declared extern
    return;
  }
  PushConstInitializer static_init(*this, decl->isFileVarDecl() ||
                                              decl->isStaticLocal());
  StrCat(token::kAssign);
  {
    PushLazyInit lazy(*this, LazyStaticInit());
    StrCat(keyword_unsafe_);
    PushBrace push(*this);
    ConvertVarDeclInitializer(decl);
  }
  StrCat(token::kSemiColon);
}

bool Converter::VisitVarDecl(clang::VarDecl *decl) {
  if (clang::isa<clang::VarTemplatePartialSpecializationDecl>(decl)) {
    return false;
  }
  if (auto *decomp = llvm::dyn_cast<clang::DecompositionDecl>(decl)) {
    if (!ConvertTupleDecompositionDecl(decomp)) {
      ReportUnsupportedStructuredBinding(decomp);
    }
    return false;
  }
  if (ConvertLambdaVarDecl(decl)) {
    return false;
  }

  if (IsGlobalVar(decl)) {
    ConvertGlobalVarDecl(decl);
  } else {
    ConvertVarDecl(decl);
  }
  EmitScopedDestructor(decl);

  return false;
}

void Converter::EmitScopedDestructor(const clang::VarDecl *decl) {
  if (in_function_formals_ || !decl->isLocalVarDecl() || IsGlobalVar(decl)) {
    return;
  }
  auto type = decl->getType();
  if (type->isReferenceType() || type->isArrayType() ||
      !TypeNeedsDestruction(type)) {
    return;
  }
  auto name = GetNamedDeclAsString(decl);
  StrCat(token::kSemiColon,
         std::format("let _dtor_{0} = ScopedDestructorUnsafe::new(&raw mut "
                     "{0}, {1}::{2})",
                     name, GetRecordName(type->getAsCXXRecordDecl()),
                     kDestructorName));
}

bool IsPointerType(clang::QualType qual_type) {
  return qual_type->isPointerType() ||
         (qual_type->isArrayType() &&
          IsPointerType(qual_type->getArrayElementTypeNoTypeQual()
                            ->getCanonicalTypeInternal()));
}

bool Converter::RecordDerivesDefault(const clang::RecordDecl *decl) {
  if (auto cxx_decl = clang::dyn_cast<clang::CXXRecordDecl>(decl)) {
    if (GetUserDefinedDefaultConstructor(cxx_decl)) {
      return false;
    }
  }

  for (auto f : decl->fields()) {
    if (f->hasInClassInitializer()) {
      return false;
    }

    // Records that contain function pointer do not derive Default
    if (auto ptr_ty = f->getType()->getAs<clang::PointerType>()) {
      if (ptr_ty->getPointeeType()->isFunctionType()) {
        return false;
      }
    }

    // Records that contain std::array do not derive Default
    if (Mapper::ToString(f->getType()).contains("std::array")) {
      return false;
    }

    // Records that contain C arrays do not derive Default
    if (f->getType()->isArrayType()) {
      return false;
    }

    // Records that contain libc types do not derive Default
    if (auto record = f->getType()->getAsRecordDecl()) {
      if (ctx_.getSourceManager().isInSystemHeader(record->getLocation()) &&
          f->getType().isPODType(ctx_)) {
        return false;
      }
    }
  }

  return true;
}

bool Converter::IsPassThroughRule(clang::Expr *expr) const {
  const auto *rule = Mapper::GetExprRule(GetCalleeOrExpr(expr));
  return rule && rule->body.size() == 1 &&
         std::holds_alternative<TranslationRule::PlaceholderFragment>(
             rule->body[0]);
}

bool Converter::RecordDerivesCopy(const clang::RecordDecl *decl) const {
  auto *derives = Mapper::MappedDerives(ctx_.getCanonicalTagType(decl));
  return derives &&
         std::find(derives->begin(), derives->end(), "Copy") != derives->end();
}

bool Converter::RecordHasCopyableFields(const clang::RecordDecl *decl) {
  if (auto *cxx = clang::dyn_cast<clang::CXXRecordDecl>(decl);
      cxx && RecordNeedsDestruction(cxx)) {
    return false;
  }
  for (auto f : decl->fields()) {
    // Records that contain std::vector, std::array, std::string or anything
    // that is translated to Vec<>, do not derive Copy
    auto mapped = Mapper::Map(f->getType());
    if (mapped.starts_with("Vec<")) {
      return false;
    }

    if (IsUniquePtr(f->getType())) {
      return false;
    }

    if (mapped.starts_with("BTreeMap<")) {
      return false;
    }

    if (auto ptr_ty = f->getType()->getAs<clang::PointerType>()) {
      if (ptr_ty->getPointeeType()->isFunctionType()) {
        if (!FunctionPointerImplementsCopy()) {
          return false;
        }
      }
    }

    // Look recursively into fields that are RecordDecl
    if (auto field_record = f->getType()->getAsRecordDecl()) {
      if (!RecordDerivesCopy(field_record)) {
        return false;
      }
    }
  }

  return true;
}

bool Converter::VisitRecordDecl(clang::RecordDecl *decl) {
  decl->dump(log());

  // VisitCXXRecordDecl already visited the record
  if (clang::isa<clang::CXXRecordDecl>(decl)) {
    return true;
  }

  if (!decl->isCompleteDefinition()) {
    return false;
  }

  if (!record_decls_.MarkDefined(GetRecordName(decl))) {
    return false;
  }

  Mapper::AddRuleForUserDefinedType(decl);
  EmitRustStructOrUnion(decl);

  return false;
}

void Converter::EmitRustStructOrUnion(clang::RecordDecl *decl) {
  // Enums and static variables. In rust they live outside the record
  for (auto *d : decl->decls()) {
    if (auto *enum_decl = llvm::dyn_cast<clang::EnumDecl>(d)) {
      VisitEnumDecl(enum_decl);
    }
    if (auto *var_decl = clang::dyn_cast<clang::VarDecl>(d)) {
      VisitVarDecl(var_decl);
    }
    if (auto *friend_decl = clang::dyn_cast<clang::FriendDecl>(d)) {
      if (auto *fn = clang::dyn_cast_or_null<clang::FunctionDecl>(
              friend_decl->getFriendDecl());
          fn && fn->isThisDeclarationADefinition()) {
        VisitFunctionDecl(fn);
      }
      if (auto *tmpl = clang::dyn_cast_or_null<clang::FunctionTemplateDecl>(
              friend_decl->getFriendDecl())) {
        for (auto *spec : tmpl->specializations()) {
          if (spec->isThisDeclarationADefinition()) {
            VisitFunctionDecl(spec);
          }
        }
      }
    }
  }

  // Inner records. In rust they live outside the record
  for (auto *d : decl->decls()) {
    if (auto *nested = clang::dyn_cast<clang::RecordDecl>(d)) {
      if (!nested->isImplicit()) {
        inner_structs_[GetID(nested)] = GetRecordName(nested);
        if (auto *cxx = clang::dyn_cast<clang::CXXRecordDecl>(nested)) {
          VisitCXXRecordDecl(cxx);
        } else {
          VisitRecordDecl(nested);
        }
      }
    }
    if (auto *nested_tmpl = clang::dyn_cast<clang::ClassTemplateDecl>(d)) {
      for (auto *spec : nested_tmpl->specializations()) {
        inner_structs_[GetID(spec)] = GetRecordName(spec);
        VisitCXXRecordDecl(spec);
      }
    }
  }

  if (decl->isUnion()) {
    EmitRustUnion(decl);
    return;
  }

  // Derived traits
  if (EmitsReprCForRecords()) {
    EmitReprC(decl);
  }
  auto attrs = GetStructAttributes(decl);
  Mapper::SetDerives(ctx_.getCanonicalTagType(decl),
                     std::vector<std::string>(attrs.begin(), attrs.end()));
  StrCat("#[derive(");
  for (auto *attr : attrs) {
    StrCat(attr, ',');
  }
  StrCat(")]");

  // Fields
  auto access = clang::dyn_cast<clang::CXXRecordDecl>(decl)
                    ? AccessSpecifierAsString(decl->getAccess())
                    : keyword::kPub;
  StrCat(access, keyword::kStruct, GetRecordName(decl));
  {
    PushBrace brace(*this);
    for (auto *field : decl->fields()) {
      VisitFieldDecl(field);
    }
  }

  // C++ method decls
  if (auto *cxx = clang::dyn_cast<clang::CXXRecordDecl>(decl)) {
    ConvertCXXRecordMethods(cxx);
    ConvertVirtualMethods(cxx);
  }

  // Traits
  if (auto *cxx = clang::dyn_cast<clang::CXXRecordDecl>(decl)) {
    AddOrdTrait(cxx);
  }
  AddCloneTrait(decl);
  AddDefaultTrait(decl);
  AddByteReprTrait(decl);
}

void Converter::ConvertLateInstantiatedMethods(clang::CXXRecordDecl *decl) {
  ConvertCXXMethodDecls(
      decl, std::format("{} {}", keyword::kImpl, GetRecordName(decl)),
      [](auto *method) {
        return IsEmittableMethod(method) && method->hasBody() &&
               !decl_ids_.contains(GetMethodID(method));
      });
}

void Converter::ConvertCXXRecordMethods(clang::CXXRecordDecl *decl) {
  ConvertCXXMethodDecls(
      decl, std::format("{} {}", keyword::kImpl, GetRecordName(decl)),
      IsEmittableMethod);

  if (GetUserDefinedDestructor(decl) || !HasFieldsNeedingDestruction(decl)) {
    return;
  }
  StrCat(keyword::kImpl, GetRecordName(decl));
  PushBrace impl_brace(*this);
  StrCat(keyword::kPub, keyword_unsafe_, keyword::kFn, kDestructorName,
         "(&mut self)");
  PushBrace fn_brace(*this);
  StrCat(DestroyMembers(decl));
}

std::string Converter::DestroyMembers(const clang::CXXRecordDecl *decl) {
  std::vector<const clang::FieldDecl *> fields;
  for (auto *field : decl->fields()) {
    if (TypeNeedsDestruction(field->getType())) {
      fields.push_back(field);
    }
  }

  std::string out;
  for (auto *field : std::ranges::reverse_view(fields)) {
    auto name = GetNamedDeclAsString(field);
    auto type = field->getType();
    if (type->isArrayType()) {
      auto *elem = type->getBaseElementTypeUnsafe()->getAsCXXRecordDecl();
      assert(elem);
      out +=
          std::format("for __e in self.{0}.iter_mut() {{ {1}::{2}(__e); }}\n",
                      name, GetRecordName(elem), kDestructorName);
    } else {
      out += std::format("{1}::{2}(&mut self.{0});\n", name,
                         GetRecordName(type->getAsCXXRecordDecl()),
                         kDestructorName);
    }
  }
  return out;
}

void Converter::EmitReprC(clang::RecordDecl *decl) {
  if (decl->hasAttr<clang::AlignedAttr>()) {
    StrCat(std::format("#[repr(C, align({}))]",
                       ctx_.getTypeAlign(ctx_.getCanonicalTagType(decl)) / 8));
    return;
  }
  StrCat("#[repr(C)]");
}

void Converter::EmitRustUnion(clang::RecordDecl *decl) {
  EmitReprC(decl);
  auto attrs = GetStructAttributes(decl);
  Mapper::SetDerives(ctx_.getCanonicalTagType(decl),
                     std::vector<std::string>(attrs.begin(), attrs.end()));
  StrCat("#[derive(");
  for (auto *attr : attrs) {
    StrCat(attr, ',');
  }
  StrCat(")]");

  StrCat(keyword::kPub, keyword::kUnion, GetRecordName(decl));
  {
    PushBrace brace(*this);
    if (decl->field_empty()) {
      StrCat("__empty: u8,");
    }
    for (auto *field : decl->fields()) {
      VisitFieldDecl(field);
    }
  }

  AddDefaultTrait(decl);
  AddByteReprTrait(decl);
}

bool Converter::VisitCXXRecordDecl(clang::CXXRecordDecl *decl) {
  decl->dump(log());

  Mapper::AddRuleForUserDefinedType(decl);
  if (!IsConvertibleCXXRecordDecl(decl)) {
    return false;
  }

  if (decl->isStruct() || decl->isClass()) {
    for (auto c : GetTemplateInstantiatedCtors(decl)) {
      if (!decl_ids_.contains(GetID(c))) {
        StrCat(keyword::kImpl, GetRecordName(decl));
        PushBrace brace(*this);
        VisitCXXMethodDecl(c);
      }
    }

    if (!record_decls_.MarkDefined(GetRecordName(decl))) {
      // Other translation units may instantiate members this one did not.
      if (!decl->isAbstract()) {
        ConvertLateInstantiatedMethods(decl);
      }
      return false;
    }

    if (decl->isAbstract()) {
      ConvertAbstractClass(decl);
      // An abstract class becomes a trait, so `EmitRustStructOrUnion` is not
      // reached -- but a nested enum lives at module scope in Rust either way
      // and is referenced by name from every derived class. Emitting the trait
      // must not lose it.
      for (auto *d : decl->decls()) {
        if (auto *enum_decl = llvm::dyn_cast<clang::EnumDecl>(d)) {
          VisitEnumDecl(enum_decl);
        }
      }
      return false;
    }

    DefineImplicitMembers(decl);
    EmitRustStructOrUnion(decl);
  } else if (decl->isUnion()) {
    if (!record_decls_.MarkDefined(GetRecordName(decl))) {
      return false;
    }
    EmitRustStructOrUnion(decl);
  } else if (survey::Enabled()) {
    survey::Record(survey::GapKind::kUnsupportedConstruct,
                   "record kind: " + std::string(decl->getKindName()),
                   decl->getLocation().printToString(ctx_.getSourceManager()));
  } else {
    // FIXME: improve error handling
    assert(0 && "unsupported record kind");
  }

  return false;
}

void Converter::DefineImplicitMembers(clang::CXXRecordDecl *decl) {
  clang::Scope tu_scope(nullptr, clang::Scope::DeclScope,
                        sema_->getDiagnostics());
  tu_scope.setEntity(ctx_.getTranslationUnitDecl());
  auto *saved_tu_scope = std::exchange(sema_->TUScope, &tu_scope);
  sema_->ForceDeclarationOfImplicitMembers(decl);
  for (auto ctor : decl->ctors()) {
    if (ctor->isCopyConstructor() && ctor->isImplicit() &&
        !ctor->doesThisDeclarationHaveABody() && !ctor->isDeleted()) {
      sema_->DefineImplicitCopyConstructor(decl->getLocation(), ctor);
    }
    if (ctor->isMoveConstructor() && !ctor->isUserProvided() &&
        !ctor->doesThisDeclarationHaveABody() && !ctor->isDeleted() &&
        !HasDefaultedCopyConstructor(decl)) {
      sema_->DefineImplicitMoveConstructor(decl->getLocation(), ctor);
    }
  }
  for (auto *method : decl->methods()) {
    if (method->isMoveAssignmentOperator() && !method->isUserProvided() &&
        !method->doesThisDeclarationHaveABody() && !method->isDeleted() &&
        !HasDefaultedCopyAssignment(decl)) {
      sema_->DefineImplicitMoveAssignment(decl->getLocation(), method);
    }
  }
  auto define_defaulted_comparison = [&](clang::FunctionDecl *fn) {
    if (!fn || !IsComparisonOperator(fn) || !fn->isDefaulted() ||
        fn->doesThisDeclarationHaveABody()) {
      return;
    }
#if CLANG_VERSION_MAJOR >= 24
    auto kind = fn->getDefaultedComparisonKind();
#else
    auto kind = sema_->getDefaultedComparisonKind(fn);
#endif
    sema_->DefineDefaultedComparison(decl->getLocation(), fn, kind);
  };
  for (auto *method : decl->methods()) {
    define_defaulted_comparison(method);
  }
  for (auto *friend_decl : decl->friends()) {
    define_defaulted_comparison(clang::dyn_cast_or_null<clang::FunctionDecl>(
        friend_decl->getFriendDecl()));
  }
  sema_->TUScope = saved_tu_scope;
}

bool Converter::VisitCXXMethodDecl(clang::CXXMethodDecl *decl) {
  if (survey::Enabled()) {
    survey::SetScope(decl->getQualifiedNameAsString());
  }
  decl->dump(log());
  if (!ShouldConvertMethod(decl)) {
    return false;
  }
  if (decl->getDescribedFunctionTemplate() || decl->isDependentContext() ||
      (!decl->isPureVirtual() && !decl->hasBody())) {
    return false;
  }
  if (!decl_ids_.insert(GetMethodID(decl)).second) {
    return false;
  }
  PushCurrFunction push_fn(*this, decl);

  if (decl->isOutOfLine() && !decl->overridden_methods().empty()) {
    return ConvertOutOfLineVirtualMethod(decl);
  }
  if (decl->isOutOfLine() && !decl->isTemplateInstantiation()) {
    return ConvertOutOfLineMethod(decl);
  }
  return ConvertCXXMethodDecl(decl);
}

bool Converter::ShouldConvertMethod(const clang::CXXMethodDecl *decl) {
  return IsConvertibleCXXMethodDecl(decl);
}

bool Converter::ConvertOutOfLineMethod(clang::CXXMethodDecl *decl) {
  StrCat(keyword::kImpl, GetRecordName(decl->getParent()));
  PushBrace impl_brace(*this);
  return ConvertCXXMethodDecl(decl);
}

std::string Converter::GetMethodName(const clang::CXXMethodDecl *decl) {
  if (clang::isa<clang::CXXDestructorDecl>(decl)) {
    return kDestructorName;
  }
  if (const char *name = GetCopyOrMoveName(decl);
      name && CanUseCopyOrMoveName(decl, name)) {
    return name;
  }
  if (IsOverloadedMethod(decl)) {
    return GetOverloadedFunctionName(decl);
  }
  if (auto *conversion = clang::dyn_cast<clang::CXXConversionDecl>(decl)) {
    return GetConversionName(
        conversion, GetUnsafeTypeAsString(conversion->getConversionType()));
  }
  return GetNamedDeclAsString(decl);
}

bool Converter::ConvertCXXMethodDecl(clang::CXXMethodDecl *decl) {
  if (auto *ctor = clang::dyn_cast<clang::CXXConstructorDecl>(decl)) {
    return VisitCXXConstructorDecl(ctor);
  }

  if (method_target_ == MethodTarget::ValueImpl &&
      (decl->isStatic() ||
       (!decl->isVirtual() && !decl->getParent()->isAbstract()))) {
    ConvertFunctionQualifiers(decl);
  }
  StrCat(keyword_unsafe_, keyword::kFn, GetMethodName(decl));

  {
    PushParen paren(*this);
    if (!decl->isStatic()) {
      StrCat(GetSelfMaybeWithMut(decl), token::kComma);
    }
    ConvertFunctionParameters(decl);
  }
  ConvertFunctionReturnType(decl);
  if (decl->isPureVirtual() || method_target_ == MethodTarget::TraitDecl) {
    StrCat(token::kSemiColon);
  } else if (method_target_ == MethodTarget::TraitDefault &&
             survey::Enabled()) {
    survey::Record(
        survey::GapKind::kMissingTraitBody,
        decl->getParent()->getQualifiedNameAsString() + "::" +
            GetMethodName(decl),
        decl->getLocation().printToString(ctx_.getSourceManager()));
    StrCat(token::kSemiColon);
  } else if (method_target_ == MethodTarget::TraitDefault) {
    // No definition of this method is visible, so there is no body to
    // translate. The INTENT was to refuse the translation rather than emit a
    // placeholder, because an `unimplemented!()` default would compile and then
    // panic at runtime.
    // ⚠️ THAT REFUSAL DOES NOT HAPPEN in this build. The `assert(0)` below is a
    // NO-OP under -DNDEBUG, and unlike the reporters at :4640/:4880 this site is
    // in DECLARATION position, which has NO empty-emission guard: the signature
    // has already been emitted at :1418-1427, so falling through emits
    // `unsafe fn m(&mut self, ...) -> T` with NEITHER a `;` NOR a `{}` body and
    // the emitted crate does not PARSE -- the worst kind of loud, since an
    // unparseable file cannot be measured at all. Deliberately left alone: the
    // two candidate fixes (`StrCat(token::kSemiColon)`, as the --survey branch
    // just above already does, which silently turns a defaulted trait method
    // into a REQUIRED one; or report_fatal_error) are a semantic choice, not a
    // mechanical one. See the assert(0) audit row in CONVERTER-QUEUE.md.
    llvm::errs() << "unsupported trait default body: no visible definition for "
                 << decl->getParent()->getQualifiedNameAsString() << "::"
                 << GetMethodName(decl) << " (declared at "
                 << decl->getLocation().printToString(ctx_.getSourceManager())
                 << ")\n";
    assert(0 && "unsupported trait default body: method has no definition");
  } else {
    PushBrace body(*this);
    EmitFunctionPreamble(decl);
    ConvertFunctionBody(decl);
  }
  return false;
}

std::string Converter::GetSelfMaybeWithMut(const clang::CXXMethodDecl *decl) {
  return MethodNeedsMutableReceiver(decl) ? "&mut self" : "&self";
}

std::string Converter::GetCtorName(clang::CXXConstructorDecl *decl) {
  if (decl->isCopyOrMoveConstructor()) {
    if (const char *name = GetCopyOrMoveName(decl);
        CanUseCopyOrMoveName(decl, name)) {
      return name;
    }
    return GetOverloadedFunctionName(decl);
  }
  return GetNumberOfConvertingCtors(decl->getParent()) != 1
             ? std::format("new_{}", GetCtorIndex(decl))
             : "new";
}

bool Converter::VisitCXXConstructorDecl(clang::CXXConstructorDecl *decl) {
  if (decl->isOutOfLine() ||
      (decl->isImplicit() && !IsConvertibleImplicitMember(decl))) {
    return false;
  }
  PushCurrFunction push_fn(*this, decl);

  if (decl->isCopyOrMoveConstructor() &&
      !decl->doesThisDeclarationHaveABody()) {
    return false;
  }

  // A visibility qualifier is not permitted on a trait item (error[E0449]), and
  // ConvertAbstractClass routes this class's constructors THROUGH the trait
  // body. Emitting `pub` there is unconditionally ill-formed, so suppress it;
  // everywhere else (an inherent `impl`) the `pub` is load-bearing and kept.
  if (!in_trait_body_) {
    ConvertFunctionQualifiers(decl);
  }
  StrCat(keyword_unsafe_, keyword::kFn, GetCtorName(decl));
  {
    PushParen paren(*this);
    ConvertFunctionParameters(decl);
  }
  StrCat(token::kArrow, "Self");
  {
    PushBrace brace(*this);
    ConvertCXXConstructorBody(decl);
  }

  return false;
}

void Converter::ConvertCXXConstructorBody(clang::CXXConstructorDecl *decl) {
  EmitFunctionPreamble(decl);
  StrCat(keyword::kLet, "mut", "this", token::kAssign);
  if (decl->isDelegatingConstructor()) {
    Convert((*decl->init_begin())->getInit());
  } else {
    StrCat("Self");
    PushBrace this_init(*this);
    EmitConstructorFieldInits(decl);
  }

  StrCat(token::kSemiColon);
  ConvertBodyStmts(decl->getBody());
  StrCat("this");
}

void Converter::EmitConstructorFieldInits(clang::CXXConstructorDecl *decl) {
  const auto *record_decl = decl->getParent();
  auto *definition_or_null = decl->getDefinition();
  assert(definition_or_null);
  auto *definition = clang::cast<clang::CXXConstructorDecl>(definition_or_null);

  for (const auto *field : record_decl->fields()) {
    auto field_name = GetNamedDeclAsString(field);
    auto field_type = field->getType();
    const clang::CXXCtorInitializer *ctor_initializer = nullptr;
    for (const auto *init : definition->inits()) {
      if (init->isMemberInitializer() && init->getMember() == field) {
        ctor_initializer = init;
        break;
      }
    }

    if (ctor_initializer) {
      StrCat(field_name, token::kColon);
      ConvertVarInit(field_type, ctor_initializer->getInit());
    } else if (auto *init = field->getInClassInitializer()) {
      StrCat(field_name, token::kColon);
      ConvertVarInit(field_type, init);
    } else {
      StrCat(field_name, token::kColon, GetDefaultAsString(field_type));
    }
    StrCat(token::kComma);
  }
}

bool Converter::VisitFieldDecl(clang::FieldDecl *decl) {
  auto access_spec = AccessSpecifierAsString(decl->getAccess());
  auto field_name = GetNamedDeclAsString(decl);
  StrCat(access_spec, std::move(field_name), token::kColon);
  Convert(decl->getType());
  StrCat(token::kComma);
  return false;
}

void Converter::EmitFunctionPreamble(clang::FunctionDecl *decl) {
  // In the header, the function might be declared as `int foo(int name_1)',
  // while in the source file the function might be defined as `int foo(int
  // name_2)'. We want to get the parameters from the definition if possible,
  // i.e. name_2.
  auto params = decl->getDefinition() ? decl->getDefinition()->parameters()
                                      : decl->parameters();
  for (auto *param : params) {
    if (HasUsableDefaultArg(param)) {
      auto name = GetNamedDeclAsString(param);
      auto type = ToString(param->getType());
      // `unwrap_or(e)` evaluates `e` EAGERLY, so a defaulted argument that is
      // itself a call runs on EVERY call, including the ones that supplied the
      // argument.  Measured: a `int n = MakeDefault()` default made MakeDefault
      // run 4 times for 4 calls, 2 of which passed n explicitly (C++: 2).  The
      // default expression must be lazy.
      auto init = std::format("{}.unwrap_or_else(|| {})", name,
                              ToString(param->getDefaultArg()));
      StrCat(std::format("let mut {} : {} = {}", name, type, init),
             token::kSemiColon);
    }
  }
}

bool Converter::VisitNamespaceDecl(clang::NamespaceDecl *decl) {
  for (auto *child : decl->decls()) {
    if (IsInMainFile(child) || !decl_ids_.contains(GetID(child))) {
      Convert(child);
    }
  }
  return false;
}

bool Converter::VisitTypedefDecl([[maybe_unused]] clang::TypedefDecl *decl) {
  return false;
}

bool Converter::VisitTypeAliasDecl(clang::TypeAliasDecl *) { return false; }

bool Converter::VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl *) {
  return false;
}

bool Converter::VisitStaticAssertDecl(clang::StaticAssertDecl *decl) {
  auto *assert_expr = decl->getAssertExpr();
  if (assert_expr->isValueDependent()) {
    return false;
  }
  std::string condition;
  if (assert_expr->getType()->isBooleanType() &&
      IsRustConstEvaluableExpr(assert_expr)) {
    condition = ToString(assert_expr);
  } else {
    bool value = false;
    ENSURE(assert_expr->EvaluateAsBooleanCondition(value, ctx_));
    condition = value ? keyword::kTrue : keyword::kFalse;
  }
  StrCat(std::format("const _: () = assert!({}{});", condition,
                     GetAssertMessageAsString(assert_expr, ctx_)));
  return false;
}

bool Converter::VisitConceptDecl(clang::ConceptDecl *) { return false; }

static bool IsaSemiColonStmt(const clang::Stmt *stmt) {
  switch (stmt->getStmtClass()) {
  case clang::Stmt::IfStmtClass:
  case clang::Stmt::WhileStmtClass:
  case clang::Stmt::DoStmtClass:
  case clang::Stmt::ForStmtClass:
  case clang::Stmt::CompoundStmtClass:
  case clang::Stmt::CXXForRangeStmtClass:
  case clang::Stmt::CaseStmtClass:
  case clang::Stmt::DefaultStmtClass:
    return false;
  default:
    return true;
  }
}

bool Converter::Convert(clang::Stmt *stmt) {
  PushExprKind push(*this, ExprKind::Void);
  // The STATEMENT-position twin of the placeholder guard in
  // Convert(Expr*, optional<QualType>) (see the argument there). Without it, a
  // statement whose lowering emits nothing -- e.g. every `operator<<` chain
  // that reaches ReportUnsupportedOperatorCall, which under -DNDEBUG merely
  // returns -- was replaced by the bare `;` appended below. That is LEGAL Rust,
  // so the construct vanished with no token and no compile error:
  // dcc/src/Transform/Sentient/LexicalOrdering.cpp's `a_ss << ...; b_ss << ...;`
  // became ` ; ;`, leaving a comparator that compares two empty strings and is
  // constantly false. Silent wrongness, strictly worse than a loud abort, and
  // invisible to every placeholder census.
  //
  // `Cpp2RustUnmappedStmt_<StmtClass>` is a path expression that NOTHING in the
  // converter or the emitted crate defines, so -- exactly like the expression
  // prefix -- it can only ever produce `E0425 cannot find value` at the precise
  // site, never silently resolve.
  const size_t before = rs_code_->size();
  const size_t hoisted_before = hoisted_records_.size();
  auto exited_visit = TraverseStmt(stmt);
  // A NullStmt IS the empty statement; emitting nothing for it is correct. A
  // DeclStmt of a TagDecl legitimately routes its text to hoisted_records_.
  const bool emitted_nothing = rs_code_->size() == before &&
                               hoisted_records_.size() == hoisted_before;
  if (stmt && emitted_nothing && !clang::isa<clang::NullStmt>(stmt)) {
    const std::string loc =
        stmt->getBeginLoc().printToString(ctx_.getSourceManager());
    std::string detail = std::string("no Rust statement text for ") +
                         stmt->getStmtClassName();
    if (curr_function_ != nullptr) {
      detail += ", reached while converting `" +
                curr_function_->getQualifiedNameAsString() + "`";
    }
    if (survey::Enabled()) {
      // Reuses kUnsupportedExpr rather than adding a kind: the sweep tooling
      // greps the existing kind names.
      survey::Record(survey::GapKind::kUnsupportedExpr, detail, loc);
    }
    StrCat(std::string("Cpp2RustUnmappedStmt_") + stmt->getStmtClassName());
    static std::set<std::string> reported;
    if (reported.insert(stmt->getStmtClassName()).second) {
      llvm::errs() << "note: " << detail << " at " << loc
                   << "; emitting the undefined placeholder "
                      "`Cpp2RustUnmappedStmt_"
                   << stmt->getStmtClassName()
                   << "` instead of silently dropping the statement\n";
    }
  }
  if (stmt && IsaSemiColonStmt(stmt)) {
    StrCat(token::kSemiColon);
  }
  return exited_visit;
}

void Converter::ConvertBody(clang::Stmt *body) {
  PushBrace brace(*this);
  ConvertBodyStmts(body);
}

void Converter::ConvertBodyStmts(clang::Stmt *body) {
  auto *compound = clang::dyn_cast_or_null<clang::CompoundStmt>(body);
  if (!compound) {
    Convert(body);
    return;
  }
  if (CompoundHasTopLevelLabel(compound)) {
    ConvertGotoBlock(compound);
    return;
  }
  for (auto *child : compound->body()) {
    Convert(child);
  }
}

bool Converter::VisitCompoundStmt(clang::CompoundStmt *stmt) {
  ConvertBody(stmt);
  return false;
}

bool Converter::VisitDeclStmt(clang::DeclStmt *stmt) {
  for (auto *decl : stmt->decls()) {
    if (clang::isa<clang::TagDecl>(decl)) {
      Buffer buf(*this);
      Convert(decl);
      hoisted_records_ += std::move(buf).str();
      continue;
    }
    Convert(decl);
    StrCat(token::kSemiColon);
  }
  return false;
}

bool Converter::VisitReturnStmt(clang::ReturnStmt *stmt) {
  auto return_type = curr_function_->getReturnType();
  if (!return_type->isVoidType()) {
    HoistMaterializedTempBindings hoist_temps(*this);
    StrCat(keyword::kReturn);
    ConvertVarInit(return_type, stmt->getRetValue());
  } else {
    Convert(stmt->getRetValue());
    StrCat(token::kSemiColon, keyword::kReturn, token::kSemiColon);
  }
  return false;
}

// --- C++ exceptions: LOUD, never silent ---------------------------------------
//
// There is no lowering for `throw`, `try` or `catch`. What existed before was
// WORSE than no lowering: RecursiveASTVisitor's default Visit returns true, so
// `Convert(stmt)` traversed INTO the CXXThrowExpr's subexpression and emitted
// only that -- `throw MyExc("boom");` became `MyExc::new(c"boom");`, an object
// constructed and dropped -- then appended a `;` and carried on. The TU
// translated rc=0 with a ZERO-BYTE log and ZERO placeholders while printing the
// statement after the throwing call; the C++ terminates with rc=134. A `catch`
// that silently does not catch is the same class of bug, so all three report.
//
// Under --survey these RECORD and CONTINUE: the complete survey is the fleet's
// work list and a survey run must enumerate every gap in one pass.
void Converter::ReportUnsupportedException(const clang::Stmt *stmt,
                                           const std::string &detail) {
  const std::string loc =
      stmt->getBeginLoc().printToString(ctx_.getSourceManager());
  std::string full = detail;
  if (curr_function_ != nullptr) {
    full += ", reached while converting `" +
            curr_function_->getQualifiedNameAsString() + "`";
  }
  if (survey::Enabled()) {
    survey::Record(survey::GapKind::kUnsupportedConstruct, full, loc);
    return;
  }
  // NOT survey mode, so this is a NAMED REFUSAL and must be fatal.
  //
  // This used to be `llvm::errs() << ...; assert(0 && ...)`. Under the release
  // build's -DNDEBUG the assert is a no-op, so this function RETURNED, every
  // caller then did `return false`, and the try/throw/catch emitted NO RUST AT
  // ALL while the converter exited 0. The divert silently vanished from the
  // output -- silent wrongness, which the playbook ranks above a loud abort --
  // and the one stderr line was the only trace, invisible to any sweep reading
  // exit codes.
  //
  // Reachability, established rather than assumed: throw/try/catch ARE lowered
  // now (4976134, 138ec80), and all seven callers of this function are the
  // deliberately-gated shapes that lowering does not model -- bare `throw;`, a
  // `catch (...)`/non-downcastable caught type, a control transfer out of a
  // `try` body or handler (return/break/continue/goto/nested try), and `try`
  // with no handler. So this is dead for every covered shape and fires only for
  // a refusal that already has a written-out reason, which is exactly the thing
  // that should stop the run by name.
  // MEASURED: without `gen_crash_diag=false` this refusal exits 134 (abort) and
  // the SIGABRT handler installed at cpp2rust.cpp:152 prints a ~36-frame
  // backtrace, so a named diagnosis presents as a converter crash. Two harms,
  // both real here: the 403-TU sweep buckets by exit code and would file this
  // with the segfaults, and the backtrace scrolls the one line that says why.
  llvm::report_fatal_error(llvm::Twine("unsupported ") + full + " at " + loc,
                           /*gen_crash_diag=*/false);
}

// `throw <expr>` lowers to `std::panic::panic_any(<expr>)`.
//
// This is context-free: the payload is carried as `Box<dyn Any + Send>` and the
// unwind IS the C++ unwind. Proven byte-exact end to end against
// $TC/shim4/clang++ (probe/excmech): throw from a callee, caught by `const&` in
// main, `what()` printed, rc=7 -- same stdout, same exit code.
//
// It relies on `-C panic=unwind`, which is the default everywhere in this repo
// (no Cargo.toml sets a `[profile.*] panic`). The lit suite's hand-rolled rustc
// line is the only place that ever said otherwise; see the comment at
// tests/lit/lit/formats/Cpp2RustTest.py:310. Under `panic=abort` this output
// compiles clean and then silently loses the control flow, so the two must stay
// in sync.
//
// A bare `throw;` (rethrow of the active exception) has no equivalent -- it
// needs the in-flight payload, which only a `catch` lowering can supply -- so it
// stays LOUD.
bool Converter::VisitCXXThrowExpr(clang::CXXThrowExpr *expr) {
  if (expr->getSubExpr() == nullptr) {
    ReportUnsupportedException(
        expr, "`throw;` (rethrow of the active exception) has no Rust lowering");
    // Do not traverse into the operand: under --survey that would emit the
    // construction of the thrown object as a discarded statement expression,
    // which is exactly the silent wrongness being reported.
    return false;
  }
  StrCat("std::panic::panic_any", token::kOpenParen);
  {
    PushExprKind push(*this, ExprKind::RValue);
    Convert(expr->getSubExpr());
  }
  StrCat(token::kCloseParen);
  computed_expr_type_ = ComputedExprType::FreshValue;
  return false;
}

// True if `s` (or anything it contains, EXCEPT the body of a nested lambda,
// which is already its own function) transfers control OUT of the statement.
// A `try` body becomes a closure, so a `return`/`break`/`continue`/`goto`
// crossing that boundary would silently mean something else: a `return` would
// return from the closure and fall through to the code after the `try`. Those
// sites stay LOUD.
static bool EscapesEnclosingFunction(const clang::Stmt *s) {
  if (s == nullptr) {
    return false;
  }
  if (clang::isa<clang::ReturnStmt, clang::BreakStmt, clang::ContinueStmt,
                 clang::GotoStmt, clang::IndirectGotoStmt, clang::LabelStmt>(
          s)) {
    return true;
  }
  // A bare `throw;` needs the in-flight payload, which this lowering does not
  // thread through; and a nested `try` is not modelled.
  if (const auto *thr = clang::dyn_cast<clang::CXXThrowExpr>(s)) {
    if (thr->getSubExpr() == nullptr) {
      return true;
    }
  }
  if (clang::isa<clang::CXXTryStmt>(s)) {
    return true;
  }
  for (const auto *child : s->children()) {
    if (child == nullptr) {
      continue;
    }
    if (clang::isa<clang::LambdaExpr>(child)) {
      continue;
    }
    if (EscapesEnclosingFunction(child)) {
      return true;
    }
  }
  return false;
}

// The single monomorphic downcast this lowering can express: the caught type
// must be a concrete, complete, non-`std::` class, so that
// `downcast::<T>()` on the `panic_any` payload is an EXACT type match against
// what `throw T(...)` boxed. A `std::exception` (or any other base-class) catch
// needs a base match, which one `downcast` CANNOT express, so it stays LOUD --
// a catch that never catches is the bug fb0bf9d fixed.
static const clang::CXXRecordDecl *DowncastableCaughtRecord(
    clang::QualType caught) {
  if (caught.isNull()) {  // `catch (...)`
    return nullptr;
  }
  clang::QualType t = caught.getNonReferenceType().getUnqualifiedType();
  if (t->isPointerType()) {
    return nullptr;
  }
  const clang::CXXRecordDecl *record = t->getAsCXXRecordDecl();
  if (record == nullptr || !record->hasDefinition()) {
    return nullptr;
  }
  if (record->isInStdNamespace() || record->getDescribedClassTemplate() ||
      clang::isa<clang::ClassTemplateSpecializationDecl>(record)) {
    return nullptr;
  }
  return record;
}

// `try { B } catch (const T &e) { H }` lowers to the shape proven byte-exact
// against $TC/shim4/clang++ at probe/excmech:
//
//   { let __cc2_hook = std::panic::take_hook();
//     std::panic::set_hook(Box::new(|_| {}));
//     let __cc2_r = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| { B }));
//     std::panic::set_hook(__cc2_hook);
//     if let Err(__cc2_p) = __cc2_r {
//       match __cc2_p.downcast::<T>() {
//         Ok(mut __cc2_e) => { let mut e = *__cc2_e; H }
//         Err(__cc2_p) => std::panic::resume_unwind(__cc2_p),
//       } } }
//
// The hook is silenced across the guarded region because a CAUGHT C++ exception
// prints nothing, while Rust's default hook would write a `thread 'main'
// panicked at` line to stderr. It is restored before `resume_unwind`, so an
// exception this handler does not match still unwinds -- it is never swallowed,
// which is the whole point: a failed `downcast` must abort, not resume normally.
bool Converter::VisitCXXTryStmt(clang::CXXTryStmt *stmt) {
  std::vector<const clang::CXXRecordDecl *> caught_records;
  for (unsigned i = 0; i < stmt->getNumHandlers(); ++i) {
    const clang::CXXCatchStmt *handler = stmt->getHandler(i);
    const clang::CXXRecordDecl *record =
        DowncastableCaughtRecord(handler->getCaughtType());
    if (record == nullptr) {
      std::string caught = handler->getCaughtType().isNull()
                               ? std::string("...")
                               : Mapper::ToString(handler->getCaughtType());
      ReportUnsupportedException(
          stmt, "`catch (" + caught +
                    ")` needs a base-class or catch-all match, which a single "
                    "monomorphic `downcast` cannot express (it would silently "
                    "never catch)");
      return false;
    }
    if (EscapesEnclosingFunction(handler->getHandlerBlock())) {
      ReportUnsupportedException(
          stmt, "`catch` handler contains a bare `throw;`, a nested `try`, or a "
                "label, none of which this lowering models");
      return false;
    }
    caught_records.push_back(record);
  }
  if (stmt->getNumHandlers() == 0) {
    ReportUnsupportedException(stmt, "`try` block with no handler");
    return false;
  }
  if (EscapesEnclosingFunction(stmt->getTryBlock())) {
    ReportUnsupportedException(
        stmt,
        "`try` body transfers control out of the block (return/break/continue/"
        "goto/bare throw/nested try); the body becomes a closure, so that would "
        "silently mean something else");
    return false;
  }

  StrCat(token::kOpenCurlyBracket);
  StrCat("let __cc2_hook = std::panic::take_hook()", token::kSemiColon);
  StrCat("std::panic::set_hook(Box::new(|_| {}))", token::kSemiColon);
  StrCat("let __cc2_r = std::panic::catch_unwind(std::panic::AssertUnwindSafe("
         "|| ");
  StrCat(token::kOpenCurlyBracket);
  ConvertBody(stmt->getTryBlock());
  StrCat(token::kCloseCurlyBracket);
  StrCat("))", token::kSemiColon);
  StrCat("std::panic::set_hook(__cc2_hook)", token::kSemiColon);
  StrCat("if let Err(__cc2_p) = __cc2_r ", token::kOpenCurlyBracket);
  for (unsigned i = 0; i < stmt->getNumHandlers(); ++i) {
    const clang::CXXCatchStmt *handler = stmt->getHandler(i);
    StrCat("match __cc2_p.downcast::<",
           GetRecordName(caught_records[i]), ">() ", token::kOpenCurlyBracket);
    StrCat("Ok(mut __cc2_e) => ", token::kOpenCurlyBracket);
    if (const clang::VarDecl *decl = handler->getExceptionDecl();
        decl != nullptr && !decl->getName().empty()) {
      // A `catch (const T &e)` parameter is a REFERENCE, and the converter
      // models a reference-typed local as a pointer-ish thing whose uses emit
      // `*e`. Binding the value would then be `*<value>`, which rustc rejects
      // (E0614). Bind a reference; bind by value only for `catch (T e)`.
      if (decl->getType()->isReferenceType()) {
        StrCat("let mut ", GetNamedDeclAsString(decl), " = &mut *__cc2_e",
               token::kSemiColon);
      } else {
        StrCat("let mut ", GetNamedDeclAsString(decl), " = *__cc2_e",
               token::kSemiColon);
      }
    } else {
      StrCat("let _ = &*__cc2_e", token::kSemiColon);
    }
    ConvertBody(handler->getHandlerBlock());
    StrCat(token::kCloseCurlyBracket, token::kComma);
    StrCat("Err(__cc2_p) => ");
  }
  StrCat("std::panic::resume_unwind(__cc2_p)", token::kComma);
  for (unsigned i = 0; i < stmt->getNumHandlers(); ++i) {
    StrCat(token::kCloseCurlyBracket);
  }
  StrCat(token::kCloseCurlyBracket);
  StrCat(token::kCloseCurlyBracket);
  return false;
}

bool Converter::VisitCXXCatchStmt(clang::CXXCatchStmt *stmt) {
  std::string caught = stmt->getCaughtType().isNull()
                           ? std::string("...")
                           : Mapper::ToString(stmt->getCaughtType());
  ReportUnsupportedException(
      stmt, "`catch (" + caught +
                ")` has no Rust lowering (it would silently never catch)");
  return false;
}

bool Converter::VisitGotoStmt(clang::GotoStmt *stmt) {
  StrCat(std::format("goto!('{})", stmt->getLabel()->getName().str()));
  return false;
}

void Converter::ConvertCondition(clang::Expr *cond) {
  PushExprKind push(*this, ExprKind::RValue);
  Convert(NormalizeToBool(cond, ctx_));
}

bool Converter::VisitIfStmt(clang::IfStmt *stmt) {
  if (auto *init = stmt->getInit()) {
    PushBrace scope(*this);
    Convert(init);
    stmt->setInit(nullptr);
    Convert(stmt);
    stmt->setInit(init);
    return false;
  }
  // `if (auto x = e)`: clang puts the VarDecl in getConditionVariableDeclStmt()
  // and leaves getInit() NULL, while getCond() is the contextual-bool cast over
  // a DeclRefExpr to `x`. Without hoisting the declaration we would faithfully
  // render a reference to a name that was never bound. Mirror the
  // init-statement hoist above: the enclosing brace makes the variable's scope
  // the whole if-statement (then AND else branches) and evaluates it once.
  if (auto *cond_var = stmt->getConditionVariableDeclStmt()) {
    PushBrace scope(*this);
    Convert(cond_var);
    stmt->setConditionVariableDeclStmt(nullptr);
    Convert(stmt);
    stmt->setConditionVariableDeclStmt(cond_var);
    return false;
  }
  StrCat(keyword::kIf);
  if (auto *cond = clang::dyn_cast<clang::ConstantExpr>(stmt->getCond());
      cond && stmt->isConstexpr()) {
    StrCat(cond->getResultAsAPSInt() != 0 ? keyword::kTrue : keyword::kFalse);
  } else {
    ConvertCondition(stmt->getCond());
  }
  ConvertBody(stmt->getThen());
  if (stmt->hasElseStorage()) {
    StrCat(keyword::kElse);
    if (clang::isa<clang::IfStmt>(stmt->getElse())) {
      Convert(stmt->getElse());
    } else {
      ConvertBody(stmt->getElse());
    }
  }
  return false;
}

bool Converter::VisitWhileStmt(clang::WhileStmt *stmt) {
  PushBreakTarget push(break_target_, BreakTarget::Loop);
  StrCat("'loop_:");
  StrCat(keyword::kWhile);
  ConvertCondition(stmt->getCond());
  curr_for_inc_.emplace_back(nullptr);
  ConvertBody(stmt->getBody());
  curr_for_inc_.pop_back();
  return false;
}

bool Converter::VisitDoStmt(clang::DoStmt *stmt) {
  PushBreakTarget push(break_target_, BreakTarget::Loop);
  const char *control_var = "__do_while";
  StrCat(keyword::kLet, "mut", control_var, token::kAssign, keyword::kTrue,
         token::kSemiColon);
  StrCat("'loop_:", keyword::kWhile, control_var, "||");
  {
    PushParen paren(*this);
    ConvertCondition(stmt->getCond());
  }
  {
    PushBrace loop_brace(*this);
    StrCat(control_var, token::kAssign, keyword::kFalse, token::kSemiColon);
    curr_for_inc_.emplace_back(nullptr);
    ConvertBodyStmts(stmt->getBody());
    curr_for_inc_.pop_back();
  }
  return false;
}

bool Converter::VisitForStmt(clang::ForStmt *stmt) {
  PushBreakTarget push(break_target_, BreakTarget::Loop);
  Convert(stmt->getInit());
  StrCat("'loop_:");
  StrCat(keyword::kWhile);
  if (stmt->getCond() == nullptr) {
    StrCat("true");
  } else {
    ConvertCondition(stmt->getCond());
  }
  {
    PushBrace brace(*this);
    curr_for_inc_.emplace_back(stmt->getInc());
    ConvertBodyStmts(stmt->getBody());
    curr_for_inc_.pop_back();
    Convert(stmt->getInc());
    StrCat(token::kSemiColon);
  }
  return false;
}

void Converter::ConvertLoopVariable(clang::VarDecl *decl,
                                    clang::Expr *range_init,
                                    const std::string &index_name,
                                    const std::string &hoisted_range_name) {
  auto loop_var_type = decl->getType();
  // A DecompositionDecl has no name of its own, so the index variable cannot be
  // derived from it -- the caller passes it in.
  auto loop_var_name =
      index_name.empty() ? GetNamedDeclAsString(decl) : index_name;
  // The range init is emitted once per iteration here, so a hoisted local MUST
  // be used in its place when the caller made one -- otherwise a by-value range
  // init is re-evaluated (and its temporary dropped) every iteration.
  auto emit_range = [&] {
    if (!hoisted_range_name.empty()) {
      StrCat(hoisted_range_name);
    } else {
      Convert(range_init);
    }
  };

  if (loop_var_type->isReferenceType()) {
    auto pointee_type = loop_var_type->getPointeeType();
    emit_range();
    if (pointee_type.isConstQualified()) {
      StrCat(std::format(".as_ptr().add({})", loop_var_name));
    } else {
      StrCat(std::format(".as_mut_ptr().add({})", loop_var_name));
    }
  } else {
    {
      PushExplicitAutoref autoref(*this, /*is_mut=*/false);
      emit_range();
    }
    StrCat(std::format("[{}]", loop_var_name));
    StrCat(".clone()");
  }
}

// A range init that is a by-value temporary reaches here wrapped in a
// MaterializeTemporaryExpr (the `auto&&` range variable binds a reference, so the
// prvalue is materialized and the node is an XVALUE, not a prvalue) -- testing
// `isPRValue()` on the unstripped node silently classifies every by-value call as
// an lvalue and the hoist never fires. Strip it and ask about the underlying
// expression, which is the prvalue we actually care about.
static bool IsByValueRangeInit(const clang::Expr *init) {
  const clang::Expr *e = init->IgnoreParenImpCasts();
  if (const auto *mte = llvm::dyn_cast<clang::MaterializeTemporaryExpr>(e)) {
    e = mte->getSubExpr()->IgnoreParenImpCasts();
  }
  return e->isPRValue();
}

// A re-evaluation-free LVALUE range init: one that `Convert()` may legitimately
// emit TWICE (once for the `.len()` bound, once for the element pointer),
// because evaluating it has no side effect and names the SAME object both
// times.
//
// WHY THIS EXISTS, measured. Before this, the only accepted lvalue shape was a
// bare `DeclRefExpr`, and that -- not the container type -- is what refused the
// three corpus sites that gate the port goal:
//   dsc/dsc2.h:1013   `for (auto& [node, refCount] : allocUsers_)`
//                     allocUsers_ is declared at dsc/dsc2.h:1007 as
//                     `std::vector<std::pair<const ScheduleNode*, int>>`
//   dsc/pcfg.h:347/352/687/692
//                     `for (auto& [_, condAddr] : srcStartCondAndVal)`
//                     declared at dsc/pcfg.h:363 as
//                     `std::vector<std::pair<PcfgLccrCond, FoldManager<int64_t>>>`
// Both ranges ARE `std::vector`. Both are *implicit member accesses* --
// `this->allocUsers_` -- so the node is a MemberExpr, is an lvalue (hence not
// hoistable by 6981701d's prvalue path), and is not a DeclRefExpr. Widening the
// accepted lvalue shape to a member-access CHAIN rooted at `this` or at a
// DeclRefExpr is therefore the whole fix; the container check is untouched.
//
// A data-member read off a re-evaluation-free base is as free as a DeclRefExpr,
// and the non-decomposing vector for-range already emits exactly such an lvalue
// twice today (`self.intervals` in LiveRange.cpp), so this adds no hazard that
// is not already pervasive. Everything else is refused: a CALL (side effects, and
// a value-returning call is the prvalue case the hoist already handles), a
// subscript, an overloaded `operator[]`/`operator*`, a deref of a computed
// pointer. Such an lvalue also cannot be hoisted -- binding it by value is a
// MOVE in Rust, not a borrow, which is the E0507 that narrowed 6981701d to
// prvalues only -- so refusing is the only correct answer for it.
static bool IsReEvaluationFreeLValue(const clang::Expr *init) {
  const clang::Expr *e = init->IgnoreParenImpCasts();
  // Bounded walk down the member chain; the bound is belt-and-braces, a member
  // chain is finite by construction.
  for (unsigned depth = 0; depth < 32; ++depth) {
    if (llvm::isa<clang::DeclRefExpr>(e) || llvm::isa<clang::CXXThisExpr>(e)) {
      return true;
    }
    const auto *me = llvm::dyn_cast<clang::MemberExpr>(e);
    if (me == nullptr) {
      return false;
    }
    // DATA members only. A MemberExpr naming a method is part of a call, and a
    // static data member is reached as a DeclRefExpr, not here.
    if (!llvm::isa<clang::FieldDecl>(me->getMemberDecl())) {
      return false;
    }
    // For `a.b` the base is the object lvalue; for `p->b` it is the pointer
    // expression. Reading either twice is free when the base itself is.
    e = me->getBase()->IgnoreParenImpCasts();
  }
  return false;
}

// A structured binding over a `std::vector<std::pair<A, B>>` element. The
// holder is already a RAW POINTER to the element (ConvertLoopVariable emits
// `.as_mut_ptr().add(i)` for the `pair &` the DecompositionDecl is), and
// `std::pair` is modelled as a Rust tuple (rules/pair/tgt_unsafe.rs), so each
// binding is a raw pointer to a tuple field. It MUST be a pointer, not a copy:
// for a mutable `pair &` holder clang gives the BindingDecls plain
// non-reference `tuple_element_t` types, so the aliasing is invisible in the
// binding types and any by-value lowering would SILENTLY DROP WRITES.
// ScopedPtrBindings + the per-use deref in VisitDeclRefExpr restore it.
bool Converter::EmitVectorDecompositionBindings(
    const clang::DecompositionDecl *decl, const std::string &holder_name) {
  auto bindings = decl->bindings();
  if (bindings.empty() || bindings.size() > 2) {
    return false;
  }
  // Only a `pair &` holder is lowered: the pointer-to-field form below is only
  // sound when the holder aliases the container's storage.
  if (!decl->getType()->isReferenceType()) {
    return false;
  }
  const bool is_const = decl->getType()->getPointeeType().isConstQualified();
  unsigned index = 0;
  for (const auto *binding : bindings) {
    StrCat(keyword::kLet);
    StrCat(GetNamedDeclAsString(binding));
    StrCat(token::kAssign);
    StrCat(std::format("&raw {} (*{}).{}", is_const ? "const" : "mut",
                       holder_name, index));
    StrCat(token::kSemiColon);
    ++index;
  }
  return true;
}

void Converter::ConvertForRangeBody(clang::CXXForRangeStmt *stmt,
                                    const clang::VarDecl *map_iter_decl) {
  PushBreakTarget push(break_target_, BreakTarget::Loop);
  std::optional<ScopedMapIterDecl> skip;
  if (map_iter_decl)
    skip.emplace(*this, map_iter_decl);
  curr_for_inc_.emplace_back(nullptr);
  ConvertBodyStmts(stmt->getBody());
  curr_for_inc_.pop_back();
}

// THE RE-EVALUATION HAZARD, and how the hoist discharges it.
// VisitCXXForRangeStmtIndexBased used to emit `Convert(getRangeInit())` TWICE --
// once for the `.len()` bound and again inside the body for the element pointer.
// That is harmless for a bare reference to an existing local, and UNSOUND for a
// range init that is a CALL RETURNING BY VALUE (the C++ temporary is
// lifetime-extended for the whole loop; the Rust rebuilt a fresh vector per
// iteration and `as_mut_ptr()` dangled into one dropped at end of statement -- a
// silent use-after-free, and any side effect in the init ran N times instead of
// once). VisitCXXForRangeStmtIndexBased now HOISTS a PRVALUE range init to one
// named local and uses that local for both the bound and the element pointer, so
// the hazard is gone and a by-value range init is acceptable here. A bare
// DeclRefExpr is deliberately NOT hoisted -- it is already evaluation-free -- and
// an lvalue that is NOT a DeclRefExpr is still emitted twice and still refused:
// hoisting an lvalue by value would be a MOVE in Rust, not a borrow.
bool Converter::IsHoistFreeDecompositionRange(clang::CXXForRangeStmt *stmt) {
  const auto *init = stmt->getRangeInit();
  if (init == nullptr) {
    return false;
  }
  // The two re-evaluation-free shapes, and only those.
  if (!IsReEvaluationFreeLValue(init) && !IsByValueRangeInit(init)) {
    return false;
  }
  // Maps go down VisitCXXForRangeStmtMap; strings are `len()-1`-indexed chars
  // and have no fields to bind. Only the vector path is lowered.
  auto class_name = GetClassName(init->getType());
  if (class_name != "std::vector") {
    return false;
  }
  // Refcount is refused wholesale: that model wraps every local in
  // `Rc<RefCell<..>>`, so `&raw mut (*holder).0` is unreachable without
  // duplicating its access-mode expansion. `keyword_unsafe_` is the only model
  // discriminator the base class has.
  if (keyword_unsafe_ == nullptr || *keyword_unsafe_ == '\0') {
    return false;
  }
  return true;
}

bool Converter::VisitCXXForRangeStmt(clang::CXXForRangeStmt *stmt) {
  auto range_init_type = stmt->getRangeInit()->getType();
  // A decomposing loop variable is only lowered on the MAP path (see
  // VisitCXXForRangeStmtMap). Every other range shape still fails loudly.
  if (auto *decomp =
          llvm::dyn_cast<clang::DecompositionDecl>(stmt->getLoopVariable())) {
    if (!IsMapLikeRangeClass(GetClassName(range_init_type)) &&
        !IsHoistFreeDecompositionRange(stmt)) {
      ReportUnsupportedStructuredBinding(decomp);
      return false;
    }
    // A DECOMPOSING loop over std::unordered_map is routed to the map path
    // (rules/unordered_map models the iterator as `UnsafeHashMapIterator`,
    // which has the same `first()`/`second()` MapIterator surface as
    // `UnsafeMapIterator`). Only the decomposing case is rerouted: a
    // non-decomposing `for (auto &kv : unordered_map)` keeps going down the
    // pre-existing (index-based) path, so no existing output changes.
    if (GetClassName(range_init_type) == "std::unordered_map") {
      return VisitCXXForRangeStmtMap(stmt);
    }
  }

  if (!Mapper::Contains(range_init_type.getUnqualifiedType())) {
    // FIXME: improve error handling
    log() << "for range stmts only for types in std namespace\n";
  }

  log() << "GetClassName: " << GetClassName(range_init_type) << '\n';

  if (GetClassName(range_init_type) == "std::map") {
    return VisitCXXForRangeStmtMap(stmt);
  }
  if (GetClassName(range_init_type) == "std::basic_string") {
    return VisitCXXForRangeStmtString(stmt);
  }
  return VisitCXXForRangeStmtVector(stmt);
}

std::string
Converter::GetDecompositionIterName(const clang::DecompositionDecl *decl) {
  auto loc = ctx_.getSourceManager().getPresumedLoc(decl->getLocation());
  if (loc.isInvalid()) {
    return std::format("__decomp_{}", static_cast<const void *>(decl));
  }
  return std::format("__decomp_{}_{}", loc.getLine(), loc.getColumn());
}

// A structured binding over a std::map element is the ONE shape this converter
// can lower without any new rule support, because the two expressions it needs
// are the two it already emits for `it->first` / `it->second` on a map
// iterator: `<it>.first()` (*const K) and `<it>.second()` (*mut V). The map
// for-range binds its loop variable to an ITERATOR, never to a pair or a tuple
// (Iterator::Item = Self), so a Rust destructuring pattern could not type-check
// here, and the tuple-like initialiser clang builds for `auto& [k, v]` -- a
// hidden holding VarDecl per binding initialised to `std::get<I>(__d)` -- has
// no rule support either. Synthesising the iterator and re-deriving the two
// accessors is the only path that needs nothing new.
bool Converter::EmitMapDecompositionBindings(
    const clang::DecompositionDecl *decl, const std::string &iter_name) {
  auto bindings = decl->bindings();
  if (bindings.size() != 2) {
    return false;
  }
  static const char *const kAccessors[] = {"first", "second"};
  unsigned index = 0;
  for (const auto *binding : bindings) {
    StrCat(keyword::kLet);
    StrCat(GetNamedDeclAsString(binding));
    StrCat(token::kAssign);
    StrCat(std::format("{}.{}()", iter_name, kAccessors[index]));
    StrCat(token::kSemiColon);
    ++index;
  }
  return true;
}

// The two range classes whose iterator model exposes `first()`/`second()`, i.e.
// the two the decomposing map lowering can address. Kept as one predicate so the
// dispatch and the iterator-name choice cannot drift apart.
bool Converter::IsMapLikeRangeClass(const std::string &class_name) {
  return class_name == "std::map" || class_name == "std::unordered_map";
}

const char *Converter::MapRangeIteratorName(const std::string &class_name) {
  return class_name == "std::unordered_map" ? "UnsafeHashMapIterator"
                                           : "UnsafeMapIterator";
}

bool Converter::VisitCXXForRangeStmtMap(clang::CXXForRangeStmt *stmt) {
  auto *loop_var = stmt->getLoopVariable();
  auto *decomp = llvm::dyn_cast<clang::DecompositionDecl>(loop_var);
  // A DecompositionDecl has no name of its own, so the iterator the loop binds
  // needs a synthetic one.
  auto loop_var_name =
      decomp ? GetDecompositionIterName(decomp) : GetNamedDeclAsString(loop_var);

  if (decomp && decomp->bindings().size() != 2) {
    // Not a key/value decomposition -- keep it loud rather than emit bindings
    // nothing defines.
    ReportUnsupportedStructuredBinding(decomp);
    return false;
  }

  StrCat("'loop_:");
  auto map_type = Mapper::Map(stmt->getRangeInit()->getType());
  StrCat(keyword::kFor, loop_var_name, keyword::kIn,
         std::string(MapRangeIteratorName(
             GetClassName(stmt->getRangeInit()->getType()))) +
             "::begin(&");
  Convert(stmt->getRangeInit());
  StrCat(std::format(" as *const {})", map_type));
  {
    PushBrace brace(*this);
    std::optional<ScopedPtrBindings> ptr_bindings;
    if (decomp) {
      if (!EmitMapDecompositionBindings(decomp, loop_var_name)) {
        ReportUnsupportedStructuredBinding(decomp);
        return false;
      }
      ptr_bindings.emplace(*this, decomp);
    }
    ConvertForRangeBody(stmt, loop_var);
  }

  return false;
}

bool Converter::VisitCXXForRangeStmtString(clang::CXXForRangeStmt *stmt) {
  return VisitCXXForRangeStmtIndexBased(stmt, "len()-1");
}

bool Converter::VisitCXXForRangeStmtVector(clang::CXXForRangeStmt *stmt) {
  return VisitCXXForRangeStmtIndexBased(stmt, "len()");
}

bool Converter::VisitCXXForRangeStmtIndexBased(clang::CXXForRangeStmt *stmt,
                                               const char *len_suffix) {
  auto *loop_var = stmt->getLoopVariable();
  auto *decomp = llvm::dyn_cast<clang::DecompositionDecl>(loop_var);
  // A DecompositionDecl has no name of its own, so both the element holder and
  // the index need synthetic ones.
  auto loop_var_name =
      decomp ? GetDecompositionIterName(decomp) : GetNamedDeclAsString(loop_var);
  auto index_name = decomp ? loop_var_name + "_i" : loop_var_name;

  // THE HOIST. The range init is needed twice (the `.len()` bound, and the
  // element pointer per iteration), so anything that is not a bare DeclRefExpr
  // is bound to ONE local first: a by-value init must be evaluated exactly once
  // and must OUTLIVE the loop, or `as_mut_ptr()` points into a temporary dropped
  // at end of statement. A bare DeclRefExpr is left inline -- it is already
  // evaluation-free, and hoisting it would rewrite most of the corpus for
  // nothing. The name is source-location-derived so it cannot collide with a
  // user local or with a second loop in the same scope.
  // MEASURED, and the reason the test is `isPRValue()` and not "not a
  // DeclRefExpr": hoisting an LVALUE range init changes a borrow into a MOVE.
  // `for (auto& x : self.intervals)` (LiveRange.cpp:97) emitted
  // `let __range = self.intervals;`, which is E0507 move-out-of-borrowed-content
  // in Rust -- a fresh error where the inline form borrowed. Only a PRVALUE (the
  // by-value temporary that is the actual hazard) is hoisted; every lvalue range
  // init is left exactly as it was emitted before, which is also what keeps the
  // corpus byte-identical.
  std::string hoisted_range;
  auto *range_init = stmt->getRangeInit();
  if (range_init != nullptr && IsByValueRangeInit(range_init)) {
    auto loc = ctx_.getSourceManager().getPresumedLoc(stmt->getBeginLoc());
    hoisted_range =
        loc.isValid() ? std::format("__range_{}_{}", loc.getLine(),
                                    loc.getColumn())
                      : std::format("__range_{}", static_cast<const void *>(stmt));
    StrCat(keyword::kLet);
    // The element pointer is taken with `as_mut_ptr()` for a non-const
    // reference loop variable, which needs the binding itself to be mutable.
    auto loop_var_type = loop_var->getType();
    if (loop_var_type->isReferenceType() &&
        !loop_var_type->getPointeeType().isConstQualified()) {
      StrCat(keyword_mut_);
    }
    StrCat(hoisted_range, token::kAssign);
    Convert(range_init);
    StrCat(token::kSemiColon);
  }

  StrCat("'loop_:");
  StrCat(keyword::kFor, index_name, keyword::kIn, "0..");
  {
    PushParen range(*this);
    if (hoisted_range.empty()) {
      Convert(stmt->getRangeInit());
    } else {
      StrCat(hoisted_range);
    }
    StrCat(token::kDot, len_suffix);
  }
  {
    PushBrace body(*this);
    StrCat(keyword::kLet);

    auto loop_var_type = loop_var->getType();
    if (!loop_var_type.isConstQualified()) {
      StrCat(keyword_mut_);
    }

    StrCat(loop_var_name);
    StrCat(token::kAssign);

    ConvertLoopVariable(loop_var, stmt->getRangeInit(),
                        decomp ? index_name : std::string{}, hoisted_range);

    StrCat(token::kSemiColon);
    std::optional<ScopedPtrBindings> ptr_bindings;
    if (decomp) {
      if (!EmitVectorDecompositionBindings(decomp, loop_var_name)) {
        ReportUnsupportedStructuredBinding(decomp);
        return false;
      }
      ptr_bindings.emplace(*this, decomp);
    }
    ConvertForRangeBody(stmt);
  }

  return false;
}

bool Converter::VisitBreakStmt([[maybe_unused]] clang::BreakStmt *stmt) {
  StrCat(keyword::kBreak);
  if (isSwitchBreak()) {
    StrCat("'switch");
  }
  return false;
}

bool Converter::VisitContinueStmt([[maybe_unused]] clang::ContinueStmt *stmt) {
  if (!curr_for_inc_.empty()) {
    Convert(curr_for_inc_.back());
    StrCat(token::kSemiColon);
  }
  StrCat(keyword::kContinue);
  StrCat("'loop_");
  return false;
}

bool Converter::Convert(clang::Expr *expr,
                        std::optional<clang::QualType> implicit_convert_to) {
  bool needs_conversion =
      expr && implicit_convert_to &&
      NeedsImplicitScalarCast(expr->IgnoreImplicit()->getType(),
                              *implicit_convert_to);
  PushParen paren(*this, needs_conversion);
  computed_expr_type_ = ComputedExprType::Unknown;
  // Exactly the type-side defect fixed at Convert(QualType) above, on the
  // EXPRESSION side, and it was silent for the same reason: an Expr that
  // reaches TraverseStmt and matches no Visit* emits ZERO TOKENS, the only
  // guard below was an assert that -DNDEBUG compiles away, and the caller then
  // splices that nothing into a position that syntactically requires an
  // expression. MEASURED after the type fix landed, so these are not a cascade
  // of it: 10 `missing condition for if expression` across 2 files -- 8 of them
  // in dialect_utils/Agen/Utils.cpp, e.g. `if { sum.postfix_inc();`, and
  // dialect_utils/VectorChain/Utils.cpp:98 `if (index != var_idx) return
  // false;` coming out as `if { return false ;` -- plus 14 `expected
  // expression, found keyword as` across 4 files, e.g. Planner.cpp
  // `let __o = ( as *mut std::fs::File);`, which is this same hole seen through
  // a cast: PushParen and the ConvertCast below wrap an operand that was never
  // emitted, so the cast's TYPE survives and its OPERAND vanishes. Every one of
  // those is a PARSE error, which makes the whole file unparseable and destroys
  // the only convergence metric there is.
  //
  // The placeholder must therefore be a syntactically valid Rust EXPRESSION
  // that cannot resolve. A path expression that nothing defines gives a clean
  // `E0425 cannot find value` at the exact site, on a file that parses. The
  // prefix is deliberately distinct from the type side's `Cpp2RustUnmapped_`
  // (it names a VALUE, not a type) and is emitted by nothing else in the
  // converter, so -- unlike the `--mangle-unmapped` spelling, which is what a
  // PORTED entity would be called and can therefore collide with a real `fn`
  // or `static` in the same TU and compile -- it can only ever fail. It carries
  // the AST statement class so the dropped construct is nameable from the rustc
  // error alone, with no converter rerun.
  const size_t before = rs_code_->size();
  bool result = TraverseStmt(expr);
  if (expr && rs_code_->size() == before) {
    const std::string loc =
        expr->getBeginLoc().printToString(ctx_.getSourceManager());
    std::string detail = std::string("no Rust expression text for ") +
                         expr->getStmtClassName() + " of type `" +
                         Mapper::ToString(expr->getType()) + "`";
    if (curr_function_ != nullptr) {
      detail += ", reached while converting `" +
                curr_function_->getQualifiedNameAsString() + "`";
    }
    if (survey::Enabled()) {
      survey::Record(survey::GapKind::kUnsupportedExpr, detail, loc);
    }
    StrCat(std::string("Cpp2RustUnmappedExpr_") + expr->getStmtClassName());
    // Forward progress on BOTH paths: something was emitted, so the value-ness
    // question below has an answer and the run continues to find the next gap.
    computed_expr_type_ = ComputedExprType::FreshValue;
    static std::set<std::string> reported;
    if (reported.insert(expr->getStmtClassName()).second) {
      llvm::errs() << "note: " << detail << " at " << loc
                   << "; emitting the undefined placeholder "
                      "`Cpp2RustUnmappedExpr_"
                   << expr->getStmtClassName() << "` so the file parses\n";
    }
  }
  if (expr && computed_expr_type_ == ComputedExprType::Unknown) {
    // Was `expr->dump(); assert(false && "computed_expr_type_ not set")`, i.e.
    // NOTHING in the shipped -DNDEBUG build: the dump went to stderr, the
    // assert evaporated, and Unknown then flowed into the value-vs-pointer
    // decisions that every caller makes off computed_expr_type_. Same class as
    // ReportUnsupportedException and the DecompositionDecl site above -- a
    // broken internal invariant presenting as plausible-looking output -- so
    // refuse by name instead. Distinct from the empty-emission case just
    // handled: tokens WERE emitted here, only their value-ness is unknown, so
    // the survey path records and keeps walking rather than substituting text.
    const std::string loc =
        expr->getBeginLoc().printToString(ctx_.getSourceManager());
    std::string detail = std::string("computed_expr_type_ not set by ") +
                         expr->getStmtClassName() + " of type `" +
                         Mapper::ToString(expr->getType()) + "`";
    if (curr_function_ != nullptr) {
      detail += ", reached while converting `" +
                curr_function_->getQualifiedNameAsString() + "`";
    }
    if (survey::Enabled()) {
      survey::Record(survey::GapKind::kUnsupportedExpr, detail, loc);
      computed_expr_type_ = ComputedExprType::Value;
    } else {
      llvm::report_fatal_error(llvm::Twine("unsupported ") + detail + " at " +
                                   loc,
                               /*gen_crash_diag=*/false);
    }
  }
  if (needs_conversion) {
    ConvertCast(*implicit_convert_to);
    computed_expr_type_ = ComputedExprType::FreshValue;
  }
  return result;
}

const clang::Expr *Converter::GetParentExpr(const clang::Expr *expr) {
  if (!expr) {
    return nullptr;
  }
  auto parents = ctx_.getParentMapContext().getParents(*expr);
  if (!parents.empty()) {
    auto parent_node = *parents.begin();
    if (auto parent_stmt = parent_node.get<clang::Stmt>()) {
      return dyn_cast<clang::Expr>(parent_stmt);
    }
  }
  return nullptr;
}

// `std::flush` cannot be handled like the other manipulators. `std::endl`,
// `std::hex` and `Setw` all fold into the *text* of the batched `write!` that
// ConvertCallToOstream emits, but a flush is an ACTION on the stream that must
// happen at its exact position in the `<<` chain -- so it is dispatched in
// ConvertCallToOstream's loop, which can force the buffered text out first,
// rather than in GetFmtArg/GetRawArg, which only append to buffers.
//
// It also cannot be a rule key: `std::flush` and `std::endl` have the IDENTICAL
// type `std::ostream &(*)(std::ostream &)`, so a single rule keyed on that
// overload would make `os << std::endl` flush without emitting a newline.
static bool IsStreamFlush(clang::Expr *arg) {
  std::string arg_str = Mapper::ToString(arg);
  return arg_str.contains("std::flush") || arg_str.contains("std::__1::flush");
}

// `os << (const char *)p` prints the bytes at p up to, but not including, the
// terminating NUL. The generic `{:}` arm of GetFmtArg sent it through `write!`,
// which needs `Display` -- and in the UNSAFE model a `const char *` maps to a
// Rust RAW pointer, which implements neither Display nor Debug:
//     error[E0277]: `*const i8` doesn't implement `std::fmt::Display`
// In the REFCOUNT model the same C++ type maps to `Ptr<u8>`, which DOES
// implement Display and already prints the C string correctly. So this is keyed
// off the MAPPED RUST TYPE rather than off the model: a raw pointer can never
// be formatted, a library pointer type can.
//
// Such an argument is routed to GetRawArg instead, i.e. to `write_all` of the
// raw bytes, so no UTF-8 validity is assumed anywhere. `to_str().unwrap()`
// would PANIC where C++ happily writes non-UTF-8 bytes -- a behaviour change,
// not a shortcut.
static bool IsRawCharPointer(clang::Expr *arg) {
  clang::QualType type = arg->getType();
  if (!type->isPointerType() || !type->getPointeeType()->isCharType()) {
    return false;
  }
  std::string mapped = Mapper::Map(type);
  return mapped.starts_with("*const ") || mapped.starts_with("*mut ");
}

bool Converter::GetFmtArg(clang::Expr *arg, std::string &fmt,
                          std::string &fmt_args, const char *&fmt_trait,
                          std::string &fmt_width) {
  std::string arg_str = Mapper::ToString(arg);
  if (auto *str_lit =
          clang::dyn_cast<clang::StringLiteral>(arg->IgnoreImplicit())) {
    if (!IsAsciiStringLiteral(str_lit)) {
      return false;
    }
    auto str = GetEscapedStringLiteral(arg);
    std::string_view trim(str);
    // Delete " from string
    trim.remove_prefix(1);
    trim.remove_suffix(1);
    fmt += trim;
  } else if (auto ch = GetEscapedUTF8CharLiteral(arg); !ch.empty()) {
    fmt += std::move(ch);
  } else if (arg_str.contains("std::endl")) {
    fmt += "\\n";
  } else if (arg_str.contains("std::hex")) {
    fmt_trait = "x";
  } else if (arg_str.contains("std::dec")) {
    fmt_trait = "";
  } else if (arg_str.contains("Setw")) {
    fmt_width = Trim(ToString(arg));
  } else if (!arg->getType()->isCharType() && !IsRawCharPointer(arg) &&
             Mapper::Map(arg->getType()) !=
                 std::format("Vec<{}>", CharRustType())) {
    fmt += ("{:" + fmt_width + fmt_trait + "}");
    fmt_width.clear(); // Reset setw after first usage
    arg_str = ToString(arg);
    if (arg->getType()->isBooleanType()) {
      arg_str = std::format("({} as u8)", std::move(arg_str));
    }
    fmt_args += std::move(arg_str) + ", ";
  } else {
    return false;
  }
  return true;
}

bool Converter::GetRawArg(clang::Expr *arg, std::string &raw_args) {
  if (arg->getType()->isCharType()) {
    raw_args += "(&[" + ToString(arg) + " as u8]";
  } else if (Mapper::Map(arg->getType()) ==
             std::format("Vec<{}>", CharRustType())) {
    PushExprKind push(*this, ExprKind::RValue);
    std::string str = ToString(arg);
    raw_args += "(&(" + str + ").iter().take((" + str +
                ").len() - 1).map(|&c| c as u8).collect::<Vec<u8>>()[..]";
  } else if (Mapper::ToString(arg).contains("std::endl")) {
    raw_args += "(&[b'\\n']";
  } else if (clang::isa<clang::StringLiteral>(arg->IgnoreImplicit())) {
    raw_args += "(b" + GetEscapedStringLiteral(arg);
  } else if (IsRawCharPointer(arg)) {
    // LAST, deliberately: a STRING LITERAL also has type `const char *` after
    // its array-to-pointer decay, and the arm above turns it into a byte string
    // directly, with no pointer round trip.
    //
    // Bytes until NUL, exactly as C++. A NULL pointer is not printed: it is UB
    // in C++, and libstdc++'s `operator<<(ostream &, const char *)` responds by
    // setting badbit and writing nothing (it is printf, not ostream, that
    // prints `(null)`), so writing nothing is the closest honest match. The
    // deref needs its own `unsafe` block: an `unsafe fn` body is not an unsafe
    // context under Rust 2024's `unsafe_op_in_unsafe_fn`.
    PushExprKind push(*this, ExprKind::RValue);
    raw_args += "(unsafe { let __cstr = " + ToString(arg) +
                "; if __cstr.is_null() { &[][..] } else { "
                "::std::ffi::CStr::from_ptr(__cstr as *const ::libc::c_char)"
                ".to_bytes() } }";
  } else {
    return false;
  }
  raw_args += " as &[u8]), ";
  return true;
}

std::string Converter::ConvertStream(clang::Expr *expr) {
  return ToString(expr);
}

std::string Converter::FlushStream(const std::string &stream) {
  return "let _ = ::std::io::Write::flush(&mut " + stream + ");";
}

void Converter::ConvertCallToOstream(clang::CallExpr *expr) {
  clang::Expr *stream = nullptr;
  auto collect_args = [expr, &stream]() -> std::vector<clang::Expr *> {
    std::vector<clang::Expr *> result;
    auto *current = clang::dyn_cast<clang::CXXOperatorCallExpr>(expr);
    if (!current) {
      return {};
    }

    while (current) {
      result.push_back(current->getArg(1));
      if (auto *next =
              clang::dyn_cast<clang::CXXOperatorCallExpr>(current->getArg(0));
          next && IsCallToOstream(next)) {
        current = next;
      } else {
        stream = current->getArg(0);
        break;
      }
    }

    std::reverse(result.begin(), result.end());
    return result;
  };

  std::vector<clang::Expr *> args = collect_args();
  if (args.empty()) {
    return;
  }

  std::string fmt;
  const char *fmt_trait = "";
  std::string fmt_width;
  std::string fmt_args;
  std::string raw_args;
  std::string stream_str = ConvertStream(stream);
  size_t arg_count = args.size();

  auto write_raw_args = [&]() {
    if (!raw_args.empty()) {
      StrCat(stream_str, ".write_all(&([", std::move(raw_args),
             "].concat()));");
      raw_args.clear();
    }
  };

  auto write_fmt_args = [&]() {
    if (!fmt_args.empty() || !fmt.empty()) {
      StrCat("write!(", stream_str, ',',
             std::format(R"("{}",)", std::move(fmt)), std::move(fmt_args),
             ");");
      fmt_args.clear();
      fmt.clear();
    }
  };

  size_t i = 0;
  while (i < arg_count) {
    size_t start = i;
    if (IsStreamFlush(args[i])) {
      // ORDERING: the pending text is part of the chain that PRECEDES this
      // manipulator, so it must be emitted before the flush, or the flush
      // would be emitted before the text it is supposed to flush.
      write_fmt_args();
      write_raw_args();
      // Rust's stdout is line-buffered and write!/print! do not flush, so an
      // explicit flush is required. The error is dropped because C++
      // `os.flush()` sets badbit rather than throwing.
      StrCat(FlushStream(stream_str));
      ++i;
      continue;
    }
    while (i < arg_count && !IsStreamFlush(args[i]) &&
           GetFmtArg(args[i], fmt, fmt_args, fmt_trait, fmt_width))
      ++i;
    write_fmt_args();
    while (i < arg_count && !IsStreamFlush(args[i]) &&
           GetRawArg(args[i], raw_args))
      ++i;
    write_raw_args();
    // Defensive: previously a chain element that neither GetFmtArg nor
    // GetRawArg could consume spun this loop forever. Be loud instead.
    assert(i != start && "no progress converting an ostream `<<` chain");
    if (i == start) {
      break;
    }
  }

  assert(*fmt_trait == '\0' && "Stream state was not restored after call");
}

void Converter::ConvertPrintf(clang::CallExpr *expr) {
  bool is_fprintf =
      Mapper::ToString(expr->getCallee()).starts_with("int fprintf");

  StrCat("printf(");
  for (unsigned i = is_fprintf; i < expr->getNumArgs(); ++i) {
    if (i == is_fprintf ? 1 : 0) {
      Convert(expr->getArg(i));
      StrCat("as *const i8");
    } else {
      Convert(expr->getArg(i));
    }
    StrCat(token::kComma);
  }
  StrCat(')');
}

void Converter::ConvertVariadicArg(clang::Expr *arg) {
  if (arg->getType()->isFunctionPointerType()) {
    Convert(arg);
    StrCat(".map_or(::std::ptr::null_mut(), |f| f as *mut ::libc::c_void)");
    return;
  }
  Convert(arg);
}

void Converter::ConvertVAArgCall(clang::CallExpr *expr) {
  if (IsBuiltinVaStart(expr)) {
    StrCat(ToString(expr->getArg(0)->IgnoreImpCasts()),
           "= VaList::new(__args)");
    return;
  }
  if (IsBuiltinVaEnd(expr)) {
    // va_end is a no-op
    return;
  }
  if (IsBuiltinVaCopy(expr)) {
    StrCat(ToString(expr->getArg(0)->IgnoreImpCasts()), '=',
           ToString(expr->getArg(1)->IgnoreImpCasts()), ".clone()");
    return;
  }
}

bool Converter::VisitCallExpr(clang::CallExpr *expr) {
  if (IsBuiltinVaStart(expr) || IsBuiltinVaEnd(expr) || IsBuiltinVaCopy(expr)) {
    ConvertVAArgCall(expr);
    SetFreshType(expr->getType());
    return false;
  }

  // p->~T() on a scalar is a no-op
  if (clang::isa<clang::CXXPseudoDestructorExpr>(
          expr->getCallee()->IgnoreParenImpCasts())) {
    SetFreshType(expr->getType());
    return false;
  }

  // p->~T() is a no-op when T has nothing to destruct
  if (auto *dtor = clang::dyn_cast_or_null<clang::CXXDestructorDecl>(
          expr->getCalleeDecl());
      dtor && !RecordNeedsDestruction(dtor->getParent())) {
    SetFreshType(expr->getType());
    return false;
  }

  if (IsImplicitAssignmentCall(expr) && !Mapper::Contains(expr->getCallee())) {
    auto *call = clang::cast<clang::CXXMemberCallExpr>(expr);
    ConvertAssignment(call->getImplicitObjectArgument(), call->getArg(0), "=");
    return false;
  }

  if (Mapper::Contains(expr->getCallee())) {
    if (Mapper::IsLibcPassthrough(GetCalleeOrExpr(expr))) {
      ConvertGenericCallExpr(expr);
      return false;
    }

    auto **args = expr->getArgs();
    auto num_args = expr->getNumArgs();
    auto ctx = CollectRefBindingTempArgs(expr);
    std::string str;
    {
      PushExprKind push(*this, ExprKind::RValue);
      str = GetMappedAsString(expr, args, num_args, &ctx);
    };

    bool deref_ref = (IsReferenceType(expr) ||
                      GetReturnTypeOfFunction(expr)->isReferenceType()) &&
                     !isAddrOf() && !isVoid();
    if (deref_ref) {
      str = "( * " + std::move(str) + " )";
    }

    if (!ctx.temporary_bindings.empty()) {
      str = std::format("{{ {} {} }}", ctx.temporary_bindings, str);
    }

    StrCat(str);
    if (deref_ref) {
      SetValueFreshness(expr->getType());
    } else if (!IsPassThroughRule(expr)) {
      SetFreshType(expr->getType());
    }
    return false;
  }

  if (IsTransparentStdCall(expr)) {
    Convert(expr->getArg(0));
    return false;
  }

  if (auto *opcall = clang::dyn_cast<clang::CXXOperatorCallExpr>(expr);
      opcall && !IsUserOperatorCall(opcall) &&
      !Mapper::Contains(expr->getCallee())) {
    return ConvertCXXOperatorCallExpr(opcall);
  }

  std::string str;
  {
    Buffer buf(*this);
    Converter::ConvertCallExpr(expr);
    str = std::move(buf).str();
  }

  auto ty = GetReturnTypeOfFunction(expr);
  auto ref = clang::dyn_cast<clang::ReferenceType>(ty);

  if (ref && !isAddrOf() && !isVoid()) {
    {
      PushParen paren(*this);
      StrCat(GetPointerDerefPrefix(ref->getPointeeType()), str);
    }
    SetValueFreshness(ref->getPointeeType());
    return false;
  }

  StrCat(str);
  SetFreshType(expr->getType());
  return false;
}

void Converter::EmitFnPtrCall(clang::Expr *callee) {
  {
    PushParen paren(*this);
    Convert(callee);
  }
  StrCat(".unwrap()");
}

std::string Converter::GetFunctionRefName(const clang::FunctionDecl *fn_decl) {
  if (auto *method = clang::dyn_cast<clang::CXXMethodDecl>(fn_decl);
      method && method->isStatic()) {
    return std::format("{}::{}", GetRecordName(method->getParent()),
                       GetMethodName(method));
  }
  return Mapper::MapFunctionName(fn_decl);
}

void Converter::ConvertFunctionToFunctionPointer(
    const clang::FunctionDecl *fn_decl) {
  StrCat(std::format("Some({})", GetFunctionRefName(fn_decl)));
  computed_expr_type_ = ComputedExprType::FreshPointer;
}

std::string Converter::ConvertFnPtrCallee(clang::Expr *arg) {
  PushExprKind push(*this, ExprKind::Callee);
  Buffer buf(*this);
  Convert(arg);
  return std::move(buf).str();
}

std::string Converter::ConvertFnPtrPlaceholder(clang::Expr *arg) {
  auto proto =
      arg->getType()->getPointeeType()->getAs<clang::FunctionProtoType>();
  return std::format("({} as {} {})", ConvertFnPtrCallee(arg), keyword_unsafe_,
                     ConvertFunctionPointerType(proto));
}

namespace {

// Strips the qualifiers/refs clang adds around an argument or parameter type so
// an exact overload match can be tested on the underlying type.
clang::QualType StripForOverloadMatch(clang::QualType ty) {
  ty = ty.getNonReferenceType().getCanonicalType().getUnqualifiedType();
  return ty;
}

} // namespace

const clang::FunctionDecl *
Converter::ResolveOverloadedCallee(clang::CallExpr *expr) {
  auto *callee = GetCallee(expr);
  if (!callee) {
    return nullptr;
  }
  const auto *ovl =
      llvm::dyn_cast<clang::OverloadExpr>(callee->IgnoreParenImpCasts());
  if (!ovl) {
    return nullptr;
  }

  const unsigned num_args = expr->getNumArgs();
  const clang::FunctionDecl *match = nullptr;
  unsigned num_matches = 0;
  for (const auto *d : ovl->decls()) {
    const auto *fn =
        llvm::dyn_cast<clang::FunctionDecl>(d->getUnderlyingDecl());
    if (!fn || fn->getNumParams() != num_args || fn->isVariadic()) {
      continue;
    }
    // An unqualified call to an INSTANCE method has an implicit `this`
    // receiver, a different emission shape; refuse rather than guess.
    if (const auto *method = llvm::dyn_cast<clang::CXXMethodDecl>(fn)) {
      if (method->isInstance()) {
        continue;
      }
    }
    bool ok = true;
    for (unsigned i = 0; i < num_args && ok; ++i) {
      ok = StripForOverloadMatch(fn->getParamDecl(i)->getType()) ==
           StripForOverloadMatch(expr->getArg(i)->getType());
    }
    if (ok) {
      ++num_matches;
      match = fn;
    }
  }
  // Exactly one exact match, or nothing. Ambiguity must stay loud.
  return num_matches == 1 ? match : nullptr;
}

void Converter::ReportUnresolvedCall(clang::CallExpr *expr,
                                     clang::Expr *callee) {
  std::string name = "<unknown callee>";
  std::string candidates;
  if (const auto *ovl = callee ? llvm::dyn_cast<clang::OverloadExpr>(
                                     callee->IgnoreParenImpCasts())
                               : nullptr) {
    name = ovl->getName().getAsString();
    for (const auto *d : ovl->decls()) {
      const auto *nd =
          llvm::dyn_cast<clang::NamedDecl>(d->getUnderlyingDecl());
      candidates += "\n    candidate: ";
      candidates += nd ? Mapper::ToString(nd) : std::string("<non-named decl>");
      if (const auto *fn =
              llvm::dyn_cast_or_null<clang::FunctionDecl>(
                  nd ? nd->getUnderlyingDecl() : nullptr)) {
        candidates +=
            " declared at " +
            fn->getLocation().printToString(ctx_.getSourceManager());
      }
    }
  } else if (callee) {
    name = Mapper::ToString(callee->getType(), Mapper::ScalarSugar::kPreserve);
  }

  std::string args;
  for (unsigned i = 0; i < expr->getNumArgs(); ++i) {
    if (i) {
      args += ", ";
    }
    const auto *arg = expr->getArg(i);
    args += arg ? Mapper::ToString(arg->getType(),
                                   Mapper::ScalarSugar::kPreserve)
                : "<null>";
  }

  const std::string loc =
      expr->getExprLoc().printToString(ctx_.getSourceManager());

  // A TYPE-DEPENDENT call is not an overload problem and must not be reported as
  // one: there are no argument types to match, so overload resolution is
  // impossible in principle. It means the converter is walking an
  // UNINSTANTIATED TEMPLATE PATTERN -- in practice the body of a GENERIC LAMBDA
  // (`[](const auto& entry) { ... isa<Key>(entry.first) ... }`, KTDFArch
  // Attributes.h:126-129), where `entry.first` is `<dependent type>` and the
  // only candidate is a function TEMPLATE, never a FunctionDecl. Name the
  // construct rather than dying on a null callee.
  bool dependent = expr->isTypeDependent() || expr->isValueDependent() ||
                   (callee != nullptr && (callee->isTypeDependent() ||
                                          callee->isValueDependent()));
  for (unsigned i = 0; !dependent && i < expr->getNumArgs(); ++i) {
    const auto *arg = expr->getArg(i);
    dependent = arg != nullptr &&
                (arg->isTypeDependent() || arg->isValueDependent());
  }
  if (dependent) {
    const auto *method =
        llvm::dyn_cast_or_null<clang::CXXMethodDecl>(curr_function_);
    const bool in_generic_lambda =
        method != nullptr && method->getParent() != nullptr &&
        method->getParent()->isGenericLambda();
    std::string dep_detail =
        std::string("type-dependent call in an uninstantiated template "
                    "pattern") +
        (in_generic_lambda ? " (inside a GENERIC LAMBDA pattern)" : "") +
        ": callee `" + name + "`, supplied argument types: (" + args +
        "); the converter walks the pattern, so the argument types are"
        " dependent and the candidates are function templates -- overload"
        " resolution is impossible here, only the INSTANTIATION can be"
        " converted" + candidates;
    if (survey::Enabled()) {
      survey::Record(survey::GapKind::kUnsupportedConstruct, dep_detail, loc);
      return;
    }
    llvm::errs() << "unsupported " << dep_detail << " at " << loc << '\n';
    assert(0 && "type-dependent call in an uninstantiated template pattern"
                " (generic lambda pattern)\n");
  }

  std::string detail = "unresolved call: callee `" + name +
                       "` has neither a function decl nor a prototype"
                       " (callee type: " +
                       (callee ? Mapper::ToString(
                                     callee->getType(),
                                     Mapper::ScalarSugar::kPreserve)
                               : std::string("<null>")) +
                       "); supplied argument types: (" + args + ")" +
                       candidates;

  if (survey::Enabled()) {
    survey::Record(survey::GapKind::kUnsupportedConstruct, detail, loc);
    return;
  }

  llvm::errs() << "unsupported " << detail << " at " << loc << '\n';
  assert(0 && "call with neither function decl nor function prototype\n");
}

Converter::CallInfo Converter::CollectCallInfo(clang::CallExpr *expr) {
  using Kind = CallArg::Kind;

  CallInfo info;
  info.expr = expr;
  auto callee = GetCallee(expr);
  unsigned arg_begin = 0;
  if (auto op_call = llvm::dyn_cast<clang::CXXOperatorCallExpr>(expr)) {
    if (clang::isa_and_nonnull<clang::CXXMethodDecl>(
            op_call->getDirectCallee())) {
      arg_begin = 1;
    }
  }

  auto decl = expr->getCalleeDecl();
  const auto *function = decl ? decl->getAsFunction() : nullptr;
  const clang::FunctionProtoType *proto = nullptr;
  if (!function) {
    auto callee_ty = callee->getType().getDesugaredType(ctx_);
    if (auto ptr_ty = callee_ty->getAs<clang::PointerType>()) {
      proto = ptr_ty->getPointeeType()->getAs<clang::FunctionProtoType>();
    }
  }
  if (!function && !proto) {
    // The callee may have survived instantiation as an unresolved overload set
    // (a recursive unqualified call to an overloaded static member inside a
    // class template, e.g. KTDFArchAttributes.h's `classof`). Resolve it here:
    // getCalleeDecl() is null, but the candidate set and the argument types are
    // both in hand.
    function = ResolveOverloadedCallee(expr);
    info.resolved_overload = function;
  }
  if (!function && !proto) {
    ReportUnresolvedCall(expr, callee);
    CallInfo bail{};
    bail.expr = expr;
    return bail;
  }

  unsigned num_args = expr->getNumArgs() - arg_begin;
  unsigned num_named_params =
      function ? function->getNumParams() : proto->getNumParams();
  info.is_variadic = function ? function->isVariadic() : proto->isVariadic();
  info.is_fn_ptr_call = !function;
  info.is_libc_passthrough = Mapper::IsLibcPassthrough(GetCalleeOrExpr(expr));

  for (unsigned i = 0; i < num_named_params && i < num_args; ++i) {
    auto *arg = expr->getArg(i + arg_begin);
    CallArg ca{
        .param_name =
            function && !function->getParamDecl(i)->getName().empty()
                ? ("_" + GetNamedDeclAsString(function->getParamDecl(i)))
                : ("_arg" + std::to_string(i)),
        .param_type = function ? function->getParamDecl(i)->getType()
                               : proto->getParamType(i),
        .expr = arg,
        .has_default =
            function && HasUsableDefaultArg(function->getParamDecl(i)),
        .kind = (IsLiteral(arg) || info.is_libc_passthrough) ? Kind::Inline
                                                             : Kind::Hoisted,
    };
    bool is_materialize = clang::isa<clang::MaterializeTemporaryExpr>(arg);
    if (is_materialize && ca.param_type->isReferenceType()) {
      ca.kind = Kind::Materialized;
    } else if (is_materialize) {
      ca.kind = Kind::Inline;
    }
    info.args.push_back(std::move(ca));
  }

  if (info.is_variadic) {
    for (unsigned i = num_named_params; i < num_args; ++i) {
      info.variadic_args.push_back(expr->getArg(i + arg_begin));
    }
  }

  // Inline arguments that don't alias
  clang::Expr *receiver = GetCallObject(expr);
  for (auto &ca : info.args) {
    if (ca.kind != Kind::Hoisted) {
      continue;
    }
    bool aliases = receiver && ArgsMayAlias(ca.expr, receiver);
    for (const auto &other : info.args) {
      if (&other != &ca && ArgsMayAlias(ca.expr, other.expr)) {
        aliases = true;
        break;
      }
    }
    if (!aliases) {
      ca.kind = Kind::Inline;
    }
  }

  return info;
}

void Converter::ConvertParamTy(clang::QualType param_type, clang::Expr *expr) {
  if (param_type->isReferenceType()) {
    PushExprKind push(*this, ExprKind::AddrOf);
    ConvertVarInit(param_type, expr);
  } else if (FunctionPointerCastNeedsTransmute() &&
             param_type->isFunctionPointerType() &&
             expr->getType()->isFunctionPointerType() &&
             !IsCastRedundantInRust(expr, param_type)) {
    ConvertFunctionPointerTransmute(expr, param_type);
    return;
  } else {
    ConvertVarInit(param_type, expr);
  }
  ConvertParamTyPointerCastIfNeeded(param_type, expr);
}

void Converter::ConvertFunctionPointerTransmute(clang::Expr *expr,
                                                clang::QualType type) {
  StrCat("std::mem::transmute::<");
  Convert(expr->getType());
  StrCat(',');
  Convert(type);
  StrCat(">(");
  Convert(expr);
  StrCat(')');
}

void Converter::ConvertParamTyPointerCastIfNeeded(clang::QualType param_type,
                                                  clang::Expr *expr) {
  if (!param_type->isPointerType() || !expr->getType()->isPointerType() ||
      IsVaListType(param_type) || IsVaListType(expr->getType())) {
    return;
  }
  switch (GetConstCastType(param_type->getPointeeType(),
                           expr->getType()->getPointeeType())) {
  case ConstCastType::MutableToConst:
    StrCat(".cast_const()");
    return;
  case ConstCastType::ConstToMutable:
    StrCat(".cast_mut()");
    return;
  default:
    break;
  }
  if (!IsCastRedundantInRust(expr, param_type)) {
    ConvertCast(param_type);
  }
}

void Converter::EmitHoistedArgs(CallInfo &info) {
  using Kind = CallArg::Kind;
  for (auto &ca : info.args) {
    switch (ca.kind) {
    case Kind::Hoisted:
      StrCat(
          std::format("let {}: {} =", ca.param_name, ToString(ca.param_type)));
      ConvertParamTy(ca.param_type, ca.expr);
      StrCat(";");
      break;
    case Kind::Materialized: {
      auto [binding, ref] =
          MaterializeTemp(ca.param_name, ca.param_type, ca.expr);
      StrCat(binding);
      ca.ref_temp_name = std::move(ref);
      break;
    }
    case Kind::Inline:
      break;
    }
  }
}

void Converter::EmitArgList(const CallInfo &info) {
  using Kind = CallArg::Kind;
  PushParen call_args(*this);

  if (!ufcs_receiver_.empty()) {
    StrCat(std::exchange(ufcs_receiver_, std::string()), token::kComma);
  }

  for (unsigned i = 0; i < info.args.size(); i++) {
    const auto &ca = info.args[i];

    if (ca.has_default && clang::isa<clang::CXXDefaultArgExpr>(ca.expr)) {
      StrCat("None", token::kComma);
      continue;
    }

    if (ca.has_default) {
      StrCat("Some");
    }

    {
      PushParen push(*this, ca.has_default);
      switch (ca.kind) {
      case Kind::Hoisted:
        StrCat(ca.param_name);
        break;
      case Kind::Materialized:
        StrCat(ca.ref_temp_name);
        break;
      case Kind::Inline:
        ConvertParamTy(ca.param_type, ca.expr);
        if (info.is_libc_passthrough) {
          StrCat(std::format(
              "as {}", Mapper::GetParamType(GetCalleeOrExpr(info.expr), i)));
        }
        break;
      }
    }

    StrCat(token::kComma);
  }

  if (info.is_variadic) {
    if (!info.is_libc_passthrough) {
      StrCat(token::kRef);
    }
    PushBracket push(*this, !info.is_libc_passthrough);
    for (auto *arg : info.variadic_args) {
      {
        PushParen p(*this);
        ConvertVariadicArg(arg);
      }
      if (!info.is_libc_passthrough) {
        StrCat(".into()");
      }
      StrCat(token::kComma);
    }
  }
}

void Converter::EmitCall(CallInfo &&info) {
  EmitHoistedArgs(info);

  if (info.resolved_overload) {
    // Callee is still an OverloadExpr in the AST, so converting it would reach
    // VisitUnresolvedLookupExpr. Spell the resolved decl instead.
    if (const auto *method =
            llvm::dyn_cast<clang::CXXMethodDecl>(info.resolved_overload)) {
      StrCat(GetUFCSName(method), token::kDoubleColon, GetMethodName(method));
    } else {
      StrCat(Mapper::MapFunctionName(info.resolved_overload));
    }
  } else if (info.is_fn_ptr_call) {
    EmitFnPtrCall(GetCallee(info.expr));
  } else if (info.is_libc_passthrough) {
    auto *direct_callee = info.expr->getDirectCallee();
    assert(direct_callee);
    StrCat("libc::", direct_callee->getName());
  } else {
    PushExprKind push(*this, ExprKind::Callee);
    Convert(GetCallee(info.expr));
  }

  EmitArgList(info);
}

void Converter::ConvertGenericCallExpr(clang::CallExpr *expr) {
  PushParen outer(*this);
  StrCat(keyword_unsafe_);
  PushBrace unsafe_brace(*this);
  EmitCall(CollectCallInfo(expr));
}

std::string Converter::GetUFCSName(const clang::CXXMethodDecl *method) const {
  return GetRecordName(method->getParent());
}

void Converter::ConvertUserOperatorCall(clang::CXXOperatorCallExpr *expr) {
  auto *callee = expr->getDirectCallee();
  PushParen outer(*this);
  StrCat(keyword_unsafe_);
  PushBrace unsafe_brace(*this);
  auto info = CollectCallInfo(expr);
  EmitHoistedArgs(info);
  if (auto *method = clang::dyn_cast<clang::CXXMethodDecl>(callee)) {
    if (method->isInstance()) {
      SetUFCSReceiver(expr->getArg(0), false, method);
    }
    StrCat(GetUFCSName(method), token::kDoubleColon, GetMethodName(method));
  } else {
    StrCat(GetNamedDeclAsString(callee->getCanonicalDecl()));
  }
  EmitArgList(info);
}

std::optional<Converter::TempMaterializationCtx>
Converter::ConvertCallExpr(clang::CallExpr *expr) {
  auto *callee = expr->getCallee();

  if (auto fn = Mapper::ToString(callee);
      fn.starts_with("int printf") || fn.starts_with("int fprintf")) {
    ConvertPrintf(expr);
  } else if (IsTransparentStdCall(expr)) {
    Convert(expr->getArg(0));
  } else if (IsBuiltinConstantP(callee)) {
    StrCat(expr->getArg(0)->isCXX11ConstantExpr(ctx_) ? token::kOne
                                                      : token::kZero);
  } else if (Mapper::Contains(callee)) {
    auto **args = expr->getArgs();
    auto num_args = expr->getNumArgs();
    auto ctx = CollectRefBindingTempArgs(expr);
    StrCat(GetMappedAsString(expr, args, num_args, &ctx));
    return ctx;
  } else if (auto *opcall = clang::dyn_cast<clang::CXXOperatorCallExpr>(expr);
             opcall && IsUserOperatorCall(opcall)) {
    ConvertUserOperatorCall(opcall);
  } else if (auto *opcall = clang::dyn_cast<clang::CXXOperatorCallExpr>(expr)) {
    ConvertCXXOperatorCallExpr(opcall);
  } else {
    ConvertGenericCallExpr(expr);
  }
  return std::nullopt;
}

static std::string getTypedLiteral(const char *num, std::string_view type) {
  if (type.contains("::")) {
    // Not a builtin type
    return std::format("({} as {})", num, type);
  }
  return std::format("{}_{}", num, type);
}

std::string Converter::getIntegerLiteral(clang::IntegerLiteral *expr,
                                         bool incl_type,
                                         const clang::QualType *type) {
  auto num_as_string = GetNumAsString(expr->getValue());
  if (num_as_string[0] != '-' && !incl_type) {
    if (type && (*type)->isFloatingType() &&
        num_as_string.find('.') == llvm::StringRef::npos) {
      num_as_string += ".0";
    }
    return std::string(num_as_string);
  }

  auto ty = type ? *type : expr->getType();
  auto type_as_string = GetUnsafeTypeAsString(ty);

  if (ty->isFloatingType() || incl_type) {
    if (expr->getValue().isZero()) {
      if (auto init = Mapper::MapInitializer(ty); !init.empty()) {
        return init;
      }
    }
    return getTypedLiteral(num_as_string.c_str(), type_as_string);
  }

  return static_cast<std::string>(num_as_string);
}

bool Converter::VisitIntegerLiteral(clang::IntegerLiteral *expr) {
  if (auto str = GetMappedAsString(expr); !str.empty()) {
    StrCat(str);
    computed_expr_type_ = ComputedExprType::FreshValue;
    return false;
  }
  StrCat(getIntegerLiteral(expr, Mapper::Map(expr->getType()) != "i32"));
  computed_expr_type_ = ComputedExprType::FreshValue;
  return false;
}

bool Converter::VisitFloatingLiteral(clang::FloatingLiteral *expr) {
  StrCat(GetNumAsString(expr->getValue()));
  computed_expr_type_ = ComputedExprType::FreshValue;
  return false;
}

bool Converter::VisitCharacterLiteral(clang::CharacterLiteral *expr) {
  if (expr->getKind() != clang::CharacterLiteralKind::Ascii) {
    PushParen paren(*this);
    StrCat(std::to_string(expr->getValue()), keyword::kAs,
           ToStringBase(expr->getType()));
    computed_expr_type_ = ComputedExprType::FreshValue;
    return false;
  }
  auto uc = static_cast<unsigned char>(expr->getValue());
  std::string ch = GetEscapedCharLiteral(expr->getValue());
  ch = (uc > 0x7F ? "b'" : "'") + std::move(ch) + '\'';
  {
    PushParen paren(*this);
    StrCat(ch, keyword::kAs, ToStringBase(expr->getType()));
  }
  computed_expr_type_ = ComputedExprType::FreshValue;
  return false;
}

std::string Converter::GetEscapedUTF8CharLiteral(clang::Expr *expr) const {
  auto char_expr =
      clang::dyn_cast<clang::CharacterLiteral>(expr->IgnoreCasts());
  if (!char_expr) {
    return {};
  }
  std::string ch = GetEscapedCharLiteral(char_expr->getValue());
  auto start = reinterpret_cast<const llvm::UTF8 *>(ch.data());
  auto end = reinterpret_cast<const llvm::UTF8 *>(start + ch.size());
  return llvm::isLegalUTF8String(&start, end) ? std::move(ch) : "";
}

std::string Converter::GetEscapedStringLiteral(clang::Expr *expr,
                                               uint64_t pad_nulls) const {
  auto str_expr = clang::dyn_cast<clang::StringLiteral>(expr->IgnoreCasts());
  assert(str_expr);
  auto raw = str_expr->getString();
  std::string out;
  out.push_back('"');
  for (unsigned char c : raw) {
    out += GetEscapedCharLiteral(static_cast<char>(c));
  }
  for (uint64_t i = 0; i < pad_nulls; ++i) {
    out += "\\0";
  }
  out.push_back('"');
  return out;
}

bool Converter::IsArrayInitContext() const {
  return !curr_init_type_.empty() && curr_init_type_.back()->isArrayType();
}

std::string
Converter::GetCodeUnitArrayLiteral(const clang::StringLiteral *expr) {
  auto elem_type =
      ToStringBase(ctx_.getAsArrayType(expr->getType())->getElementType());
  uint64_t len = expr->getLength();
  uint64_t total = len + 1;
  if (IsArrayInitContext()) {
    if (auto *arr_ty = ctx_.getAsConstantArrayType(curr_init_type_.back())) {
      total = std::max(arr_ty->getSize().getZExtValue(), len);
    }
  }
  std::string out = "[";
  for (uint64_t i = 0; i < total; ++i) {
    out += std::format("{} as {}, ", i < len ? expr->getCodeUnit(i) : 0,
                       elem_type);
  }
  out += ']';
  return out;
}

bool Converter::VisitStringLiteral(clang::StringLiteral *expr) {
  if (IsCodeUnitStringLiteral(expr)) {
    StrCat(GetCodeUnitArrayLiteral(expr));
    computed_expr_type_ = ComputedExprType::FreshValue;
    return false;
  }

  auto init_type = curr_init_type_.empty()
                       ? clang::QualType()
                       : curr_init_type_.back().getNonReferenceType();
  if (!init_type.isNull() && init_type->isArrayType()) {
    if (auto *arr_ty = ctx_.getAsConstantArrayType(init_type)) {
      uint64_t arr_size = arr_ty->getSize().getZExtValue();
      if (expr->getString().empty()) {
        StrCat(std::format("[0 as libc::c_char; {}]", arr_size));
        computed_expr_type_ = ComputedExprType::FreshValue;
        return false;
      }
      uint64_t pad = arr_size > expr->getString().size()
                         ? arr_size - expr->getString().size()
                         : 0;
      StrCat(std::format("std::mem::transmute(*b{})",
                         GetEscapedStringLiteral(expr, pad)));
      computed_expr_type_ = ComputedExprType::FreshValue;
      return false;
    }
    StrCat(std::format("std::mem::transmute(*b{})",
                       GetEscapedStringLiteral(expr, 1)));
    computed_expr_type_ = ComputedExprType::FreshValue;
    return false;
  }
  if (expr->getString().contains('\0')) {
    std::string out = "(&[";
    for (unsigned char c : expr->getString()) {
      out += getTypedLiteral(std::to_string(c).c_str(), CharRustType()) + ", ";
    }
    out += getTypedLiteral("0", CharRustType()) + "])";
    StrCat(out);
    computed_expr_type_ = ComputedExprType::FreshValue;
    // `(&[...])` IS a Rust reference. See emitted_a_reference_.
    emitted_a_reference_ = true;
    return false;
  }
  StrCat(std::format("c{}", GetEscapedStringLiteral(expr, 0)));
  computed_expr_type_ = ComputedExprType::FreshValue;
  // `c"x"` has type `&'static CStr` -- it is ALREADY a reference, so a rule
  // parameter declared `&std::ffi::CStr` is satisfied by it verbatim and adding
  // a borrow would give `&&CStr`. This is the case that made the shared-`&` half
  // undecidable from the rule IR.
  emitted_a_reference_ = true;
  return false;
}

bool Converter::VisitCXXBoolLiteralExpr(clang::CXXBoolLiteralExpr *expr) {
  StrCat(expr->getValue() ? keyword::kTrue : keyword::kFalse);
  computed_expr_type_ = ComputedExprType::FreshValue;
  return false;
}

void Converter::ConvertIntegerToEnumeralCast(clang::Expr *to,
                                             clang::Expr *from) {
  // Short circuit `(X as i32) as Enum` to `X`
  if (auto ref =
          clang::dyn_cast<clang::DeclRefExpr>(from->IgnoreParenImpCasts())) {
    if (auto ec = clang::dyn_cast<clang::EnumConstantDecl>(ref->getDecl())) {
      auto src_enum = clang::dyn_cast<clang::EnumDecl>(ec->getDeclContext());
      auto dst_enum = to->getType()->getAs<clang::EnumType>();
      if (src_enum && dst_enum && dst_enum->getDecl() == src_enum) {
        StrCat(EnumeratorName(ec));
        computed_expr_type_ = ComputedExprType::FreshValue;
        return;
      }
    }
  }
  PushParen paren(*this);
  {
    PushParen inner(*this);
    Convert(from);
  }
  StrCat(keyword::kAs, GetUnsafeTypeAsString(to->getType()));
  computed_expr_type_ = ComputedExprType::FreshValue;
}

void Converter::ConvertIntegralToBooleanCast(clang::ImplicitCastExpr *expr) {
  auto sub_expr = expr->getSubExpr();
  auto *stripped = sub_expr->IgnoreParenImpCasts();

  if (auto binop = clang::dyn_cast<clang::BinaryOperator>(stripped)) {
    // Comparisons and logical ops already produces bool, no wrap needed.
    if ((binop->isComparisonOp() || binop->isLogicalOp()) &&
        binop->getType()->isBooleanType()) {
      Convert(sub_expr);
      return;
    }
  }

  PushParen paren(*this);
  Convert(sub_expr);
  StrCat(token::kDiff);
  StrCat(token::kZero);
  computed_expr_type_ = ComputedExprType::FreshValue;
}

bool Converter::IsCastRedundantInRust(clang::Expr *expr,
                                      clang::QualType target_type) {
  auto target = GetUnsafeTypeAsString(target_type);
  if (const auto *rule = Mapper::GetExprRule(expr)) {
    return rule->return_type.type == target;
  }
  return GetUnsafeTypeAsString(expr->getType()) == target;
}

bool Converter::VisitImplicitCastExpr(clang::ImplicitCastExpr *expr) {
  auto *sub_expr = expr->getSubExpr();
  auto type = expr->getType();
  switch (expr->getCastKind()) {
  case clang::CastKind::CK_LValueToRValue: {
    PushExprKind push(*this, ExprKind::RValue);
    Convert(sub_expr);
    SetValueFreshness(type);
    break;
  }
  case clang::CastKind::CK_ArrayToPointerDecay: {
    // __va_list_tag [1] decays to __va_list_tag *. Just pass through by value
    if (IsVaListType(sub_expr->getType())) {
      Convert(sub_expr);
      break;
    }
    bool dest_pointee_const =
        expr->getType()->getPointeeType().isConstQualified();
    {
      PushExprKind push(*this, ExprKind::LValue);
      Convert(sub_expr);
    }
    if (IsStringLiteralExpr(sub_expr)) {
      StrCat(".as_ptr()");
      if (!dest_pointee_const) {
        StrCat(".cast_mut()");
      }
    } else {
      StrCat(dest_pointee_const ? ".as_ptr()" : ".as_mut_ptr()");
    }
    computed_expr_type_ = ComputedExprType::FreshPointer;
    break;
  }
  case clang::CastKind::CK_BitCast: {
    PushParen paren(*this);
    Convert(sub_expr);
    if (type->isVoidPointerType()) {
      StrCat(keyword::kAs,
             type->getPointeeType().isConstQualified() ? "*const" : "*mut");
      StrCat(ConvertPointeeType(sub_expr->getType()));
    }
    ConvertCast(type);
    SetFreshType(type);
    break;
  }
  case clang::CastKind::CK_NoOp: {
    const char *suffix = nullptr;
    bool type_changed = false;
    if (expr->getType()->isPointerType() &&
        sub_expr->getType()->isPointerType() &&
        !IsRuleArrowResult(sub_expr)) {
      switch (GetConstCastType(expr->getType()->getPointeeType(),
                               sub_expr->getType()->getPointeeType())) {
      case ConstCastType::MutableToConst:
        suffix = ".cast_const()";
        break;
      case ConstCastType::ConstToMutable:
        suffix = ".cast_mut()";
        break;
      default:
        type_changed = !IsCastRedundantInRust(sub_expr, type);
        break;
      }
    }
    if (type_changed) {
      PushParen paren(*this);
      Convert(sub_expr);
      ConvertCast(type);
      SetFreshType(type);
    } else {
      {
        PushParen paren(*this, suffix);
        Convert(sub_expr);
      }
      if (suffix) {
        StrCat(suffix);
        SetFreshType(type);
      }
    }
    break;
  }
  case clang::CastKind::CK_FunctionToPointerDecay:
  case clang::CastKind::CK_BuiltinFnToFnPtr: {
    if (isCallee()) {
      Convert(sub_expr);
    } else {
      PushExprKind push(*this, ExprKind::AddrOf);
      Convert(sub_expr);
    }
    break;
  }
  case clang::CastKind::CK_ConstructorConversion:
  case clang::CastKind::CK_DerivedToBase:
    Convert(sub_expr);
    break;
  case clang::CastKind::CK_IntegralToBoolean:
    ConvertIntegralToBooleanCast(expr);
    break;
  case clang::CastKind::CK_PointerToBoolean:
    StrCat(token::kNot);
    ConvertEqualsNullPtr(sub_expr);
    break;
  case clang::CastKind::CK_NullToPointer:
    StrCat(GetDefaultAsString(type));
    computed_expr_type_ = ComputedExprType::FreshPointer;
    break;
  default:
    if (auto *literal = clang::dyn_cast<clang::IntegerLiteral>(sub_expr)) {
      auto type = expr->getType();
      StrCat(getIntegerLiteral(literal, true, &type));
      computed_expr_type_ = ComputedExprType::FreshValue;
      break;
    }
    // Skip cast if source and target map to the same Rust type.
    if (IsCastRedundantInRust(sub_expr, type)) {
      Convert(sub_expr);
      break;
    }
    // A POINTER cast whose operand is a MAP ITERATOR's `operator->` has no Rust
    // counterpart and must not be emitted.  `it->first` on a DenseMap/unordered_map
    // iterator carries an implicit pointer base cast, because `first` is declared in
    // the `std::pair` base of `DenseMapPair`, so the operand's static C++ type is
    // `DenseMapPair<K,V> *` and the target `std::pair<K,V> *`.  The iterator maps to a
    // libcc2rs iterator VALUE (`HashMapIter`/`MapIter`), not to a pointer, and the
    // fused `->first`/`->second` rule consumes that value, so emitting the cast gave
    // `((*it.borrow()) as *mut (Value<u32>, Value<u32>)).first()` -- measured on
    // probe/dmgate.cpp as E0605 `non-primitive cast: HashMapIter<..> as *mut (..)`
    // plus E0599 `no method named first found for raw pointer`.  Nothing on the rule
    // side can suppress it; the same shape covers rules/unordered_map's f36-f39.
    if (type->isPointerType()) {
      auto *arrow =
          clang::dyn_cast<clang::CXXOperatorCallExpr>(sub_expr->IgnoreImplicit());
      if (arrow &&
          arrow->getOperator() == clang::OverloadedOperatorKind::OO_Arrow &&
          GetStrongestIteratorCategory(arrow->getArg(0)->getType()) ==
              IteratorCategory::Bidirectional) {
        Convert(sub_expr);
        break;
      }
    }
    if (type->isEnumeralType() && !sub_expr->getType()->isEnumeralType()) {
      ConvertIntegerToEnumeralCast(expr, sub_expr);
      break;
    }
    {
      PushParen outer(*this);
      if (clang::isa<clang::BinaryOperator>(sub_expr)) {
        {
          PushParen inner(*this);
          Convert(sub_expr);
        }
        ConvertCast(type);
      } else {
        PushParen inner(*this);
        Convert(sub_expr);
        ConvertCast(type);
      }
    }
    SetFreshType(type);
  }
  return false;
}

bool Converter::VisitExplicitCastExpr(clang::ExplicitCastExpr *expr) {
  auto type = expr->getTypeAsWritten();
  auto *sub_expr = expr->getSubExpr();
  if (type->isVoidType()) {
    StrCat(token::kRef);
    PushParen paren(*this);
    PushExprKind push(*this, ExprKind::Void);
    Convert(expr->getSubExpr());
    return false;
  }
  // A cast to a reference type only rebinds the operand, it converts nothing
  if (type->isReferenceType()) {
    Convert(sub_expr);
    return false;
  }
  switch (expr->getStmtClass()) {
  case clang::Stmt::CXXFunctionalCastExprClass:
    if (type->isEnumeralType() && !sub_expr->getType()->isEnumeralType()) {
      ConvertIntegerToEnumeralCast(expr, sub_expr);
      return false;
    }
    Convert(sub_expr, type);
    return false;
  case clang::Stmt::CXXReinterpretCastExprClass:
  case clang::Stmt::CXXStaticCastExprClass:
  case clang::Stmt::CStyleCastExprClass:
    if (expr->getType() == sub_expr->getType()) {
      return Convert(sub_expr);
    }
    if (type->isFunctionPointerType() ||
        sub_expr->getType()->isFunctionPointerType()) {
      ConvertFunctionPointerTransmute(sub_expr, type);
      return false;
    }
    if (type->isEnumeralType() && !sub_expr->getType()->isEnumeralType()) {
      ConvertIntegerToEnumeralCast(expr, sub_expr);
      return false;
    }
    if (type->isBooleanType() && sub_expr->getType()->isIntegerType() &&
        !sub_expr->getType()->isBooleanType()) {
      PushParen paren(*this);
      Convert(sub_expr);
      StrCat(token::kDiff, token::kZero);
      return false;
    }
    {
      PushParen paren(*this);
      Convert(sub_expr);
      if (auto *unary_oper = clang::dyn_cast<clang::UnaryOperator>(sub_expr);
          unary_oper && unary_oper->getOpcode() == clang::UO_AddrOf &&
          (clang::isa<clang::ArraySubscriptExpr>(unary_oper->getSubExpr()) ||
           clang::isa<clang::CXXOperatorCallExpr>(unary_oper->getSubExpr()))) {
        ConvertCast(sub_expr->getType());
      }
      ConvertCast(type);
    }
    return false;
  default:
    Convert(sub_expr);
    return false;
  }
}

bool Converter::VisitCXXRewrittenBinaryOperator(
    clang::CXXRewrittenBinaryOperator *expr) {
  Convert(expr->getSemanticForm());
  return false;
}

bool Converter::VisitBinaryOperator(clang::BinaryOperator *expr) {
  // Pointer-to-member dereference. There is NO pointer-to-member support
  // anywhere in the converter (`PtrMemD`/`PtrMemI`/`MemberPointer` appear in no
  // other file), so these two opcodes fell through to the generic
  // binary-operator path, where `getOpcodeStr()` printed the C++ spelling
  // VERBATIM: KTDFArchOpInterfaces.cpp.rs:114 contains a literal `->*`, which
  // is not Rust and cannot be. That is silent invalid output on a TU that
  // otherwise measured as a clean translate, so it is worth the trade of
  // turning that TU into a NAMED ABORT instead -- a refusal that says what is
  // missing beats a `.rs` that looks converted and cannot parse.
  if (expr->getOpcode() == clang::BO_PtrMemD ||
      expr->getOpcode() == clang::BO_PtrMemI) {
    const std::string loc =
        expr->getBeginLoc().printToString(ctx_.getSourceManager());
    std::string detail =
        std::string("pointer-to-member dereference `") +
        std::string(expr->getOpcodeStr()) + "` with member pointer of type `" +
        Mapper::ToString(expr->getRHS()->getType()) +
        "` has no Rust model (a C++ member pointer is an offset/vtable index, "
        "not a Rust value)";
    if (curr_function_ != nullptr) {
      detail += ", reached while converting `" +
                curr_function_->getQualifiedNameAsString() + "`";
    }
    if (survey::Enabled()) {
      survey::Record(survey::GapKind::kUnsupportedExpr, detail, loc);
      StrCat("Cpp2RustUnmappedExpr_PointerToMember");
      computed_expr_type_ = ComputedExprType::FreshValue;
      return false;
    }
    llvm::report_fatal_error(llvm::Twine("unsupported ") + detail + " at " +
                                 loc,
                             /*gen_crash_diag=*/false);
  }
  if (expr->getOpcode() == clang::BO_Cmp) {
    StrCat(std::format("std::cmp::Ord::cmp(&({}), &({}))",
                       ConvertRValue(expr->getLHS()),
                       ConvertRValue(expr->getRHS())));
    computed_expr_type_ = ComputedExprType::FreshValue;
    return false;
  }
  bool needs_cast = (expr->isComparisonOp() || expr->isLogicalOp()) &&
                    expr->getType()->isIntegerType() &&
                    !expr->getType()->isBooleanType();
  PushParen outer(*this, needs_cast);
  {
    PushParen inner(*this, needs_cast);
    ConvertBinaryOperator(expr);
  }
  if (needs_cast) {
    ConvertCast(expr->getType());
  }
  return false;
}

void Converter::ConvertBinaryOperator(clang::BinaryOperator *expr) {
  auto type = expr->getType();
  auto *lhs = expr->getLHS();
  auto *rhs = expr->getRHS();
  auto lhs_type = lhs->getType();
  auto rhs_type = rhs->getType();
  std::string_view opcode_as_string = expr->getOpcodeStr();

  if (auto *cmpd_assign_op =
          llvm::dyn_cast<clang::CompoundAssignOperator>(expr);
      expr->isCompoundAssignmentOp() &&
      GetUnsafeTypeAsString(lhs_type) !=
          GetUnsafeTypeAsString(cmpd_assign_op->getComputationResultType())) {
    auto computation_result_type = cmpd_assign_op->getComputationResultType();
    if (IsUnsignedArithOp(cmpd_assign_op)) {
      Convert(lhs);
      StrCat(token::kAssign);
      PushParen outer(*this);
      {
        PushParen inner(*this);
        Convert(lhs);
        ConvertCast(computation_result_type);
      }
      ConvertUnsignedArithBinaryOperator(expr, rhs);
    } else {
      Convert(lhs);
      StrCat(token::kAssign);
      PushParen outer(*this);
      {
        PushParen inner(*this);
        Convert(lhs);
        ConvertCast(computation_result_type);
      }
      auto op = opcode_as_string;
      op.remove_suffix(1); // remove '=' from operator
      StrCat(op);
      Convert(rhs, computation_result_type);
    }
    if (lhs_type->isBooleanType()) {
      StrCat(token::kDiff, token::kZero);
    } else {
      ConvertCast(lhs_type);
    }
  } else if (expr->isCommaOp()) {
    PushBrace brace(*this);
    {
      PushExprKind push(*this, ExprKind::Void);
      Convert(lhs);
    }
    StrCat(token::kSemiColon);
    Convert(rhs);
  } else if (IsUnsignedArithOp(expr)) {
    if (expr->isCompoundAssignmentOp()) {
      Convert(lhs);
      StrCat(token::kAssign);
    }
    {
      PushParen paren(*this);
      ConvertUnsignedArithOperand(lhs, type);
    }
    ConvertUnsignedArithBinaryOperator(expr, rhs);
    if (!expr->isCompoundAssignmentOp()) {
      computed_expr_type_ = ComputedExprType::FreshValue;
    }
  } else if (expr->isAssignmentOp()) {
    if (expr->isCompoundAssignmentOp() &&
        expr->getLHS()->getType()->isPointerType() &&
        expr->getRHS()->getType()->isIntegralOrEnumerationType()) {
      PushBrace brace(*this, !isVoid());
      Convert(lhs);
      StrCat(token::kAssign);
      {
        PushParen paren(*this);
        ConvertUnsignedArithOperand(lhs, type);
      }
      ConvertUnsignedArithBinaryOperator(expr, rhs);
      if (!isVoid()) {
        StrCat(token::kSemiColon, ConvertRValue(lhs));
      }
    } else {
      ConvertAssignment(lhs, rhs, opcode_as_string);
    }
  } else if (IsComparisonWithNullOp(expr)) {
    if (expr->getOpcode() == clang::BO_EQ) {
      ConvertEqualsNullPtr(lhs);
    } else {
      StrCat(token::kNot);
      PushParen paren(*this);
      ConvertEqualsNullPtr(lhs);
    }
  } else if (expr->isAdditiveOp() && expr->getType()->isPointerType()) {
    auto [base, idx] = lhs_type->isPointerType() ? std::make_tuple(lhs, rhs)
                                                 : std::make_tuple(rhs, lhs);
    ConvertPointerOffset(base, idx, expr->getOpcode() == clang::BO_Add);
  } else if (expr->isAdditiveOp() && lhs_type->isPointerType() &&
             rhs_type->isPointerType()) {
    {
      PushParen outer(*this);
      {
        PushParen inner(*this);
        Convert(lhs);
        StrCat(keyword::kAs, "usize", token::kMinus);
        Convert(rhs);
        StrCat(keyword::kAs, "usize");
      }
      StrCat(token::kDiv);
      auto pointee_type_as_string = ConvertPointeeType(lhs_type);
      auto size_of_as_string =
          std::format("::std::mem::size_of::<{}>()", pointee_type_as_string);
      StrCat(size_of_as_string);
    }
    ConvertCast(expr->getType());
    computed_expr_type_ = ComputedExprType::FreshValue;
  } else if (expr->isLogicalOp()) {
    {
      PushParen paren(*this);
      ConvertCondition(expr->getLHS());
    }
    StrCat(expr->getOpcodeStr());
    {
      PushParen paren(*this);
      ConvertCondition(expr->getRHS());
    }
    computed_expr_type_ = ComputedExprType::FreshValue;
  } else {
    ConvertGenericBinaryOperator(expr);
  }
}

void Converter::ConvertGenericBinaryOperator(clang::BinaryOperator *expr) {
  auto *lhs = expr->getLHS();
  auto *rhs = expr->getRHS();

  PushParen outer(*this);
  {
    PushParen lhs_paren(*this);
    Convert(lhs, GetOperandImplicitConversionTarget(expr, lhs, rhs));
  }

  StrCat(expr->getOpcodeStr());

  {
    PushParen rhs_paren(*this);
    Convert(rhs, GetOperandImplicitConversionTarget(expr, rhs, lhs));
  }
  computed_expr_type_ = ComputedExprType::FreshValue;
}

bool Converter::IsReferenceType(const clang::Expr *expr) const {
  const auto *e = IgnoreTransparentStdCall(expr->IgnoreCasts())->IgnoreCasts();
  if (const auto *call = clang::dyn_cast<clang::CallExpr>(e)) {
    return !clang::isa<clang::CXXOperatorCallExpr>(call) &&
           GetReturnTypeOfFunction(call)->isReferenceType();
  }
  if (const auto *decl_ref = clang::dyn_cast<clang::DeclRefExpr>(e)) {
    return decl_ref->getDecl()->getType()->isReferenceType();
  }
  if (const auto *member = clang::dyn_cast<clang::MemberExpr>(e)) {
    return member->getMemberDecl()->getType()->isReferenceType();
  }
  return false;
}

bool Converter::ConvertIncAndDec(clang::UnaryOperator *expr) {
  auto opcode = expr->getOpcode();
  auto *sub_expr = expr->getSubExpr();
  switch (opcode) {
  case clang::UO_PostInc: {
    PushExprKind push(*this, ExprKind::RValue);
    Convert(sub_expr);
    StrCat(".postfix_inc()");
    SetFresh();
    return true;
  }
  case clang::UO_PostDec: {
    PushExprKind push(*this, ExprKind::RValue);
    Convert(sub_expr);
    StrCat(".postfix_dec()");
    SetFresh();
    return true;
  }
  case clang::UO_PreInc: {
    PushExprKind push(*this, ExprKind::RValue);
    Convert(sub_expr);
    StrCat(".prefix_inc()");
    SetFresh();
    return true;
  }
  case clang::UO_PreDec: {
    PushExprKind push(*this, ExprKind::RValue);
    Convert(sub_expr);
    StrCat(".prefix_dec()");
    SetFresh();
    return true;
  }
  default:
    return false;
  }
}

bool Converter::VisitUnaryOperator(clang::UnaryOperator *expr) {
  if (auto str = GetMappedAsString(expr); !str.empty()) {
    StrCat(str);
    SetFreshType(expr->getType());
    return false;
  }

  auto opcode = expr->getOpcode();
  auto *sub_expr = expr->getSubExpr();
  if (ConvertIncAndDec(expr)) {
    return false;
  }
  switch (opcode) {
  case clang::UO_Extension:
  case clang::UO_Plus:
    Convert(sub_expr);
    break;
  case clang::UO_AddrOf: {
    PushParen paren(*this);
    ConvertAddrOf(sub_expr, expr->getType());
    break;
  }
  case clang::UO_Deref:
    ConvertDeref(sub_expr);
    break;
  case clang::UO_Not:
    StrCat(token::kNot);
    Convert(sub_expr);
    computed_expr_type_ = ComputedExprType::FreshValue;
    break;
  case clang::UO_LNot: {
    bool needs_int_cast =
        expr->getType()->isIntegerType() && !expr->getType()->isBooleanType();
    PushParen paren_cast(*this, needs_int_cast);
    StrCat(token::kNot);
    {
      PushParen paren_operand(*this);
      ConvertCondition(sub_expr);
    }
    if (needs_int_cast) {
      ConvertCast(expr->getType());
    }
    computed_expr_type_ = ComputedExprType::FreshValue;
    break;
  }
  case clang::UO_Minus:
    if (auto *literal = clang::dyn_cast<clang::IntegerLiteral>(sub_expr)) {
      if (sub_expr->getType()->isUnsignedIntegerType()) {
        StrCat(std::format("(-{}_i{} as {})", getIntegerLiteral(literal, false),
                           ctx_.getTypeSize(expr->getType()),
                           GetUnsafeTypeAsString(expr->getType())));
      } else {
        StrCat(token::kMinus, getIntegerLiteral(literal, true));
      }
      computed_expr_type_ = ComputedExprType::FreshValue;
      break;
    }
    [[fallthrough]];
  default:
    StrCat(expr->getOpcodeStr(opcode));
    Convert(sub_expr);
    SetFreshType(expr->getType());
  }
  return false;
}

bool Converter::VisitStmtExpr(clang::StmtExpr *expr) {
  auto *body = expr->getSubStmt();
  PushBrace brace(*this);
  auto stmts = body->body();
  size_t n = static_cast<size_t>(stmts.end() - stmts.begin());
  size_t i = 0;
  for (auto *s : stmts) {
    ++i;
    if (i == n) {
      if (auto *tail = clang::dyn_cast<clang::Expr>(s)) {
        EmitStmtExprTail(tail);
        continue;
      }
    }
    Convert(s);
  }
  return false;
}

void Converter::EmitStmtExprTail(clang::Expr *tail) { Convert(tail); }

bool Converter::VisitConditionalOperator(clang::ConditionalOperator *expr) {
  StrCat(keyword::kIf);
  ConvertCondition(expr->getCond());
  bool branch_is_addr =
      expr->isLValue() && !isRValue() && !expr->getType()->isFunctionType();
  bool branch_is_mut = curr_init_type_.empty() || IsMut(curr_init_type_.back());
  {
    PushBrace then_brace(*this);
    if (branch_is_addr) {
      StrCat(token::kRef, branch_is_mut ? keyword_mut_ : "");
    }
    PushExplicitAutoref no_autoref(*this, branch_is_addr ? std::nullopt
                                                         : autoref_mut_);
    Convert(expr->getTrueExpr(), branch_is_addr
                                     ? std::nullopt
                                     : std::make_optional(expr->getType()));
  }
  StrCat(keyword::kElse);
  {
    PushBrace else_brace(*this);
    if (branch_is_addr) {
      StrCat(token::kRef, branch_is_mut ? keyword_mut_ : "");
    }
    PushExplicitAutoref no_autoref(*this, branch_is_addr ? std::nullopt
                                                         : autoref_mut_);
    Convert(expr->getFalseExpr(), branch_is_addr
                                      ? std::nullopt
                                      : std::make_optional(expr->getType()));
  }
  return false;
}

std::string Converter::ConvertDeclRefExpr(clang::DeclRefExpr *expr) {
  if (isAddrOf()) {
    clang::Expr *addrof_op = ToAddrOf(ctx_, expr);
    if (auto str = GetMappedAsString(addrof_op); !str.empty()) {
      return str;
    }
  }

  auto *decl = expr->getDecl();
  if (ShouldReplaceWithMappedBody(expr)) {
    if (auto str = GetMappedAsString(expr); !str.empty()) {
      return str;
    }
  }

  if (auto *function = decl->getAsFunction()) {
    if (auto method = clang::dyn_cast<clang::CXXMethodDecl>(function)) {
      if (method->isStatic()) {
        return GetFunctionRefName(method);
      }
    }
    return GetNamedDeclAsString(function->getCanonicalDecl());
  }

  if (auto enum_constant = clang::dyn_cast<clang::EnumConstantDecl>(decl)) {
    auto name = EnumeratorName(enum_constant);
    if (!expr->getType()->isEnumeralType()) {
      return std::format("({} as i32)", name);
    }
    return name;
  }

  if (IsGlobalVar(expr)) {
    if (LazyStaticInit()) {
      return std::format("(*std::cell::LazyCell::force_mut(&mut *&raw mut {}))",
                         GetNamedDeclAsString(decl));
    }
    return GetNamedDeclAsString(decl);
  }

  return GetNamedDeclAsString(decl);
}

bool Converter::VisitDeclRefExpr(clang::DeclRefExpr *expr) {
  auto str = ConvertDeclRefExpr(expr);
  auto decl = expr->getDecl();

  // A binding we lowered to a raw pointer (`let k = it.first();`) is a
  // REFERENCE in C++, so every use derefs -- the same treatment a
  // reference-typed VarDecl gets below. The BindingDecl's own type is not a
  // reference type in the AST, so the test below cannot catch it.
  if (auto *binding = clang::dyn_cast<clang::BindingDecl>(decl)) {
    if (ptr_bindings_.contains(binding) && !isAddrOf()) {
      EmitDeref(std::move(str), binding->getType().getNonReferenceType());
      SetValueFreshness(expr->getType());
      return false;
    }
  }

  if (decl->getType()->getAs<clang::ReferenceType>() && !isAddrOf() &&
      !map_iter_decls_.contains(clang::dyn_cast<clang::VarDecl>(decl))) {
    EmitDeref(std::move(str), decl->getType().getNonReferenceType());
    SetValueFreshness(expr->getType());
    return false;
  }

  if (auto *fn_decl = clang::dyn_cast<clang::FunctionDecl>(decl)) {
    if (isAddrOf()) {
      ConvertFunctionToFunctionPointer(fn_decl);
      return false;
    }
    StrCat(str);
    SetFreshType(expr->getType());
    return false;
  }

  if (auto var_decl = clang::dyn_cast<clang::VarDecl>(decl)) {
    if (!var_decl->getType()->isFunctionPointerType()) {
      if (auto init = var_decl->getInit()) {
        if (auto lambda = clang::dyn_cast<clang::LambdaExpr>(
                init->IgnoreUnlessSpelledInSource())) {
          PushParen paren(*this);
          VisitLambdaExpr(lambda);
          computed_expr_type_ = ComputedExprType::FreshValue;
          return false;
        }
      }
    }
  }

  if (!decl->getType()->getAs<clang::ReferenceType>() && isAddrOf()) {
    StrCat(token::kRef, decl->getType().isConstQualified() ? "" : keyword_mut_,
           str);
    computed_expr_type_ = ComputedExprType::FreshPointer;
    return false;
  }

  StrCat(str);
  if (clang::isa<clang::EnumConstantDecl>(decl)) {
    computed_expr_type_ = ComputedExprType::FreshValue;
    return false;
  }
  SetValueFreshness(expr->getType());
  return false;
}

// An UnresolvedLookupExpr is a name clang could not bind -- typically an ADL
// call inside a class template whose template argument is still a
// SubstTemplateTypeParmType (e.g. `isa<Key>`). There is no lowering here and we
// deliberately do NOT try to resolve the lookup; without this hook the visitor
// traverses to nothing, emits no text, and the failure surfaces as the sentinel
// assert in Convert(Expr*, optional<QualType>) with no clue as to the construct.
bool Converter::VisitUnresolvedLookupExpr(clang::UnresolvedLookupExpr *expr) {
  std::string name = expr->getName().getAsString();

  std::string targs;
  if (expr->hasExplicitTemplateArgs()) {
    for (const auto &loc : expr->template_arguments()) {
      if (!targs.empty()) {
        targs += ", ";
      }
      const clang::TemplateArgument &arg = loc.getArgument();
      if (arg.getKind() == clang::TemplateArgument::Type) {
        targs += Mapper::ToString(arg.getAsType(),
                                  Mapper::ScalarSugar::kPreserve);
      } else {
        std::string buf;
        llvm::raw_string_ostream os(buf);
        arg.print(ctx_.getPrintingPolicy(), os, /*IncludeType=*/true);
        targs += buf;
      }
    }
  }

  std::string spelled = name;
  if (!targs.empty()) {
    spelled += "<" + targs + ">";
  }

  const std::string loc =
      expr->getExprLoc().printToString(ctx_.getSourceManager());

  if (survey::Enabled()) {
    survey::Record(survey::GapKind::kUnsupportedExpr,
                   std::string("UnresolvedLookupExpr: ") + spelled, loc);
    computed_expr_type_ = ComputedExprType::FreshValue;
    return false;
  }

  // FIXME: improve error handling
  llvm::errs() << "unsupported UnresolvedLookupExpr: " << spelled
               << " (unresolved ADL call, " << expr->getNumDecls()
               << " candidate decl(s)) at " << loc << '\n';
  assert(0 && "unsupported UnresolvedLookupExpr\n");
  return false;
}

bool Converter::VisitParenExpr(clang::ParenExpr *expr) {
  if (auto *bin = clang::dyn_cast<clang::BinaryOperator>(expr->getSubExpr());
      bin && (bin->isCommaOp() || (bin->isAssignmentOp() && isVoid()))) {
    Convert(expr->getSubExpr());
    return false;
  }

  {
    PushParen inner(*this);
    Convert(expr->getSubExpr());
  }

  return false;
}

// Report a SYSTEM record type that has no entry in the mapper's types_ table.
//
// Names the rule key someone has to add -- in the MAPPER's spelling, which is
// what a rule key must use, not clang's -- plus the outer type it was reached
// through and where the type is declared, so the message is actionable by
// itself. Precedent: ReportUnsupportedOperatorCall, whose printing of its exact
// rule key unblocked the three largest gates in this project.
//
// In --survey mode this RECORDS and KEEPS GOING: survey data feeds the whole
// work queue, and a survey run must enumerate every gap in one pass. Only the
// non-survey path asserts. The mangled spelling is still emitted under survey
// because survey output is never compiled, and emitting nothing would trip the
// `computed_expr_type_` sentinel (converter.cpp:1649) and hide the real gap.
void Converter::ReportUnsupportedStructuredBinding(
    const clang::DecompositionDecl *decl) {
  const std::string loc =
      decl->getLocation().printToString(ctx_.getSourceManager());

  std::string names;
  for (const auto *binding : decl->bindings()) {
    if (!names.empty()) {
      names += ", ";
    }
    names += binding->getNameAsString();
  }

  std::string detail =
      "structured binding / DecompositionDecl with " +
      std::to_string(decl->bindings().size()) + " bindings [" + names +
      "] of type `" + Mapper::ToString(decl->getType()) + "` is not implemented";
  if (curr_function_ != nullptr) {
    detail += ", reached while converting `" +
              curr_function_->getQualifiedNameAsString() + "`";
  }

  if (survey::Enabled()) {
    survey::Record(survey::GapKind::kUnsupportedConstruct, detail, loc);
    return;
  }
  // SILENT OMISSION under NDEBUG, which is the build everyone ships: the assert
  // compiled to nothing, this returned, VisitVarDecl (converter.cpp:678)
  // returned false having emitted NO TEXT for the declaration, and the process
  // exited 0 -- so the `.rs` simply lacked the variable while every later use of
  // its bindings referred to a name that was never declared. Same class as
  // ReportUnsupportedException (73cfd28); refuse loudly instead, naming the
  // construct, its bindings and the source location.
  // Same measured reason as ReportUnsupportedException above: the default
  // `gen_crash_diag=true` abort()s, which the SIGABRT handler turns into a
  // ~36-frame backtrace and rc=134, i.e. a refusal that reads as a crash.
  llvm::report_fatal_error(llvm::Twine("unsupported ") + detail + " at " + loc,
                           /*gen_crash_diag=*/false);
}

void Converter::ReportUnmappedSystemType(const clang::RecordDecl *decl) {
  const std::string key = Mapper::ToString(Mapper::GetTypeForDecl(decl));
  const std::string loc =
      decl->getLocation().printToString(ctx_.getSourceManager());
  const std::string mangled = GetRecordName(decl);

  std::string detail = "system type has no rule: `" + key +
                       "` (would be emitted as the undefined name `" + mangled +
                       "`)";
  if (curr_function_ != nullptr) {
    detail += ", reached while converting `" +
              curr_function_->getQualifiedNameAsString() + "`";
  }
  // Print the key from the SAME printer `Mapper::search(QualType)` uses, not one
  // rebuilt from the RecordDecl. `ToString(GetTypeForDecl(decl))` CANONICALISES
  // and does not elide defaulted template args, while `search()` looks up the
  // SUGARED spelling and only falls back to the canonical one IF THE TWO STRINGS
  // DIFFER -- so the old single `rule key:` instructed authors to write a key
  // that is never searched. Measured on three axes, each of which produced a
  // committed DEAD rule:
  //   search type std::__hash_impl<SenComponents>   vs  rule key: <..., void>
  //   search type ...DenseArrayAttrImpl<int64_t>    vs  rule key: <long>
  //   searched     llvm::SmallVector<long>          vs  recorded key <T1, _>
  // Both spellings are printed, clearly labelled; the FIRST is the key to write.
  detail += " rule key: " + Mapper::DescribeLastTypeSearch() +
            "; from decl (NOT a key -- canonicalised, defaulted args kept): " +
            key;

  if (survey::Enabled() || survey::MangleUnmapped()) {
    if (survey::Enabled()) {
      survey::Record(survey::GapKind::kUnmappedType, detail, loc);
    } else {
      // --mangle-unmapped: still SAY SO on stderr for every distinct type, so a
      // triage emission is never mistaken for a clean one.
      llvm::errs() << "MANGLED (triage): " << detail << " at " << loc << '\n';
    }
    StrCat(mangled);
    Mapper::AddRuleForUserDefinedType(const_cast<clang::RecordDecl *>(decl));
    return;
  }

  llvm::errs() << "unsupported " << detail << " at " << loc << '\n';
  assert(0 && "unsupported system type: no rule in types_");
}

// Report an overloaded-operator call the converter has no lowering for.
//
// This is deliberately a FUNCTION and not an inlined `default:` body: the
// OO_LessLess arm cannot fall through into `default:`, and before this existed
// a non-ostream ADL `operator<<` left the switch through a bare `break`, emitted
// no text at all, and then tripped the sentinel assert in
// Convert(Expr*, optional<QualType>) with no clue as to the construct.
//
// The message must be actionable BY ITSELF: the old text was
// `unsupported CXXOperatorCallExpr: ==`, which names neither the operand types
// nor the location, so a rule author could not tell which key to add. It now
// prints, on ONE line (census-head.sh takes a single grep match):
//   * the operator spelling,
//   * every operand's type in the MAPPER's own spelling -- that is the spelling
//     a rule key must use, not clang's,
//   * the resolved callee rendered by Mapper::ToString(NamedDecl*), which is
//     exactly the rule-key signature form the preprocessor records, and
//   * the source location.
// ⚠️ NOT A LOUD FAILURE. This comment used to claim "LOUD FAILURE, never a
// placeholder: outside --survey this still asserts", and that claim was FALSE:
// the `assert(0)` at the bottom is a NO-OP under the release build's -DNDEBUG,
// so outside --survey this function prints one stderr line and RETURNS; the
// OO_LessLess arm then `break`s and ConvertCXXOperatorCallExpr ends
// `return false`, having emitted no Rust and traversed no child. Measured: one
// non-survey run of ddl/Dialect/DdlOps.cpp printed 248 of these and exited 0.
// The same defect was found and fixed for ReportUnsupportedException (see the
// comment at the `report_fatal_error` there) but never here.
// It is now CONTAINED rather than fatal: in statement position the caller's
// guard in Convert(Stmt*) emits `Cpp2RustUnmappedStmt_...` and in value
// position Convert(Expr*, optional<QualType>) emits `Cpp2RustUnmappedExpr_...`,
// so the site is an undefined name and a guaranteed E0425 instead of a bare
// `;`. Promoting this to report_fatal_error is still the right end state (it is
// a NAMED refusal) but moves many TUs from translate-complete to abort, so it
// is deliberately left as a separate, measured change.
void Converter::ReportUnsupportedOperatorCall(
    clang::CXXOperatorCallExpr *expr) {
  const char *spelling = clang::getOperatorSpelling(expr->getOperator());

  std::string operands;
  for (unsigned i = 0; i < expr->getNumArgs(); ++i) {
    if (i) {
      operands += ", ";
    }
    const auto *arg = expr->getArg(i);
    operands += arg ? Mapper::ToString(arg->getType(),
                                       Mapper::ScalarSugar::kPreserve)
                    : "<null>";
  }

  std::string key;
  if (auto *callee = expr->getDirectCallee()) {
    key = Mapper::ToString(callee);
  }

  const std::string loc =
      expr->getExprLoc().printToString(ctx_.getSourceManager());

  if (survey::Enabled()) {
    std::string detail =
        std::string("CXXOperatorCallExpr: ") + spelling + " on (" + operands +
        ")";
    if (!key.empty()) {
      detail += " rule key: " + key;
    }
    survey::Record(survey::GapKind::kUnsupportedExpr, detail, loc);
    // Keep peeling: walk the operands so gaps *behind* this one are found
    // too, instead of hiding one layer of the onion per run.
    for (auto *arg : expr->arguments()) {
      Convert(arg);
    }
    computed_expr_type_ = ComputedExprType::FreshValue;
    return;
  }

  // FIXME: improve error handling
  llvm::errs() << "unsupported CXXOperatorCallExpr: " << spelling << " on ("
               << operands << ")"
               << (key.empty() ? std::string(" rule key: <unresolved callee>")
                               : " rule key: " + key)
               << " at " << loc << '\n';
  assert(0 && "unsupported CXXOperatorCallExpr\n");
}

bool Converter::ConvertCXXOperatorCallExpr(clang::CXXOperatorCallExpr *expr) {
  switch (expr->getOperator()) {
  case clang::OverloadedOperatorKind::OO_Equal:
    ConvertAssignment(expr->getArg(0), expr->getArg(1), "=");
    break;
  case clang::OverloadedOperatorKind::OO_Star:
  case clang::OverloadedOperatorKind::OO_Arrow:
    if (IsUniquePtr(expr->getArg(0)->getType())) {
      ConvertUniquePtrDeref(expr);
    } else if (GetStrongestIteratorCategory(expr->getArg(0)->getType()) ==
               IteratorCategory::Bidirectional) {
      Convert(expr->getArg(0));
    } else if (expr->getOperator() == clang::OverloadedOperatorKind::OO_Star) {
      PushParen paren(*this);
      StrCat(token::kStar);
      Convert(expr->getArg(0));
    } else {
      Convert(expr->getArg(0));
    }
    break;
  case clang::OverloadedOperatorKind::OO_Subscript: {
    PushExplicitAutoref autoref(*this, IsMutatingCall(expr));
    ConvertArraySubscript(expr->getArg(0), expr->getArg(1), expr->getType());
    break;
  }
  case clang::OverloadedOperatorKind::OO_LessLess:
    if (IsCallToOstream(expr)) {
      ConvertCallToOstream(expr);
      return false;
    }
    // A non-ostream `operator<<` used to leave the switch here having emitted
    // NOTHING, which surfaced later as the sentinel assert in
    // Convert(Expr*, optional<QualType>) rather than as a diagnostic naming the
    // construct. C++ cannot fall through to `default:` from here, so call the
    // same reporter that arm calls.
    ReportUnsupportedOperatorCall(expr);
    break;
  case clang::OverloadedOperatorKind::OO_Call:
    ConvertGenericCallExpr(expr);
    break;
  case clang::OverloadedOperatorKind::OO_Less:
    if (auto callee = expr->getDirectCallee()) {
      if (clang::isa<clang::CXXMethodDecl>(callee)) {
        Convert(expr->getArg(0));
        if (callee->isUserProvided()) {
          StrCat(token::kDot, GetOverloadedOperator(callee));
          PushParen paren(*this);
          StrCat(ConvertPointer(expr->getArg(1)));
        } else {
          StrCat(token::kLt);
          Convert(expr->getArg(1));
        }
      } else {
        StrCat(GetOverloadedOperator(callee));
        PushParen paren(*this);
        StrCat(ConvertFreshPointer(expr->getArg(0)), token::kComma,
               ConvertFreshPointer(expr->getArg(1)));
      }
    }
    computed_expr_type_ = ComputedExprType::FreshValue;
    break;
  default:
    ReportUnsupportedOperatorCall(expr);
    break;
  }
  return false;
}

bool Converter::VisitMemberExpr(clang::MemberExpr *expr) {
  auto *member = expr->getMemberDecl();
  if (auto *method = clang::dyn_cast<clang::CXXMethodDecl>(member);
      method && IsMethodOnPtr(method) && !Mapper::Contains(expr)) {
    SetUFCSReceiver(expr->getBase(), expr->isArrow(), method);
    StrCat(GetRecordName(method->getParent()), token::kDoubleColon,
           GetMethodName(method));
    SetFreshType(expr->getType());
    return false;
  }
  std::string str;
  {
    Buffer buf(*this);
    Converter::ConvertMemberExpr(expr);
    str = std::move(buf).str();
  }

  if (isAddrOf()) {
    bool is_reference_type = member->getType()->isReferenceType();
    if (auto *method = clang::dyn_cast<clang::CXXMethodDecl>(member)) {
      is_reference_type |= method->getReturnType()->isReferenceType();
    }

    if (is_reference_type) {
      computed_expr_type_ = ComputedExprType::Pointer;
    } else {
      StrCat(token::kRef);
      computed_expr_type_ = ComputedExprType::FreshPointer;
    }
    StrCat(str);
    return false;
  }

  if (!isAddrOf() && member->getType()->isReferenceType()) {
    EmitDeref(std::move(str), member->getType().getNonReferenceType());
    return false;
  }

  if (!isAddrOf() && member->getType()->isFunctionPointerType()) {
    PushParen paren(*this);
    StrCat(str);
    SetValueFreshness(expr->getType());
    return false;
  }

  StrCat(str);
  if (clang::isa<clang::CXXMethodDecl>(member)) {
    SetFreshType(expr->getType());
  } else {
    SetValueFreshness(expr->getType());
  }
  return false;
}

void Converter::SetUFCSReceiver(clang::Expr *base, bool is_arrow,
                                const clang::CXXMethodDecl *method) {
  if (clang::isa<clang::CXXThisExpr>(base->IgnoreParenImpCasts())) {
    bool in_ctor =
        curr_function_ && clang::isa<clang::CXXConstructorDecl>(curr_function_);
    ufcs_receiver_ = in_ctor ? "&mut this" : keyword::kSelfValue;
    return;
  }
  Buffer buf(*this);
  PushExprKind push(*this, ExprKind::LValue);
  auto object_type = is_arrow ? base->getType()->getPointeeType()
                              : base->getType().getNonReferenceType();
  bool cast_mut =
      MethodNeedsMutableReceiver(method) && object_type.isConstQualified();
  StrCat(MethodNeedsMutableReceiver(method) ? "&mut" : "&");
  if (cast_mut) {
    StrCat("*(&raw const");
  }
  if (is_arrow) {
    ConvertArrow(base);
  } else {
    Convert(base);
  }
  if (cast_mut) {
    StrCat(").cast_mut()");
  }
  ufcs_receiver_ = std::move(buf).str();
}

// Returns the inner member and the replacement string.
static std::pair<clang::MemberExpr *, std::string>
replaceNonUniformLibcField(clang::MemberExpr *expr) {
  // Example: ::struct stat::st_mtim::tv_sec -> ::libc::stat::st_mtime
  struct Mapping {
    const char *record;
    const char *inner_field;
    const char *leaf_field;
    const char *replacement;
  };
  static constexpr Mapping kFields[] = {
      {"stat", "st_mtim", "tv_sec", "st_mtime"},      // Linux
      {"stat", "st_mtimespec", "tv_sec", "st_mtime"}, // macOS
      {"in6_addr", "__in6_u", "__u6_addr8", "s6_addr"},
  };

  auto getNamedIdentifierOrNull = [](auto *decl) {
    return decl && decl->getDeclName().isIdentifier() ? decl : nullptr;
  };

  if (auto leaf = getNamedIdentifierOrNull(expr->getMemberDecl())) {
    if (auto inner = clang::dyn_cast<clang::MemberExpr>(
            expr->getBase()->IgnoreParenImpCasts())) {
      if (auto field = getNamedIdentifierOrNull(
              clang::dyn_cast<clang::FieldDecl>(inner->getMemberDecl()))) {
        if (getNamedIdentifierOrNull(field->getParent())) {
          for (const auto &m : kFields) {
            if (field->getParent()->getName() == m.record &&
                field->getName() == m.inner_field &&
                leaf->getName() == m.leaf_field) {
              return {inner, m.replacement};
            }
          }
        }
      }
    }
  }
  return {nullptr, ""};
}

// ⚠️ NOT A HARD REFUSAL, and its message must not imply one. The `assert(0)` at
// the bottom is a NO-OP under this release build's -DNDEBUG, so outside
// --survey this function prints one stderr line, `expr->dump()`s, and RETURNS;
// the caller (ConvertMemberExpr / VisitCXXThisExpr) then emits no text, and the
// empty-emission guard in Convert(Expr*, optional<QualType>) substitutes
// `Cpp2RustUnmappedExpr_<StmtClass>`. Third instance of the compiled-out-assert
// pattern, after ReportUnsupportedException (:1725-1733, now
// report_fatal_error) and ReportUnsupportedOperatorCall (:4640).
//
// ADJUDICATED 2026-09-28: KEEP THE PLACEHOLDER, do NOT promote this to
// report_fatal_error. `dsc/designSpaceConfig.h:358`'s one 240-entry NSDMI is
// 240 of the goal TU's 257 placeholders, the construct has no expressible Rust
// lowering, and per the guarantee argued at :145 nothing else emits the
// `Cpp2RustUnmapped` prefix -- so the name CANNOT silently resolve and is a
// guaranteed E0425 at the exact site, which meets the fail-loudly bar. This is
// NOT the silent-drop class fixed in 3ce6876a (that emitted nothing at all).
// Aborting would take a widely-included header's whole TU with it and hide
// every other defect behind one unmappable construct.
void Converter::ReportThisWithoutEnclosingFunction(const clang::Expr *expr,
                                                   const std::string &what) {
  const std::string loc =
      expr->getExprLoc().printToString(ctx_.getSourceManager());
  const std::string detail =
      what +
      " reached with NO enclosing function being converted (curr_function_ is "
      "null), i.e. a `this`-bearing expression outside any function body -- a "
      "non-static data member initializer (NSDMI) / in-class field "
      "initializer, or a default argument. Lowering `this` here requires "
      "knowing whether it becomes the constructor form (`this`) or the method "
      "form (`self`); the converter will not guess, because a wrong choice "
      "is silently-wrong output, not a compile error. NOT FATAL: this reporter "
      "RETURNS (see the comment above it), and the caller's empty-emission "
      "guard turns the site into an undefined "
      "`Cpp2RustUnmappedExpr_<StmtClass>`, i.e. a guaranteed E0425";
  if (survey::Enabled()) {
    survey::Record(survey::GapKind::kUnsupportedConstruct, detail, loc);
    return;
  }
  llvm::errs() << "unsupported " << detail << " at " << loc << '\n';
  expr->dump();
  assert(0 && "`this` expression converted with no enclosing function (NSDMI?)");
}

void Converter::ConvertMemberExpr(clang::MemberExpr *expr) {
  if (auto mapped = GetMappedAsString(expr); !mapped.empty()) {
    if (Mapper::ReturnsPointer(expr)) {
      StrCat(token::kStar, mapped);
    } else {
      StrCat(mapped);
    }
    return;
  }

  auto *member = expr->getMemberDecl();
  auto [inner, name_override] = replaceNonUniformLibcField(expr);
  if (inner) {
    expr = inner;
  }

  auto *base = expr->getBase();
  bool base_is_this =
      clang::isa<clang::CXXThisExpr>(base->IgnoreCasts()) && !ThisIsRustPtr();
  PushExprKind push(*this, isLValue() ? ExprKind::LValue : ExprKind::RValue);
  if (base_is_this) {
    if (curr_function_ == nullptr) {
      ReportThisWithoutEnclosingFunction(
          expr, std::string("member `") + GetNamedDeclAsString(member) +
                    "` accessed on `this`");
      return;
    }
    StrCat(clang::isa<clang::CXXConstructorDecl>(curr_function_)
               ? "this"
               : keyword::kSelfValue);
  } else if (expr->isArrow()) {
    ConvertArrow(base);
  } else {
    Convert(base);
  }

  if (auto *method = clang::dyn_cast<clang::CXXMethodDecl>(member);
      method && IsOverloadedMethod(method)) {
    StrCat(token::kDot);
    StrCat(GetMethodName(method));
  } else if (!name_override.empty()) {
    StrCat(token::kDot, name_override);
  } else if (member->getDeclName().isIdentifier()) {
    StrCat(token::kDot);
    StrCat(GetNamedDeclAsString(member));
  }
}

bool Converter::VisitCXXThisExpr(clang::CXXThisExpr *expr) {
  if (curr_function_ == nullptr) {
    ReportThisWithoutEnclosingFunction(expr, "`this` expression");
    computed_expr_type_ = ComputedExprType::FreshPointer;
    return false;
  }
  if (clang::isa<clang::CXXConstructorDecl>(curr_function_)) {
    StrCat("&raw mut this");
  } else {
    PushParen paren(*this);
    StrCat(keyword::kSelfValue, keyword::kAs, ToString(expr->getType()));
  }
  computed_expr_type_ = ComputedExprType::FreshPointer;
  return false;
}

bool Converter::VisitOpaqueValueExpr(clang::OpaqueValueExpr *expr) {
  Convert(expr->getSourceExpr());
  return false;
}

bool Converter::VisitArrayInitIndexExpr(clang::ArrayInitIndexExpr *expr) {
  StrCat("__i");
  computed_expr_type_ = ComputedExprType::FreshValue;
  return false;
}

bool Converter::VisitArrayInitLoopExpr(clang::ArrayInitLoopExpr *expr) {
  StrCat(std::format("std::array::from_fn::<_, {}, _>",
                     GetArraySize(expr->getType())));
  PushParen paren(*this);
  StrCat("|__i: usize|");
  ConvertVarInit(expr->getSubExpr()->getType(), expr->getSubExpr());
  computed_expr_type_ = ComputedExprType::FreshValue;
  return false;
}

bool Converter::VisitInitListExpr(clang::InitListExpr *expr) {
  auto *syntactic = expr->isSyntacticForm() ? expr : expr->getSyntacticForm();
  if (auto form = expr->getSemanticForm())
    expr = form;

  auto qual_type = expr->getType();
  if (qual_type->isScalarType()) {
    assert(expr->getNumInits() < 2 && "Excess elements in scalar initializer");
    if (expr->getNumInits() > 0) {
      auto init = expr->getInit(0);
      ConvertVarInit(init->getType(), init);
    } else {
      StrCat(GetDefaultAsString(qual_type));
    }
  } else if (qual_type->isRecordType()) {
    const auto *record = qual_type->getAsRecordDecl();
    if (record->getQualifiedNameAsString() == "std::array") {
      if (auto init = clang::dyn_cast<clang::InitListExpr>(expr->getInit(0))) {
        StrCat("vec!");
        VisitInitListExpr(init);
      } else {
        StrCat(GetArrayDefaultAsString(qual_type));
      }
      SetFreshType(qual_type);
      return false;
    }

    if (syntactic->getNumInits() == 0) {
      StrCat(GetDefaultAsString(qual_type));
      SetFreshType(qual_type);
      return false;
    }

    StrCat(GetUnsafeTypeAsString(qual_type));
    PushBrace brace(*this);
    int i = 0;
    for (const auto *field : record->fields()) {
      StrCat(GetNamedDeclAsString(field), token::kColon);
      ConvertVarInit(field->getType(), expr->getInit(i++));
      StrCat(token::kComma);
    }
  } else {
    if (IsInitExprOfStringLiteral(expr)) {
      Convert(expr->getInit(0)->IgnoreParenImpCasts());
      return false;
    }
    PushBracket bracket(*this);
    for (auto *init : expr->inits()) {
      ConvertVarInit(init->getType(), init);
      StrCat(token::kComma);
    }
    if (expr->hasArrayFiller()) {
      if (auto arr_ty = ctx_.getAsConstantArrayType(expr->getType())) {
        assert(
            (arr_ty->getSize().getZExtValue() - expr->getNumInits()) &&
            "Number of initializers should be less than total size of array");
        for (unsigned i = 0;
             i < arr_ty->getSize().getZExtValue() - expr->getNumInits(); ++i) {
          ConvertVarInit(expr->getArrayFiller()->getType(),
                         expr->getArrayFiller());
          StrCat(token::kComma);
        }
      }
    }
  }
  SetFreshType(qual_type);
  return false;
}

bool Converter::VisitCompoundLiteralExpr(clang::CompoundLiteralExpr *expr) {
  auto record = expr->getType()->getAsRecordDecl();
  if (!record || !record->hasAttr<clang::TransparentUnionAttr>()) {
    return true;
  }
  auto init = clang::cast<clang::InitListExpr>(expr->getInitializer());
  assert(init->getNumInits() == 1);
  PushExprKind push(*this, ExprKind::RValue);
  Convert(init->getInit(0));
  return false;
}

bool Converter::VisitArraySubscriptExpr(clang::ArraySubscriptExpr *expr) {
  auto *base = expr->getBase();
  if (base->IgnoreCasts()->getType()->isPointerType() ||
      clang::isa<clang::StringLiteral>(base->IgnoreCasts())) {
    ConvertPointerSubscript(expr);
  } else {
    ConvertArraySubscript(base, expr->getIdx(), expr->getType());
  }
  return false;
}

bool Converter::VisitCXXNullPtrLiteralExpr(clang::CXXNullPtrLiteralExpr *expr) {
  StrCat(token::kDefault);
  computed_expr_type_ = ComputedExprType::FreshPointer;
  return false;
}

bool Converter::VisitVAArgExpr(clang::VAArgExpr *expr) {
  auto va_list_expr = expr->getSubExpr();
  if (auto *cast = clang::dyn_cast<clang::ImplicitCastExpr>(va_list_expr)) {
    va_list_expr = cast->getSubExpr();
  }
  if (expr->getType()->isFunctionPointerType()) {
    StrCat("std::mem::transmute::<*mut ::libc::c_void", token::kComma);
    Convert(expr->getType());
    StrCat('>');
    PushParen paren(*this);
    {
      PushExprKind push(*this, ExprKind::RValue);
      Convert(va_list_expr);
    }
    StrCat(".arg::<*mut ::libc::c_void>()");
    SetFreshType(expr->getType());
    return false;
  }
  Convert(va_list_expr);
  StrCat(".arg::<");
  Convert(expr->getType());
  StrCat(">()");
  SetFreshType(expr->getType());
  return false;
}

bool Converter::VisitGNUNullExpr(clang::GNUNullExpr *expr) {
  StrCat(token::kDefault);
  computed_expr_type_ = ComputedExprType::FreshPointer;
  return false;
}

bool Converter::VisitCXXNewExpr(clang::CXXNewExpr *expr) {
  if (expr->isArray()) {
    if (auto *init = llvm::dyn_cast_or_null<clang::InitListExpr>(
            expr->getInitializer())) {
      StrCat("Box::leak(Box::new(");
      Convert(init);
      StrCat("))");
    } else {
      assert(expr->getArraySize().has_value());
      auto array_size_as_string = ToString(*expr->getArraySize());
      auto alloc_type_as_string = ToString(expr->getAllocatedType());
      auto default_alloc_type_as_string =
          GetDefaultAsString(expr->getAllocatedType());
      auto new_array_as_string =
          std::format("Box::leak((0..{}).map(|_| {}).collect::<Box<[{}]>>())",
                      array_size_as_string, default_alloc_type_as_string,
                      alloc_type_as_string);
      StrCat(new_array_as_string);
    }
    if (!curr_init_type_.empty() && curr_init_type_.back()->isPointerType()) {
      StrCat(".as_mut_ptr()");
    }
    SetFreshType(expr->getType());
  } else {
    auto initializer_as_string =
        expr->getInitializer() ? ToString(expr->getInitializer())
                               : GetDefaultAsString(expr->getAllocatedType());
    auto new_as_string =
        std::format("(Box::leak(Box::new({})) as {})", initializer_as_string,
                    ToString(expr->getType()));
    StrCat(new_as_string);
    SetFreshType(expr->getType());
  }
  return false;
}

bool Converter::VisitCXXDeleteExpr(clang::CXXDeleteExpr *expr) {
  auto *argument = expr->getArgument();
  auto destroyed_type = expr->getDestroyedType();
  if (!TypeNeedsDestruction(destroyed_type)) {
    EmitDeallocation(expr, ToString(argument));
    return false;
  }
  auto record_name = GetRecordName(destroyed_type->getAsCXXRecordDecl());
  PushBrace brace(*this);
  StrCat(keyword::kLet, "__p", token::kAssign, ToString(argument),
         token::kSemiColon);
  if (expr->isArrayForm()) {
    StrCat(std::format("for __i in 0..libcc2rs::malloc_usable_size(__p as *mut "
                       "::libc::c_void) / ::std::mem::size_of::<{0}>() {{ "
                       "{0}::{1}(&mut *__p.add(__i)); }}",
                       record_name, kDestructorName));
  } else {
    StrCat(std::format("{}::{}(&mut *__p)", record_name, kDestructorName),
           token::kSemiColon);
  }
  EmitDeallocation(expr, "__p");
  return false;
}

void Converter::EmitDeallocation(clang::CXXDeleteExpr *expr,
                                 const std::string &argument_as_string) {
  if (expr->isArrayForm()) {
    auto destroyed_type = expr->getDestroyedType();
    auto destroyed_type_as_string = ToString(destroyed_type);
    if (destroyed_type.isConstQualified()) {
      StrCat(std::format(
          R"(
        ::std::mem::drop(Box::from_raw(
          ::std::slice::from_raw_parts({},
            libcc2rs::malloc_usable_size({} as *mut ::libc::c_void) /
            ::std::mem::size_of::<{}>()) as *const [{}] as *mut [{}])))",
          argument_as_string, argument_as_string, destroyed_type_as_string,
          destroyed_type_as_string, destroyed_type_as_string));
    } else {
      StrCat(std::format(
          R"(
        ::std::mem::drop(Box::from_raw(
          ::std::slice::from_raw_parts_mut({},
            libcc2rs::malloc_usable_size({} as *mut ::libc::c_void) /
            ::std::mem::size_of::<{}>()))))",
          argument_as_string, argument_as_string, destroyed_type_as_string));
    }
  } else {
    StrCat(
        std::format("::std::mem::drop(Box::from_raw({}))", argument_as_string));
  }
}

void Converter::ConvertArrayCXXConstructExpr(clang::CXXConstructExpr *expr) {
  StrCat(std::format("std::array::from_fn::<_, {}, _>",
                     GetArraySize(expr->getType())));
  PushParen paren(*this);
  StrCat("|_|");
  ConvertCXXConstructExprArgs(expr);
}

void Converter::ConvertCXXConstructExprArgs(clang::CXXConstructExpr *expr) {
  HoistMaterializedTempBindings hoist_temps(*this, /*as_block=*/true);
  auto ctor = expr->getConstructor();
  StrCat(GetRecordName(ctor->getParent()), token::kDoubleColon,
         GetCtorName(ctor));
  PushParen paren(*this);

  unsigned arg_idx = 0;
  for (unsigned param_idx = 0; param_idx < ctor->getNumParams(); ++param_idx) {
    auto param = ctor->getParamDecl(param_idx);
    auto param_type = param->getType();
    bool has_default = HasUsableDefaultArg(param);

    if (arg_idx < expr->getNumArgs() &&
        clang::isa<clang::CXXDefaultArgExpr>(expr->getArg(arg_idx))) {
      assert(has_default);
      ++arg_idx;
      StrCat("None", token::kComma);
      continue;
    }

    if (arg_idx < expr->getNumArgs()) {
      clang::Expr *arg = expr->getArg(arg_idx++);
      PushBrace brace(*this);

      if (has_default) {
        StrCat("Some(");
        ConvertVarInit(param_type, arg);
        StrCat(')');
      } else {
        ConvertVarInit(param_type, arg);
      }
    } else {
      assert(has_default);
      StrCat("None");
    }
    StrCat(token::kComma);
  }
}

bool Converter::VisitCXXConstructExpr(clang::CXXConstructExpr *expr) {
  PushSuppressIteratorClone push(*this, expr);

  if (auto str = GetMappedAsString(expr, expr->getArgs(), expr->getNumArgs());
      !str.empty()) {
    StrCat(str);
    if (!IsPassThroughRule(expr)) {
      SetFreshType(expr->getType());
    }
    return false;
  }

  auto *ctor = expr->getConstructor();
  if (IsPassThroughConstructor(ctor)) {
    // Take suppress before recursing into the child.
    bool suppress = PushSuppressIteratorClone::take(*this);
    Convert(expr->getArg(0));
    if ((ctor->isCopyConstructor() || IsDefaultedMoveConstructor(ctor)) &&
        !suppress && !TypeIsCopyable(expr->getType())) {
      StrCat(".clone()");
      SetFreshType(expr->getType());
    }
    return false;
  }

  if (ctor->isDefaultConstructor() && !ctor->isUserProvided()) {
    auto ty = expr->getType();
    StrCat(GetDefaultAsString(ty));
    SetFreshType(expr->getType());
    return false;
  }

  if (expr->getType()->isArrayType()) {
    ConvertArrayCXXConstructExpr(expr);
  } else {
    ConvertCXXConstructExprArgs(expr);
  }
  SetFreshType(expr->getType());
  return false;
}

bool Converter::VisitUnaryExprOrTypeTraitExpr(
    clang::UnaryExprOrTypeTraitExpr *expr) {
  switch (expr->getKind()) {
  case clang::UnaryExprOrTypeTrait::UETT_SizeOf:
    StrCat(std::format(
        "::std::mem::size_of::<{}>()",
        GetUnsafeTypeAsString(expr->isArgumentType()
                                  ? expr->getArgumentType()
                                  : expr->getArgumentExpr()->getType())));
    computed_expr_type_ = ComputedExprType::FreshValue;
    break;
  case clang::UnaryExprOrTypeTrait::UETT_AlignOf:
  case clang::UnaryExprOrTypeTrait::UETT_PreferredAlignOf:
    StrCat(std::format(
        "::std::mem::align_of::<{}>()",
        GetUnsafeTypeAsString(expr->isArgumentType()
                                  ? expr->getArgumentType()
                                  : expr->getArgumentExpr()->getType())));
    computed_expr_type_ = ComputedExprType::FreshValue;
    break;
  default:
    // FIXME: improve error handling
    log() << "unsupported unary expr or type trait expr\n";
  }
  return false;
}

bool Converter::VisitConceptSpecializationExpr(
    clang::ConceptSpecializationExpr *expr) {
  assert(!expr->isValueDependent());
  StrCat(expr->isSatisfied() ? keyword::kTrue : keyword::kFalse);
  computed_expr_type_ = ComputedExprType::FreshValue;
  return false;
}

bool Converter::VisitRequiresExpr(clang::RequiresExpr *expr) {
  assert(!expr->isValueDependent());
  StrCat(expr->isSatisfied() ? keyword::kTrue : keyword::kFalse);
  computed_expr_type_ = ComputedExprType::FreshValue;
  return false;
}

bool Converter::VisitTypeTraitExpr(clang::TypeTraitExpr *expr) {
  clang::Expr::EvalResult result;
  ENSURE(expr->EvaluateAsInt(result, ctx_));
  StrCat(std::to_string(result.Val.getInt().getExtValue()));
  computed_expr_type_ = ComputedExprType::FreshValue;
  return false;
}

bool Converter::VisitSizeOfPackExpr(clang::SizeOfPackExpr *expr) {
  clang::Expr::EvalResult result;
  ENSURE(expr->EvaluateAsInt(result, ctx_));
  StrCat(std::to_string(result.Val.getInt().getExtValue()));
  computed_expr_type_ = ComputedExprType::FreshValue;
  return false;
}

bool Converter::VisitOffsetOfExpr(clang::OffsetOfExpr *expr) {
  std::string member_path;
  for (unsigned i = 0; i < expr->getNumComponents(); ++i) {
    const clang::OffsetOfNode &node = expr->getComponent(i);
    ENSURE(node.getKind() == clang::OffsetOfNode::Field);
    if (!member_path.empty()) {
      member_path += '.';
    }
    member_path += GetNamedDeclAsString(node.getField());
  }
  StrCat(
      std::format("::std::mem::offset_of!({}, {})",
                  GetUnsafeTypeAsString(expr->getTypeSourceInfo()->getType()),
                  member_path));
  computed_expr_type_ = ComputedExprType::FreshValue;
  return false;
}

bool Converter::VisitEnumDecl(clang::EnumDecl *decl) {
  ENSURE(decl_ids_.insert(GetID(decl)).second);
  if (Mapper::Contains(ctx_.getCanonicalTagType(decl))) {
    return false;
  }
  Mapper::AddRuleForUserDefinedType(decl);
  auto name = GetRecordName(decl);
  StrCat(std::format("pub type {} = {};", name,
                     GetUnsafeTypeAsString(decl->getIntegerType())));
  for (auto e : decl->enumerators()) {
    llvm::SmallVector<char, 32> init;
    e->getInitVal().toString(init, 10);
    StrCat(std::format("pub const {}: {} = {};", EnumeratorName(e), name,
                       std::string_view(init.data(), init.size())));
  }
  return false;
}

std::string
Converter::EnumeratorName(const clang::EnumConstantDecl *decl) const {
  auto *enum_decl = clang::cast<clang::EnumDecl>(decl->getDeclContext());
  return std::format("{}_{}", GetRecordName(enum_decl),
                     std::string_view(decl->getName()));
}

bool Converter::VisitCXXDefaultArgExpr(clang::CXXDefaultArgExpr *expr) {
  if (expr->getType()->isPointerType()) {
    StrCat(token::kDefault);
    computed_expr_type_ = ComputedExprType::FreshPointer;
    return false;
  }
  // A CXXDefaultArgExpr is a thin wrapper the parser inserts at a call site for
  // a parameter the caller omitted; getExpr() is the defaulted argument's own
  // expression, evaluated in the callee's context.  Lower it and adopt whatever
  // type that conversion computed -- without this, every non-pointer defaulted
  // argument emitted NOTHING and tripped the `computed_expr_type_ not set`
  // sentinel at the tail of Convert().  getExpr() is converted exactly once, so
  // a defaulted argument that is itself a call is not double-evaluated.
  clang::Expr *sub = expr->getExpr();
  if (!sub) {
    llvm::errs() << "CXXDefaultArgExpr with no default expression at "
                 << expr->getUsedLocation().printToString(ctx_.getSourceManager())
                 << "\n";
    assert(false && "CXXDefaultArgExpr has no sub-expression");
    return false;
  }
  Convert(sub);
  return false;
}

bool Converter::VisitConstantExpr(clang::ConstantExpr *expr) {
  Convert(expr->getSubExpr());
  SetFreshType(expr->getType());
  return false;
}

// For a GENERIC lambda (`[](auto x) {...}`), getLambdaCallOperator() returns the
// UNINSTANTIATED template pattern: every parameter type is `<dependent type>`
// and every call in the body is unresolved, so converting it can only produce
// garbage or an abort. The instantiated specialisations carry real types, so
// pick one of those instead.
clang::CXXMethodDecl *
Converter::SelectLambdaCallOperator(clang::LambdaExpr *expr) {
  auto *pattern = expr->getLambdaClass()->getLambdaCallOperator();
  if (!pattern->isTemplated()) {
    return pattern;
  }

  llvm::SmallVector<clang::CXXMethodDecl *, 4> instantiations;
  if (auto *tmpl = pattern->getDescribedFunctionTemplate()) {
    for (auto *spec : tmpl->specializations()) {
      if (auto *method = llvm::dyn_cast<clang::CXXMethodDecl>(spec)) {
        if (method->hasBody()) {
          instantiations.push_back(method);
        }
      }
    }
  }

  if (instantiations.size() == 1) {
    return instantiations.front();
  }

  auto loc = expr->getBeginLoc().printToString(ctx_.getSourceManager());
  std::string detail;
  if (instantiations.empty()) {
    detail = "generic lambda with no instantiated specialisation: its call"
             " operator is an uninstantiated template pattern, so parameter"
             " types are <dependent type> and calls in the body cannot be"
             " resolved";
  } else {
    detail = "generic lambda instantiated " +
             std::to_string(instantiations.size()) +
             " times: a Rust closure holds exactly one monomorphisation, so the"
             " remaining instantiations would be lost";
    for (auto *method : instantiations) {
      detail += "\n  candidate instantiation: ";
      for (auto p : method->parameters()) {
        detail += Mapper::ToString(p->getType()) + " ";
      }
    }
  }

  if (survey::Enabled()) {
    survey::Record(survey::GapKind::kUnsupportedConstruct, detail, loc);
    // Survey mode RECORDS and CONTINUES: fall back to the pattern (and, when
    // there is more than one instantiation, to the first one) so the rest of
    // the TU is still surveyed.
    return instantiations.empty() ? pattern : instantiations.front();
  }

  llvm::errs() << "unsupported " << detail << " at " << loc << '\n';
  assert(0 && "generic lambda whose call operator has no single instantiated"
              " specialisation\n");
  return instantiations.empty() ? pattern : instantiations.front();
}

bool Converter::VisitLambdaExpr(clang::LambdaExpr *expr) {
  if (isAddrOf() && expr->capture_size() == 0) {
    StrCat("Some");
  }
  auto *call_op = SelectLambdaCallOperator(expr);
  PushParen paren(*this);
  StrCat('|');
  for (auto p : call_op->parameters()) {
    StrCat(GetNamedDeclAsString(p), token::kColon, ToString(p->getType()),
           token::kComma);
  }
  StrCat("| {");
  EmitFunctionPreamble(call_op);
  PushCurrFunction push_fn(*this, call_op);
  ConvertFunctionBody(curr_function_);
  StrCat('}');
  return false;
}

bool Converter::VisitImplicitValueInitExpr(clang::ImplicitValueInitExpr *expr) {
  if (auto arr_ty = clang::dyn_cast<clang::ArrayType>(
          expr->getType()->getCanonicalTypeInternal().getTypePtr())) {
    if (auto const_arr_ty = clang::dyn_cast<clang::ConstantArrayType>(arr_ty)) {
      auto elem_ty = const_arr_ty->getElementType();
      if (elem_ty->isIntegerType() && !elem_ty->isEnumeralType()) {
        StrCat(std::format("[0; {}]", const_arr_ty->getSize().getZExtValue()));
        computed_expr_type_ = ComputedExprType::FreshValue;
        return false;
      }
      StrCat(
          std::format("std::array::from_fn::<_, {}, _>(|_| Default::default())",
                      const_arr_ty->getSize().getZExtValue()));
      computed_expr_type_ = ComputedExprType::FreshValue;
      return false;
    }
  }

  StrCat(GetDefaultAsString(expr->getType()));
  return false;
}

bool Converter::VisitCXXScalarValueInitExpr(
    clang::CXXScalarValueInitExpr *expr) {
  StrCat(GetDefaultAsString(expr->getType()));
  computed_expr_type_ = expr->getType()->isPointerType()
                            ? ComputedExprType::FreshPointer
                            : ComputedExprType::FreshValue;
  return false;
}

bool Converter::ConvertSwitchCaseCondition(clang::SwitchCase *stmt) {
  clang::Stmt *cur = stmt;
  clang::SwitchCase *last = nullptr;
  bool first = true;

  while (auto *sc = clang::dyn_cast<clang::SwitchCase>(cur)) {
    if (auto *case_stmt = clang::dyn_cast<clang::CaseStmt>(sc)) {
      if (!first) {
        StrCat("|| __v == ");
      }
      Convert(case_stmt->getLHS());
    }
    last = sc;
    first = false;
    cur = sc->getSubStmt();
  }

  if (clang::isa<clang::CaseStmt>(last)) {
    StrCat(" => ");
  } else /* DefaultStmt */ {
    StrCat("_ => ");
  }
  return false;
}

void Converter::EmitSwitchArm(const SwitchArm &arm, bool is_default) {
  if (is_default) {
    StrCat("_ => ");
  } else {
    StrCat("__v if __v == ");
    ConvertSwitchCaseCondition(arm.head);
  }
  if (!arm.label.empty()) {
    StrCat(std::format("'{}: ", arm.label.str()));
  }
  StrCat(token::kOpenCurlyBracket);
  for (auto *t : arm.body) {
    Convert(t);
  }
  StrCat("},");
}

bool Converter::VisitSwitchStmt(clang::SwitchStmt *stmt) {
  auto *body = clang::dyn_cast<clang::CompoundStmt>(stmt->getBody());
  assert(body);
  auto arms = AnalyzeSwitchArms(body);

  bool needs_switch_macro = std::ranges::any_of(arms, [](const SwitchArm &arm) {
    return !arm.label.empty() || arm.has_fallthrough;
  });

  PushBreakTarget push(break_target_, needs_switch_macro
                                          ? BreakTarget::FallthroughSwitch
                                          : BreakTarget::Switch);

  if (needs_switch_macro) {
    StrCat("switch!");
  } else {
    StrCat("'switch:");
  }

  PushParen switch_macro_paren(*this, needs_switch_macro);
  PushBrace switch_label_brace(*this, !needs_switch_macro);

  if (needs_switch_macro) {
    StrCat("match", ToString(stmt->getCond()));
  } else {
    StrCat(
        std::format("let __match_cond = {};", ConvertRValue(stmt->getCond())));
    StrCat("match __match_cond");
  }

  PushBrace match_brace(*this);

  const SwitchArm *default_arm = nullptr;
  for (const auto &arm : arms) {
    if (arm.is_default_case) {
      default_arm = &arm;
      continue;
    }
    EmitSwitchArm(arm, /*is_default=*/false);
  }

  if (default_arm) {
    EmitSwitchArm(*default_arm, /*is_default=*/true);
  } else {
    StrCat(R"( _ => {})");
  }

  return false;
}

// TODO: right now defaults go into the constructor, but they should also be
// placed in the Default trait impl.
bool Converter::VisitCXXDefaultInitExpr(clang::CXXDefaultInitExpr *expr) {
  Convert(expr->getExpr());
  return false;
}

bool Converter::VisitPredefinedExpr(clang::PredefinedExpr *expr) {
  Convert(expr->getFunctionName());
  return false;
}

bool Converter::VisitClassTemplateDecl(clang::ClassTemplateDecl *decl) {
  for (auto decl : decl->specializations()) {
    VisitCXXRecordDecl(decl);
  }
  return false;
}

bool Converter::VisitCXXStdInitializerListExpr(
    clang::CXXStdInitializerListExpr *expr) {
  if (expr->getSubExpr()->getType()->isArrayType()) {
    // Arrays become Vec's
    StrCat("vec!");
  }
  Convert(expr->getSubExpr());
  return false;
}

// True only when `qual_type` IS a `std::array<T, N>` specialisation -- not merely
// a type that MENTIONS one.
//
// GetArrayDefaultAsString used to test this with
// `Mapper::ToString(qual_type).contains("std::array")`, a SUBSTRING match on the
// printed type. Any type carrying a std::array in a template argument therefore
// entered the std::array branch and then asserted on the OUTER type's
// template-argument count. Measured: the field
//   std::map<uint64_t, std::array<uint32_t, 32>> progAddrAndFlitToCorrect;
// at dt_src/dbo/src/Utils/sdsc_bundle/ProgramCorrection.h:309 aborted four dbo
// TUs (InitBin.cpp, Pipeline/Pipeline.cpp, Pipeline/Driver.cpp,
// Transforms/CreateTrackers.cpp) at `template_args.size() == 2` with std::map's
// four arguments.
static bool IsStdArraySpecialization(clang::QualType qual_type) {
  const auto *record = qual_type->getAsRecordDecl();
  if (!record || record->getName() != "array") {
    return false;
  }
  const auto *ns =
      clang::dyn_cast<clang::NamespaceDecl>(record->getDeclContext());
  // libc++ declares std::array inside the inline namespace std::__1.
  while (ns && ns->isInline()) {
    ns = clang::dyn_cast<clang::NamespaceDecl>(ns->getDeclContext());
  }
  return ns && ns->getName() == "std" && ns->getDeclContext() &&
         ns->getDeclContext()->isTranslationUnit();
}

std::string Converter::GetArrayDefaultAsString(clang::QualType qual_type) {
  if (auto *array_type = clang::dyn_cast<clang::ConstantArrayType>(qual_type)) {
    auto size_as_string = GetNumAsString(array_type->getSize());
    auto element_type = array_type->getElementType();
    auto element_type_as_string = GetDefaultAsString(element_type);
    if (auto *rec = element_type->getAsRecordDecl()) {
      if (!RecordDerivesCopy(rec)) {
        return std::format("std::array::from_fn::<_, {}, _>(|_| {})",
                           size_as_string.c_str(), element_type_as_string);
      }
    }
    return std::format("[{}; {}]", element_type_as_string,
                       size_as_string.c_str());
  }
  if (auto *array_type =
          clang::dyn_cast<clang::IncompleteArrayType>(qual_type)) {
    return GetDefaultAsString(array_type->getElementType());
  }
  if (IsStdArraySpecialization(qual_type)) {
    auto maybe_template_args = GetTemplateArgs(qual_type);
    // A named refusal, not a bare assert: the old `template_args.size() == 2`
    // told a rule author nothing about which type or which arity it saw.
    if (!maybe_template_args || maybe_template_args->size() != 2) {
      llvm::errs() << "unsupported std::array default: `"
                   << Mapper::ToString(qual_type) << "` has "
                   << (maybe_template_args
                           ? std::to_string(maybe_template_args->size())
                           : std::string("no"))
                   << " template arguments, expected 2 (element type, extent)\n";
      if (survey::Enabled()) {
        survey::Record(survey::GapKind::kUnsupportedConstruct,
                       "std::array default arity", {});
        return {};
      }
      assert(0 && "unsupported std::array template-argument arity");
      return {};
    }
    auto template_args = *maybe_template_args;
    auto array_size = template_args[1];
    unsigned size = 0;
    switch (array_size.getKind()) {
    case clang::TemplateArgument::Expression: {
      auto array_size_expr = array_size.getAsExpr();
      assert(array_size_expr && !array_size_expr->isValueDependent());
      clang::Expr::EvalResult result;
      ENSURE(array_size_expr->EvaluateAsInt(result, ctx_));
      size = result.Val.getInt().getZExtValue();
      break;
    }
    case clang::TemplateArgument::Integral: {
      size = array_size.getAsIntegral().getZExtValue();
      break;
    }
    default:
      if (survey::Enabled()) {
        survey::Record(survey::GapKind::kUnsupportedConstruct,
                       "array size kind", {});
        break;
      }
      assert(0 && "Unsupported array size kind");
      break;
    }
    return std::format(
        "std::array::from_fn::<_, {}, _>(|_| Default::default()).to_vec()",
        size);
  }
  return {};
}

std::string Converter::GetDefaultAsString(clang::QualType qual_type) {
  if (qual_type->isVoidType()) {
    return "()";
  }

  if (IsVaListType(qual_type)) {
    computed_expr_type_ = ComputedExprType::FreshValue;
    return "VaList::default()";
  }

  if (auto arr = GetArrayDefaultAsString(qual_type); !arr.empty()) {
    computed_expr_type_ = ComputedExprType::FreshValue;
    return arr;
  }

  if (auto init = Mapper::MapInitializer(qual_type); !init.empty()) {
    computed_expr_type_ = ComputedExprType::FreshValue;
    return init;
  }

  if (qual_type->isPointerType()) {
    auto pointee = qual_type->getPointeeType();
    if (pointee->isFunctionType()) {
      return "None";
    }
    computed_expr_type_ = ComputedExprType::FreshPointer;
    return pointee.isConstQualified() ? "std::ptr::null()"
                                      : "std::ptr::null_mut()";
  }

  computed_expr_type_ = ComputedExprType::FreshValue;
  return GetDefaultAsStringFallback(qual_type);
}

std::string Converter::GetDefaultAsStringFallback(clang::QualType qual_type) {
  qual_type = qual_type.getUnqualifiedType().getCanonicalType();

  if (qual_type->isBooleanType()) {
    return "false";
  }

  if (qual_type->isIntegerType() && !qual_type->isEnumeralType()) {
    return getTypedLiteral("0", ToString(qual_type));
  }

  if (qual_type->isFloatingType()) {
    return getTypedLiteral("0.0", ToString(qual_type));
  }

  if (auto record = qual_type->getAsRecordDecl()) {
    if (ctx_.getSourceManager().isInSystemHeader(record->getLocation()) &&
        qual_type.isPODType(ctx_)) {
      return std::format("unsafe {{ std::mem::zeroed::<{}>() }}",
                         ToString(qual_type));
    }
  }

  if (qual_type->isEnumeralType()) {
    auto enum_decl = qual_type->castAs<clang::EnumType>()->getDecl();
    if (enum_decl->enumerators().empty()) {
      return std::string(1, token::kZero);
    }
    return EnumeratorName(*enum_decl->enumerator_begin());
  }

  return std::format("<{}>::default()", ToString(qual_type));
}

std::string Converter::ConvertVarDefaultInit(clang::QualType qual_type) {
  return GetDefaultAsString(qual_type);
}

std::string
Converter::GetOverloadedFunctionName(const clang::FunctionDecl *decl) {
  auto name = GetFunctionBaseName(decl);
  if (auto *conversion = clang::dyn_cast<clang::CXXConversionDecl>(decl)) {
    name = GetConversionName(
        conversion, GetUnsafeTypeAsString(conversion->getConversionType()));
  }
  if (auto *ctor = clang::dyn_cast<clang::CXXConstructorDecl>(decl);
      ctor && !ctor->getParent()->getIdentifier()) {
    name = GetRecordName(ctor->getParent());
  }

  if (decl->getNumParams() != 0U) {
    name += '_';
  }

  for (auto *parameter : decl->parameters()) {
    name += GetUnsafeTypeAsString(parameter->getType());
    if (parameter->getType()->isRValueReferenceType()) {
      name += "_rv";
    }
    name += '_';
  }

  if (const auto *targs = decl->getTemplateSpecializationArgs()) {
    std::vector<clang::TemplateArgument> args;
    for (const auto &arg : targs->asArray()) {
      if (arg.getKind() == clang::TemplateArgument::Pack) {
        args.insert(args.end(), arg.pack_begin(), arg.pack_end());
      } else {
        args.push_back(arg);
      }
    }
    for (const auto &arg : args) {
      name += '_';
      switch (arg.getKind()) {
      case clang::TemplateArgument::Type:
        name += Mapper::ToRustName(
            arg.getAsType().getCanonicalType().getAsString());
        break;
      case clang::TemplateArgument::Integral:
        name += Mapper::ToRustName(
            std::string(GetNumAsString(arg.getAsIntegral())));
        break;
      default:
        name += "targ";
        break;
      }
    }
  }

  auto pred = [](char ch) { return ch != ' ' && ch != '_'; };
  name.erase(std::find_if(name.rbegin(), name.rend(), pred).base(), name.end());

  if (decl->isVariadic()) {
    name += "_va";
  }
  if (const auto *method = clang::dyn_cast<clang::CXXMethodDecl>(decl)) {
    if (method->isConst()) {
      name += "_const";
    }
    if (method->isVolatile()) {
      name += "_volatile";
    }
    switch (method->getRefQualifier()) {
    case clang::RQ_LValue:
      name += "_lref";
      break;
    case clang::RQ_RValue:
      name += "_rref";
      break;
    case clang::RQ_None:
      break;
    }
  }

  ToIdentifier(name);
  ForceRustIdentifier(name, decl);
  return name;
}

// A Rust identifier admits only [A-Za-z0-9_] (and must not start with a digit).
// `ToIdentifier` folds away the punctuation that C++ TYPE SPELLINGS carry, but
// the text mangled in here is the type's RUST spelling, which comes from a RULE
// TARGET and may carry punctuation no C++ type ever produces. The measured case
// is a lifetime: a rules/mlir target of `Option<&'static dataflowir_gen::
// TdOpDef>` -- the faithful shape of MLIR's nullable `Impl*` -- left `&` and `'`
// untouched, so the converter emitted
//     pub unsafe fn getNumPhasesAttrName_Option&'staticdataflowir_genTdOpDef(..)
// and rustc died with "missing parameters for function definition" after the
// output had already been truncated. So fold anything still not an identifier
// character, and then REFUSE LOUDLY rather than emit a name rustc cannot parse:
// a broken name is silent corruption, and this project takes a loud abort over
// that every time.
void Converter::ForceRustIdentifier(std::string &name,
                                    const clang::FunctionDecl *decl) {
  auto is_ident_char = [](char c) {
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
  };

  std::string folded;
  folded.reserve(name.size());
  for (char c : name) {
    if (is_ident_char(c)) {
      folded += c;
    } else if (c == '&') {
      // Same spelling Mapper::ToRustName uses for a reference.
      folded += "ref";
    } else if (c == '\'') {
      // The apostrophe of a lifetime is punctuation, not part of the name.
      folded += '_';
    } else {
      folded += '_';
    }
  }
  if (!folded.empty() && std::isdigit(static_cast<unsigned char>(folded[0]))) {
    folded.insert(folded.begin(), '_');
  }
  name = std::move(folded);

  // Defensive: the fold above is total, so this cannot fire. If it ever does,
  // name the offending target type instead of emitting a broken definition.
  if (!std::ranges::all_of(name, is_ident_char) ||
      (!name.empty() && std::isdigit(static_cast<unsigned char>(name[0])))) {
    std::string params;
    for (const auto *p : decl->parameters()) {
      if (!params.empty()) {
        params += ", ";
      }
      params += GetUnsafeTypeAsString(p->getType());
    }
    const std::string loc =
        decl->getLocation().printToString(ctx_.getSourceManager());
    const std::string detail =
        "target type mangles to an invalid Rust identifier: `" + name +
        "` for `" + Mapper::ToString(decl) + "`; rust parameter types: (" +
        params + ")";
    if (survey::Enabled()) {
      survey::Record(survey::GapKind::kUnsupportedConstruct, detail, loc);
      return;
    }
    llvm::errs() << "unsupported " << detail << " at " << loc << '\n';
    assert(0 && "target type mangles to an invalid Rust identifier\n");
  }
}

std::string Converter::GetRecordName(const clang::NamedDecl *decl) const {
  auto ID = GetID(decl);
  if (auto it = inner_structs_.find(ID); it != inner_structs_.end()) {
    return it->second;
  }
  return Mapper::ToRustName(Mapper::ToString(Mapper::GetTypeForDecl(decl)));
}

std::vector<const char *>
Converter::GetStructAttributes(const clang::RecordDecl *decl) {
  if (decl->isUnion()) {
    return {"Copy", "Clone"};
  }

  std::vector<const char *> struct_attrs;

  if (HasDefaultedCopyConstructor(decl) && RecordHasCopyableFields(decl)) {
    struct_attrs.emplace_back("Copy");
  }

  if (HasDefaultedCopyConstructor(decl)) {
    struct_attrs.emplace_back("Clone");
  }

  if (RecordDerivesDefault(decl)) {
    struct_attrs.emplace_back("Default");
  }

  return struct_attrs;
}

std::string Converter::GetUnsafeTypeAsString(clang::QualType qual_type) const {
  std::string type_as_string;
  Converter converter(type_as_string, ctx_);
  converter.Convert(qual_type);
  return std::string(Trim(type_as_string));
}

void Converter::ConvertVarInit(clang::QualType qual_type, clang::Expr *expr) {
  if (qual_type->isReferenceType() && !IsReferenceType(expr)) {
    if (llvm::isa<clang::MaterializeTemporaryExpr>(expr->IgnoreImpCasts())) {
      StrCat(EmitMaterializedTempBinding(qual_type, expr));
      return;
    }
    if (auto *cond = clang::dyn_cast<clang::ConditionalOperator>(
            expr->IgnoreParenImpCasts());
        cond && cond->isLValue()) {
      {
        PushExprKind push(*this, ExprKind::LValue);
        PushInitType init_type(*this, qual_type);
        Convert(cond);
      }
      StrCat(keyword::kAs);
      Convert(qual_type);
      return;
    }
    StrCat(token::kRef);
    if (IsMut(qual_type)) {
      StrCat(keyword_mut_);
    }
  }
  if (qual_type->isFunctionPointerType()) {
    if (auto *lambda = clang::dyn_cast<clang::LambdaExpr>(
            expr->IgnoreUnlessSpelledInSource())) {
      PushExprKind push(*this, ExprKind::AddrOf);
      PushInitType init_type(*this, qual_type);
      VisitLambdaExpr(lambda);
      return;
    }
  }
  auto *ignore_casts = expr->IgnoreCasts();
  // FIXME: this looks very complicated
  if (auto *ctor = clang::dyn_cast<clang::CXXConstructExpr>(ignore_casts);
      ctor && ctor->getNumArgs() != 0 && IsReferenceType(ctor->getArg(0)) &&
      clang::isa<clang::CallExpr>(ctor->getArg(0)->IgnoreCasts()) &&
      !Mapper::Contains(
          clang::cast<clang::CallExpr>(ctor->getArg(0)->IgnoreCasts())
              ->getCallee()) &&
      Mapper::ToString(ctor->getConstructor()->getThisType()) ==
          "std::string") {
    {
      PushParen paren(*this);
      StrCat(token::kStar);
      PushInitType init_type(*this, qual_type);
      Convert(expr);
    }
    StrCat(".clone()");
  } else if (IsReferenceType(expr) || qual_type->isFunctionPointerType()) {
    PushExprKind push(*this, ExprKind::AddrOf);
    PushInitType init_type(*this, qual_type);
    Convert(expr, qual_type);
  } else {
    PushExprKind push(*this, ExprKind::RValue);
    PushInitType init_type(*this, qual_type);
    Convert(expr, qual_type);
  }
}

void Converter::ConvertUnsignedArithOperand(clang::Expr *expr,
                                            clang::QualType type) {
  bool needs_cast = (expr->isIntegerConstantExpr(ctx_) &&
                     !clang::isa<clang::ImplicitCastExpr>(expr)) ||
                    Mapper::Map(expr->getType()) != Mapper::Map(type);
  PushParen paren(*this, needs_cast);
  Convert(expr);
  if (needs_cast) {
    ConvertCast(type);
  }
}

void Converter::ConvertEqualsNullPtr(clang::Expr *expr) {
  StrCat('(');
  Convert(expr);
  if (IsUniquePtr(expr->getType()) ||
      expr->getType()->isFunctionPointerType()) {
    StrCat(").is_none()");
  } else {
    StrCat(").is_null()");
  }
  computed_expr_type_ = ComputedExprType::FreshValue;
}

void Converter::ConvertPointerSubscript(clang::ArraySubscriptExpr *expr) {
  auto *base = expr->getBase();
  auto *idx = expr->getIdx();
  if (isAddrOf()) {
    ConvertPointerOffset(base, idx);
  } else {
    PushParen paren(*this);
    StrCat(token::kStar);
    ConvertPointerOffset(base, idx);
  }
}

void Converter::ConvertPointerOffset(clang::Expr *base, clang::Expr *idx,
                                     bool is_addition) {
  Convert(base);
  StrCat(token::kDot, "offset");
  PushParen outer(*this);
  if (!is_addition) {
    StrCat(token::kMinus);
  }
  PushParen neg_paren(*this, !is_addition);
  {
    PushParen inner(*this);
    PushExprKind push(*this, ExprKind::RValue);
    Convert(idx);
  }
  StrCat(keyword::kAs, "isize");
  computed_expr_type_ = ComputedExprType::FreshPointer;
}

static bool IsFlexibleArrayMemberAccess(clang::ASTContext &ctx,
                                        clang::Expr *array) {
  return array->isFlexibleArrayMemberLike(
      ctx, clang::LangOptions::StrictFlexArraysLevelKind::OneZeroOrIncomplete,
      /*IgnoreTemplateOrMacroSubstitution=*/true);
}

void Converter::EmitFlexibleArrayElementPtr(clang::Expr *array,
                                            clang::Expr *idx, bool is_mut) {
  {
    PushExplicitAutoref no_autoref(*this, std::nullopt);
    Convert(array);
  }
  StrCat(is_mut ? ".as_mut_ptr()" : ".as_ptr()", ".add");
  {
    PushParen call(*this);
    {
      PushParen paren(*this);
      Convert(idx);
    }
    StrCat(keyword::kAs, "usize");
  }
}

void Converter::ConvertArraySubscript(clang::Expr *base, clang::Expr *idx,
                                      clang::QualType type) {
  if (auto inner = base->IgnoreImplicit()) {
    if (inner->getType()->isArrayType() &&
        IsFlexibleArrayMemberAccess(ctx_, inner)) {
      PushParen outer(*this);
      StrCat(token::kStar);
      EmitFlexibleArrayElementPtr(inner, idx,
                                  !inner->getType().isConstQualified());
      return;
    }
  }
  if (IsUniquePtr(base->getType())) {
    PushExplicitAutoref no_autoref(*this, std::nullopt);
    Convert(base->IgnoreImplicit());
    StrCat(".as_mut().unwrap()");
  } else {
    Convert(base->IgnoreImplicit());
  }
  PushExplicitAutoref no_autoref(*this, std::nullopt);
  PushBracket bracket(*this);
  {
    PushParen paren(*this);
    Convert(idx);
  }

  if (Mapper::Map(idx->getType()) != "usize") {
    StrCat(keyword::kAs, "usize");
  }
}

void Converter::ConvertAssignment(clang::Expr *lhs, clang::Expr *rhs,
                                  std::string_view assign_operator) {
  std::string lhs_as_string;
  {
    PushInitType init_type(*this, lhs->getType());
    lhs_as_string = ConvertLValue(lhs);
  }
  auto rhs_as_string = ConvertFreshRValue(rhs, lhs->getType());

  PushBrace brace(*this, !isVoid());

  StrCat(lhs_as_string, assign_operator, rhs_as_string);
  if (!isVoid()) {
    StrCat(token::kSemiColon,
           isAddrOf() ? ConvertRValue(lhs) : ConvertFreshRValue(lhs));
  }
}

void Converter::ConvertFunctionParameters(clang::FunctionDecl *decl) {
  in_function_formals_ = true;
  auto *definition =
      decl->getDefinition() != nullptr ? decl->getDefinition() : decl;
  for (auto *parameter : definition->parameters()) {
    ConvertVarDeclSkipInit(parameter);
    StrCat(token::kComma);
  }
  if (decl->isVariadic()) {
    StrCat("__args: &[VaArg]", token::kComma);
  }
  in_function_formals_ = false;
}

void Converter::ConvertFunctionQualifiers(clang::FunctionDecl *decl) {
  StrCat(AccessSpecifierAsString(decl->getAccess()));
}

void Converter::ConvertFunctionReturnType(clang::FunctionDecl *decl) {
  auto return_type = decl->getReturnType();
  if (!return_type->isVoidType()) {
    StrCat(token::kArrow);
    Convert(return_type);
  }
}

void Converter::ConvertFunctionMain(const clang::FunctionDecl *decl,
                                    const std::string_view main_function_name) {
  if (decl->getNumParams() != 0U) {
    StrCat(std::format(R"(
pub fn main() {{
    let mut args: Vec<Vec<u8>> = std::env::args().map(|arg| arg.as_bytes().to_vec()).collect();
    args.iter_mut().for_each(|v| v.push(0));
    let mut argv: Vec<*mut libc::c_char> = args.iter().map(|arg| arg.as_ptr() as *mut libc::c_char).collect();
    argv.push(::std::ptr::null_mut());
    unsafe {{
        __cpp2rust_init_globals();
        ::std::process::exit(main_0((argv.len() - 1) as i32, argv.as_mut_ptr()) as i32)
    }}
}})",
                       main_function_name));
  } else {
    StrCat(std::format("pub fn main() {{ unsafe {{ __cpp2rust_init_globals(); "
                       "std::process::exit({}() as i32); }} }}",
                       main_function_name));
  }
}

void Converter::ConvertAbstractClass(clang::CXXRecordDecl *decl) {
  ENSURE(abstract_structs_.insert(GetID(decl)).second);
  auto trait_name = GetRecordName(decl);
  auto access_specifier_as_string = AccessSpecifierAsString(decl->getAccess());
  auto signature = std::format("{} {} trait {}", access_specifier_as_string,
                               keyword_unsafe_, trait_name);
  auto predicate = [](auto *method) {
    return !method->isImplicit() &&
           !clang::isa<clang::CXXDestructorDecl>(method);
  };
  PushInTraitBody push_trait(*this, true);
  ConvertCXXMethodDecls(decl, signature, predicate);
}

void Converter::ConvertCXXMethodDecls(
    const clang::CXXRecordDecl *decl, const std::string_view signature,
    bool (*predicate)(clang::CXXMethodDecl *)) {
  bool first = true;
  auto convert_method = [&](clang::CXXMethodDecl *method) {
    if (predicate(method)) {
      if (first) {
        StrCat(signature, token::kOpenCurlyBracket);
        first = false;
      }
      VisitCXXMethodDecl(method);
    }
  };
  for (auto *method : decl->methods()) {
    convert_method(method);
  }
  ForEachTemplateInstantiatedMethod(decl, convert_method);
  if (!first) {
    StrCat(token::kCloseCurlyBracket);
  }
}

bool Converter::BaseTargetNamesTrait(clang::QualType base_type,
                                     std::string_view base_target) const {
  const auto *base_record = base_type->getAsCXXRecordDecl();
  if (base_record != nullptr && IsUserDefinedDecl(base_record)) {
    // A user-written base is lowered by THIS converter, and an abstract one
    // becomes a trait (ConvertAbstractClass). Leave that path untouched.
    return true;
  }
  // The base came from a system header, so its Rust spelling comes from the
  // rule table, which maps C++ types to Rust TYPES. A type is never a trait.
  if (base_target == "()") {
    // A `()`-mapped base contributes no state and no methods, so there is
    // nothing to implement: the caller emits an inherent impl instead.
    return false;
  }
  const std::string loc =
      base_record != nullptr
          ? base_record->getLocation().printToString(ctx_.getSourceManager())
          : "<unknown>";
  // MEASURED: this path is NOT a failure. On a TU whose only survey row was
  // this one, a plain run is rc=0, emits Rust, and the caller
  // (VirtualMethodsFor, below) lowers the base's virtuals as four INHERENT
  // impls -- `impl mlir_dataflow_DataflowDialect { ... }`, zero `impl ... for
  // ...`, zero occurrences of the rule target name. That is the correct
  // lowering, because a rule target names a Rust TYPE and a type is never a
  // trait. The old `assert(0)` here therefore aborted an assertions build on a
  // construct that is handled, and the old kUnsupportedConstruct record made
  // the sweep carry 269 phantom gap rows that crowded out real ones.
  //
  // What IS lost is virtual dispatch through a reference to the rule-mapped
  // base, so the row is kept as INFORMATIONAL rather than deleted. The only
  // such dispatch anywhere in the corpus is
  // `dialect->getCanonicalizationPatterns(patterns)` over
  // `ctx->getLoadedDialects()`
  // (dataflow-scheduler/lib/Conversion/backend/ScheduleIRToDFIR/KTDFLowToDFIR/
  // KTDFLowToDFIR.cpp:70), which dispatches inside MLIR over MLIR's own
  // dialects -- on the far side of the rule boundary, never through a ported
  // derived type. So nothing observable differs today; the record exists so a
  // future non-MLIR rule-mapped base with called virtuals is still traceable.
  const std::string detail =
      "rule-mapped base class `" + base_type.getAsString() + "` -> `" +
      std::string(base_target) +
      "` lowered as inherent impls (correct); virtual dispatch through a "
      "reference to the base is not available";
  if (survey::Enabled()) {
    survey::Record(survey::GapKind::kInfo, detail, loc);
  }
  return false;
}

Converter::DeferredBlock &
Converter::VirtualMethodsFor(const clang::CXXRecordDecl *decl) {
  auto name = GetRecordName(decl);
  auto [it, inserted] = virtual_methods_.try_emplace(name);
  if (inserted) {
    auto base_type = decl->bases_begin()->getType();
    auto base_target = GetUnsafeTypeAsString(base_type);
    if (BaseTargetNamesTrait(base_type, base_target)) {
      it->second.header =
          std::format("{} impl {} for {}", keyword_unsafe_, base_target, name);
    } else {
      // The base is not lowered to a Rust trait (a rule-mapped base names a
      // TYPE, not a trait), so there is no trait to implement. Its virtual
      // methods become INHERENT methods on the derived type. Emitting
      // `impl <type> for <derived>` here is a category error: rustfmt rejects
      // it outright ("expected a trait, found type").
      it->second.header = std::format("{} {}", keyword::kImpl, name);
    }
  }
  return it->second;
}

void Converter::ConvertVirtualMethods(clang::CXXRecordDecl *decl) {
  if (decl->bases_begin() == decl->bases_end()) {
    return;
  }
  bool any = false;
  Buffer buf(*this);
  for (auto *method : decl->methods()) {
    if (!method->isImplicit() && method->isVirtual()) {
      any = true;
      VisitCXXMethodDecl(method);
    }
  }
  auto body = std::move(buf).str();
  if (!any) {
    return;
  }
  VirtualMethodsFor(decl).body += body;
}

bool Converter::ConvertOutOfLineVirtualMethod(clang::CXXMethodDecl *decl) {
  auto *record = decl->getParent();
  if (record->bases_begin() == record->bases_end()) {
    return false;
  }
  Buffer buf(*this);
  auto emitted = ConvertCXXMethodDecl(decl);
  VirtualMethodsFor(record).body += std::move(buf).str();
  return emitted;
}

void Converter::ConvertOrdAndPartialOrdTraitsBase(
    std::string_view cmp_body, std::string_view eq_body,
    std::string_view record_name) {
  if (!cmp_body.empty()) {
    StrCat(keyword::kImpl, "std::cmp::Ord for ", record_name, '{');
    StrCat("fn cmp(&self, other: &Self) -> std::cmp::Ordering {");
    StrCat(std::format("{} {{", keyword_unsafe_));
    StrCat(cmp_body);
    StrCat("}}}");

    StrCat(keyword::kImpl, "std::cmp::PartialOrd for", record_name, '{');
    StrCat(R"(
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
      Some(self.cmp(other))
    }
  })");
  }

  StrCat(keyword::kImpl, "std::cmp::PartialEq for", record_name, '{');
  StrCat("fn eq(&self, other: &Self) -> bool {");
  StrCat(std::format("{} {{", keyword_unsafe_));
  StrCat(eq_body);
  StrCat("}}}");

  StrCat(keyword::kImpl, "std::cmp::Eq for", record_name, "{}");
}

std::string Converter::GetComparisonCall(const clang::FunctionDecl *op,
                                         const clang::CXXRecordDecl *decl,
                                         std::string_view lhs,
                                         std::string_view rhs) {
  auto arg = [&](unsigned i, std::string_view value) {
    if (op->getParamDecl(i)->getType()->isReferenceType()) {
      return GetComparisonReferenceArg(decl, value);
    }
    return std::format("{}.clone()", value);
  };
  if (const auto *method = clang::dyn_cast<clang::CXXMethodDecl>(op)) {
    return std::format("{}::{}({}, {})", GetUFCSName(method),
                       GetMethodName(method),
                       GetComparisonReceiver(method, decl, lhs), arg(0, rhs));
  }
  return std::format("{}({}, {})", GetNamedDeclAsString(op->getCanonicalDecl()),
                     arg(0, lhs), arg(1, rhs));
}

std::string
Converter::GetComparisonReferenceArg(const clang::CXXRecordDecl *decl,
                                     std::string_view value) {
  return std::format("{} as *const {}", value, GetRecordName(decl));
}

std::string Converter::GetComparisonReceiver(const clang::CXXMethodDecl *method,
                                             const clang::CXXRecordDecl *,
                                             std::string_view lhs) {
  if (!MethodNeedsMutableReceiver(method)) {
    return std::string(lhs);
  }
  return std::format("&mut *(&raw const *{}).cast_mut()", lhs);
}

void Converter::ConvertOrdAndPartialOrdTraits(const clang::CXXRecordDecl *decl,
                                              const clang::FunctionDecl *eq,
                                              const clang::FunctionDecl *lt,
                                              const clang::FunctionDecl *cmp) {
  std::string cmp_body, eq_body;

  if (cmp) {
    cmp_body = GetComparisonCall(cmp, decl, "self", "other");
  } else if (lt) {
    cmp_body = std::format("if {} {{ std::cmp::Ordering::Less }} else if {} {{ "
                           "std::cmp::Ordering::Greater }} else {{ "
                           "std::cmp::Ordering::Equal }}",
                           GetComparisonCall(lt, decl, "self", "other"),
                           GetComparisonCall(lt, decl, "other", "self"));
  }

  if (eq) {
    eq_body = GetComparisonCall(eq, decl, "self", "other");
  } else if (cmp) {
    eq_body = std::format("{} == std::cmp::Ordering::Equal",
                          GetComparisonCall(cmp, decl, "self", "other"));
  } else {
    eq_body = std::format("!({}) && !({})",
                          GetComparisonCall(lt, decl, "self", "other"),
                          GetComparisonCall(lt, decl, "other", "self"));
  }

  ConvertOrdAndPartialOrdTraitsBase(cmp_body, eq_body, GetRecordName(decl));
}

void Converter::AddOrdTrait(const clang::CXXRecordDecl *decl) {
  const clang::FunctionDecl *eq = nullptr;
  const clang::FunctionDecl *lt = nullptr;
  const clang::FunctionDecl *cmp = nullptr;
  auto consider = [&](const clang::FunctionDecl *fn) {
    if (!fn || fn->isImplicit() || fn->isDeleted() || !fn->hasBody() ||
        fn->getDescribedFunctionTemplate() || !IsSameTypeComparison(fn, decl)) {
      return;
    }
    switch (fn->getOverloadedOperator()) {
    case clang::OO_EqualEqual:
      eq = fn;
      break;
    case clang::OO_Less:
      lt = fn;
      break;
    case clang::OO_Spaceship:
      cmp = fn;
      break;
    default:
      break;
    }
  };
  auto consider_decl = [&](const clang::NamedDecl *found) {
    if (const auto *tmpl =
            clang::dyn_cast<clang::FunctionTemplateDecl>(found)) {
      for (const auto *spec : tmpl->specializations()) {
        consider(spec);
      }
      return;
    }
    consider(clang::dyn_cast<clang::FunctionDecl>(found));
  };
  for (const auto *method : decl->methods()) {
    consider(method);
  }
  for (auto op : {clang::OO_EqualEqual, clang::OO_Less, clang::OO_Spaceship}) {
    auto name = ctx_.DeclarationNames.getCXXOperatorName(op);
    for (const auto *found : decl->getDeclContext()->lookup(name)) {
      consider_decl(found);
    }
  }
  for (const auto *friend_decl : decl->friends()) {
    if (const auto *found = friend_decl->getFriendDecl()) {
      consider_decl(found);
    }
  }

  if (!eq && !lt && !cmp) {
    return;
  }

  ConvertOrdAndPartialOrdTraits(decl, eq, lt, cmp);
}

void Converter::AddCloneTrait(const clang::RecordDecl *decl) {
  auto *ctor = GetUserDefinedCopyConstructor(decl);
  if (!ctor) {
    return;
  }
  auto record_name = GetRecordName(decl);
  StrCat(keyword::kImpl, "Clone for", record_name);
  PushBrace impl_brace(*this);
  StrCat("fn clone(&self) -> Self");
  PushBrace fn_brace(*this);
  auto source = ctor->getParamDecl(0)->getType().getNonReferenceType();
  StrCat(std::format("unsafe {{ {}::{}(self as *const {}{}) }}", record_name,
                     GetCtorName(ctor), record_name,
                     source.isConstQualified()
                         ? ""
                         : std::format(" as *mut {}", record_name)));
}

void Converter::AddDefaultTraitForUnion(const clang::RecordDecl *decl) {
  StrCat(std::format("impl Default for {}", GetRecordName(decl)));
  PushBrace impl_brace(*this);
  StrCat("fn default() -> Self");
  PushBrace fn_brace(*this);
  StrCat("unsafe");
  PushBrace unsafe_brace(*this);
  StrCat("std::mem::zeroed()");
}

void Converter::AddDefaultTrait(const clang::RecordDecl *decl) {
  if (decl->isUnion()) {
    AddDefaultTraitForUnion(decl);
    return;
  }
  if (RecordDerivesDefault(decl)) {
    return;
  }
  auto struct_name = GetRecordName(decl);
  StrCat(std::format("impl Default for {}", struct_name));
  PushBrace impl_brace(*this);
  StrCat("fn default() -> Self");
  PushBrace fn_brace(*this);

  if (auto *cxx = clang::dyn_cast<clang::CXXRecordDecl>(decl)) {
    if (auto *default_ctor = GetUserDefinedDefaultConstructor(cxx)) {
      StrCat(keyword_unsafe_);
      PushBrace unsafe_brace(*this);
      Convert(MakeConstructExpr(ctx_, ctx_.getCanonicalTagType(decl),
                                default_ctor, {}));
      return;
    }
  }

  EmitDefaultStructLiteral(decl);
}

void Converter::EmitDefaultStructLiteral(const clang::RecordDecl *decl) {
  StrCat(GetRecordName(decl));
  PushBrace brace(*this);
  for (auto *field : decl->fields()) {
    StrCat(GetNamedDeclAsString(field), token::kColon);
    if (auto *init = field->getInClassInitializer()) {
      ConvertVarInit(field->getType(), init);
    } else {
      StrCat(GetDefaultAsString(field->getType()));
    }
    StrCat(token::kComma);
  }
}

void Converter::AddByteReprTrait(const clang::RecordDecl *decl) {}

void Converter::ConvertUnsignedArithBinaryOperator(clang::BinaryOperator *op,
                                                   clang::Expr *expr) {
  StrCat(token::kDot);
  auto opcode = op->getOpcode();
  switch (opcode) {
  case clang::BinaryOperator::Opcode::BO_Add:
  case clang::BinaryOperator::Opcode::BO_AddAssign:
    StrCat("wrapping_add");
    break;
  case clang::BinaryOperator::Opcode::BO_Sub:
  case clang::BinaryOperator::Opcode::BO_SubAssign:
    StrCat("wrapping_sub");
    break;
  case clang::BinaryOperator::Opcode::BO_Mul:
  case clang::BinaryOperator::Opcode::BO_MulAssign:
    StrCat("wrapping_mul");
    break;
  case clang::BinaryOperator::Opcode::BO_Div:
  case clang::BinaryOperator::Opcode::BO_DivAssign:
    StrCat("wrapping_div");
    break;
  case clang::BinaryOperator::Opcode::BO_Rem:
  case clang::BinaryOperator::Opcode::BO_RemAssign:
    StrCat("wrapping_rem");
    break;
  default:
    if (survey::Enabled()) {
      survey::Record(survey::GapKind::kUnsupportedExpr,
                     std::string("unsigned binary operator: ") +
                         clang::BinaryOperator::getOpcodeStr(opcode).str(),
                     op->getExprLoc().printToString(ctx_.getSourceManager()));
      break;
    }
    // FIXME: improve error handling
    llvm::errs() << "unsupported unsigned binary operator: " << opcode << '\n';
    op->dump();
    assert(0);
  }
  PushParen paren(*this);

  auto type = op->getType();
  bool is_pointer_plus_integer_op = false;

  if (auto *assign = llvm::dyn_cast<clang::CompoundAssignOperator>(op)) {
    if (op->getLHS()->getType()->isPointerType() &&
        op->getRHS()->getType()->isIntegralOrEnumerationType()) {
      type = op->getRHS()->getType();
      is_pointer_plus_integer_op = true;
    } else {
      type = assign->getComputationResultType();
    }
  }
  ConvertUnsignedArithOperand(expr, type);
  if (is_pointer_plus_integer_op) {
    StrCat("as usize");
  }
}

void Converter::ConvertAddrOf(clang::Expr *expr, clang::QualType pointer_type) {
  assert(pointer_type->isPointerType());
  if (auto ase =
          clang::dyn_cast<clang::ArraySubscriptExpr>(expr->IgnoreParens())) {
    auto base = ase->getBase();
    auto inner = base->IgnoreImplicit();
    if (base->IgnoreCasts()->getType()->isArrayType() &&
        IsFlexibleArrayMemberAccess(ctx_, inner)) {
      EmitFlexibleArrayElementPtr(
          inner, ase->getIdx(),
          !pointer_type->getPointeeType().isConstQualified());
      computed_expr_type_ = ComputedExprType::FreshPointer;
      return;
    }
  }
  if (IsReferenceType(expr) || pointer_type->isFunctionPointerType()) {
    PushExprKind push(*this, ExprKind::AddrOf);
    Convert(expr);
  } else if (IsGlobalVar(expr)) {
    StrCat("&raw", pointer_type->getPointeeType().isConstQualified()
                       ? keyword::kConst
                       : keyword_mut_);
    Convert(expr);
    ConvertCast(pointer_type);
    computed_expr_type_ = ComputedExprType::FreshPointer;
  } else {
    StrCat(token::kRef);
    if (!pointer_type->getPointeeType().isConstQualified()) {
      StrCat(keyword_mut_);
    }
    Convert(expr);
    ConvertCast(pointer_type);
    computed_expr_type_ = ComputedExprType::FreshPointer;
  }
}

void Converter::EmitDeref(std::string inner, clang::QualType pointee_type) {
  auto wrap = std::exchange(autoref_mut_, std::nullopt);
  PushParen outer(*this, wrap.has_value());
  if (wrap) {
    StrCat(*wrap ? "&mut" : "&");
  }
  PushParen paren(*this);
  StrCat(GetPointerDerefPrefix(pointee_type), std::move(inner));
  SetValueFreshness(pointee_type);
}

void Converter::ConvertDeref(clang::Expr *expr) {
  if (!isAddrOf()) {
    EmitDeref(ToString(expr), expr->getType()->getPointeeType());
  } else {
    Convert(expr);
  }
}

void Converter::ConvertArrow(clang::Expr *expr) { ConvertDeref(expr); }

void Converter::ConvertCast(clang::QualType qual_type, int line) {
  log() << "[ConvertCast] Called from line " << line << '\n';
  StrCat(keyword::kAs, GetUnsafeTypeAsString(qual_type));
}

Converter::TempMaterializationCtx
Converter::CollectRefBindingTempArgs(clang::CallExpr *expr) {
  TempMaterializationCtx ctx(expr->getNumArgs());
  if (auto *fn = expr->getCalleeDecl() ? expr->getCalleeDecl()->getAsFunction()
                                       : nullptr) {
    for (unsigned i = 0; i < expr->getNumArgs() && i < fn->getNumParams();
         ++i) {
      auto param_type = fn->getParamDecl(i)->getType();
      if (NeedsRefBindingTemp(expr->getArg(i), param_type)) {
        ctx.materialized_args[i] = param_type;
      }
    }
  }
  return ctx;
}

const std::string &Converter::TempMaterializationCtx::GetOrMaterialize(
    unsigned argument_num,
    std::function<std::pair<std::string, std::string>(const std::string &,
                                                      clang::QualType)>
        materialize_fn) {
  auto &str = materialized_refs_.at(argument_num);
  if (!str.empty()) {
    return str;
  }

  if (auto m = materialized_args.at(argument_num)) {
    auto [binding, ref] =
        materialize_fn(std::format("__tmp_{}", argument_num), *m);
    temporary_bindings += std::move(binding);
    str = std::move(ref);
    return str;
  }

  static const std::string empty_str;
  return empty_str;
}

void Converter::PlaceholderCtx::dump() const {
  llvm::errs() << "is_receiver: " << is_receiver
               << ", is_cpp_ptr: " << is_cpp_ptr
               << ", maps_to_rust_ptr: " << maps_to_rust_ptr
               << ", declared_in_rule_as_rust_ptr: "
               << declared_in_rule_as_rust_ptr
               << ", access: " << static_cast<int>(access)
               << ", arg_idx: " << arg_idx
               << ", materialize_idx: " << materialize_idx << '\n';
}

std::string Converter::ConvertPlaceholder(clang::Expr *expr, clang::Expr *arg,
                                          const PlaceholderCtx &ph_ctx) {
  if (!ph_ctx.needs_explicit_shared_borrow) {
    return ConvertPlaceholderImpl(expr, arg, ph_ctx);
  }
  // SHARED-`&` DECLARATION. The rule says `a1: &T`; a rule body is INLINED, so
  // whatever we emit here lands in argument position of a real Rust call and
  // must actually BE a `&T`. Two cases, and they are not distinguishable from
  // the IR -- only from what the emission turned out to be:
  //   * already a reference (`c"bad name "` is `&CStr`) -> leave it alone,
  //     prefixing would give `&&CStr`;
  //   * a place (`(*s)`, the by-value deref a C++ reference argument gets)
  //     -> borrow it.
  // Hence the bit: clear, convert, then ask.
  bool saved = emitted_a_reference_;
  emitted_a_reference_ = false;
  auto emitted = ConvertPlaceholderImpl(expr, arg, ph_ctx);
  bool was_reference = emitted_a_reference_;
  emitted_a_reference_ = saved;
  if (was_reference || emitted.empty()) {
    return emitted;
  }
  // Parenthesised: the emission is an arbitrary expression, and `&` binds
  // tighter than any binary operator, so `&a as *const _` / `&x + y` would
  // re-associate. A shared borrow of a place coerces (`&Vec<T>` -> `&[T]`) at
  // the call site, which is exactly where this lands.
  return "&(" + std::move(emitted) + ")";
}

std::string Converter::ConvertPlaceholderImpl(clang::Expr *expr,
                                              clang::Expr *arg,
                                              const PlaceholderCtx &ph_ctx) {
  if (arg->getType()->isFunctionPointerType()) {
    return ConvertFnPtrPlaceholder(arg);
  }

  if (ph_ctx.declared_in_rule_as_rust_ptr && arg->getType()->isArrayType()) {
    return std::format(
        "({} as {})", ConvertFreshPointer(arg),
        Mapper::GetParamType(GetCalleeOrExpr(expr), ph_ctx.arg_idx));
  }

  if (ph_ctx.needs_materialization()) {
    auto materialized = ph_ctx.materialize_ctx->GetOrMaterialize(
        static_cast<unsigned>(ph_ctx.materialize_idx),
        [this, arg](const std::string &name, clang::QualType type) {
          return MaterializeTemp(name, type, arg);
        });
    if (!materialized.empty()) {
      return materialized;
    }
  }

  if (ph_ctx.needs_pointer_receiver()) {
    auto param_type =
        Mapper::GetParamType(GetCalleeOrExpr(expr), ph_ctx.arg_idx);
    return std::format("({} as {})", ConvertFreshObject(arg, param_type),
                       param_type);
  }

  if (ph_ctx.needs_object_receiver()) {
    Buffer buf(*this);
    PushExplicitAutoref autoref(
        *this, ph_ctx.is_index_base
                   ? std::optional(ph_ctx.access ==
                                   TranslationRule::Access::kBorrowMut)
                   : std::nullopt);
    PushExprKind push(*this, ExprKind::RValue);
    ConvertDeref(arg);
    return std::move(buf).str();
  }

  if (ph_ctx.needs_ptr_wrap()) {
    return ConvertFreshObject(arg);
  }

  if (ph_ctx.needs_lvalue()) {
    auto place = ConvertLValue(arg);
    if (ph_ctx.needs_mut_borrow()) {
      // The rule's parameter is `&mut T` and the inlined body puts this
      // placeholder in argument position, so a bare place is an E0308. Emit the
      // reborrow -- `&mut (*x)` when the place is itself a deref, which is what
      // a reference-returning target produces one chain level down.
      return "&mut " + std::move(place);
    }
    return place;
  }

  if (ph_ctx.access == TranslationRule::Access::kTake) {
    if (clang::isa<clang::MaterializeTemporaryExpr>(arg)) {
      return ConvertRValue(arg);
    }
    if (auto *record = arg->getType()->getAsCXXRecordDecl();
        record && IsUserDefinedDecl(record)) {
      for (auto *ctor : record->ctors()) {
        if (!IsConvertibleMoveConstructor(ctor)) {
          continue;
        }
        Buffer buf(*this);
        Convert(MakeConstructExpr(ctx_, arg->getType(), ctor, arg));
        return std::move(buf).str();
      }
      if (TypeIsCopyable(arg->getType())) {
        return ConvertRValue(arg);
      }
      return ConvertFreshRValue(arg);
    }
    auto lvalue = ConvertLValue(arg);
    SetFresh();
    return std::format("std::mem::take(&mut {})", std::move(lvalue));
  }

  if (ph_ctx.access == TranslationRule::Access::kMove) {
    return ConvertFreshRValue(arg, ph_ctx.implicit_convert_to);
  }

  return ConvertRValue(arg, ph_ctx.implicit_convert_to);
}

std::string Converter::ConvertMappedMethodCall(
    clang::Expr *expr, const TranslationRule::MethodCallFragment &mc,
    clang::Expr **args, unsigned num_args, TempMaterializationCtx *ctx) {
  return ConvertIRFragment(mc.receiver, expr, args, num_args, ctx,
                           /*is_method_call_receiver=*/true) +
         ConvertIRFragment(mc.body, expr, args, num_args, ctx);
}

std::string Converter::GetMappedAsString(clang::Expr *expr, clang::Expr **args,
                                         unsigned num_args,
                                         TempMaterializationCtx *ctx) {
  auto *tgt_ir = Mapper::GetExprRule(GetCalleeOrExpr(expr));
  if (!tgt_ir)
    return {};

  auto result = ConvertIRFragment(tgt_ir->body, expr, args, num_args, ctx);
  if (tgt_ir->multi_statement) {
    return '{' + result + '}';
  }
  return result;
}

std::string Converter::ConvertIRFragment(
    const std::vector<TranslationRule::BodyFragment> &fragments,
    clang::Expr *expr, clang::Expr **args, unsigned num_args,
    TempMaterializationCtx *ctx, bool is_method_call_receiver) {
  using namespace TranslationRule;

  auto all_args = BuildUnifiedArgs(expr, args, num_args);

  std::string result;
  for (auto &frag : fragments) {
    if (auto *t = std::get_if<TextFragment>(&frag)) {
      result += t->text;
    } else if (auto *g = std::get_if<GenericFragment>(&frag)) {
      result += Mapper::InstantiateTemplate(GetCalleeOrExpr(expr), g->n);
    } else if (auto *ph = std::get_if<PlaceholderFragment>(&frag)) {
      auto arg_idx = ph->n;
      // NDEBUG: `assert(arg_idx < all_args.size())` is compiled to NOTHING in
      // the release build, so this used to read `all_args` OUT OF BOUNDS and
      // fault on `arg->getType()` -- the rc=139 segfault the assert existed to
      // prevent. Must survive NDEBUG. Under --survey this is a recoverable gap:
      // the 403-TU survey is the fleet's work list, so RECORD AND CONTINUE.
      if (arg_idx >= all_args.size()) {
        std::string detail =
            std::format("rule body references placeholder a{} but the call site "
                        "supplies only {} argument(s)",
                        arg_idx, all_args.size());
        const std::string loc =
            expr != nullptr
                ? expr->getExprLoc().printToString(ctx_.getSourceManager())
                : std::string("<no expr>");
        if (survey::Enabled()) {
          survey::Record(survey::GapKind::kUnsupportedConstruct, detail, loc);
          continue;
        }
        llvm::report_fatal_error(llvm::Twine(detail) + " at " + loc,
                                 /*gen_crash_diag=*/false);
      }
      auto *arg = all_args[arg_idx];
      bool is_receiver = HasReceiver(expr) && arg_idx == 0;

      PlaceholderCtx ph_ctx{
          .arg_idx = arg_idx,
          .implicit_convert_to = GetParamImplicitConvertTarget(expr, arg_idx),
          .materialize_ctx = ctx,
          .materialize_idx =
              is_receiver ? -1 : ((int)arg_idx - HasReceiver(expr)),
          .access = ph->access,
          .is_receiver = is_receiver,
          .is_cpp_ptr = arg->getType()->isPointerType(),
          .maps_to_rust_ptr = Mapper::MapsToPointer(arg->getType()),
          .declared_in_rule_as_rust_ptr =
              Mapper::ParamIsPointer(GetCalleeOrExpr(expr), arg_idx),
          .is_index_base = ph->is_index_base,
          .needs_explicit_mut_borrow =
              !is_method_call_receiver &&
              Mapper::ParamIsMutRef(GetCalleeOrExpr(expr), arg_idx),
          // Same gate as the `&mut` case, for the same reason: on a method-call
          // receiver Rust's autoref supplies the borrow, and adding one would
          // perturb every existing reference-declared iterator rule.
          .needs_explicit_shared_borrow =
              !is_method_call_receiver &&
              Mapper::ParamIsSharedRef(GetCalleeOrExpr(expr), arg_idx),
      };
      result += ConvertPlaceholder(expr, arg, ph_ctx);
    } else if (std::get_if<TranslationRule::VaArgsFragment>(&frag)) {
      result += ConvertVariadicTail(expr, all_args);
    } else if (std::get_if<TranslationRule::InitFragment>(&frag)) {
      result += ConvertInitFragment(expr, all_args);
    } else if (auto *mc =
                   std::get_if<std::unique_ptr<MethodCallFragment>>(&frag)) {
      result += ConvertMappedMethodCall(expr, **mc, args, num_args, ctx);
    }
  }

  return result;
}

std::string
Converter::ConvertVariadicTail(clang::Expr *expr,
                               const std::vector<clang::Expr *> &all_args) {
  const auto *tgt_ir = Mapper::GetExprRule(GetCalleeOrExpr(expr));
  unsigned fixed = tgt_ir ? tgt_ir->params.size() : 0;

  Buffer buf(*this);
  StrCat("&[");
  for (unsigned i = fixed; i < all_args.size(); ++i) {
    {
      PushParen p(*this);
      ConvertVariadicArg(all_args[i]);
    }
    StrCat(".into()", token::kComma);
  }
  StrCat("]");
  return std::move(buf).str();
}

std::string
Converter::ConvertInitFragment(clang::Expr *expr,
                               const std::vector<clang::Expr *> &all_args) {
  const auto *tgt_ir = Mapper::GetExprRule(GetCalleeOrExpr(expr));
  assert(tgt_ir && tgt_ir->init_type.valid());
  auto *callee = clang::cast<clang::CallExpr>(expr)->getDirectCallee();
  assert(callee);
  auto type = GetSema()
                  .getTemplateInstantiationArgs(callee)(tgt_ir->init_type.depth,
                                                        tgt_ir->init_type.index)
                  .getAsType();

  Buffer buf(*this);
  ConvertConstructFromArgs(
      type, llvm::ArrayRef(all_args).drop_front(tgt_ir->params.size()),
      expr->getExprLoc());
  return std::move(buf).str();
}

void Converter::ConvertConstructFromArgs(clang::QualType type,
                                         llvm::ArrayRef<clang::Expr *> args,
                                         clang::SourceLocation loc) {
  auto *init = BuildInitExpr(GetSema(), type, args, loc);
  assert(init && "type cannot be initialized from the arguments");
  if (auto *ctor =
          clang::dyn_cast<clang::CXXConstructExpr>(init->IgnoreImplicit())) {
    ConvertConstructedValue(type, ctor);
    return;
  }

  if (args.empty()) {
    StrCat(GetDefaultAsString(type));
    return;
  }
  Convert(init);
}

std::string Converter::AccessLValueObject(clang::MemberExpr *member) {
  auto *object = member->getBase();
  auto type = object->getType();
  if (member->isArrow()) {
    auto *op =
        clang::dyn_cast<clang::CXXOperatorCallExpr>(object->IgnoreImplicit());
    if (op && GetStrongestIteratorCategory(op->getArg(0)->getType()) ==
                  IteratorCategory::Bidirectional) {
      return ToString(object);
    }
  }
  if (type->isPointerType() ||
      (IsReferenceType(object) && clang::isa<clang::CallExpr>(object))) {
    return std::format("({}{})", GetPointerDerefPrefix(type->getPointeeType()),
                       ToString(object));
  }
  return ToString(object);
}

bool Converter::isLValue() const {
  return curr_expr_kind_.empty() || curr_expr_kind_.back() == ExprKind::LValue;
}

bool Converter::isRValue() const {
  return curr_expr_kind_.empty() || curr_expr_kind_.back() == ExprKind::RValue;
}

bool Converter::isXValue() const {
  return !curr_expr_kind_.empty() && curr_expr_kind_.back() == ExprKind::XValue;
}

bool Converter::isAddrOf() const {
  return !curr_expr_kind_.empty() &&
         (curr_expr_kind_.back() == ExprKind::AddrOf ||
          curr_expr_kind_.back() == ExprKind::Object);
}

bool Converter::isObject() const {
  return !curr_expr_kind_.empty() && curr_expr_kind_.back() == ExprKind::Object;
}

bool Converter::isVoid() const {
  return curr_expr_kind_.empty() || curr_expr_kind_.back() == ExprKind::Void;
}

bool Converter::isCallee() const {
  return !curr_expr_kind_.empty() && curr_expr_kind_.back() == ExprKind::Callee;
}

bool Converter::ShouldReplaceWithMappedBody(clang::DeclRefExpr *expr) const {
  if (clang::isa<clang::FunctionDecl>(expr->getDecl()) && isAddrOf()) {
    return false;
  }
  return true;
}

void Converter::SetFresh() {
  switch (computed_expr_type_) {
  case ComputedExprType::Value:
    computed_expr_type_ = ComputedExprType::FreshValue;
    break;
  case ComputedExprType::Pointer:
    computed_expr_type_ = ComputedExprType::FreshPointer;
    break;
  case ComputedExprType::FreshValue:
  case ComputedExprType::FreshPointer:
    break;
  case ComputedExprType::Unknown:
    assert(0 && "Unreachable ComputedExprType::Unknown");
    break;
  case ComputedExprType::Pending:
    assert(0 && "Unreachable ComputedExprType::Pending");
    break;
  }
}

void Converter::SetValueFreshness(clang::QualType type) {
  if (TypeIsCopyable(type)) {
    computed_expr_type_ = ComputedExprType::FreshValue;
  } else if (type->isPointerType() || type->isReferenceType()) {
    computed_expr_type_ = ComputedExprType::Pointer;
  } else {
    computed_expr_type_ = ComputedExprType::Value;
  }
}

void Converter::SetFreshType(clang::QualType type) {
  computed_expr_type_ = type->isPointerType() || type->isReferenceType()
                            ? ComputedExprType::FreshPointer
                            : ComputedExprType::FreshValue;
}

void Converter::dump_expr_kinds() {
  log() << "isRValue: " << isRValue() << ", isXValue: " << isXValue()
        << ", isAddrOf: " << isAddrOf() << ", isObject: " << isObject()
        << ", isVoid: " << isVoid() << '\n';
}

void Converter::ConvertConstructedValue(clang::QualType type,
                                        clang::CXXConstructExpr *ctor) {
  ConvertVarInit(type, ctor);
}

const char *Converter::GetPointerDerefPrefix(clang::QualType pointee_type) {
  return token::kStar;
}

} // namespace cpp2rust
