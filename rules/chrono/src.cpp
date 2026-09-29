// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// <chrono>.  TYPE RULES ONLY, and deliberately only the two time_point clocks.
//
// WHAT IS DELIBERATELY LEFT OUT, AND WHY (leaving it out makes it fail LOUDLY at
// translate time, which is the required outcome for anything that cannot be made
// unit-correct):
//
//  * `std::chrono::duration<long long, std::ratio<_, _>>` -- the recorder prints
//    the ratio's NON-TYPE template arguments as `_, _`, as the queue rows
//    g2838..g2843 show verbatim.  So nanoseconds (`ratio<1,1000000000>`),
//    microseconds (`ratio<1,1000000>`) and milliseconds (`ratio<1,1000>`) all
//    collapse onto ONE key.  A single `Duration` mapping would therefore serve a
//    nanosecond duration and a millisecond duration with the same body -- off by
//    10^6 and STILL COMPILING.  That is the collapsed-template-argument trap in
//    its ratio flavour, and silent wrongness is the worst outcome available, so
//    the key is NOT written.
//    ⛔ THE COLLAPSE IS STRUCTURAL IN THE CONVERTER, NOT COSMETIC IN THE ROW
//    TEXT, so it cannot be worked around from here.  MEASURED 2026-09-28:
//    `Mapper::ToString` (cpp2rust/converter/mapper.cpp:2223) ends every type
//    spelling -- the RULES KEY side and the SEARCH side alike, it is one
//    function -- with `normalizeTranslationRule` (:1830), whose sole rewrite is
//        {std::regex(R"(\b\d+\b)"), "_"}   // "Ignore constant template parameters"
//    i.e. EVERY integer literal becomes `_`.  mapper.cpp:1641 says so in the
//    tree's own words: "normalizeTranslationRule ... rewrites `\b\d+\b` -> `_`
//    when it builds a rules KEY".  Therefore there is NO C++ spelling I can
//    write here whose recorded key is anything but the one collapsed string:
//        nanoseconds = duration<long long, ratio<1, 1000000000>>  -> ratio<_, _>
//        seconds     = duration<long long, ratio<1, 1>>           -> ratio<_, _>
//    are the SAME KEY.  ⭐ AND THIS IS PROVEN BY READBACK, not just by reading
//    the converter: t1 and t2 are WRITTEN below as `std::ratio<1, 1000000000>`,
//    and `ir/<tree>/chrono/ir_src.json` records them as
//        std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<long long, std::ratio<_, _>>>
//        std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<long long, std::ratio<_, _>>>
//    -- the digits I typed are GONE from the recorded key.  So the erasure is on
//    the RULE side too, and a duration key demonstrably cannot carry its Period.
//    ⚠️⚠️ THE READBACK ABOVE IS DATED 2026-09-28 AND IT NO LONGER REPRODUCES.
//    RE-MEASURED 2026-09-29 against snap/coord44/cpp-rule-preprocessor
//    md5 a46d45dc97f80a5ba8ee07b3eb5b6a96, regenerating THIS FILE UNCHANGED into a
//    fresh clone of pin/ir.v40.  `chrono/ir_src.json` now reads back
//        t1 = std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<long long, std::ratio<1, 1000000000>>>
//        t2 = std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<long long, std::ratio<1, 1000000000>>>
//        t3 = std::chrono::time_point<std::chrono::system_clock>
//    -- THE DIGITS ARE PRESENT.  So the `\b\d+\b` -> `_` erasure described above is
//    NOT happening on the rules side at coord44, and the `ratio<_, _>` collapse this
//    refusal's FIRST premise rests on is, on that side, STALE.
//    ⛔ THAT IS NOT PERMISSION TO KEY `duration`, AND THE REFUSAL STANDS.  Three
//    things are still unmeasured, and every one of them is independently fatal if it
//    goes the wrong way:
//      (a) the SEARCH side.  Only a rules-side readback was re-run.  Until a ns key
//          is shown to match ONLY ns sites -- and a ms key ONLY ms sites -- in a real
//          translation, the collapse may simply have moved rather than gone, and a
//          per-unit key set would tie and resolve arbitrarily.
//      (b) `duration_cast<Unit>(d)` and `.count()` are refused on a SECOND,
//          independent ground: their only distinguishing argument is an EXPLICIT
//          template argument, which no module in this tree records (no module carries
//          the `explicit-template-args` marker).  Digit preservation does not touch
//          that.
//      (c) the loud-path loss.  Keying `duration` as a TYPE while `.count()`,
//          `duration_cast` and `operator-=` stay unkeyed makes those three emit
//          TEXTUALLY instead of aborting -- see the g165 note below -- which is
//          strictly worse than the current state.
//    ⭐ So the correct next step is a measurement row of its own: re-run the ratio
//    collapse experiment on BOTH sides at coord44 and, only if per-unit keys are
//    shown to discriminate, revisit this file's first premise TOGETHER with the
//    method keys that keep the path loud.  Do NOT key `duration` before then.
//    (This is also why t1/t2 are safe: an Instant/SystemTime has no unit to get
//    wrong, so for them the erasure is harmless rather than fatal.)
//    Contrast the t2/t3 default-suppression split below,
//    which WAS a printing difference and so was fixable by writing both
//    spellings; this one is not, because both sides normalize identically.
//    ⭐ THE OBSERVER, and it is decisive rather than hypothetical --
//    `external/g3log/time.cpp:43-48`, `g3::internal::to_string`, the reach site
//    behind the g284x `g3::internal...` rows:
//        auto duration     = ts.time_since_epoch();                        // ns
//        auto sec_duration = duration_cast<seconds>(duration);             // s
//        duration -= sec_duration;                                         // g818
//        auto ns = duration_cast<nanoseconds>(duration).count();           // ns
//    THREE different Period values inside FOUR lines, all three landing on the
//    one key -- and line 3 MIXES them: C++ resolves `ns -= s` through
//    common_type and subtracts 1'000'000'000 ns per second.  A single collapsed
//    body cannot know that factor, so it would subtract 1, i.e. be wrong by 10^9
//    while compiling.  And the error is immediately OBSERVED as text: this
//    function exists only to format the sub-second fraction of a timestamp
//    ("1 ms --> 001"), so the corruption is printed in the fraction field of
//    EVERY g3log line.  That is the whole refusal in one call site: the same key
//    must serve seconds and nanoseconds in the same expression.
//    The fix belongs in the converter's key normalization (carry non-type
//    template arguments instead of erasing them), NOT in rules/chrono.
//  * `duration_cast<Unit>(d)` -- its only distinguishing argument is an explicit
//    template argument, so it records as ONE key for milliseconds / microseconds /
//    seconds / nanoseconds alike.  Same refusal, same reason.
//  * `.count()` -- unit-correctness is impossible without the ratio, see above.
//  * QUEUE g148, `operator-(time_point<steady_clock,_>, time_point<steady_clock,_>)`
//    -- REFUSED on SIGN, not on units.  `Instant - Instant` in Rust yields an
//    UNSIGNED `std::time::Duration` and PANICS when the right operand is later
//    than the left, while C++ `a - b` on two time_points is well-defined and
//    NEGATIVE there.  The corpus's own dominant call site is exactly that
//    direction: g3log/g3log/time.hpp:75 computes `sys_now + (ts - hrs_now)`,
//    where `hrs_now` is a function-local `static` initialised on the FIRST call
//    to `to_system_time()` and `ts` is the timestamp of the message being
//    formatted -- so `ts` precedes `hrs_now` for every message logged before
//    that first call, and the header's own comment says the subtraction exists
//    to carry a SIGNED relative offset.  9 of this row's 11 recorded sites are
//    that one line.  A naive `a0 - a1` therefore converts a defined negative
//    duration into a PANIC, and `saturating_duration_since` converts it into a
//    silent clamp to zero -- the worse of the two.  No sign-preserving mapping
//    is available, because the only Rust type that could carry the sign is the
//    duration type, which is unkeyable for the ratio reason above.  So the key
//    is LEFT OUT and the row fails loudly at translate time.
//  * QUEUE g165, `operator+(time_point<system_clock>, duration<long long, ratio<_,_>>)`
//    -- REFUSED: UNREACHABLE.  Its second operand IS the unkeyable duration, so
//    there is no Rust type to declare for it, and its value at the sole call
//    site is the negative duration from g148.  Both reasons are independently
//    fatal.
//    ⭐⭐ IS `+` SEPARABLE FROM `-`?  NO -- AND THIS IS NOW MEASURED, not inferred.
//    2026-09-29, pin/ir.v40 (96 modules) + snap/coord44/cpp2rust
//    md5 a1bc90153318514a178849ebd27944fa.  `+` is a fair question to ask, because
//    unlike `-` it is TOTAL in both languages: `time_point + duration` cannot
//    overflow into a sign problem, so the g148 refusal does NOT cover it on its own
//    reasoning.  It is refused for two INDEPENDENT measured reasons.
//
//    (1) THE TWO OPERATORS ARE THE SAME EXPRESSION.  A COMPLETE `--survey` run
//    (rc=0, not truncated) over `common/logging.cpp` -- the ONLY TU in the corpus
//    that reaches this row; measured 1, not the 4 a bucket census had estimated --
//    reports EXACTLY TWO chrono gaps in the whole TU, and their recorded contexts are
//        operator+   @ external/g3log/g3log/time.hpp:75:60   count=1 distinct=1
//        operator-   @ external/g3log/g3log/time.hpp:75:66   count=1 distinct=1
//    i.e. the SAME LINE, six columns apart.  Line 75 is
//        return time_point_cast<system_clock::duration>(sys_now + (ts - hrs_now));
//    so the `+`'s right operand IS `(ts - hrs_now)`, the result of the refused `-`.
//    ⭐ Therefore landing `+` alone closes NOTHING: the TU would abort six columns
//    later on the `-`, which is correctly refused.  There is no site where `+`
//    appears without `-`, so there is no separable half to land.
//
//    (2) LANDING `+` REQUIRES KEYING `duration`, WHICH IS THE SIDE DOOR.  The key
//    the converter asks for is, verbatim from the abort,
//        std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<long long, std::ratio<1, 1000000000>>>
//        std::chrono::operator+(const std::chrono::time_point<std::chrono::system_clock> &,
//                               const std::chrono::duration<long long, std::ratio<1, 1000000000>> &)
//    -- the SECOND PARAMETER is a `duration`, so writing this rule at all forces a
//    Rust type for `duration`, i.e. the very type key refused at the top of this
//    file for the ratio-collapse reason.  And a `duration` type key is worse than
//    just re-opening g818: an UNMAPPED MEMBER DOES NOT ABORT -- the converter emits
//    the call TEXTUALLY -- so once `duration` has a type, the three constructs that
//    today fail LOUDLY (`.count()`, `duration_cast<Unit>()`, `operator-=`) stop
//    failing and start emitting, WITHOUT the `Cpp2RustUnmapped` marker the
//    placeholder census greps for.  That converts three loud gates into three
//    silent ones at the g3log `to_string` site whose corruption is printed in the
//    fraction field of every log line.  A type key with no method key is strictly
//    worse than no key at all, and that is exactly what this would be.
//
//    ⛔ SO: `+` stays out.  The refusal was written against `-`, `+` was re-examined
//    on its own merits, and it fails for reasons of its own.  The fix is still the
//    converter's key normalization (carry non-type template arguments instead of
//    erasing them); until then nothing in this family is separable.
//  * QUEUE g818, `duration::operator-=(const duration&)` -- REFUSED for the ratio
//    reason: receiver and operand are both the collapsed duration key, so one
//    body would serve a nanosecond and a millisecond duration alike.
//
// WHAT IS IN, and why it IS unit-safe: a `time_point` maps to a Rust instant type
// that carries NO unit at all, so the erased ratio in its second (DEFAULTED)
// template argument cannot make the mapping wrong.
//  * steady_clock is MONOTONIC       -> std::time::Instant
//  * system_clock  is WALL-CLOCK     -> std::time::SystemTime
// These are DIFFERENT Rust types and must never share a key: an Instant has no
// epoch and a SystemTime is not monotonic.
//
// The second template argument of both time_points is DEFAULTED in the source
// (`steady_clock::now()` returns `time_point<steady_clock, steady_clock::duration>`),
// but the recorder writes the FULLY RESOLVED spelling, so it is spelled out here.

#include <chrono>

using t1 = std::chrono::time_point<std::chrono::steady_clock,
                                   std::chrono::duration<long long, std::ratio<1, 1000000000>>>;
using t2 = std::chrono::time_point<std::chrono::system_clock,
                                   std::chrono::duration<long long, std::ratio<1, 1000000000>>>;

// t3 IS NOT A DUPLICATE OF t2, AND WITHOUT IT t2 IS DEAD.  MEASURED 2026-09-28:
// every one of the NINE system_clock queue rows (g178, g2846..g2853) records its
// converter-side search as the DEFAULT-SUPPRESSED spelling
//     std::chrono::time_point<std::chrono::system_clock>
// with no second template argument, because the search side prints with
// SuppressDefaultTemplateArgs.  t2's fully-spelled key cannot match that string,
// so t2 alone served ZERO rows.  The recorder, by contrast, writes the spelling
// AS WRITTEN here -- verified by readback: t3 comes back as
// `std::chrono::time_point<std::chrono::system_clock>`, t2 with the duration.
// So BOTH spellings must be present, and they are the same Rust type because a
// `SystemTime` carries no unit, which is what makes the erased ratio harmless.
// The steady_clock rows (g177, g2844, g2845) search the FULL spelling and are
// served by t1; no suppressed steady_clock key is written because no corpus site
// searches one.
using t3 = std::chrono::time_point<std::chrono::system_clock>;
