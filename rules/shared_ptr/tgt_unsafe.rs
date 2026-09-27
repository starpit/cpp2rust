// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use std::rc::Rc;

fn t1<T1>() -> Option<Rc<T1>> {
    None
}

unsafe fn f1<T1>(init: T1) -> Option<Rc<T1>> {
    Some(Rc::new(init))
}

unsafe fn f3<T1>(a0: &Option<Rc<T1>>) -> Option<Rc<T1>> {
    a0.clone()
}

unsafe fn f4<T1>() -> Option<Rc<T1>> {
    None
}

unsafe fn f5<T1>(a0: &mut Option<Rc<T1>>, a1: &mut Option<Rc<T1>>) {
    *a0 = a1.take()
}

unsafe fn f6<T1>(a0: &mut Option<Rc<T1>>, a1: &Option<Rc<T1>>) {
    *a0 = a1.clone()
}

unsafe fn f7<T1>(a0: &Option<Rc<T1>>) -> bool {
    a0.is_some()
}

unsafe fn f8<T1>(a0: &Option<Rc<T1>>) -> &mut T1 {
    &mut *(a0.as_ref().map_or(::std::ptr::null_mut(), |r| Rc::as_ptr(r) as *mut T1))
}

unsafe fn f10<T1>(a0: &Option<Rc<T1>>) -> *mut T1 {
    a0.as_ref().map_or(::std::ptr::null_mut(), |r| Rc::as_ptr(r) as *mut T1)
}

unsafe fn f11<T1>(a0: &mut Option<Rc<T1>>) {
    *a0 = None
}

unsafe fn f16<T1>(a0: &mut Option<Rc<T1>>) -> Option<Rc<T1>> {
    a0.take()
}
