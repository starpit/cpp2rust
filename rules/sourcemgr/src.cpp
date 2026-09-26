// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// llvm::SourceMgr -- the buffer table LLVM/MLIR parsers read from.
//
// SURFACE IN dt_src.  Surveyed across dxp/ ddc/ dcc/ hcc/: SourceMgr is
// CONSTRUCTED (`std::make_shared<llvm::SourceMgr>()` in dxp/util.cpp:153,
// ddc/ddl/ddl.cpp:66, dcc-standalone-main.cpp:1283, hcc-standalone-main.cpp:662;
// a stack local in DxpOptMain.cpp:197, dcc-standalone-main.cpp:1098,
// dcc/src/Driver/dcc.cpp:85, hcc-standalone-main.cpp:482) and then only
// PASSED BY REFERENCE to mlir::SourceMgrDiagnosticHandler and
// parseSourceFileForTool.  Exactly ONE method is ever called on it:
// AddNewSourceBuffer(std::unique_ptr<MemoryBuffer>, SMLoc()), at 10 sites.
//
// WHAT IS MAPPED: the TYPE and the default constructor, nothing else.
//
// WHY AddNewSourceBuffer IS NOT MAPPED, deliberately.  It SWALLOWS a
// std::unique_ptr<llvm::MemoryBuffer>, and llvm::MemoryBuffer has no rule
// module anywhere in rules/: it is ABSTRACT (`virtual ~MemoryBuffer()`), dt_src
// calls ZERO methods on it, and its factories return
// `ErrorOr<std::unique_ptr<MemoryBuffer>>` while no erroror/expected module
// exists to carry that.  A rule for AddNewSourceBuffer would therefore have to
// invent a representation for its own argument.  It stays LOUD: the converter
// aborts naming the operation instead of answering from a guess.
//
// MODEL: the buffer table itself, Vec<Vec<u8>>, one entry per added buffer.
// SourceMgr is content-carrying -- it OWNS the buffers handed to it and the
// parsers read bytes back out through it -- so it must not be modelled as a
// transparent handle (a u64 or unit), which is what an "only ever passed on"
// reading would suggest.  A default-constructed SourceMgr has an empty table,
// and that is the only state any mapped operation can produce here, since the
// one mutator is loud.  When AddNewSourceBuffer (or the diagnostic handler's
// reads) are eventually ported, Vec<Vec<u8>> is the thing they grow into; the
// buffer identifiers LLVM hands back are 1-based indices into this same table.
//
// The declaration is restated locally rather than #include'd, for the reason
// rules/statistic, rules/raw_ostream, rules/stringref and rules/twine all
// state: cpp-rule-preprocessor compiles this file with a fixed flag set and
// reaching llvm/Support/SourceMgr.h would need an absolute -I into a built LLVM
// tree.  The signature a rule matches on is `void llvm::SourceMgr::SourceMgr()`.

namespace llvm {

class SourceMgr {
public:
  SourceMgr();
  SourceMgr(SourceMgr &&);
  ~SourceMgr();
};

} // namespace llvm

using t1 = llvm::SourceMgr;

llvm::SourceMgr f1() { return llvm::SourceMgr(); }
