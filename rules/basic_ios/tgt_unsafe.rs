// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Every body below repeats the same private trait: one rule is inlined on its
// own at each call site, so the trait has to travel with it.  The three pieces
// are identical in all seven bodies and only the final bit test differs --
// keep them in step by editing all seven together.
//
// __cc2_iostate() returns libc++'s std::ios_base::iostate bits:
//   badbit = 1, eofbit = 2, failbit = 4, goodbit = 0.

// std::basic_ios<char>::good() -- state == goodbit.
unsafe fn f1(a0: Box<libcc2rs::StringStream>) -> bool {
    ({
        trait __Cc2Ios {
            fn __cc2_iostate(&self) -> u32;
        }
        impl __Cc2Ios for libcc2rs::StringStream {
            fn __cc2_iostate(&self) -> u32 {
                libcc2rs::Cc2Extract::cc2_get_state(self)
            }
        }
        impl __Cc2Ios for ::std::fs::File {
            fn __cc2_iostate(&self) -> u32 {
                // The recorded state, OR'd with a position-derived eofbit so
                // that a stream nobody has read through a state-setting rule
                // still reports eof where it used to. Only eofbit can be
                // inferred this way; failbit cannot, which is the whole point.
                let mut __h: &::std::fs::File = self;
                let __pos_eof = match ::std::io::Seek::stream_position(&mut __h) {
                    Ok(__p)
                        if __p > 0
                            && __p >= self.metadata().map(|__m| __m.len()).unwrap_or(u64::MAX) =>
                    {
                        2
                    }
                    _ => 0,
                };
                libcc2rs::Cc2Extract::cc2_get_state(self) | __pos_eof
            }
        }
        a0.__cc2_iostate() == 0
    })
}

// std::basic_ios<char>::eof() -- state & eofbit.
unsafe fn f2(a0: Box<libcc2rs::StringStream>) -> bool {
    ({
        trait __Cc2Ios {
            fn __cc2_iostate(&self) -> u32;
        }
        impl __Cc2Ios for libcc2rs::StringStream {
            fn __cc2_iostate(&self) -> u32 {
                libcc2rs::Cc2Extract::cc2_get_state(self)
            }
        }
        impl __Cc2Ios for ::std::fs::File {
            fn __cc2_iostate(&self) -> u32 {
                // The recorded state, OR'd with a position-derived eofbit so
                // that a stream nobody has read through a state-setting rule
                // still reports eof where it used to. Only eofbit can be
                // inferred this way; failbit cannot, which is the whole point.
                let mut __h: &::std::fs::File = self;
                let __pos_eof = match ::std::io::Seek::stream_position(&mut __h) {
                    Ok(__p)
                        if __p > 0
                            && __p >= self.metadata().map(|__m| __m.len()).unwrap_or(u64::MAX) =>
                    {
                        2
                    }
                    _ => 0,
                };
                libcc2rs::Cc2Extract::cc2_get_state(self) | __pos_eof
            }
        }
        a0.__cc2_iostate() & 2 != 0
    })
}

// std::basic_ios<char>::fail() -- state & (failbit | badbit).
unsafe fn f3(a0: Box<libcc2rs::StringStream>) -> bool {
    ({
        trait __Cc2Ios {
            fn __cc2_iostate(&self) -> u32;
        }
        impl __Cc2Ios for libcc2rs::StringStream {
            fn __cc2_iostate(&self) -> u32 {
                libcc2rs::Cc2Extract::cc2_get_state(self)
            }
        }
        impl __Cc2Ios for ::std::fs::File {
            fn __cc2_iostate(&self) -> u32 {
                // The recorded state, OR'd with a position-derived eofbit so
                // that a stream nobody has read through a state-setting rule
                // still reports eof where it used to. Only eofbit can be
                // inferred this way; failbit cannot, which is the whole point.
                let mut __h: &::std::fs::File = self;
                let __pos_eof = match ::std::io::Seek::stream_position(&mut __h) {
                    Ok(__p)
                        if __p > 0
                            && __p >= self.metadata().map(|__m| __m.len()).unwrap_or(u64::MAX) =>
                    {
                        2
                    }
                    _ => 0,
                };
                libcc2rs::Cc2Extract::cc2_get_state(self) | __pos_eof
            }
        }
        a0.__cc2_iostate() & 5 != 0
    })
}

// std::basic_ios<char>::bad() -- state & badbit.
unsafe fn f4(a0: Box<libcc2rs::StringStream>) -> bool {
    ({
        trait __Cc2Ios {
            fn __cc2_iostate(&self) -> u32;
        }
        impl __Cc2Ios for libcc2rs::StringStream {
            fn __cc2_iostate(&self) -> u32 {
                libcc2rs::Cc2Extract::cc2_get_state(self)
            }
        }
        impl __Cc2Ios for ::std::fs::File {
            fn __cc2_iostate(&self) -> u32 {
                // The recorded state, OR'd with a position-derived eofbit so
                // that a stream nobody has read through a state-setting rule
                // still reports eof where it used to. Only eofbit can be
                // inferred this way; failbit cannot, which is the whole point.
                let mut __h: &::std::fs::File = self;
                let __pos_eof = match ::std::io::Seek::stream_position(&mut __h) {
                    Ok(__p)
                        if __p > 0
                            && __p >= self.metadata().map(|__m| __m.len()).unwrap_or(u64::MAX) =>
                    {
                        2
                    }
                    _ => 0,
                };
                libcc2rs::Cc2Extract::cc2_get_state(self) | __pos_eof
            }
        }
        a0.__cc2_iostate() & 1 != 0
    })
}

// std::basic_ios<char>::clear() -- neither representation carries an error
// state, so resetting it is a no-op.  The receiver is still read, because the
// converter must keep evaluating the receiver expression.
unsafe fn f5(a0: Box<libcc2rs::StringStream>) {
    ({
        trait __Cc2Ios {
            fn __cc2_iostate(&self) -> u32;
        }
        impl __Cc2Ios for libcc2rs::StringStream {
            fn __cc2_iostate(&self) -> u32 {
                libcc2rs::Cc2Extract::cc2_get_state(self)
            }
        }
        impl __Cc2Ios for ::std::fs::File {
            fn __cc2_iostate(&self) -> u32 {
                // The recorded state, OR'd with a position-derived eofbit so
                // that a stream nobody has read through a state-setting rule
                // still reports eof where it used to. Only eofbit can be
                // inferred this way; failbit cannot, which is the whole point.
                let mut __h: &::std::fs::File = self;
                let __pos_eof = match ::std::io::Seek::stream_position(&mut __h) {
                    Ok(__p)
                        if __p > 0
                            && __p >= self.metadata().map(|__m| __m.len()).unwrap_or(u64::MAX) =>
                    {
                        2
                    }
                    _ => 0,
                };
                libcc2rs::Cc2Extract::cc2_get_state(self) | __pos_eof
            }
        }
        let _ = a0.__cc2_iostate();
    })
}

// std::basic_ios<char>::operator bool() -- !fail().
unsafe fn f6(a0: Box<libcc2rs::StringStream>) -> bool {
    ({
        trait __Cc2Ios {
            fn __cc2_iostate(&self) -> u32;
        }
        impl __Cc2Ios for libcc2rs::StringStream {
            fn __cc2_iostate(&self) -> u32 {
                libcc2rs::Cc2Extract::cc2_get_state(self)
            }
        }
        impl __Cc2Ios for ::std::fs::File {
            fn __cc2_iostate(&self) -> u32 {
                // The recorded state, OR'd with a position-derived eofbit so
                // that a stream nobody has read through a state-setting rule
                // still reports eof where it used to. Only eofbit can be
                // inferred this way; failbit cannot, which is the whole point.
                let mut __h: &::std::fs::File = self;
                let __pos_eof = match ::std::io::Seek::stream_position(&mut __h) {
                    Ok(__p)
                        if __p > 0
                            && __p >= self.metadata().map(|__m| __m.len()).unwrap_or(u64::MAX) =>
                    {
                        2
                    }
                    _ => 0,
                };
                libcc2rs::Cc2Extract::cc2_get_state(self) | __pos_eof
            }
        }
        a0.__cc2_iostate() & 5 == 0
    })
}

// std::basic_ios<char>::operator!() -- fail().
unsafe fn f7(a0: Box<libcc2rs::StringStream>) -> bool {
    ({
        trait __Cc2Ios {
            fn __cc2_iostate(&self) -> u32;
        }
        impl __Cc2Ios for libcc2rs::StringStream {
            fn __cc2_iostate(&self) -> u32 {
                libcc2rs::Cc2Extract::cc2_get_state(self)
            }
        }
        impl __Cc2Ios for ::std::fs::File {
            fn __cc2_iostate(&self) -> u32 {
                // The recorded state, OR'd with a position-derived eofbit so
                // that a stream nobody has read through a state-setting rule
                // still reports eof where it used to. Only eofbit can be
                // inferred this way; failbit cannot, which is the whole point.
                let mut __h: &::std::fs::File = self;
                let __pos_eof = match ::std::io::Seek::stream_position(&mut __h) {
                    Ok(__p)
                        if __p > 0
                            && __p >= self.metadata().map(|__m| __m.len()).unwrap_or(u64::MAX) =>
                    {
                        2
                    }
                    _ => 0,
                };
                libcc2rs::Cc2Extract::cc2_get_state(self) | __pos_eof
            }
        }
        a0.__cc2_iostate() & 5 != 0
    })
}

// std::basic_ios<char>::fill() -- the current pad character.
//
// Declared `-> u8` rather than `-> libc::c_char` on purpose.  `char` is spelled
// i8 by the unsafe model and u8 by the refcount model, and rules/basic_ios has
// ONE target file shared by both, so a body that names either concretely is
// wrong in one model -- while `as _` cannot infer at a call site that casts the
// result (`(..) as u8`, which is what an insertion emits) or discards it.  u8 is
// what every consumer of a char casts to, so it serves both; the one shape it
// does not serve is a bare `char c = os.fill();` in the UNSAFE model, which
// would want i8.  The setter below dodges this by typing its result from its own
// char parameter; fixing the getter properly means giving this module a
// per-model tgt_refcount.rs.
unsafe fn f8(a0: &mut Box<libcc2rs::StringStream>) -> u8 {
    ({
        use libcc2rs::Cc2Insert;
        a0.cc2_fill() as u8
    })
}

// std::basic_ios<char>::fill(char) -- set it, return the PREVIOUS one.
unsafe fn f9(a0: &mut Box<libcc2rs::StringStream>, a1: libc::c_char) -> libc::c_char {
    ({
        use libcc2rs::Cc2Insert;
        let mut __prev = a1;
        __prev = a0.cc2_fill() as _;
        a0.cc2_set_fill(a1 as u8);
        __prev
    })
}

// std::ios_base::width() -- read the pending width WITHOUT consuming it.
unsafe fn f10(a0: &mut Box<libcc2rs::StringStream>) -> libc::c_long {
    ({
        use libcc2rs::Cc2Insert;
        let __w = a0.cc2_take_width();
        a0.cc2_set_width(__w);
        __w as i64
    })
}

// std::ios_base::width(streamsize) -- set it, return the PREVIOUS one.  The
// pending width is consumed by the next insertion, not by this call.
unsafe fn f11(a0: &mut Box<libcc2rs::StringStream>, a1: libc::c_long) -> libc::c_long {
    ({
        use libcc2rs::Cc2Insert;
        let __prev = a0.cc2_take_width() as i64;
        a0.cc2_set_width(if a1 > 0 { a1 as usize } else { 0 });
        __prev
    })
}

// std::basic_ostream<char>::put(char) -- unformatted, so NO padding.
unsafe fn f12<'a>(
    a0: &'a mut Box<libcc2rs::StringStream>,
    a1: libc::c_char,
) -> &'a mut Box<libcc2rs::StringStream> {
    ({
        use libcc2rs::Cc2Insert;
        a0.cc2_write(&[a1 as u8]);
        libcc2rs::stream_mut(&mut *a0)
    })
}
