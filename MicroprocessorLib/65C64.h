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
#ifndef __65C64_H__
#define __65C64_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "65C64Decoded.h"
#include "65C64Pins.h"
#include "65Timeline.h"


//Status register P.
#define STATUS65_CARRY				0x0001
#define STATUS65_ZERO				0x0002
#define STATUS65_OVERFLOW			0x0004
#define STATUS65_NEGATIVE			0x0008
#define STATUS65_FLAG_G				0x0010
#define STATUS65_FLAG_F				0x0020
#define STATUS65_LEVEL_DISABLE		0x0040
#define STATUS65_EDGE_DISABLE		0x0080
#define STATUS65_WIDTH_Y_SHIFT		8
#define STATUS65_WIDTH_X_SHIFT		10
#define STATUS65_WIDTH_B_SHIFT		12
#define STATUS65_WIDTH_A_SHIFT		14

//Width codes in P.
#define WIDTH65_8					0
#define WIDTH65_16					1
#define WIDTH65_32					2
#define WIDTH65_64					3

#define RESET_65C64_STACK_POINTER	0x0001FFFF
#define VECTOR_65C64_BASE			0xFFFFF000
#define VECTOR_65C64_RESET			1023
#define VECTOR_65C64_EDGE_0			1022
#define VECTOR_65C64_LEVEL			1018

//stp's wake bits.  Edge interrupt i is bit i + 1.
#define WAKE65_LEVEL				0x01
#define WAKE65_EDGE_0				0x02

#define MAX_65C64_OPERAND_WORDS		4
#define MAX_65C64_TRANSFERS			8
#define MAX_65C64_TRANSFER_WORDS	40


//Memory an instruction reads or writes: a value of 1 to 8 bytes at any address.
struct S65C64Transfer
{
	uint32	uiAddress;
	uint8	uiBytes;
	uint64	uiValue;
};


//An aligned word of a transfer.  Instructions read their read words, execute, then write their write words.
struct S65C64TransferWord
{
	uint32	uiAddress;
	uint8	uiTransfer;
	uint8	uiLanes;
};


//The 65C64 from "65C64 Instruction Architecture.xlsx".  See 65C64Simulator.txt for the
//assumptions made where the spreadsheet is silent.
class C65Instruction;
class C65BusCycle;
class C65DataOperation;
class C65C64
{
protected:
	//Registers.
	uint64				muiA;
	uint64				muiB;
	uint64				muiX;
	uint64				muiY;
	uint32				muiS;
	uint32				muiV;
	uint32				muiW;
	uint32				muiC;
	uint16				muiP;
	uint32				muiInterruptDepth;

	//Internal data.
	uint64				muiDI;
	uint8				muiOperationBytes;
	uint16				mauiOperandWords[MAX_65C64_OPERAND_WORDS];
	uint32				muiPointerAddress;
	uint32				muiPointer;
	uint8				muiNumPointerWords;
	uint32				muiEffectiveAddress;
	uint32				muiVectorAddress;
	uint32				muiVector;
	S65C64Transfer		masTransfers[MAX_65C64_TRANSFERS];
	size				muiNumTransfers;
	S65C64TransferWord	masReadWords[MAX_65C64_TRANSFER_WORDS];
	size				muiNumReadWords;
	S65C64TransferWord	masWriteWords[MAX_65C64_TRANSFER_WORDS];
	size				muiNumWriteWords;
	E65Register			maePulled[MAX_65C64_TRANSFERS];

	//The instruction.
	uint16				muiOpcode;
	S65C64Decoded		msDecoded;
	C65Instruction*		mpcInstruction;
	int16				miCycle;
	int16				miNextCycle;
	bool				mbNextInstruction;
	bool				mbStopped;

	//Interrupts.
	bool				mbWaiting;
	uint8				muiWakeMask;
	uint8				muiPendingEdges;
	uint8				muiEdgeLow;
	bool				mbLevelInterrupt;

	//The bus.
	uint16				muiIOData;
	bool				mbPreviousClockLow;
	bool				mbPreviousClockHigh;
	bool				mbReset;
	C65C64Pins*			mpcPins;

public:
	void				Init(C65C64Pins* pcPins);
	void				Kill(void);

	//Clocking.
	void				InputTransition(C65Timeline* pcTimeline);
	void				ExecutePhi2Falling(C65Timeline* pcTimeline);
	void				ExecutePhi2Rising(C65Timeline* pcTimeline);
	void				Cycle(void);
	void				NextInstruction(void);
	void				DoneInstruction(void);
	void				ResetPulled(void);

	C65Instruction*		GetInstruction(void);
	C65BusCycle*		GetBusCycle(void);
	C65DataOperation*	GetDataOperation(void);
	S65C64Decoded*		GetDecoded(void);
	int16				GetCycle(void);
	bool				IsStopped(void);
	bool				IsWaiting(void);
	uint8				GetPendingEdges(void);

	//Called by the operations.
	void				ReadOpcode(void);
	void				IncrementProgramCounter(void);
	void				SetOperandWord(uint8 uiWord, uint16 uiData);
	uint32				GetPointerAddress(void);
	uint8				GetNumPointerWords(void);
	uint8				GetPointerWordLanes(uint8 uiWord);
	void				SetPointerWord(uint8 uiWord, uint16 uiData);
	size				GetNumReadWords(void);
	uint32				GetReadWordAddress(uint8 uiWord);
	uint8				GetReadWordLanes(uint8 uiWord);
	void				SetReadWord(uint8 uiWord, uint16 uiData);
	size				GetNumWriteWords(void);
	uint32				GetWriteWordAddress(uint8 uiWord);
	uint8				GetWriteWordLanes(uint8 uiWord);
	uint16				GetWriteWord(uint8 uiWord);
	uint32				GetVectorAddress(void);
	void				SetVectorWord(uint8 uiWord, uint16 uiData);
	uint8				GetNumWaitCycles(void);
	uint16				GetIOData(void);
	void				SetIOData(uint16 uiData);

	//Instruction steps.
	void				CalculateOperand(void);
	void				CalculateIndirectAddress(void);
	void				LoadVector(void);
	void				Reset(void);

	void				ILL(void);
	void				CPY(void);
	void				AND(void);
	void				OR(void);
	void				XOR(void);
	void				TST(void);
	void				ADD(void);
	void				SUB(void);
	void				CMP(void);
	void				JMP(void);
	void				ReturnSetup(void);
	void				RET(void);
	void				SEP(void);
	void				CEP(void);
	void				WTH(void);
	void				BRK(void);
	void				STP(void);
	void				GWTH(void);
	void				WTHX(void);
	void				BRANCH(void);
	void				BAL(void);
	void				NOP(void);
	void				SHL(void);
	void				SHR(void);
	void				SCP(void);
	void				NEG(void);
	void				ABS(void);
	void				FIND(void);
	void				BIST(void);
	void				STAB(void);
	void				STAT(void);
	void				BlockSetup(void);
	void				BLOCK(void);
	void				BCPY(void);
	void				BPRE(void);
	void				VectorSetup(void);
	void				VECTOR(void);
	void				PushOperand(void);
	void				PullOperandSetup(void);
	void				PullOperand(void);
	void				MUL(void);
	void				DIV(void);
	void				PushRegisters(void);
	void				PullRegistersSetup(void);
	void				PullRegisters(void);

	//Registers.
	uint64				GetRegister(E65Register eRegister);
	void				SetRegister(E65Register eRegister, uint64 uiValue, uint8 uiBytes);
	uint8				GetRegisterBytes(E65Register eRegister);
	uint32				GetRegisterAddress(E65Register eRegister);
	uint8				GetWidthCode(E65Register eRegister);
	void				SetWidthCode(E65Register eRegister, uint8 uiWidthCode);
	uint64				GetA(void);
	uint64				GetB(void);
	uint64				GetX(void);
	uint64				GetY(void);
	uint32				GetS(void);
	uint32				GetV(void);
	uint32				GetW(void);
	uint32				GetProgramCounter(void);
	uint16				GetP(void);
	uint32				GetInterruptDepth(void);
	bool				IsFlagSet(uint16 uiFlag);
	void				SetFlag(uint16 uiFlag, bool bValue);

	//Printing.
	void				GetMnemonicString(CChars* psz);
	void				GetCycleOperationString(CChars* psz);
	void				GetStatusString(CChars* psz);

	//Only for tests.  They set registers directly, bypassing reset and instructions.
	void				TestSetRegister(E65Register eRegister, uint64 uiValue);
	void				TestSetProgramCounter(uint32 uiC);
	void				TestSetP(uint16 uiP);
	void				TestStartInstruction(void);

protected:
	void				ResetInstructionState(void);
	void				SampleInterrupts(C65Timeline* pcTimeline);
	bool				EnterInterrupt(void);
	void				StartInterrupt(uint16 uiVector, uint16 uiDisable);
	void				AddTransfer(uint32 uiAddress, uint8 uiBytes, uint64 uiValue, bool bRead, bool bWrite);
	void				AddWords(S65C64TransferWord* pasWords, size* puiNumWords, uint32 uiAddress, uint8 uiBytes);
	void				PushTransfer(uint64 uiValue, uint8 uiBytes);
	void				PullTransfer(uint8 uiBytes);
	void				DataAddressKnown(void);
	uint8				CalculateOperationBytes(void);
	uint64				GetOperandValue(S65C64Operand* psOperand, uint8 uiBytes, bool bSigned);
	void				SetFirstValue(uint64 uiValue, uint8 uiBytes);
	void				ExecuteAlu(E65Instruction eInstruction);
	void				SetNegativeAndZero(uint64 uiValue, uint8 uiBytes);
	bool				GetBranchFlag(uint8 uiFlag);
	int64				GetBranchOffset(void);
	uint32				GetTarget(void);
	uint64				GetSubcopyIndex(uint16 uiField, bool bBits);
	void				SetFlagCode(uint8 uiFlag, bool bValue);
	uint64				GetBitIndex(void);
	void				BlockStep(uint64 uiData, size uiTransfer, bool bWrite, bool bSetNegativeAndZero);
	void				WriteSignalsPhi2Falling(C65Timeline* pcTimeline, C65DataOperation* pcDataOperation, C65BusCycle* pcBusCycle);
};


uint64	Mask65(uint8 uiBytes);
uint64	Extend65(uint64 uiValue, uint8 uiFromBytes, uint8 uiToBytes, bool bSigned);
uint8	WidthCodeToBytes65(uint8 uiWidthCode);
uint8	BytesToWidthCode65(uint64 uiBytes);


#endif // __65C64_H__

