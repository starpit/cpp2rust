// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// googletest's assertion interface on Rust's terms. See src.cpp for why this is
// a rule module and for the EXPECT-vs-ASSERT decision; this file is the Rust
// side of it.
//
// THE REPRESENTATION.
//
//   testing::AssertionResult          -> bool
//   testing::internal::AssertHelper   -> String (the failure message)
//   testing::Message                  -> unused (dropped)
//
// An AssertionResult in C++ carries a bool plus a lazily built message. Only the
// bool drives control flow; the message is only ever consumed by
// failure_message() on the failing path. Representing the result as a bare bool
// and the AssertHelper as the already-formatted String keeps both halves and
// needs no type that outlives the statement.
//
// WHY A THREAD-LOCAL AND NOT assert_eq!.
//
// The whole point of EXPECT_* is that it does not abort, so the translated test
// must be able to record a failure and keep going -- otherwise a test that
// reports 3 divergences in C++ reports 1 in Rust and the other 2 never run. A
// rule body is a single inlined expression with no access to the enclosing
// function, so it cannot return early or accumulate into a local. A thread-local
// is the one mechanism that lets each assertion site record independently while
// the harness reads the total at the end.
//
// `#[test] fn` then fails iff the count is non-zero, and the panic message
// carries every recorded failure -- so `cargo test` reports the same set of
// failures the gtest binary reports, not merely the first.
//
// The cell itself is NOT declared in the rule body. A `thread_local!` item inside
// the body is BLOCK-scoped, so every failure site got a fresh cell of its own and
// the harness that reads the total read one nobody had written -- every test would
// have passed regardless of its assertions, which is the worst possible direction
// to be wrong in. The body therefore calls `__cc2_gtest_record_failure`, and the
// converter emits that function and the one cell behind it exactly once
// (Converter::EmitGTestHarness).

// The one cell every failure is recorded into, and the function the rule body
// below calls. Both are at MODULE scope on purpose.
//
// f6 used to declare the `thread_local!` INSIDE its own body. A `thread_local!` is
// an item and an item inside a block is block-scoped, so each of the 157 inlined
// copies got a private cell, and the #[test] harness that reads the total read one
// that nothing had ever written: every test passed regardless of what its
// assertions actually found. That is silent wrongness of the worst kind for a
// campaign whose entire purpose is detecting divergence, so the cell is single and
// shared, and the rule body only CALLS into it.
//
// The converter emits an identical pair once into the translated output
// (Converter::EmitGTestHarness), because a rule body is inlined verbatim and
// carries no declarations with it.
thread_local! {
    pub static __CC2_GTEST_FAILS: ::std::cell::RefCell<Vec<String>> =
        const { ::std::cell::RefCell::new(Vec::new()) };
}

pub fn __cc2_gtest_record_failure(__m: String) {
    __CC2_GTEST_FAILS.with(|__f| __f.borrow_mut().push(__m));
}

// --- the types -------------------------------------------------------------

// testing::AssertionResult is the pass/fail bool it carries.
fn t1() -> bool {
    false
}

// testing::internal::AssertHelper is the already-formatted failure message.
fn t2() -> String {
    String::new()
}

// testing::Message is the streamed `<< ..` detail. None of the 296 assertions
// in the four test TUs streams anything into one, so it carries nothing.
fn t3() -> () {
    ()
}

// --- the comparison helpers ------------------------------------------------

// EXPECT_EQ / ASSERT_EQ on i32. The two stringified-expression arguments are
// dropped: the Rust harness prints real values, which is more informative.
unsafe fn f1(a0: *const libc::c_char, a1: *const libc::c_char, a2: i32, a3: i32) -> bool {
    ({
        let _ = (a0, a1);
        a2 == a3
    })
}

// EXPECT_EQ on i64.
unsafe fn f2(a0: *const libc::c_char, a1: *const libc::c_char, a2: i64, a3: i64) -> bool {
    ({
        let _ = (a0, a1);
        a2 == a3
    })
}

// EXPECT_EQ on u64 -- `.size()` comparisons in dcg_unit_test.
unsafe fn f3(a0: *const libc::c_char, a1: *const libc::c_char, a2: u64, a3: u64) -> bool {
    ({
        let _ = (a0, a1);
        a2 == a3
    })
}

// EXPECT_FLOAT_EQ. gtest's CmpHelperFloatingPointEQ is a 4-ULP comparison, NOT
// `==`, and this body reproduces that rather than tightening it to exact
// equality: EXPECT_FLOAT_EQ(attr.asFloat(), 3.14f) in operandattr_unit_test.cpp
// is only expected to hold to within 4 ULP, and an exact `==` there would report
// a failure that C++ does not report -- a false divergence, which for this
// campaign is as harmful as a missed one. NaN never compares equal and the two
// infinities compare equal to themselves, both as in gtest.
unsafe fn f4(a0: *const libc::c_char, a1: *const libc::c_char, a2: f32, a3: f32) -> bool {
    ({
        let _ = (a0, a1);
        if a2.is_nan() || a3.is_nan() {
            false
        } else if a2 == a3 {
            true
        } else {
            // Bit patterns as sign-magnitude, biased into a monotone ordering,
            // so that a subtraction counts the representable floats between
            // them. This is gtest's FloatingPoint<float>::AlmostEquals.
            let __bias = |__x: f32| -> u32 {
                let __b = __x.to_bits();
                if __b & 0x8000_0000 != 0 {
                    !__b + 1
                } else {
                    __b | 0x8000_0000
                }
            };
            let (__l, __r) = (__bias(a2), __bias(a3));
            (if __l > __r { __l - __r } else { __r - __l }) <= 4
        }
    })
}

// --- EqHelper::Compare, the remaining operand shapes -----------------------
//
// One body per shape because a generic rule cannot be written at all: see the
// SFINAE explanation in src.cpp. Each drops a0/a1 (the stringified operand
// text) for the same reason f1 does.

// EXPECT_EQ(i64, i32). WIDENS rather than truncating: C++ applies the usual
// arithmetic conversions, so `attr.asInt() == 42` compares as i64 there, and an
// `a2 as i32 == a3` here would wrap any value above i32::MAX and disagree with
// C++ on exactly the INT64_MIN/INT64_MAX cases these tests exist to check.
unsafe fn f12(a0: *const libc::c_char, a1: *const libc::c_char, a2: i64, a3: i32) -> bool {
    ({
        let _ = (a0, a1);
        a2 == a3 as i64
    })
}

// EXPECT_EQ(u32, u32).
unsafe fn f13(a0: *const libc::c_char, a1: *const libc::c_char, a2: u32, a3: u32) -> bool {
    ({
        let _ = (a0, a1);
        a2 == a3
    })
}

// EXPECT_EQ(String, String). `std::string` is `Vec<libc::c_char>` in this
// representation and both sides carry their NUL, so a plain `==` compares the
// same bytes C++ compares.
unsafe fn f14(
    a0: *const libc::c_char,
    a1: *const libc::c_char,
    a2: Vec<libc::c_char>,
    a3: Vec<libc::c_char>,
) -> bool {
    ({
        let _ = (a0, a1);
        a2 == a3
    })
}

// EXPECT_EQ(String, "literal"). The literal arrives as a pointer to a NUL-
// terminated array, so it is materialised the way rules/string f18 does --
// INCLUDING the terminator, because a2 carries one and comparing a Vec that
// ends in 0 against one that does not would report every equal string unequal.
unsafe fn f15(
    a0: *const libc::c_char,
    a1: *const libc::c_char,
    a2: Vec<libc::c_char>,
    a3: *const libc::c_char,
) -> bool {
    ({
        let _ = (a0, a1);
        a2 == {
            let __s = a3;
            ::std::slice::from_raw_parts(
                __s,
                (0..).take_while(|&__i| *__s.add(__i) != 0).count() + 1,
            )
            .to_vec()
        }
    })
}

// EXPECT_EQ(u64, u32) -- a `.size()` against an unsigned literal. Widens for
// the same reason f12 does. `u64` and NOT `usize`, matching f3's spelling of
// the same C++ `unsigned long`: they agree in size on this target, but the
// declared type is emitted as a literal cast at the call site, so `usize` here
// gave `sz == 0_u32 as usize` against a `u64` receiver -- E0308. This is the
// trap the porting playbook records as "a hand-written target signature must
// match src.cpp's spelling, not merely a type that happens to agree today".
unsafe fn f16(a0: *const libc::c_char, a1: *const libc::c_char, a2: u64, a3: u32) -> bool {
    ({
        let _ = (a0, a1);
        a2 == a3 as u64
    })
}

// --- the failure report ----------------------------------------------------

// AssertHelper's constructor: builds the message. `a0` is the
// TestPartResult::Type -- 0 = success, 1 = non-fatal (EXPECT_*), 2 = fatal
// (ASSERT_*) -- and is recorded so a reader can tell which kind of assertion
// failed. It does NOT change what this body does, and must not: the difference
// between the two IS the early return, which is C++ control flow the converter
// translates on its own, not something a rule may decide.
unsafe fn f5(a0: i32, a1: *const libc::c_char, a2: i32, a3: *const libc::c_char) -> String {
    ({
        let __cstr = |__p: *const libc::c_char| -> String {
            if __p.is_null() {
                String::new()
            } else {
                ::std::ffi::CStr::from_ptr(__p).to_string_lossy().into_owned()
            }
        };
        format!(
            "{}:{}: {} failure\n{}",
            __cstr(a1),
            a2,
            if a0 >= 2 { "Fatal" } else { "Non-fatal" },
            __cstr(a3)
        )
    })
}

// AssertHelper::operator= -- this is what actually files the failure. Records
// and RETURNS, so a non-fatal assertion falls through to the next statement
// exactly as EXPECT_* does in C++.
unsafe fn f6(a0: String, a1: ()) {
    ({
        let _ = a1;
        __cc2_gtest_record_failure(a0);
    })
}

// --- AssertionResult -------------------------------------------------------

// EXPECT_TRUE / EXPECT_FALSE: AssertionResult straight from a bool.
//
// a1 is gtest's unused `void *` overload disambiguator and is ALWAYS defaulted at
// the call site, so the body must not mention it. This used to say `let _ = a1;`,
// and because a defaulted pointer argument is emitted as a bare
// `Default::default()` with no type to infer from, every EXPECT_TRUE/EXPECT_FALSE
// site became E0790 "cannot call associated function on trait" -- two of the
// playbook's recorded traps at once ("a rule body must not touch a receiver it
// does not need", "a defaulted pointer arg arrives as Default::default() with no
// type"). Not naming it means nothing is emitted for it.
#[allow(unused_variables)]
unsafe fn f7(a0: bool, a1: Option<()>) -> bool {
    a0
}

// `if (gtest_ar)`.
unsafe fn f8(a0: bool) -> bool {
    a0
}

// gtest_ar.failure_message() on a bool-represented result: the values are
// printed by the harness, so there is no lazily built text to hand back.
unsafe fn f9(a0: bool) -> *const libc::c_char {
    ({
        let _ = a0;
        c"".as_ptr()
    })
}

// testing::Message() -- the unit value, matching t3.
unsafe fn f11() -> () {
    ()
}

// GetBoolAssertionFailureMessage(result, expr_text, actual, expected). Unlike
// f9 this one HAS the useful text -- gtest passes the stringified expression and
// the two boolean spellings -- so it is reassembled instead of dropped.
unsafe fn f10(
    a0: bool,
    a1: *const libc::c_char,
    a2: *const libc::c_char,
    a3: *const libc::c_char,
) -> Vec<libc::c_char> {
    ({
        let _ = a0;
        let __cstr = |__p: *const libc::c_char| -> String {
            if __p.is_null() {
                String::new()
            } else {
                ::std::ffi::CStr::from_ptr(__p).to_string_lossy().into_owned()
            }
        };
        let __s = format!(
            "Value of: {}\n  Actual: {}\nExpected: {}",
            __cstr(a1),
            __cstr(a2),
            __cstr(a3)
        );
        let mut __v: Vec<libc::c_char> = __s.bytes().map(|__b| __b as libc::c_char).collect();
        __v.push(0);
        __v
    })
}
