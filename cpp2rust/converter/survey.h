#pragma once

// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// Survey (discovery) mode.
//
// Normally the converter aborts on the first construct it cannot translate,
// so one run yields one observation. Survey mode instead RECORDS every gap
// and keeps going, so a whole translation unit can be inventoried in one run.
//
// Survey mode deliberately produces NO Rust output: it is a discovery pass.
// Gaps out, no code out. See cpp2rust.cpp, which skips writing -o entirely
// when survey mode is on.

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

namespace cpp2rust::survey {

enum class GapKind : uint8_t {
  kUnmappedType,
  kUnsupportedExpr,
  kUnsupportedConstruct,
  kMissingTraitBody,
};

inline const char *KindName(GapKind kind) {
  switch (kind) {
  case GapKind::kUnmappedType:
    return "unmapped-type";
  case GapKind::kUnsupportedExpr:
    return "unsupported-expr";
  case GapKind::kUnsupportedConstruct:
    return "unsupported-construct";
  case GapKind::kMissingTraitBody:
    return "missing-trait-body";
  }
  return "unknown";
}

struct Gap {
  unsigned long count = 0;
  // Distinct enclosing contexts this gap was reached through, in first-seen
  // order. This is what makes a reachability DAG possible rather than a flat
  // list: it says a rule for X is only needed because of Y.
  std::vector<std::string> contexts;
  unsigned long distinct_contexts = 0;
};

struct State {
  bool enabled = false;
  std::string out_path;
  std::string tu;
  // (kind, detail) -> Gap
  std::map<std::pair<std::string, std::string>, Gap> gaps;
  // Enclosing function/class most recently entered by the converter.
  std::string current_scope;
  bool dirty = false;
};

inline State &state() {
  static State s;
  return s;
}

inline bool Enabled() { return state().enabled; }

inline std::string Sanitize(std::string s) {
  for (auto &c : s) {
    if (c == '\t' || c == '\n' || c == '\r') {
      c = ' ';
    }
  }
  return s;
}

inline void Write() {
  auto &s = state();
  if (s.out_path.empty()) {
    return;
  }
  FILE *f = std::fopen(s.out_path.c_str(), "w");
  if (!f) {
    return;
  }
  std::fprintf(f, "#tu\t%s\n", s.tu.c_str());
  std::fprintf(f, "kind\tdetail\tcount\tcontext\tdistinct_contexts\n");
  for (const auto &[key, gap] : s.gaps) {
    std::string ctx;
    for (const auto &c : gap.contexts) {
      if (!ctx.empty()) {
        ctx += " | ";
      }
      ctx += c;
    }
    std::fprintf(f, "%s\t%s\t%lu\t%s\t%lu\n", key.first.c_str(),
                 key.second.c_str(), gap.count, ctx.c_str(),
                 gap.distinct_contexts);
  }
  std::fclose(f);
  s.dirty = false;
}

inline void OnAbort(int sig) {
  Write();
  std::signal(sig, SIG_DFL);
  std::raise(sig);
}

inline void Enable(const std::string &out_path, const std::string &tu) {
  auto &s = state();
  s.enabled = true;
  s.out_path = out_path;
  s.tu = tu;
  // A survey run may still trip an assert we have not routed yet; make sure
  // everything discovered so far survives that.
  std::atexit([] { Write(); });
  std::signal(SIGABRT, OnAbort);
  std::signal(SIGSEGV, OnAbort);
  Write();
}

// Enclosing function/class, set by the converter as it walks declarations.
inline void SetScope(std::string scope) {
  state().current_scope = std::move(scope);
}
inline const std::string &Scope() { return state().current_scope; }

// Records one gap occurrence. `detail` is the exact thing a rule author must
// key on (a C++ type string, an operator spelling, ...). `where` is the
// file:line of the site.
inline void Record(GapKind kind, const std::string &detail,
                   const std::string &where) {
  auto &s = state();
  std::string context = s.current_scope.empty() ? "<top-level>" : s.current_scope;
  if (!where.empty()) {
    context += " @ " + where;
  }
  context = Sanitize(context);
  auto &gap = s.gaps[{KindName(kind), Sanitize(detail)}];
  ++gap.count;
  bool new_gap = gap.count == 1;
  bool new_context = true;
  for (const auto &c : gap.contexts) {
    if (c == context) {
      new_context = false;
      break;
    }
  }
  if (new_context) {
    ++gap.distinct_contexts;
    if (gap.contexts.size() < 5) {
      gap.contexts.push_back(context);
    }
  }
  if (new_gap || new_context) {
    // Cheap crash-resilience: the file is small, rewrite it whenever the
    // distinct set grows.
    Write();
  }
}

} // namespace cpp2rust::survey
