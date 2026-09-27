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
