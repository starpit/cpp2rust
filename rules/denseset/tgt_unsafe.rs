// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp.  DenseSet reuses the representation rules/set gives std::set and
// rules/smallset gives SmallSet -- BTreeMap<T1, Box<T1>>, the set-as-map-keyed-
// on-its-own-element shape, with UnsafeMapIterator<T1, T1> as the iterator.  No
// new runtime type: the existing PartialEq (position by key, end() = None) is
// what DenseSet.h:144's `LHS.I == RHS.I` MEANS for a deduplicating container,
// and src.cpp argues that rather than assuming it.
//
// DenseSet's ValueInfoT DOES NOT APPEAR HERE, and neither does the bucket
// DenseMap: they are hashing policy and storage layout, not observable through
// the surface mapped here, the same way rules/smallset drops SmallSet's N.
//
// ORDER.  BTreeMap walks in key order; DenseSet leaves its order unspecified
// (it is a hash table), so this is an order-REFINEMENT, as rules/densemap
// records for DenseMap.  The probe prints a sorted signature so no evidence
// here depends on the refinement.

use libcc2rs::*;
use std::collections::BTreeMap;

fn t1<T1>() -> BTreeMap<T1, Box<T1>> {
    BTreeMap::new()
}

fn t2<T1: Clone + Ord>() -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::null()
}

fn t3<T1: Clone + Ord>() -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::null()
}

unsafe fn f1<T1>() -> BTreeMap<T1, Box<T1>> {
    BTreeMap::new()
}

// The reserve-taking constructor.  The argument is a bucket-count HINT and is
// dropped: BTreeMap has no capacity to reserve, and a hint is not observable.
// rules/densemap's f14 drops DenseMap's InitialReserve the same way.
unsafe fn f2<T1>(a0: u32) -> BTreeMap<T1, Box<T1>> {
    let _ = a0;
    BTreeMap::new()
}

// insert(const T1 &) -- PROBE-AND-INSERT, not BTreeMap::insert, and that is the
// whole contract at the three abort sites.  C++ insert KEEPS the incumbent and
// reports false when the element is already present (DenseSet.h:198 forwards to
// DenseMap::try_emplace); BTreeMap::insert would OVERWRITE the stored value and
// report the old one.  rules/set's f11 and rules/smallset's f5 are this body.
unsafe fn f3<T1: Ord + Clone>(
    a0: &mut BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> (UnsafeMapIterator<T1, T1>, bool) {
    let __inserted = !a0.contains_key(&a1);
    if __inserted {
        a0.insert(a1.clone(), Box::new(a1.clone()));
    }
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T1>>, &a1),
        __inserted,
    )
}

// insert(T1 &&) -- same contract, separate signature (src.cpp says why).
unsafe fn f4<T1: Ord + Clone>(
    a0: &mut BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> (UnsafeMapIterator<T1, T1>, bool) {
    let __inserted = !a0.contains_key(&a1);
    if __inserted {
        a0.insert(a1.clone(), Box::new(a1.clone()));
    }
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T1>>, &a1),
        __inserted,
    )
}

unsafe fn f5<T1: Ord>(a0: BTreeMap<T1, Box<T1>>, a1: T1) -> bool {
    a0.contains_key(&a1)
}

// count returns DenseSet's size_type, `unsigned` -> u32, and is 1 or 0 only
// (DenseSet.h:176 is documented "Return 1 if the specified key is in the set, 0
// otherwise").  rules/densemap's f8 is the same body for DenseMap.
unsafe fn f6<T1: Ord>(a0: BTreeMap<T1, Box<T1>>, a1: T1) -> u32 {
    if a0.contains_key(&a1) {
        1_u32
    } else {
        0_u32
    }
}

unsafe fn f7<T1>(a0: BTreeMap<T1, Box<T1>>) -> u32 {
    a0.len() as u32
}

unsafe fn f8<T1>(a0: BTreeMap<T1, Box<T1>>) -> bool {
    a0.is_empty()
}

unsafe fn f9<T1: Ord + Clone>(a0: &mut BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::begin(&*a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f10<T1: Ord + Clone>(a0: &mut BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::end(&*a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f11<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::begin(&a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f12<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::end(&a0 as *const BTreeMap<T1, Box<T1>>)
}

// find -- an iterator to the element, or end() when absent.  find_key already
// has exactly that contract (iterators.rs:86), which is why rules/set's f5 and
// rules/densemap's f3 are the same one-liner.
unsafe fn f13<T1: Ord + Clone>(
    a0: &mut BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T1>>, &a1)
}

unsafe fn f14<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>, a1: T1) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::find_key(&a0 as *const BTreeMap<T1, Box<T1>>, &a1)
}

// The four comparisons.  POSITION IDENTITY, which for a deduplicating set is the
// same relation as key identity -- see src.cpp.
unsafe fn f15<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 == a1
}

unsafe fn f16<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 != a1
}

unsafe fn f17<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 == a1
}

unsafe fn f18<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 != a1
}

// operator* -- the ELEMENT.  In this representation the element is stored as
// both the key and the value, and `second()` is the pointer into the stored
// value, which is what rules/set's f18 yields for `*it` on a std::set iterator.
unsafe fn f19<T1: Ord + Clone>(a0: UnsafeMapIterator<T1, T1>) -> *mut T1 {
    a0.second()
}

unsafe fn f20<T1: Ord + Clone>(a0: UnsafeMapIterator<T1, T1>) -> *const T1 {
    a0.second()
}

// PREFIX ++ -- advance and yield the advanced iterator, rules/set's f20.
unsafe fn f21<T1: Ord + Clone>(
    a0: &mut UnsafeMapIterator<T1, T1>,
) -> UnsafeMapIterator<T1, T1> {
    a0.inc();
    a0.clone()
}

unsafe fn f22<T1: Ord + Clone>(
    a0: &mut UnsafeMapIterator<T1, T1>,
) -> UnsafeMapIterator<T1, T1> {
    a0.inc();
    a0.clone()
}

// FAMILY TWO -- see src.cpp.  Keyed on the two-argument
// `llvm::DenseMapInfo<T1, void>` spelling that Autopilot.cpp asks for.  The
// key-info argument is hashing policy and is not represented on this side at
// all, so these bodies are f1..f22 verbatim.

fn t4<T1>() -> BTreeMap<T1, Box<T1>> {
    BTreeMap::new()
}

fn t5<T1: Clone + Ord>() -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::null()
}

fn t6<T1: Clone + Ord>() -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::null()
}

unsafe fn f23<T1>() -> BTreeMap<T1, Box<T1>> {
    BTreeMap::new()
}

// The reserve-taking constructor.  The argument is a bucket-count HINT and is
// dropped: BTreeMap has no capacity to reserve, and a hint is not observable.
// rules/densemap's f36 drops DenseMap's InitialReserve the same way.
unsafe fn f24<T1>(a0: u32) -> BTreeMap<T1, Box<T1>> {
    let _ = a0;
    BTreeMap::new()
}

// insert(const T1 &) -- PROBE-AND-INSERT, not BTreeMap::insert, and that is the
// whole contract at the three abort sites.  C++ insert KEEPS the incumbent and
// reports false when the element is already present (DenseSet.h:198 forwards to
// DenseMap::try_emplace); BTreeMap::insert would OVERWRITE the stored value and
// report the old one.  rules/set's f33 and rules/smallset's f27 are this body.
unsafe fn f25<T1: Ord + Clone>(
    a0: &mut BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> (UnsafeMapIterator<T1, T1>, bool) {
    let __inserted = !a0.contains_key(&a1);
    if __inserted {
        a0.insert(a1.clone(), Box::new(a1.clone()));
    }
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T1>>, &a1),
        __inserted,
    )
}

// insert(T1 &&) -- same contract, separate signature (src.cpp says why).
unsafe fn f26<T1: Ord + Clone>(
    a0: &mut BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> (UnsafeMapIterator<T1, T1>, bool) {
    let __inserted = !a0.contains_key(&a1);
    if __inserted {
        a0.insert(a1.clone(), Box::new(a1.clone()));
    }
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T1>>, &a1),
        __inserted,
    )
}

unsafe fn f27<T1: Ord>(a0: BTreeMap<T1, Box<T1>>, a1: T1) -> bool {
    a0.contains_key(&a1)
}

// count returns DenseSet's size_type, `unsigned` -> u32, and is 1 or 0 only
// (DenseSet.h:176 is documented "Return 1 if the specified key is in the set, 0
// otherwise").  rules/densemap's f30 is the same body for DenseMap.
unsafe fn f28<T1: Ord>(a0: BTreeMap<T1, Box<T1>>, a1: T1) -> u32 {
    if a0.contains_key(&a1) {
        1_u32
    } else {
        0_u32
    }
}

unsafe fn f29<T1>(a0: BTreeMap<T1, Box<T1>>) -> u32 {
    a0.len() as u32
}

unsafe fn f30<T1>(a0: BTreeMap<T1, Box<T1>>) -> bool {
    a0.is_empty()
}

unsafe fn f31<T1: Ord + Clone>(a0: &mut BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::begin(&*a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f32<T1: Ord + Clone>(a0: &mut BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::end(&*a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f33<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::begin(&a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f34<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::end(&a0 as *const BTreeMap<T1, Box<T1>>)
}

// find -- an iterator to the element, or end() when absent.  find_key already
// has exactly that contract (iterators.rs:86), which is why rules/set's f27 and
// rules/densemap's f25 are the same one-liner.
unsafe fn f35<T1: Ord + Clone>(
    a0: &mut BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T1>>, &a1)
}

unsafe fn f36<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>, a1: T1) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::find_key(&a0 as *const BTreeMap<T1, Box<T1>>, &a1)
}

// The four comparisons.  POSITION IDENTITY, which for a deduplicating set is the
// same relation as key identity -- see src.cpp.
unsafe fn f37<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 == a1
}

unsafe fn f38<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 != a1
}

unsafe fn f39<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 == a1
}

unsafe fn f40<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 != a1
}

// operator* -- the ELEMENT.  In this representation the element is stored as
// both the key and the value, and `second()` is the pointer into the stored
// value, which is what rules/set's f40 yields for `*it` on a std::set iterator.
unsafe fn f41<T1: Ord + Clone>(a0: UnsafeMapIterator<T1, T1>) -> *mut T1 {
    a0.second()
}

unsafe fn f42<T1: Ord + Clone>(a0: UnsafeMapIterator<T1, T1>) -> *const T1 {
    a0.second()
}

// PREFIX ++ -- advance and yield the advanced iterator, rules/set's f42.
unsafe fn f43<T1: Ord + Clone>(
    a0: &mut UnsafeMapIterator<T1, T1>,
) -> UnsafeMapIterator<T1, T1> {
    a0.inc();
    a0.clone()
}

unsafe fn f44<T1: Ord + Clone>(
    a0: &mut UnsafeMapIterator<T1, T1>,
) -> UnsafeMapIterator<T1, T1> {
    a0.inc();
    a0.clone()
}

// FAMILY THREE -- llvm::SmallDenseSet, see src.cpp.  The inline bucket count is
// an ALLOCATION STRATEGY and is dropped, exactly as rules/smallvector drops
// SmallVector's N: the non-type parameter appears in the C++ key as `_` and not
// at all on this side, so these functions take T1 alone.  Bodies are f1..f22's
// verbatim apart from the two members family three adds.

fn t7<T1>() -> BTreeMap<T1, Box<T1>> {
    BTreeMap::new()
}

fn t8<T1>() -> BTreeMap<T1, Box<T1>> {
    BTreeMap::new()
}

fn t9<T1: Clone + Ord>() -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::null()
}

fn t10<T1: Clone + Ord>() -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::null()
}

unsafe fn f45<T1>() -> BTreeMap<T1, Box<T1>> {
    BTreeMap::new()
}

unsafe fn f46<T1>(a0: u32) -> BTreeMap<T1, Box<T1>> {
    let _ = a0;
    BTreeMap::new()
}

// The COPY CONSTRUCTOR.  A DEEP copy: BTreeMap's derived Clone clones every
// Box<T1>, i.e. every stored element, which is what a C++ copy of a value-typed
// set is.  `SmallDenseSet<unsigned> new_clique(cur_clique)` at
// GraphStats.cpp:81 must not alias cur_clique -- it is inserted into immediately
// after.
unsafe fn f47<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>) -> BTreeMap<T1, Box<T1>> {
    a0.clone()
}

unsafe fn f48<T1>() -> BTreeMap<T1, Box<T1>> {
    BTreeMap::new()
}

unsafe fn f49<T1>(a0: u32) -> BTreeMap<T1, Box<T1>> {
    let _ = a0;
    BTreeMap::new()
}

unsafe fn f50<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>) -> BTreeMap<T1, Box<T1>> {
    a0.clone()
}

unsafe fn f51<T1: Ord + Clone>(
    a0: &mut BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> (UnsafeMapIterator<T1, T1>, bool) {
    let __inserted = !a0.contains_key(&a1);
    if __inserted {
        a0.insert(a1.clone(), Box::new(a1.clone()));
    }
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T1>>, &a1),
        __inserted,
    )
}

unsafe fn f52<T1: Ord + Clone>(
    a0: &mut BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> (UnsafeMapIterator<T1, T1>, bool) {
    let __inserted = !a0.contains_key(&a1);
    if __inserted {
        a0.insert(a1.clone(), Box::new(a1.clone()));
    }
    (
        UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T1>>, &a1),
        __inserted,
    )
}

// erase(key) -- DenseSet.h:180 returns true iff an element was removed, which is
// exactly BTreeMap::remove's Option being Some.
unsafe fn f53<T1: Ord>(a0: &mut BTreeMap<T1, Box<T1>>, a1: T1) -> bool {
    a0.remove(&a1).is_some()
}

unsafe fn f54<T1: Ord>(a0: BTreeMap<T1, Box<T1>>, a1: T1) -> bool {
    a0.contains_key(&a1)
}

unsafe fn f55<T1: Ord>(a0: BTreeMap<T1, Box<T1>>, a1: T1) -> u32 {
    if a0.contains_key(&a1) {
        1_u32
    } else {
        0_u32
    }
}

unsafe fn f56<T1>(a0: BTreeMap<T1, Box<T1>>) -> u32 {
    a0.len() as u32
}

unsafe fn f57<T1>(a0: BTreeMap<T1, Box<T1>>) -> bool {
    a0.is_empty()
}

unsafe fn f58<T1: Ord + Clone>(a0: &mut BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::begin(&*a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f59<T1: Ord + Clone>(a0: &mut BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::end(&*a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f60<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::begin(&a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f61<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::end(&a0 as *const BTreeMap<T1, Box<T1>>)
}

unsafe fn f62<T1: Ord + Clone>(
    a0: &mut BTreeMap<T1, Box<T1>>,
    a1: T1,
) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::find_key(&*a0 as *const BTreeMap<T1, Box<T1>>, &a1)
}

unsafe fn f63<T1: Ord + Clone>(a0: BTreeMap<T1, Box<T1>>, a1: T1) -> UnsafeMapIterator<T1, T1> {
    UnsafeMapIterator::find_key(&a0 as *const BTreeMap<T1, Box<T1>>, &a1)
}

unsafe fn f64<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 == a1
}

unsafe fn f65<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 != a1
}

unsafe fn f66<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 == a1
}

unsafe fn f67<T1: PartialEq>(
    a0: UnsafeMapIterator<T1, T1>,
    a1: UnsafeMapIterator<T1, T1>,
) -> bool {
    a0 != a1
}

unsafe fn f68<T1: Ord + Clone>(a0: UnsafeMapIterator<T1, T1>) -> *mut T1 {
    a0.second()
}

unsafe fn f69<T1: Ord + Clone>(a0: UnsafeMapIterator<T1, T1>) -> *const T1 {
    a0.second()
}

unsafe fn f70<T1: Ord + Clone>(
    a0: &mut UnsafeMapIterator<T1, T1>,
) -> UnsafeMapIterator<T1, T1> {
    a0.inc();
    a0.clone()
}

unsafe fn f71<T1: Ord + Clone>(
    a0: &mut UnsafeMapIterator<T1, T1>,
) -> UnsafeMapIterator<T1, T1> {
    a0.inc();
    a0.clone()
}

// llvm::DenseMapInfo<unsigned int, void> is an EMPTY struct -- four statics and
// no members -- so the unit type carries exactly the same (zero) information.
// Same mapping rules/stringmap t4 uses for llvm::EmptyStringSetTag.
unsafe fn t11() -> () {
    ()
}

// numeric_limits<unsigned>::max()
unsafe fn f72() -> u32 {
    u32::MAX
}

// numeric_limits<unsigned>::max() - 1
unsafe fn f73() -> u32 {
    u32::MAX - 1
}

// (unsigned)(Val * 37u) -- unsigned multiply is modular in C++, so wrapping_mul
// is the exact body, not an approximation.
unsafe fn f74(a0: u32) -> u32 {
    a0.wrapping_mul(37)
}

unsafe fn f75(a0: u32, a1: u32) -> bool {
    a0 == a1
}

// See t11/f72..f75: same bodies, ONE-ARGUMENT key.  This is the family the
// emitted code actually resolves against.
unsafe fn t12() -> () {
    ()
}

unsafe fn f76() -> u32 {
    u32::MAX
}

unsafe fn f77() -> u32 {
    u32::MAX - 1
}

unsafe fn f78(a0: u32) -> u32 {
    a0.wrapping_mul(37)
}

unsafe fn f79(a0: u32, a1: u32) -> bool {
    a0 == a1
}
