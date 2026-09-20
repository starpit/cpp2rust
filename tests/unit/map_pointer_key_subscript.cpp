#include <cassert>
#include <map>
#include <unordered_map>

struct Node {
  int x;
};

// `const T1 &key` in the map rule instantiates to `Node *const &`, which is
// spelled east of the `*`. Subscripting used to fall through to the generic
// pointer path and translate to pointer arithmetic on the map.
static void put(std::map<Node *, int> &m, Node *k, int v) { m[k] = v; }

int main() {
  Node a{1};
  Node b{2};

  std::map<Node *, int> m;
  put(m, &a, 10);
  put(m, &b, 20);
  put(m, &a, 11);
  assert(m.size() == 2);
  assert(m[&a] == 11);
  assert(m[&b] == 20);
  assert(m.at(&a) == 11);

  std::unordered_map<const Node *, int> um;
  const Node *ca = &a;
  um[ca] = 5;
  assert(um.size() == 1);
  assert(um[ca] == 5);

  return 0;
}
