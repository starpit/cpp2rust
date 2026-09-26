// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::optional<T> -> Option<Value<T>>: the contained value lives behind an Rc
// so that value() / operator* / operator-> can hand out a Ptr (resp. a borrow)
// into it, the same way a std::vector element does.

use libcc2rs::*;
use std::cell::RefCell;
use std::rc::Rc;

fn t1<T1>() -> Option<Value<T1>> {
    None
}

// --- construction ----------------------------------------------------------

fn f1<T1>() -> Option<Value<T1>> {
    None
}

fn f2<T1>() -> Option<Value<T1>> {
    None
}

fn f3<T1>(a0: T1) -> Option<Value<T1>> {
    Some(Rc::new(RefCell::new(a0)))
}

fn f4<T1>(a0: T1) -> Option<Value<T1>> {
    Some(Rc::new(RefCell::new(a0)))
}

fn f5<T1>(a0: T1) -> Option<Value<T1>> {
    Some(Rc::new(RefCell::new(a0)))
}

fn f6<T1: Clone>(a0: Option<Value<T1>>) -> Option<Value<T1>> {
    a0.as_ref()
        .map(|v| Rc::new(RefCell::new(v.borrow().clone())))
}

fn f7<T1>(a0: &mut Option<Value<T1>>) -> Option<Value<T1>> {
    a0.take()
}

// --- assignment ------------------------------------------------------------

fn f8<T1: Clone + ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: Option<Value<T1>>) {
    a0.write(
        a1.as_ref()
            .map(|v| Rc::new(RefCell::new(v.borrow().clone()))),
    )
}

fn f9<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: &mut Option<Value<T1>>) {
    a0.write(a1.take())
}

fn f10<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>) {
    a0.write(None)
}

fn f11<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: T1) {
    a0.write(Some(Rc::new(RefCell::new(a1))))
}

fn f12<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: T1) {
    a0.write(Some(Rc::new(RefCell::new(a1))))
}

fn f13<T1: ByteRepr>(a0: Ptr<Option<Value<T1>>>, a1: T1) {
    a0.write(Some(Rc::new(RefCell::new(a1))))
}

// --- observers -------------------------------------------------------------

fn f15<T1>(a0: Option<Value<T1>>) -> bool {
    a0.is_some()
}

fn f16<T1>(a0: &mut Option<Value<T1>>) -> Ptr<T1> {
    a0.as_pointer()
}

fn f17<T1>(a0: Option<Value<T1>>) -> Ptr<T1> {
    a0.as_pointer()
}

fn f18<T1>(a0: &mut Option<Value<T1>>) -> Ptr<T1> {
    a0.as_pointer()
}

fn f19<T1>(a0: Option<Value<T1>>) -> Ptr<T1> {
    a0.as_pointer()
}

fn f20<T1>(a0: &Option<Value<T1>>) -> std::cell::Ref<'_, T1> {
    a0.as_ref().unwrap().borrow()
}

fn f21<T1>(a0: &Option<Value<T1>>) -> std::cell::Ref<'_, T1> {
    a0.as_ref().unwrap().borrow()
}

fn f22<T1: Clone>(a0: Option<Value<T1>>, a1: T1) -> T1 {
    a0.as_ref().map_or(a1, |v| v.borrow().clone())
}

fn f23<T1: Clone>(a0: Option<Value<T1>>, a1: T1) -> T1 {
    a0.as_ref().map_or(a1, |v| v.borrow().clone())
}

fn f24<T1: Clone>(a0: Option<Value<T1>>, a1: T1) -> T1 {
    a0.as_ref().map_or(a1, |v| v.borrow().clone())
}

// --- comparison ------------------------------------------------------------

fn f26<T1: PartialEq>(a0: Option<Value<T1>>, a1: Option<Value<T1>>) -> bool {
    a0 == a1
}

fn f27<T1: PartialEq>(a0: Option<Value<T1>>, a1: Option<Value<T1>>) -> bool {
    a0 != a1
}

fn f28<T1>(a0: Option<Value<T1>>) -> bool {
    a0.is_none()
}

fn f29<T1>(a0: Option<Value<T1>>) -> bool {
    a0.is_some()
}

// --- optional vs a BARE VALUE (see src.cpp) --------------------------------
// The bare value is wrapped into a Value<T1> so the comparison is the same
// Option<Value<T1>> PartialEq f26/f27 use; that delegates to the INNER value
// (elements are Rc<RefCell<T>> and PartialEq compares what they hold), which
// is what C++ means.  None vs Some is false, so a disengaged optional is never
// equal to a bare value.

fn f30<T1: PartialEq>(a0: Option<Value<T1>>, a1: T1) -> bool {
    a0 == Some(Rc::new(RefCell::new(a1)))
}

fn f31<T1: PartialEq>(a0: Option<Value<T1>>, a1: T1) -> bool {
    a0 != Some(Rc::new(RefCell::new(a1)))
}

// --- optional RELATIONAL vs a BARE VALUE (see src.cpp) ----------------------
// The bare value is wrapped into a Value<T1> so the comparison is the same
// Option<Value<T1>> ordering f32-f35's unsafe twins use: Rc<T> and RefCell<T>
// both delegate PartialOrd to what they hold, and Option orders None before
// every Some, which is exactly the standard's "a disengaged optional is less
// than any value".  So `<`/`<=` are true for a disengaged optional and
// `>`/`>=` are false, and an engaged one compares its contained value.
//
// The Some(...) temporary is a fresh Rc, so only ONE borrow of the receiver is
// live across the comparison and there is no RefCell double-borrow here.

fn f32<T1: PartialOrd>(a0: Option<Value<T1>>, a1: T1) -> bool {
    a0 < Some(Rc::new(RefCell::new(a1)))
}

fn f33<T1: PartialOrd>(a0: Option<Value<T1>>, a1: T1) -> bool {
    a0 > Some(Rc::new(RefCell::new(a1)))
}

fn f34<T1: PartialOrd>(a0: Option<Value<T1>>, a1: T1) -> bool {
    a0 <= Some(Rc::new(RefCell::new(a1)))
}

fn f35<T1: PartialOrd>(a0: Option<Value<T1>>, a1: T1) -> bool {
    a0 >= Some(Rc::new(RefCell::new(a1)))
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

fn t2<T1>() -> Option<Value<T1>> {
    None
}

fn t3<T1>() -> Option<Value<T1>> {
    None
}

fn f14<T1>(a0: Option<Value<T1>>) -> bool {
    a0.is_some()
}

fn f25<T1>(a0: &mut Option<Value<T1>>) {
    *a0 = None
}
