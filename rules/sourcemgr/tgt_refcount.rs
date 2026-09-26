// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::SourceMgr -> its buffer table; see src.cpp for the model.

fn t1() -> Vec<Vec<u8>> {
    Default::default()
}

// llvm::MemoryBuffer -> ONE buffer's bytes, i.e. the ELEMENT of t1's table.
// The guaranteed NUL terminator is NOT in this Vec, matching getBuffer()'s
// StringRef; src.cpp records that dt_src calls no accessor at all, so the NUL
// boundary is unreachable here and is deliberately not pre-decided.
fn t2() -> Vec<u8> {
    Default::default()
}

fn f1() -> Vec<Vec<u8>> {
    Vec::new()
}
