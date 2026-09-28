// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::os::fd::AsFd;

// TODO: t2 and t3 should be translated to Ptr<dyn Traits>
// t1 -- `std::ostream` by VALUE (t2 is `std::ostream &`, t3 is `std::ostream *`).
//
// THIS WAS MISSING, AND ITS ABSENCE POISONED EVERY REFCOUNT TRANSLATION. src.cpp declares
// `using t1 = std::ostream;` and tgt_unsafe.rs has a `t1`, but this file did not -- the
// `using tN =` with no matching tN target case, which aborts at LOAD TIME
// (translation_rule.cpp:233, `ir_src.json type entry has no matching IR target rule`) and
// under NDEBUG presents as rc=139 with no message. `check-ir.sh` reported
// `LOAD FAILURE (refcount model) -- this tree poisons EVERY translation`.
// It was LATENT rather than new: rules/iostream had not been regenerated since #164 made
// Rust type rules standalone functions, so no published tree had ever exercised it, and it
// only surfaced when this module was regenerated for the ios_base::in/out keys.
//
// The body mirrors tgt_unsafe.rs's t1 verbatim because a plain owned value is
// model-independent -- no Ptr, no raw pointer, nothing to re-shape. Deliberately NOT
// "improved": `File::open("")` always fails so this initializer would panic if it were ever
// materialised, which is worth a separate look, but changing committed unsafe behaviour while
// fixing a load failure would conflate two things.
fn t1() -> std::fs::File {
    std::fs::File::open("").unwrap()
}

fn t2() -> Ptr<std::fs::File> {
    Ptr::null()
}

fn t3() -> Ptr<std::fs::File> {
    Ptr::null()
}

fn f1() -> ::std::fs::File {
    std::fs::File::from(std::io::stdout().as_fd().try_clone_to_owned().unwrap())
}

fn f2() -> ::std::fs::File {
    std::fs::File::from(std::io::stderr().as_fd().try_clone_to_owned().unwrap())
}

fn f3() -> Ptr<::std::fs::File> {
    libcc2rs::cout()
}

fn f4() -> Ptr<::std::fs::File> {
    libcc2rs::cerr()
}

// f5/f6 -- std::ios_base::in / ::out. Bodies identical to tgt_unsafe.rs: these are plain
// `unsigned int` constants (libcxx/ios:283-288, `typedef unsigned int openmode; in = 0x08;
// out = 0x10`), so nothing here is model-dependent.
//
// THEY ARE RESTATED ANYWAY, AND THAT IS NOT OPTIONAL. 0edd6227 added them to src.cpp and
// tgt_unsafe.rs only, on the reasoning that a value-like key needs no refcount override
// because ir_unsafe.json is the unconditional base layer. That reasoning is right about the
// OVERLAY and wrong about THIS module: because rules/iostream HAS a tgt_refcount.rs, omitting
// a key from it leaves ir_refcount.json with 6 keys against ir_src.json's 9, and the
// converter then ABORTS AT LOAD TIME in the refcount model -- `check-ir.sh` reported
// `LOAD FAILURE (refcount model) -- this tree poisons EVERY translation` with a core dump.
// So the rule is: a module that has a tgt_refcount.rs must carry EVERY key in it, even where
// the body is byte-identical. (rules/mlir legitimately runs 151 src / 149 refcount keys, so
// the omission is tolerated somewhere -- but not here, and check-ir.sh is the authority.)
fn f5() -> u32 {
    0x08
}

fn f6() -> u32 {
    0x10
}

// f7 -- std::ios_base::binary. Restated byte-identically from tgt_unsafe.rs because this
// module HAS a tgt_refcount.rs and therefore must carry EVERY key in it -- omitting one
// leaves ir_refcount.json short of ir_src.json and the converter ABORTS AT LOAD TIME in
// the refcount model, poisoning every translation. That is the incident documented above
// for f5/f6; it is not hypothetical.
fn f7() -> u32 {
    0x04
}

// t4 -- std::istream. Plain owned value, model-independent, so this is t1's body shape
// verbatim. Restated here because this module HAS a tgt_refcount.rs and must therefore
// carry EVERY key (see the f5/f6 note above -- omitting one aborts at LOAD TIME).
fn t4() -> std::fs::File {
    std::fs::File::open("").unwrap()
}

// t5 -- std::ios_base::seekdir == int-promoted unscoped enum -> i32. Byte-identical to
// tgt_unsafe.rs; restated for the same load-time reason.
fn t5() -> i32 {
    0
}
