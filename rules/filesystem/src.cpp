// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// std::filesystem::path.  Queue rows g2342-g2347 are `type` rows (all six point
// at libcxx/__filesystem/path.h:382, the class declaration, i.e. the type and
// nothing else) and g276 is the one operator row: `/` on two paths, 3 TUs and 9
// call sites across dbo/src/Transforms/EmitSpyreCode.cpp, dbo/src/Pipeline/
// Driver.cpp and dbo/src/Pipeline/Debug.cpp.
//
// MODEL: the path's native string, as a NON-NUL-TERMINATED Vec<u8>.
//
// WHY NOT TERMINATED.  rules/string's std::string IS NUL-terminated (its size()
// is len()-1) and rules/stringref's StringRef is too, but the BUFFER modules --
// rules/sstream, rules/basic_stringstream, rules/istringstream -- deliberately
// are not, and a path is a buffer, not a string: the only string-typed boundary
// it has is `.string()`, which CONSTRUCTS a std::string and so is exactly the
// place to add the terminator.  Keeping the terminator inside the path instead
// would put it in the middle of the buffer at every `a / b`, where the join has
// to append after it -- so the representation is chosen to make the join
// trivial and the conversion explicit, rather than the reverse.  The round trip
// is therefore closed by construction and is checked in the probe: f2 pops
// exactly one trailing 0 on the way in, f5 pushes exactly one on the way out,
// so `path(s).string() == s` byte for byte and rules/string's size() is right.
//
// A TYPE KEY NEEDS ITS CONSTRUCTOR -- without one the converter looks the
// default ctor up as an ordinary expr rule, misses, and falls back to
// `<mangled-type>::new()`, which gives rc=0 and then E0433.  So all three
// constructor forms the recorded call sites can reach are keyed even though no
// queue row names them: the default one (f1), the `std::string` one (f2, the
// form `std::filesystem::path(std::string(exportDir))` and
// `std::filesystem::path(dir.str())` use at EmitSpyreCode.cpp:99 and
// Debug.cpp:48/127/135), and the `const char *` one (f3, needed for the
// IMPLICIT conversion in `dir / "bundle.mlir"` at Driver.cpp:67 -- operator/
// takes `const path &`, so without f3 that site has no way to reach a path).
//
// `operator/` (f4) is libc++'s HIDDEN FRIEND at __filesystem/path.h:658,
// `friend path operator/(const path &, const path &)`, i.e. a FREE function in
// namespace std::filesystem.  It is spelled here as an UNQUALIFIED call, which
// ADL resolves: an INFIX `a0 / a1` would record NOTHING AT ALL and silently,
// and a QUALIFIED `std::filesystem::operator/(a0, a1)` aborts at
// cpp_rule_preprocessor.cpp:888 with `No viable function`.  It is not a member,
// so the `a0.operator/(a1)` member form does not apply to this overload.
//
// Semantics of the join, per [fs.path.append] on POSIX, where the preferred
// separator is '/':
//   * an ABSOLUTE right operand REPLACES the left one;
//   * otherwise the separator is inserted only if it is not already there --
//     so the result carries EXACTLY ONE separator, which is the discriminator
//     the probe checks (not concatenation and not replacement).
//
// LEFT OUT, deliberately, because no queue row reaches them and each would need
// a model this representation cannot express honestly: the MUTATING `operator/=`
// and `append` (a setter needs a &mut receiver the rule ABI cannot express --
// see "AN LVALUE-REFERENCE TARGET PARAMETER IS NOT ENFORCEABLE"), the
// decomposition accessors (parent_path/filename/stem/extension), the
// comparisons, the iterators, and every filesystem OPERATION (exists,
// create_directories, remove_all): those touch the real filesystem and belong
// with rules/stat and rules/dirent, not in a path value type.  `.c_str()` is
// also out: it would hand out a pointer into a buffer that has no terminator.

#include <filesystem>
#include <string>

using t1 = std::filesystem::path;

std::filesystem::path f1() { return std::filesystem::path(); }

std::filesystem::path f2(const std::string &a0) {
  return std::filesystem::path(a0);
}

std::filesystem::path f3(const char *a0) { return std::filesystem::path(a0); }

std::filesystem::path f4(const std::filesystem::path &a0,
                         const std::filesystem::path &a1) {
  return operator/(a0, a1);
}

std::string f5(const std::filesystem::path &a0) { return a0.string(); }

// path(std::string &&) -- libc++'s NON-template rvalue ctor at
// __filesystem/path.h:410, `path(string_type &&, format)`.  This is a SEPARATE
// key from f2 and it is the one a TEMPORARY selects, e.g.
// `std::filesystem::path(std::string(exportDir))` at EmitSpyreCode.cpp:99 and
// `std::filesystem::path(dir.str())` at Debug.cpp:48/127/135 -- every one of
// g276's path-construction sites passes a temporary, so without this key those
// sites emit `std_filesystem_path::new_N(...)` and fail with E0433.
std::filesystem::path f6(std::string &&a0) {
  return std::filesystem::path(std::move(a0));
}
