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
