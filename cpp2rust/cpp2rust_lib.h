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
// `cxx_flags` are the --cxxflags tokens, appended AFTER the compile database's
// own flags for each TU exactly as TranspileSrc appends them for --file. They
// are how a caller supplies what a compile_commands.json cannot carry -- most
// importantly `--sysroot`, without which `#include_next <inttypes.h>` in clang's
// resource-dir header resolves to nothing and EVERY TU fails (measured; see the
// note at the former --dir/--cxxflags refusal in cpp2rust.cpp).
std::string TranspileDir(std::string_view build_dir, Model model,
                         const std::vector<std::string_view> &cxx_flags,
                         const std::string &rules_dir);

} // namespace cpp2rust
