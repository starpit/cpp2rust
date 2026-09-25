// A lambda passed as a CALL ARGUMENT, which is where VisitLambdaExpr emits its
// closure through Convert(Expr *) rather than being inlined at a use of a named
// variable. Convert(Expr *) requires the expression to classify itself, and the
// closure path used to classify nothing: whether that was caught depended on
// what the lambda's BODY happened to leave in computed_expr_type_, because that
// is one converter-wide slot.
//
// So the empty-body lambda is the point of this test, not decoration: a body
// that converts nothing leaves the slot Unknown and the converter died with
// `Assertion `false && "computed_expr_type_ not set"' failed` -- a hard abort,
// invisible to --survey, and 11 of the 295 dxp TUs ended there. A body that
// ends on a value left a stale answer behind and the same closure was then
// classified by whatever its last subexpression was.
#include <assert.h>

static int log_ = 0;

template <typename F> static int run(F f, int x) {
  f(x);
  return x + 1;
}

int main() {
  // Empty body: nothing in it sets the expression kind.
  assert(run([](int) {}, 10) == 11);
  assert(log_ == 0);

  // A body that ends on a value, the case that used to pass by accident.
  assert(run([](int v) { log_ += v; }, 20) == 21);
  assert(log_ == 20);

  // A body whose last converted expression is a VALUELESS `return;`.
  assert(run(
             [](int v) {
               if (v > 100) {
                 return;
               }
               log_ += v * 2;
             },
             30) == 31);
  assert(log_ == 80);

  return 0;
}
