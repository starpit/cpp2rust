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

use libcc2rs::UnsafeMapIterator;
use std::collections::BTreeMap;

fn t1<T1>() -> UnsafeMapIterator<Vec<libc::c_char>, T1> {
    UnsafeMapIterator::null()
}

fn t2<T1>() -> UnsafeMapIterator<Vec<libc::c_char>, T1> {
    UnsafeMapIterator::null()
}

unsafe fn f1<T1: PartialEq>(
    a0: UnsafeMapIterator<Vec<libc::c_char>, T1>,
    a1: UnsafeMapIterator<Vec<libc::c_char>, T1>,
) -> bool {
    a0 == a1
}

unsafe fn f2<T1: PartialEq>(
    a0: UnsafeMapIterator<Vec<libc::c_char>, T1>,
    a1: UnsafeMapIterator<Vec<libc::c_char>, T1>,
) -> bool {
    a0 != a1
}

unsafe fn f3<T1: PartialEq>(
    a0: UnsafeMapIterator<Vec<libc::c_char>, T1>,
    a1: UnsafeMapIterator<Vec<libc::c_char>, T1>,
) -> bool {
    a0 == a1
}

unsafe fn f4<T1: PartialEq>(
    a0: UnsafeMapIterator<Vec<libc::c_char>, T1>,
    a1: UnsafeMapIterator<Vec<libc::c_char>, T1>,
) -> bool {
    a0 != a1
}

// The container type.  The KEY IS CONCRETE -- Vec<libc::c_char>, what
// rules/stringref maps llvm::StringRef to -- because StringMap's key is not a
// template parameter; only the value is generic.
fn t3<T1>() -> BTreeMap<Vec<libc::c_char>, Box<T1>> {
    BTreeMap::new()
}

// llvm::EmptyStringSetTag is an empty struct: the unit type carries exactly the
// same (zero) information.
fn t4() -> () {
    ()
}

unsafe fn f5<T1>() -> BTreeMap<Vec<libc::c_char>, Box<T1>> {
    BTreeMap::new()
}

// operator[] INSERTS a default-constructed value for an absent key --
// StringMap.h:277 is `try_emplace(Key).first->second`. Same body as
// rules/densemap f7; a get().expect() here would be the `lookup` contract, not
// this one.
unsafe fn f6<T1: Default>(
    a0: &mut BTreeMap<Vec<libc::c_char>, Box<T1>>,
    a1: Vec<libc::c_char>,
) -> &mut T1 {
    a0.entry(a1).or_default().as_mut()
}

unsafe fn f7<T1>(
    a0: &mut BTreeMap<Vec<libc::c_char>, Box<T1>>,
    a1: Vec<libc::c_char>,
) -> UnsafeMapIterator<Vec<libc::c_char>, T1> {
    UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<Vec<libc::c_char>, Box<T1>>, &a1)
}

unsafe fn f8<T1>(
    a0: &mut BTreeMap<Vec<libc::c_char>, Box<T1>>,
) -> UnsafeMapIterator<Vec<libc::c_char>, T1> {
    UnsafeMapIterator::begin(&*a0 as *const BTreeMap<Vec<libc::c_char>, Box<T1>>)
}

unsafe fn f9<T1>(
    a0: &mut BTreeMap<Vec<libc::c_char>, Box<T1>>,
) -> UnsafeMapIterator<Vec<libc::c_char>, T1> {
    UnsafeMapIterator::end(&*a0 as *const BTreeMap<Vec<libc::c_char>, Box<T1>>)
}

// The const-receiver forms -- same bodies, different matched signature.
unsafe fn f10<T1>(
    a0: BTreeMap<Vec<libc::c_char>, Box<T1>>,
    a1: Vec<libc::c_char>,
) -> UnsafeMapIterator<Vec<libc::c_char>, T1> {
    UnsafeMapIterator::find_key(&a0 as *const BTreeMap<Vec<libc::c_char>, Box<T1>>, &a1)
}

unsafe fn f11<T1>(
    a0: BTreeMap<Vec<libc::c_char>, Box<T1>>,
) -> UnsafeMapIterator<Vec<libc::c_char>, T1> {
    UnsafeMapIterator::begin(&a0 as *const BTreeMap<Vec<libc::c_char>, Box<T1>>)
}

unsafe fn f12<T1>(
    a0: BTreeMap<Vec<libc::c_char>, Box<T1>>,
) -> UnsafeMapIterator<Vec<libc::c_char>, T1> {
    UnsafeMapIterator::end(&a0 as *const BTreeMap<Vec<libc::c_char>, Box<T1>>)
}
