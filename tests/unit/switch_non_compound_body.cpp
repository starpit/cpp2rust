#include <cassert>

// The grammar allows any statement as a switch body, not just a compound one.
// gtest's GTEST_AMBIGUOUS_ELSE_BLOCKER_ (used by every EXPECT_*/ASSERT_*)
// expands to exactly this, so the body is a CaseStmt.
#define BLOCKER                                                                \
  switch (0)                                                                   \
  case 0:                                                                      \
  default:

static int blocker(int x) {
  int r = 0;
  BLOCKER if (x > 0) r = 1;
  else r = 2;
  return r;
}

static int single_case(int x) {
  int r = 0;
  switch (x)
  case 1:
    r = 7;
  return r;
}

int main() {
  assert(blocker(5) == 1);
  assert(blocker(-5) == 2);
  assert(single_case(1) == 7);
  assert(single_case(2) == 0);
  return 0;
}
