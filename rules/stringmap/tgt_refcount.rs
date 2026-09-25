// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp.  StringMap's iterator reuses the SAME runtime type std::map,
// std::unordered_map and llvm::DenseMap already use, so the comparisons are
// one-liners -- the work was establishing that the existing representation
// (PartialEq = position by key, end() = None) is what LLVM's
// `LHS.Ptr == RHS.Ptr` means.  The key is fixed to the byte-string
// representation rules/string and rules/stringref use, because StringMap's key
// is not a template parameter.

// The container is the SAME BTreeMap + MapIter pair rules/map, rules/densemap
// and rules/unordered_map use; only the key is pinned, to the byte-string
// representation rules/stringref gives llvm::StringRef.  See src.cpp for why the
// C++ type has to be restated with its defaulted allocator parameter.

use libcc2rs::*;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::rc::Rc;

fn t1<T1: 'static>() -> RefcountMapIter<Vec<u8>, T1> {
    RefcountMapIter::null()
}

fn t2<T1: 'static>() -> RefcountMapIter<Vec<u8>, T1> {
    RefcountMapIter::null()
}

fn f1<T1: PartialEq>(
    a0: RefcountMapIter<Vec<u8>, T1>,
    a1: RefcountMapIter<Vec<u8>, T1>,
) -> bool {
    a0 == a1
}

fn f2<T1: PartialEq>(
    a0: RefcountMapIter<Vec<u8>, T1>,
    a1: RefcountMapIter<Vec<u8>, T1>,
) -> bool {
    a0 != a1
}

fn f3<T1: PartialEq>(
    a0: RefcountMapIter<Vec<u8>, T1>,
    a1: RefcountMapIter<Vec<u8>, T1>,
) -> bool {
    a0 == a1
}

fn f4<T1: PartialEq>(
    a0: RefcountMapIter<Vec<u8>, T1>,
    a1: RefcountMapIter<Vec<u8>, T1>,
) -> bool {
    a0 != a1
}

// The container type.  The KEY IS CONCRETE -- Vec<u8>, what rules/stringref maps
// llvm::StringRef to in this model -- because StringMap's key is not a template
// parameter; only the value is generic.
fn t3<T1>() -> BTreeMap<Vec<u8>, Value<T1>> {
    BTreeMap::new()
}

fn f5<T1>() -> BTreeMap<Vec<u8>, Value<T1>> {
    BTreeMap::new()
}

// operator[] INSERTS a default-constructed value for an absent key --
// StringMap.h:277 is `try_emplace(Key).first->second`. Same body as
// rules/densemap f7.
fn f6<T1: Default + 'static>(
    a0: Ptr<BTreeMap<Vec<u8>, Value<T1>>>,
    a1: Vec<u8>,
) -> Ptr<T1> {
    a0.with_mut(|__v: &mut BTreeMap<Vec<u8>, Value<T1>>| {
        __v.entry(a1)
            .or_insert_with(|| Rc::new(RefCell::new(<T1>::default())))
            .as_pointer()
    })
}

fn f7<T1: 'static>(
    a0: Ptr<BTreeMap<Vec<u8>, Value<T1>>>,
    a1: Vec<u8>,
) -> RefcountMapIter<Vec<u8>, T1> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f8<T1: 'static>(a0: Ptr<BTreeMap<Vec<u8>, Value<T1>>>) -> RefcountMapIter<Vec<u8>, T1> {
    RefcountMapIter::begin(a0)
}

fn f9<T1: 'static>(a0: Ptr<BTreeMap<Vec<u8>, Value<T1>>>) -> RefcountMapIter<Vec<u8>, T1> {
    RefcountMapIter::end(a0)
}

// The const-receiver forms -- same bodies, different matched signature.
fn f10<T1: 'static>(
    a0: Ptr<BTreeMap<Vec<u8>, Value<T1>>>,
    a1: Vec<u8>,
) -> RefcountMapIter<Vec<u8>, T1> {
    RefcountMapIter::find_key(a0, &a1)
}

fn f11<T1: 'static>(a0: Ptr<BTreeMap<Vec<u8>, Value<T1>>>) -> RefcountMapIter<Vec<u8>, T1> {
    RefcountMapIter::begin(a0)
}

fn f12<T1: 'static>(a0: Ptr<BTreeMap<Vec<u8>, Value<T1>>>) -> RefcountMapIter<Vec<u8>, T1> {
    RefcountMapIter::end(a0)
}
