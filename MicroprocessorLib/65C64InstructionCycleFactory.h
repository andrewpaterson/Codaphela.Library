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
#ifndef __65C64_INSTRUCTION_CYCLE_FACTORY_H__
#define __65C64_INSTRUCTION_CYCLE_FACTORY_H__
#include "65C64Func.h"
#include "65C64Decoded.h"
#include "65InstructionCycles.h"
#include "65BusCycle.h"

//Addressing
#include "65ConstantOffset.h"
#include "65PointerAddressOffset.h"
#include "65ProgramCounterOffset.h"
#include "65TransferWordOffset.h"
#include "65VectorAddressOffset.h"

//Operations
#include "65DoneInstruction.h"
#include "65Execute.h"
#include "65ExecuteIfLastPointerWord.h"
#include "65ExecuteIfLastTransferWord.h"
#include "65IncrementProgramCounter.h"
#include "65InternalOperation.h"
#include "65OpcodeCycle.h"
#include "65ReadOpcode.h"
#include "65ReadOperandWord.h"
#include "65ReadPointerWord.h"
#include "65ReadTransferWord.h"
#include "65ReadVectorWord.h"
#include "65WaitOperation.h"
#include "65WriteTransferWord.h"


//The bus cycles an instruction form needs.  Instructions with the same form share the same cycles.
struct S65C64Form
{
	E65Instruction	eInstruction;
	uint8			uiOperandWords;
	bool			bIndirect;
	E65Access		eAccess;
	uint8			uiMaxTransferWords;
	uint8			uiMaxWait;
	bool			bVector;
	C65C64Func		fOperand;
	C65C64Func		fExecute;
};


class C65C64InstructionCycleFactory
{
public:
	static C65InstructionCycles*	CreateFetchNextCycles(void);
	static C65InstructionCycles*	CreateResetCycles(void);
	static C65InstructionCycles*	CreateInterruptCycles(void);
	static C65InstructionCycles*	CreateWaitCycles(void);
	static C65InstructionCycles*	CreateCycles(S65C64Form* psForm);

protected:
	static C65InstructionCycles*	InstructionCycles(C65BusCycle* pcBusCycle, ...);
	static C65BusCycle*				BusCycle(C65AddressOffsetArray* papcAddressOffsets, C65OperationArray* papcOperations);
	static C65AddressOffsetArray*	Address(C65AddressOffset* pcOffset, ...);
	static C65OperationArray*		Operation(C65Operation* pcOperation, ...);

	static C65ProgramCounterOffset*		PC(void);
	static C65PointerAddressOffset*		PA(void);
	static C65VectorAddressOffset*		VA(void);
	static C65TransferWordOffset*		R(uint8 uiWord);
	static C65TransferWordOffset*		W(uint8 uiWord);
	static C65ConstantOffset*			o(int32 iOffset);

	static C65OpcodeCycle*				Opcode(void);
	static C65ReadOpcode*				Read_Opcode(void);
	static C65IncrementProgramCounter*	C_inc(void);
	static C65ReadOperandWord*			Read_Operand(uint8 uiWord);
	static C65ReadPointerWord*			Read_Pointer(uint8 uiWord);
	static C65ReadTransferWord*			Read_Transfer(uint8 uiWord);
	static C65WriteTransferWord*		Write_Transfer(uint8 uiWord);
	static C65ReadVectorWord*			Read_Vector(uint8 uiWord);
	static C65InternalOperation*		IO(void);
	static C65WaitOperation*			Wait(uint8 uiWait);
	static C65Execute*					E(C65C64Func fFunction, bool bInitialSide);
	static C65ExecuteIfLastPointerWord*	E_LastPointer(uint8 uiWord, C65C64Func fFunction);
	static C65ExecuteIfLastTransferWord*	E_LastTransfer(uint8 uiWord, C65C64Func fFunction);
	static C65DoneInstruction*			DONE(void);
};


#endif // __65C64_INSTRUCTION_CYCLE_FACTORY_H__

