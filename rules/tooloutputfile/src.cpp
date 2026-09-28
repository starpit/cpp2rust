// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::ToolOutputFile -- THE OUTPUT FILE WHOSE DESTRUCTOR UNLINKS IT.
//
// WHY THIS MODULE EXISTS AND WHY IT IS NOT rules/raw_ostream.  The two TUs that
// ask for this type are the PORT GOAL's OWN DRIVERS --
//     dxp/src/Driver/dxp-driver.cpp
//     dxp/tools/DxpOptMain.cpp
// -- and both were bucket B on exactly one abort:
//     LLVM ERROR: unsupported unmapped type `llvm::ToolOutputFile` has no model
//     in types_, while mapping `std::unique_ptr<llvm::ToolOutputFile>`
// A separate module rather than a block in rules/raw_ostream because the model
// is NOT a stream: it is a real `libcc2rs` struct with a `Drop`, and it BORROWS
// rules/raw_ostream's stream model for its `os()` return type.  Keeping it apart
// means rules/raw_ostream's 18 live insertion keys are not disturbed.
//
// ⛔⛔ THIS TYPE WAS A STANDING REFUSAL IN THIS TREE (see rules/error_code
// src.cpp:19-35) AND THE REFUSAL WAS RIGHT ABOUT THE MODEL IT WAS REFUSING.
// The refusal was against a VALUE model -- an opaque unit, or a plain file
// handle -- and the reason was measured at the call sites, not assumed:
//
//     dxp-driver.cpp:79-84
//         if (runPipeline(output->os(), config) != 0) {
//           llvm::errs() << "[DXP] Error: Pipeline execution failed\n";
//           return 1;                    // <-- returns WITHOUT keep()
//         }
//         output->keep();
//
//     DxpOptMain.cpp:283-289
//         if (failed(::dxp::DxpOptMain(outputFile->os(), std::move(inputFile),
//                                      registry, config, sbfContext))) {
//           return failure();            // <-- returns WITHOUT keep()
//         }
//         outputFile->keep();
//
// In BOTH, the FAILURE path returns without calling keep(), so `~ToolOutputFile`
// unlinking the partially written file is LOAD-BEARING OBSERVABLE BEHAVIOUR.  A
// value model would leave a TRUNCATED output file on disk after a failed
// pipeline, silently, which is strictly worse than the loud abort.
//
// ⭐ WHAT CHANGED IS THAT THE SAME MEASUREMENT NAMES AN HONEST MODEL: a real
// `libcc2rs` struct whose `Drop` unlinks the file unless `keep()` ran.  That is
// precisely what Rust's `Drop` is for, and it is the shape
// `libcc2rs::InFlightDiagnostic` (rules/mlir t70) already uses for
// `~InFlightDiagnostic`'s observable report.  The semantics were read out of
// `llvm/Support/ToolOutputFile.h` in this toolchain and are quoted in full in
// `libcc2rs/src/tool_output_file.rs`; the short version is
//     CleanupInstaller { std::string Filename; bool Keep; };  ~CleanupInstaller();
//     class ToolOutputFile { CleanupInstaller Installer;
//                            std::optional<raw_fd_ostream> OSHolder;
//                            raw_fd_ostream *OS;
//       raw_fd_ostream &os() { return *OS; }
//       void keep() { Installer.Keep = true; }
//       StringRef getFilename();  const std::string &outputFilename(); };
// with the header's own comment that `CleanupInstaller` is declared FIRST so the
// stream is destructed (flushed, closed) BEFORE the file is removed -- which the
// Rust `Drop` reproduces by flushing first, unconditionally.
//
// ⭐ THE BEHAVIOUR IS EXECUTED, NOT ARGUED.  `libcc2rs` carries ten tests for it
// (`tool_output_file::tests`), including:
//   dropping_without_keep_removes_the_file      -- write, drop, assert GONE
//   keep_makes_the_file_survive_...             -- write, keep, drop, assert BYTES
//   an_existing_file_is_truncated_and_still_unlinked_without_keep
//   open_failure_is_reported_and_creates_nothing
//   an_outstanding_os_pointer_does_not_suppress_the_unlink
//
// ⭐ HOW `std::unique_ptr<T>` COMPOSES WITH THE `Drop`, checked against
// rules/unique_ptr rather than assumed, because a collapsing unique_ptr model is
// exactly where a destructor with observable effects would be lost:
//     unsafe   : `std::unique_ptr<T>` -> `Option<Box<T>>`
//     refcount : `std::unique_ptr<T>` -> `Option<Value<T>>` = `Option<Rc<RefCell<T>>>`
// BOTH RUN `T::drop`.  `Option<Box<T>>` drops its `Box`, which drops the `T`;
// `Option<Rc<RefCell<T>>>` drops the last strong handle, which drops the
// `RefCell<T>`, which drops the `T`.  `rules/unique_ptr` is not a collapsing
// model -- it is an owning one in both cases -- so the `Drop` survives the
// wrapper.  AND `os()`'s result does not extend the lifetime: in the refcount
// model a `Ptr<T>` is a WEAK handle (`Ptr::upgrade` exists precisely because it
// is), so an outstanding stream pointer cannot suppress the unlink.  That is the
// tenth test above.  ⚠️ The one place the wrapper DOES differ from C++ is
// `reset()`/`operator=` (rules/unique_ptr f5/f14): C++ destroys the old pointee
// there too, and so does `Option::replace`, so the unlink still happens -- but no
// site in this corpus resets one of these, so that path is not claimed as
// measured.
//
// WHY THE DECLARATIONS BELOW ARE LOCAL AND NOT #include <llvm/Support/...>:
// verbatim the reason rules/raw_ostream gives -- cpp-rule-preprocessor compiles
// this file with a fixed flag set and the only way to reach LLVM's headers would
// be absolute -I paths into a tree not everyone has.  A rule matches on a
// SIGNATURE STRING, so a restatement matches iff it agrees with LLVM exactly;
// the declarations here were transcribed from
// `llvm/Support/ToolOutputFile.h` and `mlir/Support/FileUtilities.h` in
// toolchain/llvm/LLVM-22.1.3-Linux-X64/include.
//
// ⛔ NOT COVERED, AND LEFT OUT SO THEY FAIL LOUDLY:
//   * `ToolOutputFile(StringRef, std::error_code &, sys::fs::OpenFlags)` -- the
//     REAL constructor.  Its third parameter is `sys::fs::OpenFlags`, a bitmask
//     enum with no type rule anywhere in this tree, and its second is the
//     `std::error_code &` out-parameter `rules/error_code` documents at length
//     as unwritable by any rule.  No site in the corpus calls it directly (both
//     go through `mlir::openOutputFile`), so keying it would be a dead key that
//     ALSO claims an error report it cannot make.  Any TU that does call it
//     directly still aborts loudly.
//   * `ToolOutputFile(StringRef, int FD)` -- adopts an existing descriptor.  Not
//     reached in the corpus.
//   * `getFilename()` / `outputFilename()` -- not reached at either call site;
//     `libcc2rs::ToolOutputFile::filename()` exists for them, but an unasked key
//     is dead-key noise, so they are deliberately not spelled here.

#include <memory>
#include <string>

namespace llvm {

// Restated from llvm/ADT/StringRef.h, byte-for-byte as rules/raw_ostream
// restates it.  ⛔ NO `using tN = llvm::StringRef;` HERE: rules/stringref owns
// that type rule (its t1), and a second type rule for one spelling in two
// modules is load-order-dependent.  This declaration exists only so the
// signatures below can name the type.
class StringRef {
  const char *Data;
  unsigned long Length;
};

// raw_fd_ostream, restated EMPTY and WITHOUT its base.  rules/raw_ostream owns
// this type (its t4/t5) and models it as a `std::fs::File`; only the name is
// needed here, as `os()`'s return type.  Same convention rules/mlir uses when it
// names `llvm::raw_ostream` without owning it.
class raw_fd_ostream;

// llvm/Support/ToolOutputFile.h.  Only the two members the corpus reaches are
// declared; see the NOT COVERED list above for the rest and why.
class ToolOutputFile {
public:
  raw_fd_ostream &os();
  void keep();
};

} // namespace llvm

namespace mlir {

// mlir/Support/FileUtilities.h:41-43.
//
// ⛔⛔ "WE DO NOT PORT MLIR" IS NOT VIOLATED HERE, AND HERE IS THE BOUNDARY
// ARGUMENT RATHER THAN AN ASSERTION.  The settled refusal is about MLIR's IR
// INFRASTRUCTURE -- Operation/Type/Attribute/Value and the dialects -- whose
// models come from dataflowir-gen's .td parser.  `mlir::openOutputFile` is not
// part of that: it is a THREE-STATEMENT FREE FUNCTION in MLIR's Support layer
// whose entire body is
//     std::error_code error;
//     auto result = std::make_unique<llvm::ToolOutputFile>(outputFilename, error,
//                                                          llvm::sys::fs::OF_None);
//     if (error) { if (errorMessage) *errorMessage = "cannot open output file "
//                                    + outputFilename.str() + ": " + error.message();
//                  return nullptr; }
//     return result;
// i.e. open a file, or report why not.  It is keyed HERE rather than in
// rules/mlir because it is the ONLY way either call site constructs a
// ToolOutputFile, and because keying it here touches no type rule rules/mlir
// owns: its return type belongs to rules/unique_ptr + this module, and its two
// parameter types to rules/stringref and rules/string.  ⚠️ If a later slot
// decides Support-layer free functions belong in rules/mlir after all, this one
// block moves; nothing else here does.
//
// ⭐ AND IT IS MODELLABLE HONESTLY, WHICH IS THE WHOLE REASON IT IS HERE.  The
// failure branch is the one the drivers test (`if (!output) { errs() << msg;
// return 1; }`), and the target body ACTUALLY ATTEMPTS THE OPEN against a real
// filesystem, so `nullptr` means the open really failed, for the real reason.
// That is the branch `rules/error_code` could not reach and correctly refused to
// fake: there, no rule could ever write a nonzero error_code, so every site took
// the success branch unconditionally.  Here the branch is observed.
//
// ⚠️ THE ERROR *TEXT* IS NOT BYTE-IDENTICAL AND THAT IS STATED, NOT HIDDEN.
// C++ appends `std::error_code::message()`, the libc/locale string
// ("No such file or directory"); the model appends `std::io::Error`'s Display
// ("No such file or directory (os error 2)").  Prefix and filename match; the
// reason differs by a parenthesised errno.  It is diagnostic text on a failure
// path, printed to stderr, and the BRANCH -- the thing a program can depend on --
// is exact.  See `open_error_message` in libcc2rs/src/tool_output_file.rs.
//
// ⛔ `openInputFile` IS DELIBERATELY NOT KEYED.  It returns
// `std::unique_ptr<llvm::MemoryBuffer>`, and `llvm::MemoryBuffer` has NO model
// anywhere in this tree (grepped: rules/mlir:1090 and rules/support:188/217 only
// mention it in prose).  A buffer whose contents are the input program is not
// something this module can invent, so DxpOptMain.cpp's `openInputFile` call
// must keep aborting loudly.  That is a DIFFERENT row with a different owner.
std::unique_ptr<llvm::ToolOutputFile> openOutputFile(llvm::StringRef,
                                                    std::string *);

} // namespace mlir

// ⚠️ INDICES t800 / f800-f802, not t1/f1-f3, DELIBERATELY.  This is a new module,
// so nothing can collide today -- but several slots are live in the rule tree and
// the low indices are what a concurrent slot folding this into an existing module
// would reach for.  Indices are per-module and need not be dense.

// t800 -- the type.  Model: `libcc2rs::ToolOutputFile`.
using t800 = llvm::ToolOutputFile;

// f800 -- `raw_fd_ostream &os()`.  The stream the object owns; ToolOutputFile.h
// is `{ return *OS; }`, so this is an accessor with no side effect.
llvm::raw_fd_ostream &f800(llvm::ToolOutputFile &o) { return o.os(); }

// f801 -- `void keep()`.  ToolOutputFile.h is `{ Installer.Keep = true; }`.
// ⛔ THIS IS THE KEY THAT MUST NOT BE A NO-OP.  A no-op keep() would make the
// SUCCESS path delete the output file it just wrote -- the mirror image of the
// failure the refusal was about, and just as silent.  The target body sets the
// flag the `Drop` reads, and `keep_makes_the_file_survive_with_the_expected_bytes`
// executes it.
void f801(llvm::ToolOutputFile &o) { return o.keep(); }

// f802 -- `mlir::openOutputFile(StringRef, std::string *)`.  See the boundary and
// fidelity notes at its declaration above.  Called UNQUALIFIED inside
// `namespace mlir`... no: it is called QUALIFIED here because it is declared in a
// namespace this file is not inside, which is the spelling the corpus uses too
// (`openOutputFile(...)` at both sites resolves through ADL-free ordinary lookup
// after `using namespace mlir;`, and the mapper keys the function's QUALIFIED
// name, not the written specifier, for a non-operator free function).
std::unique_ptr<llvm::ToolOutputFile> f802(llvm::StringRef name,
                                           std::string *errorMessage) {
  return mlir::openOutputFile(name, errorMessage);
}
