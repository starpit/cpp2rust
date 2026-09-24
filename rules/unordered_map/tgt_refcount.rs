// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::rc::Rc;

fn t1<T1, T2>() -> BTreeMap<T1, Value<T2>> {
    BTreeMap::new()
}

fn t2<T1: Clone + Ord + 'static, T2: 'static>() -> RefcountMapIter<T1, T2> {
    RefcountMapIter::null()
}

fn t3<T1: Clone + Ord + 'static, T2: 'static>() -> RefcountMapIter<T1, T2> {
    RefcountMapIter::null()
}

fn f1<T1: Ord + Clone + ByteRepr + 'static, T2: Default + ByteRepr + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
) -> Ptr<T2> {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        __v.entry(a1)
            .or_insert_with(|| Rc::new(RefCell::new(<T2>::default())))
            .as_pointer()
    })
}

fn f2<T1, T2>(a0: BTreeMap<T1, Value<T2>>) -> usize {
    a0.len()
}

fn f3<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: RefcountMapIter<T1, T2>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::erase(a0, &a1)
}

fn f4<T1, T2>(a0: BTreeMap<T1, Value<T2>>) -> bool {
    a0.is_empty()
}

fn f5<T1, T2>() -> BTreeMap<T1, Value<T2>> {
    BTreeMap::new()
}

// COPY IS DEEP, RECURSIVELY.  The old body unwrapped exactly ONE Value layer
// (`Rc::new(RefCell::new(v.borrow().clone()))`), which is right for a scalar
// element and WRONG the moment the element is itself a container: the inner
// .clone() then copies Rc HANDLES and the copy ALIASES the original.  Measured
// against clang-built C++: `map<int,map<int,long>> b(a); b[1][2]=22` gave
// C++ a=11, refcount a=22.  DeepClone recurses, so it is correct at every depth.
fn f6<T1: Ord + DeepClone, T2: DeepClone>(
    a0: BTreeMap<T1, Value<T2>>,
) -> BTreeMap<T1, Value<T2>> {
    a0.deep_clone()
}

fn f7<T1: Ord, T2: Default>(a0: &mut BTreeMap<T1, Value<T2>>, a1: T1) -> Ptr<T2> {
    a0.get(&a1).expect("out of range!").as_pointer()
}

fn f8<T1: Ord + Clone + ByteRepr + 'static, T2: Default + ByteRepr + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
) -> Ptr<T2> {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        __v.entry(a1)
            .or_insert_with(|| Rc::new(RefCell::new(<T2>::default())))
            .as_pointer()
    })
}

fn f9<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::end(a0)
}

fn f10<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f11<T1: PartialEq, T2: PartialEq>(
    a0: RefcountMapIter<T1, T2>,
    a1: RefcountMapIter<T1, T2>,
) -> bool {
    a0 != a1
}

fn f12<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::begin(a0)
}

fn f13<T1: PartialEq, T2: PartialEq>(
    a0: RefcountMapIter<T1, T2>,
    a1: RefcountMapIter<T1, T2>,
) -> bool {
    a0 == a1
}

fn f14<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::end(a0)
}

fn f15<T1: Ord, T2: Default>(a0: &mut BTreeMap<T1, Value<T2>>, a1: T1) -> Ptr<T2> {
    a0.get(&a1).expect("out of range!").as_pointer()
}

fn f16<T1: PartialEq, T2: PartialEq>(
    a0: RefcountMapIter<T1, T2>,
    a1: RefcountMapIter<T1, T2>,
) -> bool {
    a0 == a1
}

fn f17<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f18<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::begin(a0)
}

fn f19<T1: Clone, T2>(a0: RefcountMapIter<T1, T2>) -> RefcountMapIter<T1, T2> {
    a0
}

fn f20<T1: Ord + Clone + 'static, T2: 'static>(a0: RefcountMapIter<T1, T2>) -> Value<T1> {
    a0.first()
}

fn f21<T1: Ord + Clone + 'static, T2: 'static>(a0: RefcountMapIter<T1, T2>) -> Value<T2> {
    a0.second()
}

fn f22<T1: Ord + Clone + 'static, T2: 'static>(a0: RefcountMapIter<T1, T2>) -> Value<T1> {
    a0.first()
}

fn f23<T1: Ord + Clone + 'static, T2: 'static>(a0: RefcountMapIter<T1, T2>) -> Value<T2> {
    a0.second()
}

fn f24<T1: 'static, T2: 'static>(a0: Ptr<BTreeMap<T1, Value<T2>>>) -> BTreeMap<T1, Value<T2>> {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| std::mem::take(__v))
}

fn f25<T1: 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: Ptr<BTreeMap<T1, Value<T2>>>,
) {
    let __src = a1.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| std::mem::take(__v));
    a0.write(__src)
}

fn f26<T1: Ord + DeepClone + 'static, T2: DeepClone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: BTreeMap<T1, Value<T2>>,
) {
    a0.write(a1.deep_clone())
}

fn f27<T1: PartialEq, T2: PartialEq>(
    a0: RefcountMapIter<T1, T2>,
    a1: RefcountMapIter<T1, T2>,
) -> bool {
    a0 != a1
}

fn f28<T1: Ord + Clone + 'static, T2: 'static>(
    a0: &mut RefcountMapIter<T1, T2>,
) -> RefcountMapIter<T1, T2> {
    a0.inc();
    a0.clone()
}

fn f29<T1: Ord + Clone + 'static, T2: 'static>(
    a0: &mut RefcountMapIter<T1, T2>,
) -> RefcountMapIter<T1, T2> {
    a0.inc();
    a0.clone()
}

fn f30<T1: 'static, T2: 'static>(a0: Ptr<BTreeMap<T1, Value<T2>>>) {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| __v.clear())
}

fn f31<T1: Ord, T2>(a0: BTreeMap<T1, Value<T2>>, a1: T1) -> usize {
    if a0.contains_key(&a1) {
        1_usize
    } else {
        0_usize
    }
}

// A rule argument is substituted textually at every use, so the rules below
// bind theirs once: repeating it would evaluate the argument twice and leave
// the duplicated literal's type unconstrained.
fn f32<T1: Ord + Clone + 'static, T2: Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: (Value<T1>, Value<T2>),
) -> (Value<RefcountMapIter<T1, T2>>, Value<bool>) {
    let __pair = a1;
    let __key: T1 = <T1>::clone(&*__pair.0.borrow());
    let __value: T2 = <T2>::clone(&*__pair.1.borrow());
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), Rc::new(RefCell::new(__value)));
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f33<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: &mut (Value<T1>, Value<T2>),
) -> (Value<RefcountMapIter<T1, T2>>, Value<bool>) {
    let __pair = a1;
    let __key: T1 = <T1>::clone(&*__pair.0.borrow());
    let __value: Value<T2> = __pair.1.clone();
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), __value);
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f34<T1: Ord + 'static, T2: 'static>(a0: Ptr<BTreeMap<T1, Value<T2>>>, a1: T1) -> usize {
    a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        if __v.remove(&a1).is_some() {
            1_usize
        } else {
            0_usize
        }
    })
}

fn f35<T1: Ord + Clone + 'static, T2: 'static>(
    a0: &mut RefcountMapIter<T1, T2>,
) -> RefcountMapIter<T1, T2> {
    let __old = a0.clone();
    a0.inc();
    __old
}

fn f36<T1: Ord + Clone + 'static, T2: 'static>(
    a0: &mut RefcountMapIter<T1, T2>,
) -> RefcountMapIter<T1, T2> {
    let __old = a0.clone();
    a0.inc();
    __old
}

fn f37<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: RefcountMapIter<T1, T2>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::erase(a0, &a1)
}

fn f38<T1: 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: Ptr<BTreeMap<T1, Value<T2>>>,
) {
    let __lhs = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| std::mem::take(__v));
    let __rhs = a1.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| std::mem::take(__v));
    a0.write(__rhs);
    a1.write(__lhs)
}

fn f39<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
    a2: T2,
) -> (Value<RefcountMapIter<T1, T2>>, Value<bool>) {
    let __key: T1 = a1;
    let __value: T2 = a2;
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), Rc::new(RefCell::new(__value)));
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f40<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
    a2: T2,
) -> (Value<RefcountMapIter<T1, T2>>, Value<bool>) {
    let __key: T1 = a1;
    let __value: T2 = a2;
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), Rc::new(RefCell::new(__value)));
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f41<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: &mut (Value<T1>, Value<T2>),
) -> (Value<RefcountMapIter<T1, T2>>, Value<bool>) {
    let __pair = a1;
    let __key: T1 = <T1>::clone(&*__pair.0.borrow());
    let __value: Value<T2> = __pair.1.clone();
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), __value);
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f42<T1: Ord + Clone + ByteRepr + 'static, T2: Clone + ByteRepr + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: Ptr<(Value<T1>, Value<T2>)>,
) -> (Value<RefcountMapIter<T1, T2>>, Value<bool>) {
    let (__key, __value) =
        a1.with(|__p: &(Value<T1>, Value<T2>)| (__p.0.borrow().clone(), __p.1.borrow().clone()));
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), Rc::new(RefCell::new(__value)));
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f43<T1: Ord + Clone + 'static, T2: Clone + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: (Value<T1>, Value<T2>),
) -> (Value<RefcountMapIter<T1, T2>>, Value<bool>) {
    let __pair = a1;
    let __key: T1 = <T1>::clone(&*__pair.0.borrow());
    let __value: T2 = <T2>::clone(&*__pair.1.borrow());
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), Rc::new(RefCell::new(__value)));
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f44<T1: Ord + Clone + ByteRepr + 'static, T2: Clone + ByteRepr + 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: Ptr<(Value<T1>, Value<T2>)>,
) -> (Value<RefcountMapIter<T1, T2>>, Value<bool>) {
    let (__key, __value) =
        a1.with(|__p: &(Value<T1>, Value<T2>)| (__p.0.borrow().clone(), __p.1.borrow().clone()));
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), Rc::new(RefCell::new(__value)));
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}

fn f45<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::begin(a0)
}

fn f46<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
) -> RefcountMapIter<T1, T2> {
    RefcountMapIter::end(a0)
}


fn f47<T1: PartialEq, T2: PartialEq>(a0: BTreeMap<T1, Value<T2>>, a1: BTreeMap<T1, Value<T2>>) -> bool {
    a0 == a1
}

fn f48<T1: PartialEq, T2: PartialEq>(a0: BTreeMap<T1, Value<T2>>, a1: BTreeMap<T1, Value<T2>>) -> bool {
    a0 != a1
}

fn f49<T1: Ord + Clone, T2>(a0: Vec<(Value<T1>, Value<T2>)>) -> BTreeMap<T1, Value<T2>> {
    a0.into_iter()
        .map(|(__k, __v): (Value<T1>, Value<T2>)| (__k.borrow().clone(), __v))
        .collect()
}

fn f50<T1: Ord + Clone + 'static, T2: 'static>(
    a0: Ptr<BTreeMap<T1, Value<T2>>>,
    a1: T1,
    a2: T2,
) -> (Value<RefcountMapIter<T1, T2>>, Value<bool>) {
    let __key: T1 = a1;
    let __value: T2 = a2;
    let __inserted = a0.with_mut(|__v: &mut BTreeMap<T1, Value<T2>>| {
        if __v.contains_key(&__key) {
            false
        } else {
            __v.insert(__key.clone(), Rc::new(RefCell::new(__value)));
            true
        }
    });
    (
        Rc::new(RefCell::new(RefcountMapIter::find_key(a0, &__key))),
        Rc::new(RefCell::new(__inserted)),
    )
}
