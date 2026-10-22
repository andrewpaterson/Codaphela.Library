#include "W65CompoundAddress.h"
#include "W65InstructionCycleBuilder.h"


NotMemoryLock RMW = false;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65BusCycle* BusCycle(CW65AddressOffsetArray* papcAddressOffsets, CW65OperationArray* papcOperations)
{
    CW65BusCycle* pcBusCycle;

    pcBusCycle = NewMalloc<CW65BusCycle>();
    pcBusCycle->Init(papcAddressOffsets, papcOperations);
    return pcBusCycle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65InstructionCycles* InstructionCycles(EAddressingMode eAddressingMode, CW65BusCycle* pcBusCycle, ...)
{
    va_list					vaMarker;
    CW65InstructionCycles*		pcInstructionCycles;
	size		            iCount;
	CW65BusCycleArray*			papcBusCycles;

	papcBusCycles = NewMalloc<CW65BusCycleArray>();
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
			pcBusCycle = va_arg(vaMarker, CW65BusCycle*);
        }
        va_end(vaMarker);
    }

	pcInstructionCycles = NewMalloc<CW65InstructionCycles>();
	pcInstructionCycles->Init(eAddressingMode, papcBusCycles);

    return pcInstructionCycles;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65OperationArray* Operation(CW65Operation* pcOperation, ...)
{
	va_list		        vaMarker;
	size		        iCount;
	CW65OperationArray*	papcOperations;

	papcOperations = NewMalloc<CW65OperationArray>();
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
			pcOperation = va_arg(vaMarker, CW65Operation*);
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


ADDRESS(CW65ProgramCounter,				PC)
ADDRESS(CW65StackPointer,					S)
ADDRESS(CW65ProgramBank,					PBR)
ADDRESS(CW65AbsoluteAddress,				AA)
ADDRESS(CW65XIndex,						X)
ADDRESS(CW65YIndex,						Y)
ADDRESS(CW65DataBank,						DBR)
ADDRESS(CW65AbsoluteAddressHigh,			AAH)
ADDRESS(CW65DirectOffset,					DirectOffset)
ADDRESS(CW65DirectPage,					DP)
ADDRESS(CW65NewProgramCounter,				New_PC)
ADDRESS(CW65NewProgramBank,				New_PBR)
ADDRESS(CW65AddressBank,					AAB)
ADDRESS(CW65DirectOffset,					D0)
ADDRESS(CW65AbsoluteAddressLowPlusXLow,	AAL_XL)
ADDRESS(CW65AbsoluteAddressLowPlusYLow,	AAL_YL)
ADDRESS(CW65SignedData,					SignedData)
ADDRESS(CW65SignedDataLow,					SignedDataLow)


OPERATION(CW65InternalFirst,				Opcode)
OPERATION(CW65IncrementProgramCounter,		PC_inc)
OPERATION(CW65DecrementStackPointer,		SP_dec)
OPERATION(CW65IncrementStackPointer,		SP_inc)
OPERATION(CW65WriteProgramBank,			Write_PBR)
OPERATION(CW65WriteProgramCounterLow,		Write_PCL)
OPERATION(CW65WriteProgramCounterHigh,		Write_PCH)
OPERATION(CW65WriteProcessorStatus,		Write_PS)
OPERATION(CW65ReadProcessorStatus,			Read_PS)
OPERATION(CW65DoneInstruction,				DONE)
OPERATION(CW65DoneInstructionIfEmulation,	DONE_Emulation)
OPERATION(CW65WriteAbsoluteAddressLow,		Write_AAL)
OPERATION(CW65WriteAbsoluteAddressHigh,	Write_AAH)
OPERATION(CW65NoteTwo,						NoteTwo)
OPERATION(CW65NoteThree,					NoteThree)
OPERATION(CW65NoteSix,						NoteSix)
OPERATION(CW65NoteSeven,					NoteSeven)
OPERATION(CW65WaitOperation,				WaitOperation)
OPERATION(CW65RestoreAbortValues,			RestoreAbortValues)
OPERATION(CW65NoteDecimal,					NoteDecimal)
OPERATION(CW65DoneInstructionIfBinary,		DONE_Binary)
OPERATION(CW65DoneInstructionIfSamePage,	DONE_SamePage)


CW65DirectPage* DPL(void)
{
	CW65DirectPage*	pcType;

	pcType = NewMalloc<CW65DirectPage>();
	pcType->Init(false);
	return pcType;
}


CW65StackPointer* StackPointer(EStackPointerMode eMode)
{
	CW65StackPointer*	pcType;

	pcType = NewMalloc<CW65StackPointer>();
	pcType->Init(eMode);
	return pcType;
}


CW65StackPointer* S1(void)
{
	return StackPointer(SPM_PagePlusOne);
}


CW65StackPointer* S16(void)
{
	return StackPointer(SPM_Linear);
}


CW65ConstantOffset* oM(int32 iOffset)
{
	CW65ConstantOffset*	pcType;

	pcType = NewMalloc<CW65ConstantOffset>();
	pcType->Init(iOffset, true);
	return pcType;
}


CW65ConstantOffset* o(int32 iOffset)
{
	CW65ConstantOffset*	pcType;

	pcType = NewMalloc<CW65ConstantOffset>();
	pcType->Init(iOffset); 
	return pcType;
}


CW65WriteProcessorStatus* Write_PSB(void)
{
	CW65WriteProcessorStatus*	pcType;

	pcType = NewMalloc<CW65WriteProcessorStatus>();
	pcType->Init(true);
	return pcType;
}


CW65ConstantOffset* Constant(uint16 uiAddress)
{
	CW65ConstantOffset*	pcType;

	pcType = NewMalloc<CW65ConstantOffset>();
	pcType->InitAddress(uiAddress);
	return pcType;
}


CW65BranchAddress* BranchAddress(EBranchAddress eAddress)
{
	CW65BranchAddress*	pcType;

	pcType = NewMalloc<CW65BranchAddress>();
	pcType->Init(eAddress);
	return pcType;
}


CW65BranchAddress* BranchOrigin(void)
{
	return BranchAddress(BA_Origin);
}


CW65BranchAddress* BranchPageCross(void)
{
	return BranchAddress(BA_PageCross);
}


CW65WriteDataHigh* Write_DataHigh(NotMemoryLock bNotMemoryLock)
{
	CW65WriteDataHigh*		pcType;

	pcType = NewMalloc<CW65WriteDataHigh>();
	pcType->Init(bNotMemoryLock);
	return pcType;
}


CW65WriteDataHigh* Write_DataHigh(void)
{
	return Write_DataHigh(true);
}


CW65InterruptAddress* VA(CW65InterruptVector* pcInterruptVector)
{
	CW65InterruptAddress* pcType;

	pcType = NewMalloc<CW65InterruptAddress>();
	pcType->Init(pcInterruptVector);
	return pcType;
}


CW65InternalOperation* IO(void)
{
	CW65InternalOperation* pcType;

	pcType = NewMalloc<CW65InternalOperation>();
	pcType->Init(true);
	return pcType;
}


CW65InternalOperation* IO(bool bValidProgramAddress, bool bValidDataAddress, NotMemoryLock bNotMemoryLock)
{
	CW65InternalOperation* pcType;

	pcType = NewMalloc<CW65InternalOperation>();
	pcType->Init(bValidProgramAddress, bValidDataAddress, bNotMemoryLock);
	return pcType;
}


CW65ReadAbsoluteAddressHigh* Read_AAH(NotMemoryLock bNotMemoryLock)
{
	CW65ReadAbsoluteAddressHigh* pcType;

	pcType = NewMalloc<CW65ReadAbsoluteAddressHigh>();
	pcType->Init(bNotMemoryLock, true);
	return pcType;
}


CW65ReadAbsoluteAddressHigh* Read_AAH(void)
{
	return Read_AAH(true);
}


CW65ReadAbsoluteAddressLow* Read_AAL(void)
{
	CW65ReadAbsoluteAddressLow* pcType;

	pcType = NewMalloc<CW65ReadAbsoluteAddressLow>();
	pcType->Init(true, true);
	return pcType;
}


CW65ReadAbsoluteAddressLow* Read_AAVL(void)
{
	CW65ReadAbsoluteAddressLow* pcType;

	pcType = NewMalloc<CW65ReadAbsoluteAddressLow>();
	pcType->Init(true, false);
	return pcType;
}


CW65ReadAbsoluteAddressHigh* Read_AAVH(void)
{
	CW65ReadAbsoluteAddressHigh* pcType;

	pcType = NewMalloc<CW65ReadAbsoluteAddressHigh>();
	pcType->Init(true, false);
	return pcType;
}


CW65ReadDirectOffset* Read_D0(void)
{
	CW65ReadDirectOffset* pcType;

	pcType = NewMalloc<CW65ReadDirectOffset>();
	pcType->Init(true);
	return pcType;
}


CW65ReadProgramBank* Read_NewPBR(void)
{
	CW65ReadProgramBank* pcType;

	pcType = NewMalloc<CW65ReadProgramBank>();
	pcType->Init(true);
	return pcType;
}


CW65ReadProgramCounterHigh* Read_NewPCH(void)
{
	CW65ReadProgramCounterHigh* pcType;

	pcType = NewMalloc<CW65ReadProgramCounterHigh>();
	pcType->Init(true);
	return pcType;
}


CW65ReadProgramCounterLow* Read_NewPCL(void)
{
	CW65ReadProgramCounterLow* pcType;

	pcType = NewMalloc<CW65ReadProgramCounterLow>();
	pcType->Init(true);
	return pcType;
}


CW65SetProgramCounter* PC_e(CW65AddressOffsetArray* papcAddressOffsets)
{
	CW65SetProgramCounter* pcType;

	pcType = NewMalloc<CW65SetProgramCounter>();
	pcType->Init(papcAddressOffsets);
	return pcType;
}


CW65ReadAbsoluteAddressBank* Read_AAB(void)
{
	CW65ReadAbsoluteAddressBank* pcType;

	pcType = NewMalloc<CW65ReadAbsoluteAddressBank>();
	pcType->Init(true);
	return pcType;
}


CW65ReadDataBank* Read_DBR(void)
{
	CW65ReadDataBank* pcType;

	pcType = NewMalloc<CW65ReadDataBank>();
	pcType->Init(true);
	return pcType;
}


CW65ReadAbsoluteAddressLow* Read_AAL(NotMemoryLock bNotMemoryLock)
{
	CW65ReadAbsoluteAddressLow* pcType;

	pcType = NewMalloc<CW65ReadAbsoluteAddressLow>();
	pcType->Init(bNotMemoryLock, true);
	return pcType;
}


CW65InternalOperation* IO(NotMemoryLock bNotMemoryLock)
{
	CW65InternalOperation* pcType;

	pcType = NewMalloc<CW65InternalOperation>();
	pcType->Init(bNotMemoryLock);
	return pcType;
}


CW65ReadDataHigh* Read_DataHigh(NotMemoryLock bNotMemoryLock)
{
	CW65ReadDataHigh* pcType;

	pcType = NewMalloc<CW65ReadDataHigh>();
	pcType->Init(bNotMemoryLock);
	return pcType;
}


CW65ReadDataHigh* Read_DataHigh(void)
{
	return Read_DataHigh(true);
}


CW65ReadDataLow* Read_DataLow(NotMemoryLock bNotMemoryLock)
{
	CW65ReadDataLow* pcType;

	pcType = NewMalloc<CW65ReadDataLow>();
	pcType->Init(bNotMemoryLock);
	return pcType;
}

CW65ReadDataLow* Read_DataLow(void)
{
	return Read_DataLow(true);
}


CW65WriteDataLow* Write_DataLow(NotMemoryLock bNotMemoryLock)
{
	CW65WriteDataLow* pcType;

	pcType = NewMalloc<CW65WriteDataLow>();
	pcType->Init(bNotMemoryLock);
	return pcType;
}


CW65WriteDataLow* Write_DataLow()
{
	return Write_DataLow(true);
}


CW65ReadDirectOffset* Read_D0(NotMemoryLock bNotMemoryLock)
{
	CW65ReadDirectOffset* pcType;

	pcType = NewMalloc<CW65ReadDirectOffset>();
	pcType->Init(bNotMemoryLock);
	return pcType;
}


CW65SetProgramBank* PBR_e(uint8 uiBank)
{
	CW65SetProgramBank* pcType;

	pcType = NewMalloc<CW65SetProgramBank>();
	pcType->Init(uiBank);
	return pcType;
}


CW65ExecuteIf16Bit* E16Bit(CW65CFunc fConsumer, EWidthFromRegister eWidth, bool bInitialSide)
{
	CW65ExecuteIf16Bit* pcType;

	pcType = NewMalloc<CW65ExecuteIf16Bit>();
	pcType->Init(fConsumer, eWidth, bInitialSide);
	return pcType;
}


CW65ExecuteIf8Bit* E8Bit(CW65CFunc fConsumer, EWidthFromRegister eWidth, bool bInitialSide)
{
	CW65ExecuteIf8Bit* pcType;

	pcType = NewMalloc<CW65ExecuteIf8Bit>();
	pcType->Init(fConsumer, eWidth, bInitialSide);
	return pcType;
}


CW65Execute* E(CW65CFunc fConsumer, bool bInitialSide)
{
	CW65Execute* pcType;

	pcType = NewMalloc<CW65Execute>();
	pcType->Init(fConsumer, bInitialSide);
	return pcType;
}


CW65DoneInstructionIf8Bit* DONE8Bit(EWidthFromRegister eWidth)
{
	CW65DoneInstructionIf8Bit* pcType;

	pcType = NewMalloc<CW65DoneInstructionIf8Bit>();
	pcType->Init(eWidth);
	return pcType;
}


CW65DoneInstructionIf16Bit* DONE16Bit(EWidthFromRegister eWidth)
{
	CW65DoneInstructionIf16Bit* pcType;

	pcType = NewMalloc<CW65DoneInstructionIf16Bit>();
	pcType->Init(eWidth);
	return pcType;
}


CW65NoteOne* NoteOne(EWidthFromRegister eWidth)
{
	CW65NoteOne* pcType;

	pcType = NewMalloc<CW65NoteOne>();
	pcType->Init(eWidth);
	return pcType;
}


CW65NoteFourX* NoteFourX(bool bNextWillRead)
{
	CW65NoteFourX* pcType;

	pcType = NewMalloc<CW65NoteFourX>();
	pcType->Init(bNextWillRead);
	return pcType;
}


CW65NoteFourY* NoteFourY(bool bNextWillRead)
{
	CW65NoteFourY* pcType;

	pcType = NewMalloc<CW65NoteFourY>();
	pcType->Init(bNextWillRead);
	return pcType;
}


CW65InstructionCycles* CreateFetchOpcodeCycles(void)
{
	CW65ReadOpcode* pcReadOpcode;

	pcReadOpcode = NewMalloc<CW65ReadOpcode>();
	pcReadOpcode->Init();

	//0*
	return InstructionCycles(AM_Opcode,
		BusCycle(	Address(PBR(), PC(), NULL), Operation(pcReadOpcode, PC_inc(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackHardwareInterruptCycles(CW65InterruptVector* pcInterruptVector, CW65CFunc fOperation, bool bInitialSide)
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


CW65InstructionCycles* CreateStackResetCycles(CW65InterruptVector* pcInterruptVector, CW65CFunc fOperation, bool bInitialSide)
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

