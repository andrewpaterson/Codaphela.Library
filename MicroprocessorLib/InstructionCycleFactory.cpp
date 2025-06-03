#include "CompoundAddress.h"
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
	size		            iCount;
	CBusCycleArray*			papcBusCycles;

	papcBusCycles = NewMalloc<CBusCycleArray>();
	papcBusCycles->Init();

    if (pcBusCycle)
    {
		iCount = 0;

        va_start(vaMarker, pcBusCycle);
        while (pcBusCycle)
        {
			if (iCount > 10)
			{
				LOG_ERROR("Bus Cycles have no terminal NULL.");
				return NULL;
			}

			papcBusCycles->Add(pcBusCycle);

			iCount++;
			pcBusCycle = va_arg(vaMarker, CBusCycle*);
        }
        va_end(vaMarker);
    }

	pcInstructionCycles = NewMalloc<CInstructionCycles>();
	pcInstructionCycles->Init(eAddressingMode, papcBusCycles);

    return pcInstructionCycles;
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
			if (iCount > 10)
			{
				LOG_ERROR("Operations have no terminal NULL.");
				return NULL;
			}

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


OPERATION(CInternalFirst,				Opcode)
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


CExecuteIf16Bit* E16Bit(CW65C816Func fConsumer, EWidthFromRegister eWidth, bool bInitialSide)
{
	CExecuteIf16Bit* pcType;

	pcType = NewMalloc<CExecuteIf16Bit>();
	pcType->Init(fConsumer, eWidth, bInitialSide);
	return pcType;
}


CExecuteIf8Bit* E8Bit(CW65C816Func fConsumer, EWidthFromRegister eWidth, bool bInitialSide)
{
	CExecuteIf8Bit* pcType;

	pcType = NewMalloc<CExecuteIf8Bit>();
	pcType->Init(fConsumer, eWidth, bInitialSide);
	return pcType;
}


CExecute* E(CW65C816Func fConsumer, bool bInitialSide)
{
	CExecute* pcType;

	pcType = NewMalloc<CExecute>();
	pcType->Init(fConsumer, bInitialSide);
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


CInstructionCycles* CreateFetchOpcodeCycles(void)
{
	CReadOpcode* pcReadOpcode;

	pcReadOpcode = NewMalloc<CReadOpcode>();
	pcReadOpcode->Init();

	//0*
	return InstructionCycles(AM_Opcode,
		BusCycle(	Address(PBR(), PC(), NULL), Operation(pcReadOpcode, PC_inc(), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//1a
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),			Operation(Read_DataLow(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DBR(), AA(), o(1), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//1a
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),			Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DBR(), AA(), o(1), NULL),	Operation(Write_DataHigh(), DONE16Bit(eWidth), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteJMPCycles(void)
{
	//1b
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCH(), PC_inc(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteJSRCycles(void)
{
	//1c
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_PCH(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_PCL(), SP_dec(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteRMWCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//1d
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAL(RMW), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAH(RMW), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),			Operation(Read_DataLow(RMW), NULL)),
		BusCycle(	Address(DBR(), AA(), o(1), NULL),	Operation(Read_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DBR(), AA(), o(1), NULL),	Operation(IO(RMW), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(DBR(), AA(), o(1), NULL),	Operation(NoteThree(), Write_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),			Operation(NoteThree(), Write_DataLow(RMW), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteIndexedIndirectWithXJMPCycles(void)
{
	//2a
	return InstructionCycles(AM_AbsoluteIndexedIndirectWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), AA(), X(), NULL),		Operation(Read_NewPCL(), NULL)),
		BusCycle(	Address(PBR(), AA(), X(), o(1), NULL),	Operation(Read_NewPCH(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteIndexedIndirectWithXJSRCycles(void)
{
	//2b
	return InstructionCycles(AM_AbsoluteIndexedIndirectWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(S(), NULL),						Operation(Write_PCH(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),						Operation(Write_PCL(), SP_dec(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAH(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), AA(), X(), NULL),		Operation(Read_NewPCL(), NULL)),
		BusCycle(	Address(PBR(), AA(), X(), o(1), NULL),	Operation(Read_NewPCH(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteIndirectJMLCycles(void)
{
	//3a
	return InstructionCycles(AM_AbsoluteIndirectLong,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(AA(), NULL),			Operation(Read_NewPCL(), NULL)),
		BusCycle(	Address(AA(), o(1), NULL),		Operation(Read_NewPCH(), NULL)),
		BusCycle(	Address(AA(), o(2), NULL),		Operation(Read_NewPBR(), PC_e(Address(New_PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteIndirectJMPCycles(void)
{
	//3b
	return InstructionCycles(AM_AbsoluteIndirect,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(AA(), NULL),			Operation(Read_NewPCL(), NULL)),
		BusCycle(	Address(AA(), o(1), NULL),		Operation(Read_NewPCH(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteLongWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//4a
	return InstructionCycles(AM_AbsoluteLong,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAB(), PC_inc(), NULL)),
		BusCycle(	Address(AAB(), AA(), NULL),			Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), AA(), o(1), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteLongCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//4a
	return InstructionCycles(AM_AbsoluteLong,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_AAB(), PC_inc(), NULL)),
		BusCycle(	Address(AAB(), AA(), NULL),			Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), AA(), o(1), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteLongJMLCycles(void)
{
	//4b
	return InstructionCycles(AM_AbsoluteLong,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPBR(), PC_inc(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteLongJSLCycles(void)
{
	//4c
	return InstructionCycles(AM_AbsoluteLong,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCH(), PC_inc(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_PBR(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPBR(), PC_inc(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_PCH(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_PCL(), SP_dec(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteLongIndexedWithXCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//5
	return InstructionCycles(AM_AbsoluteLongIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAB(), PC_inc(), NULL)),
		BusCycle(	Address(AAB(), AA(), X(), NULL),		Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), AA(), X(), o(1), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteLongIndexedWithXWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//5
	return InstructionCycles(AM_AbsoluteLongIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAB(), PC_inc(), NULL)),
		BusCycle(	Address(AAB(), AA(), X(), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), AA(), X(), o(1), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteIndexedWithXWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//6a
	return InstructionCycles(AM_AbsoluteIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AAH(), AAL_XL(), NULL),	Operation(IO(), NoteFourX(false), NULL)),
		BusCycle(	Address(DBR(), AA(), X(), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), AA(), X(), o(1), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteIndexedWithXCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//6a
	return InstructionCycles(AM_AbsoluteIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AAH(), AAL_XL(), NULL),	Operation(IO(), NoteFourX(true), NULL)),
		BusCycle(	Address(DBR(), AA(), X(), NULL),		Operation(Read_DataLow(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DBR(), AA(), X(), o(1), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteIndexedWithXRMWCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//6b
	return InstructionCycles(AM_AbsoluteIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAL(RMW), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAH(RMW), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AAH(), AAL_XL(), NULL),	Operation(IO(RMW), NULL)),
		BusCycle(	Address(DBR(), AA(), X(), NULL),		Operation(Read_DataLow(RMW), NULL)),
		BusCycle(	Address(DBR(), AA(), X(), o(1), NULL),	Operation(Read_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DBR(), AA(), X(), o(1), NULL),	Operation(IO(RMW), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(DBR(), AA(), X(), o(1), NULL),	Operation(NoteThree(), Write_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DBR(), AA(), X(), NULL),		Operation(NoteThree(), Write_DataLow(RMW), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//7
	return InstructionCycles(AM_AbsoluteIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AAH(), AAL_YL(), NULL),	Operation(IO(), NoteFourY(false), NULL)),
		BusCycle(	Address(DBR(), AA(), Y(), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), AA(), Y(), o(1), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateAbsoluteIndexedWithYCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//7
	return InstructionCycles(AM_AbsoluteIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AAH(), AAL_YL(), NULL),	Operation(IO(), NoteFourY(true), NULL)),
		BusCycle(	Address(DBR(), AA(), Y(), NULL),		Operation(Read_DataLow(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DBR(), AA(), Y(), o(1), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CInstructionCycles* CreateAccumulatorCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//8
	return InstructionCycles(AM_Accumulator,
		BusCycle(	Address(PBR(), PC(), NULL),	Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),	Operation(IO(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateBlockMoveCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//9a & 9b
	return InstructionCycles(AM_BlockMove,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_DBR(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAB(), PC_inc(), NULL)),
		BusCycle(	Address(AAB(), X(), NULL),		Operation(Read_DataLow(), NULL)),
		BusCycle(	Address(DBR(), Y(), NULL),		Operation(Write_DataLow(), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(DBR(), Y(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(DBR(), Y(), NULL),		Operation(IO(), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//10a
	return InstructionCycles(AM_Direct,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(Read_DataLow(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//10a
	return InstructionCycles(AM_Direct,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),	Operation(Write_DataHigh(), DONE16Bit(eWidth), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectRMWCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//10b
	return InstructionCycles(AM_Direct,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(RMW), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(Read_DataLow(RMW), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),	Operation(Read_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),	Operation(IO(RMW), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),	Operation(NoteThree(), Write_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(NoteThree(), Write_DataLow(RMW), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectIndexedIndirectWithXCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//11
	return InstructionCycles(AM_DirectIndexedIndirectWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(DP(), D0(), X(), NULL),			Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), X(), o(1), NULL),	Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),				Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), AA(), o(1), NULL),		Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectIndexedIndirectWithXWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//11
	return InstructionCycles(AM_DirectIndexedIndirectWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(DP(), D0(), X(), NULL),			Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), X(), o(1), NULL),	Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),				Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), AA(), o(1), NULL),		Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectIndirectCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//12
	return InstructionCycles(AM_DirectIndirect,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),	Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),			Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), AA(), o(1), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectIndirectWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//12
	return InstructionCycles(AM_DirectIndirect,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),	Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),			Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), AA(), o(1), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectIndirectIndexedWithYCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//13
	return InstructionCycles(AM_DirectIndirectIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),							Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),					Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DBR(), AAH(), AAL_YL(), NULL),				Operation(IO(), NoteFourY(true), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), Y(), NULL), NULL),		Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), Y(), o(1), NULL),	NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectIndirectIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//13
	return InstructionCycles(AM_DirectIndirectIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),				Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),		Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DBR(), AAH(), AAL_YL(), NULL),	Operation(IO(), NoteFourY(false), NULL)),
		BusCycle(	Address(DBR(), AA(), Y(), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), AA(), Y(), o(1), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectIndirectLongIndexedWithYCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//14
	return InstructionCycles(AM_DirectIndirectLongIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),				Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),		Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DP(), D0(), o(2), NULL),		Operation(Read_AAB(), NULL)),
		BusCycle(	Address(AAB(), AA(), Y(), NULL),		Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), AA(), Y(), o(1), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectIndirectLongIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//14
	return InstructionCycles(AM_DirectIndirectLongIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),				Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),		Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DP(), D0(), o(2), NULL),		Operation(Read_AAB(), NULL)),
		BusCycle(	Address(AAB(), AA(), Y(), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), AA(), Y(), o(1), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectIndirectLongCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//15
	return InstructionCycles(AM_DirectIndirectLong,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),	Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DP(), D0(), o(2), NULL),	Operation(Read_AAB(), NULL)),
		BusCycle(	Address(AAB(), AA(), NULL),			Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), AA(), o(1), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectIndirectLongWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//15
	return InstructionCycles(AM_DirectIndirectLong,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),	Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DP(), D0(), o(2), NULL),	Operation(Read_AAB(), NULL)),
		BusCycle(	Address(AAB(), AA(), NULL),			Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), AA(), o(1), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectIndexedWithXCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//16a
	return InstructionCycles(AM_DirectIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(DP(), D0(), X(), NULL),			Operation(Read_DataLow(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DP(), D0(), X(), o(1), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectIndexedWithXWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//16a
	return InstructionCycles(AM_DirectIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(DP(), D0(), X(), NULL),			Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DP(), D0(), X(), o(1), NULL),		Operation(Write_DataHigh(), DONE16Bit(eWidth), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectIndexedWithXRMWCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//16b
	return InstructionCycles(AM_DirectIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(RMW), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(RMW), NoteTwo(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(RMW), NULL)),
		BusCycle(	Address(DP(), D0(), X(), NULL),			Operation(Read_DataLow(RMW), NULL)),
		BusCycle(	Address(DP(), D0(), X(), o(1), NULL),		Operation(Read_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DP(), D0(), X(), o(1), NULL),		Operation(IO(RMW), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(DP(), D0(), X(), o(1), NULL),		Operation(NoteThree(), Write_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DP(), D0(), X(), NULL),			Operation(NoteThree(), Write_DataLow(RMW), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectIndexedWithYCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//17
	return InstructionCycles(AM_DirectIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(DP(), D0(), Y(), NULL),			Operation(Read_DataLow(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DP(), D0(), Y(), o(1), NULL),		Operation(Read_DataHigh(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CInstructionCycles* CreateDirectIndexedWithYWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//17
	return InstructionCycles(AM_DirectIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(DP(), D0(), Y(), NULL),			Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DP(), D0(), Y(), o(1), NULL),		Operation(Write_DataHigh(), DONE16Bit(eWidth), NULL)),
		NULL);
}


CInstructionCycles* CreateImmediateREPSEPCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//18
	return InstructionCycles(AM_Immediate,
		BusCycle(	Address(PBR(), PC(), NULL),	Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),	Operation(PC_inc(), Read_DataLow(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),	Operation(Read_DataHigh(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateImmediateCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//18
	return InstructionCycles(AM_Immediate,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(PC_inc(), Read_DataLow(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(PC_inc(), Read_DataHigh(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CInstructionCycles* CreateImpliedCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//19a
	return InstructionCycles(AM_Implied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateWDMImpliedCycles(CW65C816Func fOperation, bool bInitialSide)
{
	return InstructionCycles(AM_Implied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(PC_inc(), IO(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateImpliedXBACycles(CW65C816Func fOperation, bool bInitialSide)
{
	//19b
	return InstructionCycles(AM_Implied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStopTheClockCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//19c
	return InstructionCycles(AM_StopTheClock,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateWaitForInterruptCycles()
{
	//19d
	return InstructionCycles(AM_WaitForInterrupt,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(WaitOperation(), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateRelativeShortCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//20
	return InstructionCycles(AM_Relative,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_DataLow(), PC_inc(), E(fOperation, bInitialSide), NULL)),  //Done if branch not taken
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NoteSix(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(PC_e(Address(PBR(), PC(), SignedDataLow(), NULL)), IO(), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateRelativeLongCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//21
	return InstructionCycles(AM_RelativeLong,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_DataLow(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_DataHigh(), PC_inc(), E(fOperation, bInitialSide), NULL)),  //Done if branch not taken
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(PC_e(Address(PBR(), PC(), SignedData(), NULL)), IO(), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackHardwareInterruptCycles(CInterruptVector* pcInterruptVector, CW65C816Func fOperation, bool bInitialSide)
{
	//22a
	return InstructionCycles(AM_StackInterruptHardware,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(IO(true, true, true), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PBR(), SP_dec(), PBR_e(0), NoteSeven(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PCH(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PCL(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PS(), SP_dec(), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(VA(pcInterruptVector), NULL),			Operation(Read_AAVL(), NULL)),
		BusCycle(	Address(VA(pcInterruptVector), o(1), NULL),		Operation(Read_AAVH(), PC_e(Address(AA(), NULL)), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackAbortInterruptCycles(CInterruptVector* pcInterruptVector, CW65C816Func fOperation, bool bInitialSide)
{
	//22a
	return InstructionCycles(AM_StackInterruptHardware,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(IO(true, true, true), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(IO(), RestoreAbortValues(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PBR(), SP_dec(), PBR_e(0), NoteSeven(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PCH(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PCL(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PS(), SP_dec(), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(VA(pcInterruptVector), NULL),			Operation(Read_AAVL(), NULL)),
		BusCycle(	Address(VA(pcInterruptVector), o(1), NULL),		Operation(Read_AAVH(), PC_e(Address(AA(), NULL)), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackResetCycles(CInterruptVector* pcInterruptVector, CW65C816Func fOperation, bool bInitialSide)
{
	//22a
	return InstructionCycles(AM_StackInterruptHardware,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(IO(true, true, true), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(IO(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(IO(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(IO(), SP_dec(), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(VA(pcInterruptVector), NULL),			Operation(Read_AAVL(), NULL)),
		BusCycle(	Address(VA(pcInterruptVector), o(1), NULL),		Operation(Read_AAVH(), PC_e(Address(PBR(), AA(), NULL)), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackPullCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//22b
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), o(1), NULL),		Operation(Read_DataLow(), SP_inc(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(S(), o(1), NULL),		Operation(Read_DataHigh(), SP_inc(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CInstructionCycles* CreateStackPLBCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22b
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), o(1), NULL),		Operation(SP_inc(), Read_DataLow(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackPLDCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22b
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), o(1), NULL),		Operation(Read_DataLow(), SP_inc(), NULL)),
		BusCycle(	Address(S(), o(1), NULL),		Operation(Read_DataHigh(), SP_inc(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackPLPCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22b
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), o(1), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), o(1), NULL),		Operation(Read_DataLow(), SP_inc(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackImpliedPHPCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22c
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(E(fOperation, bInitialSide), Write_DataLow(), SP_dec(), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackPushCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//22c
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_DataHigh(), SP_dec(), NoteOne(eWidth), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_DataLow(), SP_dec(), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackPHBCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22c
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(E(fOperation, bInitialSide), Write_DataLow(), SP_dec(), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackPHDCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22c
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(E(fOperation, bInitialSide), Write_DataHigh(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_DataLow(), SP_dec(), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackPHKCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22c
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(E(fOperation, bInitialSide), Write_DataLow(), SP_dec(), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackPEACycles()
{
	//22d
	return InstructionCycles(AM_StackImmediate,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAH(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_AAH(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_AAL(), SP_dec(), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackPEICycles()
{
	//22e
	return InstructionCycles(AM_StackDirectIndirect,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(), D0(), NULL),			Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(), D0(), o(1), NULL),	Operation(Read_AAH(), NULL)),
		BusCycle(	Address(S(), NULL),					Operation(Write_AAH(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),					Operation(Write_AAL(), SP_dec(), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackPERCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22f
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_DataLow(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_DataHigh(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(E(fOperation, bInitialSide), Write_DataHigh(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_DataLow(), SP_dec(), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackRTICycles()
{
	//22g
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), o(1), NULL),		Operation(Read_PS(), SP_inc(), NULL)),  //Processor status
		BusCycle(	Address(S(), o(1), NULL),		Operation(Read_NewPCL(), SP_inc(), NULL)),
		BusCycle(	Address(S(), o(1), NULL),		Operation(Read_NewPCH(), SP_inc(), NULL)),
		BusCycle(	Address(S(), o(1), NULL),		Operation(Read_NewPBR(), SP_inc(), PC_e(Address(New_PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackRTSCycles()
{
	//22h
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), o(1), NULL),		Operation(Read_NewPCL(), SP_inc(), NULL)),
		BusCycle(	Address(S(), o(1), NULL),		Operation(Read_NewPCH(), SP_inc(), NULL)),
		BusCycle(	Address(S(), NULL), 			Operation(IO(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackRTLCycles()
{
	//22i
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), o(1), NULL),		Operation(Read_NewPCL(), SP_inc(), NULL)),
		BusCycle(	Address(S(), o(1), NULL),		Operation(Read_NewPCH(), SP_inc(), NULL)),
		BusCycle(	Address(S(), o(1), NULL),		Operation(Read_NewPBR(), SP_inc(), PC_e(Address(New_PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackSoftwareInterruptCycles(CInterruptVector* pcInterruptVector, CW65C816Func fOperation, bool bInitialSide)
{
	//22j
	return InstructionCycles(AM_StackInterruptSoftware,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(IO(true, false, true), PC_inc(), NULL)),  //Read and ignore the signature byte.
		BusCycle(	Address(S(), NULL),								Operation(Write_PBR(), SP_dec(), PBR_e(0), NoteSeven(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PCH(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PCL(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PS(), SP_dec(), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(VA(pcInterruptVector), NULL),			Operation(Read_AAL(), NULL)),
		BusCycle(	Address(VA(pcInterruptVector), o(1), NULL),		Operation(Read_AAH(), PC_e(Address(PBR(), AA(), NULL)), DONE(), NULL)),
		NULL);
}


CInstructionCycles* CreateStackRelativeCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//23
	return InstructionCycles(AM_StackRelative,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(IO(), NULL)),
		BusCycle(	Address(S(), D0(), NULL),			Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(S(), D0(), o(1), NULL),		Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateStackRelativeWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//23
	return InstructionCycles(AM_StackRelative,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(IO(), NULL)),
		BusCycle(	Address(S(), D0(), NULL),			Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(S(), D0(), o(1), NULL),		Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateStackRelativeIndirectIndexedWithYCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//24
	return InstructionCycles(AM_StackRelativeIndirectIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(S(), D0(), NULL),				Operation(Read_AAL(), NULL)),
		BusCycle(	Address(S(), D0(), o(1), NULL),			Operation(Read_AAH(), NULL)),
		BusCycle(	Address(S(), D0(), o(1), NULL),			Operation(IO(), NULL)),
		BusCycle(	Address(DBR(), AA(), Y(), NULL),		Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), AA(), Y(), o(1), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CInstructionCycles* CreateStackRelativeIndirectIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//24
	return InstructionCycles(AM_StackRelativeIndirectIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(S(), D0(), NULL),				Operation(Read_AAL(), NULL)),
		BusCycle(	Address(S(), D0(), o(1), NULL),			Operation(Read_AAH(), NULL)),
		BusCycle(	Address(S(), D0(), o(1), NULL),			Operation(IO(), NULL)),
		BusCycle(	Address(DBR(), AA(), Y(), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), AA(), Y(), o(1), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}

