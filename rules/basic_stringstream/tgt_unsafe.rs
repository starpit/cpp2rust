// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp for the model: a std::stringstream is a Vec<u8>, which already
// implements std::io::Write by APPENDING, and appending is the only operation
// the converter's built-in ostream lowering performs on it.

fn t1() -> Vec<u8> {
    Vec::new()
}

fn f1() -> Vec<u8> {
    Vec::new()
}

// .str() -- a COPY out of the buffer into std::string's representation, which in
// the unsafe model is a NUL-terminated Vec<libc::c_char>.  The terminator is not
// part of the string (rules/string's size() is len()-1), so exactly one is
// appended here.  Copying rather than moving a0 through matters: `ss.str()` must
// not empty the stream.
fn f2(a0: Vec<u8>) -> Vec<libc::c_char> {
    let mut __s: Vec<libc::c_char> = a0.iter().map(|&__b| __b as libc::c_char).collect();
    __s.push(0);
    __s
}

// f3 -- `std::stringstream ss(str)`, the overload the whole 35-TU row actually
// needed (see src.cpp).  std::string in the unsafe model is a NUL-TERMINATED
// Vec<libc::c_char> (rules/string's size() is len()-1), and a stringstream is a
// plain Vec<u8> of exactly the bytes, so the terminator is DROPPED here -- the
// mirror image of f2, which appends one.  saturating_sub, not `- 1`: an empty
// std::string is a one-element Vec holding only the NUL, and a corpus caller
// with an empty env var would otherwise panic on the slice.
//
// a1 is the DEFAULTED `openmode` argument, kept in the key and mentioned exactly
// once because a target parameter cannot be named `_a1` (ir.rs:40 rejects it).
// It is deliberately ignored, and THAT IS NOT WITHOUT COST -- see the
// "UNREACHABLE IN THE CORPUS" block in src.cpp.  Correction to what this comment
// used to claim: in|out|ate are NOT indistinguishable.  `ate` means "put position
// at end", i.e. append; plain in|out means put position 0, i.e. OVERWRITE from
// the front.  A Vec<u8> can only append, so a corpus site doing
// construct-from-string-then-insert would get `abcXY12` where C++ gives `12cXY`
// (measured, /home/agent/work/ssput/ssput.cpp).  No corpus site does that -- the
// grep is in src.cpp -- so the defect is unreachable and the model stands.  The
// state-touching members that would observe the difference (seekp/tellp/seekg/
// the str() setter) stay unmapped so they abort loudly.
fn f3(a0: Vec<libc::c_char>, a1: libc::c_uint) -> Vec<u8> {
    let _ = a1;
    let __s = a0;
    let __n = __s.len().saturating_sub(1);
    __s[..__n].iter().map(|&__b| __b as u8).collect()
}
