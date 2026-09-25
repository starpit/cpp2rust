// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::StringMap -- the container itself and its iterator.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/ADT/StringMap.h>
// ---------------------------------------------------------------------------
// Same reason as rules/densemap and rules/stringref: cpp-rule-preprocessor
// compiles this file with a fixed flag set, and the only flags that would reach
// LLVM's headers are absolute -I paths into whatever LLVM tree the target
// project happens to have built.  So the signatures are restated here, and a
// restatement matches iff it agrees with LLVM exactly.
//
// WHAT AN ITERATOR COMPARISON MEANS -- the same argument as rules/densemap
// ----------------------------------------------------------------------
// C++ is the specification.  LLVM's own body at llvm/ADT/StringMap.h:488 is
//
//     return LHS.Ptr == RHS.Ptr;
//
// i.e. IDENTITY OF POSITION -- the bucket address -- and NOT equality of the
// pointed-to values, exactly as DenseMap.h:1225, DenseSet.h:145 and
// ilist_iterator.h:175 are.  A value-comparing body would be SILENTLY WRONG for
// a map holding duplicate values, and that is measured rather than argued: the
// verification probe stores 7 under two different keys and checks that
// iterators to them are UNEQUAL.  Against clang-built C++ that is 0; a
// value-comparing body gives 1.
//
// So NO NEW RUNTIME TYPE IS NEEDED.  libcc2rs's map iterators
// (libcc2rs/src/iterators.rs) already have exactly this semantics -- PartialEq
// is position by key, with end() being `key: None` -- which is why
// rules/densemap's f1/f2 are one-liners and why these are too.  StringMap is a
// hash table and leaves its iteration order unspecified, so the ordered
// BTreeMap the map iterators walk is an order-REFINING translation: the same
// argument rules/unordered_map records for std::unordered_map and rules/densemap
// records for DenseMap.
//
// THE KEY IS NOT A TEMPLATE PARAMETER.  DenseMap is DenseMap<K, V> and its rule
// is generic in both; StringMap is StringMap<ValueTy> with the key fixed to a
// string.  So the iterator's Rust type is instantiated at the byte-string
// representation rules/string and rules/stringref use -- Vec<libc::c_char> in
// the unsafe model, Vec<u8> in the refcount model -- and is generic only in the
// VALUE.  That is what makes this module small, and it is also the reason it
// cannot simply be folded into rules/densemap.
//
// operator== IS A HIDDEN FRIEND, and getting that wrong costs a silent miss
// ------------------------------------------------------------------------
// llvm/ADT/StringMap.h:486-493 declares AND DEFINES both operators INSIDE the
// class:
//
//     friend bool operator==(const StringMapIterBase &LHS,
//                            const StringMapIterBase &RHS) { ... }
//
// A hidden friend is findable only by ADL on the operands, so restating it at
// NAMESPACE SCOPE as a template does not produce the same entity: the call in
// this file then reports "No viable function" and the rule silently never
// resolves, so the module builds and covers nothing.  rules/ilist hit exactly
// this, and rules/densemap's restatement (its f1/f2) is a friend inside
// DenseMapIterator for the same reason.  Declared as a friend below, the call
// resolves and the converter prints the signature string it searches for.
//
// The iterator type is named through StringMap's own nested typedef rather than
// spelled as `StringMapIterBase<T1, false>`.  StringMapIterBase takes a NON-TYPE
// template parameter (`template <typename ValueTy, bool IsConst>`,
// StringMap.h:442), and the nested typedefs (StringMap.h:220-221) are what the
// target code actually writes; going through them fixes the bool for us and
// still canonicalises to `llvm::StringMapIterBase<T1, false>` /
// `<T1, true>`, which is the type the converter reports.  This is the same
// route rules/densemap takes with `typename llvm::DenseMap<T1, T2>::iterator`.
//
// THE CONTAINER TYPE HAS A SECOND, DEFAULTED TEMPLATE PARAMETER
// ------------------------------------------------------------
// llvm/ADT/StringMap.h:130 is
//
//     template <typename ValueTy, typename AllocatorTy = MallocAllocator>
//     class StringMap : public StringMapImpl, private AllocatorHolder<AllocatorTy>
//
// and it is restated with both parameters because that is what LLVM declares.
//
// WHAT WAS ACTUALLY MISSING, MEASURED, was not the parameter arity: it was that
// the previous draft of this module had NO TYPE RULE FOR THE CONTAINER AT ALL.
// It restated StringMap only to carry the nested iterator typedefs, aliased t1/t2
// to the iterators, and never aliased the map itself, so the map reached the
// target unmapped:
//     error[E0425]: cannot find type `llvm_StringMap_unsigned_int__llvm_MallocAllocator_`
//     error[E0433]: cannot find module or crate `llvm_..._MallocAllocator_`
//     error[E0605]: non-primitive cast: `std::vec::Vec<i8>` as `usize`
// The mangled Rust name carries the defaulted allocator, which is what made the
// arity look load-bearing; it is not.  The rule IR records the signature as
// `llvm::StringMap<T1>` -- cpp-rule-preprocessor ELIDES the defaulted argument,
// the same elision rules/densemap records for DenseMap's three defaults -- and a
// one-parameter restatement was tried and ALSO matches (`7 0 1 1 1` in both
// models).  The two-parameter form is kept because it is what the header says,
// not because the match depends on it.  The third error was the subscript:
// `m["aa"]` built a StringRef key (Vec<c_char>) and then reached an operator[]
// with no rule, so the converter treated the receiver as an array and cast the
// key to an index.
//
// THE KEY IS A StringRef, NOT A TEMPLATE PARAMETER.  find/operator[] take
// `StringRef Key` BY VALUE (StringMap.h:237, :277), so the Rust parameter is the
// byte-string representation rules/stringref gives StringRef -- Vec<c_char> /
// Vec<u8> -- and the map is a BTreeMap keyed on exactly that.  StringRef is
// restated here (minimally, and only as far as being a complete type for a
// by-value parameter needs) for the same reason everything else in this file is.
//
// operator[] INSERTS.  StringMap.h:277 is `return try_emplace(Key).first->second;`
// so a read of an absent key CREATES a default-constructed value, exactly as
// std::map's does; the bodies are rules/densemap f7's entry()/or_default(), not
// a get().expect(), and that is the difference between operator[] and lookup
// that rules/densemap records at f30.
//
// find MISSES TO end().  StringMap.h:240 returns `end()` when FindKey fails, and
// MapIter::find_key already does precisely that (`key: None`), so find and end
// agree by construction rather than by coincidence -- which is what makes the
// probe's `empty.begin() == empty.end()` a real check.
//
// NOT MODELLED, deliberately -- each is a loud abort rather than a guessed body:
// insert / try_emplace / erase / lookup / count / contains / size / empty /
// clear on the map, the iterator's operator++ and operator-> / operator*, the
// iterator-to-const_iterator conversion (StringMap.h:482), the two-argument
// find(Key, FullHashValue) overload, StringMapKeyIterator and keys(), and
// StringMap's own == / != at StringMap.h:295/:314 (which are WHOLE-MAP value
// comparisons, a different question from the iterator's).  The surface here is
// the one the verification probe reaches: construct, subscript, find,
// begin/end, compare.

namespace llvm {

// llvm/Support/Allocator.h -- StringMap's DEFAULT allocator argument.  It is
// restated only so the default argument below names the same type LLVM's does;
// nothing about it is modelled.
class MallocAllocator {};

// llvm/ADT/StringRef.h -- the key type.  Restated minimally: find and
// operator[] take it BY VALUE, so it has to be a COMPLETE type here, but no
// member of it is used.  rules/stringref is what gives it a representation; the
// only thing that matters in this file is that the parameter type spells
// `llvm::StringRef`, which is the string the converter matches on.
class StringRef {
public:
  StringRef() = default;
  StringRef(const char *);
  StringRef(const StringRef &);
};

// Restated from llvm/ADT/StringMap.h:442.  Only the two hidden friends matter
// for signature matching, but the class has to be a template with the same
// parameter list (including the non-type `bool IsConst`) for the nested typedefs
// below to name the same canonical type LLVM's do.
template <typename ValueTy, bool IsConst> class StringMapIterBase {
public:
  StringMapIterBase() = default;

  // llvm/ADT/StringMap.h:486 -- a HIDDEN FRIEND, defined in-class.  See the
  // header comment: declared at namespace scope instead, this never resolves.
  friend bool operator==(const StringMapIterBase &LHS,
                         const StringMapIterBase &RHS);
  // llvm/ADT/StringMap.h:491
  friend bool operator!=(const StringMapIterBase &LHS,
                         const StringMapIterBase &RHS);
};

// Restated from llvm/ADT/StringMap.h:130.  THE SECOND PARAMETER AND ITS
// DEFAULT ARE LOAD-BEARING -- see the header comment; without them the type
// rule's string is `llvm::StringMap<T1>` and matches nothing.
template <typename ValueTy, typename AllocatorTy = MallocAllocator>
class StringMap {
public:
  // llvm/ADT/StringMap.h:139 -- StringMap() is the only constructor a
  // default-constructed map resolves to (unlike DenseMap's, whose default
  // argument makes it resolve to DenseMap(unsigned); see rules/densemap f14).
  StringMap();

  // llvm/ADT/StringMap.h:221 / :220
  using iterator = StringMapIterBase<ValueTy, false>;
  using const_iterator = StringMapIterBase<ValueTy, true>;

  // llvm/ADT/StringMap.h:223-230
  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;

  // llvm/ADT/StringMap.h:237 / :246 -- the key is a StringRef BY VALUE.
  iterator find(StringRef Key);
  const_iterator find(StringRef Key) const;

  // llvm/ADT/StringMap.h:277 -- `return try_emplace(Key).first->second;`, i.e.
  // it INSERTS a default-constructed value for an absent key.
  ValueTy &operator[](StringRef Key);
};

// Restated from llvm/ADT/StringSet.h:26.  `llvm::StringSet<>` is
// `StringMap<EmptyStringSetTag>`, and the tag is an EMPTY struct -- it declares
// no members at all, it exists only to give the map a zero-information value
// type.  The honest Rust counterpart is therefore the unit type.
//
// This rule closes a RECURSION gap, not an outer-match gap: the iterator rule
// `StringMapIterBase<T1, false>` above already matched with
// T1 = llvm::EmptyStringSetTag; what aborted was mapper.cpp's recursive
// descent into the substituted template argument (mapper.cpp:1189-1194), which
// requires the argument to have a rule of its own.
struct EmptyStringSetTag {};

} // namespace llvm

// The container type.  Written with ONE parameter and instantiating TWO: the
// default supplies llvm::MallocAllocator, so this canonicalises to the string
// the converter actually reports, `llvm::StringMap<T1, llvm::MallocAllocator>`.
template <typename T1> using t3 = llvm::StringMap<T1>;

// The value type of llvm::StringSet<>.  Not a template: it is a concrete empty
// tag struct, so the alias takes no parameters.
using t4 = llvm::EmptyStringSetTag;

template <typename T1> using t1 = typename llvm::StringMap<T1>::iterator;

template <typename T1> using t2 = typename llvm::StringMap<T1>::const_iterator;

template <typename T1>
bool f1(typename llvm::StringMap<T1>::iterator a,
        typename llvm::StringMap<T1>::iterator b) {
  return operator==(a, b);
}

template <typename T1>
bool f2(typename llvm::StringMap<T1>::iterator a,
        typename llvm::StringMap<T1>::iterator b) {
  return operator!=(a, b);
}

template <typename T1>
bool f3(typename llvm::StringMap<T1>::const_iterator a,
        typename llvm::StringMap<T1>::const_iterator b) {
  return operator==(a, b);
}

template <typename T1>
bool f4(typename llvm::StringMap<T1>::const_iterator a,
        typename llvm::StringMap<T1>::const_iterator b) {
  return operator!=(a, b);
}

template <typename T1> llvm::StringMap<T1> f5() { return llvm::StringMap<T1>(); }

template <typename T1>
T1 &f6(llvm::StringMap<T1> &o, llvm::StringRef k) {
  return o.operator[](k);
}

template <typename T1>
typename llvm::StringMap<T1>::iterator f7(llvm::StringMap<T1> &o,
                                          llvm::StringRef k) {
  return o.find(k);
}

template <typename T1>
typename llvm::StringMap<T1>::iterator f8(llvm::StringMap<T1> &o) {
  return o.begin();
}

template <typename T1>
typename llvm::StringMap<T1>::iterator f9(llvm::StringMap<T1> &o) {
  return o.end();
}

// The const-receiver forms.  As rules/densemap records at t3/f22-f24, the const
// iterator is a DIFFERENT type string (the trailing `true` forces the rest to be
// spelled), so each const overload needs its own rule even though both map to
// the same Rust type.
template <typename T1>
typename llvm::StringMap<T1>::const_iterator f10(const llvm::StringMap<T1> &o,
                                                 llvm::StringRef k) {
  return o.find(k);
}

template <typename T1>
typename llvm::StringMap<T1>::const_iterator f11(const llvm::StringMap<T1> &o) {
  return o.begin();
}

template <typename T1>
typename llvm::StringMap<T1>::const_iterator f12(const llvm::StringMap<T1> &o) {
  return o.end();
}
