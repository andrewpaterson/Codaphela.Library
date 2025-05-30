#ifndef __INSTRUCTION_CYCLE_FACTORY_H__
#define __INSTRUCTION_CYCLE_FACTORY_H__
#include "AddressingMode.h"
#include "WidthFromRegister.h"
#include "W65C816Func.h"

//Addressing
#include "AbsoluteAddress.h"
#include "AbsoluteAddressHigh.h"
#include "AbsoluteAddressLowPlusXLow.h"
#include "AbsoluteAddressLowPlusYLow.h"
#include "AddressBank.h"
#include "AddressOffset.h"
#include "ConstantOffset.h"
#include "DataBank.h"
#include "DirectOffset.h"
#include "DirectPage.h"
#include "InterruptAddress.h"
#include "NewProgramBank.h"
#include "NewProgramCounter.h"
#include "ProgramBank.h"
#include "ProgramCounter.h"
#include "SignedData.h"
#include "SignedDataLow.h"
#include "StackPointer.h"
#include "XIndex.h"
#include "YIndex.h"

//Operations
#include "DecrementStackPointer.h"
#include "DoneInstruction.h"
#include "DoneInstructionIf8Bit.h"
#include "DoneInstructionIf16Bit.h"
#include "Execute.h"
#include "ExecuteIf8Bit.h"
#include "ExecuteIf16Bit.h"
#include "IncrementProgramCounter.h"
#include "IncrementStackPointer.h"
#include "InternalFirst.h"
#include "InternalOperation.h"
#include "ReadAbsoluteAddressLow.h"
#include "ReadAbsoluteAddressHigh.h"
#include "ReadAbsoluteAddressBank.h"
#include "ReadDataLow.h"
#include "ReadDataHigh.h"
#include "ReadDataBank.h"
#include "ReadDirectOffset.h"
#include "ReadOpcode.h"
#include "ReadProcessorStatus.h"
#include "ReadProgramBank.h"
#include "ReadProgramCounterHigh.h"
#include "ReadProgramCounterLow.h"
#include "RestoreAbortValues.h"
#include "SetProgramBank.h"
#include "SetProgramCounter.h"
#include "WaitOperation.h"
#include "WriteAbsoluteAddressHigh.h"
#include "WriteAbsoluteAddressLow.h"
#include "WriteDataHigh.h"
#include "WriteDataLow.h"
#include "WriteProcessorStatus.h"
#include "WriteProgramBank.h"
#include "WriteProgramCounterHigh.h"
#include "WriteProgramCounterLow.h"

//Notes
#include "NoteOne.h"
#include "NoteTwo.h"
#include "NoteThree.h"
#include "NoteFourX.h"
#include "NoteFourY.h"
#include "NoteSix.h"
#include "NoteSeven.h"

#include "InstructionCycles.h"


CInstructionCycles* CreateFetchOpcodeCycles(void);
CInstructionCycles* CreateAbsoluteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CInstructionCycles* CreateAbsoluteWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CInstructionCycles* CreateAbsoluteJMPCycles(void);
CInstructionCycles* CreateAbsoluteJSRCycles(void);
CInstructionCycles* CreateAbsoluteRMWCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateAbsoluteIndexedIndirectWithXJMPCycles(void);
CInstructionCycles* CreateAbsoluteIndexedIndirectWithXJSRCycles(void);
CInstructionCycles* CreateAbsoluteIndirectJMLCycles(void);
CInstructionCycles* CreateAbsoluteIndirectJMPCycles(void);
CInstructionCycles* CreateAbsoluteLongWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateAbsoluteLongCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateAbsoluteLongJMLCycles(void);
CInstructionCycles* CreateAbsoluteLongJSLCycles(void);
CInstructionCycles* CreateAbsoluteLongIndexedWithXCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateAbsoluteLongIndexedWithXWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateAbsoluteIndexedWithXWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateAbsoluteIndexedWithXCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CInstructionCycles* CreateAbsoluteIndexedWithXRMWCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateAbsoluteIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateAbsoluteIndexedWithYCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CInstructionCycles* CreateAccumulatorCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateBlockMoveCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateDirectCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CInstructionCycles* CreateDirectWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CInstructionCycles* CreateDirectRMWCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateDirectIndexedIndirectWithXCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateDirectIndexedIndirectWithXWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateDirectIndirectCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateDirectIndirectWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateDirectIndirectIndexedWithYCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateDirectIndirectIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateDirectIndirectLongIndexedWithYCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateDirectIndirectLongIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateDirectIndirectLongCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateDirectIndirectLongWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateDirectIndexedWithXCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CInstructionCycles* CreateDirectIndexedWithXWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CInstructionCycles* CreateDirectIndexedWithXRMWCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateDirectIndexedWithYCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CInstructionCycles* CreateDirectIndexedWithYWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CInstructionCycles* CreateImmediateREPSEPCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateImmediateCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CInstructionCycles* CreateImpliedCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateWDMImpliedCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateImpliedXBACycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStopTheClockCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateWaitForInterruptCycles(void);
CInstructionCycles* CreateRelativeShortCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateRelativeLongCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackHardwareInterruptCycles(CInterruptVector* pcInterruptVector, CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackAbortInterruptCycles(CInterruptVector* pcInterruptVector, CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackResetCycles(CInterruptVector* pcInterruptVector, CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackPullCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CInstructionCycles* CreateStackPLBCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackPLDCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackPLPCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackImpliedPHPCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackPushCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide);
CInstructionCycles* CreateStackPHBCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackPHDCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackPHKCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackPEACycles(void);
CInstructionCycles* CreateStackPEICycles(void);
CInstructionCycles* CreateStackPERCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackRTICycles(void);
CInstructionCycles* CreateStackRTSCycles(void);
CInstructionCycles* CreateStackRTLCycles(void);
CInstructionCycles* CreateStackSoftwareInterruptCycles(CInterruptVector* pcInterruptVector, CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackRelativeCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackRelativeWriteCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackRelativeIndirectIndexedWithYCycles(CW65C816Func fOperation, bool bInitialSide);
CInstructionCycles* CreateStackRelativeIndirectIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide);


#endif // __INSTRUCTION_CYCLE_FACTORY_H__

