// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model, the corpus enumeration, the argv-parsing caveat and
// the list of names deliberately left unmapped.
//
// MODEL: `llvm::cl::opt<T>` IS its value.  opt_storage<T, _, _> is
// `{ T Value; ... }` with `operator T()`, and opt derives from it, so both map
// to `T1`.  `cl::initializer<T>` is a one-field carrier for the default that
// `cl::init(x)` builds, so it too maps to `T1` and `f1` is the identity.
//
// No tgt_refcount.rs: every target here is a plain value with no reference
// shape to differ on, and a partial tgt_refcount.rs that omitted a type key
// would abort at load (the rules/iostream t1 shape).

use libcc2rs::*;

fn t1<T1: Default>() -> T1 {
    Default::default()
}

fn t2<T1: Default>() -> T1 {
    Default::default()
}

fn t4() -> i32 {
    0
}


fn t5<T1: Default>() -> T1 {
    Default::default()
}

fn t6<T1: Default, T2>() -> T1 {
    Default::default()
}

// t770 `llvm::cl::list_storage<T1, bool>` -> `Vec<T1>`, 4 sites / 1 file.
//   CommandLine.h:1610-1613: the `<DataType, bool>` partial specialisation IS a
//   `std::vector<DataType>` with a thin API over it (the header says so itself), and its
//   `Default` / `DefaultAssigned` fields are the same unobservable cl::init bookkeeping
//   that t2's `opt_storage` carries.  ⭐ The converter has ALREADY emitted the owning
//   field as `pub tileSizes: Vec<i64>` and already serves `tileSizes[i]` and the range-for
//   over it from its own `Vec` handling, so this is the model in the file, not a new claim.
//   See src.cpp for the site census and for why the cl::opt sibling refusals do not apply.
fn t770<T1>() -> Vec<T1> {
    Vec::new()
}

// f670 `list_storage<T1, bool>::size() const` -> THE VECTOR LENGTH.
//   CommandLine.h:1627 is `return Storage.size();`.  ⛔ NO `- 1` here, unlike
//   rules/stringref's `f7`: that model is a NUL-TERMINATED `Vec<libc::c_char>` and this one
//   is a plain element vector with no terminator, so `len()` IS the count.  `size_type` is
//   `std::vector<long>::size_type`, i.e. `unsigned long` -> `u64`.
fn f670<T1>(a0: &Vec<T1>) -> u64 {
    a0.len() as u64
}

// f671 `list_storage<T1, bool>::empty() const` -> `is_empty()`.
//   CommandLine.h:1629 is `return Storage.empty();`.  Again no terminator to discount, so
//   this is `is_empty()` and not rules/stringref's `f5` (`len() <= 1`).
fn f671<T1>(a0: &Vec<T1>) -> bool {
    a0.is_empty()
}

// t1900 `llvm::cl::initializer<char[_]>` -> `Vec<u8>`.
//   CommandLine.h:430 is a one-field carrier `const Ty &Init`; for `Ty = char[N]` the
//   field IS the char array, and the converter already lowers a C++ char-array literal
//   as a byte-string slice (`(b"]\r" as &[u8])`, fresh38 isa.cpp.rs:959).  `Vec<u8>` is
//   the owning form of that -- a rule TYPE target cannot spell a lifetime.
//   ⛔ No member is declared: `Init` is the only one and it is never read by name in the
//   corpus, so a future read fails loudly instead of answering.
//   ⛔ This does NOT carry `cl::init(...)`'s value into an `Option`: that goes through
//   the variadic ctor, which no rule signature can spell.  See src.cpp.
fn t1900() -> Vec<u8> {
    Vec::new()
}

// t1910 `llvm::cl::opt_storage<T1, false, true>` -> `T1`, the same value model t2
//   uses for the `isClass == false` specialisation -- but here it is the HEADER'S
//   OWN STATEMENT rather than a modelling choice: CommandLine.h:1354 declares the
//   primary template as `class opt_storage : public DataType`, so the storage IS-A
//   DataType by inheritance.  `Default` is the unobservable cl::init bookkeeping
//   t2 already drops.  See src.cpp for why the `true` matters and for the list of
//   members left undeclared so they stay loud.
fn t1910<T1: Default>() -> T1 {
    Default::default()
}

// f1910 `opt_storage<T1, false, true>::getValue()` -> THE IDENTITY.
//   CommandLine.h:1359 is literally `return *this;`.  No value is invented and
//   none can be lost.  Non-const receiver only; the `const` overload is a
//   different key and is deliberately unkeyed.
unsafe fn f1910<T1>(a0: &mut T1) -> &mut T1 {
    a0
}

// f1911 `opt<T1>::operator=(const T1 &)` -> ASSIGN, THEN RETURN THE RECEIVER.
//   CommandLine.h:1478 is `{ this->setValue(Val); return this->getValue(); }` and
//   `setValue` is `{ DataType::operator=(V); if (initial) Default = V; }` with
//   `initial` defaulted false at every corpus call site, so the observable effect
//   is exactly `*this = Val` returning `*this`.
//   ⚠️ `Clone::clone(&(*a1))`, NOT `a1.clone()`: a `const &` parameter cloned in a
//   reference-producing position emits `&((*x).clone())`, a reference to a
//   temporary.
//   ⚠️ THE `'a` BINDER IS REQUIRED, measured: with two reference parameters rustc
//   cannot elide the returned lifetime (`E0106: expected named lifetime
//   parameter`), and the rule preprocessor turns that into a core dump with no
//   `OK` line.  `a1` keeps its own anonymous lifetime; only `a0` and the result
//   are tied, which is what the C++ does.
//   ⛔⛔ THE ASSIGNMENT MUST GO THROUGH A NESTED `fn`, NOT `*a0 = ...`.  MEASURED.
//   The converter substitutes `a0` AND `*a0` with the SAME argument text, so the
//   obvious body `*a0 = Clone::clone(&(*a1)); a0` emitted
//       &mut (*o) = Clone::clone(&(*&(*s)));
//   -- an assignment whose left-hand side is a `&mut` expression, which is not a
//   place: `error[E0070]: invalid left-hand side of assignment`.  Passing the
//   reference to a nested `fn` keeps the deref INSIDE a body the converter does
//   not rewrite, so `a0` is substituted only in argument position where it is
//   already a `&mut`.  ⚠️ The `fn` is nested in the block on purpose: a
//   FILE-LEVEL helper item in a `tgt_*.rs` is NOT copied into the emitted output
//   (it type-checks, regenerates OK, and the converter emits the bare name).
unsafe fn f1911<'a, T1: Clone>(a0: &'a mut T1, a1: &T1) -> &'a mut T1 {
    fn __cl_assign<T: Clone>(dst: &mut T, src: &T) {
        *dst = Clone::clone(src);
    }
    __cl_assign(a0, a1);
    a0
}

// t2410 `llvm::cl::OptionEnumValue` -> `libcc2rs::ClOptionEnumValue`, and
// t2411 `llvm::cl::ValuesClass`     -> `Vec<ClOptionEnumValue>`.
//
// ⛔⛔ THE `()` MODEL THAT USED TO BE HERE EMITTED TEXT THAT IS NOT RUST, and it is
//   the reason this block was rewritten.  Both types are CONSTRUCT-ONLY on this
//   corpus -- that measurement (0 declared objects of either type against 114
//   mentions of `clEnumVal*`/`OptionEnumValue`; 0 hits for `.Description` /
//   `->Description`) is CORRECT and the conclusion drawn from it was BACKWARDS.
//   `clEnumValN` is a macro expanding to an AGGREGATE INITIALISER
//   (CommandLine.h:686), and both models' `VisitInitListExpr` emit a mapped record
//   type's aggregate init as a Rust STRUCT LITERAL whose PATH is the mapped type's
//   own name (`GetUnsafeTypeAsString`, converter.cpp:7964 /
//   converter_refcount.cpp:1844) and whose field names are the C++ field names
//   verbatim (`GetNamedDeclAsString(field)` over the REAL `RecordDecl`).  So `()`
//   produced `() { Name : ... , Value : ... , Description : ... , }` -- a struct
//   literal body with `()` as its path -- at 46 sites in 13 TUs, every one of which
//   rustfmt rejects with `error: struct literal body without path`.
//   ⭐ AN OPAQUE `()` KEY IS LEGITIMATE ONLY WHERE THE CORPUS NEVER TOUCHES THE
//   VALUE AT ALL.  CONSTRUCTION IS A TOUCH: it is the one syntactic position in
//   which a mapped type's NAME is printed.  This is the fifth member of the
//   documented "a type key that is worse than no key" class and the first one where
//   the hazard is construction rather than member access.
//
//   THE MODEL.  CommandLine.h:679 is `struct OptionEnumValue { StringRef Name;
//   int Value; StringRef Description; }` and :693's `ValuesClass` holds a
//   `SmallVector<OptionEnumValue, 4> Values`, so the faithful shapes are a
//   three-field struct and a `Vec` of it.  The struct lives in `libcc2rs` (cl.rs)
//   rather than in this file because a FILE-LEVEL item in a `tgt_*.rs` is NOT
//   copied into the emitted output -- the f1911 note above records that measurement
//   -- so a struct declared here would type-check the rule and leave the emission
//   naming an undefined type.
//   ⭐ ITS THREE TYPE PARAMETERS ARE THE TWO MODELS, NOT GENERALITY.  The field
//   VALUES are converted by `ConvertVarInit(field->getType(), ...)` per model
//   (`StringRef` -> `Vec<libc::c_char>` unsafe, `Value<Vec<u8>>` refcount; `int` ->
//   `i32` vs `Value<i32>`) while the PATH is printed once, so no single concrete
//   field type can serve both arms.  The defaults in cl.rs are the unsafe shapes,
//   matching this file.
//   ⛔⛔ AND THAT MAKES THIS A PARTIAL FIX, NOT A CLOSE.  An earlier version of this
//   note claimed the parameters are INFERRED from the field expressions in
//   struct-literal expression position, so one bare path served both arms.
//   REFUTED with rustc: the converter ANNOTATES the temporary
//   (`let mut __tmp_68: Vec<ClOptionEnumValue> = ...` unsafe,
//   `let __tmp_61: Value<Vec<ClOptionEnumValue>> = ...` refcount), and a defaulted
//   parameter in TYPE position is not an inference variable -- so the bare path
//   pins the unsafe shapes on both legs.  The unsafe arm type-checks; the refcount
//   arm is 3x E0308 per site.  ⭐ Nothing reachable regresses -- every site is an
//   argument to the deliberately-refused `Cpp2RustUnmappedFn_values_N`, so an
//   intentional E0425 already sits upstream -- but the refcount arm went from
//   unparseable to ill-typed, not to correct.  See cl.rs.
//   ⛔ STILL NO MEMBER KEY, deliberately, and none is needed: the only reader of the
//   three fields anywhere is `ValuesClass::apply` (CommandLine.h:702), which is
//   reachable only by descending into `llvm::cl::values(...)` -- a variadic that no
//   rule `fN` signature can spell, left as a loud `Cpp2RustUnmappedFn_values_N`
//   placeholder.  A method call on either carrier therefore still fails `E0599`.
fn t2410() -> ClOptionEnumValue {
    Default::default()
}

fn t2411() -> Vec<ClOptionEnumValue> {
    Vec::new()
}
