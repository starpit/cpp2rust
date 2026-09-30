// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <deque>
#include <vector>

template <typename T, typename A> using Init = A;

template <typename T1> using t1 = std::deque<T1>;

template <typename T1> T1 &f1(std::deque<T1> &o) { return o.back(); }

template <typename T1> T1 &f2(std::deque<T1> &o) { return o.front(); }

template <typename T1> bool f3(const std::deque<T1> &o) { return o.empty(); }

template <typename T1> void f4(std::deque<T1> &o, T1 &&value) {
  return o.push_back(std::move(value));
}

template <typename T1> void f5(std::deque<T1> &o) { return o.pop_front(); }

template <typename T1>
void f7(std::deque<std::vector<T1>> &o, const std::vector<T1> &value) {
  return o.push_back(value);
}

template <typename T1> std::deque<T1> f8(const std::deque<T1> &o) {
  return std::deque<T1>(o);
}

template <typename T1> std::deque<T1> f9(std::deque<T1> &&o) {
  return std::deque<T1>(std::move(o));
}

template <typename T1>
std::deque<T1> &f10(std::deque<T1> &dst, const std::deque<T1> &src) {
  return dst.operator=(src);
}

template <typename T1>
std::deque<T1> &f11(std::deque<T1> &dst, std::deque<T1> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1, typename... Args>
T1 &f12(std::deque<T1> &o, Init<T1, Args> &&...args) {
  return o.emplace_back(std::forward<Args>(args)...);
}

template <typename T1, typename... Args>
std::vector<T1> &f13(std::deque<std::vector<T1>> &o,
                     Init<std::vector<T1>, Args> &&...args) {
  return o.emplace_back(std::forward<Args>(args)...);
}

template <typename T1> std::deque<T1> f14() { return std::deque<T1>(); }

// g419 (fixed 2026-09-28) -- the free comparison operators.  libc++ declares them
// in the INLINE namespace `std::__1`, and the converter searches for
// `bool std::__1::operator==(const std::deque<long> &, const std::deque<long> &)`
// (its own diagnostic prints that spelling).  The UNQUALIFIED call form below is
// what records that spelling: the `__1` component comes from the RESOLVED callee's
// qualified name, not from how the call is written, so writing `std::__1::` here is
// neither necessary nor possible (it aborts at cpp_rule_preprocessor.cpp:888,
// because the rule's own synthesized namespace has no `std::__1` to look into).
template <typename T1>
bool f15(const std::deque<T1> &a, const std::deque<T1> &b) {
  return operator==(a, b);
}

template <typename T1>
bool f16(const std::deque<T1> &a, const std::deque<T1> &b) {
  return operator!=(a, b);
}

// f17 -- push_back FOR AN LVALUE OF AN ARBITRARY ELEMENT TYPE.  f4 keys only the
// `T1 &&` overload and f7 only `std::deque<std::vector<T1>>`, so `d.push_back(x)`
// with `x` a named lvalue of any other element type was unmapped.
template <typename T1> void f17(std::deque<T1> &o, const T1 &value) {
  return o.push_back(value);
}

// ============================================================================
// g3108 -- `at` / `resize`, AND THE POINTER-ELEMENT `push_back` SIBLING.
//
// WHY THESE WERE MISSING.  `e67efaf6` fixed the RECEIVER of an unmapped mutating
// member call (23 sites in dxp__dxp_standalone.cpp stopped emitting the
// `Cpp2RustUnmappedExpr_*` sentinel) and said in its own message that the METHOD
// NAME was left on MapFunctionName's literal-C++ fallback.  MEASURED here with
// `cpp2rust -verbose | grep 'search expr'` on the goal TU, refcount leg, binary
// at e67efaf6 -- the converter searched for, and found NOTHING for:
//     long & std::deque<long>::at(unsigned long)                  -> None
//     const long & std::deque<long>::at(unsigned long) const      -> None
//     void std::deque<long>::resize(unsigned long)                -> None
//     void std::deque<FoldFunction<long> *>::push_back(FoldFunction<long> *const &)  -> None
//       (and the same for FoldFunction<int>, FoldFunction<std::vector<int>>,
//        FoldFunction<std::vector<long>>)
// while `long & std::vector<long>::at(unsigned long)` HIT `rules/vector`'s f7.
// So f18/f19/f20/f21 are plain missing keys and f22 is the const-placement
// dead-key class, argued below.
//
// ⛔ `at` RETURNS A REFERENCE AND THE CORPUS WRITES THROUGH IT.  The measured
// witness is `coordinates.at(ci).at(di) = rhs` in
// dsm/translators/multiAIUPerfToSgr/multiAIUPerfToSgr.cpp, over a
// `std::vector<std::deque<int64_t>>`.  A lowering that returned a VALUE or a
// `.clone()` would compile and then drop the write on a temporary -- silently, and
// invisibly to every census in this harness.  Both arms therefore return a
// WRITABLE handle: `*mut T1` under unsafe, `Ptr<T1>` under refcount.
//
// ⭐ THE DISCRIMINATOR FOR THE SHAPE IS `rules/vector`'s f7/f98, NOT its f50.
// f7 (`a0: Ptr<T1> -> Ptr<T1>`, body `a0.offset(a1 as isize)`) is the one that is
// actually exercised and correct: at line 4021 of the goal TU's refcount emission
// it produces `(myVec.decay() as Ptr<i64>).offset(idx).read()` -- ELEMENT stride,
// and `.read()` yields the element.  f50 declares the same C++ signature with
// `a0: Ptr<Vec<T1>>` and at line 32777 produces
// `(self.data_vec_.as_pointer() as Ptr<Vec<i64>>).offset(idx).read()` for a
// `std::vector<int64_t>` -- a `Vec<i64>`-STRIDE offset whose `.read()` has type
// `Vec<i64>` where an `i64` is wanted.  That is a pre-existing bug in f50, rowed
// separately; do not propagate it.  f19 below follows f98, not f50.
//
// ⭐ `resize` IS DELIBERATELY UNSAFE-ARM-ONLY, and that is not an omission.
// `ir_unsafe.json` is the unconditional base for both models
// (translation_rule.cpp:487-494), and `rules/vector`'s f15/f54/f74/f102 are
// unsafe-arm-only resize keys that demonstrably fire on the REFCOUNT leg: the goal
// TU's refcount emission carries 15 `.resize_with(` calls, spelled
// `X.with_mut(|__v: &mut Vec<..>| __v.resize_with(..))` for a `Ptr` receiver and
// `(*X.borrow_mut()).resize_with(..)` for a `Value` receiver.  The converter knows
// how to produce the `&mut Vec<T1>` receiver the unsafe signature asks for in both
// models, so a second arm would add nothing but a place to diverge.
//
// f18 -- at(size_type), NON-CONST.  The writable one.
template <typename T1> T1 &f18(std::deque<T1> &o, std::size_t idx) {
  return o.at(idx);
}

// f19 -- at(size_type) const.  28 of the goal TU's 51 unmapped
// at/resize/push_back occurrences are `at_usize_const`, so this is the larger
// half.  Shape copied from `rules/vector`'s f98 (`Ptr<T1>`), not f50.
template <typename T1>
const T1 &f19(const std::deque<T1> &o, std::size_t idx) {
  return o.at(idx);
}

// f20 -- resize(size_type).
template <typename T1> void f20(std::deque<T1> &o, std::size_t n) {
  return o.resize(n);
}

// f21 -- resize(size_type, const value_type &).  NOT searched for by the goal TU
// (only the 1-arg form is), but `perfDscToSdsc.cpp:3546` and `:3911` in the
// 266-file link unit write `fold_dim_indices.resize(n, 0)`, so the overload is
// live in the link unit.  `value_type` is spelled through the container so that
// `const long &` is clang's word and not mine -- the same reason
// `rules/vector`'s f54/f102 spell it that way.
template <typename T1>
void f21(std::deque<T1> &o, std::size_t n,
         const typename std::deque<T1>::value_type &value) {
  return o.resize(n, value);
}

// ⛔⛔ f22 WAS WRITTEN, MEASURED WRONG, AND DELIBERATELY REMOVED -- `push_back` FOR A
// POINTER ELEMENT TYPE IS **NOT WRITABLE IN THIS LAYER**.  Read this before adding it
// back; the key text records fine and the rules crate compiles, and it is still wrong.
//
// (1) THE MEASUREMENT.  `cpp2rust -verbose | grep 'search expr'` on
//     dxp/dxp_standalone.cpp, refcount leg, binary at e67efaf6: of 100 distinct
//     `at`/`resize`/`push_back` signatures the converter searched for, 19 returned
//     `result: None`, and every `push_back` / const-`at` among them has ` *const `
//     in it, e.g.
//        void std::deque<FoldFunction<long> *>::push_back(FoldFunction<long> *const &)
//     f17 records `void std::deque<T1>::push_back(const T1 &)`, whose literals include
//     `push_back(const `; clang writes the corpus's TOP-LEVEL CONST TO THE RIGHT OF THE
//     STAR, so that literal is absent and the match fails.  The negative half of the
//     same measurement is what names the class rather than guessing it: the `&&`
//     overloads HIT on the very same element types
//     (`void std::deque<FoldFunction<long> *>::push_back(FoldFunction<long> * &&)` ->
//     f4), so a pointer element type is not in itself a barrier.  It is the literal
//     `const ` immediately before a placeholder.  NOT the SuppressDefaultTemplateArgs
//     skew, NOT the operator>= angle-depth desync, NOT the matchTemplate swallow.
//
// (2) WHY THE OBVIOUS FIX IS WRONG.  The `rules/vector` f127..f136 technique -- write a
//     sibling over `std::deque<T1 *>` and spell the starred type through
//     `::const_reference` so clang prints ` *const &` for you -- makes the key MATCH.
//     But the sibling's `T1` is then the POINTEE, so the TARGET has to spell the
//     pointer itself, and the only spellings available to a rule are `*mut T1`
//     (unsafe) and `Ptr<T1>` (refcount).  The converter's own model of a C++ pointer
//     is `PtrDyn<dyn X_Virtual>` when the pointee is POLYMORPHIC and `Ptr<X>` when it
//     is not, and every corpus row here is polymorphic (`FoldFunction<T>` has virtual
//     members).  MEASURED with the project rustc 1.98.0 against the real libcc2rs
//     rlib (`verif/g3108/snip/t.rs`), positive and negative control in one file:
//        Vec<Ptr<Plain>>              + `as Ptr<Vec<Ptr<Plain>>>`  -> COMPILES
//        Vec<PtrDyn<dyn FF_Virtual>>  + `as Ptr<Vec<Ptr<FF>>>`     -> E0277,
//          "the trait bound Rc<RefCell<Vec<PtrDyn<dyn FF_Virtual>>>>:
//           AsPointer<Vec<Ptr<FF>>> is not satisfied"
//     So the key would emit, at all 9 goal-TU sites,
//     `(fifo.as_pointer() as Ptr<Vec<Ptr<FoldFunction_int_>>>).with_mut_ref(...)`
//     against a container the converter models as `Vec<PtrDyn<dyn
//     FoldFunction_int___Virtual>>`.  That is coverage-shaped and wrong, so per RULE 2
//     the key is left OUT.
//
// (3) THE DISCRIMINATOR -- where the same lowering IS correct, and why.  f4
//     (`push_back(T1 &&)`) and `rules/vector`'s f7 (`T1 & ...::at(unsigned long)`) both
//     fire on these very element types and are correct, because their `T1` binds to
//     the WHOLE POINTER TYPE and the converter substitutes ITS OWN model for it
//     (`PtrDyn<dyn ..._Virtual>`).  The pointee-placeholder siblings are correct only
//     where the pointee is non-polymorphic -- which is also a latent limit on the
//     already-landed `rules/vector` f127..f136, rowed separately.
//
// (4) THE UNBLOCKER, IN THE RIGHT LAYER -- and it is NOT the rules layer, because no
//     rule anywhere can name the converter's model of `T1 *` (grep: `PtrDyn` appears in
//     0 of 93 `rules/*/tgt_*.rs`).  Either:
//       (a) make `matchTemplate` (cpp_rule_preprocessor) normalise a top-level
//           `X *const &` parameter to the same placeholder shape as `const X &`, so
//           f17/f4 and vector's f21/f80, f50/f98 match pointer element types with `T1`
//           still bound to the POINTER; or
//       (b) make the converter substitute its OWN modelled element type for a
//           pointee-bound placeholder used in pointer position, instead of the rule's
//           literal `Ptr<T1>` / `*mut T1`.
//     (a) is the smaller change and fixes 16 of the 19 measured misses at once
//           (7 vector push_back, 4 deque push_back, 3 vector const-at, and the 2
//           `rules/map` `at(const X *const &)` rows).
