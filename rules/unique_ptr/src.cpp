// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstddef>
#include <memory>

template <typename T, typename A> using Init = A;

template <typename T1> using t1 = std::unique_ptr<T1>;
template <typename T1> using t2 = std::unique_ptr<T1[]>;

template <typename T2, typename T1> std::unique_ptr<T1[]> f1(std::size_t n) {
  return std::make_unique<T1[]>(n);
}

template <typename T1> T1 *f2(std::unique_ptr<T1> &o) { return o.get(); }

template <typename T1> std::unique_ptr<T1> f3(T1 *p) {
  return std::unique_ptr<T1>(p);
}

template <typename T1> std::unique_ptr<T1[]> f4(T1 *p) {
  return std::unique_ptr<T1[]>(p);
}

template <typename T1> void f5(std::unique_ptr<T1> &o, T1 *p) {
  return o.reset(p);
}

template <typename T1> void f6(std::unique_ptr<T1[]> &o, T1 *p) {
  return o.reset(p);
}

template <typename T1> T1 *f7(std::unique_ptr<T1[]> &o) { return o.get(); }

template <typename T1, typename... Args>
std::unique_ptr<T1> f8(Init<T1, Args> &&...args) {
  return std::make_unique<T1>(std::forward<Args>(args)...);
}

template <typename T1> void f9(std::unique_ptr<T1[]> &o) {
  return o.reset(nullptr);
}

template <typename T1> std::unique_ptr<T1> f10() {
  return std::unique_ptr<T1>();
}

template <typename T1> std::unique_ptr<T1[]> f11() {
  return std::unique_ptr<T1[]>();
}

template <typename T1> std::unique_ptr<T1> f12(std::unique_ptr<T1> &&o) {
  return std::unique_ptr<T1>(std::move(o));
}

template <typename T1> std::unique_ptr<T1[]> f13(std::unique_ptr<T1[]> &&o) {
  return std::unique_ptr<T1[]>(std::move(o));
}

template <typename T1>
std::unique_ptr<T1> &f14(std::unique_ptr<T1> &dst, std::unique_ptr<T1> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1>
std::unique_ptr<T1[]> &f15(std::unique_ptr<T1[]> &dst,
                           std::unique_ptr<T1[]> &&src) {
  return dst.operator=(std::move(src));
}

// ---------------------------------------------------------------------------
// NULL TESTS -- `p == nullptr` / `p != nullptr`, and the REVERSED spelling
// `nullptr == p`.  Exactly the shape rules/shared_ptr landed as f17/f18
// (commit 0934519); the target codebase writes all four.
//
// Called UNQUALIFIED: an infix `o == nullptr` records NOTHING (the preprocessor
// needs call form) and a QUALIFIED `std::operator==` aborts the preprocessor at
// cpp_rule_preprocessor.cpp:888.
//
// `std::nullptr_t` carries no information -- the converter emits the `nullptr`
// literal as `Default::default()` -- so the target binds that operand to `()`
// and leaves it deliberately unused.
//
// Unlike shared_ptr there is no pointer-identity variant here: two distinct
// unique_ptrs can never hold the same pointer, so `p == q` is not a shape the
// target codebase can write and no key is added for it.
template <typename T1>
bool f16(const std::unique_ptr<T1> &o, std::nullptr_t n) {
  return operator==(o, n);
}

template <typename T1>
bool f17(const std::unique_ptr<T1> &o, std::nullptr_t n) {
  return operator!=(o, n);
}

template <typename T1>
bool f18(std::nullptr_t n, const std::unique_ptr<T1> &o) {
  return operator==(n, o);
}

template <typename T1>
bool f19(std::nullptr_t n, const std::unique_ptr<T1> &o) {
  return operator!=(n, o);
}

// ---------------------------------------------------------------------------
// f20 -- `std::unique_ptr<T1>(std::nullptr_t)`.
//
// The ctor OVERLOAD the `::new_N` fabrication class was missing for this module
// (FABRICATED-NEWN.md: 234 sites / 117 files at receiver
// `std_unique_ptr_std_string__std_default_delete_std_string__`).  The 117 files
// are ONE C++ row: the inline `InstrInfo::operator=(const InstrInfo &)` in
// sys-arch-spec/progir/progir.h:292-294, re-emitted per including TU, whose
// ternary
//     comment_ = other.comment_ ? std::make_unique<std::string>(*other.comment_)
//                               : nullptr;
// converts the `nullptr` arm to `unique_ptr<std::string>`, i.e. exactly this
// ctor.  `f10` (the default ctor) already exists and RESOLVES; this overload did
// not, so the converter emitted
//     std_unique_ptr_std_string__std_default_delete_std_string__::new_1({
//         Default::default() })
// -- a function defined nowhere -- at rc=0 with no placeholder token.
//
// Same shape, same fix as rules/shared_ptr f20.  `std::nullptr_t` carries no
// information (the converter emits the `nullptr` literal as
// `Default::default()`), so the target binds that operand to `()` and leaves it
// deliberately unused.
//
// OWNERSHIP (the refusal criterion for this module): a null unique_ptr owns
// NOTHING.  No pointer is transferred in, none is duplicated, and the result is
// the same empty `Option` that `f10` yields, so there is no observer of a
// double-free, a leak, or two live owners.  Safe to express.
template <typename T1> std::unique_ptr<T1> f20(std::nullptr_t n) {
  return std::unique_ptr<T1>(n);
}

// ---------------------------------------------------------------------------
// f21/f22 -- `explicit operator bool() const noexcept`.
//
// THE CONVERTER ALREADY CALLS THIS AND THE CALLEE DID NOT EXIST.  An unmapped
// MEMBER does not abort (the converter emits the call TEXTUALLY, with
// `operator bool` spelled `to_bool`), so every boolean use of a unique_ptr came
// out as `unsafe { p.to_bool() }` -- and the only `fn to_bool` in this tree is
// `libcc2rs::IStream::to_bool`, so each site was a SILENT `error[E0599]` that
// rc=0 and rustfmt cannot see.  Measured at HEAD 907226d5 against pin/ir.v36:
// 16 such sites in dxp/src/Driver/dxp-driver.cpp and 14 in dxp/tools/DxpOptMain.cpp
// (the two TUs the rules/tooloutputfile row just took B -> A), of which the
// unique_ptr ones are these keys' work.
//
// ONE KEY COVERS EVERY BOOLEAN CONTEXT.  A contextual conversion to bool is a
// CXXMemberCallExpr to `operator bool` in clang's AST regardless of the spelling
// at the use site, so all five shapes the target codebase writes resolve through
// the SAME key.  All five, measured in the emitted Rust of the two driver TUs:
//   * `if (!output)`                          dxp-driver.cpp:70          -> `if !(p.to_bool())`
//   * `if (tag_)`                             progir.h:317, :338, :342   -> `if (p.to_bool())`
//   * `(bool)tag_ && !getTagStr().empty()`     progir.h:333, :335         -> `(p.to_bool()) && ...`
//   * `tag_ ? *tag_ : empty`                   progir.h:345, :348         -> `if (p.to_bool()) {..} else {..}`
//   * `other.comment_ ? make_unique(..) : nullptr`  progir.h:292, :294    -> same, on a `(*other).` receiver
// So there is no shape left silently unresolved by covering only `!p`.
//
// `const` RECEIVER, deliberately -- unlike rules/shared_ptr f7, which takes a
// mutable `&`.  Four of the sites above are inside `const` member functions
// (`hasTag`/`hasComment`/`getTagStr`/`getCommentStr`), so a rule whose parameter
// is a mutable reference would need a mutable borrow of `self` there.  The
// recorded key is unaffected: it is the CALLEE's signature, and libc++ declares
// the operator `const`, so `const &` is strictly the more permissive spelling.
//
// NOT the same shape as f16-f19.  Those key the free `operator==`/`operator!=`
// against `std::nullptr_t`, which the target codebase also writes; this is the
// conversion operator, a MEMBER, and a site using one never routes through the
// other.
//
// OWNERSHIP (this module's refusal criterion): a boolean test neither transfers
// nor duplicates a pointer, and it cannot observe a double-free or two live
// owners.  `is_some()` is exact in both models -- the unsafe model is
// `Option<Box<T1>>` and the refcount model `Option<Value<T1>>`, and in both an
// empty `Option` is exactly a null `unique_ptr`.
template <typename T1> bool f21(const std::unique_ptr<T1> &o) {
  return o.operator bool();
}

template <typename T1> bool f22(const std::unique_ptr<T1[]> &o) {
  return o.operator bool();
}
