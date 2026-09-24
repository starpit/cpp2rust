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

// Whether this run recorded at least one gap.
//
// Survey mode exists so that ONE run enumerates every gap, and to do that it
// recovers past each one. But recovery makes the run EXIT ZERO, so a harness
// that scores a TU by exit status counts a surveyed TU as a success -- and the
// two errors compound in opposite directions:
//
//   * a census run with --survey inflates its OK count (measured: 9 TUs of 58
//     scored OK while having recorded gaps);
//   * and because recovery takes a different code path, the gap that is recorded
//     is not always the one that would have been fatal -- a refusal site can be
//     reached in the non-recovered run and MISSED by the survey. (Measured: the
//     OstreamInsertion refusal on dsc/pcfg.cpp's DataFormats appears when the TU
//     is translated normally and appears nowhere in its survey TSV.)
//
// So survey mode must enumerate gaps AND still report failure. This lets the
// caller exit non-zero when anything was recorded, which keeps "what gaps are
// there" and "does this TU translate" from being answered by the same number.
bool SurveyFoundGaps();

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
