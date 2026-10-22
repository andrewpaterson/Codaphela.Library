/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdarg.h>
#include "BaseLib/Logger.h"
#include "BaseLib/ConstructorCall.h"
#include "65C64.h"
#include "65C64InstructionCycleFactory.h"


#define MAX_65C64_CYCLE_PARAMETERS	64


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65InstructionCycles* C65C64InstructionCycleFactory::InstructionCycles(C65BusCycle* pcBusCycle, ...)
{
	va_list					vaMarker;
	C65InstructionCycles*	pcInstructionCycles;
	C65BusCycleArray*		papcBusCycles;
	size					iCount;

	papcBusCycles = NewMalloc<C65BusCycleArray>();
	papcBusCycles->Init();

	iCount = 0;
	va_start(vaMarker, pcBusCycle);
	while (pcBusCycle)
	{
		if (iCount > MAX_65C64_CYCLE_PARAMETERS)
		{
			LOG_ERROR("Bus Cycles have no terminal NULL.");
			break;
		}

		papcBusCycles->Add(pcBusCycle);

		iCount++;
		pcBusCycle = va_arg(vaMarker, C65BusCycle*);
	}
	va_end(vaMarker);

	pcInstructionCycles = NewMalloc<C65InstructionCycles>();
	pcInstructionCycles->Init(papcBusCycles);
	return pcInstructionCycles;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65BusCycle* C65C64InstructionCycleFactory::BusCycle(C65AddressOffsetArray* papcAddressOffsets, C65OperationArray* papcOperations)
{
	C65BusCycle*	pcBusCycle;

	pcBusCycle = NewMalloc<C65BusCycle>();
	pcBusCycle->Init(papcAddressOffsets, papcOperations);
	return pcBusCycle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65AddressOffsetArray* C65C64InstructionCycleFactory::Address(C65AddressOffset* pcOffset, ...)
{
	va_list					vaMarker;
	C65AddressOffsetArray*	papcOffsets;
	size					iCount;

	papcOffsets = NewMalloc<C65AddressOffsetArray>();
	papcOffsets->Init();

	iCount = 0;
	va_start(vaMarker, pcOffset);
	while (pcOffset)
	{
		if (iCount > MAX_65C64_CYCLE_PARAMETERS)
		{
			LOG_ERROR("Address Offsets have no terminal NULL.");
			break;
		}

		papcOffsets->Add(pcOffset);

		iCount++;
		pcOffset = va_arg(vaMarker, C65AddressOffset*);
	}
	va_end(vaMarker);

	return papcOffsets;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65OperationArray* C65C64InstructionCycleFactory::Operation(C65Operation* pcOperation, ...)
{
	va_list				vaMarker;
	C65OperationArray*	papcOperations;
	size				iCount;

	papcOperations = NewMalloc<C65OperationArray>();
	papcOperations->Init();

	iCount = 0;
	va_start(vaMarker, pcOperation);
	while (pcOperation)
	{
		if (iCount > MAX_65C64_CYCLE_PARAMETERS)
		{
			LOG_ERROR("Operations have no terminal NULL.");
			break;
		}

		papcOperations->Add(pcOperation);

		iCount++;
		pcOperation = va_arg(vaMarker, C65Operation*);
	}
	va_end(vaMarker);

	return papcOperations;
}


#define NEW_65(type, init) \
	type*	pcType; \
	pcType = NewMalloc<type>(); \
	pcType->init; \
	return pcType;


C65ProgramCounterOffset*		C65C64InstructionCycleFactory::PC(void)										{ NEW_65(C65ProgramCounterOffset, Init()) }
C65PointerAddressOffset*		C65C64InstructionCycleFactory::PA(void)										{ NEW_65(C65PointerAddressOffset, Init()) }
C65VectorAddressOffset*			C65C64InstructionCycleFactory::VA(void)										{ NEW_65(C65VectorAddressOffset, Init()) }
C65TransferWordOffset*			C65C64InstructionCycleFactory::R(uint8 uiWord)								{ NEW_65(C65TransferWordOffset, Init(uiWord, false)) }
C65TransferWordOffset*			C65C64InstructionCycleFactory::W(uint8 uiWord)								{ NEW_65(C65TransferWordOffset, Init(uiWord, true)) }
C65ConstantOffset*				C65C64InstructionCycleFactory::o(int32 iOffset)								{ NEW_65(C65ConstantOffset, Init(iOffset)) }
C65OpcodeCycle*					C65C64InstructionCycleFactory::Opcode(void)									{ NEW_65(C65OpcodeCycle, Init()) }
C65ReadOpcode*					C65C64InstructionCycleFactory::Read_Opcode(void)							{ NEW_65(C65ReadOpcode, Init()) }
C65IncrementProgramCounter*		C65C64InstructionCycleFactory::C_inc(void)									{ NEW_65(C65IncrementProgramCounter, Init()) }
C65ReadOperandWord*				C65C64InstructionCycleFactory::Read_Operand(uint8 uiWord)					{ NEW_65(C65ReadOperandWord, Init(uiWord)) }
C65ReadPointerWord*				C65C64InstructionCycleFactory::Read_Pointer(uint8 uiWord)					{ NEW_65(C65ReadPointerWord, Init(uiWord)) }
C65ReadTransferWord*			C65C64InstructionCycleFactory::Read_Transfer(uint8 uiWord)					{ NEW_65(C65ReadTransferWord, Init(uiWord)) }
C65WriteTransferWord*			C65C64InstructionCycleFactory::Write_Transfer(uint8 uiWord)					{ NEW_65(C65WriteTransferWord, Init(uiWord)) }
C65ReadVectorWord*				C65C64InstructionCycleFactory::Read_Vector(uint8 uiWord)					{ NEW_65(C65ReadVectorWord, Init(uiWord)) }
C65InternalOperation*			C65C64InstructionCycleFactory::IO(void)										{ NEW_65(C65InternalOperation, Init()) }
C65WaitOperation*				C65C64InstructionCycleFactory::Wait(uint8 uiWait)							{ NEW_65(C65WaitOperation, Init(uiWait)) }
C65Execute*						C65C64InstructionCycleFactory::E(C65C64Func fFunction, bool bInitialSide)	{ NEW_65(C65Execute, Init(fFunction, bInitialSide)) }
C65ExecuteIfLastPointerWord*	C65C64InstructionCycleFactory::E_LastPointer(uint8 uiWord, C65C64Func fFunction)	{ NEW_65(C65ExecuteIfLastPointerWord, Init(uiWord, fFunction)) }
C65ExecuteIfLastTransferWord*	C65C64InstructionCycleFactory::E_LastTransfer(uint8 uiWord, C65C64Func fFunction)	{ NEW_65(C65ExecuteIfLastTransferWord, Init(uiWord, fFunction)) }
C65DoneInstruction*				C65C64InstructionCycleFactory::DONE(void)									{ NEW_65(C65DoneInstruction, Init()) }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65InstructionCycles* C65C64InstructionCycleFactory::CreateFetchNextCycles(void)
{
	return InstructionCycles(
		BusCycle(	Address(PC(), NULL),	Operation(Read_Opcode(), NULL)),
		NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65InstructionCycles* C65C64InstructionCycleFactory::CreateResetCycles(void)
{
	return InstructionCycles(
		BusCycle(	Address(PC(), NULL),			Operation(IO(), E(&C65C64::Reset, true), NULL)),
		BusCycle(	Address(PC(), NULL),			Operation(IO(), NULL)),
		BusCycle(	Address(VA(), NULL),			Operation(Read_Vector(0), NULL)),
		BusCycle(	Address(VA(), o(2), NULL),		Operation(Read_Vector(1), E(&C65C64::LoadVector, false), NULL)),
		NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Taken instead of an opcode fetch.  C and P were pushed when the interrupt was taken.
//
//////////////////////////////////////////////////////////////////////////
C65InstructionCycles* C65C64InstructionCycleFactory::CreateInterruptCycles(void)
{
	C65BusCycleArray*		papcBusCycles;
	C65InstructionCycles*	pcInstructionCycles;
	uint8					i;

	papcBusCycles = NewMalloc<C65BusCycleArray>();
	papcBusCycles->Init();

	//4 bytes of C and 2 bytes of P take up to 5 words.
	for (i = 0; i < 5; i++)
	{
		papcBusCycles->Add(BusCycle(Address(W(i), NULL), Operation(Write_Transfer(i), NULL)));
	}
	papcBusCycles->Add(BusCycle(Address(VA(), NULL), Operation(Read_Vector(0), NULL)));
	papcBusCycles->Add(BusCycle(Address(VA(), o(2), NULL), Operation(Read_Vector(1), E(&C65C64::LoadVector, false), NULL)));

	pcInstructionCycles = NewMalloc<C65InstructionCycles>();
	pcInstructionCycles->Init(papcBusCycles);
	return pcInstructionCycles;
}


//////////////////////////////////////////////////////////////////////////
//
// An internal cycle repeated (the clock edges are ignored) until an interrupt wakes the processor.
//
//////////////////////////////////////////////////////////////////////////
C65InstructionCycles* C65C64InstructionCycleFactory::CreateWaitCycles(void)
{
	return InstructionCycles(
		BusCycle(	Address(PC(), NULL),	Operation(IO(), NULL)),
		NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AddOperation(C65OperationArray* papcOperations, C65Operation* pcOperation)
{
	if (pcOperation)
	{
		papcOperations->Add(pcOperation);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65InstructionCycles* C65C64InstructionCycleFactory::CreateCycles(S65C64Form* psForm)
{
	C65BusCycleArray*		papcBusCycles;
	C65InstructionCycles*	pcInstructionCycles;
	C65OperationArray*		papcOperations;
	bool					bReads;
	bool					bWrites;
	bool					bExecuteAtAddress;
	uint8					i;

	//The instruction executes when its address is known unless it reads memory first.
	bReads = (psForm->eAccess == MA65_Read) || (psForm->eAccess == MA65_ReadWrite);
	bWrites = (psForm->eAccess == MA65_Write) || (psForm->eAccess == MA65_ReadWrite);
	bExecuteAtAddress = !bReads;

	papcBusCycles = NewMalloc<C65BusCycleArray>();
	papcBusCycles->Init();

	//The opcode, then the operand words.  The operand is known after the last of them.
	papcOperations = Operation(Opcode(), C_inc(), NULL);
	if (psForm->uiOperandWords == 0)
	{
		AddOperation(papcOperations, psForm->fOperand ? E(psForm->fOperand, false) : NULL);
		if (bExecuteAtAddress && !psForm->bIndirect)
		{
			AddOperation(papcOperations, psForm->fExecute ? E(psForm->fExecute, false) : NULL);
		}
	}
	papcBusCycles->Add(BusCycle(Address(PC(), NULL), papcOperations));

	for (i = 0; i < psForm->uiOperandWords; i++)
	{
		papcOperations = Operation(Read_Operand(i), C_inc(), NULL);
		if (i == psForm->uiOperandWords - 1)
		{
			AddOperation(papcOperations, psForm->fOperand ? E(psForm->fOperand, false) : NULL);
			if (bExecuteAtAddress && !psForm->bIndirect)
			{
				AddOperation(papcOperations, psForm->fExecute ? E(psForm->fExecute, false) : NULL);
			}
		}
		papcBusCycles->Add(BusCycle(Address(PC(), NULL), papcOperations));
	}

	//A 32 bit pointer takes 2 or 3 words.
	if (psForm->bIndirect)
	{
		for (i = 0; i < 3; i++)
		{
			papcOperations = Operation(Read_Pointer(i), E_LastPointer(i, &C65C64::CalculateIndirectAddress), NULL);
			if (bExecuteAtAddress)
			{
				AddOperation(papcOperations, psForm->fExecute ? E_LastPointer(i, psForm->fExecute) : NULL);
			}
			papcBusCycles->Add(BusCycle(Address(PA(), o(2 * i), NULL), papcOperations));
		}
	}

	if (bReads)
	{
		for (i = 0; i < psForm->uiMaxTransferWords; i++)
		{
			papcOperations = Operation(Read_Transfer(i), NULL);
			AddOperation(papcOperations, psForm->fExecute ? E_LastTransfer(i, psForm->fExecute) : NULL);
			papcBusCycles->Add(BusCycle(Address(R(i), NULL), papcOperations));
		}
	}

	if (bWrites)
	{
		for (i = 0; i < psForm->uiMaxTransferWords; i++)
		{
			papcBusCycles->Add(BusCycle(Address(W(i), NULL), Operation(Write_Transfer(i), NULL)));
		}
	}

	if (psForm->bVector)
	{
		papcBusCycles->Add(BusCycle(Address(VA(), NULL), Operation(Read_Vector(0), NULL)));
		papcBusCycles->Add(BusCycle(Address(VA(), o(2), NULL), Operation(Read_Vector(1), E(&C65C64::LoadVector, false), NULL)));
	}

	for (i = 0; i < psForm->uiMaxWait; i++)
	{
		papcBusCycles->Add(BusCycle(Address(PC(), NULL), Operation(Wait(i), NULL)));
	}

	pcInstructionCycles = NewMalloc<C65InstructionCycles>();
	pcInstructionCycles->Init(papcBusCycles);
	return pcInstructionCycles;
}

