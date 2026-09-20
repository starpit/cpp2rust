// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// Overlays tgt_unsafe.rs: only the rules whose Rust differs in the refcount
// model are repeated here. Those are exactly the rules that name a raw C
// pointer (Ptr<T> instead of *mut T), plus the assignment operators, which
// take their destination as a Ptr in this model.

use libcc2rs::*;

fn f9<T1>(a0: Ptr<T1>) -> Option<Value<T1>> {
    a0.to_owned_opt()
}

fn f12<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: Option<Value<T1>>) {
    a0.write(a1.clone())
}

fn f13<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: &mut Option<Value<T1>>) {
    a0.write(a1.take())
}

fn f14<T1>(a0: Option<Value<T1>>) -> Ptr<T1> {
    a0.as_pointer()
}

// operator-> must yield the pointee itself, not a pointer: the converter
// appends the field access straight onto whatever this rule produces.
fn f15<T1>(a0: &Option<Value<T1>>) -> std::cell::Ref<'_, T1> {
    a0.as_ref().unwrap().borrow()
}

fn f16<T1>(a0: Option<Value<T1>>) -> Ptr<T1> {
    a0.as_pointer()
}

fn f20<T1>(a0: &mut Option<Value<T1>>, a1: Ptr<T1>) {
    let _p: Ptr<_> = a1;
    *a0 = _p.to_owned_opt()
}
