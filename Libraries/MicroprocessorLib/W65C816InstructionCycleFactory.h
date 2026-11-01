#ifndef __INSTRUCTION_CYCLE_FACTORY_H__
#define __INSTRUCTION_CYCLE_FACTORY_H__
#include "W65C816Func.h"
#include "W65InstructionCycleBuilder.h"


CW65InstructionCycles* CreateAbsoluteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CW65InstructionCycles* CreateAbsoluteWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CW65InstructionCycles* CreateAbsoluteJMPCycles(void);
CW65InstructionCycles* CreateAbsoluteJSRCycles(void);
CW65InstructionCycles* CreateAbsoluteRMWCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateAbsoluteIndexedIndirectWithXJMPCycles(void);
CW65InstructionCycles* CreateAbsoluteIndexedIndirectWithXJSRCycles(void);
CW65InstructionCycles* CreateAbsoluteIndirectJMLCycles(void);
CW65InstructionCycles* CreateAbsoluteIndirectJMPCycles(void);
CW65InstructionCycles* CreateAbsoluteLongWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateAbsoluteLongCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateAbsoluteLongJMLCycles(void);
CW65InstructionCycles* CreateAbsoluteLongJSLCycles(void);
CW65InstructionCycles* CreateAbsoluteLongIndexedWithXCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateAbsoluteLongIndexedWithXWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateAbsoluteIndexedWithXWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateAbsoluteIndexedWithXCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CW65InstructionCycles* CreateAbsoluteIndexedWithXRMWCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateAbsoluteIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateAbsoluteIndexedWithYCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CW65InstructionCycles* CreateAccumulatorCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateBlockMoveCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateDirectCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CW65InstructionCycles* CreateDirectWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CW65InstructionCycles* CreateDirectRMWCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateDirectIndexedIndirectWithXCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateDirectIndexedIndirectWithXWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateDirectIndirectCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateDirectIndirectWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateDirectIndirectIndexedWithYCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateDirectIndirectIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateDirectIndirectLongIndexedWithYCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateDirectIndirectLongIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateDirectIndirectLongCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateDirectIndirectLongWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateDirectIndexedWithXCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CW65InstructionCycles* CreateDirectIndexedWithXWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CW65InstructionCycles* CreateDirectIndexedWithXRMWCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateDirectIndexedWithYCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CW65InstructionCycles* CreateDirectIndexedWithYWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CW65InstructionCycles* CreateImmediateREPSEPCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateImmediateCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CW65InstructionCycles* CreateImpliedCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateWDMImpliedCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateImpliedXBACycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStopTheClockCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateWaitForInterruptCycles(void);
CW65InstructionCycles* CreateRelativeShortCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateRelativeLongCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStackAbortInterruptCycles(CW65InterruptVector* pcInterruptVector, CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStackPullCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CW65InstructionCycles* CreateStackPLBCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStackPLDCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStackPLPCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStackImpliedPHPCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStackPushCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CW65InstructionCycles* CreateStackPHBCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStackPHDCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStackPHKCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStackPEACycles(void);
CW65InstructionCycles* CreateStackPEICycles(void);
CW65InstructionCycles* CreateStackPERCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStackRTICycles(void);
CW65InstructionCycles* CreateStackRTSCycles(void);
CW65InstructionCycles* CreateStackRTLCycles(void);
CW65InstructionCycles* CreateStackSoftwareInterruptCycles(CW65InterruptVector* pcInterruptVector, CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStackRelativeCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStackRelativeWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStackRelativeIndirectIndexedWithYCycles(CW65C816Func fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStackRelativeIndirectIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide);


#endif // __INSTRUCTION_CYCLE_FACTORY_H__

