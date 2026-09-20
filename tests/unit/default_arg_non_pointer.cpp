#include <cassert>
#include <string>

// A defaulted argument is passed as `None`, so its expression is never
// converted -- but a class-typed default is a MaterializeTemporaryExpr, and
// hoisting it emitted a binding nothing reads by converting the
// CXXDefaultArgExpr, which asserted.
static int tag(int v, std::string suffix = "") {
  return v + (int)suffix.size();
}

struct Holder {
  int n;
  explicit Holder(int n, std::string name = "anon") : n(n + (int)name.size()) {}
};

int main() {
  assert(tag(1) == 1);
  assert(tag(1, "abc") == 4);

  Holder a(0);
  assert(a.n == 4);

  Holder b(0, "xy");
  assert(b.n == 2);

  return 0;
}
