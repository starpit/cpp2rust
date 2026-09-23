// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn f4(a0: Vec<u8>) -> Box<libcc2rs::StringStream> {
    Box::new(libcc2rs::StringStream::from_vec(
        a0.iter().copied().take_while(|&c| c != 0).collect(),
    ))
}

fn f5(a0: Vec<u8>) -> Box<libcc2rs::StringStream> {
    Box::new(libcc2rs::StringStream::from_vec(
        a0.iter().copied().take_while(|&c| c != 0).collect(),
    ))
}

fn f6(a0: Vec<u8>) -> Box<libcc2rs::StringStream> {
    Box::new(libcc2rs::StringStream::from_vec(
        a0.iter().copied().take_while(|&c| c != 0).collect(),
    ))
}

fn f7(a0: Box<libcc2rs::StringStream>) -> Vec<u8> {
    let mut __s: Vec<u8> = a0.to_vec();
    __s.push(0);
    __s
}

fn f8(a0: Box<libcc2rs::StringStream>) -> Vec<u8> {
    let mut __s: Vec<u8> = a0.to_vec();
    __s.push(0);
    __s
}

fn f9(a0: Box<libcc2rs::StringStream>) -> Vec<u8> {
    let mut __s: Vec<u8> = a0.to_vec();
    __s.push(0);
    __s
}

fn f10(a0: Ptr<Box<libcc2rs::StringStream>>, a1: Vec<u8>) {
    a0.with_mut(|__v: &mut Box<libcc2rs::StringStream>| {
        __v.clear();
        __v.extend(a1.iter().copied().take_while(|&c| c != 0));
    });
}

fn f11(a0: Ptr<Box<libcc2rs::StringStream>>, a1: Vec<u8>) {
    a0.with_mut(|__v: &mut Box<libcc2rs::StringStream>| {
        __v.clear();
        __v.extend(a1.iter().copied().take_while(|&c| c != 0));
    });
}

fn f12(a0: Ptr<Box<libcc2rs::StringStream>>, a1: Vec<u8>) {
    a0.with_mut(|__v: &mut Box<libcc2rs::StringStream>| {
        __v.clear();
        __v.extend(a1.iter().copied().take_while(|&c| c != 0));
    });
}

// ---------------------------------------------------------------------------
// The ten numeric extractors, std::istream::operator>>(T &), and f33, the
// manipulator.  Three things were wrong with the shape these had, and one
// rewrite fixes all three.
//
// 1. THE RADIX WAS IGNORED.  `inFile >> std::hex >> lineno` parsed hexadecimal
//    input as DECIMAL -- no placeholder, no from_str_radix anywhere, and the
//    DT_CHECK(lineno != -1) downstream then passed on the wrong number.  The
//    base cannot be read at the extraction site, because it is STICKY PER
//    STREAM: `f >> std::hex >> a; f >> b;` gives a=255 b=16 in C++, with no
//    manipulator anywhere near the second read.  So the body asks the STREAM
//    for its radix and never looks at syntax.
//
// 2. THEY DID NOT COMPILE ON A FILE STREAM.  These print ONE signature
//    (`std::istream`) for both Rust representations, and the old bodies were
//    written against the string-stream drain buffer directly -- `a0.len()`,
//    `a0[__i]`, `a0.drain(..)` -- which is `File::len()` on a std::ifstream.
//
// 3. A CHAINED EXTRACTION RE-RAN ITS RECEIVER.  The converter inlines a body by
//    textual substitution and re-emits the receiver EXPRESSION at every
//    placeholder, so the old f21 body -- which named its receiver in 23
//    fragments -- nested the inner extraction of `ss >> a >> b`, side effects
//    and all, 23 times inside the outer one.
//
// Each body is now ONE call to a libcc2rs helper, with the receiver named
// exactly once.  Two details of that shape are load-bearing and were both paid
// for by a failing compile:
//
//   * the dispatch trait is libcc2rs::Cc2Extract, declared in the LIBRARY,
//     where rules/basic_ios declares its trait inside the rule body.  That
//     difference is forced: `operator>>` returns the stream, so a chained
//     extraction makes a rule its own receiver, and a body carrying its own
//     trait+impl items then nests two IDENTICAL impls into one function --
//     `error[E0034]: multiple applicable items in scope`.  Per-rule method
//     names do not help, because the collision is a rule nested in itself.
//     rules/basic_ios never meets this: `good()` returns bool, so a predicate
//     can never be its own receiver.
//
//   * the body is a single EXPRESSION, not a sequence of statements.  The
//     refcount model spells the receiver `&mut (*ss.borrow_mut())`, and that
//     `RefMut` temporary dies at the end of its statement, so a multi-statement
//     body gives `error[E0716]: temporary value dropped while borrowed`.
// ---------------------------------------------------------------------------

// The refcount overlay differs from the unsafe bodies in exactly one respect,
// and it is representation bookkeeping rather than behaviour: the OPERAND is a
// Ptr<T>, so it is written with `.write(..)` rather than through a `&mut`.  The
// RECEIVER stays spelled `&mut Box<..>` and NOT `Ptr<Box<..>>`: a Ptr-typed
// parameter makes the converter emit `f.as_pointer()` typed as
// `Ptr<Box<StringStream>>` at the CALL SITE, outside the body where no trait can
// reach it, which on a std::ifstream is `Ptr<File>` and gives
// `error[E0605]: non-primitive cast: Ptr<File> as Ptr<Box<Vec<u8>>>`.  Measured
// against the previous rules, which had exactly that bug.

fn f19<'a>(
    a0: &'a mut Box<libcc2rs::StringStream>,
    a1: Ptr<i32>,
) -> &'a mut Box<libcc2rs::StringStream> {
    (libcc2rs::extract_int(&mut *a0, |__tok, __radix| {
        a1.write(libcc2rs::parse_i64(__tok, __radix) as i32);
    }))
}

fn f20<'a>(
    a0: &'a mut Box<libcc2rs::StringStream>,
    a1: Ptr<u32>,
) -> &'a mut Box<libcc2rs::StringStream> {
    (libcc2rs::extract_int(&mut *a0, |__tok, __radix| {
        a1.write(libcc2rs::parse_u64(__tok, __radix) as u32);
    }))
}

fn f21<'a>(
    a0: &'a mut Box<libcc2rs::StringStream>,
    a1: Ptr<i64>,
) -> &'a mut Box<libcc2rs::StringStream> {
    (libcc2rs::extract_int(&mut *a0, |__tok, __radix| {
        a1.write(libcc2rs::parse_i64(__tok, __radix) as i64);
    }))
}

fn f22<'a>(
    a0: &'a mut Box<libcc2rs::StringStream>,
    a1: Ptr<u64>,
) -> &'a mut Box<libcc2rs::StringStream> {
    (libcc2rs::extract_int(&mut *a0, |__tok, __radix| {
        a1.write(libcc2rs::parse_u64(__tok, __radix) as u64);
    }))
}

fn f23<'a>(
    a0: &'a mut Box<libcc2rs::StringStream>,
    a1: Ptr<i64>,
) -> &'a mut Box<libcc2rs::StringStream> {
    (libcc2rs::extract_int(&mut *a0, |__tok, __radix| {
        a1.write(libcc2rs::parse_i64(__tok, __radix) as i64);
    }))
}

fn f24<'a>(
    a0: &'a mut Box<libcc2rs::StringStream>,
    a1: Ptr<u64>,
) -> &'a mut Box<libcc2rs::StringStream> {
    (libcc2rs::extract_int(&mut *a0, |__tok, __radix| {
        a1.write(libcc2rs::parse_u64(__tok, __radix) as u64);
    }))
}

fn f25<'a>(
    a0: &'a mut Box<libcc2rs::StringStream>,
    a1: Ptr<i16>,
) -> &'a mut Box<libcc2rs::StringStream> {
    (libcc2rs::extract_int(&mut *a0, |__tok, __radix| {
        a1.write(libcc2rs::parse_i64(__tok, __radix) as i16);
    }))
}

fn f26<'a>(
    a0: &'a mut Box<libcc2rs::StringStream>,
    a1: Ptr<u16>,
) -> &'a mut Box<libcc2rs::StringStream> {
    (libcc2rs::extract_int(&mut *a0, |__tok, __radix| {
        a1.write(libcc2rs::parse_u64(__tok, __radix) as u16);
    }))
}

fn f27<'a>(
    a0: &'a mut Box<libcc2rs::StringStream>,
    a1: Ptr<f32>,
) -> &'a mut Box<libcc2rs::StringStream> {
    (libcc2rs::extract_float(&mut *a0, |__tok| {
        a1.write(__tok.parse::<f32>().unwrap_or(Default::default()));
    }))
}

fn f28<'a>(
    a0: &'a mut Box<libcc2rs::StringStream>,
    a1: Ptr<f64>,
) -> &'a mut Box<libcc2rs::StringStream> {
    (libcc2rs::extract_float(&mut *a0, |__tok| {
        a1.write(__tok.parse::<f64>().unwrap_or(Default::default()));
    }))
}

fn f29(a0: &mut Box<libcc2rs::StringStream>, a1: Ptr<Vec<u8>>) -> Ptr<Box<libcc2rs::StringStream>> {
    ({
        trait __Cc2Tok {
            fn __cc2_token(&mut self) -> Vec<u8>;
        }
        impl __Cc2Tok for libcc2rs::StringStream {
            fn __cc2_token(&mut self) -> Vec<u8> {
                let mut __i = 0usize;
                while __i < self.len() && self[__i].is_ascii_whitespace() {
                    __i += 1;
                }
                let __b = __i;
                while __i < self.len() && !self[__i].is_ascii_whitespace() {
                    __i += 1;
                }
                let __t = self[__b..__i].to_vec();
                self.drain(..__i);
                __t
            }
        }
        impl __Cc2Tok for ::std::fs::File {
            fn __cc2_token(&mut self) -> Vec<u8> {
                use ::std::io::Read;
                use ::std::io::Seek;
                let mut __out: Vec<u8> = Vec::new();
                let mut __b = [0u8; 1];
                loop {
                    match self.read(&mut __b) {
                        Ok(0) => break,
                        Ok(_) => {
                            if __b[0].is_ascii_whitespace() {
                                if __out.is_empty() {
                                    continue;
                                }
                                // C++ stops AT the delimiter without consuming
                                // it: the leading run of whitespace is eaten,
                                // the trailing one is not.
                                let _ = self.seek(::std::io::SeekFrom::Current(-1));
                                break;
                            }
                            __out.push(__b[0]);
                        }
                        Err(_) => break,
                    }
                }
                __out
            }
        }
        let mut __out: Vec<u8> = a0.__cc2_token();
        __out.push(0);
        a1.write(__out);
        Ptr::<Box<libcc2rs::StringStream>>::null()
    })
}

fn f30(a0: &mut Box<libcc2rs::StringStream>, a1: Ptr<Vec<u8>>, a2: u8) -> Ptr<Box<libcc2rs::StringStream>> {
    ({
        trait __Cc2Line {
            fn __cc2_getline(&mut self, __d: u8) -> Vec<u8>;
        }
        impl __Cc2Line for libcc2rs::StringStream {
            fn __cc2_getline(&mut self, __d: u8) -> Vec<u8> {
                let __pos = self.iter().position(|&c| c == __d);
                let __keep = __pos.unwrap_or(self.len());
                let __end = __pos.map(|__p| __p + 1).unwrap_or(self.len());
                self.drain(..__end).take(__keep).collect()
            }
        }
        impl __Cc2Line for ::std::fs::File {
            fn __cc2_getline(&mut self, __d: u8) -> Vec<u8> {
                use ::std::io::Read;
                let mut __out: Vec<u8> = Vec::new();
                let mut __b = [0u8; 1];
                loop {
                    match self.read(&mut __b) {
                        Ok(0) => break,
                        Ok(_) => {
                            if __b[0] == __d {
                                break;
                            }
                            __out.push(__b[0]);
                        }
                        Err(_) => break,
                    }
                }
                __out
            }
        }
        // The delimiter is this rule's own parameter, not a fixed newline.
        let mut __out: Vec<u8> = a0.__cc2_getline(a2);
        __out.push(0);
        a1.write(__out);
        Ptr::<Box<libcc2rs::StringStream>>::null()
    })
}

fn f31(a0: &mut Box<libcc2rs::StringStream>, a1: Ptr<Vec<u8>>) -> Ptr<Box<libcc2rs::StringStream>> {
    ({
        trait __Cc2Line1 {
            fn __cc2_getline1(&mut self) -> Vec<u8>;
        }
        impl __Cc2Line1 for libcc2rs::StringStream {
            fn __cc2_getline1(&mut self) -> Vec<u8> {
                let __pos = self.iter().position(|&c| c == b'\n');
                let __keep = __pos.unwrap_or(self.len());
                let __end = __pos.map(|__p| __p + 1).unwrap_or(self.len());
                self.drain(..__end).take(__keep).collect()
            }
        }
        impl __Cc2Line1 for ::std::fs::File {
            fn __cc2_getline1(&mut self) -> Vec<u8> {
                use ::std::io::Read;
                let mut __out: Vec<u8> = Vec::new();
                let mut __b = [0u8; 1];
                loop {
                    match self.read(&mut __b) {
                        Ok(0) => break,
                        Ok(_) => {
                            if __b[0] == b'\n' {
                                break;
                            }
                            __out.push(__b[0]);
                        }
                        Err(_) => break,
                    }
                }
                __out
            }
        }
        let mut __out: Vec<u8> = a0.__cc2_getline1();
        __out.push(0);
        a1.write(__out);
        Ptr::<Box<libcc2rs::StringStream>>::null()
    })
}

fn f32(a0: &mut Box<libcc2rs::StringStream>, a1: Ptr<u8>) -> Ptr<Box<libcc2rs::StringStream>> {
    ({
        trait __Cc2Ch {
            fn __cc2_char(&mut self) -> u8;
        }
        impl __Cc2Ch for libcc2rs::StringStream {
            fn __cc2_char(&mut self) -> u8 {
                let mut __i = 0usize;
                while __i < self.len() && self[__i].is_ascii_whitespace() {
                    __i += 1;
                }
                let __ch = if __i < self.len() { self[__i] } else { 0 };
                let __end = ::std::cmp::min(__i + 1, self.len());
                self.drain(..__end);
                __ch
            }
        }
        impl __Cc2Ch for ::std::fs::File {
            fn __cc2_char(&mut self) -> u8 {
                use ::std::io::Read;
                let mut __b = [0u8; 1];
                loop {
                    match self.read(&mut __b) {
                        Ok(0) => return 0,
                        // A single char consumes exactly the char it returns,
                        // so unlike the token form there is nothing to push
                        // back: the file position is already where C++ leaves
                        // it.
                        Ok(_) if !__b[0].is_ascii_whitespace() => return __b[0],
                        Ok(_) => continue,
                        Err(_) => return 0,
                    }
                }
            }
        }
        a1.write(a0.__cc2_char());
        Ptr::<Box<libcc2rs::StringStream>>::null()
    })
}


// operator>>(std::ios_base &(*)(std::ios_base &)) -- apply a manipulator.
//
// This needs a refcount overlay purely because of how each model spells a
// function pointer: the unsafe body's parameter is `unsafe fn(*mut u32) -> *mut
// u32`, and in this model the same C++ type arrives as a safe `fn` over
// `Ptr<u32>`.  The behaviour, and the reason the state has to live on the
// stream at all, are identical -- see the note in src.cpp.

fn f33<'a>(
    a0: &'a mut Box<libcc2rs::StringStream>,
    a1: fn(Ptr<u32>) -> Ptr<u32>,
) -> &'a mut Box<libcc2rs::StringStream> {
    (libcc2rs::manip_refcount(&mut *a0, a1))
}
