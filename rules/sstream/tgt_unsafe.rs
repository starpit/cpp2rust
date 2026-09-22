// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1() -> Box<Vec<u8>> {
    Box::new(Vec::new())
}

fn t2() -> Box<Vec<u8>> {
    Box::new(Vec::new())
}

fn t3() -> Box<Vec<u8>> {
    Box::new(Vec::new())
}

fn t4() -> Box<Vec<u8>> {
    Box::new(Vec::new())
}

fn t5() -> Box<Vec<u8>> {
    Box::new(Vec::new())
}

unsafe fn f1() -> Box<Vec<u8>> {
    Box::new(Vec::new())
}

unsafe fn f2() -> Box<Vec<u8>> {
    Box::new(Vec::new())
}

unsafe fn f3() -> Box<Vec<u8>> {
    Box::new(Vec::new())
}

unsafe fn f4(a0: Vec<libc::c_char>) -> Box<Vec<u8>> {
    Box::new(
        a0.iter()
            .take_while(|&&c| c != 0)
            .map(|&c| c as u8)
            .collect(),
    )
}

unsafe fn f5(a0: Vec<libc::c_char>) -> Box<Vec<u8>> {
    Box::new(
        a0.iter()
            .take_while(|&&c| c != 0)
            .map(|&c| c as u8)
            .collect(),
    )
}

unsafe fn f6(a0: Vec<libc::c_char>) -> Box<Vec<u8>> {
    Box::new(
        a0.iter()
            .take_while(|&&c| c != 0)
            .map(|&c| c as u8)
            .collect(),
    )
}

unsafe fn f7(a0: Box<Vec<u8>>) -> Vec<libc::c_char> {
    let mut __s: Vec<libc::c_char> = a0.iter().map(|&b| b as libc::c_char).collect();
    __s.push(0);
    __s
}

unsafe fn f8(a0: Box<Vec<u8>>) -> Vec<libc::c_char> {
    let mut __s: Vec<libc::c_char> = a0.iter().map(|&b| b as libc::c_char).collect();
    __s.push(0);
    __s
}

unsafe fn f9(a0: Box<Vec<u8>>) -> Vec<libc::c_char> {
    let mut __s: Vec<libc::c_char> = a0.iter().map(|&b| b as libc::c_char).collect();
    __s.push(0);
    __s
}

unsafe fn f10(a0: &mut Box<Vec<u8>>, a1: Vec<libc::c_char>) {
    a0.clear();
    a0.extend(a1.iter().take_while(|&&c| c != 0).map(|&c| c as u8));
}

unsafe fn f11(a0: &mut Box<Vec<u8>>, a1: Vec<libc::c_char>) {
    a0.clear();
    a0.extend(a1.iter().take_while(|&&c| c != 0).map(|&c| c as u8));
}

unsafe fn f12(a0: &mut Box<Vec<u8>>, a1: Vec<libc::c_char>) {
    a0.clear();
    a0.extend(a1.iter().take_while(|&&c| c != 0).map(|&c| c as u8));
}

unsafe fn f19(a0: &mut Box<Vec<u8>>, a1: &mut i32) {
    let __tok: String = {
        let mut __i = 0usize;
        while __i < a0.len() && a0[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < a0.len() && (a0[__i] == b'-' || a0[__i] == b'+') {
            __i += 1;
        }
        while __i < a0.len() && (a0[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&a0[__b..__i]).into_owned();
        a0.drain(..__i);
        __t
    };
    *a1 = __tok.parse::<i32>().unwrap_or(Default::default());
}

unsafe fn f20(a0: &mut Box<Vec<u8>>, a1: &mut u32) {
    let __tok: String = {
        let mut __i = 0usize;
        while __i < a0.len() && a0[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < a0.len() && (a0[__i] == b'-' || a0[__i] == b'+') {
            __i += 1;
        }
        while __i < a0.len() && (a0[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&a0[__b..__i]).into_owned();
        a0.drain(..__i);
        __t
    };
    *a1 = __tok.parse::<u32>().unwrap_or(Default::default());
}

unsafe fn f21(a0: &mut Box<Vec<u8>>, a1: &mut i64) {
    let __tok: String = {
        let mut __i = 0usize;
        while __i < a0.len() && a0[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < a0.len() && (a0[__i] == b'-' || a0[__i] == b'+') {
            __i += 1;
        }
        while __i < a0.len() && (a0[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&a0[__b..__i]).into_owned();
        a0.drain(..__i);
        __t
    };
    *a1 = __tok.parse::<i64>().unwrap_or(Default::default());
}

unsafe fn f22(a0: &mut Box<Vec<u8>>, a1: &mut u64) {
    let __tok: String = {
        let mut __i = 0usize;
        while __i < a0.len() && a0[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < a0.len() && (a0[__i] == b'-' || a0[__i] == b'+') {
            __i += 1;
        }
        while __i < a0.len() && (a0[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&a0[__b..__i]).into_owned();
        a0.drain(..__i);
        __t
    };
    *a1 = __tok.parse::<u64>().unwrap_or(Default::default());
}

unsafe fn f23(a0: &mut Box<Vec<u8>>, a1: &mut i64) {
    let __tok: String = {
        let mut __i = 0usize;
        while __i < a0.len() && a0[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < a0.len() && (a0[__i] == b'-' || a0[__i] == b'+') {
            __i += 1;
        }
        while __i < a0.len() && (a0[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&a0[__b..__i]).into_owned();
        a0.drain(..__i);
        __t
    };
    *a1 = __tok.parse::<i64>().unwrap_or(Default::default());
}

unsafe fn f24(a0: &mut Box<Vec<u8>>, a1: &mut u64) {
    let __tok: String = {
        let mut __i = 0usize;
        while __i < a0.len() && a0[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < a0.len() && (a0[__i] == b'-' || a0[__i] == b'+') {
            __i += 1;
        }
        while __i < a0.len() && (a0[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&a0[__b..__i]).into_owned();
        a0.drain(..__i);
        __t
    };
    *a1 = __tok.parse::<u64>().unwrap_or(Default::default());
}

unsafe fn f25(a0: &mut Box<Vec<u8>>, a1: &mut i16) {
    let __tok: String = {
        let mut __i = 0usize;
        while __i < a0.len() && a0[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < a0.len() && (a0[__i] == b'-' || a0[__i] == b'+') {
            __i += 1;
        }
        while __i < a0.len() && (a0[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&a0[__b..__i]).into_owned();
        a0.drain(..__i);
        __t
    };
    *a1 = __tok.parse::<i16>().unwrap_or(Default::default());
}

unsafe fn f26(a0: &mut Box<Vec<u8>>, a1: &mut u16) {
    let __tok: String = {
        let mut __i = 0usize;
        while __i < a0.len() && a0[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < a0.len() && (a0[__i] == b'-' || a0[__i] == b'+') {
            __i += 1;
        }
        while __i < a0.len() && (a0[__i].is_ascii_digit()) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&a0[__b..__i]).into_owned();
        a0.drain(..__i);
        __t
    };
    *a1 = __tok.parse::<u16>().unwrap_or(Default::default());
}

unsafe fn f27(a0: &mut Box<Vec<u8>>, a1: &mut f32) {
    let __tok: String = {
        let mut __i = 0usize;
        while __i < a0.len() && a0[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < a0.len() && (a0[__i] == b'-' || a0[__i] == b'+') {
            __i += 1;
        }
        while __i < a0.len() && (a0[__i].is_ascii_digit()
                || a0[__i] == b'.'
                || a0[__i] == b'e'
                || a0[__i] == b'E'
                || ((a0[__i] == b'-' || a0[__i] == b'+')
                    && (a0[__i - 1] == b'e' || a0[__i - 1] == b'E'))) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&a0[__b..__i]).into_owned();
        a0.drain(..__i);
        __t
    };
    *a1 = __tok.parse::<f32>().unwrap_or(Default::default());
}

unsafe fn f28(a0: &mut Box<Vec<u8>>, a1: &mut f64) {
    let __tok: String = {
        let mut __i = 0usize;
        while __i < a0.len() && a0[__i].is_ascii_whitespace() {
            __i += 1;
        }
        let __b = __i;
        if __i < a0.len() && (a0[__i] == b'-' || a0[__i] == b'+') {
            __i += 1;
        }
        while __i < a0.len() && (a0[__i].is_ascii_digit()
                || a0[__i] == b'.'
                || a0[__i] == b'e'
                || a0[__i] == b'E'
                || ((a0[__i] == b'-' || a0[__i] == b'+')
                    && (a0[__i - 1] == b'e' || a0[__i - 1] == b'E'))) {
            __i += 1;
        }
        let __t = String::from_utf8_lossy(&a0[__b..__i]).into_owned();
        a0.drain(..__i);
        __t
    };
    *a1 = __tok.parse::<f64>().unwrap_or(Default::default());
}

unsafe fn f29(a0: &mut Box<Vec<u8>>, a1: &mut Vec<libc::c_char>) {
    ({
        trait __Cc2Tok {
            fn __cc2_token(&mut self) -> Vec<u8>;
        }
        impl __Cc2Tok for Vec<u8> {
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
        let __tok: Vec<u8> = a0.__cc2_token();
        let mut __out: Vec<libc::c_char> = __tok.iter().map(|&b| b as libc::c_char).collect();
        __out.push(0);
        *a1 = __out;
    })
}

unsafe fn f30(a0: &mut Box<Vec<u8>>, a1: &mut Vec<libc::c_char>, a2: libc::c_char) {
    ({
        trait __Cc2Line {
            fn __cc2_getline(&mut self, __d: u8) -> Vec<u8>;
        }
        impl __Cc2Line for Vec<u8> {
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
        let __line: Vec<u8> = a0.__cc2_getline(a2 as u8);
        let mut __out: Vec<libc::c_char> = __line.iter().map(|&b| b as libc::c_char).collect();
        __out.push(0);
        *a1 = __out;
    })
}

unsafe fn f31(a0: &mut Box<Vec<u8>>, a1: &mut Vec<libc::c_char>) {
    let __line: Vec<u8> = {
        let __pos = a0.iter().position(|&c| c == b'\n');
        let __keep = __pos.unwrap_or(a0.len());
        let __end = __pos.map(|__p| __p + 1).unwrap_or(a0.len());
        a0.drain(..__end).take(__keep).collect()
    };
    let mut __out: Vec<libc::c_char> = __line.iter().map(|&b| b as libc::c_char).collect();
    __out.push(0);
    *a1 = __out;
}

unsafe fn f32(a0: &mut Box<Vec<u8>>, a1: &mut libc::c_char) {
    ({
        trait __Cc2Ch {
            fn __cc2_char(&mut self) -> u8;
        }
        impl __Cc2Ch for Vec<u8> {
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
        *a1 = a0.__cc2_char() as libc::c_char;
    })
}
