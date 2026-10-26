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
#include "Timeline.h"


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


//Memory an instruction reads or writes: a value of 1 to 8 bytes at any address.  Its aligned words follow from the
//address and the bytes.  Instructions read the words of the transfers that are read, execute, then write the words
//of those that are written.
struct S65C64Transfer
{
	uint32	uiAddress;
	uint8	uiBytes;
	bool	bRead;
	bool	bWrite;
	uint64	uiValue;
};


//The 65C64 from "65C64 Instruction Architecture.xlsx".  See 65C64Simulator.txt for the
//assumptions made where the spreadsheet is silent.
class C65Instruction;
class C65BusCycle;
class C65DataOperation;


//Which instruction a core is running: the opcode's, one of the factory's own sequences or none yet.
enum E65C64InstructionKind
{
	IK65_None,
	IK65_Opcode,
	IK65_Reset,
	IK65_FetchNext,
	IK65_Interrupt,
	IK65_Wait,
};


//What the core's decoded instruction was made from: the opcode, an interrupt's vector or nothing yet.
enum E65C64DecodedKind
{
	DK65_None,
	DK65_Opcode,
	DK65_Interrupt,
};


//Everything a core remembers between clock edges, as plain data, for saving a running core and loading it again.  The
//decoded instruction and the instruction aren't kept; they're made again from their kinds, the opcode and the vector.
struct S65C64CoreState
{
	//Registers.
	uint64					uiA;
	uint64					uiB;
	uint64					uiX;
	uint64					uiY;
	uint32					uiS;
	uint32					uiV;
	uint32					uiW;
	uint32					uiC;
	uint16					uiP;
	uint32					uiInterruptDepth;

	//Internal data.
	uint64					uiDI;
	uint8					uiOperationBytes;
	uint16					auiOperandWords[MAX_65C64_OPERAND_WORDS];
	uint32					uiPointerAddress;
	uint32					uiPointer;
	uint8					uiNumPointerWords;
	uint32					uiEffectiveAddress;
	uint32					uiVectorAddress;
	uint32					uiVector;
	S65C64Transfer			asTransfers[MAX_65C64_TRANSFERS];
	E65Register				aePulled[MAX_65C64_TRANSFERS];
	size					uiNumTransfers;

	//The instruction.  The core runs msDecoded and mpcInstruction: their kinds are only filled in by GetState.
	uint16					uiOpcode;
	E65C64DecodedKind		eDecoded;
	uint16					uiDecodedVector;  //DK65_Interrupt.
	E65C64InstructionKind	eInstruction;
	int16					iCycle;
	int16					iNextCycle;
	bool					bNextInstruction;
	bool					bStopped;

	//Interrupts.
	bool					bWaiting;
	uint8					uiWakeMask;
	uint8					uiPendingEdges;
	uint8					uiEdgeLow;
	bool					bLevelInterrupt;

	//The bus.
	uint16					uiIOData;
	bool					bPreviousClockLow;
	bool					bPreviousClockHigh;
	bool					bReset;
};
class C65C64
{
protected:
	S65C64CoreState		msState;
	S65C64Decoded		msDecoded;
	C65Instruction*		mpcInstruction;
	C65C64Pins*			mpcPins;

public:
	void				Init(C65C64Pins* pcPins);
	void				Kill(void);

	void				GetState(S65C64CoreState* psState);
	void				SetState(S65C64CoreState* psState);

	//Clocking.
	void				InputTransition(CTimeline* pcTimeline);
	void				ExecutePhi2Falling(CTimeline* pcTimeline);
	void				ExecutePhi2Rising(CTimeline* pcTimeline);
	void				Cycle(void);
	void				NextInstruction(void);
	void				DoneInstruction(void);
	void				ResetPulled(void);

	C65Instruction*		GetInstruction(void);
	C65BusCycle*		GetBusCycle(void);
	C65DataOperation*	GetDataOperation(void);
	S65C64Decoded*		GetDecoded(void);
	int16				GetCycle(void);
	uint16				GetOpcode(void);
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
	void				SampleInterrupts(CTimeline* pcTimeline);
	bool				EnterInterrupt(void);
	void				StartInterrupt(uint16 uiVector, uint16 uiDisable);
	void				DecodeInterrupt(uint16 uiVector);
	void				AddTransfer(uint32 uiAddress, uint8 uiBytes, uint64 uiValue, bool bRead, bool bWrite);
	size				CountTransferWords(bool bWrite);
	S65C64Transfer*		FindTransferWord(uint8 uiWord, bool bWrite, uint32* puiWordAddress);
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
	void				WriteSignalsPhi2Falling(CTimeline* pcTimeline, C65DataOperation* pcDataOperation, C65BusCycle* pcBusCycle);
};


uint64	Mask65(uint8 uiBytes);
uint64	Extend65(uint64 uiValue, uint8 uiFromBytes, uint8 uiToBytes, bool bSigned);
uint8	WidthCodeToBytes65(uint8 uiWidthCode);
uint8	BytesToWidthCode65(uint64 uiBytes);
uint8	BitsToWidthCode65(uint64 uiBits);


#endif // __65C64_H__

