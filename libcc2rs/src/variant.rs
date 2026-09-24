// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

//! `std::variant` is a tagged union; a Rust enum is a tagged union.  So this is
//! a Rust enum -- one per arity, `Variant2` .. `Variant8` -- and `rules/variant`
//! maps `std::variant<A, B, ..>` onto it directly.  Nothing here approximates
//! anything: the discriminant is the discriminant, the payload is the payload.
//!
//! Two representations use the SAME enum, which is the point of putting it here
//! rather than in a rule body: the unsafe model instantiates it at the bare
//! alternative types (`Variant2<i64, Vec<c_char>>`) and the refcount model at
//! `Value<..>` (`Variant2<Value<i64>, Value<Vec<u8>>>`), the same split
//! `rules/tuple` has between `(T1, T2)` and `(Value<T1>, Value<T2>)`.
//!
//! WHY THE ACCESSORS ARE `cc2_`-PREFIXED FREE-ISH METHODS RATHER THAN A
//! `match` IN THE RULE BODY.  A rule body is inlined verbatim into arbitrary
//! expression position (see the "wrap the body in `({ .. })`" note in the
//! porting playbook), the return-type annotation is gone after inlining, and a
//! comment inside a body lands in the generated Rust.  A named method keeps all
//! three problems inside this file, where they are ordinary Rust.
//!
//! `cc2_N()` PANICS on the wrong alternative, which is C++'s
//! `std::bad_variant_access`.  It must not return a default: `std::get<T>` on
//! the wrong alternative is an ERROR in C++ and a silently-defaulted value is
//! exactly the class of wrongness the port is trying not to introduce.  The
//! panic message names the expected and actual index so a hit is diagnosable.
//!
//! `Default` is the FIRST alternative, which is C++: a default-constructed
//! `std::variant` value-initialises alternative 0 and `index()` returns 0.
//!
//! `cc2_deep_clone` exists for the refcount model only and clones the
//! pointed-to VALUE rather than the `Rc` handle.  C++ `a = b` on a variant
//! copies the object; a handle clone would make the copy alias the original,
//! which is the aliasing bug `rules/pair` documents at length.

use crate::reinterpret::ByteRepr;
use std::cell::RefCell;
use std::rc::Rc;

#[derive(Clone, PartialEq, Eq, PartialOrd, Ord, Hash, Debug)]
pub enum Variant2<T1, T2> {
    V1(T1),
    V2(T2),
}

impl<T1: Default, T2: Default> Default for Variant2<T1, T2> {
    fn default() -> Self {
        Variant2::V1(T1::default())
    }
}

impl<T1: ByteRepr, T2: ByteRepr> ByteRepr for Variant2<T1, T2> {}

impl<T1, T2> Variant2<T1, T2> {
    pub fn cc2_index(&self) -> u64 {
        match self {
            Variant2::V1(_) => 0,
            Variant2::V2(_) => 1,
        }
    }
    pub fn cc2_holds_1(&self) -> bool {
        matches!(self, Variant2::V1(_))
    }

    pub fn cc2_1(&self) -> &T1 {
        match self {
            Variant2::V1(v) => v,
            other => panic!(
                "std::bad_variant_access: get<0> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_1_mut(&mut self) -> &mut T1 {
        match self {
            Variant2::V1(v) => v,
            other => panic!(
                "std::bad_variant_access: get<0> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_2(&self) -> bool {
        matches!(self, Variant2::V2(_))
    }

    pub fn cc2_2(&self) -> &T2 {
        match self {
            Variant2::V2(v) => v,
            other => panic!(
                "std::bad_variant_access: get<1> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_2_mut(&mut self) -> &mut T2 {
        match self {
            Variant2::V2(v) => v,
            other => panic!(
                "std::bad_variant_access: get<1> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
}

impl<T1: crate::DeepClone, T2: crate::DeepClone> Variant2<Rc<RefCell<T1>>, Rc<RefCell<T2>>> {
    pub fn cc2_deep_clone(&self) -> Self {
        match self {
            Variant2::V1(v) => Variant2::V1(crate::DeepClone::deep_clone(v)),
            Variant2::V2(v) => Variant2::V2(crate::DeepClone::deep_clone(v)),
        }
    }
}

impl<T1, T2> Variant2<Rc<RefCell<T1>>, Rc<RefCell<T2>>> {
    pub fn cc2_ptr_1(&self) -> crate::rc::Ptr<T1> {
        crate::rc::AsPointer::<T1>::as_pointer(self.cc2_1())
    }

    pub fn cc2_ptr_2(&self) -> crate::rc::Ptr<T2> {
        crate::rc::AsPointer::<T2>::as_pointer(self.cc2_2())
    }
}

// A variant must be DeepClone itself, or it could never be a container's
// ELEMENT -- `BTreeMap<K, Value<Variant2<..>>>::deep_clone` needs it -- and
// every `std::map<K, std::variant<..>>` copy would be a compile error.
impl<T1: crate::DeepClone, T2: crate::DeepClone> crate::DeepClone for Variant2<T1, T2> {
    fn deep_clone(&self) -> Self {
        match self {
            Variant2::V1(v) => Variant2::V1(crate::DeepClone::deep_clone(v)),
            Variant2::V2(v) => Variant2::V2(crate::DeepClone::deep_clone(v)),
        }
    }
}

#[derive(Clone, PartialEq, Eq, PartialOrd, Ord, Hash, Debug)]
pub enum Variant3<T1, T2, T3> {
    V1(T1),
    V2(T2),
    V3(T3),
}

impl<T1: Default, T2: Default, T3: Default> Default for Variant3<T1, T2, T3> {
    fn default() -> Self {
        Variant3::V1(T1::default())
    }
}

impl<T1: ByteRepr, T2: ByteRepr, T3: ByteRepr> ByteRepr for Variant3<T1, T2, T3> {}

impl<T1, T2, T3> Variant3<T1, T2, T3> {
    pub fn cc2_index(&self) -> u64 {
        match self {
            Variant3::V1(_) => 0,
            Variant3::V2(_) => 1,
            Variant3::V3(_) => 2,
        }
    }
    pub fn cc2_holds_1(&self) -> bool {
        matches!(self, Variant3::V1(_))
    }

    pub fn cc2_1(&self) -> &T1 {
        match self {
            Variant3::V1(v) => v,
            other => panic!(
                "std::bad_variant_access: get<0> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_1_mut(&mut self) -> &mut T1 {
        match self {
            Variant3::V1(v) => v,
            other => panic!(
                "std::bad_variant_access: get<0> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_2(&self) -> bool {
        matches!(self, Variant3::V2(_))
    }

    pub fn cc2_2(&self) -> &T2 {
        match self {
            Variant3::V2(v) => v,
            other => panic!(
                "std::bad_variant_access: get<1> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_2_mut(&mut self) -> &mut T2 {
        match self {
            Variant3::V2(v) => v,
            other => panic!(
                "std::bad_variant_access: get<1> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_3(&self) -> bool {
        matches!(self, Variant3::V3(_))
    }

    pub fn cc2_3(&self) -> &T3 {
        match self {
            Variant3::V3(v) => v,
            other => panic!(
                "std::bad_variant_access: get<2> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_3_mut(&mut self) -> &mut T3 {
        match self {
            Variant3::V3(v) => v,
            other => panic!(
                "std::bad_variant_access: get<2> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
}

impl<T1: crate::DeepClone, T2: crate::DeepClone, T3: crate::DeepClone> Variant3<Rc<RefCell<T1>>, Rc<RefCell<T2>>, Rc<RefCell<T3>>> {
    pub fn cc2_deep_clone(&self) -> Self {
        match self {
            Variant3::V1(v) => Variant3::V1(crate::DeepClone::deep_clone(v)),
            Variant3::V2(v) => Variant3::V2(crate::DeepClone::deep_clone(v)),
            Variant3::V3(v) => Variant3::V3(crate::DeepClone::deep_clone(v)),
        }
    }
}

impl<T1, T2, T3> Variant3<Rc<RefCell<T1>>, Rc<RefCell<T2>>, Rc<RefCell<T3>>> {
    pub fn cc2_ptr_1(&self) -> crate::rc::Ptr<T1> {
        crate::rc::AsPointer::<T1>::as_pointer(self.cc2_1())
    }

    pub fn cc2_ptr_2(&self) -> crate::rc::Ptr<T2> {
        crate::rc::AsPointer::<T2>::as_pointer(self.cc2_2())
    }

    pub fn cc2_ptr_3(&self) -> crate::rc::Ptr<T3> {
        crate::rc::AsPointer::<T3>::as_pointer(self.cc2_3())
    }
}

// A variant must be DeepClone itself, or it could never be a container's
// ELEMENT -- `BTreeMap<K, Value<Variant2<..>>>::deep_clone` needs it -- and
// every `std::map<K, std::variant<..>>` copy would be a compile error.
impl<T1: crate::DeepClone, T2: crate::DeepClone, T3: crate::DeepClone> crate::DeepClone for Variant3<T1, T2, T3> {
    fn deep_clone(&self) -> Self {
        match self {
            Variant3::V1(v) => Variant3::V1(crate::DeepClone::deep_clone(v)),
            Variant3::V2(v) => Variant3::V2(crate::DeepClone::deep_clone(v)),
            Variant3::V3(v) => Variant3::V3(crate::DeepClone::deep_clone(v)),
        }
    }
}

#[derive(Clone, PartialEq, Eq, PartialOrd, Ord, Hash, Debug)]
pub enum Variant4<T1, T2, T3, T4> {
    V1(T1),
    V2(T2),
    V3(T3),
    V4(T4),
}

impl<T1: Default, T2: Default, T3: Default, T4: Default> Default for Variant4<T1, T2, T3, T4> {
    fn default() -> Self {
        Variant4::V1(T1::default())
    }
}

impl<T1: ByteRepr, T2: ByteRepr, T3: ByteRepr, T4: ByteRepr> ByteRepr for Variant4<T1, T2, T3, T4> {}

impl<T1, T2, T3, T4> Variant4<T1, T2, T3, T4> {
    pub fn cc2_index(&self) -> u64 {
        match self {
            Variant4::V1(_) => 0,
            Variant4::V2(_) => 1,
            Variant4::V3(_) => 2,
            Variant4::V4(_) => 3,
        }
    }
    pub fn cc2_holds_1(&self) -> bool {
        matches!(self, Variant4::V1(_))
    }

    pub fn cc2_1(&self) -> &T1 {
        match self {
            Variant4::V1(v) => v,
            other => panic!(
                "std::bad_variant_access: get<0> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_1_mut(&mut self) -> &mut T1 {
        match self {
            Variant4::V1(v) => v,
            other => panic!(
                "std::bad_variant_access: get<0> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_2(&self) -> bool {
        matches!(self, Variant4::V2(_))
    }

    pub fn cc2_2(&self) -> &T2 {
        match self {
            Variant4::V2(v) => v,
            other => panic!(
                "std::bad_variant_access: get<1> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_2_mut(&mut self) -> &mut T2 {
        match self {
            Variant4::V2(v) => v,
            other => panic!(
                "std::bad_variant_access: get<1> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_3(&self) -> bool {
        matches!(self, Variant4::V3(_))
    }

    pub fn cc2_3(&self) -> &T3 {
        match self {
            Variant4::V3(v) => v,
            other => panic!(
                "std::bad_variant_access: get<2> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_3_mut(&mut self) -> &mut T3 {
        match self {
            Variant4::V3(v) => v,
            other => panic!(
                "std::bad_variant_access: get<2> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_4(&self) -> bool {
        matches!(self, Variant4::V4(_))
    }

    pub fn cc2_4(&self) -> &T4 {
        match self {
            Variant4::V4(v) => v,
            other => panic!(
                "std::bad_variant_access: get<3> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_4_mut(&mut self) -> &mut T4 {
        match self {
            Variant4::V4(v) => v,
            other => panic!(
                "std::bad_variant_access: get<3> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
}

impl<T1: crate::DeepClone, T2: crate::DeepClone, T3: crate::DeepClone, T4: crate::DeepClone> Variant4<Rc<RefCell<T1>>, Rc<RefCell<T2>>, Rc<RefCell<T3>>, Rc<RefCell<T4>>> {
    pub fn cc2_deep_clone(&self) -> Self {
        match self {
            Variant4::V1(v) => Variant4::V1(crate::DeepClone::deep_clone(v)),
            Variant4::V2(v) => Variant4::V2(crate::DeepClone::deep_clone(v)),
            Variant4::V3(v) => Variant4::V3(crate::DeepClone::deep_clone(v)),
            Variant4::V4(v) => Variant4::V4(crate::DeepClone::deep_clone(v)),
        }
    }
}

impl<T1, T2, T3, T4> Variant4<Rc<RefCell<T1>>, Rc<RefCell<T2>>, Rc<RefCell<T3>>, Rc<RefCell<T4>>> {
    pub fn cc2_ptr_1(&self) -> crate::rc::Ptr<T1> {
        crate::rc::AsPointer::<T1>::as_pointer(self.cc2_1())
    }

    pub fn cc2_ptr_2(&self) -> crate::rc::Ptr<T2> {
        crate::rc::AsPointer::<T2>::as_pointer(self.cc2_2())
    }

    pub fn cc2_ptr_3(&self) -> crate::rc::Ptr<T3> {
        crate::rc::AsPointer::<T3>::as_pointer(self.cc2_3())
    }

    pub fn cc2_ptr_4(&self) -> crate::rc::Ptr<T4> {
        crate::rc::AsPointer::<T4>::as_pointer(self.cc2_4())
    }
}

// A variant must be DeepClone itself, or it could never be a container's
// ELEMENT -- `BTreeMap<K, Value<Variant2<..>>>::deep_clone` needs it -- and
// every `std::map<K, std::variant<..>>` copy would be a compile error.
impl<T1: crate::DeepClone, T2: crate::DeepClone, T3: crate::DeepClone, T4: crate::DeepClone> crate::DeepClone for Variant4<T1, T2, T3, T4> {
    fn deep_clone(&self) -> Self {
        match self {
            Variant4::V1(v) => Variant4::V1(crate::DeepClone::deep_clone(v)),
            Variant4::V2(v) => Variant4::V2(crate::DeepClone::deep_clone(v)),
            Variant4::V3(v) => Variant4::V3(crate::DeepClone::deep_clone(v)),
            Variant4::V4(v) => Variant4::V4(crate::DeepClone::deep_clone(v)),
        }
    }
}

#[derive(Clone, PartialEq, Eq, PartialOrd, Ord, Hash, Debug)]
pub enum Variant5<T1, T2, T3, T4, T5> {
    V1(T1),
    V2(T2),
    V3(T3),
    V4(T4),
    V5(T5),
}

impl<T1: Default, T2: Default, T3: Default, T4: Default, T5: Default> Default for Variant5<T1, T2, T3, T4, T5> {
    fn default() -> Self {
        Variant5::V1(T1::default())
    }
}

impl<T1: ByteRepr, T2: ByteRepr, T3: ByteRepr, T4: ByteRepr, T5: ByteRepr> ByteRepr for Variant5<T1, T2, T3, T4, T5> {}

impl<T1, T2, T3, T4, T5> Variant5<T1, T2, T3, T4, T5> {
    pub fn cc2_index(&self) -> u64 {
        match self {
            Variant5::V1(_) => 0,
            Variant5::V2(_) => 1,
            Variant5::V3(_) => 2,
            Variant5::V4(_) => 3,
            Variant5::V5(_) => 4,
        }
    }
    pub fn cc2_holds_1(&self) -> bool {
        matches!(self, Variant5::V1(_))
    }

    pub fn cc2_1(&self) -> &T1 {
        match self {
            Variant5::V1(v) => v,
            other => panic!(
                "std::bad_variant_access: get<0> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_1_mut(&mut self) -> &mut T1 {
        match self {
            Variant5::V1(v) => v,
            other => panic!(
                "std::bad_variant_access: get<0> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_2(&self) -> bool {
        matches!(self, Variant5::V2(_))
    }

    pub fn cc2_2(&self) -> &T2 {
        match self {
            Variant5::V2(v) => v,
            other => panic!(
                "std::bad_variant_access: get<1> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_2_mut(&mut self) -> &mut T2 {
        match self {
            Variant5::V2(v) => v,
            other => panic!(
                "std::bad_variant_access: get<1> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_3(&self) -> bool {
        matches!(self, Variant5::V3(_))
    }

    pub fn cc2_3(&self) -> &T3 {
        match self {
            Variant5::V3(v) => v,
            other => panic!(
                "std::bad_variant_access: get<2> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_3_mut(&mut self) -> &mut T3 {
        match self {
            Variant5::V3(v) => v,
            other => panic!(
                "std::bad_variant_access: get<2> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_4(&self) -> bool {
        matches!(self, Variant5::V4(_))
    }

    pub fn cc2_4(&self) -> &T4 {
        match self {
            Variant5::V4(v) => v,
            other => panic!(
                "std::bad_variant_access: get<3> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_4_mut(&mut self) -> &mut T4 {
        match self {
            Variant5::V4(v) => v,
            other => panic!(
                "std::bad_variant_access: get<3> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_5(&self) -> bool {
        matches!(self, Variant5::V5(_))
    }

    pub fn cc2_5(&self) -> &T5 {
        match self {
            Variant5::V5(v) => v,
            other => panic!(
                "std::bad_variant_access: get<4> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_5_mut(&mut self) -> &mut T5 {
        match self {
            Variant5::V5(v) => v,
            other => panic!(
                "std::bad_variant_access: get<4> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
}

impl<T1: crate::DeepClone, T2: crate::DeepClone, T3: crate::DeepClone, T4: crate::DeepClone, T5: crate::DeepClone> Variant5<Rc<RefCell<T1>>, Rc<RefCell<T2>>, Rc<RefCell<T3>>, Rc<RefCell<T4>>, Rc<RefCell<T5>>> {
    pub fn cc2_deep_clone(&self) -> Self {
        match self {
            Variant5::V1(v) => Variant5::V1(crate::DeepClone::deep_clone(v)),
            Variant5::V2(v) => Variant5::V2(crate::DeepClone::deep_clone(v)),
            Variant5::V3(v) => Variant5::V3(crate::DeepClone::deep_clone(v)),
            Variant5::V4(v) => Variant5::V4(crate::DeepClone::deep_clone(v)),
            Variant5::V5(v) => Variant5::V5(crate::DeepClone::deep_clone(v)),
        }
    }
}

impl<T1, T2, T3, T4, T5> Variant5<Rc<RefCell<T1>>, Rc<RefCell<T2>>, Rc<RefCell<T3>>, Rc<RefCell<T4>>, Rc<RefCell<T5>>> {
    pub fn cc2_ptr_1(&self) -> crate::rc::Ptr<T1> {
        crate::rc::AsPointer::<T1>::as_pointer(self.cc2_1())
    }

    pub fn cc2_ptr_2(&self) -> crate::rc::Ptr<T2> {
        crate::rc::AsPointer::<T2>::as_pointer(self.cc2_2())
    }

    pub fn cc2_ptr_3(&self) -> crate::rc::Ptr<T3> {
        crate::rc::AsPointer::<T3>::as_pointer(self.cc2_3())
    }

    pub fn cc2_ptr_4(&self) -> crate::rc::Ptr<T4> {
        crate::rc::AsPointer::<T4>::as_pointer(self.cc2_4())
    }

    pub fn cc2_ptr_5(&self) -> crate::rc::Ptr<T5> {
        crate::rc::AsPointer::<T5>::as_pointer(self.cc2_5())
    }
}

// A variant must be DeepClone itself, or it could never be a container's
// ELEMENT -- `BTreeMap<K, Value<Variant2<..>>>::deep_clone` needs it -- and
// every `std::map<K, std::variant<..>>` copy would be a compile error.
impl<T1: crate::DeepClone, T2: crate::DeepClone, T3: crate::DeepClone, T4: crate::DeepClone, T5: crate::DeepClone> crate::DeepClone for Variant5<T1, T2, T3, T4, T5> {
    fn deep_clone(&self) -> Self {
        match self {
            Variant5::V1(v) => Variant5::V1(crate::DeepClone::deep_clone(v)),
            Variant5::V2(v) => Variant5::V2(crate::DeepClone::deep_clone(v)),
            Variant5::V3(v) => Variant5::V3(crate::DeepClone::deep_clone(v)),
            Variant5::V4(v) => Variant5::V4(crate::DeepClone::deep_clone(v)),
            Variant5::V5(v) => Variant5::V5(crate::DeepClone::deep_clone(v)),
        }
    }
}

#[derive(Clone, PartialEq, Eq, PartialOrd, Ord, Hash, Debug)]
pub enum Variant6<T1, T2, T3, T4, T5, T6> {
    V1(T1),
    V2(T2),
    V3(T3),
    V4(T4),
    V5(T5),
    V6(T6),
}

impl<T1: Default, T2: Default, T3: Default, T4: Default, T5: Default, T6: Default> Default for Variant6<T1, T2, T3, T4, T5, T6> {
    fn default() -> Self {
        Variant6::V1(T1::default())
    }
}

impl<T1: ByteRepr, T2: ByteRepr, T3: ByteRepr, T4: ByteRepr, T5: ByteRepr, T6: ByteRepr> ByteRepr for Variant6<T1, T2, T3, T4, T5, T6> {}

impl<T1, T2, T3, T4, T5, T6> Variant6<T1, T2, T3, T4, T5, T6> {
    pub fn cc2_index(&self) -> u64 {
        match self {
            Variant6::V1(_) => 0,
            Variant6::V2(_) => 1,
            Variant6::V3(_) => 2,
            Variant6::V4(_) => 3,
            Variant6::V5(_) => 4,
            Variant6::V6(_) => 5,
        }
    }
    pub fn cc2_holds_1(&self) -> bool {
        matches!(self, Variant6::V1(_))
    }

    pub fn cc2_1(&self) -> &T1 {
        match self {
            Variant6::V1(v) => v,
            other => panic!(
                "std::bad_variant_access: get<0> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_1_mut(&mut self) -> &mut T1 {
        match self {
            Variant6::V1(v) => v,
            other => panic!(
                "std::bad_variant_access: get<0> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_2(&self) -> bool {
        matches!(self, Variant6::V2(_))
    }

    pub fn cc2_2(&self) -> &T2 {
        match self {
            Variant6::V2(v) => v,
            other => panic!(
                "std::bad_variant_access: get<1> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_2_mut(&mut self) -> &mut T2 {
        match self {
            Variant6::V2(v) => v,
            other => panic!(
                "std::bad_variant_access: get<1> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_3(&self) -> bool {
        matches!(self, Variant6::V3(_))
    }

    pub fn cc2_3(&self) -> &T3 {
        match self {
            Variant6::V3(v) => v,
            other => panic!(
                "std::bad_variant_access: get<2> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_3_mut(&mut self) -> &mut T3 {
        match self {
            Variant6::V3(v) => v,
            other => panic!(
                "std::bad_variant_access: get<2> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_4(&self) -> bool {
        matches!(self, Variant6::V4(_))
    }

    pub fn cc2_4(&self) -> &T4 {
        match self {
            Variant6::V4(v) => v,
            other => panic!(
                "std::bad_variant_access: get<3> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_4_mut(&mut self) -> &mut T4 {
        match self {
            Variant6::V4(v) => v,
            other => panic!(
                "std::bad_variant_access: get<3> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_5(&self) -> bool {
        matches!(self, Variant6::V5(_))
    }

    pub fn cc2_5(&self) -> &T5 {
        match self {
            Variant6::V5(v) => v,
            other => panic!(
                "std::bad_variant_access: get<4> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_5_mut(&mut self) -> &mut T5 {
        match self {
            Variant6::V5(v) => v,
            other => panic!(
                "std::bad_variant_access: get<4> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_6(&self) -> bool {
        matches!(self, Variant6::V6(_))
    }

    pub fn cc2_6(&self) -> &T6 {
        match self {
            Variant6::V6(v) => v,
            other => panic!(
                "std::bad_variant_access: get<5> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_6_mut(&mut self) -> &mut T6 {
        match self {
            Variant6::V6(v) => v,
            other => panic!(
                "std::bad_variant_access: get<5> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
}

impl<T1: crate::DeepClone, T2: crate::DeepClone, T3: crate::DeepClone, T4: crate::DeepClone, T5: crate::DeepClone, T6: crate::DeepClone> Variant6<Rc<RefCell<T1>>, Rc<RefCell<T2>>, Rc<RefCell<T3>>, Rc<RefCell<T4>>, Rc<RefCell<T5>>, Rc<RefCell<T6>>> {
    pub fn cc2_deep_clone(&self) -> Self {
        match self {
            Variant6::V1(v) => Variant6::V1(crate::DeepClone::deep_clone(v)),
            Variant6::V2(v) => Variant6::V2(crate::DeepClone::deep_clone(v)),
            Variant6::V3(v) => Variant6::V3(crate::DeepClone::deep_clone(v)),
            Variant6::V4(v) => Variant6::V4(crate::DeepClone::deep_clone(v)),
            Variant6::V5(v) => Variant6::V5(crate::DeepClone::deep_clone(v)),
            Variant6::V6(v) => Variant6::V6(crate::DeepClone::deep_clone(v)),
        }
    }
}

impl<T1, T2, T3, T4, T5, T6> Variant6<Rc<RefCell<T1>>, Rc<RefCell<T2>>, Rc<RefCell<T3>>, Rc<RefCell<T4>>, Rc<RefCell<T5>>, Rc<RefCell<T6>>> {
    pub fn cc2_ptr_1(&self) -> crate::rc::Ptr<T1> {
        crate::rc::AsPointer::<T1>::as_pointer(self.cc2_1())
    }

    pub fn cc2_ptr_2(&self) -> crate::rc::Ptr<T2> {
        crate::rc::AsPointer::<T2>::as_pointer(self.cc2_2())
    }

    pub fn cc2_ptr_3(&self) -> crate::rc::Ptr<T3> {
        crate::rc::AsPointer::<T3>::as_pointer(self.cc2_3())
    }

    pub fn cc2_ptr_4(&self) -> crate::rc::Ptr<T4> {
        crate::rc::AsPointer::<T4>::as_pointer(self.cc2_4())
    }

    pub fn cc2_ptr_5(&self) -> crate::rc::Ptr<T5> {
        crate::rc::AsPointer::<T5>::as_pointer(self.cc2_5())
    }

    pub fn cc2_ptr_6(&self) -> crate::rc::Ptr<T6> {
        crate::rc::AsPointer::<T6>::as_pointer(self.cc2_6())
    }
}

// A variant must be DeepClone itself, or it could never be a container's
// ELEMENT -- `BTreeMap<K, Value<Variant2<..>>>::deep_clone` needs it -- and
// every `std::map<K, std::variant<..>>` copy would be a compile error.
impl<T1: crate::DeepClone, T2: crate::DeepClone, T3: crate::DeepClone, T4: crate::DeepClone, T5: crate::DeepClone, T6: crate::DeepClone> crate::DeepClone for Variant6<T1, T2, T3, T4, T5, T6> {
    fn deep_clone(&self) -> Self {
        match self {
            Variant6::V1(v) => Variant6::V1(crate::DeepClone::deep_clone(v)),
            Variant6::V2(v) => Variant6::V2(crate::DeepClone::deep_clone(v)),
            Variant6::V3(v) => Variant6::V3(crate::DeepClone::deep_clone(v)),
            Variant6::V4(v) => Variant6::V4(crate::DeepClone::deep_clone(v)),
            Variant6::V5(v) => Variant6::V5(crate::DeepClone::deep_clone(v)),
            Variant6::V6(v) => Variant6::V6(crate::DeepClone::deep_clone(v)),
        }
    }
}

#[derive(Clone, PartialEq, Eq, PartialOrd, Ord, Hash, Debug)]
pub enum Variant7<T1, T2, T3, T4, T5, T6, T7> {
    V1(T1),
    V2(T2),
    V3(T3),
    V4(T4),
    V5(T5),
    V6(T6),
    V7(T7),
}

impl<T1: Default, T2: Default, T3: Default, T4: Default, T5: Default, T6: Default, T7: Default> Default for Variant7<T1, T2, T3, T4, T5, T6, T7> {
    fn default() -> Self {
        Variant7::V1(T1::default())
    }
}

impl<T1: ByteRepr, T2: ByteRepr, T3: ByteRepr, T4: ByteRepr, T5: ByteRepr, T6: ByteRepr, T7: ByteRepr> ByteRepr for Variant7<T1, T2, T3, T4, T5, T6, T7> {}

impl<T1, T2, T3, T4, T5, T6, T7> Variant7<T1, T2, T3, T4, T5, T6, T7> {
    pub fn cc2_index(&self) -> u64 {
        match self {
            Variant7::V1(_) => 0,
            Variant7::V2(_) => 1,
            Variant7::V3(_) => 2,
            Variant7::V4(_) => 3,
            Variant7::V5(_) => 4,
            Variant7::V6(_) => 5,
            Variant7::V7(_) => 6,
        }
    }
    pub fn cc2_holds_1(&self) -> bool {
        matches!(self, Variant7::V1(_))
    }

    pub fn cc2_1(&self) -> &T1 {
        match self {
            Variant7::V1(v) => v,
            other => panic!(
                "std::bad_variant_access: get<0> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_1_mut(&mut self) -> &mut T1 {
        match self {
            Variant7::V1(v) => v,
            other => panic!(
                "std::bad_variant_access: get<0> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_2(&self) -> bool {
        matches!(self, Variant7::V2(_))
    }

    pub fn cc2_2(&self) -> &T2 {
        match self {
            Variant7::V2(v) => v,
            other => panic!(
                "std::bad_variant_access: get<1> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_2_mut(&mut self) -> &mut T2 {
        match self {
            Variant7::V2(v) => v,
            other => panic!(
                "std::bad_variant_access: get<1> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_3(&self) -> bool {
        matches!(self, Variant7::V3(_))
    }

    pub fn cc2_3(&self) -> &T3 {
        match self {
            Variant7::V3(v) => v,
            other => panic!(
                "std::bad_variant_access: get<2> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_3_mut(&mut self) -> &mut T3 {
        match self {
            Variant7::V3(v) => v,
            other => panic!(
                "std::bad_variant_access: get<2> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_4(&self) -> bool {
        matches!(self, Variant7::V4(_))
    }

    pub fn cc2_4(&self) -> &T4 {
        match self {
            Variant7::V4(v) => v,
            other => panic!(
                "std::bad_variant_access: get<3> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_4_mut(&mut self) -> &mut T4 {
        match self {
            Variant7::V4(v) => v,
            other => panic!(
                "std::bad_variant_access: get<3> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_5(&self) -> bool {
        matches!(self, Variant7::V5(_))
    }

    pub fn cc2_5(&self) -> &T5 {
        match self {
            Variant7::V5(v) => v,
            other => panic!(
                "std::bad_variant_access: get<4> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_5_mut(&mut self) -> &mut T5 {
        match self {
            Variant7::V5(v) => v,
            other => panic!(
                "std::bad_variant_access: get<4> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_6(&self) -> bool {
        matches!(self, Variant7::V6(_))
    }

    pub fn cc2_6(&self) -> &T6 {
        match self {
            Variant7::V6(v) => v,
            other => panic!(
                "std::bad_variant_access: get<5> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_6_mut(&mut self) -> &mut T6 {
        match self {
            Variant7::V6(v) => v,
            other => panic!(
                "std::bad_variant_access: get<5> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_7(&self) -> bool {
        matches!(self, Variant7::V7(_))
    }

    pub fn cc2_7(&self) -> &T7 {
        match self {
            Variant7::V7(v) => v,
            other => panic!(
                "std::bad_variant_access: get<6> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_7_mut(&mut self) -> &mut T7 {
        match self {
            Variant7::V7(v) => v,
            other => panic!(
                "std::bad_variant_access: get<6> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
}

impl<T1: crate::DeepClone, T2: crate::DeepClone, T3: crate::DeepClone, T4: crate::DeepClone, T5: crate::DeepClone, T6: crate::DeepClone, T7: crate::DeepClone> Variant7<Rc<RefCell<T1>>, Rc<RefCell<T2>>, Rc<RefCell<T3>>, Rc<RefCell<T4>>, Rc<RefCell<T5>>, Rc<RefCell<T6>>, Rc<RefCell<T7>>> {
    pub fn cc2_deep_clone(&self) -> Self {
        match self {
            Variant7::V1(v) => Variant7::V1(crate::DeepClone::deep_clone(v)),
            Variant7::V2(v) => Variant7::V2(crate::DeepClone::deep_clone(v)),
            Variant7::V3(v) => Variant7::V3(crate::DeepClone::deep_clone(v)),
            Variant7::V4(v) => Variant7::V4(crate::DeepClone::deep_clone(v)),
            Variant7::V5(v) => Variant7::V5(crate::DeepClone::deep_clone(v)),
            Variant7::V6(v) => Variant7::V6(crate::DeepClone::deep_clone(v)),
            Variant7::V7(v) => Variant7::V7(crate::DeepClone::deep_clone(v)),
        }
    }
}

impl<T1, T2, T3, T4, T5, T6, T7> Variant7<Rc<RefCell<T1>>, Rc<RefCell<T2>>, Rc<RefCell<T3>>, Rc<RefCell<T4>>, Rc<RefCell<T5>>, Rc<RefCell<T6>>, Rc<RefCell<T7>>> {
    pub fn cc2_ptr_1(&self) -> crate::rc::Ptr<T1> {
        crate::rc::AsPointer::<T1>::as_pointer(self.cc2_1())
    }

    pub fn cc2_ptr_2(&self) -> crate::rc::Ptr<T2> {
        crate::rc::AsPointer::<T2>::as_pointer(self.cc2_2())
    }

    pub fn cc2_ptr_3(&self) -> crate::rc::Ptr<T3> {
        crate::rc::AsPointer::<T3>::as_pointer(self.cc2_3())
    }

    pub fn cc2_ptr_4(&self) -> crate::rc::Ptr<T4> {
        crate::rc::AsPointer::<T4>::as_pointer(self.cc2_4())
    }

    pub fn cc2_ptr_5(&self) -> crate::rc::Ptr<T5> {
        crate::rc::AsPointer::<T5>::as_pointer(self.cc2_5())
    }

    pub fn cc2_ptr_6(&self) -> crate::rc::Ptr<T6> {
        crate::rc::AsPointer::<T6>::as_pointer(self.cc2_6())
    }

    pub fn cc2_ptr_7(&self) -> crate::rc::Ptr<T7> {
        crate::rc::AsPointer::<T7>::as_pointer(self.cc2_7())
    }
}

// A variant must be DeepClone itself, or it could never be a container's
// ELEMENT -- `BTreeMap<K, Value<Variant2<..>>>::deep_clone` needs it -- and
// every `std::map<K, std::variant<..>>` copy would be a compile error.
impl<T1: crate::DeepClone, T2: crate::DeepClone, T3: crate::DeepClone, T4: crate::DeepClone, T5: crate::DeepClone, T6: crate::DeepClone, T7: crate::DeepClone> crate::DeepClone for Variant7<T1, T2, T3, T4, T5, T6, T7> {
    fn deep_clone(&self) -> Self {
        match self {
            Variant7::V1(v) => Variant7::V1(crate::DeepClone::deep_clone(v)),
            Variant7::V2(v) => Variant7::V2(crate::DeepClone::deep_clone(v)),
            Variant7::V3(v) => Variant7::V3(crate::DeepClone::deep_clone(v)),
            Variant7::V4(v) => Variant7::V4(crate::DeepClone::deep_clone(v)),
            Variant7::V5(v) => Variant7::V5(crate::DeepClone::deep_clone(v)),
            Variant7::V6(v) => Variant7::V6(crate::DeepClone::deep_clone(v)),
            Variant7::V7(v) => Variant7::V7(crate::DeepClone::deep_clone(v)),
        }
    }
}

#[derive(Clone, PartialEq, Eq, PartialOrd, Ord, Hash, Debug)]
pub enum Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    V1(T1),
    V2(T2),
    V3(T3),
    V4(T4),
    V5(T5),
    V6(T6),
    V7(T7),
    V8(T8),
}

impl<T1: Default, T2: Default, T3: Default, T4: Default, T5: Default, T6: Default, T7: Default, T8: Default> Default for Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    fn default() -> Self {
        Variant8::V1(T1::default())
    }
}

impl<T1: ByteRepr, T2: ByteRepr, T3: ByteRepr, T4: ByteRepr, T5: ByteRepr, T6: ByteRepr, T7: ByteRepr, T8: ByteRepr> ByteRepr for Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {}

impl<T1, T2, T3, T4, T5, T6, T7, T8> Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    pub fn cc2_index(&self) -> u64 {
        match self {
            Variant8::V1(_) => 0,
            Variant8::V2(_) => 1,
            Variant8::V3(_) => 2,
            Variant8::V4(_) => 3,
            Variant8::V5(_) => 4,
            Variant8::V6(_) => 5,
            Variant8::V7(_) => 6,
            Variant8::V8(_) => 7,
        }
    }
    pub fn cc2_holds_1(&self) -> bool {
        matches!(self, Variant8::V1(_))
    }

    pub fn cc2_1(&self) -> &T1 {
        match self {
            Variant8::V1(v) => v,
            other => panic!(
                "std::bad_variant_access: get<0> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_1_mut(&mut self) -> &mut T1 {
        match self {
            Variant8::V1(v) => v,
            other => panic!(
                "std::bad_variant_access: get<0> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_2(&self) -> bool {
        matches!(self, Variant8::V2(_))
    }

    pub fn cc2_2(&self) -> &T2 {
        match self {
            Variant8::V2(v) => v,
            other => panic!(
                "std::bad_variant_access: get<1> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_2_mut(&mut self) -> &mut T2 {
        match self {
            Variant8::V2(v) => v,
            other => panic!(
                "std::bad_variant_access: get<1> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_3(&self) -> bool {
        matches!(self, Variant8::V3(_))
    }

    pub fn cc2_3(&self) -> &T3 {
        match self {
            Variant8::V3(v) => v,
            other => panic!(
                "std::bad_variant_access: get<2> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_3_mut(&mut self) -> &mut T3 {
        match self {
            Variant8::V3(v) => v,
            other => panic!(
                "std::bad_variant_access: get<2> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_4(&self) -> bool {
        matches!(self, Variant8::V4(_))
    }

    pub fn cc2_4(&self) -> &T4 {
        match self {
            Variant8::V4(v) => v,
            other => panic!(
                "std::bad_variant_access: get<3> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_4_mut(&mut self) -> &mut T4 {
        match self {
            Variant8::V4(v) => v,
            other => panic!(
                "std::bad_variant_access: get<3> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_5(&self) -> bool {
        matches!(self, Variant8::V5(_))
    }

    pub fn cc2_5(&self) -> &T5 {
        match self {
            Variant8::V5(v) => v,
            other => panic!(
                "std::bad_variant_access: get<4> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_5_mut(&mut self) -> &mut T5 {
        match self {
            Variant8::V5(v) => v,
            other => panic!(
                "std::bad_variant_access: get<4> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_6(&self) -> bool {
        matches!(self, Variant8::V6(_))
    }

    pub fn cc2_6(&self) -> &T6 {
        match self {
            Variant8::V6(v) => v,
            other => panic!(
                "std::bad_variant_access: get<5> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_6_mut(&mut self) -> &mut T6 {
        match self {
            Variant8::V6(v) => v,
            other => panic!(
                "std::bad_variant_access: get<5> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_7(&self) -> bool {
        matches!(self, Variant8::V7(_))
    }

    pub fn cc2_7(&self) -> &T7 {
        match self {
            Variant8::V7(v) => v,
            other => panic!(
                "std::bad_variant_access: get<6> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_7_mut(&mut self) -> &mut T7 {
        match self {
            Variant8::V7(v) => v,
            other => panic!(
                "std::bad_variant_access: get<6> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
    pub fn cc2_holds_8(&self) -> bool {
        matches!(self, Variant8::V8(_))
    }

    pub fn cc2_8(&self) -> &T8 {
        match self {
            Variant8::V8(v) => v,
            other => panic!(
                "std::bad_variant_access: get<7> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }

    pub fn cc2_8_mut(&mut self) -> &mut T8 {
        match self {
            Variant8::V8(v) => v,
            other => panic!(
                "std::bad_variant_access: get<7> on a variant holding alternative {}",
                other.cc2_index()
            ),
        }
    }
}

impl<T1: crate::DeepClone, T2: crate::DeepClone, T3: crate::DeepClone, T4: crate::DeepClone, T5: crate::DeepClone, T6: crate::DeepClone, T7: crate::DeepClone, T8: crate::DeepClone> Variant8<Rc<RefCell<T1>>, Rc<RefCell<T2>>, Rc<RefCell<T3>>, Rc<RefCell<T4>>, Rc<RefCell<T5>>, Rc<RefCell<T6>>, Rc<RefCell<T7>>, Rc<RefCell<T8>>> {
    pub fn cc2_deep_clone(&self) -> Self {
        match self {
            Variant8::V1(v) => Variant8::V1(crate::DeepClone::deep_clone(v)),
            Variant8::V2(v) => Variant8::V2(crate::DeepClone::deep_clone(v)),
            Variant8::V3(v) => Variant8::V3(crate::DeepClone::deep_clone(v)),
            Variant8::V4(v) => Variant8::V4(crate::DeepClone::deep_clone(v)),
            Variant8::V5(v) => Variant8::V5(crate::DeepClone::deep_clone(v)),
            Variant8::V6(v) => Variant8::V6(crate::DeepClone::deep_clone(v)),
            Variant8::V7(v) => Variant8::V7(crate::DeepClone::deep_clone(v)),
            Variant8::V8(v) => Variant8::V8(crate::DeepClone::deep_clone(v)),
        }
    }
}

impl<T1, T2, T3, T4, T5, T6, T7, T8> Variant8<Rc<RefCell<T1>>, Rc<RefCell<T2>>, Rc<RefCell<T3>>, Rc<RefCell<T4>>, Rc<RefCell<T5>>, Rc<RefCell<T6>>, Rc<RefCell<T7>>, Rc<RefCell<T8>>> {
    pub fn cc2_ptr_1(&self) -> crate::rc::Ptr<T1> {
        crate::rc::AsPointer::<T1>::as_pointer(self.cc2_1())
    }

    pub fn cc2_ptr_2(&self) -> crate::rc::Ptr<T2> {
        crate::rc::AsPointer::<T2>::as_pointer(self.cc2_2())
    }

    pub fn cc2_ptr_3(&self) -> crate::rc::Ptr<T3> {
        crate::rc::AsPointer::<T3>::as_pointer(self.cc2_3())
    }

    pub fn cc2_ptr_4(&self) -> crate::rc::Ptr<T4> {
        crate::rc::AsPointer::<T4>::as_pointer(self.cc2_4())
    }

    pub fn cc2_ptr_5(&self) -> crate::rc::Ptr<T5> {
        crate::rc::AsPointer::<T5>::as_pointer(self.cc2_5())
    }

    pub fn cc2_ptr_6(&self) -> crate::rc::Ptr<T6> {
        crate::rc::AsPointer::<T6>::as_pointer(self.cc2_6())
    }

    pub fn cc2_ptr_7(&self) -> crate::rc::Ptr<T7> {
        crate::rc::AsPointer::<T7>::as_pointer(self.cc2_7())
    }

    pub fn cc2_ptr_8(&self) -> crate::rc::Ptr<T8> {
        crate::rc::AsPointer::<T8>::as_pointer(self.cc2_8())
    }
}

// A variant must be DeepClone itself, or it could never be a container's
// ELEMENT -- `BTreeMap<K, Value<Variant2<..>>>::deep_clone` needs it -- and
// every `std::map<K, std::variant<..>>` copy would be a compile error.
impl<T1: crate::DeepClone, T2: crate::DeepClone, T3: crate::DeepClone, T4: crate::DeepClone, T5: crate::DeepClone, T6: crate::DeepClone, T7: crate::DeepClone, T8: crate::DeepClone> crate::DeepClone for Variant8<T1, T2, T3, T4, T5, T6, T7, T8> {
    fn deep_clone(&self) -> Self {
        match self {
            Variant8::V1(v) => Variant8::V1(crate::DeepClone::deep_clone(v)),
            Variant8::V2(v) => Variant8::V2(crate::DeepClone::deep_clone(v)),
            Variant8::V3(v) => Variant8::V3(crate::DeepClone::deep_clone(v)),
            Variant8::V4(v) => Variant8::V4(crate::DeepClone::deep_clone(v)),
            Variant8::V5(v) => Variant8::V5(crate::DeepClone::deep_clone(v)),
            Variant8::V6(v) => Variant8::V6(crate::DeepClone::deep_clone(v)),
            Variant8::V7(v) => Variant8::V7(crate::DeepClone::deep_clone(v)),
            Variant8::V8(v) => Variant8::V8(crate::DeepClone::deep_clone(v)),
        }
    }
}
