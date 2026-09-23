// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <map>
#include <utility>

template <typename T1, typename T2> using t1 = std::map<T1, T2>;

template <typename T1, typename T2>
using t2 = typename std::map<T1, T2>::const_iterator;

template <typename T1, typename T2>
using t3 = typename std::map<T1, T2>::iterator;

template <typename T1, typename T2> T2 &f1(std::map<T1, T2> &o, const T1 &key) {
  return o.operator[](key);
}

template <typename T1, typename T2> std::size_t f2(const std::map<T1, T2> &o) {
  return o.size();
}

template <typename T1, typename T2>
typename std::map<T1, T2>::iterator f3(std::map<T1, T2> &o,
                                       typename std::map<T1, T2>::iterator it) {
  return o.erase(it);
}

template <typename T1, typename T2> std::map<T1, T2> f5() {
  return std::map<T1, T2>();
}

template <typename T1, typename T2>
std::map<T1, T2> f6(const std::map<T1, T2> &&o) {
  return std::map<T1, T2>(std::move(o));
}

template <typename T1, typename T2> T2 &f7(std::map<T1, T2> &o, const T1 &key) {
  return o.at(key);
}

template <typename T1, typename T2> T2 &f8(std::map<T1, T2> &o, T1 &&key) {
  return o.operator[](std::move(key));
}

template <typename T1, typename T2>
typename std::map<T1, T2>::const_iterator f9(const std::map<T1, T2> &o) {
  return o.end();
}

template <typename T1, typename T2>
typename std::map<T1, T2>::iterator f10(std::map<T1, T2> &o, const T1 &key) {
  return o.find(key);
}

template <typename T1, typename T2>
bool f11(typename std::map<T1, T2>::iterator a,
         typename std::map<T1, T2>::iterator b) {
  return operator!=(a, b);
}

template <typename T1, typename T2>
typename std::map<T1, T2>::iterator f12(std::map<T1, T2> &o) {
  return o.begin();
}

template <typename T1, typename T2>
bool f13(typename std::map<T1, T2>::const_iterator a,
         typename std::map<T1, T2>::const_iterator b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
typename std::map<T1, T2>::iterator f14(std::map<T1, T2> &o) {
  return o.end();
}

template <typename T1, typename T2>
const T2 &f15(const std::map<T1, T2> &o, const T1 &key) {
  return o.at(key);
}

template <typename T1, typename T2>
bool f16(typename std::map<T1, T2>::iterator a,
         typename std::map<T1, T2>::iterator b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
typename std::map<T1, T2>::const_iterator f17(const std::map<T1, T2> &o,
                                              const T1 &key) {
  return o.find(key);
}

template <typename T1, typename T2>
typename std::map<T1, T2>::const_iterator
f19(const typename std::map<T1, T2>::iterator &it) {
  return typename std::map<T1, T2>::const_iterator(it);
}

template <typename T1, typename T2>
const T1 &f20(typename std::map<T1, T2>::const_iterator it) {
  return it->first;
}

template <typename T1, typename T2>
const T2 &f21(typename std::map<T1, T2>::const_iterator it) {
  return it->second;
}

template <typename T1, typename T2>
const T1 &f22(typename std::map<T1, T2>::iterator it) {
  return it->first;
}

template <typename T1, typename T2>
T2 &f23(typename std::map<T1, T2>::iterator it) {
  return it->second;
}

template <typename T1, typename T2> std::map<T1, T2> f24(std::map<T1, T2> &&o) {
  return std::map<T1, T2>(std::move(o));
}

template <typename T1, typename T2>
std::map<T1, T2> &f25(std::map<T1, T2> &dst, std::map<T1, T2> &&src) {
  return dst.operator=(std::move(src));
}

template <typename T1, typename T2>
std::map<T1, T2> &f26(std::map<T1, T2> &dst, const std::map<T1, T2> &src) {
  return dst.operator=(src);
}

template <typename T1, typename T2>
bool f27(typename std::map<T1, T2>::const_iterator a,
         typename std::map<T1, T2>::const_iterator b) {
  return operator!=(a, b);
}

template <typename T1, typename T2>
typename std::map<T1, T2>::iterator &
f28(typename std::map<T1, T2>::iterator &it) {
  return it.operator++();
}

template <typename T1, typename T2>
typename std::map<T1, T2>::const_iterator &
f29(typename std::map<T1, T2>::const_iterator &it) {
  return it.operator++();
}

template <typename T1, typename T2>
bool f30(const std::map<T1, T2> &a, const std::map<T1, T2> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
bool f31(const std::map<T1, T2> &a, const std::map<T1, T2> &b) {
  return operator!=(a, b);
}

// The initializer-list constructor: `std::map<K, V> m = {{k, v}, ...};`.
// libc++ declares it as
//     map(initializer_list<value_type>, const key_compare& = key_compare())
// so the resolved rule carries the comparator parameter too, and the converter
// passes `None` for it.  std::pair<const T1, T2> appears only INSIDE this
// signature -- no std::pair<const T1, T2> TYPE rule is introduced, so the
// `const T *` ambiguity that killed the earlier attempt at that family does
// not arise here: T1 and T2 are already pinned by std::map<T1, T2>.
template <typename T1, typename T2>
std::map<T1, T2> f32(std::initializer_list<std::pair<const T1, T2>> a0) {
  return std::map<T1, T2>(a0);
}

// ---------------------------------------------------------------------------
// The POSTFIX ++ on a map iterator: `it++`, as opposed to f28/f29's `++it`.
//
// libc++ prints the two distinctly -- the prefix returns `__map_iterator &`
// and the postfix returns `__map_iterator` and takes an int -- so f28/f29 do
// not cover the postfix form, and `it++` in dsc/dims.cpp:736 died at
// converter.cpp:3455 with "unsupported CXXOperatorCallExpr: ++".
//
// Postfix semantics are the ones the C++ standard specifies and the call site
// relies on: ADVANCE the receiver, RETURN the value it had before. Getting
// that backwards would be silent -- the loop would still terminate, on the
// wrong element. libcc2rs already implements exactly this as
// `PostfixInc for MapIter` (iterators.rs), which both models reuse; the
// prefix rules call `inc()` then `clone()`, so the two are not interchangeable.
// ---------------------------------------------------------------------------

template <typename T1, typename T2>
typename std::map<T1, T2>::iterator
f33(typename std::map<T1, T2>::iterator &it, int a1) {
  return it.operator++(a1);
}

template <typename T1, typename T2>
typename std::map<T1, T2>::const_iterator
f34(typename std::map<T1, T2>::const_iterator &it, int a1) {
  return it.operator++(a1);
}

// ---------------------------------------------------------------------------
// The four RELATIONAL operators on std::map.
//
// f30/f31 already cover == and !=; `<`, `<=`, `>` and `>=` had no rule, and
// `new_sp.bigDimToSize_ >= new_sp.dimToSize_` in
// dcg/dcg_fe/pcfg_gen/stcdpOp.cpp:90 (both std::map<std::string, double>,
// inside a DT_CHECK) died at converter.cpp:3455.
//
// This is only sound because Rust's derived Ord on BTreeMap agrees with C++'s
// std::lexicographical_compare over the (key, value) SEQUENCE, which is what
// std::map's relational operators are specified to do. That was not assumed --
// it was measured: six discriminating pairs (same keys/different values, one
// map a strict PREFIX of another, a differing KEY, and equal maps) give
// identical answers from clang-compiled std::map and from BTreeMap:
// `10 01 01 11 01 01` for both.
//
// NOTE the receiver, and why `>=` here is not the `>=` a scalar would get: the
// elements are compared through Box<T2>/Value<T2>, whose Ord forwards to T2,
// so the comparison reaches the mapped values rather than the handles.
// ---------------------------------------------------------------------------

template <typename T1, typename T2>
bool f35(const std::map<T1, T2> &a, const std::map<T1, T2> &b) {
  return operator<(a, b);
}

template <typename T1, typename T2>
bool f36(const std::map<T1, T2> &a, const std::map<T1, T2> &b) {
  return operator<=(a, b);
}

template <typename T1, typename T2>
bool f37(const std::map<T1, T2> &a, const std::map<T1, T2> &b) {
  return operator>(a, b);
}

template <typename T1, typename T2>
bool f38(const std::map<T1, T2> &a, const std::map<T1, T2> &b) {
  return operator>=(a, b);
}

// std::map::emplace.  Had no rule at all, so `m.emplace(k, v)` fell through to
// a mangled `BTreeMap::emplace` that does not exist -- rc=0 from the
// translator and E0599 from rustc.  Modelled exactly as rules/unordered_map
// f39/f40 already model the same two spellings.  emplace KEEPS THE INCUMBENT on
// a duplicate key, which BTreeMap::insert does not: the same discrepancy
// rules/set was fixed for in 8e0a2d6, so the contains_key guard is load
// bearing, not defensive.
template <typename T1, typename T2>
std::pair<typename std::map<T1, T2>::iterator, bool>
f39(std::map<T1, T2> &o, T1 &&key, T2 &&value) {
  return o.emplace(std::move(key), std::move(value));
}

template <typename T1, typename T2>
std::pair<typename std::map<T1, T2>::iterator, bool>
f40(std::map<T1, T2> &o, const T1 &key, const T2 &value) {
  return o.emplace(key, value);
}

// The NON-CONST lvalue spelling.  f39 covers `emplace(T1 &&, T2 &&)` and f40
// `emplace(const T1 &, const T2 &)`, but clang resolves `m.emplace(k, v)` for
// two plain lvalues to `emplace(T1 &, T2 &)`, which matches neither -- and that
// is the spelling dsc/dsc2.cpp:179 and :255 actually use.  Verified with the
// --verbose oracle, which prints exactly this signature with an empty result.
// Same COPY semantics as f40: emplace forwards to T's copy constructor here.
template <typename T1, typename T2>
std::pair<typename std::map<T1, T2>::iterator, bool>
f41(std::map<T1, T2> &o, T1 &key, T2 &value) {
  return o.emplace(key, value);
}

// ---------------------------------------------------------------------------
// count, erase-by-key, and empty.  All three had NO rule at all on std::map,
// even though rules/unordered_map has carried them since it was written
// (f31, f34, f4 there).  The converter emitted `BTreeMap::count`,
// `BTreeMap::erase_<mangled>` and `BTreeMap::empty`, none of which exist:
// translator rc=0, then three E0599s from rustc.  Modelled exactly as
// rules/unordered_map already models the same three members, since both
// modules share the same BTreeMap representation.
//
// The RETURN TYPES are the part that is easy to get silently wrong, so they
// were read off the standard and confirmed against clang-compiled C++ rather
// than assumed from the member name:
//
//   * std::map::count(k) is NOT multimap's -- a map holds at most one element
//     per key, so it answers 0 or 1 and never more.  `contains_key` mapped to
//     1_usize / 0_usize is therefore exact, not an approximation.
//   * std::map::erase(k) returns the NUMBER ERASED (0 or 1), not an iterator.
//     The iterator-returning erase is the one that takes an iterator, and that
//     is f3, a different overload with a different signature.  Returning an
//     iterator here would have been silently wrong at every arithmetic use.
//
// Both are read-only about the missing-key case, which the probe covers:
// count(absent)=0 and erase(absent)=0, with the map otherwise unchanged.
// ---------------------------------------------------------------------------

template <typename T1, typename T2>
std::size_t f42(const std::map<T1, T2> &o, const T1 &key) {
  return o.count(key);
}

template <typename T1, typename T2>
std::size_t f43(std::map<T1, T2> &o, const T1 &key) {
  return o.erase(key);
}

template <typename T1, typename T2> bool f44(const std::map<T1, T2> &o) {
  return o.empty();
}

// ---------------------------------------------------------------------------
// insert and swap.  Added ON EVIDENCE of use, not speculatively: the six
// std::map::insert sites and the one swap site were found by resolving the
// declared type of every receiver in dcg/ ddc/ dsc/ dbo/, and each produced a
// real E0599 before this.  `dsc/pcfg.cpp:3968` is the braced form
// (`regs.insert({std::stoi(reg.first), ids})`), and
// `dsc/sdsc-perfmodel/perfmodel.cpp:1000` and `:1006` are the range form
// (`dtNode->dtInfo.insert(other.begin(), other.end())`).
//
// NOTE the key type on f45: the braced argument materialises a
// std::pair<const T1, T2> -- value_type, with the const -- not a
// std::pair<T1, T2>, so this is the `insert(value_type &&)` overload rather
// than the forwarding one rules/unordered_map's f41 covers.  As in f32, the
// `const T1` spelling appears only INSIDE a std::map<T1, T2> signature, so T1
// and T2 are already pinned and the `const T *` ambiguity that killed an
// earlier standalone std::pair<const T1, T2> type rule cannot arise.
//
// DUPLICATE-KEY SEMANTICS, the part that would have been silent: C++ insert
// KEEPS THE INCUMBENT and reports false, whereas BTreeMap::insert OVERWRITES
// and returns the old value.  That holds for the RANGE form too -- every
// element already present is skipped, not assigned -- which is why f46's body
// tests contains_key per element rather than calling extend() or append().
// Measured against clang-compiled C++: inserting {"q":2.5,"p":9.9} into
// {"p":1.5} leaves p at 1.5, and the probe asserts exactly that.
//
// f46 returns void: the two-iterator insert has no return value in C++, unlike
// the single-element form.
// ---------------------------------------------------------------------------

template <typename T1, typename T2>
std::pair<typename std::map<T1, T2>::iterator, bool>
f45(std::map<T1, T2> &o, std::pair<const T1, T2> &&v) {
  return o.insert(std::move(v));
}

template <typename T1, typename T2>
void f46(std::map<T1, T2> &o, typename std::map<T1, T2>::iterator first,
         typename std::map<T1, T2>::iterator last) {
  return o.insert(first, last);
}

template <typename T1, typename T2>
void f47(std::map<T1, T2> &o, std::map<T1, T2> &a0) {
  return o.swap(a0);
}

// std::map::clear.  This one is NOT an E0599, which is why it hid: BTreeMap has
// a `clear` of its own, so with no rule the call fell through to it and the
// unsafe model happened to compile.  The refcount model did not -- the receiver
// is emitted as `(*a.borrow()).clear()`, an immutable borrow, giving
// E0596 "cannot borrow data in dereference of Ref<..> as mutable".  Confirmed
// pre-existing against the pristine rules, not introduced by the rules above.
//
// A rule is the fix because it is what makes the receiver arrive as a Ptr, so
// the body can take the mutable borrow through with_mut -- exactly what
// rules/unordered_map's f30 already does for the same member.  33 sites in
// dcg/ ddc/ dsc/ dbo/ (e.g. dsc/dataOpDsc.cpp:612, ddc/ddc.h:530).
template <typename T1, typename T2> void f48(std::map<T1, T2> &o) {
  return o.clear();
}

// ---------------------------------------------------------------------------
// try_emplace and insert_or_assign.  Added on evidence: ten try_emplace sites
// (dsc/dsc2Pcfg.cpp:49, :79, :83, :2250, :2301, dsc/superdsc.cpp:1270, :1285,
// :1291 -- receivers std::map<int, std::map<SenComponents, uint64_t>> and
// std::map<int, SenPcfg>) and insert_or_assign at dsc/superdsc.cpp:812 and
// ddc/ddcv1.cpp:2770/:2773.  Both were nonexistent BTreeMap methods before:
// translator rc=0, then E0599.
//
// THESE TWO ARE OPPOSITES ON A DUPLICATE KEY AND THE DIFFERENCE IS SILENT.
// Measured against clang-compiled C++ rather than assumed:
//
//     m[1] = 10;
//     m.insert_or_assign(1, 111) -> reports false, and ASSIGNS: m.at(1) == 111
//     m.try_emplace(1, 999)      -> reports false, and DOES NOT: stays 111
//
// So insert_or_assign is the one operation in this file whose body must NOT be
// guarded by contains_key -- a plain BTreeMap::insert is exactly right, and
// adding the guard would have been the silent bug.  try_emplace is the reverse
// and does need the guard.  Both report `false` for an existing key, so the
// bool alone does not distinguish them and a probe that only checked the bool
// would pass either way; the probes therefore read the VALUE back.
//
// f51 is the one-argument try_emplace (`pcfgMap.try_emplace(c)`), which is the
// form every dt_src site actually uses: it default-constructs the mapped value,
// and on a duplicate must leave the incumbent untouched -- the probe mutates
// the incumbent between two calls and asserts the mutation survives.
// ---------------------------------------------------------------------------

template <typename T1, typename T2>
std::pair<typename std::map<T1, T2>::iterator, bool>
f49(std::map<T1, T2> &o, const T1 &key, T2 &value) {
  return o.insert_or_assign(key, value);
}

template <typename T1, typename T2>
std::pair<typename std::map<T1, T2>::iterator, bool>
f50(std::map<T1, T2> &o, const T1 &key, T2 &value) {
  return o.try_emplace(key, value);
}

template <typename T1, typename T2>
std::pair<typename std::map<T1, T2>::iterator, bool>
f51(std::map<T1, T2> &o, const T1 &key) {
  return o.try_emplace(key);
}

// insert of a std::pair<T1, T2> -- WITHOUT the const on the key, so it binds to
// the forwarding insert() overload rather than to f45's insert(value_type &&).
// This is what `.insert(std::make_pair(k, v))` spells, and it is used at
// ddc/transformations/automatic_shuffle/shuffle.cpp:723, :932 and :949
// (receivers std::map<AbstractLayout, std::shared_ptr<GraphNode>> and
// std::map<uint32_t, EdgeType>).  rules/unordered_map carries the identical
// rule as its f41; std::map had no equivalent, so those three sites emitted a
// nonexistent BTreeMap method.  Same keep-the-incumbent guard as f45.
template <typename T1, typename T2>
std::pair<typename std::map<T1, T2>::iterator, bool>
f52(std::map<T1, T2> &o, std::pair<T1, T2> &&v) {
  return o.insert(std::move(v));
}
