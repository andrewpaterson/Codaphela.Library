#ifndef __INSTRUCTION_CYCLE_BUILDER_H__
#define __INSTRUCTION_CYCLE_BUILDER_H__
#include "AddressingMode.h"
#include "W65WidthFromRegister.h"
#include "W65CFunc.h"

//Addressing
#include "W65AbsoluteAddress.h"
#include "W65AbsoluteAddressHigh.h"
#include "W65AbsoluteAddressLowPlusXLow.h"
#include "W65AbsoluteAddressLowPlusYLow.h"
#include "W65AddressBank.h"
#include "W65AddressOffset.h"
#include "W65ConstantOffset.h"
#include "W65DataBank.h"
#include "W65DirectOffset.h"
#include "W65DirectPage.h"
#include "W65InterruptAddress.h"
#include "W65NewProgramBank.h"
#include "W65NewProgramCounter.h"
#include "W65ProgramBank.h"
#include "W65ProgramCounter.h"
#include "W65SignedData.h"
#include "W65SignedDataLow.h"
#include "W65StackPointer.h"
#include "W65XIndex.h"
#include "W65YIndex.h"

//Operations
#include "W65DecrementStackPointer.h"
#include "W65DoneInstruction.h"
#include "W65DoneInstructionIf8Bit.h"
#include "W65DoneInstructionIfEmulation.h"
#include "W65DoneInstructionIf16Bit.h"
#include "W65Execute.h"
#include "W65ExecuteIf8Bit.h"
#include "W65ExecuteIf16Bit.h"
#include "W65IncrementProgramCounter.h"
#include "W65IncrementStackPointer.h"
#include "W65InternalFirst.h"
#include "W65InternalOperation.h"
#include "W65ReadAbsoluteAddressLow.h"
#include "W65ReadAbsoluteAddressHigh.h"
#include "W65ReadAbsoluteAddressBank.h"
#include "W65ReadDataLow.h"
#include "W65ReadDataHigh.h"
#include "W65ReadDataBank.h"
#include "W65ReadDirectOffset.h"
#include "W65ReadOpcode.h"
#include "W65ReadProcessorStatus.h"
#include "W65ReadProgramBank.h"
#include "W65ReadProgramCounterHigh.h"
#include "W65ReadProgramCounterLow.h"
#include "W65RestoreAbortValues.h"
#include "W65SetProgramBank.h"
#include "W65SetProgramCounter.h"
#include "W65WaitOperation.h"
#include "W65WriteAbsoluteAddressHigh.h"
#include "W65WriteAbsoluteAddressLow.h"
#include "W65WriteDataHigh.h"
#include "W65WriteDataLow.h"
#include "W65WriteProcessorStatus.h"
#include "W65WriteProgramBank.h"
#include "W65WriteProgramCounterHigh.h"
#include "W65WriteProgramCounterLow.h"

//Notes
#include "W65NoteOne.h"
#include "W65NoteTwo.h"
#include "W65NoteThree.h"
#include "W65NoteFourX.h"
#include "W65NoteFourY.h"
#include "W65NoteSix.h"
#include "W65NoteSeven.h"

#include "W65InstructionCycles.h"
#include "W65NoteDecimal.h"
#include "W65DoneInstructionIfBinary.h"
#include "W65DoneInstructionIfSamePage.h"
#include "W65BranchAddress.h"
#include "W65CompoundAddress.h"


//Helpers to describe the bus cycles of an instruction.  Shared by the W65C02S and W65C816S
//instruction cycle factories.
extern NotMemoryLock RMW;

CW65BusCycle* BusCycle(CW65AddressOffsetArray* papcAddressOffsets, CW65OperationArray* papcOperations);
CW65InstructionCycles* InstructionCycles(EAddressingMode eAddressingMode, CW65BusCycle* pcBusCycle, ...);
CW65OperationArray* Operation(CW65Operation* pcOperation, ...);
CW65ProgramCounter*	PC(void);
CW65StackPointer*	S(void);
CW65ProgramBank*	PBR(void);
CW65AbsoluteAddress*	AA(void);
CW65XIndex*	X(void);
CW65YIndex*	Y(void);
CW65DataBank*	DBR(void);
CW65AbsoluteAddressHigh*	AAH(void);
CW65DirectOffset*	DirectOffset(void);
CW65DirectPage*	DP(void);
CW65NewProgramCounter*	New_PC(void);
CW65NewProgramBank*	New_PBR(void);
CW65AddressBank*	AAB(void);
CW65DirectOffset*	D0(void);
CW65AbsoluteAddressLowPlusXLow*	AAL_XL(void);
CW65AbsoluteAddressLowPlusYLow*	AAL_YL(void);
CW65SignedData*	SignedData(void);
CW65SignedDataLow*	SignedDataLow(void);
CW65InternalFirst*	Opcode(void);
CW65IncrementProgramCounter*	PC_inc(void);
CW65DecrementStackPointer*	SP_dec(void);
CW65IncrementStackPointer*	SP_inc(void);
CW65WriteProgramBank*	Write_PBR(void);
CW65WriteProgramCounterLow*	Write_PCL(void);
CW65WriteProgramCounterHigh*	Write_PCH(void);
CW65WriteProcessorStatus*	Write_PS(void);
CW65ReadProcessorStatus*	Read_PS(void);
CW65DoneInstruction*	DONE(void);
CW65DoneInstructionIfEmulation*	DONE_Emulation(void);
CW65WriteAbsoluteAddressLow*	Write_AAL(void);
CW65WriteAbsoluteAddressHigh*	Write_AAH(void);
CW65NoteTwo*	NoteTwo(void);
CW65NoteThree*	NoteThree(void);
CW65NoteSix*	NoteSix(void);
CW65NoteSeven*	NoteSeven(void);
CW65WaitOperation*	WaitOperation(void);
CW65RestoreAbortValues*	RestoreAbortValues(void);
CW65DirectPage* DPL(void);
CW65StackPointer* StackPointer(EStackPointerMode eMode);
CW65StackPointer* S1(void);
CW65StackPointer* S16(void);
CW65ConstantOffset* oM(int32 iOffset);
CW65ConstantOffset* o(int32 iOffset);
CW65WriteProcessorStatus* Write_PSB(void);
CW65ConstantOffset* Constant(uint16 uiAddress);
CW65BranchAddress* BranchOrigin(void);
CW65BranchAddress* BranchPageCross(void);
CW65NoteDecimal*	NoteDecimal(void);
CW65DoneInstructionIfBinary*	DONE_Binary(void);
CW65DoneInstructionIfSamePage*	DONE_SamePage(void);
CW65WriteDataHigh* Write_DataHigh(NotMemoryLock bNotMemoryLock);
CW65WriteDataHigh* Write_DataHigh(void);
CW65InterruptAddress* VA(CW65InterruptVector* pcInterruptVector);
CW65InternalOperation* IO(void);
CW65InternalOperation* IO(bool bValidProgramAddress, bool bValidDataAddress, NotMemoryLock bNotMemoryLock);
CW65ReadAbsoluteAddressHigh* Read_AAH(NotMemoryLock bNotMemoryLock);
CW65ReadAbsoluteAddressHigh* Read_AAH(void);
CW65ReadAbsoluteAddressLow* Read_AAL(void);
CW65ReadAbsoluteAddressLow* Read_AAVL(void);
CW65ReadAbsoluteAddressHigh* Read_AAVH(void);
CW65ReadDirectOffset* Read_D0(void);
CW65ReadProgramBank* Read_NewPBR(void);
CW65ReadProgramCounterHigh* Read_NewPCH(void);
CW65ReadProgramCounterLow* Read_NewPCL(void);
CW65SetProgramCounter* PC_e(CW65AddressOffsetArray* papcAddressOffsets);
CW65ReadAbsoluteAddressBank* Read_AAB(void);
CW65ReadDataBank* Read_DBR(void);
CW65ReadAbsoluteAddressLow* Read_AAL(NotMemoryLock bNotMemoryLock);
CW65InternalOperation* IO(NotMemoryLock bNotMemoryLock);
CW65ReadDataHigh* Read_DataHigh(NotMemoryLock bNotMemoryLock);
CW65ReadDataHigh* Read_DataHigh(void);
CW65ReadDataLow* Read_DataLow(NotMemoryLock bNotMemoryLock);
CW65ReadDataLow* Read_DataLow(void);
CW65WriteDataLow* Write_DataLow(NotMemoryLock bNotMemoryLock);
CW65WriteDataLow* Write_DataLow();
CW65ReadDirectOffset* Read_D0(NotMemoryLock bNotMemoryLock);
CW65SetProgramBank* PBR_e(uint8 uiBank);
CW65ExecuteIf16Bit* E16Bit(CW65CFunc fConsumer, EWidthFromRegister eWidth, bool bInitialSide);
CW65ExecuteIf8Bit* E8Bit(CW65CFunc fConsumer, EWidthFromRegister eWidth, bool bInitialSide);
CW65Execute* E(CW65CFunc fConsumer, bool bInitialSide);
CW65DoneInstructionIf8Bit* DONE8Bit(EWidthFromRegister eWidth);
CW65DoneInstructionIf16Bit* DONE16Bit(EWidthFromRegister eWidth);
CW65NoteOne* NoteOne(EWidthFromRegister eWidth);
CW65NoteFourX* NoteFourX(bool bNextWillRead);
CW65NoteFourY* NoteFourY(bool bNextWillRead);
CW65InstructionCycles* CreateFetchOpcodeCycles(void);
CW65InstructionCycles* CreateStackHardwareInterruptCycles(CW65InterruptVector* pcInterruptVector, CW65CFunc fOperation, bool bInitialSide);
CW65InstructionCycles* CreateStackResetCycles(CW65InterruptVector* pcInterruptVector, CW65CFunc fOperation, bool bInitialSide);


#endif // __INSTRUCTION_CYCLE_BUILDER_H__

