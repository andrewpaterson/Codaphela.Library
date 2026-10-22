#include "CompoundAddress.h"
#include "InstructionCycleBuilder.h"


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
OPERATION(CDoneInstructionIfEmulation,	DONE_Emulation)
OPERATION(CWriteAbsoluteAddressLow,		Write_AAL)
OPERATION(CWriteAbsoluteAddressHigh,	Write_AAH)
OPERATION(CNoteTwo,						NoteTwo)
OPERATION(CNoteThree,					NoteThree)
OPERATION(CNoteSix,						NoteSix)
OPERATION(CNoteSeven,					NoteSeven)
OPERATION(CWaitOperation,				WaitOperation)
OPERATION(CRestoreAbortValues,			RestoreAbortValues)
OPERATION(CNoteDecimal,					NoteDecimal)
OPERATION(CDoneInstructionIfBinary,		DONE_Binary)
OPERATION(CDoneInstructionIfSamePage,	DONE_SamePage)


CDirectPage* DPL(void)
{
	CDirectPage*	pcType;

	pcType = NewMalloc<CDirectPage>();
	pcType->Init(false);
	return pcType;
}


CStackPointer* StackPointer(EStackPointerMode eMode)
{
	CStackPointer*	pcType;

	pcType = NewMalloc<CStackPointer>();
	pcType->Init(eMode);
	return pcType;
}


CStackPointer* S1(void)
{
	return StackPointer(SPM_PagePlusOne);
}


CStackPointer* S16(void)
{
	return StackPointer(SPM_Linear);
}


CConstantOffset* oM(int32 iOffset)
{
	CConstantOffset*	pcType;

	pcType = NewMalloc<CConstantOffset>();
	pcType->Init(iOffset, true);
	return pcType;
}


CConstantOffset* o(int32 iOffset)
{
	CConstantOffset*	pcType;

	pcType = NewMalloc<CConstantOffset>();
	pcType->Init(iOffset); 
	return pcType;
}


CWriteProcessorStatus* Write_PSB(void)
{
	CWriteProcessorStatus*	pcType;

	pcType = NewMalloc<CWriteProcessorStatus>();
	pcType->Init(true);
	return pcType;
}


CConstantOffset* Constant(uint16 uiAddress)
{
	CConstantOffset*	pcType;

	pcType = NewMalloc<CConstantOffset>();
	pcType->InitAddress(uiAddress);
	return pcType;
}


CBranchAddress* BranchAddress(EBranchAddress eAddress)
{
	CBranchAddress*	pcType;

	pcType = NewMalloc<CBranchAddress>();
	pcType->Init(eAddress);
	return pcType;
}


CBranchAddress* BranchOrigin(void)
{
	return BranchAddress(BA_Origin);
}


CBranchAddress* BranchPageCross(void)
{
	return BranchAddress(BA_PageCross);
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


CExecuteIf16Bit* E16Bit(CW65CFunc fConsumer, EWidthFromRegister eWidth, bool bInitialSide)
{
	CExecuteIf16Bit* pcType;

	pcType = NewMalloc<CExecuteIf16Bit>();
	pcType->Init(fConsumer, eWidth, bInitialSide);
	return pcType;
}


CExecuteIf8Bit* E8Bit(CW65CFunc fConsumer, EWidthFromRegister eWidth, bool bInitialSide)
{
	CExecuteIf8Bit* pcType;

	pcType = NewMalloc<CExecuteIf8Bit>();
	pcType->Init(fConsumer, eWidth, bInitialSide);
	return pcType;
}


CExecute* E(CW65CFunc fConsumer, bool bInitialSide)
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


CInstructionCycles* CreateStackHardwareInterruptCycles(CInterruptVector* pcInterruptVector, CW65CFunc fOperation, bool bInitialSide)
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


CInstructionCycles* CreateStackResetCycles(CInterruptVector* pcInterruptVector, CW65CFunc fOperation, bool bInitialSide)
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

