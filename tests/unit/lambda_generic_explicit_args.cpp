// no-compile
// Generic lambdas TRANSLATE now (refcount defers a generic lambda var-decl to the
// base model's inline-at-each-use strategy), so the old `translation-fail` marker is
// obsolete.  But an EXPLICIT-template-argument call on the closure still emits
// `operator_call_<sig>` methods that do not exist on a Rust closure --
// `error[E0599]: no method named operator_call_char_char_const`.  Loud at rustc, not
// a wrong value, so `no-compile` is the accurate marker until the call-operator
// selection is mapped for the explicit-args spelling.
// base model's inline-at-each-use strategy, which is the only one that can express
// one at two instantiations (a Rust closure value is monomorphic).  This test's
// `translation-fail` marker documented that limitation and is now obsolete, so the
// test is promoted to a real end-to-end check: lit translates, compiles, runs and
// diffs against the same source built with clang.
#include <assert.h>

struct Val {
  int x;
};

static int sum(Val a, Val b) { return a.x + b.x; }

int main() {
  int total = 0;
  auto tally = [&total]<typename T, typename U> {
    total += sizeof(T) + sizeof(U);
  };
  tally.operator()<char, char>();
  tally.operator()<int, char>();
  assert(total == 7);

  Val v{5};
  int acc = 0;
  auto pick = [&v, &acc]<typename Q> { acc += sum(static_cast<Q>(v), v); };
  pick.operator()<Val &>();
  pick.operator()<const Val &>();
  pick.operator()<Val &&>();
  assert(acc == 30);

  auto cast_to = []<typename T>(int x) { return static_cast<T>(x) / 2; };
  assert(cast_to.operator()<int>(5) == 2);
  assert(cast_to.operator()<double>(5) == 2.5);

  return 0;
}
