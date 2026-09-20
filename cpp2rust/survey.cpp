// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

#include "survey.h"

#include <memory>
#include <system_error>

namespace cpp2rust {
namespace {

std::unique_ptr<llvm::raw_fd_ostream> out_;

// Tabs and newlines would break the one-line-per-gap format.
std::string Sanitize(std::string_view s) {
  std::string out;
  out.reserve(s.size());
  for (char c : s) {
    out += (c == '\t' || c == '\n' || c == '\r') ? ' ' : c;
  }
  return out;
}

bool Record(std::string_view site, std::string_view detail,
            const std::string &loc) {
  if (!out_) {
    return false;
  }
  *out_ << Sanitize(site) << '\t' << Sanitize(detail) << '\t'
        << (loc.empty() ? "-" : Sanitize(loc)) << '\n';
  // A survey run is expected to die partway through, so pay for the flush.
  out_->flush();
  return true;
}

} // namespace

void SetSurvey(const std::string &path) {
  if (path.empty()) {
    out_.reset();
    return;
  }
  std::error_code ec;
  auto os = std::make_unique<llvm::raw_fd_ostream>(path, ec);
  if (ec) {
    llvm::errs() << "ERROR: failed to open survey file " << path << ": "
                 << ec.message() << '\n';
    return;
  }
  out_ = std::move(os);
}

bool IsSurvey() { return out_ != nullptr; }

bool ReportUnsupported(std::string_view site, std::string_view detail) {
  return Record(site, detail, {});
}

bool ReportUnsupported(std::string_view site, std::string_view detail,
                       clang::SourceLocation loc,
                       const clang::ASTContext &ctx) {
  if (!out_) {
    return false;
  }
  return Record(site, detail,
                loc.isValid() ? loc.printToString(ctx.getSourceManager())
                              : std::string());
}

std::string UnsupportedPlaceholder(std::string_view site,
                                   std::string_view detail) {
  std::string out = "cpp2rust_unsupported!(\"";
  out += site;
  if (!detail.empty()) {
    out += ": ";
    // The detail lands inside a Rust string literal.
    for (char c : detail) {
      if (c == '"' || c == '\\') {
        out += '\\';
      }
      if (c == '\n') {
        out += "\\n";
        continue;
      }
      out += c;
    }
  }
  out += "\")";
  return out;
}

} // namespace cpp2rust
