// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

mod reinterpret;
pub use reinterpret::ByteRepr;

mod rc;
pub use rc::*;

mod cstr;

mod void;
pub use void::*;

mod ptr_dyn;
pub use ptr_dyn::*;

include!(concat!(env!("OUT_DIR"), "/rule_shims.rs"));

mod fn_ptr_arg;

mod fn_ptr;
pub use fn_ptr::FnPtr;

mod callable;
pub use callable::*;

mod inc;
pub use inc::*;

mod dec;
pub use dec::*;

mod rules;
pub use rules::*;

mod io;
pub use io::*;

mod alloc;
pub use alloc::*;

mod iterators;
pub use iterators::*;

mod compat;
pub use compat::*;

mod va_args;
pub use va_args::*;

mod fd;
pub use fd::*;

mod format;
pub use format::*;

mod variant;
pub use variant::*;

mod sync;
pub use sync::*;

// `mlir::InFlightDiagnostic`: an accumulating message buffer that PRINTS ON
// `Drop`.  rules/mlir t70/f21 map to it; the type's destructor is its whole
// purpose, so it cannot be an opaque unit.
mod diag;
pub use diag::*;

// `llvm::APInt`: a fixed-width integer whose WIDTH IS PART OF THE VALUE, since
// `getSExtValue()` sign-extends from `BitWidth` and `operator==` asserts on it.
// rules/apint t2/f5-f9 map to it.  A named struct rather than a tuple because
// every member is a call on an `APInt` receiver and a tuple receiver carries no
// member rules.
mod apint;
pub use apint::*;

// `IStream`: the STICKY-FAILBIT input stream.  C++ stream error state is sticky
// -- once extraction fails, every later `>>` is a no-op -- so the flag has to
// live in the stream object, not in a per-call `Result`.  See istream.rs.
mod istream;
pub use istream::*;

// `llvm::ToolOutputFile`: an output file that its DESTRUCTOR UNLINKS unless
// `keep()` ran.  Both of the port goal's own drivers (dxp-driver.cpp:79-84,
// DxpOptMain.cpp:283-289) return on the failure path WITHOUT calling `keep()`,
// so a value model over a bare file handle would silently leave a truncated
// output file behind.  rules/tooloutputfile maps to it.
mod tool_output_file;
pub use tool_output_file::*;

pub use libcc2rs_macros::{ByteRepr, goto, goto_block, switch};
