// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

//! `mlir::InFlightDiagnostic` -- AN ACCUMULATING BUFFER THAT PRINTS ON `Drop`.
//!
//! WHY THIS IS NOT AN OPAQUE UNIT.  The C++ type's whole purpose is a
//! destructor with an OBSERVABLE EFFECT:
//!
//!     mlir/IR/Diagnostics.h:325-328
//!         ~InFlightDiagnostic() { if (isInFlight()) report(); }
//!     mlir/IR/Diagnostics.h:319-324
//!         InFlightDiagnostic(InFlightDiagnostic &&rhs) ... { rhs.abandon(); }
//!
//! i.e. the accumulated message reaches the DiagnosticEngine WHEN THE VALUE
//! DIES, and the explicit `rhs.abandon()` in the move constructor is what makes
//! it reach the engine EXACTLY ONCE across an arbitrarily long `<<` chain.
//! Mapping this to `()` would translate, compile, and SILENTLY DELETE EVERY
//! DIAGNOSTIC the ported compiler emits -- and the dominant call sites are
//! tablegen-generated parse/verify bodies (KTDFAttributes.cpp.inc:132,
//! KTDFLowering.cpp.inc:30, SDSCBundleTypes.cpp.inc:216) whose ONLY externally
//! visible behaviour IS the message.  Same rule that refused
//! `mlir::OwningOpRef` (rules/mlir/src.cpp, "THIRD SLOT" note).
//!
//! HOW EXACTLY-ONCE IS MODELLED IN RUST, and why no `abandon` is needed on the
//! chain itself.  Every `shl_*` takes `self` BY VALUE and returns `Self` by
//! value.  In Rust a by-value `self` is MOVED into the method and the returned
//! value is a move OUT of it: a moved-from binding is statically dead and
//! `Drop::drop` is NOT run for it.  So a chain
//!
//!     shl_x(shl_y(shl_z(new(), a), b), c)
//!
//! creates ONE live value that is threaded through, and its `drop` runs exactly
//! once, at the end of the enclosing full expression -- which is precisely when
//! the C++ temporary `InFlightDiagnostic` dies.  `abandon()` is still provided
//! because `mlir::InFlightDiagnostic::abandon()` is real API that ported code
//! can call, and because `Diagnostic`-returning overloads need a way to suppress.
//!
//! WHERE THE MESSAGE GOES: STDERR.  MLIR's default handler
//! (`SourceMgrDiagnosticHandler`, and the fallback in `DiagnosticEngine::emit`)
//! writes to `llvm::errs()`.  There is no DiagnosticEngine in the ported Rust,
//! so the faithful destination is the same stream.  The severity prefix is
//! `error: ` because every mapped site in dxp_standalone reaches this type
//! through `emitError`/`emitOpError`.

/// The accumulating diagnostic.  `message` is what has been streamed so far;
/// `live` is C++'s `isInFlight()` -- true until the message is reported or
/// abandoned.
pub struct InFlightDiagnostic {
    message: String,
    live: bool,
}

impl InFlightDiagnostic {
    /// A fresh, in-flight diagnostic with an empty message.
    pub fn new() -> Self {
        InFlightDiagnostic {
            message: String::new(),
            live: true,
        }
    }

    /// `mlir::InFlightDiagnostic::abandon()` -- suppress the report.  Returns
    /// `Self` so it can appear in the same by-value chain shape as `shl_*`.
    pub fn abandon(mut self) -> Self {
        self.live = false;
        self
    }

    /// True while the message has not yet been reported or abandoned;
    /// `isInFlight()`.
    pub fn is_in_flight(&self) -> bool {
        self.live
    }

    /// The accumulated text, for tests.
    pub fn message(&self) -> &str {
        &self.message
    }

    /// `operator<<` for anything with a Display rendering that matches C++'s
    /// `llvm::raw_ostream` rendering: integers, `&str`, `String`, `char`.
    pub fn shl_display<T: ::std::fmt::Display>(mut self, v: T) -> Self {
        use ::std::fmt::Write;
        let _ = write!(self.message, "{}", v);
        self
    }

    /// `operator<<` for the byte-slice string models this port uses for
    /// `llvm::StringRef` / `std::string` (`Vec<u8>` / `Vec<c_char>`).  A
    /// trailing NUL, if present, is not part of the string.
    pub fn shl_bytes(mut self, b: &[u8]) -> Self {
        let end = b.iter().position(|&c| c == 0).unwrap_or(b.len());
        self.message
            .push_str(&String::from_utf8_lossy(&b[..end]));
        self
    }

    /// `operator<<` for the UNSAFE model's string payloads, which are
    /// `Vec<libc::c_char>` (i.e. `i8` on every target this port builds for):
    /// `llvm::StringRef` (rules/stringref t1), `llvm::StringLiteral` (t2) and
    /// `std::string` (rules/string t1) all arrive in that shape.  Reinterpreting
    /// `&[c_char]` as `&[u8]` is a no-op cast -- same size, same alignment, and
    /// `u8` has no invalid bit patterns -- and then the NUL-stopping rule of
    /// `shl_bytes` applies unchanged, so the unsafe and refcount models append
    /// the same text.
    ///
    /// It exists as a method rather than being inlined into a rule body because a
    /// rule body is ONE INLINED EXPRESSION in which every `aN` RE-EVALUATES its
    /// argument: spelling the cast inline would need `a1.as_ptr()` AND `a1.len()`,
    /// mentioning the streamed argument twice.
    pub fn shl_c_chars(self, v: &[::std::os::raw::c_char]) -> Self {
        let b: &[u8] =
            unsafe { ::std::slice::from_raw_parts(v.as_ptr() as *const u8, v.len()) };
        self.shl_bytes(b)
    }

    /// `operator<<(const char (&)[N])` in the UNSAFE model, where a string
    /// literal in this position arrives as a `*const c_char` pointing at NUL
    /// terminated bytes (rules/stringref f8's spelling, and the reason it is a
    /// pointer rather than a slice is recorded there).
    ///
    /// # Safety
    /// `p` must be null or point to a NUL-terminated byte string.
    pub unsafe fn shl_c_str(self, p: *const ::std::os::raw::c_char) -> Self {
        if p.is_null() {
            return self;
        }
        let mut n = 0usize;
        while unsafe { *p.add(n) } != 0 {
            n += 1;
        }
        let bytes = unsafe { ::std::slice::from_raw_parts(p as *const u8, n) };
        self.shl_bytes(bytes)
    }
}

impl Default for InFlightDiagnostic {
    fn default() -> Self {
        Self::new()
    }
}

impl Drop for InFlightDiagnostic {
    fn drop(&mut self) {
        if self.live {
            self.live = false;
            eprintln!("error: {}", self.message);
        }
    }
}
