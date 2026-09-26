// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::MapVector -- an INSERTION-ORDER-PRESERVING map.
//
// WHY THE DECLARATION IS LOCAL: same reason as rules/densemap and
// rules/smallset -- cpp-rule-preprocessor compiles this file with a fixed flag
// set, so an #include of <llvm/ADT/MapVector.h> would make `ninja` fail for
// anyone without an LLVM tree at the same absolute path.
//
// THE DEFAULTED TEMPLATE ARGUMENTS ARE KEPT OUT OF THE PARAMETER LIST.
// The real declaration (llvm/ADT/MapVector.h) is
//     template <typename KeyT, typename ValueT,
//               typename MapType = DenseMap<KeyT, unsigned>,
//               typename VectorType = SmallVector<std::pair<KeyT, ValueT>, 0>>
//     class MapVector;
// Clang's default PrintingPolicy has SuppressDefaultTemplateArgs TRUE, so the
// USE SITE prints only the two written arguments.  Read back out of the real
// census abort for dbo__src__Transforms__sdsc_bundle__SchedulerPasses.cpp:
//     cpp_type: llvm::MapVector<long, std::pair<VariableOperator,
//                               std::vector<VariableDefinition::OperandType>>>
// -- two arguments, not four.  So the restatement below takes TWO parameters and
// pins the map/vector choice in nested typedefs, which is exactly the fix
// rules/smallset records for llvm::SmallSet<long, _, std::less<long>>.  A
// four-parameter restatement would record a four-argument key and never match.
//
// REPRESENTATION: Vec<(K, V)>, NOT BTreeMap.
// This is the one property that must not be fudged, and it is why MapVector does
// not reuse the BTreeMap model that rules/densemap, rules/map and
// rules/unordered_map share.  DenseMap leaves its iteration order UNSPECIFIED,
// so an ordered BTreeMap is an order-REFINING translation there.  MapVector's
// entire reason for existing over DenseMap is that iteration yields entries in
// INSERTION ORDER -- LLVM implements it as a vector of entries plus an index
// map, and dt_src uses it in compiler passes (FlatteningLocalRegions.cpp:76,
// BufferExpansion.cpp:451, ConstructThreeStagePipeline.cpp:525,
// UnitTypeDiscovery.h:39, RoutingGraph.h:127, SymbolExpr.h:82) whose OUTPUT
// ORDER depends on it.  A BTreeMap-backed model compiles, iterates in key
// order, and silently reorders what those passes emit.  So the model is a Vec of
// entries, matching LLVM's own implementation.

#include <utility>
#include <vector>

namespace llvm {

template <typename KeyT, typename ValueT> class MapVector {
  // The defaulted parameters of the real declaration, pinned here instead of
  // being restated as template parameters -- see the header comment.
  typedef std::vector<std::pair<KeyT, ValueT>> VectorType;
  VectorType Vector;

public:
  typedef std::pair<KeyT, ValueT> value_type;
  MapVector();
};

} // namespace llvm

// The container type itself.  This is what the census abort needs: the TU uses
// the MapVector only as a mapped member type, so the type entry alone is what
// clears `cpp_type: llvm::MapVector<...>`.
template <typename T1, typename T2> using t1 = llvm::MapVector<T1, T2>;
