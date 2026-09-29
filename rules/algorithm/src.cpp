// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

struct T2 {
  friend bool operator<(T2 a, T2 b) { return false; }
  bool operator()(const T2 &, const T2 &) const;
};

struct T1 {
  using value_type = T2;
  using difference_type = std::ptrdiff_t;
  using reference = T2 &;
  using pointer = T2 *;
  using iterator_category = std::random_access_iterator_tag;

  pointer p = nullptr;

  T1() = default;

  operator T2() const { return {}; }

  reference operator*() const { return *p; }
  pointer operator->() const { return p; }
  reference operator[](difference_type n) const { return p[n]; }

  T1 &operator++() {
    ++p;
    return *this;
  }
  T1 operator++(int) {
    T1 tmp = *this;
    ++*this;
    return tmp;
  }
  T1 &operator--() {
    --p;
    return *this;
  }
  T1 operator--(int) {
    T1 tmp = *this;
    --*this;
    return tmp;
  }

  T1 &operator+=(difference_type n) {
    p += n;
    return *this;
  }
  T1 &operator-=(difference_type n) {
    p -= n;
    return *this;
  }
  friend T1 operator+(T1 it, difference_type n) {
    it += n;
    return it;
  }
  friend T1 operator+(difference_type n, T1 it) {
    it += n;
    return it;
  }
  friend T1 operator-(T1 it, difference_type n) {
    it -= n;
    return it;
  }
  friend difference_type operator-(T1 a, T1 b) { return a.p - b.p; }

  friend bool operator==(T1 a, T1 b) { return a.p == b.p; }
  friend bool operator!=(T1 a, T1 b) { return a.p != b.p; }
  friend bool operator<(T1 a, T1 b) { return a.p < b.p; }
  friend bool operator>(T1 a, T1 b) { return a.p > b.p; }
  friend bool operator<=(T1 a, T1 b) { return a.p <= b.p; }
  friend bool operator>=(T1 a, T1 b) { return a.p >= b.p; }

  T1 &operator=(const T2 &rhs) { return *this; }
};

template <typename T1> void f1(T1 first, T1 last) {
  return std::sort(first, last);
}

template <typename T1, typename T2> T2 f2(T1 first, T1 last, T2 d_first) {
  return std::copy(first, last, d_first);
}

template <class T1, class T2> T1 f3(T1 first, T1 last, const T2 &value) {
  return std::find(first, last, value);
}

void f6(T1 first, T1 last, T2 comp) {
  return std::stable_sort(first, last, comp);
}

template <typename T1> T1 *f8(T1 *first, T1 *last) {
  return std::max_element(first, last);
}

template <typename T1> void f9(T1 &a0, T1 &a1) { return std::swap(a0, a1); }

template <typename T1>
typename std::vector<T1>::iterator f10(typename std::vector<T1>::iterator a0,
                                       typename std::vector<T1>::iterator a1) {
  return std::unique(a0, a1);
}

template <typename T1, typename T2>
void f12(typename std::vector<T1>::iterator a0,
         typename std::vector<T1>::iterator a1, const T2 &a2) {
  return std::fill(a0, a1, a2);
}

std::ostream_iterator<char> f13(std::string::iterator a0,
                                std::string::iterator a1,
                                std::ostream_iterator<char> a2) {
  return std::copy(a0, a1, a2);
}

void f14(T1 *first, T1 *last, T2 comp) {
  return std::stable_sort(first, last, comp);
}

template <typename T1> const T1 &f16(const T1 &a, const T1 &b) {
  return std::min(a, b);
}

template <typename T1> const T1 &f17(const T1 &a, const T1 &b) {
  return std::max(a, b);
}

// f18 -- `<algorithm>`'s FREE FUNCTION `std::replace(first, last, old, new)`.
// REATTRIBUTION: SENU-WORKLIST row 8 filed this under rules/string as
// `std::string::replace`.  That is WRONG.  The failing site (senulatorProg.cpp
// rustc.err:2945, .rs:4939) is
//     replace_65(___first, ___last, &mut ___old_value, &mut ___new_value)
// with `___first/___last: *mut libc::c_char` and `___old_value/___new_value:
// libc::c_char` -- an iterator pair plus two values, i.e. the <algorithm> free
// function, not a std::string member.  rules/string's only `replace` is f14
// `s.replace(pos, count, p, n)`, a different call entirely.
// `std::replace` takes NO callable (that is `replace_if`), so the CallableN
// convention f6/f14 need does not arise here.
// Iterator model: spelled `T1 *` as f8/f14 do (the site's iterators really are
// `char *`), and the value parameters as `const T1 &` -- which lowers to
// `*const T1` in the unsafe model and `Ptr<T1>` in refcount, matching f16/f17.
// The emitted call passes `&mut <lvalue>`, which coerces to `*const T1`.
// MEASURED READBACK CORRECTION: the `T1 *` spelling recorded as
//   void std::replace(T1 *, T1 *, const T1 &, const T1 &)
// and was DEAD -- `replace_65` survived unchanged.  The site's iterators are
// `std::string::iterator` (the converter lowers them to `*mut libc::c_char`,
// which is why the `T1 *` form looked right), so the key must be spelled on
// `std::string::iterator`, exactly as f13 does for `std::copy`.
void f18(std::string::iterator a0, std::string::iterator a1, const char &a2,
         const char &a3) {
  return std::replace(a0, a1, a2, a3);
}

// ===========================================================================
// ⭐⭐ SUPERSEDED IN PART by f19 below (2026-09-29).  The mechanism analysis in this
// block is CORRECT and is why f19 exists -- but its verdict "NOT KEYABLE TODAY /
// BOTH KEYS ARE DELIBERATELY ABSENT" is now FALSE for the BY-NAME (fn-pointer)
// spelling: converter commit b1c42014 made a mapped function referenced BY NAME in a
// rule-placeholder operand lower to the CALLABLE form instead of a zero-argument
// call, which is exactly the abort this block describes.  The TYPE-VARIABLE finding
// below still stands and is why the GENERIC spelling remains unwritten -- see f19.
// ===========================================================================
// ===========================================================================
// TODAY, AND THE REASON IS A CONVERTER GAP, NOT A MISSING SPELLING.  MEASURED
// 2026-09-29 on the GOAL TU (dxp/dxp_standalone.cpp), twice.
//
// The 3 placeholders are all the lowercasing idiom
// `transform(s.begin(), s.end(), s.begin(), <tolower>)`: two pass a LAMBDA
// (dxp_standalone.cpp:108, :116) and one passes `::tolower` BY NAME, i.e. an
// `int (*)(int) noexcept` (dsc/designSpaceConfig.h:318).
//
// Both spellings RECORD CORRECTLY -- read back out of ir_src.json:
//   std::__wrap_iter<char *> std::transform(std::__wrap_iter<char *>,
//       std::__wrap_iter<char *>, std::__wrap_iter<char *>, T1)
//   std::__wrap_iter<char *> std::transform(std::__wrap_iter<char *>,
//       std::__wrap_iter<char *>, std::__wrap_iter<char *>, int (*)(int) noexcept)
// (the operand must be T1, not T2: rule type variables must be CONSECUTIVE from
// T1 -- `rule-preprocessor` aborts `generics: not consecutive. Got: ["T2"]` --
// and f19's iterators are spelled concretely, so T1 is the only one available.
// It needs a unary `char operator()(char) const` on struct T1 to compile.)
//
// AND EITHER KEY TURNS THE GOAL TU FROM `A rc=0 38,377` INTO A BUCKET-B ABORT:
//   LLVM ERROR: rule body references placeholder a0 but the call site supplies
//   only 0 argument(s) at .../dsc/designSpaceConfig.h:318:66
// Column 66 is the ARGUMENT `::tolower`.  As soon as ANY rule matches the
// enclosing transform call, the converter lowers that argument as an ordinary
// expression, matches rules/cctype's `tolower` rule -- whose body references a0 --
// and aborts, because a function NAME supplies zero arguments.  The UNMAPPED path
// never reaches that step (it emits `Some(libcc2rs::tolower_unsafe)`), which is
// exactly why the site is harmless as a placeholder and fatal as a match.
// ⭐ A TYPE VARIABLE CANNOT DODGE IT: the operand placeholder unifies with
// `int (*)(int) noexcept` too, so the lambda-only key aborts at the SAME site --
// measured separately, with f20 removed and f19 alone in the tree.
// land first is the CONVERTER: passing a mapped function BY NAME as a callable
// operand must lower to the callable form (as the unmapped path already does)
// instead of being translated as a 0-argument call of that function's rule.
// ===========================================================================
// f19 -- `std::transform(first, last, d_first, <int (*)(int) noexcept>)`, the
// `::tolower` LOWERCASING IDIOM over a `std::string`, passed BY NAME.  It is
// keyable only since the converter learned to lower a by-name function operand
// as a CALLABLE instead of as a zero-argument call of that function's own rule
// (converter.cpp ConvertFnPtrCallee) -- before that, ANY rule matching the
// enclosing `transform` aborted with `rule body references placeholder a0 but
// the call site supplies only 0 argument(s)` at the OPERAND, not at the call.
//
// THE OPERAND IS SPELLED CONCRETELY, NOT AS A TYPE VARIABLE, ON PURPOSE.  A
// generic `T1` operand records fine and matches the two LAMBDA sites
// (`[](unsigned char c) { return std::tolower(c); }`) as well -- but one Rust
// `where T1: Callable1<A, R>` cannot serve both shapes: the by-name operand
// arrives as `unsafe fn(i32) -> i32` (`Callable1<i32, i32>`) and the lambda as a
// closure over `unsigned char`.  The lambda sites therefore stay LOUD rather
// than being swallowed into a body whose bound would be wrong for them.
//
// `std::string::iterator` for the iterators, exactly as f13/f18 do: the site's
// iterators are `std::__wrap_iter<char *>` and a `T1 *` spelling records a DEAD
// key (see f18's readback correction).
std::string::iterator f19(std::string::iterator a0, std::string::iterator a1,
                          std::string::iterator a2,
                          int (*a3)(int) noexcept) {
  return std::transform(a0, a1, a2, a3);
}
