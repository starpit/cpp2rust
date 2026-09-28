// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;
use std::cell::RefCell;
use std::collections::{HashMap, HashSet};
use std::hash::Hash;
use std::rc::Rc;

fn t1<T1, T2>() -> HashMap<T1, Value<T2>> {
    HashMap::new()
}

fn t2<T1>() -> HashSet<T1> {
    HashSet::new()
}

fn f1<T1: Eq + Hash + Clone + 'static, T2: Default + 'static>(
    a0: Ptr<HashMap<T1, Value<T2>>>,
    a1: T1,
) -> Ptr<T2> {
    a0.with_mut(|__v: &mut HashMap<T1, Value<T2>>| {
        __v.entry(a1)
            .or_insert_with(|| Rc::new(RefCell::new(<T2>::default())))
            .as_pointer()
    })
}

fn f2<T1, T2>(a0: HashMap<T1, Value<T2>>) -> usize {
    a0.len()
}

fn f3<T1, T2>() -> HashMap<T1, Value<T2>> {
    HashMap::new()
}

fn f4<T1: Eq + Hash, T2: Default>(a0: &mut HashMap<T1, Value<T2>>, a1: T1) -> Ptr<T2> {
    a0.get(&a1).expect("out of range!").as_pointer()
}

fn f5<T1: Eq + Hash, T2: Default>(a0: &mut HashMap<T1, Value<T2>>, a1: T1) -> Ptr<T2> {
    a0.get(&a1).expect("out of range!").as_pointer()
}

fn f6<T1: Eq + Hash, T2>(a0: HashMap<T1, Value<T2>>, a1: T1) -> usize {
    (if a0.contains_key(&a1) { 1 } else { 0 })
}

fn f8<T1: Eq + Hash + 'static, T2: 'static>(a0: Ptr<HashMap<T1, Value<T2>>>, a1: T1) -> usize {
    a0.with_mut(|__v: &mut HashMap<T1, Value<T2>>| {
        if __v.remove(&a1).is_some() { 1 } else { 0 }
    })
}

fn f9<T1, T2>(a0: HashMap<T1, Value<T2>>) -> bool {
    a0.is_empty()
}

fn f10<T1: 'static, T2: 'static>(a0: Ptr<HashMap<T1, Value<T2>>>) {
    a0.with_mut(|__v: &mut HashMap<T1, Value<T2>>| __v.clear())
}

fn f11<T1: Eq + Hash, T2: PartialEq>(
    a0: &HashMap<T1, Value<T2>>,
    a1: &HashMap<T1, Value<T2>>,
) -> bool {
    a0 == a1
}

fn f13<T1: Eq + Hash + Clone + 'static, T2: Clone + 'static>(
    a0: Ptr<HashMap<T1, Value<T2>>>,
    a1: HashMap<T1, Value<T2>>,
) {
    a0.write(
        a1.iter()
            .map(|(k, v)| (k.clone(), Rc::new(RefCell::new(v.borrow().clone()))))
            .collect(),
    )
}

fn f14<T1>() -> HashSet<T1> {
    HashSet::new()
}

fn f15<T1>(a0: HashSet<T1>) -> usize {
    a0.len()
}

fn f16<T1: Eq + Hash>(a0: HashSet<T1>, a1: T1) -> usize {
    (if a0.contains(&a1) { 1 } else { 0 })
}

fn f18<T1: Eq + Hash + 'static>(a0: Ptr<HashSet<T1>>, a1: T1) -> usize {
    a0.with_mut(|__v: &mut HashSet<T1>| if __v.remove(&a1) { 1 } else { 0 })
}

fn f19<T1>(a0: HashSet<T1>) -> bool {
    a0.is_empty()
}

fn f20<T1: 'static>(a0: Ptr<HashSet<T1>>) {
    a0.with_mut(|__v: &mut HashSet<T1>| __v.clear())
}

fn f21<T1: Eq + Hash>(a0: &HashSet<T1>, a1: &HashSet<T1>) -> bool {
    a0 == a1
}


fn t3<T1: Clone + Eq + Hash + 'static, T2: 'static>() -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::null()
}

fn t4<T1: Clone + Eq + Hash + 'static, T2: 'static>() -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::null()
}

fn f22<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: Ptr<HashMap<T1, Value<T2>>>,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::begin(a0)
}

fn f23<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: Ptr<HashMap<T1, Value<T2>>>,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::end(a0)
}

fn f24<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: Ptr<HashMap<T1, Value<T2>>>,
    a1: T1,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::find_key(a0, &a1)
}

fn f25<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: Ptr<HashMap<T1, Value<T2>>>,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::begin(a0)
}

fn f26<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: Ptr<HashMap<T1, Value<T2>>>,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::end(a0)
}

fn f27<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: Ptr<HashMap<T1, Value<T2>>>,
    a1: T1,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::find_key(a0, &a1)
}

fn f28<T1: PartialEq, T2>(
    a0: RefcountHashMapIter<T1, T2>,
    a1: RefcountHashMapIter<T1, T2>,
) -> bool {
    a0 == a1
}

fn f29<T1: PartialEq, T2>(
    a0: RefcountHashMapIter<T1, T2>,
    a1: RefcountHashMapIter<T1, T2>,
) -> bool {
    a0 != a1
}

fn f30<T1: PartialEq, T2>(
    a0: RefcountHashMapIter<T1, T2>,
    a1: RefcountHashMapIter<T1, T2>,
) -> bool {
    a0 == a1
}

fn f31<T1: PartialEq, T2>(
    a0: RefcountHashMapIter<T1, T2>,
    a1: RefcountHashMapIter<T1, T2>,
) -> bool {
    a0 != a1
}

fn f32<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: &mut RefcountHashMapIter<T1, T2>,
) -> RefcountHashMapIter<T1, T2> {
    a0.prefix_inc()
}

fn f33<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: &mut RefcountHashMapIter<T1, T2>,
) -> RefcountHashMapIter<T1, T2> {
    a0.postfix_inc()
}

fn f34<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: &mut RefcountHashMapIter<T1, T2>,
) -> RefcountHashMapIter<T1, T2> {
    a0.prefix_inc()
}

fn f35<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: &mut RefcountHashMapIter<T1, T2>,
) -> RefcountHashMapIter<T1, T2> {
    a0.postfix_inc()
}

fn f36<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: RefcountHashMapIter<T1, T2>,
) -> Value<T1> {
    a0.first()
}

fn f37<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: RefcountHashMapIter<T1, T2>,
) -> Value<T2> {
    a0.second()
}

fn f38<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: RefcountHashMapIter<T1, T2>,
) -> Value<T1> {
    a0.first()
}

fn f39<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: RefcountHashMapIter<T1, T2>,
) -> Value<T2> {
    a0.second()
}

fn f40<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: Ptr<HashMap<T1, Value<T2>>>,
    a1: RefcountHashMapIter<T1, T2>,
) -> RefcountHashMapIter<T1, T2> {
    RefcountHashMapIter::erase(a0, &a1)
}

// --- std::unordered_set<T1>::const_iterator (== ::iterator in libc++) ---------
fn t5<T1: Eq + Hash + Clone + 'static>() -> RefcountHashSetIter<T1> {
    RefcountHashSetIter::null()
}

fn f41<T1: Eq + Hash + Clone + 'static>(
    a0: Ptr<HashSet<T1>>,
    a1: T1,
) -> (RefcountHashSetIter<T1>, bool) {
    {
        let __p = a0;
        let __k = a1;
        let __inserted = Ptr::with_mut(&__p, |__s: &mut HashSet<T1>| {
            HashSet::insert(__s, __k.clone())
        });
        (RefcountHashSetIter::find_key(__p, &__k), __inserted)
    }
}

fn f42<T1: Eq + Hash + Clone + 'static>(
    a0: Ptr<HashSet<T1>>,
    a1: T1,
) -> (RefcountHashSetIter<T1>, bool) {
    {
        let __p = a0;
        let __k = a1;
        let __inserted = Ptr::with_mut(&__p, |__s: &mut HashSet<T1>| {
            HashSet::insert(__s, __k.clone())
        });
        (RefcountHashSetIter::find_key(__p, &__k), __inserted)
    }
}

fn f43<T1: Eq + Hash + Clone + 'static>(a0: Ptr<HashSet<T1>>) -> RefcountHashSetIter<T1> {
    RefcountHashSetIter::begin(a0)
}

fn f44<T1: Eq + Hash + Clone + 'static>(a0: Ptr<HashSet<T1>>) -> RefcountHashSetIter<T1> {
    RefcountHashSetIter::end(a0)
}

fn f45<T1: Eq + Hash + Clone + 'static>(a0: Ptr<HashSet<T1>>) -> RefcountHashSetIter<T1> {
    RefcountHashSetIter::begin(a0)
}

fn f46<T1: Eq + Hash + Clone + 'static>(a0: Ptr<HashSet<T1>>) -> RefcountHashSetIter<T1> {
    RefcountHashSetIter::end(a0)
}

fn f47<T1: Eq + Hash + Clone + 'static>(
    a0: Ptr<HashSet<T1>>,
    a1: T1,
) -> RefcountHashSetIter<T1> {
    RefcountHashSetIter::find_key(a0, &a1)
}

fn f48<T1: Eq + Hash + Clone + 'static>(
    a0: Ptr<HashSet<T1>>,
    a1: T1,
) -> RefcountHashSetIter<T1> {
    RefcountHashSetIter::find_key(a0, &a1)
}

fn f49<T1: PartialEq>(a0: RefcountHashSetIter<T1>, a1: RefcountHashSetIter<T1>) -> bool {
    a0 == a1
}

fn f50<T1: PartialEq>(a0: RefcountHashSetIter<T1>, a1: RefcountHashSetIter<T1>) -> bool {
    a0 != a1
}

fn f51<T1: Eq + Hash + Clone + 'static>(
    a0: &mut RefcountHashSetIter<T1>,
) -> RefcountHashSetIter<T1> {
    a0.prefix_inc()
}

fn f52<T1: Eq + Hash + Clone + 'static>(
    a0: &mut RefcountHashSetIter<T1>,
) -> RefcountHashSetIter<T1> {
    a0.postfix_inc()
}


fn f54<T1: Eq + Hash + Clone + 'static>(
    a0: Ptr<HashSet<T1>>,
    a1: RefcountHashSetIter<T1>,
) -> RefcountHashSetIter<T1> {
    RefcountHashSetIter::erase(a0, &a1)
}

fn f55<T1: Eq + Hash + Clone + 'static, T2: Default + 'static>(
    a0: Ptr<HashMap<T1, Value<T2>>>,
    a1: T1,
) -> Ptr<T2> {
    a0.with_mut(|__v: &mut HashMap<T1, Value<T2>>| {
        __v.entry(a1)
            .or_insert_with(|| Rc::new(RefCell::new(<T2>::default())))
            .as_pointer()
    })
}

fn f56<T1: Eq + Hash + Clone, T2>(a0: Vec<(Value<T1>, Value<T2>)>) -> HashMap<T1, Value<T2>> {
    a0.into_iter()
        .rev()
        .map(|(__k, __v)| (__k.borrow().clone(), __v))
        .collect::<HashMap<T1, Value<T2>>>()
}

fn f57<T1: Eq + Hash + Clone + 'static, T2: 'static>(
    a0: Ptr<HashMap<T1, Value<T2>>>,
    a1: T1,
    a2: T2,
) -> (RefcountHashMapIter<T1, T2>, bool) {
    {
        let __p = a0;
        let __k = a1;
        let __v = a2;
        let __inserted = !Ptr::with_ref(&__p, |__m: &HashMap<T1, Value<T2>>| {
            __m.contains_key(&__k)
        });
        if __inserted {
            Ptr::with_mut(&__p, |__m: &mut HashMap<T1, Value<T2>>| {
                __m.insert(__k.clone(), Rc::new(RefCell::new(__v)));
            });
        }
        (RefcountHashMapIter::find_key(__p, &__k), __inserted)
    }
}
