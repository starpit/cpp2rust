// The expression lookup key carries the template arguments that a resolved
// signature cannot show: `std::holds_alternative<float>` and
// `std::holds_alternative<int>` print one and the same signature, and so do
// `std::get<0>` and `std::get<1>` on a `std::tuple<int, int>`.
//
// Every call below goes through that path, and every one of them must keep
// matching the rule it matched before, because no rule in rules/ is keyed by
// a template argument yet:
//
//   std::get<I>          a non-deduced VALUE argument -- the key gains <0>
//                        and <1>, and the rules still match because the
//                        lookup falls back to the signature-only spelling
//   make_unique<T>,      a non-deduced TYPE argument -- the key gains <i32>
//   make_shared<T>
//   unique_ptr<T>(),     libc++ spells these with a defaulted dummy
//   pair<T, U>()         parameter (`bool _Dummy = true`, and pair's
//                        `__check_pair_construction`). A DEFAULTED parameter
//                        is never part of the key, or 198 of the 1609
//                        existing rules would be respelled in terms of a
//                        standard library implementation detail.
//   std::max, std::min   fully deduced -- the key must not move at all
#include <algorithm>
#include <cstdio>
#include <memory>
#include <tuple>
#include <utility>

int main() {
  std::tuple<int, double> t{1, 2.5};
  printf("%d %d\n", std::get<0>(t), (int)(std::get<1>(t) * 2));

  auto owned = std::make_unique<int>(7);
  printf("%d\n", *owned);

  auto shared = std::make_shared<int>(9);
  printf("%d\n", *shared);

  std::unique_ptr<int> empty;
  printf("%d\n", empty.get() == nullptr);

  std::pair<int, int> pair;
  printf("%d %d\n", pair.first, pair.second);

  int a = 3;
  int b = 4;
  int hi = std::max(a, b);
  int lo = std::min(a, b);
  printf("%d %d\n", hi, lo);
  return 0;
}
