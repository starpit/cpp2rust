// no-compile
//
// The generated output does not compile: an inheriting constructor is CALLED
// but never EMITTED. The marker takes a MODEL LIST after its colon, not prose,
// so it is spelled bare and the reason is here.
//
// `using Base::Base;` gives Derived a constructor of its own, and clang builds
// fresh ParmVarDecls for it that carry NO default argument -- the defaults stay
// on Base's constructor, where [class.inhctor.init] puts them, so overload
// resolution at `Derived d1(1);` builds a CXXDefaultArgExpr against BASE's
// parameter. Asking Derived's own parameter answered "no default" and
// ConvertCXXConstructExprArgs died on `Assertion `has_default' failed`: a hard
// abort, invisible to --survey, on 5 of the 295 dxp TUs
// (mlir::affine::FlatAffineValueConstraints, which inherits
// FlatLinearValueConstraints(IntegerSet, ValueRange = {}), and dt_src's own
// DuplicateReusedTogglePattern, which inherits mlir's OpRewritePattern).
//
// Translation now succeeds and the call sites come out as
// `Derived::new_1({ 1 }, None)` and `Derived::new_1({ 2 }, { Some(3) })` -- the
// same Option convention the base's own constructor is emitted with, so the
// default `7` is applied in the None arm only.
//
// The output does not COMPILE, for a separate and pre-existing reason:
// EmitInheritedStructMethods re-emits a concrete base's methods on the derived
// struct but deliberately skips CXXConstructorDecl, so `impl Derived` has no
// `new_1` and rustc says E0599. That is true with or without a default argument
// in the picture -- `struct D : B { using B::B; };` with a single-parameter base
// constructor and no defaults emits the same dangling call -- so it is marked
// here rather than worked around. Supplying that constructor by hand (the body
// the converter already emits for Base, renamed) and running it prints
// `d1=107 d2=203` in both models, which is what clang-built C++ prints.
#include <assert.h>
#include <stdio.h>

struct Base {
  int a;
  int b;
  Base(int a_, int b_ = 7) : a(a_), b(b_) {}
  int code() const { return a * 100 + b; }
};

struct Derived : Base {
  using Base::Base;
};

int main() {
  Derived d1(1);    // b defaults to 7
  Derived d2(2, 3); // b is given
  printf("d1=%d d2=%d\n", d1.code(), d2.code());
  assert(d1.code() == 107);
  assert(d2.code() == 203);
  return 0;
}
