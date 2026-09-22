#include <array>
#include <cassert>
#include <utility>
#include <vector>

// `f({v})` where `f` takes a reference and `v` already has the parameter's type.
// No initializer_list and no aggregate can be formed from that, so the
// reference binds DIRECTLY to `v`: no temporary, no copy, no move. The braces
// are pure syntax.

static int sum_vec(const std::vector<int> &v) {
  int n = 0;
  for (int x : v)
    n += x;
  return n;
}

static int sum_vec_rvref(std::vector<int> &&v) {
  int n = 0;
  for (int x : v)
    n += x;
  return n;
}

static int sum_arr(const std::array<int, 3> &a) { return a[0] + a[1] + a[2]; }

struct Two {
  int a;
  int b;
};

static int sum_two(const Two &t) { return t.a + t.b; }

// A one-field aggregate whose field has the element's type is the other side of
// the boundary: `{i}` there is a real aggregate init and MUST build a value.
struct One {
  int x;
};

static int get_one(const One &o) { return o.x; }

int main() {
  std::vector<int> v = {1, 2, 3};
  assert(sum_vec({v}) == 6);
  // Binding did not consume `v`.
  assert(v.size() == 3);

  // An xvalue element binds directly too, and moves nothing.
  assert(sum_vec({std::move(v)}) == 6);
  assert(v.size() == 3);
  assert(sum_vec_rvref({std::move(v)}) == 6);
  assert(v.size() == 3);

  // std::array used to silently translate to a DEFAULT-filled array here,
  // dropping the element, so this asserted 0 == 60 in Rust while C++ said 60.
  std::array<int, 3> a = {10, 20, 30};
  assert(sum_arr({a}) == 60);
  assert(sum_arr(a) == 60);

  Two t = {4, 5};
  assert(sum_two({t}) == 9);
  // Real aggregate init of the same record still works.
  assert(sum_two({6, 7}) == 13);

  int i = 8;
  assert(get_one({i}) == 8);
  One o = {9};
  assert(get_one({o}) == 9);

  return 0;
}
