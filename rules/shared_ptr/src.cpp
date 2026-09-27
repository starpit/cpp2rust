// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstddef>
#include <memory>

template <typename T, typename A> using Init = A;

template <typename T1> using t1 = std::shared_ptr<T1>;

template <typename T2, typename T1, typename... Args>
std::shared_ptr<T1> f1(Init<T1, Args> &&...args) {
  return std::make_shared<T1>(std::forward<Args>(args)...);
}

template <typename T1> std::shared_ptr<T1> f3(const std::shared_ptr<T1> &o) {
  return std::shared_ptr<T1>(o);
}

template <typename T1> std::shared_ptr<T1> f4() {
  return std::shared_ptr<T1>();
}

template <typename T1>
std::shared_ptr<T1> &f5(std::shared_ptr<T1> &dst, std::shared_ptr<T1> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1>
std::shared_ptr<T1> &f6(std::shared_ptr<T1> &dst,
                        const std::shared_ptr<T1> &src) {
  return dst.operator=(src);
}

template <typename T1> bool f7(std::shared_ptr<T1> &o) {
  return o.operator bool();
}

template <typename T1> T1 &f8(std::shared_ptr<T1> &o) {
  return o.operator*();
}

// `operator->` IS DELIBERATELY NOT MAPPED. Removed rather than shipped.
//
// The converter treats an `operator->` rule's result as a PLACE of type T1: for
// `a->x` in the refcount model it emits `<rule result>.x.borrow_mut()` with no
// `upgrade().deref()` step, so a rule returning a Ptr<T1> gives
// `error[E0609]: no field \`x\` on type \`libcc2rs::Ptr<S>\``. A rule body cannot
// produce a place for a shared pointee without returning a borrow guard, which
// the rule language cannot express.
//
// So the key is LEFT OUT, and `a->x` now fails loudly at translate time instead
// of translating rc=0 and then failing to compile. `(*a).x` works in both models
// and is the translatable spelling. Do not "fix" this by adding a refcount body
// that type-checks in isolation -- the defect is the converter's asymmetry
// between its `operator*` and `operator->` lowering, which wants its own row.

template <typename T1> T1 *f10(std::shared_ptr<T1> &o) { return o.get(); }

template <typename T1> void f11(std::shared_ptr<T1> &o) { return o.reset(); }

template <typename T1> std::shared_ptr<T1> f16(std::shared_ptr<T1> &&o) {
  return std::shared_ptr<T1>(std::move(o));
}

// ---------------------------------------------------------------------------
// NULL TESTS -- `p == nullptr` / `p != nullptr`.
//
// These are NOT the same shape as f7 (`operator bool`) even though they answer
// the same question: f7 is a MEMBER conversion operator and fires only where a
// shared_ptr appears in a boolean context, while these are the free
// `std::__1::operator==/!=(const shared_ptr<T> &, std::nullptr_t)` overloads and
// fire on an explicit comparison against the literal.  The target codebase
// writes both spellings, so both need keys; the BODIES reuse f7's answer
// (is_none / is_some) because a null shared_ptr is exactly `None` in both models.
//
// Called UNQUALIFIED so ADL picks up the overload libc++ actually declares: an
// infix `o == nullptr` records NOTHING (the preprocessor needs call form), and a
// qualified `std::operator==` is not guaranteed viable across standard modes.
//
// `std::nullptr_t` carries no information -- the converter emits the `nullptr`
// literal as `Default::default()` -- so the target binds a1 to `()` and leaves it
// unused, the same treatment rules/mlir f2 gives the StringAttr null test.
template <typename T1>
bool f17(const std::shared_ptr<T1> &o, std::nullptr_t n) {
  return operator==(o, n);
}

template <typename T1>
bool f18(const std::shared_ptr<T1> &o, std::nullptr_t n) {
  return operator!=(o, n);
}

// Pointer IDENTITY between two shared_ptrs -- shared_ptr's operator== compares
// the stored POINTERS, not the pointees, so the body must be Rc::ptr_eq and NOT
// a value comparison.  Two separately-made shared_ptrs holding equal values are
// UNEQUAL here, which is the discriminator a value-comparing body would fail.
template <typename T1>
bool f19(const std::shared_ptr<T1> &a, const std::shared_ptr<T1> &b) {
  return operator==(a, b);
}
