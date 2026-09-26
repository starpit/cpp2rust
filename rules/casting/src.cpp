// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// ---------------------------------------------------------------------------
// LLVM RTTI on MLIR ops: llvm::isa<Op>(Operation *), plus one TYPE rule per
// generated op marker.
//
// Measured from -verbose on a real TU: the call sites bind the `const From &`
// overload and the converter's SECOND-TIER key carries the explicit argument:
//     bool llvm::isa<mlir::arith::ConstantOp>(mlir::Operation *const &)
//     bool llvm::isa<mlir::IntegerAttr>(const mlir::Attribute &)   <-- NOT an op
//
// SCOPE WARNING 1 -- ATTRIBUTES MUST STAY LOUD.  The attribute form above goes
// through this same llvm::isa, and dataflowir-gen generates markers per OP only:
// there is no attribute row to compare.  `T2` is therefore PINNED to
// `mlir::Operation *` rather than left generic; a generic receiver would match
// the attribute form and answer it with OpInst's op-row comparison -- plausible
// output, wrong question.  Do NOT relax `mlir::Operation *const &`.
//
// SCOPE WARNING 2 -- Op-WRAPPER receivers are out of scope.  `isa<Op>(const
// mlir::vectorchain::CastOp &)`, where the receiver is a wrapper rather than an
// Operation*, is unmodelled.
//
// KEY-SPELLING NOTE: a tN covers the BARE and POINTER spellings of its type but
// NOT the reference-qualified one -- measured, `search type mlir::arith::ConstantOp &`
// returns None and falls back to `*mut`, which happens to be the right Rust type.
//
// The type rules exist because without them the converter substitutes its own
// BARE ported name into is_a::<>, which resolves to the opaque newtype it also
// ports (`pub struct mlir_arith_ConstantOp(pub u64);`) and then fails the MlirOp
// bound -- E0277, measured at 11 sites on VectorOperands.cpp.  Mapping the type
// makes the marker path fully qualified, needing no import.
// ---------------------------------------------------------------------------

namespace mlir {
class Operation;
class ModuleOp;
class UnrealizedConversionCastOp;
namespace affine {
class AffineApplyOp;
class AffineDelinearizeIndexOp;
class AffineForOp;
class AffineIfOp;
class AffineLinearizeIndexOp;
class AffineLoadOp;
class AffineMaxOp;
class AffineMinOp;
class AffineParallelOp;
class AffinePrefetchOp;
class AffineStoreOp;
class AffineVectorLoadOp;
class AffineVectorStoreOp;
class AffineYieldOp;
} // namespace affine
namespace agen {
class CompositeIndirectLoadAndStoreOp;
class CompositeIndirectLoadOp;
class CompositeIndirectStoreOp;
class CompositeLoadAndStoreOp;
class CompositeLoadOp;
class CompositeMemoryInterleaveOp;
class CompositeStoreOp;
class IndirectVectorLoadOp;
class IndirectVectorStoreOp;
class SetTransferMaskStateOp;
class SymbolicVectorLoadOp;
class SymbolicVectorStoreOp;
class VectorLoadOp;
class VectorStoreOp;
class YieldOp;
} // namespace agen
namespace arith {
class AddFOp;
class AddIOp;
class AddUIExtendedOp;
class AndIOp;
class BitcastOp;
class CeilDivSIOp;
class CeilDivUIOp;
class CmpFOp;
class CmpIOp;
class ConstantOp;
class DivFOp;
class DivSIOp;
class DivUIOp;
class ExtFOp;
class ExtSIOp;
class ExtUIOp;
class FPToSIOp;
class FPToUIOp;
class FloorDivSIOp;
class IndexCastOp;
class IndexCastUIOp;
class MaxNumFOp;
class MaxSIOp;
class MaxUIOp;
class MaximumFOp;
class MinNumFOp;
class MinSIOp;
class MinUIOp;
class MinimumFOp;
class MulFOp;
class MulIOp;
class MulSIExtendedOp;
class MulUIExtendedOp;
class NegFOp;
class OrIOp;
class RemFOp;
class RemSIOp;
class RemUIOp;
class SIToFPOp;
class ScalingExtFOp;
class ScalingTruncFOp;
class SelectOp;
class ShLIOp;
class ShRSIOp;
class ShRUIOp;
class SubFOp;
class SubIOp;
class TruncFOp;
class TruncIOp;
class UIToFPOp;
class XOrIOp;
} // namespace arith
namespace dataflow {
class CreateGroupOp;
class CreateMulticastGroupOp;
class GetLocalUnitOp;
class GetLogicalMemoryViewOp;
class GetPagedLogicalMemoryViewOp;
class GetUnitOp;
class ImplicitSyncOnStreamingBufferOp;
class OpaqueOp;
class ProgramUnitOp;
class ReceiveOp;
class ReturnOp;
class SendOp;
class SyncRecvOp;
class SyncSendOp;
} // namespace dataflow
namespace func {
class CallIndirectOp;
class CallOp;
class ConstantOp;
class FuncOp;
class ReturnOp;
} // namespace func
namespace scf {
class ConditionOp;
class ExecuteRegionOp;
class ForOp;
class ForallOp;
class IfOp;
class InParallelOp;
class IndexSwitchOp;
class ParallelOp;
class ReduceOp;
class ReduceReturnOp;
class WhileOp;
class YieldOp;
} // namespace scf
namespace sentient {
class AddOp;
class BinaryOp;
class ConstantOp;
class CopyOp;
class ForOp;
class IfOp;
class IncrMaskOp;
class LoadAndExtractScalarOp;
class LoadAndSendOp;
class LoadAndStoreOp;
class LoadComputeAndSendOp;
class LoadOp;
class LogicalPortOp;
class MacOp;
class MulOp;
class NOPOp;
class OpaqueOp;
class ReceiveAndExtractScalarOp;
class ReceiveAndStoreOp;
class SetActiveMaskValueOp;
class SetMaskOp;
class SetSendDestinationOp;
class SplatOp;
class SubOp;
class SyncOp;
class TernaryOp;
class UnaryOp;
class VectorConstantOp;
class YieldOp;
} // namespace sentient
namespace symbol {
class CreateIdOp;
class CreateSymbolOp;
class SymbolImmutableMappingOp;
class SymbolQueryMapOp;
} // namespace symbol
namespace trace {
class ProgramUnitTraceOp;
class ReturnOp;
} // namespace trace
namespace uniform {
class DefImmutableMappingOp;
class EqualizePatternOp;
class QueryMapOp;
class UniformizeRegionsOp;
class YieldOp;
} // namespace uniform
namespace vectorchain {
class BinaryOp;
class CastOp;
class ConstantBitstreamOp;
class CreateAffineMaskOp;
class ElementWiseCompareOp;
class ElementWiseSelectionOp;
class ExpEstimateOp;
class FastExpOp;
class FloorOp;
class LnEstimateOp;
class MergeOp;
class MultiplyAndAccumulateOp;
class MultiplyOp;
class NegOp;
class PackOp;
class RecEstimateOp;
class RotateOp;
class RsqrtEstimateOp;
class ScanWithGapOp;
class SelectOp;
class ShuffleOp;
class SigmoidEstimateOp;
class TanhEstimateOp;
} // namespace vectorchain
} // namespace mlir

namespace llvm {
template <typename To, typename From> bool isa(const From &Val);
} // namespace llvm

template <typename T1> bool f1(mlir::Operation *const &a0) {
  return llvm::isa<T1>(a0);
}

using t1 = mlir::ModuleOp;
using t2 = mlir::UnrealizedConversionCastOp;
using t3 = mlir::affine::AffineApplyOp;
using t4 = mlir::affine::AffineDelinearizeIndexOp;
using t5 = mlir::affine::AffineForOp;
using t6 = mlir::affine::AffineIfOp;
using t7 = mlir::affine::AffineLinearizeIndexOp;
using t8 = mlir::affine::AffineLoadOp;
using t9 = mlir::affine::AffineMaxOp;
using t10 = mlir::affine::AffineMinOp;
using t11 = mlir::affine::AffineParallelOp;
using t12 = mlir::affine::AffinePrefetchOp;
using t13 = mlir::affine::AffineStoreOp;
using t14 = mlir::affine::AffineVectorLoadOp;
using t15 = mlir::affine::AffineVectorStoreOp;
using t16 = mlir::affine::AffineYieldOp;
using t17 = mlir::agen::CompositeIndirectLoadAndStoreOp;
using t18 = mlir::agen::CompositeIndirectLoadOp;
using t19 = mlir::agen::CompositeIndirectStoreOp;
using t20 = mlir::agen::CompositeLoadAndStoreOp;
using t21 = mlir::agen::CompositeLoadOp;
using t22 = mlir::agen::CompositeMemoryInterleaveOp;
using t23 = mlir::agen::CompositeStoreOp;
using t24 = mlir::agen::IndirectVectorLoadOp;
using t25 = mlir::agen::IndirectVectorStoreOp;
using t26 = mlir::agen::SetTransferMaskStateOp;
using t27 = mlir::agen::SymbolicVectorLoadOp;
using t28 = mlir::agen::SymbolicVectorStoreOp;
using t29 = mlir::agen::VectorLoadOp;
using t30 = mlir::agen::VectorStoreOp;
using t31 = mlir::agen::YieldOp;
using t32 = mlir::arith::AddFOp;
using t33 = mlir::arith::AddIOp;
using t34 = mlir::arith::AddUIExtendedOp;
using t35 = mlir::arith::AndIOp;
using t36 = mlir::arith::BitcastOp;
using t37 = mlir::arith::CeilDivSIOp;
using t38 = mlir::arith::CeilDivUIOp;
using t39 = mlir::arith::CmpFOp;
using t40 = mlir::arith::CmpIOp;
using t41 = mlir::arith::ConstantOp;
using t42 = mlir::arith::DivFOp;
using t43 = mlir::arith::DivSIOp;
using t44 = mlir::arith::DivUIOp;
using t45 = mlir::arith::ExtFOp;
using t46 = mlir::arith::ExtSIOp;
using t47 = mlir::arith::ExtUIOp;
using t48 = mlir::arith::FPToSIOp;
using t49 = mlir::arith::FPToUIOp;
using t50 = mlir::arith::FloorDivSIOp;
using t51 = mlir::arith::IndexCastOp;
using t52 = mlir::arith::IndexCastUIOp;
using t53 = mlir::arith::MaxNumFOp;
using t54 = mlir::arith::MaxSIOp;
using t55 = mlir::arith::MaxUIOp;
using t56 = mlir::arith::MaximumFOp;
using t57 = mlir::arith::MinNumFOp;
using t58 = mlir::arith::MinSIOp;
using t59 = mlir::arith::MinUIOp;
using t60 = mlir::arith::MinimumFOp;
using t61 = mlir::arith::MulFOp;
using t62 = mlir::arith::MulIOp;
using t63 = mlir::arith::MulSIExtendedOp;
using t64 = mlir::arith::MulUIExtendedOp;
using t65 = mlir::arith::NegFOp;
using t66 = mlir::arith::OrIOp;
using t67 = mlir::arith::RemFOp;
using t68 = mlir::arith::RemSIOp;
using t69 = mlir::arith::RemUIOp;
using t70 = mlir::arith::SIToFPOp;
using t71 = mlir::arith::ScalingExtFOp;
using t72 = mlir::arith::ScalingTruncFOp;
using t73 = mlir::arith::SelectOp;
using t74 = mlir::arith::ShLIOp;
using t75 = mlir::arith::ShRSIOp;
using t76 = mlir::arith::ShRUIOp;
using t77 = mlir::arith::SubFOp;
using t78 = mlir::arith::SubIOp;
using t79 = mlir::arith::TruncFOp;
using t80 = mlir::arith::TruncIOp;
using t81 = mlir::arith::UIToFPOp;
using t82 = mlir::arith::XOrIOp;
using t83 = mlir::dataflow::CreateGroupOp;
using t84 = mlir::dataflow::CreateMulticastGroupOp;
using t85 = mlir::dataflow::GetLocalUnitOp;
using t86 = mlir::dataflow::GetLogicalMemoryViewOp;
using t87 = mlir::dataflow::GetPagedLogicalMemoryViewOp;
using t88 = mlir::dataflow::GetUnitOp;
using t89 = mlir::dataflow::ImplicitSyncOnStreamingBufferOp;
using t90 = mlir::dataflow::OpaqueOp;
using t91 = mlir::dataflow::ProgramUnitOp;
using t92 = mlir::dataflow::ReceiveOp;
using t93 = mlir::dataflow::ReturnOp;
using t94 = mlir::dataflow::SendOp;
using t95 = mlir::dataflow::SyncRecvOp;
using t96 = mlir::dataflow::SyncSendOp;
using t97 = mlir::func::CallIndirectOp;
using t98 = mlir::func::CallOp;
using t99 = mlir::func::ConstantOp;
using t100 = mlir::func::FuncOp;
using t101 = mlir::func::ReturnOp;
using t102 = mlir::scf::ConditionOp;
using t103 = mlir::scf::ExecuteRegionOp;
using t104 = mlir::scf::ForOp;
using t105 = mlir::scf::ForallOp;
using t106 = mlir::scf::IfOp;
using t107 = mlir::scf::InParallelOp;
using t108 = mlir::scf::IndexSwitchOp;
using t109 = mlir::scf::ParallelOp;
using t110 = mlir::scf::ReduceOp;
using t111 = mlir::scf::ReduceReturnOp;
using t112 = mlir::scf::WhileOp;
using t113 = mlir::scf::YieldOp;
using t114 = mlir::sentient::AddOp;
using t115 = mlir::sentient::BinaryOp;
using t116 = mlir::sentient::ConstantOp;
using t117 = mlir::sentient::CopyOp;
using t118 = mlir::sentient::ForOp;
using t119 = mlir::sentient::IfOp;
using t120 = mlir::sentient::IncrMaskOp;
using t121 = mlir::sentient::LoadAndExtractScalarOp;
using t122 = mlir::sentient::LoadAndSendOp;
using t123 = mlir::sentient::LoadAndStoreOp;
using t124 = mlir::sentient::LoadComputeAndSendOp;
using t125 = mlir::sentient::LoadOp;
using t126 = mlir::sentient::LogicalPortOp;
using t127 = mlir::sentient::MacOp;
using t128 = mlir::sentient::MulOp;
using t129 = mlir::sentient::NOPOp;
using t130 = mlir::sentient::OpaqueOp;
using t131 = mlir::sentient::ReceiveAndExtractScalarOp;
using t132 = mlir::sentient::ReceiveAndStoreOp;
using t133 = mlir::sentient::SetActiveMaskValueOp;
using t134 = mlir::sentient::SetMaskOp;
using t135 = mlir::sentient::SetSendDestinationOp;
using t136 = mlir::sentient::SplatOp;
using t137 = mlir::sentient::SubOp;
using t138 = mlir::sentient::SyncOp;
using t139 = mlir::sentient::TernaryOp;
using t140 = mlir::sentient::UnaryOp;
using t141 = mlir::sentient::VectorConstantOp;
using t142 = mlir::sentient::YieldOp;
using t143 = mlir::symbol::CreateIdOp;
using t144 = mlir::symbol::CreateSymbolOp;
using t145 = mlir::symbol::SymbolImmutableMappingOp;
using t146 = mlir::symbol::SymbolQueryMapOp;
using t147 = mlir::trace::ProgramUnitTraceOp;
using t148 = mlir::trace::ReturnOp;
using t149 = mlir::uniform::DefImmutableMappingOp;
using t150 = mlir::uniform::EqualizePatternOp;
using t151 = mlir::uniform::QueryMapOp;
using t152 = mlir::uniform::UniformizeRegionsOp;
using t153 = mlir::uniform::YieldOp;
using t154 = mlir::vectorchain::BinaryOp;
using t155 = mlir::vectorchain::CastOp;
using t156 = mlir::vectorchain::ConstantBitstreamOp;
using t157 = mlir::vectorchain::CreateAffineMaskOp;
using t158 = mlir::vectorchain::ElementWiseCompareOp;
using t159 = mlir::vectorchain::ElementWiseSelectionOp;
using t160 = mlir::vectorchain::ExpEstimateOp;
using t161 = mlir::vectorchain::FastExpOp;
using t162 = mlir::vectorchain::FloorOp;
using t163 = mlir::vectorchain::LnEstimateOp;
using t164 = mlir::vectorchain::MergeOp;
using t165 = mlir::vectorchain::MultiplyAndAccumulateOp;
using t166 = mlir::vectorchain::MultiplyOp;
using t167 = mlir::vectorchain::NegOp;
using t168 = mlir::vectorchain::PackOp;
using t169 = mlir::vectorchain::RecEstimateOp;
using t170 = mlir::vectorchain::RotateOp;
using t171 = mlir::vectorchain::RsqrtEstimateOp;
using t172 = mlir::vectorchain::ScanWithGapOp;
using t173 = mlir::vectorchain::SelectOp;
using t174 = mlir::vectorchain::ShuffleOp;
using t175 = mlir::vectorchain::SigmoidEstimateOp;
using t176 = mlir::vectorchain::TanhEstimateOp;
