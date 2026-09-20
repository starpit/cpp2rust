#include <cassert>
#include <deque>

// The DerivedToBase cast target here is spelled `std::deque<Base *>::value_type`
// -- a typedef. isPointerType() looks through sugar, dyn_cast<PointerType> does
// not, so the guarded dyn_cast handed back null and getPointeeType() asserted.
struct Base {
  virtual ~Base() = default;
  virtual int id() const = 0;
};

struct Derived : Base {
  int id() const override { return 7; }
};

int main() {
  Derived x;

  Base *b = &x;
  assert(b->id() == 7);

  std::deque<Base *> d;
  d.push_front(&x);
  d.push_back(&x);
  assert(d.size() == 2);
  assert(!d.empty());

  return 0;
}
