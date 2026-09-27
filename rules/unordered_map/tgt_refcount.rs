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
    a0: &mut HashMap<T1, Value<T2>>,
    a1: T1,
) -> Ptr<T2> {
    a0.entry(a1)
        .or_insert_with(|| Rc::new(RefCell::new(<T2>::default())))
        .as_pointer()
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

fn f8<T1: Eq + Hash, T2: 'static>(a0: &mut HashMap<T1, Value<T2>>, a1: T1) -> usize {
    (if a0.remove(&a1).is_some() { 1 } else { 0 })
}

fn f9<T1, T2>(a0: HashMap<T1, Value<T2>>) -> bool {
    a0.is_empty()
}

fn f10<T1: 'static, T2: 'static>(a0: &mut HashMap<T1, Value<T2>>) {
    a0.clear()
}

fn f11<T1: Eq + Hash, T2: PartialEq>(
    a0: &HashMap<T1, Value<T2>>,
    a1: &HashMap<T1, Value<T2>>,
) -> bool {
    a0 == a1
}

fn f13<T1: Eq + Hash + Clone + 'static, T2: Clone + 'static>(
    a0: &mut HashMap<T1, Value<T2>>,
    a1: HashMap<T1, Value<T2>>,
) {
    *a0 = a1
        .iter()
        .map(|(k, v)| (k.clone(), Rc::new(RefCell::new(v.borrow().clone()))))
        .collect()
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

fn f18<T1: Eq + Hash + 'static>(a0: &mut HashSet<T1>, a1: T1) -> usize {
    (if a0.remove(&a1) { 1 } else { 0 })
}

fn f19<T1>(a0: HashSet<T1>) -> bool {
    a0.is_empty()
}

fn f20<T1: 'static>(a0: &mut HashSet<T1>) {
    a0.clear()
}

fn f21<T1: Eq + Hash>(a0: &HashSet<T1>, a1: &HashSet<T1>) -> bool {
    a0 == a1
}

