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

