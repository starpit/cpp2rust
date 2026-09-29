// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Every body is a SELF-CONTAINED BLOCK EXPRESSION carrying its own
// `unsafe extern "C"` item -- see src.cpp for why a file-level item would be
// strictly worse than no key at all.

unsafe fn f1(a0: *mut *mut ::libc::c_void, a1: ::libc::c_int) -> ::libc::c_int {
    {
        unsafe extern "C" {
            fn backtrace(__array: *mut *mut ::libc::c_void, __size: ::libc::c_int) -> ::libc::c_int;
        }
        backtrace(a0, a1)
    }
}

unsafe fn f2(a0: *const *mut ::libc::c_void, a1: ::libc::c_int) -> *mut *mut ::libc::c_char {
    {
        unsafe extern "C" {
            fn backtrace_symbols(
                __array: *const *mut ::libc::c_void,
                __size: ::libc::c_int,
            ) -> *mut *mut ::libc::c_char;
        }
        backtrace_symbols(a0, a1)
    }
}

unsafe fn f3(
    a0: *const ::libc::c_char,
    a1: *mut ::libc::c_char,
    a2: *mut usize,
    a3: *mut ::libc::c_int,
) -> *mut ::libc::c_char {
    {
        unsafe extern "C" {
            fn __cxa_demangle(
                __mangled_name: *const ::libc::c_char,
                __output_buffer: *mut ::libc::c_char,
                __length: *mut usize,
                __status: *mut ::libc::c_int,
            ) -> *mut ::libc::c_char;
        }
        __cxa_demangle(a0, a1, a2, a3)
    }
}
