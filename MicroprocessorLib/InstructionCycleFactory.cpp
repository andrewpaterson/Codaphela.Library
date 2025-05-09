#include "InstructionCycleFactory.h"


NotMemoryLock RMW = false;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBusCycle* BusCycle(CAddressOffsetArray* papcAddressOffsets, COperationArray* papcOperations)
{
    CBusCycle* pcBusCycle;

    pcBusCycle = NewMalloc<CBusCycle>();
    pcBusCycle->Init(papcAddressOffsets, papcOperations);
    return pcBusCycle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstructionCycles* InstructionCycles(EAddressingMode eAddressingMode, CBusCycle* pcBusCycle, ...)
{
    va_list					vaMarker;
    CInstructionCycles*		pcInstructionCycles;
	CBusCycleArray			apcBusCycles;

    if (pcBusCycle)
    {
        va_start(vaMarker, pcBusCycle);
		pcBusCycle = va_arg(vaMarker, CBusCycle*);
        while (pcBusCycle)
        {
			pcBusCycle = va_arg(vaMarker, CBusCycle*);
			apcBusCycles.Add(pcBusCycle);
        }
        va_end(vaMarker);
    }

	pcInstructionCycles = NewMalloc<CInstructionCycles>();
	pcInstructionCycles->Init(eAddressingMode, &apcBusCycles);

    return pcInstructionCycles;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAddressOffsetArray* Address(CAddressOffset* pcOffset, ...)
{
	va_list		            vaMarker;
	size		            iCount;
	CAddressOffsetArray* papcOffsets;

	papcOffsets = NewMalloc<CAddressOffsetArray>();
	papcOffsets->Init();

	if (pcOffset)
	{
		iCount = 0;

		va_start(vaMarker, pcOffset);
		while (pcOffset)
		{
			papcOffsets->Add(pcOffset);
			iCount++;
			pcOffset = va_arg(vaMarker, CAddressOffset*);
		}
		va_end(vaMarker);
	}

	return papcOffsets;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
COperationArray* Operation(COperation* pcOperation, ...)
{
	va_list		        vaMarker;
	size		        iCount;
	COperationArray*	papcOperations;

	papcOperations = NewMalloc<COperationArray>();
	papcOperations->Init();

	if (pcOperation)
	{
		iCount = 0;

		va_start(vaMarker, pcOperation);
		while (pcOperation)
		{
			papcOperations->Add(pcOperation);
			iCount++;
			pcOperation = va_arg(vaMarker, COperation*);
		}
		va_end(vaMarker);
	}

	return papcOperations;
}


#define ADDRESS(type, name) type* name(void) \
{ \
	type*	pcType; \
	pcType = NewMalloc<type>(); \
	pcType->Init(); \
	return pcType; \
}


#define OPERATION(type, name) type* name(void) \
{ \
	type*	pcType; \
	pcType = NewMalloc<type>(); \
	pcType->Init(); \
	return pcType; \
}


ADDRESS(CProgramCounter,				PC)
ADDRESS(CStackPointer,					S)
ADDRESS(CProgramBank,					PBR)
ADDRESS(CAbsoluteAddress,				AA)
ADDRESS(CXIndex,						X)
ADDRESS(CYIndex,						Y)
ADDRESS(CDataBank,						DBR)
ADDRESS(CAbsoluteAddressHigh,			AAH)
ADDRESS(CDirectOffset,					DirectOffset)
ADDRESS(CDirectPage,					DP)
ADDRESS(CNewProgramCounter,				New_PC)
ADDRESS(CNewProgramBank,				New_PBR)
ADDRESS(CAddressBank,					AAB)
ADDRESS(CDirectOffset,					D0)
ADDRESS(CAbsoluteAddressLowPlusXLow,	AAL_XL)
ADDRESS(CAbsoluteAddressLowPlusYLow,	AAL_YL)
ADDRESS(CSignedData,					SignedData)
ADDRESS(CSignedDataLow,					SignedDataLow)


OPERATION(CInternalFirst,				OpCode)
OPERATION(CIncrementProgramCounter,		PC_inc)
OPERATION(CDecrementStackPointer,		SP_dec)
OPERATION(CIncrementStackPointer,		SP_inc)
OPERATION(CWriteProgramBank,			Write_PBR)
OPERATION(CWriteProgramCounterLow,		Write_PCL)
OPERATION(CWriteProgramCounterHigh,		Write_PCH)
OPERATION(CWriteProcessorStatus,		Write_PS)
OPERATION(CReadProcessorStatus,			Read_PS)
OPERATION(CDoneInstruction,				DONE)
OPERATION(CWriteAbsoluteAddressLow,		Write_AAL)
OPERATION(CWriteAbsoluteAddressHigh,	Write_AAH)
OPERATION(CNoteTwo,						NoteTwo)
OPERATION(CNoteThree,					NoteThree)
OPERATION(CNoteSix,						NoteSix)
OPERATION(CNoteSeven,					NoteSeven)
OPERATION(CWaitOperation,				WaitOperation)
OPERATION(CRestoreAbortValues,			RestoreAbortValues)


CConstantOffset* o(int32 iOffset)
{
	CConstantOffset*	pcType;

	pcType = NewMalloc<CConstantOffset>();
	pcType->Init(iOffset); 
	return pcType;
}


CWriteDataHigh* Write_DataHigh(NotMemoryLock bNotMemoryLock)
{
	CWriteDataHigh*		pcType;

	pcType = NewMalloc<CWriteDataHigh>();
	pcType->Init(bNotMemoryLock);
	return pcType;
}


CWriteDataHigh* Write_DataHigh(void)
{
	return Write_DataHigh(true);
}


CInterruptAddress* VA(CInterruptVector* pcInterruptVector)
{
	CInterruptAddress* pcType;

	pcType = NewMalloc<CInterruptAddress>();
	pcType->Init(pcInterruptVector);
	return pcType;
}


CInternalOperation* IO(void)
{
	CInternalOperation* pcType;

	pcType = NewMalloc<CInternalOperation>();
	pcType->Init(true);
	return pcType;
}


CInternalOperation* IO(bool bValidProgramAddress, bool bValidDataAddress, NotMemoryLock bNotMemoryLock)
{
	CInternalOperation* pcType;

	pcType = NewMalloc<CInternalOperation>();
	pcType->Init(bValidProgramAddress, bValidDataAddress, bNotMemoryLock);
	return pcType;
}

CReadAbsoluteAddressHigh* Read_AAH(NotMemoryLock bNotMemoryLock)
{
	CReadAbsoluteAddressHigh* pcType;

	pcType = NewMalloc<CReadAbsoluteAddressHigh>();
	pcType->Init(bNotMemoryLock, true);
	return pcType;
}


CReadAbsoluteAddressHigh* Read_AAH(void)
{
	return Read_AAH(true);
}


CReadAbsoluteAddressLow* Read_AAL(void)
{
	CReadAbsoluteAddressLow* pcType;

	pcType = NewMalloc<CReadAbsoluteAddressLow>();
	pcType->Init(true, true);
	return pcType;
}


CReadAbsoluteAddressLow* Read_AAVL(void)
{
	CReadAbsoluteAddressLow* pcType;

	pcType = NewMalloc<CReadAbsoluteAddressLow>();
	pcType->Init(true, false);
	return pcType;
}


CReadAbsoluteAddressHigh* Read_AAVH(void)
{
	CReadAbsoluteAddressHigh* pcType;

	pcType = NewMalloc<CReadAbsoluteAddressHigh>();
	pcType->Init(true, false);
	return pcType;
}


CReadDirectOffset* Read_D0(void)
{
	CReadDirectOffset* pcType;

	pcType = NewMalloc<CReadDirectOffset>();
	pcType->Init(true);
	return pcType;
}


CReadProgramBank* Read_NewPBR(void)
{
	CReadProgramBank* pcType;

	pcType = NewMalloc<CReadProgramBank>();
	pcType->Init(true);
	return pcType;
}


CReadProgramCounterHigh* Read_NewPCH(void)
{
	CReadProgramCounterHigh* pcType;

	pcType = NewMalloc<CReadProgramCounterHigh>();
	pcType->Init(true);
	return pcType;
}


CReadProgramCounterLow* Read_NewPCL(void)
{
	CReadProgramCounterLow* pcType;

	pcType = NewMalloc<CReadProgramCounterLow>();
	pcType->Init(true);
	return pcType;
}


CSetProgramCounter* PC_e(CAddressOffsetArray* papcAddressOffsets)
{
	CSetProgramCounter* pcType;

	pcType = NewMalloc<CSetProgramCounter>();
	pcType->Init(papcAddressOffsets);
	return pcType;
}


CReadAbsoluteAddressBank* Read_AAB(void)
{
	CReadAbsoluteAddressBank* pcType;

	pcType = NewMalloc<CReadAbsoluteAddressBank>();
	pcType->Init(true);
	return pcType;
}


CReadDataBank* Read_DBR(void)
{
	CReadDataBank* pcType;

	pcType = NewMalloc<CReadDataBank>();
	pcType->Init(true);
	return pcType;
}


CReadAbsoluteAddressLow* Read_AAL(NotMemoryLock bNotMemoryLock)
{
	CReadAbsoluteAddressLow* pcType;

	pcType = NewMalloc<CReadAbsoluteAddressLow>();
	pcType->Init(bNotMemoryLock, true);
	return pcType;
}


CInternalOperation* IO(NotMemoryLock bNotMemoryLock)
{
	CInternalOperation* pcType;

	pcType = NewMalloc<CInternalOperation>();
	pcType->Init(bNotMemoryLock);
	return pcType;
}


CReadDataHigh* Read_DataHigh(NotMemoryLock bNotMemoryLock)
{
	CReadDataHigh* pcType;

	pcType = NewMalloc<CReadDataHigh>();
	pcType->Init(bNotMemoryLock);
	return pcType;
}


CReadDataHigh* Read_DataHigh(void)
{
	return Read_DataHigh(true);
}


CReadDataLow* Read_DataLow(NotMemoryLock bNotMemoryLock)
{
	CReadDataLow* pcType;

	pcType = NewMalloc<CReadDataLow>();
	pcType->Init(bNotMemoryLock);
	return pcType;
}

CReadDataLow* Read_DataLow(void)
{
	return Read_DataLow(true);
}


CWriteDataLow* Write_DataLow(NotMemoryLock bNotMemoryLock)
{
	CWriteDataLow* pcType;

	pcType = NewMalloc<CWriteDataLow>();
	pcType->Init(bNotMemoryLock);
	return pcType;
}


CWriteDataLow* Write_DataLow()
{
	return Write_DataLow(true);
}


CReadDirectOffset* Read_D0(NotMemoryLock bNotMemoryLock)
{
	CReadDirectOffset* pcType;

	pcType = NewMalloc<CReadDirectOffset>();
	pcType->Init(bNotMemoryLock);
	return pcType;
}


CSetProgramBank* PBR_e(uint8 uiBank)
{
	CSetProgramBank* pcType;

	pcType = NewMalloc<CSetProgramBank>();
	pcType->Init(uiBank);
	return pcType;
}


CExecuteIf16Bit* E16Bit(CW65C816Func fConsumer, EWidthFromRegister eWidth)
{
	CExecuteIf16Bit* pcType;

	pcType = NewMalloc<CExecuteIf16Bit>();
	pcType->Init(fConsumer, eWidth);
	return pcType;
}


CExecuteIf8Bit* E8Bit(CW65C816Func fConsumer, EWidthFromRegister eWidth)
{
	CExecuteIf8Bit* pcType;

	pcType = NewMalloc<CExecuteIf8Bit>();
	pcType->Init(fConsumer, eWidth);
	return pcType;
}


CExecute* E(CW65C816Func fConsumer)
{
	CExecute* pcType;

	pcType = NewMalloc<CExecute>();
	pcType->Init(fConsumer);
	return pcType;
}


CDoneInstructionIf8Bit* DONE8Bit(EWidthFromRegister eWidth)
{
	CDoneInstructionIf8Bit* pcType;

	pcType = NewMalloc<CDoneInstructionIf8Bit>();
	pcType->Init(eWidth);
	return pcType;
}


CDoneInstructionIf16Bit* DONE16Bit(EWidthFromRegister eWidth)
{
	CDoneInstructionIf16Bit* pcType;

	pcType = NewMalloc<CDoneInstructionIf16Bit>();
	pcType->Init(eWidth);
	return pcType;
}


CNoteOne* NoteOne(EWidthFromRegister eWidth)
{
	CNoteOne* pcType;

	pcType = NewMalloc<CNoteOne>();
	pcType->Init(eWidth);
	return pcType;
}


CNoteFourX* NoteFourX(bool bNextWillRead)
{
	CNoteFourX* pcType;

	pcType = NewMalloc<CNoteFourX>();
	pcType->Init(bNextWillRead);
	return pcType;
}


CNoteFourY* NoteFourY(bool bNextWillRead)
{
	CNoteFourY* pcType;

	pcType = NewMalloc<CNoteFourY>();
	pcType->Init(bNextWillRead);
	return pcType;
}


CInstructionCycles* CreateFetchOpCodeCycles(void)
{
	CReadOpCode* pcReadOpCode;

	pcReadOpCode = NewMalloc<CReadOpCode>();
	pcReadOpCode->Init();

	//0*
	return InstructionCycles(AM_OpCode,
		BusCycle(	Address(PBR(), PC()), Operation(pcReadOpCode, PC_inc())));
}


CInstructionCycles* CreateAbsoluteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
{
	//1a
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC()),			Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_AAL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_AAH(), PC_inc())),
		BusCycle(	Address(DBR(), AA()),			Operation(Read_DataLow(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth))),
		BusCycle(	Address(DBR(), AA(), o(1)),		Operation(Read_DataHigh(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth))));
}


CInstructionCycles* CreateAbsoluteWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
{
	//1a
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC()),			Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_AAL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_AAH(), PC_inc())),
		BusCycle(	Address(DBR(), AA()),			Operation(E(fOperation), Write_DataLow(), DONE8Bit(eWidth))),
		BusCycle(	Address(DBR(), AA(), o(1)),		Operation(Write_DataHigh(), DONE16Bit(eWidth))));
}


CInstructionCycles* CreateAbsoluteJMPCycles(void)
{
	//1b
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_NewPCL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_NewPCH(), PC_inc(), PC_e(Address(PBR(), New_PC())), DONE())));
}


CInstructionCycles* CreateAbsoluteJSRCycles(void)
{
	//1c
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_NewPCL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_NewPCH(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(S()),			Operation(Write_PCH(), SP_dec())),
		BusCycle(	Address(S()),			Operation(Write_PCL(), SP_dec(), PC_e(Address(PBR(), New_PC())), DONE())));
}


CInstructionCycles* CreateAbsoluteRMWCycles(CW65C816Func fOperation)
{
	//1d
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC()),			Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_AAL(RMW), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_AAH(RMW), PC_inc())),
		BusCycle(	Address(DBR(), AA()),			Operation(Read_DataLow(RMW))),
		BusCycle(	Address(DBR(), AA(), o(1)),		Operation(Read_DataHigh(RMW), NoteOne(WFR_M))),
		BusCycle(	Address(DBR(), AA(), o(1)),		Operation(IO(RMW), E(fOperation))),
		BusCycle(	Address(DBR(), AA(), o(1)),		Operation(NoteThree(), Write_DataHigh(RMW), NoteOne(WFR_M))),
		BusCycle(	Address(DBR(), AA()),			Operation(NoteThree(), Write_DataLow(RMW), DONE())));
}


CInstructionCycles* CreateAbsoluteIndexedIndirectWithXJMPCycles(void)
{
	//2a
	return InstructionCycles(AM_AbsoluteIndexedIndirectWithX,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAH(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO())),
		BusCycle(	Address(PBR(), AA(), X()),			Operation(Read_NewPCL())),
		BusCycle(	Address(PBR(), AA(), X(), o(1)),	Operation(Read_NewPCH(), PC_e(Address(PBR(), New_PC())), DONE())));
}


CInstructionCycles* CreateAbsoluteIndexedIndirectWithXJSRCycles(void)
{
	//2b
	return InstructionCycles(AM_AbsoluteIndexedIndirectWithX,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAL(), PC_inc())),
		BusCycle(	Address(S()),						Operation(Write_PCH(), SP_dec())),
		BusCycle(	Address(S()),						Operation(Write_PCL(), SP_dec())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAH())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO(), PC_inc())),
		BusCycle(	Address(PBR(), AA(), X()),			Operation(Read_NewPCL())),
		BusCycle(	Address(PBR(), AA(), X(), o(1)),	Operation(Read_NewPCH(), PC_e(Address(PBR(), New_PC())), DONE())));
}


CInstructionCycles* CreateAbsoluteIndirectJMLCycles(void)
{
	//3a
	return InstructionCycles(AM_AbsoluteIndirectLong,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_AAL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_AAH(), PC_inc())),
		BusCycle(	Address(AA()),			Operation(Read_NewPCL())),
		BusCycle(	Address(AA(), o(1)),	Operation(Read_NewPCH())),
		BusCycle(	Address(AA(), o(2)),	Operation(Read_NewPBR(), PC_e(Address(New_PBR(), New_PC())), DONE())));
}


CInstructionCycles* CreateAbsoluteIndirectJMPCycles(void)
{
	//3b
	return InstructionCycles(AM_AbsoluteIndirect,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_AAL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_AAH(), PC_inc())),
		BusCycle(	Address(AA()),			Operation(Read_NewPCL())),
		BusCycle(	Address(AA(), o(1)),	Operation(Read_NewPCH(), PC_e(Address(PBR(), New_PC())), DONE())));
}


CInstructionCycles* CreateAbsoluteLongWriteCycles(CW65C816Func fOperation)
{
	//4a
	return InstructionCycles(AM_AbsoluteLong,
		BusCycle(	Address(PBR(), PC()),			Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_AAL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_AAH(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_AAB(), PC_inc())),
		BusCycle(	Address(AAB(), AA()),			Operation(E(fOperation), Write_DataLow(), DONE8Bit(WFR_M))),
		BusCycle(	Address(AAB(), AA(), o(1)),		Operation(Write_DataHigh(), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateAbsoluteLongCycles(CW65C816Func fOperation)
{
	//4a
	return InstructionCycles(AM_AbsoluteLong,
		BusCycle(	Address(PBR(), PC()),			Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_AAL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_AAH(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_AAB(), PC_inc())),
		BusCycle(	Address(AAB(), AA()),			Operation(Read_DataLow(), E8Bit(fOperation, WFR_M), DONE8Bit(WFR_M))),
		BusCycle(	Address(AAB(), AA(), o(1)),		Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateAbsoluteLongJMLCycles(void)
{
	//4b
	return InstructionCycles(AM_AbsoluteLong,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_NewPCL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_NewPCH(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_NewPBR(), PC_inc(), PC_e(Address(PBR(), New_PC())), DONE())));
}


CInstructionCycles* CreateAbsoluteLongJSLCycles(void)
{
	//4c
	return InstructionCycles(AM_AbsoluteLong,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_NewPCL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_NewPCH(), PC_inc())),
		BusCycle(	Address(S()),			Operation(Write_PBR(), SP_dec())),
		BusCycle(	Address(S()),			Operation(IO())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_NewPBR(), PC_inc())),
		BusCycle(	Address(S()),			Operation(Write_PCH(), SP_dec())),
		BusCycle(	Address(S()),			Operation(Write_PCL(), SP_dec(), PC_e(Address(PBR(), New_PC())), DONE())));
}


CInstructionCycles* CreateAbsoluteLongIndexedWithXCycles(CW65C816Func fOperation)
{
	//5
	return InstructionCycles(AM_AbsoluteLongIndexedWithX,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAH(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAB(), PC_inc())),
		BusCycle(	Address(AAB(), AA(), X()),			Operation(Read_DataLow(), E8Bit(fOperation, WFR_M), DONE8Bit(WFR_M))),
		BusCycle(	Address(AAB(), AA(), X(), o(1)),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateAbsoluteLongIndexedWithXWriteCycles(CW65C816Func fOperation)
{
	//5
	return InstructionCycles(AM_AbsoluteLongIndexedWithX,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAH(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAB(), PC_inc())),
		BusCycle(	Address(AAB(), AA(), X()),			Operation(E(fOperation), Write_DataLow(), DONE8Bit(WFR_M))),
		BusCycle(	Address(AAB(), AA(), X(), o(1)),	Operation(Write_DataHigh(), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateAbsoluteIndexedWithXWriteCycles(CW65C816Func fOperation)
{
	//6a
	return InstructionCycles(AM_AbsoluteIndexedWithX,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAH(), PC_inc())),
		BusCycle(	Address(DBR(), AAH(), AAL_XL()),	Operation(IO(), NoteFourX(false))),
		BusCycle(	Address(DBR(), AA(), X()),			Operation(E(fOperation), Write_DataLow(), DONE8Bit(WFR_M))),
		BusCycle(	Address(DBR(), AA(), X(), o(1)),	Operation(Write_DataHigh(), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateAbsoluteIndexedWithXCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
{
	//6a
	return InstructionCycles(AM_AbsoluteIndexedWithX,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAH(), PC_inc())),
		BusCycle(	Address(DBR(), AAH(), AAL_XL()),	Operation(IO(), NoteFourX(true))),
		BusCycle(	Address(DBR(), AA(), X()),			Operation(Read_DataLow(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth))),
		BusCycle(	Address(DBR(), AA(), X(), o(1)),	Operation(Read_DataHigh(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth))));
}


CInstructionCycles* CreateAbsoluteIndexedWithXRMWCycles(CW65C816Func fOperation)
{
	//6b
	return InstructionCycles(AM_AbsoluteIndexedWithX,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAL(RMW), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAH(RMW), PC_inc())),
		BusCycle(	Address(DBR(), AAH(), AAL_XL()),	Operation(IO(RMW))),
		BusCycle(	Address(DBR(), AA(), X()),			Operation(Read_DataLow(RMW))),
		BusCycle(	Address(DBR(), AA(), X(), o(1)),	Operation(Read_DataHigh(RMW), NoteOne(WFR_M))),
		BusCycle(	Address(DBR(), AA(), X(), o(1)),	Operation(IO(RMW), E(fOperation))),
		BusCycle(	Address(DBR(), AA(), X(), o(1)),	Operation(NoteThree(), Write_DataHigh(RMW), NoteOne(WFR_M))),
		BusCycle(	Address(DBR(), AA(), X()),			Operation(NoteThree(), Write_DataLow(RMW), DONE())));
}


CInstructionCycles* CreateAbsoluteIndexedWithYWriteCycles(CW65C816Func fOperation)
{
	//7
	return InstructionCycles(AM_AbsoluteIndexedWithY,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAH(), PC_inc())),
		BusCycle(	Address(DBR(), AAH(), AAL_YL()),	Operation(IO(), NoteFourY(false))),
		BusCycle(	Address(DBR(), AA(), Y()),			Operation(E(fOperation), Write_DataLow(), DONE8Bit(WFR_M))),
		BusCycle(	Address(DBR(), AA(), Y(), o(1)),	Operation(Write_DataHigh(), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateAbsoluteIndexedWithYCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
{
	//7
	return InstructionCycles(AM_AbsoluteIndexedWithY,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_AAH(), PC_inc())),
		BusCycle(	Address(DBR(), AAH(), AAL_YL()),	Operation(IO(), NoteFourY(true))),
		BusCycle(	Address(DBR(), AA(), Y()),			Operation(Read_DataLow(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth))),
		BusCycle(	Address(DBR(), AA(), Y(), o(1)),	Operation(Read_DataHigh(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth))));
}


CInstructionCycles* CreateAccumulatorCycles(CW65C816Func fOperation)
{
	//8
	return InstructionCycles(AM_Accumulator,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO(), E(fOperation), DONE())));
}


CInstructionCycles* CreateBlockMoveCycles(CW65C816Func fOperation)
{
	//9a & 9b
	return InstructionCycles(AM_BlockMove,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_DBR(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_AAB(), PC_inc())),
		BusCycle(	Address(AAB(), X()),	Operation(Read_DataLow())),
		BusCycle(	Address(DBR(), Y()),	Operation(Write_DataLow(), E(fOperation))),
		BusCycle(	Address(DBR(), Y()),	Operation(IO())),
		BusCycle(	Address(DBR(), Y()),	Operation(IO(), DONE())));
}


CInstructionCycles* CreateDirectCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
{
	//10a
	return InstructionCycles(AM_Direct,
		BusCycle(	Address(PBR(), PC()),		Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),		Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),		Operation(IO(), NoteTwo())),
		BusCycle(	Address(DP(), D0()),		Operation(Read_DataLow(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth))),
		BusCycle(	Address(DP(), D0(), o(1)),	Operation(Read_DataHigh(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth))));
}


CInstructionCycles* CreateDirectWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
{
	//10a
	return InstructionCycles(AM_Direct,
		BusCycle(	Address(PBR(), PC()),		Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),		Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),		Operation(IO(), NoteTwo())),
		BusCycle(	Address(DP(), D0()),		Operation(E(fOperation), Write_DataLow(), DONE8Bit(eWidth))),
		BusCycle(	Address(DP(), D0(), o(1)),	Operation(Write_DataHigh(), DONE16Bit(eWidth))));
}


CInstructionCycles* CreateDirectRMWCycles(CW65C816Func fOperation)
{
	//10b
	return InstructionCycles(AM_Direct,
		BusCycle(	Address(PBR(), PC()),		Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),		Operation(Read_D0(RMW), PC_inc())),
		BusCycle(	Address(PBR(), PC()),		Operation(IO(), NoteTwo())),
		BusCycle(	Address(DP(), D0()),		Operation(Read_DataLow(RMW))),
		BusCycle(	Address(DP(), D0(), o(1)),	Operation(Read_DataHigh(RMW), NoteOne(WFR_M))),
		BusCycle(	Address(DP(), D0(), o(1)),	Operation(IO(RMW), E(fOperation))),
		BusCycle(	Address(DP(), D0(), o(1)),	Operation(NoteThree(), Write_DataHigh(RMW), NoteOne(WFR_M))),
		BusCycle(	Address(DP(), D0()),		Operation(NoteThree(), Write_DataLow(RMW), DONE())));
}


CInstructionCycles* CreateDirectIndexedIndirectWithXCycles(CW65C816Func fOperation)
{
	//11
	return InstructionCycles(AM_DirectIndexedIndirectWithX,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO(), NoteTwo())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO())),
		BusCycle(	Address(DP(), D0(), X()),			Operation(Read_AAL())),
		BusCycle(	Address(DP(), D0(), X(), o(1)),		Operation(Read_AAH())),
		BusCycle(	Address(DBR(), AA()),				Operation(Read_DataLow(), E8Bit(fOperation, WFR_M), DONE8Bit(WFR_M))),
		BusCycle(	Address(DBR(), AA(), o(1)),			Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateDirectIndexedIndirectWithXWriteCycles(CW65C816Func fOperation)
{
	//11
	return InstructionCycles(AM_DirectIndexedIndirectWithX,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO(), NoteTwo())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO())),
		BusCycle(	Address(DP(), D0(), X()),			Operation(Read_AAL())),
		BusCycle(	Address(DP(), D0(), X(), o(1)),		Operation(Read_AAH())),
		BusCycle(	Address(DBR(), AA()),				Operation(E(fOperation), Write_DataLow(), DONE8Bit(WFR_M))),
		BusCycle(	Address(DBR(), AA(), o(1)),			Operation(Write_DataHigh(), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateDirectIndirectCycles(CW65C816Func fOperation)
{
	//12
	return InstructionCycles(AM_DirectIndirect,
		BusCycle(	Address(PBR(), PC()),			Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(IO(), NoteTwo())),
		BusCycle(	Address(DP(), D0()),			Operation(Read_AAL())),
		BusCycle(	Address(DP(), D0(), o(1)),		Operation(Read_AAH())),
		BusCycle(	Address(DBR(), AA()),			Operation(Read_DataLow(), E8Bit(fOperation, WFR_M), DONE8Bit(WFR_M))),
		BusCycle(	Address(DBR(), AA(), o(1)),		Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateDirectIndirectWriteCycles(CW65C816Func fOperation)
{
	//12
	return InstructionCycles(AM_DirectIndirect,
		BusCycle(	Address(PBR(), PC()),			Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(IO(), NoteTwo())),
		BusCycle(	Address(DP(), D0()),			Operation(Read_AAL())),
		BusCycle(	Address(DP(), D0(), o(1)),		Operation(Read_AAH())),
		BusCycle(	Address(DBR(), AA()),			Operation(E(fOperation), Write_DataLow(), DONE8Bit(WFR_M))),
		BusCycle(	Address(DBR(), AA(), o(1)),		Operation(Write_DataHigh(), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateDirectIndirectIndexedWithYCycles(CW65C816Func fOperation)
{
	//13
	return InstructionCycles(AM_DirectIndirectIndexedWithY,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO(), NoteTwo())),
		BusCycle(	Address(DP(), D0()),				Operation(Read_AAL())),
		BusCycle(	Address(DP(), D0(), o(1)),			Operation(Read_AAH())),
		BusCycle(	Address(DBR(), AAH(), AAL_YL()),	Operation(IO(), NoteFourY(true))),
		BusCycle(	Address(DBR(), AA(), Y()),			Operation(Read_DataLow(), E8Bit(fOperation, WFR_M), DONE8Bit(WFR_M))),
		BusCycle(	Address(DBR(), AA(), Y(), o(1)),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateDirectIndirectIndexedWithYWriteCycles(CW65C816Func fOperation)
{
	//13
	return InstructionCycles(AM_DirectIndirectIndexedWithY,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO(), NoteTwo())),
		BusCycle(	Address(DP(), D0()),				Operation(Read_AAL())),
		BusCycle(	Address(DP(), D0(), o(1)),			Operation(Read_AAH())),
		BusCycle(	Address(DBR(), AAH(), AAL_YL()),	Operation(IO(), NoteFourY(false))),
		BusCycle(	Address(DBR(), AA(), Y()),			Operation(E(fOperation), Write_DataLow(), DONE8Bit(WFR_M))),
		BusCycle(	Address(DBR(), AA(), Y(), o(1)),	Operation(Write_DataHigh(), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateDirectIndirectLongIndexedWithYCycles(CW65C816Func fOperation)
{
	//14
	return InstructionCycles(AM_DirectIndirectLongIndexedWithY,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO(), NoteTwo())),
		BusCycle(	Address(DP(), D0()),				Operation(Read_AAL())),
		BusCycle(	Address(DP(), D0(), o(1)),			Operation(Read_AAH())),
		BusCycle(	Address(DP(), D0(), o(2)),			Operation(Read_AAB())),
		BusCycle(	Address(AAB(), AA(), Y()),			Operation(Read_DataLow(), E8Bit(fOperation, WFR_M), DONE8Bit(WFR_M))),
		BusCycle(	Address(AAB(), AA(), Y(), o(1)),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateDirectIndirectLongIndexedWithYWriteCycles(CW65C816Func fOperation)
{
	//14
	return InstructionCycles(AM_DirectIndirectLongIndexedWithY,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO(), NoteTwo())),
		BusCycle(	Address(DP(), D0()),				Operation(Read_AAL())),
		BusCycle(	Address(DP(), D0(), o(1)),			Operation(Read_AAH())),
		BusCycle(	Address(DP(), D0(), o(2)),			Operation(Read_AAB())),
		BusCycle(	Address(AAB(), AA(), Y()),			Operation(E(fOperation), Write_DataLow(), DONE8Bit(WFR_M))),
		BusCycle(	Address(AAB(), AA(), Y(), o(1)),	Operation(Write_DataHigh(), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateDirectIndirectLongCycles(CW65C816Func fOperation)
{
	//15
	return InstructionCycles(AM_DirectIndirectLong,
		BusCycle(	Address(PBR(), PC()),			Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(IO(), NoteTwo())),
		BusCycle(	Address(DP(), D0()),			Operation(Read_AAL())),
		BusCycle(	Address(DP(), D0(), o(1)),		Operation(Read_AAH())),
		BusCycle(	Address(DP(), D0(), o(2)),		Operation(Read_AAB())),
		BusCycle(	Address(AAB(), AA()),			Operation(Read_DataLow(), E8Bit(fOperation, WFR_M), DONE8Bit(WFR_M))),
		BusCycle(	Address(AAB(), AA(), o(1)),		Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateDirectIndirectLongWriteCycles(CW65C816Func fOperation)
{
	//15
	return InstructionCycles(AM_DirectIndirectLong,
		BusCycle(	Address(PBR(), PC()),			Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),			Operation(IO(), NoteTwo())),
		BusCycle(	Address(DP(), D0()),			Operation(Read_AAL())),
		BusCycle(	Address(DP(), D0(), o(1)),		Operation(Read_AAH())),
		BusCycle(	Address(DP(), D0(), o(2)),		Operation(Read_AAB())),
		BusCycle(	Address(AAB(), AA()),			Operation(E(fOperation), Write_DataLow(), DONE8Bit(WFR_M))),
		BusCycle(	Address(AAB(), AA(), o(1)),		Operation(Write_DataHigh(), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateDirectIndexedWithXCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
{
	//16a
	return InstructionCycles(AM_DirectIndexedWithX,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO(), NoteTwo())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO())),
		BusCycle(	Address(DP(), D0(), X()),			Operation(Read_DataLow(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth))),
		BusCycle(	Address(DP(), D0(), X(), o(1)),		Operation(Read_DataHigh(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth))));
}


CInstructionCycles* CreateDirectIndexedWithXWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
{
	//16a
	return InstructionCycles(AM_DirectIndexedWithX,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO(), NoteTwo())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO())),
		BusCycle(	Address(DP(), D0(), X()),			Operation(E(fOperation), Write_DataLow(), DONE8Bit(eWidth))),
		BusCycle(	Address(DP(), D0(), X(), o(1)),		Operation(Write_DataHigh(), DONE16Bit(eWidth))));
}


CInstructionCycles* CreateDirectIndexedWithXRMWCycles(CW65C816Func fOperation)
{
	//16b
	return InstructionCycles(AM_DirectIndexedWithX,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_D0(RMW), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO(RMW), NoteTwo())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO(RMW))),
		BusCycle(	Address(DP(), D0(), X()),			Operation(Read_DataLow(RMW))),
		BusCycle(	Address(DP(), D0(), X(), o(1)),		Operation(Read_DataHigh(RMW), NoteOne(WFR_M))),
		BusCycle(	Address(DP(), D0(), X(), o(1)),		Operation(IO(RMW), E(fOperation))),
		BusCycle(	Address(DP(), D0(), X(), o(1)),		Operation(NoteThree(), Write_DataHigh(RMW), NoteOne(WFR_M))),
		BusCycle(	Address(DP(), D0(), X()),			Operation(NoteThree(), Write_DataLow(RMW), DONE())));
}


CInstructionCycles* CreateDirectIndexedWithYCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
{
	//17
	return InstructionCycles(AM_DirectIndexedWithY,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO(), NoteTwo())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO())),
		BusCycle(	Address(DP(), D0(), Y()),			Operation(Read_DataLow(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth))),
		BusCycle(	Address(DP(), D0(), Y(), o(1)),		Operation(Read_DataHigh(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth))));
}


CInstructionCycles* CreateDirectIndexedWithYWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
{
	//17
	return InstructionCycles(AM_DirectIndexedWithY,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO(), NoteTwo())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO())),
		BusCycle(	Address(DP(), D0(), Y()),			Operation(E(fOperation), Write_DataLow(), DONE8Bit(eWidth))),
		BusCycle(	Address(DP(), D0(), Y(), o(1)),		Operation(Write_DataHigh(), DONE16Bit(eWidth))));
}


CInstructionCycles* CreateImmediateREPSEPCycles(CW65C816Func fOperation)
{
	//18
	return InstructionCycles(AM_Immediate,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(PC_inc(), Read_DataLow())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_DataHigh(), E(fOperation), DONE())));
}


CInstructionCycles* CreateImmediateCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
{
	//18
	return InstructionCycles(AM_Immediate,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(PC_inc(), Read_DataLow(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth))),
		BusCycle(	Address(PBR(), PC()),	Operation(PC_inc(), Read_DataHigh(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth))));
}


CInstructionCycles* CreateImpliedCycles(CW65C816Func fOperation)
{
	//19a
	return InstructionCycles(AM_Implied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO(), E(fOperation), DONE())));
}


CInstructionCycles* CreateWDMImpliedCycles(CW65C816Func fOperation)
{
	return InstructionCycles(AM_Implied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(PC_inc(), IO(), E(fOperation), DONE())));
}


CInstructionCycles* CreateImpliedXBACycles(CW65C816Func fOperation)
{
	//19b
	return InstructionCycles(AM_Implied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO(), E(fOperation), DONE())));
}


CInstructionCycles* CreateStopTheClockCycles(CW65C816Func fOperation)
{
	//19c
	return InstructionCycles(AM_StopTheClock,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO(), E(fOperation), DONE())));
}


CInstructionCycles* CreateWaitForInterruptCycles()
{
	//19d
	return InstructionCycles(AM_WaitForInterrupt,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(PBR(), PC()),	Operation(WaitOperation(), DONE())));
}


CInstructionCycles* CreateRelativeShortCycles(CW65C816Func fOperation)
{
	//20
	return InstructionCycles(AM_Relative,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_DataLow(), PC_inc(), E(fOperation))),  //Done if branch not taken
		BusCycle(	Address(PBR(), PC()),	Operation(IO(), NoteSix())),
		BusCycle(	Address(PBR(), PC()),	Operation(PC_e(Address(PBR(), PC(), SignedDataLow())), IO(), DONE())));
}


CInstructionCycles* CreateRelativeLongCycles(CW65C816Func fOperation)
{
	//21
	return InstructionCycles(AM_RelativeLong,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_DataLow(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_DataHigh(), PC_inc(), E(fOperation))),  //Done if branch not taken
		BusCycle(	Address(PBR(), PC()),	Operation(PC_e(Address(PBR(), PC(), SignedData())), IO(), DONE())));
}


CInstructionCycles* CreateStackHardwareInterruptCycles(CInterruptVector* pcInterruptVector, CW65C816Func fOperation)
{
	//22a
	return InstructionCycles(AM_StackInterruptHardware,
		BusCycle(	Address(PBR(), PC()),					Operation(IO(true, true, true))),
		BusCycle(	Address(PBR(), PC()),					Operation(IO())),
		BusCycle(	Address(S()),							Operation(Write_PBR(), SP_dec(), PBR_e(0), NoteSeven())),
		BusCycle(	Address(S()),							Operation(Write_PCH(), SP_dec())),
		BusCycle(	Address(S()),							Operation(Write_PCL(), SP_dec())),
		BusCycle(	Address(S()),							Operation(Write_PS(), SP_dec(), E(fOperation))),
		BusCycle(	Address(VA(pcInterruptVector)),			Operation(Read_AAVL())),
		BusCycle(	Address(VA(pcInterruptVector), o(1)),	Operation(Read_AAVH(), PC_e(Address(AA())), DONE())));
}


CInstructionCycles* CreateStackAbortInterruptCycles(CInterruptVector* pcInterruptVector, CW65C816Func fOperation)
{
	//22a
	return InstructionCycles(AM_StackInterruptHardware,
		BusCycle(	Address(PBR(), PC()),					Operation(IO(true, true, true))),
		BusCycle(	Address(PBR(), PC()),					Operation(IO(), RestoreAbortValues())),
		BusCycle(	Address(S()),							Operation(Write_PBR(), SP_dec(), PBR_e(0), NoteSeven())),
		BusCycle(	Address(S()),							Operation(Write_PCH(), SP_dec())),
		BusCycle(	Address(S()),							Operation(Write_PCL(), SP_dec())),
		BusCycle(	Address(S()),							Operation(Write_PS(), SP_dec(), E(fOperation))),
		BusCycle(	Address(VA(pcInterruptVector)),			Operation(Read_AAVL())),
		BusCycle(	Address(VA(pcInterruptVector), o(1)),	Operation(Read_AAVH(), PC_e(Address(AA())), DONE())));
}


CInstructionCycles* CreateStackResetCycles(CInterruptVector* pcInterruptVector, CW65C816Func fOperation)
{
	//22a
	return InstructionCycles(AM_StackInterruptHardware,
		BusCycle(	Address(PBR(), PC()),					Operation(IO(true, true, true))),
		BusCycle(	Address(PBR(), PC()),					Operation(IO(), E(fOperation))),
		BusCycle(	Address(S()),							Operation(IO(), SP_dec())),
		BusCycle(	Address(S()),							Operation(IO(), SP_dec())),
		BusCycle(	Address(S()),							Operation(IO(), SP_dec())),
		BusCycle(	Address(VA(pcInterruptVector)),			Operation(Read_AAVL())),
		BusCycle(	Address(VA(pcInterruptVector), o(1)),	Operation(Read_AAVH(), PC_e(Address(PBR(), AA())), DONE())));
}


CInstructionCycles* CreateStackPullCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
{
	//22b
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(S(), o(1)),		Operation(Read_DataLow(), SP_inc(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth))),
		BusCycle(	Address(S(), o(1)),		Operation(Read_DataHigh(), SP_inc(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth))));
}


CInstructionCycles* CreateStackPLBCycles(CW65C816Func fOperation)
{
	//22b
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(S(), o(1)),		Operation(SP_inc(), Read_DataLow(), E(fOperation), DONE())));
}


CInstructionCycles* CreateStackPLDCycles(CW65C816Func fOperation)
{
	//22b
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(S(), o(1)),		Operation(Read_DataLow(), SP_inc())),
		BusCycle(	Address(S(), o(1)),		Operation(Read_DataHigh(), SP_inc(), E(fOperation), DONE())));
}


CInstructionCycles* CreateStackPLPCycles(CW65C816Func fOperation)
{
	//22b
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(S(), o(1)),		Operation(IO())),
		BusCycle(	Address(S(), o(1)),		Operation(Read_DataLow(), SP_inc(), E(fOperation), DONE())));
}


CInstructionCycles* CreateStackImpliedPHPCycles(CW65C816Func fOperation)
{
	//22c
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(S()),			Operation(E(fOperation), Write_DataLow(), SP_dec(), DONE())));
}


CInstructionCycles* CreateStackPushCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
{
	//22c
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO(), E(fOperation))),
		BusCycle(	Address(S()),			Operation(Write_DataHigh(), SP_dec(), NoteOne(eWidth))),
		BusCycle(	Address(S()),			Operation(Write_DataLow(), SP_dec(), DONE())));
}


CInstructionCycles* CreateStackPHBCycles(CW65C816Func fOperation)
{
	//22c
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(S()),			Operation(E(fOperation), Write_DataLow(), SP_dec(), DONE())));
}


CInstructionCycles* CreateStackPHDCycles(CW65C816Func fOperation)
{
	//22c
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(S()),			Operation(E(fOperation), Write_DataHigh(), SP_dec())),
		BusCycle(	Address(S()),			Operation(Write_DataLow(), SP_dec(), DONE())));
}


CInstructionCycles* CreateStackPHKCycles(CW65C816Func fOperation)
{
	//22c
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(S()),			Operation(E(fOperation), Write_DataLow(), SP_dec(), DONE())));
}


CInstructionCycles* CreateStackPEACycles()
{
	//22d
	return InstructionCycles(AM_StackImmediate,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_AAL(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_AAH())),
		BusCycle(	Address(S()),			Operation(Write_AAH(), SP_dec())),
		BusCycle(	Address(S()),			Operation(Write_AAL(), SP_dec(), DONE())));
}


CInstructionCycles* CreateStackPEICycles()
{
	//22e
	return InstructionCycles(AM_StackDirectIndirect,
		BusCycle(	Address(PBR(), PC()),		Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),		Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),		Operation(IO(), NoteTwo())),
		BusCycle(	Address(DP(), D0()),		Operation(Read_AAL())),
		BusCycle(	Address(DP(), D0(), o(1)),	Operation(Read_AAH())),
		BusCycle(	Address(S()),				Operation(Write_AAH(), SP_dec())),
		BusCycle(	Address(S()),				Operation(Write_AAL(), SP_dec(), DONE())));
}


CInstructionCycles* CreateStackPERCycles(CW65C816Func fOperation)
{
	//22f
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_DataLow(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(Read_DataHigh(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(S()),			Operation(E(fOperation), Write_DataHigh(), SP_dec())),
		BusCycle(	Address(S()),			Operation(Write_DataLow(), SP_dec(), DONE())));
}


CInstructionCycles* CreateStackRTICycles()
{
	//22g
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(S(), o(1)),		Operation(Read_PS(), SP_inc())),  //Processor status
		BusCycle(	Address(S(), o(1)),		Operation(Read_NewPCL(), SP_inc())),
		BusCycle(	Address(S(), o(1)),		Operation(Read_NewPCH(), SP_inc())),
		BusCycle(	Address(S(), o(1)),		Operation(Read_NewPBR(), SP_inc(), PC_e(Address(New_PBR(), New_PC())), DONE())));
}


CInstructionCycles* CreateStackRTSCycles()
{
	//22h
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(S(), o(1)),		Operation(Read_NewPCL(), SP_inc())),
		BusCycle(	Address(S(), o(1)),		Operation(Read_NewPCH(), SP_inc())),
		BusCycle(	Address(S()), 			Operation(IO(), PC_e(Address(PBR(), New_PC())), DONE())));
}


CInstructionCycles* CreateStackRTLCycles()
{
	//22i
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC()),	Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),	Operation(IO())),
		BusCycle(	Address(S(), o(1)),		Operation(Read_NewPCL(), SP_inc())),
		BusCycle(	Address(S(), o(1)),		Operation(Read_NewPCH(), SP_inc())),
		BusCycle(	Address(S(), o(1)),		Operation(Read_NewPBR(), SP_inc(), PC_e(Address(New_PBR(), New_PC())), DONE())));
}


CInstructionCycles* CreateStackSoftwareInterruptCycles(CInterruptVector* pcInterruptVector, CW65C816Func fOperation)
{
	//22j
	return InstructionCycles(AM_StackInterruptSoftware,
		BusCycle(	Address(PBR(), PC()),					Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),					Operation(IO(true, false, true), PC_inc())),  //Read and ignore the signature byte.
		BusCycle(	Address(S()),							Operation(Write_PBR(), SP_dec(), PBR_e(0), NoteSeven())),
		BusCycle(	Address(S()),							Operation(Write_PCH(), SP_dec())),
		BusCycle(	Address(S()),							Operation(Write_PCL(), SP_dec())),
		BusCycle(	Address(S()),							Operation(Write_PS(), SP_dec(), E(fOperation))),
		BusCycle(	Address(VA(pcInterruptVector)),			Operation(Read_AAL())),
		BusCycle(	Address(VA(pcInterruptVector), o(1)),	Operation(Read_AAH(), PC_e(Address(PBR(), AA())), DONE())));
}


CInstructionCycles* CreateStackRelativeCycles(CW65C816Func fOperation)
{
	//23
	return InstructionCycles(AM_StackRelative,
		BusCycle(	Address(PBR(), PC()),		Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),		Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),		Operation(IO())),
		BusCycle(	Address(S(), D0()),			Operation(Read_DataLow(), E8Bit(fOperation, WFR_M), DONE8Bit(WFR_M))),
		BusCycle(	Address(S(), D0(), o(1)),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateStackRelativeWriteCycles(CW65C816Func fOperation)
{
	//23
	return InstructionCycles(AM_StackRelative,
		BusCycle(	Address(PBR(), PC()),		Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),		Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),		Operation(IO())),
		BusCycle(	Address(S(), D0()),			Operation(E(fOperation), Write_DataLow(), DONE8Bit(WFR_M))),
		BusCycle(	Address(S(), D0(), o(1)),	Operation(Write_DataHigh(), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateStackRelativeIndirectIndexedWithYCycles(CW65C816Func fOperation)
{
	//24
	return InstructionCycles(AM_StackRelativeIndirectIndexedWithY,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO())),
		BusCycle(	Address(S(), D0()),					Operation(Read_AAL())),
		BusCycle(	Address(S(), D0(), o(1)),			Operation(Read_AAH())),
		BusCycle(	Address(S(), D0(), o(1)),			Operation(IO())),
		BusCycle(	Address(DBR(), AA(), Y()),			Operation(Read_DataLow(), E8Bit(fOperation, WFR_M), DONE8Bit(WFR_M))),
		BusCycle(	Address(DBR(), AA(), Y(), o(1)),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M), DONE16Bit(WFR_M))));
}


CInstructionCycles* CreateStackRelativeIndirectIndexedWithYWriteCycles(CW65C816Func fOperation)
{
	//24
	return InstructionCycles(AM_StackRelativeIndirectIndexedWithY,
		BusCycle(	Address(PBR(), PC()),				Operation(OpCode(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(Read_D0(), PC_inc())),
		BusCycle(	Address(PBR(), PC()),				Operation(IO())),
		BusCycle(	Address(S(), D0()),					Operation(Read_AAL())),
		BusCycle(	Address(S(), D0(), o(1)),			Operation(Read_AAH())),
		BusCycle(	Address(S(), D0(), o(1)),			Operation(IO())),
		BusCycle(	Address(DBR(), AA(), Y()),			Operation(E(fOperation), Write_DataLow(), DONE8Bit(WFR_M))),
		BusCycle(	Address(DBR(), AA(), Y(), o(1)),	Operation(Write_DataHigh(), DONE16Bit(WFR_M))));
}

