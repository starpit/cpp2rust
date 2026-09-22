// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <cstddef>
#include <memory>

template <typename T1> using t1 = std::shared_ptr<T1>;
template <typename T1> using t2 = std::weak_ptr<T1>;

// ---------------------------------------------------------------------------
// std::shared_ptr<T[]> -- shared ownership of an ARRAY, and the reason
// rules/carray refuses to add `template <typename T1> using t3 = T1[];`
//
// That refusal is correct and still stands: the alias alone clears the
// "Type is not present in types_" abort on `char[]`, but it leaves
// shared_ptr<char[]> mapped by t1 above, i.e. Option<Value<char[]>> over a
// SINGLE element. `buf[0]` then lowers to `.offset(0).write(..)` on the
// shared_ptr handle itself and the crate does not build. An element-type
// alias cannot fix that, because the thing that is wrong is the CONTAINER.
//
// So the array-ness is modelled here instead, as its own type rule. That is
// exactly the split rules/unique_ptr already makes -- t1 = unique_ptr<T1>,
// t2 = unique_ptr<T1[]> -- and it is the closest precedent in the tree:
//
//   model      shared_ptr<T1>          shared_ptr<T1[]>
//   unsafe     Option<Value<T1>>       Option<Value<Box<[T1]>>>
//   refcount   Option<Value<T1>>       Option<Value<Box<[T1]>>>
//
// Both models agree here, unlike unique_ptr (Option<Box<[T1]>> vs
// Option<Value<Box<[T1]>>>), because shared_ptr is shared ownership in BOTH
// models -- an Rc, never a Box. Only the pointer spelling differs, and that
// is what tgt_refcount.rs overlays.
//
// Why `Box<[T1]>` inside the Rc rather than `Vec<T1>`: a shared_ptr<T[]>
// never grows, libcc2rs already has PtrKind::StackArray/HeapArray over
// Rc<RefCell<Box<[T]>>>, and the AsPointer<T> impl for
// Option<Rc<RefCell<Box<[T]>>>> -- which is what `.get()` needs -- already
// exists. Nothing new is required in libcc2rs for this.
//
// The specificity tie-break picks the right rule without any converter
// change. All three type rules land in the SAME bucket (GetTypeMapKey stops
// at '<', so the key is "std::shared_ptr" for each) and search() takes the
// longest matching `src`:
//
//   std::shared_ptr<T1>     19 chars   matches anything
//   std::shared_ptr<T1[]>   21 chars   matches only array specializations
//   std::shared_ptr<void>   21 chars   matches only void
//
// 21 > 19, so an array beats the generic rule; and `<T1[]>` vs `<void>` can
// never both match one spelling, so the equal length is not an ambiguity.
//
// Scope, measured rather than assumed: of the 14 shared_ptr<T[]> sites in
// dt_src, five DO use operator[] (dip.cpp:107/115, deeprt_fold_init.cpp:33,
// dsm.cpp:24194 via `&initBin[i]`) and they are all outside the scheduler
// scope. Inside dcg/ ddc/ dsc/ dbo/ the whole surface is: construct from
// `new T[n]`, write through `.get()` with snprintf/memcpy, and
// reinterpret_pointer_cast<void>. operator[] is therefore NOT given a rule
// here -- see the note on f38 below for why that is the honest choice and
// not an oversight.
// ---------------------------------------------------------------------------

template <typename T1> using t3 = std::shared_ptr<T1[]>;

// ---------------------------------------------------------------------------
// std::shared_ptr<void> -- the type every one of those casts targets
//
// This is a SEPARATE type rule from t1 because `void` is not a Rust type.
// t1 maps it to Option<Value<::libc::c_void>> in the unsafe model and
// Option<Value<()>> in the refcount model, and neither survives contact with
// the code around it: measured on a 7-line probe, the refcount model emits
// `Ptr<()>` where a `&AnyPtr` is wanted and fails with two E0308s at
// `frame.ptr_.get()` and at `memcpy(.., frame.ptr_.get(), n)`. A `()` element
// is also a ZST, which `Ptr::reinterpret_cast` explicitly panics on
// ("cannot reinterpret_cast to zero-sized type"), so even the version that
// compiled could not have run.
//
// libcc2rs already has the right type for a type-erased pointer, and it is
// what the converter ALREADY uses for a plain `void *` field: AnyPtr. It
// carries the element type it was built from inside an Rc<dyn ErasedPtr>, so
// `reinterpret_cast::<U>()` returns the original Ptr<U> when U matches and
// falls back to a byte view when it does not -- which is exactly what a
// `reinterpret_cast<T *>(frame.ptr_.get())` needs, and that raw-pointer form
// is how every reader in the tree actually gets at the bytes. Mapping
// shared_ptr<void> to AnyPtr therefore makes the round trip
// `shared_ptr<T[]> -> shared_ptr<void> -> T *` type-check AND preserve bytes;
// verified end to end, including a 15-byte memcpy read back element by
// element. (The shared_ptr-to-shared_ptr cast back out is a different matter
// -- see the note at the end of this file.)
//
// What is knowingly given up: AnyPtr is a raw handle, so it does not own the
// buffer and does not keep it alive. In C++ the shared_ptr<void> stored in
// ProgramFrame::ptr_ is the LAST owner of the array -- the local
// shared_ptr<T[]> goes out of scope one line later. Modelling that honestly
// would need an owning erased handle in libcc2rs, which does not exist. The
// rules below therefore LEAK the array (std::mem::forget / Ptr::alloc_array's
// already-leaked Rc) instead of dropping it: a leak is a resource bug, a
// dangling read is silent wrongness, and the playbook ranks silent wrongness
// worse. This is called out in tgt_unsafe.rs at f37 so the next reader does
// not mistake the leak for an accident.
// ---------------------------------------------------------------------------

using t4 = std::shared_ptr<void>;

// ---------------------------------------------------------------------------
// std::make_shared
//
// Rust has no variadic generics, so (like make_unique) we write specialized
// versions for 0, 1 and 2 arguments. Each argument count needs one rule per
// value category because `Args&&...` deduces a different signature for
// rvalues, lvalues and const lvalues.
//
// Caveat on the two-argument form (f5/f6/f7): a rule body cannot call an
// arbitrary C++ constructor, so the target builds T1 with `From<(T2, T3)>`.
// That is right for types that have such an impl and a plain Rust type error
// for those that do not -- cpp2rust does not (yet) emit From impls for
// translated constructors.
// ---------------------------------------------------------------------------

template <typename T1> std::shared_ptr<T1> f1() {
  return std::make_shared<T1>();
}

template <typename T1, typename T2> std::shared_ptr<T1> f2(T2 &&a0) {
  return std::make_shared<T1>(std::move(a0));
}

template <typename T1, typename T2> std::shared_ptr<T1> f3(T2 &a0) {
  return std::make_shared<T1>(a0);
}

template <typename T1, typename T2> std::shared_ptr<T1> f4(const T2 &a0) {
  return std::make_shared<T1>(a0);
}

template <typename T1, typename T2, typename T3>
std::shared_ptr<T1> f5(T2 &&a0, T3 &&a1) {
  return std::make_shared<T1>(std::move(a0), std::move(a1));
}

template <typename T1, typename T2, typename T3>
std::shared_ptr<T1> f6(T2 &a0, T3 &a1) {
  return std::make_shared<T1>(a0, a1);
}

template <typename T1, typename T2, typename T3>
std::shared_ptr<T1> f7(const T2 &a0, const T3 &a1) {
  return std::make_shared<T1>(a0, a1);
}

// ---------------------------------------------------------------------------
// std::shared_ptr construction
// ---------------------------------------------------------------------------

template <typename T1> std::shared_ptr<T1> f8() {
  return std::shared_ptr<T1>();
}

template <typename T1> std::shared_ptr<T1> f9(T1 *a0) {
  return std::shared_ptr<T1>(a0);
}

template <typename T1>
std::shared_ptr<T1> f10(const std::shared_ptr<T1> &a0) {
  return std::shared_ptr<T1>(a0);
}

template <typename T1> std::shared_ptr<T1> f11(std::shared_ptr<T1> &&a0) {
  return std::shared_ptr<T1>(std::move(a0));
}

// ---------------------------------------------------------------------------
// std::shared_ptr assignment
// ---------------------------------------------------------------------------

template <typename T1>
std::shared_ptr<T1> &f12(std::shared_ptr<T1> &dst,
                         const std::shared_ptr<T1> &src) {
  return dst.operator=(src);
}

template <typename T1>
std::shared_ptr<T1> &f13(std::shared_ptr<T1> &dst, std::shared_ptr<T1> &&src) {
  return dst.operator=(std::move(src));
}

// ---------------------------------------------------------------------------
// std::shared_ptr observers
// ---------------------------------------------------------------------------

template <typename T1> T1 &f14(const std::shared_ptr<T1> &a0) {
  return a0.operator*();
}

template <typename T1> T1 *f15(const std::shared_ptr<T1> &a0) {
  return a0.operator->();
}

template <typename T1> T1 *f16(const std::shared_ptr<T1> &a0) {
  return a0.get();
}

template <typename T1> long f17(const std::shared_ptr<T1> &a0) {
  return a0.use_count();
}

template <typename T1> bool f18(const std::shared_ptr<T1> &a0) {
  return a0.operator bool();
}

// ---------------------------------------------------------------------------
// std::shared_ptr modifiers
// ---------------------------------------------------------------------------

template <typename T1> void f19(std::shared_ptr<T1> &a0) { return a0.reset(); }

template <typename T1> void f20(std::shared_ptr<T1> &a0, T1 *a1) {
  return a0.reset(a1);
}

// ---------------------------------------------------------------------------
// std::shared_ptr comparison
// ---------------------------------------------------------------------------

template <typename T1>
bool f21(const std::shared_ptr<T1> &a0, const std::shared_ptr<T1> &a1) {
  return operator==(a0, a1);
}

template <typename T1>
bool f22(const std::shared_ptr<T1> &a0, const std::shared_ptr<T1> &a1) {
  return operator!=(a0, a1);
}

template <typename T1>
bool f23(const std::shared_ptr<T1> &a0, std::nullptr_t a1) {
  return operator==(a0, a1);
}

template <typename T1>
bool f24(const std::shared_ptr<T1> &a0, std::nullptr_t a1) {
  return operator!=(a0, a1);
}

// ---------------------------------------------------------------------------
// std::weak_ptr
// ---------------------------------------------------------------------------

template <typename T1> std::weak_ptr<T1> f25() { return std::weak_ptr<T1>(); }

template <typename T1> std::weak_ptr<T1> f26(const std::shared_ptr<T1> &a0) {
  return std::weak_ptr<T1>(a0);
}

template <typename T1> std::weak_ptr<T1> f27(const std::weak_ptr<T1> &a0) {
  return std::weak_ptr<T1>(a0);
}

template <typename T1>
std::weak_ptr<T1> &f28(std::weak_ptr<T1> &dst, const std::shared_ptr<T1> &src) {
  return dst.operator=(src);
}

template <typename T1>
std::weak_ptr<T1> &f29(std::weak_ptr<T1> &dst, const std::weak_ptr<T1> &src) {
  return dst.operator=(src);
}

template <typename T1> std::shared_ptr<T1> f30(const std::weak_ptr<T1> &a0) {
  return a0.lock();
}

template <typename T1> bool f31(const std::weak_ptr<T1> &a0) {
  return a0.expired();
}

template <typename T1> long f32(const std::weak_ptr<T1> &a0) {
  return a0.use_count();
}

template <typename T1> void f33(std::weak_ptr<T1> &a0) { return a0.reset(); }

// ---------------------------------------------------------------------------
// std::shared_ptr<T[]> -- construction, observers, and the cast to void
//
// The four scheduler sites are all the same three lines:
//
//   std::shared_ptr<T[]> b(new T[n]);          <- f34
//   memcpy/snprintf(b.get(), .., n);           <- f36
//   frame.ptr_ = std::reinterpret_pointer_cast<void>(b);   <- f37
//
// f34 is the ctor from a raw pointer. It is the ARRAY overload, distinct from
// f9's `shared_ptr<T1>(T1 *)` even though the C++ parameter type is spelled
// identically -- the receiver differs, so the printed signature differs
// ("std::shared_ptr<T1[]>::shared_ptr(T1 *)") and the two do not collide.
// This mirrors rules/unique_ptr f3 vs f4 exactly.
// ---------------------------------------------------------------------------

template <typename T1> std::shared_ptr<T1[]> f34(T1 *a0) {
  return std::shared_ptr<T1[]>(a0);
}

template <typename T1> std::shared_ptr<T1[]> f35() {
  return std::shared_ptr<T1[]>();
}

// `.get()` on the array form. Returns a pointer to the FIRST element, which
// is what snprintf and memcpy then write through -- not a pointer to the
// handle, which is the bug the carray refusal predicted.
template <typename T1> T1 *f36(const std::shared_ptr<T1[]> &a0) {
  return a0.get();
}

template <typename T1>
std::shared_ptr<void> f37(const std::shared_ptr<T1[]> &a0) {
  return std::reinterpret_pointer_cast<void>(a0);
}

// NOT ADDED: `T1 &std::shared_ptr<T1[]>::operator[](std::ptrdiff_t)`.
//
// It is writable -- `a0.as_pointer().offset(i)` in the refcount model, and an
// index into the Box<[T1]> in the unsafe one -- and with t3 in place it would
// no longer produce the broken lowering rules/carray warned about. It is left
// out because no site in the scheduler scope (dcg/ ddc/ dsc/ dbo/) uses it:
// the five sites in dt_src that do are dip.cpp, deeprt_fold_init.cpp and
// dsm.cpp, all outside it. A rule with no site to verify it against could
// only be checked by inspection, and this file's history is a list of rules
// that looked right and were silently wrong. When a scope that needs it is
// translated, add it WITH a probe that reads back the value it wrote.

// ---------------------------------------------------------------------------
// std::shared_ptr<void> -- the members reachable on ProgramFrame::ptr_
//
// Measured over the whole tree, the surface on a shared_ptr<void> is: assign
// from the cast (f42/f43), `.get()` (f38), compare against nullptr (f40/f41),
// and default-construct as a struct member (f39). No operator*, no
// operator->, no use_count, no reset -- consistent with it being a type-erased
// handoff slot and nothing else.
//
// f40/f41 are the free operator==/!= against std::nullptr_t, not the
// shared_ptr-to-shared_ptr forms: `frame.ptr_ == nullptr` is the only
// comparison the tree performs on one of these.
// ---------------------------------------------------------------------------

void *f38(const std::shared_ptr<void> &a0) { return a0.get(); }

std::shared_ptr<void> f39() { return std::shared_ptr<void>(); }

bool f40(const std::shared_ptr<void> &a0, std::nullptr_t a1) {
  return operator==(a0, a1);
}

bool f41(const std::shared_ptr<void> &a0, std::nullptr_t a1) {
  return operator!=(a0, a1);
}

std::shared_ptr<void> &f42(std::shared_ptr<void> &dst,
                           const std::shared_ptr<void> &src) {
  return dst.operator=(src);
}

std::shared_ptr<void> &f43(std::shared_ptr<void> &dst,
                           std::shared_ptr<void> &&src) {
  return dst.operator=(std::move(src));
}

// NOT ADDED, and this is a real design boundary rather than laziness:
// `std::shared_ptr<T1> std::reinterpret_pointer_cast(const
// std::shared_ptr<void> &)` -- the cast back OUT of void, which
// dsc/superdsc.cpp:20 does to read a senulator_prog header out of the frame.
//
// The problem is not the cast, it is the RESULT TYPE. The result is a
// shared_ptr<T1>, which t1 maps to Option<Value<T1>> -- an OWNING Rc that
// aliases the same object. Neither model can produce one from a type-erased
// handle:
//   * refcount: AnyPtr::reinterpret_cast gives a Ptr<T1>, and turning that
//     into an Rc means to_owned_opt(), which panics outright on a
//     Reinterpreted pointer ("Can't own a reinterpreted pointer") -- and for
//     a StackArray it would be a double-owner.
//   * unsafe: an Rc cannot be conjured from a *mut c_void at all.
//
// The only body that would type-check reads the bytes and builds a FRESH
// Rc -- a deep copy. That is correct for superdsc.cpp:20, which only calls
// `->isR5()`, and silently wrong for any site that writes through the result,
// because the write would land in the copy and never reach the frame. A rule
// that is right for one caller and silently wrong for the next is the exact
// failure class this project ranks worst, so it is not written.
//
// Left out, the site translates and then fails to COMPILE on an undefined
// `reinterpret_pointer_cast_N` -- loud, at the right place, naming the
// construct. That is the acceptable outcome. Making it work needs one of two
// decisions nobody has made: an owning type-erased handle in libcc2rs
// (Rc<dyn Any> plus a downcast), or mapping shared_ptr<T> to a pointer rather
// than an Rc, which changes every existing shared_ptr site.
//
// Note this is NOT one of the four scheduler sites this sub-model exists for:
// all four cast INTO void (f37), which is the direction that works.
