#include <cassert>
#include <string>

// `std::string::npos` is a constant declared in a header nothing translates,
// so there is no Rust global to name it by: it used to come out as a
// reference to an undeclared `npos_0`. Clang folds the use, so emit the value.
static bool found(size_t pos) { return pos != std::string::npos; }

int main() {
  size_t n = std::string::npos;
  assert(n != 0);
  assert(n == (size_t)-1);
  assert(!found(n));
  assert(found(3));

  size_t m = std::string::npos;
  m = m - 1;
  assert(m == (size_t)-2);

  return 0;
}
