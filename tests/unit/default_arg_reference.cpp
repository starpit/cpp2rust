#include <cassert>
#include <string>
#include <vector>

// A default argument on a REFERENCE parameter. The parameter type lowers to a
// POINTER, so the default cannot be the default VALUE -- it has to be a pointer
// to storage. In C++ the default `{}` is a temporary materialised by the binding
// whose lifetime reaches the end of the full-expression containing the call, so
// the callee sees a live reference to a default-constructed object.
//
// The default is also evaluated LAZILY: only at the call sites that omit it.
static int called = 0;

static std::vector<long> make() {
  ++called;
  return std::vector<long>{1, 2};
}

static long sum(const std::vector<long> &values = {}) {
  long acc = (long)values.size() * 1000;
  for (unsigned i = 0; i < values.size(); ++i) {
    acc += values[i];
  }
  return acc;
}

static int len(const std::string &name = "abc") { return (int)name.size(); }

static long scalar(const long &k = 7) { return k; }

static long lazy(const std::vector<long> &values = make()) {
  return (long)values.size();
}

// An lvalue default names an object that already exists, so the reference must
// denote that very object rather than a copy of it.
static std::vector<long> shared{9};

static void grow(std::vector<long> &values = shared) { values.push_back(1); }

struct Holder {
  long n;
  explicit Holder(const std::vector<long> &values = {}) {
    n = (long)values.size();
  }
};

int main() {
  assert(sum() == 0);
  std::vector<long> x{4, 5, 6};
  assert(sum(x) == 3015);

  assert(len() == 3);
  std::string y = "hi";
  assert(len(y) == 2);

  assert(scalar() == 7);
  long k = 42;
  assert(scalar(k) == 42);

  assert(called == 0);
  assert(lazy(x) == 3);
  assert(called == 0);
  assert(lazy() == 2);
  assert(called == 1);

  grow();
  assert(shared.size() == 2);
  std::vector<long> own{};
  grow(own);
  assert(own.size() == 1);
  assert(shared.size() == 2);

  Holder a;
  assert(a.n == 0);
  Holder b(x);
  assert(b.n == 3);

  return 0;
}
