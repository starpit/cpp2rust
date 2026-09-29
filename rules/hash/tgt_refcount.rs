// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// `std::__hash_impl<T>` is a STATELESS functor (no members at all), so it carries no
// representation.  A type rule's target must still YIELD an initializer -- a
// `-> ()` target panics at syntactic.rs:591 -- so it is modelled as a zero `usize`.
fn t1<T1>() -> usize {
    0
}

fn t2<T1>() -> usize {
    0
}

// f1 -- `size_t std::__hash_impl<T1>::operator()(T1) const`, the IDENTITY CAST
// mandated by libc++ hash.h:367-372 (and reached from the enum specialisation at
// :358-365 via the underlying integer).  a0 is the stateless functor receiver,
// modelled as `usize` by t1, and is deliberately dropped; a1 is mentioned EXACTLY
// ONCE so a side-effecting operand is evaluated once, matching C++.
//
// Rule bodies are INLINED at the use site (ir.rs:52), so this becomes
// `(*x).storage_ as usize` where the C++ was `std::hash<SenComponents>()(x.storage_)`
// -- replacing the `0((*x).storage_)` that 427 sites emitted.
// ⚠️ `a1 as usize` IS NOT WRITABLE HERE and the reason is worth recording: rule target
// bodies are TYPE-CHECKED by the Rust stage (rustc, via semantic.rs), and `as` on a bare
// generic is `error[E0605]: non-primitive cast: T1 as usize`.  MEASURED 2026-09-29; it
// then took the stage down with `panicked at src/semantic.rs:260: unresolved
// access="unknown" in f1` + SIGSEGV.  No std trait makes `as` legal on a generic, so the
// cast is routed through `i128`, which is the one integer type that holds EVERY C++
// integer type this port can reach (i8..i64, u8..u64, and every fixed-underlying enum)
// WITHOUT loss.  `i128 as usize` then truncates to the low 64 bits, which is bit-for-bit
// what `static_cast<size_t>` does: -1i32 -> -1i128 -> 0xFFFF_FFFF_FFFF_FFFF, and
// u64::MAX -> itself.  The bound is a device to make the TARGET compile, exactly as
// rules/plus's `T1: std::ops::Add<Output = T1> + Copy` is; at the use site the body is
// inlined and the bound is not re-checked.
unsafe fn f1<T1: Into<i128>>(a0: usize, a1: T1) -> usize {
    <T1 as Into<i128>>::into(a1) as usize
}
