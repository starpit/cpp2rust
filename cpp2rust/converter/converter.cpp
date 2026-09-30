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
#include <cctype>
#include <format>
#include <ranges>
#include <string_view>
#include <utility>

#include "compiler.h"
#include "converter/converter_lib.h"
#include "converter/lex.h"
#include "converter/mapper.h"
#include "converter/survey.h"

namespace cpp2rust {
std::unordered_map<std::string, std::string> Converter::inner_structs_;
std::unordered_set<std::string> Converter::decl_ids_;
std::unordered_set<std::string> Converter::emitted_impl_methods_;
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

// TRANSPARENT ONE-FIELD CARRIER -> the payload type it carries, or a null
// QualType when `decl` is not one.
//
// `llvm::cl::initializer<Ty>` (CommandLine.h:430) is the whole family: a class
// template whose ONLY non-static data member is `const Ty &Init`. It exists to
// give `cl::init(v)` a distinct type so the variadic `cl::opt` ctor can
// dispatch on it; it carries no state of its own and has no behaviour beyond
// `apply`, which nothing in this corpus reaches. So `initializer<Ty>` IS `Ty`
// as far as emitted Rust is concerned, and the honest lowering is to erase it.
//
// ⭐ WHY THIS IS A CONVERTER LOWERING AND NOT A RULE KEY, which is the whole
// point of the change. A rule key `template <typename T1> using tNNNN =
// llvm::cl::initializer<T1>;` was measured as a NET REGRESSION and reverted
// (rules/cl/src.cpp:121): once the key matches, the MAPPER has to map the bound
// argument T1 through `types_`, and the corpus instantiates
// `initializer<char[1]>` (from `cl::init("")`, 38 sites), for which `types_`
// had no entry -- so the key turned one loud abort into a different loud abort
// on MORE TUs.
//
// ⭐ MEASURED, and it corrects what I first wrote here: this lowering CANNOT
// regress the `char[_]` bucket, because `Converter::Convert(QualType)` consults
// `Mapper::Map` FIRST (:130) and only falls through to `TraverseType` ->
// `VisitRecordType` when the lookup came back empty. Every instantiation that
// already HAS a concrete key -- `rules/cl` t1900 `initializer<char[1]>`,
// `rules/mlir` t2600 `initializer<DCC::ProgIRFormat>` -- is therefore answered
// by its key and never reaches this function at all. So this is a pure
// FALLBACK: it can only fire where the alternative was the loud abort.
// Two probes measured that (/home/agent/work/initc.probe/):
//   carrier.cpp  `cl::init("")` + `cl::init(false)` + `cl::init(<enum>)`
//                BASE: LLVM ERROR on `initializer<Colour>`.  PATCHED: rc=0,
//                `__tmp_0: Vec<u8>` (t1900's answer, UNCHANGED), `__tmp_1:
//                bool`, `__tmp_2: Colour`.
//   noenum.cpp   the same file with the enum option removed, i.e. ONLY the
//                `char[_]` and `bool` payloads: BASE and PATCHED emissions are
//                BYTE-IDENTICAL (`diff -q` silent).
// That byte-identity is the whole answer to the reverted experiment: the rule
// key had to bind `T1` and so touched the `char[_]` sites; this does not touch
// them.
//
// ⛔ STRUCTURE IS CHECKED, NOT ASSUMED. The name alone is not enough: matching
// on a spelling and then emitting the first template argument would silently
// emit the wrong type if the class ever held more than the one reference. So
// the definition must be visible, have exactly one field, and that field must
// be an lvalue reference to the template argument. A future `initializer` that
// is not a transparent carrier therefore falls straight through to the loud
// `ReportUnmappedSystemType` path instead of being mis-lowered.
static clang::QualType TransparentCarrierPayload(clang::ASTContext &ctx,
                                                 const clang::RecordDecl *d) {
  const auto *spec = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(d);
  if (spec == nullptr) {
    return {};
  }
  if (spec->getQualifiedNameAsString() != "llvm::cl::initializer") {
    return {};
  }
  const clang::TemplateArgumentList &args = spec->getTemplateArgs();
  if (args.size() != 1 || args[0].getKind() != clang::TemplateArgument::Type) {
    return {};
  }
  const clang::CXXRecordDecl *def = spec->getDefinition();
  if (def == nullptr) {
    return {};
  }
  const clang::QualType payload = args[0].getAsType();
  clang::QualType field_type;
  int fields = 0;
  for (const auto *field : def->fields()) {
    ++fields;
    field_type = field->getType();
  }
  if (fields != 1 || !field_type->isLValueReferenceType()) {
    return {};
  }
  if (!ctx.hasSameUnqualifiedType(field_type.getNonReferenceType(), payload)) {
    return {};
  }
  return payload;
}

bool Converter::VisitRecordType(clang::RecordType *type) {
  auto *decl = type->getDecl();

  // Erase a transparent one-field carrier and emit its payload type instead.
  // Placed before the lambda arm and before the system-type refusal because a
  // carrier is neither: it is a system record with no rule that nevertheless
  // has an exact, checkable Rust spelling.
  if (const clang::QualType payload = TransparentCarrierPayload(ctx_, decl);
      !payload.isNull()) {
    Convert(payload);
    return false;
  }
  if (auto lambda = clang::dyn_cast<clang::CXXRecordDecl>(decl)) {
    if (lambda->isLambda()) {
      if (in_function_formals_) {
        // ⛔ NOT `getLambdaCallOperator()` DIRECTLY. For a GENERIC lambda
        // (`[](auto&& data) {...}`) that is the UNINSTANTIATED TEMPLATE
        // PATTERN, whose FunctionProtoType still holds clang's *unsubstituted*
        // spellings: every dependent parameter prints as `type-parameter-0-0`
        // and the deduced return type prints as the bare `auto` sugar. Neither
        // is a type the mapper can key or `TraverseType` can emit tokens for,
        // so BOTH fell through to `Convert(QualType)`'s placeholder branch
        // (:227) and the formal came out as
        //   func: *mut impl Fn(*mut Cpp2RustUnmapped_typenegparameterneg_neg_)
        //           -> Cpp2RustUnmapped_auto
        // -- MEASURED on the two generic lambdas at dsc/dataOpDsc.h:167 and
        // :171 (`FoldManager<T>::apply`), which account for ALL 44 + 44
        // surviving `Cpp2RustUnmapped_auto` /
        // `Cpp2RustUnmapped_typenegparameterneg_neg_` sites in the fresh32
        // 56-file bucket-A sweep, four lines per file across 11 files, both
        // placeholders always on the SAME line.
        //
        // This is the same question `SelectLambdaCallOperator` (:6252) already
        // answers for the lambda BODY, which is why the body converts with real
        // types while its own signature did not: the closure emitted by
        // `VisitLambdaExpr` names `Vec<i64>` in its parameter list, and the
        // formal it is passed to named a placeholder. Asking the same question
        // here makes the two agree.
        //
        // ⭐ AND WHEN CLANG HAS NO SUBSTITUTED TYPE TO GIVE, KEEP THE
        // PLACEHOLDER. A generic lambda with zero instantiated specialisations
        // in this TU, or with more than one, has no single monomorphisation to
        // name; guessing one would emit a type that is plausible and WRONG,
        // and a wrong type in a formal can silently compile, whereas the
        // placeholder is a loud E0412. So those cases fall through to the
        // pattern and keep emitting exactly what they emit today.
        const clang::CXXMethodDecl *call_op = lambda->getLambdaCallOperator();
        llvm::SmallVector<clang::CXXMethodDecl *, 4> instantiations;
        CollectLambdaCallOperatorInstantiations(lambda, instantiations);
        if (instantiations.size() == 1) {
          call_op = instantiations.front();
        }
        StrCat(ConvertFunctionPointerType(
            call_op->getType()->getAs<clang::FunctionProtoType>(),
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

// Defined below, next to `GetLifetimeBinders`'s shared scanner.
static std::string ElideNamedLifetimes(const std::string &spelling);

std::pair<std::string, std::string>
Converter::MaterializeTemp(const std::string &binding_name,
                           clang::QualType param_type, clang::Expr *expr) {
  auto pointee = param_type.getNonReferenceType();
  auto value = ConvertRValue(expr, pointee);
  auto type_str = ToStringBase(pointee);
  const auto *decl = in_const_initializer_ ? keyword::kStatic : keyword::kLet;
  // Same defect as `EmitHoistedArgs` (:4941): a PARAMETER type annotating a
  // function-body `let`. ⛔ Only the `let` arm -- a `static` is a declaration
  // and `'_` there is E0637, so a named lifetime reaching the const-initializer
  // arm is left exactly as it is and stays loud (E0261) rather than being
  // silently rewritten into something that is not the same type.
  if (decl == keyword::kLet) {
    type_str = ElideNamedLifetimes(type_str);
  }

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
  // A POINTER TO AN ABSTRACT RECORD MUST NAME THE TRAIT, AND THAT ANSWER MUST
  // NOT DEPEND ON EMISSION ORDER. `abstract_structs_` is populated by
  // `ConvertAbstractClass` (:6460), so a pointer type converted BEFORE its
  // pointee's class is converted sees an EMPTY set and falls through to the
  // bare record name. MEASURED on LoopUnroll.cpp: of the eight
  // `InheritWithClone<Base, Derived>::clone` out-of-line definitions returning
  // an abstract `Base *`, seven emitted `*mut dyn dsc2_ScheduleNode__Virtual`
  // and the one instantiated FIRST (`<ScheduleNode, BlockNode>`, dsc2.h:526,
  // inside `ScheduleNode`'s own derived-class chain) emitted the bare
  // `*mut dsc2_ScheduleNode` -- E0053 against a trait signature that came from
  // the mapper's order-free `isAbstract()` path
  // (`AddRuleForUserDefinedType`).
  //
  // So ask the definition directly, under exactly the condition `ConvertClass`
  // (:1317) emits the trait under: user-defined, convertible, abstract. This
  // is NOT a third mechanism -- `isAbstract()` is already the authority both
  // here and in the mapper; the set is kept as a floor so nothing that names a
  // trait today stops naming one.
  const auto *pointee_record = llvm::dyn_cast_or_null<clang::CXXRecordDecl>(
      pointee_type->isRecordType() ? pointee_type->getAsRecordDecl() : nullptr);
  const clang::CXXRecordDecl *pointee_def =
      pointee_record != nullptr ? pointee_record->getDefinition() : nullptr;
  if (pointee_type->isRecordType() &&
      (abstract_structs_.contains(GetID(pointee_type->getAsRecordDecl())) ||
       (pointee_def != nullptr && IsUserDefinedDecl(pointee_def) &&
        IsConvertibleCXXRecordDecl(pointee_def) && pointee_def->isAbstract()))) {
    // The trait is named `<Record>__Virtual`, which the recursive type visit
    // below cannot produce: it would emit the bare record name, and appending
    // the suffix with a trailing `StrCat` yields `dyn Name __Virtual` (token
    // spacing). So emit the trait name here and stop.
    StrCat(keyword::kDyn);
    StrCat(GetRecordName(pointee_type->getAsRecordDecl()) + "__Virtual");
    return false;
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
  StrCat(keyword_unsafe_, keyword::kFn, std::move(function_name),
         GetLifetimeBinders(decl));
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
  // The virtual-method suppression is about PARAMETERS, not locals: a virtual
  // method's signature is also emitted without a body (trait method), and `mut`
  // on a parameter pattern there is E0642 ("patterns aren't allowed in
  // functions without bodies"). It was suppressing `mut` on every LOCAL in the
  // body too, which is a compile-level defect emitted at rc=0 with no
  // placeholder token: measured on dbo/src/Transforms/PlacePrograms.cpp, whose
  // `runOnOperation() override` declares
  //   let in_order: Option<Vec<mlir_dbo_ProgramInOrder>> = ...;
  // and then reaches it through the non-const `optional::operator*` rule, which
  // lowers to `.as_mut().expect(...)` and so takes `&mut self` -- E0596. The
  // `as_mut()` is CORRECT (one of the two uses builds a `MutableArrayRef`), so
  // the missing `mut` is the bug, not the lowering.
  const bool virtual_method_parm = method_or_null != nullptr &&
                                   method_or_null->isVirtual() &&
                                   clang::isa<clang::ParmVarDecl>(decl);
  return ((hoisted_decls_.contains(decl) ||
           (!type.isConstQualified() && !type->isReferenceType())) &&
          !virtual_method_parm && !IsGlobalVar(decl) && name != "_");
}

// Scans a RENDERED Rust type spelling for lifetime names, order-stably and
// deduped. `'static` is pre-declared in every scope and `'_` is the inferred
// placeholder, so neither is ever a binder and both are skipped.
//
// Shared by the two consumers of the same question: `GetLifetimeBinders`
// (:8196), which DECLARES what it finds on a function signature, and
// `ElideNamedLifetimes` below, which REWRITES what it finds because the site it
// guards cannot declare anything.
static std::vector<std::string> ScanLifetimeNames(const std::string &spelling) {
  auto is_ident_char = [](char c) {
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
  };
  std::vector<std::string> names;
  for (size_t i = 0; i + 1 < spelling.size(); ++i) {
    if (spelling[i] != '\'') {
      continue;
    }
    size_t j = i + 1;
    // A lifetime name is `'` followed by an identifier; anything else is not
    // one (a Rust type spelling carries no character literals, but do not rely
    // on that).
    if (std::isdigit(static_cast<unsigned char>(spelling[j])) != 0 ||
        !is_ident_char(spelling[j])) {
      continue;
    }
    while (j < spelling.size() && is_ident_char(spelling[j])) {
      ++j;
    }
    std::string name = spelling.substr(i, j - i);
    i = j - 1;
    if (name == "'static" || name == "'_") {
      continue;
    }
    if (std::ranges::find(names, name) == names.end()) {
      names.push_back(std::move(name));
    }
  }
  return names;
}

// ⛔ A FUNCTION-BODY `let` TYPE ANNOTATION HAS NO BINDER SLOT. `63546ba2` added
// the `<'a>` slot to SIGNATURE emission (VisitFunctionDecl, ConvertCXXMethodDecl,
// VisitCXXConstructorDecl) so a lifetime out of a rule TARGET is declared there.
// A local `let` has nowhere to put one: Rust grammar admits no generics on a
// `let`, and the enclosing function is NOT the binder's source here -- MEASURED
// on dataflow-scheduler/.../Dialect/KTDF/KTDFTypes.cpp:580, whose emitted
// enclosing signature is
//     pub unsafe fn parse(parser: *mut mlir_AsmParser) -> ...::ir::Ty {
// with no `'a` in it at all, because the `function_ref` is a LOCAL variable in
// the C++ body, not a parameter. So the binder genuinely does not exist in
// scope and declaring one on `parse` would be a fabrication.
//
// The region is INFERRED from the initialiser, so the annotation must simply
// STOP NAMING IT -- which for a borrowed trait object means FULL ELISION, not
// the `'_` placeholder:
//     let _action: Option<&'a (dyn Fn(*mut dyn V) -> *mut dyn V + 'a)>
//     let _action: Option<& (dyn Fn(*mut dyn V) -> *mut dyn V)>
// This is the SAME TYPE, not a weaker one: behind a reference the default
// object lifetime bound IS the reference's lifetime, so `&(dyn Fn..)` means
// exactly `&'x (dyn Fn.. + 'x)`.
//
// ⛔ `'_` IS NOT A SUBSTITUTE AND THAT IS MEASURED, not assumed. Rewriting to
// `'_` in place clears the E0261s and then trades in `E0106: missing lifetime
// specifier` -- one error for two, which by this project's standard is not
// progress. rustc's own note gives the reason and it is not about elision at
// all: `-> *mut dyn V + '_` is `error: ambiguous `+` in a type`, because the
// trailing bound binds to the RETURN type `dyn V`, not to the outer `dyn Fn`.
// That ambiguity is in the rule TARGET's spelling and predates this change; the
// undeclared `'a` was masking it. Dropping the bound removes both.
//
// ⛔ NOT `'static` EITHER. `'static` on a borrow of a local is a silent
// lifetime lie: it compiles and then forces either a leak or unsoundness, and
// "it compiled" is precisely the evidence this port must not accept. rustc
// suggests exactly that here ("consider using the `'static` lifetime") and the
// suggestion is refused. Full elision asserts nothing -- it asks the borrow
// checker, which still rejects a genuinely too-short borrow.
static std::string ElideNamedLifetimes(const std::string &spelling) {
  if (ScanLifetimeNames(spelling).empty()) {
    return spelling;
  }
  auto is_ident_char = [](char c) {
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
  };
  std::string out;
  out.reserve(spelling.size());
  for (size_t i = 0; i < spelling.size(); ++i) {
    if (spelling[i] != '\'' || i + 1 >= spelling.size()) {
      out += spelling[i];
      continue;
    }
    size_t j = i + 1;
    if (std::isdigit(static_cast<unsigned char>(spelling[j])) != 0 ||
        !is_ident_char(spelling[j])) {
      out += spelling[i];
      continue;
    }
    while (j < spelling.size() && is_ident_char(spelling[j])) {
      ++j;
    }
    std::string name = spelling.substr(i, j - i);
    i = j - 1;
    if (name == "'static" || name == "'_") {
      out += name;
      continue;
    }
    // A named lifetime that is an OBJECT BOUND (`dyn Trait + 'a`) takes its `+`
    // with it: leaving a dangling `+` is a syntax error, and the bound is what
    // the default object lifetime bound will now supply.
    size_t k = out.size();
    while (k > 0 && std::isspace(static_cast<unsigned char>(out[k - 1])) != 0) {
      --k;
    }
    if (k > 0 && out[k - 1] == '+') {
      out.resize(k - 1);
    }
  }
  return out;
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
  }
  // Only a `let` binding may carry `'_`: a `static`/`static mut` item, like a
  // signature, is a DECLARATION whose type must name real regions, and `'_`
  // there is E0637 ("`'_` cannot be used here"). So the elision below is gated
  // on having emitted `let`, not merely on being local.
  bool emitted_let = false;
  if (decl->isFileVarDecl()) {
    // handled above
  } else if (decl->isStaticLocal()) {
    StrCat(keyword::kStatic, keyword_mut_);
  } else if (decl->isLocalVarDecl()) {
    StrCat(keyword::kLet);
    emitted_let = true;
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
    // Rendered through the virtual `ToString`, so this observes exactly the
    // text `Convert(qual_type)` is about to emit and contributes none of it --
    // the same redirect `GetLifetimeBinders` uses.
    //
    // ⭐ THE PROBE IS ONLY A PROBE. If no named lifetime is present -- which is
    // the case for all but a handful of spellings in the whole corpus -- fall
    // through to `Convert(qual_type)` so the emitted bytes are UNCHANGED.
    // Re-emitting the rendered string instead would have retokenised every
    // `let` in the corpus: measured on dsc/dims.cpp as 90 whitespace-only
    // changed lines (`: T = x` -> `: T  = x`), a diff with no defect in it that
    // would have buried the two lines that matter.
    if (emitted_let) {
      auto spelling = ToString(qual_type);
      if (ScanLifetimeNames(spelling).empty()) {
        Convert(qual_type);
      } else {
        StrCat(ElideNamedLifetimes(spelling));
      }
    } else {
      Convert(qual_type);
    }
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
// The prvalue a reference holder's initializer binds to a TEMPORARY, or nullptr
// when the holder aliases an object that outlives the declaration.
//
// ⭐ WHY THIS DISTINCTION IS THE WHOLE RELAXATION.  The scalars-only filter on
// the reference-holder path below exists to stop a by-value lowering from
// SILENTLY DROPPING A WRITE through an alias (see the REFERENCE paragraph
// above).  When the holder is `const auto &` bound to a PRVALUE there is no
// alias to drop: the pair is a temporary that nothing else can name, its
// lifetime is extended only to cover the bindings, and clang gives the
// BindingDecls plain non-reference `tuple_element_t` types -- i.e. C++ ITSELF
// COPIES each element out of the temporary.  A Rust MOVE out of a by-value
// tuple is therefore equal-or-better fidelity than the C++ copy it replaces,
// and it is sound for a CLASS-typed element, which is exactly the case the
// scalars-only filter was refusing.
//
// MEASURED SITE (util/variabledefinition/VariableDefinition.cpp:310, the last
// remaining abort on that TU):
//     const auto& [_, inserted] = defs.emplace(sym, std::move(newExpression));
//   DecompositionDecl 'const std::pair<std::__hash_map_iterator<..>, bool> &'
//   BindingDecl _        'const std::__hash_map_iterator<..>'  <- CLASS type
//   BindingDecl inserted 'const bool'
// `rules/unordered_map` f57 supplies the `emplace` -> `pair<iterator, bool>`
// call and `rules/pair` t1 models the pair as a BARE RUST 2-TUPLE, so `.0`/`.1`
// are the only sound emission and both elements are reachable by FIELD ACCESS.
//
// ⛔ WHY NOT THE POINTER-HOLDER FORM (`EmitVectorDecompositionBindings`, i.e.
// `&raw const (*holder).N`) THAT THE TWO BRANCHES ABOVE USE.  Both of those
// holders are pointers into storage that ALREADY EXISTS and outlives the
// statement -- a container node, or a `*def` the caller owns.  Here the holder
// IS the temporary, so a `*const` holder would have to be `&<call>()` and would
// depend on Rust temporary-lifetime extension to not dangle; and every binding
// would become a raw pointer needing a per-use deref, which for `inserted`
// means the LOAD-BEARING bool is read through a pointer at every `if` that
// tests it.  A by-value holder needs neither, and reuses the by-value emission
// already at the bottom of this function unchanged -- so this relaxation adds
// NO new emission machinery at all and still emits only `.0` / `.1`, never a
// method call, and so cannot fabricate an unmapped member.
//
// ⛔ AND THE BOOL IS NOT DISCARDED OR INVERTED: `inserted` is bound to `.1` of
// the very tuple f57 returns, in source order, with no negation anywhere on
// this path -- `DT_CHECK(inserted)` at the measured site keeps testing exactly
// what `emplace` reported.  The iterator half stays an IDENTITY because f57
// builds it with `find_key` on the LIVE map, and moving that iterator value out
// of a dead temporary pair does not copy the node it points at.
// ⭐ MEMBER-WISE STRUCTURED BINDING ([dcl.struct.bind]/4), the second of the two
// forms the standard defines and the one this converter had never recognised.
// Returns the FieldDecl each binding names, in declaration order, or an EMPTY
// vector when the decomposition is not member-wise (or is one this function
// refuses).
//
// HOW IT IS TOLD APART FROM THE TUPLE-LIKE FORM (/3).  For a tuple-like holder
// clang synthesises one HOLDING VarDecl per binding, initialised to
// `std::get<I>(__d)`; for the member-wise form there are NO holding vars at all
// and each binding IS a member of the decomposed object.  `getHoldingVar() ==
// nullptr` for every binding is therefore an exact discriminator, and it is
// checked FIRST so a tuple-like holder can never be spelled by field name.
//
// WHY THIS MATTERS: the positional spelling `.0` / `.1` is WRONG for a plain
// struct.  `struct RowGroupNodeInfo { ScheduleNode* node; int row;
// CoordinateBaseType beta; }` (ddc/ddc.h:566) is emitted as a Rust struct with
// NAMED fields, so `(*h).0` is `E0609: no field 0`.  The bindings correspond
// 1:1, in order, to the non-static data members, so the field NAME is the only
// sound spelling -- and it needs no rule and no tuple model at all.
//
// ⛔ WHAT IS REFUSED, and why each exclusion is load-bearing:
//  - A REFERENCE field.  This is the single guard that keeps the `llvm::
//    enumerate` family (`llvm::detail::enumerator_result<size_t, X &>`, four
//    measured TUs) out: its aliasing lives in a reference MEMBER while clang
//    hands the BindingDecls plain non-reference types, so the aliasing is
//    invisible in the binding types.  `PcfgInfo` (dsc/dsc2Pcfg.h:102, `SenPcfg&
//    pcfg;`) is excluded by the very same test.  A reference member is also
//    modelled pointer-shaped, so `&raw const (*h).f` would be one level off.
//    (enumerator_result is excluded a SECOND time independently: it is tuple-
//    like, so it never reaches this function at all.)
//  - A BASE CLASS, an anonymous struct/union member, a bitfield, a union, or a
//    field count that does not equal the binding count.  Member-wise binding
//    over any of those either is not what this simple 1:1 pairing models or has
//    no addressable field to point at.
//  - A record that HAS A RULE (`Mapper::Contains`).  Then the Rust type is
//    whatever the rule says and its field names are not the C++ ones.
std::vector<const clang::FieldDecl *>
GetMemberwiseBindingFields(const clang::DecompositionDecl *decl) {
  const std::vector<const clang::FieldDecl *> kNotMemberwise;
  auto bindings = decl->bindings();
  if (bindings.empty()) {
    return kNotMemberwise;
  }
  for (const auto *binding : bindings) {
    if (binding->getHoldingVar() != nullptr) {
      return kNotMemberwise;
    }
  }
  auto value_type = decl->getType().getNonReferenceType();
  const clang::CXXRecordDecl *record = value_type->getAsCXXRecordDecl();
  if (record == nullptr) {
    return kNotMemberwise;
  }
  // ⛔ THE RECORD MUST BE ONE THIS TU EMITS AS ITS OWN RUST STRUCT, because that
  // is the only case in which the Rust type carries the C++ FIELD NAMES.
  //
  // ⚠️ MEASURED, and it is exactly the mis-recording trap this construct is
  // known for: the obvious test `!Mapper::Contains(value_type)` is USELESS here.
  // `Contains` is TRUE for a struct defined in the TU itself -- `search()` falls
  // through to `LooksLikeUserDefinedTypeName` (mapper.cpp:2366) and synthesises
  // an IDENTITY rule from `user_tags_` -- so gating on `!Contains` rejected
  // every member-wise site and the whole arm measured as "no effect" on all four
  // target TUs while the code was in fact reached. Instrumented to reject-3 to
  // find that, then replaced with the predicate that actually answers the
  // question: is this an identity-mapped USER tag (emitted as a Rust struct with
  // these field names) rather than a rule-modelled library type (whose Rust
  // shape and field names are whatever the rule says)?
  if (!Mapper::LooksLikeUserDefinedTypeName(
          Mapper::ToString(value_type.getUnqualifiedType()), nullptr)) {
    return kNotMemberwise;
  }
  record = record->getDefinition();
  if (record == nullptr || record->isUnion() ||
      record->getNumBases() != 0 || record->getNumVBases() != 0) {
    return kNotMemberwise;
  }
  std::vector<const clang::FieldDecl *> fields;
  for (const auto *field : record->fields()) {
    if (field->isAnonymousStructOrUnion() || field->isBitField() ||
        field->getType()->isReferenceType()) {
      return kNotMemberwise;
    }
    fields.push_back(field);
  }
  if (fields.size() != bindings.size()) {
    return kNotMemberwise;
  }
  return fields;
}

// The number of TOP-LEVEL elements of a parenthesised Rust tuple model, or 0
// when `model` is not one. Used as the arity gate that replaces the old "with
// exactly two bindings a parenthesised model can only be a 2-tuple" argument,
// which stops holding once more than two bindings are allowed.
//
// ⚠️ A comma nested inside `(`/`<`/`[` is NOT a separator: `(A, HashMap<K, V>)`
// has TWO elements and counting bare commas would say three. `>` is treated as
// a closer ONLY while an angle bracket is open, so the `>` of a `->` in a
// fn-pointer model cannot desync the depth -- the measured failure mode that
// killed a whole family of rule keys elsewhere in this project.
static unsigned RustTupleModelArity(const std::string &model) {
  if (model.size() < 2 || model.front() != '(' || model.back() != ')') {
    return 0;
  }
  int round = 0;
  int square = 0;
  int angle = 0;
  unsigned elements = 1;
  for (std::size_t i = 1; i + 1 < model.size(); ++i) {
    const char c = model[i];
    switch (c) {
      case '(': ++round; break;
      case ')': --round; break;
      case '[': ++square; break;
      case ']': --square; break;
      case '<': ++angle; break;
      case '>':
        // Not a closer when it is the tail of `->`, and not a closer when no
        // angle bracket is open.
        if (angle > 0 && !(i > 0 && model[i - 1] == '-')) {
          --angle;
        }
        break;
      case ',':
        if (round == 0 && square == 0 && angle == 0) {
          ++elements;
        }
        break;
      default: break;
    }
    if (round < 0 || square < 0) {
      return 0;
    }
  }
  if (round != 0 || square != 0 || angle != 0) {
    return 0;
  }
  // `()` is the unit type, not a 1-tuple.
  if (model == "()") {
    return 0;
  }
  return elements;
}

static const clang::Expr *GetTemporaryHolderInit(const clang::Expr *init) {
  if (init == nullptr) {
    return nullptr;
  }
  // Deliberately NOT IgnoreParenImpCasts: an implicit cast between the
  // temporary and the reference would be the very node that tells us a
  // conversion happened, and only these two wrappers are known-transparent.
  const clang::Expr *expr = init->IgnoreParens();
  if (const auto *cleanups = clang::dyn_cast<clang::ExprWithCleanups>(expr)) {
    expr = cleanups->getSubExpr()->IgnoreParens();
  }
  if (const auto *temp =
          clang::dyn_cast<clang::MaterializeTemporaryExpr>(expr)) {
    return temp->getSubExpr();
  }
  return nullptr;
}

// Defined with the by-value decomposition emitter it was written for (:3153).
// Forward-declared because the mutable-reference-holder arm of
// ConvertTupleDecompositionDecl below reuses it rather than re-deriving it --
// getting this predicate wrong is a silently-dropped write, so there must be
// exactly one copy of it.
static bool HolderHoldsReference(clang::QualType holder_type);

bool Converter::ConvertTupleDecompositionDecl(clang::DecompositionDecl *decl) {
  // Only a local `let` is lowered: a file-scope or static-local decomposition
  // would need one `static mut` per binding plus the init hoisting that goes
  // with it, and no measured shape is one.
  if (!decl->isLocalVarDecl() || decl->isStaticLocal() || !decl->hasInit()) {
    return false;
  }
  auto bindings = decl->bindings();
  // ⭐ ARITY IS NO LONGER FIXED AT TWO. Every downstream emitter carries its own
  // arity gate -- `EmitMapDecompositionBindings` still refuses anything but two
  // (its accessor table has two entries), `EmitVectorDecompositionBindings`
  // gates on the member-wise field count or on the tuple model's counted arity,
  // and the by-value/const-ref path at the bottom of this function does the
  // same. So widening here cannot let an unspelled shape through.
  if (bindings.empty()) {
    return false;
  }
  // ---- MAP-ELEMENT branch, routed to the MAP-ACCESSOR lowering ----
  // Measured shape (dsc/dims.cpp:731, dsc2.cpp:3707, dsc2.h:88, inside
  // DataStructDims::pruneMaxSymbolicVolumes):
  //     const auto &[symDims, volumeLimit] = *it;   // it = map.begin()
  //   DecompositionDecl  'const std::pair<const std::set<PrimaryDimTypes>, int> &'
  // The first binding is a `std::set`, i.e. a RecordType, so the scalars-only
  // test on the tuple-index path below refuses it -- and it MUST keep refusing
  // it: `let symDims = (*holder).0;` is a MOVE OUT OF A RAW-POINTER DEREF
  // (E0507, the error measured at LiveRange.cpp:97). Relaxing that test would
  // need `&(*holder).0` plus ptr-binding registration, i.e. it collapses into
  // this lowering anyway. So handle the map case HERE, by the accessors:
  //     let symDims = it.first();        // *const K
  //     let volumeLimit = it.second();   // *mut V
  // plus a ptr_bindings_ registration so VisitDeclRefExpr (:4529) derefs at
  // every use. That binds the `std::set` BY RAW POINTER -- zero clone, zero
  // move -- and the body's uses (`symDims.begin()/.end()`, `for (const auto
  // &symDim : symDims)`) go through the same per-use deref the map range-for
  // already relies on. It does NOT depend on the holder being const: nothing
  // is copied, so there is no write to drop, and `second()` already hands back
  // a `*mut V`.
  //
  // DISCRIMINATOR. `IsMapLikeRangeClass` (:2485) cannot be reused: it tests the
  // RANGE class (`std::map`/`std::unordered_map`), and here there is no range --
  // only an iterator, whose spelling is an INTERNAL standard-library name (see
  // the measured note below). Nor can Mapper answer for it: the iterator type has
  // NO rule at all (grep either spelling over the rules tree = zero hits),
  // because the map for-range never asks -- it picks the Rust iterator name from
  // the CONTAINER via MapRangeIteratorName(:2490). So the iterator is identified
  // by its record name, AND by the map `value_type` signature
  // `std::pair<const K, V>`: the const-qualified FIRST element is what separates
  // a map/unordered_map node from a `std::set` node, whose iterator shares the
  // same internal class template family.
  if (const auto *deref = clang::dyn_cast<clang::CXXOperatorCallExpr>(
          decl->getInit()->IgnoreParenImpCasts());
      deref != nullptr &&
      deref->getOperator() == clang::OverloadedOperatorKind::OO_Star &&
      deref->getNumArgs() == 1) {
    const std::string iter_class =
        GetClassName(deref->getArg(0)->getType().getNonReferenceType());
    // MEASURED, and an earlier plan got this wrong by reading an `-ast-dump`
    // taken with generic flags instead of the per-TU flags from the compile DB:
    // this toolchain parses dsc/dims.cpp against LIBC++, where the iterator is
    // `std::__map_iterator`, NOT libstdc++'s `std::_Rb_tree_iterator`. Both
    // families are listed so the predicate does not depend on which standard
    // library a TU's flags select.
    const bool map_iter = iter_class == "std::__map_iterator" ||
                          iter_class == "std::__map_const_iterator" ||
                          iter_class == "std::__hash_map_iterator" ||
                          iter_class == "std::__hash_map_const_iterator" ||
                          iter_class == "std::_Rb_tree_iterator" ||
                          iter_class == "std::_Rb_tree_const_iterator" ||
                          iter_class == "std::__detail::_Node_iterator" ||
                          iter_class == "std::__detail::_Node_const_iterator";
    auto value_type = decl->getType().getNonReferenceType();
    bool map_value = false;
    if (map_iter && GetClassName(value_type) == "std::pair") {
      if (const auto *spec =
              clang::dyn_cast_or_null<clang::ClassTemplateSpecializationDecl>(
                  value_type->getAsCXXRecordDecl())) {
        const auto &args = spec->getTemplateArgs();
        map_value = args.size() == 2 &&
                    args[0].getKind() == clang::TemplateArgument::Type &&
                    args[0].getAsType().isConstQualified();
      }
    }
    // Re-emitting the iterator expression once per accessor is only sound if it
    // is side-effect-free, so the operand is restricted to a plain variable
    // reference -- which is the measured shape. Anything else stays on the loud
    // refusal path rather than being evaluated twice.
    const auto *iter_ref = clang::dyn_cast<clang::DeclRefExpr>(
        deref->getArg(0)->IgnoreParenImpCasts());
    if (map_value && iter_ref != nullptr &&
        clang::isa<clang::VarDecl>(iter_ref->getDecl())) {
      // REFCOUNT IS REFUSED here for the same reason as on the tuple path
      // below: that model wraps every local in `Rc<RefCell<..>>`, so the
      // per-use deref these bindings need is not what it would emit. Gate
      // BEFORE any emission, and register ptr_bindings_ only after we have
      // committed to emitting, so the refcount model never inherits derefs for
      // bindings it did not emit.
      // ⭐⭐ ROW g3036 OPENS THIS ARM TO REFCOUNT, and corrects the reasoning the
      // paragraph above used to carry. The premise was "that model wraps every
      // local in `Rc<RefCell<..>>`, so the per-use deref these bindings need is
      // not what it would emit" -- which conflates TWO different derefs. The
      // bindings do not need a per-use deref BECAUSE they are pointers; they
      // need it because the UNSAFE accessors return `*const K` / `*mut V`. The
      // refcount `MapIterator` impl (libcc2rs/src/iterators.rs:151 for
      // `RefcountMapIter`, :413 for `RefcountHashMapIter`) returns
      // `Value<K>` / `Value<V>` -- which IS this model's native form for a
      // local -- so the SAME two accessor calls are correct and NO
      // `ptr_bindings_` registration may be made. That is exactly the split
      // `c151701d` established for the decomposing map RANGE-for
      // (ConverterRefCount::VisitCXXForRangeStmtMap); this is the standalone
      // `let` twin of it, and it needs nothing new in libcc2rs and no new key.
      //
      // ⛔ THE ONE THING THAT IS NOT SHARED WITH THE RANGE-FOR ARM, and the
      // reason a model hook is needed at all rather than reusing `iter_text`:
      // there the receiver is the `for` pattern variable, a BARE
      // `RefcountMapIter<K, V>`, so `<iter>.first()` type-checks. HERE the
      // receiver is a user LOCAL, and this model boxes it --
      // MEASURED, refcount, pin/cpp2rust 12820a5d + pin/ir.v44, on
      // `for (auto it = m.begin(); it != m.end();)` over
      // `std::map<std::set<int>, int>`:
      //     let it: Value<RefcountMapIter<std::collections::BTreeSet<i32>, i32>>
      //         = Rc::new(RefCell::new(RefcountMapIter::begin(..)));
      //     ... (*it.borrow()).first() ... (*it.borrow_mut()).prefix_inc()
      // so a bare `it.first()` would be `E0599` on `Rc<RefCell<..>>` -- rc=0
      // output that does not compile, i.e. the silent class this project
      // refuses. The receiver spelling is therefore asked of the MODEL.
      //
      // ⭐ AND THE `std::set` FIRST COMPONENT OF THIS ROW'S WITNESS NEEDS
      // NOTHING SPECIAL, which is where the unsafe and refcount stories diverge
      // most. Under unsafe a container element is what forced this whole branch
      // to exist (`let symDims = (*holder).0;` is E0507, a move out of a
      // raw-pointer deref). Under refcount `first()` is generic in K and hands
      // back `Value<K>` for any K, so a `std::set` component is spelled
      // IDENTICALLY to a scalar one: `Value<BTreeSet<PrimaryDimTypes>>`, and
      // the body's `symDims.begin()` / `for (const auto &d : symDims)` go
      // through this model's ordinary boxed-local path.
      //
      // ALIASING, which is what `const auto &[k, v] = *it` asks for, and it is
      // the landed range-for arm's argument verbatim because it is the same two
      // accessor bodies: `second()` returns `m.get(key).clone()` on a
      // `BTreeMap<K, Value<V>>`, and cloning an `Rc` SHARES the cell, so
      // `volumeLimit` aliases the map's element exactly as the C++ `const int &`
      // does. `first()` returns `Rc::new(RefCell::new(key.clone()))`, i.e. a
      // FRESH cell holding a deep copy -- sound because a map key is `const` in
      // C++ (`value_type` is `pair<const K, V>`), so no well-formed program can
      // write through `k` and observe the difference.
      const bool unsafe_model =
          keyword_unsafe_ != nullptr && *keyword_unsafe_ != '\0';
      // ⛔ GATED BEFORE ANY EMISSION. The hook builds a STRING (the base
      // implementation captures the conversion into a `Buffer`, exactly as the
      // inline code it replaced did), and an empty return is a REFUSAL that
      // leaves the loud diagnostic in place having emitted nothing.
      const std::string iter_text = DecompositionMapIterReceiver(
          const_cast<clang::DeclRefExpr *>(iter_ref));
      if (iter_text.empty()) {
        return false;
      }
      if (!EmitMapDecompositionBindings(decl, iter_text)) {
        return false;
      }
      // NOT scoped. ScopedPtrBindings (converter.h:1081) is an RAII guard whose
      // lifetime is a for-range BODY conversion; a standalone DeclStmt has its
      // uses in the REST of the enclosing CompoundStmt, outside VisitDeclStmt's
      // dynamic extent, so there is no region to hang a guard on. A plain
      // insert is correct instead: a BindingDecl* is unique per source site, and
      // pointer-ness is a property of the lowering we just emitted, not of a
      // scope.
      //
      // ⛔⛔ UNSAFE ONLY. On the refcount arm the two bindings are already
      // `Value<..>`, so registering them would make `VisitDeclRefExpr` (:4529)
      // emit `(*symDims)` for a non-pointer -- `E0614` at rc=0. That is not
      // merely redundant, it is positively wrong, and it is the same trap
      // `VisitCXXForRangeStmtIndexBased` documents for a by-value loop variable.
      if (unsafe_model) {
        for (const auto *binding : bindings) {
          ptr_bindings_.insert(binding);
        }
      }
      return true;
    }
  }
  // ---- RAW-POINTER-DEREF branch, routed to the FIELD-POINTER lowering ----
  // Measured shape (util/variabledefinition/VariableDefinition.cpp:95, inside
  // VariableDefinition::exportJsonStr, where `def` comes from
  // `for (const auto* def : sorted)`):
  //     const auto& [var, expr] = *def;
  //   DecompositionDecl 'const std::pair<const long, VariableDefinition::ExprType> &'
  // The deref of a RAW POINTER is a plain UnaryOperator, NOT a
  // CXXOperatorCallExpr, so the map-iterator branch above is never entered; and
  // the second element is a CLASS type, so the scalars-only filter on the
  // const-ref-holder path below refuses it -- correctly, because that path
  // COPIES the holder and a class element would need a clone.
  //
  // Nothing is copied or moved HERE: each binding becomes a raw pointer to a
  // field of the pointed-to pair, exactly as `EmitVectorDecompositionBindings`
  // already does for a vector element, so a class-typed element is sound and
  // cannot hit E0507. `std::pair` is modelled as a BARE RUST 2-TUPLE
  // (`rules/pair/tgt_unsafe.rs`), so `.0`/`.1` -- and NOT any `.first()` /
  // `.second()` accessor, which do not exist anywhere in the rules tree -- is
  // the only sound emission. The accessors the map branch above emits live on
  // the map ITERATOR (libcc2rs/src/iterators.rs), not on a pair, so that
  // branch's style must NOT be copied here.
  if (const auto *deref = clang::dyn_cast<clang::UnaryOperator>(
          decl->getInit()->IgnoreParenImpCasts());
      deref != nullptr && deref->getOpcode() == clang::UO_Deref &&
      decl->getType()->isLValueReferenceType() &&
      decl->getType().getNonReferenceType().isConstQualified()) {
    // Re-emitting the pointer expression once per binding is only sound if it
    // is evaluation-free, so the operand is restricted to a plain variable
    // reference of pointer type -- the measured shape. Anything else stays on
    // the loud refusal path rather than being evaluated twice.
    const auto *ptr_ref = clang::dyn_cast<clang::DeclRefExpr>(
        deref->getSubExpr()->IgnoreParenImpCasts());
    bool bindings_ok = true;
    for (const auto *binding : bindings) {
      if (binding->getType()->isReferenceType()) {
        bindings_ok = false;
      }
    }
    if (bindings_ok && ptr_ref != nullptr &&
        clang::isa<clang::VarDecl>(ptr_ref->getDecl()) &&
        ptr_ref->getType()->isPointerType()) {
      auto value_type =
          decl->getType().getNonReferenceType().getUnqualifiedType();
      // The model must be a Rust tuple for `.0` / `.1` to mean the C++
      // elements. With exactly two bindings a parenthesised model can only be a
      // 2-tuple -- any other arity would not have type-checked in C++.
      if (Mapper::Contains(value_type)) {
        const std::string model = Mapper::Map(value_type);
        if (model.size() >= 2 && model.front() == '(' && model.back() == ')') {
          // REFCOUNT IS REFUSED for the same reason as on both paths around
          // this one: that model wraps every local in `Rc<RefCell<..>>`, so the
          // per-use deref these bindings need is not what it would emit. Gate
          // BEFORE any emission, and register ptr_bindings_ only after we have
          // committed to emitting.
          if (keyword_unsafe_ == nullptr || *keyword_unsafe_ == '\0') {
            return false;
          }
          std::string ptr_text;
          {
            Buffer buf(*this);
            Convert(const_cast<clang::Expr *>(
                static_cast<const clang::Expr *>(ptr_ref)));
            ptr_text = std::move(buf).str();
          }
          if (!EmitVectorDecompositionBindings(decl, ptr_text)) {
            return false;
          }
          // NOT scoped, for the same reason as the map branch above (:895): a
          // standalone DeclStmt has its uses in the REST of the enclosing
          // CompoundStmt, outside any guard's dynamic extent, and a
          // BindingDecl* is unique per source site.
          for (const auto *binding : bindings) {
            ptr_bindings_.insert(binding);
          }
          return true;
        }
      }
    }
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
  bool ref_holder = type->isReferenceType();
  // ⭐ TEMPORARY-HOLDER ARM. See GetTemporaryHolderInit above for why a
  // reference holder bound to a prvalue is lowered BY VALUE and why that makes
  // a CLASS-typed binding sound here when the scalars-only filter below
  // (correctly) refuses one for a holder that aliases live storage.
  const bool temp_holder =
      ref_holder && type->isLValueReferenceType() &&
      type.getNonReferenceType().isConstQualified() &&
      GetTemporaryHolderInit(decl->getInit()) != nullptr;
  // ⭐ THE BINDINGS THAT GET THE POINTER FORM instead of a by-value element read.
  // Only ever populated on an LVALUE-REFERENCE-holder arm below -- the non-scalar
  // elements of a const-reference holder, and EVERY element of a mutable one;
  // empty means the emission is byte-for-byte what it was before this set existed.
  std::unordered_set<const clang::BindingDecl *> ptr_form;
  // ⭐ A MUTABLE lvalue-reference holder. Drives `*mut` instead of `*const` on
  // the holder annotation and `&raw mut` instead of `&raw const` on every
  // binding. Only ever set on the mutable arm below; false means every emitted
  // byte is exactly what it was before this flag existed.
  bool mut_holder = false;
  if (temp_holder) {
    for (const auto *binding : bindings) {
      // A reference binding still aliases observably; those stay on the loud
      // refusal path exactly as before.
      if (binding->getType()->isReferenceType()) {
        return false;
      }
    }
    type = type.getNonReferenceType();
    ref_holder = false;
  } else if (ref_holder) {
    if (!type->isLValueReferenceType()) {
      return false;
    }
    auto pointee = type.getNonReferenceType();
    // ⭐ THE MUTABLE-REFERENCE HOLDER. Measured shape (ddc/ddcv1.cpp:1495, inside
    // `ddc::Ddc::exploreAssignDataStages`, the first abort of that TU at
    // 18a29078):
    //     auto& [dim, size] = tn->unitTimeTransferChunkSize_[i].sizeDim_;
    //   DecompositionDecl 'dsc2::ScheduleNode::Size &'   (dsc/dsc2.h:486)
    //   BindingDecl dim   'PrimaryDimTypes'   <- enum, NOT const, NOT a reference
    //   BindingDecl size  'int'               <- scalar, NOT const
    // This is the `let` path (VisitDeclStmt -> here), NOT a range-for: neither
    // VisitCXXForRangeStmt nor EmitVectorDecompositionBindings is on it.
    //
    // ⛔⛔ EVERY BINDING TAKES THE POINTER FORM ON THIS ARM, WITHOUT EXCEPTION,
    // AND THAT IS THE WHOLE CORRECTNESS ARGUMENT. In C++ `dim` and `size` are
    // lvalues that NAME `h.dim_` and `h.size_`, so `size = 4;` writes THROUGH to
    // the container element. The const-reference arm below may read a scalar
    // element by value (`(*h).N`) precisely because every binding there is const
    // and no write can travel back; here writes can and do travel back, so a
    // by-value read would SILENTLY DROP THEM -- a lowering that is strictly worse
    // than the loud abort and the exact failure this construct is policed for.
    // `&raw mut (*h).field` plus a `ptr_bindings_` registration makes every use a
    // deref of a `*mut` into the container element, so a write reaches the
    // container and object identity is preserved. Nothing is copied, so E0507
    // cannot arise for a class-typed element either, and the scalar/record
    // distinction the const arm needs does not apply.
    //
    // ⛔ THE ALIASING-HOLDER SUBSET IS STILL REFUSED, via the same
    // `HolderHoldsReference` the by-value arm uses (:3153) -- do not re-derive
    // it. `llvm::detail::enumerator_result<size_t, X &>` and
    // `DscPcfgTranslator::PcfgInfo` (member `SenPcfg &pcfg`) hold a reference, so
    // clang hands NON-reference binding types while the C++ still names the
    // original through that member. Refusing them here keeps this arm's
    // guarantee -- "the pointer names the very object the holder names" -- true
    // by construction instead of by inspection.
    const bool mut_ref_holder = !pointee.isConstQualified();
    if (mut_ref_holder) {
      if (HolderHoldsReference(type)) {
        return false;
      }
      for (const auto *binding : bindings) {
        auto bt = binding->getType();
        // A reference BINDING aliases observably on top of the holder's own
        // aliasing and is refused on this construct everywhere else.
        if (bt->isReferenceType()) {
          return false;
        }
        // ⛔ Same restriction as the const arm, and for the same reason: an
        // array element would need `&raw mut` of an array plus a decayed use, and
        // a member-pointer / vector / complex element has no established model
        // here. Those stay on the LOUD refusal path rather than being guessed at.
        if (!bt->isPointerType() && !bt->isEnumeralType() &&
            !bt->isIntegralType(ctx_) && !bt->isFloatingType() &&
            !bt->isRecordType()) {
          return false;
        }
        ptr_form.insert(binding);
      }
      mut_holder = true;
      type = pointee;
    } else {
    for (const auto *binding : bindings) {
      auto bt = binding->getType();
      if (bt->isReferenceType() || !bt.isConstQualified()) {
        return false;
      }
      if (bt->isPointerType() &&
          !bt->getPointeeType().isConstQualified()) {
        return false;
      }
      // ⭐ NON-SCALAR ELEMENT: THE POINTER FORM, not a by-value read and NOT a
      // copy. A scalar element is read by value (`(*h).N`) because the copy is
      // bit-for-bit and needs no clone; a CLASS-typed element read the same way
      // is `E0507: cannot move out of a dereference of a raw pointer` -- rc=0
      // with Rust that does not compile, which is the failure mode this whole
      // construct is policed for. So spell it the way the map branch (:1140) and
      // `EmitVectorDecompositionBindings` (:3075) already spell an aliased
      // element:
      //     let sdscName = &raw const (*__decomp_2753_17).0;   // *const Vec<c_char>
      // plus a `ptr_bindings_` registration, so `VisitDeclRefExpr` (:5417)
      // derefs at every use. NOTHING IS COPIED and NOTHING IS MOVED: the
      // binding is a raw pointer INTO the very object the C++ reference names,
      // which is also why OBJECT IDENTITY is preserved exactly -- the hazard a
      // clone would have introduced cannot arise. Every binding on this arm is
      // const-qualified (checked above) and the holder is a `const` lvalue
      // reference, so no write can travel back through the alias either.
      //
      // MEASURED WITNESS (dsc/sdsc-perfmodel/perfmodel.cpp:2753, the first abort
      // of that TU after a7217888):
      //     const auto& [sdscName, opCategory, idealCycles] = perfData[i];
      //   DecompositionDecl 'const std::tuple<std::string, std::string, long> &'
      //   BindingDecl sdscName     'const std::string'  <- RECORD, pointer form
      //   BindingDecl opCategory   'const std::string'  <- RECORD, pointer form
      //   BindingDecl idealCycles  'const long'         <- scalar, by value
      // i.e. the two forms MIX within one decomposition, which is why this is a
      // per-binding set and not a whole-decl flag.
      //
      // ⛔ RESTRICTED TO A RECORD TYPE. An array element would need `&raw const`
      // of an array and a decayed use, a member-pointer or a vector/complex
      // element has no established model here, and each of those stays on the
      // LOUD refusal path rather than being guessed at.
      if (!bt->isPointerType() && !bt->isEnumeralType() &&
          !bt->isIntegralType(ctx_) && !bt->isFloatingType()) {
        if (!bt->isRecordType()) {
          return false;
        }
        ptr_form.insert(binding);
      }
    }
    // Everything below is written against the VALUE type.
    type = pointee;
    }
  } else {
    for (const auto *binding : bindings) {
      if (binding->getType()->isReferenceType()) {
        return false;
      }
    }
  }
  // ⭐ MEMBER-WISE ARM ([dcl.struct.bind]/4). A plain struct has NO tuple model
  // at all, so the `Mapper::Contains` + parenthesised-model gate below refuses
  // it -- and refusing it was correct only as long as the emission could spell
  // the elements ONLY positionally. It can now spell them by FIELD NAME, which
  // is what the bindings of a member-wise decomposition actually name.
  //
  // MEASURED SITE (dsc-based-utils/DSC2ToDataflowIR/V3/SNControlFlowLowering.cpp:911):
  //     const auto &[dim, kind] = node->dims_[dim_idx];
  //   DecompositionDecl 'const PrimaryDimAndKind &'   (dsc/dims.h:76)
  //   BindingDecl dim   'const PrimaryDimTypes'   <- enum, NOT a reference
  //   BindingDecl kind  'const MetaDimKind'       <- enum, NOT a reference
  // The holder is a CONST reference, so this arm inherits the const-ref
  // reasoning verbatim: the holder becomes a `*const PrimaryDimAndKind` and each
  // binding is a READ THROUGH THAT POINTER (`(*h).dim_`), not a copy of the
  // struct -- so there is nothing to clone, nothing to move, and no write can
  // travel back because every binding is const.
  //
  // ⛔ HOW THIS ARM EXCLUDES THE `llvm::enumerate` FAMILY, which must stay
  // refused: `llvm::detail::enumerator_result` is TUPLE-LIKE (it has `get<I>`),
  // so `GetMemberwiseBindingFields` returns empty for it on the holding-var test
  // alone and it never reaches here; and independently, its aliasing lives in a
  // reference MEMBER, which that helper refuses outright.
  const std::vector<const clang::FieldDecl *> memberwise_fields =
      GetMemberwiseBindingFields(decl);
  if (memberwise_fields.empty()) {
    if (!Mapper::Contains(type.getUnqualifiedType())) {
      return false;
    }
    // The model must be a Rust tuple of EXACTLY this arity for `.N` to mean the
    // C++ elements. (The old argument -- "with exactly two bindings a
    // parenthesised model can only be a 2-tuple" -- does not survive allowing a
    // third binding, so the arity is now counted, not inferred.)
    if (RustTupleModelArity(Mapper::Map(type.getUnqualifiedType())) !=
        bindings.size()) {
      return false;
    }
  }

  // ⭐⭐ REFCOUNT, SECOND STEP: THE POINTER-FORM SUB-ARMS OF THE
  // LVALUE-REFERENCE HOLDER ARE NOW LOWERED TOO -- the MUTABLE reference
  // holder, and the const holder with a RECORD-typed element. They were left
  // refused by the first step (the const/scalar arm) on the stated ground that
  // `&raw mut/const (*h).N` + a `ptr_bindings_` registration "has no
  // established refcount form". MEASURED, that is not a missing form -- it is a
  // form that does not EXIST in this model, because the distinction the
  // pointer form makes does not exist here:
  //
  //   * In the unsafe model the element read `(*h).N` is a MOVE OUT OF the pair
  //     (E0507 for a record) and a COPY (a dropped write, for a mutable
  //     holder), so a record/mutable element has to be taken by raw pointer.
  //   * In the refcount model the very same place `(*h.upgrade().deref()).N`
  //     has type `Value<T>` = `Rc<RefCell<T>>` (`rules/pair/tgt_refcount.rs`
  //     maps `std::pair` to a BARE 2-tuple `(Value<T1>, Value<T2>)`, not a
  //     boxed tuple), and `DecompositionHolderElement` clones it. CLONING AN
  //     `Rc` SHARES THE CELL. So the binding is already a handle ON the pair's
  //     own element -- nothing is copied and nothing is moved, which is
  //     exactly the property the raw pointer was introduced to obtain.
  //
  // ⭐⭐ WHY A WRITE TRAVELS BACK, which is the whole correctness question on
  // the MUTABLE arm and is NOT the same argument the const arm used (there,
  // every binding is const and there is no write to lose):
  //     auto &[coord, data] = pr;   //  pr : std::pair<std::deque<long>,
  //     coord.push_back(3);         //       std::vector<long>> &
  //   ->
  //     let h: Ptr<(Value<Deque<i64>>, Value<Vec<i64>>)> = ..;
  //     let coord = (*h.upgrade().deref()).0.clone();
  //     coord.borrow_mut().push_back(3);
  // `coord` and the pair's `.0` are two `Rc` handles to ONE `RefCell`, so
  // `borrow_mut()` through either reaches the same storage: the write is
  // visible through the pair, and through the container the pair lives in,
  // because this model never copies a `Value<T>` -- it copies the handle. The
  // C++ `coord` is a `std::deque<long> &` naming `pr.first`; the Rust `coord`
  // names the same cell `(*h..).0` names. OBJECT IDENTITY IS PRESERVED IN BOTH
  // DIRECTIONS, read and write. (This is also why `mut` is NOT emitted for such
  // a binding: the handle is never reassigned, mutation goes through the cell.)
  //
  // ⛔ AND THE POINTER FORM IS POSITIVELY WRONG HERE, not merely unnecessary: a
  // `ptr_bindings_` registration makes `VisitDeclRefExpr` (:6530) deref every
  // use, and `(*coord)` where `coord : Value<T>` is not a raw pointer is a hard
  // rustc error. So `ptr_form` is CLEARED for this model rather than special
  // cased at each use site -- one statement, and every downstream test
  // (`ptr_form.contains`, the registration loop) then behaves as it already
  // does for the const/scalar arm that is known to work.
  //
  // ⭐ REFCOUNT: THE LVALUE-REFERENCE HOLDER ARM IS LOWERED; every
  // other arm of this function stays refused for that model.
  //
  // `keyword_unsafe_` is the only model discriminator the base class has --
  // ConverterRefCount passes "" for it (converter_refcount.cpp:33) and
  // ConverterUnsafe passes "unsafe".
  //
  // WHAT THE OLD BLANKET REFUSAL GOT RIGHT, and why it still applies to the
  // BY-VALUE holder: that model wraps a named local in `Rc<RefCell<..>>`, so
  // `let h = <tuple init>;` comes out as
  // `Rc<RefCell<(Rc<RefCell<i32>>, Rc<RefCell<bool>>)>>` and `h.0` is
  // `E0609: no field `0``. That is a real measured refusal and it is kept.
  //
  // WHAT IT GOT WRONG: on a REFERENCE holder this function does NOT let the
  // model box the local. It writes the annotation itself (`ref_holder` branch
  // below) and reads each element through it, so the two model-specific pieces
  // are just the pointer spelling and the deref spelling -- and the refcount
  // model already has both, MEASURED by asking the converter to translate the
  // hand-written equivalent of the target shape
  //     const std::pair<const FoldDimProp *, BaseFuncType> &h = dim_prop_.at(i);
  //     const FoldDimProp *fp_dim = h.first;
  // which it lowers, with no change of any kind, to
  //     let h: Ptr<(Value<*const FoldDimProp>, Value<BaseFuncType>)> = ..;
  //     .. (*h.upgrade().deref()).0 ..
  // i.e. `std::pair` is a BARE 2-TUPLE OF `Value<..>` in this model
  // (`rules/pair/tgt_refcount.rs`: `(Value<T1>, Value<T2>)`), NOT a boxed
  // tuple, so there is no cell to borrow before the index and no E0609. The
  // holder is `Ptr<T>` (ConverterRefCount::VisitReferenceType, :197) and the
  // deref carries `.upgrade().deref()`
  // (ConverterRefCount::GetPointerDerefSuffix, :2959) -- both supplied by the
  // two hooks this arm now goes through.
  //
  // ALIASING. `(*h..).N` is a place of type `Value<..>` = `Rc<RefCell<..>>`;
  // the refcount hook appends `.clone()`, and cloning an `Rc` SHARES the cell,
  // so the binding names the very object the pair element names -- object
  // identity preserved, nothing copied, nothing moved. (The `.clone()` is not
  // optional: moving the `Rc` out of a deref of a `Ptr` is `E0507`.) It is
  // also strictly stronger than C++ needs here: every binding on this arm is
  // const-qualified (checked above), so no write can travel back through the
  // alias in either language. That is the same argument as the map-range
  // decomposition's `second()`, which likewise clones an `Rc<RefCell<V>>`.
  //
  // ⛔ EVERY OTHER ARM STAYS LOUD, gated here BEFORE any emission so a refusal
  // leaves no partial text:
  //   * the BY-VALUE holder (`ref_holder` false, which includes the
  //     temporary-holder arm that deliberately clears it): the E0609 case
  //     above.
  //   * the MEMBER-WISE arm: `(*h).field_` on a refcount struct reaches an
  //     `Rc<RefCell<..>>` FIELD, which is a different spelling again and has
  //     no measured witness under this model.
  const bool refcount_model =
      keyword_unsafe_ == nullptr || *keyword_unsafe_ == '\0';
  // See the POINTER-FORM block above: in this model the element read is already
  // a shared-`Rc` handle, so the raw-pointer form is both unnecessary and
  // unspellable. Remember that it WAS non-empty, because the annotation check
  // below is what keeps a record element with no model from being emitted as a
  // `let` whose type does not exist.
  const bool ptr_form_cleared =
      refcount_model && ref_holder && memberwise_fields.empty() &&
      !ptr_form.empty();
  if (ptr_form_cleared) {
    ptr_form.clear();
  }
  // ⭐⭐ ROW g3086 OPENS THE **BY-VALUE** HOLDER TO REFCOUNT, and the blanket
  // refusal above (kept verbatim for every other arm) named the right error for
  // the wrong reason. Its text was: "that model wraps a named local in
  // `Rc<RefCell<..>>`, so `let h = <tuple init>;` comes out as
  // `Rc<RefCell<(Rc<RefCell<i32>>, Rc<RefCell<bool>>)>>` and `h.0` is `E0609:
  // no field `0``."
  //
  // MEASURED, `pin/cpp2rust a0b0a707` + `pin/ir.v45`, on the hand-written
  // equivalent (`verif/g3086/pv.cpp`, refcount leg):
  //     std::pair<long, bool> p = std::make_pair(7L, true);
  //   ->  let p: Value<(Value<i64>, Value<bool>)> = Rc::new(RefCell::new((..)));
  //       let a: Value<i64> = Rc::new(RefCell::new((*(*p.borrow()).0.borrow())));
  // so the boxing is REAL -- but it is not the obstacle, because this function
  // writes the holder's annotation ITSELF and reads each element itself. The
  // only thing that was missing is that the annotation was written as
  // `Convert(type)` (the UNBOXED tuple model) while `ConvertVarInit` on this
  // model emits `BoxValue(..)` (converter_refcount.cpp:3185) -- an `E0308`
  // mismatch, not an `E0609`. Both halves now go through a hook, so the model
  // spells its own `Value<..>` annotation and its own element read.
  //
  // ⭐ WHY A DEEP COPY IS THE CORRECT ELEMENT READ HERE, and why that is NOT the
  // argument the reference-holder arm uses. `auto [a, b] = expr;` COPIES `expr`
  // into a holder that C++ gives no name to, and the bindings name that
  // holder's members. Nothing else in the program can reach the holder, so
  // there is no aliasing to preserve and no write to lose: a fresh cell holding
  // a clone is observationally identical to sharing the (unreachable) holder's
  // cell. That is the opposite of the reference-holder arm, where `.clone()` of
  // the element's `Rc` must SHARE the cell because the holder names live
  // storage. `ConverterRefCount::VisitCXXForRangeStmtVectorDecomposition`
  // (converter_refcount.cpp:2634ff) draws exactly this line for the by-value
  // loop variable of a decomposing vector for-range, with the same reasoning
  // and the same emitted spelling.
  //
  // ⛔ EVERY OTHER ARM STAYS LOUD, still gated before any emission: the
  // TEMPORARY-holder arm (`temp_holder` clears `ref_holder`, so it must be
  // excluded explicitly -- its holder is a prvalue bound to a `const &`, a
  // combination with no measured refcount witness), and the MEMBER-WISE arm
  // (`(*h).field_` reaches an `Rc<RefCell<..>>` FIELD, a third spelling again).
  const bool refcount_by_value =
      refcount_model && !ref_holder && !temp_holder &&
      memberwise_fields.empty() &&
      DecompositionValueHolderSupported(type.getUnqualifiedType(),
                                        bindings.size());
  if (refcount_model && !(ref_holder && memberwise_fields.empty()) &&
      !refcount_by_value) {
    return false;
  }

  // ⭐ THE ELEMENT-MODEL GATE for the temporary-holder arm, vetted BEFORE any
  // emission. `Mapper::Map` is NOT usable for this: measured by `rules/
  // unordered_map` f57, `Mapper::Map(std::pair<__hash_map_iterator<..>, bool>)`
  // answers the UNSUBSTITUTED template text `(T1, T2)`, so it says nothing
  // about whether the elements themselves have models -- the substitution
  // happens inside `Convert(QualType)`. And `Mapper::Contains` on the ELEMENT is
  // the wrong test in the other direction: it is false for a class that has no
  // rule but IS emitted as a struct in this same TU, which is a perfectly good
  // model. So convert the holder annotation into a throwaway Buffer and require
  // it to be placeholder-free. That is exactly "every element's own model is a
  // mapped value type", tested on the text that will actually be emitted.
  //
  // ⛔ WHY IT IS A GATE AND NOT A WARNING: `Cpp2RustUnmapped_` in a type
  // annotation is loud, but a CLASS-typed element is precisely the case the
  // scalars-only filter used to refuse, so accepting one whose model does not
  // exist would trade a loud abort for a `let` of a nonexistent type. Refuse
  // instead and let the original abort stand.
  // The member-wise arm is gated the same way, and for the same reason: the
  // struct has no rule by construction (GetMemberwiseBindingFields refuses one
  // that does), so the ONLY evidence its Rust model exists is that its own
  // annotation comes out placeholder-free.
  // ⭐ AND THE POINTER-FORM ARM, for the identical reason: a class-typed element
  // is exactly the case the scalars-only filter used to refuse, so `&raw const
  // (*h).N` must not be emitted unless that element HAS a model -- otherwise a
  // loud abort would be traded for a `let` whose type does not exist. The holder
  // annotation is the right thing to test because `Convert(QualType)` SUBSTITUTES
  // the element types (`Mapper::Map` does not -- it answers the unsubstituted
  // `(T1, T2)` template text), so a placeholder-free holder annotation is
  // precisely "every element's own model is a mapped value type".
  // ⭐ `ptr_form_cleared` is in this test because clearing the set must not
  // remove the check the set used to trigger: the elements that were in it are
  // exactly the record-typed ones, i.e. the ones whose own model can be
  // missing. Without it a refcount record element with no rule would emit
  // `let x = (*h.upgrade().deref()).0.clone();` against a holder annotation
  // containing `Cpp2RustUnmapped` -- rc=0 and dead at rustc, the one outcome
  // that is worse than the abort.
  // ⭐ `refcount_by_value` is in this test for the same reason `ptr_form_cleared`
  // is: the element read this arm emits is a `.clone()` of a place whose type is
  // the element's own model, so an element with NO model would trade a loud
  // abort for a `let` of a type that does not exist. It is refcount-only, so no
  // unsafe byte moves.
  if (temp_holder || !memberwise_fields.empty() || !ptr_form.empty() ||
      ptr_form_cleared || refcount_by_value) {
    std::string annotation;
    {
      Buffer buf(*this);
      Convert(type);
      annotation = std::move(buf).str();
    }
    if (annotation.find("Cpp2RustUnmapped") != std::string::npos) {
      return false;
    }
  }

  HoistMaterializedTempBindings hoist_temps(*this);
  // A DecompositionDecl has no name of its own.
  const std::string holder = GetDecompositionIterName(decl);
  StrCat(keyword::kLet, holder, token::kColon);
  // Annotate: without the type, `let t = <init>;` left one measured case at
  // `E0282: type annotations needed` because the tuple element types are only
  // pinned by the (separate) binding statements.
  // A REFERENCE holder is annotated as a RAW POINTER TO the model, not as the
  // model itself. MEASURED (goal TU, three sites): `dim_prop_.at(i)` lowers
  // through `rules/vector`'s POINTER-returning `at`, so a value annotation is
  // `E0308 expected tuple, found *mut _` and `holder.0` on a raw pointer is
  // `E0609`/`E0614` -- Rust does not auto-deref a raw pointer for field access.
  // `Convert(decl->getType())` cannot be used to get the `*const`:
  // `Convert(QualType)` consults `Mapper::Map` FIRST (:126), which answers for
  // the REFERENCE type with the *value* model and drops the reference-ness, and
  // `VisitReferenceType` is only reached when no rule matched. So build the
  // annotation the way `VisitPointerType` (:498) does: sigil, then pointee.
  // Pointer-holder + deref-at-use is the convention the converter already
  // ships in `EmitVectorDecompositionBindings` and `VisitCXXForRangeStmtMap`.
  if (ref_holder) {
    // A MUTABLE reference holder annotates `*mut`, so the `&mut <init>`
    // ConvertVarInit already emits for a non-const reference QualType (it tests
    // `IsMut`, converter_lib.cpp:269) coerces to it. `&mut T` -> `*mut T` is a
    // built-in coercion at a `let` with an explicit annotation; `&mut T` to
    // `*const T` would also coerce, but would then make the binding pointers
    // `*const` and no write could reach the container.
    // Through the hook, so the refcount model can spell its own `Ptr<T>`; the
    // base implementation is the `*const`/`*mut` + `Convert(type)` that used to
    // be inlined here, so the unsafe emission is unchanged byte for byte.
    EmitDecompositionHolderAnnotation(type, mut_holder);
  } else {
    // Through the hook, so the refcount model can spell its own boxed
    // `Value<..>` annotation and match what its `ConvertVarInit` emits for the
    // init below. The base implementation is the `Convert(type)` that used to be
    // inlined here, so the unsafe emission is unchanged byte for byte.
    EmitDecompositionValueHolderAnnotation(type);
  }
  StrCat(token::kAssign);
  // Hand the ORIGINAL reference QualType to ConvertVarInit so its
  // reference-reconciliation branch (:6170) is reachable: when the init is NOT
  // already a reference/pointer it inserts `&`, and `&T` coerces to `*const T`
  // at a `let` with an explicit annotation. Passing the stripped value type
  // suppressed that branch entirely, which is how the E0308 got emitted.
  ConvertVarInit(ref_holder ? decl->getType() : type, decl->getInit());
  StrCat(token::kSemiColon);

  unsigned index = 0;
  for (const auto *binding : bindings) {
    const std::string binding_name = GetNamedDeclAsString(binding);
    // ⛔ `let mut _ = ..;` IS NOT LEGAL RUST -- rustc rejects it with "`mut`
    // must be followed by a named binding", because `_` is a wildcard PATTERN
    // and not an identifier. `_` is a real identifier in C++ and is the
    // conventional spelling for the element of a structured binding the caller
    // does not use, so it reaches here verbatim: measured at
    // VariableDefinition.cpp:310, `const auto& [_, inserted] = ..`. Emit the
    // wildcard without `mut`; it discards the element, which is what the source
    // asked for. If the C++ did name `_` and then USE it, VisitDeclRefExpr
    // spells it `_`, which is a hard rustc error in expression position -- i.e.
    // that case FAILS LOUDLY rather than being silently miscompiled.
    if (binding_name == "_") {
      StrCat(keyword::kLet, "_", token::kAssign);
      // ⭐ A REFCOUNT reference-holder binding is a `Value<..>` = an
      // `Rc<RefCell<..>>` handle that is never REASSIGNED -- writes go through
      // `borrow_mut()` -- so `mut` here would only be an `unused_mut`. That is
      // how this model already spells its own locals
      // (`let r: Value<i32> = Rc::new(RefCell::new(0));`, measured on the
      // hand-written equivalent of the target shape).
    } else if (ptr_form.contains(binding) ||
               (ref_holder && refcount_model) || refcount_by_value) {
      // A pointer-form binding is a const alias that is never reassigned, so
      // `mut` would only be an `unused_mut` warning -- and this is the same
      // plain `let` that `EmitVectorDecompositionBindings` already emits for the
      // identical `&raw const` form. `ptr_form` is empty for every shape that was
      // accepted before, so this cannot move an existing byte.
      StrCat(keyword::kLet, binding_name, token::kAssign);
    } else {
      StrCat(keyword::kLet, keyword::kMut, binding_name, token::kAssign);
    }
    // The holder is a raw pointer when it came from a reference, so the element
    // is reached through a deref. Reading through the pointer (rather than
    // copying the holder first) is also what keeps the binding observing the
    // aliased pair -- the bindings are all const, so no write travels back.
    // MEMBER-WISE decompositions are spelled by FIELD NAME; tuple-like ones by
    // INDEX. See the member-wise arm above: the bindings of a member-wise
    // decomposition name the non-static data members in declaration order, and
    // the Rust struct carries those same names, so `.0` would be E0609.
    const std::string element =
        memberwise_fields.empty()
            ? std::to_string(index)
            : GetNamedDeclAsString(memberwise_fields[index]);
    // A NON-SCALAR element is taken BY POINTER (`&raw const (*h).N`), never read
    // by value: the by-value read of a class element is E0507. On the MUTABLE
    // arm every element is taken by pointer regardless of scalarness, because a
    // by-value read would drop a write. `ptr_form` is only ever non-empty on an
    // lvalue-reference-holder arm, so `ref_holder` is necessarily true here and
    // `(*h)` is the correct base.
    if (ptr_form.contains(binding)) {
      // ⭐ `&raw mut` on the mutable-holder arm, so `(*binding) = v` at every use
      // WRITES INTO the container element -- see the arm's comment above for why
      // anything else silently drops the write.
      StrCat(std::format("&raw {} (*{}).{}", mut_holder ? "mut" : "const",
                         holder, element));
    } else {
      // Through the hook, so the refcount model can add its own
      // `.upgrade().deref()` and the `.clone()` that shares the element's
      // `Rc`; the base implementation is the `(*h).N` that used to be inlined
      // here, so the unsafe emission is unchanged byte for byte.
      // Both arms go through a hook now: the reference-holder one so refcount
      // can add `.upgrade().deref()` + the sharing `.clone()`, and the
      // BY-VALUE one so it can mint a FRESH cell holding a deep copy (see the
      // `refcount_by_value` block above for why a copy -- not a shared `Rc` --
      // is what `auto [a, b] = expr;` means). Both base implementations are the
      // text that was inlined here, so the unsafe emission is unchanged.
      StrCat(ref_holder
                 ? DecompositionHolderElement(holder, element, type)
                 : DecompositionValueHolderElement(holder, element, type));
    }
    StrCat(token::kSemiColon);
    ++index;
  }
  // Register the pointer-form bindings only AFTER the emission is committed, so
  // a path that refused above can never leave a stale deref behind. NOT scoped,
  // for the same reason as the map branch (:1145): a standalone DeclStmt has its
  // uses in the REST of the enclosing CompoundStmt, outside any RAII guard's
  // dynamic extent, and a BindingDecl* is unique per source site.
  for (const auto *binding : ptr_form) {
    ptr_bindings_.insert(binding);
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
      // An abstract class emits BOTH: a `<Name>__Virtual` trait carrying the
      // virtual methods, AND the ordinary struct for the record itself. The
      // struct is required -- an abstract C++ base still has fields, nested
      // enums, static members and non-virtual methods, and derived records
      // name it. So fall through to `EmitRustStructOrUnion` instead of
      // returning.
      //
      // Do NOT re-add a nested-enum loop here: `EmitRustStructOrUnion` (:1108)
      // already opens by visiting every nested `EnumDecl`, so a loop at this
      // point would emit each nested enum twice (E0428). The loop that used to
      // live here existed *only* because this path never reached
      // `EmitRustStructOrUnion`.
      ConvertAbstractClass(decl);
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
  // An out-of-line definition whose (record, emitted name) pair was ALREADY
  // emitted -- typically by the in-class definition in the header, seen in a
  // sibling TU of the same `--dir` crate -- would land as a second
  // `impl <Record> { fn <name> }`, which is E0201. `decl_ids_` cannot see this
  // because `GetMethodID` embeds the source location. Skip the redefinition,
  // never the first definition.
  if (decl->isOutOfLine() &&
      emitted_impl_methods_.contains(EmittedMethodKey(decl))) {
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
  // ALREADY inside `impl <same record> { ... }` (ConvertCXXMethodDecls opened it
  // and is now walking ForEachTemplateInstantiatedMethod): emit the method as a
  // member of THAT block. Opening a second `impl` here nests one item-level
  // block inside another, which Rust rejects outright -- and because rustfmt is
  // the pipeline's only Rust parser, the whole TU then yields no `.rs` at all.
  // Measured 2026-09-29 on dcc/src/Analysis/{LoopTree,OperationTree}.cpp and
  // .../VectorChainToSentientPT/Analysis/LoopMaskTree.cpp: 7 nested blocks each,
  // whole-TU translation otherwise complete, all three rc=1 solely on this.
  if (open_item_block_record_ == GetRecordName(decl->getParent())) {
    return ConvertCXXMethodDecl(decl);
  }
  StrCat(keyword::kImpl, GetRecordName(decl->getParent()));
  PushBrace impl_brace(*this);
  return ConvertCXXMethodDecl(decl);
}

std::string Converter::EmittedMethodKey(const clang::CXXMethodDecl *decl) {
  return GetRecordName(decl->getParent()) + "::" + GetMethodName(decl);
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
  // Record the emitted identity for every method that gets a BODY in an impl
  // block (trait declarations and pure virtuals are signatures, not
  // definitions, so they must not claim the pair).
  if (!decl->isPureVirtual() && method_target_ != MethodTarget::TraitDecl) {
    emitted_impl_methods_.insert(EmittedMethodKey(decl));
  }
  StrCat(keyword_unsafe_, keyword::kFn, GetMethodName(decl),
         GetLifetimeBinders(decl));

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

// True when `stmt` reads through `this` anywhere in its subtree.  Detecting
// `CXXThisExpr` covers both emission entry points (ConvertMemberExpr's
// base_is_this arm and VisitCXXThisExpr).
static bool StmtMentionsThis(const clang::Stmt *stmt) {
  llvm::SmallVector<const clang::Stmt *, 16> work{stmt};
  while (!work.empty()) {
    const clang::Stmt *cur = work.pop_back_val();
    if (cur == nullptr) {
      continue;
    }
    if (clang::isa<clang::CXXThisExpr>(cur)) {
      return true;
    }
    // ⛔ `CXXDefaultInitExpr::children()` IS EMPTY -- the NSDMI it stands for
    // hangs off the FieldDecl, not off this node.  Measured: without this hop
    // the `dsc/designSpaceConfig.cpp` witness was missed entirely, because
    // clang materialises an IMPLICIT CXXCtorInitializer per NSDMI field and its
    // getInit() is exactly a CXXDefaultInitExpr, so the walk terminated at
    // depth 0 and reported "no `this`" for the 240-site initializer that
    // demonstrably emits `&mut this.N_.in__`.  Same shape for a defaulted
    // argument.
    if (const auto *die = clang::dyn_cast<clang::CXXDefaultInitExpr>(cur)) {
      work.push_back(die->getExpr());
      continue;
    }
    if (const auto *dae = clang::dyn_cast<clang::CXXDefaultArgExpr>(cur)) {
      work.push_back(dae->getExpr());
      continue;
    }
    for (const clang::Stmt *child : cur->children()) {
      work.push_back(child);
    }
  }
  return false;
}

// True when the field-initializer list this constructor will EMIT (i.e. the one
// EmitConstructorFieldInits selects: mem-initializer first, NSDMI second,
// type-default last) contains an initializer that reads through `this`.
//
// ⛔ WHY THIS EXISTS -- the use-before-binding defect this predicate gates.
// ConvertCXXConstructorBody's normal shape is
//     let mut this = Self { f: <init>, ... };
// so an initializer that reads `this` is emitted INSIDE the very literal that
// binds `this`.  Measured on `dsc/designSpaceConfig.cpp` (emitted line ~22861):
// `DesignSpaceConfig`'s 240-entry NSDMI `paramNameToVal = {{"nin", &N_.in_}, ...}`
// emits `(&mut this.N_.in__ as *mut f64).into()` inside that literal -- an E0425
// use of `this` before its `let` completes, at rc=0, today.
//
// The pointers genuinely ALIAS the fields (C++ stores &N_.in_, not a copy), so
// the SEMANTICS are right and only the emission ORDER is wrong.  C++ gets away
// with it because the object is constructed AT ITS FINAL ADDRESS; `-> Self` has
// no in-place construction guarantee, which is why the two-phase rewrite
//     let mut __s = Self{..}; __s.f = &mut __s.g; __s
// is NOT the fix: it compiles and is silently wrong, because returning `__s` by
// value moves the object and leaves every raw pointer aimed at the dead frame.
// The shape that works is an in-place constructor (`new_at(*mut Self)`) whose
// receiver already has its final address, with `new() -> Self` delegating to it
// so that every existing call site -- `Type::new()` as an EXPRESSION nested in
// an enclosing struct literal, and `impl Default { fn default() -> Self {
// unsafe { Type::new() } } }` -- keeps working unchanged.
static bool CtorEmitsThisBearingFieldInit(clang::CXXConstructorDecl *decl) {
  if (decl->isDelegatingConstructor()) {
    // A delegating constructor runs no field initializers of its own.
    return false;
  }
  auto *definition =
      clang::dyn_cast_or_null<clang::CXXConstructorDecl>(decl->getDefinition());
  if (definition == nullptr) {
    return false;
  }
  for (const auto *field : decl->getParent()->fields()) {
    const clang::CXXCtorInitializer *ctor_initializer = nullptr;
    for (const auto *init : definition->inits()) {
      if (init->isMemberInitializer() && init->getMember() == field) {
        ctor_initializer = init;
        break;
      }
    }
    // An IMPLICIT in-class-init entry stands for the NSDMI; look at the NSDMI
    // itself, which is what ConvertVarInit ultimately emits.
    const clang::Expr *emitted =
        (ctor_initializer != nullptr &&
         !ctor_initializer->isInClassMemberInitializer())
            ? ctor_initializer->getInit()
            : field->getInClassInitializer();
    if (emitted != nullptr && StmtMentionsThis(emitted)) {
      return true;
    }
  }
  return false;
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
  auto ctor_name = GetCtorName(decl);

  // ⭐ IN-PLACE ARM.  See CtorEmitsThisBearingFieldInit above for why `-> Self`
  // cannot express this constructor at all.  `in_trait_body_` is excluded because
  // `MaybeUninit::<Self>::uninit()` in a trait default body needs `Self: Sized`,
  // which a trait does not imply; an abstract class with a `this`-bearing NSDMI
  // would therefore trade one error for another.
  if (!in_trait_body_ && CtorEmitsThisBearingFieldInit(decl)) {
    EmitInPlaceConstructor(decl, ctor_name);
    return false;
  }

  StrCat(keyword_unsafe_, keyword::kFn, ctor_name, GetLifetimeBinders(decl));
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

// `{name}_at(__cc2_this: *mut Self, ..)` -- the real constructor -- plus
// `{name}(..) -> Self` delegating to it.  The split is what makes a
// `this`-bearing field initializer expressible: inside `_at` the object already
// has its final address, so `this` is bound (to `&mut *__cc2_this`) BEFORE any
// initializer runs, and `&mut this.f` is a well-formed pointer to the field of
// the object being constructed rather than a read of a not-yet-bound `let`.
//
// The whole literal is still built in ONE `ptr::write`, deliberately: the field
// ADDRESSES are what the initializers capture, and those are fixed by
// `__cc2_this` alone, so overwriting the (uninitialised) contents afterwards
// leaves every captured pointer valid.  A field-by-field two-phase assignment
// would instead need a default value for each aliased field and would drop any
// field whose type has no `Default`.
//
// ⚠️ The `-> Self` wrapper necessarily MOVES the finished object out of the
// local slot, so for a self-referential class the wrapper is a fidelity
// compromise, not a fix; `_at` is the entry point that is actually correct, and
// it is emitted so that a caller which owns a place can use it.  The wrapper
// exists because every measured call site is an EXPRESSION producing a value
// into an enclosing literal (`<DataStructDims>::default()` nested in another
// struct literal, `impl Default ... unsafe { T::new() }`), i.e. no caller owns a
// place -- which is exactly why an `__init_nsdmi(&mut self)` convention that
// demands one does not fit and was not taken.
void Converter::EmitInPlaceConstructor(clang::CXXConstructorDecl *decl,
                                       const std::string &ctor_name) {
  // Mirror ConvertFunctionParameters: the DEFINITION's parameter names are the
  // ones its emitted signature will use, so forward exactly those.
  const clang::FunctionDecl *definition = decl->getDefinition();
  if (definition == nullptr) {
    definition = decl;
  }

  StrCat(keyword_unsafe_, keyword::kFn, ctor_name + "_at",
         GetLifetimeBinders(decl));
  {
    PushParen paren(*this);
    StrCat("__cc2_this: *mut Self", token::kComma);
    ConvertFunctionParameters(decl);
  }
  {
    PushBrace brace(*this);
    // ⛔ `this` here is a `&mut Self`, NOT the `let mut this = Self { .. }` VALUE
    // that ConvertCXXConstructorBody emits, so VisitCXXThisExpr's `&raw mut this`
    // is a `*mut &mut Self` -- one indirection too many.  Measured: a BARE `*this`
    // in a this-bearing NSDMI (`CElem c{*this, 3}`) came out as
    // `&mut (*&raw mut this)`, i.e. `&mut &mut Self` where a `*mut Self` is
    // wanted, = E0308.  ConvertMemberExpr was unaffected because its base_is_this
    // arm spells the receiver `this` itself and `this.field` is right either way,
    // which is why 68dbf8c6's MemberExpr witness did not expose it.
    PushThisIsMutRef push_this(*this, true);
    EmitFunctionPreamble(decl);
    StrCat(keyword::kLet, "this", token::kColon, "&mut Self", token::kAssign,
           "&mut *__cc2_this", token::kSemiColon);
    StrCat("::std::ptr::write");
    {
      PushParen write_paren(*this);
      StrCat("__cc2_this", token::kComma);
      StrCat("Self");
      {
        PushBrace this_init(*this);
        EmitConstructorFieldInits(decl);
      }
    }
    StrCat(token::kSemiColon);
    ConvertBodyStmts(decl->getBody());
  }

  if (!in_trait_body_) {
    ConvertFunctionQualifiers(decl);
  }
  StrCat(keyword_unsafe_, keyword::kFn, ctor_name, GetLifetimeBinders(decl));
  {
    PushParen paren(*this);
    ConvertFunctionParameters(decl);
  }
  StrCat(token::kArrow, "Self");
  {
    PushBrace brace(*this);
    StrCat("let mut __cc2_slot = ::std::mem::MaybeUninit::<Self>::uninit()",
           token::kSemiColon);
    StrCat(std::format("Self::{}_at", ctor_name));
    {
      PushParen paren(*this);
      StrCat("__cc2_slot.as_mut_ptr()", token::kComma);
      for (auto *parameter : definition->parameters()) {
        StrCat(GetNamedDeclAsString(parameter), token::kComma);
      }
      if (decl->isVariadic()) {
        StrCat("__args", token::kComma);
      }
    }
    StrCat(token::kSemiColon);
    StrCat("__cc2_slot.assume_init()");
  }
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
// ⛔ THE PERMANENT SUBSET: a holder that itself HOLDS A REFERENCE.
// `llvm::detail::enumerator_result<size_t, mlir::Value &>` and
// `DscPcfgTranslator::PcfgInfo` (dsc/dsc2Pcfg.h:102, member `SenPcfg &pcfg`) are
// the measured members. Such a holder COPIES ITS ALIAS: in C++ a by-value copy
// of it still names the original object through the reference member, so writes
// through the bindings DO propagate -- and a Rust by-value lowering would
// silently drop them. Clang is no help at the binding level: it hands plain
// NON-REFERENCE binding types even for an aliasing holder, so the aliasing is
// invisible there and has to be read off the HOLDER type instead. Both spellings
// are covered: a reference TEMPLATE ARGUMENT (`tuple<Value, BlockArgument &>`,
// `pair<A, B &>`, `enumerator_result<size_t, X &>`) and a reference DATA MEMBER
// (`PcfgInfo`). Refuse both; the loud abort is the correct answer for them.
static bool HolderHoldsReference(clang::QualType holder_type) {
  auto value_type = holder_type.getNonReferenceType();
  const clang::CXXRecordDecl *record = value_type->getAsCXXRecordDecl();
  if (record == nullptr) {
    return false;
  }
  if (const auto *spec =
          clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(record)) {
    for (const auto &arg : spec->getTemplateArgs().asArray()) {
      if (arg.getKind() == clang::TemplateArgument::Type &&
          arg.getAsType()->isReferenceType()) {
        return true;
      }
    }
  }
  const clang::CXXRecordDecl *def = record->getDefinition();
  if (def == nullptr) {
    // No definition means the members cannot be inspected, so the reference
    // question cannot be answered -- treat that as "holds one" and refuse.
    return true;
  }
  for (const auto *field : def->fields()) {
    if (field->getType()->isReferenceType()) {
      return true;
    }
  }
  return false;
}

bool Converter::EmitVectorDecompositionBindings(
    const clang::DecompositionDecl *decl, const std::string &holder_name) {
  auto bindings = decl->bindings();
  if (bindings.empty()) {
    return false;
  }
  // ⭐ THE BY-VALUE LOOP VARIABLE, measured as the shape that now dominates this
  // row. `for (auto [node, refcount] : an->allocUsers_)` (dsc/dsc2.cpp:936,
  // ddc/ddcv1.cpp:52, ddc/ddc_fold.cpp:902, dsc/pcfg.cpp:2341) gives a
  // NON-reference DecompositionDecl, and this emitter used to refuse it outright
  // -- which is where all four of those TUs aborted at HEAD.
  //
  // ⭐ FOR A BY-VALUE HOLDER THE POINTER FORM WOULD BE WRONG, not merely
  // unnecessary: `ConvertLoopVariable`'s non-reference branch emits
  // `range[i].clone()`, i.e. the holder is a FRESH LOCAL COPY, exactly as the C++
  // by-value loop variable is a fresh copy of the element. So the sound spelling
  // is a plain `holder.N` / `holder.field` read of that local -- no `(*h)`, no
  // `&raw`, and NO ALIASING AT ALL, which is precisely what the C++ means: a
  // write through one of these bindings must NOT reach the container, and does
  // not. Nothing is copied twice and nothing is cloned here: reading a field out
  // of a local is a partial MOVE, which Rust allows for a local (unlike the
  // move-out-of-raw-pointer-deref E0507 that forces the pointer form on an
  // aliasing holder).
  const bool by_value = !decl->getType()->isReferenceType();
  if (by_value) {
    // A reference BINDING would alias observably and is refused on this
    // construct everywhere else for the same reason.
    for (const auto *binding : bindings) {
      if (binding->getType()->isReferenceType()) {
        return false;
      }
    }
    // ⛔ And the permanent subset -- an aliasing holder whose alias survives the
    // copy. See HolderHoldsReference above.
    if (HolderHoldsReference(decl->getType())) {
      return false;
    }
  }
  // ⭐ MORE THAN TWO BINDINGS. The two-binding case is left EXACTLY as it was --
  // no new gate, so the existing corpus is byte-identical -- because its callers
  // already guarantee a `std::pair`-shaped element. Beyond two bindings the
  // element spelling has to be established here, and there are exactly two
  // sound spellings:
  //   MEMBER-WISE (a plain struct): the field NAME. Measured at ddc/ddc.h:603,
  //     `for (auto& [node, row, beta] : nodeInfo)` over
  //     `std::vector<RowGroupNodeInfo>` -- three named fields, no tuple model,
  //     so `.0` would be E0609. See GetMemberwiseBindingFields for the
  //     discriminator and for why a reference member is refused.
  //   TUPLE-LIKE: the INDEX, but only once the model is confirmed to be a Rust
  //     tuple of exactly this arity. Measured at
  //     dsc/sdsc-perfmodel/perfmodel.cpp:2740,
  //     `for (const auto& [sdscName, opCategory, idealCycles] : perfData)` over
  //     `std::vector<std::tuple<std::string, std::string, int64_t>>`.
  // ⭐ NOTHING IS COPIED on either spelling: every binding becomes a raw pointer
  // INTO the container element, so there is no holder copy to lose a write
  // through, no object identity to change, and no clone for a class-typed field
  // (a `std::string` field is pointed at, never moved -- so no E0507).
  // ⭐ THE ARITY EXEMPTION DOES NOT EXTEND TO THE BY-VALUE HOLDER. The
  // two-binding case skips the spelling check only to keep the pre-existing
  // corpus byte-identical -- and for a by-value holder there is no pre-existing
  // output to preserve, because it used to abort. So a by-value holder is always
  // spelling-checked, at every arity: without it a two-binding member-wise
  // struct (dsc/pcfg.cpp:2341, `ComputeLoc`) would be spelled `.0`, which is
  // E0609.
  std::vector<const clang::FieldDecl *> fields;
  if (by_value || bindings.size() != 2) {
    fields = GetMemberwiseBindingFields(decl);
    if (fields.empty()) {
      auto value_type =
          decl->getType().getNonReferenceType().getUnqualifiedType();
      if (!Mapper::Contains(value_type)) {
        return false;
      }
      if (RustTupleModelArity(Mapper::Map(value_type)) != bindings.size()) {
        return false;
      }
      for (const auto *binding : bindings) {
        // A reference binding aliases observably and is refused here for the
        // same reason as everywhere else on this construct.
        if (binding->getType()->isReferenceType()) {
          return false;
        }
      }
    }
  }
  // `getPointeeType()` is only meaningful for the reference holder; a by-value
  // holder never spells `&raw` at all.
  const bool is_const =
      by_value || decl->getType()->getPointeeType().isConstQualified();
  unsigned index = 0;
  for (const auto *binding : bindings) {
    const std::string element =
        fields.empty() ? std::to_string(index)
                       : GetNamedDeclAsString(fields[index]);
    StrCat(keyword::kLet);
    // ⛔ `let mut _ = ..` is not legal Rust (`_` is a wildcard PATTERN, not an
    // identifier), which is why `mut` is conditional on the name. A by-value
    // binding is a fresh local the body may assign to, so it gets `mut`; the
    // pointer form is a const alias that is never reassigned and keeps the plain
    // `let` it has always emitted, so no existing byte moves.
    const std::string binding_name = GetNamedDeclAsString(binding);
    if (by_value && binding_name != "_") {
      StrCat(keyword_mut_);
    }
    StrCat(binding_name);
    StrCat(token::kAssign);
    if (by_value) {
      StrCat(std::format("{}.{}", holder_name, element));
    } else {
      StrCat(std::format("&raw {} (*{}).{}", is_const ? "const" : "mut",
                         holder_name, element));
    }
    StrCat(token::kSemiColon);
    ++index;
  }
  // ⭐ NO `ptr_bindings_` REGISTRATION FOR THE BY-VALUE ARM, deliberately: the
  // caller registers the whole DecompositionDecl's bindings via
  // ScopedPtrBindings, which would make `VisitDeclRefExpr` deref a binding that
  // is a VALUE, not a pointer. The caller is responsible for skipping that when
  // this returns on the by-value arm; see VisitCXXForRangeStmtIndexBased.
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
  // ⭐ THE REFCOUNT WHOLESALE REFUSAL IS GONE, and the comment it carried was
  // wrong about WHY. It said `&raw mut (*holder).0` is unreachable in that model
  // -- true, but irrelevant, because THE REFCOUNT MODEL NEVER TAKES THE
  // INDEX-BASED PATH AT ALL. `ConverterRefCount::VisitCXXForRangeStmtVector`
  // (converter_refcount.cpp) lowers a vector range-for as an ITERATOR loop over
  // `Ptr<element>` (`for mut e in v.decay() as Ptr<(Value<A>, Value<B>)>`,
  // measured on a hand probe), so there is no `as_mut_ptr().add(i)` and no
  // `&raw` spelling to reproduce: the element is reached as
  // `(*e.upgrade().deref()).N`, and `.clone()` of that field is an `Rc` clone
  // that SHARES the element's `RefCell` -- exactly the aliasing a mutable
  // `auto& [a, b]` binding needs. Refusing here also refused the BY-VALUE
  // decomposing loop, which that model could always have lowered.
  //
  // ⛔ The refcount arm still refuses several sub-shapes, but it does so in its
  // OWN override, where the element model is visible -- and it refuses there
  // BEFORE any emission, via ReportUnsupportedStructuredBinding, so a refusal
  // still leaves no partial text. The two re-evaluation preconditions above are
  // deliberately left in place for that model even though its header emits the
  // range init exactly once: they are satisfied by every measured corpus site,
  // so keeping them costs nothing and keeps one gate rather than two.
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
    // A DECOMPOSING loop over llvm::DenseMap is routed to the map path for the
    // same reason, with the same "only the decomposing case" restriction: a
    // non-decomposing `for (auto &kv : dense_map)` keeps going down the
    // pre-existing (index-based) path, so no existing output changes.
    //
    // REFCOUNT IS REFUSED, deliberately and loudly. rules/densemap's REFCOUNT
    // t4 is `RefcountHashMapIter<T1, T2>` -- a DIFFERENT Rust type from the
    // unsafe arm's `libcc2rs::HashMapIter<T1, *const HashMap<T1, T2>>` -- so
    // the iterator spelling MapRangeIteratorName hands back would not be the
    // one that model uses. `keyword_unsafe_` is the only model discriminator
    // the base class has; this mirrors the gate on the standalone
    // map-iterator decomposition path in ConvertTupleDecompositionDecl.
    if (GetClassName(range_init_type) == "llvm::DenseMap") {
      if (keyword_unsafe_ == nullptr || *keyword_unsafe_ == '\0') {
        ReportUnsupportedStructuredBinding(decomp);
        return false;
      }
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
  // ⭐ ROW g3006. A NON-DECOMPOSING `for (auto &kv : unordered_map)` used to fall
  // through to the POSITIONAL path at the bottom of this function, which emitted
  // `m.as_ptr().add(i)` / a tuple index against the MAP itself. MEASURED
  // 2026-09-29 with snap/coord44 + pin/ir.v42 on a probe doing `kv.first`,
  // `kv.second` and a write through `kv.second`: translation was **rc=0** in both
  // models and only `rustc` caught it -- refcount `error[E0609]: no field 0/1 on
  // type HashMap<i32, Rc<RefCell<i32>>>`, unsafe `error[E0599]: no method named
  // as_mut_ptr found for struct HashMap`. rc=0 + a rustc error is the one failure
  // shape no bucket census can see, which is why this is routed rather than left.
  //
  // ⭐ WHY ROUTING IS THE SMALL FIX, and the earlier comment below ("routing it
  // to `.iter()` would bind `kv` to a Rust `(&K, &V)` tuple") is answered rather
  // than contradicted: the map path does NOT bind a tuple. It binds the MODELLED
  // ITERATOR and registers the loop variable in `map_iter_decls_`, so every
  // `kv.first` / `kv.second` in the body is rewritten to the iterator's
  // `first()` / `second()` accessors. The IDENTICAL shape over `std::map` is
  // measured CORRECT end to end today -- the same both-members-plus-write probe
  // with `std::map` gives `unsafe: MATCH 11 11` and `refcount: MATCH 11 11`,
  // i.e. the aliasing write reaches the map -- so what unordered_map needed was
  // not a new lowering, only the dispatch and the iterator SPELLING (see
  // ConverterRefCount::VisitCXXForRangeStmtMap, which used to hardcode
  // `RefcountMapIter`).
  //
  // ⚠️ `llvm::DenseMap` is deliberately NOT routed here. Its non-decomposing
  // range is broken the same way, but the refcount model cannot name a
  // `MapIterator` for it (rules/densemap's refcount t4 has no `MapIterator`
  // impl, see RefCountMapRangeIteratorName) and the unsafe arm's accessors are
  // `key_ptr()`/`value_ptr()` rather than `first()`/`second()`
  // (MapDecompositionUsesPtrAccessors), which the member-expr rewrite for a
  // NAMED loop variable does not implement. Routing it would trade a known
  // silent error for a different one; it stays a separate row.
  // ⛔⛔ AND THE ROUTING IS **NOT** THE FIX -- MEASURED, and this is the part of
  // the row that a design could not have told you. Routing it (built, md5
  // d41df53d84cf63979a587ac9911efdae, pin/ir.v42) DOES bind the right iterator,
  // and the body then reads:
  //     'loop_: for kv in RefcountHashMapIter::begin(m.clone()) {
  //         (*t.borrow_mut()) += (*kv.0.borrow());
  // i.e. `kv.first` STILL lowers to a TUPLE INDEX, now against the iterator:
  //   refcount `error[E0609]: no field 0 on type
  //             HashMapIter<i32, libcc2rs::Ptr<HashMap<i32, Rc<RefCell<i32>>>>>`
  //   unsafe   `error[E0609]: no field 0 on type
  //             HashMapIter<i32, *const HashMap<i32, Box<i32>>>`
  // -- rc=0 again, a DIFFERENT silent wrongness, which is exactly what this row
  // exists to stop. The reason is that `kv.first` is resolved through the MAPPED
  // TYPE of its base: for `std::map` the pair maps onto `RefcountMapIter` and
  // hits rules/map f20/f21 (`it->first` / `it->second`, bodies `a0.first()`),
  // while for `std::unordered_map` it maps onto the MONOMORPHISED PAIR (a Rust
  // tuple, hence `.0`) and rules/unordered_map's f36-f39 on
  // `RefcountHashMapIter` are never reached. rules/unordered_map/src.cpp:132
  // already names that gap ("lowering needs a pair whose first element is a
  // mapped iterator type"). Closing it is a MAPPER/rules row, not a dispatch row.
  //
  // ⭐ SO THE LOWERING IS REFUSED LOUDLY INSTEAD, gated HERE -- before any
  // emission, so a refused shape leaves no partial text -- because a loud abort
  // is strictly better than today's `rc=0` + `E0609`, which no bucket census can
  // see. When the mapper row lands, delete this block and put back
  // `return VisitCXXForRangeStmtMap(stmt);`: the refcount arm's iterator
  // spelling is already class-keyed for it (see
  // ConverterRefCount::VisitCXXForRangeStmtMap).
  //
  // ⚠️ `llvm::DenseMap`'s non-decomposing range is broken the SAME way and is
  // deliberately left alone: it is a different rules module with different
  // accessor names (`key_ptr()`/`value_ptr()`, see
  // MapDecompositionUsesPtrAccessors), and refusing it here would change corpus
  // TUs this row has not measured. Its own row, stated rather than widened.
  if (GetClassName(range_init_type) == "std::unordered_map") {
    const auto *loop_var = stmt->getLoopVariable();
    // ⭐⭐ ROW g3013 CLOSES THE MAPPER HALF, so the REFERENCE shape is no longer
    // refused -- it is routed to the map path, which is the fix the previous row
    // built and correctly measured as insufficient ON ITS OWN.
    //
    // What changed is NOT here: `Mapper::ToString(const clang::Expr *)`'s
    // for-range member-key branch now accepts `std::unordered_map<` as well as
    // `std::map<`, so `kv.first` in the body is keyed as
    // `std::unordered_map<K,V>::iterator->first` and reaches
    // rules/unordered_map f36-f39 instead of lowering as the monomorphised
    // pair's tuple index. With that in place the map path's existing machinery
    // is sufficient: it binds the modelled iterator (MapRangeIteratorName /
    // RefCountMapRangeIteratorName) and registers the loop variable in
    // `map_iter_decls_` so its uses do not deref.
    //
    // ⛔ THE BY-VALUE SHAPE KEEPS THE LOUD ABORT, deliberately and measured-as-
    // untested: `for (auto kv : m)` would bind `kv` to the iterator BY VALUE and
    // the refcount arm already refuses a by-value loop variable over any
    // map-like class other than `std::map` (see
    // ConverterRefCount::VisitCXXForRangeStmtMap, whose `EmitByValueShadow`
    // shape is a PRE-EXISTING E0308 even for `std::map`). Narrowing the refusal's
    // domain to the shape that is not proven is the deliverable; removing it
    // wholesale is not.
    if (loop_var->getType()->isReferenceType()) {
      return VisitCXXForRangeStmtMap(stmt);
    }
    const std::string loc =
        loop_var->getLocation().printToString(ctx_.getSourceManager());
    std::string detail =
        "BY-VALUE non-decomposing range-`for` over `std::unordered_map` (loop "
        "variable `" +
        GetNamedDeclAsString(loop_var) + "` of type `" +
        Mapper::ToString(loop_var->getType()) +
        "`) is not implemented: the positional lowering emits a field access the "
        "mapped `HashMap` does not have, and the map path would bind the "
        "modelled iterator BY VALUE, whose by-value shadow is a pre-existing "
        "E0308 even for `std::map` (the REFERENCE shape is lowered, ROW g3013)";
    if (curr_function_ != nullptr) {
      detail += ", reached while converting `" +
                curr_function_->getQualifiedNameAsString() + "`";
    }
    if (survey::Enabled()) {
      survey::Record(survey::GapKind::kUnsupportedConstruct, detail, loc);
      return false;
    }
    llvm::report_fatal_error(llvm::Twine("unsupported ") + detail + " at " + loc,
                             /*gen_crash_diag=*/false);
  }
  if (GetClassName(range_init_type) == "std::basic_string") {
    return VisitCXXForRangeStmtString(stmt);
  }
  // ⛔ THIS FALL-THROUGH IS WHERE THE POSITIONAL ASSUMPTION IS MADE: every range
  // that is not a map and not a string is lowered as `0..range.len()` plus
  // `range.as_ptr().add(i)` (VisitCXXForRangeStmtIndexBased +
  // ConvertLoopVariable), which never consults `begin`/`end` and is simply wrong
  // for a container that has no positional element access. Measured 2026-09-28:
  // `for (const auto& written : producer_writes)` over an
  // `llvm::SmallDenseSet<mlir::Attribute>` (ScratchpadConflicts.cpp:149) emitted
  // `producer_writes.as_ptr()` against a Rust `std::collections::HashSet`, which
  // is `E0599: no method named as_ptr` -- at rc=0, with no placeholder token, so
  // NO bucket census could see it. `.len()` exists on `HashSet`, which is
  // exactly why only real `rustc` finds this class.
  //
  // ⚠️ THE SET BRANCH IS ADDITIVE AND IS DELIBERATELY NOT A WIDER FIX. Maps are
  // ALSO non-indexable, and `for (auto &kv : unordered_map)` still goes down the
  // positional path from here -- routing it to `.iter()` would bind `kv` to a
  // Rust `(&K, &V)` tuple where the C++ loop variable is a
  // `std::pair<const K, V> &`, i.e. a DIFFERENT silent wrongness. Only the SET
  // shape is lowered, because there and only there the iterator element IS the
  // loop variable's referent.
  if (IsSetLikeRangeInit(range_init_type)) {
    return VisitCXXForRangeStmtSet(stmt);
  }
  return VisitCXXForRangeStmtVector(stmt);
}

// ⭐ WHY THIS TESTS THE MAPPED RUST TYPE AND NOT THE C++ CLASS NAME, measured.
// A C++-name allowlist is the obvious shape and it MISFIRES: `llvm::SetVector`
// and `llvm::SmallSetVector` are set classes by name and by C++ semantics, and
// rules/setvector deliberately models them as `Vec<T>` (tgt_unsafe.rs:9 -- the
// insertion order is observable at every corpus site), so they ARE indexable in
// Rust and the positional lowering is already CORRECT for them. Emitting
// `.iter()` there would replace working output with a loop variable of the wrong
// Rust type. The only thing that decides indexability is what the range is
// modelled AS, so that is what gets asked.
//
// The predicate is therefore an allowlist of RUST target spellings, and it can
// only fire on a container this converter already knows is a `HashSet`/
// `BTreeSet`; everything else -- including every unmapped type, every user
// struct and every set class modelled as a `Vec` -- takes exactly the path it
// took before this change.
static bool IsNonIndexableSetTargetType(const std::string &rust_type) {
  return rust_type.starts_with("std::collections::HashSet<") ||
         rust_type.starts_with("std::collections::BTreeSet<");
}

// ⚠️ A NAME PRE-FILTER, AND ONLY A PRE-FILTER. It cannot decide indexability --
// `llvm::SetVector` passes it and is correctly rejected by the Rust-type test --
// and it is not what makes the predicate safe. It exists because ASKING the
// mapper has an OBSERVABLE SIDE EFFECT: `Mapper::search` synthesises an identity
// rule for a project type and prints `note: project leaf type ... has no types_
// entry yet`. MEASURED 2026-09-28: without this filter, 16 of 32 corpus TUs grew
// extra `note:` lines on stderr (their emitted Rust stayed byte-identical, and
// every bucket and abort was unchanged) purely because every non-map, non-string
// range init was now being mapped. Narrowing to set-NAMED classes keeps the
// mapper out of the path of every range this row does not touch, so "everything
// else takes exactly the path it took before" is true of the diagnostics too and
// not only of the output.
static bool HasSetLikeClassName(const std::string &class_name) {
  auto pos = class_name.rfind("::");
  std::string leaf =
      pos == std::string::npos ? class_name : class_name.substr(pos + 2);
  return leaf.find("Set") != std::string::npos ||
         leaf.find("set") != std::string::npos;
}

bool Converter::IsSetLikeRangeInit(clang::QualType range_init_type) {
  if (!HasSetLikeClassName(GetClassName(range_init_type))) {
    return false;
  }
  auto unqualified = range_init_type.getUnqualifiedType();
  // ⚠️ `Mapper::Contains` FIRST, and not as a "has a rule" test -- it is a known
  // TRUE for any user struct in the TU, because `search()` falls through to
  // `LooksLikeUserDefinedTypeName` and SYNTHESISES an identity rule. It is used
  // here purely as a guard so `Mapper::Map` is never asked about a type it has
  // no model for (which can report an unmapped system type, and that report is
  // fatal in some builds). The real discriminator is the spelling test below,
  // and a synthesised identity mapping hands back the user tag name, which
  // cannot start with `std::collections::`.
  if (!Mapper::Contains(unqualified)) {
    return false;
  }
  return IsNonIndexableSetTargetType(Mapper::Map(unqualified));
}

// A range-for over a container modelled as a Rust SET. The element sequence is
// reached through `iter()` -- the only access a `HashSet`/`BTreeSet` has -- and
// the loop variable is then bound to exactly the same REPRESENTATION the
// positional path would have given it, so nothing downstream of the loop
// changes:
//   `const T &`  ->  `*const T`   (was `range.as_ptr().add(i)`)
//   `T &`        ->  `*mut T`     (was `range.as_mut_ptr().add(i)`)
//   `T`          ->  a clone      (was `range[i].clone()`)
// Keeping the raw-pointer representation is load-bearing, not conservatism: a
// C++ reference is a raw pointer everywhere else in this model, and every USE of
// the loop variable is emitted by VisitDeclRefExpr on that assumption. Binding
// `&T` instead would type-check at some use sites and not others.
//
// `std::ptr::from_ref` / `.cast_mut()` rather than `as *const _` because the
// inferred-target cast has no type to infer from at a `let` with no annotation,
// and rather than a bare `&T` because of the representation point above. Neither
// introduces an autoref, so neither can trip `dangerous_implicit_autorefs`
// (deny-by-default in rustc 1.98).
//
// A DECOMPOSING loop variable never reaches here: VisitCXXForRangeStmt refuses
// it loudly above, for every range that is not map-like or a hoist-free vector.
// That is correct for a set -- a set element is not a key/value pair.
bool Converter::VisitCXXForRangeStmtSet(clang::CXXForRangeStmt *stmt) {
  auto *loop_var = stmt->getLoopVariable();
  auto loop_var_name = GetNamedDeclAsString(loop_var);
  // The iterator element needs its own name: the positional path reuses the loop
  // variable's name for the INDEX and shadows it, which works only because the
  // index is an integer. Here the element and the binding have different types,
  // and the binding's initialiser reads the element, so shadowing would be a
  // self-reference. Source-location-derived so it cannot collide with a user
  // local or with a second loop in the same scope.
  auto loc = ctx_.getSourceManager().getPresumedLoc(stmt->getBeginLoc());
  auto elem_name =
      loc.isValid()
          ? std::format("__elem_{}_{}", loc.getLine(), loc.getColumn())
          : std::format("__elem_{}", static_cast<const void *>(stmt));

  StrCat("'loop_:");
  StrCat(keyword::kFor, elem_name, keyword::kIn);
  {
    // The range init is emitted ONCE here -- there is no `.len()` bound to
    // evaluate it a second time for -- so the re-evaluation hazard the
    // positional path needs its prvalue hoist for does not exist on this path,
    // and no hoist is added.
    PushParen range(*this);
    Convert(stmt->getRangeInit());
    StrCat(token::kDot, "iter()");
  }
  {
    PushBrace body(*this);
    auto loop_var_type = loop_var->getType();
    // ⛔ MODEL HOOK, and it must come BEFORE the `let` below: the refcount
    // representation of a reference-to-element needs a second statement in front
    // of the binding (the owning `Value<T>` the `Ptr<T>` points into), so it
    // cannot be expressed as a replacement for just the initialiser. An empty
    // return declines and leaves the unsafe emission below byte-for-byte as it
    // was -- verified on `dsc/dims.cpp`, whose unsafe output is unchanged.
    if (loop_var_type->isReferenceType()) {
      auto model_binding =
          ForRangeSetRefElementBinding(loop_var_name, elem_name, loop_var_type);
      if (!model_binding.empty()) {
        StrCat(model_binding);
        ConvertForRangeBody(stmt);
        return false;
      }
    }
    StrCat(keyword::kLet);
    if (!loop_var_type.isConstQualified()) {
      StrCat(keyword_mut_);
    }
    StrCat(loop_var_name);
    StrCat(token::kAssign);
    if (loop_var_type->isReferenceType()) {
      if (loop_var_type->getPointeeType().isConstQualified()) {
        StrCat(std::format("std::ptr::from_ref({})", elem_name));
      } else {
        StrCat(std::format("std::ptr::from_ref({}).cast_mut()", elem_name));
      }
    } else {
      StrCat(std::format("{}.clone()", elem_name));
    }
    StrCat(token::kSemiColon);
    ConvertForRangeBody(stmt);
  }
  return false;
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
    const clang::DecompositionDecl *decl, const std::string &iter_name,
    bool ptr_accessors) {
  auto bindings = decl->bindings();
  if (bindings.size() != 2) {
    return false;
  }
  static const char *const kTraitAccessors[] = {"first", "second"};
  static const char *const kInherentAccessors[] = {"key_ptr", "value_ptr"};
  const char *const *kAccessors =
      ptr_accessors ? kInherentAccessors : kTraitAccessors;
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

// The UNSAFE receiver for the standalone-`let` map decomposition: the plain
// conversion of the iterator variable reference. This is byte-for-byte the code
// that used to sit inline in ConvertTupleDecompositionDecl's map branch, moved
// behind a hook only so the refcount model can spell its boxed local. The
// operand is already restricted to a side-effect-free DeclRefExpr by the caller,
// which is what makes re-emitting it once per accessor sound.
std::string Converter::DecompositionMapIterReceiver(
    clang::DeclRefExpr *iter_ref) {
  Buffer buf(*this);
  Convert(iter_ref);
  return std::move(buf).str();
}

// The UNSAFE model declines: its own `std::ptr::from_ref(elem)` binding two
// screens up IS the correct representation for this model, and the comment there
// spells out why (a C++ reference is a raw pointer everywhere in it). Declining
// here rather than returning that text keeps the unsafe path on exactly the code
// that shipped before the hook, so the hook cannot move the unsafe emission.
std::string Converter::ForRangeSetRefElementBinding(const std::string &,
                                                    const std::string &,
                                                    clang::QualType) {
  return {};
}

// The range classes whose MODELLED iterator exposes a key accessor and a value
// accessor, i.e. the ones the decomposing map lowering can address. Kept as one
// predicate so the dispatch, the iterator-name choice and the accessor-name
// choice cannot drift apart.
//
// `llvm::DenseMap` qualifies for exactly the same reason `std::unordered_map`
// does -- rules/densemap models it as a `std::collections::HashMap<K, V>` whose
// iterator (t4/t5) is `libcc2rs::HashMapIter<K, *const HashMap<K, V>>`, with
// begin/end (f7-f10), ++ (f11-f14) and key/value (f15-f18) all keyed. The ONE
// place it differs is the ACCESSOR NAMES; see
// MapDecompositionUsesPtrAccessors.
bool Converter::IsMapLikeRangeClass(const std::string &class_name) {
  return class_name == "std::map" || class_name == "std::unordered_map" ||
         class_name == "llvm::DenseMap";
}

const char *Converter::MapRangeIteratorName(const std::string &class_name) {
  // rules/densemap's t4/t5 is the BARE `libcc2rs::HashMapIter<K, *const
  // HashMap<K, V>>`, NOT the `UnsafeHashMapIterator` alias -- that alias is
  // bound to `HashMap<K, Box<V>>` while densemap's t1 is a bare
  // `HashMap<K, V>`. `HashMapIter::begin` is inherent on the generic type, so
  // naming the type itself is what type-checks, and it is what f7 emits.
  if (class_name == "llvm::DenseMap") {
    return "libcc2rs::HashMapIter";
  }
  return class_name == "std::unordered_map" ? "UnsafeHashMapIterator"
                                           : "UnsafeMapIterator";
}

// MEASURED out of rules/densemap/tgt_unsafe.rs (the comment above f15): the
// `MapIterator` trait -- whose methods are `first()` / `second()` -- is
// implemented for the two libcc2rs ALIASES only, and a generic impl for
// `HashMapIter<K, *const HashMap<K, V>>` CANNOT be added beside the `Box<V>`
// one because the two overlap at V = Box<V'> and rustc rejects it with E0119.
// libcc2rs therefore carries the same two accessors as INHERENT methods
// `key_ptr()` / `value_ptr()` on the generic impl, and rules/densemap's
// f15-f18 call those. Both families return the same two things -- `*const K`
// and `*mut V` -- which is why the ptr_bindings_ registration below is
// identical for either and only the spelling has to switch.
bool Converter::MapDecompositionUsesPtrAccessors(const std::string &class_name) {
  return class_name == "llvm::DenseMap";
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
      if (!EmitMapDecompositionBindings(
              decomp, loop_var_name,
              MapDecompositionUsesPtrAccessors(
                  GetClassName(stmt->getRangeInit()->getType())))) {
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
      // ⭐ THE GUARD MUST FOLLOW THE FORM THAT WAS ACTUALLY EMITTED. A REFERENCE
      // loop variable gets the `&raw` pointer form, so every use needs the
      // per-use deref this registration installs. A BY-VALUE loop variable gets
      // a plain `holder.N` VALUE, and registering it here would make
      // `VisitDeclRefExpr` emit `(*binding)` for a non-pointer -- E0614, at rc=0.
      // The condition is the same one EmitVectorDecompositionBindings branches
      // on, so the two cannot drift apart silently.
      if (decomp->getType()->isReferenceType()) {
        ptr_bindings.emplace(*this, decomp);
      }
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
  if (expr && rs_code_->size() == before && !HasDeferredEmission()) {
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
    // The PushParen above wraps operand AND cast together -- `( x as T )` --
    // so it does NOT save a block-form operand from the `as`. `before` is
    // exactly where this operand's text starts, so reuse it.
    ParenthesizeBlockCastOperand(before);
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

std::string Converter::EscapeFmtBraces(std::string_view text) {
  std::string out;
  out.reserve(text.size());
  for (char c : text) {
    if (c == '{' || c == '}') {
      out += c;
    }
    out += c;
  }
  return out;
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
    // LITERAL text, so braces must be doubled. `trim` is already backslash-
    // escaped, which is orthogonal: a `{` in it is a real brace either way.
    fmt += EscapeFmtBraces(trim);
  } else if (auto ch = GetEscapedUTF8CharLiteral(arg); !ch.empty()) {
    // `os << '{'` is literal text too.
    fmt += EscapeFmtBraces(ch);
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

// ConvertStream gives the unsafe model the PLACE `(*os)` behind the `*mut File`
// that a `std::ostream&` parameter maps to, so the chain's value -- a reference
// to the same stream -- is a raw pointer back to that place. `&raw mut` rather
// than `&mut` because every other stream use in the unsafe model is a raw
// pointer deref and a `&mut` here would collide with them under Stacked
// Borrows.
std::string Converter::StreamValue(const std::string &stream) {
  return "&raw mut " + stream;
}

// ⭐ An ostream `<<` chain is lowered to STATEMENTS (`write!(<os>, ..);`,
// `<os>.write_all(..);`) by ConvertCallToOstream, and a statement is only legal
// where the chain's VALUE is discarded. C++ `os << x` evaluates to
// `std::ostream&`, and real code reaches that value: in
// `sys-arch-spec/dpc/dpc.cpp:1394`,
//     outStream << INDENT << prefix << "_" << instr.instn_;
// the trailing operand has a USER-DEFINED `operator<<(ostream&,
// Isa::InstOpCode)`, so the built-in prefix of the chain lands in that call's
// ARGUMENT slot. EmitHoistedArgs then wrapped a statement in an expression:
//     let _os: Ptr<std::fs::File> = (write!(outStream, "..",);).clone();
//                                                          ^ rustfmt:
//     error: expected one of `)`, `,`, `.`, `?`, or an operator, found `;`
// on a 4,530-line `.rs` the converter had otherwise finished -- a defect no
// bucket census sees, because the converter's own exit code is 0.
//
// ⚠️ THIS IS NOT A REFCOUNT-ONLY DEFECT, and the unsafe model is the WORSE
// half: there the same site emits
//     let _os: *mut std::fs::File = &mut write!((*out), "a",);
//     (*out).write_all(&([..].concat()));
// which PARSES. The chain's remaining writes leak out as sibling statements
// after the binding, so `_os` is bound to `&mut Result<(),Error>` and the
// writes happen in the wrong place. Measured on /home/agent/work/semi/probe.cpp
// with from-master 177f30db: `rc=0` in the unsafe model and a rustfmt reject in
// refcount, from ONE converter defect.
//
// Transparent parents are skipped: a discarded chain is routinely wrapped in an
// ExprWithCleanups, and that wrapper is an Expr. Under-detection here is the
// status quo (a statement in statement position), over-detection would break a
// working site, so the list is deliberately conservative.
bool Converter::OstreamChainValueIsUsed(clang::Expr *expr) {
  const clang::Expr *cur = expr;
  while (const clang::Expr *parent = GetParentExpr(cur)) {
    if (clang::isa<clang::ParenExpr>(parent) ||
        clang::isa<clang::ExprWithCleanups>(parent) ||
        clang::isa<clang::ConstantExpr>(parent) ||
        clang::isa<clang::CXXBindTemporaryExpr>(parent)) {
      cur = parent;
      continue;
    }
    return true;
  }
  // ⭐ NO Expr PARENT DOES NOT MEAN "VALUE DISCARDED": the consumer may be a
  // Stmt. `return <chain>;` out of `std::ostream &operator<<(std::ostream &,
  // T)` is exactly that case -- VisitReturnStmt (converter.cpp:3083) hands the
  // operand to ConvertVarInit, so the value IS used, while the parent walk
  // above answered false because a ReturnStmt is a Stmt and GetParentExpr
  // dyn_casts to Expr. That produced, in the unsafe model,
  //     return &mut (*os).write_all(&([..].concat()));
  // i.e. `&mut Result<(),Error>` where the declared return type is
  // `*mut std::fs::File` -- E0308 -- and in refcount
  //     return os.write_all(&([..].concat()));
  // i.e. `Result<(),Error>` where `Ptr<File>` is declared: E0308 in BOTH
  // models, from the same predicate. (The refcount half is NOT saved by
  // ConverterRefCount::IsReferenceType's CXXOperatorCallExpr override: that
  // override only stops ConvertVarInit taking an address, it cannot conjure the
  // StreamValue tail, which only ConvertCallToOstream emits.)
  //
  // Both callers of this predicate want `true` here and neither wants anything
  // else, so there is no need to narrow to return position at the call sites:
  //   * ConvertCallToOstream (:4804) -> wraps the writes in a block whose tail
  //     is StreamValue(), the reference value the chain evaluates to;
  //   * IsOstreamChainValue (:4753) -> ConvertVarInit's third conjunct, which
  //     must now fire because VisitReturnStmt reaches ConvertVarInit with
  //     exactly that block as the initialiser.
  // Together they emit `return { (*os).write_all(..); &raw mut (*os) };` under
  // unsafe and `return { os.write_all(..); (os).clone() };` under refcount --
  // C++ ground truth, which is "do the writes, then yield the stream".
  // MEASURED at compile level, rustc 1.98.0, on the exact emitted text
  // minimised: E0308 before / rc=0 after, in BOTH models
  // (snap/g3053/shape.rs, shapefix.rs, shape_rc.rs, shapefix_rc.rs).
  //
  // ⚠️ NO `isVoidType()` GUARD, deliberately, because it would be VACUOUS: a
  // `return <expr>;` whose operand has non-void type is ill-formed in a
  // void-returning function, so clang never builds this AST. The one shape that
  // looks like a counter-example -- `return (void)(os << x);` -- has a
  // CStyleCastExpr parent, which is an Expr, so it answers true one iteration
  // earlier and is untouched by this arm.
  //
  // ⛔ OTHER non-Expr consumers, censused rather than assumed. Each answers
  // false here and stays exactly as loud/wrong as it is today:
  //   * a VarDecl initialiser -- `std::ostream &r = os << x;` -- whose parent
  //     node is a Decl, so GetParentExpr is null. (Hoisted argument bindings are
  //     NOT this case: their AST parent is still the enclosing
  //     CXXOperatorCallExpr, which is why 35c9eaec's site works.)
  //   * `co_return <chain>;` (CoreturnStmt).
  //   * the tail statement of a GNU StmtExpr, `({ os << x; })`.
  // ⭐ CENSUSED, not assumed: over all of repos/dt_src, `git grep -n co_return`
  // is 0 hits; a stream-reference variable initialised from a chain
  // (`(ostream|ofstream|stringstream|osyncstream)\s*&\s*\w+\s*=[^;]*<<`) is 0
  // hits; and the 11 hits for `\(\{[^}]*<<` are ALL `LLVM_DEBUG({ llvm::dbgs()
  // << ..; })` -- a macro argument whose braces are a plain block, not a GNU
  // statement expression, with the chain's value discarded inside it. So the
  // corpus has no witness for any of the three, and inventing lowerings for
  // them would be untestable guesswork; naming them is the honest deliverable.
  auto parents = ctx_.getParentMapContext().getParents(*cur);
  if (parents.empty()) {
    return false;
  }
  const auto *ret = parents.begin()->get<clang::ReturnStmt>();
  return ret != nullptr && ret->getRetValue() == cur;
}

// ⭐ THE RULE ConvertVarInit's `&mut` IMPLEMENTS, stated so this narrowing can
// be judged: a C++ `T&` parameter maps to a Rust `*mut T`, and MOST arguments
// bound to one are PLACES (an lvalue object of type `T`), so the initialiser has
// to take an address -- `&mut <place>`, which coerces to `*mut T`.
// `IsReferenceType(expr)` is the place-vs-value discriminator: when the
// expression is ALREADY a reference value (a call returning `T&`, a DeclRef to a
// `T&` variable, a `T&` member) it is passed through instead.
//
// That discriminator deliberately excludes CXXOperatorCallExpr wholesale
// (converter.cpp:6370), and for good reason: a mapped operator's rule body is
// INLINED TEXT, and the reference-returning ones overwhelmingly lower to a
// Rust PLACE, not a pointer -- `v[i]` for a `T& operator[]`, the assigned-to
// object for a `S& operator=`. Dropping the `&mut` for those would be wrong in
// the other direction.
//
// The ostream `<<` chain is the exception, and it is an exception in BOTH
// models: since 6a468a27 a chain whose value is consumed is emitted as a BLOCK
// whose tail is StreamValue(), which is `&raw mut (*os)` under unsafe and
// `(os).clone()` under refcount. Both of those are the reference VALUE, never a
// place. So `&mut` on top of it produced
//     let _os: *mut std::fs::File = &mut { ...; &raw mut (*out) };
// i.e. a `&mut *mut File` bound to a `*mut File` -- E0308.
//
// ⚠️ This is a PRE-EXISTING ConvertVarInit defect, not a regression from
// 6a468a27, and it was invisible in the refcount model because
// ConverterRefCount::IsReferenceType (converter_refcount.cpp:3567) ALREADY
// overrides the base predicate with exactly the missing case (any
// CXXOperatorCallExpr returning a reference). Only the unsafe model, which uses
// the base predicate, wraps the chain. That asymmetry is why the row had to run
// both models to see it at all.
//
// Narrower than the refcount override on purpose: this keys on the ONE construct
// whose lowering is known to yield a pointer value, so no mapped
// subscript/assignment operator changes.
//
// ⛔⛔ THE `OstreamChainValueIsUsed` CONJUNCT IS LOAD-BEARING, AND I FIRST WROTE
// THE OPPOSITE HERE: "a ConvertVarInit initialiser slot consumes its value by
// construction, so the extra conjunct would be VACUOUS." MEASURED FALSE at TU
// scale on sys-arch-spec/dpc/dpc.cpp, unsafe model, which has THREE sites, not
// one. Two of them are `return <chain>;` out of a
// `std::ostream &operator<<(std::ostream &, Isa::InstOpCode)`:
//     return &mut (*os).write_all(&([..].concat()));   // *mut File <- &mut Result
// `OstreamChainValueIsUsed` walks GetParentExpr, and a ReturnStmt is a Stmt, not
// an Expr, so it returns FALSE there -- no block is emitted and there is no
// StreamValue tail to pass through. Dropping the `&mut` at those two sites turns
// `&mut Result<(),Error>` into `Result<(),Error>`: still E0308, just differently
// wrong, on text that was not mine to change. With the conjunct they are left
// BYTE-IDENTICAL.
// ⭐ The residue is therefore named, not hidden: `return <ostream chain>;` in a
// reference-returning `operator<<` is a SECOND defect of the same family, living
// in `OstreamChainValueIsUsed`'s parent walk (return position is not an Expr
// parent), which this row was told not to touch. 2 sites in dpc.cpp.
bool Converter::IsOstreamChainValue(clang::Expr *expr) {
  auto *call =
      clang::dyn_cast<clang::CXXOperatorCallExpr>(expr->IgnoreParenImpCasts());
  return call != nullptr &&
         call->getOperator() == clang::OverloadedOperatorKind::OO_LessLess &&
         IsCallToOstream(call) && OstreamChainValueIsUsed(call);
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

  // See OstreamChainValueIsUsed. When the chain's value is consumed, the
  // statements have to be wrapped in a block expression whose tail is the
  // stream itself, so that the whole thing is an EXPRESSION of the stream's
  // type. In statement position nothing changes -- PushDelim's `enabled` flag
  // emits no braces -- so this cannot move text at the sites that already work.
  const bool value_used = OstreamChainValueIsUsed(expr);
  PushBrace chain_block(*this, value_used);

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

  if (value_used) {
    StrCat(StreamValue(stream_str));
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

  // ⭐⭐ THE THIRD STATE OF `Mapper::Contains(expr->getCallee()) == false`, and
  // the reason it has to exist: on false this function does NOT abort, it falls
  // through to generic `ConvertCallExpr` and prints the literal C++ method name.
  // That is rc=0, a plausible `.rs`, and a rustc E0599 one stage later that no
  // census in this project can see -- which makes RULE 2's "leave the key out
  // and make it FAIL LOUDLY at translate time" UNACHIEVABLE for a member.
  //
  // ⛔ OPT-IN ONLY. `GetRefusedRule` is non-null only for a key a rule module
  // explicitly marked `refused`; an unmapped member that no rule mentions is
  // untouched and still falls through exactly as before. With nothing refused
  // anywhere in the tree `refused_exprs_` is empty and this is a null check on
  // an empty container.
  //
  // ⭐ PLACED FIRST among the `Contains`-false arms on purpose. The
  // `IsImplicitAssignmentCall` arm immediately below and the
  // `ConvertCXXOperatorCallExpr` arm further down are both reached on a
  // `Contains` miss and both emit text; a refusal checked after them would be
  // silently swallowed for those two constructs, which is the same
  // level-too-low mistake the refusal exists to fix.
  //
  // ⛔⛔ AND THIS ARM ALONE IS NOT ENOUGH: this function is `virtual` and
  // `ConverterRefCount::VisitCallExpr` overrides it, so every override needs the
  // same guard at the same structural position. See `RefuseIfRefusedMember`.
  if (RefuseIfRefusedMember(expr)) {
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
  // A FUNCTION PASSED BY NAME IS A CALLABLE, NOT A CALL -- and this function is
  // reached ONLY from ConvertFnPtrPlaceholder, i.e. only for a RULE placeholder
  // whose argument has function-pointer type (converter.cpp:9610).
  //
  // Without this case, `std::transform(b, e, b, ::tolower)` -- once ANY rule
  // matches the enclosing `transform` -- lowered its callable operand by
  // descending into `Convert(arg)` under ExprKind::Callee.  The
  // CK_FunctionToPointerDecay arm at converter.cpp:5606 then forwards the
  // sub-expression UNCHANGED when `isCallee()` (it must: a direct call emits
  // `f(x)`, not `Some(f)(x)`), so the `::tolower` DeclRefExpr arrived in
  // ConvertDeclRefExpr (:6383) with `isAddrOf()` FALSE.  There
  // ShouldReplaceWithMappedBody (:9909) returns true, GetMappedAsString
  // substitutes rules/cctype f1's BODY, and its first `a0` fragment aborts in
  // ConvertIRFragment (:9750):
  //   LLVM ERROR: rule body references placeholder a0 but the call site
  //   supplies only 0 argument(s) at dsc/designSpaceConfig.h:318:66
  // -- correctly, because a function NAME supplies no arguments.  Measured
  // 2026-09-29; it made `std::transform` unkeyable (rules/algorithm's header
  // note) and turned the goal TU from `A rc=0` into a bucket-B abort.
  //
  // The ADDR-OF path already gets this right -- ConvertFunctionToFunctionPointer
  // just above emits `Some(GetFunctionRefName(fn))`, which is why the SAME
  // operand is harmless while `transform` is unmapped.  So take the same name
  // here.  ConvertFnPtrPlaceholder supplies the `as unsafe fn(..)` cast in place
  // of the `Some(..)`, which is what a rule parameter of function-pointer type
  // needs (libcc2rs' `Callable{N}` is implemented for `unsafe fn(..) -> R`, not
  // for `Option<..>`), so the two paths now name the SAME callable.
  if (const auto *ref =
          clang::dyn_cast<clang::DeclRefExpr>(arg->IgnoreParenImpCasts())) {
    if (const auto *fn = clang::dyn_cast<clang::FunctionDecl>(ref->getDecl())) {
      return GetFunctionRefName(fn);
    }
  }
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

// If `expr` is a call THROUGH a pointer-to-member -- `(obj->*pm)(args)` or
// `(obj.*pm)(args)` -- returns the `->*`/`.*` node, otherwise nullptr. The
// callee of such a call is a ParenExpr wrapping the operator, and its type is
// `<bound member function type>` rather than a function pointer, which is why
// it reaches neither getDirectCallee() nor the FunctionProtoType path below.
static clang::BinaryOperator *GetPtrMemCallee(clang::CallExpr *expr) {
  auto *bin_op =
      clang::dyn_cast<clang::BinaryOperator>(GetCallee(expr)->IgnoreParenImpCasts());
  return (bin_op != nullptr && bin_op->isPtrMemOp()) ? bin_op : nullptr;
}

// Resolves the callee of a pointer-to-member call when the member pointer is a
// COMPILE-TIME CONSTANT, and returns nullptr in every other case.
//
// KTDFArchIntrinsics.h is the whole reason this exists:
//
//     using GetAttrNameFn = StringAttr (KTDFArchDialect::*)() const;
//     template <GetAttrNameFn GetAttrName, class AttrConstraint = Attribute>
//     struct IntrinsicAttr : AttrConstraint {
//       static auto getAttrName(MLIRContext* context) -> StringAttr {
//         const auto* dialect = context->getLoadedDialect<KTDFArchDialect>();
//         return (dialect->*GetAttrName)();                        // <-- here
//       }
//     };
//
// `GetAttrName` is a NON-TYPE TEMPLATE PARAMETER, i.e. a compile-time constant.
// In every instantiation the `->*` therefore names exactly ONE member function
// and the call carries no runtime indirection at all: it is a direct call
// wearing member-pointer syntax. The standing refusal in VisitBinaryOperator
// ("a C++ member pointer is an offset/vtable index, not a Rust value") is
// correct for a member pointer that is a VALUE, and simply does not apply to
// this shape -- nothing needs to be modelled, because the callee is statically
// known.
//
// That the two cases are distinguishable is not an accident of this one header,
// it is checkable, and it was checked: `GetAttrNameFn` occurs exactly THREE
// times in the whole source tree -- the alias above and the two
// template-parameter declarations -- and never as a variable, parameter, field
// or array element. Its callee set is closed at seven, every one of them a
// literal `&`-of-member naming a nullary `const` getter.
//
// ⛔ ANYTHING that is not a direct reference to a member function returns
// nullptr and keeps aborting loudly. In particular `Fn f = ...; (d->*f)();`
// reaches a DeclRefExpr to a VarDecl, which is a genuine runtime value:
// resolving that statically would be UNSOUND, not conservative.
static const clang::CXXMethodDecl *
ResolveConstantMemberPointerCallee(clang::Expr *member_ptr) {
  // Peel the sugar a non-type template argument arrives wrapped in. Every step
  // here is value-preserving -- none of them can substitute one member function
  // for another -- which is precisely what makes the peel safe.
  for (clang::Expr *prev = nullptr; member_ptr != prev;) {
    prev = member_ptr;
    member_ptr = member_ptr->IgnoreParenImpCasts();
    if (auto *subst =
            clang::dyn_cast<clang::SubstNonTypeTemplateParmExpr>(member_ptr)) {
      // The instantiated form: the node wraps the template ARGUMENT that was
      // substituted in, i.e. `&KTDFArchDialect::getBandwidthAttrName`.
      member_ptr = subst->getReplacement();
    } else if (auto *const_expr =
                   clang::dyn_cast<clang::ConstantExpr>(member_ptr)) {
      member_ptr = const_expr->getSubExpr();
    } else if (auto *unary = clang::dyn_cast<clang::UnaryOperator>(member_ptr);
               unary != nullptr && unary->getOpcode() == clang::UO_AddrOf) {
      // The `&` of `&KTDFArchDialect::getBandwidthAttrName`.
      member_ptr = unary->getSubExpr();
    }
  }
  auto *ref = clang::dyn_cast<clang::DeclRefExpr>(member_ptr);
  if (ref == nullptr) {
    return nullptr;
  }
  return clang::dyn_cast<clang::CXXMethodDecl>(ref->getDecl());
}

Converter::CallInfo Converter::CollectCallInfo(clang::CallExpr *expr) {
  using Kind = CallArg::Kind;

  CallInfo info;
  info.expr = expr;
  auto callee = GetCallee(expr);

  // A call through a CONSTANT pointer-to-member is a direct call; recognise it
  // here so EmitCall can spell the callee rather than convert the `->*`, which
  // is what reaches VisitBinaryOperator's pointer-to-member abort.
  //
  // IsMethodOnPtr mirrors the gate VisitMemberExpr uses for `obj->method()`:
  // it is the condition under which the method is emitted as an inherent
  // `fn(&self, ...)` that a UFCS spelling can actually name. When it does not
  // hold there is no direct call to emit, so this is left alone and the
  // existing abort still fires.
  if (auto *ptr_mem = GetPtrMemCallee(expr)) {
    if (const auto *method =
            ResolveConstantMemberPointerCallee(ptr_mem->getRHS());
        method != nullptr && IsMethodOnPtr(method)) {
      info.ptr_mem_callee = method;
      info.ptr_mem_object = ptr_mem->getLHS();
      info.ptr_mem_is_arrow = ptr_mem->getOpcode() == clang::BO_PtrMemI;
    }
  }
  unsigned arg_begin = 0;
  if (auto op_call = llvm::dyn_cast<clang::CXXOperatorCallExpr>(expr)) {
    if (clang::isa_and_nonnull<clang::CXXMethodDecl>(
            op_call->getDirectCallee())) {
      arg_begin = 1;
    }
  }

  auto decl = expr->getCalleeDecl();
  const auto *function = decl ? decl->getAsFunction() : nullptr;
  // The resolved member function IS the callee's declaration, so supplying it
  // here gives the arity/variadic/param-type queries below the right answers
  // and keeps `is_fn_ptr_call` false -- there is no function pointer involved.
  if (function == nullptr) {
    function = info.ptr_mem_callee;
  }
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
      // ⭐ THE MEASURED #3 COMPILE GATE LIVES HERE, not on a C++ local. The
      // annotation is a PARAMETER type pasted into a function-body `let`, so
      // `llvm::function_ref`'s faithful borrow model
      // `Option<&'a (dyn Fn() -> T + 'a)>` arrives with a `'a` that has no
      // binder in scope: `63546ba2` declares binders on the signature of the
      // function BEING EMITTED, and this `let` is inside the signature of the
      // function doing the CALLING, which mentions no `'a` at all. MEASURED on
      // dataflow-scheduler/.../Utils/PipelineTreeLegalizer.cpp:312, emitted
      // inside `unsafe fn collectViolations(&mut self, tree: *mut ...)`:
      //     let _action: Option<&'a (dyn Fn(...) -> ... + 'a)> = ...
      // Nothing here CAN declare a binder, and the region is inferred from the
      // initialiser -- which is what `'_` means. See `ElideNamedLifetimes`.
      StrCat(std::format("let {}: {} =", ca.param_name,
                         ElideNamedLifetimes(ToString(ca.param_type))));
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

  if (info.ptr_mem_callee != nullptr) {
    // Direct call through a constant pointer-to-member. This is byte-for-byte
    // the emission VisitMemberExpr already performs for `obj->method()`: the
    // UFCS name, plus the receiver threaded through `ufcs_receiver_`, which
    // EmitArgList prepends as the first argument. Nothing models the member
    // pointer, because there is no member pointer left at this point -- see
    // ResolveConstantMemberPointerCallee for why that is sound here and why it
    // is deliberately not attempted for a member pointer that is a value.
    SetUFCSReceiver(info.ptr_mem_object, info.ptr_mem_is_arrow,
                    info.ptr_mem_callee);
    StrCat(GetUFCSName(info.ptr_mem_callee), token::kDoubleColon,
           GetMethodName(info.ptr_mem_callee));
  } else if (info.resolved_overload) {
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
    // ⛔ NON-CALLABLE CALLEE GUARD.  Reaching here emits the callee EXPRESSION in
    // callee position.  That is correct for a function/function-pointer callee,
    // but for an `operator()` invocation the callee expression is the RECEIVER
    // OBJECT, and the method must instead be spelled UFCS -- which is exactly what
    // ConvertUserOperatorCall does for a project functor
    // (`MyCmp::operator_call(&<MyCmp>::default(), a, b)`).  So an OO_Call whose
    // resolved callee is a CXXMethodDecl that gets this far has LOST the method,
    // and the receiver's MAPPED VALUE lands in callee position.
    //
    // MEASURED, 2026-09-29: for a system functor whose rule models the type as a
    // scalar -- rules/hash keys `t2 :: std::hash<T1>` as `usize` with no
    // `operator()` key -- the receiver's mapped value is `usize`'s default, so
    // `std::hash<T>()(v)` emitted the literal callee `0(v)`:
    //     return ((unsafe { 0((*x).storage_) }) ^ ((unsafe { 0((*x).unit_) }) << 1))
    // 427 such sites across 190 of 312 emitted TUs.  ⛔ THE CLASS IS INVISIBLE TO
    // EVERY EXISTING DETECTOR: rc=0, rustfmt-clean, no placeholder token, and not
    // even an undefined name -- `0` is an ordinary integer literal.  It surfaces
    // only as rustc E0618 "expected function, found {integer}", i.e. only behind a
    // TU that type-checks, and no corpus TU did.
    //
    // ⛔ THE ASYMMETRY THIS CLOSES: `std::less<int>` has NO type key, so it aborts
    // loudly via ReportUnmappedSystemType.  `std::hash<int>` HAS one, so it was
    // silently wrong.  Having a type key but no `operator()` key was therefore
    // STRICTLY WORSE than having no key at all, because it bypassed the loud path.
    // Under this project's fail-loudly rule the missing key must abort, not emit.
    //
    // WHY THE PREDICATE IS STRUCTURAL and not "is the mapped type a scalar".  A
    // scalar-typed receiver is only the shape this class happened to take; a
    // functor modelled as `()` (rules/plus) or as an opaque handle is just as
    // non-callable.  The load-bearing fact is that an OO_Call on a CXXMethodDecl
    // has NO correct generic emission at all, whatever the receiver maps to.
    //
    // NEGATIVE CONTROLS, and they hold BY CONSTRUCTION, not by luck: every working
    // project-functor form (`MyCmp()(a,b)`, `MyHash()(v)`, `MyCmp c; c(a,b)`) is
    // routed to ConvertUserOperatorCall by ConvertCallExpr (:4988), and a functor
    // with an `operator()` RULE KEY is routed to the `Mapper::Contains` arm
    // (:4985).  Neither reaches EmitCall, so neither can trip this.
    // ⛔ LAMBDAS ARE EXCLUDED, AND THE EXCLUSION IS LOAD-BEARING, NOT A SOFTENING.
    // A lambda call `f(a, b)` is ALSO an OO_Call whose callee decl is the closure
    // type's `operator()`, so the structural test above matches it -- but for a
    // lambda the generic emission is CORRECT, because a C++ closure is modelled as
    // a RUST CLOSURE and a Rust closure IS callable in callee position.  Converting
    // the closure object and calling it is exactly right there.
    //
    // MEASURED, 2026-09-29, and this is why the exclusion exists: without it the
    // guard's FIRST abort on dsc/dims.cpp was not the hash class at all but
    //     unsupported call operator has no rule:
    //       `void operator()(std::map<std::string, BaseFuncType> &,
    //                        const std::map<BaseFuncType, std::string> &) const`
    //       on receiver `auto operator()(type-parameter-_-_ &, ...) const`
    // i.e. a lambda invocation -- a FALSE POSITIVE that would have flipped every
    // lambda-calling TU to an abort.  The receiver type printing as
    // `auto operator()(type-parameter-_-_ &, ...) const` is the tell: that is a
    // closure type, which has no name to print.
    //
    // The discriminator is `CXXRecordDecl::isLambda()` on the method's PARENT --
    // the closure record itself -- not any property of the call site, so it cannot
    // be confused by a lambda stored in a variable, passed through a template
    // parameter, or invoked via an explicit `.operator()(...)`.
    auto *opcall = clang::dyn_cast<clang::CXXOperatorCallExpr>(info.expr);
    const auto *call_op =
        opcall != nullptr
            ? clang::dyn_cast_or_null<clang::CXXMethodDecl>(
                  opcall->getDirectCallee())
            : nullptr;
    if (call_op != nullptr && opcall->getOperator() == clang::OO_Call &&
        call_op->getParent() != nullptr && !call_op->getParent()->isLambda()) {
      // In survey mode ReportNonCallableCallee RECORDS and peels the operands
      // itself, so bail here rather than falling through and converting both the
      // callee and the argument list a second time.
      ReportNonCallableCallee(opcall);
      return;
    }
    PushExprKind push(*this, ExprKind::Callee);
    Convert(GetCallee(info.expr));
  }

  EmitArgList(info);
}

// The refusal for the guard in EmitCall above.  Shape and `gen_crash_diag=false`
// copied from ReportUnsupportedOperatorCall (:6346) for the reason recorded
// there: the default abort()s into a ~36-frame backtrace and rc=134, i.e. a
// deliberate refusal that reads as a crash.
//
// The message names the RULE KEY TO WRITE, because the fix for every instance of
// this class is an `operator()` key on the receiver's type.  It is printed with
// `Mapper::ToString(callee)` -- the same printer the rule matcher searches with --
// so the spelling can be pasted into a rule source; and the receiver TYPE is
// printed with ScalarSugar::kPreserve so a scalar-modelled functor is visible as
// what it is.
void Converter::ReportNonCallableCallee(clang::CXXOperatorCallExpr *expr) {
  const auto *callee = expr->getDirectCallee();
  const std::string key = callee ? Mapper::ToString(callee) : "<unresolved>";
  const std::string recv =
      expr->getNumArgs() > 0 && expr->getArg(0) != nullptr
          ? Mapper::ToString(expr->getArg(0)->getType(),
                             Mapper::ScalarSugar::kPreserve)
          : "<none>";
  const std::string loc =
      expr->getExprLoc().printToString(ctx_.getSourceManager());

  std::string detail = "call operator has no rule: `" + key +
                       "` on receiver `" + recv +
                       "` (the receiver's mapped VALUE would be emitted in "
                       "callee position, e.g. `0(v)`)";
  if (curr_function_ != nullptr) {
    detail += ", reached while converting `" +
              curr_function_->getQualifiedNameAsString() + "`";
  }

  if (survey::Enabled()) {
    survey::Record(survey::GapKind::kUnsupportedExpr, detail, loc);
    // Keep peeling, exactly as ReportUnsupportedOperatorCall does, so gaps
    // *behind* this one are found in the same run instead of one per sweep.
    for (auto *arg : expr->arguments()) {
      Convert(arg);
    }
    computed_expr_type_ = ComputedExprType::FreshValue;
    return;
  }

  llvm::report_fatal_error(llvm::Twine("unsupported ") + detail + " at " + loc,
                           /*gen_crash_diag=*/false);
}

// ⭐⭐ THE LOUD HALF OF RULE 2, FOR A MEMBER. Same shape, same wording skeleton
// and the same `reached while converting` half as ReportNonCallableCallee above
// and as the type-level `ReportUnmappedSystemType`, so the census harness
// classifies the TU as bucket B with NO harness change: the line begins
// `LLVM ERROR: unsupported ` and ends ` at <file:line:col>`.
//
// WHAT IT REPLACES, and this is the whole point of the row: without it the only
// two states a rule author could express for a member were
//   (1) no key   -> silent fall-through, literal C++ method name, rc=0, E0599
//                   one stage later, invisible to every census here;
//   (2) a key    -> a body the author has just decided cannot be written
//                   correctly.
// Both are wrong for a member whose semantics are not portable today. This is
// the third state: the key is DECLARED so the miss is attributable, and NO
// translation is invented.
//
// ⛔ Survey mode is handled exactly as the other reporters handle it -- record
// and keep peeling -- so a survey sweep still finds the gaps BEHIND this one
// instead of one per run. A refusal is a gap report, not a crash.
bool Converter::RefuseIfRefusedMember(clang::CallExpr *expr) {
  if (const auto *refused = Mapper::GetRefusedRule(expr->getCallee())) {
    ReportRefusedMember(expr, *refused);
    return true;
  }
  return false;
}

void Converter::ReportRefusedMember(
    clang::CallExpr *expr, const TranslationRule::RefusedRule &refused) {
  const auto *callee = expr->getDirectCallee();
  const std::string key =
      callee != nullptr ? Mapper::ToString(callee) : Mapper::ToString(expr);
  const std::string loc =
      expr->getExprLoc().printToString(ctx_.getSourceManager());

  std::string detail =
      "member is DELIBERATELY REFUSED: `" + key + "` (rule `" + refused.origin +
      "`, refused src `" + refused.src +
      "`); the rule tree declares this member unportable rather than emitting "
      "the literal C++ name, which would be rc=0 here and rustc E0599 one "
      "stage later";
  if (curr_function_ != nullptr) {
    detail += ", reached while converting `" +
              curr_function_->getQualifiedNameAsString() + "`";
  }

  if (survey::Enabled()) {
    survey::Record(survey::GapKind::kUnsupportedExpr, detail, loc);
    for (auto *arg : expr->arguments()) {
      Convert(arg);
    }
    computed_expr_type_ = ComputedExprType::FreshValue;
    return;
  }

  llvm::report_fatal_error(llvm::Twine("unsupported ") + detail + " at " + loc,
                           /*gen_crash_diag=*/false);
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
  // A `CK_UserDefinedConversion` node is pure noise: its sub-expression is the
  // `CXXMemberCallExpr` on the conversion operator, which already carries the
  // conversion's result type. It was absent from this switch and so landed in
  // `default:`, which synthesised a spurious `as T` around the call whenever
  // the target mapped to a different Rust type -- the `(id as TypeID)()` shape.
  case clang::CastKind::CK_UserDefinedConversion:
  case clang::CastKind::CK_DerivedToBase:
  // `CK_UncheckedDerivedToBase` is the SAME conversion as `CK_DerivedToBase`
  // with the null check elided, and clang emits it for EVERY implicit object
  // argument, i.e. for every call of an inherited member. It was absent from
  // this switch -- one grep for `DerivedToBase` over the whole converter
  // returned exactly one hit -- so it landed in `default:`, which synthesised a
  // spurious `as T` on a non-primitive. A derived-to-base conversion is never a
  // Rust cast.
  //
  // THREE MEASURED SHAPES, 2026-09-28, all of them from this one missing case
  // (45 sites over 5 bucket-A TUs, `as Vec<` alone going 45 -> 0):
  //  1. Base mapped to a DIFFERENT Rust type. Probe from
  //     dcc/.../Analyses/AddressPinningScheme.h (`using AddrTy = int64_t;
  //     AddrListTy pinned_addrs_`, `pinned_addrs_.size()` in a const method):
  //         Vec::len(&(self.pinned_addrs_ as Vec<u32>))
  //     -> rustc `error[E0605]: non-primitive cast: Vec<i64> as Vec<u32>`.
  //     The target is `Vec<u32>` while the field is `Vec<i64>` because the base
  //     `SmallVectorImpl<T>` is mapped with `T` unsubstituted -- which is also
  //     why `IsCastRedundantInRust` did not suppress it, and why the shape is
  //     INVISIBLE whenever the element type happens to be `unsigned`.
  //  2. Base UNMAPPED, so the cast target was a fabricated
  //     `Cpp2RustUnmapped_*` name -- `mlir::OpTrait::OneTypedResult<..>::Impl`,
  //     `mlir::IROperand<OpOperand, Value>`, `llvm::SmallPtrSetImplBase`,
  //     `std::ios`. rustc `error[E0425]: cannot find type ...`, and these were
  //     REACHED: AddressPinningAndToggle.cpp 1988 -> 1942 errors,
  //     RegisterTypeAssignment.cpp 1725 -> 1710, dxp_standalone.cpp 268 -> 264,
  //     every single delta a removal of exactly this cast.
  //  3. Base mapped to the SAME Rust type, e.g. `(self.next_ as
  //     Vec<Option<Box<dsc2_ScheduleNode>>>).is_empty()` and
  //     `(*(self.loopNode as *mut SenPcfgNode)).name`. Here `as` is a trivial
  //     COERCION cast, so the value form is a MOVE out of `&self` (E0507) and
  //     the pointer form is an unsound reinterpretation that COMPILES.
  //
  // ⚠️ WHAT THIS DOES **NOT** FIX, stated because shape 3 looks fixed and is
  // not: when the base subobject's own FIELD is then read, dropping the cast
  // leaves `(*self.loopNode).name` -- and the emitted `SenPcfgMvloopNode`
  // (dsc/pcfg.h:149, `: public SenPcfgNode`) carries NONE of the base's fields,
  // so there is no `name` to project to and no spelling for one. That site was
  // silently reading the wrong bytes before and is a loud E0609 now; the base
  // projection itself is a separate, unfixed row (ABSTRACT-BASE-DESIGN.md).
  case clang::CastKind::CK_UncheckedDerivedToBase:
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

// Mark a FUNCTION name the mapper had no rule for, exactly the way an unmapped
// TYPE is marked at converter.cpp:233 -- and say so on stderr.
//
// WHY. `ConvertDeclRefExpr`'s function branch below is reached ONLY after
// `Mapper::Contains(callee)` returned false (VisitCallExpr:4311) and
// `ShouldReplaceWithMappedBody` returned false, i.e. only after a SEARCH MISS.
// It then returned `GetNamedDeclAsString(canonical)`, which is byte-for-byte the
// name a PORTED function gets, and printed NOTHING. So `cast<T>(x)` emitted
// `cast_29 ( x )`: a plausible, undefined, unannounced identifier.
//
// The type path had both halves (a `Cpp2RustUnmapped_` spelling *and* a
// `no rule` note); the function path had neither. That asymmetry is the whole
// defect. Measured over the 312-TU fresh38 sweep: 304 TUs (97.4%) carry at least
// one such name, 8366 distinct names, 155962 call sites, three names
// (`cast`/`dyn_cast_or_null`/`next`) accounting for ~80% of the sites. Every one
// of those TUs is bucket A, rc=0 and rustfmt-clean, so the gap is invisible to
// every bucket census -- and `rustc` stops at NAME RESOLUTION, so the type and
// borrow errors in the rest of the file are UNOBSERVABLE. The undefined name is
// not a local defect, it is a measurement blackout over the entire TU.
//
// ⚠️ THE DISCRIMINATOR IS NOT THE NAME SHAPE. `senComponentsToString_0` (a
// genuinely ported project global) and `get_109` (a missed llvm accessor) are
// indistinguishable by spelling; a `_<N>`-suffix rule would rename real decls.
// The test used here is whether the project can EVER emit an item for the decl:
// `VisitTranslationUnitDecl` (converter.cpp:590) only descends into decls that
// pass `IsUserDefinedDecl`, so a function whose canonical declaration sits in a
// SYSTEM header is never ported by this TU nor by any other -- which is precisely
// the criterion `ReportUnmappedSystemType` already uses. In the real compile
// database llvm and mlir arrive through `-isystem`
// (`-isystem .../LLVM-22.1.3-Linux-X64/include`), so `cast`, `dyn_cast_or_null`,
// `isa`, `get` and `registerPass` all land here, while the project's own
// dataflow-scheduler headers arrive through plain `-I` and do not.
//
// ⛔ DELIBERATELY CONSERVATIVE, IN ONE DIRECTION ONLY. Three cases are left
// UNMARKED even though some of them are undefined today:
//   * a non-system decl with no definition in this TU (defined in a sibling
//     `.cpp`). Marking it would break a WHOLE-PROGRAM emission, where the other
//     TU's `fn helper_7` does define the name. Under-marking costs coverage;
//     over-marking would turn a linkable call into a broken one.
//   * an invalid location (compiler builtins), which `isInSystemHeader` cannot
//     classify and which the libc passthrough rules usually catch earlier.
//   * static methods, which leave through `GetFunctionRefName` above and are a
//     different naming path (`Record::method`), not a bare identifier.
// The `Cpp2RustUnmappedFn_` prefix is emitted by nothing else, so -- unlike the
// bare mangled name -- it can only ever fail to resolve, never silently bind to
// an unrelated `fn` of the same name emitted in the same file (same argument as
// converter.cpp:145). It is distinct from the type side's `Cpp2RustUnmapped_` and
// from `Cpp2RustUnmappedExpr_`/`Cpp2RustUnmappedStmt_` so a census can attribute
// a site to the callee path by prefix alone.
static bool IsSystemFunctionDecl(const clang::FunctionDecl *fn) {
  const clang::FunctionDecl *canonical = fn->getCanonicalDecl();
  const auto &src_mgr = canonical->getASTContext().getSourceManager();
  const clang::SourceLocation loc = canonical->getLocation();
  if (loc.isInvalid()) {
    return false;
  }
  return src_mgr.isInSystemHeader(loc) || src_mgr.isInSystemMacro(loc);
}

std::string Converter::MarkUnmappedFunctionRef(const clang::FunctionDecl *fn,
                                               std::string name) {
  if (!IsSystemFunctionDecl(fn)) {
    return name;
  }
  const std::string marked = "Cpp2RustUnmappedFn_" + name;
  static std::set<std::string> reported;
  if (reported.insert(name).second) {
    std::string detail = "system function has no rule: `" +
                         fn->getQualifiedNameAsString() + "` : `" +
                         fn->getType().getAsString() +
                         "` (would be emitted as the undefined name `" + name +
                         "`); emitting the marked placeholder `" + marked +
                         "` so the miss is visible";
    if (curr_function_ != nullptr) {
      detail += ", reached while converting `" +
                curr_function_->getQualifiedNameAsString() + "`";
    }
    llvm::errs() << "note: called " << detail << " (declared at "
                 << fn->getCanonicalDecl()->getLocation().printToString(
                        ctx_.getSourceManager())
                 << ")\n";
  }
  return marked;
}

// ⭐ THE VARIABLE HALF OF THE UNDEFINED-NAME CLASS, and the #1 compile-level gate
// that a FUNCTION census is structurally blind to: the site is a variable, not a
// call, so `Cpp2RustUnmappedFn_` above could never reach it. Measured on the
// 210-TU compile census (stubgen, first-abort ranking): E0425 is the first abort
// of 80 of 207 measurable TUs, and 19 of those 80 abort on a global/static
// LazyCell reference whose `static` item is nowhere in the file --
//   (*std::cell::LazyCell::force_mut(&mut *&raw mut digits_25))
// with no `digits_25` anywhere. Two DIFFERENT causes share this one emission
// site, which is why the function-local-static form and the header-global form
// ("Class G", 71 names / 31 files) are ONE defect:
//   * `static constexpr int digits = std::numeric_limits<size_t>::digits;`
//     -- the PROJECT's `digits_24` IS emitted; its INITIALIZER refers to the
//     system static data member `numeric_limits<size_t>::digits`, which is not
//     (8 of the 19 TUs: dbo/Pipeline, dbo/Utils/sdsc_bundle/*, ...).
//   * `if (llvm::DebugFlag)` -- a file-scope `extern bool` in an `-isystem`
//     header, emitted as `DebugFlag_117` and defined by NO TU in the corpus
//     (5 of the 19).
// The remaining 6 (`rapidUnitAccessMap_<N>`) are NOT this class and are
// deliberately left alone: `SystemDefinitions::rapidUnitAccessMap` is a PROJECT
// static data member, defined in `sys-arch-spec/sysdef.cpp`, and IS emitted --
// as `rapidUnitAccessMap_39` in `sys-arch-spec__sysdef.cpp.rs`. Their defect is a
// cross-TU decl-id MISMATCH (_39 vs _75/_37/_94/_170), a different row; marking
// them would break a whole-program emission where the name does resolve. Same
// conservative direction as the callee path above.
//
// TWO OUTCOMES, and the split is on whether C++ itself fixes the value:
//  1. CONSTANT-FOLD. A system integral constant has exactly one value and clang
//     evaluates it right here, so emitting that value is not a fabricated
//     initializer -- it is the value C++ produces. `numeric_limits<size_t>::digits`
//     becomes `64`. ⛔ NOT done when the address is taken (`&64` would be a
//     temporary, not the object) and not for non-integral or non-constant decls.
//  2. MARK. Anything else gets `Cpp2RustUnmappedVar_<name>` plus a `note:`.
//     ⛔ Deliberately NOT wrapped in `LazyCell::force_mut(&raw mut ...)`: that
//     wrapper asserts a `LazyCell` item exists for the decl, and the whole point
//     is that it never will. This does NOT lower the E0425 count for those sites
//     and is not claimed to -- it converts a name that is INDISTINGUISHABLE from
//     a real project global (`DebugFlag_117` would silently bind to a project
//     `DebugFlag` that happened to get decl id 117 in a whole-program build --
//     the converter.cpp:145 hazard) into one that can only ever fail to resolve
//     and is attributable by prefix alone.
// ⛔ A fabricated initializer is forbidden here for a reason: a C++
// function-local `static` initialises exactly once, on first use, and persists
// across calls, so a wrong initializer or a per-call re-init is silently wrong in
// a way nothing downstream would catch.
static const clang::VarDecl *SystemGlobalVarDecl(const clang::ASTContext &ctx,
                                                 const clang::Expr *expr) {
  const auto *ref = clang::dyn_cast<clang::DeclRefExpr>(expr->IgnoreImplicit());
  if (ref == nullptr) {
    return nullptr;
  }
  const auto *var = clang::dyn_cast<clang::VarDecl>(ref->getDecl());
  if (var == nullptr) {
    return nullptr;
  }
  const clang::VarDecl *canonical = var->getCanonicalDecl();
  const clang::SourceLocation loc = canonical->getLocation();
  if (loc.isInvalid()) {
    // Compiler builtins; `isInSystemHeader` cannot classify these. Left to the
    // existing path, same as the callee side.
    return nullptr;
  }
  const auto &src_mgr = ctx.getSourceManager();
  if (!src_mgr.isInSystemHeader(loc) && !src_mgr.isInSystemMacro(loc)) {
    return nullptr;
  }
  return canonical;
}

std::string Converter::ConvertSystemGlobalVarRef(clang::DeclRefExpr *expr) {
  const clang::VarDecl *canonical = SystemGlobalVarDecl(ctx_, expr);
  if (canonical == nullptr) {
    return {};
  }
  const auto &src_mgr = ctx_.getSourceManager();
  const clang::SourceLocation loc = canonical->getLocation();

  std::string name = GetNamedDeclAsString(expr->getDecl());
  if (!isAddrOf() && !expr->isValueDependent() &&
      expr->getType()->isIntegralOrEnumerationType()) {
    clang::Expr::EvalResult result;
    if (expr->EvaluateAsRValue(result, ctx_) && result.Val.isInt() &&
        !result.hasSideEffects()) {
      if (expr->getType()->isBooleanType()) {
        return result.Val.getInt().getBoolValue() ? std::string(keyword::kTrue)
                                                  : std::string(keyword::kFalse);
      }
      llvm::SmallString<40> digits;
      result.Val.getInt().toString(digits, 10);
      return std::string(digits);
    }
  }

  const std::string marked = "Cpp2RustUnmappedVar_" + name;
  static std::set<std::string> reported;
  if (reported.insert(name).second) {
    std::string detail =
        "system global has no rule and no constant value: `" +
        canonical->getQualifiedNameAsString() + "` : `" +
        canonical->getType().getAsString() +
        "` (would be emitted as the undefined name `" + name +
        "`); emitting the marked placeholder `" + marked +
        "` so the miss is visible";
    if (curr_function_ != nullptr) {
      detail += ", reached while converting `" +
                curr_function_->getQualifiedNameAsString() + "`";
    }
    llvm::errs() << "note: read " << detail << " (declared at "
                 << loc.printToString(src_mgr) << ")\n";
  }
  return marked;
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
    return MarkUnmappedFunctionRef(
        function, GetNamedDeclAsString(function->getCanonicalDecl()));
  }

  if (auto enum_constant = clang::dyn_cast<clang::EnumConstantDecl>(decl)) {
    auto name = EnumeratorName(enum_constant);
    if (!expr->getType()->isEnumeralType()) {
      return std::format("({} as i32)", name);
    }
    return name;
  }

  if (IsGlobalVar(expr)) {
    // A system-header global/static is never emitted by ANY TU, so the LazyCell
    // wrapper below would reference an item that does not exist.
    if (auto str = ConvertSystemGlobalVarRef(expr); !str.empty()) {
      return str;
    }
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
          // A reference to a lambda-initialised variable is lowered by INLINING
          // the lambda body right here. A lambda that calls ITSELF through that
          // variable --
          //   std::function<void(const T *, unsigned)> f =
          //       [&](const T *n, unsigned d) { ... f(child, d + 1); ... };
          // -- therefore re-enters this site on the SAME VarDecl and inlines
          // forever. Unbounded recursion, not a hang: it exhausts the 8 MB stack
          // and SIGSEGVs at whichever frame happens to touch the guard page,
          // with no diagnostic at all. That is why this one construct read as a
          // "genuine NULL clang::Expr" for two days and appeared to MOVE between
          // unrelated commits: the reported crash site was
          //   ConvertMemberExpr -> ConvertDeref -> Convert(Expr*)        (pin)
          //   VisitDeclRefExpr  -> ConvertDeclRefExpr -> GetMappedAsString
          //                     -> Mapper::search -> IsUserDefinedDecl
          //                     -> SourceManager::getFileCharacteristic (907226d5)
          // i.e. it tracked frame sizes, not a bug at any of those places. The
          // repeating cycle is visible in the dump itself:
          //   ConvertFunctionBody -> Convert(Stmt*) -> ... -> VisitDeclRefExpr
          //   -> VisitLambdaExpr -> ConvertFunctionBody -> ...
          // MEASURED on dataflow-scheduler/lib/Analysis/PipelineTree.cpp:251
          // (`findDeepestLoop`): one MemberExpr re-converted 4088 times before
          // the fault, 40797 mapper searches for a TU that emits nothing.
          //
          // A CORRECT lowering needs the lambda emitted ONCE as a named Rust
          // item (or a fix-point wrapper) so that the self-reference lowers to a
          // CALL instead of an inline; the inline-at-every-reference strategy
          // cannot express it at all. Until that exists this refuses LOUDLY and
          // names the variable: simply not inlining on re-entry would drop the
          // recursive call and emit a silently wrong lowering, which is strictly
          // worse than the crash it replaces.
          if (!inlining_lambda_vars_.insert(var_decl).second) {
            const std::string detail =
                "self-recursive lambda `" + var_decl->getNameAsString() +
                "`: the lambda assigned to it refers to itself, so there is no "
                "finite inline expansion of its body (a correct lowering must "
                "emit the lambda as a named item and call it)";
            const std::string loc =
                var_decl->getLocation().printToString(ctx_.getSourceManager());
            if (survey::Enabled()) {
              survey::Record(survey::GapKind::kUnsupportedConstruct, detail,
                             loc);
              // Do NOT recurse: the survey is a work list, so record the gap and
              // emit the name, which is what a non-recursive reference emits.
              // Deliberately NOT erasing here -- the entry belongs to the OUTER
              // expansion that is still on the stack, and dropping it would let
              // the next self-reference in the same body start the recursion over.
              StrCat(str);
              SetValueFreshness(expr->getType());
              return false;
            }
            llvm::report_fatal_error(llvm::Twine("unsupported ") + detail +
                                         " at " + loc,
                                     /*gen_crash_diag=*/false);
          }
          PushParen paren(*this);
          VisitLambdaExpr(lambda);
          inlining_lambda_vars_.erase(var_decl);
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

  // ⛔ THIS WAS `llvm::errs() << ...; assert(0 && ...)` AND THE assert IS COMPILED
  // OUT OF THE SHIPPING PIN (-DNDEBUG), so the function RETURNED having emitted NO
  // TEXT for the type and the converter carried on with an empty spelling. Measured
  // 2026-09-28 on the 403-TU sweep: SIX of the eight bucket-`C` "SEGV-no-diag" TUs
  // reach this line, and the SEGV they were classified by is a DOWNSTREAM VICTIM of
  // the empty spelling, several frames later and in four different places
  // (EmitMaterializedTempBinding, Convert(Expr*), ConvertDeclRefExpr,
  // std::filesystem::canonical) -- which is why the class taught us nothing. An
  // asserts-ON build turns all six into this one named assertion.
  // Same measured reason, and the same fix, as ReportUnsupportedException and the
  // structured-binding refusal above: refuse LOUDLY in every build configuration
  // rather than emitting a silently wrong lowering. `gen_crash_diag=false` because
  // the default abort()s, which the SIGABRT handler turns into a backtrace and
  // rc=134, i.e. a refusal that reads as a crash.
  llvm::report_fatal_error(llvm::Twine("unsupported ") + detail + " at " + loc,
                           /*gen_crash_diag=*/false);
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

  // ⛔ SAME `assert(0)` FALL-THROUGH AS ReportUnmappedSystemType (:5809): the assert
  // is compiled out of the shipping pin (-DNDEBUG), so this RETURNED having emitted
  // NO TEXT for the operator call and the converter carried on. Measured 2026-09-28
  // on the 403-TU sweep: dcc/src/Transform/Sentient/Utils.cpp is a bucket-`C`
  // "SEGV-no-diag" TU whose crash is a DOWNSTREAM VICTIM of this silent return, and
  // an asserts-ON build turns it into this named assertion. Refuse loudly in every
  // build configuration instead; `gen_crash_diag=false` so the refusal does not
  // read as a crash (the default abort()s into a backtrace and rc=134).
  llvm::report_fatal_error(
      llvm::Twine("unsupported CXXOperatorCallExpr: ") + spelling + " on (" +
          operands + ")" +
          (key.empty() ? std::string(" rule key: <unresolved callee>")
                       : " rule key: " + key) +
          " at " + loc,
      /*gen_crash_diag=*/false);
}

// True when `type` is a `std::pair` whose every element is a type for which
// Rust's derived tuple `PartialEq` is EXACTLY C++'s `std::pair::operator==`,
// i.e. a builtin scalar, an enum, or a pointer. `std::pair<A, B>::operator==`
// is specified as `a.first == b.first && a.second == b.second`, and the Rust
// model of `std::pair` is a tuple `(A, B)`, whose `PartialEq` is the same
// lexicographic-conjunction over the same fields -- so for scalar elements the
// two agree bit for bit (integers compare by value, pointers by address,
// floats with the same IEEE NaN behaviour).
//
// ⚠️ DELIBERATELY NOT WIDENED past scalars. The moment an element has a
// user-provided `operator==` with non-structural semantics (a case-insensitive
// string, an interned handle, an epsilon float compare), a derived Rust `==`
// would be SILENTLY WRONG output rather than a compile error. Those keep
// falling through to ReportUnsupportedOperatorCall, which is the correct
// trade: an operator that fails loudly beats one that compares the wrong way.
static bool IsScalarElementStdPair(clang::QualType type) {
  auto *record = type.getNonReferenceType()
                     .getUnqualifiedType()
                     ->getAsCXXRecordDecl();
  if (!record || record->getQualifiedNameAsString() != "std::pair") {
    return false;
  }
  const auto *spec =
      clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(record);
  if (!spec) {
    return false;
  }
  const auto &args = spec->getTemplateArgs();
  if (args.size() != 2) {
    return false;
  }
  for (unsigned i = 0; i < args.size(); ++i) {
    if (args[i].getKind() != clang::TemplateArgument::Type) {
      return false;
    }
    auto element = args[i].getAsType();
    // `bool`/enum/integer/float/pointer only. Anything with a record type --
    // including a nested `std::pair` -- is refused, because its `operator==`
    // may not be structural.
    if (!element->isIntegralOrEnumerationType() &&
        !element->isFloatingType() && !element->isPointerType()) {
      return false;
    }
  }
  return true;
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
  case clang::OverloadedOperatorKind::OO_EqualEqual:
  case clang::OverloadedOperatorKind::OO_ExclaimEqual:
    // `std::pair` compared with the libstdc++/libc++ free `operator==`. The
    // callee lives in a system header, so IsUserOperatorCall() is false and
    // ConvertCallExpr() routes here; no rule key matched either, so before this
    // arm existed every one of these fell into `default:` and became a
    // `Cpp2RustUnmappedExpr_CXXOperatorCallExpr`. The Rust model of the operand
    // is a tuple, for which `==` is native and structurally identical -- see
    // IsScalarElementStdPair for exactly how far that identity is trusted.
    if (expr->getNumArgs() == 2 &&
        IsScalarElementStdPair(expr->getArg(0)->getType()) &&
        IsScalarElementStdPair(expr->getArg(1)->getType())) {
      StrCat(std::format(
          "(({}) {} ({}))", ConvertRValue(expr->getArg(0)),
          expr->getOperator() == clang::OverloadedOperatorKind::OO_EqualEqual
              ? "=="
              : "!=",
          ConvertRValue(expr->getArg(1))));
      computed_expr_type_ = ComputedExprType::FreshValue;
      break;
    }
    ReportUnsupportedOperatorCall(expr);
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
  } else if (auto *conversion =
                 clang::dyn_cast<clang::CXXConversionDecl>(member)) {
    // A LOST USER-DEFINED CONVERSION used to land in the silent fall-off below.
    //
    // A `CXXConversionDecl`'s DeclName kind is `CXXConversionFunctionName`, NOT
    // an identifier, so the `isIdentifier()` arm was false; it is not an
    // overloaded operator and a record cannot declare two conversions to the
    // same type, so `IsOverloadedMethod` (converter_lib.cpp:344, which needs a
    // same-name count > 1) was false too. With no terminal `else`, the member
    // name was DISCARDED: only the base printed, and `EmitArgList` then
    // appended `()`, so `if (attr)` on a class with `operator bool` came out as
    // `if (unsafe { attr() })` -- a call on a value that is not a function.
    // That is silently-wrong output wherever the name happens to resolve.
    //
    // `GetMethodName` already routes `CXXConversionDecl` to `GetConversionName`
    // (:1410), which is what emits the `to_bool` / `to_TypeID` names the
    // declaration side has always produced -- and which, before this fix, were
    // dead code with ZERO call sites in the whole emitted corpus.
    StrCat(token::kDot);
    StrCat(GetMethodName(conversion));
  } else if (!name_override.empty()) {
    StrCat(token::kDot, name_override);
  } else if (member->getDeclName().isIdentifier()) {
    StrCat(token::kDot);
    StrCat(GetNamedDeclAsString(member));
  } else {
    // THE MISSING `else` WAS THE ENTIRE DEFECT. Any DeclName kind that is
    // neither an identifier nor one of the arms above (`CXXOperatorName`,
    // `CXXLiteralOperatorName`, `CXXDeductionGuideName`, ...) used to fall off
    // the end of this function emitting NOTHING AT ALL -- no `.`, no name --
    // leaving a bare base expression that the caller then decorated with an
    // argument list. Refuse loudly instead, and emit no token: a fabricated or
    // plausible-looking name could silently resolve to an unrelated item.
    ReportUnsupportedMemberName(expr, member);
  }
}

// A `MemberExpr` whose member name has no lowering in `ConvertMemberExpr`.
//
// This is the terminal `else` that did not exist until the lost-conversion row.
// It MUST NOT emit a token and MUST stop the run: the failure mode it replaces
// is the silent-drop class (nothing emitted, converter exits 0, the member
// access turns into a call on the base), which the playbook ranks strictly
// worse than a loud abort. No `todo!()`, no `unimplemented!()`, no
// `UNSUPPORTED`, no placeholder identifier -- an unnamed member is not an
// unmappable construct with a known site, it is a converter gap, and naming it
// at translate time is the only way it cannot be mistaken for working output.
//
// Under --survey this RECORDs and returns, because a survey run must enumerate
// every gap in one pass (same contract as ReportUnsupportedException).
void Converter::ReportUnsupportedMemberName(const clang::MemberExpr *expr,
                                            const clang::NamedDecl *member) {
  const std::string loc =
      expr->getExprLoc().printToString(ctx_.getSourceManager());
  std::string full =
      std::string("member access whose DeclName kind (") +
      std::to_string(static_cast<int>(member->getDeclName().getNameKind())) +
      ") has no name lowering, spelled `" +
      member->getDeclName().getAsString() + "`, on member of class `" +
      std::string(member->getDeclKindName()) + "`";
  if (curr_function_ != nullptr) {
    full += ", reached while converting `" +
            curr_function_->getQualifiedNameAsString() + "`";
  }
  if (survey::Enabled()) {
    survey::Record(survey::GapKind::kUnsupportedConstruct, full, loc);
    return;
  }
  llvm::report_fatal_error(llvm::Twine("unsupported ") + full + " at " + loc,
                           /*gen_crash_diag=*/false);
}

bool Converter::VisitCXXThisExpr(clang::CXXThisExpr *expr) {
  if (curr_function_ == nullptr) {
    ReportThisWithoutEnclosingFunction(expr, "`this` expression");
    computed_expr_type_ = ComputedExprType::FreshPointer;
    return false;
  }
  if (clang::isa<clang::CXXConstructorDecl>(curr_function_)) {
    // `this` is a `&mut Self` in the in-place `impl Default` arm and a `Self`
    // VALUE in ConvertCXXConstructorBody; `&raw mut this` is only right for the
    // second, so reborrow through the reference in the first.
    StrCat(this_is_mut_ref_ ? "&raw mut *this" : "&raw mut this");
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

// Is `expr` a place Rust is ALLOWED to move out of?  Exactly one shape is: a
// whole local variable or parameter, which the function owns.  Everything else
// -- `*p`, `p->f`, `self.f`, a reference-returning call, a global -- is a place
// the function does not own, and moving out of it is E0507 ("cannot move out of
// a raw pointer" / "of a shared reference").  A local of REFERENCE type is
// excluded too: its Rust model is a pointer, so the C++ `std::move(r)` is a move
// out of `*r`, not out of `r`.
//
// Used only to keep the move-constructor clone below from firing where C++'s
// move already translates to a plain Rust move -- `return attr;` must not become
// `return attr.clone();`.
static bool IsMovableRustPlace(const clang::Expr *expr) {
  const auto *e = IgnoreTransparentStdCall(expr)->IgnoreParenImpCasts();
  if (const auto *ref = clang::dyn_cast<clang::DeclRefExpr>(e)) {
    const auto *var = clang::dyn_cast<clang::VarDecl>(ref->getDecl());
    return var && var->hasLocalStorage() && !var->getType()->isReferenceType();
  }
  return false;
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
    // A DEFAULTED MOVE CONSTRUCTOR OF A *SYSTEM* RECORD WAS EXCLUDED, AND THAT
    // IS AN E0507 AT EVERY TABLEGEN `construct`.  IsDefaultedMoveConstructor
    // additionally requires IsUserDefinedDecl(ctor->getParent()) -- correct for
    // its other job (deciding which implicit members to PORT) but wrong here,
    // where the only question is whether the Rust model needs an owned value.
    // Measured witness, `AccessTileTypeStorage::construct` in KtdpDialect.cpp
    // (KeyTy = std::tuple<llvm::ArrayRef<int64_t>, mlir::Type>):
    //   auto shape       = std::move(std::get<0>(tblgenKey));
    //   auto elementType = std::move(std::get<1>(tblgenKey));
    // `mlir::Type` declares `Type(const Type &) = default`, which SUPPRESSES the
    // implicit move constructor, so overload resolution picks the COPY ctor and
    // the first disjunct already emits `.clone()`.  `llvm::ArrayRef<T>` declares
    // no copy ctor, so the implicit MOVE ctor is selected -- and because
    // `llvm::` is not user code the second disjunct was false, so the sibling
    // line got no `.clone()` and read `let mut shape: Vec<i64> = (*p);`, i.e.
    // `cannot move out of a raw pointer`.  Two lines apart, same shape, opposite
    // treatment.
    //
    // ⛔ NOT A BLANKET CLONE, AND THE TWO EXTRA GATES ARE BOTH MEASURED.  A
    // clone where C++ moved is a silent copy, so the new disjunct fires only for
    // the shape Rust genuinely cannot express:
    //   * `!TypeIsCopyable` (already there) asks about the RUST MODEL's derives,
    //     via Mapper::MappedDerives -- a system record whose model does derive
    //     Copy is untouched.  t19 `llvm::ArrayRef<T1>` -> `Vec<T1>` records no
    //     derives, so it is correctly non-Copy here.
    //   * `!isFresh()` drops the case where the operand already came out as an
    //     owned value; there is nothing to clone.
    //   * `!IsMovableRustPlace` drops the case where the operand is a whole local
    //     or parameter, which Rust CAN move out of.  Without it this row also
    //     rewrote 10 x `return attr;` -> `return attr.clone();` and
    //     `shape: shape` -> `shape: shape.clone()` in the same TU: all legal
    //     before and after, so all pure pessimisation.  Measured delta with the
    //     gate in place across the five dialect TUs: exactly the E0507 lines.
    // What survives is a move out of a place the function does not own, for which
    // `.clone()` is the conservative spelling.  `std::mem::take(&mut place)` is
    // the tighter translation of a genuine move (ConvertPlaceholder's kTake arm
    // uses it) but is NOT usable here: it is sound only when the source is
    // provably dead, and for a handle-modelled type (`ir::Ty`, `mlir::Attr`) it
    // would additionally clear a handle the caller may still read.  Clone matches
    // the copy-ctor arm above, which is what the `mlir::Type` sibling relies on.
    if ((ctor->isCopyConstructor() || IsDefaultedMoveConstructor(ctor) ||
         (ctor->isMoveConstructor() && !isFresh() &&
          !IsMovableRustPlace(expr->getArg(0)))) &&
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

// The lambda call operator's INSTANTIATED specialisations -- the ones that
// carry real (substituted) parameter and return types. Leaves `out` EMPTY for a
// non-generic lambda, whose `getLambdaCallOperator()` is already the real
// thing, and also for a generic lambda that this TU never calls (nothing was
// ever instantiated, so clang has no substituted type to give and the caller
// must keep whatever it would have emitted for the pattern).
void Converter::CollectLambdaCallOperatorInstantiations(
    const clang::CXXRecordDecl *lambda_class,
    llvm::SmallVectorImpl<clang::CXXMethodDecl *> &out) {
  const clang::CXXMethodDecl *pattern = lambda_class->getLambdaCallOperator();
  if (pattern == nullptr || !pattern->isTemplated()) {
    return;
  }
  if (auto *tmpl = pattern->getDescribedFunctionTemplate()) {
    for (auto *spec : tmpl->specializations()) {
      if (auto *method = llvm::dyn_cast<clang::CXXMethodDecl>(spec)) {
        if (method->hasBody()) {
          out.push_back(method);
        }
      }
    }
  }
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
  CollectLambdaCallOperatorInstantiations(expr->getLambdaClass(),
                                          instantiations);

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
  // A closure expression is a fresh, owned VALUE. This must be set AFTER
  // ConvertFunctionBody, because converting the body leaves
  // `computed_expr_type_` describing whatever the body's last expression was --
  // which is a statement of the closure's INTERIOR, not of the closure
  // expression itself. Without this the `Convert(clang::Expr*)` sentinel at
  // :2630 fires with `computed_expr_type_ not set by LambdaExpr` (or by the
  // MaterializeTemporaryExpr that wraps it, when the lambda is bound to a
  // temporary), which is `llvm::report_fatal_error` -- a hard abort AFTER the
  // closure text was already emitted.
  computed_expr_type_ = ComputedExprType::FreshValue;
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
  // See IsOstreamChainValue: an ostream `<<` chain is already the reference
  // VALUE a `std::ostream&` maps to in both models, so it must not be
  // address-of'd here.
  if (qual_type->isReferenceType() && !IsReferenceType(expr) &&
      !IsOstreamChainValue(expr)) {
    if (llvm::isa<clang::MaterializeTemporaryExpr>(expr->IgnoreImpCasts())) {
      StrCat(EmitMaterializedTempBinding(qual_type, expr));
      return;
    }
    if (auto *cond = clang::dyn_cast<clang::ConditionalOperator>(
            expr->IgnoreParenImpCasts());
        cond && cond->isLValue()) {
      // ⛔ THE PARENTHESES ARE LOAD-BEARING, NOT COSMETIC. `Convert(cond)` here
      // always emits an `if`/`else` BLOCK, and the `as` two lines down cannot
      // take a block as its left operand -- see
      // ParenthesizeBlockCastOperand(). Measured 2026-09-28: this exact site
      // was the ONLY defect in
      // dcc/src/Transform/Sentient/{AddressPinningAndToggle,
      // RegisterTypeAssignment}.cpp, 2 parse errors each, and both TUs were
      // otherwise complete bucket-A output failing solely on rustfmt (rc=1).
      const size_t cond_start = rs_code_->size();
      {
        PushExprKind push(*this, ExprKind::LValue);
        PushInitType init_type(*this, qual_type);
        Convert(cond);
      }
      ParenthesizeBlockCastOperand(cond_start);
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

// A rule TARGET may be spelled with a Rust LIFETIME BINDER. The faithful model
// of a non-owning, type-erased callable reference -- `llvm::function_ref`, which
// LLVM documents as never-stored and which appears in this corpus ONLY in
// parameter position -- is a BORROW, `Option<&'a (dyn Fn() -> T + 'a)>`, not an
// owning `Box<dyn Fn() -> T>`: a `Box` imposes `'static` plus an allocation and
// does not compile at the call site.
//
// ⛔ NOTHING IN SIGNATURE EMISSION EVER DECLARED THOSE BINDERS. The emitters at
// :590 / :1625 / :1711 go straight from the function NAME to `(` to the rendered
// parameter types -- there is no `<...>` slot anywhere between them -- so a `'a`
// coming out of a rule target reached the output UNDECLARED at every use site:
//     pub unsafe fn verify(                      // <-- no <'a> here
//         mut emitError: Option<&'a (dyn Fn() -> libcc2rs::InFlightDiagnostic + 'a)>,
// and rustc answers E0261 "use of undeclared lifetime name `'a`". That made the
// borrow -- the only faithful model -- unwritable, which is why a
// `rules/functional` slot had to revert a key whose reach was already PROVEN
// (anchored `Cpp2RustUnmapped_llvm_function_ref_...` went 3 -> 0 on
// dcc/.../Ktdp/KtdpTypes.cpp with the mapped spelling taking its exact place).
//
// So: collect the binders out of the RENDERED parameter and return spellings and
// declare them on the emitted function. Rendering is done through `ToString`,
// which redirects `rs_code_` into a scratch Buffer, so this observes exactly the
// text that is about to be emitted without contributing any of it.
//
// `'static` is pre-declared in every scope and `'_` is the inferred placeholder;
// declaring either as a generic parameter is an error, so both are skipped.
std::string Converter::GetLifetimeBinders(clang::FunctionDecl *decl) {
  std::vector<std::string> binders;
  // ⭐ ONE scanner, two consumers: `ScanLifetimeNames` (:849) is also what the
  // local-`let` path uses to decide whether it must elide. A second copy of the
  // scan would have been a second answer to the same question.
  auto scan = [&](const std::string &spelling) {
    for (auto &name : ScanLifetimeNames(spelling)) {
      if (std::ranges::find(binders, name) == binders.end()) {
        binders.push_back(std::move(name));
      }
    }
  };
  // Render under the same formals flag the real parameter emission uses, so a
  // spelling that differs in formal position cannot hide a binder.
  const bool saved_formals = in_function_formals_;
  in_function_formals_ = true;
  auto *definition =
      decl->getDefinition() != nullptr ? decl->getDefinition() : decl;
  for (const auto *parameter : definition->parameters()) {
    scan(ToString(parameter->getType()));
  }
  in_function_formals_ = saved_formals;
  if (!decl->getReturnType()->isVoidType()) {
    scan(ToString(decl->getReturnType()));
  }
  if (binders.empty()) {
    return "";
  }
  std::string out(1, token::kLt);
  for (size_t i = 0; i < binders.size(); ++i) {
    if (i != 0) {
      out += ", ";
    }
    out += binders[i];
  }
  out += token::kGt;
  return out;
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
  // The record itself still emits as a struct under its own name (see
  // VisitCXXRecordDecl), so the trait takes a distinct name.
  auto trait_name = GetRecordName(decl) + "__Virtual";
  auto access_specifier_as_string = AccessSpecifierAsString(decl->getAccess());
  auto signature = std::format("{} {} trait {}", access_specifier_as_string,
                               keyword_unsafe_, trait_name);
  // Must stay CAPTURELESS: `ConvertCXXMethodDecls` takes a raw function
  // pointer. Only virtual methods belong in the trait; a constructor has no
  // `self` and is rejected outright (E0038), and it is now emitted as an
  // inherent member of the struct instead.
  auto predicate = [](auto *method) {
    return method->isVirtual() &&
           !clang::isa<clang::CXXConstructorDecl>(method);
  };
  PushInTraitBody push_trait(*this, true);
  // A C++ vtable slot is INHERITED: `runOnOperation` is declared pure virtual on
  // `mlir::Pass`, several levels above the tablegen `...PassBase` whose trait
  // this is, and `getNodeName` is declared pure virtual on `OperationTreeNode`,
  // one level above `PipelineTreeNode`. `decl->methods()` returns only the
  // methods declared IN `decl`, so the trait used to omit both while the derived
  // class's `impl` carried them -- error[E0407], 96 sites / 75 TUs of the
  // compile-level corpus, and the #2 first-abort gate. The trait must therefore
  // declare the UNFILLED slots of the whole base chain as well.
  ConvertCXXMethodDecls(decl, signature, predicate,
                        InheritedUnfilledVirtuals(decl));
}

// Every vtable slot `decl` inherits and does NOT fill: a pure virtual declared
// in some base of `decl` that no nearer declaration overrides. Walked
// breadth-first from `decl` itself so that a nearer declaration always wins.
//
// The walk crosses the port boundary deliberately -- `mlir::Pass` is not ported
// and never will be, but the ported class's vtable still HAS its slots, and the
// derived class's `override` still has to land somewhere. It is safe because
// only PURE slots are emitted while EVERY virtual shadows: `mlir::Pass` declares
// `canScheduleOn` pure and `mlir::OperationPass<T>` overrides it non-pure, so
// the non-pure override shadows the pure declaration and nothing is emitted for
// it. Emitting it would have been error[E0046] on every pass, since no ported
// class implements it. `runOnOperation` has no such override anywhere in the
// chain, which is exactly why it is the one slot that must be declared.
std::vector<clang::CXXMethodDecl *>
Converter::InheritedUnfilledVirtuals(const clang::CXXRecordDecl *decl) {
  std::vector<clang::CXXMethodDecl *> out;
  // Slots a NEARER declaration already fills, keyed on the C++ canonical decl
  // via `overridden_methods()`.
  //
  // ⛔ NOT keyed on the emitted Rust name, which was the first attempt and was
  // MEASURED WRONG: `mlir::Pass::canScheduleOn` emits as
  // `canScheduleOn_Optiondataflowir_genTdOpDef_const` (IsOverloadedMethod is true
  // in that class) while `mlir::OperationPass<T>`'s `final` override of the very
  // same signature emits as plain `canScheduleOn`. The names did not match, the
  // pure declaration was not shadowed, and the trait grew an unimplementable item
  // -- error[E0046] on every pass TU, i.e. exactly the error-for-error trade this
  // row is not allowed to make. The override EDGE is the ground truth; the name
  // is a rendering of it.
  std::unordered_set<const clang::CXXMethodDecl *> filled;
  auto mark_filled = [&filled](const clang::CXXMethodDecl *method) {
    std::vector<const clang::CXXMethodDecl *> work{method};
    while (!work.empty()) {
      const auto *current = work.back();
      work.pop_back();
      for (const auto *base_method : current->overridden_methods()) {
        if (filled.insert(base_method->getCanonicalDecl()).second) {
          work.push_back(base_method);
        }
      }
    }
  };
  // Second guard, independent of the first: two DIFFERENT slots must never emit
  // the same Rust name into one trait body (that is error[E0428]).
  std::unordered_set<std::string> emitted_names;
  for (auto *method : decl->methods()) {
    if (method->isVirtual() && !clang::isa<clang::CXXConstructorDecl>(method)) {
      emitted_names.insert(GetMethodName(method));
    }
  }
  std::unordered_set<const clang::CXXRecordDecl *> seen;
  std::vector<const clang::CXXRecordDecl *> frontier{decl};
  bool is_own = true;
  while (!frontier.empty()) {
    std::vector<const clang::CXXRecordDecl *> next;
    for (const auto *record : frontier) {
      if (record == nullptr ||
          !seen.insert(record->getCanonicalDecl()).second) {
        continue;
      }
      const auto *definition = record->getDefinition();
      if (definition == nullptr) {
        continue;
      }
      for (auto *method : definition->methods()) {
        if (!method->isVirtual() ||
            clang::isa<clang::CXXConstructorDecl>(method)) {
          continue;
        }
        const bool owned = filled.contains(method->getCanonicalDecl());
        // Even a method that is skipped below still fills what it overrides.
        mark_filled(method);
        if (owned || is_own || !method->isPureVirtual()) {
          continue;
        }
        if (!emitted_names.insert(GetMethodName(method)).second) {
          continue;
        }
        out.push_back(method);
      }
      for (const auto &base : definition->bases()) {
        next.push_back(base.getType()->getAsCXXRecordDecl());
      }
    }
    is_own = false;
    frontier = std::move(next);
  }
  return out;
}

// The set of Rust method names `<decl>__Virtual` declares, computed from the AST
// so that it DOES NOT DEPEND ON EMISSION ORDER (the derived class's `impl` body
// is built eagerly during traversal, the base's trait when the base record is
// visited). It mirrors ConvertAbstractClass exactly: `decl`'s own emittable
// virtuals plus InheritedUnfilledVirtuals. A method with neither a body nor a
// pure-virtual marker is NOT emitted by VisitCXXMethodDecl and so is NOT in the
// set -- routing an override of it to the trait impl would be E0407 again.
std::unordered_set<std::string>
Converter::VirtualTraitMethodNames(const clang::CXXRecordDecl *decl) {
  std::unordered_set<std::string> names;
  auto add = [&](clang::CXXMethodDecl *method) {
    if (!method->isVirtual() ||
        clang::isa<clang::CXXConstructorDecl>(method) ||
        !IsConvertibleCXXMethodDecl(method) ||
        (!method->isPureVirtual() && !method->hasBody())) {
      return;
    }
    names.insert(GetMethodName(method));
  };
  for (auto *method : decl->methods()) {
    add(method);
  }
  ForEachTemplateInstantiatedMethod(decl, add);
  for (auto *method : InheritedUnfilledVirtuals(decl)) {
    names.insert(GetMethodName(method));
  }
  return names;
}

void Converter::ConvertCXXMethodDecls(
    const clang::CXXRecordDecl *decl, const std::string_view signature,
    bool (*predicate)(clang::CXXMethodDecl *),
    llvm::ArrayRef<clang::CXXMethodDecl *> inherited) {
  bool first = true;
  // Held for exactly as long as the block below is open, so that an out-of-line
  // method reached from either loop knows it is ALREADY inside
  // `<signature> { ... }` and must not open a second, nested `impl` -- which
  // Rust rejects ("implementation is not supported in `trait`s or `impl`s") and
  // therefore costs the whole TU its `.rs`. See open_item_block_record_.
  std::optional<PushOpenItemBlockRecord> open_block;
  auto open = [&] {
    if (first) {
      StrCat(signature, token::kOpenCurlyBracket);
      first = false;
      if (!in_trait_body_) {
        open_block.emplace(*this, GetRecordName(decl));
      }
    }
  };
  auto convert_method = [&](clang::CXXMethodDecl *method) {
    if (predicate(method)) {
      open();
      VisitCXXMethodDecl(method);
    }
  };
  for (auto *method : decl->methods()) {
    convert_method(method);
  }
  ForEachTemplateInstantiatedMethod(decl, convert_method);
  for (auto *method : inherited) {
    if (!IsConvertibleCXXMethodDecl(method)) {
      continue;
    }
    open();
    // Deliberately NOT VisitCXXMethodDecl: `decl_ids_` is keyed on the
    // declaration, and the SAME base declaration legitimately appears in the
    // trait of every class that inherits the slot. Going through Visit would
    // silently drop it from all but the first, which is the failure mode this
    // whole change exists to remove. A pure virtual emits a signature and a
    // semicolon, never a body, so no definition is duplicated either.
    PushCurrFunction push_fn(*this, method);
    ConvertCXXMethodDecl(method);
  }
  if (!first) {
    StrCat(token::kCloseCurlyBracket);
    open_block.reset();
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
      it->second.header = std::format("{} impl {}__Virtual for {}",
                                      keyword_unsafe_, base_target, name);
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

Converter::DeferredBlock &
Converter::InherentVirtualMethodsFor(const clang::CXXRecordDecl *decl) {
  auto name = GetRecordName(decl);
  // A DISTINCT key from VirtualMethodsFor's, so a class can have both blocks.
  // Two inherent `impl <T> { ... }` blocks for one type are legal Rust.
  auto [it, inserted] = virtual_methods_.try_emplace(name + " __inherent");
  if (inserted) {
    it->second.header = std::format("{} {}", keyword::kImpl, name);
  }
  return it->second;
}

// True if `method`, an override on `decl`, has a slot in the trait of `decl`'s
// first base. False means the slot's declaration lives outside what this
// converter lowers -- `SplitDFIROutputPass::runOnOperation` overrides a pure
// virtual of `mlir::Pass`, whose class is NOT ported and therefore has no trait,
// and no amount of walking can invent one. Such a method becomes an INHERENT
// method of the derived struct: the body is kept verbatim and stays callable,
// only dispatch through a base reference is lost -- and the only thing that ever
// dispatches `runOnOperation` is MLIR's own PassManager, on the far side of the
// rule boundary. This is the same lowering, and the same reasoning, that
// BaseTargetNamesTrait already applies to a rule-mapped base.
bool Converter::VirtualMethodHasTraitSlot(const clang::CXXRecordDecl *decl,
                                         clang::CXXMethodDecl *method) {
  const auto *base = decl->bases_begin()->getType()->getAsCXXRecordDecl();
  if (base == nullptr || base->getDefinition() == nullptr) {
    return true;
  }
  if (VirtualTraitMethodNames(base->getDefinition())
          .contains(GetMethodName(method))) {
    return true;
  }
  if (survey::Enabled()) {
    survey::Record(
        survey::GapKind::kInfo,
        "virtual `" + method->getQualifiedNameAsString() +
            "` fills a vtable slot declared outside the ported base chain of `" +
            base->getQualifiedNameAsString() +
            "`; lowered as an inherent method (was error[E0407])",
        method->getLocation().printToString(ctx_.getSourceManager()));
  }
  return false;
}

void Converter::AddVirtualMethodBody(const clang::CXXRecordDecl *decl,
                                     clang::CXXMethodDecl *method,
                                     std::string body) {
  if (body.empty()) {
    return;
  }
  if (VirtualMethodHasTraitSlot(decl, method)) {
    VirtualMethodsFor(decl).body += std::move(body);
  } else {
    InherentVirtualMethodsFor(decl).body += std::move(body);
  }
}

void Converter::ConvertVirtualMethods(clang::CXXRecordDecl *decl) {
  if (decl->bases_begin() == decl->bases_end()) {
    return;
  }
  // PER METHOD, not per class: an override whose slot the base's trait declares
  // and one whose slot it cannot must land in DIFFERENT blocks.
  for (auto *method : decl->methods()) {
    if (method->isImplicit() || !method->isVirtual()) {
      continue;
    }
    Buffer buf(*this);
    VisitCXXMethodDecl(method);
    AddVirtualMethodBody(decl, method, std::move(buf).str());
  }
}

bool Converter::ConvertOutOfLineVirtualMethod(clang::CXXMethodDecl *decl) {
  auto *record = decl->getParent();
  if (record->bases_begin() == record->bases_end()) {
    return false;
  }
  Buffer buf(*this);
  auto emitted = ConvertCXXMethodDecl(decl);
  AddVirtualMethodBody(record, decl, std::move(buf).str());
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

// The user-PROVIDED default constructor of `decl`, whether or not its BODY is
// visible in this TU.  GetUserDefinedDefaultConstructor() (converter_lib.cpp
// :629) additionally demands `hasBody()`, and that single predicate is what
// splits the two behaviours measured on `dsc/designSpaceConfig.h`'s 240-entry
// NSDMI `std::map<std::string,double*> paramNameToVal = {{"nin", &N_.in_}, ...}`
// (:359-609):
//   * `dsc/designSpaceConfig.cpp` DEFINES `DesignSpaceConfig::DesignSpaceConfig()`,
//     so hasBody() is true, AddDefaultTrait takes the ctor arm, and the NSDMI is
//     walked inside the emitted `fn new()` where a receiver is in scope.  That TU
//     emits ZERO Cpp2RustUnmappedExpr_MemberExpr.
//   * the 12 TUs that include only the HEADER see the in-class DECLARATION at
//     :135 with no body, fall through to EmitDefaultStructLiteral, and walk the
//     same NSDMI with curr_function_ == nullptr -- 240 sites each, 2,880 total,
//     and 240x12 exactly because there is zero variance between them.
// A declared-but-not-defined default constructor is still the constructor C++
// runs for `T{}`, so delegating to it is the faithful lowering AND it is the
// only emitted context in which `this` is spellable at all.
static clang::CXXConstructorDecl *
GetUserProvidedDefaultConstructorDecl(const clang::CXXRecordDecl *decl) {
  for (auto *ctor : decl->ctors()) {
    if (ctor->isUserProvided() && ctor->isDefaultConstructor()) {
      return ctor;
    }
  }
  return nullptr;
}

// True when some field's in-class initializer reads through `this`.  Such an
// initializer CANNOT be emitted inside a struct literal: the literal is the
// expression that PRODUCES the object, so no object and no binding exist yet,
// and ConvertMemberExpr / VisitCXXThisExpr reach ReportThisWithoutEnclosingFunction.
// Detecting `CXXThisExpr` anywhere in the initializer covers both entry points.
// ANY default constructor of `decl`, INCLUDING the implicit one.  This is the
// gap 68dbf8c6 left open: GetUserProvidedDefaultConstructorDecl above demands
// `isUserProvided()`, so a class that declares NO constructor at all -- e.g.
// `dcc/tools/Options/dcc-pass-option.h`'s
// `struct EmitSentientIROptions : mlir::PassPipelineOptions<...> { Option<int>
// OptLevel{*this, "opt-level", ..}; .. }` -- had nothing to delegate to and kept
// its placeholder: 6 `Cpp2RustUnmappedExpr_CXXThisExpr` per including TU.
// An implicit default constructor is NOT emitted as Rust (VisitCXXConstructorDecl
// :2362 returns early for `isImplicit()` unless IsConvertibleImplicitMember), so
// it must NOT be delegated to by name -- that would trade E0425 on the
// placeholder for E0425 on a fabricated `T::new()`.  It is used only as the
// `curr_function_` token that tells ConvertMemberExpr / VisitCXXThisExpr to spell
// the CONSTRUCTOR receiver, while AddDefaultTrait emits the body itself.
static clang::CXXConstructorDecl *
GetAnyDefaultConstructorDecl(const clang::CXXRecordDecl *decl) {
  for (auto *ctor : decl->ctors()) {
    if (ctor->isDefaultConstructor()) {
      return ctor;
    }
  }
  return nullptr;
}

static bool HasThisBearingFieldInit(const clang::RecordDecl *decl) {
  for (const auto *field : decl->fields()) {
    if (const auto *init = field->getInClassInitializer();
        init != nullptr && StmtMentionsThis(init)) {
      return true;
    }
  }
  return false;
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
    // Widen the ctor arm to a body-less declaration ONLY for the classes the
    // struct-literal arm provably cannot emit, so every other class keeps its
    // current output byte-for-byte.  A `this`-bearing NSDMI with no user-provided
    // default constructor anywhere has nothing to delegate to and deliberately
    // keeps its placeholder: a loud E0425 beats inventing a receiver.
    auto *default_ctor = GetUserDefinedDefaultConstructor(cxx);
    if (default_ctor == nullptr && HasThisBearingFieldInit(decl)) {
      default_ctor = GetUserProvidedDefaultConstructorDecl(cxx);
    }
    if (default_ctor != nullptr) {
      StrCat(keyword_unsafe_);
      PushBrace unsafe_brace(*this);
      Convert(MakeConstructExpr(ctx_, ctx_.getCanonicalTagType(decl),
                                default_ctor, {}));
      return;
    }
    // ⭐ IN-PLACE ARM, for the class that has NO user-provided constructor at
    // all.  Same shape and the same justification as EmitInPlaceConstructor
    // (:2430): the object's ADDRESS must exist before any initializer runs, so
    // the literal is built through one `ptr::write` into a `MaybeUninit` slot
    // whose pointer is what the initializers capture.  A field-by-field
    // two-phase assignment is not an option -- it needs a `Default` for every
    // aliased field (`Option<int>` has none) and would drop a garbage value.
    // ⚠️ Returning by value MOVES the finished object out of the slot, so for a
    // self-referential class this is the same fidelity compromise the
    // `-> Self` wrapper at :2471 already makes; it is not a soundness fix.  It
    // is taken because `fn default() -> Self` is the signature the trait
    // dictates and every measured call site is an expression, so no caller owns
    // a place.  What it DOES buy over the placeholder: the captured pointer is a
    // real pointer to a real object of the right type, and the file type-checks.
    if (HasThisBearingFieldInit(decl)) {
      if (auto *implicit_ctor = GetAnyDefaultConstructorDecl(cxx)) {
        PushCurrFunction push_fn(*this, implicit_ctor);
        PushThisIsMutRef push_this(*this, true);
        StrCat(keyword_unsafe_);
        PushBrace unsafe_brace(*this);
        StrCat("let mut __cc2_slot = ::std::mem::MaybeUninit::<Self>::uninit()",
               token::kSemiColon);
        StrCat("let __cc2_this: *mut Self = __cc2_slot.as_mut_ptr()",
               token::kSemiColon);
        StrCat(keyword::kLet, "this", token::kColon, "&mut Self",
               token::kAssign, "&mut *__cc2_this", token::kSemiColon);
        StrCat("::std::ptr::write");
        {
          PushParen write_paren(*this);
          StrCat("__cc2_this", token::kComma);
          EmitDefaultStructLiteral(decl);
        }
        StrCat(token::kSemiColon);
        StrCat("__cc2_slot.assume_init()");
        return;
      }
      // No default constructor at all (e.g. every constructor takes arguments):
      // there is no `impl Default` this class can honestly have, and the
      // placeholder's E0425 stays as the loud marker.
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

// See the header for WHY this is mandatory and why dropping the cast instead
// would be silently wrong.
// See the header for the measurement and for why `>`-family operators are
// deliberately excluded.
bool Converter::AngleCastOperandNeedsParens(std::string_view operand_text,
                                           std::string_view opcode) {
  if (opcode != "<" && opcode != "<<" && opcode != "<=") {
    return false;
  }
  // Scan for an `as` token at bracket depth 0. A depth-0 `as` means the
  // operand's own outermost form is (or contains at top level) a cast, so the
  // parser is still inside a TYPE when `opcode` arrives.
  int depth = 0;
  bool found = false;
  for (size_t i = 0; i < operand_text.size(); ++i) {
    const char c = operand_text[i];
    if (c == '(' || c == '[' || c == '{') {
      ++depth;
      continue;
    }
    if (c == ')' || c == ']' || c == '}') {
      --depth;
      if (depth < 0) {
        llvm::report_fatal_error(
            llvm::Twine("unsupported cast operand shape: unbalanced closing "
                        "bracket in the operand text `") +
                std::string(operand_text) + "` emitted left of `" +
                std::string(opcode) +
                "`; cannot decide whether the cast needs parentheses",
            /*gen_crash_diag=*/false);
      }
      continue;
    }
    if (depth != 0 || c != 'a' || i + 1 >= operand_text.size() ||
        operand_text[i + 1] != 's') {
      continue;
    }
    const bool left_ok =
        i == 0 || !(std::isalnum(static_cast<unsigned char>(
                        operand_text[i - 1])) ||
                    operand_text[i - 1] == '_');
    const size_t after = i + 2;
    const bool right_ok =
        after >= operand_text.size() ||
        !(std::isalnum(static_cast<unsigned char>(operand_text[after])) ||
          operand_text[after] == '_');
    if (left_ok && right_ok) {
      found = true;
    }
  }
  if (depth != 0) {
    llvm::report_fatal_error(
        llvm::Twine("unsupported cast operand shape: `") +
            std::string(operand_text) + "` emitted left of `" +
            std::string(opcode) +
            "` has unbalanced brackets, so the top-level `as` scan that decides "
            "whether to parenthesise the cast cannot be trusted",
        /*gen_crash_diag=*/false);
  }
  return found;
}

std::string
Converter::ParenthesizeAngleCastOperandText(std::string operand_text,
                                            std::string_view opcode) {
  if (!AngleCastOperandNeedsParens(operand_text, opcode)) {
    return operand_text;
  }
  return "(" + operand_text + ")";
}

void Converter::ParenthesizeAngleCastOperand(size_t operand_start,
                                             std::string_view opcode) {
  if (rs_code_ == nullptr || operand_start >= rs_code_->size()) {
    return;
  }
  std::string_view tail(*rs_code_);
  tail.remove_prefix(operand_start);
  if (!AngleCastOperandNeedsParens(tail, opcode)) {
    return;
  }
  rs_code_->insert(operand_start, "(");
  rs_code_->append(")");
}

void Converter::ParenthesizeBlockCastOperand(size_t operand_start) {
  if (rs_code_ == nullptr || operand_start >= rs_code_->size()) {
    return;
  }
  std::string_view tail(*rs_code_);
  tail.remove_prefix(operand_start);
  while (!tail.empty() &&
         std::isspace(static_cast<unsigned char>(tail.front()))) {
    tail.remove_prefix(1);
  }
  if (tail.empty()) {
    return;
  }
  // Every Rust expression form that is a BLOCK, i.e. whose textual extent ends
  // at a `}` and which therefore cannot be an `as` operand unparenthesised.
  // A bare `{` is included: `ConvertAssignment` and the ctor-argument emitter
  // both wrap their payload in one.
  static constexpr std::string_view kBlockStarters[] = {
      "if", "match", "unsafe", "loop", "while", "for", "{"};
  bool starts_block = false;
  for (std::string_view kw : kBlockStarters) {
    if (!tail.starts_with(kw)) {
      continue;
    }
    if (kw == "{") {
      starts_block = true;
      break;
    }
    // Only a whole keyword introduces a block; `iffy_name` and `formula` must
    // not be mistaken for one.
    const char after = tail.size() > kw.size() ? tail[kw.size()] : '\0';
    starts_block = !(std::isalnum(static_cast<unsigned char>(after)) ||
                     after == '_' || after == ':');
    break;
  }
  if (!starts_block) {
    return;
  }
  rs_code_->insert(operand_start, "(");
  StrCat(token::kCloseParen);
}

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
    // An array (or a `c"..."` literal) does NOT `as`-cast to a raw pointer in
    // Rust -- that is E0606 "casting `&CStr` as `*const i8` is invalid". Take
    // its address with `.as_ptr()`, which is the SAME spelling the
    // CK_ArrayToPointerDecay path already emits at :3833-3837. That path is
    // reached when the C++ parameter is `const char *` (the argument decays);
    // here the C++ parameter is a REFERENCE TO ARRAY, so clang inserts no
    // decay node and we must take the address ourselves. The trailing
    // pointer-to-pointer `as` is kept so the pointee type still matches the
    // rule's declared parameter type (legal, and value-preserving).
    return std::format(
        "({}.as_ptr() as {})", ConvertFreshPointer(arg),
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
    // ⛔ THE PARENTHESES ARE LOAD-BEARING AND A BARE `{ ... }` IS A PARSE ERROR
    // HALF THE TIME IT IS USED.
    //
    // `Mapper::parenthesizeBodyIfNeeded` deliberately skips multi-statement
    // rules on the grounds that `{ ... }` "is already a single primary
    // expression". It is not: in Rust a block at the START of a STATEMENT is
    // parsed as a *statement*, not as an operand, so as soon as the surrounding
    // emission appends a binary operator the block terminates the statement and
    // the operator has nothing to bind to. Measured on
    // `dcc/src/Conversion/DataflowToSentient/DataflowToSentient.cpp`, where
    // `src_unit_name.substr(0, 2) != "l3"` -- `substr` being a multi-statement
    // rule -- emitted
    //     if {let s = ...; {let mut __tmp1 = ...; __tmp1}  != <rhs>}
    // and rustfmt REFUSED the file with 5x `error: expected expression, found
    // `!=``. The block is the tail expression of the `if` condition's own
    // block, i.e. statement position, so `{...} != rhs` cannot parse. The same
    // shape is one `as` or one `.method()` away in every other consumer.
    //
    // Wrapping is safe in the other direction too: a parenthesised block is a
    // primary expression everywhere a block expression was already accepted as
    // an *operand*, and a statement-position use of a mapped call is emitted
    // with its own terminator by ConvertStmt, so `({ ... }) ;` is well-formed.
    return "({" + result + "})";
  }
  return result;
}

// THE RULE `needs_explicit_mut_borrow` IMPLEMENTS: a rule parameter declared
// `&mut T` must receive an actual Rust `&mut`, and a `kBorrowMut` placeholder's
// emission is a PLACE (`ConvertLValue` -- `mySymDims`, `(*data)`), never a
// reference. So the converter supplies the `&mut`.
//
// THE ONE CASE WHERE IT MUST NOT: the rule body's own text already wrote it.
// `rules/set`'s `f13` is `let __set = &mut *a0;` -- correct Rust for a
// `&mut std::set<T1>` parameter -- and the preprocessor records it as the text
// `"...let __set = &mut "` followed by a `kBorrowMut` placeholder for `a0`,
// i.e. it consumed the `*` and left the `&mut` in the text on purpose: text
// `&mut ` + place IS `&mut *a0` with `a0` substituted. Adding a second one
// yields `&mut &mut mySymDims`, and `&mut &mut BTreeSet<T> as *const
// BTreeSet<T>` is rustc E0606 (an `as` cast does NOT deref-coerce, unlike the
// `BTreeSet::insert(__set, ..)` on the line above it, which is why the defect
// survived to rc=0).
//
// NOT VACUOUS, measured over all 98 modules of `pin/ir.v45` x both models:
// of 694 `kBorrowMut` placeholders, 669 keep the converter-supplied borrow
// (403 open their fragment list, 266 follow text that ends in something other
// than a borrow -- e.g. `rules/algorithm`'s
// `::std::slice::from_raw_parts_mut(`), and exactly 25 are preceded by `&mut`
// and are the class this suppresses (`algorithm` f9, `bitset` f4/f8,
// `mlir` f1300/f1800, `set` f6/f13/f23, ...). Zero are preceded by a bare `&`.
static bool RuleTextAlreadyBorrowsMut(std::string_view text) {
  while (!text.empty() && (text.back() == ' ' || text.back() == '\t' ||
                           text.back() == '\n' || text.back() == '\r')) {
    text.remove_suffix(1);
  }
  // `&` cannot be part of a Rust identifier, so `ends_with("&mut")` is already
  // a token test -- there is no `foo&mut` to be confused by.
  return text.ends_with("&mut");
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
      // A rule body's text fragment that OPENS A NEW LINE closes the previous
      // one, and rustfmt REFUSES a file with `error[internal]: left behind
      // trailing whitespace` -- an internal error, not a parse error, so the
      // Rust is otherwise fine and the whole file simply goes unformatted.
      //
      // The whitespace is not in the rule source. `rules/support`'s
      // `llvm_unreachable` body ends a line with the bare placeholder `a2`, and
      // the converter's expression emitter renders an integer literal with a
      // TRAILING SPACE (`66_u32 `), so the substitution lands a space at
      // end-of-line. Measured on
      // `dcc/.../TransformPagedMemViewManager.cpp` (line 14365, `66_u32 `) and
      // `dsc-based-utils/.../SNStickMaskLowering.cpp` (line 43581, `6073_u32 `)
      // -- the only two trailing-whitespace refusals in the 312-TU corpus.
      //
      // ⭐ TRIMMED HERE AND NOT AT THE LITERAL EMITTER, AND NOT AS A WHOLE-FILE
      // POST-PASS. Dropping the literal emitter's trailing space would change
      // interior spacing in every emitted file for no correctness gain, and a
      // whole-file trim would edit the INTERIOR of a multi-line string literal
      // (rule bodies are inlined verbatim, string literals included). Trimming
      // only the bytes that sit immediately before a rule-body newline touches
      // nothing rustc can observe: the one way it could reach inside a literal
      // is a rule body whose multi-line string literal has a PLACEHOLDER as the
      // last thing on one of its lines, which no rule body does -- a rule's
      // format string uses `{}`, not `a0`.
      if (!t->text.empty() && t->text.front() == '\n') {
        auto end = result.size();
        while (end > 0 && (result[end - 1] == ' ' || result[end - 1] == '\t')) {
          --end;
        }
        result.resize(end);
      }
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
              Mapper::ParamIsMutRef(GetCalleeOrExpr(expr), arg_idx) &&
              !RuleTextAlreadyBorrowsMut(result),
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
  // Replay the nested-argument descent recorded by cpp-rule-preprocessor's
  // `findTemplateArgument` (see InitTypeLocation).  Every step MUST resolve:
  // silently keeping the outer type would construct the WRONG type at every
  // arrival site with nothing for a census to see.
  for (unsigned step : tgt_ir->init_type.path) {
    const auto *spec =
        clang::dyn_cast_or_null<clang::ClassTemplateSpecializationDecl>(
            type->getAsCXXRecordDecl());
    auto args = spec ? spec->getTemplateArgs().asArray()
                     : llvm::ArrayRef<clang::TemplateArgument>();
    if (step >= args.size() ||
        args[step].getKind() != clang::TemplateArgument::Type) {
      llvm::report_fatal_error(
          llvm::Twine("cpp2rust: rule init type: '") + Mapper::ToString(type) +
              "' has no type template argument " + llvm::Twine(step) +
              ", so the recorded init_type path cannot be replayed for the "
              "call to '" +
              Mapper::ToString(callee) + "' at " +
              expr->getExprLoc().printToString(ctx_.getSourceManager()),
          /*gen_crash_diag=*/false);
    }
    type = args[step].getAsType();
  }

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
  // ⛔ THIS USED TO BE `assert(init && ...)`, WHICH IS A NO-OP IN THE SHIPPED
  // RelWithDebInfo (-DNDEBUG) BUILD -- the null then flowed into
  // `init->IgnoreImplicit()` and the tool died with SIGSEGV naming nothing, or
  // worse, emitted a partially-constructed value.  Same precedent as
  // cpp_rule_preprocessor.cpp's `fail`.
  //
  // ⭐ THE CASE THAT MAKES THIS LOAD-BEARING: an `Init<>` pack key carries NO
  // arity in its `src` spelling (libc++'s `emplace(Args&&...)` prints as
  // `emplace(&&...)` at every arity), so an arity-3+ call -- e.g.
  // `emplace(piecewise_construct, ...)` -- reaches here with 3 arguments for a
  // 2-element `pair`.  There is no aN placeholder count to bound it, so the
  // ONLY thing standing between that call and a silently mis-constructed value
  // is this diagnostic.  It must name the type, the arity and the site.
  if (!init) {
    std::string arg_types;
    for (const auto *arg : args) {
      if (!arg_types.empty()) {
        arg_types += ", ";
      }
      arg_types += Mapper::ToString(arg->getType());
    }
    llvm::report_fatal_error(
        llvm::Twine("cpp2rust: rule body's `init` cannot construct '") +
            Mapper::ToString(type) + "' from the " + llvm::Twine(args.size()) +
            " argument(s) supplied at the call site (" + arg_types +
            ") at " + loc.printToString(ctx_.getSourceManager()) +
            " -- the rule key does not discriminate this arity",
        /*gen_crash_diag=*/false);
  }
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

void Converter::EmitDecompositionHolderAnnotation(clang::QualType value_type,
                                                  bool is_mut) {
  // A MUTABLE reference holder annotates `*mut`, so the `&mut <init>`
  // ConvertVarInit already emits for a non-const reference QualType coerces to
  // it; `*const` would make the binding pointers `*const` and no write could
  // reach the container. This is verbatim the text that was inlined at
  // ConvertTupleDecompositionDecl (:1196) before it became a hook.
  StrCat(is_mut ? "*mut" : "*const");
  Convert(value_type);
}

std::string Converter::DecompositionHolderElement(const std::string &holder,
                                                  const std::string &element,
                                                  clang::QualType value_type) {
  return std::format("(*{}).{}", holder, element);
}

bool Converter::DecompositionValueHolderSupported(clang::QualType value_type,
                                                  std::size_t arity) {
  // The unsafe model has shipped this shape since before these became hooks
  // and it is MEASURED to run correctly (`verif/g3086/bv.cpp`: `MATCH`), so
  // there is nothing left to gate here -- the arity/model checks in
  // `ConvertTupleDecompositionDecl` are the whole precondition.
  return true;
}

void Converter::EmitDecompositionValueHolderAnnotation(
    clang::QualType value_type) {
  // Verbatim the `Convert(type)` that was inlined at
  // ConvertTupleDecompositionDecl's by-value holder annotation before this
  // became a hook, so the unsafe emission is unchanged byte for byte.
  Convert(value_type);
}

std::string Converter::DecompositionValueHolderElement(
    const std::string &holder, const std::string &element,
    clang::QualType value_type) {
  // Verbatim the `std::format("{}.{}", holder, element)` that was inlined at
  // ConvertTupleDecompositionDecl's by-value element read before this became a
  // hook, so the unsafe emission is unchanged byte for byte.
  return std::format("{}.{}", holder, element);
}

} // namespace cpp2rust
