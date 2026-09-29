// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use std::collections::{HashMap, HashSet};
use std::hash::Hash;
use libcc2rs::{MapIterator, PostfixInc, PrefixInc, UnsafeHashMapIterator};

fn t1<T1, T2>() -> HashMap<T1, Box<T2>> {
    HashMap::new()
}

fn t2<T1>() -> HashSet<T1> {
    HashSet::new()
}

unsafe fn f1<T1: Eq + Hash, T2: Default>(a0: &mut HashMap<T1, Box<T2>>, a1: T1) -> &mut T2 {
    a0.entry(a1).or_default().as_mut()
}

unsafe fn f2<T1, T2>(a0: HashMap<T1, Box<T2>>) -> usize {
    a0.len()
}

unsafe fn f3<T1, T2>() -> HashMap<T1, Box<T2>> {
    HashMap::new()
}

unsafe fn f4<T1: Eq + Hash, T2>(a0: &mut HashMap<T1, Box<T2>>, a1: T1) -> *const T2 {
    (a0.get(&a1).expect("out of range!").as_ref() as *const T2)
}

unsafe fn f5<T1: Eq + Hash, T2>(a0: &mut HashMap<T1, Box<T2>>, a1: T1) -> *const T2 {
    (a0.get(&a1).expect("out of range!").as_ref() as *const T2)
}

unsafe fn f6<T1: Eq + Hash, T2>(a0: HashMap<T1, Box<T2>>, a1: T1) -> usize {
    (if a0.contains_key(&a1) { 1 } else { 0 })
}

unsafe fn f8<T1: Eq + Hash, T2>(a0: &mut HashMap<T1, Box<T2>>, a1: T1) -> usize {
    (if a0.remove(&a1).is_some() { 1 } else { 0 })
}

unsafe fn f9<T1, T2>(a0: HashMap<T1, Box<T2>>) -> bool {
    a0.is_empty()
}

unsafe fn f10<T1, T2>(a0: &mut HashMap<T1, Box<T2>>) {
    a0.clear()
}

unsafe fn f11<T1: Eq + Hash, T2: PartialEq>(
    a0: &HashMap<T1, Box<T2>>,
    a1: &HashMap<T1, Box<T2>>,
) -> bool {
    a0 == a1
}

unsafe fn f13<T1: Clone + Eq + Hash, T2: Clone>(
    a0: &mut HashMap<T1, Box<T2>>,
    a1: HashMap<T1, Box<T2>>,
) {
    let __src = a1.clone();
    a0.clear();
    a0.extend(__src);
}

unsafe fn f14<T1>() -> HashSet<T1> {
    HashSet::new()
}

unsafe fn f15<T1>(a0: HashSet<T1>) -> usize {
    a0.len()
}

unsafe fn f16<T1: Eq + Hash>(a0: HashSet<T1>, a1: T1) -> usize {
    (if a0.contains(&a1) { 1 } else { 0 })
}

unsafe fn f18<T1: Eq + Hash>(a0: &mut HashSet<T1>, a1: T1) -> usize {
    (if a0.remove(&a1) { 1 } else { 0 })
}

unsafe fn f19<T1>(a0: HashSet<T1>) -> bool {
    a0.is_empty()
}

unsafe fn f20<T1>(a0: &mut HashSet<T1>) {
    a0.clear()
}

unsafe fn f21<T1: Eq + Hash>(a0: &HashSet<T1>, a1: &HashSet<T1>) -> bool {
    a0 == a1
}


fn t3<T1: Clone + Eq + Hash, T2>() -> UnsafeHashMapIterator<T1, T2> {
    UnsafeHashMapIterator::null()
}

fn t4<T1: Clone + Eq + Hash, T2>() -> UnsafeHashMapIterator<T1, T2> {
    UnsafeHashMapIterator::null()
}

unsafe fn f22<T1: Eq + Hash + Clone, T2>(
    a0: &mut HashMap<T1, Box<T2>>,
) -> UnsafeHashMapIterator<T1, T2> {
    UnsafeHashMapIterator::begin(&*a0 as *const HashMap<T1, Box<T2>>)
}

unsafe fn f23<T1: Eq + Hash + Clone, T2>(
    a0: &mut HashMap<T1, Box<T2>>,
) -> UnsafeHashMapIterator<T1, T2> {
    UnsafeHashMapIterator::end(&*a0 as *const HashMap<T1, Box<T2>>)
}

unsafe fn f24<T1: Eq + Hash + Clone, T2>(
    a0: &mut HashMap<T1, Box<T2>>,
    a1: T1,
) -> UnsafeHashMapIterator<T1, T2> {
    UnsafeHashMapIterator::find_key(&*a0 as *const HashMap<T1, Box<T2>>, &a1)
}

unsafe fn f25<T1: Eq + Hash + Clone, T2>(
    a0: HashMap<T1, Box<T2>>,
) -> UnsafeHashMapIterator<T1, T2> {
    UnsafeHashMapIterator::begin(&a0 as *const HashMap<T1, Box<T2>>)
}

unsafe fn f26<T1: Eq + Hash + Clone, T2>(
    a0: HashMap<T1, Box<T2>>,
) -> UnsafeHashMapIterator<T1, T2> {
    UnsafeHashMapIterator::end(&a0 as *const HashMap<T1, Box<T2>>)
}

unsafe fn f27<T1: Eq + Hash + Clone, T2>(
    a0: HashMap<T1, Box<T2>>,
    a1: T1,
) -> UnsafeHashMapIterator<T1, T2> {
    UnsafeHashMapIterator::find_key(&a0 as *const HashMap<T1, Box<T2>>, &a1)
}

unsafe fn f28<T1: PartialEq, T2>(
    a0: UnsafeHashMapIterator<T1, T2>,
    a1: UnsafeHashMapIterator<T1, T2>,
) -> bool {
    a0 == a1
}

unsafe fn f29<T1: PartialEq, T2>(
    a0: UnsafeHashMapIterator<T1, T2>,
    a1: UnsafeHashMapIterator<T1, T2>,
) -> bool {
    a0 != a1
}

unsafe fn f30<T1: PartialEq, T2>(
    a0: UnsafeHashMapIterator<T1, T2>,
    a1: UnsafeHashMapIterator<T1, T2>,
) -> bool {
    a0 == a1
}

unsafe fn f31<T1: PartialEq, T2>(
    a0: UnsafeHashMapIterator<T1, T2>,
    a1: UnsafeHashMapIterator<T1, T2>,
) -> bool {
    a0 != a1
}

unsafe fn f32<T1: Eq + Hash + Clone, T2>(
    a0: &mut UnsafeHashMapIterator<T1, T2>,
) -> UnsafeHashMapIterator<T1, T2> {
    a0.prefix_inc()
}

unsafe fn f33<T1: Eq + Hash + Clone, T2>(
    a0: &mut UnsafeHashMapIterator<T1, T2>,
) -> UnsafeHashMapIterator<T1, T2> {
    a0.postfix_inc()
}

unsafe fn f34<T1: Eq + Hash + Clone, T2>(
    a0: &mut UnsafeHashMapIterator<T1, T2>,
) -> UnsafeHashMapIterator<T1, T2> {
    a0.prefix_inc()
}

unsafe fn f35<T1: Eq + Hash + Clone, T2>(
    a0: &mut UnsafeHashMapIterator<T1, T2>,
) -> UnsafeHashMapIterator<T1, T2> {
    a0.postfix_inc()
}

unsafe fn f36<T1: Eq + Hash + Clone, T2>(a0: UnsafeHashMapIterator<T1, T2>) -> *const T1 {
    a0.first()
}

unsafe fn f37<T1: Eq + Hash + Clone, T2>(a0: UnsafeHashMapIterator<T1, T2>) -> *mut T2 {
    a0.second()
}

unsafe fn f38<T1: Eq + Hash + Clone, T2>(a0: UnsafeHashMapIterator<T1, T2>) -> *const T1 {
    a0.first()
}

unsafe fn f39<T1: Eq + Hash + Clone, T2>(a0: UnsafeHashMapIterator<T1, T2>) -> *mut T2 {
    a0.second()
}

unsafe fn f40<T1: Eq + Hash + Clone, T2>(
    a0: &mut HashMap<T1, Box<T2>>,
    a1: UnsafeHashMapIterator<T1, T2>,
) -> UnsafeHashMapIterator<T1, T2> {
    UnsafeHashMapIterator::erase(&*a0 as *const HashMap<T1, Box<T2>>, &a1)
}

// --- std::unordered_set<T1>::const_iterator (== ::iterator in libc++) ---------
// One type key: libc++ typedefs both to __hash_const_iterator, and the element type
// is T1 DIRECTLY (no __hash_value_type wrapper), so this is NOT t3/t4's shape.
fn t5<T1: Eq + Hash + Clone>() -> libcc2rs::UnsafeHashSetIterator<T1> {
    libcc2rs::UnsafeHashSetIterator::null()
}

unsafe fn f41<T1: Eq + Hash + Clone>(
    a0: &mut HashSet<T1>,
    a1: T1,
) -> (libcc2rs::UnsafeHashSetIterator<T1>, bool) {
    {
        let __k = a1;
        let __set = &mut *a0;
        let __inserted = HashSet::insert(__set, __k.clone());
        (
            libcc2rs::UnsafeHashSetIterator::find_key(__set as *const HashSet<T1>, &__k),
            __inserted,
        )
    }
}

unsafe fn f42<T1: Eq + Hash + Clone>(
    a0: &mut HashSet<T1>,
    a1: T1,
) -> (libcc2rs::UnsafeHashSetIterator<T1>, bool) {
    {
        let __k = a1;
        let __set = &mut *a0;
        let __inserted = HashSet::insert(__set, __k.clone());
        (
            libcc2rs::UnsafeHashSetIterator::find_key(__set as *const HashSet<T1>, &__k),
            __inserted,
        )
    }
}

unsafe fn f43<T1: Eq + Hash + Clone>(a0: HashSet<T1>) -> libcc2rs::UnsafeHashSetIterator<T1> {
    libcc2rs::UnsafeHashSetIterator::begin(&a0 as *const HashSet<T1>)
}

unsafe fn f44<T1: Eq + Hash + Clone>(a0: HashSet<T1>) -> libcc2rs::UnsafeHashSetIterator<T1> {
    libcc2rs::UnsafeHashSetIterator::end(&a0 as *const HashSet<T1>)
}

unsafe fn f45<T1: Eq + Hash + Clone>(a0: HashSet<T1>) -> libcc2rs::UnsafeHashSetIterator<T1> {
    libcc2rs::UnsafeHashSetIterator::begin(&a0 as *const HashSet<T1>)
}

unsafe fn f46<T1: Eq + Hash + Clone>(a0: HashSet<T1>) -> libcc2rs::UnsafeHashSetIterator<T1> {
    libcc2rs::UnsafeHashSetIterator::end(&a0 as *const HashSet<T1>)
}

unsafe fn f47<T1: Eq + Hash + Clone>(
    a0: HashSet<T1>,
    a1: T1,
) -> libcc2rs::UnsafeHashSetIterator<T1> {
    libcc2rs::UnsafeHashSetIterator::find_key(&a0 as *const HashSet<T1>, &a1)
}

unsafe fn f48<T1: Eq + Hash + Clone>(
    a0: HashSet<T1>,
    a1: T1,
) -> libcc2rs::UnsafeHashSetIterator<T1> {
    libcc2rs::UnsafeHashSetIterator::find_key(&a0 as *const HashSet<T1>, &a1)
}

unsafe fn f49<T1: PartialEq>(
    a0: libcc2rs::UnsafeHashSetIterator<T1>,
    a1: libcc2rs::UnsafeHashSetIterator<T1>,
) -> bool {
    a0 == a1
}

unsafe fn f50<T1: PartialEq>(
    a0: libcc2rs::UnsafeHashSetIterator<T1>,
    a1: libcc2rs::UnsafeHashSetIterator<T1>,
) -> bool {
    a0 != a1
}

unsafe fn f51<T1: Eq + Hash + Clone>(
    a0: &mut libcc2rs::UnsafeHashSetIterator<T1>,
) -> libcc2rs::UnsafeHashSetIterator<T1> {
    a0.prefix_inc()
}

unsafe fn f52<T1: Eq + Hash + Clone>(
    a0: &mut libcc2rs::UnsafeHashSetIterator<T1>,
) -> libcc2rs::UnsafeHashSetIterator<T1> {
    a0.postfix_inc()
}


unsafe fn f54<T1: Eq + Hash + Clone>(
    a0: &mut HashSet<T1>,
    a1: libcc2rs::UnsafeHashSetIterator<T1>,
) -> libcc2rs::UnsafeHashSetIterator<T1> {
    libcc2rs::UnsafeHashSetIterator::erase(&*a0 as *const HashSet<T1>, &a1)
}

unsafe fn f55<T1: Eq + Hash, T2: Default>(a0: &mut HashMap<T1, Box<T2>>, a1: T1) -> &mut T2 {
    a0.entry(a1).or_default().as_mut()
}

unsafe fn f56<T1: Eq + Hash, T2>(a0: Vec<(T1, T2)>) -> HashMap<T1, Box<T2>> {
    a0.into_iter()
        .rev()
        .map(|(__k, __v)| (__k, Box::new(__v)))
        .collect::<HashMap<T1, Box<T2>>>()
}

// NOTE ON `a0`: the receiver is spelled DIRECTLY, three times, instead of being
// bound once to `let __map = &mut *a0;`.  MEASURED: the bound form emitted
// `let __map = &mut &mut firstIndex;` -- a DOUBLE mutable reference, which makes
// `HashMap::insert(__map, ..)` an E0308 and `__map as *const HashMap<..>` an
// invalid cast.  Repeating `a0` is safe here for the same reason it is in f13:
// a rule body is inlined as one expression, so each `aN` occurrence re-evaluates
// its argument, and the receiver of a member call is a place expression.
unsafe fn f57<T1: Eq + Hash + Clone, T2>(
    a0: &mut HashMap<T1, Box<T2>>,
    init: (T1, T2),
) -> (UnsafeHashMapIterator<T1, T2>, bool) {
    {
        let (__k, __v) = init;
        let __inserted = !a0.contains_key(&__k);
        if __inserted {
            a0.insert(__k.clone(), Box::new(__v));
        }
        (
            UnsafeHashMapIterator::find_key(&*a0 as *const HashMap<T1, Box<T2>>, &__k),
            __inserted,
        )
    }
}
