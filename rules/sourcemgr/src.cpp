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
// std::unique_ptr<llvm::MemoryBuffer>, and its factories return
// `ErrorOr<std::unique_ptr<MemoryBuffer>>` while no erroror/expected module
// exists to carry that.  A rule for AddNewSourceBuffer would therefore have to
// invent a representation for the error channel.  It stays LOUD: the converter
// aborts naming the operation instead of answering from a guess.
//
// ---------------------------------------------------------------------------
// llvm::MemoryBuffer -- ADDED HERE, as t2, and a TYPE RULE IS THE WHOLE ROW.
// ---------------------------------------------------------------------------
// Publishing this module's IR (it had been committed but never generated into
// either shared tree, silently disabling it) moved ddc/ddl/ddl.cpp from
// `cpp_type: llvm::SourceMgr` on to `cpp_type: llvm::MemoryBuffer`.  It belongs
// here rather than in a new module because the two types are one decision: a
// SourceMgr IS a table of MemoryBuffers, so their representations must agree or
// AddNewSourceBuffer can never be written.
//
// SURFACE IN dt_src, measured not assumed.  56 lines mention MemoryBuffer
// outside ddb/'s vendored rapidjson (whose GenericMemoryBuffer is an unrelated
// type).  Of those, the ONLY member or static calls on llvm::MemoryBuffer in the
// entire tree are three `MemoryBuffer::getFileOrSTDIN(...)`:
// dxp/src/Driver/dxp-driver.cpp:115, dcc/src/Driver/dcc.cpp:78, and
// hcc/src/Driver/hcc.cpp:57 -- and the last is COMMENTED OUT.  Every other
// mention is the type appearing as `std::unique_ptr<llvm::MemoryBuffer>`: a
// parameter (21 sites: `buffer`, `ownedBuffer`, `chunkBuffer`), moved along and
// finally handed to SourceMgr::AddNewSourceBuffer or mlir::parseSourceFile.
//
// So dt_src calls ZERO accessors: no getBuffer, getBufferStart, getBufferEnd,
// getBufferSize, getBufferIdentifier, getMemBuffer or getMemBufferCopy anywhere.
//   grep -rn 'getBuffer\|getMemBuffer\|getBufferStart\|getBufferEnd\|\
//             getBufferIdentifier\|MemoryBuffer::' --include=*.cpp --include=*.h
//   (every other hit is an unrelated project name: DesignSpaceConfig::
//   getBufferCapacityForNode, DataFIFO::getBufferForRead, an MLIR op's
//   getBufferSize() accessor, getBufferizationOptions.)
// That matters for exactly the two hazards a MemoryBuffer model can get wrong:
//   * the GUARANTEED NUL just past the end, which getBufferEnd() and getBuffer()
//     (a StringRef that must EXCLUDE the NUL) expose -- unexercised, because
//     nothing reads bytes back out;
//   * getMemBuffer (BORROWS) versus getMemBufferCopy (COPIES), where modelling
//     one as the other silently changes whether a later mutation is visible --
//     unexercised, because neither factory is called.
// Neither can be fudged here because neither is reachable.  When an accessor is
// eventually needed, it must be added with the NUL boundary spelled out; this
// module deliberately does not pre-decide it.
//
// MODEL: Vec<u8>, one buffer's bytes -- the ELEMENT of SourceMgr's Vec<Vec<u8>>.
// Content-carrying for the same reason SourceMgr is: the parsers read bytes back
// out through it, so a transparent handle would be wrong the moment
// AddNewSourceBuffer is written.  The NUL terminator is NOT part of this Vec, to
// match getBuffer()'s StringRef; a getBufferEnd() rule would have to append it.
//
// THE FACTORIES STAY LOUD, deliberately.  getFileOrSTDIN and getFile are
// SYSCALLS returning `ErrorOr<std::unique_ptr<MemoryBuffer>>`, and the port has
// taken no decision on error_code -> io::Result -- rules/filesystem's header
// records that it left its filesystem-touching calls loud for exactly this
// reason.  Mapping them would mean inventing that decision, so the three call
// sites above keep aborting by name.  No constructor rule either: MemoryBuffer
// is ABSTRACT (`virtual ~MemoryBuffer()`), so there is no ctor to key.
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

class MemoryBuffer {
public:
  virtual ~MemoryBuffer();
};

class SourceMgr {
public:
  SourceMgr();
  SourceMgr(SourceMgr &&);
  ~SourceMgr();
};

} // namespace llvm

using t1 = llvm::SourceMgr;
using t2 = llvm::MemoryBuffer;

llvm::SourceMgr f1() { return llvm::SourceMgr(); }
