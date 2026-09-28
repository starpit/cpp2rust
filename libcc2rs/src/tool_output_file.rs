// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

//! `llvm::ToolOutputFile` -- AN OUTPUT FILE THAT IS UNLINKED ON `Drop` UNLESS
//! `keep()` RAN.
//!
//! WHY THIS IS NOT A PLAIN FILE HANDLE.  Read out of
//! `llvm/Support/ToolOutputFile.h` (LLVM-22.1.3, this toolchain), not
//! paraphrased:
//!
//! ```text
//! class CleanupInstaller {
//! public:
//!   std::string Filename;   /// The name of the file.
//!   bool Keep;              /// The flag which indicates whether we should not delete the file.
//!   StringRef getFilename() { return Filename; }
//!   explicit CleanupInstaller(StringRef Filename);
//!   ~CleanupInstaller();
//! };
//!
//! /// - The file is automatically deleted if the process is killed.
//! /// - The file is automatically deleted when the ToolOutputFile
//! ///   object is destroyed unless the client calls keep().
//! class ToolOutputFile {
//!   /// This class is declared before the raw_fd_ostream so that it is constructed
//!   /// before the raw_fd_ostream is constructed and destructed after the
//!   /// raw_fd_ostream is destructed.
//!   CleanupInstaller Installer;
//!   std::optional<raw_fd_ostream> OSHolder;
//!   raw_fd_ostream *OS;
//! public:
//!   ToolOutputFile(StringRef Filename, std::error_code &EC, sys::fs::OpenFlags Flags);
//!   ToolOutputFile(StringRef Filename, int FD);
//!   raw_fd_ostream &os() { return *OS; }
//!   StringRef getFilename() { return Installer.getFilename(); }
//!   void keep() { Installer.Keep = true; }
//!   const std::string &outputFilename() { return Installer.Filename; }
//! };
//! ```
//!
//! THE DESTRUCTOR IS LOAD-BEARING AT BOTH PORT-GOAL CALL SITES, measured at the
//! sites rather than assumed:
//!
//! ```text
//! dxp/src/Driver/dxp-driver.cpp:79-84
//!     if (runPipeline(output->os(), config) != 0) {
//!       llvm::errs() << "[DXP] Error: Pipeline execution failed\n";
//!       return 1;                       // <-- NO keep(): file must be UNLINKED
//!     }
//!     output->keep();
//!
//! dxp/tools/DxpOptMain.cpp:283-289
//!     if (failed(::dxp::DxpOptMain(outputFile->os(), ...))) {
//!       return failure();               // <-- NO keep(): file must be UNLINKED
//!     }
//!     outputFile->keep();
//! ```
//!
//! So a value model over a bare `std::fs::File` would leave a TRUNCATED,
//! PARTIALLY WRITTEN output file on disk after a failed pipeline, and do it
//! silently -- strictly worse than the loud abort this type used to produce.
//! Rust's `Drop` expresses the C++ destructor directly, which is why this type
//! exists as a real struct.  Same criterion, same shape as [`crate::InFlightDiagnostic`].
//!
//! DESTRUCTION ORDER IS PART OF THE SEMANTICS, and the header says so in a
//! comment: `CleanupInstaller` is declared FIRST precisely so that it is
//! destroyed LAST, i.e. the `raw_fd_ostream` flushes and closes BEFORE the file
//! is removed.  [`Drop::drop`] below therefore flushes first, unconditionally,
//! and only then decides whether to unlink.
//!
//! WHAT IS *NOT* MODELLED, named so nobody reads this as full fidelity:
//!   * "The file is automatically deleted if the process is killed" -- that is
//!     LLVM's `sys::RemoveFileOnSignal` signal handler.  A Rust `Drop` does not
//!     run on `SIGKILL` either, and installing a process-wide signal handler
//!     from a translated rule body is out of scope.  The `Drop` half -- the half
//!     both call sites depend on -- is what is modelled.
//!   * `sys::fs::OpenFlags`.  The only flag set reachable from the corpus is
//!     MLIR's `openOutputFile`, which passes `OF_None`: create/truncate for
//!     writing in text mode.  [`ToolOutputFile::create`] implements exactly
//!     that; no other flag combination is expressible, so no other one can be
//!     silently mistranslated.
//!   * the `(StringRef, int FD)` constructor, which adopts a descriptor and is
//!     not reached anywhere in the corpus.

use crate::{AsPointer, Ptr, Value};
use std::cell::RefCell;
use std::rc::Rc;

/// `llvm::ToolOutputFile`.
///
/// The stream lives in a `Value<std::fs::File>` (`Rc<RefCell<File>>`) rather
/// than in a bare field because `os()` has to hand out the port's spelling of
/// `raw_fd_ostream &` in BOTH models: `Ptr<std::fs::File>` in the refcount model
/// (via [`AsPointer`]) and `*mut std::fs::File` in the unsafe model (via
/// `RefCell::as_ptr`).  One representation serves both, and a `Ptr` is a WEAK
/// handle, so a stream pointer handed out by `os()` does not keep the file alive
/// past this object's `Drop` -- which is correct: in C++ the `raw_fd_ostream`
/// dies with the `ToolOutputFile`.
///
/// ```
/// let dir = std::env::temp_dir().join(format!("cc2_toolout_doc_{}", std::process::id()));
/// std::fs::create_dir_all(&dir).unwrap();
/// let path = dir.join("kept.txt");
/// let p = path.to_str().unwrap().to_string();
///
/// // Dropped WITHOUT keep(): the file is removed, as ~ToolOutputFile does.
/// {
///     let f = libcc2rs::ToolOutputFile::create(&p).unwrap();
///     f.write_all(b"partial output");
/// }
/// assert!(!path.exists());
///
/// // keep() makes it survive, with the bytes that were written.
/// {
///     let f = libcc2rs::ToolOutputFile::create(&p).unwrap();
///     f.write_all(b"final output");
///     f.keep();
/// }
/// assert_eq!(std::fs::read(&path).unwrap(), b"final output");
/// std::fs::remove_dir_all(&dir).unwrap();
/// ```
pub struct ToolOutputFile {
    /// `CleanupInstaller::Filename`.
    filename: String,
    /// `CleanupInstaller::Keep`.  `Cell`-free: `keep()` is reached through a
    /// `*mut`/`Ptr` receiver in the translated code, which already gives
    /// mutable access, so a plain `bool` behind `&mut self` is enough.
    keep: RefCell<bool>,
    /// `OSHolder` / `OS`.  See the struct doc for why it is an `Rc<RefCell<_>>`.
    stream: Value<std::fs::File>,
}

impl ToolOutputFile {
    /// `ToolOutputFile(StringRef Filename, std::error_code &EC, OF_None)` --
    /// create/truncate `filename` for writing.
    ///
    /// Returns `Err` with the real `std::io::Error` when the open fails, which
    /// is the C++ constructor writing a nonzero code into its `EC`
    /// out-parameter.  ⭐ THIS IS THE HONEST HALF OF THE ROW
    /// `rules/error_code` could not reach: there, no rule could ever WRITE a
    /// nonzero `error_code`, so every site took the success branch.  Here the
    /// open is really attempted against a real filesystem, so the failure
    /// branch is reachable for the real reason (bad path, unwritable
    /// directory), and callers that test it get the right answer.
    pub fn create(filename: &str) -> std::io::Result<Self> {
        let file = std::fs::File::create(filename)?;
        Ok(ToolOutputFile {
            filename: filename.to_string(),
            keep: RefCell::new(false),
            stream: Rc::new(RefCell::new(file)),
        })
    }

    /// `void keep()` -- `Installer.Keep = true`.  After this, `Drop` leaves the
    /// file on disk.
    ///
    /// Takes `&self` rather than `&mut self` because the translated call sites
    /// reach it through `output->keep()` on a `unique_ptr`, i.e. through a
    /// handle the converter spells as a shared borrow; the flag is therefore in
    /// a `RefCell`.
    pub fn keep(&self) {
        *self.keep.borrow_mut() = true;
    }

    /// True once [`Self::keep`] has run.  Not C++ API -- `CleanupInstaller::Keep`
    /// is public there, but nothing in the corpus reads it; this exists for the
    /// tests.
    pub fn is_kept(&self) -> bool {
        *self.keep.borrow()
    }

    /// `StringRef getFilename()` / `const std::string &outputFilename()`.
    pub fn filename(&self) -> &str {
        &self.filename
    }

    /// `raw_fd_ostream &os()` in the REFCOUNT model: `rules/raw_ostream` maps
    /// `raw_ostream &` to `Ptr<std::fs::File>`, and this is that pointer.
    pub fn os_ptr(&self) -> Ptr<std::fs::File> {
        self.stream.as_pointer()
    }

    /// `raw_fd_ostream &os()` in the UNSAFE model: `rules/raw_ostream` maps
    /// `raw_ostream &` to `*mut std::fs::File`.
    ///
    /// # Safety
    ///
    /// The returned pointer is valid only while this `ToolOutputFile` is alive,
    /// which is exactly the lifetime C++ gives `os()`'s referent.
    pub fn os_unsafe(&self) -> *mut std::fs::File {
        self.stream.as_ref().as_ptr()
    }

    /// Write bytes to the file.  Not a C++ member of this class -- writes go
    /// through `os()` and `rules/raw_ostream`'s `operator<<` -- but the tests
    /// and the doc example need a way to put bytes in without going through a
    /// raw pointer, and a faithful `Drop` test has to have something to delete.
    pub fn write_all(&self, buf: &[u8]) {
        let _ = ::std::io::Write::write_all(&mut *self.stream.borrow_mut(), buf);
    }
}

impl Drop for ToolOutputFile {
    /// `~ToolOutputFile` -> `~CleanupInstaller`.
    ///
    /// Flush FIRST and unconditionally: the header's own comment says
    /// `CleanupInstaller` is declared before the stream so that the stream is
    /// destructed (flushed, closed) before the cleanup runs.  Then remove the
    /// file unless `keep()` was called.
    ///
    /// The `remove_file` result is discarded for the same reason
    /// `~CleanupInstaller` discards `sys::fs::remove`'s: a destructor cannot
    /// report, and a file that is already gone is the outcome that was wanted.
    fn drop(&mut self) {
        let _ = ::std::io::Write::flush(&mut *self.stream.borrow_mut());
        if !*self.keep.borrow() {
            let _ = std::fs::remove_file(&self.filename);
        }
    }
}

/// The bytes of one of the port's NUL-terminated string models as a `String`.
/// `llvm::StringRef` (rules/stringref t1), `llvm::StringLiteral` (t2) and
/// `std::string` (rules/string t1) all carry a trailing NUL that is not part of
/// the string, so it is dropped here -- the same asymmetry
/// `rules/raw_ostream` f6/f7 rely on.
fn model_string(bytes: &[u8]) -> String {
    let end = bytes.iter().position(|&c| c == 0).unwrap_or(bytes.len());
    String::from_utf8_lossy(&bytes[..end]).into_owned()
}

/// MLIR's `openOutputFile` error text, rebuilt.
///
/// ⚠️ THE TEXT IS NOT BYTE-IDENTICAL TO C++ AND THAT IS DELIBERATE, stated
/// rather than hidden.  `mlir/Support/FileUtilities.cpp` writes
/// `"cannot open output file " + filename + ": " + error.message()`, and
/// `std::error_code::message()` is the libc/locale string
/// (`"No such file or directory"`), which `rules/error_code` refuses to model
/// for exactly that reason.  Rust's `std::io::Error` Display gives
/// `"No such file or directory (os error 2)"`.  So the PREFIX and the filename
/// match and the reason differs by a parenthesised errno.
///
/// Why that is acceptable here while `error_code::message()` was refused there:
/// this text is only ever reached on the failure branch, is written to a
/// diagnostic the caller prints to stderr, and -- critically -- the BRANCH
/// itself is modelled exactly (a real open is attempted, `None` really means
/// the open really failed).  The refusal in `rules/error_code` was about a model
/// that could not observe the failure AT ALL and so always took the success
/// branch; this one observes it for the real reason.
fn open_error_message(filename: &str, e: &std::io::Error) -> String {
    format!("cannot open output file {}: {}", filename, e)
}

/// `mlir::openOutputFile(StringRef, std::string *)` in the UNSAFE model.
///
/// Exists as a function rather than inline in the rule body because the body
/// would otherwise have to mention `a0` three times, and a rule body is one
/// inlined expression in which every `aN` re-evaluates its argument (see
/// [`crate::InFlightDiagnostic::shl_c_chars`] for the same reason).
///
/// # Safety
///
/// `error_message` must be null or point to a live `Vec<c_char>` std::string
/// model, which is what a `std::string *` argument is in this model.
pub unsafe fn tool_output_file_open_unsafe(
    filename: Vec<::libc::c_char>,
    error_message: *mut Vec<::libc::c_char>,
) -> Option<Box<ToolOutputFile>> {
    // ⚠️ TAKEN BY VALUE, NOT AS `&[c_char]`, ON PURPOSE.  A rule body that had to
    // write `&a0` would record a BORROW placeholder and the converter adds its own
    // (rules/raw_ostream f560 measured all three spellings); a by-value parameter
    // needs no borrow in the body at all.
    let bytes: &[u8] =
        unsafe { ::std::slice::from_raw_parts(filename.as_ptr() as *const u8, filename.len()) };
    let name = model_string(bytes);
    match ToolOutputFile::create(&name) {
        Ok(f) => Some(Box::new(f)),
        Err(e) => {
            if !error_message.is_null() {
                let mut v: Vec<::libc::c_char> = open_error_message(&name, &e)
                    .into_bytes()
                    .into_iter()
                    .map(|b| b as ::libc::c_char)
                    .collect();
                v.push(0);
                unsafe { *error_message = v };
            }
            None
        }
    }
}

/// `mlir::openOutputFile(StringRef, std::string *)` in the REFCOUNT model.
/// Same behaviour as [`tool_output_file_open_unsafe`]; the string models are
/// `Vec<u8>` and `Ptr<Vec<u8>>` here.
pub fn tool_output_file_open_refcount(
    filename: Vec<u8>,
    error_message: Ptr<Vec<u8>>,
) -> Option<Value<ToolOutputFile>> {
    let name = model_string(&filename);
    match ToolOutputFile::create(&name) {
        Ok(f) => Some(Rc::new(RefCell::new(f))),
        Err(e) => {
            if !error_message.is_null() {
                let mut v: Vec<u8> = open_error_message(&name, &e).into_bytes();
                v.push(0);
                error_message.with_mut(|s: &mut Vec<u8>| *s = v);
            }
            None
        }
    }
}

#[cfg(test)]
mod tests {
    use super::ToolOutputFile;

    /// A private directory per test, under the process temp dir.  ⚠️ These are
    /// scratch paths for an in-process filesystem assertion, not artifacts: the
    /// project rule "nothing of value on /tmp" is about work products, and each
    /// test removes its own directory.
    fn scratch(tag: &str) -> std::path::PathBuf {
        let d = std::env::temp_dir().join(format!(
            "cc2_toolout_{}_{}_{:?}",
            std::process::id(),
            tag,
            std::thread::current().id()
        ));
        let _ = std::fs::remove_dir_all(&d);
        std::fs::create_dir_all(&d).unwrap();
        d
    }

    #[test]
    fn writes_go_to_the_named_path() {
        let d = scratch("writes");
        let p = d.join("out.txt");
        let ps = p.to_str().unwrap().to_string();
        let f = ToolOutputFile::create(&ps).unwrap();
        assert_eq!(f.filename(), ps);
        f.write_all(b"hello ");
        f.write_all(b"world");
        f.keep();
        drop(f);
        assert_eq!(std::fs::read(&p).unwrap(), b"hello world");
        std::fs::remove_dir_all(&d).unwrap();
    }

    /// ⭐ THE ROW'S WHOLE POINT: the failure path at dxp-driver.cpp:79-84 and
    /// DxpOptMain.cpp:283-289 returns WITHOUT `keep()`, so the partially
    /// written file must not survive.
    #[test]
    fn dropping_without_keep_removes_the_file() {
        let d = scratch("nokeep");
        let p = d.join("out.txt");
        let ps = p.to_str().unwrap().to_string();
        {
            let f = ToolOutputFile::create(&ps).unwrap();
            f.write_all(b"partially written output");
            assert!(p.exists(), "the file must exist while the object is alive");
            assert!(!f.is_kept());
        }
        assert!(
            !p.exists(),
            "~ToolOutputFile must unlink the file when keep() was not called"
        );
        std::fs::remove_dir_all(&d).unwrap();
    }

    #[test]
    fn keep_makes_the_file_survive_with_the_expected_bytes() {
        let d = scratch("keep");
        let p = d.join("out.txt");
        let ps = p.to_str().unwrap().to_string();
        {
            let f = ToolOutputFile::create(&ps).unwrap();
            f.write_all(b"final output");
            f.keep();
            assert!(f.is_kept());
        }
        assert!(p.exists());
        assert_eq!(std::fs::read(&p).unwrap(), b"final output");
        std::fs::remove_dir_all(&d).unwrap();
    }

    /// `OF_None` is create-or-TRUNCATE, so an already-existing file is
    /// overwritten, not appended to -- and it is still removed on a keep-less
    /// drop, which is the case where the C++ deletes a file it did not create.
    #[test]
    fn an_existing_file_is_truncated_and_still_unlinked_without_keep() {
        let d = scratch("exists");
        let p = d.join("out.txt");
        let ps = p.to_str().unwrap().to_string();
        std::fs::write(&p, b"PRE-EXISTING CONTENT").unwrap();
        {
            let f = ToolOutputFile::create(&ps).unwrap();
            f.write_all(b"ab");
            // Truncated on open: only the new bytes, not the old tail.
            f.keep();
        }
        assert_eq!(std::fs::read(&p).unwrap(), b"ab");
        {
            let f = ToolOutputFile::create(&ps).unwrap();
            f.write_all(b"xy");
        }
        assert!(!p.exists(), "a keep-less drop removes even a file it did not create");
        std::fs::remove_dir_all(&d).unwrap();
    }

    /// The error-on-open path: `std::fs::File::create` into a directory that
    /// does not exist fails, which is the C++ ctor writing a nonzero
    /// `std::error_code`, which is what makes `mlir::openOutputFile` return
    /// nullptr.  ⭐ And nothing is left behind: no `Drop` runs, because no
    /// object was constructed.
    #[test]
    fn open_failure_is_reported_and_creates_nothing() {
        let d = scratch("err");
        let p = d.join("no_such_dir").join("out.txt");
        let ps = p.to_str().unwrap().to_string();
        // `unwrap_err` would need `ToolOutputFile: Debug`, and this type must NOT
        // derive Debug: a derived Debug on a struct holding an open file handle
        // is noise, and the C++ class has no such rendering.
        let e = match ToolOutputFile::create(&ps) {
            Ok(_) => panic!("opening into a nonexistent directory must fail"),
            Err(e) => e,
        };
        assert_eq!(e.kind(), std::io::ErrorKind::NotFound);
        assert!(!p.exists());
        std::fs::remove_dir_all(&d).unwrap();
    }

    /// `os()` in both models points at the same descriptor the object owns, so
    /// bytes written through it land in the file and are removed with it.
    #[test]
    fn os_writes_reach_the_file_in_both_models() {
        let d = scratch("os");
        let p = d.join("out.txt");
        let ps = p.to_str().unwrap().to_string();
        {
            let f = ToolOutputFile::create(&ps).unwrap();
            // Unsafe-model spelling: *mut std::fs::File.
            let raw = f.os_unsafe();
            unsafe {
                let _ = ::std::io::Write::write_all(&mut *raw, b"via-raw ");
            }
            // Refcount-model spelling: Ptr<std::fs::File>.
            let _ = f.os_ptr().write_all(b"via-ptr");
            f.keep();
        }
        assert_eq!(std::fs::read(&p).unwrap(), b"via-raw via-ptr");
        std::fs::remove_dir_all(&d).unwrap();
    }

    /// A `Ptr` from `os()` is a WEAK handle: it does not keep the file object
    /// alive, so the `Drop`-unlink still happens on schedule.
    #[test]
    fn an_outstanding_os_pointer_does_not_suppress_the_unlink() {
        let d = scratch("weak");
        let p = d.join("out.txt");
        let ps = p.to_str().unwrap().to_string();
        let held;
        {
            let f = ToolOutputFile::create(&ps).unwrap();
            held = f.os_ptr();
            let _ = held.write_all(b"data");
            assert!(p.exists());
        }
        assert!(!p.exists(), "an outstanding os() Ptr must not keep the file alive");
        std::fs::remove_dir_all(&d).unwrap();
    }

    /// `mlir::openOutputFile` on a good path, UNSAFE model: a real file, and the
    /// `Drop` still unlinks it when `keep()` is not called.
    #[test]
    fn open_unsafe_succeeds_and_still_unlinks() {
        let d = scratch("openun");
        let p = d.join("out.txt");
        let mut name: Vec<::libc::c_char> = p
            .to_str()
            .unwrap()
            .bytes()
            .map(|b| b as ::libc::c_char)
            .collect();
        name.push(0);
        {
            let f = unsafe {
                super::tool_output_file_open_unsafe(name, ::std::ptr::null_mut())
            }
            .expect("open must succeed");
            f.write_all(b"partial");
            assert!(p.exists());
        }
        assert!(!p.exists());
        std::fs::remove_dir_all(&d).unwrap();
    }

    /// `mlir::openOutputFile` on a BAD path: returns `None` -- which is the
    /// `nullptr` both call sites branch on -- and writes the error message
    /// through the `std::string *` out-parameter.
    #[test]
    fn open_unsafe_failure_returns_none_and_fills_the_message() {
        let d = scratch("openunerr");
        let p = d.join("no_such_dir").join("out.txt");
        let mut name: Vec<::libc::c_char> = p
            .to_str()
            .unwrap()
            .bytes()
            .map(|b| b as ::libc::c_char)
            .collect();
        name.push(0);
        let mut msg: Vec<::libc::c_char> = vec![0];
        let r = unsafe { super::tool_output_file_open_unsafe(name, &mut msg) };
        assert!(r.is_none(), "a failed open must yield the nullptr the caller tests");
        let text: String = msg
            .iter()
            .take_while(|&&c| c != 0)
            .map(|&c| c as u8 as char)
            .collect();
        assert!(text.starts_with("cannot open output file "), "got {:?}", text);
        assert!(text.contains(p.to_str().unwrap()), "got {:?}", text);
        std::fs::remove_dir_all(&d).unwrap();
    }

    /// The refcount model's `openOutputFile`: same two outcomes, and the
    /// `Value<ToolOutputFile>` still runs `Drop` when the last strong handle
    /// dies.
    #[test]
    fn open_refcount_both_outcomes() {
        let d = scratch("openrc");
        let p = d.join("out.txt");
        let mut name: Vec<u8> = p.to_str().unwrap().as_bytes().to_vec();
        name.push(0);
        {
            let v = super::tool_output_file_open_refcount(name, crate::Ptr::null())
                .expect("open must succeed");
            v.borrow().write_all(b"partial");
            assert!(p.exists());
        }
        assert!(!p.exists(), "dropping the Value must unlink, keep() never ran");

        let mut bad: Vec<u8> = d.join("nope").join("x.txt").to_str().unwrap().as_bytes().to_vec();
        bad.push(0);
        let owner: crate::Value<Vec<u8>> =
            ::std::rc::Rc::new(::std::cell::RefCell::new(vec![0u8]));
        let r = super::tool_output_file_open_refcount(
            bad,
            crate::AsPointer::as_pointer(&owner),
        );
        assert!(r.is_none());
        let text = String::from_utf8_lossy(&owner.borrow()[..]).into_owned();
        assert!(text.starts_with("cannot open output file "), "got {:?}", text);
        std::fs::remove_dir_all(&d).unwrap();
    }


    /// ⭐⭐ THE EMITTED SHAPE, COMPILED AND EXECUTED.
    ///
    /// `rc=0` from the converter does not mean the output compiles -- `rustfmt` is
    /// the only Rust parser in that pipeline.  So this test is the TEXT THE
    /// CONVERTER ACTUALLY EMITS for dxp-driver.cpp:74-84, read back out of
    /// `dxp__src__Driver__dxp-driver.cpp.rs` (lines 33575-33627) and pasted here
    /// with only the C++ identifier names kept:
    ///
    /// ```text
    ///   let mut errorMessage: Vec<libc::c_char> = vec![0];
    ///   let mut output: Option<Box<libcc2rs::ToolOutputFile>> = unsafe {
    ///       libcc2rs::tool_output_file_open_unsafe(
    ///           (*outputFile).clone(),
    ///           (&mut errorMessage as *mut Vec<libc::c_char>),
    ///       )
    ///   };
    ///   ...
    ///   let _os: *mut std::fs::File = (*output.as_deref_mut().unwrap()).os_unsafe();
    ///   ...
    ///   (*output.as_deref_mut().unwrap()).keep();
    /// ```
    ///
    /// Compiling it here proves the three keys type-check through
    /// `rules/unique_ptr`'s `Option<Box<T>>`; running it proves the `Drop` still
    /// fires through that wrapper, which is the claim the row exists to make.
    ///
    /// ⛔ ONE LINE OF THE EMITTED TEXT IS DELIBERATELY NOT REPRODUCED:
    /// `if !(unsafe { output.to_bool() })`.  `to_bool` on an `Option<Box<T>>` is
    /// DEFINED NOWHERE in this tree (grepped: the only `fn to_bool` is
    /// `IStream::to_bool`), so that line is `error[E0599]` -- a PRE-EXISTING
    /// `rules/unique_ptr` gap (it has f16-f19 for `p == nullptr` but no
    /// `operator bool`) that this row merely made REACHABLE by unblocking the TU.
    /// It is a different row with a different owner; `is_some()` stands in for it
    /// here so this test measures THIS module's keys and not that one.
    #[test]
    fn the_emitted_shape_compiles_and_the_drop_still_fires_through_unique_ptr() {
        let d = scratch("emitted");
        let path = d.join("out.txt");
        let mut outputFile: Vec<::libc::c_char> = path
            .to_str()
            .unwrap()
            .bytes()
            .map(|b| b as ::libc::c_char)
            .collect();
        outputFile.push(0);
        let outputFile = &outputFile;

        // ---- FAILURE PATH: the pipeline fails, so `keep()` is never reached.
        {
            let mut errorMessage: Vec<::libc::c_char> = vec![0];
            let mut output: Option<Box<ToolOutputFile>> = unsafe {
                super::tool_output_file_open_unsafe(
                    (*outputFile).clone(),
                    (&mut errorMessage as *mut Vec<::libc::c_char>),
                )
            };
            assert!(output.is_some());
            let _os: *mut std::fs::File = (*output.as_deref_mut().unwrap()).os_unsafe();
            unsafe {
                let _ = ::std::io::Write::write_all(&mut *_os, b"half a pipeline");
            }
            assert!(path.exists());
            // `return 1;` -- no keep().
        }
        assert!(
            !path.exists(),
            "the failure path must leave NO output file: dxp-driver.cpp:79-84 returns without keep()"
        );

        // ---- SUCCESS PATH: same text, plus the emitted `keep()`.
        {
            let mut errorMessage: Vec<::libc::c_char> = vec![0];
            let mut output: Option<Box<ToolOutputFile>> = unsafe {
                super::tool_output_file_open_unsafe(
                    (*outputFile).clone(),
                    (&mut errorMessage as *mut Vec<::libc::c_char>),
                )
            };
            let _os: *mut std::fs::File = (*output.as_deref_mut().unwrap()).os_unsafe();
            unsafe {
                let _ = ::std::io::Write::write_all(&mut *_os, b"a whole pipeline");
            }
            (*output.as_deref_mut().unwrap()).keep();
        }
        assert_eq!(std::fs::read(&path).unwrap(), b"a whole pipeline");
        std::fs::remove_dir_all(&d).unwrap();
    }

}
