// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::unordered_map<K,V> -> HashMap<K,V>, std::unordered_set<K> -> HashSet<K>.
//
// SEMANTIC DIFFERENCE, NOT PAPERED OVER: std::map is ordered and BTreeMap matches it,
// but std::unordered_map / std::unordered_set have UNSPECIFIED iteration order and
// Rust's HashMap/HashSet order differs from libc++'s (and, being randomly seeded,
// differs between runs). Lookup, insertion, erase, size, count, contains and
// container equality (which is order-independent for unordered containers) are
// faithful. Any code that ITERATES an unordered container and depends on the order
// it observes is behaviour this mapping does NOT reproduce -- nor does C++ portably.
//
// std::hash<T> is deliberately LEFT UNMAPPED: the converter already ports it
// successfully, and HashMap brings its own hasher, so no hash function is modelled here.
//
// Iterator-returning operations (find/begin/end/insert/emplace) are NOT mapped: the
// only iterator helpers in libcc2rs (UnsafeMapIterator/RefcountMapIter) are keyed on
// BTreeMap and require Ord, so a HashMap iterator would be a libcc2rs change outside
// this module. Those keys are omitted rather than approximated.
//
// contains() is NOT mapped either: the rule preprocessor parses at C++17, where
// unordered_map::contains / unordered_set::contains do not exist ("No viable
// function"). count() covers the same corpus uses. Likewise operator!= is absent
// from libc++'s C++20 headers (rewritten from ==), so only operator== is mapped.

#include <unordered_map>
#include <unordered_set>
#include <utility>

template <typename T, typename A> using Init = A;

template <typename T1, typename T2> using t1 = std::unordered_map<T1, T2>;

template <typename T1> using t2 = std::unordered_set<T1>;

template <typename T1, typename T2>
T2 &f1(std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.operator[](key);
}

template <typename T1, typename T2>
std::size_t f2(const std::unordered_map<T1, T2> &o) {
  return o.size();
}

template <typename T1, typename T2> std::unordered_map<T1, T2> f3() {
  return std::unordered_map<T1, T2>();
}

template <typename T1, typename T2>
T2 &f4(std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.at(key);
}

template <typename T1, typename T2>
const T2 &f5(const std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.at(key);
}

template <typename T1, typename T2>
std::size_t f6(const std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.count(key);
}

template <typename T1, typename T2>
std::size_t f8(std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.erase(key);
}

template <typename T1, typename T2>
bool f9(const std::unordered_map<T1, T2> &o) {
  return o.empty();
}

template <typename T1, typename T2> void f10(std::unordered_map<T1, T2> &o) {
  return o.clear();
}

template <typename T1, typename T2>
bool f11(const std::unordered_map<T1, T2> &a, const std::unordered_map<T1, T2> &b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
std::unordered_map<T1, T2> &f13(std::unordered_map<T1, T2> &dst,
                                const std::unordered_map<T1, T2> &src) {
  return dst.operator=(src);
}

template <typename T1> std::unordered_set<T1> f14() {
  return std::unordered_set<T1>();
}

template <typename T1> std::size_t f15(const std::unordered_set<T1> &o) {
  return o.size();
}

template <typename T1>
std::size_t f16(const std::unordered_set<T1> &o, const T1 &key) {
  return o.count(key);
}

template <typename T1>
std::size_t f18(std::unordered_set<T1> &o, const T1 &key) {
  return o.erase(key);
}

template <typename T1> bool f19(const std::unordered_set<T1> &o) {
  return o.empty();
}

template <typename T1> void f20(std::unordered_set<T1> &o) {
  return o.clear();
}

template <typename T1>
bool f21(const std::unordered_set<T1> &a, const std::unordered_set<T1> &b) {
  return operator==(a, b);
}


// ---------------------------------------------------------------------------
// Iterator-returning operations. Backed by libcc2rs's HashMapIter
// (UnsafeHashMapIterator / RefcountHashMapIter), added alongside the BTreeMap
// MapIter and following the same protocol, so the converter's for-range lowering
// and the it->first / it->second routing to MapIterator::first/second work unchanged.
// ORDER CAVEAT: see libcc2rs/src/iterators.rs -- traversal visits every element
// exactly once but in an order that differs from libc++'s.
//
// insert / emplace are still NOT mapped: they return std::pair<iterator, bool>, whose
// lowering needs a pair whose first element is a mapped iterator type, and that is not
// modelled here. Left out rather than approximated.
// ---------------------------------------------------------------------------

template <typename T1, typename T2>
using t3 = typename std::unordered_map<T1, T2>::iterator;

template <typename T1, typename T2>
using t4 = typename std::unordered_map<T1, T2>::const_iterator;

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator f22(std::unordered_map<T1, T2> &o) {
  return o.begin();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator f23(std::unordered_map<T1, T2> &o) {
  return o.end();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator f24(std::unordered_map<T1, T2> &o,
                                                  const T1 &key) {
  return o.find(key);
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator
f25(const std::unordered_map<T1, T2> &o) {
  return o.begin();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator
f26(const std::unordered_map<T1, T2> &o) {
  return o.end();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator
f27(const std::unordered_map<T1, T2> &o, const T1 &key) {
  return o.find(key);
}

template <typename T1, typename T2>
bool f28(typename std::unordered_map<T1, T2>::iterator a,
         typename std::unordered_map<T1, T2>::iterator b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
bool f29(typename std::unordered_map<T1, T2>::iterator a,
         typename std::unordered_map<T1, T2>::iterator b) {
  return operator!=(a, b);
}

template <typename T1, typename T2>
bool f30(typename std::unordered_map<T1, T2>::const_iterator a,
         typename std::unordered_map<T1, T2>::const_iterator b) {
  return operator==(a, b);
}

template <typename T1, typename T2>
bool f31(typename std::unordered_map<T1, T2>::const_iterator a,
         typename std::unordered_map<T1, T2>::const_iterator b) {
  return operator!=(a, b);
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator &
f32(typename std::unordered_map<T1, T2>::iterator &it) {
  return it.operator++();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator
f33(typename std::unordered_map<T1, T2>::iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator &
f34(typename std::unordered_map<T1, T2>::const_iterator &it) {
  return it.operator++();
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::const_iterator
f35(typename std::unordered_map<T1, T2>::const_iterator a0, int a1) {
  return a0.operator++(a1);
}

template <typename T1, typename T2>
const T1 &f36(typename std::unordered_map<T1, T2>::iterator it) {
  return it->first;
}

template <typename T1, typename T2>
T2 &f37(typename std::unordered_map<T1, T2>::iterator it) {
  return it->second;
}

template <typename T1, typename T2>
const T1 &f38(typename std::unordered_map<T1, T2>::const_iterator it) {
  return it->first;
}

template <typename T1, typename T2>
const T2 &f39(typename std::unordered_map<T1, T2>::const_iterator it) {
  return it->second;
}

template <typename T1, typename T2>
typename std::unordered_map<T1, T2>::iterator
f40(std::unordered_map<T1, T2> &o,
    typename std::unordered_map<T1, T2>::const_iterator it) {
  return o.erase(it);
}


// ---------------------------------------------------------------------------
// std::unordered_set<T1>'s iterator.  MEASURED DISTINCTION, do NOT reuse t3/t4:
// libc++ spells it `std::__hash_const_iterator<std::__hash_node<T1, void *> *>`,
// with the element type T1 DIRECTLY -- there is NO `std::__hash_value_type<K, V>`
// wrapper, which is exactly what every unordered_MAP iterator key above carries.
// In libc++ `unordered_set<T1>::iterator` and `::const_iterator` are the SAME
// type, so ONE type key (t5) serves both; only the RECEIVER's constness
// distinguishes begin()/end()/find() below, and those are separate keys.
//
// ORDER CAVEAT (not a defect, and not papered over): traversal visits every
// element exactly once, but the ORDER differs from libc++'s and is unspecified in
// C++ either way.  See the header comment and libcc2rs/src/iterators.rs.
// ---------------------------------------------------------------------------

template <typename T1> using t5 = typename std::unordered_set<T1>::const_iterator;

// Both insert overloads are required: `s.insert(33)` on a literal binds
// insert(T1 &&), not insert(const T1 &), and a missing overload emits the mangled
// fallback name instead of the rule (measured on rules/set).
template <typename T1>
std::pair<typename std::unordered_set<T1>::iterator, bool>
f41(std::unordered_set<T1> &o, const T1 &k) {
  return o.insert(k);
}

template <typename T1>
std::pair<typename std::unordered_set<T1>::iterator, bool>
f42(std::unordered_set<T1> &o, T1 &&k) {
  return o.insert(std::move(k));
}

template <typename T1>
typename std::unordered_set<T1>::const_iterator f43(const std::unordered_set<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::unordered_set<T1>::const_iterator f44(const std::unordered_set<T1> &o) {
  return o.end();
}

template <typename T1>
typename std::unordered_set<T1>::iterator f45(std::unordered_set<T1> &o) {
  return o.begin();
}

template <typename T1>
typename std::unordered_set<T1>::iterator f46(std::unordered_set<T1> &o) {
  return o.end();
}

template <typename T1>
typename std::unordered_set<T1>::const_iterator
f47(const std::unordered_set<T1> &o, const T1 &k) {
  return o.find(k);
}

template <typename T1>
typename std::unordered_set<T1>::iterator f48(std::unordered_set<T1> &o,
                                              const T1 &k) {
  return o.find(k);
}

template <typename T1>
bool f49(typename std::unordered_set<T1>::const_iterator a,
         typename std::unordered_set<T1>::const_iterator b) {
  return operator==(a, b);
}

template <typename T1>
bool f50(typename std::unordered_set<T1>::const_iterator a,
         typename std::unordered_set<T1>::const_iterator b) {
  return operator!=(a, b);
}

template <typename T1>
typename std::unordered_set<T1>::const_iterator &
f51(typename std::unordered_set<T1>::const_iterator &it) {
  return it.operator++();
}

template <typename T1>
typename std::unordered_set<T1>::const_iterator
f52(typename std::unordered_set<T1>::const_iterator a0, int a1) {
  return a0.operator++(a1);
}

// f53 (operator* on an unordered_set iterator) is DELIBERATELY ABSENT, loudly.
// It needs libcc2rs::SetIterator implemented for HashSetIter (added in
// libcc2rs/src/iterators.rs), but the rule preprocessor type-checks targets against a
// PINNED libcc2rs artifact, and its staleness guard is keyed on new ITEM NAMES -- a new
// trait IMPL adds no name, so the guard passes and rustc then rejects the target with
// E0277 "the trait bound HashSetIter<..>: SetIterator is not satisfied".  Re-add f53 once
// the pinned libcc2rs is refreshed; do not approximate it.

template <typename T1>
typename std::unordered_set<T1>::iterator
f54(std::unordered_set<T1> &o, typename std::unordered_set<T1>::const_iterator it) {
  return o.erase(it);
}

// f55 -- operator[] TAKING AN RVALUE KEY.  MEASURED 2026-09-28 from
// `probe/umapdecomp/p.cpp` (`std::unordered_map<int,long> um; um[7] = 100;`):
//     search expr long & std::unordered_map<int, long>::operator[](int &&), result:
//     None
// f1 keys only the `const T1 &` overload, so a SUBSCRIPT WITH A PRVALUE KEY -- which
// is what `um[7]`, `um[i + 1]` and `um[some_call()]` all are, since libc++ declares
// both `mapped_type &operator[](const key_type &)` and `mapped_type &operator[](key_type &&)`
// and overload resolution picks the && one for any rvalue -- found NO key and fell
// through to the converter's generic array-subscript lowering, emitting
// `um[(7) as usize] = 100_i64` against a `HashMap<i32, Box<i64>>` (two E0308s: the
// index is not a `&i32`, and the value is not a `Box<i64>`).  `rules/map` has had
// this pair since the start (f1 + f8, byte-identical bodies); this is the missing
// unordered twin.  The body is `entry(k).or_default()`, which INSERTS a
// default-constructed value for a missing key -- exactly what C++ `operator[]` does,
// and why it must not be a `get`/panic form.
template <typename T1, typename T2>
T2 &f55(std::unordered_map<T1, T2> &o, T1 &&key) {
  return o.operator[](std::move(key));
}

// f56 -- the INITIALIZER-LIST CONSTRUCTOR, i.e. `std::unordered_map<K,V> m = {{k,v}, ...}`.
// MEASURED 2026-09-28 from the fallback readback of `probe/umilist/p.cpp`:
//     std_unordered_map_std_string__int__std_hash_std_string___std_equal_to_std_string___
//     std_allocator_std_pair_const_std_string__int___ :: new_1 ( { vec! [ (..) , .. ] } , )
// Two facts that differ from `rules/map`'s f35 and were read off the emission rather than
// guessed: (a) the braced init list already lowers to a `vec![...]`, same as map; but
// (b) there is NO extra argument -- the call site carries the init list ALONE, with a
// single trailing comma and no `None`.  map's f35 needs the `const std::less<T1> &`
// parameter because libc++ declares `map(initializer_list, const key_compare& = ...)`
// as ONE constructor with a DEFAULTED comparator, which the recorder collapses to a
// literal `None` argument; libc++'s `unordered_map(initializer_list<value_type>)` is by
// contrast its own 1-parameter OVERLOAD (the bucket-count/hasher/equal/allocator forms
// are separate overloads), so Hash, KeyEqual and Allocator appear only in the mangled
// TYPE name and never as arguments.  Hence a 1-arg key, not a 4-arg one.
// The body uses `.rev()` before `collect()` because HashMap's FromIterator inserts in
// iteration order and a later duplicate REPLACES the value (last-wins), while
// std::unordered_map's init-list construction keeps the FIRST of equivalent keys;
// reversing makes the two agree.  `a0` occurs EXACTLY ONCE -- a rule body is inlined as
// one expression, so every `aN` occurrence re-evaluates that argument.
template <typename T1, typename T2>
std::unordered_map<T1, T2>
f56(const std::initializer_list<std::pair<const T1, T2>> &a0) {
  return std::unordered_map<T1, T2>(a0);
}

// f57 -- `unordered_map::emplace(key, value)`, i.e. THE `pair<iterator, bool>`
// RETURN that every `auto [it, inserted] = m.emplace(k, v)` decomposes.  This
// supersedes the header note above ("insert / emplace are still NOT mapped ...
// needs a pair whose first element is a mapped iterator type, and that is not
// modelled here"): t3 + rules/pair's generic t1 ALREADY compose into that model.
// MEASURED 2026-09-28 on probe/umapempl/p.cpp (`std::unordered_map<int64_t,size_t>`,
// harness --cxxflags, -verbose, run to exit rc=0):
//     search type std::pair<std::__hash_map_iterator<std::__hash_iterator<
//       std::__hash_node<std::__hash_value_type<long, unsigned long>, void *> *>>,
//       bool>, result: (T1, T2)                      <- the TYPE already resolves
//     search expr std::pair<std::__hash_map_iterator<std::__hash_iterator<
//       std::__hash_node<std::__hash_value_type<long, unsigned long>, void *> *>>,
//       bool> std::unordered_map<long, unsigned long>::emplace(&&...), result:
//     None                                           <- the CALL did not
// ⛔ AND THE MISS WAS SILENT: the TU stayed rc=0 and emitted
//     firstIndex.emplace(&mut val, &mut ___args_1)
// textually against a `HashMap<i64, Box<u64>>`, with NO placeholder token and
// nothing for `pin/no-placeholders.sh` or any bucket census to see.  An unmapped
// MEMBER does not abort.
//
// ⭐⭐ ARITY IS NOT IN THE KEY, SO THIS RULE IS ARITY-GENERIC VIA `Init<>`.  libc++'s
// emplace is `template<class... Args> pair<iterator,bool> emplace(Args&&...)`, and
// `Mapper::ToString` prints the DECLARATION, so both the converter's ask and the
// recorded key read `emplace(&&...)` whatever the call's arity is -- ONE key serves
// every arity and no fixed-arity rule can be correct for all of them.  MEASURED: the
// ask spelling is byte-identical at arity 1 and arity 2 (same md5, `cmp` clean),
// because `HasFunctionParameterPack` resolves through `getPrimaryTemplate()` and
// `GetExprCallArity` returns nullopt on a top-level `...`, so the `#arity` bucket that
// would separate them is never formed.  A longer-`src` sibling key cannot separate
// them either (same libc++ decl -> same bucket, EQUAL length, and `search`'s tie-break
// is a strict `>`), and neither can a receiver-specialised key
// (`unordered_map<long,long>` is emplaced at arity 1 at dxp.cpp:1128 and at arity 2 at
// eight other sites).  rules/functional f17-f22 and rules/tuple f6-f9 get away with
// per-arity keys only because their arity lives in the RETURN type; `emplace` returns
// `pair<iterator,bool>` at every arity.
//
// ⭐ THIS NEEDED A PREPROCESSOR FIX AND IT IS WORTH KNOWING WHY.  `Init<T, Args>`
// routes through `cpp_rule_preprocessor.cpp addPackRule`, which records WHERE `T`
// lives relative to the CALLEE's template arguments so the converter can recover the
// concrete type per call site.  For `unordered_map<K,V,H,E,A>` those arguments are K,
// V, hash<K>, equal_to<K> and allocator<pair<const K,V>> -- `pair<const K,V>` is NOT
// among them, only the ALLOCATOR wrapping it is, and the old (depth,index)-only
// encoding therefore aborted with "Init type ... is not a template argument".  The
// encoding now also carries a NESTED-ARGUMENT PATH, so this type is named as
// "template argument 0 of template argument 4", and the converter replays that
// descent.  ⛔ Do NOT drop the `const` from `pair<const T1, T2>`: `pair<T1, T2>` is
// not the allocator's argument and would fail the same way (measured).
//
// ⛔ ARITY 3+ IS NOT SILENTLY ACCEPTED.  An Init<> body has no aN placeholder count to
// bound it, so an `emplace(piecewise_construct, ...)` call would otherwise have
// reached `ConvertConstructFromArgs` with 3 arguments for a 2-element pair.
// `BuildInitExpr` returns null there and that path is now a `report_fatal_error`
// naming the type, the arity and the site (it was an assert, i.e. a NO-OP in the
// shipped -DNDEBUG build).  The corpus' four arity-3+ sites are all on std::map /
// std::set, which have no emplace key, so nothing binds here today.
//
// SEMANTICS, both load-bearing and both honest here:
//   * `inserted` -- C++ emplace returns FALSE and LEAVES THE EXISTING VALUE ALONE
//     when the key is already present.  The bodies therefore test membership
//     first and only insert on a miss; they never report an unconditional `true`
//     and never overwrite.  (`HashMap::insert` alone would have done BOTH wrong.)
//   * the ITERATOR half -- `find_key` on the LIVE map, exactly as f24/f27 and as
//     f41/f42 do for unordered_set.  It is an IDENTITY, not a copy: `it->second`
//     routes through f37, whose `second()` yields `*mut T2` into the map's own
//     node, so a write through it reaches the container.  A cloned value would
//     have made `it->second = x` a write to a temporary.
// `init` is bound to a `let` BEFORE the membership test, so the whole argument list is
// evaluated exactly once and unconditionally -- a rule body is inlined as one
// expression, and a value used only inside the `if` would have skipped its side
// effects on the already-present path.
template <typename T1, typename T2, typename... Args>
std::pair<typename std::unordered_map<T1, T2>::iterator, bool>
f57(std::unordered_map<T1, T2> &o,
    Init<std::pair<const T1, T2>, Args> &&...args) {
  return o.emplace(std::forward<Args>(args)...);
}

// ===========================================================================
// ⛔⛔ `emplace` AT ARITY 1 (`m.emplace(some_pair)`) IS NOT KEYABLE TODAY, AND THE
// REASON IS A PREPROCESSOR GAP, NOT A MISSING SPELLING.  MEASURED 2026-09-29 on
// pin/cpp2rust md5 a1bc90153318514a178849ebd27944fa + pin/ir.v41.
//
// THE SYMPTOM.  `dxp/dxp.cpp:1126-1129` builds
//     std::unordered_map<VariableSymbol, VariableDefinition::ValueType> values;
//     for (const auto &symVal : dataTensorStartAddrSymVals) values.emplace(symVal);
// where `symVal` is a `const std::pair<VariableSymbol, ValueType> &` -- i.e. ONE
// argument, the whole value_type.  f57's body binds a1 AND a2, so the TU dies:
//     LLVM ERROR: rule body references placeholder a2 but the call site supplies
//     only 2 argument(s) at .../dxp/dxp.cpp:1128:12
// That is a WHOLE-TU abort (bucket B, 0 lines emitted), not a bad site: the
// arity check at converter.cpp:9748-9762 calls `report_fatal_error` in every mode
// except `--survey`.  Reproduced standalone in 9 lines (`m.emplace(p)`).
//
// ⭐⭐ NEITHER OF THE TWO OBVIOUS FIXES EXISTS.  Both were measured, not reasoned:
//
// (1) NARROWING f57 SO IT CANNOT MATCH ARITY 1 IS IMPOSSIBLE.  The ask spelling is
//     BYTE-IDENTICAL at both arities.  Two single-arity probes, `-verbose`:
//       arity 1  m.emplace(p)      | search expr std::pair<std::__hash_map_iterator<
//       arity 2  m.emplace(1, 2L)  |   std::__hash_iterator<std::__hash_node<
//                                  |   std::__hash_value_type<int, long>, void *> *>>,
//                                  |   bool> std::unordered_map<int, long>::emplace(&&...)
//     Both lines md5 d1f4c6473d1843f837a13392e3d62c25, `cmp` clean.  The cause is
//     `Mapper::HasFunctionParameterPack` (mapper.cpp:2872-2876), which resolves
//     through `getPrimaryTemplate()`: a SPECIALIZATION of a variadic template
//     therefore still prints the pack marker, so `ToString` renders `(&&...)` at
//     EVERY arity on the ask side exactly as it does on the load side.  And the
//     `#arity` bucket that would otherwise separate them is never formed on either
//     side, because `GetExprCallArity` returns nullopt on a top-level `...`
//     (mapper.cpp:237-240).  There is no string to narrow.
//
// (2) A SIBLING KEY PLUS THE LONGER-`src` TIE-BREAK DOES NOT APPLY EITHER, and
//     adding one would be a SILENT MIS-BIND rather than a fix.  A second rule
//     spelled on the same receiver records the SAME src (it comes from
//     `Mapper::ToString(callee)`, and the callee is the same libc++ variadic
//     `emplace` declaration), so both land in the same bucket AND their srcs are
//     the same LENGTH.  `search`'s tie-break is a STRICT `>` --
//     `if (!rule || this_rule.src.size() > rule->src.size())`, mapper.cpp:907 --
//     so on equal lengths the winner is whichever the `unordered_multimap` bucket
//     happens to yield FIRST.  An arity-1 call could bind the arity-2 body or the
//     reverse, nondeterministically, and it would COMPILE.
//     ⭐ WHY `rules/functional` f17-f22 AND `rules/tuple` f6-f9 ARE DIFFERENT, and
//     it is not luck: their arity is carried by a part of the key that is NOT the
//     parameter list.  `std::tie`'s RETURN type is `std::tuple<T1 &, ..., TN &>`,
//     so the arity-2 and arity-27 keys are genuinely different strings of
//     genuinely different length and the tie-break resolves them.
//     `unordered_map::emplace` always returns `pair<iterator, bool>`, INDEPENDENT
//     of arity, so that discriminator does not transfer.
//
// ⛔ AND DO NOT "FIX" THIS BY REWRITING f57's BODY FOR ARITY 1.  Both arities are
// real in this corpus (the arity-2 site that f57 was written for is quoted in the
// f57 note above, `firstIndex.emplace(k, v)`), so an arity-1 body would turn that
// site into a rustc E0308 on `let (__k, __v) = <key>;`.  That is LOUD, not silent,
// but it is still trading one broken arity for the other, not a fix.
//
// ⭐⭐ WHAT HAS TO LAND, AND IT IS FIVE LINES IN THE PREPROCESSOR, NOT HERE.  The
// arity-GENERIC pack mechanism (`Init<T, Args> &&...args`, what `rules/vector`
// f112 uses for `emplace_back`) is the right shape and would serve EVERY arity
// from one key: `Converter::ConvertInitFragment` (converter.cpp:9822-9837) hands
// the actual arguments to `BuildInitExpr`, which copy-initialises the pair from a
// single pair argument and 2-ary-constructs it from `(k, v)` -- correct at both,
// with no arity in the key at all.  The ONE thing in the way is that
// `cpp_rule_preprocessor.cpp:286-303 findTemplateArgument` records the init type
// as a (depth, index) PAIR INTO THE CALLEE'S OWN TEMPLATE ARGUMENT LIST, so the
// init type must be VERBATIM one of those arguments.  For `unordered_map<K, V, H,
// E, A>::emplace` they are `Args...`, K, V, `hash<K>`, `equal_to<K>` and
// `allocator<pair<const K, V>>` -- and `value_type` = `pair<const K, V>` is NOT
// among them; it is only reachable INSIDE the allocator argument.  Measured, both
// directions, on the pinned preprocessor md5 a46d45dc97f80a5ba8ee07b3eb5b6a96:
//    Init<std::pair<const T1, T2>, Args>  -> ERROR: Init type std::pair<const T1,
//        T2> is not a template argument of ... ::emplace(&&...)   (exit 1, NO IR)
//    Init<std::pair<T1, T2>, Args>        -> the SAME ERROR, so it is STRUCTURAL,
//        not a const-qualification mismatch
//    Init<T2, Args>                       -> `OK unordered_map -> ...`
// ⭐ That last line is the POSITIVE CONTROL and it is the whole point: the pack
// path itself works fine for `unordered_map` -- `getInitType`, `addPackRule` and
// the Rust `init:` parameter all resolve -- and ONLY the (depth, index) encoding
// of the init type is too weak to name `pair<const K, V>`.  (It is also wrong
// semantically, of course: `Init<T2, ...>` would construct the VALUE from the
// arguments.  It is here as a control, and it is NOT a key.)  The preprocessor
// needs to record the init type STRUCTURALLY -- as a spelling the converter
// re-substitutes -- or `findTemplateArgument` needs to search NESTED template
// arguments.  Until then f57 stays arity-2-only and every arity-1 site stays a
// LOUD whole-TU abort, which is the correct failure while the shape is unkeyable.
//
// ⚠️⚠️ AND THE ARITY-1 ABORT IS NOT THE WORST SHAPE THIS KEY CAN MEET.  A
// 2-PLACEHOLDER BODY UNDER AN ARITY-FREE KEY IS SILENTLY WRONG AT ARITY >= 3:
// the bounds check at converter.cpp:9748 only fires when the body asks for MORE
// arguments than the site supplies, so at arity 3 or 4 `a1`/`a2` bind the FIRST
// TWO and every further argument is DROPPED -- no abort, no placeholder token,
// nothing for `pin/no-placeholders.sh` or a bucket census to see.
// ⭐ MEASURED on the corpus 2026-09-29.  Funnel: 216 raw `emplace(` occurrences
// repo-wide -> 201 non-vendored (`external/`, `common/json/` nlohmann,
// `dataflow-scheduler/external/`) -> 153 `.emplace(`/`->emplace(` after removing 48
// `try_emplace(` -> 143 on an ASSOCIATIVE receiver after removing 10 that are
// `std::optional` (6), `std::vector` (3) and `std::queue` (1).  `emplace_back` /
// `emplace_front` (794 non-vendored) are a different member and are not counted.
// Arities counted by top-level commas at paren/brace/angle depth 0:
//     container            arity1  arity2  arity3  arity4   total
//     std::map                  1      84       3       0      88
//     std::unordered_map        1      43       0       0      44
//     std::set                  6       1       0       1       8
//     std::unordered_set        1       0       0       0       1
//     generic (map-or-umap)     0       2       0       0       2
//     TOTAL                     9     130       3       1     143
// No `multimap`/`multiset`/`unordered_multimap`/`unordered_multiset` receiver
// appears anywhere.  The four arity->=3 sites are
//     dsm/dsm.cpp:3928, dsc-based-utils/progtailor/progtailor.cpp:334 and :472
//        -- all three `emplace(std::piecewise_construct, forward_as_tuple(core,
//           comp), forward_as_tuple(...))`, and all three read `.first->second`
//     dxp/test/dxp_unittest.cpp:1404
//        -- `seenLocs.emplace(loc.flitId, loc.sliceId, loc.stPosn, loc.endPosn)`,
//           reading `.second`
// ⭐ NONE of them is reachable from f57: the three piecewise sites are
// `std::map<std::pair<int, SenComponents>, ...>` (declared at progtailor.cpp:319
// and :447, and dsm.cpp:3912) and the fourth is `std::set<std::tuple<uint32_t,
// uint32_t, uint32_t, uint32_t>>` (dxp_unittest.cpp:1398).  So there is NO silent
// mis-bind in the tree today -- but only because of the last paragraph below.
//
// ⭐⭐ AND THE CENSUS CLOSES THE ONE ESCAPE HATCH THE ARGUMENT ABOVE LEAVES OPEN.
// The remaining idea would be a RECEIVER-SPECIALIZED key -- spell the receiver
// concretely so the arity-1 sites get their own longer `src`, the way
// `rules/algorithm` f13/f18 spell `std::string::iterator` instead of `T1 *`.  That
// CANNOT WORK HERE, because the SAME receiver type is emplaced at BOTH arities:
//   * `std::unordered_map<VariableSymbol, VariableDefinition::ValueType>` is
//     arity 1 at dxp/dxp.cpp:1128 and arity 2 at
//     dbo/src/Utils/sdsc_bundle/ProgramCorrection.cpp:2348.  And the spelling is
//     not even the discriminator it looks like: `VariableSymbol = int64_t`
//     (util/variabledefinition/VariableDefinition.h:32) and
//     `VariableDefinition::ValueType = VariableSymbol` (:95), so the converter's
//     DESUGARED ask for all of them is `std::unordered_map<long, long>` -- arity 1
//     at dxp.cpp:1128 and arity 2 at eight other sites.
//   * the same shape appears on `std::set`:
//     `std::set<std::pair<SenComponents, SenComponents>>` is arity 2 at
//     DSC2ToDataflowIR/V3/SNTransferLowering.cpp:2603 and arity 1 at :2651, from
//     two character-identical parameter declarations in ONE file (:2566, :2619).
// Of the 64 distinct receiver spellings those are the only two emplaced at more
// than one arity -- but two is enough: no key spelled on the receiver can separate
// the arities, so the (depth,index) fix below is the only one that works.
//
// ⛔⛔ `rules/map` AND `rules/set` HAVE NO `emplace` KEY AT ALL (checked, both
// modules), AND THAT IS NOW A DELIBERATE ABSENCE, NOT AN OVERSIGHT.  Adding a
// 2-ary `std::map::emplace` key would inherit the arity-1 abort AND immediately
// turn those three `piecewise_construct` sites into silent argument drops, because
// `std::map::emplace` / `std::set::emplace` are the same variadic
// `template<class... Args>` shape with the same arity-free key and the same
// arity-independent `pair<iterator, bool>` return -- every measurement above
// transfers verbatim.  DO NOT ADD ONE until the preprocessor change above has
// landed.  The corpus's own return-value usage says the same thing from the other
// side.  Over the 143 sites: 117 DISCARDED, 15 READS_SECOND, 8 READS_FIRST, 3
// READS_BOTH, 0 RETURNED.  So 26 sites genuinely read the pair and BOTH halves are
// load-bearing -- `.second` alone at dxp_unittest.cpp:1404, dsc/dsc2.cpp:1369 and
// :5171, dsc/pcfg.cpp:2605, dsc/dims.cpp:768, progtailor/regstitcher.cpp:27/:81/
// :119/:139/:142, progtailor.cpp:764, util/utils.h:150,
// VariableDefinition.cpp:310, DataConvertInfoGenerate.cpp:44 and
// deeprt_scheduler_codegen_pipeline.cpp:550; `.first->second` at dsm.cpp:3928,
// progtailor.cpp:334, ddc_transformation_util.cpp:130, ddc/ddcv1.cpp:1031 and
// :1169, ddl_conversion.cpp:1332, dsc/dsc2.cpp:3487 and :3731; and BOTH at
// regstitcher.cpp:71, ProgramCorrection.cpp:2600 and sys-arch-spec/dpc/dpc.cpp:591.
// A body may therefore neither fabricate `true` nor discard the iterator, which is
// what f57 already gets right.  `dxp/dxp.cpp:1128` itself DISCARDS the pair, but
// that is no licence to drop it from the model: it is one of 117.
// ⚠️ `try_emplace` (48 non-vendored sites) has the same `pair<iterator, bool>`
// return and the same arity-free variadic key, so everything above applies to it
// too.  It is likewise unkeyed, deliberately.
// ===========================================================================
