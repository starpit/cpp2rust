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

mod libc_shims;
pub use libc_shims::*;

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

mod stream_fmt;
pub use stream_fmt::*;

mod variant;
pub use variant::*;

mod deep_clone;
pub use deep_clone::*;

mod cmp_set;
pub use cmp_set::*;

mod vec_map_iter;
pub use vec_map_iter::*;

pub use libcc2rs_macros::{ByteRepr, goto, goto_block, switch};
