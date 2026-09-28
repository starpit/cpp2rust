// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn t1<T1>() -> Option<Box<T1>> {
    None
}

fn t2<T1>() -> Option<Box<[T1]>> {
    None
}

unsafe fn f1<T1: Default>(a0: usize) -> Option<Box<[T1]>> {
    Some((0..a0).map(|_| <T1>::default()).collect::<Box<[_]>>())
}
unsafe fn f2<T1>(a0: &mut Option<Box<T1>>) -> *mut T1 {
    a0.as_deref_mut()
        .map_or(::std::ptr::null_mut(), |v| v as *mut T1)
}
unsafe fn f3<T1>(a0: *mut T1) -> Option<Box<T1>> {
    Some(Box::from_raw(a0))
}
unsafe fn f4<T1>(a0: *mut T1) -> Option<Box<T1>> {
    Some(Box::from_raw(a0))
}
unsafe fn f5<T1: Default>(a0: &mut Option<Box<T1>>, a1: *mut T1) {
    let _a0: *mut T1 = a1;
    if _a0.is_null() {
        a0.take();
    } else {
        a0.replace(Box::from_raw(_a0));
    }
}
unsafe fn f6<T1>(a0: &mut Option<Box<T1>>, a1: *mut T1) {
    a0.replace(Box::from_raw(a1));
}
unsafe fn f7<T1>(a0: &mut Option<Box<[T1]>>) -> *mut T1 {
    a0.as_deref_mut()
        .map_or(::std::ptr::null_mut(), |s| s.as_mut_ptr())
}

unsafe fn f8<T1>(init: T1) -> Option<Box<T1>> {
    Some(Box::new(init))
}

unsafe fn f9<T1>(a0: &mut Option<Box<[T1]>>) {
    a0.take();
}

unsafe fn f10<T1: Default>() -> Option<Box<T1>> {
    None
}

unsafe fn f11<T1>() -> Option<Box<[T1]>> {
    None
}

unsafe fn f12<T1>(a0: &mut Option<Box<T1>>) -> Option<Box<T1>> {
    a0.take()
}

unsafe fn f13<T1>(a0: &mut Option<Box<[T1]>>) -> Option<Box<[T1]>> {
    a0.take()
}

unsafe fn f14<T1>(a0: &mut Option<Box<T1>>, a1: &mut Option<Box<T1>>) {
    match a1.take() {
        Some(v) => {
            a0.replace(v);
        }
        None => {
            a0.take();
        }
    }
}

unsafe fn f15<T1>(a0: &mut Option<Box<[T1]>>, a1: &mut Option<Box<[T1]>>) {
    match a1.take() {
        Some(v) => {
            a0.replace(v);
        }
        None => {
            a0.take();
        }
    }
}

// f16-f19 -- null tests.  The nullptr_t operand is `()` (the converter emits the
// nullptr literal as Default::default()) and is deliberately unused.
unsafe fn f16<T1>(a0: &Option<Box<T1>>, a1: ()) -> bool {
    let _: () = a1;
    a0.is_none()
}

unsafe fn f17<T1>(a0: &Option<Box<T1>>, a1: ()) -> bool {
    let _: () = a1;
    a0.is_some()
}

unsafe fn f18<T1>(a0: (), a1: &Option<Box<T1>>) -> bool {
    let _: () = a0;
    a1.is_none()
}

unsafe fn f19<T1>(a0: (), a1: &Option<Box<T1>>) -> bool {
    let _: () = a0;
    a1.is_some()
}

// f20 -- unique_ptr<T1>(nullptr).  The nullptr_t operand is `()` and unused;
// a null unique_ptr owns nothing, so this is the same empty value as f10.
unsafe fn f20<T1>(a0: ()) -> Option<Box<T1>> {
    let _: () = a0;
    None
}
