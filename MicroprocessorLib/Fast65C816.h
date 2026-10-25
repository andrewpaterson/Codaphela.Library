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
#ifndef __FAST_65C816_H__
#define __FAST_65C816_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "W65C816.h"


//Memory outside the flat block.  Each is one bus cycle the W65C816S drives with VDA or VPA high.
typedef uint8(*CFast65C816ReadByte)(void* pvContext, uint32 uiAddress);
typedef void(*CFast65C816WriteByte)(void* pvContext, uint32 uiAddress, uint8 uiValue);


#define FAST_65C816_NEXT_INSTRUCTION	0
#define FAST_65C816_NEXT_IRQ			1
#define FAST_65C816_NEXT_NMI			2


//An instruction level W65C816S.  It does what CW65C816 does an instruction at a time, without pins, bus
//cycles or a timeline.  One Step is one CW65CMeta::TickInstruction: an instruction, a hardware interrupt,
//one byte of a block move, or a cycle while WAI waits.  Interrupt inputs are sampled at the end of each
//Step, as the simulator samples them on an instruction's last cycle.
//
//It counts every clock cycle (internal ones too) and makes the memory accesses the simulator makes with
//VDA or VPA high, in the same order.  Internal cycles (VDA and VPA low) are only counted.  ABORTB and RDY
//are not modelled.  NMIB is edge triggered.
//
//Memory is a flat block at 00:0000 (puiMemory, uiMemorySize bytes, read and written directly) and the
//callbacks for every other address, and for a hole in the block (SetHole, where devices are).
class CFast65C816
{
protected:
	uint16					muiC;
	uint16					muiX;
	uint16					muiY;
	uint16					muiS;
	uint16					muiD;
	uint16					muiPC;
	uint8					muiDBR;
	uint8					muiPBR;
	uint8					muiP;		//M and X (bits 5 and 4) are always set in emulation mode.
	bool					mbE;

	bool					mbStopped;
	bool					mbWaiting;
	bool					mbIrq;
	bool					mbNmi;
	bool					mbNmiPending;
	uint8					muiNext;	//FAST_65C816_NEXT_*, decided at the end of each Step.

	uint64					muiCycles;
	uint32					muiWrap;	//How the byte after an operand's address wraps, see Next.

	uint8*					mpuiMemory;
	uint32					muiMemorySize;
	uint32					muiHoleStart;
	uint32					muiHoleSize;
	CFast65C816ReadByte		mfReadByte;
	CFast65C816WriteByte	mfWriteByte;
	void*					mpvContext;

public:
	void		Init(CFast65C816ReadByte fReadByte, CFast65C816WriteByte fWriteByte, void* pvContext, uint8* puiMemory, uint32 uiMemorySize);
	void		Kill(void);

	void		SetHole(uint32 uiStart, uint32 uiEnd);
	void		Reset(void);
	bool		Step(void);
	size		Run(size uiMaxInstructions);

	void		SetIRQ(bool bLow);
	void		SetNMI(bool bLow);

	uint16		GetC(void);
	uint16		GetX(void);
	uint16		GetY(void);
	uint16		GetS(void);
	uint16		GetD(void);
	uint8		GetDataBank(void);
	uint8		GetProgramBank(void);
	uint16		GetProgramCounter(void);
	uint32		GetProgramAddress(void);
	uint8		GetP(void);
	bool		IsEmulation(void);
	bool		IsStopped(void);
	bool		IsWaiting(void);
	uint64		NumCycles(void);

	bool		AppendDifferences(CW65C816* pcMPU, CChars* psz);
	void		AppendDifference(CChars* psz, const char* szName, uint64 uiOther, uint64 uiThis);

	//Only for tests.  They set registers directly, bypassing reset and instructions.
	void		TestSetC(uint16 uiC);
	void		TestSetX(uint16 uiX);
	void		TestSetY(uint16 uiY);
	void		TestSetS(uint16 uiS);
	void		TestSetD(uint16 uiD);
	void		TestSetDataBank(uint8 uiDataBank);
	void		TestSetProgramCounter(uint8 uiProgramBank, uint16 uiProgramCounter);
	void		TestSetP(uint8 uiP, bool bEmulation);

protected:
	void		Execute(void);
	void		Decide(void);
	void		Interrupt(uint16 uiNativeVector, uint16 uiEmulationVector, bool bHardware);

	uint8		Read(uint32 uiAddress);
	void		Write(uint32 uiAddress, uint8 uiValue);
	uint8		Fetch(void);
	uint16		Fetch16(void);
	uint32		Next(uint32 uiAddress);
	void		Push(uint8 uiValue);
	void		Push16(uint8 uiValue);
	uint8		Pull(void);
	uint8		Pull16(void);

	void		SetP(uint8 uiP);
	void		SetNZ8(uint8 uiValue);
	void		SetNZ16(uint16 uiValue);
	void		SetNZM(uint16 uiValue);
	void		SetNZX(uint16 uiValue);
	bool		IsM16(void);
	bool		IsX16(void);

	uint16		ReadM(uint32 uiAddress);
	uint16		ReadX(uint32 uiAddress);
	void		WriteM(uint32 uiAddress, uint16 uiValue);
	void		WriteX(uint32 uiAddress, uint16 uiValue);
	uint16		ImmediateM(void);
	uint16		ImmediateX(void);

	uint32		Direct(uint32 uiOffset);
	uint32		AddressAbsolute(void);
	uint32		AddressAbsoluteIndexed(uint16 uiIndex, bool bAlways);
	uint32		AddressLong(void);
	uint32		AddressLongX(void);
	uint32		AddressDirect(void);
	uint32		AddressDirectIndexed(uint16 uiIndex);
	uint32		AddressIndirect(void);
	uint32		AddressIndirectX(void);
	uint32		AddressIndirectY(bool bAlways);
	uint32		AddressIndirectLong(void);
	uint32		AddressIndirectLongY(void);
	uint32		AddressStack(void);
	uint32		AddressStackY(void);

	void		ORA(uint16 uiValue);
	void		AND(uint16 uiValue);
	void		EOR(uint16 uiValue);
	void		ADC(uint16 uiValue);
	void		SBC(uint16 uiValue);
	void		Decimal(uint16 uiValue, bool bSubtract);
	void		Compare(uint16 uiRegister, uint16 uiValue, bool b16);
	void		BIT(uint16 uiValue);
	void		LDA(uint16 uiValue);
	void		LDX(uint16 uiValue);
	void		LDY(uint16 uiValue);

	uint16		ASL(uint16 uiValue);
	uint16		LSR(uint16 uiValue);
	uint16		ROL(uint16 uiValue);
	uint16		ROR(uint16 uiValue);
	uint16		INC(uint16 uiValue);
	uint16		DEC(uint16 uiValue);
	uint16		TSB(uint16 uiValue);
	uint16		TRB(uint16 uiValue);
	void		ModifyAccumulator(uint8 uiOperation);
	void		Modify(uint32 uiAddress, uint8 uiOperation);
	uint16		Operate(uint8 uiOperation, uint16 uiValue);

	void		Branch(bool bTaken);
	void		SetIndex(uint16* puiIndex, uint16 uiValue);
	void		TransferToA(uint16 uiValue);
	void		BlockMove(int iStep);
};


#endif // __FAST_65C816_H__

