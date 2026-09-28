// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Overlay on tgt_unsafe.rs.  t800 IS SPELLED HERE even though its model
// (`libcc2rs::ToolOutputFile`) is identical in both models: rules/mlir's t70 --
// the `libcc2rs::InFlightDiagnostic` type key this row is shaped after -- is
// present in BOTH of its tgt files, and following that costs three lines, whereas
// relying on rules/raw_ostream's "value types are not repeated" convention for a
// type that is NOT a bare std value is the kind of guess that reads as a dead key.
//
// REFCOUNT MODEL SPELLINGS that differ from the unsafe ones:
//   llvm::ToolOutputFile &  -> Ptr<libcc2rs::ToolOutputFile>
//   llvm::raw_fd_ostream &  -> Ptr<std::fs::File>       (rules/raw_ostream t5)
//   llvm::StringRef         -> Vec<u8>                  (rules/stringref t1)
//   std::string *           -> Ptr<Vec<u8>>             (rules/string t1)
//   std::unique_ptr<T>      -> Option<Value<T>>         (rules/unique_ptr t1)

use libcc2rs::*;

// See tgt_unsafe.rs t800: no honest default exists, so this aborts loudly rather
// than inventing a path whose `Drop` would unlink something.
fn t800() -> libcc2rs::ToolOutputFile {
    libcc2rs::ToolOutputFile::create("").unwrap()
}

// f800 -- `os()`.  `Ptr<std::fs::File>` is rules/raw_ostream's `raw_ostream &`, so
// every insertion key applies unchanged.
//
// ⚠️ `with_ref`, NOT `with`: `Ptr::with` is bounded on `T: ByteRepr` and
// `libcc2rs::ToolOutputFile` is not a byte-representable value (it owns an open
// descriptor).  `with_ref` has no such bound -- that is what it is for.
fn f800(a0: Ptr<libcc2rs::ToolOutputFile>) -> Ptr<std::fs::File> {
    a0.with_ref(|__r: &libcc2rs::ToolOutputFile| __r.os_ptr())
}

// f801 -- `keep()`.  Sets the flag the `Drop` reads; NOT a no-op.
fn f801(a0: Ptr<libcc2rs::ToolOutputFile>) {
    a0.with_ref(|__r: &libcc2rs::ToolOutputFile| __r.keep());
}

// f802 -- `mlir::openOutputFile`.  `None` is the `nullptr` both drivers branch on.
// The returned `Option<Value<..>>` OWNS the object, so the `Drop`-unlink runs when
// the caller's local dies -- the composition argument is in src.cpp.
fn f802(a0: Vec<u8>, a1: Ptr<Vec<u8>>) -> Option<Value<libcc2rs::ToolOutputFile>> {
    libcc2rs::tool_output_file_open_refcount(a0, a1)
}
