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

#include "InstructionCycles.h"


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

OPERATION(CInternalFirst,				OpCode)
OPERATION(CIncrementProgramCounter,	PC_inc)
OPERATION(CDecrementStackPointer,		SP_dec)
OPERATION(CIncrementStackPointer,		SP_inc)
OPERATION(CWriteProgramBank,			Write_PBR)
OPERATION(CReadProgramCounterHigh,	Read_NewPCH)
OPERATION(CSetProgramCounter,			PC_e)
OPERATION(CSetProgramCounter,			Write_PCL)
OPERATION(CWriteProgramCounterHigh,	Write_PCH)
OPERATION(CWriteProcessorStatus,		Write_PS)
OPERATION(CReadProcessorStatus,		Read_PS)
OPERATION(CDoneInstruction,			DONE)
OPERATION(CWriteAbsoluteAddressLow,	Write_AAL)
OPERATION(CWriteAbsoluteAddressHigh,	Write_AAH)


CConstantOffset* o(int32 iOffset)
{
	CConstantOffset*	pcType;

	pcType = NewMalloc<CConstantOffset>();
	pcType->Init(iOffset); 
	return pcType;
}


CWriteDataHigh* Write_DataHigh(bool bNotMemoryLock)
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

CReadAbsoluteAddressHigh* Read_AAH(bool bNotMemoryLock)
{
	CReadAbsoluteAddressHigh* pcType;

	pcType = NewMalloc<CReadAbsoluteAddressHigh>();
	pcType->Init(bNotMemoryLock, true);
	return pcType;
}

CReadAbsoluteAddressHigh* Read_AAH()
{
	return Read_AAH(true);
}

CReadProgramBank* Read_NewPBR()
{
	CReadProgramBank* pcType;

	pcType = NewMalloc<CReadProgramBank>();
	pcType->Init(true);
	return pcType;
}

CReadProgramCounterLow* Read_NewPCL()
{
	CReadProgramCounterLow* pcType;

	pcType = NewMalloc<CReadProgramCounterLow>();
	pcType->Init(true);
	return pcType;
}

CReadAbsoluteAddressLow* Read_AAL()
{
	return new CReadAbsoluteAddressLow(true, true);
}

CReadAbsoluteAddressLow* Read_AAVL()
{
	return new CReadAbsoluteAddressLow(true, false);
}

CReadAbsoluteAddressHigh* Read_AAVH()
{
	return new CReadAbsoluteAddressHigh(true, false);
}


CReadDirectOffset Read_D0()
{
	return new CReadDirectOffset(true);
}

CReadAbsoluteAddressBank Read_AAB()
{
	return new CReadAbsoluteAddressBank(true);
}

CReadDataBank Read_DBR()
{
	return new CReadDataBank(true);
}


CReadAbsoluteAddressLow Read_AAL(bool bNotMemoryLock)
{
	return new CReadAbsoluteAddressLow(bNotMemoryLock, true);
}

CInternalOperation IO(bool bNotMemoryLock)
{
	return new CInternalOperation(bNotMemoryLock);
}

CReadDataHigh Read_DataHigh()
{
	return Read_DataHigh(true);
}

CReadDataHigh Read_DataHigh(bool bNotMemoryLock)
{
	return new CReadDataHigh(bNotMemoryLock);
}

CReadDataLow Read_DataLow(bool bNotMemoryLock)
{
	return new CReadDataLow(bNotMemoryLock);
}

CReadDataLow Read_DataLow()
{
	return Read_DataLow(true);
}

CWriteDataLow Write_DataLow()
{
	return Write_DataLow(true);
}

CReadDirectOffset Read_D0(bool bNotMemoryLock)
{
	return new CReadDirectOffset(bNotMemoryLock);
}

CWriteDataLow Write_DataLow(bool bNotMemoryLock)
{
	return new CWriteDataLow(bNotMemoryLock);
}

CSetProgramBank PBR_e(int bank)
{
	return new CSetProgramBank(bank);
}


CExecuteIf16Bit E16Bit(CW65C816Func fConsumer, EWidthFromRegister eWidth)
{

	return new CExecuteIf16Bit(fConsumer, eWidth);
}

CExecuteIf8Bit E8Bit(CW65C816Func fConsumer, EWidthFromRegister eWidth)
{
	return new CExecuteIf8Bit(fConsumer, eWidth);
}

CExecute E(CW65C816Func fConsumer)
{
	return new CExecute(fConsumer);
}

class CInstructionCycleFactory
{
public:
	CInstructionCycles* createFetchOpCodeCycles()
	{
		CReadOpCode* pcReadOpCode;

		pcReadOpCode = NewMalloc<CReadOpCode>();
		pcReadOpCode->Init();

		//0*
		return InstructionCycles(AM_OpCode,
			BusCycle(Address(PBR(), PC()), Operation(pcReadOpCode, PC_inc())));
	}


	CInstructionCycles* createAbsoluteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
	{
		//1a
		return InstructionCycles(AM_Absolute,
			BusCycle(Address(PBR(), PC()), Operation(OpCode(), PC_inc())),
			BusCycle(Address(PBR(), PC()), Operation(Read_AAL(), PC_inc())),
			BusCycle(Address(PBR(), PC()), Operation(Read_AAH(), PC_inc())),
			BusCycle(Address(DBR(), AA()), Operation(Read_DataLow(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth))),
			BusCycle(Address(DBR(), AA(), o(1)), Operation(Read_DataHigh(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth))));
	}


	CInstructionCycles* createAbsoluteWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
	{
		//1a
		return InstructionCycles(AM_Absolute,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
			BusCycle(Address(DBR(), AA()), E(fOperation), Write_DataLow(), DONE8Bit(eWidth)),
			BusCycle(Address(DBR(), AA(), o(1)), Write_DataHigh(), DONE16Bit(eWidth)));
	}


	CInstructionCycles* createAbsoluteJMPCycles()
	{
		//1b
		return InstructionCycles(AM_Absolute,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_NewPCL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_NewPCH(), PC_inc(), PC_e(PBR(), New_PC()), DONE()));
	}


	CInstructionCycles* createAbsoluteJSRCycles()
	{
		//1c
		return InstructionCycles(AM_Absolute,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_NewPCL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_NewPCH(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S()), Write_PCH(), SP_dec()),
			BusCycle(Address(S()), Write_PCL(), SP_dec(), PC_e(PBR(), New_PC()), DONE()));
	}


	CInstructionCycles* createAbsoluteRMWCycles(CW65C816Func fOperation)
	{
		//1d
		return InstructionCycles(AM_Absolute,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(RMW), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAH(RMW), PC_inc()),
			BusCycle(Address(DBR(), AA()), Read_DataLow(RMW)),
			BusCycle(Address(DBR(), AA(), o(1)), Read_DataHigh(RMW), new NoteOne(M)),
			BusCycle(Address(DBR(), AA(), o(1)), IO(RMW), E(fOperation)),
			BusCycle(Address(DBR(), AA(), o(1)), new NoteThree(), Write_DataHigh(RMW), new NoteOne(M)),
			BusCycle(Address(DBR(), AA()), new NoteThree(), Write_DataLow(RMW), DONE()));
	}


	CInstructionCycles* createAbsoluteIndexedIndirectWithXJMPCycles()
	{
		//2a
		return InstructionCycles(AM_AbsoluteIndexedIndirectWithX,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(PBR(), AA(), X()), Read_NewPCL()),
			BusCycle(Address(PBR(), AA(), X(), o(1)), Read_NewPCH(), PC_e(PBR(), New_PC()), DONE()));
	}


	CInstructionCycles* createAbsoluteIndexedIndirectWithXJSRCycles()
	{
		//2b
		return InstructionCycles(AM_AbsoluteIndexedIndirectWithX,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
			BusCycle(Address(S()), Write_PCH(), SP_dec()),
			BusCycle(Address(S()), Write_PCL(), SP_dec()),
			BusCycle(Address(PBR(), PC()), Read_AAH()),
			BusCycle(Address(PBR(), PC()), IO(), PC_inc()),
			BusCycle(Address(PBR(), AA(), X()), Read_NewPCL()),
			BusCycle(Address(PBR(), AA(), X(), o(1)), Read_NewPCH(), PC_e(PBR(), New_PC()), DONE()));
	}


	CInstructionCycles* createAbsoluteIndirectJMLCycles()
	{
		//3a
		return InstructionCycles(AM_AbsoluteIndirectLong,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
			BusCycle(Address(AA()), Read_NewPCL()),
			BusCycle(Address(AA(), o(1)), Read_NewPCH()),
			BusCycle(Address(AA(), o(2)), Read_NewPBR(), PC_e(New_PBR(), New_PC()), DONE()));
	}


	CInstructionCycles* createAbsoluteIndirectJMPCycles()
	{
		//3b
		return InstructionCycles(AM_AbsoluteIndirect,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
			BusCycle(Address(AA()), Read_NewPCL()),
			BusCycle(Address(AA(), o(1)), Read_NewPCH(), PC_e(PBR(), New_PC()), DONE()));
	}


	CInstructionCycles* createAbsoluteLongWriteCycles(CW65C816Func fOperation)
	{
		//4a
		return InstructionCycles(AM_AbsoluteLong,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAB(), PC_inc()),
			BusCycle(Address(AAB(), AA()), E(fOperation), Write_DataLow(), DONE8Bit(M)),
			BusCycle(Address(AAB(), AA(), o(1)), Write_DataHigh(), DONE16Bit(M)));
	}


	CInstructionCycles* createAbsoluteLongCycles(CW65C816Func fOperation)
	{
		//4a
		return InstructionCycles(AM_AbsoluteLong,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAB(), PC_inc()),
			BusCycle(Address(AAB(), AA()), Read_DataLow(), E8Bit(fOperation, M), DONE8Bit(M)),
			BusCycle(Address(AAB(), AA(), o(1)), Read_DataHigh(), E16Bit(fOperation, M), DONE16Bit(M)));
	}


	CInstructionCycles* createAbsoluteLongJMLCycles()
	{
		//4b
		return InstructionCycles(AM_AbsoluteLong,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_NewPCL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_NewPCH(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_NewPBR(), PC_inc(), PC_e(PBR(), New_PC()), DONE()));
	}


	CInstructionCycles* createAbsoluteLongJSLCycles()
	{
		//4c
		return InstructionCycles(AM_AbsoluteLong,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_NewPCL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_NewPCH(), PC_inc()),
			BusCycle(Address(S()), Write_PBR(), SP_dec()),
			BusCycle(Address(S()), IO()),
			BusCycle(Address(PBR(), PC()), Read_NewPBR(), PC_inc()),
			BusCycle(Address(S()), Write_PCH(), SP_dec()),
			BusCycle(Address(S()), Write_PCL(), SP_dec(), PC_e(PBR(), New_PC()), DONE()));
	}


	CInstructionCycles* createAbsoluteLongIndexedWithXCycles(CW65C816Func fOperation)
	{
		//5
		return InstructionCycles(AM_AbsoluteLongIndexedWithX,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAB(), PC_inc()),
			BusCycle(Address(AAB(), AA(), X()), Read_DataLow(), E8Bit(fOperation, M), DONE8Bit(M)),
			BusCycle(Address(AAB(), AA(), X(), o(1)), Read_DataHigh(), E16Bit(fOperation, M), DONE16Bit(M)));
	}


	CInstructionCycles* createAbsoluteLongIndexedWithXWriteCycles(CW65C816Func fOperation)
	{
		//5
		return InstructionCycles(AM_AbsoluteLongIndexedWithX,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAB(), PC_inc()),
			BusCycle(Address(AAB(), AA(), X()), E(fOperation), Write_DataLow(), DONE8Bit(M)),
			BusCycle(Address(AAB(), AA(), X(), o(1)), Write_DataHigh(), DONE16Bit(M)));
	}


	CInstructionCycles* createAbsoluteIndexedWithXWriteCycles(CW65C816Func fOperation)
	{
		//6a
		return InstructionCycles(AM_AbsoluteIndexedWithX,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
			BusCycle(Address(DBR(), AAH(), AAL_XL()), IO(), new NoteFourX(false)),
			BusCycle(Address(DBR(), AA(), X()), E(fOperation), Write_DataLow(), DONE8Bit(M)),
			BusCycle(Address(DBR(), AA(), X(), o(1)), Write_DataHigh(), DONE16Bit(M)));
	}


	CInstructionCycles* createAbsoluteIndexedWithXCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
	{
		//6a
		return InstructionCycles(AM_AbsoluteIndexedWithX,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
			BusCycle(Address(DBR(), AAH(), AAL_XL()), IO(), new NoteFourX(true)),
			BusCycle(Address(DBR(), AA(), X()), Read_DataLow(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth)),
			BusCycle(Address(DBR(), AA(), X(), o(1)), Read_DataHigh(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth)));
	}


	CInstructionCycles* createAbsoluteIndexedWithXRMWCycles(CW65C816Func fOperation)
	{
		//6b
		return InstructionCycles(AM_AbsoluteIndexedWithX,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(RMW), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAH(RMW), PC_inc()),
			BusCycle(Address(DBR(), AAH(), AAL_XL()), IO(RMW)),
			BusCycle(Address(DBR(), AA(), X()), Read_DataLow(RMW)),
			BusCycle(Address(DBR(), AA(), X(), o(1)), Read_DataHigh(RMW), new NoteOne(M)),
			BusCycle(Address(DBR(), AA(), X(), o(1)), IO(RMW), E(fOperation)),
			BusCycle(Address(DBR(), AA(), X(), o(1)), new NoteThree(), Write_DataHigh(RMW), new NoteOne(M)),
			BusCycle(Address(DBR(), AA(), X()), new NoteThree(), Write_DataLow(RMW), DONE()));
	}


	CInstructionCycles* createAbsoluteIndexedWithYWriteCycles(CW65C816Func fOperation)
	{
		//7
		return InstructionCycles(AM_AbsoluteIndexedWithY,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
			BusCycle(Address(DBR(), AAH(), AAL_YL()), IO(), new NoteFourY(false)),
			BusCycle(Address(DBR(), AA(), Y()), E(fOperation), Write_DataLow(), DONE8Bit(M)),
			BusCycle(Address(DBR(), AA(), Y(), o(1)), Write_DataHigh(), DONE16Bit(M)));
	}


	CInstructionCycles* createAbsoluteIndexedWithYCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
	{
		//7
		return InstructionCycles(AM_AbsoluteIndexedWithY,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAH(), PC_inc()),
			BusCycle(Address(DBR(), AAH(), AAL_YL()), IO(), new NoteFourY(true)),
			BusCycle(Address(DBR(), AA(), Y()), Read_DataLow(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth)),
			BusCycle(Address(DBR(), AA(), Y(), o(1)), Read_DataHigh(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth)));
	}


	CInstructionCycles* createAccumulatorCycles(CW65C816Func fOperation)
	{
		//8
		return InstructionCycles(AM_Accumulator,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), E(fOperation), DONE()));
	}


	CInstructionCycles* createBlockMoveCycles(CW65C816Func fOperation)
	{
		//9a & 9b
		return InstructionCycles(AM_AddressingMode.BlockMove,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_DBR(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAB(), PC_inc()),
			BusCycle(Address(AAB(), X()), Read_DataLow()),
			BusCycle(Address(DBR(), Y()), Write_DataLow(), E(fOperation)),
			BusCycle(Address(DBR(), Y()), IO()),
			BusCycle(Address(DBR(), Y()), IO(), DONE()));
	}


	CInstructionCycles* createDirectCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
	{
		//10a
		return InstructionCycles(AM_Direct,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(DP(), D0()), Read_DataLow(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth)),
			BusCycle(Address(DP(), D0(), o(1)), Read_DataHigh(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth)));
	}


	CInstructionCycles* createDirectWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
	{
		//10a
		return InstructionCycles(AM_Direct,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(DP(), D0()), E(fOperation), Write_DataLow(), DONE8Bit(eWidth)),
			BusCycle(Address(DP(), D0(), o(1)), Write_DataHigh(), DONE16Bit(eWidth)));
	}


	CInstructionCycles* createDirectRMWCycles(CW65C816Func fOperation)
	{
		//10b
		return InstructionCycles(AM_Direct,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(RMW), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(DP(), D0()), Read_DataLow(RMW)),
			BusCycle(Address(DP(), D0(), o(1)), Read_DataHigh(RMW), new NoteOne(M)),
			BusCycle(Address(DP(), D0(), o(1)), IO(RMW), E(fOperation)),
			BusCycle(Address(DP(), D0(), o(1)), new NoteThree(), Write_DataHigh(RMW), new NoteOne(M)),
			BusCycle(Address(DP(), D0()), new NoteThree(), Write_DataLow(RMW), DONE()));
	}


	CInstructionCycles* createDirectIndexedIndirectWithXCycles(CW65C816Func fOperation)
	{
		//11
		return InstructionCycles(AM_DirectIndexedIndirectWithX,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(DP(), D0(), X()), Read_AAL()),
			BusCycle(Address(DP(), D0(), X(), o(1)), Read_AAH()),
			BusCycle(Address(DBR(), AA()), Read_DataLow(), E8Bit(fOperation, M), DONE8Bit(M)),
			BusCycle(Address(DBR(), AA(), o(1)), Read_DataHigh(), E16Bit(fOperation, M), DONE16Bit(M)));
	}


	CInstructionCycles* createDirectIndexedIndirectWithXWriteCycles(CW65C816Func fOperation)
	{
		//11
		return InstructionCycles(AM_DirectIndexedIndirectWithX,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(DP(), D0(), X()), Read_AAL()),
			BusCycle(Address(DP(), D0(), X(), o(1)), Read_AAH()),
			BusCycle(Address(DBR(), AA()), E(fOperation), Write_DataLow(), DONE8Bit(M)),
			BusCycle(Address(DBR(), AA(), o(1)), Write_DataHigh(), DONE16Bit(M)));
	}


	CInstructionCycles* createDirectIndirectCycles(CW65C816Func fOperation)
	{
		//12
		return InstructionCycles(AM_DirectIndirect,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(DP(), D0()), Read_AAL()),
			BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
			BusCycle(Address(DBR(), AA()), Read_DataLow(), E8Bit(fOperation, M), DONE8Bit(M)),
			BusCycle(Address(DBR(), AA(), o(1)), Read_DataHigh(), E16Bit(fOperation, M), DONE16Bit(M)));
	}


	CInstructionCycles* createDirectIndirectWriteCycles(CW65C816Func fOperation)
	{
		//12
		return InstructionCycles(AM_DirectIndirect,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(DP(), D0()), Read_AAL()),
			BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
			BusCycle(Address(DBR(), AA()), E(fOperation), Write_DataLow(), DONE8Bit(M)),
			BusCycle(Address(DBR(), AA(), o(1)), Write_DataHigh(), DONE16Bit(M)));
	}


	CInstructionCycles* createDirectIndirectIndexedWithYCycles(CW65C816Func fOperation)
	{
		//13
		return InstructionCycles(AM_DirectIndirectIndexedWithY,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(DP(), D0()), Read_AAL()),
			BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
			BusCycle(Address(DBR(), AAH(), AAL_YL()), IO(), new NoteFourY(true)),
			BusCycle(Address(DBR(), AA(), Y()), Read_DataLow(), E8Bit(fOperation, M), DONE8Bit(M)),
			BusCycle(Address(DBR(), AA(), Y(), o(1)), Read_DataHigh(), E16Bit(fOperation, M), DONE16Bit(M)));
	}


	CInstructionCycles* createDirectIndirectIndexedWithYWriteCycles(CW65C816Func fOperation)
	{
		//13
		return InstructionCycles(AM_DirectIndirectIndexedWithY,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(DP(), D0()), Read_AAL()),
			BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
			BusCycle(Address(DBR(), AAH(), AAL_YL()), IO(), new NoteFourY(false)),
			BusCycle(Address(DBR(), AA(), Y()), E(fOperation), Write_DataLow(), DONE8Bit(M)),
			BusCycle(Address(DBR(), AA(), Y(), o(1)), Write_DataHigh(), DONE16Bit(M)));
	}


	CInstructionCycles* createDirectIndirectLongIndexedWithYCycles(CW65C816Func fOperation)
	{
		//14
		return InstructionCycles(AM_DirectIndirectLongIndexedWithY,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(DP(), D0()), Read_AAL()),
			BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
			BusCycle(Address(DP(), D0(), o(2)), Read_AAB()),
			BusCycle(Address(AAB(), AA(), Y()), Read_DataLow(), E8Bit(fOperation, M), DONE8Bit(M)),
			BusCycle(Address(AAB(), AA(), Y(), o(1)), Read_DataHigh(), E16Bit(fOperation, M), DONE16Bit(M)));
	}


	CInstructionCycles* createDirectIndirectLongIndexedWithYWriteCycles(CW65C816Func fOperation)
	{
		//14
		return InstructionCycles(AM_DirectIndirectLongIndexedWithY,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(DP(), D0()), Read_AAL()),
			BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
			BusCycle(Address(DP(), D0(), o(2)), Read_AAB()),
			BusCycle(Address(AAB(), AA(), Y()), E(fOperation), Write_DataLow(), DONE8Bit(M)),
			BusCycle(Address(AAB(), AA(), Y(), o(1)), Write_DataHigh(), DONE16Bit(M)));
	}


	CInstructionCycles* createDirectIndirectLongCycles(CW65C816Func fOperation)
	{
		//15
		return InstructionCycles(AM_DirectIndirectLong,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(DP(), D0()), Read_AAL()),
			BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
			BusCycle(Address(DP(), D0(), o(2)), Read_AAB()),
			BusCycle(Address(AAB(), AA()), Read_DataLow(), E8Bit(fOperation, M), DONE8Bit(M)),
			BusCycle(Address(AAB(), AA(), o(1)), Read_DataHigh(), E16Bit(fOperation, M), DONE16Bit(M)));
	}


	CInstructionCycles* createDirectIndirectLongWriteCycles(CW65C816Func fOperation)
	{
		//15
		return InstructionCycles(AM_DirectIndirectLong,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(DP(), D0()), Read_AAL()),
			BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
			BusCycle(Address(DP(), D0(), o(2)), Read_AAB()),
			BusCycle(Address(AAB(), AA()), E(fOperation), Write_DataLow(), DONE8Bit(M)),
			BusCycle(Address(AAB(), AA(), o(1)), Write_DataHigh(), DONE16Bit(M)));
	}


	CInstructionCycles* createDirectIndexedWithXCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
	{
		//16a
		return InstructionCycles(AM_DirectIndexedWithX,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(DP(), D0(), X()), Read_DataLow(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth)),
			BusCycle(Address(DP(), D0(), X(), o(1)), Read_DataHigh(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth)));
	}


	CInstructionCycles* createDirectIndexedWithXWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
	{
		//16a
		return InstructionCycles(AM_DirectIndexedWithX,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(DP(), D0(), X()), E(fOperation), Write_DataLow(), DONE8Bit(eWidth)),
			BusCycle(Address(DP(), D0(), X(), o(1)), Write_DataHigh(), DONE16Bit(eWidth)));
	}


	CInstructionCycles* createDirectIndexedWithXRMWCycles(CW65C816Func fOperation)
	{
		//16b
		return InstructionCycles(AM_DirectIndexedWithX,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(RMW), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(RMW), new NoteTwo()),
			BusCycle(Address(PBR(), PC()), IO(RMW)),
			BusCycle(Address(DP(), D0(), X()), Read_DataLow(RMW)),
			BusCycle(Address(DP(), D0(), X(), o(1)), Read_DataHigh(RMW), new NoteOne(M)),
			BusCycle(Address(DP(), D0(), X(), o(1)), IO(RMW), E(fOperation)),
			BusCycle(Address(DP(), D0(), X(), o(1)), new NoteThree(), Write_DataHigh(RMW), new NoteOne(M)),
			BusCycle(Address(DP(), D0(), X()), new NoteThree(), Write_DataLow(RMW), DONE()));
	}


	CInstructionCycles* createDirectIndexedWithYCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
	{
		//17
		return InstructionCycles(AM_DirectIndexedWithY,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(DP(), D0(), Y()), Read_DataLow(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth)),
			BusCycle(Address(DP(), D0(), Y(), o(1)), Read_DataHigh(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth)));
	}


	CInstructionCycles* createDirectIndexedWithYWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
	{
		//17
		return InstructionCycles(AM_DirectIndexedWithY,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(DP(), D0(), Y()), E(fOperation), Write_DataLow(), DONE8Bit(eWidth)),
			BusCycle(Address(DP(), D0(), Y(), o(1)), Write_DataHigh(), DONE16Bit(eWidth)));
	}


	CInstructionCycles* createImmediateREPSEPCycles(CW65C816Func fOperation)
	{
		//18
		return InstructionCycles(AM_Immediate,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), PC_inc(), Read_DataLow()),
			BusCycle(Address(PBR(), PC()), Read_DataHigh(), E(fOperation), DONE()));
	}


	CInstructionCycles* createImmediateCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
	{
		//18
		return InstructionCycles(AM_Immediate,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), PC_inc(), Read_DataLow(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth)),
			BusCycle(Address(PBR(), PC()), PC_inc(), Read_DataHigh(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth)));
	}


	CInstructionCycles* createImpliedCycles(CW65C816Func fOperation)
	{
		//19a
		return InstructionCycles(AM_Implied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), E(fOperation), DONE()));
	}


	CInstructionCycles* createWDMImpliedCycles(CW65C816Func fOperation)
	{
		return InstructionCycles(AM_Implied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), PC_inc(), IO(), E(fOperation), DONE()));
	}


	CInstructionCycles* createImpliedXBACycles(CW65C816Func fOperation)
	{
		//19b
		return InstructionCycles(AM_Implied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(PBR(), PC()), IO(), E(fOperation), DONE()));
	}


	CInstructionCycles* createStopTheClockCycles(CW65C816Func fOperation)
	{
		//19c
		return InstructionCycles(AM_StopTheClock,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(PBR(), PC()), IO(), E(fOperation), DONE()));
	}


	CInstructionCycles* createWaitForInterruptCycles()
	{
		//19d
		return InstructionCycles(AM_WaitForInterrupt,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(PBR(), PC()), new WaitOperation(), DONE()));
	}


	CInstructionCycles* createRelativeShortCycles(CW65C816Func fOperation)
	{
		//20
		return InstructionCycles(AM_Relative,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_DataLow(), PC_inc(), E(fOperation)),  //Done if branch not taken
			BusCycle(Address(PBR(), PC()), IO(), new NoteSix()),
			BusCycle(Address(PBR(), PC()), PC_e(PBR(), PC(), new SignedDataLow()), IO(), DONE()));
	}


	CInstructionCycles* createRelativeLongCycles(CW65C816Func fOperation)
	{
		//21
		return InstructionCycles(AM_RelativeLong,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_DataLow(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_DataHigh(), PC_inc(), E(fOperation)),  //Done if branch not taken
			BusCycle(Address(PBR(), PC()), PC_e(PBR(), PC(), new SignedData()), IO(), DONE()));
	}


	CInstructionCycles* createStackHardwareInterruptCycles(InterruptVector interruptVector, CW65C816Func fOperation)
	{
		//22a
		return InstructionCycles(AM_StackInterruptHardware,
			BusCycle(Address(PBR(), PC()), new InternalOperation(true, true, true)),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S()), Write_PBR(), SP_dec(), PBR_e(0), new NoteSeven()),
			BusCycle(Address(S()), Write_PCH(), SP_dec()),
			BusCycle(Address(S()), Write_PCL(), SP_dec()),
			BusCycle(Address(S()), Write_PS(), SP_dec(), E(fOperation)),
			BusCycle(Address(VA(interruptVector)), Read_AAVL()),
			BusCycle(Address(VA(interruptVector), o(1)), Read_AAVH(), PC_e(AA()), DONE()));
	}


	CInstructionCycles* createStackAbortInterruptCycles(InterruptVector interruptVector, CW65C816Func fOperation)
	{
		//22a
		return InstructionCycles(AM_StackInterruptHardware,
			BusCycle(Address(PBR(), PC()), new InternalOperation(true, true, true)),
			BusCycle(Address(PBR(), PC()), IO(), new RestoreAbortValues()),
			BusCycle(Address(S()), Write_PBR(), SP_dec(), PBR_e(0), new NoteSeven()),
			BusCycle(Address(S()), Write_PCH(), SP_dec()),
			BusCycle(Address(S()), Write_PCL(), SP_dec()),
			BusCycle(Address(S()), Write_PS(), SP_dec(), E(fOperation)),
			BusCycle(Address(VA(interruptVector)), Read_AAVL()),
			BusCycle(Address(VA(interruptVector), o(1)), Read_AAVH(), PC_e(AA()), DONE()));
	}


	CInstructionCycles* createStackResetCycles(InterruptVector interruptVector, CW65C816Func fOperation)
	{
		//22a
		return InstructionCycles(AM_StackInterruptHardware,
			BusCycle(Address(PBR(), PC()), new InternalOperation(true, true, true)),
			BusCycle(Address(PBR(), PC()), IO(), E(fOperation)),
			BusCycle(Address(S()), IO(), SP_dec()),
			BusCycle(Address(S()), IO(), SP_dec()),
			BusCycle(Address(S()), IO(), SP_dec()),
			BusCycle(Address(VA(interruptVector)), Read_AAVL()),
			BusCycle(Address(VA(interruptVector), o(1)), Read_AAVH(), PC_e(PBR(), AA()), DONE()));
	}


	CInstructionCycles* createStackPullCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
	{
		//22b
		return InstructionCycles(AM_StackImplied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S(), o(1)), Read_DataLow(), SP_inc(), E8Bit(fOperation, eWidth), DONE8Bit(eWidth)),
			BusCycle(Address(S(), o(1)), Read_DataHigh(), SP_inc(), E16Bit(fOperation, eWidth), DONE16Bit(eWidth)));
	}


	CInstructionCycles* createStackPLBCycles(CW65C816Func fOperation)
	{
		//22b
		return InstructionCycles(AM_StackImplied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S(), o(1)), SP_inc(), Read_DataLow(), E(fOperation), DONE()));
	}


	CInstructionCycles* createStackPLDCycles(CW65C816Func fOperation)
	{
		//22b
		return InstructionCycles(AM_StackImplied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S(), o(1)), Read_DataLow(), SP_inc()),
			BusCycle(Address(S(), o(1)), Read_DataHigh(), SP_inc(), E(fOperation), DONE()));
	}


	CInstructionCycles* createStackPLPCycles(CW65C816Func fOperation)
	{
		//22b
		return InstructionCycles(AM_StackImplied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S(), o(1)), IO()),
			BusCycle(Address(S(), o(1)), Read_DataLow(), SP_inc(), E(fOperation), DONE()));
	}


	CInstructionCycles* createStackImpliedPHPCycles(CW65C816Func fOperation)
	{
		//22c
		return InstructionCycles(AM_StackImplied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S()), E(fOperation), Write_DataLow(), SP_dec(), DONE()));
	}


	CInstructionCycles* createStackPushCycles(CW65C816Func fOperation, EWidthFromRegister eWidth)
	{
		//22c
		return InstructionCycles(AM_StackImplied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), E(fOperation)),
			BusCycle(Address(S()), Write_DataHigh(), SP_dec(), new NoteOne(eWidth)),
			BusCycle(Address(S()), Write_DataLow(), SP_dec(), DONE()));
	}


	CInstructionCycles* createStackPHBCycles(CW65C816Func fOperation)
	{
		//22c
		return InstructionCycles(AM_StackImplied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S()), E(fOperation), Write_DataLow(), SP_dec(), DONE()));
	}


	CInstructionCycles* createStackPHDCycles(CW65C816Func fOperation)
	{
		//22c
		return InstructionCycles(AM_StackImplied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S()), E(fOperation), Write_DataHigh(), SP_dec()),
			BusCycle(Address(S()), Write_DataLow(), SP_dec(), DONE()));
	}


	CInstructionCycles* createStackPHKCycles(CW65C816Func fOperation)
	{
		//22c
		return InstructionCycles(AM_StackImplied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S()), E(fOperation), Write_DataLow(), SP_dec(), DONE()));
	}


	CInstructionCycles* createStackPEACycles()
	{
		//22d
		return InstructionCycles(AM_StackImmediate,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAL(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_AAH()),
			BusCycle(Address(S()), Write_AAH(), SP_dec()),
			BusCycle(Address(S()), Write_AAL(), SP_dec(), DONE()));
	}


	CInstructionCycles* createStackPEICycles()
	{
		//22e
		return InstructionCycles(AM_StackDirectIndirect,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), new NoteTwo()),
			BusCycle(Address(DP(), D0()), Read_AAL()),
			BusCycle(Address(DP(), D0(), o(1)), Read_AAH()),
			BusCycle(Address(S()), Write_AAH(), SP_dec()),
			BusCycle(Address(S()), Write_AAL(), SP_dec(), DONE()));
	}


	CInstructionCycles* createStackPERCycles(CW65C816Func fOperation)
	{
		//22f
		return InstructionCycles(AM_StackImplied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_DataLow(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_DataHigh(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S()), E(fOperation), Write_DataHigh(), SP_dec()),
			BusCycle(Address(S()), Write_DataLow(), SP_dec(), DONE()));
	}


	CInstructionCycles* createStackRTICycles()
	{
		//22g
		return InstructionCycles(AM_StackImplied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S(), o(1)), Read_PS(), SP_inc()),  //Processor status
			BusCycle(Address(S(), o(1)), Read_NewPCL(), SP_inc()),
			BusCycle(Address(S(), o(1)), Read_NewPCH(), SP_inc()),
			BusCycle(Address(S(), o(1)), Read_NewPBR(), SP_inc(), PC_e(New_PBR(), New_PC()), DONE()));
	}


	CInstructionCycles* createStackRTSCycles()
	{
		//22h
		return InstructionCycles(AM_StackImplied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S(), o(1)), Read_NewPCL(), SP_inc()),
			BusCycle(Address(S(), o(1)), Read_NewPCH(), SP_inc()),
			BusCycle(Address(S()), IO(), PC_e(PBR(), New_PC()), DONE()));
	}


	CInstructionCycles* createStackRTLCycles()
	{
		//22i
		return InstructionCycles(AM_StackImplied,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S(), o(1)), Read_NewPCL(), SP_inc()),
			BusCycle(Address(S(), o(1)), Read_NewPCH(), SP_inc()),
			BusCycle(Address(S(), o(1)), Read_NewPBR(), SP_inc(), PC_e(New_PBR(), New_PC()), DONE()));
	}


	CInstructionCycles* createStackSoftwareInterruptCycles(InterruptVector interruptVector, CW65C816Func fOperation)
	{
		//22j
		return InstructionCycles(AM_StackInterruptSoftware,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), new InternalOperation(true, false, true), PC_inc()),  //Read and ignore the signature byte.
			BusCycle(Address(S()), Write_PBR(), SP_dec(), PBR_e(0), new NoteSeven()),
			BusCycle(Address(S()), Write_PCH(), SP_dec()),
			BusCycle(Address(S()), Write_PCL(), SP_dec()),
			BusCycle(Address(S()), Write_PS(), SP_dec(), E(fOperation)),
			BusCycle(Address(VA(interruptVector)), Read_AAL()),
			BusCycle(Address(VA(interruptVector), o(1)), Read_AAH(), PC_e(PBR(), AA()), DONE()));
	}


	CInstructionCycles* createStackRelativeCycles(CW65C816Func fOperation)
	{
		//23
		return InstructionCycles(AM_StackRelative,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S(), D0()), Read_DataLow(), E8Bit(fOperation, M), DONE8Bit(M)),
			BusCycle(Address(S(), D0(), o(1)), Read_DataHigh(), E16Bit(fOperation, M), DONE16Bit(M)));
	}


	CInstructionCycles* createStackRelativeWriteCycles(CW65C816Func fOperation)
	{
		//23
		return InstructionCycles(AM_StackRelative,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S(), D0()), E(fOperation), Write_DataLow(), DONE8Bit(M)),
			BusCycle(Address(S(), D0(), o(1)), Write_DataHigh(), DONE16Bit(M)));
	}


	CInstructionCycles* createStackRelativeIndirectIndexedWithYCycles(CW65C816Func fOperation)
	{
		//24
		return InstructionCycles(AM_StackRelativeIndirectIndexedWithY,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S(), D0()), Read_AAL()),
			BusCycle(Address(S(), D0(), o(1)), Read_AAH()),
			BusCycle(Address(S(), D0(), o(1)), IO()),
			BusCycle(Address(DBR(), AA(), Y()), Read_DataLow(), E8Bit(fOperation, M), DONE8Bit(M)),
			BusCycle(Address(DBR(), AA(), Y(), o(1)), Read_DataHigh(), E16Bit(fOperation, M), DONE16Bit(M)));
	}


	CInstructionCycles* createStackRelativeIndirectIndexedWithYWriteCycles(CW65C816Func fOperation)
	{
		//24
		return InstructionCycles(AM_StackRelativeIndirectIndexedWithY,
			BusCycle(Address(PBR(), PC()), OpCode(), PC_inc()),
			BusCycle(Address(PBR(), PC()), Read_D0(), PC_inc()),
			BusCycle(Address(PBR(), PC()), IO()),
			BusCycle(Address(S(), D0()), Read_AAL()),
			BusCycle(Address(S(), D0(), o(1)), Read_AAH()),
			BusCycle(Address(S(), D0(), o(1)), IO()),
			BusCycle(Address(DBR(), AA(), Y()), E(fOperation), Write_DataLow(), DONE8Bit(M)),
			BusCycle(Address(DBR(), AA(), Y(), o(1)), Write_DataHigh(), DONE16Bit(M)));
	}
};



#endif // __INSTRUCTION_CYCLE_FACTORY_H__

