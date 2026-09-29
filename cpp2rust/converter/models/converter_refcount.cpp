// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include "converter/models/converter_refcount.h"

#include <clang/AST/RecordLayout.h>
#include <clang/Basic/OperatorKinds.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/Support/ErrorHandling.h>

#include <algorithm>
#include <format>
#include <ranges>

#include "compiler.h"
#include "converter/converter_lib.h"
#include "converter/lex.h"
#include "converter/mapper.h"
#include "converter/survey.h"
#include "tu_guard.h"

namespace cpp2rust {
std::map<std::string, ConverterRefCount::MethodsOnPtr>
    ConverterRefCount::methods_on_ptr_;

void ConverterRefCount::EmitMethodsOnPtr(std::string &out) {
  for (const auto &[name, methods] : methods_on_ptr_) {
    EmitDeferredBlock(methods.trait, out);
    EmitDeferredBlock(methods.impl, out);
  }
}

ConverterRefCount::ConverterRefCount(std::string &rs_code,
                                     clang::ASTContext &ctx)
    : Converter(rs_code, ctx, "", ""),
      conversion_kind_({ConversionKind::Unboxed}) {}

void ConverterRefCount::EmitFilePreamble() {
  StrCat(R"(
extern crate libcc2rs;
use libcc2rs::*;
use std::cell::RefCell;
use std::collections::{BTreeMap, HashMap, HashSet};
use std::io::{Read, Write, Seek};
use std::io::prelude::*;
use std::os::fd::AsFd;
use std::rc::{Rc, Weak};
)");
}

static bool IsBoxedType(std::string_view type) {
  return type.starts_with("Vec<") || type.starts_with("Box<");
}

static bool IsBoxedType(clang::QualType type) {
  return IsBoxedType(Mapper::Map(type.getUnqualifiedType()));
}

static bool NeedsMutAccess(const clang::CXXMethodDecl *method,
                           clang::QualType base_type) {
  return !method->isConst() && IsBoxedType(base_type);
}

static bool IsPointerType(clang::QualType type) {
  return type->isPointerType() ||
         GetStrongestIteratorCategory(type) == IteratorCategory::Contiguous;
}

bool ConverterRefCount::PendingDeref::compute_inner_boxed(clang::Expr *expr) {
  if (!expr) {
    return false;
  }
  if (!IsBoxedType(expr->getType().getNonReferenceType())) {
    return false;
  }
  if (auto *ase = clang::dyn_cast<clang::ArraySubscriptExpr>(expr)) {
    auto base_type = ase->getBase()->IgnoreCasts()->getType();
    if (base_type->isPointerType())
      return IsBoxedType(base_type->getPointeeType());
    return IsBoxedType(base_type.getNonReferenceType());
  }
  if (auto *oce = clang::dyn_cast<clang::CXXOperatorCallExpr>(expr)) {
    return IsBoxedType(oce->getArg(0)->getType().getNonReferenceType());
  }
  return false;
}

void ConverterRefCount::PendingDeref::set(std::string str, bool fresh,
                                          clang::Expr *expr) {
  assert_consumed();
  set_unchecked(std::move(str), fresh, expr);
}

void ConverterRefCount::PendingDeref::set_unchecked(std::string str, bool fresh,
                                                    clang::Expr *expr) {
  value = std::move(str);
  pointee_is_boxed = compute_inner_boxed(expr);
  ptr_is_fresh = fresh;
  type = ComputedExprType::Pending;
}

std::string ConverterRefCount::GetInnerType(clang::QualType type) {
  PushConversionKind push(*this, ConversionKind::Unboxed);
  auto str = ToString(type);
  auto pos = str.find('<');
  auto end = str.rfind('>');
  if (str[pos + 1] == '[' && str[end - 1] == ']') {
    // Unwrap inner array type
    pos++;
    end--;
  }
  return std::move(str).substr(pos + 1, end - pos - 1);
}

ConverterRefCount::PushUnboxedIfSimple::PushUnboxedIfSimple(
    ConverterRefCount &c, std::string_view outer, clang::QualType inner_type)
    : c(c) {
  bool unboxed = outer == "Ptr<%>" || outer == "%";

  // Vectors are boxed until the last element
  if (!unboxed && (outer == "Vec<%>" || outer == "Box<%>")) {
    if (!IsBoxedType(inner_type)) {
      unboxed = true;
    }
  }

  c.conversion_kind_.push_back(unboxed ? ConversionKind::Unboxed
                                       : ConversionKind::FullRefCount);
}

std::string
ConverterRefCount::GetSafeTypeAsString(clang::QualType qual_type) const {
  std::string type_as_string;
  ConverterRefCount converter(type_as_string, ctx_);
  converter.Convert(qual_type);
  return std::string(Trim(type_as_string));
}

bool ConverterRefCount::NeedsMut(const clang::VarDecl *decl,
                                 clang::QualType type,
                                 llvm::StringRef /*name*/) const {
  return hoisted_decls_.contains(decl) && type->isReferenceType();
}

std::string ConverterRefCount::BoxType(std::string &&str) const {
  switch (getConversionKind()) {
  case ConversionKind::Unboxed:
  case ConversionKind::Pointee:
  case ConversionKind::Ptr:
    return std::move(str);
  case ConversionKind::FullRefCount:
    return std::format("Value<{}>", std::move(str));
  }
  std::unreachable();
}

std::string ConverterRefCount::BoxValue(std::string &&str) const {
  switch (getConversionKind()) {
  case ConversionKind::Unboxed:
  case ConversionKind::Pointee:
  case ConversionKind::Ptr:
    return std::move(str);
  case ConversionKind::FullRefCount:
    return std::format("Rc::new(RefCell::new({}))", std::move(str));
  }
  std::unreachable();
}

bool ConverterRefCount::Convert(clang::QualType qual_type) {
  // Catch va_list before desugaring
  if (IsVaListType(qual_type)) {
    StrCat(BoxType("VaList"));
    return false;
  }

  if (!Mapper::Contains(qual_type))
    qual_type = qual_type.getUnqualifiedType().getDesugaredType(ctx_);

  if (qual_type->isReferenceType() || qual_type->isIncompleteArrayType()) {
    return Converter::Convert(qual_type);
  }

  StrCat(BoxType(Converter::ToStringBase(qual_type)));
  return false;
}

bool ConverterRefCount::VisitIncompleteArrayType(
    clang::IncompleteArrayType *type) {
  std::string str;
  {
    PushUnboxedIfSimple push(*this, "Box<%>", type->getElementType());
    str = std::format("Box<[{}]>", ToString(type->getElementType()));
  }
  StrCat(BoxType(std::move(str)));
  return false;
}

bool ConverterRefCount::VisitReferenceType(clang::ReferenceType *type) {
  auto pointee_type = type->getPointeeType();
  if (pointee_type->isArrayType()) {
    // A reference to an array decays straight to a pointer to its first
    // element, the same way a by-value array parameter would, instead of
    // going through a pointer to the whole boxed array.
    auto element_type = pointee_type->getAsArrayTypeUnsafe()->getElementType();
    PushConversionKind push1(*this, ConversionKind::Ptr,
                             !element_type->isArrayType());
    PushConversionKind push2(*this, ConversionKind::FullRefCount,
                             element_type->isArrayType());
    StrCat("Ptr<");
    Convert(element_type);
    StrCat(token::kGt);
    return false;
  }
  PushConversionKind push(*this, ConversionKind::Pointee);
  StrCat("Ptr<");
  Convert(pointee_type);
  StrCat(token::kGt);
  return false;
}

std::string ConverterRefCount::ConvertFunctionPointerType(
    const clang::FunctionProtoType *proto, FnProtoType kind) {
  PushConversionKind push(*this, ConversionKind::Unboxed);
  return Converter::ConvertFunctionPointerType(proto, kind);
}

namespace {

// A POINTER TO AN ABSTRACT RECORD MUST NAME THE TRAIT, AND THAT ANSWER MUST NOT
// DEPEND ON EMISSION ORDER. `abstract_structs_` is populated ONLY by
// `ConvertAbstractClass` (converter.cpp:6460), reached from `ConvertClass`
// (:1317) AFTER the `record_decls_.MarkDefined(...)` early return (:1309), so a
// pointer type converted BEFORE its pointee's class is converted sees an EMPTY
// set and falls through to the bare record name. Measured and fixed on the
// unsafe side in 22b974ee (LoopUnroll.cpp: seven of eight
// `InheritWithClone<Base, Derived>::clone` definitions spelled
// `dyn ...__Virtual`, and the one instantiated FIRST in source order spelled the
// bare record -- same template, same return type, two spellings in one file).
// This is the refcount mirror of that predicate.
//
// Ask the definition directly, under exactly the condition `ConvertClass`
// emits the trait under: user-defined, convertible, abstract ON THE DEFINITION.
// NOT a third mechanism -- `isAbstract()` is already the authority in
// `Converter::VisitPointerType` and in the mapper's order-free path
// (mapper.cpp:1461-1470). The `IsUserDefinedDecl` + `IsConvertibleCXXRecordDecl`
// guards are LOAD-BEARING: without them a pointer to an abstract RULE-MAPPED or
// non-convertible record would name a trait that was never emitted (E0405).
bool IsAbstractByDefinition(const clang::CXXRecordDecl *record) {
  const clang::CXXRecordDecl *def =
      record != nullptr ? record->getDefinition() : nullptr;
  return def != nullptr && IsUserDefinedDecl(def) &&
         IsConvertibleCXXRecordDecl(def) && def->isAbstract();
}

} // namespace

bool ConverterRefCount::VisitPointerType(clang::PointerType *type) {
  if (auto proto = type->getPointeeType()->getAs<clang::FunctionProtoType>()) {
    StrCat(std::format("FnPtr<{}>", ConvertFunctionPointerType(proto)));
    return false;
  }

  if (IsVaListType(clang::QualType(type, 0))) {
    StrCat("VaList");
    return false;
  }

  if (type->isVoidPointerType()) {
    StrCat("AnyPtr");
    return false;
  }

  auto pointee_type = type->getPointeeType();
  PushConversionKind push1(*this, ConversionKind::Ptr,
                           !pointee_type->isArrayType());
  PushConversionKind push2(*this, ConversionKind::FullRefCount,
                           pointee_type->isArrayType());
  // The set is kept as a FLOOR so nothing that names a trait today stops naming
  // one; the direct ask removes the emission-order dependence.
  if (pointee_type->isRecordType() &&
      (abstract_structs_.contains(GetID(pointee_type->getAsRecordDecl())) ||
       IsAbstractByDefinition(llvm::dyn_cast_or_null<clang::CXXRecordDecl>(
           pointee_type->getAsRecordDecl())))) {
    // The trait is `<Record>__Virtual` (`<Record>` is the struct), and that
    // name cannot come from the recursive type visit below -- it would emit the
    // bare record name. So emit the trait name here and stop.
    StrCat("PtrDyn<dyn",
           GetRecordName(pointee_type->getAsRecordDecl()) + "__Virtual",
           token::kGt);
    return false;
  }
  StrCat("Ptr<");
  Convert(pointee_type);
  StrCat(token::kGt);
  return false;
}

bool ConverterRefCount::VisitRecordType(clang::RecordType *type) {
  PushConversionKind push(*this, ConversionKind::Unboxed);
  return Converter::VisitRecordType(type);
}

bool ConverterRefCount::VisitConstantArrayType(clang::ConstantArrayType *type) {
  auto conv = getConversionKind();
  PushConversionKind push(*this, ConversionKind::Unboxed);

  switch (conv) {
  case ConversionKind::Unboxed:
    StrCat('[');
    Convert(type->getElementType());
    StrCat(std::format("; {}]", GetNumAsString(type->getSize()).c_str()));
    break;
  case ConversionKind::Ptr:
    Convert(type->getElementType());
    break;
  case ConversionKind::Pointee:
  case ConversionKind::FullRefCount:
    StrCat("Box<[");
    Convert(type->getElementType());
    StrCat("]>");
    break;
  }
  return false;
}

std::string ConverterRefCount::ConvertFreshLValue(clang::Expr *expr) {
  auto str = ConvertLValue(expr);
  if (isFresh()) {
    return str;
  }
  SetFresh();
  return std::format("({}).clone()", std::move(str));
}

std::string ConverterRefCount::ConvertObject(clang::Expr *expr,
                                             ObjectShape shape) {
  PushExprKind push(*this, ExprKind::Object);
  auto saved_shape = std::exchange(object_shape_, shape);
  auto str = ToString(expr);
  object_shape_ = saved_shape;
  if (shape == ObjectShape::Element && expr->getType()->isPointerType()) {
    auto pointee = expr->getType()->getPointeeType();
    if (IsBoxedType(pointee) || pointee->isArrayType()) {
      computed_expr_type_ = ComputedExprType::FreshPointer;
      return std::format("{}.decay()", std::move(str));
    }
  }
  return str;
}

std::string
ConverterRefCount::ConvertFreshObject(clang::Expr *expr,
                                      std::string_view target_ptr_type) {
  auto shape = ObjectShape::Whole;
  if (!target_ptr_type.empty()) {
    auto type = expr->getType().getNonReferenceType();
    auto pointee = type->isPointerType() ? type->getPointeeType() : type;
    if (IsBoxedType(pointee) || pointee->isArrayType()) {
      auto normalize = [](std::string s) {
        std::erase(s, ' ');
        for (size_t pos; (pos = s.find("::<")) != std::string::npos;)
          s.erase(pos, 2);
        return s;
      };
      if (normalize(std::string(target_ptr_type)) ==
          normalize(ConvertPtrType(pointee))) {
        shape = ObjectShape::Element;
      }
    }
  }
  auto str = ConvertObject(expr, shape);
  if (isFresh()) {
    return str;
  }
  SetFresh();
  return std::format("({}).clone()", std::move(str));
}

std::string ConverterRefCount::ConvertFresh(
    clang::Expr *expr, std::optional<clang::QualType> implicit_convert_to) {
  auto str = ToString(expr, implicit_convert_to);
  if (isFresh() || expr->getType()->isVoidType() || isVoid()) {
    return str;
  }
  SetFresh();
  return std::format("({}).clone()", std::move(str));
}

std::string ConverterRefCount::ConvertFreshRValue(
    clang::Expr *expr, std::optional<clang::QualType> implicit_convert_to) {
  auto str = ConvertRValue(expr, implicit_convert_to);
  if (!isFresh() && !expr->getType()->isVoidType()) {
    SetFresh();
    return std::format("({}).clone()", std::move(str));
  }
  SetFresh();
  return str;
}

std::string ConverterRefCount::ConvertFreshPointer(clang::Expr *expr) {
  auto str = ConvertPointer(expr);
  if (isFresh()) {
    return str;
  }
  SetFresh();
  return std::format("({}).clone()", std::move(str));
}

std::pair<std::string, std::string>
ConverterRefCount::MaterializeTemp(const std::string &binding_name,
                                   clang::QualType param_type,
                                   clang::Expr *expr) {
  auto pointee = param_type.getNonReferenceType();
  auto value = ConvertRValue(expr, pointee);
  auto type_str = ToStringBase(pointee);
  const auto *decl = in_const_initializer_ ? keyword::kStatic : keyword::kLet;

  auto binding = std::format("{} {} : Value<{}> = Rc::new(RefCell::new({}));",
                             decl, binding_name, type_str, value);
  auto ref =
      in_const_initializer_ ? ".with(Value::as_pointer)" : ".as_pointer()";
  return {binding, binding_name + ref};
}

std::string ConverterRefCount::ConvertPtrType(clang::QualType type) {
  std::string str;
  // decays into Ptr; remove the outer type Vec<>
  if (IsBoxedType(type)) {
    str = GetInnerType(type);
  } else {
    PushConversionKind push(*this, ConversionKind::Ptr);
    str = ToString(type);
  }
  return std::format("Ptr<{}>", std::move(str));
}

bool ConverterRefCount::VisitArraySubscriptExpr(
    clang::ArraySubscriptExpr *expr) {
  auto *base = expr->getBase();
  if (base->IgnoreCasts()->getType()->isPointerType() ||
      IsUnionArrayMember(base) ||
      (IsReferenceType(base) &&
       base->IgnoreCasts()->getType()->isArrayType())) {
    ConvertPointerSubscript(expr);
  } else {
    if (!base->IgnoreCasts()->getType()->isArrayType()) {
      if (isLValue()) {
        pending_deref_.assert_consumed();
        Buffer buf(*this);
        ConvertArraySubscript(base, expr->getIdx(), expr->getType());
        pending_deref_.set_unchecked(std::move(buf).str(), isFresh(), expr);
        return false;
      }
      PushParen paren(*this);
      StrCat(GetPointerDerefPrefix(expr->getType()));
      ConvertArraySubscript(base, expr->getIdx(), expr->getType());
      StrCat(GetPointerDerefSuffix(expr->getType()));
      SetValueFreshness(expr->getType());
    } else {
      ConvertArraySubscript(base, expr->getIdx(), expr->getType());
    }
  }
  return false;
}

bool ConverterRefCount::VisitCXXRecordDecl(clang::CXXRecordDecl *decl) {
  if (decl_ids_.count(GetID(decl))) {
    return false;
  }
  Converter::VisitCXXRecordDecl(decl);
  return false;
}

bool ConverterRefCount::VisitOffsetOfExpr(clang::OffsetOfExpr *expr) {
  clang::Expr::EvalResult result;
  ENSURE(expr->EvaluateAsInt(result, ctx_));
  StrCat(std::format("{}_usize", result.Val.getInt().getZExtValue()));
  computed_expr_type_ = ComputedExprType::FreshValue;
  return false;
}

std::string
ConverterRefCount::GetComparisonReferenceArg(const clang::CXXRecordDecl *decl,
                                             std::string_view value) {
  PushConversionKind push(*this, ConversionKind::FullRefCount);
  return BoxValue(GetShallowCopy(decl, value)) + ".as_pointer()";
}

std::string
ConverterRefCount::GetComparisonReceiver(const clang::CXXMethodDecl *,
                                         const clang::CXXRecordDecl *decl,
                                         std::string_view lhs) {
  PushConversionKind push(*this, ConversionKind::FullRefCount);
  return std::format("&{}.as_pointer()", BoxValue(GetShallowCopy(decl, lhs)));
}

std::string ConverterRefCount::GetShallowCopy(const clang::RecordDecl *decl,
                                              std::string_view src) {
  std::string fields;
  for (auto *field : decl->fields()) {
    auto name = GetNamedDeclAsString(field);
    fields += std::format("{0}: {1}.{0}.clone(),", name, src);
  }
  return std::format("{} {{ {} }}", GetRecordName(decl), fields);
}

void ConverterRefCount::AddCloneTrait(const clang::RecordDecl *decl) {
  auto record_name = GetRecordName(decl);

  if (decl->isUnion()) {
    StrCat("impl Clone for", record_name);
    PushBrace impl_brace(*this);
    StrCat("fn clone(&self) -> Self");
    PushBrace fn_brace(*this);
    StrCat(record_name,
           "{ __bytes: Rc::new(RefCell::new(self.__bytes.borrow().clone())) }");
    return;
  }

  if (HasDefaultedCopyConstructor(decl) && RecordHasOnlyReferenceFields(decl)) {
    return;
  }
  auto *cxx = clang::dyn_cast<clang::CXXRecordDecl>(decl);
  if (!cxx) {
    StrCat(keyword::kImpl, "Clone for", record_name);
    PushBrace impl_brace(*this);
    StrCat("fn clone(&self) -> Self");
    PushBrace fn_brace(*this);
    StrCat("Self");
    PushBrace init_brace(*this);
    for (auto *field : decl->fields()) {
      auto name = GetNamedDeclAsString(field);
      StrCat(std::format(
          "{0}: Rc::new(RefCell::new((*self.{0}.borrow()).clone())),", name));
    }
    return;
  }

  if (!HasCallableCopyConstructor(cxx)) {
    return;
  }

  StrCat(keyword::kImpl, "Clone for", record_name, '{');
  StrCat("fn clone(&self) -> Self {");

  if (auto *ctor = GetUserDefinedCopyConstructor(cxx)) {
    PushConversionKind push(*this, ConversionKind::FullRefCount);
    StrCat(std::format("let __src: Value<{}> = {};", record_name,
                       BoxValue(GetShallowCopy(decl, "self"))));
    StrCat(std::format("{}::{}(__src.as_pointer())", record_name,
                       GetCtorName(ctor)));
  } else {
    for (auto ctor : cxx->ctors()) {
      if (ctor->isCopyConstructor()) {
        PushConversionKind push(*this, ConversionKind::FullRefCount);
        ConvertCXXConstructorBody(ctor);
        break;
      }
    }
  }

  StrCat('}');
  StrCat('}');
}

void ConverterRefCount::AddDefaultTrait(const clang::RecordDecl *decl) {
  PushConversionKind push(*this, ConversionKind::FullRefCount);
  Converter::AddDefaultTrait(decl);
}

void ConverterRefCount::AddDefaultTraitForUnion(const clang::RecordDecl *decl) {
  auto name = GetRecordName(decl);
  StrCat("impl Default for", name);
  PushBrace impl_brace(*this);
  StrCat("fn default() -> Self");
  PushBrace fn_brace(*this);
  StrCat(std::format(
      "{} {{ __bytes: Rc::new(RefCell::new(Box::from([0u8; {}]))) }}", name,
      ctx_.getASTRecordLayout(decl).getSize().getQuantity()));
}

void ConverterRefCount::EmitRustUnion(clang::RecordDecl *decl) {
  auto name = GetRecordName(decl);

  auto attrs = GetStructAttributes(decl);
  Mapper::SetDerives(ctx_.getCanonicalTagType(decl),
                     std::vector<std::string>(attrs.begin(), attrs.end()));

  StrCat(std::format("pub struct {} {{ __bytes: Value<Box<[u8]>> }}", name));

  StrCat("impl", name);
  {
    PushBrace impl_brace(*this);
    for (auto *field : decl->fields()) {
      PushConversionKind push(*this, ConversionKind::Unboxed);
      auto ty =
          field->getType()->isArrayType()
              ? ToString(
                    field->getType()->getAsArrayTypeUnsafe()->getElementType())
              : ToString(field->getType());
      StrCat(std::format(
          "pub fn {}(&self) -> Ptr<{}> {{ (self.__bytes.as_pointer() "
          "as Ptr<u8>).reinterpret_cast() }}",
          GetNamedDeclAsString(field), ty));
    }
  }

  AddCloneTrait(decl);
  AddDefaultTrait(decl);
  AddByteReprTrait(decl);
}

void ConverterRefCount::AddByteReprTrait(const clang::RecordDecl *decl) {
  if (RecordDerivesByteRepr(decl)) {
    return;
  }

  auto struct_name = GetRecordName(decl);

  if (!TypeImplementsByteRepr(ctx_.getCanonicalTagType(decl))) {
    StrCat(std::format("impl ByteRepr for {}", struct_name));
    PushBrace brace(*this);
    return;
  }

  StrCat("impl ByteRepr for ", struct_name);
  PushBrace impl_brace(*this);

  if (decl->isUnion()) {
    StrCat(std::format("fn byte_size() -> usize {{ {} }}",
                       ctx_.getTypeSize(ctx_.getCanonicalTagType(decl)) / 8));
    StrCat("fn to_bytes(&self, buf: &mut [u8]) { "
           "buf.copy_from_slice(&self.__bytes.borrow()); }");
    StrCat(std::format("fn from_bytes(buf: &[u8]) -> Self {{ {} {{ __bytes: "
                       "Rc::new(RefCell::new(Box::from(buf))) }} }}",
                       struct_name));
    return;
  }

  const auto &layout = ctx_.getASTRecordLayout(decl);

  StrCat(std::format("fn byte_size() -> usize {{ {} }}",
                     ctx_.getTypeSize(ctx_.getCanonicalTagType(decl)) / 8));

  StrCat("fn to_bytes(&self, buf: &mut [u8])");
  {
    PushBrace fn_brace(*this);
    unsigned idx = 0;
    for (auto *field : decl->fields()) {
      auto byte_off = layout.getFieldOffset(idx) / 8;
      auto byte_size = ctx_.getTypeSize(field->getType()) / 8;
      StrCat(std::format("(*self.{}.borrow()).to_bytes(&mut buf[{}..{}]);",
                         GetNamedDeclAsString(field), byte_off,
                         byte_off + byte_size));
      ++idx;
    }
  }

  StrCat("fn from_bytes(buf: &[u8]) -> Self");
  {
    PushBrace fn_brace(*this);
    StrCat("Self");
    PushBrace lit_brace(*this);
    unsigned idx = 0;
    for (auto *field : decl->fields()) {
      auto byte_off = layout.getFieldOffset(idx) / 8;
      auto byte_size = ctx_.getTypeSize(field->getType()) / 8;
      PushConversionKind push(*this, ConversionKind::FullRefCount);
      std::string storage_ty = ToString(field->getType());
      Unwrap(storage_ty, "Value<", ">");
      StrCat(std::format(
          "{}: Rc::new(RefCell::new(<{}>::from_bytes(&buf[{}..{}]))),",
          GetNamedDeclAsString(field), storage_ty, byte_off,
          byte_off + byte_size));
      ++idx;
    }
  }
}

std::string
ConverterRefCount::GetSelfMaybeWithMut(const clang::CXXMethodDecl *decl) {
  return "&self";
}

bool ConverterRefCount::VisitCXXConstructorDecl(
    clang::CXXConstructorDecl *decl) {
  PushConversionKind push(*this, ConversionKind::FullRefCount);
  return Converter::VisitCXXConstructorDecl(decl);
}

bool ConverterRefCount::VisitFieldDecl(clang::FieldDecl *decl) {
  PushConversionKind push(*this, ConversionKind::FullRefCount);
  return Converter::VisitFieldDecl(decl);
}

void ConverterRefCount::EmitFunctionPreamble(clang::FunctionDecl *decl) {
  // In the header, the function might be declared as `int foo(int name_1)',
  // while in the source file the function might be defined as `int foo(int
  // name_2)'. We want to get the parameters from the definition if possible,
  // i.e. name_2.
  PushConversionKind push(*this, ConversionKind::FullRefCount);
  auto params = decl->getDefinition() ? decl->getDefinition()->parameters()
                                      : decl->parameters();
  for (auto *param : params) {
    if (!param->getType()->isReferenceType()) {
      auto name = GetNamedDeclAsString(param);
      // Skip emitting the preamble for unnamed parameters
      if (name == "_") {
        continue;
      }

      auto type = ToString(param->getType());
      auto init = name;

      if (HasUsableDefaultArg(param)) {
        // Lazy: see the note at Converter::EmitFunctionPreamble.  `unwrap_or`
        // evaluates the default expression even when the caller supplied one.
        init = std::format("{}.unwrap_or_else(|| {})", name,
                           ToString(param->getDefaultArg()));
      }

      StrCat(std::format("let {} : {} = Rc::new(RefCell::new({}))", name, type,
                         init),
             token::kSemiColon);
    }
  }
}

void ConverterRefCount::ConvertVaListVarDecl(clang::VarDecl *decl) {
  if (clang::isa<clang::ParmVarDecl>(decl)) {
    // va_list parameter (decayed to __va_list_tag *)
  } else {
    // va_list local variable
    StrCat(keyword::kLet);
  }

  StrCat(GetNamedDeclAsString(decl), token::kColon, "Value<VaList>");
}

bool ConverterRefCount::ConvertLambdaVarDecl(clang::VarDecl *decl) {
  return false;
}

bool ConverterRefCount::ConvertVarDeclSkipInit(clang::VarDecl *decl) {
  bool unboxed = in_function_formals_;
  PushConversionKind push(*this, unboxed ? ConversionKind::Unboxed
                                         : ConversionKind::FullRefCount);
  return Converter::ConvertVarDeclSkipInit(decl);
}

void ConverterRefCount::EmitHoistedInArmAssignment(clang::VarDecl *decl) {
  if (!decl->hasInit()) {
    return;
  }

  const auto type = decl->getType();
  if (type->isReferenceType()) {
    StrCat(GetNamedDeclAsString(decl));
  } else {
    StrCat(token::kStar, GetNamedDeclAsString(decl), ".borrow_mut()");
  }

  PushConversionKind push(*this, ConversionKind::FullRefCount);
  StrCat(token::kAssign);
  StrCat(ConvertVarInitValue(type, decl->getInit()));
  StrCat(token::kSemiColon);
}

void ConverterRefCount::ConvertGlobalVarDecl(clang::VarDecl *decl) {
  std::string str;
  {
    Buffer buf(*this);
    ConvertVarDecl(decl);
    str = std::move(buf).str();
    if (str.empty()) {
      return;
    }
  }
  StrCat("thread_local!");
  {
    PushParen paren(*this);
    StrCat(str);
  }
  StrCat(token::kSemiColon);
}

bool ConverterRefCount::VisitVarDecl(clang::VarDecl *decl) {
  bool unboxed = in_function_formals_;
  PushConversionKind push(*this, unboxed ? ConversionKind::Unboxed
                                         : ConversionKind::FullRefCount);
  if (decl->getType()->isReferenceType()) {
    PushExprKind push(*this, ExprKind::AddrOf);
    Converter::VisitVarDecl(decl);
  } else {
    Converter::VisitVarDecl(decl);
  }
  return false;
}

void ConverterRefCount::EmitScopedDestructor(const clang::VarDecl *decl) {
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
         std::format("let _dtor_{0} = ScopedDestructor::new(&{0}, |__p| "
                     "__p.{1}())",
                     name, kDestructorName));
}

bool ConverterRefCount::ConvertIncAndDec(clang::UnaryOperator *expr) {
  auto opcode = expr->getOpcode();
  auto *sub_expr = expr->getSubExpr();

  const char *method = nullptr;
  switch (opcode) {
  case clang::UO_PostInc:
    method = "postfix_inc";
    break;
  case clang::UO_PostDec:
    method = "postfix_dec";
    break;
  case clang::UO_PreInc:
    method = "prefix_inc";
    break;
  case clang::UO_PreDec:
    method = "prefix_dec";
    break;
  default:
    return false;
  }

  auto str = ConvertLValue(sub_expr);
  if (!pending_deref_.empty()) {
    StrCat(pending_deref_.take(), ".with_mut(|__v| __v.", method, "())");
  } else {
    StrCat(str, '.', method, "()");
  }
  SetFreshType(expr->getType());
  return true;
}

bool ConverterRefCount::VisitConditionalOperator(
    clang::ConditionalOperator *expr) {
  StrCat(keyword::kIf);
  ConvertCondition(expr->getCond());
  {
    PushBrace then_brace(*this);
    StrCat(ConvertFresh(expr->getTrueExpr(), expr->getType()));
  }
  StrCat(keyword::kElse);
  {
    PushBrace else_brace(*this);
    StrCat(ConvertFresh(expr->getFalseExpr(), expr->getType()));
  }
  return false;
}

bool ConverterRefCount::VisitDeclRefExpr(clang::DeclRefExpr *expr) {
  if (isAddrOf()) {
    clang::Expr *addrof_op = ToAddrOf(ctx_, expr);
    if (auto str = GetMappedAsString(addrof_op); !str.empty()) {
      StrCat(str);
      SetFreshType(expr->getType());
      return false;
    }
  }

  if (ShouldReplaceWithMappedBody(expr)) {
    if (auto str = GetMappedAsString(expr); !str.empty()) {
      StrCat(str);
      SetFreshType(expr->getType());
      return false;
    }
  }

  auto str = ConvertDeclRefExpr(expr);
  auto decl = expr->getDecl();

  if (auto fn_decl = clang::dyn_cast<clang::FunctionDecl>(decl)) {
    if (isAddrOf()) {
      ConvertFunctionToFunctionPointer(fn_decl);
    } else {
      StrCat(str);
      SetFreshType(expr->getType());
    }
    return false;
  }

  if (clang::isa<clang::EnumConstantDecl>(decl)) {
    StrCat(str);
    computed_expr_type_ = ComputedExprType::FreshValue;
    return false;
  }

  const auto decl_t = decl->getType();
  bool is_global_value = false, is_global_ptr = false;
  if (IsGlobalVar(expr)) {
    if (decl_t->isReferenceType()) {
      str += ".with(Ptr::clone)";
      is_global_ptr = true;
    } else {
      is_global_value = true;
    }
  }

  if (auto *ref = decl_t->getAs<clang::ReferenceType>()) {
    if (map_iter_decls_.contains(clang::dyn_cast<clang::VarDecl>(decl))) {
      StrCat(str);
      SetValueFreshness(expr->getType());
      return false;
    }

    if (isObject() && WantsElementPtr() && IsBoxedType(ref->getPointeeType())) {
      StrCat(str, ".decay()");
      computed_expr_type_ = ComputedExprType::FreshPointer;
      return false;
    }

    // references are not boxed
    if (isAddrOf()) {
      StrCat(str);
      computed_expr_type_ = ComputedExprType::Pointer;
    } else {
      if (str == "self") {
        StrCat(str);
      } else {
        if (isLValue()) {
          pending_deref_.set(str, /*fresh=*/false);
          return false;
        }
        StrCat(DerefPtrExpr(str, ref->getPointeeType()));
      }
      SetValueFreshness(expr->getType());
    }
    return false;
  }

  if (isAddrOf()) {
    if (is_global_value) {
      StrCat(std::format("{}.with(|v| v.as_pointer())", std::move(str)));
    } else if (is_global_ptr) {
      StrCat(str);
    } else {
      StrCat(str, ".as_pointer()");
    }
    computed_expr_type_ = ComputedExprType::FreshPointer;
    return false;
  }

  bool fresh = false;
  if (isRValue()) {
    if (is_global_value) {
      StrCat(str, TypeIsCopyable(decl_t) ? ".with(|rc| *rc.borrow())"
                                         : ".with(|rc| rc.borrow().clone())");
      fresh = true;
    } else if (is_global_ptr) {
      StrCat(str);
      fresh = true;
    } else {
      StrCat(std::format("(*{}.borrow())", std::move(str)));
    }
  } else {
    if (is_global_value) {
      str += ".with(Value::clone)";
    }
    StrCat(std::format("(*{}.borrow_mut())", std::move(str)));
  }

  if (auto *decl = clang::dyn_cast<clang::VarDecl>(expr->getDecl())) {
    if (decl->getType()->isPointerType()) {
      computed_expr_type_ = ComputedExprType::Pointer;
      return false;
    }
  }
  if (fresh) {
    SetFreshType(expr->getType());
  } else {
    SetValueFreshness(expr->getType());
  }
  return false;
}

// Translates a C printf format string in place into a Rust format string,
// returning one entry per conversion -- null for "pass the argument through",
// otherwise the `as` cast the Rust side needs.
//
// UNKNOWN CONVERSION SPECIFIERS. The set understood here is a subset of C's
// (`%f` alone, for instance, is not in it -- only the `%.0Nf` shape is), so an
// unknown specifier is a GENUINE, RECOVERABLE PORTING GAP: the corpus is
// allowed to contain one and the fleet needs to be told which.
//
// This used to end in `llvm::errs() << ...; assert(0);` as the LAST statement of
// the loop body WITHOUT ADVANCING `pos`. Under the release build's -DNDEBUG the
// assert is a no-op, so control fell back to the `while` with `pos` unchanged,
// `find('%', pos)` returned the same offset, and the converter re-printed
// `Unknown printf format:` forever: a HANG plus unbounded log growth, which in a
// 403-TU sweep burns the timeout slot and yields no diagnosis at all. Measured
// before this change: 114,997 identical lines / 3.7 MB in 25 s on a `printf("val
// %f\n", d)`, killed by `timeout`.
//
// So both paths now make real forward progress past the offending specifier:
//   * under --survey, RECORD the gap and continue -- survey mode is the fleet's
//     403-TU work list and must never abort or spin on a recoverable gap;
//   * otherwise `report_fatal_error`, NAMING the specifier and the call site,
//     because silently emitting a Rust format string with a leftover C
//     conversion in it would be silent wrongness.
// Converting the assert alone would have turned the hang into an abort on a path
// survey mode has to survive, which is why the survey arm is not optional.
static std::vector<const char *> printf2fmt(std::string &format,
                                            const std::string &loc) {
  std::vector<const char *> types;
  // FIRST, before a single `{}` placeholder is inserted below: everything in
  // `format` right now is LITERAL C text, and everything inserted after this
  // point is a placeholder that must NOT be escaped. Doing it in this order is
  // what keeps the two apart -- escaping afterwards would double the `{}` this
  // function just wrote and print `{}` instead of the argument.
  format = Converter::EscapeFmtBraces(format);
  size_t pos = 0;
  while ((pos = format.find('%', pos)) != std::string::npos) {
    if (pos + 1 >= format.size())
      break;

    switch (auto c = format[pos + 1]) {
    case 'c':
      types.emplace_back("u8 as char");
      format.replace(pos, 2, "{}");
      pos += 2;
      continue;
    case 'd':
    case 'i':
    case 's':
    case 'u':
      types.emplace_back();
      format.replace(pos, 2, "{}");
      pos += 2;
      continue;
    case 'p':
      types.emplace_back();
      format.replace(pos, 2, "{:?}");
      pos += 2;
      continue;
    case '%':
      // `%%` is a LITERAL percent: it consumes NO argument, so it must NOT push
      // a `types` entry. `types` is indexed by ARGUMENT (see ConvertPrintf), and
      // an entry here shifted every later argument's cast by one -- measured on
      // `printf("100%% %c\n", ch)`, where the `%c` cast landed on the `%%` slot
      // and the char printed as its numeric value.
      format.replace(pos, 2, "%");
      pos += 2;
      continue;
    case 'l':
      if (pos + 2 < format.size() &&
          (format[pos + 2] == 'd' || format[pos + 2] == 'u')) {
        types.emplace_back();
        format.replace(pos, 3, "{}");
        pos += 2;
        continue;
      }
      if (pos + 3 < format.size() && format[pos + 2] == 'l' &&
          (format[pos + 3] == 'd' || format[pos + 3] == 'u')) {
        types.emplace_back();
        format.replace(pos, 4, "{}");
        pos += 2;
        continue;
      }
      break;
    case 'z':
      if (pos + 2 < format.size() &&
          (format[pos + 2] == 'd' || format[pos + 2] == 'u')) {
        types.emplace_back();
        format.replace(pos, 3, "{}");
        pos += 2;
        continue;
      }
      break;
    case '.':
      if (pos + 3 < format.size() && format[pos + 2] == '0') {
        auto end = format.find_first_not_of("0123456789", pos + 3);
        if (end != std::string::npos && format[end] == 'f') {
          auto repl = "{:." + format.substr(pos + 3, end - pos - 3) + '}';
          format.replace(pos, end - pos + 1, repl);
          pos += repl.size();
          types.emplace_back();
          continue;
        }
      }
      break;
    default:
      if (c >= '0' && c <= '9') {
        auto end = format.find_first_not_of("0123456789", pos + 2);
        if (end != std::string::npos) {
          auto repl = "{:" + format.substr(pos + 1, end - pos - 1);
          bool ok = true;
          switch (c = format[end]) {
          case 'd':
            break;
          case 'x':
            repl += c;
            break;
          case 'z':
            if (end + 1 < format.size() && format[end + 1] == 'u') {
              ++end;
            } else {
              ok = false;
            }
            break;
          default:
            ok = false;
            break;
          }
          if (ok) {
            repl += '}';
            format.replace(pos, end - pos + 1, repl);
            pos += repl.size();
            types.emplace_back();
            continue;
          }
        }
      }
    }
    // `pos + 1 < format.size()` is guaranteed by the loop head above, so the
    // specifier character always exists.
    const std::string detail =
        std::string("unknown printf conversion `%") + format[pos + 1] +
        "` in format " + format;
    if (survey::Enabled()) {
      survey::Record(survey::GapKind::kUnsupportedConstruct, detail, loc);
      // FORWARD PROGRESS, the whole point: step past `%` and the specifier so
      // the next `find('%', pos)` cannot return this offset again. Survey mode
      // writes no Rust (see survey.h), so leaving the C conversion in `format`
      // and leaving the Rust format string malformed costs nothing here. The
      // null `types` entry keeps the vector in step with the argument this
      // specifier consumes, because the caller indexes `types` by argument.
      types.emplace_back();
      pos += 2;
      continue;
    }
    llvm::report_fatal_error(
        llvm::Twine(detail) + (loc.empty() ? "" : " at " + loc),
        // Third named refusal, same measured reason: the default abort()s, and
        // the sweep buckets by exit code, so all three now exit 1 alike.
        /*gen_crash_diag=*/false);
  }
  return types;
}

void ConverterRefCount::ConvertPrintf(clang::CallExpr *expr) {
  bool is_fprintf =
      Mapper::ToString(expr->getCallee()).starts_with("int fprintf");
  std::string format;
  if (auto *str = clang::dyn_cast<clang::StringLiteral>(
          expr->getArg(is_fprintf)->IgnoreImplicit())) {
    format = GetEscapedStringLiteral(str);
  } else {
    llvm::errs() << "Unknown fprintf format: ";
    expr->getArg(1)->dump();
    llvm::errs() << '\n';
    cpp2rust::tu_guard::BailOut("converter_refcount.cpp unknown fprintf format");
  }
  bool ends_newline = format.ends_with("\\n\"");

  auto fd = is_fprintf ? Mapper::ToString(expr->getArg(0)) : "stdout";
  if (fd == "stdout" || fd == "__stdoutp") {
    StrCat(ends_newline ? "println!(" : "print!(");
  } else if (fd == "stderr" || fd == "__stderrp") {
    StrCat(ends_newline ? "eprintln!(" : "eprint!(");
  } else {
    llvm::errs() << "Unknown fprintf fd: " << fd << '\n';
    cpp2rust::tu_guard::BailOut("converter_refcount.cpp unknown fprintf fd");
  }
  if (ends_newline) {
    format.replace(format.size() - 3, 2, "");
  }
  auto types = printf2fmt(
      format, expr->getBeginLoc().printToString(ctx_.getSourceManager()));
  StrCat(format);

  // INVARIANT: `types` is parallel to the VARIADIC ARGUMENTS -- printf2fmt
  // pushes exactly one entry per conversion that consumes an argument, and a
  // NULL entry means "pass this argument through with no cast". So the cast for
  // argument `i` is `types[i - first]`, nothing else.
  //
  // This used to read `if (types[j]) StrCat(kAs, types[j++]);`, which advanced
  // `j` ONLY on a non-null entry: every pass-through conversion PINNED `j`, and
  // the next cast-needing conversion then took its cast from the WRONG SLOT and
  // applied it to an argument it does not belong to. Measured on
  // `printf("%d %c\n", 65, 'B')`: the `%d` entry is null, so `j` stayed 0 and
  // the `u8 as char` cast from the `%c` slot was applied to the FIRST argument,
  // emitting `println!("{} {}", 65 as u8 as char, 'B' as u8)` -- prints "A 66"
  // where C prints "65 B". It compiled and printed the wrong values, i.e. silent
  // wrongness. 73cfd28 made it more reachable: the survey arm of printf2fmt now
  // pushes a null entry for an unknown specifier, so `%f` followed by `%c`
  // under --survey hits it too.
  const unsigned first = is_fprintf + 1;
  for (unsigned i = first, e = expr->getNumArgs(); i < e; ++i) {
    StrCat(token::kComma);
    Convert(expr->getArg(i));
    // A variadic argument with no conversion left to describe it (more args than
    // conversions) gets no cast rather than an out-of-bounds read.
    if (unsigned j = i - first; j < types.size() && types[j])
      StrCat(keyword::kAs, types[j]);
  }
  StrCat(')');
}

bool ConverterRefCount::VisitCallExpr(clang::CallExpr *expr) {
  if (IsBuiltinVaStart(expr) || IsBuiltinVaEnd(expr) || IsBuiltinVaCopy(expr)) {
    ConvertVAArgCall(expr);
    return false;
  }

  // p->~T() on a scalar is a no-op
  if (clang::isa<clang::CXXPseudoDestructorExpr>(
          expr->getCallee()->IgnoreParenImpCasts())) {
    return false;
  }

  // p->~T() is a no-op when T has nothing to destruct
  if (auto *dtor = clang::dyn_cast_or_null<clang::CXXDestructorDecl>(
          expr->getCalleeDecl());
      dtor && !RecordNeedsDestruction(dtor->getParent())) {
    return false;
  }

  if (IsImplicitAssignmentCall(expr) && !Mapper::Contains(expr->getCallee())) {
    auto *call = clang::cast<clang::CXXMemberCallExpr>(expr);
    ConvertAssignment(call->getImplicitObjectArgument(), call->getArg(0), "=");
    return false;
  }

  if (IsTransparentStdCall(expr)) {
    return Converter::VisitCallExpr(expr);
  }

  if (auto *opcall = clang::dyn_cast<clang::CXXOperatorCallExpr>(expr);
      opcall && !IsUserOperatorCall(opcall) &&
      !Mapper::Contains(expr->getCallee())) {
    return ConvertCXXOperatorCallExpr(opcall);
  }

  std::optional<TempMaterializationCtx> ctx;
  std::string str;
  {
    PushConversionKind push(*this, ConversionKind::Unboxed);
    Buffer buf(*this);
    ctx = Converter::ConvertCallExpr(expr);
    str = std::move(buf).str();
  }

  auto ty = GetReturnTypeOfFunction(expr);
  auto ref = clang::dyn_cast<clang::ReferenceType>(ty);

  if (ref && !isAddrOf() && !isVoid()) {
    // pending_deref_'s contract is "this string is a `Ptr<T>`; a later
    // EmitSetOrAssign / ConvertMappedMethodCall will consume it as
    // `.write(..)` / `.with_mut(..)`". A rule that returns a Rust `&mut T` is
    // NOT a Ptr, and nothing downstream will ever consume it -- the inner call
    // of a chain then contributes the empty string and leaves the slot set,
    // which is the `pending_deref_ not consumed` abort. Such a return is
    // already a reference, so deref it here into an ordinary place; the
    // enclosing placeholder reborrows it as `&mut (*..)` when its own rule
    // parameter is declared `&mut`.
    if (isLValue() && !Mapper::ReturnsMutRef(expr)) {
      if (ctx && !ctx->temporary_bindings.empty()) {
        str = std::format("{{ {} {} }}", ctx->temporary_bindings, str);
      }
      pending_deref_.set(str, /*fresh=*/true);
      return false;
    }
    // A RULE THAT RETURNS A RUST `&mut T` IS NOT A `Ptr<T>` -- the same
    // invariant as the `pending_deref_` gate above, at the OTHER exit of this
    // branch. `DerefPtrExpr` speaks the Ptr protocol: for a non-POD pointee it
    // appends `.upgrade().deref()`, which exists on `Ptr<T>` and on nothing
    // else. Applying it to a `&mut T` return -- e.g. the receiver of the second
    // link of an `operator<<` chain, `&mut InFlightDiagnostic` -- emits
    // `&mut (*shl_bytes_mut(..).upgrade().deref())` and gives
    // `E0599: no method named `upgrade` found for mutable reference`.
    // A Rust reference dereferences with a plain `*` for every pointee type.
    str = Mapper::ReturnsMutRef(expr)
              ? std::format("({}{})", token::kStar, str)
              : DerefPtrExpr(str, ref->getPointeeType());
    if (ctx && !ctx->temporary_bindings.empty()) {
      str = std::format("{{ {} {} }}", ctx->temporary_bindings, str);
    }
    StrCat(str);
    SetValueFreshness(ref->getPointeeType());
    return false;
  }

  if (isAddrOf() && !ty->isReferenceType() && !IsPointerType(ty)) {
    PushConversionKind push(*this, ConversionKind::FullRefCount);
    StrCat(BoxValue(std::move(str)), ".as_pointer()");
    return false;
  }

  if (isObject() && WantsElementPtr() && ref &&
      IsBoxedType(ref->getPointeeType())) {
    StrCat(std::format("{}.decay()", std::move(str)));
    computed_expr_type_ = ComputedExprType::FreshPointer;
    return false;
  }

  if (ctx && !ctx->temporary_bindings.empty()) {
    str = std::format("({{ {} {} }})", ctx->temporary_bindings, str);
  }
  StrCat(str);
  if (IsPassThroughRule(expr)) {
    return false;
  }
  if (IsPointerType(ty) || ty->isReferenceType()) {
    computed_expr_type_ = ComputedExprType::FreshPointer;
  } else {
    computed_expr_type_ = ComputedExprType::FreshValue;
  }
  return false;
}

bool ConverterRefCount::VisitStringLiteral(clang::StringLiteral *expr) {
  if (IsCodeUnitStringLiteral(expr)) {
    auto arr = GetCodeUnitArrayLiteral(expr);
    if (IsArrayInitContext()) {
      StrCat(std::format("Box::from({})", arr));
    } else {
      // Same bit, same mechanism as the `b"..."` arm below: `&[...]` is ALREADY
      // a reference, so without this the ParamIsSharedRef path prefixes a
      // second borrow and a `&[u8]` rule parameter gets `&&[...]`. Only this
      // arm -- the `Box::from(...)` arm above is a value.
      StrCat('&' + arr);
      emitted_a_reference_ = true;
    }
    computed_expr_type_ = ComputedExprType::FreshValue;
    return false;
  }

  if (IsArrayInitContext()) {
    uint64_t pad = 1;
    if (auto *arr_ty = ctx_.getAsConstantArrayType(curr_init_type_.back())) {
      uint64_t arr_size = arr_ty->getSize().getZExtValue();
      if (expr->getString().empty()) {
        StrCat(std::format("vec![0u8; {}].into_boxed_slice()", arr_size));
        computed_expr_type_ = ComputedExprType::FreshValue;
        return false;
      }
      pad = arr_size > expr->getString().size()
                ? arr_size - expr->getString().size()
                : 0;
    }
    StrCat(std::format("Box::from(*b{})", GetEscapedStringLiteral(expr, pad)));
    computed_expr_type_ = ComputedExprType::FreshValue;
    return false;
  }
  StrCat(std::format("b{}", GetEscapedStringLiteral(expr, 0)));
  computed_expr_type_ = ComputedExprType::FreshValue;
  // `b"x"` has type `&'static [u8; N]` -- it is ALREADY a reference, exactly
  // like the `c"x"` and embedded-NUL arms of the base model's
  // VisitStringLiteral. Without this bit the ParamIsSharedRef path prefixes a
  // borrow and a `&[u8]` rule parameter gets `&&'static [u8; N]`.
  emitted_a_reference_ = true;
  return false;
}

bool ConverterRefCount::VisitImplicitCastExpr(clang::ImplicitCastExpr *expr) {
  auto *sub_expr = expr->getSubExpr();

  if (expr->isXValue() && sub_expr->isLValue()) {
    Convert(sub_expr);
    computed_expr_type_ = ComputedExprType::Value;
    return false;
  }

  if (auto *unary = clang::dyn_cast<clang::UnaryOperator>(sub_expr);
      expr->getCastKind() == clang::CastKind::CK_LValueToRValue && unary &&
      (unary->isPostfix() || unary->isPrefix())) {
    return Convert(sub_expr);
  }

  if (expr->getCastKind() == clang::CastKind::CK_BitCast) {
    if (expr->getType()->isVoidPointerType()) {
      if (sub_expr->getType()->isVoidPointerType()) {
        return Convert(sub_expr);
      }
      PushConversionKind push(*this, ConversionKind::Unboxed);
      if (sub_expr->getType()->isPointerType() &&
          sub_expr->getType()->getPointeeType()->isArrayType()) {
        StrCat(std::format("({} as Ptr<{}>).to_any()",
                           ConvertFreshPointer(sub_expr),
                           ToString(sub_expr->getType()
                                        ->getPointeeType()
                                        ->getAsArrayTypeUnsafe()
                                        ->getElementType())));
      } else if (IsStringLiteralExpr(sub_expr)) {
        StrCat(std::format("{}.to_any()", ConvertFreshPointer(sub_expr)));
      } else {
        StrCat(std::format("({} as {}).to_any()", ConvertFreshPointer(sub_expr),
                           ToString(sub_expr->getType())));
      }
      computed_expr_type_ = ComputedExprType::FreshPointer;
    } else if (sub_expr->getType()->isVoidPointerType() &&
               expr->getType()->isPointerType()) {
      Convert(sub_expr);
      PushConversionKind push(*this, ConversionKind::Unboxed);
      StrCat(std::format(".reinterpret_cast::<{}>()",
                         ConvertPointeeType(expr->getType())));
      computed_expr_type_ = ComputedExprType::FreshPointer;
    } else {
      Convert(sub_expr);
    }
    return false;
  }

  if (expr->getCastKind() == clang::CastKind::CK_DerivedToBase) {
    if (expr->getType()->isPointerType()) {
      auto ptype = clang::dyn_cast<clang::PointerType>(expr->getType());
      auto pointee_type = ptype->getPointeeType()->getAsCXXRecordDecl();

      // Same floor + direct ask as `VisitPointerType` above, and for a reason
      // that is NOT cosmetic: this predicate decides whether the cast is wrapped
      // in `.to_dyn`, while the TARGET TYPE SPELLING comes from
      // `ConvertPointeeType` -> `Unwrap(ToString(ptr_type), "PtrDyn<", ">")`,
      // i.e. it inherits `VisitPointerType`'s answer. If the two predicates
      // disagreed, a target spelled `dyn <Name>__Virtual` would be produced by
      // an unwrapped cast. Inheriting the spelling is correct; no suffix is
      // appended here, so there is no double-suffix risk.
      if (pointee_type && (abstract_structs_.contains(GetID(pointee_type)) ||
                           IsAbstractByDefinition(pointee_type))) {
        PushConversionKind push(*this, ConversionKind::Unboxed);
        StrCat(std::format("{}.to_dyn::<{}>(|w| w)",
                           ToString(sub_expr->IgnoreCasts()),
                           ConvertPointeeType(expr->getType())));
        computed_expr_type_ = ComputedExprType::FreshPointer;
        return false;
      }
    }
  }

  if (expr->getCastKind() == clang::CastKind::CK_ArrayToPointerDecay) {
    if (IsVaListType(sub_expr->getType())) {
      Convert(sub_expr);
      return false;
    }
    if (IsStringLiteralExpr(sub_expr)) {
      auto code_unit = ToStringBase(
          ctx_.getAsArrayType(
                  sub_expr->IgnoreParens()->IgnoreImplicit()->getType())
              ->getElementType());
      StrCat(std::format("Ptr::<{}>::from_string_literal({})", code_unit,
                         ToString(sub_expr->IgnoreParens())));
      computed_expr_type_ = ComputedExprType::FreshPointer;
      return false;
    } else {
      // we need to write (var.as_pointer as Ptr<T>) because Rust isn't
      // smart enough to pick the right specialization
      PushConversionKind push(*this, ConversionKind::Unboxed);
      PushParen paren(*this);
      StrCat(ConvertPointer(sub_expr));
      if (!IsReferenceType(sub_expr)) {
        StrCat(keyword::kAs, ToString(expr->getType()));
      }
      return false;
    }
  }

  if (expr->getCastKind() == clang::CastKind::CK_NullToPointer) {
    PushConversionKind push(*this, ConversionKind::Unboxed);
    StrCat(GetDefaultAsString(expr->getType()));
    computed_expr_type_ = ComputedExprType::FreshPointer;
    return false;
  }

  if (expr->getCastKind() == clang::CastKind::CK_NoOp) {
    Convert(sub_expr);

    if (expr->getType()->isPointerType() &&
        sub_expr->getType()->isPointerType()) {
      auto dest_type = ConvertPointeeType(expr->getType());
      if (dest_type != ConvertPointeeType(sub_expr->getType())) {
        StrCat(std::format(".reinterpret_cast::<{}>()", dest_type));
        computed_expr_type_ = ComputedExprType::FreshPointer;
        return false;
      }
    }
    return false;
  }

  return Converter::VisitImplicitCastExpr(expr);
}

void ConverterRefCount::EmitFnPtrCall(clang::Expr *callee) {
  Convert(callee);
  StrCat(".call");
}

void ConverterRefCount::ConvertFunctionToFunctionPointer(
    const clang::FunctionDecl *fn_decl) {
  StrCat(std::format("FnPtr::<{}>::new({})",
                     ConvertFunctionPointerType(
                         fn_decl->getType()->getAs<clang::FunctionProtoType>()),
                     GetFunctionRefName(fn_decl)));
  computed_expr_type_ = ComputedExprType::FreshPointer;
}

std::string ConverterRefCount::ConvertFnPtrPlaceholder(clang::Expr *arg) {
  return ConvertFnPtrCallee(arg);
}

void ConverterRefCount::ConvertEqualsNullPtr(clang::Expr *expr) {
  StrCat('(');
  Convert(expr);
  StrCat(").is_null()");
  computed_expr_type_ = ComputedExprType::FreshValue;
}

bool ConverterRefCount::VisitFunctionPointerCast(
    clang::ExplicitCastExpr *expr) {
  if (expr->getType()->isFunctionPointerType() ||
      expr->getSubExpr()->getType()->isFunctionPointerType()) {
    if (expr->getSubExpr()->getType()->isFunctionPointerType() &&
        expr->getType()->isFunctionPointerType()) {
      auto target_proto =
          expr->getType()->getPointeeType()->getAs<clang::FunctionProtoType>();
      StrCat(std::format("{}.cast::<{}>()", ToString(expr->getSubExpr()),
                         ConvertFunctionPointerType(target_proto)));
    } else if (expr->getSubExpr()->getType()->isFunctionPointerType() ||
               expr->getType()->isVoidPointerType()) {
      Convert(expr->getSubExpr());
      StrCat(".to_any()");
    } else if (expr->getSubExpr()->getType()->isVoidPointerType() ||
               expr->getType()->isFunctionPointerType()) {
      auto target_proto =
          expr->getType()->getPointeeType()->getAs<clang::FunctionProtoType>();
      auto fn_type = ConvertFunctionPointerType(target_proto);
      StrCat(std::format("{}.cast_fn::<{}>().expect(\"ub:wrong fn type\")",
                         ToString(expr->getSubExpr()), fn_type));
    } else {
      assert(0 && "Unhandled function pointer cast");
    }
    computed_expr_type_ = ComputedExprType::FreshPointer;
    return false;
  }

  return true;
}

bool ConverterRefCount::VisitExplicitCastExpr(clang::ExplicitCastExpr *expr) {
  if (expr->getTypeAsWritten()->isVoidType()) {
    StrCat(token::kRef);
    PushParen paren(*this);
    PushExprKind push(*this, ExprKind::Void);
    Convert(expr->getSubExpr());
    return false;
  }
  if (expr->getCastKind() == clang::CK_NullToPointer) {
    PushConversionKind push(*this, ConversionKind::Unboxed);
    StrCat(GetDefaultAsString(expr->getType()));
    computed_expr_type_ = ComputedExprType::FreshPointer;
    return false;
  }
  switch (expr->getStmtClass()) {
  case clang::Stmt::CXXReinterpretCastExprClass:
    // ⛔ THE `assert(0)`-SURVIVES-NDEBUG CLASS AGAIN, and this instance was a
    // SIGSEGV, not a bad emission. The guard here used to be
    //   assert(expr->getType()->isPointerType() &&
    //          "Only pointer casts are supported in reinterpret_cast");
    // which the release build compiles to NOTHING, so a non-pointer target fell
    // straight through to `getPointeeType()` -- a NULL QualType -- and
    // GetUnsafeTypeAsString -> Converter::Convert(QualType) dereferenced it.
    // MEASURED: `sys-arch-spec/senulatorprog/senulatorProg.cpp:328`
    //   `int64_t bin = reinterpret_cast<int64_t>(initBin);`
    // and `util/memtracker/mem_track.cpp` both died `rc=139` with NO
    // `LLVM ERROR` line and an unsymbolised stack -- the worst outcome in this
    // pipeline, because there is no diagnostic for a census to classify.
    //
    // ⭐ AND IT IS AN ACCIDENTAL ASYMMETRY, NOT A MISSING FEATURE. A pointer/
    // integral `reinterpret_cast` carries the SAME clang CastKind as the
    // C-style cast of the same operand (`CK_PointerToIntegral` /
    // `CK_IntegralToPointer`), and the CStyleCast/StaticCast arm below already
    // translates exactly those two kinds. Only the SPELLING differed, so
    // routing both here is not a semantic choice -- it is deleting the
    // asymmetry. Kept as a separate block rather than merged case labels
    // because the rest of that arm (void-pointer peeling, function-pointer
    // casts) is NOT valid for `reinterpret_cast`.
    if (expr->getCastKind() == clang::CastKind::CK_PointerToIntegral) {
      StrCat(std::format("{}.to_int()", ToString(expr->getSubExpr())));
      computed_expr_type_ = ComputedExprType::FreshValue;
      return false;
    }
    if (expr->getCastKind() == clang::CastKind::CK_IntegralToPointer) {
      std::string dst_type;
      {
        PushConversionKind push(*this, ConversionKind::Unboxed);
        dst_type = ToString(expr->getType());
      }
      StrCat(std::format("<{}>::from_int({})", dst_type,
                         ToString(expr->getSubExpr())));
      computed_expr_type_ = ComputedExprType::FreshPointer;
      return false;
    }
    // Anything else with a non-pointer target is genuinely unhandled. It must
    // be a failure that SURVIVES NDEBUG, so it is a `report_fatal_error` that
    // NAMES the written type and the location -- a loud abort is strictly
    // better than a segfault because the TU becomes classifiable.
    // `getPointeeType()` is null for references too on some shapes, so the test
    // stays on the TARGET type rather than on the pointee.
    if (!expr->getType()->isPointerType()) {
      llvm::report_fatal_error(
          llvm::Twine("unsupported reinterpret_cast to non-pointer type `") +
              expr->getTypeAsWritten().getAsString() + "` (cast kind `" +
              expr->getCastKindName() + "`, operand type `" +
              expr->getSubExpr()->getType().getAsString() + "`) at " +
              expr->getExprLoc().printToString(ctx_.getSourceManager()),
          /*gen_crash_diag=*/false);
    }
    StrCat(
        std::format("{}.reinterpret_cast::<{}>()", ToString(expr->getSubExpr()),
                    GetUnsafeTypeAsString(expr->getType()->getPointeeType())));
    computed_expr_type_ = ComputedExprType::FreshPointer;
    return false;
  case clang::Stmt::CStyleCastExprClass:
  case clang::Stmt::CXXStaticCastExprClass:
    if (expr->getCastKind() == clang::CastKind::CK_PointerToIntegral ||
        expr->getCastKind() == clang::CastKind::CK_IntegralToPointer) {
      std::string dst_type;
      {
        PushConversionKind push(*this, ConversionKind::Unboxed);
        dst_type = ToString(expr->getType());
      }
      if (expr->getCastKind() == clang::CastKind::CK_PointerToIntegral) {
        StrCat(std::format("{}.to_int()", ToString(expr->getSubExpr())));
        computed_expr_type_ = ComputedExprType::FreshValue;
      } else {
        StrCat(std::format("<{}>::from_int({})", dst_type,
                           ToString(expr->getSubExpr())));
        computed_expr_type_ = ComputedExprType::FreshPointer;
      }
      return false;
    }

    if (!VisitFunctionPointerCast(expr)) {
      return false;
    } else if (expr->getSubExpr()->getType()->isVoidPointerType() &&
               expr->getType()->isVoidPointerType()) {
      return Convert(expr->getSubExpr());
    } else if (expr->getSubExpr()->getType()->isVoidPointerType() &&
               expr->getType()->isPointerType()) {
      Convert(expr->getSubExpr());
      PushConversionKind push(*this, ConversionKind::Unboxed);
      StrCat(std::format(".reinterpret_cast::<{}>()",
                         ConvertPointeeType(expr->getType())));
      computed_expr_type_ = ComputedExprType::FreshPointer;
      return false;
    } else if (expr->getType()->isVoidPointerType() &&
               expr->getSubExpr()->getType()->isPointerType()) {
      StrCat(
          std::format("{}.to_any()", ConvertFreshPointer(expr->getSubExpr())));
      computed_expr_type_ = ComputedExprType::FreshPointer;
      return false;
    } else if (expr->getSubExpr()->getType()->isPointerType() &&
               !expr->getSubExpr()->isNullPointerConstant(
                   ctx_, clang::Expr::NPC_ValueDependentIsNull)) {
      StrCat(std::format("{}.reinterpret_cast::<{}>()",
                         ToString(expr->getSubExpr()),
                         ConvertPointeeType(expr->getType())));
      computed_expr_type_ = ComputedExprType::FreshPointer;
      return false;
    }
    return Converter::VisitExplicitCastExpr(expr);
  case clang::Stmt::CXXFunctionalCastExprClass:
    return Converter::VisitExplicitCastExpr(expr);
  default:
    return Convert(expr->getSubExpr());
  }
}

bool ConverterRefCount::VisitUnaryExprOrTypeTraitExpr(
    clang::UnaryExprOrTypeTraitExpr *expr) {
  auto arg_type = expr->isArgumentType() ? expr->getArgumentType()
                                         : expr->getArgumentExpr()->getType();
  switch (expr->getKind()) {
  case clang::UnaryExprOrTypeTrait::UETT_SizeOf:
    // TODO: Once Values are dropped from fields, precomputation should be gone
    if (RustSizeDivergesFromC(arg_type)) {
      StrCat(std::format("{}usize", ctx_.getTypeSize(arg_type) / 8));
      computed_expr_type_ = ComputedExprType::FreshValue;
      return false;
    }
    break;
  case clang::UnaryExprOrTypeTrait::UETT_AlignOf:
  case clang::UnaryExprOrTypeTrait::UETT_PreferredAlignOf:
    // TODO: Once Values are dropped from fields, precomputation should be gone
    if (RustSizeDivergesFromC(arg_type)) {
      StrCat(std::format("{}usize", ctx_.getTypeAlign(arg_type) / 8));
      computed_expr_type_ = ComputedExprType::FreshValue;
      return false;
    }
    break;
  default:
    break;
  }
  return Converter::VisitUnaryExprOrTypeTraitExpr(expr);
}

bool ConverterRefCount::VisitStmtExpr(clang::StmtExpr *expr) {
  PushConversionKind push(*this, ConversionKind::FullRefCount);
  return Converter::VisitStmtExpr(expr);
}

void ConverterRefCount::EmitStmtExprTail(clang::Expr *tail) {
  StrCat("let __result = ");
  Convert(tail);
  StrCat(token::kSemiColon);
  StrCat("__result");
  SetFreshType(tail->getType());
}

void ConverterRefCount::ConvertBinaryOperator(clang::BinaryOperator *expr) {
  auto *lhs = expr->getLHS();
  auto *rhs = expr->getRHS();
  auto lhs_type = lhs->getType();
  auto rhs_type = rhs->getType();
  std::string_view opcode_as_string = expr->getOpcodeStr();

  if (auto *assign = llvm::dyn_cast<clang::CompoundAssignOperator>(expr);
      assign && GetSafeTypeAsString(lhs_type) !=
                    GetSafeTypeAsString(assign->getComputationResultType())) {
    auto computation_result_type = assign->getComputationResultType();
    PushBrace brace(*this);
    StrCat(keyword::kLet, "rhs_0", token::kAssign);
    if (IsUnsignedArithOp(assign)) {
      PushParen outer(*this);
      {
        PushParen inner(*this);
        StrCat(ConvertRValue(lhs));
        ConvertCast(computation_result_type);
      }
      ConvertUnsignedArithBinaryOperator(expr, rhs);
    } else {
      PushParen outer(*this);
      {
        PushParen inner(*this);
        StrCat(ConvertRValue(lhs));
        ConvertCast(computation_result_type);
      }
      auto op = opcode_as_string;
      op.remove_suffix(1); // remove '=' from operator
      StrCat(op);
      Convert(rhs);
    }
    if (lhs_type->isBooleanType()) {
      StrCat(token::kDiff, token::kZero);
    } else {
      ConvertCast(lhs_type);
    }
    StrCat(token::kSemiColon);
    EmitSetOrAssign(lhs, "rhs_0");
    return;
  }

  if (IsUnsignedArithOp(expr)) {
    PushBrace brace(*this, expr->isCompoundAssignmentOp());
    if (expr->isCompoundAssignmentOp()) {
      StrCat(keyword::kLet, "rhs_0", token::kAssign);
    }
    {
      PushParen paren(*this);
      if (expr->isCompoundAssignmentOp() && lhs->isLValue()) {
        StrCat(ConvertRValue(lhs));
      } else {
        ConvertUnsignedArithOperand(lhs, expr->getType());
      }
    }
    ConvertUnsignedArithBinaryOperator(expr, rhs);
    if (expr->isCompoundAssignmentOp()) {
      StrCat(token::kSemiColon);
      EmitSetOrAssign(lhs, "rhs_0");
    } else {
      computed_expr_type_ = ComputedExprType::FreshValue;
    }
    return;
  }

  // pointer subtraction. The Sub trait gets elements by Value, so we need
  // fresh pointers
  if (expr->isAdditiveOp() && lhs_type->isPointerType() &&
      rhs_type->isPointerType()) {
    {
      PushParen paren(*this);
      StrCat(ConvertFreshPointer(lhs), expr->getOpcodeStr(),
             ConvertFreshPointer(rhs));
    }
    ConvertCast(expr->getType());
    computed_expr_type_ = ComputedExprType::FreshValue;
    return;
  }

  if (expr->isAssignmentOp()) {
    ConvertAssignment(lhs, rhs, opcode_as_string);
    return;
  }

  Converter::ConvertBinaryOperator(expr);
}

bool ConverterRefCount::VisitInitListExpr(clang::InitListExpr *expr) {
  auto *syntactic = expr->isSyntacticForm() ? expr : expr->getSyntacticForm();
  if (auto form = expr->getSemanticForm())
    expr = form;

  auto qual_type = expr->getType();
  if (qual_type->isScalarType()) {
    PushConversionKind push(*this, ConversionKind::Unboxed);
    Converter::VisitInitListExpr(expr);
    computed_expr_type_ = ComputedExprType::FreshValue;
    return false;
  }

  if (qual_type->isRecordType()) {
    const auto *record = qual_type->getAsRecordDecl();
    if (record->getQualifiedNameAsString() == "std::array") {
      if (auto init = clang::dyn_cast<clang::InitListExpr>(expr->getInit(0))) {
        StrCat("vec!");
        PushConversionKind push(*this, ConversionKind::Unboxed);
        ConverterRefCount::VisitInitListExpr(init);
      } else {
        StrCat(GetArrayDefaultAsString(qual_type));
      }
      computed_expr_type_ = ComputedExprType::FreshValue;
      return false;
    }

    if (syntactic->getNumInits() == 0) {
      {
        PushConversionKind push(*this, ConversionKind::Unboxed);
        StrCat(GetDefaultAsString(qual_type));
      }
      computed_expr_type_ = ComputedExprType::FreshValue;
      return false;
    }

    StrCat(GetUnsafeTypeAsString(qual_type));
    {
      PushBrace brace(*this);
      unsigned i = 0;
      PushConversionKind push(*this, ConversionKind::FullRefCount);
      for (const auto *field : record->fields()) {
        StrCat(GetNamedDeclAsString(field), token::kColon);
        if (i < expr->getNumInits()) {
          ConvertVarInit(field->getType(), expr->getInit(i++));
        } else {
          StrCat(GetDefaultAsString(field->getType()));
        }
        StrCat(token::kComma);
      }
    }
    computed_expr_type_ = ComputedExprType::FreshValue;
    return false;
  }

  if (IsInitExprOfStringLiteral(expr)) {
    Convert(expr->getInit(0)->IgnoreParenImpCasts());
    computed_expr_type_ = ComputedExprType::FreshValue;
    return false;
  }

  auto conv = getConversionKind();
  // 2D arrays are FullRefCount'ed on the second level as well.
  PushConversionKind push(
      *this, ConversionKind::Unboxed,
      !(expr->getNumInits() > 0 && expr->getInit(0)->getType()->isArrayType()));

  switch (conv) {
  case ConversionKind::Unboxed:
  case ConversionKind::Ptr:
    Converter::VisitInitListExpr(expr);
    break;
  case ConversionKind::Pointee:
  case ConversionKind::FullRefCount:
    StrCat("Box::new(");
    Converter::VisitInitListExpr(expr);
    StrCat(')');
    break;
  }
  computed_expr_type_ = ComputedExprType::FreshValue;
  return false;
}

bool ConverterRefCount::VisitCXXStdInitializerListExpr(
    clang::CXXStdInitializerListExpr *expr) {
  PushConversionKind push(*this, ConversionKind::Unboxed);
  return Converter::VisitCXXStdInitializerListExpr(expr);
}

void ConverterRefCount::ConvertUnionMemberAccessor(clang::MemberExpr *expr) {
  auto member = expr->getMemberDecl();
  std::string str;
  {
    Buffer buf(*this);
    PushExprKind push(*this, isLValue() ? ExprKind::LValue : ExprKind::RValue);
    Converter::ConvertMemberExpr(expr);
    str = std::move(buf).str();
  }
  str += "()";

  if (isAddrOf()) {
    if (member->getType()->isArrayType()) {
      PushConversionKind push(*this, ConversionKind::Unboxed);
      StrCat(std::format(
          "{}.reinterpret_cast::<{}>()", str,
          ToString(
              member->getType()->getAsArrayTypeUnsafe()->getElementType())));
      computed_expr_type_ = ComputedExprType::FreshPointer;
    } else {
      StrCat(str);
      computed_expr_type_ = ComputedExprType::Pointer;
    }
    return;
  }

  if (isLValue()) {
    pending_deref_.set(str, /*fresh=*/true);
    return;
  }
  StrCat(DerefPtrExpr(str, member->getType()));
  SetValueFreshness(member->getType());
}

bool ConverterRefCount::VisitMemberExpr(clang::MemberExpr *expr) {
  auto *member = expr->getMemberDecl();
  bool known = Mapper::Contains(expr);

  if (auto *method = clang::dyn_cast<clang::CXXMethodDecl>(member);
      method && !known) {
    if (IsMethodOnPtr(method)) {
      SetUFCSReceiver(expr->getBase(), expr->isArrow(), method);
      StrCat(TraitName(method->getParent()), token::kDoubleColon,
             GetMethodName(method));
      SetFreshType(expr->getType());
      return false;
    }
    // User-defined types have Value<T> fields; the struct itself is read-only
    // and only needs an immutable borrow. Non-user-defined types (STL)
    // need a mutable borrow for non-const methods
    auto base_type = expr->getBase()->getType().getNonReferenceType();
    if (base_type->isPointerType()) {
      base_type = base_type->getPointeeType();
    }
    bool needs_mut = NeedsMutAccess(method, base_type);
    PushExprKind push(*this, needs_mut ? ExprKind::LValue : ExprKind::RValue);
    Converter::ConvertMemberExpr(expr);
    SetFreshType(expr->getType());
    return false;
  }

  if (auto *parent =
          clang::dyn_cast<clang::RecordDecl>(member->getDeclContext());
      parent && parent->isUnion() && clang::isa<clang::FieldDecl>(member)) {
    ConvertUnionMemberAccessor(expr);
    return false;
  }

  std::string str;
  if (known) {
    str = GetMappedAsString(expr);
  } else {
    Buffer buf(*this);
    PushExprKind push(*this, ExprKind::RValue);
    Converter::ConvertMemberExpr(expr);
    str = std::move(buf).str();
  }

  if (isAddrOf()) {
    StrCat(str);
    if (member->getType()->isReferenceType()) {
      computed_expr_type_ = ComputedExprType::Pointer;
    } else {
      StrCat(".as_pointer()");
      computed_expr_type_ = ComputedExprType::FreshPointer;
    }
    return false;
  }

  if (member->getType()->isReferenceType()) {
    if (isLValue()) {
      pending_deref_.set(str, /*fresh=*/false);
      return false;
    }
    StrCat(DerefPtrExpr(str, member->getType().getNonReferenceType()));
  } else if (isRValue()) {
    StrCat(std::format("(*{}.borrow())", std::move(str)));
  } else {
    StrCat(std::format("(*{}.borrow_mut())", std::move(str)));
  }
  SetValueFreshness(expr->getType());
  return false;
}

bool ConverterRefCount::VisitCXXNewExpr(clang::CXXNewExpr *expr) {
  if (expr->isArray()) {
    if (auto *init = llvm::dyn_cast_or_null<clang::InitListExpr>(
            expr->getInitializer())) {
      StrCat("Ptr::alloc_array(");
      Convert(init);
      StrCat(')');
    } else {
      auto array_size_as_string = ToString(*expr->getArraySize());
      auto alloc_type = expr->getAllocatedType();
      PushConversionKind push(*this, ConversionKind::Unboxed);
      auto alloc_type_as_string = ToString(alloc_type);
      auto default_alloc_type_as_string = GetDefaultAsString(alloc_type);

      StrCat(std::format(
          "Ptr::alloc_array((0..{}).map(|_| {}).collect::<Box<[{}]>>())",
          array_size_as_string, default_alloc_type_as_string,
          alloc_type_as_string));
    }
  } else {
    StrCat("Ptr::alloc(");
    if (expr->getInitializer() == nullptr) {
      StrCat("Default::default()");
    } else {
      Convert(expr->getInitializer());
    }
    StrCat(')');
  }
  computed_expr_type_ = ComputedExprType::FreshPointer;
  return false;
}

bool ConverterRefCount::VisitCXXDeleteExpr(clang::CXXDeleteExpr *expr) {
  if (!TypeNeedsDestruction(expr->getDestroyedType())) {
    Convert(expr->getArgument());
    StrCat(".delete()");
    return false;
  }

  PushBrace brace(*this);
  StrCat(keyword::kLet, "__p", token::kAssign, ToString(expr->getArgument()),
         token::kSemiColon);
  if (expr->isArrayForm()) {
    StrCat(std::format("for __i in 0..__p.len() {{ __p.offset(__i as "
                       "isize).{}(); }}",
                       kDestructorName));
    StrCat("__p.delete();");
  } else {
    StrCat(std::format("__p.{}();", kDestructorName));
    StrCat("__p.delete();");
  }
  return false;
}

void ConverterRefCount::EmitByValueShadow(const std::string &loop_var_name,
                                          clang::QualType type,
                                          std::string box_expr,
                                          const std::string &type_override) {
  if (!type->isReferenceType()) {
    PushConversionKind push(*this, ConversionKind::FullRefCount);
    auto type_str = type_override.empty() ? ToString(type) : type_override;
    StrCat(keyword::kLet, loop_var_name, token::kColon, type_str,
           token::kAssign);
    StrCat(BoxValue(std::move(box_expr)), token::kSemiColon);
  }
}

// ⭐ THE RECEIVER for the STANDALONE `const auto &[k, v] = *it;` map arm under
// refcount (row g3036, witness dsc/dims.cpp:731 inside
// `DataStructDims::pruneMaxSymbolicVolumes`).
//
// MEASURED (pin/cpp2rust 12820a5ddbba194f7a13fc585167e5c2 + pin/ir.v44,
// `--model=refcount`) on a probe holding a `std::map<std::set<int>, int>`
// iterator in a local: this model BOXES the iterator local, and derefs it at
// every use --
//     let it: Value<RefcountMapIter<std::collections::BTreeSet<i32>, i32>> =
//         Rc::new(RefCell::new(RefcountMapIter::begin(..)));
//     ... (*it.borrow()).first() ... (*it.borrow_mut()).prefix_inc() ...
// `first()` / `second()` are `&self` methods on the ITERATOR, not on
// `Rc<RefCell<_>>`, so the base's bare `<it>.first()` would be `E0599` at rc=0.
// `.borrow()` (shared) and not `.borrow_mut()` is what those two `&self`
// signatures ask for, and it is what the converter already emits for every other
// const method call on a boxed local.
//
// ⛔ AND THE GUARD IS NOT VACUOUS, which the base's own gates cannot supply. The
// caller admits any iterator in the map-iterator CLASS list whose `value_type`
// is `std::pair<const K, V>` -- and under libc++ a `std::multimap` iterator is
// the SAME class `std::__map_iterator` with the SAME `pair<const K, V>`
// `value_type`, so it passes every one of those gates. There is no `rules/multimap`
// (the rules tree carries `map` and `unordered_map` only), so without this test
// a multimap witness would emit `first()` / `second()` on a type with no
// `MapIterator` impl. Asking the MODEL what the iterator maps to, and requiring
// it to name one of the two refcount iterators that carry the impl
// (`RefcountMapIter`, iterators.rs:151; `RefcountHashMapIter`, :413), is what
// makes the arm's domain exactly the shapes libcc2rs can serve.
//
// ⭐ NOTE WHAT IS *NOT* NEEDED: the iterator TYPE is never named in the emitted
// text, only the existing local is, so unlike
// `RefCountMapRangeIteratorName` this arm does not have to choose between
// `RefcountMapIter` and `RefcountHashMapIter` -- both implement the same trait
// with the same `Value<K>` / `Value<V>` associated types, so one emission covers
// `std::map` and `std::unordered_map` alike.
std::string ConverterRefCount::DecompositionMapIterReceiver(
    clang::DeclRefExpr *iter_ref) {
  auto iter_type =
      iter_ref->getType().getNonReferenceType().getUnqualifiedType();
  if (!Mapper::Contains(iter_type)) {
    return {};
  }
  const std::string model = Mapper::Map(iter_type);
  // `find` rather than a prefix test: the mapped spelling is the iterator type
  // itself here, but a model that wrapped it would still be served by the same
  // accessors, and an over-tight test would silently turn this arm back off.
  if (model.find("RefcountMapIter") == std::string::npos &&
      model.find("RefcountHashMapIter") == std::string::npos) {
    return {};
  }
  return "(*" + GetNamedDeclAsString(iter_ref->getDecl()) + ".borrow())";
}

// The REFCOUNT map-range iterator whose `MapIterator` impl a decomposing loop
// can address, or nullptr for a class this model cannot spell one for.
//
// MEASURED out of libcc2rs/src/iterators.rs: `impl MapIterator for
// RefcountMapIter<K, V>` (:151) and `impl MapIterator for
// RefcountHashMapIter<K, V>` (:413) both give `first() -> Value<K>` and
// `second() -> Value<V>`, i.e. the EXACT two accessors the unsafe decomposition
// lowering uses, already in this model's native local form. `llvm::DenseMap` is
// deliberately absent: rules/densemap's refcount t4 is `RefcountHashMapIter`
// too, but the base VisitCXXForRangeStmt (converter.cpp:3926) already refuses a
// decomposing DenseMap range under refcount BEFORE dispatch reaches here, so
// listing it would be dead code that only looks like coverage.
static const char *RefCountMapRangeIteratorName(const std::string &class_name) {
  if (class_name == "std::map") {
    return "RefcountMapIter";
  }
  if (class_name == "std::unordered_map") {
    return "RefcountHashMapIter";
  }
  return nullptr;
}

bool ConverterRefCount::VisitCXXForRangeStmtMap(clang::CXXForRangeStmt *stmt) {
  auto *loop_var = stmt->getLoopVariable();
  auto *decomp = llvm::dyn_cast<clang::DecompositionDecl>(loop_var);
  // A DecompositionDecl has no name of its own, so the iterator the loop binds
  // needs a synthetic one -- the SAME helper the unsafe map path uses
  // (converter.cpp:3812), so two nested decomposing loops cannot collide.
  // Falling through to GetNamedDeclAsString for a decomposition reached the
  // generic namer and died with
  // `report_fatal_error("Unexpected unnamed construct")`
  // (converter_lib.cpp:847) -- a refusal presenting as an internal error.
  const std::string loop_var_name =
      decomp ? GetDecompositionIterName(decomp)
             : GetNamedDeclAsString(loop_var);
  // ⭐ THE DECOMPOSING ARM. It used to be an unconditional
  // ReportUnsupportedStructuredBinding, on the stated grounds that the unsafe
  // path's bindings are RAW POINTERS this model has no equivalent for. That
  // premise is wrong in one specific, measured way: the unsafe path does not
  // build pointers itself, it calls `<iter>.first()` / `<iter>.second()` on the
  // MODELLED ITERATOR and the POINTER-ness comes from
  // `impl MapIterator for UnsafeMapIterator`. The refcount iterator implements
  // the same trait returning `Value<K>` / `Value<V>` -- which IS the refcount
  // form of a local -- so the identical emission is correct here and needs
  // nothing new in libcc2rs and no new rule key.
  //
  // ⛔ AND NO `ptr_bindings_` REGISTRATION, which is the whole difference from
  // the unsafe arm. There the bindings are `*const K` / `*mut V` and
  // VisitDeclRefExpr must deref at every use; here they are already `Value<..>`,
  // so registering them would emit `(*k)` for a non-pointer -- E0614 at rc=0,
  // i.e. exactly the silent-wrongness class this project refuses.
  //
  // ALIASING, which is what `auto &[k, v]` asks for: `second()` hands back
  // `m.get(key).clone()` on a `BTreeMap<K, Value<V>>`, and cloning an
  // `Rc<RefCell<V>>` SHARES the storage, so a write through `v` reaches the map
  // exactly as the C++ reference does. `first()` hands back a fresh
  // `Rc::new(RefCell::new(key.clone()))`, i.e. a COPY -- sound because a map
  // key is `const` in C++ (`value_type` is `pair<const K, V>`) and no
  // well-formed program writes through `k`.
  if (decomp != nullptr) {
    const char *const iter_type =
        RefCountMapRangeIteratorName(GetClassName(stmt->getRangeInit()->getType()));
    // Any range class this model cannot name a `MapIterator` for, and any arity
    // other than two (EmitMapDecompositionBindings' accessor table has exactly
    // two entries), stays on the LOUD refusal path naming the shape. Gate
    // BEFORE emitting anything so a refused shape leaves no partial text.
    if (iter_type == nullptr || decomp->bindings().size() != 2) {
      ReportUnsupportedStructuredBinding(decomp);
      return false;
    }
    StrCat("'loop_:");
    StrCat(keyword::kFor, loop_var_name, keyword::kIn,
           std::string(iter_type) + "::begin(",
           ConvertObject(stmt->getRangeInit()), ')');
    PushBrace brace(*this);
    // No EmitByValueShadow: that shadows a NAMED loop variable with a boxed
    // `Value<pair>`; a decomposition has no name to shadow and its two bindings
    // are boxed individually by the accessors below.
    if (!EmitMapDecompositionBindings(decomp, loop_var_name)) {
      ReportUnsupportedStructuredBinding(decomp);
      return false;
    }
    ConvertForRangeBody(stmt, loop_var);
    return false;
  }

  // ⭐⭐ ROW g3006 CORRECTS THE COMMENT THAT USED TO STAND HERE. It said the
  // hardcoded `RefcountMapIter` was correct because "a non-decomposing
  // unordered_map range NEVER ARRIVES HERE", and that routing it was "a
  // separate, larger row, because the loop variable's Rust type has to become
  // the `std::pair<const K, V> &` the C++ program sees". That second claim is
  // REFUTED by measurement: the map path does NOT need a pair-shaped value,
  // because it registers the loop variable in `map_iter_decls_` and every
  // `kv.first` / `kv.second` in the body is rewritten to the modelled iterator's
  // `first()` / `second()` accessors. MEASURED 2026-09-29 (snap/coord44,
  // pin/ir.v42) on a probe using BOTH members plus a write through `kv.second`,
  // over a `std::map<int,int> &`: `unsafe: MATCH 11 11` and
  // `refcount: MATCH 11 11` -- the arm below is correct end to end, aliasing
  // included. So `std::unordered_map` is now routed here too
  // (`Converter::VisitCXXForRangeStmt`), and the ONLY thing that had to change
  // is the iterator SPELLING: unordered_map's refcount iterator is
  // `RefcountHashMapIter` (iterators.rs:413), not `RefcountMapIter` (:151).
  // ⭐ THE NON-DECOMPOSING ARM. The iterator type used to be the hardcoded
  // string "RefcountMapIter", which was correct only because `std::map` was the
  // only class the dispatch let in. ROW g3006 routes a non-decomposing
  // `std::unordered_map` range here too (converter.cpp:VisitCXXForRangeStmt), and
  // that model's iterator is `RefcountHashMapIter` -- naming `RefcountMapIter`
  // for a `HashMap` would be `E0308`/`E0599` at rustc, i.e. the same silent class
  // this row exists to close. Ask the SAME helper the decomposing arm asks.
  const char *const nd_iter_type =
      RefCountMapRangeIteratorName(GetClassName(stmt->getRangeInit()->getType()));
  // ⛔ LOUD REFUSAL, GATED BEFORE ANY EMISSION so a refused shape leaves no
  // partial text (the pattern c151701d established).
  //
  // Two shapes are refused:
  //  * a map-like class this model cannot name a `MapIterator` for (today only
  //    `llvm::DenseMap`, which the dispatch does not route here -- the gate is
  //    the guarantee, not dead code, because the dispatch and this spelling are
  //    two files apart);
  //  * a BY-VALUE loop variable over a class OTHER than `std::map`.
  //    ⚠️ MEASURED, and the reason for that exclusion: `EmitByValueShadow`
  //    fires only for a non-reference loop variable and emits
  //    `let kv: Value<pair<..>> = Rc::new(RefCell::new(kv));` where `kv` is the
  //    ITERATOR, not a pair -- an `E0308` that is PRE-EXISTING for `std::map`
  //    (`for (auto kv : some_map)`) and is NOT this row's to change: refusing it
  //    here would move existing `std::map` sites from rc=0 to abort, which is a
  //    widening, not a fix. It is recorded as its own row instead. For the class
  //    this row NEWLY routes, the same shape is refused rather than emitted
  //    wrong.
  if (nd_iter_type == nullptr ||
      (!loop_var->getType()->isReferenceType() &&
       GetClassName(stmt->getRangeInit()->getType()) != "std::map")) {
    const std::string loc =
        loop_var->getLocation().printToString(ctx_.getSourceManager());
    std::string detail =
        "non-decomposing range-`for` loop variable `" + loop_var_name +
        "` of type `" + Mapper::ToString(loop_var->getType()) +
        "` over map-like range `" +
        GetClassName(stmt->getRangeInit()->getType()) +
        "` is not implemented in the refcount model";
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

  StrCat("'loop_:");
  // ⭐ ConvertFreshObject, NOT ConvertObject (slot-mapiter, on this arm).
  // `RefcountMapIter<K, V>` is `MapIter<K, Ptr<BTreeMap<K, Value<V>>>>` and
  // `MapIter::begin` takes its `MapRef` BY VALUE (libcc2rs/src/iterators.rs:75);
  // `Ptr<T>` is `Clone` but deliberately NOT `Copy` (rc.rs:180), so handing it a
  // PLACE moves it and two loops over one `std::map &` gave
  // `error[E0382]: use of moved value` at rc=0. ConvertFreshObject is
  // `ConvertObject(expr, ObjectShape::Whole)` plus `.clone()` only when the
  // result is not already fresh, and `Ptr::clone` clones the HANDLE, never the
  // map, so aliasing is unchanged. The same reasoning applies verbatim to
  // `RefcountHashMapIter`, which is the same `MapIter` shape over a `HashMap`.
  StrCat(keyword::kFor, loop_var_name, keyword::kIn,
         std::string(nd_iter_type) + "::begin(",
         ConvertFreshObject(stmt->getRangeInit()), ')');
  PushBrace brace(*this);

  EmitByValueShadow(
      loop_var_name, loop_var->getType(), std::string(loop_var_name),
      "Value<" + Mapper::Map(GetForRangeIteratorType(stmt)) + '>');

  ConvertForRangeBody(stmt, loop_var);

  return false;
}

// If `call` is `std::move(b)` / `std::forward<..>(b)` for some BindingDecl `b`
// of `decomp`, return that binding; otherwise nullptr.
//
// ⚠️ The name test is deliberately namespace-blind and therefore OVER-broad: a
// user function called `move` taking one argument matches too. Over-broad is the
// safe direction here -- it can only route a site into the narrower, take-based
// lowering below or into the refusal, never into the sharing lowering that is the
// silent-wrong one.
static const clang::BindingDecl *
MovedBindingOfCall(const clang::CallExpr *call,
                   const clang::DecompositionDecl *decomp) {
  const auto *fn = call->getDirectCallee();
  if (fn == nullptr || call->getNumArgs() != 1 ||
      (fn->getNameAsString() != "move" &&
       fn->getNameAsString() != "forward")) {
    return nullptr;
  }
  const auto *arg = call->getArg(0)->IgnoreParenImpCasts();
  const auto *ref = llvm::dyn_cast<clang::DeclRefExpr>(arg);
  if (ref == nullptr) {
    return nullptr;
  }
  for (const auto *binding : decomp->bindings()) {
    if (ref->getDecl() == binding) {
      return binding;
    }
  }
  return nullptr;
}

// Every BindingDecl of `decomp` that is moved from anywhere inside `stmt`.
static void
CollectMovedBindings(const clang::Stmt *stmt,
                     const clang::DecompositionDecl *decomp,
                     llvm::SmallPtrSetImpl<const clang::BindingDecl *> &out) {
  if (stmt == nullptr) {
    return;
  }
  if (const auto *call = llvm::dyn_cast<clang::CallExpr>(stmt)) {
    if (const auto *binding = MovedBindingOfCall(call, decomp)) {
      out.insert(binding);
    }
  }
  for (const clang::Stmt *child : stmt->children()) {
    CollectMovedBindings(child, decomp, out);
  }
}

// How many times does `stmt` mention `decl`?
static unsigned CountDeclRefs(const clang::Stmt *stmt,
                              const clang::ValueDecl *decl) {
  if (stmt == nullptr) {
    return 0;
  }
  unsigned count = 0;
  if (const auto *ref = llvm::dyn_cast<clang::DeclRefExpr>(stmt)) {
    if (ref->getDecl() == decl) {
      ++count;
    }
  }
  for (const clang::Stmt *child : stmt->children()) {
    count += CountDeclRefs(child, decl);
  }
  return count;
}

// Is the range of this loop a TEMPORARY -- i.e. did the loop's `auto&& __range`
// bind a prvalue that nothing outside the loop can name?
//
// ⚠️ The prvalue-ness is NOT on the expression `getRangeInit()` returns: binding
// `auto&& __range = f()` wraps the call in a MaterializeTemporaryExpr (itself
// often inside an ExprWithCleanups), and the WRAPPER is an xvalue. So peel to the
// materialized subexpression and ask there; asking the wrapper returns false for
// every temporary range and the narrowing below would never fire.
static bool IsTemporaryRangeInit(const clang::Expr *init) {
  if (init == nullptr) {
    return false;
  }
  const clang::Expr *expr = init->IgnoreParens();
  while (true) {
    if (const auto *cleanups = llvm::dyn_cast<clang::ExprWithCleanups>(expr)) {
      expr = cleanups->getSubExpr()->IgnoreParens();
      continue;
    }
    if (const auto *cast = llvm::dyn_cast<clang::ImplicitCastExpr>(expr)) {
      if (cast->getCastKind() == clang::CK_NoOp) {
        expr = cast->getSubExpr()->IgnoreParens();
        continue;
      }
    }
    break;
  }
  if (const auto *materialized =
          llvm::dyn_cast<clang::MaterializeTemporaryExpr>(expr)) {
    return materialized->getSubExpr()->IgnoreParenImpCasts()->isPRValue();
  }
  return expr->isPRValue();
}

// Can `RefCell::take()` stand for the C++ move-out of an object of this type?
//
// `take()` is `replace(Default::default())`, so it needs a Rust `Default`, and
// the C++ side of that is a usable default constructor -- `std::deque<int64_t>`
// and `std::vector<int64_t>` (both `Vec<i64>` here) have one. ⛔ Raw pointers and
// fixed arrays are refused even though C++ default-initialises them: Rust has no
// `Default` for `*const T`, and the array `Default` impls stop at 32 elements, so
// emitting `take()` there would be a rustc error rather than a lowering. Anything
// this returns false for keeps the LOUD refusal.
static bool IsTakeableMovedOutType(clang::QualType type) {
  const clang::QualType stripped =
      type.getNonReferenceType().getUnqualifiedType();
  if (stripped->isPointerType() || stripped->isMemberPointerType() ||
      stripped->isArrayType() || stripped->isFunctionType()) {
    return false;
  }
  if (stripped->isScalarType()) {
    return true;
  }
  if (const auto *record = stripped->getAsCXXRecordDecl()) {
    return record->getDefinition() != nullptr && record->hasDefaultConstructor();
  }
  return false;
}

// Split `text` on TOP-LEVEL commas. ⛔ Depth is counted over `<>`, `()` AND `[]`
// together: a same-depth-comma bug in exactly this kind of scan is the recorded
// "swallow" class, and a Rust tuple element can be `Ptr<HashMap<K, V>>` or
// `[i8; 4]`, both of which contain commas that are NOT separators.
static std::vector<std::string> SplitTopLevelCommas(std::string_view text) {
  std::vector<std::string> parts;
  int depth = 0;
  size_t start = 0;
  for (size_t i = 0; i < text.size(); ++i) {
    const char c = text[i];
    if (c == '<' || c == '(' || c == '[') {
      ++depth;
    } else if (c == '>' || c == ')' || c == ']') {
      --depth;
    } else if (c == ',' && depth == 0) {
      parts.emplace_back(text.substr(start, i - start));
      start = i + 1;
    }
  }
  parts.emplace_back(text.substr(start));
  for (auto &part : parts) {
    while (!part.empty() && part.front() == ' ') {
      part.erase(part.begin());
    }
    while (!part.empty() && part.back() == ' ') {
      part.pop_back();
    }
  }
  return parts;
}

// A DECOMPOSING range-for over an indexable container, in THIS model.
//
// ⭐ THE SHAPE, and it is NOT the index-based one the base class emits. This
// model's vector range-for is an ITERATOR loop whose variable is a
// `Ptr<element>` (measured on a hand probe, refcount, `Ptr<Vec<(Value<Vec<i64>>,
// Value<Vec<i64>>)>>`):
//     'loop_: for mut e in v.decay() as Ptr<(Value<Vec<i64>>, Value<Vec<i64>>)>
// so the element fields are reached as `(*e.upgrade().deref()).N`, and a
// `.clone()` of such a field is an `Rc` CLONE THAT SHARES THE ELEMENT'S OWN
// `RefCell`. That is what makes a MUTABLE `auto& [a, b]` binding sound here
// without any pointer form: `a` and the pair's `.0` are two `Rc`s to ONE cell, so
// `borrow_mut()` through either reaches the same storage and a write travels back
// into the container exactly as the C++ requires.
//
// ⛔ AND THEREFORE NO `ptr_bindings_` REGISTRATION. A binding emitted here is a
// `Value<T>`, not a pointer; registering it would make `VisitDeclRefExpr` spell
// every use `(*a)`, which is a hard rustc error on a non-pointer.
bool ConverterRefCount::VisitCXXForRangeStmtVectorDecomposition(
    clang::CXXForRangeStmt *stmt, clang::DecompositionDecl *decomp) {
  auto bindings = decomp->bindings();
  auto *range_init = stmt->getRangeInit();
  auto elem_type = decomp->getType().getNonReferenceType().getUnqualifiedType();

  // ================= EVERY GATE IS BEFORE ANY EMISSION =================
  // (1) The element spelling. `.0`/`.1` only mean the C++ elements for a type
  // modelled as a Rust tuple, and only `std::pair` is established here; a
  // member-wise struct would need field NAMES (E0609 otherwise) and has no
  // measured witness under this model.
  if (bindings.size() != 2 || GetClassName(elem_type) != "std::pair") {
    ReportUnsupportedStructuredBinding(decomp);
    return false;
  }
  // (2) BOTH tuple components must be `Value<..>` in this model -- that is the
  // whole aliasing argument above. A component that is a bare value (a POD
  // field, say) would be COPIED by `.clone()` and a write through the binding
  // would be silently dropped; a component that is a raw pointer model
  // (`Value<*const T>`) has the recorded `no method named upgrade` defect. The
  // element text is taken from the very same `ConvertPtrType` the `as` cast
  // below emits, so the check cannot drift from what is emitted.
  const std::string ptr_type = ConvertPtrType(range_init->getType());
  std::string elem_text;
  if (ptr_type.starts_with("Ptr<") && ptr_type.ends_with(">")) {
    elem_text = ptr_type.substr(4, ptr_type.size() - 5);
  }
  if (elem_text.size() < 2 || elem_text.front() != '(' ||
      elem_text.back() != ')') {
    ReportUnsupportedStructuredBinding(decomp);
    return false;
  }
  const auto components = SplitTopLevelCommas(
      std::string_view(elem_text).substr(1, elem_text.size() - 2));
  if (components.size() != bindings.size()) {
    ReportUnsupportedStructuredBinding(decomp);
    return false;
  }
  for (const auto &component : components) {
    if (!component.starts_with("Value<")) {
      ReportUnsupportedStructuredBinding(decomp);
      return false;
    }
  }
  // (3) THE BY-VALUE HOLDER. `for (auto [a, b] : v)` COPIES the element in C++,
  // so a write through `a` must NOT reach the container -- and `.clone()` of a
  // `Value<..>` is an `Rc` clone that SHARES the cell, so cloning is exactly
  // wrong here. The holder is therefore NOT refused; it gets a DIFFERENT
  // spelling, `Rc::new(RefCell::new(elem.borrow().clone()))`, which mints a
  // FRESH cell holding a clone of the value -- a genuine copy, so the write
  // lands in the binding's own cell and the container is untouched.
  //
  // ⭐ MEASURED, and it corrects the reason the earlier refusal gave ("this model
  // has no established deep-copy spelling for an arbitrary element"): it has
  // two, and they agree. `rules/pair`'s refcount `f2` -- the pair COPY
  // CONSTRUCTOR -- is literally `(Rc::new(RefCell::new(a0.0.borrow().clone())),
  // Rc::new(RefCell::new(a0.1.borrow().clone())))`, i.e. the per-component deep
  // copy emitted below, and `VisitCXXForRangeStmtVector`'s own by-value arm for
  // a `Value<..>` element (the `IsBoxedType` branch) emits
  // `Rc::new(RefCell::new(x.borrow().clone()))` for the same reason. So this is
  // the model's ESTABLISHED by-value spelling, reused, not a new invention.
  //
  // ⛔ AND A NARROWING I WAS HANDED AND MEASURED TO BE VACUOUS, recorded so it is
  // not proposed a third time: "allow the by-value holder iff no component is
  // `Value<`-prefixed, because then `.clone()` is a genuine copy". For
  // `std::pair` NO component is ever non-`Value<`: `rules/pair`'s refcount `t1`
  // is `(Value<T1>, Value<T2>)` unconditionally, and the emitted element type
  // for the measured witness `std::vector<std::pair<PrimaryDimTypes, int>>` is
  // `Ptr<(Value<PDT>, Value<i32>)>` -- an ENUM component and an `int` component
  // are BOTH boxed. Gate (2) ABOVE therefore never fires for a `std::pair`, so
  // that narrowing would have been dead code that only looked like coverage.
  const bool by_value = !decomp->getType()->isReferenceType();
  // (4) `std::move` OUT OF A BINDING. `std::move(data)` is a move of the
  // CONTAINER ELEMENT in C++: the callee gets the value and the element is left
  // in its moved-from state (empty, for every mapped container type here). The
  // model's `.clone()` of the element's `Value<..>` transfers NOTHING -- callee
  // and container share one cell -- so that spelling is silently wrong at rc=0
  // and must never be reached for this sub-shape.
  //
  // ⭐ THE SOUND SPELLING IS `take()`, and the reason the original refusal gave
  // for rejecting it ("it CHANGES OBJECT IDENTITY for every other alias of that
  // cell") IS WRONG: `take()` leaves the SAME `RefCell` in place and only
  // replaces its contents, so another `Rc` to that cell still names the same
  // object and sees it emptied -- which is exactly what a C++ reference to a
  // moved-from `vector`/`deque` sees. `take()` therefore MATCHES C++ rather than
  // diverging from it, and what has to be established is not "no other alias"
  // but the two things below.
  //
  // (4a) THE RANGE MUST BE A TEMPORARY. For a named container C++ `std::move`
  // does not guarantee a move at all -- it only casts to an rvalue reference, and
  // a callee taking `const T&` leaves the element INTACT. Whether the callee
  // consumes is not decidable here (the measured site's callee is
  // `std::forward<Func>(func)`, a template parameter), so emptying the element
  // unconditionally could empty one C++ kept. Over a temporary range that
  // divergence is unobservable: the container is unnamed, dies at the end of the
  // loop, and -- measured in the emitted Rust, `Rc::new(RefCell::new(({ mk_1()
  // })))...` -- its `Rc` is MINTED INLINE IN THE LOOP HEADER, so no other handle
  // to it can exist, and Rust extends that temporary over the whole loop exactly
  // as C++ does.
  //
  // (4b) THE MOVED BINDING MUST BE MENTIONED EXACTLY ONCE IN THE BODY, i.e. only
  // as the move operand. That is what makes hoisting the `take()` to the top of
  // the body (where the binding is emitted) equivalent to taking at the move
  // site: no statement in between can read the cell, so WHEN it is emptied cannot
  // be observed. A second mention would need the take at the move site itself.
  //
  // Anything else -- a named range, a binding read again after the move, or an
  // element type with no usable default -- keeps the LOUD refusal.
  llvm::SmallPtrSet<const clang::BindingDecl *, 2> moved_bindings;
  CollectMovedBindings(stmt->getBody(), decomp, moved_bindings);
  if (!moved_bindings.empty()) {
    // ⛔ `std::move` OUT OF A *BY-VALUE* BINDING MUST STAY REFUSED, and it is not
    // the same question as the reference holder's. There the `take()` empties the
    // CONTAINER element, which is what C++ does. Here C++ moves out of the loop's
    // own COPY and the container is untouched, so `take()` would be the silent
    // rc=0 wrong in the opposite direction -- it would empty an element C++ kept.
    // The deep copy below already gives the callee a cell of its own, so a sound
    // spelling plausibly exists; it is NOT established here and no witness has
    // been measured for the combination, so it keeps the LOUD refusal rather than
    // being guessed at.
    if (by_value) {
      ReportUnsupportedStructuredBinding(decomp);
      return false;
    }
    if (!IsTemporaryRangeInit(range_init)) {
      ReportUnsupportedStructuredBinding(decomp);
      return false;
    }
    for (const auto *binding : moved_bindings) {
      if (CountDeclRefs(stmt->getBody(), binding) != 1 ||
          !IsTakeableMovedOutType(binding->getType())) {
        ReportUnsupportedStructuredBinding(decomp);
        return false;
      }
    }
  }
  // ====================== COMMITTED TO EMITTING ======================
  // A DecompositionDecl has no name of its own, so the holder needs one.
  // Source-location-derived so it cannot collide with a user local or with a
  // second loop in the same scope.
  // The raw encoding of the source location is a stable per-TU offset (no
  // SourceManager needed here, and this file does not have a complete one), so
  // the name is deterministic for a given TU and cannot collide with a user
  // local or with a second loop in the same scope.
  const std::string holder =
      std::format("__elem_{}", decomp->getBeginLoc().getRawEncoding());

  StrCat("'loop_:");
  StrCat(keyword::kFor, "mut", holder, keyword::kIn,
         ConvertObject(range_init, ObjectShape::Element));
  StrCat(keyword::kAs, ptr_type);

  PushBrace brace(*this);
  const char *deref = GetPointerDerefSuffix(elem_type);
  unsigned index = 0;
  for (const auto *binding : bindings) {
    const std::string binding_name = GetNamedDeclAsString(binding);
    // ⛔ `let mut _ = ..` is not legal Rust (`_` is a wildcard PATTERN, not an
    // identifier), and `_` is the conventional C++ spelling for an unused
    // element, so `mut` is conditional on the name.
    StrCat(keyword::kLet);
    if (binding_name != "_") {
      StrCat("mut");
    }
    // ⭐ A MOVED-FROM BINDING IS `take()`n OUT OF THE ELEMENT, NOT CLONED: the
    // callee must get a cell of its own (so it owns the value, as the C++ move
    // gives it) and the container element must be left empty (as the C++ move
    // leaves it). `.clone()` here would do neither -- it shares one cell, which
    // is the silent rc=0 wrong that gate (4) exists to prevent. Gate (4) has
    // already established that the container is an inline temporary and that this
    // binding is mentioned only as the move operand, which is what makes taking
    // HERE rather than at the move site equivalent.
    //
    // ⭐ AND A BY-VALUE HOLDER IS DEEP-COPIED, NOT CLONED. `.clone()` of the
    // element's `Value<..>` is an `Rc` clone that SHARES the cell, which is the
    // ALIASING the reference holder wants and the exact opposite of what `for
    // (auto [a, b] : v)` means. `Rc::new(RefCell::new(elem.borrow().clone()))`
    // mints a fresh cell holding a clone of the value, so a write through the
    // binding stays in the binding -- see gate (3). Gate (4) has refused the
    // by-value + `std::move` combination, so these three arms are disjoint.
    const std::string element =
        std::format("(*{}{}).{}", holder, deref, index);
    StrCat(binding_name, token::kAssign,
           moved_bindings.contains(binding)
               ? std::format("Rc::new(RefCell::new({}.take()))", element)
           : by_value
               ? std::format("Rc::new(RefCell::new({}.borrow().clone()))",
                             element)
               : std::format("{}.clone()", element),
           token::kSemiColon);
    ++index;
  }

  ConvertForRangeBody(stmt);

  return false;
}

bool ConverterRefCount::VisitCXXForRangeStmtVector(
    clang::CXXForRangeStmt *stmt) {
  auto *loop_var = stmt->getLoopVariable();

  // ⛔ THE DISPATCH MUST PRECEDE `GetNamedDeclAsString`, not merely the emission:
  // a DecompositionDecl is UNNAMED, and that helper is fatal ("Unexpected unnamed
  // construct") on it -- measured, as an abort on the very probe this arm exists
  // for. It has no name of its own and needs the element fields bound one by one;
  // everything below assumes a named loop variable.
  if (auto *decomp = llvm::dyn_cast<clang::DecompositionDecl>(loop_var)) {
    return VisitCXXForRangeStmtVectorDecomposition(stmt, decomp);
  }

  auto loop_var_name = GetNamedDeclAsString(loop_var);

  StrCat("'loop_:");
  StrCat(keyword::kFor,
         stmt->getLoopVariable()->getType().isConstQualified() ? "" : "mut",
         loop_var_name, keyword::kIn,
         ConvertObject(stmt->getRangeInit(), ObjectShape::Element));
  StrCat(keyword::kAs, ConvertPtrType(stmt->getRangeInit()->getType()));

  PushBrace brace(*this);

  // handle multi-level types such as Vec<Value<Vec<T>>>
  if (IsBoxedType(stmt->getRangeInit()->getType()) &&
      GetInnerType(stmt->getRangeInit()->getType()).starts_with("Value<")) {
    StrCat(keyword::kLet, loop_var_name, token::kColon);

    if (loop_var->getType()->isReferenceType()) {
      StrCat(ToString(loop_var->getType()), token::kAssign, loop_var_name,
             GetPointerDerefSuffix(loop_var->getType().getNonReferenceType()),
             ".as_pointer()");
    } else {
      PushConversionKind push(*this, ConversionKind::FullRefCount);
      StrCat(ToString(loop_var->getType()), token::kAssign,
             "Rc::new(RefCell::new(", loop_var_name,
             GetPointerDerefSuffix(loop_var->getType()), ".borrow().clone()))");
    }
    StrCat(token::kSemiColon);
  } else {
    auto type = loop_var->getType();
    bool copy = type.isPODType(ctx_) && !type->isRecordType();
    EmitByValueShadow(loop_var_name, type,
                      loop_var_name + GetPointerDerefSuffix(type) +
                          (copy ? "" : ".clone()"));
  }

  ConvertForRangeBody(stmt);

  return false;
}

bool ConverterRefCount::VisitCXXForRangeStmtString(
    clang::CXXForRangeStmt *stmt) {
  auto *loop_var = stmt->getLoopVariable();
  auto loop_var_name = GetNamedDeclAsString(loop_var);

  StrCat("'loop_:");
  StrCat(keyword::kFor,
         stmt->getLoopVariable()->getType().isConstQualified() ? "" : "mut",
         loop_var_name, keyword::kIn,
         ConvertObject(stmt->getRangeInit(), ObjectShape::Element));
  StrCat(".to_string_iterator() as StringIterator<",
         ToString(loop_var->getType().getNonReferenceType()), '>');

  PushBrace brace(*this);

  EmitByValueShadow(loop_var_name, loop_var->getType(),
                    loop_var_name + GetPointerDerefSuffix(loop_var->getType()) +
                        ".clone()");
  ConvertForRangeBody(stmt);

  return false;
}

bool ConverterRefCount::VisitArrayInitLoopExpr(clang::ArrayInitLoopExpr *expr) {
  StrCat("Box::new");
  PushParen outer(*this);
  PushConversionKind push(*this, ConversionKind::Unboxed);
  return Converter::VisitArrayInitLoopExpr(expr);
}

void ConverterRefCount::ConvertArrayCXXConstructExpr(
    clang::CXXConstructExpr *expr) {
  StrCat("Box::new");
  PushParen outer(*this);
  StrCat(std::format("std::array::from_fn::<_, {}, _>",
                     GetArraySize(expr->getType())));
  PushParen inner(*this);
  StrCat("|_|");
  ConvertCXXConstructExprArgs(expr);
}

std::string ConverterRefCount::ConvertStream(clang::Expr *expr) {
  return ConvertPointer(expr);
}

// `Ptr<T: Write>` carries INHERENT `write_fmt`/`write_all` (libcc2rs rc.rs:571)
// rather than implementing `Write`, and it has no `Deref`, so neither
// `Write::flush(&mut p)` nor `p.flush()` resolves. Reach the inner stream with
// the pub `with_mut` instead.
std::string ConverterRefCount::FlushStream(const std::string &stream) {
  return "let _ = (" + stream +
         ").with_mut(|__s| ::std::io::Write::flush(__s));";
}

bool ConverterRefCount::VisitCXXConstructExpr(clang::CXXConstructExpr *expr) {
  PushConversionKind push(*this, ConversionKind::Unboxed);
  PushSuppressIteratorClone push_suppress(*this, expr);

  if (auto str = GetMappedAsString(expr, expr->getArgs(), expr->getNumArgs());
      !str.empty()) {
    if (isAddrOf()) {
      StrCat(std::format("Rc::new(RefCell::new({})).as_pointer()",
                         std::move(str)));
      computed_expr_type_ = ComputedExprType::FreshPointer;
    } else {
      StrCat(str);
      if (!IsPassThroughRule(expr)) {
        computed_expr_type_ = ComputedExprType::FreshValue;
      }
    }
    return false;
  }

  auto *ctor = expr->getConstructor();
  if (IsRValueConvertingConstructor(ctor) ||
      (ctor->isMoveConstructor() && !IsUserDefinedDecl(ctor->getParent()))) {
    StrCat(ConvertLValue(expr->getArg(0)));
    return false;
  }

  if (ctor->isCopyOrMoveConstructor() &&
      !IsConvertibleCopyOrMoveConstructor(ctor)) {
    StrCat(PushSuppressIteratorClone::take(*this)
               ? ConvertRValue(expr->getArg(0))
               : ConvertFreshRValue(expr->getArg(0)));
    return false;
  }

  if (ctor->isDefaultConstructor() && !ctor->isUserProvided()) {
    auto ty = expr->getType();
    StrCat(GetDefaultAsString(ty));
    SetFreshType(ty);
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

bool ConverterRefCount::VisitImplicitValueInitExpr(
    clang::ImplicitValueInitExpr *expr) {
  PushConversionKind push(*this, ConversionKind::Unboxed);
  if (auto arr_ty = clang::dyn_cast<clang::ArrayType>(
          expr->getType()->getCanonicalTypeInternal().getTypePtr())) {
    if (clang::isa<clang::ConstantArrayType>(arr_ty)) {
      StrCat("Box::new(");
      Converter::VisitImplicitValueInitExpr(expr);
      StrCat(')');
      computed_expr_type_ = ComputedExprType::FreshValue;
      return false;
    }
  }

  return Converter::VisitImplicitValueInitExpr(expr);
}

bool ConverterRefCount::VisitCXXScalarValueInitExpr(
    clang::CXXScalarValueInitExpr *expr) {
  PushConversionKind push(*this, ConversionKind::Unboxed);
  return Converter::VisitCXXScalarValueInitExpr(expr);
}

void ConverterRefCount::ConvertVariadicArg(clang::Expr *arg) {
  if (arg->getType()->isPointerType()) {
    StrCat(ConvertFreshPointer(arg));
    return;
  }
  Convert(arg);
}

bool ConverterRefCount::VisitVAArgExpr(clang::VAArgExpr *expr) {
  auto va_list_expr = expr->getSubExpr();
  if (auto *cast = clang::dyn_cast<clang::ImplicitCastExpr>(va_list_expr)) {
    va_list_expr = cast->getSubExpr();
  }
  StrCat(ConvertLValue(va_list_expr));
  StrCat(".arg::<");
  {
    PushConversionKind push(*this, ConversionKind::Unboxed);
    StrCat(ToString(expr->getType()));
  }
  StrCat(">()");
  SetFreshType(expr->getType());
  return false;
}

bool ConverterRefCount::VisitCXXDefaultArgExpr(clang::CXXDefaultArgExpr *expr) {
  return Converter::VisitCXXDefaultArgExpr(expr);
}

std::string
ConverterRefCount::GetArrayDefaultAsString(clang::QualType qual_type) {
  if (auto *array_type = clang::dyn_cast<clang::ConstantArrayType>(qual_type)) {
    const auto &size = array_type->getSize();
    auto size_as_string = GetNumAsString(size);
    auto element_type = array_type->getElementType();
    PushConversionKind push(*this, element_type->isArrayType()
                                       ? ConversionKind::FullRefCount
                                       : ConversionKind::Unboxed);
    auto element_type_as_string = ToString(element_type);
    auto default_as_string = GetDefaultAsString(element_type);
    return std::format("(0..{}).map(|_| {}).collect::<Box<[{}]>>()",
                       size_as_string.c_str(), default_as_string,
                       element_type_as_string);
  }
  return Converter::GetArrayDefaultAsString(qual_type);
}

std::string ConverterRefCount::GetDefaultAsString(clang::QualType qual_type) {
  if (IsVaListType(qual_type)) {
    computed_expr_type_ = ComputedExprType::FreshValue;
    return BoxValue("VaList::default()");
  }

  if (auto arr = GetArrayDefaultAsString(qual_type); !arr.empty()) {
    computed_expr_type_ = ComputedExprType::FreshValue;
    return BoxValue(std::move(arr));
  }

  if (auto init = Mapper::MapInitializer(qual_type); !init.empty()) {
    computed_expr_type_ = ComputedExprType::FreshValue;
    return BoxValue(std::move(init));
  }

  std::string ret;
  if (qual_type->isPointerType()) {
    auto pointee_type = qual_type->getPointeeType();
    if (pointee_type->isFunctionType()) {
      auto *proto = pointee_type->getAs<clang::FunctionProtoType>();
      assert(proto && "Function pointer default without a prototype");
      ret =
          std::format("FnPtr::<{}>::null()", ConvertFunctionPointerType(proto));
    } else {
      if (pointee_type->isVoidType()) {
        ret = "AnyPtr::default()";
      } else {
        PushConversionKind push(*this, ConversionKind::Unboxed);
        ret = std::format("Ptr::<{}>::null()", ConvertPointeeType(qual_type));
      }
    }
  } else {
    return Converter::GetDefaultAsString(qual_type);
  }
  computed_expr_type_ = ComputedExprType::FreshPointer;
  return BoxValue(std::move(ret));
}

std::string
ConverterRefCount::GetDefaultAsStringFallback(clang::QualType qual_type) {
  auto canonical = qual_type.getUnqualifiedType().getCanonicalType();
  if (canonical->isBooleanType() ||
      (canonical->isIntegerType() && !canonical->isEnumeralType()) ||
      canonical->isFloatingType()) {
    std::string unboxed;
    {
      PushConversionKind push(*this, ConversionKind::Unboxed);
      unboxed = Converter::GetDefaultAsStringFallback(qual_type);
    }
    return BoxValue(std::move(unboxed));
  }

  return std::format("<{}>::default()", ToString(qual_type));
}

std::string
ConverterRefCount::ConvertVarDefaultInit(clang::QualType qual_type) {
  PushConversionKind push(*this, ConversionKind::FullRefCount);
  return GetDefaultAsString(qual_type);
}

std::vector<const char *>
ConverterRefCount::GetStructAttributes(const clang::RecordDecl *decl) {
  std::vector<const char *> attrs;

  if (decl->isUnion()) {
    return attrs;
  }

  if (HasDefaultedCopyConstructor(decl) && RecordHasOnlyReferenceFields(decl)) {
    attrs.emplace_back("Clone");
  }

  if (RecordDerivesByteRepr(decl)) {
    attrs.emplace_back("ByteRepr");
  }

  if (RecordDerivesDefault(decl)) {
    attrs.emplace_back("Default");
  }
  return attrs;
}

std::string ConverterRefCount::ConvertVarInitValue(clang::QualType qual_type,
                                                   clang::Expr *expr) {
  if (auto lambda = clang::dyn_cast<clang::LambdaExpr>(
          expr->IgnoreUnlessSpelledInSource())) {
    Buffer buf(*this);
    PushConversionKind push(*this, ConversionKind::Unboxed);
    if (qual_type->isFunctionPointerType() && lambda->capture_size() == 0) {
      StrCat("FnPtr::new(");
      VisitLambdaExpr(lambda);
      StrCat(')');
    } else {
      VisitLambdaExpr(lambda);
    }
    return std::move(buf).str();
  }

  PushInitType init_type(*this, qual_type);
  if (qual_type->isReferenceType() || qual_type->isFunctionPointerType()) {
    if (llvm::isa<clang::MaterializeTemporaryExpr>(expr->IgnoreImpCasts())) {
      return EmitMaterializedTempBinding(qual_type, expr);
    }
    if (qual_type.getNonReferenceType()->isArrayType()) {
      if (IsStringLiteralExpr(expr)) {
        auto code_unit = ToStringBase(
            ctx_.getAsArrayType(
                    expr->IgnoreParens()->IgnoreImplicit()->getType())
                ->getElementType());
        return std::format("Ptr::<{}>::from_string_literal({})", code_unit,
                           ToString(expr->IgnoreParens()->IgnoreImplicit()));
      }
      return std::format("({} as {})", ConvertFreshPointer(expr),
                         ToString(qual_type));
    }
    if (qual_type->isFunctionPointerType()) {
      return ConvertFnPtrValue(qual_type, expr);
    }
    return ConvertFreshPointer(expr);
  }
  return ConvertFreshRValue(expr, qual_type);
}

std::string ConverterRefCount::ConvertFnPtrValue(clang::QualType qual_type,
                                                 clang::Expr *expr) {
  auto base = ConvertFreshPointer(expr);

  // `qual_type` (the expected fn pointer type) and `expr`'s own fn pointer
  // type may differ in spelling only because the translation maps two
  // typedefs of the same C type to distinct Rust types (e.g. size_t vs
  // unsigned long) -- FnPtr::cast handles this (and any other cast)
  // automatically, so just insert it whenever the two differ.
  auto target_proto =
      qual_type->getPointeeType()->getAs<clang::FunctionProtoType>();
  if (!target_proto || !expr->getType()->isFunctionPointerType()) {
    return base;
  }
  auto src_proto =
      expr->getType()->getPointeeType()->getAs<clang::FunctionProtoType>();
  if (!src_proto) {
    return base;
  }
  auto fn_type = ConvertFunctionPointerType(target_proto);
  if (ConvertFunctionPointerType(src_proto) == fn_type) {
    return base;
  }
  return std::format("{}.cast::<{}>()", base, fn_type);
}

void ConverterRefCount::ConvertVarInit(clang::QualType qual_type,
                                       clang::Expr *expr) {
  bool is_ref = qual_type->isReferenceType();
  PushConversionKind push(*this, ConversionKind::Unboxed, is_ref);
  StrCat(BoxValue(ConvertVarInitValue(qual_type, expr)));
}

bool ConverterRefCount::EmitGlobalValueAssign(clang::Expr *lhs,
                                              std::string_view assign_operator,
                                              std::string_view rhs) {
  auto *decl_ref = clang::dyn_cast<clang::DeclRefExpr>(lhs->IgnoreImplicit());
  if (!decl_ref) {
    return false;
  }
  auto *var = clang::dyn_cast<clang::VarDecl>(decl_ref->getDecl());
  if (!var || !IsGlobalVar(var) || var->getType()->isReferenceType()) {
    return false;
  }
  StrCat(std::format("{}.with(|rc| *rc.borrow_mut() {} {})",
                     GetNamedDeclAsString(var), assign_operator, rhs));
  return true;
}

void ConverterRefCount::EmitSetOrAssign(clang::Expr *lhs,
                                        std::string_view rhs) {
  if (EmitGlobalValueAssign(lhs, "=", rhs)) {
    computed_expr_type_ = ComputedExprType::FreshValue;
    return;
  }
  auto lhs_str = ConvertLValue(lhs);
  if (!pending_deref_.empty()) {
    auto ptr = pending_deref_.take();
    StrCat(ptr, ".write(", rhs, ')');
  } else {
    StrCat(lhs_str, token::kAssign, rhs);
  }
  computed_expr_type_ = ComputedExprType::FreshValue;
}

void ConverterRefCount::ConvertAssignment(clang::Expr *lhs, clang::Expr *rhs,
                                          std::string_view assign_operator) {
  auto rhs_as_string = ConvertFreshRValue(rhs, lhs->getType());

  PushBrace brace(*this, isRValue());

  if (MayCauseBorrowMutError(lhs, rhs)) {
    StrCat(keyword::kLet, "__rhs", token::kAssign, rhs_as_string,
           token::kSemiColon);
    rhs_as_string = "__rhs";
  }

  if (assign_operator == "=") {
    EmitSetOrAssign(lhs, rhs_as_string);
  } else if (EmitGlobalValueAssign(lhs, assign_operator, rhs_as_string)) {
    computed_expr_type_ = ComputedExprType::FreshValue;
  } else {
    auto lhs_str = ConvertLValue(lhs);
    if (!pending_deref_.empty()) {
      bool fresh = pending_deref_.is_fresh();
      auto ptr = pending_deref_.take();
      auto op = assign_operator;
      op.remove_suffix(1); // remove '='
      {
        PushBrace brace(*this);
        StrCat(std::format("let _ptr = {}{};", ptr, fresh ? "" : ".clone()"));
        StrCat(std::format("_ptr.write(_ptr.read() {} {})", op, rhs_as_string));
      }
    } else {
      StrCat(lhs_str, assign_operator, rhs_as_string);
    }
    computed_expr_type_ = ComputedExprType::FreshValue;
  }

  if (isRValue()) {
    StrCat(token::kSemiColon, ConvertFreshRValue(lhs));
  }
}

void ConverterRefCount::ConvertGenericBinaryOperator(
    clang::BinaryOperator *expr) {
  auto lhs = expr->getLHS();
  auto rhs = expr->getRHS();
  std::string_view opcode = expr->getOpcodeStr();

  auto lhs_vars = GetAllVars(lhs);
  auto rhs_vars = GetAllVars(rhs);

  auto predicate = [](auto *var) {
    return var->getType()->isPointerType() || var->getType()->isReferenceType();
  };

  auto sides_contains_literal = rhs_vars.empty() || lhs_vars.empty();
  auto same_var_on_both_sides = lhs_vars == rhs_vars;
  auto sides_contain_ptr_or_deref = std::ranges::any_of(rhs_vars, predicate) ||
                                    std::ranges::any_of(lhs_vars, predicate);

  auto both_sides_have_va_arg = same_var_on_both_sides &&
                                ContainsVAArgExpr(lhs) &&
                                ContainsVAArgExpr(rhs);

  auto may_cause_borrow_mut_err =
      both_sides_have_va_arg ||
      (!sides_contains_literal && !same_var_on_both_sides &&
       sides_contain_ptr_or_deref);

  if (may_cause_borrow_mut_err) {
    // ⛔ `_lhs` is a plain local, never a cast, so the operand that can end in a
    // type here is the one spliced after the opcode? No -- it is `_lhs`'s
    // INITIALISER that is safe (it is followed by `;`), and the operand left of
    // `opcode` is `_lhs` itself. The RHS text is the one that can carry a
    // trailing `as T`, and a trailing type is harmless because nothing follows
    // it. So only the base-`Convert` path below can produce the defect, and the
    // guard here is kept for the shape where a future edit puts an expression
    // rather than `_lhs` on the left.
    StrCat(std::format(
        "{{ let _lhs = {}; _lhs {} {} }}",
        ConvertFreshRValue(lhs,
                           GetOperandImplicitConversionTarget(expr, lhs, rhs)),
        opcode,
        ConvertFreshRValue(
            rhs, GetOperandImplicitConversionTarget(expr, rhs, lhs))));
    computed_expr_type_ = ComputedExprType::FreshValue;
    return;
  }

  PushParen outer(*this);
  // ⭐ THIS IS THE g3019 SITE. The BASE Converter::ConvertGenericBinaryOperator
  // wraps each operand in its OWN `PushParen` (converter.cpp), which is why the
  // unsafe model never shows the defect. This refcount override dropped the
  // per-operand parens and keeps only the outer one -- and an outer paren does
  // NOT help, because `( x as usize << 1 )` is still a parse error: the parser
  // is inside the type `usize` when it meets `<<`.
  //
  // ⛔ The fix is NOT to restore the per-operand `PushParen`: that would add two
  // parentheses to EVERY binary operator the refcount model emits and churn
  // every emitted file. It is to parenthesise only the operand that actually
  // ends in a type, only before a `<`-family operator.
  const size_t lhs_start = rs_code_ != nullptr ? rs_code_->size() : 0;
  Convert(lhs, GetOperandImplicitConversionTarget(expr, lhs, rhs));
  ParenthesizeAngleCastOperand(lhs_start, opcode);
  StrCat(opcode);
  Convert(rhs, GetOperandImplicitConversionTarget(expr, rhs, lhs));
  computed_expr_type_ = ComputedExprType::FreshValue;
}

void ConverterRefCount::ConvertUniquePtrDeref(
    clang::CXXOperatorCallExpr *expr) {
  if (isAddrOf()) {
    StrCat(ConvertRValue(expr->getArg(0)), ".as_pointer()");
    computed_expr_type_ = ComputedExprType::FreshPointer;
  } else {
    StrCat(std::format("(*{}.as_ref().unwrap().borrow{}())",
                       ToString(expr->getArg(0)), isRValue() ? "" : "_mut"));
    SetValueFreshness(expr->getType());
  }
}

bool ConverterRefCount::ConvertCXXOperatorCallExpr(
    clang::CXXOperatorCallExpr *expr) {
  switch (expr->getOperator()) {
  case clang::OverloadedOperatorKind::OO_Equal:
    ConvertAssignment(expr->getArg(0), expr->getArg(1), "=");
    break;

  case clang::OverloadedOperatorKind::OO_Arrow:
  case clang::OverloadedOperatorKind::OO_Star:
    if (IsUniquePtr(expr->getArg(0)->getType())) {
      ConvertUniquePtrDeref(expr);
      break;
    }

    if (isLValue()) {
      auto ptr = ToString(expr->getArg(0));
      pending_deref_.set(std::move(ptr), isFresh());
      break;
    }

    if (GetStrongestIteratorCategory(expr->getArg(0)->getType()) ==
        IteratorCategory::Bidirectional) {
      Convert(expr->getArg(0));
      break;
    }

    {
      bool deref = !isAddrOf();
      PushParen paren(*this, deref);
      if (deref) {
        StrCat(GetPointerDerefPrefix(expr->getType()));
      }
      Convert(expr->getArg(0));
      if (deref) {
        StrCat(GetPointerDerefSuffix(expr->getType()));
        SetValueFreshness(expr->getType());
      }
    }
    break;

  case clang::OverloadedOperatorKind::OO_Subscript: {
    if (IsUniquePtr(expr->getArg(0)->getType())) {
      StrCat(
          std::format("{}.as_ref().unwrap()", ConvertRValue(expr->getArg(0))));
      if (isAddrOf()) {
        StrCat(std::format(".as_pointer().offset(({}))",
                           ConvertRValue(expr->getArg(1))));
      } else {
        if (isRValue()) {
          StrCat(".borrow()");
        } else {
          StrCat(".borrow_mut()");
        }
        StrCat(std::format("[({}) as usize]", ConvertRValue(expr->getArg(1))));
      }
      SetValueFreshness(expr->getType());
      break;
    }

    bool is_inner_boxed =
        IsBoxedType(expr->getType().getNonReferenceType()) &&
        IsBoxedType(expr->getArg(0)->getType().getNonReferenceType());

    if (isLValue()) {
      PushConversionKind push_ck(*this, ConversionKind::Unboxed);
      pending_deref_.set(
          std::format("({} as {}).offset({})",
                      ConvertObject(expr->getArg(0), ObjectShape::Element),
                      ConvertPtrType(expr->getArg(0)->getType()),
                      ConvertSubscriptIndex(expr->getArg(1))),
          /*fresh=*/true, expr);
      break;
    }

    {
      bool deref = !isAddrOf();
      PushParen paren(*this, deref);
      if (deref) {
        StrCat(GetPointerDerefPrefix(expr->getType()));
      }

      if (is_inner_boxed && !isObject()) {
        StrCat('(');
      }

      PushConversionKind push(*this, ConversionKind::Unboxed);
      StrCat(std::format("({} as {}).offset({})",
                         ConvertObject(expr->getArg(0), ObjectShape::Element),
                         ConvertPtrType(expr->getArg(0)->getType()),
                         ConvertSubscriptIndex(expr->getArg(1))));

      if (is_inner_boxed) {
        StrCat(GetPointerDerefSuffix(expr->getType()), ".as_pointer()");
        if (!isObject()) {
          StrCat(std::format("as Ptr<{}>)", ToString(expr->getType())));
        }
      }

      if (isAddrOf()) {
        computed_expr_type_ = ComputedExprType::FreshPointer;
      } else {
        StrCat(GetPointerDerefSuffix(expr->getType()));
        SetValueFreshness(expr->getType());
      }
    }
    break;
  }
  default:
    return Converter::ConvertCXXOperatorCallExpr(expr);
  }
  return false;
}

void ConverterRefCount::ConvertFunctionParameters(clang::FunctionDecl *decl) {
  PushConversionKind push(*this, ConversionKind::Unboxed);
  if (decl->isMain() && (decl->getNumParams() != 0U)) {
    StrCat(std::format("{}: i32, {}: Ptr<Ptr<u8>>",
                       GetNamedDeclAsString(decl->getParamDecl(0)),
                       GetNamedDeclAsString(decl->getParamDecl(1))));
  } else {
    Converter::ConvertFunctionParameters(decl);
  }
}

std::string ConverterRefCount::ConvertSubscriptIndex(clang::Expr *idx) {
  auto str = ConvertRValue(idx);
  if (idx->getType()->isEnumeralType()) {
    return std::format("({}) as isize", str);
  }
  return str;
}

void ConverterRefCount::ConvertArraySubscript(clang::Expr *base,
                                              clang::Expr *idx,
                                              clang::QualType type) {
  if (isAddrOf()) {
    bool is_inner_boxed = false;
    if (auto base_arr_ty = clang::dyn_cast<clang::ArrayType>(
            base->IgnoreImplicit()->getType().getTypePtr())) {
      is_inner_boxed = clang::isa<clang::ArrayType>(
          base_arr_ty->getElementType().getTypePtr());
    }

    {
      PushParen paren(*this, is_inner_boxed);
      if (IsStringLiteralExpr(base)) {
        auto code_unit = ToStringBase(
            ctx_.getAsArrayType(
                    base->IgnoreParens()->IgnoreImplicit()->getType())
                ->getElementType());
        StrCat(std::format("Ptr::<{}>::from_string_literal({}).offset({})",
                           code_unit,
                           ToString(base->IgnoreParens()->IgnoreImplicit()),
                           ConvertSubscriptIndex(idx)));
      } else {
        StrCat(std::format("({} as {}).offset({})",
                           ToString(base->IgnoreImplicit()),
                           ConvertPtrType(base->IgnoreImplicit()->getType()),
                           ConvertSubscriptIndex(idx)));
      }

      if (is_inner_boxed) {
        StrCat(GetPointerDerefSuffix(type), ".as_pointer()");
      }
    }

    computed_expr_type_ = ComputedExprType::FreshPointer;
  } else {
    if (isLValue() &&
        clang::isa<clang::ArraySubscriptExpr>(base->IgnoreImplicit())) {
      PushExprKind push(*this, ExprKind::RValue);
      Convert(base->IgnoreImplicit());
    } else {
      Convert(base->IgnoreImplicit());
    }
    if (clang::isa<clang::ArraySubscriptExpr>(base->IgnoreImplicit())) {
      if (isRValue()) {
        StrCat(".borrow()");
      } else {
        StrCat(".borrow_mut()");
      }
    }
    StrCat(std::format("[({}) as usize]", ConvertRValue(idx)));
    SetValueFreshness(type);
  }
}

void ConverterRefCount::ConvertPointerSubscript(
    clang::ArraySubscriptExpr *expr) {
  auto *base = expr->getBase();
  auto *idx = expr->getIdx();

  if (isLValue()) {
    pending_deref_.assert_consumed();
    Buffer buf(*this);
    ConvertPointerOffset(base, idx);
    pending_deref_.set_unchecked(std::move(buf).str(), isFresh(), expr);
    return;
  }

  bool deref = !isAddrOf();
  PushParen paren(*this, deref);
  if (deref) {
    StrCat(GetPointerDerefPrefix(expr->getType()));
  }
  ConvertPointerOffset(base, idx);
  if (deref) {
    StrCat(GetPointerDerefSuffix(expr->getType()));
    SetValueFreshness(expr->getType());
  }
}

std::string ConverterRefCount::ForceGlobalInit(const clang::VarDecl *decl) {
  return std::format("let _ = {}.with(|_| ());", GetNamedDeclAsString(decl));
}

void ConverterRefCount::ConvertFunctionMain(
    const clang::FunctionDecl *decl,
    const std::string_view main_function_name) {
  if (decl->getNumParams() != 0U) {
    StrCat(std::format(R"(
pub fn main() {{
    let argv: Vec<Value<Vec<u8>>> = ::std::env::args()
        .map(|x| Rc::new(RefCell::new(x.as_bytes().to_vec())))
        .collect();
    let mut argv: Value<Vec<Ptr<u8>>> = Rc::new(RefCell::new(
        argv.iter().map(|x| {{ x.borrow_mut().push(0); x.as_pointer() }}).collect(),
    ));
    (*argv.borrow_mut()).push(Ptr::null());
    __cpp2rust_init_globals();
    ::std::process::exit({}(::std::env::args().len() as i32,
                                argv.as_pointer()));
}})",
                       main_function_name));
  } else {
    StrCat(std::format("pub fn main() {{ __cpp2rust_init_globals(); "
                       "std::process::exit({}()); }}",
                       main_function_name));
  }
}

void ConverterRefCount::ConvertAddrOf(clang::Expr *expr,
                                      clang::QualType pointer_type) {
  StrCat(ConvertPointer(expr));
}

void ConverterRefCount::ConvertDeref(clang::Expr *expr) {
  auto pointee_type = expr->getType()->getPointeeType();

  if (isLValue()) {
    auto ptr = ToString(expr);
    pending_deref_.set(std::move(ptr), isFresh());
    return;
  }

  {
    bool deref = !isAddrOf();
    PushParen paren(*this, deref);
    if (deref) {
      StrCat(GetPointerDerefPrefix(pointee_type));
    }
    Convert(expr);
    if (deref) {
      StrCat(GetPointerDerefSuffix(pointee_type));
      SetValueFreshness(pointee_type);
    }
  }

  if (isObject() && WantsElementPtr() &&
      (IsBoxedType(pointee_type) || pointee_type->isArrayType())) {
    StrCat(".decay()");
    computed_expr_type_ = ComputedExprType::FreshPointer;
  }
}

void ConverterRefCount::ConvertArrow(clang::Expr *expr) {
  auto *op = clang::dyn_cast<clang::CXXOperatorCallExpr>(expr);
  bool is_overloaded_arrow =
      op && op->getOperator() == clang::OverloadedOperatorKind::OO_Arrow;

  if (!is_overloaded_arrow || IsUserOperatorCall(op)) {
    auto ptr = ToString(expr);
    StrCat(DerefPtrExpr(ptr, expr->getType()->getPointeeType()));
    SetValueFreshness(expr->getType()->getPointeeType());
    return;
  }

  if (GetStrongestIteratorCategory(op->getArg(0)->getType()) ==
      IteratorCategory::Bidirectional) {
    Convert(op->getArg(0));
    return;
  }

  // A rule-resolved `operator->` yields a `Ptr<T>`, so reaching a FIELD through it
  // still needs the pointer deref that `AccessLValueObject` already applies for a
  // METHOD call on the same base. Without it, `p->v` emitted
  // `<Ptr<S> result>.v.borrow_mut()` -- measured as rustc E0609
  // `no field 'v' on type libcc2rs::Ptr<S>` on both `std::shared_ptr<S>` and
  // `std::optional<S>` receivers, in the read and the write direction alike. Bare
  // `Convert(expr)` cannot supply it: it lands in the OO_Arrow arm with
  // `expr->getType()` = `S *`, which is POD and not a record, so the deref helper
  // answers `.read()`/`""`. The POINTEE is the type to hand it, exactly as
  // `AccessLValueObject` does.
  if (IsRuleArrowResult(expr)) {
    auto pointee_type = expr->getType()->getPointeeType();
    StrCat(DerefPtrExpr(ToString(expr), pointee_type));
    SetValueFreshness(pointee_type);
    return;
  }

  Convert(expr);
}

std::string ConverterRefCount::AccessLValueObject(clang::MemberExpr *member) {
  auto *method = clang::dyn_cast<clang::CXXMethodDecl>(member->getMemberDecl());
  auto *object = member->getBase();

  bool is_mut = method && !method->isConst();
  if (member->isArrow()) {
    auto *op =
        clang::dyn_cast<clang::CXXOperatorCallExpr>(object->IgnoreImplicit());
    if (op && GetStrongestIteratorCategory(op->getArg(0)->getType()) ==
                  IteratorCategory::Bidirectional) {
      return ConvertRValue(op->getArg(0));
    }
    auto str = is_mut ? ConvertLValue(object) : ConvertRValue(object);
    auto pointee_type = object->getType()->getPointeeType();
    return DerefPtrExpr(str, pointee_type);
  }
  return is_mut ? ConvertLValue(object) : ConvertRValue(object);
}

void ConverterRefCount::ConvertConstructedValue(clang::QualType type,
                                                clang::CXXConstructExpr *ctor) {
  PushConversionKind push(*this, ConversionKind::Unboxed);
  ConvertVarInit(type, ctor);
}

const char *
ConverterRefCount::GetPointerDerefSuffix(clang::QualType pointee_type) {
  if (pointee_type.isPODType(ctx_) && !pointee_type->isRecordType()) {
    return ".read()";
  }
  return ".upgrade().deref()";
}

void ConverterRefCount::EmitDecompositionHolderAnnotation(
    clang::QualType value_type, bool is_mut) {
  // Byte-for-byte what `ConverterRefCount::VisitReferenceType` (:197) emits for
  // a non-array pointee, which is the annotation this model ALREADY gives the
  // hand-written equivalent of a decomposition holder
  // (`const std::pair<..> &h = v.at(i);` -> `let h: Ptr<(Value<..>,
  // Value<..>)> = ..;`) -- deliberately the same three lines rather than a call
  // to it, because that function takes a `ReferenceType *` we do not have here
  // and synthesising one would be worse than repeating three tokens.
  //
  // `is_mut` is deliberately UNUSED, and as of the mutable-holder step it is
  // reached with `is_mut == true`: this model has no `*mut`/`*const`
  // distinction to make. `ConverterRefCount::VisitReferenceType` spells BOTH
  // `T &` and `const T &` as `Ptr<T>` -- mutability lives in the `RefCell` a
  // `Value<T>` wraps, not in the handle -- so a mutable reference holder is the
  // same `Ptr<(Value<T1>, Value<T2>)>` and a write through a binding reaches the
  // pair by sharing that element's `Rc`, not by writing through the holder. See
  // the POINTER-FORM block in ConvertTupleDecompositionDecl for the write
  // argument and for why `ptr_form` is cleared for this model.
  PushConversionKind push(*this, ConversionKind::Pointee);
  StrCat("Ptr<");
  Convert(value_type);
  StrCat(token::kGt);
}

std::string ConverterRefCount::DecompositionHolderElement(
    const std::string &holder, const std::string &element,
    clang::QualType value_type) {
  // `.upgrade().deref()` is what a `Ptr<T>` deref costs in this model, and
  // `.clone()` is MANDATORY, not cosmetic: the place `(*h..).N` has type
  // `Value<..>` = `Rc<RefCell<..>>`, and moving it out of a deref of a `Ptr` is
  // `E0507`. Cloning an `Rc` SHARES the cell, so the binding names the very
  // object the pair element names -- the same aliasing the C++ `const auto &`
  // asks for, and the same argument as the map-range decomposition's
  // `second()`.
  //
  // ⭐ AND THAT SHARING IS WHAT MAKES THE MUTABLE HOLDER (`auto &[a, b] = pr;`)
  // SOUND ON THE SAME SPELLING, where the const arm's "every binding is const,
  // so there is no write to lose" argument does not apply: a write goes through
  // `a.borrow_mut()`, and `a` and `(*h..).0` are two `Rc`s to ONE `RefCell`, so
  // the write lands in the pair's own element. The unsafe model needs
  // `&raw mut` there only because ITS element read is a copy; here the read is a
  // handle, so there is no write to drop.
  return std::format("(*{}{}).{}.clone()", holder,
                     GetPointerDerefSuffix(value_type), element);
}

const char *
ConverterRefCount::GetPointerDerefPrefix(clang::QualType pointee_type) {
  if (pointee_type.isPODType(ctx_) && !pointee_type->isRecordType()) {
    return "";
  }
  return token::kStar;
}

std::string ConverterRefCount::DerefPtrExpr(std::string_view ptr_expr,
                                            clang::QualType pointee_type) {
  return std::format("({}{}{})", GetPointerDerefPrefix(pointee_type), ptr_expr,
                     GetPointerDerefSuffix(pointee_type));
}

bool ConverterRefCount::IsReferenceType(const clang::Expr *expr) const {
  if (Converter::IsReferenceType(expr)) {
    return true;
  }
  if (auto *call =
          clang::dyn_cast<clang::CXXOperatorCallExpr>(expr->IgnoreCasts())) {
    return GetReturnTypeOfFunction(call)->isReferenceType();
  }
  return false;
}

std::string ConverterRefCount::ConvertMappedMethodCall(
    clang::Expr *expr, const TranslationRule::MethodCallFragment &mc,
    clang::Expr **args, unsigned num_args, TempMaterializationCtx *ctx) {
  auto receiver_ph = mc.getReceiverPlaceholder();
  if (!receiver_ph || receiver_ph->access == TranslationRule::Access::kBorrow ||
      receiver_ph->access == TranslationRule::Access::kMove) {
    return Converter::ConvertMappedMethodCall(expr, mc, args, num_args, ctx);
  }

  auto arg_idx = receiver_ph->n;
  auto *arg = BuildUnifiedArgs(expr, args, num_args)[arg_idx];
  if (auto *call = clang::dyn_cast<clang::CallExpr>(arg->IgnoreCasts());
      call && IsTransparentStdCall(call)) {
    arg = call->getArg(0);
  }

  if (!arg->getType()->isPointerType() && !IsReferenceType(arg)) {
    return Converter::ConvertMappedMethodCall(expr, mc, args, num_args, ctx);
  }

  auto param_type = Mapper::GetParamType(GetCalleeOrExpr(expr), arg_idx);

  if (arg->getType()->isPointerType()) {
    return std::format("{}.with_mut(|__v: {}| __v{})", ConvertPointer(arg),
                       param_type,
                       ConvertIRFragment(mc.body, expr, args, num_args, ctx));
  }

  ConvertIRFragment(mc.receiver, expr, args, num_args, ctx);
  assert(!pending_deref_.empty());

  bool is_boxed = pending_deref_.is_boxed();
  auto ptr = pending_deref_.take();
  auto body = ConvertIRFragment(mc.body, expr, args, num_args, ctx);
  SetFreshType(expr->getType());

  if (is_boxed) {
    return std::format(
        "{}.with_mut(|__v: &mut Value<{}>| (*__v.borrow_mut()){})", ptr,
        ToString(arg->getType()), body);
  }

  return std::format("{}.with_mut(|__v: {}| __v{})", ptr, param_type, body);
}

std::string ConverterRefCount::ConvertPointeeType(clang::QualType ptr_type) {
  assert(!ptr_type.isNull() && ptr_type->isPointerType());
  PushConversionKind push(*this, ConversionKind::Unboxed);
  auto pointee = ptr_type->getPointeeType();
  if (!pointee->isRecordType()) {
    return std::string(Trim(ToString(pointee)));
  }

  // Pointee of a pointer to incomplete type is an incomplete type that does
  // not have a translation rule. Hence ToString(ptr_type->getPointeeType()) is
  // not enough
  auto str = ToString(ptr_type);
  Unwrap(str, "PtrDyn<", ">");
  Unwrap(str, "Ptr<", ">");
  return std::string(Trim(str));
}

void ConverterRefCount::ConvertParamTyPointerCastIfNeeded(
    clang::QualType param_type, clang::Expr *expr) {
  if (!param_type->isPointerType() || !expr->getType()->isPointerType() ||
      IsVaListType(param_type) || IsVaListType(expr->getType())) {
    return;
  }
  auto dest_type = ConvertPointeeType(param_type);
  if (dest_type != ConvertPointeeType(expr->getType())) {
    StrCat(std::format(".reinterpret_cast::<{}>()", dest_type));
  }
}

bool ConverterRefCount::ShouldConvertMethod(const clang::CXXMethodDecl *decl) {
  if (clang::isa<clang::CXXDestructorDecl>(decl)) {
    return IsMethodOnPtr(decl);
  }
  return Converter::ShouldConvertMethod(decl);
}

bool ConverterRefCount::ThisIsRustPtr() const {
  auto *method = clang::dyn_cast_or_null<clang::CXXMethodDecl>(curr_function_);
  return method && (IsMethodOnPtr(method) ||
                    clang::isa<clang::CXXConstructorDecl>(method));
}

void ConverterRefCount::SetUFCSReceiver(clang::Expr *base, bool is_arrow,
                                        const clang::CXXMethodDecl *method) {
  if (!IsMethodOnPtr(method)) {
    Converter::SetUFCSReceiver(base, is_arrow, method);
    return;
  }
  bool base_is_pointer = is_arrow && !clang::isa<clang::CXXOperatorCallExpr>(
                                         base->IgnoreParenImpCasts());
  if (clang::isa<clang::CXXThisExpr>(base->IgnoreParenImpCasts())) {
    bool in_ctor =
        curr_function_ && clang::isa<clang::CXXConstructorDecl>(curr_function_);
    if (in_ctor) {
      ufcs_receiver_ = "&this";
    } else if (ThisIsRustPtr()) {
      ufcs_receiver_ = keyword::kSelfValue;
    } else {
      ufcs_receiver_ = token::kRef + ConvertPointer(base);
    }
    return;
  }
  if (IsTemporaryObject(base) && base->getType()->isRecordType() &&
      !IsReferenceType(base->IgnoreImplicit())) {
    PushConversionKind push(*this, ConversionKind::FullRefCount);
    ufcs_receiver_ =
        token::kRef + BoxValue(ConvertRValue(base)) + ".as_pointer()";
    return;
  }
  ufcs_receiver_ = token::kRef + (base_is_pointer ? ConvertRValue(base)
                                                  : ConvertPointer(base));
}

std::string
ConverterRefCount::GetUFCSName(const clang::CXXMethodDecl *method) const {
  return IsMethodOnPtr(method) ? TraitName(method->getParent())
                               : GetRecordName(method->getParent());
}

std::string
ConverterRefCount::TraitName(const clang::CXXRecordDecl *decl) const {
  return GetRecordName(decl) + "Impl";
}

ConverterRefCount::MethodsOnPtr &
ConverterRefCount::MethodsOnPtrFor(const clang::CXXRecordDecl *decl) {
  auto name = GetRecordName(decl);
  auto [it, inserted] = methods_on_ptr_.try_emplace(name);
  if (inserted) {
    it->second.trait.header = std::format("pub trait {}", TraitName(decl));
    it->second.impl.header =
        std::format("impl {} for Ptr<{}>", TraitName(decl), name);
  }
  return it->second;
}

bool ConverterRefCount::ConvertOutOfLineMethod(clang::CXXMethodDecl *decl) {
  if (!IsMethodOnPtr(decl)) {
    return Converter::ConvertOutOfLineMethod(decl);
  }
  Buffer buf(*this);
  {
    PushMethodTarget push(*this, MethodTarget::PtrImpl);
    ConvertCXXMethodDecl(decl);
  }
  MethodsOnPtrFor(decl->getParent()).impl.body += std::move(buf).str();
  return false;
}

void ConverterRefCount::ConvertMethodOnPtrTraitDecl(
    clang::CXXMethodDecl *method) {
  Buffer buf(*this);
  {
    PushCurrFunction push_fn(*this, method);
    PushMethodTarget push(*this, method->getDefinition()
                                     ? MethodTarget::TraitDecl
                                     : MethodTarget::TraitDefault);
    ConvertCXXMethodDecl(method);
  }
  MethodsOnPtrFor(method->getParent()).trait.body += std::move(buf).str();
}

void ConverterRefCount::ConvertMethodOnPtr(clang::CXXMethodDecl *method) {
  if (!method->isThisDeclarationADefinition()) {
    return;
  }
  Buffer buf(*this);
  {
    PushMethodTarget push(*this, MethodTarget::PtrImpl);
    VisitCXXMethodDecl(method);
  }
  MethodsOnPtrFor(method->getParent()).impl.body += std::move(buf).str();
}

void ConverterRefCount::ConvertLateInstantiatedMethods(
    clang::CXXRecordDecl *decl) {
  Converter::ConvertCXXMethodDecls(
      decl, std::format("{} {}", keyword::kImpl, GetRecordName(decl)),
      [](auto *method) {
        return IsEmittableMethod(method) && method->hasBody() &&
               !IsMethodOnPtr(method) &&
               !decl_ids_.contains(GetMethodID(method));
      });
  auto convert_method = [&](clang::CXXMethodDecl *method) {
    if (IsEmittableMethod(method) && method->hasBody() &&
        IsMethodOnPtr(method) && !decl_ids_.contains(GetMethodID(method))) {
      ConvertMethodOnPtr(method);
    }
  };
  for (auto *method : decl->methods()) {
    convert_method(method);
  }
  ForEachTemplateInstantiatedMethod(decl, convert_method);
}

void ConverterRefCount::ConvertCXXRecordMethods(clang::CXXRecordDecl *decl) {
  auto struct_name = GetRecordName(decl);

  ConvertCXXMethodDecls(decl, std::format("{} {}", keyword::kImpl, struct_name),
                        [](auto *method) {
                          return IsEmittableMethod(method) &&
                                 !IsMethodOnPtr(method);
                        });

  auto convert_method = [&](clang::CXXMethodDecl *method) {
    if (!IsMethodOnPtr(method)) {
      return;
    }
    // Match the unsafe model's contract (converter.cpp:1363): a method that is
    // declared but not defined in this TU is SKIPPED, not translated. Here that
    // means emitting neither the trait declaration nor the impl entry -- a Rust
    // trait must be complete and the sibling impl cannot supply the body
    // (ConvertMethodOnPtr returns early for non-definitions), so the trait would
    // need a default body and there is no C++ body to translate. Emitting an
    // `unimplemented!()` default is banned (see converter.cpp:1440). Skipping
    // makes refcount fail where unsafe already does -- rustc E0599 at the call
    // site -- instead of aborting the whole translation. Pure virtuals are kept:
    // they legitimately lower to a body-less trait method.
    if (!method->isPureVirtual() && !method->getDefinition()) {
      return;
    }
    ConvertMethodOnPtrTraitDecl(method);
    ConvertMethodOnPtr(method);
  };
  for (auto *method : decl->methods()) {
    convert_method(method);
  }
  ForEachTemplateInstantiatedMethod(decl, convert_method);

  if (!GetUserDefinedDestructor(decl) && HasFieldsNeedingDestruction(decl)) {
    MethodsOnPtrFor(decl).trait.body +=
        std::format("fn {}(&self);\n", kDestructorName);
    MethodsOnPtrFor(decl).impl.body += std::format(
        "fn {}(&self) {{ {} }}\n", kDestructorName, DestroyMembers(decl));
  }
}

std::string
ConverterRefCount::DestroyMembers(const clang::CXXRecordDecl *decl) {
  std::vector<const clang::FieldDecl *> fields;
  for (auto *field : decl->fields()) {
    if (TypeNeedsDestruction(field->getType())) {
      fields.push_back(field);
    }
  }

  std::string out;
  for (auto *field : std::ranges::reverse_view(fields)) {
    auto name = GetNamedDeclAsString(field);
    if (field->getType()->isArrayType()) {
      auto *elem =
          field->getType()->getBaseElementTypeUnsafe()->getAsCXXRecordDecl();
      assert(elem);
      out += std::format(
          "{{ let __p = (*self.upgrade().deref()).{0}.as_pointer(); for __i in "
          "0..__p.len() {{ {2}::{1}(&__p.offset(__i as isize)); }} }}\n",
          name, kDestructorName, TraitName(elem));
    } else {
      out += std::format("(*self.upgrade().deref()).{0}.as_pointer().{1}();\n",
                         name, kDestructorName);
    }
  }
  return out;
}

void ConverterRefCount::ConvertCXXConstructorBody(
    clang::CXXConstructorDecl *decl) {
  EmitFunctionPreamble(decl);
  auto record_name = GetRecordName(decl->getParent());
  StrCat(keyword::kLet, "__this", token::kColon,
         std::format("Value<{}>", record_name), token::kAssign,
         "Rc::new(RefCell::new(");
  if (decl->isDelegatingConstructor()) {
    Convert((*decl->init_begin())->getInit());
  } else {
    StrCat("Self");
    PushBrace this_init(*this);
    EmitConstructorFieldInits(decl);
  }
  StrCat("))", token::kSemiColon);
  StrCat(keyword::kLet, "this", token::kColon,
         std::format("Ptr<{}>", record_name), token::kAssign,
         "__this.as_pointer()", token::kSemiColon);
  ConvertBodyStmts(decl->getBody());
  StrCat("Rc::try_unwrap(__this).ok().unwrap().into_inner()");
}

bool ConverterRefCount::VisitCXXThisExpr(
    [[maybe_unused]] clang::CXXThisExpr *expr) {
  bool in_ctor =
      curr_function_ && clang::isa<clang::CXXConstructorDecl>(curr_function_);
  if (in_ctor) {
    StrCat("this");
  } else {
    StrCat("(*", keyword::kSelfValue, ')');
  }
  computed_expr_type_ = ComputedExprType::Pointer;
  return false;
}
} // namespace cpp2rust
