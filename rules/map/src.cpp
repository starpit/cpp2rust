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
typename std::map<T1, T2>::const_iterator f27(const std::map<T1, T2> &o) {
  return o.begin();
}

template <typename T1, typename T2>
bool f28(typename std::map<T1, T2>::const_iterator a,
         typename std::map<T1, T2>::const_iterator b) {
  return operator!=(a, b);
}

template <typename T1, typename T2>
bool f29(const std::map<T1, T2> &a, const std::map<T1, T2> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
bool f30(const std::map<T1, T2> &a, const std::map<T1, T2> &b) {
  return operator!=(a, b);
}

template <typename T1, typename T2>
typename std::map<T1, T2>::const_iterator &
f31(typename std::map<T1, T2>::const_iterator &it) {
  return it.operator++();
}

template <typename T1, typename T2>
typename std::map<T1, T2>::const_iterator
f32(typename std::map<T1, T2>::const_iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1, typename T2>
typename std::map<T1, T2>::iterator &
f33(typename std::map<T1, T2>::iterator &it) {
  return it.operator++();
}

template <typename T1, typename T2>
typename std::map<T1, T2>::iterator
f34(typename std::map<T1, T2>::iterator a0, int a1) {
  return a0.operator++(a1);
}

// f35 -- the INITIALIZER-LIST CONSTRUCTOR, i.e. the NSDMI form
// `std::map<K,V> m = {{k, v}, ...}`.  MEASURED from the fallback readback:
//   std_map_..._new_1({ vec![...] }, None,)
// so (a) the braced init list already lowers to a `vec![...]`, and (b) the
// COLLAPSED DEFAULTED COMPARATOR shows up as a literal `None` SECOND argument --
// the key must therefore carry the comparator parameter and must NOT be shaped
// as a 1-arg ctor.  `const std::initializer_list<...> &` spelling copied from
// rules/vector's f36.
// The body uses `.rev()` before `collect()` because `collect` is LAST-duplicate-
// wins while std::map's init-list insert keeps the FIRST; reversing makes the two
// agree.  `a0` occurs EXACTLY ONCE, which matters: a rule body is inlined as one
// expression, so every `aN` occurrence re-evaluates that argument -- and the
// motivating site (dsc/designSpaceConfig.h:358) has 240 elements.
template <typename T1, typename T2>
std::map<T1, T2> f35(const std::initializer_list<std::pair<const T1, T2>> &a0,
                     const std::less<T1> &a1) {
  return std::map<T1, T2>(a0, a1);
}

// ---------------------------------------------------------------------------
// SWALLOW FIX -- /home/agent/work/SWALLOW-AUDIT.md row #2.
// `GetTypeMapKey` (mapper.cpp:115) truncates at the first `<`, so arity is NOT
// part of the bucket key, and in `matchTemplate` a same-depth comma is not a
// delimiter -- only the next literal run is.  So t1's T2 (nextLit `>`) binds the
// WHOLE comma-joined tail of a 3-ary instantiation:
//   LLVM ERROR: unsupported unmapped type `std::vector<bool>, std::greater<void>`
//   LLVM ERROR: unsupported unmapped type `std::pair<std::optional<long>, bool *>,
//                                          (lambda at .../CFGSimplificationSentientLevel.cpp)`
// SWALLOW-SAFETY: t1 keeps every 2-ary instantiation it serves today, because
// against `std::map<int, float>` this key's T2 has nextLit `", "` and
// findNextLiteralSameDepth finds no further same-depth `", "` -> npos -> NO
// match at all.  Where both match (3-ary) this src is longer (20 vs 16 chars)
// and search()'s tie-break (mapper.cpp:430-437) prefers it.  T3 is a free
// parameter of THIS rule's own template, so SuppressDefaultTemplateArgs cannot
// elide it (the mechanism that made a `DenseMapInfo<T1, void>` spelling a dead
// duplicate).
// ⚠️ FIDELITY CAVEAT, reported and not hidden: the comparator T3 is DROPPED by
// the model, and a 3-ary std::map is only ever written BECAUSE the comparator is
// non-default -- both observed instantiations use `std::greater<void>` / a
// lambda, i.e. an order that BTreeMap's Ord does not reproduce.  No member of a
// 3-ary map is keyed (every f-key spells a 2-ary receiver and swallows the same
// way), so keyed operations still fail loudly; the residual exposure is a
// converter-lowered range-for, which would iterate ASCENDING.  See the report.
template <typename T1, typename T2, typename T3>
using t4 = std::map<T1, T2, T3>;
