// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp: this module KEYS the container that already exists in
// libcc2rs/src/iterators.rs:1432 and adds nothing to libcc2rs.
//
// The bound is `Ord + Clone` because that is the bound on the struct itself
// (iterators.rs:1437) -- NOT `Ord + Copy`; `EquivalenceClasses<mlir::Value>` and
// `<void *>` are instantiated in this corpus and neither is Copy-only.
//
// The initialiser is `EquivalenceClasses::new()`, i.e. LLVM's
// `EquivalenceClasses()` default ctor (header:107): three empty maps/vectors.
// `Default for EquivalenceClasses` (iterators.rs:1447) forwards to exactly this.
//
// There is no tgt_refcount.rs.  This module has exactly ONE type key, its body
// contains no raw-pointer text and no `as_pointer()`, and placeholder access
// modes are expanded model-awarely, so the unsafe base layer (ir_unsafe.json is
// the unconditional base for BOTH models) is correct under refcount too.  A
// tgt_refcount.rs whose body would be byte-identical is omitted rather than
// restated -- and omitting the FILE is safe, whereas omitting a tN from a
// tgt_refcount.rs that exists is the rules/iostream t1 load abort.

fn t1<T1: Ord + Clone>() -> libcc2rs::EquivalenceClasses<T1> {
    libcc2rs::EquivalenceClasses::new()
}

// `llvm::EquivalenceClasses<T>::member_iterator` -> libcc2rs::MemberIter<T>
// (iterators.rs:1348). No bound: `impl<T> Default for MemberIter<T>`
// (iterators.rs:1354) is unconditional, and C++'s
// `explicit member_iterator() = default` default-constructs to the end sentinel,
// which is exactly what that impl gives (it leaves `Node` uninitialised in C++;
// the only value a default can usefully denote is `member_end()`).
fn t2<T1>() -> libcc2rs::MemberIter<T1> {
    libcc2rs::MemberIter::default()
}

// `llvm::EquivalenceClasses<T>::ECValue` -> libcc2rs::ECValue<T>
// (iterators.rs:1316). There is NO default initialiser to give: LLVM's ECValue
// declares only `ECValue(const ElemTy &Data)` and `ECValue(const ECValue &)`
// (EquivalenceClasses.h:76-80), so a default-init of an ECValue is ill-formed
// C++ and cannot occur in any faithfully translated TU. Every use in this corpus
// obtains one from `*I` / `insert()`. A `panic!` is therefore the only honest
// body -- loud if anything ever reaches it -- and it is `!`, so it needs no
// bound and constructs nothing. (libcc2rs::ECValue has private fields and no
// public ctor; giving it a Default would need a libcc2rs change plus a
// target_preprocessor re-pin, and an unreachable initialiser does not justify
// one.)
fn t3<T1>() -> libcc2rs::ECValue<T1> {
    panic!("ub: llvm::EquivalenceClasses<T>::ECValue has no default constructor")
}

// ============================================================================
// MEMBER (FUNCTION) KEY TARGETS -- see src.cpp for the measurement that motivated
// them and for the per-key header citations and refusals.  Every body is a single
// call into libcc2rs/src/iterators.rs:1453-1611; nothing is modelled here.
//
// ALL RETURNS ARE BY VALUE.  `MemberIter<T>` is a snapshot (`chain: Vec<T>, idx`),
// `get_leader_value` clones out of the BTreeMap, `is_leader` is a bool, `insert`
// returns an `ECValue<T>` snapshot.  That is what makes these landable rather than
// a dangling-reference refusal under the refcount model.
//
// STILL NO tgt_refcount.rs, for the same reason the type keys have none (and the
// same reason rules/apint has none): no body below contains raw-pointer text or
// `as_pointer()`, the bodies are model-independent, and ir_unsafe.json is the
// unconditional base for BOTH models.  Adding a byte-identical tgt_refcount.rs
// would only create a second place to forget a key -- which is the rules/iostream
// t1 load abort.  Omitting the FILE is safe; omitting an fN from a
// tgt_refcount.rs that EXISTS is not.

// `EC.findLeader(V)` -- llvm::EquivalenceClasses<T>::find_leader (iterators.rs:1541).
unsafe fn f1<T1: Ord + Clone>(
    a0: &libcc2rs::EquivalenceClasses<T1>,
    a1: &T1,
) -> libcc2rs::MemberIter<T1> {
    a0.find_leader(a1.clone())
}

// `EC.findLeader(ECV)` -- the ECValue overload; Rust cannot overload, so it lands on
// the separately-named find_leader_of (iterators.rs:1552).
unsafe fn f2<T1: Ord + Clone>(
    a0: &libcc2rs::EquivalenceClasses<T1>,
    a1: &libcc2rs::ECValue<T1>,
) -> libcc2rs::MemberIter<T1> {
    a0.find_leader_of(a1)
}

// `EC.unionSets(L1, L2)` -- the member_iterator overload (iterators.rs:1587).
// L1 survives as leader and L2's chain is appended, so the returned iterator is
// L1's, matching header:302-311 and the `updated_leader == it_A` test.
unsafe fn f3<T1: Ord + Clone>(
    a0: &mut libcc2rs::EquivalenceClasses<T1>,
    a1: libcc2rs::MemberIter<T1>,
    a2: libcc2rs::MemberIter<T1>,
) -> libcc2rs::MemberIter<T1> {
    a0.union_sets_iters(a1, a2)
}

// `EC.member_begin(ECV)` (iterators.rs:1519). A non-leader yields the end sentinel,
// exactly as C++ passes nullptr.
unsafe fn f4<T1: Ord + Clone>(
    a0: &libcc2rs::EquivalenceClasses<T1>,
    a1: &libcc2rs::ECValue<T1>,
) -> libcc2rs::MemberIter<T1> {
    a0.member_begin(a1)
}

// `EC.member_end()` (iterators.rs:1528) -- an empty chain, which MemberIter's
// PartialEq makes equal to any exhausted iterator, as two null Nodes are in C++.
unsafe fn f5<T1: Ord + Clone>(
    a0: &libcc2rs::EquivalenceClasses<T1>,
) -> libcc2rs::MemberIter<T1> {
    a0.member_end()
}

// `EC.getLeaderValue(V)` (iterators.rs:1560). C++ returns `const ElemTy &`; this
// returns T BY VALUE -- a reference into a union-find forest is invalidated by
// unionSets, so the value return is the honest one. Panics where C++ asserts.
unsafe fn f6<T1: Ord + Clone>(a0: &libcc2rs::EquivalenceClasses<T1>, a1: &T1) -> T1 {
    a0.get_leader_value(a1.clone())
}

// `ECV.isLeader()` (iterators.rs:1327) -- member of the nested ECValue (t3).
unsafe fn f7<T1: Clone>(a0: &libcc2rs::ECValue<T1>) -> bool {
    a0.is_leader()
}

// `EC.insert(V)` (iterators.rs:1464) -- idempotent. Returns a SNAPSHOT ECValue, not
// C++'s `const ECValue &`; see the caveat in src.cpp. The corpus's only caller
// discards the result.
unsafe fn f8<T1: Ord + Clone>(
    a0: &mut libcc2rs::EquivalenceClasses<T1>,
    a1: &T1,
) -> libcc2rs::ECValue<T1> {
    a0.insert(a1.clone())
}

// `EC.empty()` -> is_empty (iterators.rs:1478); `empty` is a Rust verb clash, hence
// the rename, which is why a textual emission of `empty()` resolves to nothing.
unsafe fn f9<T1: Ord + Clone>(a0: &libcc2rs::EquivalenceClasses<T1>) -> bool {
    a0.is_empty()
}

// ============================================================================
// OPERATOR KEY TARGETS ON member_iterator (t2 -> libcc2rs::MemberIter<T>).
// Nothing is added to libcc2rs: every trait used below already exists.
// STILL NO tgt_refcount.rs -- no body here contains raw-pointer text or
// `as_pointer()`, every body is a single model-independent libcc2rs call, and
// ir_unsafe.json is the unconditional base for BOTH models.  Adding a
// byte-identical tgt_refcount.rs would only create a second place to forget a key,
// which is the rules/iostream t1 load abort.

// `MI1 == MI2` -- MemberIter's PartialEq (iterators.rs:1366) compares the CURRENT
// ELEMENT, so an exhausted iterator and member_end() both read None and compare
// equal (as two null Nodes do), while elements being unique in the forest makes
// element equality the same predicate as C++ node-address equality.
unsafe fn f10<T1: Clone + PartialEq>(
    a0: &libcc2rs::MemberIter<T1>,
    a1: &libcc2rs::MemberIter<T1>,
) -> bool {
    a0 == a1
}

unsafe fn f11<T1: Clone + PartialEq>(
    a0: &libcc2rs::MemberIter<T1>,
    a1: &libcc2rs::MemberIter<T1>,
) -> bool {
    a0 != a1
}

// prefix ++ -- PrefixInc (iterators.rs:1407), which delegates to next_member() and
// asserts rather than walking off the end (C++ derefs a null Node there).
unsafe fn f12<T1: Clone>(a0: &mut libcc2rs::MemberIter<T1>) -> libcc2rs::MemberIter<T1> {
    libcc2rs::PrefixInc::prefix_inc(a0)
}

// postfix ++ yields the OLD position -- PostfixInc (iterators.rs:1413).
// The C++ side's dummy `int` disambiguator has no target parameter, exactly as
// rules/deque_iterator f8 omits it.
unsafe fn f13<T1: Clone>(a0: &mut libcc2rs::MemberIter<T1>) -> libcc2rs::MemberIter<T1> {
    libcc2rs::PostfixInc::postfix_inc(a0)
}

// `*MI` -- at() (iterators.rs:1393). Panics on the end iterator, matching C++'s
// `assert(Node != nullptr && "Dereferencing end()!")`.
unsafe fn f14<T1: Clone>(a0: &libcc2rs::MemberIter<T1>) -> T1 {
    a0.at()
}
