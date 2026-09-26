// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

fn f1<T1: dataflowir_gen::MlirOp>(a0: *mut dataflowir_gen::fmt::OpInst) -> bool {
    dataflowir_gen::fmt::OpInst::is_a::<T1>(unsafe { &*a0 })
}

fn t1() -> dataflowir_gen::ops::mlir_ModuleOp {
    dataflowir_gen::ops::mlir_ModuleOp
}
fn t2() -> dataflowir_gen::ops::mlir_UnrealizedConversionCastOp {
    dataflowir_gen::ops::mlir_UnrealizedConversionCastOp
}
fn t3() -> dataflowir_gen::ops::mlir_affine_AffineApplyOp {
    dataflowir_gen::ops::mlir_affine_AffineApplyOp
}
fn t4() -> dataflowir_gen::ops::mlir_affine_AffineDelinearizeIndexOp {
    dataflowir_gen::ops::mlir_affine_AffineDelinearizeIndexOp
}
fn t5() -> dataflowir_gen::ops::mlir_affine_AffineForOp {
    dataflowir_gen::ops::mlir_affine_AffineForOp
}
fn t6() -> dataflowir_gen::ops::mlir_affine_AffineIfOp {
    dataflowir_gen::ops::mlir_affine_AffineIfOp
}
fn t7() -> dataflowir_gen::ops::mlir_affine_AffineLinearizeIndexOp {
    dataflowir_gen::ops::mlir_affine_AffineLinearizeIndexOp
}
fn t8() -> dataflowir_gen::ops::mlir_affine_AffineLoadOp {
    dataflowir_gen::ops::mlir_affine_AffineLoadOp
}
fn t9() -> dataflowir_gen::ops::mlir_affine_AffineMaxOp {
    dataflowir_gen::ops::mlir_affine_AffineMaxOp
}
fn t10() -> dataflowir_gen::ops::mlir_affine_AffineMinOp {
    dataflowir_gen::ops::mlir_affine_AffineMinOp
}
fn t11() -> dataflowir_gen::ops::mlir_affine_AffineParallelOp {
    dataflowir_gen::ops::mlir_affine_AffineParallelOp
}
fn t12() -> dataflowir_gen::ops::mlir_affine_AffinePrefetchOp {
    dataflowir_gen::ops::mlir_affine_AffinePrefetchOp
}
fn t13() -> dataflowir_gen::ops::mlir_affine_AffineStoreOp {
    dataflowir_gen::ops::mlir_affine_AffineStoreOp
}
fn t14() -> dataflowir_gen::ops::mlir_affine_AffineVectorLoadOp {
    dataflowir_gen::ops::mlir_affine_AffineVectorLoadOp
}
fn t15() -> dataflowir_gen::ops::mlir_affine_AffineVectorStoreOp {
    dataflowir_gen::ops::mlir_affine_AffineVectorStoreOp
}
fn t16() -> dataflowir_gen::ops::mlir_affine_AffineYieldOp {
    dataflowir_gen::ops::mlir_affine_AffineYieldOp
}
fn t17() -> dataflowir_gen::ops::mlir_agen_CompositeIndirectLoadAndStoreOp {
    dataflowir_gen::ops::mlir_agen_CompositeIndirectLoadAndStoreOp
}
fn t18() -> dataflowir_gen::ops::mlir_agen_CompositeIndirectLoadOp {
    dataflowir_gen::ops::mlir_agen_CompositeIndirectLoadOp
}
fn t19() -> dataflowir_gen::ops::mlir_agen_CompositeIndirectStoreOp {
    dataflowir_gen::ops::mlir_agen_CompositeIndirectStoreOp
}
fn t20() -> dataflowir_gen::ops::mlir_agen_CompositeLoadAndStoreOp {
    dataflowir_gen::ops::mlir_agen_CompositeLoadAndStoreOp
}
fn t21() -> dataflowir_gen::ops::mlir_agen_CompositeLoadOp {
    dataflowir_gen::ops::mlir_agen_CompositeLoadOp
}
fn t22() -> dataflowir_gen::ops::mlir_agen_CompositeMemoryInterleaveOp {
    dataflowir_gen::ops::mlir_agen_CompositeMemoryInterleaveOp
}
fn t23() -> dataflowir_gen::ops::mlir_agen_CompositeStoreOp {
    dataflowir_gen::ops::mlir_agen_CompositeStoreOp
}
fn t24() -> dataflowir_gen::ops::mlir_agen_IndirectVectorLoadOp {
    dataflowir_gen::ops::mlir_agen_IndirectVectorLoadOp
}
fn t25() -> dataflowir_gen::ops::mlir_agen_IndirectVectorStoreOp {
    dataflowir_gen::ops::mlir_agen_IndirectVectorStoreOp
}
fn t26() -> dataflowir_gen::ops::mlir_agen_SetTransferMaskStateOp {
    dataflowir_gen::ops::mlir_agen_SetTransferMaskStateOp
}
fn t27() -> dataflowir_gen::ops::mlir_agen_SymbolicVectorLoadOp {
    dataflowir_gen::ops::mlir_agen_SymbolicVectorLoadOp
}
fn t28() -> dataflowir_gen::ops::mlir_agen_SymbolicVectorStoreOp {
    dataflowir_gen::ops::mlir_agen_SymbolicVectorStoreOp
}
fn t29() -> dataflowir_gen::ops::mlir_agen_VectorLoadOp {
    dataflowir_gen::ops::mlir_agen_VectorLoadOp
}
fn t30() -> dataflowir_gen::ops::mlir_agen_VectorStoreOp {
    dataflowir_gen::ops::mlir_agen_VectorStoreOp
}
fn t31() -> dataflowir_gen::ops::mlir_agen_YieldOp {
    dataflowir_gen::ops::mlir_agen_YieldOp
}
fn t32() -> dataflowir_gen::ops::mlir_arith_AddFOp {
    dataflowir_gen::ops::mlir_arith_AddFOp
}
fn t33() -> dataflowir_gen::ops::mlir_arith_AddIOp {
    dataflowir_gen::ops::mlir_arith_AddIOp
}
fn t34() -> dataflowir_gen::ops::mlir_arith_AddUIExtendedOp {
    dataflowir_gen::ops::mlir_arith_AddUIExtendedOp
}
fn t35() -> dataflowir_gen::ops::mlir_arith_AndIOp {
    dataflowir_gen::ops::mlir_arith_AndIOp
}
fn t36() -> dataflowir_gen::ops::mlir_arith_BitcastOp {
    dataflowir_gen::ops::mlir_arith_BitcastOp
}
fn t37() -> dataflowir_gen::ops::mlir_arith_CeilDivSIOp {
    dataflowir_gen::ops::mlir_arith_CeilDivSIOp
}
fn t38() -> dataflowir_gen::ops::mlir_arith_CeilDivUIOp {
    dataflowir_gen::ops::mlir_arith_CeilDivUIOp
}
fn t39() -> dataflowir_gen::ops::mlir_arith_CmpFOp {
    dataflowir_gen::ops::mlir_arith_CmpFOp
}
fn t40() -> dataflowir_gen::ops::mlir_arith_CmpIOp {
    dataflowir_gen::ops::mlir_arith_CmpIOp
}
fn t41() -> dataflowir_gen::ops::mlir_arith_ConstantOp {
    dataflowir_gen::ops::mlir_arith_ConstantOp
}
fn t42() -> dataflowir_gen::ops::mlir_arith_DivFOp {
    dataflowir_gen::ops::mlir_arith_DivFOp
}
fn t43() -> dataflowir_gen::ops::mlir_arith_DivSIOp {
    dataflowir_gen::ops::mlir_arith_DivSIOp
}
fn t44() -> dataflowir_gen::ops::mlir_arith_DivUIOp {
    dataflowir_gen::ops::mlir_arith_DivUIOp
}
fn t45() -> dataflowir_gen::ops::mlir_arith_ExtFOp {
    dataflowir_gen::ops::mlir_arith_ExtFOp
}
fn t46() -> dataflowir_gen::ops::mlir_arith_ExtSIOp {
    dataflowir_gen::ops::mlir_arith_ExtSIOp
}
fn t47() -> dataflowir_gen::ops::mlir_arith_ExtUIOp {
    dataflowir_gen::ops::mlir_arith_ExtUIOp
}
fn t48() -> dataflowir_gen::ops::mlir_arith_FPToSIOp {
    dataflowir_gen::ops::mlir_arith_FPToSIOp
}
fn t49() -> dataflowir_gen::ops::mlir_arith_FPToUIOp {
    dataflowir_gen::ops::mlir_arith_FPToUIOp
}
fn t50() -> dataflowir_gen::ops::mlir_arith_FloorDivSIOp {
    dataflowir_gen::ops::mlir_arith_FloorDivSIOp
}
fn t51() -> dataflowir_gen::ops::mlir_arith_IndexCastOp {
    dataflowir_gen::ops::mlir_arith_IndexCastOp
}
fn t52() -> dataflowir_gen::ops::mlir_arith_IndexCastUIOp {
    dataflowir_gen::ops::mlir_arith_IndexCastUIOp
}
fn t53() -> dataflowir_gen::ops::mlir_arith_MaxNumFOp {
    dataflowir_gen::ops::mlir_arith_MaxNumFOp
}
fn t54() -> dataflowir_gen::ops::mlir_arith_MaxSIOp {
    dataflowir_gen::ops::mlir_arith_MaxSIOp
}
fn t55() -> dataflowir_gen::ops::mlir_arith_MaxUIOp {
    dataflowir_gen::ops::mlir_arith_MaxUIOp
}
fn t56() -> dataflowir_gen::ops::mlir_arith_MaximumFOp {
    dataflowir_gen::ops::mlir_arith_MaximumFOp
}
fn t57() -> dataflowir_gen::ops::mlir_arith_MinNumFOp {
    dataflowir_gen::ops::mlir_arith_MinNumFOp
}
fn t58() -> dataflowir_gen::ops::mlir_arith_MinSIOp {
    dataflowir_gen::ops::mlir_arith_MinSIOp
}
fn t59() -> dataflowir_gen::ops::mlir_arith_MinUIOp {
    dataflowir_gen::ops::mlir_arith_MinUIOp
}
fn t60() -> dataflowir_gen::ops::mlir_arith_MinimumFOp {
    dataflowir_gen::ops::mlir_arith_MinimumFOp
}
fn t61() -> dataflowir_gen::ops::mlir_arith_MulFOp {
    dataflowir_gen::ops::mlir_arith_MulFOp
}
fn t62() -> dataflowir_gen::ops::mlir_arith_MulIOp {
    dataflowir_gen::ops::mlir_arith_MulIOp
}
fn t63() -> dataflowir_gen::ops::mlir_arith_MulSIExtendedOp {
    dataflowir_gen::ops::mlir_arith_MulSIExtendedOp
}
fn t64() -> dataflowir_gen::ops::mlir_arith_MulUIExtendedOp {
    dataflowir_gen::ops::mlir_arith_MulUIExtendedOp
}
fn t65() -> dataflowir_gen::ops::mlir_arith_NegFOp {
    dataflowir_gen::ops::mlir_arith_NegFOp
}
fn t66() -> dataflowir_gen::ops::mlir_arith_OrIOp {
    dataflowir_gen::ops::mlir_arith_OrIOp
}
fn t67() -> dataflowir_gen::ops::mlir_arith_RemFOp {
    dataflowir_gen::ops::mlir_arith_RemFOp
}
fn t68() -> dataflowir_gen::ops::mlir_arith_RemSIOp {
    dataflowir_gen::ops::mlir_arith_RemSIOp
}
fn t69() -> dataflowir_gen::ops::mlir_arith_RemUIOp {
    dataflowir_gen::ops::mlir_arith_RemUIOp
}
fn t70() -> dataflowir_gen::ops::mlir_arith_SIToFPOp {
    dataflowir_gen::ops::mlir_arith_SIToFPOp
}
fn t71() -> dataflowir_gen::ops::mlir_arith_ScalingExtFOp {
    dataflowir_gen::ops::mlir_arith_ScalingExtFOp
}
fn t72() -> dataflowir_gen::ops::mlir_arith_ScalingTruncFOp {
    dataflowir_gen::ops::mlir_arith_ScalingTruncFOp
}
fn t73() -> dataflowir_gen::ops::mlir_arith_SelectOp {
    dataflowir_gen::ops::mlir_arith_SelectOp
}
fn t74() -> dataflowir_gen::ops::mlir_arith_ShLIOp {
    dataflowir_gen::ops::mlir_arith_ShLIOp
}
fn t75() -> dataflowir_gen::ops::mlir_arith_ShRSIOp {
    dataflowir_gen::ops::mlir_arith_ShRSIOp
}
fn t76() -> dataflowir_gen::ops::mlir_arith_ShRUIOp {
    dataflowir_gen::ops::mlir_arith_ShRUIOp
}
fn t77() -> dataflowir_gen::ops::mlir_arith_SubFOp {
    dataflowir_gen::ops::mlir_arith_SubFOp
}
fn t78() -> dataflowir_gen::ops::mlir_arith_SubIOp {
    dataflowir_gen::ops::mlir_arith_SubIOp
}
fn t79() -> dataflowir_gen::ops::mlir_arith_TruncFOp {
    dataflowir_gen::ops::mlir_arith_TruncFOp
}
fn t80() -> dataflowir_gen::ops::mlir_arith_TruncIOp {
    dataflowir_gen::ops::mlir_arith_TruncIOp
}
fn t81() -> dataflowir_gen::ops::mlir_arith_UIToFPOp {
    dataflowir_gen::ops::mlir_arith_UIToFPOp
}
fn t82() -> dataflowir_gen::ops::mlir_arith_XOrIOp {
    dataflowir_gen::ops::mlir_arith_XOrIOp
}
fn t83() -> dataflowir_gen::ops::mlir_dataflow_CreateGroupOp {
    dataflowir_gen::ops::mlir_dataflow_CreateGroupOp
}
fn t84() -> dataflowir_gen::ops::mlir_dataflow_CreateMulticastGroupOp {
    dataflowir_gen::ops::mlir_dataflow_CreateMulticastGroupOp
}
fn t85() -> dataflowir_gen::ops::mlir_dataflow_GetLocalUnitOp {
    dataflowir_gen::ops::mlir_dataflow_GetLocalUnitOp
}
fn t86() -> dataflowir_gen::ops::mlir_dataflow_GetLogicalMemoryViewOp {
    dataflowir_gen::ops::mlir_dataflow_GetLogicalMemoryViewOp
}
fn t87() -> dataflowir_gen::ops::mlir_dataflow_GetPagedLogicalMemoryViewOp {
    dataflowir_gen::ops::mlir_dataflow_GetPagedLogicalMemoryViewOp
}
fn t88() -> dataflowir_gen::ops::mlir_dataflow_GetUnitOp {
    dataflowir_gen::ops::mlir_dataflow_GetUnitOp
}
fn t89() -> dataflowir_gen::ops::mlir_dataflow_ImplicitSyncOnStreamingBufferOp {
    dataflowir_gen::ops::mlir_dataflow_ImplicitSyncOnStreamingBufferOp
}
fn t90() -> dataflowir_gen::ops::mlir_dataflow_OpaqueOp {
    dataflowir_gen::ops::mlir_dataflow_OpaqueOp
}
fn t91() -> dataflowir_gen::ops::mlir_dataflow_ProgramUnitOp {
    dataflowir_gen::ops::mlir_dataflow_ProgramUnitOp
}
fn t92() -> dataflowir_gen::ops::mlir_dataflow_ReceiveOp {
    dataflowir_gen::ops::mlir_dataflow_ReceiveOp
}
fn t93() -> dataflowir_gen::ops::mlir_dataflow_ReturnOp {
    dataflowir_gen::ops::mlir_dataflow_ReturnOp
}
fn t94() -> dataflowir_gen::ops::mlir_dataflow_SendOp {
    dataflowir_gen::ops::mlir_dataflow_SendOp
}
fn t95() -> dataflowir_gen::ops::mlir_dataflow_SyncRecvOp {
    dataflowir_gen::ops::mlir_dataflow_SyncRecvOp
}
fn t96() -> dataflowir_gen::ops::mlir_dataflow_SyncSendOp {
    dataflowir_gen::ops::mlir_dataflow_SyncSendOp
}
fn t97() -> dataflowir_gen::ops::mlir_func_CallIndirectOp {
    dataflowir_gen::ops::mlir_func_CallIndirectOp
}
fn t98() -> dataflowir_gen::ops::mlir_func_CallOp {
    dataflowir_gen::ops::mlir_func_CallOp
}
fn t99() -> dataflowir_gen::ops::mlir_func_ConstantOp {
    dataflowir_gen::ops::mlir_func_ConstantOp
}
fn t100() -> dataflowir_gen::ops::mlir_func_FuncOp {
    dataflowir_gen::ops::mlir_func_FuncOp
}
fn t101() -> dataflowir_gen::ops::mlir_func_ReturnOp {
    dataflowir_gen::ops::mlir_func_ReturnOp
}
fn t102() -> dataflowir_gen::ops::mlir_scf_ConditionOp {
    dataflowir_gen::ops::mlir_scf_ConditionOp
}
fn t103() -> dataflowir_gen::ops::mlir_scf_ExecuteRegionOp {
    dataflowir_gen::ops::mlir_scf_ExecuteRegionOp
}
fn t104() -> dataflowir_gen::ops::mlir_scf_ForOp {
    dataflowir_gen::ops::mlir_scf_ForOp
}
fn t105() -> dataflowir_gen::ops::mlir_scf_ForallOp {
    dataflowir_gen::ops::mlir_scf_ForallOp
}
fn t106() -> dataflowir_gen::ops::mlir_scf_IfOp {
    dataflowir_gen::ops::mlir_scf_IfOp
}
fn t107() -> dataflowir_gen::ops::mlir_scf_InParallelOp {
    dataflowir_gen::ops::mlir_scf_InParallelOp
}
fn t108() -> dataflowir_gen::ops::mlir_scf_IndexSwitchOp {
    dataflowir_gen::ops::mlir_scf_IndexSwitchOp
}
fn t109() -> dataflowir_gen::ops::mlir_scf_ParallelOp {
    dataflowir_gen::ops::mlir_scf_ParallelOp
}
fn t110() -> dataflowir_gen::ops::mlir_scf_ReduceOp {
    dataflowir_gen::ops::mlir_scf_ReduceOp
}
fn t111() -> dataflowir_gen::ops::mlir_scf_ReduceReturnOp {
    dataflowir_gen::ops::mlir_scf_ReduceReturnOp
}
fn t112() -> dataflowir_gen::ops::mlir_scf_WhileOp {
    dataflowir_gen::ops::mlir_scf_WhileOp
}
fn t113() -> dataflowir_gen::ops::mlir_scf_YieldOp {
    dataflowir_gen::ops::mlir_scf_YieldOp
}
fn t114() -> dataflowir_gen::ops::mlir_sentient_AddOp {
    dataflowir_gen::ops::mlir_sentient_AddOp
}
fn t115() -> dataflowir_gen::ops::mlir_sentient_BinaryOp {
    dataflowir_gen::ops::mlir_sentient_BinaryOp
}
fn t116() -> dataflowir_gen::ops::mlir_sentient_ConstantOp {
    dataflowir_gen::ops::mlir_sentient_ConstantOp
}
fn t117() -> dataflowir_gen::ops::mlir_sentient_CopyOp {
    dataflowir_gen::ops::mlir_sentient_CopyOp
}
fn t118() -> dataflowir_gen::ops::mlir_sentient_ForOp {
    dataflowir_gen::ops::mlir_sentient_ForOp
}
fn t119() -> dataflowir_gen::ops::mlir_sentient_IfOp {
    dataflowir_gen::ops::mlir_sentient_IfOp
}
fn t120() -> dataflowir_gen::ops::mlir_sentient_IncrMaskOp {
    dataflowir_gen::ops::mlir_sentient_IncrMaskOp
}
fn t121() -> dataflowir_gen::ops::mlir_sentient_LoadAndExtractScalarOp {
    dataflowir_gen::ops::mlir_sentient_LoadAndExtractScalarOp
}
fn t122() -> dataflowir_gen::ops::mlir_sentient_LoadAndSendOp {
    dataflowir_gen::ops::mlir_sentient_LoadAndSendOp
}
fn t123() -> dataflowir_gen::ops::mlir_sentient_LoadAndStoreOp {
    dataflowir_gen::ops::mlir_sentient_LoadAndStoreOp
}
fn t124() -> dataflowir_gen::ops::mlir_sentient_LoadComputeAndSendOp {
    dataflowir_gen::ops::mlir_sentient_LoadComputeAndSendOp
}
fn t125() -> dataflowir_gen::ops::mlir_sentient_LoadOp {
    dataflowir_gen::ops::mlir_sentient_LoadOp
}
fn t126() -> dataflowir_gen::ops::mlir_sentient_LogicalPortOp {
    dataflowir_gen::ops::mlir_sentient_LogicalPortOp
}
fn t127() -> dataflowir_gen::ops::mlir_sentient_MacOp {
    dataflowir_gen::ops::mlir_sentient_MacOp
}
fn t128() -> dataflowir_gen::ops::mlir_sentient_MulOp {
    dataflowir_gen::ops::mlir_sentient_MulOp
}
fn t129() -> dataflowir_gen::ops::mlir_sentient_NOPOp {
    dataflowir_gen::ops::mlir_sentient_NOPOp
}
fn t130() -> dataflowir_gen::ops::mlir_sentient_OpaqueOp {
    dataflowir_gen::ops::mlir_sentient_OpaqueOp
}
fn t131() -> dataflowir_gen::ops::mlir_sentient_ReceiveAndExtractScalarOp {
    dataflowir_gen::ops::mlir_sentient_ReceiveAndExtractScalarOp
}
fn t132() -> dataflowir_gen::ops::mlir_sentient_ReceiveAndStoreOp {
    dataflowir_gen::ops::mlir_sentient_ReceiveAndStoreOp
}
fn t133() -> dataflowir_gen::ops::mlir_sentient_SetActiveMaskValueOp {
    dataflowir_gen::ops::mlir_sentient_SetActiveMaskValueOp
}
fn t134() -> dataflowir_gen::ops::mlir_sentient_SetMaskOp {
    dataflowir_gen::ops::mlir_sentient_SetMaskOp
}
fn t135() -> dataflowir_gen::ops::mlir_sentient_SetSendDestinationOp {
    dataflowir_gen::ops::mlir_sentient_SetSendDestinationOp
}
fn t136() -> dataflowir_gen::ops::mlir_sentient_SplatOp {
    dataflowir_gen::ops::mlir_sentient_SplatOp
}
fn t137() -> dataflowir_gen::ops::mlir_sentient_SubOp {
    dataflowir_gen::ops::mlir_sentient_SubOp
}
fn t138() -> dataflowir_gen::ops::mlir_sentient_SyncOp {
    dataflowir_gen::ops::mlir_sentient_SyncOp
}
fn t139() -> dataflowir_gen::ops::mlir_sentient_TernaryOp {
    dataflowir_gen::ops::mlir_sentient_TernaryOp
}
fn t140() -> dataflowir_gen::ops::mlir_sentient_UnaryOp {
    dataflowir_gen::ops::mlir_sentient_UnaryOp
}
fn t141() -> dataflowir_gen::ops::mlir_sentient_VectorConstantOp {
    dataflowir_gen::ops::mlir_sentient_VectorConstantOp
}
fn t142() -> dataflowir_gen::ops::mlir_sentient_YieldOp {
    dataflowir_gen::ops::mlir_sentient_YieldOp
}
fn t143() -> dataflowir_gen::ops::mlir_symbol_CreateIdOp {
    dataflowir_gen::ops::mlir_symbol_CreateIdOp
}
fn t144() -> dataflowir_gen::ops::mlir_symbol_CreateSymbolOp {
    dataflowir_gen::ops::mlir_symbol_CreateSymbolOp
}
fn t145() -> dataflowir_gen::ops::mlir_symbol_SymbolImmutableMappingOp {
    dataflowir_gen::ops::mlir_symbol_SymbolImmutableMappingOp
}
fn t146() -> dataflowir_gen::ops::mlir_symbol_SymbolQueryMapOp {
    dataflowir_gen::ops::mlir_symbol_SymbolQueryMapOp
}
fn t147() -> dataflowir_gen::ops::mlir_trace_ProgramUnitTraceOp {
    dataflowir_gen::ops::mlir_trace_ProgramUnitTraceOp
}
fn t148() -> dataflowir_gen::ops::mlir_trace_ReturnOp {
    dataflowir_gen::ops::mlir_trace_ReturnOp
}
fn t149() -> dataflowir_gen::ops::mlir_uniform_DefImmutableMappingOp {
    dataflowir_gen::ops::mlir_uniform_DefImmutableMappingOp
}
fn t150() -> dataflowir_gen::ops::mlir_uniform_EqualizePatternOp {
    dataflowir_gen::ops::mlir_uniform_EqualizePatternOp
}
fn t151() -> dataflowir_gen::ops::mlir_uniform_QueryMapOp {
    dataflowir_gen::ops::mlir_uniform_QueryMapOp
}
fn t152() -> dataflowir_gen::ops::mlir_uniform_UniformizeRegionsOp {
    dataflowir_gen::ops::mlir_uniform_UniformizeRegionsOp
}
fn t153() -> dataflowir_gen::ops::mlir_uniform_YieldOp {
    dataflowir_gen::ops::mlir_uniform_YieldOp
}
fn t154() -> dataflowir_gen::ops::mlir_vectorchain_BinaryOp {
    dataflowir_gen::ops::mlir_vectorchain_BinaryOp
}
fn t155() -> dataflowir_gen::ops::mlir_vectorchain_CastOp {
    dataflowir_gen::ops::mlir_vectorchain_CastOp
}
fn t156() -> dataflowir_gen::ops::mlir_vectorchain_ConstantBitstreamOp {
    dataflowir_gen::ops::mlir_vectorchain_ConstantBitstreamOp
}
fn t157() -> dataflowir_gen::ops::mlir_vectorchain_CreateAffineMaskOp {
    dataflowir_gen::ops::mlir_vectorchain_CreateAffineMaskOp
}
fn t158() -> dataflowir_gen::ops::mlir_vectorchain_ElementWiseCompareOp {
    dataflowir_gen::ops::mlir_vectorchain_ElementWiseCompareOp
}
fn t159() -> dataflowir_gen::ops::mlir_vectorchain_ElementWiseSelectionOp {
    dataflowir_gen::ops::mlir_vectorchain_ElementWiseSelectionOp
}
fn t160() -> dataflowir_gen::ops::mlir_vectorchain_ExpEstimateOp {
    dataflowir_gen::ops::mlir_vectorchain_ExpEstimateOp
}
fn t161() -> dataflowir_gen::ops::mlir_vectorchain_FastExpOp {
    dataflowir_gen::ops::mlir_vectorchain_FastExpOp
}
fn t162() -> dataflowir_gen::ops::mlir_vectorchain_FloorOp {
    dataflowir_gen::ops::mlir_vectorchain_FloorOp
}
fn t163() -> dataflowir_gen::ops::mlir_vectorchain_LnEstimateOp {
    dataflowir_gen::ops::mlir_vectorchain_LnEstimateOp
}
fn t164() -> dataflowir_gen::ops::mlir_vectorchain_MergeOp {
    dataflowir_gen::ops::mlir_vectorchain_MergeOp
}
fn t165() -> dataflowir_gen::ops::mlir_vectorchain_MultiplyAndAccumulateOp {
    dataflowir_gen::ops::mlir_vectorchain_MultiplyAndAccumulateOp
}
fn t166() -> dataflowir_gen::ops::mlir_vectorchain_MultiplyOp {
    dataflowir_gen::ops::mlir_vectorchain_MultiplyOp
}
fn t167() -> dataflowir_gen::ops::mlir_vectorchain_NegOp {
    dataflowir_gen::ops::mlir_vectorchain_NegOp
}
fn t168() -> dataflowir_gen::ops::mlir_vectorchain_PackOp {
    dataflowir_gen::ops::mlir_vectorchain_PackOp
}
fn t169() -> dataflowir_gen::ops::mlir_vectorchain_RecEstimateOp {
    dataflowir_gen::ops::mlir_vectorchain_RecEstimateOp
}
fn t170() -> dataflowir_gen::ops::mlir_vectorchain_RotateOp {
    dataflowir_gen::ops::mlir_vectorchain_RotateOp
}
fn t171() -> dataflowir_gen::ops::mlir_vectorchain_RsqrtEstimateOp {
    dataflowir_gen::ops::mlir_vectorchain_RsqrtEstimateOp
}
fn t172() -> dataflowir_gen::ops::mlir_vectorchain_ScanWithGapOp {
    dataflowir_gen::ops::mlir_vectorchain_ScanWithGapOp
}
fn t173() -> dataflowir_gen::ops::mlir_vectorchain_SelectOp {
    dataflowir_gen::ops::mlir_vectorchain_SelectOp
}
fn t174() -> dataflowir_gen::ops::mlir_vectorchain_ShuffleOp {
    dataflowir_gen::ops::mlir_vectorchain_ShuffleOp
}
fn t175() -> dataflowir_gen::ops::mlir_vectorchain_SigmoidEstimateOp {
    dataflowir_gen::ops::mlir_vectorchain_SigmoidEstimateOp
}
fn t176() -> dataflowir_gen::ops::mlir_vectorchain_TanhEstimateOp {
    dataflowir_gen::ops::mlir_vectorchain_TanhEstimateOp
}
