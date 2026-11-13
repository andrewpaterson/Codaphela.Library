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
#include "65C64InstructionCycles.h"
#include "65C64BusCycle.h"

//Addressing
#include "65C64ConstantOffset.h"
#include "65C64PointerAddressOffset.h"
#include "65C64ProgramCounterOffset.h"
#include "65C64TransferWordOffset.h"
#include "65C64VectorAddressOffset.h"

//Operations
#include "65C64DoneInstruction.h"
#include "65C64Execute.h"
#include "65C64ExecuteIfLastPointerWord.h"
#include "65C64ExecuteIfLastTransferWord.h"
#include "65C64IncrementProgramCounter.h"
#include "65C64InternalOperation.h"
#include "65C64OpcodeCycle.h"
#include "65C64ReadOpcode.h"
#include "65C64ReadOperandWord.h"
#include "65C64ReadPointerWord.h"
#include "65C64ReadTransferWord.h"
#include "65C64ReadVectorWord.h"
#include "65C64WaitOperation.h"
#include "65C64WriteTransferWord.h"


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
	static C65C64InstructionCycles*	CreateFetchNextCycles(void);
	static C65C64InstructionCycles*	CreateResetCycles(void);
	static C65C64InstructionCycles*	CreateInterruptCycles(void);
	static C65C64InstructionCycles*	CreateWaitCycles(void);
	static C65C64InstructionCycles*	CreateCycles(S65C64Form* psForm);

protected:
	static C65C64InstructionCycles*	InstructionCycles(C65C64BusCycle* pcBusCycle, ...);
	static C65C64BusCycle*				BusCycle(C65C64AddressOffsetArray* papcAddressOffsets, C65OperationArray* papcOperations);
	static C65C64AddressOffsetArray*	Address(C65C64AddressOffset* pcOffset, ...);
	static C65OperationArray*		Operation(C65C64Operation* pcOperation, ...);

	static C65C64ProgramCounterOffset*		PC(void);
	static C65C64PointerAddressOffset*		PA(void);
	static C65C64VectorAddressOffset*		VA(void);
	static C65C64TransferWordOffset*		R(uint8 uiWord);
	static C65C64TransferWordOffset*		W(uint8 uiWord);
	static C65C64ConstantOffset*			o(int32 iOffset);

	static C65C64OpcodeCycle*				Opcode(void);
	static C65C64ReadOpcode*				Read_Opcode(void);
	static C65C64IncrementProgramCounter*	C_inc(void);
	static C65C64ReadOperandWord*			Read_Operand(uint8 uiWord);
	static C65C64ReadPointerWord*			Read_Pointer(uint8 uiWord);
	static C65C64ReadTransferWord*			Read_Transfer(uint8 uiWord);
	static C65C64WriteTransferWord*		Write_Transfer(uint8 uiWord);
	static C65C64ReadVectorWord*			Read_Vector(uint8 uiWord);
	static C65C64InternalOperation*		IO(void);
	static C65C64WaitOperation*			Wait(uint8 uiWait);
	static C65C64Execute*					E(C65C64Func fFunction, bool bInitialSide);
	static C65C64ExecuteIfLastPointerWord*	E_LastPointer(uint8 uiWord, C65C64Func fFunction);
	static C65C64ExecuteIfLastTransferWord*	E_LastTransfer(uint8 uiWord, C65C64Func fFunction);
	static C65C64DoneInstruction*			DONE(void);
};


#endif // __65C64_INSTRUCTION_CYCLE_FACTORY_H__

