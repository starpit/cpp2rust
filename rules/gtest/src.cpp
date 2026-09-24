// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// googletest's assertion interface, lowered onto Rust's own test harness.
//
// WHY THIS IS A RULE MODULE AND NOT MACRO HANDLING IN THE CONVERTER.
//
// EXPECT_EQ / ASSERT_TRUE / ... are preprocessor macros, so by the time clang
// builds an AST they no longer exist: there is nothing named `EXPECT_EQ` for
// the converter to match on. What survives is an ordinary C++ shape, and it is
// the same shape for every assertion macro in the suite (gtest_pred_impl.h:77):
//
//     if (const ::testing::AssertionResult gtest_ar = (expression))
//       ;
//     else
//       on_failure(gtest_ar.failure_message())
//
// So the only thing a translator can key on is the FUNCTION CALLS inside that
// shape -- `testing::internal::EqHelper::Compare`, the `AssertionResult(bool)`
// constructor, `AssertionResult::operator bool()`, and the `AssertHelper`
// construction plus its `operator=` that reports a failure. Those are exactly
// what a translation rule keys on, and the file below spells one per call.
// Writing this as converter code would mean re-deriving from AST shape what the
// rule mechanism already matches by signature, in the one place in the codebase
// that is NOT where translation decisions live.
//
// The opposite half of the job -- not translating gtest's own internals
// (testing::internal::RE, MutexBase, pthread_mutex_t, GTestLog) -- is already
// solved by `--opaque-namespace=testing`, and that is a configuration choice,
// not a rule. Measured on these four TUs: it removes all 8 surveyed gaps (all 8
// were inside gtest headers, none in dt_src) and ~1,000 lines per TU of
// translated framework. These rules and that flag are designed to be used
// together: the flag makes the framework a boundary, and these rules give the
// handful of boundary calls that CARRY THE TEST'S MEANING a real Rust meaning
// instead of an undefined opaque handle.
//
// EXPECT vs ASSERT -- THE SEMANTIC DECISION, STATED EXPLICITLY.
//
// ASSERT_* aborts the current test; EXPECT_* records a failure and CONTINUES,
// so one C++ test can report several failures. `assert_eq!` aborts. Mapping
// EXPECT_* onto `assert_eq!` would therefore CHANGE SEMANTICS: a test that
// reports 3 failures in C++ would report 1 in Rust, and the 2nd and 3rd
// assertions would never run -- which also means any divergence they would have
// exposed is hidden. For a port whose entire purpose is finding divergences,
// that is the worst possible direction to be wrong in.
//
// This module does NOT conflate them, and does not have to, because the
// distinction is not lost at macro expansion: it survives as ordinary C++
// control flow that cpp2rust already translates correctly.
//
//   * `AssertHelper(kNonFatalFailure, ..)` vs `(kFatalFailure, ..)` is a plain
//     enum argument, visible in the AST (`f5` takes it as a parameter);
//   * ASSERT_* additionally expands to `return;` after the failure report,
//     which is a real ReturnStmt in the body.
//
// So the failure REPORT is what these rules translate, and the control flow --
// continue vs return -- comes free from the surrounding C++ that the converter
// already handles. The Rust side accumulates failures in a thread-local and
// panics at the end of the test (see tgt_unsafe.rs `__cc2_gtest`), which
// reproduces BOTH behaviours exactly: a non-fatal failure records and falls
// through, a fatal one records and the translated `return` leaves the function.
// The count of failures a test reports is therefore preserved, not truncated.
//
// WHAT IS SUPPORTED, AND WHY EXACTLY THIS SET.
//
// Driven by what the four real test TUs use, counted rather than guessed
// (dsc/test/{dsc_unit_test,dsc_pcfg_unit_test,operandattr_unit_test}.cpp and
// dcg/test/dcg_unit_test.cpp):
//
//     186  EXPECT_EQ
//      54  EXPECT_TRUE
//      38  EXPECT_FALSE
//      10  EXPECT_GE
//       8  EXPECT_FLOAT_EQ
//       0  ASSERT_* of any kind
//
// 296 assertions, all non-fatal. EXPECT_EQ/GE go through EqHelper::Compare and
// the CmpHelper** family; EXPECT_TRUE/FALSE construct an AssertionResult from a
// bool. EXPECT_FLOAT_EQ goes through CmpHelperFloatingPointEQ<float>, which is
// gtest's 4-ULP comparison and NOT `==` -- spelled as its own rule (`f4`) so it
// keeps that meaning rather than silently becoming exact equality.
//
// No TEST_P, no TYPED_TEST, no INSTANTIATE_TEST_SUITE_P, no EXPECT_THROW, no
// death tests, no matchers: none appear in these four files. That is deliberate
// per the brief -- support what is there rather than build a gtest emulator.
// A construct outside this set does not silently misbehave: its calls find no
// rule and, under --opaque-namespace=testing, surface as an undefined opaque
// handle that rustc names.

// The googletest headers are VENDORED in this directory, beside this file, and
// included by a relative quoted path so that resolving them needs no include
// flag and no gtest installed on the build machine.
//
// This follows what rules/brotli and rules/rustls already do for third-party
// headers: brotli keeps brotli/{decode,encode,port,types}.h in its own rule
// directory and says `#include "brotli/decode.h"`. A `cxxflags` file with an
// absolute `-I` would be the wrong mechanism here -- every other cxxflags file
// in the tree holds only portable flags (rules/compare's is `-std=c++20`), an
// absolute path pointing into one machine's toolchain cannot resolve on CI or on
// the maintainer's box, and this repo has already had one cleanup removing
// exactly that class of hardcoded path. Vendoring is also what makes the rule
// signatures REPRODUCIBLE: these rules key on gtest's exact resolved
// signatures, so the headers that resolve them must be pinned rather than
// whatever version happens to be installed.
//
// Vendored from googletest 1.15.2, googletest/include/gtest (23 headers, 576K).
#include "gtest/gtest.h"

// ---------------------------------------------------------------------------
// the types
//
// A rule module has to map these three itself: `--opaque-namespace=testing`
// would otherwise make each an opaque handle -- a distinct undefined Rust type
// that no rule body could accept -- and the declared type of `gtest_ar` would
// not match the bool the comparison helpers return.
//
//   t1  testing::AssertionResult        -> bool   (the pass/fail it carries)
//   t2  testing::internal::AssertHelper -> String (the formatted failure)
//   t3  testing::Message                -> ()     (streamed detail, unused)
// ---------------------------------------------------------------------------

using t1 = testing::AssertionResult;
using t2 = testing::internal::AssertHelper;
using t3 = testing::Message;

// ---------------------------------------------------------------------------
// the comparison helpers: EXPECT_EQ / EXPECT_NE / EXPECT_LT / EXPECT_GE / ...
//
// Each returns a testing::AssertionResult, which these rules represent as a
// plain Rust bool (see tgt_unsafe.rs). The two leading `const char *` are the
// stringified operand expressions, used only for gtest's failure text; the Rust
// harness prints the values instead, which is strictly more informative and is
// why those two parameters are dropped in the target.
// ---------------------------------------------------------------------------

// EXPECT_EQ / ASSERT_EQ on integers.
testing::AssertionResult f1(const char *a0, const char *a1, const int &a2,
                            const int &a3) {
  return testing::internal::EqHelper::Compare(a0, a1, a2, a3);
}

// EXPECT_EQ on 64-bit integers -- dcg_unit_test compares .size() results, and
// operandattr_unit_test compares int64_t values including INT64_MIN/MAX.
testing::AssertionResult f2(const char *a0, const char *a1, const long &a2,
                            const long &a3) {
  return testing::internal::EqHelper::Compare(a0, a1, a2, a3);
}

testing::AssertionResult f3(const char *a0, const char *a1,
                            const unsigned long &a2, const unsigned long &a3) {
  return testing::internal::EqHelper::Compare(a0, a1, a2, a3);
}

// EXPECT_FLOAT_EQ. gtest compares floats within 4 ULP, not exactly; the target
// body reproduces that rather than emitting `==`.
testing::AssertionResult f4(const char *a0, const char *a1, float a2,
                            float a3) {
  return testing::internal::CmpHelperFloatingPointEQ<float>(a0, a1, a2, a3);
}

// ---------------------------------------------------------------------------
// the failure report
//
// `AssertHelper(type, file, line, message) = Message()` is how both EXPECT_ and
// ASSERT_ record a failure. `type` distinguishes them and is passed through, so
// the Rust side records a fatal and a non-fatal failure differently rather than
// treating every failure as an abort.
// ---------------------------------------------------------------------------

// The report is spelled in C++ as two calls: constructing an AssertHelper that
// carries where and what failed, then `operator=` on it, which is what actually
// files the failure with the running test. Both need a rule.
//
// f5 is the CONSTRUCTOR. It cannot do the reporting itself: in C++ the object is
// a temporary whose `operator=` does the work, and a rule body is inlined at the
// call site, so a constructor body that reported would report even where the
// result is discarded. It therefore builds the failure message -- location plus
// gtest's own text -- and hands it on as a String, which is what the Rust
// representation of an AssertHelper is (see tgt_unsafe.rs).
testing::internal::AssertHelper f5(testing::TestPartResult::Type a0,
                                   const char *a1, int a2, const char *a3) {
  return testing::internal::AssertHelper(a0, a1, a2, a3);
}

// f6 is the `operator=` that files it. `a0` is the message built by f5; the
// Message operand carries the streamed `<< ..` detail, which none of the 296
// assertions in these four files uses, so it is dropped.
void f6(const testing::internal::AssertHelper &a0, const testing::Message &a1) {
  return a0.operator=(a1);
}

// ---------------------------------------------------------------------------
// AssertionResult itself
// ---------------------------------------------------------------------------

// EXPECT_TRUE / EXPECT_FALSE build an AssertionResult straight from a bool.
testing::AssertionResult f7(const bool &a0, void *a1) {
  return testing::AssertionResult(a0, a1);
}

// The `if (gtest_ar)` test in the macro expansion.
bool f8(const testing::AssertionResult &a0) { return a0.operator bool(); }

// gtest_ar.failure_message(), the argument to the failure report above.
const char *f9(const testing::AssertionResult &a0) {
  return a0.failure_message();
}

// EXPECT_TRUE/FALSE's failure text helper. Returns a std::string in C++; the
// Rust harness prints its own message, so this is the boolean-assertion
// counterpart of f8.
std::string f10(const testing::AssertionResult &a0, const char *a1,
               const char *a2, const char *a3) {
  return testing::internal::GetBoolAssertionFailureMessage(a0, a1, a2, a3);
}

// testing::Message's default constructor. Every assertion macro builds one --
// `AssertHelper(..) = Message()` -- so without a rule the flattened name
// `testing_Message::testing_Message1()` is undefined in 156 places in a single
// translated TU. It carries the streamed `<< ..` detail, which none of the 296
// assertions in these four files uses, so it constructs nothing.
testing::Message f11() { return testing::Message(); }
