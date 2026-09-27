// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp.  std::thread::id is modelled as u64; the model is identical in
// both models because u64 is not a pointer and not a container.

fn t1() -> u64 {
    0
}

fn f1() -> u64 {
    let mut __h = std::collections::hash_map::DefaultHasher::new();
    std::hash::Hash::hash(&std::thread::current().id(), &mut __h);
    std::hash::Hasher::finish(&__h) | 1
}

fn f2(a0: u64, a1: u64) -> bool {
    a0 == a1
}

fn f3(a0: u64, a1: u64) -> bool {
    a0 != a1
}

// Default-constructed std::thread::id == "not any thread"; libc++ holds a zero
// __libcpp_thread_id there, and f1 guarantees a live id is never 0.
fn f4() -> u64 {
    0
}
