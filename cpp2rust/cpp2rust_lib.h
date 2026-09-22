#pragma once

// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include <string>
#include <vector>

#include "converter/factory.h"

namespace cpp2rust {

std::string TranspileSrc(std::string_view cc_code, Model model,
                         const std::vector<std::string_view> &cxx_flags,
                         const std::string &rules_dir,
                         std::string_view filename);

// `ok` is false when clang failed on any TU -- a missing header, a parse error.
// The caller MUST honour it: a TU whose header could not be opened still yields
// a non-empty string, translated from an incomplete AST, so "did we produce
// output" is not the same question as "did it work". Scoring that as success is
// how survey.py came to report OK for TUs that had silently skipped a header.
std::string TranspileDir(std::string_view build_dir, Model model,
                         const std::string &rules_dir, bool *ok = nullptr);

} // namespace cpp2rust
