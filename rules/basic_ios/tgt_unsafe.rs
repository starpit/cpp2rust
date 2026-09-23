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
                if self.is_empty() { 6 } else { 0 }
            }
        }
        impl __Cc2Ios for ::std::fs::File {
            fn __cc2_iostate(&self) -> u32 {
                let mut __h: &::std::fs::File = self;
                match ::std::io::Seek::stream_position(&mut __h) {
                    Ok(__p)
                        if __p > 0
                            && __p >= self.metadata().map(|__m| __m.len()).unwrap_or(u64::MAX) =>
                    {
                        2
                    }
                    _ => 0,
                }
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
                if self.is_empty() { 6 } else { 0 }
            }
        }
        impl __Cc2Ios for ::std::fs::File {
            fn __cc2_iostate(&self) -> u32 {
                let mut __h: &::std::fs::File = self;
                match ::std::io::Seek::stream_position(&mut __h) {
                    Ok(__p)
                        if __p > 0
                            && __p >= self.metadata().map(|__m| __m.len()).unwrap_or(u64::MAX) =>
                    {
                        2
                    }
                    _ => 0,
                }
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
                if self.is_empty() { 6 } else { 0 }
            }
        }
        impl __Cc2Ios for ::std::fs::File {
            fn __cc2_iostate(&self) -> u32 {
                let mut __h: &::std::fs::File = self;
                match ::std::io::Seek::stream_position(&mut __h) {
                    Ok(__p)
                        if __p > 0
                            && __p >= self.metadata().map(|__m| __m.len()).unwrap_or(u64::MAX) =>
                    {
                        2
                    }
                    _ => 0,
                }
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
                if self.is_empty() { 6 } else { 0 }
            }
        }
        impl __Cc2Ios for ::std::fs::File {
            fn __cc2_iostate(&self) -> u32 {
                let mut __h: &::std::fs::File = self;
                match ::std::io::Seek::stream_position(&mut __h) {
                    Ok(__p)
                        if __p > 0
                            && __p >= self.metadata().map(|__m| __m.len()).unwrap_or(u64::MAX) =>
                    {
                        2
                    }
                    _ => 0,
                }
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
                if self.is_empty() { 6 } else { 0 }
            }
        }
        impl __Cc2Ios for ::std::fs::File {
            fn __cc2_iostate(&self) -> u32 {
                let mut __h: &::std::fs::File = self;
                match ::std::io::Seek::stream_position(&mut __h) {
                    Ok(__p)
                        if __p > 0
                            && __p >= self.metadata().map(|__m| __m.len()).unwrap_or(u64::MAX) =>
                    {
                        2
                    }
                    _ => 0,
                }
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
                if self.is_empty() { 6 } else { 0 }
            }
        }
        impl __Cc2Ios for ::std::fs::File {
            fn __cc2_iostate(&self) -> u32 {
                let mut __h: &::std::fs::File = self;
                match ::std::io::Seek::stream_position(&mut __h) {
                    Ok(__p)
                        if __p > 0
                            && __p >= self.metadata().map(|__m| __m.len()).unwrap_or(u64::MAX) =>
                    {
                        2
                    }
                    _ => 0,
                }
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
                if self.is_empty() { 6 } else { 0 }
            }
        }
        impl __Cc2Ios for ::std::fs::File {
            fn __cc2_iostate(&self) -> u32 {
                let mut __h: &::std::fs::File = self;
                match ::std::io::Seek::stream_position(&mut __h) {
                    Ok(__p)
                        if __p > 0
                            && __p >= self.metadata().map(|__m| __m.len()).unwrap_or(u64::MAX) =>
                    {
                        2
                    }
                    _ => 0,
                }
            }
        }
        a0.__cc2_iostate() & 5 != 0
    })
}
