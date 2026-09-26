// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::optional<T> maps to Rust's Option<T>. In the unsafe model the contained
// value is stored inline; the refcount overlay boxes it in a Value<T> so that
// value()/operator* can hand out a Ptr into it.

fn t1<T1>() -> Option<T1> {
    None
}

// --- construction ----------------------------------------------------------

unsafe fn f1<T1>() -> Option<T1> {
    None
}

unsafe fn f2<T1>() -> Option<T1> {
    None
}

unsafe fn f3<T1>(a0: T1) -> Option<T1> {
    Some(a0)
}

unsafe fn f4<T1>(a0: T1) -> Option<T1> {
    Some(a0)
}

unsafe fn f5<T1>(a0: T1) -> Option<T1> {
    Some(a0)
}

unsafe fn f6<T1: Clone>(a0: Option<T1>) -> Option<T1> {
    a0.clone()
}

unsafe fn f7<T1>(a0: &mut Option<T1>) -> Option<T1> {
    a0.take()
}

// --- assignment ------------------------------------------------------------

unsafe fn f8<T1: Clone>(a0: &mut Option<T1>, a1: Option<T1>) {
    *a0 = a1.clone()
}

unsafe fn f9<T1>(a0: &mut Option<T1>, a1: &mut Option<T1>) {
    *a0 = a1.take()
}

unsafe fn f10<T1>(a0: &mut Option<T1>) {
    *a0 = None
}

unsafe fn f11<T1>(a0: &mut Option<T1>, a1: T1) {
    *a0 = Some(a1)
}

unsafe fn f12<T1>(a0: &mut Option<T1>, a1: T1) {
    *a0 = Some(a1)
}

unsafe fn f13<T1>(a0: &mut Option<T1>, a1: T1) {
    *a0 = Some(a1)
}

// --- observers -------------------------------------------------------------

unsafe fn f15<T1>(a0: Option<T1>) -> bool {
    a0.is_some()
}

unsafe fn f16<T1>(a0: &mut Option<T1>) -> *mut T1 {
    a0.as_mut()
        .map_or(::std::ptr::null_mut(), |v| v as *mut T1)
}

unsafe fn f17<T1>(a0: &Option<T1>) -> *const T1 {
    a0.as_ref().map_or(::std::ptr::null(), |v| v as *const T1)
}

unsafe fn f18<T1>(a0: &mut Option<T1>) -> *mut T1 {
    a0.as_mut()
        .map_or(::std::ptr::null_mut(), |v| v as *mut T1)
}

unsafe fn f19<T1>(a0: &Option<T1>) -> *const T1 {
    a0.as_ref().map_or(::std::ptr::null(), |v| v as *const T1)
}

unsafe fn f20<T1>(a0: &mut Option<T1>) -> &mut T1 {
    a0.as_mut().unwrap()
}

unsafe fn f21<T1>(a0: &Option<T1>) -> &T1 {
    a0.as_ref().unwrap()
}

unsafe fn f22<T1: Clone>(a0: Option<T1>, a1: T1) -> T1 {
    a0.as_ref().map_or(a1, |v| v.clone())
}

unsafe fn f23<T1: Clone>(a0: Option<T1>, a1: T1) -> T1 {
    a0.as_ref().map_or(a1, |v| v.clone())
}

unsafe fn f24<T1: Clone>(a0: Option<T1>, a1: T1) -> T1 {
    a0.as_ref().map_or(a1, |v| v.clone())
}

// --- comparison ------------------------------------------------------------

unsafe fn f26<T1: PartialEq>(a0: Option<T1>, a1: Option<T1>) -> bool {
    a0 == a1
}

unsafe fn f27<T1: PartialEq>(a0: Option<T1>, a1: Option<T1>) -> bool {
    a0 != a1
}

unsafe fn f28<T1>(a0: Option<T1>) -> bool {
    a0.is_none()
}

unsafe fn f29<T1>(a0: Option<T1>) -> bool {
    a0.is_some()
}

// --- optional vs a BARE VALUE (see src.cpp) --------------------------------
// `Some(a1)` then Option's own PartialEq is exactly the standard's rule:
// Some(x) == Some(v) compares the values, and None == Some(v) is false, so a
// disengaged optional is never equal to a bare value and `!=` is true for it.
// Writing this as an engagement test, or as `a0.unwrap() == a1`, would be
// wrong on one of those cases and would also panic on the disengaged one.

unsafe fn f30<T1: PartialEq>(a0: Option<T1>, a1: T1) -> bool {
    a0 == Some(a1)
}

unsafe fn f31<T1: PartialEq>(a0: Option<T1>, a1: T1) -> bool {
    a0 != Some(a1)
}

// --- optional RELATIONAL vs a BARE VALUE (see src.cpp) ----------------------
// `Some(a1)` then Option's own PartialOrd is exactly the standard's rule,
// because Rust derives Option's ordering with None < Some(_): a disengaged
// optional compares LESS THAN every bare value, so `<`/`<=` are true for it and
// `>`/`>=` are false, which is what [optional.comparewitht] specifies.  An
// engaged one compares its contained VALUE.  Writing these as an engagement
// test, or as `a0.unwrap() <= a1`, would be wrong on the disengaged case (and
// the unwrap would panic there).  This is NOT the same shape as f30/f31: for ==
// disengaged is unequal both ways, here it has a definite side.

unsafe fn f32<T1: PartialOrd>(a0: Option<T1>, a1: T1) -> bool {
    a0 < Some(a1)
}

unsafe fn f33<T1: PartialOrd>(a0: Option<T1>, a1: T1) -> bool {
    a0 > Some(a1)
}

unsafe fn f34<T1: PartialOrd>(a0: Option<T1>, a1: T1) -> bool {
    a0 <= Some(a1)
}

unsafe fn f35<T1: PartialOrd>(a0: Option<T1>, a1: T1) -> bool {
    a0 >= Some(a1)
}

// --- has_value()/reset() ---------------------------------------------------
// libc++ declares these two in private base classes of std::optional, so both
// the type rules and the expression rules name the base class (see src.cpp).
//
// No #[cfg] here. The src.cpp side is guarded on _LIBCPP_VERSION, and the
// preprocessor already drops the whole rule when that #if is false, so the key
// sets stay in correspondence either way. A #[cfg(target_os = ...)] would be
// strictly WRONG: it is evaluated against the HOST that runs the preprocessor,
// which on this pod is Linux even though the parse is libc++, and it was what
// silently deleted these two rules and turned every has_value() into E0599.

fn t2<T1>() -> Option<T1> {
    None
}

fn t3<T1>() -> Option<T1> {
    None
}

unsafe fn f14<T1>(a0: Option<T1>) -> bool {
    a0.is_some()
}

unsafe fn f25<T1>(a0: &mut Option<T1>) {
    *a0 = None
}
