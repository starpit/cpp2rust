// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <map>
#include <utility>

// The arity-generic pack helper, copied verbatim from rules/unordered_map:31 and
// rules/vector.  `Init<T, Args>` tells cpp-rule-preprocessor "this pack parameter
// initialises a T"; the converter's ConvertInitFragment/BuildInitExpr then builds
// the T from whatever the call site actually supplies, so ONE key serves EVERY
// arity and no arity ever appears in the recorded key.
template <typename T, typename A> using Init = A;

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

// g850 -- `operator>=` on two std::map<std::string, double>.
// DECISION: C++ `operator>=` on std::map is a LEXICOGRAPHIC compare over the
// SORTED sequence of key-value pairs.  Both models spell the receiver as a
// `BTreeMap<T1, _>`, whose own `Ord` is likewise lexicographic over its sorted
// (key, value) pairs -- and the value wrappers compare BY CONTENT
// (`Box<T2>: Ord if T2: Ord`, `Rc<RefCell<T2>>: Ord if T2: Ord`), not by
// address -- so the ordering IS reproduced and a plain `a0 >= a1` is exact.
// (Had the model been a HashMap or a Vec this would have had to be refused:
// no order at all, or an insertion-dependent one.)
template <typename T1, typename T2>
bool f36(const std::map<T1, T2> &a, const std::map<T1, T2> &b) {
  return operator>=(a, b);
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

// --- llvm::MapVector -------------------------------------------------------
//
// llvm/ADT/MapVector.h:32 --
//   template <typename KeyT, typename ValueT,
//             typename MapType = DenseMap<KeyT, unsigned>,
//             typename VectorType = SmallVector<std::pair<KeyT, ValueT>, 0>>
//   class MapVector {
//     MapType Map;        // key -> index into Vector
//     VectorType Vector;  // THE STORAGE, in insertion order
//     ...
//   };
//
// ⛔ WHY THIS LIVES IN rules/map AND NOT IN A NEW rules/mapvector: creating a
// 95th module would break the 94-module gate for every other live slot in the
// same cut.  A module is only a key container; the bucket is keyed off the
// spelling (`llvm::MapVector`), which `GetTypeMapKey`'s truncate-at-first-`<`
// puts in a DIFFERENT bucket from `std::map`, so t5 can neither swallow nor be
// swallowed by t1/t4.
//
// MODEL: `Vec<(KeyT, <boxed>ValueT)>`, i.e. the `Vector` member itself.
// ⭐ THE INSERTION ORDER IS THE WHOLE POINT OF THE TYPE.  MapVector exists in
// LLVM precisely because DenseMap's iteration order is nondeterministic and a
// compiler pass that iterates it emits nondeterministic output.  Every corpus
// use is an ordered walk (dbo SymbolDefinitions, dcc unit_to_ops /
// equivalence_classes, dataflow-scheduler memref_to_groups / buffer_to_loop_ivs
// / nodes_).  So:
//   * `HashMap` would be SILENTLY WRONG -- it destroys the one property the
//     type was chosen for, while type-checking and compiling.
//   * `BTreeMap` (this module's model for std::map) would be SILENTLY WRONG in
//     the same way -- it iterates in KEY order, not insertion order, so a pass
//     would still be deterministic but would emit a DIFFERENT order than C++.
//     That is the harder bug to see, which is why it is named here.
//   * `Vec<(K, V)>` reproduces insertion order exactly.  The `Map` member is a
//     pure index -- redundant state, not information -- so dropping it loses
//     nothing semantically; it costs lookup O(n) instead of O(1), which is a
//     performance difference, not a fidelity one.
// The value is BOXED (`Box`/`Value`) for the same reason t1 boxes it: a Vec
// reallocates, and MapVector's `operator[]` returns `ValueT &`, so the payload
// needs a stable address before any member can be keyed honestly.
//
// ⛔ ONLY THE TYPE IS KEYED; EVERY MEMBER IS DELIBERATELY LEFT OUT, so each one
// fails LOUDLY at translate time rather than quietly.  This key exists because
// the measured first abort of
// dbo/src/Transforms/sdsc_bundle/CopyProgramBinaries.cpp is
//   `unsupported unmapped type llvm::MapVector<long, std::pair<...>> has no
//    model in types_, while mapping llvm::FailureOr<llvm::MapVector<...>>`
// i.e. the OUTER `llvm::FailureOr<T1>` (rules/support t3) ALREADY MATCHES and
// binds T1; the row is gated purely on T1 having no model.  A type model is
// therefore the minimum and the maximum that this observer justifies -- writing
// `insert`/`lookup`/`operator[]` bodies here would be unobserved guesswork, and
// a second key beside an unproven one is how the six-dead-key misdiagnosis in
// rules/support happened.
//
// RESTATED WITH TWO TEMPLATE PARAMETERS, NOT FOUR, ON PURPOSE.  The recorder
// drops TRAILING DEFAULTED template arguments, so every corpus site searches as
// the 2-ary spelling (queue g271/g973-g977 all show
// `searched as: llvm::FailureOr<llvm::MapVector<long, std::pair<VariableOperator,
// std::vector<VariableDefinition::OperandType>>>>` -- 2 args -- against a
// `from decl` line carrying all four).  A 4-ary restatement would need
// DenseMap/DenseMapInfo/detail::DenseMapPair/SmallVector restated too, none of
// which cpp-rule-preprocessor's fixed flag set can reach, and would record a
// 4-ary key that the 2-ary search can never find.  The rule file fixes only the
// SPELLING of the key, and this is the spelling that is searched.
namespace llvm {
template <typename KeyT, typename ValueT> class MapVector {
public:
  ValueT &operator[](const KeyT &Key);
  std::pair<std::pair<KeyT, ValueT> *, bool> insert(std::pair<KeyT, ValueT> &&KV);
  std::size_t count(const KeyT &Key) const;
  std::size_t size() const;
  bool empty() const;
  void clear();
};
} // namespace llvm

template <typename T1, typename T2> using t5 = llvm::MapVector<T1, T2>;

// --- MapVector members ---------------------------------------------------
// CENSUS (dt_src corpus, git grep on MapVector-typed vars/fields, sites
// counted with `grep -o | wc -l`, never `grep -c`):
//   operator[]   10 sites  (RoutingGraph.cpp nodes_, BufferExpansion.cpp
//                           memref_to_groups/buffer_to_loop_ivs,
//                           FlatteningLocalRegions.cpp unit_to_ops/
//                           equivalence_classes, ConstructThreeStagePipeline
//                           fifo_type_groups, UnitTypeDiscovery result)  KEYED (f37)
//   insert        3 sites  (same files, explicit .insert({k,v}) calls)     KEYED (f38)
//   count         2 sites                                                 KEYED (f39)
//   size          2 sites                                                 KEYED (f40)
//   empty         1 site                                                  KEYED (f41)
//   clear         1 site                                                  KEYED (f42)
//   range-for    12 sites: 7 single-variable (`for (auto &kv : m)`) already
//                fall through to the existing Vec<(K,V)> positional model
//                via Converter::VisitCXXForRangeStmtVector -- no key needed;
//                5 DECOMPOSING (`for (auto &[k,v] : m)`) are refused LOUDLY
//                by ReportUnsupportedStructuredBinding regardless of any
//                rules change (IsMapLikeRangeClass omits MapVector) --
//                NOT KEYED, a converter-dispatch gap out of scope here.
//   find/end/it->second: 1 site total                                     NOT KEYED
//     (leaving a key out is preferred over an unproven one -- see brief).
//
// PLAIN REFERENCE receivers for all six (not `Ptr<...>`): the census
// majority is a local variable or by-ref parameter, not a struct field.
// A struct-field call site (e.g. RoutingGraph::nodes_) may need a
// `Ptr`-based overload later -- a named follow-on gap, not silently
// assumed away.
template <typename T1, typename T2>
T2 &f37(llvm::MapVector<T1, T2> &o, const T1 &key) {
  return o.operator[](key);
}

// ⚠️ THIS RETURNS THE FULL PAIR, NOT `.second` -- MEASURED, NOT A STYLE CHOICE.
// A body that reads `return o.insert(...).second;` (a member-call result then
// a field access, both inside ONE rule body) makes cpp-rule-preprocessor
// SIGSEGV deterministically (bisected: crashes with the field access present
// in EITHER model regardless of the declared pair's element types; the same
// body with `.first` instead of `.second` does NOT crash, so it is specific
// to chaining a field access onto a call result inside a rule body, not to
// this type). rules/smallptrset's f1 already establishes the fix as the
// working convention: key the CALL (`insert`) to return the whole
// `std::pair<iterator, bool>`, and let rules/pair's own generic `.second`
// accessor (its f1) match the follow-on MemberExpr at the call site
// separately -- the two AST nodes are matched independently, so the body
// here must not pre-compose them.
template <typename T1, typename T2>
std::pair<std::pair<T1, T2> *, bool> f38(llvm::MapVector<T1, T2> &o,
                                          std::pair<T1, T2> &&kv) {
  return o.insert(static_cast<std::pair<T1, T2> &&>(kv));
}

template <typename T1, typename T2>
std::size_t f39(const llvm::MapVector<T1, T2> &o, const T1 &key) {
  return o.count(key);
}

template <typename T1, typename T2>
std::size_t f40(const llvm::MapVector<T1, T2> &o) {
  return o.size();
}

template <typename T1, typename T2>
bool f41(const llvm::MapVector<T1, T2> &o) {
  return o.empty();
}

template <typename T1, typename T2>
void f42(llvm::MapVector<T1, T2> &o) {
  return o.clear();
}



// ===========================================================================
// g3091 -- `std::map::emplace`, ARITY-GENERIC.
//
// ⛔⛔ THIS KEY WAS A DELIBERATE, DOCUMENTED ABSENCE AND THE REFUSAL IS NOW
// OBSOLETE.  `rules/unordered_map/src.cpp` (the block above its f57, ~line 620)
// says verbatim: "`rules/map` AND `rules/set` HAVE NO `emplace` KEY AT ALL
// (checked, both modules), AND THAT IS NOW A DELIBERATE ABSENCE, NOT AN
// OVERSIGHT.  ...  DO NOT ADD ONE until the preprocessor change above has
// landed."  ⭐ **THE NAMED UNBLOCKER HAS LANDED.**  That refusal's own text names
// its unblocker -- `cpp_rule_preprocessor.cpp findTemplateArgument` recording the
// init type through a NESTED template-argument path instead of a bare
// (depth,index) pair -- and f57's OWN note, three hundred lines earlier in the
// same file, records that it shipped: "The encoding now also carries a
// NESTED-ARGUMENT PATH, so this type is named as 'template argument 0 of template
// argument 4', and the converter replays that descent."  f57 is today spelled
// `Init<std::pair<const T1, T2>, Args> &&...args`, i.e. exactly the
// arity-GENERIC shape the refusal said was unavailable.  The refusal text was
// simply never revisited after its own blocker was fixed.
//
// WHY ARITY-GENERIC IS THE ONLY CORRECT SHAPE HERE, and every one of these is
// unordered_map's own measurement, transferred because the C++ declaration is
// the same variadic `template<class... Args> pair<iterator,bool> emplace(Args&&...)`:
//   * the recorded key is `...std::map<T1, T2>::emplace(&&...)` -- NO arity.
//     `Mapper::HasFunctionParameterPack` resolves through getPrimaryTemplate(), so
//     a specialisation still prints the pack marker, and `GetExprCallArity`
//     returns nullopt on a top-level `...`, so no `#arity` bucket is formed on
//     either the load or the ask side.  There is no string to narrow and no
//     second key that could coexist (equal-length `src`, strict `>` tie-break at
//     mapper.cpp:907 -> nondeterministic winner).
//   * therefore a FIXED-ARITY body would be wrong in BOTH directions on this
//     corpus: the 1 arity-1 site would abort the whole TU
//     (converter.cpp:9748 "rule body references placeholder a2 but the call site
//     supplies only 2 argument(s)"), and the 3 arity-3
//     `emplace(std::piecewise_construct, forward_as_tuple(..), forward_as_tuple(..))`
//     sites (dsm/dsm.cpp:3928, progtailor/progtailor.cpp:334 and :472 -- all three
//     on `std::map<std::pair<int, SenComponents>, ...>`, i.e. all three reachable
//     from THIS key) would SILENTLY DROP every argument past the second.  An
//     Init<> pack has no aN placeholder count, so it takes neither branch: arity 1
//     copy-initialises the pair from the single pair argument, and arity >= 3
//     reaches `BuildInitExpr` -> null -> `report_fatal_error` naming type, arity
//     and site.  ⭐ Silent-wrong becomes loud-wrong, which is RULE 2.
//
// SEMANTICS (the C++/Rust difference the brief warned about, and it is NOT fatal
// here): C++ `emplace` is *insert if absent, and report whether it inserted*;
// `BTreeMap::insert` OVERWRITES and returns the OLD VALUE.  Those are different
// functions, so the body does NOT use bare `insert`: it tests membership first and
// only inserts on a miss, so an already-present key keeps its existing value and
// reports `false`.  The ITERATOR half is `find_key` on the LIVE map -- an
// identity, not a copy -- so `.first->second = x` writes into the container's own
// node.  Both halves are load-bearing on this corpus: of the 143 associative
// `emplace` sites unordered_map's census funnelled, 15 read `.second`, 8 read
// `.first`, 3 read both.
template <typename T1, typename T2, typename... Args>
std::pair<typename std::map<T1, T2>::iterator, bool>
f43(std::map<T1, T2> &o, Init<std::pair<const T1, T2>, Args> &&...args) {
  return o.emplace(std::forward<Args>(args)...);
}

// g3091 -- `std::map::count(const T1 &) const`.
// FAITHFUL, not an approximation: std::map is a UNIQUE-key container, so
// `count(k)` is exactly `contains_key(k) as usize` and its range is {0, 1}.
// (On `multimap` it would not be -- and this key cannot reach a multimap: the
// receiver spelling is `std::map<T1, T2>` and `GetTypeMapKey` truncates at the
// first `<`, putting `std::multimap` in a different bucket.  No multimap receiver
// appears anywhere in the corpus in any case.)
// RECEIVER SHAPE mirrors f2 (`size() const`), the closest landed analogue: a
// const member whose return is a scalar, not a place, on the same receiver
// spelling.  f7/f15 (`at`) take `&mut` instead because they return a pointer
// INTO the map and so need a place; `count` does not.
template <typename T1, typename T2>
std::size_t f44(const std::map<T1, T2> &o, const T1 &key) {
  return o.count(key);
}
