// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use libcc2rs::*;

fn t1<T1>() -> Vec<T1> {
    Default::default()
}

fn t2<T1>() -> *mut T1 {
    Default::default()
}

fn t3<T1>() -> Vec<Vec<T1>> {
    Vec::new()
}

fn t4<T1>() -> *const T1 {
    Default::default()
}

// `std::vector<T1 *>::const_iterator` == `std::__wrap_iter<T1 *const *>`.  Element type
// is `T1 *` (unsafe model: `*mut T1`), and a const_iterator is t4's `*const <elem>`, so
// this is EXACTLY t4 instantiated at `*mut T1` -- no new representation, no new
// semantics.  Deliberately NOT `*const *const T1`: the CONST in `T1 *const *` binds to
// the POINTER the iterator yields, not to the pointee.
fn t8<T1>() -> *const *mut T1 {
    Default::default()
}

fn t5<T1>() -> Vec<T1> {
    Default::default()
}

#[cfg(target_os = "linux")]
fn t6<T1>() -> *mut T1 {
    Default::default()
}

#[cfg(target_os = "linux")]
fn t7<T1>() -> *const T1 {
    Default::default()
}

unsafe fn f1<T1>(a0: &mut Vec<T1>, a1: *const T1) -> *const T1 {
    let pos = a1.offset_from(a0.as_ptr()) as usize;
    a0.remove(pos);
    a1
}
unsafe fn f2<T1>(a0: Vec<T1>) -> usize {
    a0.len()
}
unsafe fn f3<T1>(a0: Vec<T1>) -> bool {
    a0.is_empty()
}
unsafe fn f4<T1>() -> Vec<T1> {
    Vec::new()
}
unsafe fn f5<T1>(a0: &mut Vec<T1>) {
    a0.pop();
}
unsafe fn f6<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr()
}
unsafe fn f7<T1>(a0: &mut Vec<T1>, a1: usize) -> *mut T1 {
    &mut (a0)[a1 as usize]
}
unsafe fn f8<T1: Default>(a0: usize) -> Vec<T1> {
    (0..(a0) as usize)
        .map(|_| <T1>::default())
        .collect::<Vec<_>>()
}
unsafe fn f9<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    ((a0).first_mut().unwrap())
}
unsafe fn f10<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    ((a0).last_mut().unwrap())
}
unsafe fn f11<T1>(a0: Vec<T1>) -> usize {
    a0.capacity()
}
unsafe fn f12<T1>(a0: &mut Vec<T1>, a1: usize) {
    if a1 as usize > a0.capacity() as usize {
        let len_0 = a0.len();
        a0.reserve_exact(a1 as usize - len_0 as usize);
    }
}
unsafe fn f13<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr()
}
unsafe fn f14<T1: Default>(a0: &mut Vec<T1>, a1: &mut T1) {
    a0.push(std::mem::take(&mut *a1))
}
unsafe fn f15<T1: Default>(a0: &mut Vec<T1>, a1: usize) {
    let __a0 = a1 as usize;
    a0.resize_with(__a0, || <T1>::default())
}
unsafe fn f16<T1>(a0: &mut Vec<T1>) {
    a0.clear()
}
unsafe fn f17<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr().add(a0.len())
}
unsafe fn f18<T1>(a0: &mut Vec<T1>, a1: *const T1, a2: T1) {
    let pos = a1.offset_from(a0.as_ptr()) as usize;
    a0.insert(pos, a2);
}
unsafe fn f19<T1: Default + Clone>(a0: usize, a1: T1) -> Vec<T1> {
    vec![a1; a0 as usize]
}
unsafe fn f20<T1>(a0: &mut Vec<T1>, a1: *const T1, a2: T1) {
    let pos = a1.offset_from(a0.as_ptr()) as usize;
    a0.insert(pos, a2);
}
unsafe fn f21<T1: Clone>(a0: &mut Vec<T1>, a1: T1) {
    let a0_clone = a1.clone();
    a0.push(a0_clone)
}
unsafe fn f22<T1>(a0: *mut T1) -> *mut T1 {
    a0
}
unsafe fn f23<T1>(a0: *const T1) -> *const T1 {
    a0
}
unsafe fn f24<T1>(a0: *const T1) -> *const T1 {
    a0
}
unsafe fn f25<T1>(a0: *mut T1, a1: usize) -> *mut T1 {
    a0.add(a1 as usize)
}
unsafe fn f26<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 != a1
}
unsafe fn f27<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 == a1
}

unsafe fn f28<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.postfix_inc()
}
unsafe fn f29<T1: Clone>(a0: Vec<Vec<T1>>) -> Vec<Vec<T1>> {
    a0.clone()
}

unsafe fn f30<T1: Default + Clone>(a0: usize) -> Vec<Vec<T1>> {
    (0..(a0) as usize)
        .map(|_| <Vec<T1>>::default())
        .collect::<Vec<_>>()
}
unsafe fn f31<T1>(a0: &mut Vec<Vec<T1>>, a1: &mut Vec<T1>) {
    a0.push(std::mem::take(&mut *a1))
}
unsafe fn f32<T1: Default>(a0: &mut Vec<Vec<T1>>, a1: usize) {
    a0.resize_with(a1 as usize, || <Vec<T1>>::default())
}

unsafe fn f33<T1>(a0: *const T1, a1: *const T1) -> isize {
    a0.offset_from(a1)
}

unsafe fn f34<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.prefix_inc()
}

unsafe fn f35<T1: Clone>(a0: *const T1, a1: *const T1) -> Vec<T1> {
    core::slice::from_raw_parts(a0, (a1).offset_from(a0) as usize).to_vec()
}

unsafe fn f36<T1>(a0: Vec<T1>) -> Vec<T1> {
    a0
}

unsafe fn f37<T1: TryFrom<T2>, T2: Clone>(a0: *mut T2, a1: *mut T2) -> Vec<T1> {
    core::slice::from_raw_parts(a0, (a1).offset_from(a0) as usize)
        .iter()
        .map(|x| <T1>::try_from(x.clone()).ok().unwrap())
        .collect()
}

unsafe fn f38(a0: usize, a1: bool) -> Vec<bool> {
    (0..(a0) as usize).map(|_| a1).collect::<Vec<bool>>()
}

unsafe fn f40<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr().add(a0.len())
}

unsafe fn f41<T1>(a0: &mut Vec<T1>) -> *const T1 {
    a0.as_ptr()
}

unsafe fn f42<T1: Ord>(a0: *const T1, a1: *const T1) -> *const T1 {
    core::slice::from_raw_parts(a0, (a1).offset_from(a0) as usize)
        .iter()
        .max()
        .unwrap()
}

unsafe fn f43<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr()
}

unsafe fn f44<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr().add(a0.len())
}

unsafe fn f47(a0: Vec<bool>) -> Vec<bool> {
    a0
}

unsafe fn f48(a0: &mut Vec<bool>, a1: &mut Vec<bool>) {
    std::mem::swap(&mut *a0, &mut *a1)
}

unsafe fn f50<T1: Copy>(a0: &mut Vec<T1>, a1: usize) -> *mut T1 {
    if a1 as usize >= a0.len() {
        panic!("out of bounds access")
    } else {
        (a0).as_mut_ptr().add(a1 as usize)
    }
}

unsafe fn f51<T1>(a0: &Vec<T1>) -> &T1 {
    ((a0).last().unwrap())
}

unsafe fn f52<T1: Clone>(a0: &mut Vec<Vec<T1>>, a1: Vec<T1>) {
    a0.push(a1.clone())
}

unsafe fn f53<T1: Clone>(a0: &mut Vec<T1>, a1: *const T1, a2: *const T1, a3: *const T1) -> *mut T1 {
    let __off = a1.offset_from(a0.as_ptr()) as usize;
    let count = a3.offset_from(a2) as usize;
    a0.splice(
        __off..__off,
        std::slice::from_raw_parts(a2, count).iter().cloned(),
    );
    a0.as_mut_ptr().add(__off)
}

unsafe fn f54<T1: Default + Clone>(a0: &mut Vec<T1>, a1: usize, a2: T1) {
    let __a0 = a1 as usize;
    a0.resize(__a0, a2)
}

unsafe fn f55<T1: Clone>(a0: &mut Vec<T1>, a1: &mut Vec<T1>) {
    let __src = std::mem::take(&mut *a1);
    a0.clear();
    a0.extend(__src);
}

unsafe fn f56<T1>(a0: &mut Vec<Vec<T1>>) -> *mut Vec<T1> {
    ((a0).last_mut().unwrap())
}

unsafe fn f57<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr().add(a0.len())
}

unsafe fn f58<T1: Clone>(a0: &mut Vec<T1>, a1: Vec<T1>) {
    let __src = a1.clone();
    a0.clear();
    a0.extend(__src);
}

unsafe fn f59<T1>(a0: &mut Vec<T1>) {
    a0.shrink_to_fit()
}

unsafe fn f60<T1>(a0: &mut Vec<T1>, a1: *const T1) -> *const T1 {
    let pos = a1.offset_from(a0.as_ptr()) as usize;
    a0.remove(pos);
    a1
}

unsafe fn f61<T1>(a0: Vec<T1>) -> usize {
    a0.len()
}

unsafe fn f62<T1>(a0: Vec<T1>) -> bool {
    a0.is_empty()
}

unsafe fn f63<T1>() -> Vec<T1> {
    Vec::new()
}

unsafe fn f64<T1>(a0: &mut Vec<T1>) {
    a0.pop();
}

unsafe fn f65<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr()
}

unsafe fn f66<T1>(a0: &mut Vec<T1>, a1: usize) -> *mut T1 {
    &mut (a0)[a1 as usize]
}

unsafe fn f67<T1: Default>(a0: usize) -> Vec<T1> {
    (0..(a0) as usize)
        .map(|_| <T1>::default())
        .collect::<Vec<_>>()
}

unsafe fn f68<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    ((a0).first_mut().unwrap())
}

unsafe fn f69<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    ((a0).last_mut().unwrap())
}

unsafe fn f70<T1>(a0: Vec<T1>) -> usize {
    a0.capacity()
}

unsafe fn f71<T1>(a0: &mut Vec<T1>, a1: usize) {
    if a1 as usize > a0.capacity() as usize {
        let len_0 = a0.len();
        a0.reserve_exact(a1 as usize - len_0 as usize);
    }
}

unsafe fn f72<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr()
}

unsafe fn f73<T1: Default>(a0: &mut Vec<T1>, a1: &mut T1) {
    a0.push(std::mem::take(&mut *a1))
}

unsafe fn f74<T1: Default>(a0: &mut Vec<T1>, a1: usize) {
    let __a0 = a1 as usize;
    a0.resize_with(__a0, || <T1>::default())
}

unsafe fn f75<T1>(a0: &mut Vec<T1>) {
    a0.clear()
}

unsafe fn f76<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr().add(a0.len())
}

unsafe fn f77<T1>(a0: &mut Vec<T1>, a1: *const T1, a2: T1) {
    let pos = a1.offset_from(a0.as_ptr()) as usize;
    a0.insert(pos, a2);
}

unsafe fn f78<T1: Default + Clone>(a0: usize, a1: T1) -> Vec<T1> {
    vec![a1; a0 as usize]
}

unsafe fn f79<T1>(a0: &mut Vec<T1>, a1: *const T1, a2: T1) {
    let pos = a1.offset_from(a0.as_ptr()) as usize;
    a0.insert(pos, a2);
}

unsafe fn f80<T1: Clone>(a0: &mut Vec<T1>, a1: T1) {
    let a0_clone = a1.clone();
    a0.push(a0_clone)
}

unsafe fn f81<T1>(a0: *mut T1) -> *mut T1 {
    a0
}

unsafe fn f82<T1>(a0: *const T1) -> *const T1 {
    a0
}

unsafe fn f83<T1>(a0: *const T1) -> *const T1 {
    a0
}

unsafe fn f84<T1>(a0: *mut T1, a1: usize) -> *mut T1 {
    a0.add(a1 as usize)
}

unsafe fn f85<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 != a1
}
unsafe fn f86<T1>(a0: *const T1, a1: *const T1) -> bool {
    a0 == a1
}

unsafe fn f87<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.postfix_inc()
}

unsafe fn f88<T1>(a0: *const T1, a1: *const T1) -> isize {
    a0.offset_from(a1)
}

unsafe fn f89<T1>(a0: &mut *mut T1) -> *mut T1 {
    a0.prefix_inc()
}

unsafe fn f90<T1: Clone>(a0: *const T1, a1: *const T1) -> Vec<T1> {
    core::slice::from_raw_parts(a0, (a1).offset_from(a0) as usize).to_vec()
}

unsafe fn f91<T1>(a0: Vec<T1>) -> Vec<T1> {
    a0
}

unsafe fn f92<T1: TryFrom<T2>, T2: Clone>(a0: *mut T2, a1: *mut T2) -> Vec<T1> {
    core::slice::from_raw_parts(a0, (a1).offset_from(a0) as usize)
        .iter()
        .map(|x| <T1>::try_from(x.clone()).ok().unwrap())
        .collect()
}

unsafe fn f93<T1>(a0: &mut Vec<T1>) -> *const T1 {
    a0.as_ptr()
}

unsafe fn f94<T1: Ord>(a0: *const T1, a1: *const T1) -> *const T1 {
    core::slice::from_raw_parts(a0, (a1).offset_from(a0) as usize)
        .iter()
        .max()
        .unwrap()
}

unsafe fn f95<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr()
}

unsafe fn f96<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr().add(a0.len())
}

unsafe fn f97(a0: &mut Vec<bool>, a1: &mut Vec<bool>) {
    std::mem::swap(&mut *a0, &mut *a1)
}

unsafe fn f98<T1: Copy>(a0: &mut Vec<T1>, a1: usize) -> *mut T1 {
    if a1 as usize >= a0.len() {
        panic!("out of bounds access")
    } else {
        (a0).as_mut_ptr().add(a1 as usize)
    }
}

unsafe fn f99<T1>(a0: &Vec<T1>) -> &T1 {
    ((a0).last().unwrap())
}

unsafe fn f100<T1: Clone>(a0: &mut Vec<Vec<T1>>, a1: Vec<T1>) {
    a0.push(a1.clone())
}

unsafe fn f101<T1: Clone>(
    a0: &mut Vec<T1>,
    a1: *const T1,
    a2: *const T1,
    a3: *const T1,
) -> *mut T1 {
    let __off = a1.offset_from(a0.as_ptr()) as usize;
    let count = a3.offset_from(a2) as usize;
    a0.splice(
        __off..__off,
        std::slice::from_raw_parts(a2, count).iter().cloned(),
    );
    a0.as_mut_ptr().add(__off)
}

unsafe fn f102<T1: Default + Clone>(a0: &mut Vec<T1>, a1: usize, a2: T1) {
    let __a0 = a1 as usize;
    a0.resize(__a0, a2)
}

unsafe fn f103<T1: Clone>(a0: &mut Vec<T1>, a1: &mut Vec<T1>) {
    let __src = std::mem::take(&mut *a1);
    a0.clear();
    a0.extend(__src);
}

unsafe fn f104<T1>(a0: Vec<T1>) -> *const T1 {
    a0.as_ptr().add(a0.len())
}

unsafe fn f105<T1: Clone>(a0: &mut Vec<T1>, a1: Vec<T1>) {
    let __src = a1.clone();
    a0.clear();
    a0.extend(__src);
}

unsafe fn f106<T1>(a0: &mut Vec<T1>) {
    a0.shrink_to_fit()
}

unsafe fn f107<T1>(a0: &mut Vec<T1>) -> Vec<T1> {
    std::mem::take(&mut *a0)
}

unsafe fn f108<T1>(a0: &mut Vec<T1>) -> Vec<T1> {
    std::mem::take(&mut *a0)
}

unsafe fn f109<T1: Clone>(a0: Vec<T1>) -> Vec<T1> {
    a0.clone()
}

unsafe fn f110<T1: Clone>(a0: Vec<T1>) -> Vec<T1> {
    a0.clone()
}

unsafe fn f111<T1: Clone>(a0: &mut Vec<Vec<T1>>, a1: &mut Vec<Vec<T1>>) {
    let __src = std::mem::take(&mut *a1);
    a0.clear();
    a0.extend(__src);
}

unsafe fn f112<T1>(a0: &mut Vec<T1>, init: T1) {
    let __init = init;
    a0.push(__init)
}

unsafe fn f113<T1>(a0: &mut Vec<Vec<T1>>, init: Vec<T1>) {
    let __init = init;
    a0.push(__init)
}

unsafe fn f114<T1>(a0: &mut Vec<T1>, init: T1) {
    let __init = init;
    a0.push(__init)
}

unsafe fn f115<T1: PartialEq>(a0: &Vec<T1>, a1: &Vec<T1>) -> bool {
    a0 == a1
}
unsafe fn f116<T1: PartialEq>(a0: &Vec<T1>, a1: &Vec<T1>) -> bool {
    a0 != a1
}
unsafe fn f117<T1: PartialEq>(a0: &Vec<T1>, a1: &Vec<T1>) -> bool {
    a0 == a1
}
unsafe fn f118<T1: PartialEq>(a0: &Vec<T1>, a1: &Vec<T1>) -> bool {
    a0 != a1
}

unsafe fn f119<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr().add(a0.len())
}

unsafe fn f120<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr()
}

// f121 -- `it += n` MUTATES the receiver and returns it; f34 (prefix ++) is the
// same shape.  f122 -- `it - n` is pure and moves BACKWARD, so the offset is
// negated; a body using `+n` would silently read from the wrong end.
// ⛔⛔ f121 IS SEQUENCED THROUGH `__p` AND STORES VIA
// `drop(core::mem::replace(a0, ..))`, NOT `*a0 = (*a0).offset(..)`, 2026-09-28.
// Same swallowed-deref class as rules/mlir (0a14e018), and this site swallowed on
// BOTH SIDES of the assignment.
// A placeholder whose rule parameter is `&mut T` and which sits in a NON-RECEIVER
// position is emitted as `&mut <place>` -- converter.cpp:8030-8038,
// `needs_lvalue() && needs_mut_borrow()` returns `"&mut " + place`, gated on
// `needs_explicit_mut_borrow = !is_method_call_receiver && ParamIsMutRef(..)`.
// The rule's leading `*` is SWALLOWED by the preprocessor, never emitted, so the
// assignment target emitted `&mut <iter-lvalue> = ..;` --
// `error[E0070]: invalid left-hand side of assignment`.
// ⭐ THE CLASS rc=0 CANNOT SEE: E0070 is raised in HIR lowering / type-check, not
// in the parser.  `rustfmt` -- the only Rust parser in this pipeline -- parses
// `&mut x = v` at rc=0 and re-emits it verbatim, so the TU reports clean.
// ⚠️ The `(*a0)` RECEIVER also recorded `access: "borrow_mut"` (ir_unsafe.json),
// emitting `(&mut place).offset(..)`, which compiles only by autoderef; the twin
// site rules/deque_iterator f18 recorded `"move"` from character-identical source.
// So the deref is removed there too: `a0.offset(..)` is a METHOD-CALL RECEIVER,
// and for a receiver the converter suppresses the explicit borrow and lets Rust's
// autoref supply it.  That is the spelling f120/f122 and the refcount overlay
// already use.
// ⭐ THE FIX RELIES ON THE SWALLOW, DELIBERATELY: bare `a0` in the `replace`
// argument records the IDENTICAL `{arg 0, access: "borrow_mut"}`, so it emits
// `replace(&mut <place>, __p)` -- exactly what `core::mem::replace` wants.
// ⚠️ DO NOT "restore" the `*`: both `replace(&mut *a0, __p)` and
// `replace(*a0, __p)` make rule-preprocessor ABORT with
// `semantic.rs:260: unresolved access="unknown"` -- a deref or re-borrow of a
// placeholder inside a CALL ARGUMENT has no access classification.
// ⚠️ Sequencing through `__p` also keeps this overlay on the SAME shape as the
// refcount one, where it is load-bearing for the "RefCell already borrowed"
// reason documented on the refcount f121 below.
// `*mut T1` is `Copy`, so `replace` is trivially well-typed and the `drop` is a
// no-op, kept for correspondence with the C++ assignment.  Returning `__p` is
// identical to the old trailing `*a0`: the return type is `*mut T1` BY VALUE.
unsafe fn f121<T1>(a0: &mut *mut T1, a1: i64) -> *mut T1 {
    let __p: *mut T1 = a0.offset(a1 as isize);
    drop(core::mem::replace(a0, __p));
    __p
}

unsafe fn f122<T1>(a0: *mut T1, a1: i64) -> *mut T1 {
    a0.offset(-(a1 as isize))
}

// f123..f126 -- relational comparison of two `std::vector<T1>::iterator`.  In the
// unsafe model the iterator IS the raw element pointer, so this is the raw-pointer
// comparison C++ performs.  Defined only for two iterators into the same vector,
// which is also all C++ defines.
unsafe fn f123<T1>(a0: *mut T1, a1: *mut T1) -> bool {
    a0 >= a1
}

unsafe fn f124<T1>(a0: *mut T1, a1: *mut T1) -> bool {
    a0 <= a1
}

unsafe fn f125<T1>(a0: *mut T1, a1: *mut T1) -> bool {
    a0 < a1
}

unsafe fn f126<T1>(a0: *mut T1, a1: *mut T1) -> bool {
    a0 > a1
}

// f127..f136 -- pointer-mono siblings of f43/f44/f57/f41/f24/f95/f96/f104/f93/f83.
// Bodies COPIED verbatim from the originals, at element type `*mut T1` (the unsafe
// model's spelling of `T1 *`, exactly as t8 does).  No new representation.
unsafe fn f127<T1>(a0: Vec<*mut T1>) -> *const *mut T1 {
    a0.as_ptr()
}

unsafe fn f128<T1>(a0: Vec<*mut T1>) -> *const *mut T1 {
    a0.as_ptr().add(a0.len())
}

unsafe fn f129<T1>(a0: Vec<*mut T1>) -> *const *mut T1 {
    a0.as_ptr().add(a0.len())
}

unsafe fn f130<T1>(a0: &mut Vec<*mut T1>) -> *const *mut T1 {
    a0.as_ptr()
}

unsafe fn f131<T1>(a0: *const *mut T1) -> *const *mut T1 {
    a0
}

unsafe fn f132<T1>(a0: Vec<*mut T1>) -> *const *mut T1 {
    a0.as_ptr()
}

unsafe fn f133<T1>(a0: Vec<*mut T1>) -> *const *mut T1 {
    a0.as_ptr().add(a0.len())
}

unsafe fn f134<T1>(a0: Vec<*mut T1>) -> *const *mut T1 {
    a0.as_ptr().add(a0.len())
}

unsafe fn f135<T1>(a0: &mut Vec<*mut T1>) -> *const *mut T1 {
    a0.as_ptr()
}

unsafe fn f136<T1>(a0: *const *mut T1) -> *const *mut T1 {
    a0
}
