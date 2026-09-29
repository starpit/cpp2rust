// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

//! `mlir::OwningOpRef<OpTy>` -- THE OWNING HANDLE WHOSE DESTRUCTOR ERASES THE OP.
//!
//! Transcribed from `mlir/IR/OwningOpRef.h` in this toolchain:
//!
//! ```text
//!   template <typename OpTy> class OwningOpRef {
//!     OwningOpRef(std::nullptr_t = nullptr) : op(nullptr) {}
//!     OwningOpRef(OpTy op) : op(op) {}
//!     OwningOpRef(OwningOpRef &&other) : op(other.release()) {}
//!     ~OwningOpRef() { if (op) op->erase(); }          // <-- THE OBSERVABLE EFFECT
//!     OwningOpRef &operator=(OwningOpRef &&other) { if (op) op->erase(); ... }
//!     OpTy get() const;  OpTy operator*() const;  auto operator->();
//!     explicit operator bool() const;
//!     OpTy release() { OpTy released(nullptr); std::swap(released, op); return released; }
//!     OpTy op;
//!   };
//! ```
//!
//! ⛔ WHY THIS IS NOT `Option<OpTy>` AND WHY ITS `Drop` PANICS.
//!
//! `rules/mlir` was RIGHT to refuse a value model for this type (its own note at
//! src.cpp:2361): `~OwningOpRef` erases the adopted operation, the corpus relies on
//! that in its own words at `dcc/src/Driver/dcc.cpp:60-70` -- "for an external
//! context, module_ (OwningOpRef) erases the module it adopted and the caller frees
//! the context" -- and an opaque unit has no destructor, so `()` would SILENTLY drop
//! the erase.
//!
//! ⭐ BUT THE ERASE CANNOT BE PERFORMED EITHER, and that is a fact about the OP
//! MODEL, not about this wrapper.  `mlir::ModuleOp` is `rules/mlir` t61, AN OPAQUE
//! UNIT, because `builtin.module` is an MLIR builtin with no `TD_OPS` row and so no
//! `dataflowir_gen::fmt::OpInst` can name it.  A unit holds no operation, so there is
//! nothing for a `Drop` to erase.  `Operation::erase` is mapped NOWHERE in the rule
//! tree for any op handle.
//!
//! ⭐ SO THE HONEST MODEL IS OWNERSHIP PLUS A TRIPWIRE: hold the op in an `Option`,
//! and make `Drop` PANIC if it still holds one.  Then
//!   * a handle that never adopts an op (`OwningOpRef()` / after `release()`) drops
//!     silently, because C++ does nothing in that case either -- `if (op)` is false;
//!   * a handle that DOES adopt one fails LOUDLY at the exact point where C++ would
//!     have erased, instead of quietly leaking the op.  That is the ranking this
//!     project keeps: a loud failure beats a silent wrong lowering.
//! No constructor and no accessor is keyed in `rules/mlir`, so every adopting site
//! and every `get()`/`operator*`/`operator->`/`release()` read still fails at
//! TRANSLATE time; this `Drop` is the backstop for a path that gets there anyway.
//!
//! ⚠️ `release()` is faithful and is NOT a workaround: in C++ it takes the op out of
//! the handle precisely so the destructor does not erase it, which is what
//! `dcc.cpp:68` calls it for.  So `release()` disarming the tripwire is the C++
//! behaviour, not a hole in it.

/// An owning reference to an MLIR op.  See the module comment for why `Drop`
/// panics rather than erasing.
#[derive(Debug)]
pub struct OwningOpRef<OpTy> {
    op: Option<OpTy>,
}

impl<OpTy> OwningOpRef<OpTy> {
    /// `OwningOpRef(std::nullptr_t = nullptr)` -- the default-constructed handle.
    /// Holds no op, so its `Drop` is a no-op, exactly as `if (op)` is false in C++.
    pub fn null() -> Self {
        OwningOpRef { op: None }
    }

    /// `OwningOpRef(OpTy op)` -- adopt an op.  From here on the handle owes an
    /// `erase()` that no rule can perform, so dropping it panics.
    pub fn adopt(op: OpTy) -> Self {
        OwningOpRef { op: Some(op) }
    }

    /// `explicit operator bool() const`.
    pub fn is_valid(&self) -> bool {
        self.op.is_some()
    }

    /// `OpTy release()` -- hand the op out and stop owing the erase.
    pub fn release(&mut self) -> Option<OpTy> {
        self.op.take()
    }
}

impl<OpTy> Drop for OwningOpRef<OpTy> {
    fn drop(&mut self) {
        if self.op.is_some() {
            panic!(
                "mlir::OwningOpRef dropped while still holding an op: ~OwningOpRef \
                 erases the adopted operation, and no rule in this tree models \
                 Operation::erase for an op handle (mlir::ModuleOp is an opaque unit). \
                 The erase is NOT performed, so failing here rather than leaking it \
                 silently. Use release() if the op is owned elsewhere."
            );
        }
    }
}

#[cfg(test)]
mod tests {
    use super::OwningOpRef;

    // The default-constructed handle holds nothing, so dropping it is silent --
    // this is the shape `dxp/dxp.h:76`'s member has in `dxp_standalone.cpp`, which
    // never assigns it.
    #[test]
    fn a_null_handle_drops_silently() {
        let r: OwningOpRef<()> = OwningOpRef::null();
        assert!(!r.is_valid());
        drop(r);
    }

    // release() takes the op out, so the tripwire is disarmed -- dcc.cpp:68.
    #[test]
    fn release_disarms_the_tripwire() {
        let mut r = OwningOpRef::adopt(7i32);
        assert!(r.is_valid());
        assert_eq!(r.release(), Some(7));
        assert!(!r.is_valid());
        drop(r);
    }

    // EXECUTED, not argued: a handle that still owns an op fails LOUDLY on drop.
    #[test]
    #[should_panic(expected = "still holding an op")]
    fn dropping_an_adopted_op_panics_because_the_erase_is_not_modelled() {
        let r = OwningOpRef::adopt(7i32);
        assert!(r.is_valid());
        drop(r);
    }
}
