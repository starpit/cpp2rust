// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use std::collections::{HashMap, HashSet};
use std::hash::Hash;

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
    *a0 = a1.clone()
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

