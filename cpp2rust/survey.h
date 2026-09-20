// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceLocation.h>
#include <llvm/Support/raw_ostream.h>

#include <string>
#include <string_view>

namespace cpp2rust {

// Survey mode.
//
// By default the translator aborts on the first construct it does not support,
// which makes sizing a large port a serial exercise: fix one gap, re-run, find
// the next. In survey mode those same sites record the construct and let
// translation continue with a placeholder instead. The emitted Rust is NOT
// usable -- the point is that a single run enumerates every gap in a codebase.
//
// Gaps are streamed to `path` and flushed as they are found, because the run
// that most needs surveying is the one that still dies partway through. Each
// line is `site <TAB> detail <TAB> location`.
void SetSurvey(const std::string &path);
bool IsSurvey();

// Records one unsupported construct. `site` names the translator location (a
// stable id, e.g. "CXXOperatorCallExpr"); `detail` describes the construct.
// Returns true when the caller should recover and carry on, false when it
// should keep its original fatal behaviour.
bool ReportUnsupported(std::string_view site, std::string_view detail);
bool ReportUnsupported(std::string_view site, std::string_view detail,
                       clang::SourceLocation loc, const clang::ASTContext &ctx);

// The placeholder token emitted in place of a construct that was surveyed
// rather than translated.
std::string UnsupportedPlaceholder(std::string_view site,
                                   std::string_view detail);

} // namespace cpp2rust
