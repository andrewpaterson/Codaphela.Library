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
#ifndef __FAST_65C64_H__
#define __FAST_65C64_H__
#include "BaseLib/PrimitiveTypes.h"
#include "65C64.h"


//Memory outside the flat block.  ReadWord is given the aligned address the bus would carry and returns the
//two bytes there.  WriteByte is called for each byte written.
typedef uint16(*CFast65C64ReadWord)(void* pvContext, uint32 uiAddress);
typedef void(*CFast65C64WriteByte)(void* pvContext, uint32 uiAddress, uint8 uiValue);


//An instruction level 65C64.  It does what C65C64 does an instruction at a time, without pins, bus cycles or
//a timeline (see 65C64Simulator.txt).  One Step is one C65C64Meta::TickInstruction: block, vector, multiply
//and divide steps are each a Step, and an interrupt is taken at the end of the Step it follows.  It counts
//the clock cycles C65C64 would take (including internal and waiting cycles) and makes the same memory
//accesses, so memory sees the same reads and writes.
//
//Memory is a flat block at 0000:0000 (puiMemory, uiMemorySize bytes, read and written directly) and the
//callbacks for every other address.  The host must be little endian.
struct SFast65C64Op;
class CFast65C64
{
protected:
	uint64				mauiRegisters[8];	//By E65Register.  The zero register is always 0, S, V and W are 32 bits.
	uint64				mauiMasks[8];		//Each register's width.
	uint8				mauiBytes[8];
	uint32				muiC;
	uint16				muiP;
	uint32				muiInterruptDepth;
	bool				mbStopped;
	bool				mbWaiting;

	uint8				muiWakeMask;
	uint8				muiPendingEdges;
	uint8				muiEdgeLow;
	bool				mbLevelInterrupt;

	uint64				muiCycles;

	uint8*				mpuiMemory;
	uint32				muiMemorySize;
	uint32				muiWordLimit;
	uint32				muiQuadLimit;
	CFast65C64ReadWord	mfReadWord;
	CFast65C64WriteByte	mfWriteByte;
	void*				mpvContext;

public:
	void		Init(CFast65C64ReadWord fReadWord, CFast65C64WriteByte fWriteByte, void* pvContext, uint8* puiMemory, uint32 uiMemorySize);
	void		Kill(void);

	void		Reset(void);
	bool		Step(void);
	size		Run(size uiMaxInstructions);

	void		SetEdgeInterrupt(size uiInterrupt, bool bLow);
	void		SetLevelInterrupt(bool bLow);

	uint64		GetA(void);
	uint64		GetB(void);
	uint64		GetX(void);
	uint64		GetY(void);
	uint32		GetS(void);
	uint32		GetV(void);
	uint32		GetW(void);
	uint32		GetProgramCounter(void);
	uint16		GetP(void);
	uint32		GetInterruptDepth(void);
	bool		IsFlagSet(uint16 uiFlag);
	bool		IsStopped(void);
	bool		IsWaiting(void);
	uint8		GetPendingEdges(void);
	uint64		NumCycles(void);

	//Only for tests.  They set registers directly, bypassing reset and instructions.
	void		TestSetRegister(E65Register eRegister, uint64 uiValue);
	void		TestSetProgramCounter(uint32 uiC);
	void		TestSetP(uint16 uiP);

protected:
	void		Execute(void);
	void		SetWidths(void);
	void		SetRegister(uint8 uiRegister, uint64 uiValue, uint8 uiBytes);
	uint32		GetRegisterAddress(uint8 uiRegister);
	void		SetNegativeAndZero(uint64 uiValue, uint8 uiBytes);
	void		SetFlag(uint16 uiFlag, bool bValue);
	bool		GetFlagCode(uint8 uiFlag);
	void		SetFlagCode(uint8 uiFlag, bool bValue);
	void		SetWidthCode(uint8 uiRegister, uint8 uiWidthCode);

	uint16		ReadWord(uint32 uiAddress);
	uint16		FetchWord(void);
	uint64		ReadValue(uint32 uiAddress, uint8 uiBytes);
	void		WriteValue(uint32 uiAddress, uint8 uiBytes, uint64 uiValue);
	void		Push(uint8 uiStack, uint64 uiValue, uint8 uiBytes);
	uint32		Pull(uint8 uiStack, uint8 uiBytes);
	uint32		ReadVector(uint16 uiVector);
	bool		EnterInterrupt(void);
	void		StartInterrupt(uint16 uiVector, uint16 uiDisable);

	uint32		BaseAddress(SFast65C64Op* psOp, uint16* auiWords);
	uint32		DataAddress(SFast65C64Op* psOp, uint16* auiWords);
	uint64		OperandValue(uint8 uiKind, uint8 uiRegister, SFast65C64Op* psOp, uint8 uiBytes, bool bSigned, uint64 uiImmediate, uint64 uiMemory);
	uint64		SubcopyIndex(uint16 uiField, bool bBits);
	uint64		BitIndex(SFast65C64Op* psOp, uint16* auiWords);
	void		BlockStep(uint8 uiBytes, bool bVector, bool bDecrement, bool bDecrementW, bool bAllowStop, uint8 uiStopStyle);

	void		Alu(SFast65C64Op* psOp, uint16* auiWords);
	void		Jump(SFast65C64Op* psOp, uint16* auiWords);
	void		Return(SFast65C64Op* psOp);
	void		Subcopy(SFast65C64Op* psOp, uint16* auiWords);
	void		Find(SFast65C64Op* psOp);
	void		StatusToBit(SFast65C64Op* psOp, uint16* auiWords);
	void		Block(SFast65C64Op* psOp);
	void		Vector(SFast65C64Op* psOp);
	void		Multiply(void);
	void		Divide(void);
	void		PushRegisters(SFast65C64Op* psOp);
	void		PullRegisters(SFast65C64Op* psOp);
	void		PullOperand(SFast65C64Op* psOp, uint16* auiWords);
};


#endif // __FAST_65C64_H__

