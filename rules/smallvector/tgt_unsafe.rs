// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::SmallVector<T, N> -> Vec<T>. The inline capacity is dropped: it is an
// allocation strategy, not observable behaviour, exactly as rules/array drops
// std::array's non-type argument. src.cpp records what that costs (capacity()
// gets no rule) and what was checked before deciding it costs nothing else.
//
// Bodies are the ones rules/vector already proved for the same operations on
// the same representation, so the two containers cannot drift: f1/f2 are
// vector's f2/f3, f3/f4/f5 are its f9/f10/f7, f6/f7 its f13/f17, f9 its f14
// shape, f19/f20 its f107/f108, f23/f24 its f55, f25/f26 its f58.

use libcc2rs::*;

fn t1<T1>() -> Vec<T1> {
    Default::default()
}

fn t2<T1>() -> Vec<T1> {
    Default::default()
}

fn t3<T1>() -> Vec<T1> {
    Default::default()
}

fn t4<T1>() -> Vec<T1> {
    Default::default()
}

fn t5<T1>() -> Vec<T1> {
    Default::default()
}

fn t6<T1>() -> Vec<Vec<T1>> {
    Vec::new()
}

// size()/empty() are keyed on `llvm::SmallVectorBase<unsigned int>`, which
// carries the SIZE type and not the element type, so ONE rule serves every
// instantiation -- and the element type is therefore not bindable here. The
// receiver is spelled `Vec<()>` for exactly that reason: a generic `Vec<T1>`
// is REJECTED at load ("generic T1 declared but missing from src",
// translation_rule.cpp), since T1 appears nowhere in the key, and a plausible
// concrete element type like `Vec<u8>` would be a claim about the receiver that
// is false for every other instantiation. `()` says the element type is
// irrelevant, which is the truth: `len`/`is_empty` read the length and never
// touch an element. The type is not emitted -- the receiver placeholder is a
// `borrow`, and Mapper::GetParamType is only consulted for a POINTER or OBJECT
// receiver -- so nothing downstream can act on the spelling. Same shape as
// rules/vector's f47, whose key is likewise element-free.
unsafe fn f1(a0: Vec<()>) -> usize {
    a0.len()
}

unsafe fn f2(a0: Vec<()>) -> bool {
    a0.is_empty()
}

unsafe fn f3<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    ((a0).first_mut().unwrap())
}

unsafe fn f4<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    ((a0).last_mut().unwrap())
}

// The borrow is explicit -- `&mut (a0)[i]`, which is what rules/vector's `at`
// writes, becomes `&mut (*v)[i]` for a reference receiver and rustc denies that
// (`dangerous_implicit_autorefs`); `&mut *a0` reborrows through the Vec's
// DerefMut to a slice first, and indexing a slice is still bounds-checked, so
// an out-of-range index still panics rather than reading past the end.
unsafe fn f5<T1>(a0: &mut Vec<T1>, a1: usize) -> *mut T1 {
    &mut (&mut *a0)[a1 as usize]
}

unsafe fn f6<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr()
}

unsafe fn f7<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr().add(a0.len())
}

unsafe fn f8<T1>(a0: &mut Vec<T1>) -> *mut T1 {
    a0.as_mut_ptr()
}

// push_back takes T BY VALUE on this branch of SmallVectorTemplateBase, so the
// copy C++ makes has already happened at the call and the element is pushed as
// it arrives -- the same shape as rules/vector's f14, and NOT its f21, which
// clones precisely because its C++ parameter is `const T &`.
unsafe fn f9<T1>(a0: &mut Vec<T1>, a1: T1) {
    a0.push(a1)
}

unsafe fn f10<T1>(a0: &mut Vec<T1>) {
    a0.pop();
}

unsafe fn f11<T1>(a0: &mut Vec<T1>) {
    a0.clear()
}

// append(const SmallVectorImpl<T> &) COPIES the argument's elements onto the
// end of the receiver and leaves the argument alone, so the elements are cloned
// -- moving them out would empty a container the C++ still owns.
unsafe fn f12<T1: Clone>(a0: &mut Vec<T1>, a1: Vec<T1>) {
    a0.extend(a1.iter().cloned())
}

// erase(first, last) removes [first, last) and returns an iterator to the
// element that followed the range, which after the removal is the position
// `first` held -- LLVM returns `(iterator)CS` for exactly that reason.
unsafe fn f14<T1>(a0: &mut Vec<T1>, a1: *const T1, a2: *const T1) -> *mut T1 {
    let first = a1.offset_from(a0.as_ptr()) as usize;
    let last = a2.offset_from(a0.as_ptr()) as usize;
    a0.drain(first..last);
    a0.as_mut_ptr().add(first)
}

unsafe fn f15<T1>() -> Vec<T1> {
    Vec::new()
}

unsafe fn f16<T1>() -> Vec<T1> {
    Vec::new()
}

unsafe fn f17<T1: Clone>(a0: Vec<T1>) -> Vec<T1> {
    a0.clone()
}

unsafe fn f18<T1: Clone>(a0: Vec<T1>) -> Vec<T1> {
    a0.clone()
}

unsafe fn f19<T1>(a0: &mut Vec<T1>) -> Vec<T1> {
    std::mem::take(&mut *a0)
}

unsafe fn f20<T1>(a0: &mut Vec<T1>) -> Vec<T1> {
    std::mem::take(&mut *a0)
}

// A std::initializer_list is already a Vec on this side (rules/initializer_list),
// and the elements are the vector's, so construction from one is the identity --
// rules/vector's f36 for the same C++ constructor.
unsafe fn f21<T1>(a0: Vec<T1>) -> Vec<T1> {
    a0
}

unsafe fn f22<T1>(a0: Vec<T1>) -> Vec<T1> {
    a0
}

unsafe fn f23<T1: Clone>(a0: &mut Vec<T1>, a1: &mut Vec<T1>) {
    *a0 = std::mem::take(&mut *a1)
}

unsafe fn f24<T1: Clone>(a0: &mut Vec<T1>, a1: &mut Vec<T1>) {
    *a0 = std::mem::take(&mut *a1)
}

unsafe fn f25<T1: Clone>(a0: &mut Vec<T1>, a1: Vec<T1>) {
    *a0 = a1.clone()
}

unsafe fn f26<T1: Clone>(a0: &mut Vec<T1>, a1: Vec<T1>) {
    *a0 = a1.clone()
}

// `v = {a, b}` REPLACES the contents, it does not append.
unsafe fn f27<T1>(a0: &mut Vec<T1>, a1: Vec<T1>) {
    *a0 = a1
}

unsafe fn f28<T1>(a0: &mut Vec<T1>, a1: Vec<T1>) {
    *a0 = a1
}
