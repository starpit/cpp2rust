// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

unsafe fn f1(a0: i32) -> i32 {
    a0
}

// f2 -- `std::setfill(char)`.  THE IDENTITY IS THE CORRECT BODY: see src.cpp for
// the measurement.  The value this produces is consumed by
// `Converter::GetFmtArg` as the FILL CHARACTER of the next field, where it is
// cast `as u8 as char` -- so the rule hands back the caller's own `char`
// expression and nothing else.  The body is INLINED, so after substitution the
// emitted text is just the argument; the declared type is what lets the rules
// crate compile, not what reaches the call site.
unsafe fn f2(a0: libc::c_char) -> libc::c_char {
    a0
}
