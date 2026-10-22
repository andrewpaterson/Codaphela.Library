#ifndef __INSTRUCTION_CYCLE_BUILDER_H__
#define __INSTRUCTION_CYCLE_BUILDER_H__
#include "AddressingMode.h"
#include "WidthFromRegister.h"
#include "W65CFunc.h"

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
#include "DoneInstructionIfEmulation.h"
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
#include "ReadOpCode.h"
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
#include "NoteDecimal.h"
#include "DoneInstructionIfBinary.h"
#include "DoneInstructionIfSamePage.h"
#include "BranchAddress.h"
#include "CompoundAddress.h"


//Helpers to describe the bus cycles of an instruction.  Shared by the W65C02S and W65C816S
//instruction cycle factories.
extern NotMemoryLock RMW;

CBusCycle* BusCycle(CAddressOffsetArray* papcAddressOffsets, COperationArray* papcOperations);
CInstructionCycles* InstructionCycles(EAddressingMode eAddressingMode, CBusCycle* pcBusCycle, ...);
COperationArray* Operation(COperation* pcOperation, ...);
CProgramCounter*	PC(void);
CStackPointer*	S(void);
CProgramBank*	PBR(void);
CAbsoluteAddress*	AA(void);
CXIndex*	X(void);
CYIndex*	Y(void);
CDataBank*	DBR(void);
CAbsoluteAddressHigh*	AAH(void);
CDirectOffset*	DirectOffset(void);
CDirectPage*	DP(void);
CNewProgramCounter*	New_PC(void);
CNewProgramBank*	New_PBR(void);
CAddressBank*	AAB(void);
CDirectOffset*	D0(void);
CAbsoluteAddressLowPlusXLow*	AAL_XL(void);
CAbsoluteAddressLowPlusYLow*	AAL_YL(void);
CSignedData*	SignedData(void);
CSignedDataLow*	SignedDataLow(void);
CInternalFirst*	Opcode(void);
CIncrementProgramCounter*	PC_inc(void);
CDecrementStackPointer*	SP_dec(void);
CIncrementStackPointer*	SP_inc(void);
CWriteProgramBank*	Write_PBR(void);
CWriteProgramCounterLow*	Write_PCL(void);
CWriteProgramCounterHigh*	Write_PCH(void);
CWriteProcessorStatus*	Write_PS(void);
CReadProcessorStatus*	Read_PS(void);
CDoneInstruction*	DONE(void);
CDoneInstructionIfEmulation*	DONE_Emulation(void);
CWriteAbsoluteAddressLow*	Write_AAL(void);
CWriteAbsoluteAddressHigh*	Write_AAH(void);
CNoteTwo*	NoteTwo(void);
CNoteThree*	NoteThree(void);
CNoteSix*	NoteSix(void);
CNoteSeven*	NoteSeven(void);
CWaitOperation*	WaitOperation(void);
CRestoreAbortValues*	RestoreAbortValues(void);
CDirectPage* DPL(void);
CStackPointer* StackPointer(EStackPointerMode eMode);
CStackPointer* S1(void);
CStackPointer* S16(void);
CConstantOffset* oM(int32 iOffset);
CConstantOffset* o(int32 iOffset);
CWriteProcessorStatus* Write_PSB(void);
CConstantOffset* Constant(uint16 uiAddress);
CBranchAddress* BranchOrigin(void);
CBranchAddress* BranchPageCross(void);
CNoteDecimal*	NoteDecimal(void);
CDoneInstructionIfBinary*	DONE_Binary(void);
CDoneInstructionIfSamePage*	DONE_SamePage(void);
CWriteDataHigh* Write_DataHigh(NotMemoryLock bNotMemoryLock);
CWriteDataHigh* Write_DataHigh(void);
CInterruptAddress* VA(CInterruptVector* pcInterruptVector);
CInternalOperation* IO(void);
CInternalOperation* IO(bool bValidProgramAddress, bool bValidDataAddress, NotMemoryLock bNotMemoryLock);
CReadAbsoluteAddressHigh* Read_AAH(NotMemoryLock bNotMemoryLock);
CReadAbsoluteAddressHigh* Read_AAH(void);
CReadAbsoluteAddressLow* Read_AAL(void);
CReadAbsoluteAddressLow* Read_AAVL(void);
CReadAbsoluteAddressHigh* Read_AAVH(void);
CReadDirectOffset* Read_D0(void);
CReadProgramBank* Read_NewPBR(void);
CReadProgramCounterHigh* Read_NewPCH(void);
CReadProgramCounterLow* Read_NewPCL(void);
CSetProgramCounter* PC_e(CAddressOffsetArray* papcAddressOffsets);
CReadAbsoluteAddressBank* Read_AAB(void);
CReadDataBank* Read_DBR(void);
CReadAbsoluteAddressLow* Read_AAL(NotMemoryLock bNotMemoryLock);
CInternalOperation* IO(NotMemoryLock bNotMemoryLock);
CReadDataHigh* Read_DataHigh(NotMemoryLock bNotMemoryLock);
CReadDataHigh* Read_DataHigh(void);
CReadDataLow* Read_DataLow(NotMemoryLock bNotMemoryLock);
CReadDataLow* Read_DataLow(void);
CWriteDataLow* Write_DataLow(NotMemoryLock bNotMemoryLock);
CWriteDataLow* Write_DataLow();
CReadDirectOffset* Read_D0(NotMemoryLock bNotMemoryLock);
CSetProgramBank* PBR_e(uint8 uiBank);
CExecuteIf16Bit* E16Bit(CW65CFunc fConsumer, EWidthFromRegister eWidth, bool bInitialSide);
CExecuteIf8Bit* E8Bit(CW65CFunc fConsumer, EWidthFromRegister eWidth, bool bInitialSide);
CExecute* E(CW65CFunc fConsumer, bool bInitialSide);
CDoneInstructionIf8Bit* DONE8Bit(EWidthFromRegister eWidth);
CDoneInstructionIf16Bit* DONE16Bit(EWidthFromRegister eWidth);
CNoteOne* NoteOne(EWidthFromRegister eWidth);
CNoteFourX* NoteFourX(bool bNextWillRead);
CNoteFourY* NoteFourY(bool bNextWillRead);
CInstructionCycles* CreateFetchOpcodeCycles(void);
CInstructionCycles* CreateStackHardwareInterruptCycles(CInterruptVector* pcInterruptVector, CW65CFunc fOperation, bool bInitialSide);
CInstructionCycles* CreateStackResetCycles(CInterruptVector* pcInterruptVector, CW65CFunc fOperation, bool bInitialSide);


#endif // __INSTRUCTION_CYCLE_BUILDER_H__

