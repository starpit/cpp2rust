// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model, the header transcription, the two measured call
// sites and the unique_ptr composition argument.
//
// UNSAFE MODEL SPELLINGS used here:
//   llvm::ToolOutputFile          -> libcc2rs::ToolOutputFile
//   llvm::ToolOutputFile &        -> *mut libcc2rs::ToolOutputFile
//   llvm::raw_fd_ostream &        -> *mut std::fs::File        (rules/raw_ostream t5)
//   llvm::StringRef               -> Vec<libc::c_char>          (rules/stringref t1)
//   std::string *                 -> *mut Vec<libc::c_char>     (rules/string t1)
//   std::unique_ptr<T>            -> Option<Box<T>>             (rules/unique_ptr t1)

// t800 -- the type's DEFAULT VALUE, i.e. the value a declaration the converter
// cannot initialise gets.
//
// ⚠️ IT PANICS, ON PURPOSE, AND THAT IS rules/raw_ostream's OWN CONVENTION FOR
// THIS POSITION (`t1`/`t4`/`t540` are all `std::fs::File::open("").unwrap()`).
// There is no honest default for this type: a real `ToolOutputFile` names a real
// path whose file its `Drop` will unlink, so ANY invented default either creates
// a file nobody asked for or -- worse -- names a path something else owns and
// deletes it.  An empty filename cannot be opened, so `create("")` fails and the
// `unwrap` aborts LOUDLY at the point of use.  Correct code never evaluates a
// type rule's body.
fn t800() -> libcc2rs::ToolOutputFile {
    libcc2rs::ToolOutputFile::create("").unwrap()
}

// f800 -- `os()`.  Hands back the port's `raw_fd_ostream &`, a raw pointer to the
// `std::fs::File` this object owns, so every `rules/raw_ostream` insertion key
// (f5-f18) applies to it unchanged.
//
// ⛔⛔ THE RECEIVER IS `&mut libcc2rs::ToolOutputFile`, NOT `*mut ...`, AND THE
// BODY IS A BARE METHOD CALL WITH NO EXPLICIT BORROW AND NO TYPE ANNOTATION.
// THREE SHAPES WERE TRIED AND THE EMITTED TEXT READ BACK OUT OF
// dxp-driver.cpp.rs EACH TIME.  All three give rc=0 -- `rustfmt` is the only Rust
// parser in this pipeline -- so only the readback discriminates:
//
//  (1) `a0: *mut _` + `let __r: &_ = &*a0; __r.os_unsafe()`
//      The receiver is a `unique_ptr`, so the converter substituted
//          &mut output.as_deref_mut().unwrap() as *mut libcc2rs::ToolOutputFile
//      `as_deref_mut().unwrap()` is ALREADY `&mut T`, so that is `&mut &mut T`
//      cast to `*mut T`: `error[E0605]: non-primitive cast`.
//
//  (2) `a0: &mut _` + `let __r: &_ = a0; __r.os_unsafe()`
//      A `&mut` parameter makes the converter substitute a DEREFERENCED PLACE,
//          (*output.as_deref_mut().unwrap())
//      which has type `ToolOutputFile`, not `&ToolOutputFile`, so MY OWN
//      `: &libcc2rs::ToolOutputFile` annotation is then `error[E0308]`.
//      ⭐ The lesson: with a `&mut T` parameter the placeholder is a PLACE, and
//      any reference type annotation the body adds fights the substitution.
//
//  (3) `a0: &mut _` + `a0.os_unsafe()`   <- what is written below.
//      Emits `(*output.as_deref_mut().unwrap()).os_unsafe()`: a method call on a
//      place, which auto-refs correctly.  ⭐ AND `dangerous_implicit_autorefs`
//      (deny-by-default in this rustc) DOES NOT FIRE, because it is about
//      autoref through a RAW POINTER deref; `*(&mut T)` is not one.  That lint is
//      exactly why shape (1) looked attractive, and shape (1) is the broken one.
unsafe fn f800(a0: &mut libcc2rs::ToolOutputFile) -> *mut std::fs::File {
    a0.os_unsafe()
}

// f801 -- `keep()`.  Sets the flag the `Drop` reads; NOT a no-op.  Same receiver
// shape and same reasons as f800 -- see its (1)/(2)/(3) note.
unsafe fn f801(a0: &mut libcc2rs::ToolOutputFile) {
    a0.keep();
}

// f802 -- `mlir::openOutputFile`.  `None` is the `nullptr` both drivers branch on
// and means the open REALLY failed; on failure the message is written through the
// `std::string *` out-parameter, as MLIR does.
//
// The work is in `libcc2rs` rather than inline because an inline body would have
// to mention `a0` three times (bytes, len, and the name), and a rule body is one
// inlined expression in which every `aN` RE-EVALUATES its argument.
unsafe fn f802(
    a0: Vec<libc::c_char>,
    a1: *mut Vec<libc::c_char>,
) -> Option<Box<libcc2rs::ToolOutputFile>> {
    unsafe { libcc2rs::tool_output_file_open_unsafe(a0, a1) }
}
