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
#include "CPUFlags.h"
#include "Fast65C816.h"


#define FAST_816_C		STATUS_CARRY
#define FAST_816_Z		STATUS_ZERO
#define FAST_816_I		STATUS_INTERRUPT_DISABLE
#define FAST_816_D		STATUS_DECIMAL
#define FAST_816_X		STATUS_INDEX_WIDTH
#define FAST_816_M		STATUS_ACCUMULATOR_WIDTH
#define FAST_816_V		STATUS_OVERFLOW
#define FAST_816_N		STATUS_NEGATIVE

//The byte after an operand's address: anywhere in the 16MB, within bank 0 or within the direct page.
#define FAST_816_WRAP_LONG	0xFFFFFF
#define FAST_816_WRAP_BANK	0xFFFF
#define FAST_816_WRAP_PAGE	0xFF


//Read-modify-write operations.
enum EFast65C816Modify
{
	F816M_ASL,
	F816M_LSR,
	F816M_ROL,
	F816M_ROR,
	F816M_INC,
	F816M_DEC,
	F816M_TSB,
	F816M_TRB,
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::Init(CFast65C816ReadByte fReadByte, CFast65C816WriteByte fWriteByte, void* pvContext, uint8* puiMemory, uint32 uiMemorySize)
{
	muiC = 0;
	muiX = 0;
	muiY = 0;
	muiS = 0x1FF;
	muiD = 0;
	muiPC = 0;
	muiDBR = 0;
	muiPBR = 0;
	muiP = FAST_816_M | FAST_816_X | FAST_816_I;
	mbE = true;

	mbStopped = false;
	mbWaiting = false;
	mbIrq = false;
	mbNmi = false;
	mbNmiPending = false;
	muiNext = FAST_65C816_NEXT_INSTRUCTION;

	muiCycles = 0;
	muiWrap = FAST_816_WRAP_LONG;

	mfReadByte = fReadByte;
	mfWriteByte = fWriteByte;
	mpvContext = pvContext;
	mfSignal = NULL;
	mpuiMemory = puiMemory;
	muiMemorySize = puiMemory ? uiMemorySize : 0;
	muiHoleStart = 0;
	muiHoleSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::Kill(void)
{
	mfReadByte = NULL;
	mfWriteByte = NULL;
	mpvContext = NULL;
	mfSignal = NULL;
	mpuiMemory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Addresses uiStart to uiEnd - 1 in the flat block go to the callbacks (memory mapped devices).
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::SetHole(uint32 uiStart, uint32 uiEnd)
{
	muiHoleStart = uiStart;
	muiHoleSize = uiEnd - uiStart;
}


//////////////////////////////////////////////////////////////////////////
//
// fSignal is called with the FAST_65C816_SIGNAL_* events, with the context given to Init.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::SetSignal(CFast65C816Signal fSignal)
{
	mfSignal = fSignal;
}


//////////////////////////////////////////////////////////////////////////
//
// Emulation mode, two internal cycles, three with S counting down, then the reset vector.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::Reset(void)
{
	uint8	uiLow;
	uint8	uiHigh;

	mbStopped = false;
	mbWaiting = false;
	mbNmiPending = false;

	mbE = true;
	muiP = (uint8)((muiP | FAST_816_M | FAST_816_X | FAST_816_I) & ~FAST_816_D);
	muiX &= 0xFF;
	muiY &= 0xFF;
	muiD = 0;
	muiDBR = 0;
	muiPBR = 0;
	muiS = (uint16)(0x100 | ((muiS - 3) & 0xFF));
	muiCycles += 5;

	if (mfSignal)
	{
		mfSignal(mpvContext, FAST_65C816_SIGNAL_VECTOR_PULL);
	}
	uiLow = Read(0xFFFC);
	uiHigh = Read(0xFFFD);
	muiPC = (uint16)(uiLow | (uiHigh << 8));
	Decide();
}


//////////////////////////////////////////////////////////////////////////
//
// One CW65CMeta::TickInstruction.  Returns false if the processor stopped.
//
//////////////////////////////////////////////////////////////////////////
bool CFast65C816::Step(void)
{
	if (mbStopped)
	{
		return false;
	}

	if (mbWaiting)
	{
		if (!mbNmiPending && !mbIrq)
		{
			muiCycles++;
			return true;
		}

		//WAI ends on IRQB even when I is set, then the next instruction runs.
		mbWaiting = false;
		Decide();
	}

	if (muiNext == FAST_65C816_NEXT_INSTRUCTION)
	{
		Execute();
	}
	else if (muiNext == FAST_65C816_NEXT_NMI)
	{
		mbNmiPending = false;
		Interrupt(0xFFEA, 0xFFFA, true);
	}
	else
	{
		Interrupt(0xFFEE, 0xFFFE, true);
	}

	if (mbE)
	{
		//Instructions that use all 16 bits of S (caveat 7.1) may leave page 1, SH is 01 again afterwards.
		muiS = (uint16)(0x100 | (muiS & 0xFF));
	}

	if (mbStopped)
	{
		return false;
	}
	if (!mbWaiting)
	{
		Decide();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Steps until the processor stops.  Returns the number of steps, counting the one that stopped.
//
//////////////////////////////////////////////////////////////////////////
size CFast65C816::Run(size uiMaxInstructions)
{
	size	i;

	for (i = 0; i < uiMaxInstructions; i++)
	{
		if (!Step())
		{
			return i + 1;
		}
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
// IRQB is level sensitive.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::SetIRQ(bool bLow)
{
	mbIrq = bLow;
}


//////////////////////////////////////////////////////////////////////////
//
// NMIB is latched when it falls.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::SetNMI(bool bLow)
{
	if (bLow && !mbNmi)
	{
		mbNmiPending = true;
	}
	mbNmi = bLow;
}


//////////////////////////////////////////////////////////////////////////
//
// What the next Step does, from the inputs at the end of this one.  NMI before IRQ.
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::Decide(void)
{
	if (mbNmiPending)
	{
		muiNext = FAST_65C816_NEXT_NMI;
	}
	else if (mbIrq && !(muiP & FAST_816_I))
	{
		muiNext = FAST_65C816_NEXT_IRQ;
	}
	else
	{
		muiNext = FAST_65C816_NEXT_INSTRUCTION;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A hardware interrupt (bHardware) or BRK and COP after their opcode.  PBR is only pushed in native mode.
// In emulation mode the pushed P has B (bit 4) clear for a hardware interrupt and set for BRK and COP.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::Interrupt(uint16 uiNativeVector, uint16 uiEmulationVector, bool bHardware)
{
	uint8	uiP;
	uint8	uiLow;
	uint8	uiHigh;
	uint16	uiVector;

	if (bHardware)
	{
		//The opcode is fetched and ignored.
		Read(((uint32)muiPBR << 16) | muiPC);
		muiCycles++;
	}
	else
	{
		//The signature byte.
		Fetch();
	}

	if (!mbE)
	{
		Push(muiPBR);
	}
	Push((uint8)(muiPC >> 8));
	Push((uint8)muiPC);

	uiP = muiP;
	if (mbE && bHardware)
	{
		uiP &= ~FAST_816_X;
	}
	Push(uiP);

	muiP = (uint8)((muiP | FAST_816_I) & ~FAST_816_D);
	muiPBR = 0;

	uiVector = mbE ? uiEmulationVector : uiNativeVector;
	if (mfSignal)
	{
		mfSignal(mpvContext, FAST_65C816_SIGNAL_VECTOR_PULL);
	}
	uiLow = Read(uiVector);
	uiHigh = Read(uiVector + 1);
	muiPC = (uint16)(uiLow | (uiHigh << 8));
}


//////////////////////////////////////////////////////////////////////////
//
// A bus cycle with VDA or VPA high.
//
//////////////////////////////////////////////////////////////////////////
inline uint8 CFast65C816::Read(uint32 uiAddress)
{
	muiCycles++;
	if ((uiAddress < muiMemorySize) && ((uiAddress - muiHoleStart) >= muiHoleSize))
	{
		return mpuiMemory[uiAddress];
	}
	return mfReadByte(mpvContext, uiAddress);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::Write(uint32 uiAddress, uint8 uiValue)
{
	muiCycles++;
	if ((uiAddress < muiMemorySize) && ((uiAddress - muiHoleStart) >= muiHoleSize))
	{
		mpuiMemory[uiAddress] = uiValue;
		return;
	}
	mfWriteByte(mpvContext, uiAddress, uiValue);
}


//////////////////////////////////////////////////////////////////////////
//
// The byte at PBR:PC.  PC wraps within the bank.
//
//////////////////////////////////////////////////////////////////////////
inline uint8 CFast65C816::Fetch(void)
{
	uint8	uiValue;

	uiValue = Read(((uint32)muiPBR << 16) | muiPC);
	muiPC++;
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C816::Fetch16(void)
{
	uint8	uiLow;
	uint8	uiHigh;

	uiLow = Fetch();
	uiHigh = Fetch();
	return (uint16)(uiLow | (uiHigh << 8));
}


//////////////////////////////////////////////////////////////////////////
//
// The address of the byte after uiAddress, wrapped as muiWrap says.
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C816::Next(uint32 uiAddress)
{
	return (uiAddress & ~muiWrap) | ((uiAddress + 1) & muiWrap);
}


//////////////////////////////////////////////////////////////////////////
//
// The 6502 instructions' stack stays in page 1 in emulation mode.
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::Push(uint8 uiValue)
{
	Write(muiS, uiValue);
	if (mbE)
	{
		muiS = (uint16)(0x100 | ((muiS - 1) & 0xFF));
	}
	else
	{
		muiS--;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The W65C816S instructions' stack uses all 16 bits of S (caveat 7.1).
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::Push16(uint8 uiValue)
{
	Write(muiS, uiValue);
	muiS--;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint8 CFast65C816::Pull(void)
{
	if (mbE)
	{
		muiS = (uint16)(0x100 | ((muiS + 1) & 0xFF));
	}
	else
	{
		muiS++;
	}
	return Read(muiS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint8 CFast65C816::Pull16(void)
{
	muiS++;
	return Read(muiS);
}


//////////////////////////////////////////////////////////////////////////
//
// M and X are always set in emulation mode.  Setting X clears XH and YH.
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::SetP(uint8 uiP)
{
	if (mbE)
	{
		uiP |= FAST_816_M | FAST_816_X;
	}
	muiP = uiP;
	if (uiP & FAST_816_X)
	{
		muiX &= 0xFF;
		muiY &= 0xFF;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::SetNZ8(uint8 uiValue)
{
	muiP = (uint8)((muiP & ~(FAST_816_N | FAST_816_Z)) | (uiValue & FAST_816_N) | (uiValue == 0 ? FAST_816_Z : 0));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::SetNZ16(uint16 uiValue)
{
	muiP = (uint8)((muiP & ~(FAST_816_N | FAST_816_Z)) | ((uiValue >> 8) & FAST_816_N) | (uiValue == 0 ? FAST_816_Z : 0));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::SetNZM(uint16 uiValue)
{
	if (IsM16())
	{
		SetNZ16(uiValue);
	}
	else
	{
		SetNZ8((uint8)uiValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::SetNZX(uint16 uiValue)
{
	if (IsX16())
	{
		SetNZ16(uiValue);
	}
	else
	{
		SetNZ8((uint8)uiValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline bool CFast65C816::IsM16(void)
{
	return !(muiP & FAST_816_M);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline bool CFast65C816::IsX16(void)
{
	return !(muiP & FAST_816_X);
}


//////////////////////////////////////////////////////////////////////////
//
// The memory operand in the accumulator's width, low byte first.
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C816::ReadM(uint32 uiAddress)
{
	uint8	uiLow;

	uiLow = Read(uiAddress);
	if (IsM16())
	{
		return (uint16)(uiLow | (Read(Next(uiAddress)) << 8));
	}
	return uiLow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C816::ReadX(uint32 uiAddress)
{
	uint8	uiLow;

	uiLow = Read(uiAddress);
	if (IsX16())
	{
		return (uint16)(uiLow | (Read(Next(uiAddress)) << 8));
	}
	return uiLow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::WriteM(uint32 uiAddress, uint16 uiValue)
{
	Write(uiAddress, (uint8)uiValue);
	if (IsM16())
	{
		Write(Next(uiAddress), (uint8)(uiValue >> 8));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::WriteX(uint32 uiAddress, uint16 uiValue)
{
	Write(uiAddress, (uint8)uiValue);
	if (IsX16())
	{
		Write(Next(uiAddress), (uint8)(uiValue >> 8));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C816::ImmediateM(void)
{
	if (IsM16())
	{
		return Fetch16();
	}
	return Fetch();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C816::ImmediateX(void)
{
	if (IsX16())
	{
		return Fetch16();
	}
	return Fetch();
}


//////////////////////////////////////////////////////////////////////////
//
// A direct page byte, uiOffset from D.  In emulation mode with DL zero it wraps within the page, otherwise
// within bank 0.  Sets muiWrap to match.
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C816::Direct(uint32 uiOffset)
{
	if (mbE && ((muiD & 0xFF) == 0))
	{
		muiWrap = FAST_816_WRAP_PAGE;
		return muiD | (uiOffset & 0xFF);
	}
	muiWrap = FAST_816_WRAP_BANK;
	return (muiD + uiOffset) & 0xFFFF;
}


//////////////////////////////////////////////////////////////////////////
//
// a
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C816::AddressAbsolute(void)
{
	muiWrap = FAST_816_WRAP_LONG;
	return ((uint32)muiDBR << 16) | Fetch16();
}


//////////////////////////////////////////////////////////////////////////
//
// a,x and a,y.  The internal cycle is taken when the index crosses a page, the index is 16 bits or the
// instruction writes (bAlways).
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C816::AddressAbsoluteIndexed(uint16 uiIndex, bool bAlways)
{
	uint16	uiAddress;

	uiAddress = Fetch16();
	if (bAlways || IsX16() || (((uiAddress & 0xFF) + (uiIndex & 0xFF)) > 0xFF))
	{
		muiCycles++;
	}
	muiWrap = FAST_816_WRAP_LONG;
	return ((((uint32)muiDBR << 16) | uiAddress) + uiIndex) & 0xFFFFFF;
}


//////////////////////////////////////////////////////////////////////////
//
// al
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C816::AddressLong(void)
{
	uint16	uiAddress;

	uiAddress = Fetch16();
	muiWrap = FAST_816_WRAP_LONG;
	return ((uint32)Fetch() << 16) | uiAddress;
}


//////////////////////////////////////////////////////////////////////////
//
// al,x
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C816::AddressLongX(void)
{
	return (AddressLong() + muiX) & 0xFFFFFF;
}


//////////////////////////////////////////////////////////////////////////
//
// d.  An internal cycle when DL is not zero (note 2).
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C816::AddressDirect(void)
{
	uint8	uiOffset;

	uiOffset = Fetch();
	if (muiD & 0xFF)
	{
		muiCycles++;
	}
	return Direct(uiOffset);
}


//////////////////////////////////////////////////////////////////////////
//
// d,x and d,y
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C816::AddressDirectIndexed(uint16 uiIndex)
{
	uint8	uiOffset;

	uiOffset = Fetch();
	muiCycles += (muiD & 0xFF) ? 2 : 1;
	return Direct((uint32)uiOffset + uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// (d)
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C816::AddressIndirect(void)
{
	uint8	uiOffset;
	uint8	uiLow;
	uint8	uiHigh;

	uiOffset = Fetch();
	if (muiD & 0xFF)
	{
		muiCycles++;
	}
	uiLow = Read(Direct(uiOffset));
	uiHigh = Read(Direct((uint32)uiOffset + 1));
	muiWrap = FAST_816_WRAP_LONG;
	return ((uint32)muiDBR << 16) | uiLow | (uiHigh << 8);
}


//////////////////////////////////////////////////////////////////////////
//
// (d,x)
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C816::AddressIndirectX(void)
{
	uint8	uiOffset;
	uint8	uiLow;
	uint8	uiHigh;

	uiOffset = Fetch();
	muiCycles += (muiD & 0xFF) ? 2 : 1;
	uiLow = Read(Direct((uint32)uiOffset + muiX));
	uiHigh = Read(Direct((uint32)uiOffset + muiX + 1));
	muiWrap = FAST_816_WRAP_LONG;
	return ((uint32)muiDBR << 16) | uiLow | (uiHigh << 8);
}


//////////////////////////////////////////////////////////////////////////
//
// (d),y.  The internal cycle as for a,y.
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C816::AddressIndirectY(bool bAlways)
{
	uint8	uiOffset;
	uint8	uiLow;
	uint8	uiHigh;

	uiOffset = Fetch();
	if (muiD & 0xFF)
	{
		muiCycles++;
	}
	uiLow = Read(Direct(uiOffset));
	uiHigh = Read(Direct((uint32)uiOffset + 1));
	if (bAlways || IsX16() || ((uiLow + (muiY & 0xFF)) > 0xFF))
	{
		muiCycles++;
	}
	muiWrap = FAST_816_WRAP_LONG;
	return ((((uint32)muiDBR << 16) | uiLow | (uiHigh << 8)) + muiY) & 0xFFFFFF;
}


//////////////////////////////////////////////////////////////////////////
//
// [d].  The pointer never wraps within the page.
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C816::AddressIndirectLong(void)
{
	uint8	uiOffset;
	uint32	uiPointer;
	uint8	uiLow;
	uint8	uiHigh;
	uint8	uiBank;

	uiOffset = Fetch();
	if (muiD & 0xFF)
	{
		muiCycles++;
	}
	uiPointer = muiD + uiOffset;
	uiLow = Read(uiPointer & 0xFFFF);
	uiHigh = Read((uiPointer + 1) & 0xFFFF);
	uiBank = Read((uiPointer + 2) & 0xFFFF);
	muiWrap = FAST_816_WRAP_LONG;
	return ((uint32)uiBank << 16) | (uiHigh << 8) | uiLow;
}


//////////////////////////////////////////////////////////////////////////
//
// [d],y
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C816::AddressIndirectLongY(void)
{
	return (AddressIndirectLong() + muiY) & 0xFFFFFF;
}


//////////////////////////////////////////////////////////////////////////
//
// d,s
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C816::AddressStack(void)
{
	uint8	uiOffset;

	uiOffset = Fetch();
	muiCycles++;
	muiWrap = FAST_816_WRAP_BANK;
	return (muiS + uiOffset) & 0xFFFF;
}


//////////////////////////////////////////////////////////////////////////
//
// (d,s),y
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C816::AddressStackY(void)
{
	uint32	uiPointer;
	uint8	uiLow;
	uint8	uiHigh;

	uiPointer = AddressStack();
	uiLow = Read(uiPointer);
	uiHigh = Read((uiPointer + 1) & 0xFFFF);
	muiCycles++;
	muiWrap = FAST_816_WRAP_LONG;
	return ((((uint32)muiDBR << 16) | uiLow | (uiHigh << 8)) + muiY) & 0xFFFFFF;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::LDA(uint16 uiValue)
{
	if (IsM16())
	{
		muiC = uiValue;
		SetNZ16(uiValue);
	}
	else
	{
		muiC = (uint16)((muiC & 0xFF00) | (uiValue & 0xFF));
		SetNZ8((uint8)uiValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::LDX(uint16 uiValue)
{
	muiX = uiValue;
	SetNZX(uiValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::LDY(uint16 uiValue)
{
	muiY = uiValue;
	SetNZX(uiValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::ORA(uint16 uiValue)
{
	LDA(muiC | uiValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::AND(uint16 uiValue)
{
	LDA(muiC & uiValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::EOR(uint16 uiValue)
{
	LDA(muiC ^ uiValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::ADC(uint16 uiValue)
{
	uint32	uiResult;
	uint32	uiAccumulator;

	if (muiP & FAST_816_D)
	{
		Decimal(uiValue, false);
		return;
	}

	if (IsM16())
	{
		uiAccumulator = muiC;
		uiResult = uiAccumulator + uiValue + (muiP & FAST_816_C);
		muiP = (uint8)(muiP & ~(FAST_816_C | FAST_816_V));
		muiP |= (uiResult > 0xFFFF) ? FAST_816_C : 0;
		muiP |= (~(uiAccumulator ^ uiValue) & (uiAccumulator ^ uiResult) & 0x8000) ? FAST_816_V : 0;
		muiC = (uint16)uiResult;
		SetNZ16(muiC);
	}
	else
	{
		uiAccumulator = muiC & 0xFF;
		uiValue &= 0xFF;
		uiResult = uiAccumulator + uiValue + (muiP & FAST_816_C);
		muiP = (uint8)(muiP & ~(FAST_816_C | FAST_816_V));
		muiP |= (uiResult > 0xFF) ? FAST_816_C : 0;
		muiP |= (~(uiAccumulator ^ uiValue) & (uiAccumulator ^ uiResult) & 0x80) ? FAST_816_V : 0;
		muiC = (uint16)((muiC & 0xFF00) | (uiResult & 0xFF));
		SetNZ8((uint8)uiResult);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::SBC(uint16 uiValue)
{
	if (muiP & FAST_816_D)
	{
		Decimal(uiValue, true);
		return;
	}
	ADC((uint16)~uiValue);
}


//////////////////////////////////////////////////////////////////////////
//
// ADC and SBC with D set, as CW65CCore::ExecuteDecimal.  Each nibble is added with the carry from the one
// below and adjusted, SBC adds the complement of the operand.  V comes from the result before the top
// nibble is adjusted.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::Decimal(uint16 uiValue, bool bSubtract)
{
	int32	iNibbles;
	int32	iAccumulator;
	int32	iOperand;
	int32	iResult;
	int32	iMask;
	int32	iShift;
	int32	iLimit;
	int32	iNibble;
	bool	bCarry;

	iNibbles = IsM16() ? 4 : 2;
	iMask = (1 << (iNibbles * 4)) - 1;
	iAccumulator = muiC & iMask;
	iOperand = uiValue & iMask;
	if (bSubtract)
	{
		iOperand = ~iOperand & iMask;
	}
	bCarry = (muiP & FAST_816_C) != 0;

	iResult = 0;
	for (iNibble = 0; iNibble < iNibbles; iNibble++)
	{
		iShift = iNibble * 4;
		iResult = (iAccumulator & (0xf << iShift)) + (iOperand & (0xf << iShift)) + ((bCarry ? 1 : 0) << iShift) + (iResult & ((1 << iShift) - 1));
		if (iNibble < iNibbles - 1)
		{
			iLimit = (0x10 << iShift) - 1;
			if (!bSubtract && (iResult > (0xa << iShift) - 1))
			{
				iResult += 0x6 << iShift;
			}
			else if (bSubtract && (iResult <= iLimit))
			{
				iResult -= 0x6 << iShift;
			}
			bCarry = iResult > iLimit;
		}
	}

	iShift = (iNibbles - 1) * 4;
	muiP = (uint8)(muiP & ~(FAST_816_C | FAST_816_V));
	if ((~(iAccumulator ^ iOperand) & (iAccumulator ^ iResult) & (0x8 << iShift)) != 0)
	{
		muiP |= FAST_816_V;
	}
	if (!bSubtract && (iResult > (0xa << iShift) - 1))
	{
		iResult += 0x6 << iShift;
	}
	else if (bSubtract && (iResult <= iMask))
	{
		iResult -= 0x6 << iShift;
	}
	if (iResult > iMask)
	{
		muiP |= FAST_816_C;
	}
	LDA((uint16)(iResult & iMask));
}


//////////////////////////////////////////////////////////////////////////
//
// CMP, CPX and CPY.
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::Compare(uint16 uiRegister, uint16 uiValue, bool b16)
{
	if (b16)
	{
		muiP = (uint8)((muiP & ~FAST_816_C) | ((uiRegister >= uiValue) ? FAST_816_C : 0));
		SetNZ16((uint16)(uiRegister - uiValue));
	}
	else
	{
		uiRegister &= 0xFF;
		uiValue &= 0xFF;
		muiP = (uint8)((muiP & ~FAST_816_C) | ((uiRegister >= uiValue) ? FAST_816_C : 0));
		SetNZ8((uint8)(uiRegister - uiValue));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// BIT other than immediate: N and V from the operand, Z from A AND the operand.
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::BIT(uint16 uiValue)
{
	uint16	uiTop;

	uiTop = IsM16() ? (uint16)(uiValue >> 8) : uiValue;
	muiP = (uint8)((muiP & ~(FAST_816_N | FAST_816_V | FAST_816_Z)) | (uiTop & (FAST_816_N | FAST_816_V)));
	if (IsM16() ? ((muiC & uiValue) == 0) : (((muiC & uiValue) & 0xFF) == 0))
	{
		muiP |= FAST_816_Z;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C816::ASL(uint16 uiValue)
{
	if (IsM16())
	{
		muiP = (uint8)((muiP & ~FAST_816_C) | (uiValue >> 15));
		uiValue = (uint16)(uiValue << 1);
	}
	else
	{
		muiP = (uint8)((muiP & ~FAST_816_C) | ((uiValue >> 7) & 1));
		uiValue = (uint16)((uiValue << 1) & 0xFF);
	}
	SetNZM(uiValue);
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C816::LSR(uint16 uiValue)
{
	muiP = (uint8)((muiP & ~FAST_816_C) | (uiValue & 1));
	uiValue >>= 1;
	SetNZM(uiValue);
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C816::ROL(uint16 uiValue)
{
	uint16	uiCarry;

	uiCarry = muiP & FAST_816_C;
	if (IsM16())
	{
		muiP = (uint8)((muiP & ~FAST_816_C) | (uiValue >> 15));
		uiValue = (uint16)((uiValue << 1) | uiCarry);
	}
	else
	{
		muiP = (uint8)((muiP & ~FAST_816_C) | ((uiValue >> 7) & 1));
		uiValue = (uint16)(((uiValue << 1) | uiCarry) & 0xFF);
	}
	SetNZM(uiValue);
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C816::ROR(uint16 uiValue)
{
	uint16	uiCarry;

	uiCarry = muiP & FAST_816_C;
	muiP = (uint8)((muiP & ~FAST_816_C) | (uiValue & 1));
	uiValue = (uint16)((uiValue >> 1) | (uiCarry << (IsM16() ? 15 : 7)));
	SetNZM(uiValue);
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C816::INC(uint16 uiValue)
{
	uiValue = IsM16() ? (uint16)(uiValue + 1) : (uint16)((uiValue + 1) & 0xFF);
	SetNZM(uiValue);
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C816::DEC(uint16 uiValue)
{
	uiValue = IsM16() ? (uint16)(uiValue - 1) : (uint16)((uiValue - 1) & 0xFF);
	SetNZM(uiValue);
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C816::TSB(uint16 uiValue)
{
	uint16	uiMask;

	uiMask = IsM16() ? 0xFFFF : 0xFF;
	muiP = (uint8)((muiP & ~FAST_816_Z) | (((muiC & uiValue & uiMask) == 0) ? FAST_816_Z : 0));
	return (uint16)((uiValue | muiC) & uiMask);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C816::TRB(uint16 uiValue)
{
	uint16	uiMask;

	uiMask = IsM16() ? 0xFFFF : 0xFF;
	muiP = (uint8)((muiP & ~FAST_816_Z) | (((muiC & uiValue & uiMask) == 0) ? FAST_816_Z : 0));
	return (uint16)(uiValue & ~muiC & uiMask);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C816::Operate(uint8 uiOperation, uint16 uiValue)
{
	switch (uiOperation)
	{
	case F816M_ASL:
		return ASL(uiValue);
	case F816M_LSR:
		return LSR(uiValue);
	case F816M_ROL:
		return ROL(uiValue);
	case F816M_ROR:
		return ROR(uiValue);
	case F816M_INC:
		return INC(uiValue);
	case F816M_DEC:
		return DEC(uiValue);
	case F816M_TSB:
		return TSB(uiValue);
	default:
		return TRB(uiValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::ModifyAccumulator(uint8 uiOperation)
{
	muiCycles++;
	if (IsM16())
	{
		muiC = Operate(uiOperation, muiC);
	}
	else
	{
		muiC = (uint16)((muiC & 0xFF00) | Operate(uiOperation, muiC & 0xFF));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Read, an internal cycle, then write high byte first.  In emulation mode the internal cycle drives RWB low
// but neither VDA nor VPA, so it is not an access.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::Modify(uint32 uiAddress, uint8 uiOperation)
{
	uint16	uiValue;
	uint32	uiHighAddress;

	if (IsM16())
	{
		uiHighAddress = Next(uiAddress);
		uiValue = (uint16)(Read(uiAddress) | (Read(uiHighAddress) << 8));
		muiCycles++;
		uiValue = Operate(uiOperation, uiValue);
		Write(uiHighAddress, (uint8)(uiValue >> 8));
		Write(uiAddress, (uint8)uiValue);
	}
	else
	{
		uiValue = Read(uiAddress);
		muiCycles++;
		uiValue = Operate(uiOperation, uiValue);
		Write(uiAddress, (uint8)uiValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Taken: an internal cycle, and another in emulation mode when the target is on another page (note 6).
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::Branch(bool bTaken)
{
	int8	iOffset;
	uint16	uiTarget;

	iOffset = (int8)Fetch();
	if (bTaken)
	{
		uiTarget = (uint16)(muiPC + iOffset);
		muiCycles++;
		if (mbE && ((uiTarget ^ muiPC) & 0xFF00))
		{
			muiCycles++;
		}
		muiPC = uiTarget;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// TAX, TAY, TSX, TXY and TYX: all 16 bits of the source when the index registers are 16 bit.
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::SetIndex(uint16* puiIndex, uint16 uiValue)
{
	muiCycles++;
	if (IsX16())
	{
		*puiIndex = uiValue;
		SetNZ16(uiValue);
	}
	else
	{
		*puiIndex = uiValue & 0xFF;
		SetNZ8((uint8)uiValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// TXA and TYA.
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C816::TransferToA(uint16 uiValue)
{
	muiCycles++;
	LDA(uiValue);
}


//////////////////////////////////////////////////////////////////////////
//
// One byte of MVN (iStep 1) or MVP (iStep -1).  The instruction repeats until C counts down to FFFF.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::BlockMove(int iStep)
{
	uint8	uiSourceBank;
	uint8	uiValue;
	uint16	uiMask;

	muiDBR = Fetch();
	uiSourceBank = Fetch();
	uiValue = Read(((uint32)uiSourceBank << 16) | muiX);
	Write(((uint32)muiDBR << 16) | muiY, uiValue);
	muiCycles += 2;

	uiMask = IsX16() ? 0xFFFF : 0xFF;
	muiC--;
	muiX = (uint16)((muiX + iStep) & uiMask);
	muiY = (uint16)((muiY + iStep) & uiMask);
	if (muiC != 0xFFFF)
	{
		muiPC -= 3;
	}
}


#define FAST_816_READ_M(base, OPERATION) \
	case base + 0x01:	OPERATION(ReadM(AddressIndirectX()));					break; \
	case base + 0x03:	OPERATION(ReadM(AddressStack()));						break; \
	case base + 0x05:	OPERATION(ReadM(AddressDirect()));						break; \
	case base + 0x07:	OPERATION(ReadM(AddressIndirectLong()));				break; \
	case base + 0x09:	OPERATION(ImmediateM());								break; \
	case base + 0x0D:	OPERATION(ReadM(AddressAbsolute()));					break; \
	case base + 0x0F:	OPERATION(ReadM(AddressLong()));						break; \
	case base + 0x11:	OPERATION(ReadM(AddressIndirectY(false)));				break; \
	case base + 0x12:	OPERATION(ReadM(AddressIndirect()));					break; \
	case base + 0x13:	OPERATION(ReadM(AddressStackY()));						break; \
	case base + 0x15:	OPERATION(ReadM(AddressDirectIndexed(muiX)));			break; \
	case base + 0x17:	OPERATION(ReadM(AddressIndirectLongY()));				break; \
	case base + 0x19:	OPERATION(ReadM(AddressAbsoluteIndexed(muiY, false)));	break; \
	case base + 0x1D:	OPERATION(ReadM(AddressAbsoluteIndexed(muiX, false)));	break; \
	case base + 0x1F:	OPERATION(ReadM(AddressLongX()));						break;

#define FAST_816_MODIFY(base, OPERATION) \
	case base + 0x06:	Modify(AddressDirect(), OPERATION);							break; \
	case base + 0x0A:	ModifyAccumulator(OPERATION);								break; \
	case base + 0x0E:	Modify(AddressAbsolute(), OPERATION);						break; \
	case base + 0x16:	Modify(AddressDirectIndexed(muiX), OPERATION);				break; \
	case base + 0x1E:	Modify(AddressAbsoluteIndexed(muiX, true), OPERATION);		break;

#define FAST_816_CMP(uiValue)	Compare(muiC, uiValue, IsM16())


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::Execute(void)
{
	uint8	uiOpcode;
	uint8	uiLow;
	uint8	uiHigh;
	uint8	uiBank;
	uint16	uiValue;
	uint32	uiAddress;
	bool	bCarry;

	uiOpcode = Fetch();
	switch (uiOpcode)
	{
	FAST_816_READ_M(0x00, ORA)
	FAST_816_READ_M(0x20, AND)
	FAST_816_READ_M(0x40, EOR)
	FAST_816_READ_M(0x60, ADC)
	FAST_816_READ_M(0xA0, LDA)
	FAST_816_READ_M(0xC0, FAST_816_CMP)
	FAST_816_READ_M(0xE0, SBC)

	FAST_816_MODIFY(0x00, F816M_ASL)
	FAST_816_MODIFY(0x20, F816M_ROL)
	FAST_816_MODIFY(0x40, F816M_LSR)
	FAST_816_MODIFY(0x60, F816M_ROR)

	case 0x81:	WriteM(AddressIndirectX(), muiC);						break;
	case 0x83:	WriteM(AddressStack(), muiC);							break;
	case 0x85:	WriteM(AddressDirect(), muiC);							break;
	case 0x87:	WriteM(AddressIndirectLong(), muiC);					break;
	case 0x8D:	WriteM(AddressAbsolute(), muiC);						break;
	case 0x8F:	WriteM(AddressLong(), muiC);							break;
	case 0x91:	WriteM(AddressIndirectY(true), muiC);					break;
	case 0x92:	WriteM(AddressIndirect(), muiC);						break;
	case 0x93:	WriteM(AddressStackY(), muiC);							break;
	case 0x95:	WriteM(AddressDirectIndexed(muiX), muiC);				break;
	case 0x97:	WriteM(AddressIndirectLongY(), muiC);					break;
	case 0x99:	WriteM(AddressAbsoluteIndexed(muiY, true), muiC);		break;
	case 0x9D:	WriteM(AddressAbsoluteIndexed(muiX, true), muiC);		break;
	case 0x9F:	WriteM(AddressLongX(), muiC);							break;

	case 0x64:	WriteM(AddressDirect(), 0);								break;
	case 0x74:	WriteM(AddressDirectIndexed(muiX), 0);					break;
	case 0x9C:	WriteM(AddressAbsolute(), 0);							break;
	case 0x9E:	WriteM(AddressAbsoluteIndexed(muiX, true), 0);			break;

	case 0x84:	WriteX(AddressDirect(), muiY);							break;
	case 0x8C:	WriteX(AddressAbsolute(), muiY);						break;
	case 0x94:	WriteX(AddressDirectIndexed(muiX), muiY);				break;
	case 0x86:	WriteX(AddressDirect(), muiX);							break;
	case 0x8E:	WriteX(AddressAbsolute(), muiX);						break;
	case 0x96:	WriteX(AddressDirectIndexed(muiY), muiX);				break;

	case 0xA0:	LDY(ImmediateX());										break;
	case 0xA4:	LDY(ReadX(AddressDirect()));							break;
	case 0xAC:	LDY(ReadX(AddressAbsolute()));							break;
	case 0xB4:	LDY(ReadX(AddressDirectIndexed(muiX)));					break;
	case 0xBC:	LDY(ReadX(AddressAbsoluteIndexed(muiX, false)));		break;
	case 0xA2:	LDX(ImmediateX());										break;
	case 0xA6:	LDX(ReadX(AddressDirect()));							break;
	case 0xAE:	LDX(ReadX(AddressAbsolute()));							break;
	case 0xB6:	LDX(ReadX(AddressDirectIndexed(muiY)));					break;
	case 0xBE:	LDX(ReadX(AddressAbsoluteIndexed(muiY, false)));		break;

	case 0xC0:	Compare(muiY, ImmediateX(), IsX16());					break;
	case 0xC4:	Compare(muiY, ReadX(AddressDirect()), IsX16());			break;
	case 0xCC:	Compare(muiY, ReadX(AddressAbsolute()), IsX16());		break;
	case 0xE0:	Compare(muiX, ImmediateX(), IsX16());					break;
	case 0xE4:	Compare(muiX, ReadX(AddressDirect()), IsX16());			break;
	case 0xEC:	Compare(muiX, ReadX(AddressAbsolute()), IsX16());		break;

	case 0x24:	BIT(ReadM(AddressDirect()));							break;
	case 0x2C:	BIT(ReadM(AddressAbsolute()));							break;
	case 0x34:	BIT(ReadM(AddressDirectIndexed(muiX)));					break;
	case 0x3C:	BIT(ReadM(AddressAbsoluteIndexed(muiX, false)));		break;
	case 0x89:
		//BIT # only changes Z.
		uiValue = ImmediateM();
		if (!IsM16())
		{
			uiValue &= 0xFF;
		}
		muiP = (uint8)((muiP & ~FAST_816_Z) | (((muiC & uiValue) == 0) ? FAST_816_Z : 0));
		break;

	case 0x04:	Modify(AddressDirect(), F816M_TSB);						break;
	case 0x0C:	Modify(AddressAbsolute(), F816M_TSB);					break;
	case 0x14:	Modify(AddressDirect(), F816M_TRB);						break;
	case 0x1C:	Modify(AddressAbsolute(), F816M_TRB);					break;
	case 0xC6:	Modify(AddressDirect(), F816M_DEC);						break;
	case 0xCE:	Modify(AddressAbsolute(), F816M_DEC);					break;
	case 0xD6:	Modify(AddressDirectIndexed(muiX), F816M_DEC);			break;
	case 0xDE:	Modify(AddressAbsoluteIndexed(muiX, true), F816M_DEC);	break;
	case 0xE6:	Modify(AddressDirect(), F816M_INC);						break;
	case 0xEE:	Modify(AddressAbsolute(), F816M_INC);					break;
	case 0xF6:	Modify(AddressDirectIndexed(muiX), F816M_INC);			break;
	case 0xFE:	Modify(AddressAbsoluteIndexed(muiX, true), F816M_INC);	break;
	case 0x1A:	ModifyAccumulator(F816M_INC);							break;
	case 0x3A:	ModifyAccumulator(F816M_DEC);							break;

	case 0xE8:	SetIndex(&muiX, (uint16)(muiX + 1));					break;
	case 0xC8:	SetIndex(&muiY, (uint16)(muiY + 1));					break;
	case 0xCA:	SetIndex(&muiX, (uint16)(muiX - 1));					break;
	case 0x88:	SetIndex(&muiY, (uint16)(muiY - 1));					break;

	case 0xAA:	SetIndex(&muiX, muiC);									break;
	case 0xA8:	SetIndex(&muiY, muiC);									break;
	case 0xBA:	SetIndex(&muiX, muiS);									break;
	case 0x9B:	SetIndex(&muiY, muiX);									break;
	case 0xBB:	SetIndex(&muiX, muiY);									break;
	case 0x8A:	TransferToA(muiX);										break;
	case 0x98:	TransferToA(muiY);										break;
	case 0x9A:
		//TXS
		muiCycles++;
		muiS = mbE ? (uint16)(0x100 | (muiX & 0xFF)) : muiX;
		break;
	case 0x1B:
		//TCS
		muiCycles++;
		muiS = mbE ? (uint16)(0x100 | (muiC & 0xFF)) : muiC;
		break;
	case 0x3B:
		//TSC
		muiCycles++;
		muiC = muiS;
		SetNZ16(muiC);
		break;
	case 0x5B:
		//TCD
		muiCycles++;
		muiD = muiC;
		SetNZ16(muiD);
		break;
	case 0x7B:
		//TDC
		muiCycles++;
		muiC = muiD;
		SetNZ16(muiC);
		break;
	case 0xEB:
		//XBA
		muiCycles += 2;
		muiC = (uint16)((muiC >> 8) | (muiC << 8));
		SetNZ8((uint8)muiC);
		break;
	case 0xFB:
		//XCE
		muiCycles++;
		bCarry = (muiP & FAST_816_C) != 0;
		muiP = (uint8)((muiP & ~FAST_816_C) | (mbE ? FAST_816_C : 0));
		mbE = bCarry;
		if (mbE)
		{
			muiS = (uint16)(0x100 | (muiS & 0xFF));
			SetP(muiP);
		}
		else if (mfSignal)
		{
			mfSignal(mpvContext, FAST_65C816_SIGNAL_NATIVE);
		}
		break;

	case 0x18:	muiCycles++;	muiP &= ~FAST_816_C;	break;
	case 0x38:	muiCycles++;	muiP |= FAST_816_C;		break;
	case 0x58:	muiCycles++;	muiP &= ~FAST_816_I;	break;
	case 0x78:	muiCycles++;	muiP |= FAST_816_I;		break;
	case 0xD8:	muiCycles++;	muiP &= ~FAST_816_D;	break;
	case 0xF8:	muiCycles++;	muiP |= FAST_816_D;		break;
	case 0xB8:	muiCycles++;	muiP &= ~FAST_816_V;	break;
	case 0xEA:	muiCycles++;							break;
	case 0x42:
		//WDM skips its signature byte without reading it.
		muiCycles++;
		muiPC++;
		break;

	case 0xC2:
		//REP
		uiValue = Fetch();
		muiCycles++;
		SetP((uint8)(muiP & ~uiValue));
		break;
	case 0xE2:
		//SEP
		uiValue = Fetch();
		muiCycles++;
		SetP((uint8)(muiP | uiValue));
		break;

	case 0x10:	Branch(!(muiP & FAST_816_N));		break;
	case 0x30:	Branch((muiP & FAST_816_N) != 0);	break;
	case 0x50:	Branch(!(muiP & FAST_816_V));		break;
	case 0x70:	Branch((muiP & FAST_816_V) != 0);	break;
	case 0x90:	Branch(!(muiP & FAST_816_C));		break;
	case 0xB0:	Branch((muiP & FAST_816_C) != 0);	break;
	case 0xD0:	Branch(!(muiP & FAST_816_Z));		break;
	case 0xF0:	Branch((muiP & FAST_816_Z) != 0);	break;
	case 0x80:	Branch(true);						break;
	case 0x82:
		//BRL
		uiValue = Fetch16();
		muiCycles++;
		muiPC = (uint16)(muiPC + uiValue);
		break;

	case 0x4C:
		//JMP a
		muiPC = Fetch16();
		break;
	case 0x5C:
		//JML al
		uiValue = Fetch16();
		muiPBR = Fetch();
		muiPC = uiValue;
		break;
	case 0x6C:
		//JMP (a)
		uiValue = Fetch16();
		uiLow = Read(uiValue);
		uiHigh = Read((uint16)(uiValue + 1));
		muiPC = (uint16)(uiLow | (uiHigh << 8));
		break;
	case 0xDC:
		//JML [a]
		uiValue = Fetch16();
		uiLow = Read(uiValue);
		uiHigh = Read((uint16)(uiValue + 1));
		muiPBR = Read((uint16)(uiValue + 2));
		muiPC = (uint16)(uiLow | (uiHigh << 8));
		break;
	case 0x7C:
		//JMP (a,x)
		uiValue = (uint16)(Fetch16() + muiX);
		muiCycles++;
		uiLow = Read(((uint32)muiPBR << 16) | uiValue);
		uiHigh = Read(((uint32)muiPBR << 16) | (uint16)(uiValue + 1));
		muiPC = (uint16)(uiLow | (uiHigh << 8));
		break;
	case 0x20:
		//JSR a pushes the address of its last byte.
		uiLow = Fetch();
		uiHigh = Read(((uint32)muiPBR << 16) | muiPC);
		muiCycles++;
		Push((uint8)(muiPC >> 8));
		Push((uint8)muiPC);
		muiPC = (uint16)(uiLow | (uiHigh << 8));
		break;
	case 0xFC:
		//JSR (a,x)
		uiLow = Fetch();
		Push16((uint8)(muiPC >> 8));
		Push16((uint8)muiPC);
		uiHigh = Read(((uint32)muiPBR << 16) | muiPC);
		muiCycles++;
		uiValue = (uint16)((uiLow | (uiHigh << 8)) + muiX);
		uiLow = Read(((uint32)muiPBR << 16) | uiValue);
		uiHigh = Read(((uint32)muiPBR << 16) | (uint16)(uiValue + 1));
		muiPC = (uint16)(uiLow | (uiHigh << 8));
		break;
	case 0x22:
		//JSL al
		uiValue = Fetch16();
		Push16(muiPBR);
		muiCycles++;
		uiBank = Read(((uint32)muiPBR << 16) | muiPC);
		Push16((uint8)(muiPC >> 8));
		Push16((uint8)muiPC);
		muiPBR = uiBank;
		muiPC = uiValue;
		break;
	case 0x60:
		//RTS
		muiCycles += 2;
		uiLow = Pull();
		uiHigh = Pull();
		muiCycles++;
		muiPC = (uint16)((uiLow | (uiHigh << 8)) + 1);
		break;
	case 0x6B:
		//RTL
		muiCycles += 2;
		uiLow = Pull16();
		uiHigh = Pull16();
		muiPBR = Pull16();
		muiPC = (uint16)((uiLow | (uiHigh << 8)) + 1);
		break;
	case 0x40:
		//RTI.  The pulled P takes effect at the end.
		if (mfSignal)
		{
			mfSignal(mpvContext, FAST_65C816_SIGNAL_RTI);
		}
		muiCycles += 2;
		uiValue = Pull();
		uiLow = Pull();
		uiHigh = Pull();
		if (!mbE)
		{
			muiPBR = Pull();
		}
		muiPC = (uint16)(uiLow | (uiHigh << 8));
		SetP((uint8)uiValue);
		break;
	case 0x00:
		//BRK
		Interrupt(0xFFE6, 0xFFFE, false);
		break;
	case 0x02:
		//COP
		Interrupt(0xFFE4, 0xFFF4, false);
		break;

	case 0x08:
		//PHP
		muiCycles++;
		Push(muiP);
		break;
	case 0x28:
		//PLP
		muiCycles += 2;
		SetP(Pull());
		break;
	case 0x48:
		//PHA
		muiCycles++;
		if (IsM16())
		{
			Push((uint8)(muiC >> 8));
		}
		Push((uint8)muiC);
		break;
	case 0xDA:
		//PHX
		muiCycles++;
		if (IsX16())
		{
			Push((uint8)(muiX >> 8));
		}
		Push((uint8)muiX);
		break;
	case 0x5A:
		//PHY
		muiCycles++;
		if (IsX16())
		{
			Push((uint8)(muiY >> 8));
		}
		Push((uint8)muiY);
		break;
	case 0x68:
		//PLA
		muiCycles += 2;
		uiValue = Pull();
		if (IsM16())
		{
			uiValue |= Pull() << 8;
		}
		LDA(uiValue);
		break;
	case 0xFA:
		//PLX
		muiCycles += 2;
		uiValue = Pull();
		if (IsX16())
		{
			uiValue |= Pull() << 8;
		}
		LDX(uiValue);
		break;
	case 0x7A:
		//PLY
		muiCycles += 2;
		uiValue = Pull();
		if (IsX16())
		{
			uiValue |= Pull() << 8;
		}
		LDY(uiValue);
		break;
	case 0x8B:
		//PHB
		muiCycles++;
		Push(muiDBR);
		break;
	case 0x4B:
		//PHK
		muiCycles++;
		Push(muiPBR);
		break;
	case 0x0B:
		//PHD
		muiCycles++;
		Push16((uint8)(muiD >> 8));
		Push16((uint8)muiD);
		break;
	case 0xAB:
		//PLB
		muiCycles += 2;
		muiDBR = Pull16();
		SetNZ8(muiDBR);
		break;
	case 0x2B:
		//PLD
		muiCycles += 2;
		uiLow = Pull16();
		uiHigh = Pull16();
		muiD = (uint16)(uiLow | (uiHigh << 8));
		SetNZ16(muiD);
		break;
	case 0xF4:
		//PEA
		uiValue = Fetch16();
		Push16((uint8)(uiValue >> 8));
		Push16((uint8)uiValue);
		break;
	case 0xD4:
		//PEI
		uiLow = Fetch();
		if (muiD & 0xFF)
		{
			muiCycles++;
		}
		uiAddress = muiD + uiLow;
		uiLow = Read(uiAddress & 0xFFFF);
		uiHigh = Read((uiAddress + 1) & 0xFFFF);
		Push16(uiHigh);
		Push16(uiLow);
		break;
	case 0x62:
		//PER
		uiValue = Fetch16();
		muiCycles++;
		uiValue = (uint16)(uiValue + muiPC);
		Push16((uint8)(uiValue >> 8));
		Push16((uint8)uiValue);
		break;

	case 0x54:	BlockMove(1);		break;
	case 0x44:	BlockMove(-1);		break;

	case 0xDB:
		//STP
		muiCycles += 2;
		mbStopped = true;
		break;
	case 0xCB:
		//WAI
		muiCycles += 2;
		mbWaiting = true;
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFast65C816::GetC(void)				{ return muiC; }
uint16 CFast65C816::GetX(void)				{ return muiX; }
uint16 CFast65C816::GetY(void)				{ return muiY; }
uint16 CFast65C816::GetS(void)				{ return muiS; }
uint16 CFast65C816::GetD(void)				{ return muiD; }
uint8 CFast65C816::GetDataBank(void)		{ return muiDBR; }
uint8 CFast65C816::GetProgramBank(void)		{ return muiPBR; }
uint16 CFast65C816::GetProgramCounter(void)	{ return muiPC; }
uint32 CFast65C816::GetProgramAddress(void)	{ return ((uint32)muiPBR << 16) | muiPC; }
uint8 CFast65C816::GetP(void)				{ return muiP; }
bool CFast65C816::IsEmulation(void)			{ return mbE; }
bool CFast65C816::IsStopped(void)			{ return mbStopped; }
bool CFast65C816::IsWaiting(void)			{ return mbWaiting; }
uint64 CFast65C816::NumCycles(void)			{ return muiCycles; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::TestSetC(uint16 uiC)				{ muiC = uiC; }
void CFast65C816::TestSetX(uint16 uiX)				{ muiX = uiX; }
void CFast65C816::TestSetY(uint16 uiY)				{ muiY = uiY; }
void CFast65C816::TestSetS(uint16 uiS)				{ muiS = mbE ? (uint16)(0x100 | (uiS & 0xFF)) : uiS; }
void CFast65C816::TestSetD(uint16 uiD)				{ muiD = uiD; }
void CFast65C816::TestSetDataBank(uint8 uiDataBank)	{ muiDBR = uiDataBank; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::TestSetProgramCounter(uint8 uiProgramBank, uint16 uiProgramCounter)
{
	muiPBR = uiProgramBank;
	muiPC = uiProgramCounter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::TestSetP(uint8 uiP, bool bEmulation)
{
	mbE = bEmulation;
	SetP(uiP);
	if (mbE)
	{
		muiS = (uint16)(0x100 | (muiS & 0xFF));
	}
	Decide();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C816::AppendDifference(CChars* psz, const char* szName, uint64 uiOther, uint64 uiThis)
{
	if (uiOther != uiThis)
	{
		psz->Append(" ");
		psz->Append(szName);
		psz->Append(" simulator 0x");
		psz->AppendHexHiLo(&uiOther, 8);
		psz->Append(" fast 0x");
		psz->AppendHexHiLo(&uiThis, 8);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Appends each register that differs from the simulator's.  Returns true if any did.
//
//////////////////////////////////////////////////////////////////////////
bool CFast65C816::AppendDifferences(CW65C816* pcMPU, CChars* psz)
{
	size	uiLength;

	uiLength = psz->Length();
	AppendDifference(psz, "C", pcMPU->GetC(), muiC);
	AppendDifference(psz, "X", pcMPU->GetX(), muiX);
	AppendDifference(psz, "Y", pcMPU->GetY(), muiY);
	AppendDifference(psz, "S", pcMPU->GetStackPointer16Bit(), muiS);
	AppendDifference(psz, "D", pcMPU->GetDirectPage(), muiD);
	AppendDifference(psz, "DBR", pcMPU->GetDataBank(), muiDBR);
	AppendDifference(psz, "PBR", pcMPU->GetProgramCounter()->GetBank(), muiPBR);
	AppendDifference(psz, "PC", pcMPU->GetProgramCounter()->GetOffset(), muiPC);
	AppendDifference(psz, "P", pcMPU->GetProcessorRegisterValue(), muiP);
	AppendDifference(psz, "E", pcMPU->IsEmulation(), mbE);
	AppendDifference(psz, "Stopped", pcMPU->IsStopped(), mbStopped);
	AppendDifference(psz, "Waiting", pcMPU->IsWaiting(), mbWaiting);
	return psz->Length() != uiLength;
}

