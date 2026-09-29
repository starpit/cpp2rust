// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::cl -- COMMAND-LINE FLAG HOLDERS (llvm/Support/CommandLine.h).
//
// WHY A VALUE MODEL IS THE HONEST ONE
// -----------------------------------
// Every `cl::opt` in this corpus is a FILE-STATIC GLOBAL holding one value,
// e.g. dbo/src/Transforms/GrowTrackers.cpp:34
//     static llvm::cl::opt<bool> DisableThisPass(
//         "dbo-grow-trackers-disable", llvm::cl::desc("..."),
//         llvm::cl::init(false));
// read later as a plain `bool`.  CommandLine.h:1422-1447 shows what the object
// IS: `opt_storage<DataType, false, false>` is literally `{ DataType Value;
// OptionValue<DataType> Default; }` with `operator DataType() const`.  So the
// faithful Rust model of `cl::opt<T>` is `T` itself, and of
// `opt_storage<T, false, false>` likewise `T` -- the implicit conversion
// becomes the identity, which is exactly what it is.
//
// ARGV PARSING, CALLED OUT AS INSTRUCTED: 13 sites really do call
// `cl::ParseCommandLineOptions` -- dxp/tools/DxpOptMain.cpp:250 and
// dbo/tools/dbo-opt/dbo-opt.cpp among them, all of them tool `main()`s, and all
// of them use the 2-3 arg `cl::opt<T, /*ExternalStorage=*/true>` form.  THIS
// MODULE DOES NOT MODEL THAT.  A value model cannot observe argv, so for those
// TUs the flags would keep their compiled-in defaults.  Those TUs are driver
// mains, not the library TUs this row was measured on (the A-row census records
// only the 1-ary `llvm::cl::opt<T>` spelling); the 2-ary ExternalStorage
// spelling is deliberately NOT keyed here so it keeps aborting loudly rather
// than being silently wrong.
//
// KEYED: only the TYPE names the census measured as placeholders, plus the one
// free function (`cl::init`) whose body is unambiguous.
//   t1  llvm::cl::opt<T1>                       18 A-TUs
//   t2  llvm::cl::opt_storage<T1, T2, T3>       16 A-TUs; the DECLARING class of
//       Value/getValue/operator DataType -- `opt` inherits them (header:1450),
//       and a rule keyed on `opt` for an inherited member is a DEAD key
//       (the measured llvm::FailureOr shape).  Keyed on the base.
//   t3  llvm::cl::initializer<T1>               20 A-TUs -- header:430, a
//       one-field `const Ty &Init` carrier produced by cl::init.
//   t4  llvm::cl::NumOccurrencesFlag            13 A-TUs -- header:113, a plain
//       C enum with values 0x00..0x04; modelled as i32, its underlying type.
//   f1  llvm::cl::init(const T1 &)              returns the value it carries.
//
// DELIBERATELY NOT KEYED, and why -- an unreached key is an unchecked key:
//   * llvm::cl::ValuesClass (13 A-TUs) and llvm::cl::OptionEnumValue (13):
//     header:679/693.  OptionEnumValue is `{ StringRef Name; int Value;
//     StringRef Description; }` and ValuesClass is a SmallVector of them.  The
//     corpus only ever CONSTRUCTS them (clEnumValN inside cl::values, e.g.
//     dcc/src/Transform/Sentient/RegisterInitialization.cpp:39-66) and never
//     reads a field, so a unit model would pass every site -- but it would also
//     be a model with no observer, i.e. unfalsifiable.  Left unmapped and named
//     in the report instead of guessed.
//   * llvm::cl::initializer<char[N]> (the `initializer_chararr_arr_` spelling,
//     15 A-TUs): the recorded spelling of an array type is not yet read back
//     verbatim, and keying it from the mangled name is exactly the guess this
//     project forbids.
//   * `opt`'s variadic constructor `template <class... Mods> explicit
//     opt(const Mods &...)` (header:1500ff): a rule signature cannot spell a
//     pack, so the construction expressions stay unmapped.  This is why the
//     placeholder count drops but does not reach zero.
//   * cl::desc / cl::cat / cl::values / cl::Hidden and the whole Option base.

namespace llvm {
namespace cl {

// Restated from llvm/Support/CommandLine.h:113.
enum NumOccurrencesFlag {
  Optional = 0x00,
  ZeroOrMore = 0x01,
  Required = 0x02,
  OneOrMore = 0x03,
  ConsumeAfter = 0x05
};

// Restated from llvm/Support/CommandLine.h:430.
template <class Ty> struct initializer {
  const Ty &Init;
  initializer(const Ty &Val);
};

// Restated from llvm/Support/CommandLine.h:1354/1422.  The primary template is
// 3-ary; the corpus only instantiates <T, false, false>.
template <class DataType, bool ExternalStorage, bool isClass>
class opt_storage {
public:
  DataType &getValue();
  operator DataType() const;
};

// Restated from llvm/Support/CommandLine.h:1450.  Only the first parameter is
// recorded by the census; the other two are defaulted and do not appear in the
// recorded spelling.
template <class DataType, bool ExternalStorage = false,
          class ParserClass = int>
class opt : public opt_storage<DataType, ExternalStorage, false> {};

// llvm/Support/CommandLine.h -- initializer<Ty> init(const Ty &Val).
template <class Ty> initializer<Ty> init(const Ty &Val);

} // namespace cl
} // namespace llvm

template <typename T1> using t1 = llvm::cl::opt<T1>;

// MEASURED, and it cost a regen: writing this as
// `template <typename T1, bool T2, bool T3> using t2 = opt_storage<T1,T2,T3>`
// recorded as `llvm::cl::opt_storage<T1, true, true>` -- the preprocessor does
// NOT keep a non-type bool parameter as a wildcard, it prints a value.  The
// census spelling is `<bool, false, false>`, so `<T1, true, true>` was a DEAD
// key.  The bools are therefore written LITERALLY.
template <typename T1> using t2 = llvm::cl::opt_storage<T1, false, false>;

// t3 REMOVED AFTER MEASUREMENT.  `template <typename T1> using t3 =
// llvm::cl::initializer<T1>;` records fine, but it turned two bucket-A TUs into
// bucket B:
//   LLVM ERROR: unsupported unmapped type `char[_]` has no model in types_,
//   while mapping `llvm::cl::initializer<char[_]>`
// 15 A-TUs instantiate `cl::initializer<char[N]>` (from `cl::init("some string")`
// -- e.g. dcc/.../Deuniform.cpp).  While the class was UNMAPPED the converter
// emitted one placeholder and carried on; once the key MATCHES, the converter
// must map the template ARGUMENT, and `char[_]` has no model anywhere in the
// tree, so it aborts.  Keying initializer therefore requires a `char[_]` model
// first (a rules/carray row), and until that exists this key is a NET
// REGRESSION, not a fix.  Left out deliberately; measured, not guessed.

using t4 = llvm::cl::NumOccurrencesFlag;


// ---------------------------------------------------------------------------
// SWALLOW FIX -- /home/agent/work/SWALLOW-AUDIT.md row #3.  t1 `llvm::cl::opt<T1>`
// binds its ONLY placeholder to the comma-joined tail of a 2-/3-ary
// instantiation (arity is not part of the bucket key; a same-depth comma is not
// a delimiter):
//   LLVM ERROR: unsupported unmapped type `std::string, true`
//   LLVM ERROR: unsupported unmapped type `DCC::ProgIRFormat, false,
//       mlir::detail::PassOptions::GenericOptionParser<DCC::ProgIRFormat>`
// The value model is UNAFFECTED by the extra arguments: ExternalStorage only
// changes WHERE the DataType lives, and ParserClass only how argv is parsed --
// neither is observable in a value model, so both keys map to T1 exactly as t1
// does.  (The argv caveat above is unchanged and still applies.)
// SWALLOW-SAFETY: each key demands a LITERAL `, true` / `, false, ` that cannot
// occur in the bare `llvm::cl::opt<Foo>` t1 serves, so t1's traffic is
// untouched; and the two new keys cannot match each other.  Non-type bool
// arguments record LITERALLY, not as `_` -- proof is the sibling t2
// `llvm::cl::opt_storage<T1, false, false>` in this very module.  In t6 the
// `false` is NON-TRAILING (T2 follows), so SuppressDefaultTemplateArgs, which
// drops only trailing defaults, keeps it.
template <typename T1> using t5 = llvm::cl::opt<T1, true>;
template <typename T1, typename T2> using t6 = llvm::cl::opt<T1, false, T2>;

// ---------------------------------------------------------------------------
// t770 / f670 / f671 -- `llvm::cl::list_storage<long, bool>`, 4 emitted sites / 1 file
// (fresh37: dataflow-scheduler/lib/Transforms/TileSCFForLoops.cpp, emitted 4724, 4739,
// 4758 `.size()` and 5014 `.empty()`).
//
// ⭐ THE VECTOR MODEL IS FORCED BY THE HEADER, NOT CHOSEN.  CommandLine.h:1610-1613 --
//     template <class DataType> class list_storage<DataType, bool> {
//       std::vector<DataType> Storage;
//       std::vector<OptionValue<DataType>> Default;
//       bool DefaultAssigned = false;
// -- the `<DataType, bool>` PARTIAL SPECIALISATION is the INTERNAL-storage one and it IS a
// `std::vector<DataType>` with a thin API over it (the header's own comment: "Originally
// this code inherited from std::vector ... implements the minimum subset of the
// std::vector API required for all the current clients").  `Default` and
// `DefaultAssigned` are the cl::init bookkeeping, unobservable in a value model exactly as
// `opt_storage`'s `Default` is unobservable in t2's.  So `list_storage<T, bool>` IS
// `Vec<T>`, which is the same "a cl holder IS its value" model this module already rests
// on -- no new claim.
//
// ⭐⭐ AND THE CONVERTER HAS ALREADY COMMITTED TO IT INDEPENDENTLY, which is what makes this
// a completion rather than a guess.  In the SAME emitted file the field is
//     4133:    pub tileSizes: Vec<i64>,
// and the two OTHER things the C++ does to `tileSizes` are ALREADY emitted correctly
// AGAINST A `Vec<i64>`, through the converter's own vector handling and not through any
// `list_storage` rule:
//     C++ :185/:187  tileSizes[i]            -> 4761/4777  self.tileSizes[(i)]
//     C++ :196       for (int64_t t : tileSizes)
//                                            -> 4791/4792  for .. in 0..(self.tileSizes.len())
//                                                          self.tileSizes[tile_size].clone()
// So `Vec<T1>` is not merely compatible with the field the converter chose, it is that
// field.  A DIFFERENT model here would contradict text already in the file.
//
// ⛔ ONLY `size` AND `empty` ARE READ ON THIS RECEIVER, which is what makes the type key
// complete rather than the "trade 4 loud placeholders for 4 census-INVISIBLE textual
// calls" bargain -- AN UNMAPPED MEMBER DOES NOT ABORT, it is emitted textually at rc=0
// with no placeholder token.  Censused over all 58 emitting `.rs` at fresh37/out:
//   grep -rohE 'Cpp2RustUnmapped_llvm_cl_list_storage_long__bool_[^A-Za-z0-9_][^a-zA-Z]*[.] *[A-Za-z_]+' \
//     | sed 's/.*\. *//' | sort | uniq -c    ->   1 empty / 2 size
// (2 not 3 for `size` because ONE of the three is LINE-WRAPPED by rustfmt between the cast
// and the `.size()` -- emitted 4738-4740 -- and a single-line regex cannot see across it.
// Reconciled against the direct per-line grep, which finds all four: 4724, 4739, 4758
// `size`, 5014 `empty`.  3 + 1 = 4 = the census site count.)  ⛔ THE OTHER TWENTY-ODD
// MEMBERS OF THE SPECIALISATION ARE LEFT UNDECLARED so a site that reads one FAILS LOUDLY:
// `begin`, `end`, `push_back`, `operator[]`, `clear`, `erase`, `insert`, `front`,
// `addValue`, `getDefault`, `assignDefault`, `overwriteDefault`, `isDefaultAssigned`,
// `operator std::vector<DataType> &`, `operator ArrayRef<DataType>`, `operator&`.  Note
// that `operator[]` and `begin`/`end` ARE used by this very TU -- and, per the emitted
// lines quoted above, the converter serves them from its own `Vec` handling WITHOUT ever
// asking for a `list_storage` member, which is why leaving them undeclared costs nothing.
//
// ⛔ NOT A cl::opt SIBLING REFUSAL.  The measured refusals next door do not apply here and
// each was re-checked rather than inherited:
//   * `cl::OptionEnumValue` / `cl::ValuesClass` are refused because the corpus only ever
//     CONSTRUCTS them and never reads a field, so a unit model would be UNFALSIFIABLE.
//     The opposite holds here: both keyed members are READ, at 4 sites, and each returns a
//     value the surrounding code branches on (`size() != nested_loops.size()`, `!empty()`).
//   * `PassOptions::Option<T>` is refused because a VARIADIC constructor makes the
//     compiled-in `init(...)` unreachable, measured to flip `init(true)` to `false`.  That
//     is a CONSTRUCTION defect and this row keys NO CONSTRUCTOR -- no `fN` builds a
//     `list_storage`, the 4 sites are all DerivedToBase cast receivers of a field that
//     already exists.  ⚠️ The argv caveat in this module's header is UNCHANGED and still
//     applies: a value model cannot observe argv, so `tileSizes` holds its compiled-in
//     default.  Nothing in this row makes that better or worse.
//   * `cl::initializer<T1>` was removed here because keying it forced the converter to map
//     `char[_]`, which has no model and ABORTS.  This key's only template argument is
//     `long` in the corpus and `Vec<T1>` imposes no requirement on it beyond what the
//     already-emitted `Vec<i64>` field proves, so there is no analogous forced mapping.
//
// ⛔ SWALLOW-SAFETY.  `GetTypeMapKey` truncates at the first `<`, so the bucket is
// `llvm::cl::list_storage`; `grep -rn 'list_storage' rules/*/src.cpp` finds it in NO other
// module, so the bucket holds exactly t770.  The key has ONE placeholder, and
// `matchTemplate`'s capture (`findNextLiteralSameDepth`) stops it at the LITERAL `, bool>`
// tail -- and `list_storage` is 2-ary (CommandLine.h:1610), so there is no third argument
// for T1 to swallow past a same-depth comma even in principle.  The `bool` is written
// LITERALLY, the t2 (`opt_storage<T1, false, false>`) discipline in this file: t2 cost a
// regen by writing `<T1, true, true>` when the census spelling was `<bool, false, false>`.
// Here `bool` is a TYPE argument, so it records as the concrete type it is.
// ⚠️ NO DEFAULTED ARGUMENTS on `list_storage` (unlike `opt`, whose 2nd and 3rd are
// defaulted and drop out of the recorded spelling), so there is nothing for
// SuppressDefaultTemplateArgs to remove and the recorded key cannot drift.
// ⚠️ `int64_t` is `long` on this target, which is the spelling the recorded key carries;
// `T1` wildcards it, so the key does not depend on that.

namespace llvm {
namespace cl {

// Restated from llvm/Support/CommandLine.h:1610.  The PRIMARY template is 2-ary and is the
// EXTERNAL-storage form; only the `<DataType, bool>` partial specialisation -- the internal
// one that owns a `std::vector<DataType>` -- is declared with members, and only the two
// members the emitting corpus reads.  Both are `const` in the header (:1627, :1629), so
// f670/f671 take `const &` receivers (the f650 / `FileLineColLoc::getLine` precedent).
template <class DataType, class StorageClass> class list_storage;

template <class DataType> class list_storage<DataType, bool> {
public:
  unsigned long size() const;
  bool empty() const;
};

} // namespace cl
} // namespace llvm

// t770 -- `llvm::cl::list_storage<long, bool>`, 4 sites / 1 file.
template <typename T1> using t770 = llvm::cl::list_storage<T1, bool>;

// f670 -- `unsigned long llvm::cl::list_storage<T1, bool>::size() const`.
// CommandLine.h:1627 is `return Storage.size();`, i.e. the vector length.
template <typename T1>
unsigned long f670(const llvm::cl::list_storage<T1, bool> &a0) {
  return a0.size();
}

// f671 -- `bool llvm::cl::list_storage<T1, bool>::empty() const`.
// CommandLine.h:1629 is `return Storage.empty();`.
template <typename T1> bool f671(const llvm::cl::list_storage<T1, bool> &a0) {
  return a0.empty();
}

// ⭐⭐⭐ MEASURED CORRECTION TO THE SITE COUNT ABOVE -- the same finding as rules/mlir's
// t770 block, and it applies to this row for the same reason.  The "4 emitted sites" figure
// is from the fresh37 census, whose binary was the 602a787f pin.  AT THE CURRENT PIN
// (968ab2be, HEAD 8f467983) THE PLACEHOLDER TOKEN IS ALREADY GONE AND THE DEFECT IS WORSE:
// HEAD~1 is `6f102175 converter: CK_UncheckedDerivedToBase -- a derived-to-base is never a
// Rust cast`, so the DerivedToBase cast that carried
// `Cpp2RustUnmapped_llvm_cl_list_storage_long__bool_` is no longer emitted and `size()` /
// `empty()` land DIRECTLY ON THE `Vec<i64>` FIELD, emitted SILENTLY AND TEXTUALLY as
// undefined methods -- no token, invisible to every bucket census.  Measured, both legs on
// the same snapshot binary, ir.v35 + this module at clean HEAD vs. with t770/f670/f671, in
// dataflow-scheduler/lib/Transforms/TileSCFForLoops.cpp:
//     BEFORE  self.tileSizes.size()   3 (emitted 4670, 4681, 4696)
//             self.tileSizes.empty()  1 (emitted 4945)
//             self.tileSizes.len() as u64  0     self.tileSizes.is_empty()  0
//     AFTER   self.tileSizes.size()   0          self.tileSizes.empty()     0
//             self.tileSizes.len() as u64  3     self.tileSizes.is_empty()  1
// 3 + 1 = 4 = the census site count, all four closed.  ⭐ IN-FILE CONTROL, unchanged across
// the pair: `Cpp2RustUnmapped_mlir_Pass_ListOption_long_` 1 -> 1 (emitted 4365), and the
// `self.tileSizes[(i)]` / `for .. in 0..(self.tileSizes.len())` lines at 4697/4713/4727/4728
// are byte-identical -- so nothing but the two keyed members moved, and the `Vec` model this
// row asserts is demonstrably the one those untouched lines already rely on.


// ===========================================================================
// PASS 2026-09-29: t1900 `llvm::cl::initializer<char[_]>` -- THE KEY THIS FILE
// RECORDED AS UNSPELLABLE, NOW MEASURED SPELLABLE.
//
// ⛔ THE RECORDED REFUSAL ABOVE IS OVERTURNED BY MEASUREMENT, NOT BY ARGUMENT.
// The t3 block says: "llvm::cl::initializer<char[N]> (the `initializer_chararr_arr_`
// spelling, 15 A-TUs): the recorded spelling of an array type is not yet read back
// verbatim, and keying it from the mangled name is exactly the guess this project
// forbids."  ⭐ It IS read back verbatim.  Probed in this module and read out of
// `ir_src.json`:
//     using tP = llvm::cl::initializer<char[1]>;   ->  "llvm::cl::initializer<char[_]>"
//     using tQ = char[1];                          ->  "char[_]"
//     template <typename T1> using tR = T1[1];     ->  "T1[_]"
// The ARRAY BOUND records as `_` whatever integer is written, so ONE fully concrete
// key covers every `char[N]` instantiation in the corpus -- `cl::init("")` (N=1) and
// `cl::init("host_senprog.so")` (N=17) land on the same recorded key.  This is no
// longer a guess from a mangled name; it is the recorded string.
//
// ⭐ AND THE KEY IS FULLY CONCRETE, WHICH IS WHAT MAKES IT SAFE.  The t3 refusal's
// OTHER half -- "once the key MATCHES, the converter must map the template ARGUMENT,
// and `char[_]` has no model anywhere in the tree, so it aborts" -- was measured
// against the PLACEHOLDER key `initializer<T1>`, whose `T1` has to bind to a mapped
// type.  `initializer<char[_]>` has NO placeholder, so `matchTemplate`'s capture has
// nothing to capture and the `char[_]` argument is never itself looked up.  ⛔ AND
// `char[_]` IS DELIBERATELY *NOT* KEYED HERE: a concrete `char[_]` key would match
// EVERY one-dimensional char array in the corpus (`char buf[256]` included), which is
// a blast radius this row has not censused.  The probe above proves it is spellable;
// spelling it is a separate row.
//
// THE ROW.  `llvm::cl::initializer<char[_]>` is the 7th-largest `searched as:` string
// in the fresh38 403-TU sweep (154 occurrences) and it is the MEASURED SUCCESSOR GATE
// of rules/mlir's t1200/t1201: that block records, verbatim, that after t1201 landed
// dbo/src/Transforms/EmitSpyreCode.cpp moved from
//   `mlir::Pass::Option<std::string, llvm::cl::parser<std::string>>`  to
//   `llvm::cl::initializer<char[_]>` ... reached while converting
//   `mlir::dbo::impl::EmitSpyreCodePassBase<...>::EmitSpyreCodePassBase`
//   at CommandLine.h:430:28
// i.e. reached INSIDE the generated pass-base constructor, lowering the
// `::llvm::cl::init("")` argument of the variadic `Option` ctor.
//
// THE MODEL, AND WHAT IT DOES *NOT* CLAIM.  CommandLine.h:430 is
//     template <class Ty> struct initializer { const Ty &Init; ... };
// -- a one-field carrier.  For `Ty = char[N]` the field IS the char array, and the
// converter already lowers a C++ char-array literal as a Rust BYTE-STRING SLICE:
// measured in fresh38/out/sys-arch-spec__isa__isa.cpp.rs:959, `(b"]\r" as &[u8])`.
// `Vec<u8>` is the owning form of that, chosen over `&[u8]` because a rule target
// cannot spell a lifetime for a type key.  ⛔ NO MEMBER IS DECLARED: `Init` is the
// only one, it is never read by name anywhere in the corpus (`rg -n
// '\.Init\b' repos/dt_src` over the corpus finds no `cl::initializer` receiver), and
// leaving it undeclared makes any future read a loud rustc error rather than a wrong
// answer.
// ⛔⛔ AND THIS KEY DOES NOT MAKE `cl::init(...)`'s VALUE REACH AN `Option`.  It gives
// the CARRIER a model so the translation proceeds; the carrier is then handed to
// `Option`'s VARIADIC ctor (PassOptions.h:192-204), which no rule signature can spell,
// so the constructed option still does not receive it.  That is the g2964 payload
// problem and it is NOT solved here -- see the report for the specification.  This row
// claims exactly one thing: the carrier type has a model.
//
// SWALLOW-SAFETY.  `GetTypeMapKey` truncates at the first `<`, so the bucket is
// `llvm::cl::initializer`.  `grep -n 'cl::initializer' rules/*/src.cpp` finds the name
// only in this module's comments and in rules/mlir's t700 note; the bucket therefore
// holds exactly t1900 and it is FULLY CONCRETE, so the same-depth-comma capture bug
// cannot fire.  No `>` occurs inside the argument, so the `operator>=` angle-depth
// desync class does not apply either.
// ===========================================================================
namespace llvm {
namespace cl {
// Restated from llvm/Support/CommandLine.h:430.  Re-declared here (the declaration
// near the top of this file was removed with t3) purely so the key can be spelled;
// no member is declared, deliberately.
template <class Ty> struct initializer;
}  // namespace cl
}  // namespace llvm

using t1900 = llvm::cl::initializer<char[1]>;
