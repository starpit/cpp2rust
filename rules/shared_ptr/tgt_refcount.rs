// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn t1<T1>() -> Option<Value<T1>> {
    None
}

fn f1<T1>(init: T1) -> Option<Value<T1>> {
    Some(Rc::new(RefCell::new(init)))
}

fn f3<T1>(a0: &Option<Value<T1>>) -> Option<Value<T1>> {
    a0.clone()
}

fn f4<T1>() -> Option<Value<T1>> {
    None
}

fn f5<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: &mut Option<Value<T1>>) {
    a0.write(a1.take())
}

fn f6<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: &Option<Value<T1>>) {
    a0.write(a1.clone())
}

fn f7<T1>(a0: &Option<Value<T1>>) -> bool {
    a0.is_some()
}

fn f8<T1>(a0: &Option<Value<T1>>) -> Ptr<T1> {
    a0.as_ref().map_or(Ptr::null(), |v| Value::as_pointer(v))
}

fn f10<T1>(a0: &Option<Value<T1>>) -> Ptr<T1> {
    a0.as_ref().map_or(Ptr::null(), |v| Value::as_pointer(v))
}

fn f11<T1>(a0: &mut Option<Value<T1>>) {
    *a0 = None
}

fn f16<T1>(a0: &mut Option<Value<T1>>) -> Option<Value<T1>> {
    a0.take()
}
