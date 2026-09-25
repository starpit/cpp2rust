// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// See src.cpp.  EquivalenceClasses<T1> is a FLAT LEADER MAP, BTreeMap<T1, T1>
// mapping every inserted element to the leader of its class, and
// member_iterator is Option<T1> -- the leader value, with None standing for
// member_end().
//
// WHY A FLAT MAP AND NOT A PARENT/PATH-COMPRESSION FOREST.  Both are plausible;
// this one is chosen because TRANSITIVITY is structural rather than emergent.
// unionSets rewrites EVERY member of one class in one pass, so after
// unionSets(1,2) and unionSets(2,3) the entry for 3 already names 1 -- there is
// no chain left to walk and no way for a missing compression step to leave 1
// and 3 in different classes.  A parent forest gets the same answer only if
// every read resolves the chain correctly, which is a per-body obligation this
// shape does not have.
//
// WHICH ELEMENT LEADS.  EquivalenceClasses.h:306..:322 unconditionally makes the
// FIRST argument's leader the merged leader (it clears L2's leader flag, sets
// `L2LV.Leader = &L1LV`, and returns L1); there is no union-by-rank.  f2 below
// reproduces that: the l2 members are rewritten to l1 and l1 is returned.  No
// rule EXPOSES a leader value, though -- getLeaderValue and member_iterator's
// operator* are deliberately unmapped (src.cpp says why) -- so nothing
// observable here depends on that reproduction being right.  What IS observable
// is which elements share a leader, and that is what the probe checks.
//
// WHAT THIS REPRESENTATION CANNOT EXPRESS, restated from src.cpp because it is
// the point: no stable address per element, hence no `const ElemTy &` return
// (getLeaderValue, getOrInsertLeaderValue, *MI), and no ECValue identity, hence
// no `iterator`, no `begin()/end()`, no `isLeader()`, no member walk.  Those are
// absent from src.cpp and stay loud aborts.

use std::collections::BTreeMap;

fn t1<T1>() -> BTreeMap<T1, T1> {
    BTreeMap::new()
}

fn t2<T1>() -> Option<T1> {
    None
}

unsafe fn f1<T1>() -> BTreeMap<T1, T1> {
    BTreeMap::new()
}

// unionSets(V1, V2).  Inserts either operand if absent (EquivalenceClasses.h:306
// does `insert(V1), insert(V2)` first), then merges V2's class INTO V1's and
// returns V1's leader.  The `movers` collect is what makes the result
// transitive: every element whose leader is l2 -- not just V2 itself -- is
// rewritten, so a third element already unioned into l2's class moves with it.
unsafe fn f2<T1: Ord + Clone>(a0: &mut BTreeMap<T1, T1>, a1: T1, a2: T1) -> Option<T1> {
    if !a0.contains_key(&a1) {
        a0.insert(a1.clone(), a1.clone());
    }
    if !a0.contains_key(&a2) {
        a0.insert(a2.clone(), a2.clone());
    }
    let l1 = a0.get(&a1).cloned().unwrap();
    let l2 = a0.get(&a2).cloned().unwrap();
    if l1 != l2 {
        let movers: Vec<T1> = a0
            .iter()
            .filter(|kv| *kv.1 == l2)
            .map(|kv| kv.0.clone())
            .collect();
        for k in movers {
            a0.insert(k, l1.clone());
        }
    }
    Some(l1)
}

// findLeader -- None for a value never inserted, which is exactly member_end()
// (EquivalenceClasses.h:295 returns `member_iterator(nullptr)` there).
unsafe fn f3<T1: Ord + Clone>(a0: BTreeMap<T1, T1>, a1: T1) -> Option<T1> {
    a0.get(&a1).cloned()
}

unsafe fn f4<T1>(a0: BTreeMap<T1, T1>) -> Option<T1> {
    let _ = &a0;
    None
}

// isEquivalent -- EquivalenceClasses.h:326: self-equality is a fast path that
// answers true even for a value that was never inserted, then it compares
// findLeader results and requires the first to exist.  Both halves are
// reproduced; dropping the fast path would make isEquivalent(x, x) false for an
// uninserted x, which C++ answers true.
unsafe fn f5<T1: Ord + Clone>(a0: BTreeMap<T1, T1>, a1: T1, a2: T1) -> bool {
    // NO `return`, deliberately: a rule body is INLINED into the caller, so an
    // early `return true;` returns from the CALLER -- measured as
    // `error[E0308]: expected i32, found bool` inside the probe's `main_0`, with
    // the rule's text sitting in the middle of main.  A rule body must be a
    // single expression; `||` is how this one spells the self-equality fast path.
    // ONE expression, no `let`: in the refcount model the receiver is
    // `(*x.borrow())`, so binding `let l1 = a0.get(&a1);` borrows a TEMPORARY
    // that is dropped at the end of the statement -- E0716, measured.  The
    // double `get` is the price of not binding.
    a1 == a2 || (a0.get(&a1).is_some() && a0.get(&a1) == a0.get(&a2))
}

unsafe fn f6<T1: Ord>(a0: BTreeMap<T1, T1>, a1: T1) -> bool {
    a0.contains_key(&a1)
}

unsafe fn f7<T1>(a0: BTreeMap<T1, T1>) -> bool {
    a0.is_empty()
}

// member_iterator ==/!= -- position identity on the leader node, which on this
// side is equality of the leader VALUE.  Two leader positions are equal iff the
// classes are the same class, and None == None is member_end() == member_end().
unsafe fn f8<T1: PartialEq>(a0: Option<T1>, a1: Option<T1>) -> bool {
    a0 == a1
}

unsafe fn f9<T1: PartialEq>(a0: Option<T1>, a1: Option<T1>) -> bool {
    a0 != a1
}
