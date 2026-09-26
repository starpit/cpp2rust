// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::SourceMgr -> its buffer table.  Why Vec<Vec<u8>> and not a handle, and
// why AddNewSourceBuffer is deliberately absent, are in src.cpp.

fn t1() -> Vec<Vec<u8>> {
    Default::default()
}

unsafe fn f1() -> Vec<Vec<u8>> {
    Vec::new()
}
