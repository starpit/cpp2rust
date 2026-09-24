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
// Each body repeats the declaration because rule bodies are inlined
// independently; the items are identical in every body and `__CC2_GTEST_FAILS`
// is one thread-local however many copies of the declaration the output holds.

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
        thread_local! {
            pub static __CC2_GTEST_FAILS: ::std::cell::RefCell<Vec<String>> =
                const { ::std::cell::RefCell::new(Vec::new()) };
        }
        let _ = a1;
        __CC2_GTEST_FAILS.with(|__f| __f.borrow_mut().push(a0));
    })
}

// --- AssertionResult -------------------------------------------------------

// EXPECT_TRUE / EXPECT_FALSE: AssertionResult straight from a bool.
unsafe fn f7(a0: bool, a1: Option<()>) -> bool {
    ({
        let _ = a1;
        a0
    })
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
