// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// std::optional<T1>.  THREE type rules, all mapping to the same Rust type:
// libc++ splits std::optional's members over two base class templates, and
// `cxxMethodNameLookup` (cpp_rule_preprocessor.cpp:725) resolves a receiver from
// its DECLARED PARAMETER TYPE -- so `has_value()` and `reset()` MUST be written
// against __optional_storage_base / __optional_destruct_base or the preprocessor
// aborts with assert(0 && "Rule resolution failed") (:888).
//
// std::nullopt_t is taken as a PARAMETER everywhere, never spelled as
// `std::nullopt`: copy-initialising a by-value nullopt_t from the `const
// nullopt_t` lvalue `std::nullopt` finds no viable copy constructor in the
// preprocessor's synthetic context.
//
// `operator->` IS MAPPED (f30/f31).  An earlier comment here refused it, citing
// that the converter treats an operator-> rule's result as a PLACE of type T1.
// That observation is TRUE and it is also exactly what is WANTED: `o->m` must
// lower to a place so the member access can be taken on it.  MEASURED 2026-09-27:
// the E0609 that produced the refusal came from a BODY RETURNING A POINTER
// (`Ptr<T1>`) being used as a place -- a BODY bug, not a converter limit.  The
// fix is that the body must yield the PLACE, i.e. the SAME TEXT as f11/f12
// (`operator*`), never a pointer.  Verified with `-verbose`: both keys report
// `Matching:` in both models and both models emit f30/f31's body with the correct
// receiver.  `(*o).f` remains an equivalent spelling, but it is no longer the only
// translatable one.
//
// RESIDUAL, measured and NOT this rule's defect: in the REFCOUNT model an arrow
// followed by a FIELD access gives `error[E0609]: no field \`v\` on type
// \`libcc2rs::Ptr<S>\``, because the converter emits `<Ptr result>.v.borrow_mut()`
// without the `upgrade().deref()` step it DOES emit for the same field access
// behind `operator*`.  rules/shared_ptr's f9 reproduces it identically, so it is a
// CONVERTER asymmetry between the two lowerings, not a rule bug and not fixable in
// a body.  Arrow-to-METHOD-CALL is correct in both models today.  Two further
// converter sites are already filed against this wave: unsafe emits a spurious
// `.cast_const()` (converter.cpp:3175-3184, the CK_NoOp arm of
// VisitImplicitCastExpr) and refcount a spurious `.decay()`.

#include <optional>
#include <utility>

template <typename T1> using t1 = std::optional<T1>;
template <typename T1> using t2 = std::__optional_storage_base<T1>;
template <typename T1> using t3 = std::__optional_destruct_base<T1>;
using t4 = std::nullopt_t;

template <typename T1> std::optional<T1> f1() { return std::optional<T1>(); }

// THE CONVERTING CONSTRUCTOR `optional(T1 &&)` IS THE MISSING KEY, AND IT STILL
// CANNOT BE RECORDED.  HARVESTED 2026-09-27 with `-verbose` on
// `std::optional<S> a{S{7}};`:
//     search expr void std::optional<S>::optional(S &&), result:
//     None
// after which the converter emitted `let mut a: Option<S> = S { v: 7 };` -- the
// initializer BARE, with the `Some(...)` wrap MISSING (unsafe: `expected
// Option<S>, found S`).  So the missing `Some` is a MISSING KEY, not a converter
// defect: the converter simply found no rule for the converting constructor and
// fell through to emitting the initializer.
//
// MEASURED ATTEMPTS, both recording the WRONG key and therefore NOT SHIPPED
// (either would be a duplicate of f4 with a different body, i.e. an ambiguous
// rule):
//   * `return std::optional<T1>(std::move(a0));`  -> records
//     `void std::optional<T1>::optional(const std::optional<T1> &)`
//   * `return std::optional<T1>{std::move(a0)};`  -> records the SAME thing
//     (brace-init does NOT change the recorded overload; checked in ir_src.json)
// The preprocessor's synthetic resolution picks the COPY constructor over
// libc++'s `template<class U = T> optional(U&&)` in this dependent context.
//
// CORRECTION, MEASURED 2026-09-27 -- the `explicit-template-args` HYPOTHESIS IS
// DEAD.  This comment used to say the next step was a spelling that forces U,
// "which probably needs the `explicit-template-args` marker
// (regen-rule.sh:68)".  Two slots have now measured that: THREE different
// spellings all record the COPY ctor, INCLUDING with the marker turned ON, so
// the marker changes nothing here and is not the missing piece.  (It is also not
// carried by any module in the tree -- see the corrected note in
// regen-rule.sh:68 itself.)
// The defect is still live and still a MISSING KEY, not a wrong body:
// `search expr void std::optional<S>::optional(S &&)` returns None, so
// `std::optional<S> a{S{7}}` emits `let mut a: Option<S> = S { v: 7 };` with the
// `Some(...)` wrap missing in BOTH models.  The key wanted is
// `void std::optional<T1>::optional(T1 &&)`.
// THE REMAINING PATH, from evidence in rules/support: its `f18` records
// `llvm::FailureOr<T1>::FailureOr(T1 &&)` SUCCESSFULLY, because support RESTATES
// the class, so the ctor is a plain non-template rather than libc++'s
// `template<class U = T> optional(U&&)`.  The recorder only produces a key
// STRING and the converter matches by string, so RESTATING `std::optional<T1>`
// with a non-template `optional(T1&&)` should record the wanted key distinctly
// from the copy ctor.  That is a WHOLE-MODULE rewrite (30 keys, plus the two
// `__optional_*_base` receivers that exist precisely because libc++ splits the
// members) and it is depended on by rules/support/FailureOr, so it needs a slot
// of its own -- do not start it piecemeal.

template <typename T1> std::optional<T1> f3(std::nullopt_t n) {
  return std::optional<T1>(n);
}

template <typename T1> std::optional<T1> f4(const std::optional<T1> &o) {
  return std::optional<T1>(o);
}

template <typename T1> std::optional<T1> f5(std::optional<T1> &&o) {
  return std::optional<T1>(std::move(o));
}

template <typename T1>
std::optional<T1> &f6(std::optional<T1> &d, std::nullopt_t n) {
  return d.operator=(n);
}

template <typename T1> std::optional<T1> &f7(std::optional<T1> &d, T1 &&v) {
  return d.operator=(std::move(v));
}

template <typename T1>
std::optional<T1> &f8(std::optional<T1> &d, const std::optional<T1> &s) {
  return d.operator=(s);
}

template <typename T1> bool f9(const std::__optional_storage_base<T1> &o) {
  return o.has_value();
}

template <typename T1> bool f10(const std::optional<T1> &o) {
  return o.operator bool();
}

template <typename T1> T1 &f11(std::optional<T1> &o) {
  return o.operator*();
}

template <typename T1> const T1 &f12(const std::optional<T1> &o) {
  return o.operator*();
}

template <typename T1> T1 &f13(std::optional<T1> &o) { return o.value(); }

template <typename T1> const T1 &f14(const std::optional<T1> &o) {
  return o.value();
}

template <typename T1> T1 f15(const std::optional<T1> &o, T1 &d) {
  return o.value_or(d);
}

template <typename T1> void f16(std::__optional_destruct_base<T1> &o) {
  return o.reset();
}

template <typename T1> bool f17(const std::optional<T1> &o, std::nullopt_t n) {
  return operator==(o, n);
}

template <typename T1> bool f18(std::nullopt_t n, const std::optional<T1> &o) {
  return operator==(n, o);
}

template <typename T1> bool f19(const std::optional<T1> &o, std::nullopt_t n) {
  return operator!=(o, n);
}

template <typename T1> bool f20(std::nullopt_t n, const std::optional<T1> &o) {
  return operator!=(n, o);
}

template <typename T1>
bool f21(const std::optional<T1> &a, const std::optional<T1> &b) {
  return operator==(a, b);
}

template <typename T1>
bool f22(const std::optional<T1> &a, const std::optional<T1> &b) {
  return operator!=(a, b);
}

template <typename T1> bool f23(const std::optional<T1> &o, const T1 &v) {
  return operator==(o, v);
}

template <typename T1> bool f24(const std::optional<T1> &o, const T1 &v) {
  return operator!=(o, v);
}

template <typename T1>
std::optional<T1> &f25(std::optional<T1> &d, std::optional<T1> &&s) {
  return d.operator=(std::move(s));
}

// `operator->` IS NOW MAPPED (f30/f31), and the earlier refusal above was wrong.
// MEASURED SYMPTOM without these: `x->member` on any optional-like receiver
// (llvm::FailureOr<T> inherits every reader from std::optional, so it keys HERE)
// fell through to the converter's raw arrow fallback, which emits the RECEIVER
// ITSELF as if it were a pointer -- scratch-fo measured unsafe
// `(*(n).cast_const()).as_ptr()`, E0599 `no method named cast_const found for
// enum Option<T>`, and refcount `(n.as_pointer().decay() as Ptr<u8>)`, E0599 `no
// method named decay`.  The converter treating the result as a PLACE of type T1
// is exactly what is wanted here, so the BODY must yield the place (identical
// text to f11/f12's `operator*`), never a pointer.  rules/shared_ptr's E0609 came
// from a body returning `Ptr<T1>` used as a place -- a body bug, not a converter
// limit.
template <typename T1> T1 *f30(std::optional<T1> &o) { return o.operator->(); }

template <typename T1> const T1 *f31(const std::optional<T1> &o) {
  return o.operator->();
}

// f32 -- `std::nullopt`, i.e. `inline constexpr std::nullopt_t std::nullopt`
// (libcxx/__utility/in_place.h / optional).  It is a namespace-scope global with a
// definition, but it lives in a SYSTEM HEADER, and the converter only emits
// `pub static mut X_<n>: LazyCell<..>` for a VarDecl it actually traverses -- a
// system-header decl never is.  So the use site emits
//     (*std::cell::LazyCell::force_mut(&mut *&raw mut nullopt_101))
// with no declaration anywhere in the file: E0425, rc=0, no placeholder token.
// 3 of 78 bucket-A census TUs carry it (6 use sites in ktir-mlir-frontend
// KtdpDialect.cpp alone).
//
// SEARCHED SPELLING, read back from the converter's -verbose log, NOT guessed:
//     search expr std::nullopt, result:
//     None
// The plain qualified name.  NOTE this only happens where the surrounding
// construction is NOT itself keyed: `return std::nullopt;` into an
// `std::optional<int>` matches f3 (`optional(std::nullopt_t)`) and the argument is
// never emitted, which is why the tiny synthetic case shows no dangle.  The corpus
// sites are the ones that reach VisitDeclRefExpr -- a by-value `nullopt_t`
// copy-construct, and argument positions -- so a key here is the only fix.
//
// THE TARGET IS `()`, NOT `None`, AND THAT IS DELIBERATE.  The searched expression's
// TYPE is `std::nullopt_t`, which this module already models as the unit type (t4,
// `fn t4() -> ()`, "it is a tag"); the readback confirms the converter agrees --
// `search type std::nullopt_t, result: ()`.  `()` has exactly one value, so mapping
// the unique value of a unit type to `()` is faithful rather than approximate, and it
// is what f6/f17..f20 already expect to receive in their `a1: ()` parameter.  A body
// of `None` would be an Option-typed expression standing in for a nullopt_t-typed one
// and would mistype every argument position (the corpus shows
// `let _args_3: () = <here>`), so it is rejected even though the C++ reads like None.
// What is NOT fixed by this key: two of the six KtdpDialect sites are
// `return <nullopt>` from a function whose Rust return type is Option<..>, i.e. they
// need the nullopt_t -> optional CONVERSION keyed at that site, not this declref.
// That is a separate row and is deliberately left out rather than papered over here.
const std::nullopt_t &f32() { return std::nullopt; }
