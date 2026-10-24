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
#include <string.h>
#include "65C64Decoded.h"
#include "Fast65C64.h"


#define NUM_FAST_65C64_OPCODES	0x10000


//An opcode's fields as Execute uses them, from Decode65C64.  Registers are E65Register values; a register
//field that is not used is 0, the zero register, which reads as 0 with no width.
struct SFast65C64Op
{
	uint8	uiInstruction;
	uint8	uiOperandWords;
	uint8	uiAddressWords;
	uint8	uiFirstKind;
	uint8	uiFirstRegister;
	uint8	uiSecondKind;
	uint8	uiSecondRegister;
	uint8	uiImmediateBytes;
	uint8	uiOperationFirst;	//The registers an ALU or shift operation's width comes from.
	uint8	uiOperationSecond;
	uint8	uiPre1;
	uint8	uiPre2;
	uint8	uiPost;
	uint8	uiDataAccess;		//The memory operand's access (E65Access), MA65_None when there is no data.
	bool	bMemory;
	bool	bIndirect;
	bool	bSigned;
	uint8	uiStack;
	bool	bPushP;
	bool	bPushC;
	bool	bRelative;
	bool	bPullP;
	bool	bPullC;
	uint8	uiMask;
	uint8	uiFlagFirst;
	uint8	uiFlagSecond;
	uint8	uiCondition;
	uint8	uiWait;
	uint8	uiWidthFirst;
	uint8	uiWidthSecond;
	uint8	uiWidthBytes;
	bool	bDecrement;
	uint8	uiSource;
	uint8	uiDestination;
	uint8	uiCount;
	uint8	uiCountValue;
	bool	bNotZero;
	uint8	uiUnitBits;
	uint8	uiIndexStyle;
	bool	bAllowStop;
	uint8	uiStopStyle;
	bool	bDecrementW;
	uint8	uiAccess;
	uint16	uiVector;
	int32	iEmbedded;
	int32	iBranchOffset;
	bool	bDecoded;
};


//Decoded as they are first executed.
static SFast65C64Op		gasFast65C64Ops[NUM_FAST_65C64_OPCODES];

static const uint64		gauiFast65Masks[9] = { 0, 0xFFULL, 0xFFFFULL, 0xFFFFFFULL, 0xFFFFFFFFULL, 0xFFFFFFFFFFULL, 0xFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL };
static const uint64		gauiFast65Limits[8] = { 0, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFULL, 0xFFFFFFFFULL, 0xFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL };
static const uint8		gauiFast65WidthBytes[4] = { 1, 2, 4, 8 };
static const uint16		gauiFast65Flags[8] = { 0, 0, STATUS65_CARRY, STATUS65_ZERO, STATUS65_NEGATIVE, STATUS65_OVERFLOW, STATUS65_FLAG_F, STATUS65_FLAG_G };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void DecodeFast65C64Op(uint16 uiOpcode, SFast65C64Op* psOp)
{
	S65C64Decoded	sDecoded;

	Decode65C64(uiOpcode, &sDecoded);
	memset(psOp, 0, sizeof(SFast65C64Op));

	psOp->uiInstruction = (uint8)sDecoded.eInstruction;
	psOp->uiOperandWords = sDecoded.uiOperandWords;
	psOp->uiAddressWords = sDecoded.uiAddressWords;
	psOp->uiFirstKind = (uint8)sDecoded.sFirst.eKind;
	psOp->uiSecondKind = (uint8)sDecoded.sSecond.eKind;
	if (sDecoded.sFirst.eKind == OK65_Register)
	{
		psOp->uiFirstRegister = (uint8)sDecoded.sFirst.eRegister;
	}
	if (sDecoded.sSecond.eKind == OK65_Register)
	{
		psOp->uiSecondRegister = (uint8)sDecoded.sSecond.eRegister;
	}
	if (sDecoded.sSecond.eKind == OK65_Embedded)
	{
		psOp->iEmbedded = (int32)sDecoded.sSecond.iEmbedded;
	}
	psOp->uiImmediateBytes = sDecoded.sSecond.uiImmediateBytes;

	//An ALU operation is its first register's width, else its second register's, else 32 bits.  A shift
	//only looks at its first register.
	psOp->uiOperationFirst = psOp->uiFirstRegister;
	if ((sDecoded.eInstruction != I65_Shl) && (sDecoded.eInstruction != I65_Shr))
	{
		psOp->uiOperationSecond = psOp->uiSecondRegister;
	}

	psOp->uiPre1 = (uint8)sDecoded.ePre1;
	psOp->uiPre2 = (uint8)sDecoded.ePre2;
	psOp->uiPost = (uint8)sDecoded.ePost;
	psOp->bMemory = (sDecoded.sFirst.eKind == OK65_Memory) || (sDecoded.sSecond.eKind == OK65_Memory);
	psOp->bIndirect = sDecoded.bIndirect;
	psOp->bSigned = sDecoded.bSigned;
	psOp->uiDataAccess = sDecoded.bMemoryData ? (uint8)sDecoded.eDataAccess : (uint8)MA65_None;
	psOp->uiAccess = (uint8)sDecoded.eAccess;

	psOp->uiStack = (uint8)sDecoded.eStack;
	psOp->bPushP = sDecoded.bPushP;
	psOp->bPushC = sDecoded.bPushC;
	psOp->bRelative = sDecoded.bRelative;
	psOp->bPullP = sDecoded.bPullP;
	psOp->bPullC = sDecoded.bPullC;
	psOp->uiMask = sDecoded.uiMask;
	psOp->uiFlagFirst = sDecoded.uiFlagFirst;
	psOp->uiFlagSecond = sDecoded.uiFlagSecond;
	psOp->uiCondition = sDecoded.uiCondition;
	psOp->iBranchOffset = sDecoded.iBranchOffset;
	psOp->uiWait = sDecoded.uiWait;
	psOp->uiVector = sDecoded.uiVector;
	psOp->uiWidthFirst = (uint8)sDecoded.eWidthFirst;
	psOp->uiWidthSecond = (uint8)sDecoded.eWidthSecond;
	psOp->uiWidthBytes = sDecoded.uiWidthBytes;
	psOp->bDecrement = sDecoded.bDecrement;

	psOp->uiSource = (uint8)sDecoded.eSource;
	psOp->uiDestination = (uint8)sDecoded.eDestination;
	psOp->uiCount = (uint8)sDecoded.eCount;
	psOp->uiCountValue = sDecoded.uiCount;
	psOp->bNotZero = sDecoded.bNotZero;
	psOp->uiUnitBits = sDecoded.uiUnitBits;
	psOp->uiIndexStyle = sDecoded.uiIndexStyle;
	psOp->bAllowStop = sDecoded.bAllowStop;
	psOp->uiStopStyle = sDecoded.uiStopStyle;
	psOp->bDecrementW = sDecoded.bDecrementW;

	psOp->bDecoded = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static inline SFast65C64Op* GetFast65C64Op(uint16 uiOpcode)
{
	SFast65C64Op*	psOp;

	psOp = &gasFast65C64Ops[uiOpcode];
	if (!psOp->bDecoded)
	{
		DecodeFast65C64Op(uiOpcode, psOp);
	}
	return psOp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static inline uint64 Extend(uint64 uiValue, uint8 uiFromBytes, uint8 uiToBytes, bool bSigned)
{
	uiValue &= gauiFast65Masks[uiFromBytes];
	if (bSigned && (uiFromBytes < uiToBytes) && (uiFromBytes != 0) && ((uiValue >> (8 * uiFromBytes - 1)) & 1))
	{
		uiValue |= ~gauiFast65Masks[uiFromBytes];
	}
	return uiValue & gauiFast65Masks[uiToBytes];
}


//////////////////////////////////////////////////////////////////////////
//
// The number of aligned words the bytes uiAddress ... uiAddress+uiBytes-1 touch.
//
//////////////////////////////////////////////////////////////////////////
static inline uint32 CountWords(uint32 uiAddress, uint8 uiBytes)
{
	return ((uiAddress & 1) + uiBytes + 1) / 2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static inline uint8 BytesToWidthCode(uint64 uiBytes)
{
	if (uiBytes <= 1)
	{
		return WIDTH65_8;
	}
	else if (uiBytes == 2)
	{
		return WIDTH65_16;
	}
	else if (uiBytes <= 4)
	{
		return WIDTH65_32;
	}
	return WIDTH65_64;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::Init(CFast65C64ReadWord fReadWord, CFast65C64WriteByte fWriteByte, void* pvContext, uint8* puiMemory, uint32 uiMemorySize)
{
	memset(mauiRegisters, 0, sizeof(mauiRegisters));
	mauiRegisters[R65_S] = RESET_65C64_STACK_POINTER;
	muiC = 0;
	muiP = STATUS65_EDGE_DISABLE | STATUS65_LEVEL_DISABLE;
	muiInterruptDepth = 1;
	mbStopped = false;
	mbWaiting = false;

	muiWakeMask = 0;
	muiPendingEdges = 0;
	muiEdgeLow = 0;
	mbLevelInterrupt = false;

	muiCycles = 0;

	mfReadWord = fReadWord;
	mfWriteByte = fWriteByte;
	mpvContext = pvContext;
	mpuiMemory = puiMemory;
	muiMemorySize = puiMemory ? uiMemorySize : 0;
	muiWordLimit = (muiMemorySize >= 2) ? muiMemorySize - 1 : 0;
	muiQuadLimit = (muiMemorySize >= 8) ? muiMemorySize - 7 : 0;

	mauiBytes[R65_Zero] = 0;
	mauiMasks[R65_Zero] = 0;
	mauiBytes[R65_S] = 4;
	mauiBytes[R65_V] = 4;
	mauiBytes[R65_W] = 4;
	mauiMasks[R65_S] = 0xFFFFFFFFULL;
	mauiMasks[R65_V] = 0xFFFFFFFFULL;
	mauiMasks[R65_W] = 0xFFFFFFFFULL;
	SetWidths();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::Kill(void)
{
	mfReadWord = NULL;
	mfWriteByte = NULL;
	mpvContext = NULL;
	mpuiMemory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Two internal cycles then the reset vector.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::Reset(void)
{
	mbStopped = false;
	mbWaiting = false;
	muiWakeMask = 0;
	muiPendingEdges = 0;

	memset(mauiRegisters, 0, sizeof(mauiRegisters));
	mauiRegisters[R65_S] = RESET_65C64_STACK_POINTER;
	muiP = STATUS65_EDGE_DISABLE | STATUS65_LEVEL_DISABLE;
	muiInterruptDepth = 1;
	SetWidths();

	muiCycles += 2;
	muiC = ReadVector(VECTOR_65C64_RESET);
}


//////////////////////////////////////////////////////////////////////////
//
// One C65C64Meta::TickInstruction: an instruction (or a step of one) then an interrupt if one is taken, or a
// cycle while stp waits.  Returns false if the processor stopped.
//
//////////////////////////////////////////////////////////////////////////
bool CFast65C64::Step(void)
{
	if (mbStopped)
	{
		return false;
	}

	if (mbWaiting)
	{
		//An interrupt stp allows wakes the processor even if the interrupt is disabled.
		muiCycles++;
		if (((muiPendingEdges << 1) & muiWakeMask) || (mbLevelInterrupt && (muiWakeMask & WAKE65_LEVEL)))
		{
			mbWaiting = false;
			EnterInterrupt();
		}
		return true;
	}

	Execute();
	if (mbStopped)
	{
		return false;
	}
	if (muiPendingEdges || mbLevelInterrupt)
	{
		EnterInterrupt();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Steps until the processor stops.  Returns the number of steps, counting the one that stopped.
//
//////////////////////////////////////////////////////////////////////////
size CFast65C64::Run(size uiMaxInstructions)
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
// Edge interrupts are latched when their input falls.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::SetEdgeInterrupt(size uiInterrupt, bool bLow)
{
	uint8	uiBit;

	uiBit = (uint8)(1 << uiInterrupt);
	if (bLow && !(muiEdgeLow & uiBit))
	{
		muiPendingEdges |= uiBit;
	}

	if (bLow)
	{
		muiEdgeLow |= uiBit;
	}
	else
	{
		muiEdgeLow &= ~uiBit;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The level interrupt is taken while it is low.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::SetLevelInterrupt(bool bLow)
{
	mbLevelInterrupt = bLow;
}


//////////////////////////////////////////////////////////////////////////
//
// Edge interrupts first (0 before 1, 2 and 3), then the level interrupt.
//
//////////////////////////////////////////////////////////////////////////
bool CFast65C64::EnterInterrupt(void)
{
	size	i;
	uint8	uiBit;

	if (!(muiP & STATUS65_EDGE_DISABLE) && (muiPendingEdges != 0))
	{
		for (i = 0; i < NUM_65C64_EDGE_INTERRUPTS; i++)
		{
			uiBit = (uint8)(1 << i);
			if (muiPendingEdges & uiBit)
			{
				muiPendingEdges &= ~uiBit;
				StartInterrupt((uint16)(VECTOR_65C64_EDGE_0 - i), STATUS65_EDGE_DISABLE | STATUS65_LEVEL_DISABLE);
				return true;
			}
		}
	}

	if (!(muiP & STATUS65_LEVEL_DISABLE) && mbLevelInterrupt)
	{
		StartInterrupt(VECTOR_65C64_LEVEL, STATUS65_LEVEL_DISABLE);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// C (the next instruction) and P are pushed on S, then the vector is read.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::StartInterrupt(uint16 uiVector, uint16 uiDisable)
{
	Push(R65_S, muiC, 4);
	Push(R65_S, muiP, 2);
	muiP |= uiDisable;
	muiInterruptDepth++;
	mbWaiting = false;
	muiC = ReadVector(uiVector);
}


//////////////////////////////////////////////////////////////////////////
//
// A, B, X and Y's widths from P.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::SetWidths(void)
{
	mauiBytes[R65_A] = gauiFast65WidthBytes[(muiP >> STATUS65_WIDTH_A_SHIFT) & 3];
	mauiBytes[R65_B] = gauiFast65WidthBytes[(muiP >> STATUS65_WIDTH_B_SHIFT) & 3];
	mauiBytes[R65_X] = gauiFast65WidthBytes[(muiP >> STATUS65_WIDTH_X_SHIFT) & 3];
	mauiBytes[R65_Y] = gauiFast65WidthBytes[(muiP >> STATUS65_WIDTH_Y_SHIFT) & 3];
	mauiMasks[R65_A] = gauiFast65Masks[mauiBytes[R65_A]];
	mauiMasks[R65_B] = gauiFast65Masks[mauiBytes[R65_B]];
	mauiMasks[R65_X] = gauiFast65Masks[mauiBytes[R65_X]];
	mauiMasks[R65_Y] = gauiFast65Masks[mauiBytes[R65_Y]];
}


//////////////////////////////////////////////////////////////////////////
//
// Only the bytes in uiBytes change.  Writes to the zero register are dropped.
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C64::SetRegister(uint8 uiRegister, uint64 uiValue, uint8 uiBytes)
{
	uint64	uiMask;

	uiMask = gauiFast65Masks[uiBytes] & gauiFast65Limits[uiRegister];
	mauiRegisters[uiRegister] = (mauiRegisters[uiRegister] & ~uiMask) | (uiValue & uiMask);
}


//////////////////////////////////////////////////////////////////////////
//
// A register used as an address: its value in its width.
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C64::GetRegisterAddress(uint8 uiRegister)
{
	return (uint32)(mauiRegisters[uiRegister] & mauiMasks[uiRegister]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C64::SetNegativeAndZero(uint64 uiValue, uint8 uiBytes)
{
	muiP &= ~(STATUS65_NEGATIVE | STATUS65_ZERO);
	if ((uiValue >> (8 * uiBytes - 1)) & 1)
	{
		muiP |= STATUS65_NEGATIVE;
	}
	if ((uiValue & gauiFast65Masks[uiBytes]) == 0)
	{
		muiP |= STATUS65_ZERO;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C64::SetFlag(uint16 uiFlag, bool bValue)
{
	if (bValue)
	{
		muiP |= uiFlag;
	}
	else
	{
		muiP &= ~uiFlag;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Status flags as the branches number them: 0, 1, c, z, n, v, f, g.
//
//////////////////////////////////////////////////////////////////////////
inline bool CFast65C64::GetFlagCode(uint8 uiFlag)
{
	if (uiFlag < 2)
	{
		return uiFlag == 1;
	}
	return (muiP & gauiFast65Flags[uiFlag]) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::SetFlagCode(uint8 uiFlag, bool bValue)
{
	if ((uiFlag >= 2) && (uiFlag < 8))
	{
		SetFlag(gauiFast65Flags[uiFlag], bValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::SetWidthCode(uint8 uiRegister, uint8 uiWidthCode)
{
	int	iShift;

	switch (uiRegister)
	{
	case R65_A:
		iShift = STATUS65_WIDTH_A_SHIFT;
		break;
	case R65_B:
		iShift = STATUS65_WIDTH_B_SHIFT;
		break;
	case R65_X:
		iShift = STATUS65_WIDTH_X_SHIFT;
		break;
	case R65_Y:
		iShift = STATUS65_WIDTH_Y_SHIFT;
		break;
	default:
		return;
	}
	muiP = (uint16)((muiP & ~(3 << iShift)) | ((uiWidthCode & 3) << iShift));
}


//////////////////////////////////////////////////////////////////////////
//
// The two bytes at uiAddress, as the bus reads them.  Not a cycle.
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C64::ReadWord(uint32 uiAddress)
{
	if (uiAddress < muiWordLimit)
	{
		return (uint16)(mpuiMemory[uiAddress] | (mpuiMemory[uiAddress + 1] << 8));
	}
	return mfReadWord(mpvContext, uiAddress);
}


//////////////////////////////////////////////////////////////////////////
//
// An opcode or operand word at C.  The bus only carries aligned addresses, so an odd C reads the word below.
//
//////////////////////////////////////////////////////////////////////////
inline uint16 CFast65C64::FetchWord(void)
{
	uint16	uiWord;

	uiWord = ReadWord(muiC & 0xFFFFFFFE);
	muiC += 2;
	muiCycles++;
	return uiWord;
}


//////////////////////////////////////////////////////////////////////////
//
// A value of uiBytes at any address: a cycle for each aligned word it touches.  Words outside the flat
// memory are read whole, as the bus reads them.
//
//////////////////////////////////////////////////////////////////////////
inline uint64 CFast65C64::ReadValue(uint32 uiAddress, uint8 uiBytes)
{
	uint64	uiValue;
	uint32	uiWords;
	uint32	uiWordAddress;
	uint32	uiOffset;
	uint16	uiData;
	uint32	i;

	uiWords = CountWords(uiAddress, uiBytes);
	muiCycles += uiWords;
	if (uiAddress < muiQuadLimit)
	{
		memcpy(&uiValue, &mpuiMemory[uiAddress], sizeof(uint64));
		return uiValue & gauiFast65Masks[uiBytes];
	}

	uiValue = 0;
	for (i = 0; i < uiWords; i++)
	{
		uiWordAddress = (uiAddress & 0xFFFFFFFE) + 2 * i;
		uiData = ReadWord(uiWordAddress);
		uiOffset = uiWordAddress - uiAddress;
		if (uiOffset < uiBytes)
		{
			uiValue |= (uint64)(uiData & 0xFF) << (8 * uiOffset);
		}
		uiOffset++;
		if (uiOffset < uiBytes)
		{
			uiValue |= (uint64)(uiData >> 8) << (8 * uiOffset);
		}
	}
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
// A value of uiBytes at any address: a cycle for each aligned word it touches.  The bytes are written in
// address order.
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C64::WriteValue(uint32 uiAddress, uint8 uiBytes, uint64 uiValue)
{
	uint32	uiByteAddress;
	uint8	i;

	muiCycles += CountWords(uiAddress, uiBytes);
	if ((uiAddress < muiMemorySize) && (uiBytes <= muiMemorySize - uiAddress))
	{
		memcpy(&mpuiMemory[uiAddress], &uiValue, uiBytes);
		return;
	}

	for (i = 0; i < uiBytes; i++)
	{
		uiByteAddress = uiAddress + i;
		if (uiByteAddress < muiMemorySize)
		{
			mpuiMemory[uiByteAddress] = (uint8)(uiValue >> (8 * i));
		}
		else
		{
			mfWriteByte(mpvContext, uiByteAddress, (uint8)(uiValue >> (8 * i)));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The value's top byte goes at the stack register's address, which then moves down past it.
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C64::Push(uint8 uiStack, uint64 uiValue, uint8 uiBytes)
{
	uint32	uiAddress;

	uiAddress = GetRegisterAddress(uiStack);
	SetRegister(uiStack, uiAddress - uiBytes, mauiBytes[uiStack]);
	WriteValue(uiAddress - uiBytes + 1, uiBytes, uiValue);
}


//////////////////////////////////////////////////////////////////////////
//
// Moves the stack register up by uiBytes and returns the address of the value to read.
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C64::Pull(uint8 uiStack, uint8 uiBytes)
{
	uint32	uiAddress;

	uiAddress = GetRegisterAddress(uiStack) + uiBytes;
	SetRegister(uiStack, uiAddress, mauiBytes[uiStack]);
	return uiAddress - uiBytes + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFast65C64::ReadVector(uint16 uiVector)
{
	uint32	uiAddress;

	uiAddress = VECTOR_65C64_BASE + 4 * uiVector;
	muiCycles += 2;
	return (uint32)ReadWord(uiAddress) | ((uint32)ReadWord(uiAddress + 2) << 16);
}


//////////////////////////////////////////////////////////////////////////
//
// The end of a block or vector operation step (n and z are already set).  V (and W) move on by the element
// width, A counts down and v is set when it is negative.  C goes back to the opcode until then, or until an
// allowed stop condition holds.
//
//////////////////////////////////////////////////////////////////////////
inline void CFast65C64::BlockStep(uint8 uiBytes, bool bVector, bool bDecrement, bool bDecrementW, bool bAllowStop, uint8 uiStopStyle)
{
	uint8	uiABytes;
	uint64	uiA;
	bool	bNegative;
	bool	bStop;

	mauiRegisters[R65_V] = (uint32)(bDecrement ? mauiRegisters[R65_V] - uiBytes : mauiRegisters[R65_V] + uiBytes);
	if (bVector)
	{
		mauiRegisters[R65_W] = (uint32)(bDecrementW ? mauiRegisters[R65_W] - uiBytes : mauiRegisters[R65_W] + uiBytes);
	}

	uiABytes = mauiBytes[R65_A];
	uiA = (mauiRegisters[R65_A] - 1) & mauiMasks[R65_A];
	SetRegister(R65_A, uiA, uiABytes);
	bNegative = ((uiA >> (8 * uiABytes - 1)) & 1) != 0;
	SetFlag(STATUS65_OVERFLOW, bNegative);

	bStop = false;
	if (bAllowStop)
	{
		switch (uiStopStyle)
		{
		case 0:
			bStop = (muiP & STATUS65_ZERO) != 0;
			break;
		case 1:
			bStop = (muiP & STATUS65_ZERO) == 0;
			break;
		case 2:
			bStop = (muiP & STATUS65_NEGATIVE) != 0;
			break;
		default:
			bStop = (muiP & STATUS65_NEGATIVE) == 0;
			break;
		}
	}

	if (!bNegative && !bStop)
	{
		muiC -= 2;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The memory operand's address: words + Pre1 + Pre2, through a 32 bit pointer plus Post when indirect.
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C64::BaseAddress(SFast65C64Op* psOp, uint16* auiWords)
{
	uint32	uiAddress;

	if (psOp->uiAddressWords == 2)
	{
		uiAddress = (uint32)auiWords[0] | ((uint32)auiWords[1] << 16);
	}
	else if (psOp->uiAddressWords == 1)
	{
		uiAddress = auiWords[0];
	}
	else
	{
		uiAddress = 0;
	}
	return uiAddress + GetRegisterAddress(psOp->uiPre1) + GetRegisterAddress(psOp->uiPre2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline uint32 CFast65C64::DataAddress(SFast65C64Op* psOp, uint16* auiWords)
{
	uint32	uiAddress;

	uiAddress = BaseAddress(psOp, auiWords);
	if (psOp->bIndirect)
	{
		uiAddress = (uint32)ReadValue(uiAddress, 4) + GetRegisterAddress(psOp->uiPost);
	}
	return uiAddress;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static inline uint64 Immediate(SFast65C64Op* psOp, uint16* auiWords)
{
	uint64	uiValue;
	uint8	i;

	uiValue = 0;
	for (i = 0; i < psOp->uiOperandWords; i++)
	{
		uiValue |= (uint64)auiWords[i] << (16 * i);
	}
	return uiValue & gauiFast65Masks[psOp->uiImmediateBytes];
}


//////////////////////////////////////////////////////////////////////////
//
// An operand's value at uiBytes.  uiMemory is the memory operand's value.
//
//////////////////////////////////////////////////////////////////////////
inline uint64 CFast65C64::OperandValue(uint8 uiKind, uint8 uiRegister, SFast65C64Op* psOp, uint8 uiBytes, bool bSigned, uint64 uiImmediate, uint64 uiMemory)
{
	switch (uiKind)
	{
	case OK65_Register:
		return Extend(mauiRegisters[uiRegister], mauiBytes[uiRegister], uiBytes, bSigned);
	case OK65_Immediate:
		return Extend(uiImmediate, psOp->uiImmediateBytes, uiBytes, bSigned);
	case OK65_Embedded:
		return (uint64)(int64)psOp->iEmbedded & gauiFast65Masks[uiBytes];
	case OK65_Memory:
		return uiMemory & gauiFast65Masks[uiBytes];
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// cpy, and, or, xor, tst, add, sub, cmp, shl and shr: the operand's memory is read, the operation runs, then
// the result is written back if the memory operand is first.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::Alu(SFast65C64Op* psOp, uint16* auiWords)
{
	uint8	uiBytes;
	uint32	uiBits;
	uint64	uiMask;
	uint64	uiImmediate;
	uint64	uiMemory;
	uint32	uiAddress;
	uint64	uiFirst;
	uint64	uiSecond;
	uint64	uiResult;
	bool	bResult;

	uiBytes = mauiBytes[psOp->uiOperationFirst];
	if (uiBytes == 0)
	{
		uiBytes = mauiBytes[psOp->uiOperationSecond];
		if (uiBytes == 0)
		{
			uiBytes = 4;
		}
	}
	uiBits = 8 * uiBytes;
	uiMask = gauiFast65Masks[uiBytes];

	uiImmediate = (psOp->uiSecondKind == OK65_Immediate) ? Immediate(psOp, auiWords) : 0;
	uiMemory = 0;
	uiAddress = 0;
	if (psOp->bMemory)
	{
		uiAddress = DataAddress(psOp, auiWords);
		if (psOp->uiDataAccess & MA65_Read)
		{
			uiMemory = ReadValue(uiAddress, uiBytes);
		}
	}

	uiFirst = OperandValue(psOp->uiFirstKind, psOp->uiFirstRegister, psOp, uiBytes, false, 0, uiMemory);
	bResult = true;
	if (psOp->uiInstruction < I65_Shl)
	{
		uiSecond = OperandValue(psOp->uiSecondKind, psOp->uiSecondRegister, psOp, uiBytes, psOp->bSigned, uiImmediate, uiMemory);
		switch (psOp->uiInstruction)
		{
		case I65_Cpy:
			uiResult = uiSecond;
			SetNegativeAndZero(uiResult, uiBytes);
			break;
		case I65_And:
			uiResult = uiFirst & uiSecond;
			SetNegativeAndZero(uiResult, uiBytes);
			break;
		case I65_Or:
			uiResult = uiFirst | uiSecond;
			SetNegativeAndZero(uiResult, uiBytes);
			break;
		case I65_Xor:
			uiResult = uiFirst ^ uiSecond;
			SetNegativeAndZero(uiResult, uiBytes);
			break;
		case I65_Tst:
			uiResult = 0;
			bResult = false;
			SetNegativeAndZero(uiFirst & uiSecond, uiBytes);
			break;
		case I65_Add:
			uiResult = (uiFirst + uiSecond) & uiMask;
			SetNegativeAndZero(uiResult, uiBytes);
			SetFlag(STATUS65_CARRY, (uiBytes >= 8) ? (uiResult < uiFirst) : ((((uiFirst + uiSecond) >> uiBits) & 1) != 0));
			SetFlag(STATUS65_OVERFLOW, ((((uiFirst ^ uiResult) & (uiSecond ^ uiResult)) >> (uiBits - 1)) & 1) != 0);
			break;
		default:
			//sub and cmp.
			uiResult = (uiFirst - uiSecond) & uiMask;
			bResult = psOp->uiInstruction == I65_Sub;
			SetFlag(STATUS65_OVERFLOW, ((((uiFirst ^ uiSecond) & (uiFirst ^ uiResult)) >> (uiBits - 1)) & 1) != 0);
			SetNegativeAndZero(uiResult, uiBytes);
			SetFlag(STATUS65_CARRY, uiFirst >= uiSecond);
			break;
		}
	}
	else
	{
		//c is the last bit shifted out, v the one before it.  shr's n is the sign before shifting.
		uiSecond = OperandValue(psOp->uiSecondKind, psOp->uiSecondRegister, psOp, 8, false, uiImmediate, uiMemory);
		if (psOp->uiInstruction == I65_Shl)
		{
			uiResult = (uiSecond >= uiBits) ? 0 : (uiFirst << uiSecond) & uiMask;
			SetFlag(STATUS65_CARRY, (uiSecond >= 1) && (uiSecond <= uiBits) && ((uiFirst >> (uiBits - uiSecond)) & 1));
			SetFlag(STATUS65_OVERFLOW, (uiSecond >= 2) && (uiSecond <= uiBits + 1) && ((uiFirst >> (uiBits + 1 - uiSecond)) & 1));
		}
		else
		{
			uiResult = (uiSecond >= uiBits) ? 0 : uiFirst >> uiSecond;
			SetFlag(STATUS65_NEGATIVE, ((uiFirst >> (uiBits - 1)) & 1) != 0);
			SetFlag(STATUS65_CARRY, (uiSecond >= 1) && (uiSecond <= uiBits) && ((uiFirst >> (uiSecond - 1)) & 1));
			SetFlag(STATUS65_OVERFLOW, (uiSecond >= 2) && (uiSecond <= uiBits + 1) && ((uiFirst >> (uiSecond - 2)) & 1));
		}
		SetFlag(STATUS65_ZERO, uiResult == 0);
	}

	if (bResult)
	{
		if (psOp->uiFirstKind == OK65_Memory)
		{
			uiMemory = uiResult & uiMask;
		}
		else
		{
			SetRegister(psOp->uiFirstRegister, uiResult, uiBytes);
		}
	}

	if (psOp->uiDataAccess & MA65_Write)
	{
		WriteValue(uiAddress, uiBytes, uiMemory);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// jmp, jsr, bra, brsr and psh (no target): C is already the next instruction.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::Jump(SFast65C64Op* psOp, uint16* auiWords)
{
	uint32	uiTarget;
	uint8	uiRegister;

	switch (psOp->uiSecondKind)
	{
	case OK65_Register:
		uiRegister = psOp->uiSecondRegister;
		uiTarget = (uint32)Extend(mauiRegisters[uiRegister], mauiBytes[uiRegister], 4, psOp->bRelative);
		break;
	case OK65_Immediate:
		uiTarget = (uint32)Extend(Immediate(psOp, auiWords), psOp->uiImmediateBytes, 4, psOp->bRelative);
		break;
	case OK65_Memory:
		uiTarget = DataAddress(psOp, auiWords);
		break;
	default:
		uiTarget = 0;
		break;
	}

	if (psOp->bPushC)
	{
		Push(psOp->uiStack, muiC, 4);
	}
	if (psOp->bPushP)
	{
		Push(psOp->uiStack, muiP, 2);
	}

	if (psOp->uiSecondKind != OK65_None)
	{
		muiC = psOp->bRelative ? muiC + uiTarget : uiTarget;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// ret, pul and rti: P (if asked) then C are pulled.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::Return(SFast65C64Op* psOp)
{
	uint32	uiAddressP;
	uint32	uiAddressC;
	uint64	uiValueP;
	uint64	uiValueC;

	uiAddressP = psOp->bPullP ? Pull(psOp->uiStack, 2) : 0;
	uiAddressC = psOp->bPullC ? Pull(psOp->uiStack, 4) : 0;
	uiValueP = psOp->bPullP ? ReadValue(uiAddressP, 2) : 0;
	uiValueC = psOp->bPullC ? ReadValue(uiAddressC, 4) : 0;

	if (psOp->bPullP)
	{
		muiP = (uint16)uiValueP;
		SetWidths();
	}
	if (psOp->bPullC)
	{
		muiC = (uint32)uiValueC;
	}
	if ((psOp->uiInstruction == I65_Rti) && (muiInterruptDepth > 0))
	{
		muiInterruptDepth--;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A subcopy index field: a style bit, then a register or an embedded index.
//
//////////////////////////////////////////////////////////////////////////
inline uint64 CFast65C64::SubcopyIndex(uint16 uiField, bool bBits)
{
	uint8	uiRegister;

	if (uiField & 0x80)
	{
		return bBits ? ((uiField >> 1) & 0x3F) : ((uiField >> 4) & 0x7);
	}
	uiRegister = (uint8)((uiField >> 4) & 7);
	return mauiRegisters[uiRegister] & mauiMasks[uiRegister];
}


//////////////////////////////////////////////////////////////////////////
//
// scp and scb, ignoring the registers' widths.  The source is read before the destination is written.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::Subcopy(SFast65C64Op* psOp, uint16* auiWords)
{
	bool	bBits;
	uint16	uiWord;
	uint32	uiUnit;
	uint64	uiUnitMask;
	uint64	uiUnits;
	uint64	uiCount;
	uint64	uiSourceIndex;
	uint64	uiDestinationIndex;
	uint64	uiSource;
	uint64	uiDestination;
	uint64	uiValue;
	uint64	i;
	uint8	uiBytes;

	bBits = psOp->uiInstruction == I65_Scb;
	uiWord = auiWords[0];
	uiUnit = bBits ? 1 : 8;
	uiUnitMask = bBits ? 1 : 0xFF;
	uiUnits = 64 / uiUnit;

	if (psOp->uiCountValue != 0)
	{
		uiCount = psOp->uiCountValue;
		if (bBits)
		{
			uiCount += ((uiWord >> 8) & 1) * 16 + (uiWord & 1) * 32;
		}
	}
	else
	{
		uiCount = mauiRegisters[psOp->uiCount] & mauiMasks[psOp->uiCount];
	}
	uiSourceIndex = SubcopyIndex((uint16)(uiWord >> 8), bBits);
	uiDestinationIndex = SubcopyIndex((uint16)(uiWord & 0xFF), bBits);

	uiSource = mauiRegisters[psOp->uiSource];
	uiDestination = mauiRegisters[psOp->uiDestination];
	for (i = 0; (i < uiCount) && (uiDestinationIndex + i < uiUnits); i++)
	{
		if (uiSourceIndex + i < uiUnits)
		{
			uiValue = (uiSource >> ((uiSourceIndex + i) * uiUnit)) & uiUnitMask;
		}
		else
		{
			uiValue = 0;
		}
		uiDestination &= ~(uiUnitMask << ((uiDestinationIndex + i) * uiUnit));
		uiDestination |= uiValue << ((uiDestinationIndex + i) * uiUnit);
	}
	SetRegister(psOp->uiDestination, uiDestination, 8);

	uiBytes = mauiBytes[psOp->uiDestination];
	if (uiBytes != 0)
	{
		SetNegativeAndZero(mauiRegisters[psOp->uiDestination], uiBytes);
	}
	else
	{
		SetFlag(STATUS65_NEGATIVE, false);
		SetFlag(STATUS65_ZERO, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The finds: scan the first register from the unit at the index register.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::Find(SFast65C64Op* psOp)
{
	uint8	uiIndexBytes;
	uint64	uiIndexMask;
	uint64	uiValue;
	uint64	uiUnits;
	uint64	uiUnitMask;
	uint64	uiIndex;
	uint64	uiUnit;
	uint64	uiStep;
	bool	bFound;

	uiValue = mauiRegisters[psOp->uiSource] & mauiMasks[psOp->uiSource];
	uiUnits = (8 * mauiBytes[psOp->uiSource]) / psOp->uiUnitBits;
	uiUnitMask = (1ULL << psOp->uiUnitBits) - 1;
	//The zero register is index 0 scanned with an 8 bit index (the result is dropped).
	uiIndexBytes = mauiBytes[psOp->uiCount];
	uiIndexMask = gauiFast65Masks[(uiIndexBytes != 0) ? uiIndexBytes : 1];
	uiIndex = mauiRegisters[psOp->uiCount] & uiIndexMask;
	uiStep = psOp->bDecrement ? uiIndexMask : 1;

	if (muiP & STATUS65_CARRY)
	{
		uiIndex = (uiIndex + uiStep) & uiIndexMask;
	}

	bFound = false;
	while (uiIndex < uiUnits)
	{
		uiUnit = (uiValue >> (uiIndex * psOp->uiUnitBits)) & uiUnitMask;
		if (psOp->bNotZero ? (uiUnit != 0) : (uiUnit != uiUnitMask))
		{
			bFound = true;
			break;
		}
		uiIndex = (uiIndex + uiStep) & uiIndexMask;
	}

	SetRegister(psOp->uiCount, uiIndex, uiIndexBytes);
	SetFlag(STATUS65_OVERFLOW, !bFound);
	SetFlag(STATUS65_CARRY, true);
}


//////////////////////////////////////////////////////////////////////////
//
// bist and stab's bit index: style 7 is a register, 6 immediate (both in the operand word), 0 to 5 the index.
//
//////////////////////////////////////////////////////////////////////////
inline uint64 CFast65C64::BitIndex(SFast65C64Op* psOp, uint16* auiWords)
{
	uint8	uiRegister;

	if (psOp->uiIndexStyle == 7)
	{
		uiRegister = (uint8)((auiWords[0] >> 13) & 7);
		return mauiRegisters[uiRegister] & mauiMasks[uiRegister];
	}
	else if (psOp->uiIndexStyle == 6)
	{
		return (auiWords[0] >> 10) & 0x3F;
	}
	return psOp->uiIndexStyle;
}


//////////////////////////////////////////////////////////////////////////
//
// A status flag to a run of bits.  32 or more repeated bits take a cycle more.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::StatusToBit(SFast65C64Op* psOp, uint16* auiWords)
{
	uint8	uiRegister;
	uint16	uiWord;
	uint16	uiRepeatStyle;
	uint64	uiRepeat;
	uint64	uiIndex;
	uint64	uiValue;
	uint64	uiBit;
	uint64	i;
	bool	bDecrement;
	bool	bFlag;

	uiIndex = BitIndex(psOp, auiWords);
	bFlag = GetFlagCode(psOp->uiFlagFirst);
	uiRepeat = 1;
	bDecrement = false;
	if (psOp->uiOperandWords != 0)
	{
		uiWord = auiWords[0];
		uiRepeatStyle = (uiWord >> 8) & 3;
		bDecrement = uiRepeatStyle >= 2;
		if ((uiRepeatStyle & 1) == 0)
		{
			uiRegister = (uint8)((uiWord >> 5) & 7);
			uiRepeat = mauiRegisters[uiRegister] & mauiMasks[uiRegister];
		}
		else
		{
			uiRepeat = (uiWord >> 2) & 0x3F;
		}
	}

	uiValue = mauiRegisters[psOp->uiDestination];
	for (i = 0; (i < uiRepeat) && (i < 128); i++)
	{
		uiBit = bDecrement ? uiIndex - i : uiIndex + i;
		if (uiBit < 64)
		{
			uiValue &= ~(1ULL << uiBit);
			uiValue |= (bFlag ? 1ULL : 0ULL) << uiBit;
		}
	}
	SetRegister(psOp->uiDestination, uiValue, 8);

	if (uiRepeat >= 32)
	{
		muiCycles++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Block operations on the element at V, R's width wide: D = [V] op R.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::Block(SFast65C64Op* psOp)
{
	uint8	uiBytes;
	uint32	uiAddress;
	uint64	uiMemory;
	uint64	uiRegister;
	uint64	uiData;
	bool	bWrite;

	uiBytes = mauiBytes[psOp->uiSource];
	uiAddress = (uint32)mauiRegisters[R65_V];
	uiMemory = (psOp->uiAccess != MA65_Write) ? ReadValue(uiAddress, uiBytes) : 0;
	uiRegister = mauiRegisters[psOp->uiSource];
	bWrite = psOp->uiAccess != MA65_Read;

	switch (psOp->uiInstruction)
	{
	case I65_Bcpy:
		//R -> [V], only v is set.
		uiData = uiRegister;
		break;
	case I65_Band:
	case I65_Btst:
		uiData = uiMemory & uiRegister;
		break;
	case I65_Bor:
		uiData = uiMemory | uiRegister;
		break;
	case I65_Bxor:
		uiData = uiMemory ^ uiRegister;
		break;
	case I65_Badd:
		uiData = uiMemory + uiRegister;
		break;
	case I65_Bsub:
	case I65_Bcmp:
		uiData = uiMemory - uiRegister;
		break;
	case I65_Bpand:
		uiData = uiMemory & uiRegister;
		break;
	case I65_Bpor:
		uiData = uiMemory | uiRegister;
		break;
	case I65_Bpxor:
		uiData = uiMemory ^ uiRegister;
		break;
	default:
		//bpcpy.
		uiData = uiRegister;
		break;
	}
	uiData &= gauiFast65Masks[uiBytes];

	//The pre operations set n and z from the element read, the others (except copy) from D.
	if (psOp->uiInstruction >= I65_Bpcpy)
	{
		SetNegativeAndZero(uiMemory, uiBytes);
	}
	else if (psOp->uiInstruction != I65_Bcpy)
	{
		SetNegativeAndZero(uiData, uiBytes);
	}

	BlockStep(uiBytes, false, psOp->bDecrement, false, psOp->bAllowStop, psOp->uiStopStyle);
	if (bWrite)
	{
		WriteValue(uiAddress, uiBytes, uiData);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Vector operations: [W] is read, then [V] (not for copy and invert), D = [V] op [W] is written to [V] (not
// for test and compare).
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::Vector(SFast65C64Op* psOp)
{
	uint8	uiBytes;
	uint32	uiAddressV;
	uint64	uiW;
	uint64	uiV;
	uint64	uiData;
	bool	bPre;

	uiBytes = psOp->uiWidthBytes;
	uiAddressV = (uint32)mauiRegisters[R65_V];
	uiW = ReadValue((uint32)mauiRegisters[R65_W], uiBytes);
	uiV = ((psOp->uiInstruction != I65_Vcpy) && (psOp->uiInstruction != I65_Vnot)) ? ReadValue(uiAddressV, uiBytes) : 0;

	bPre = false;
	switch (psOp->uiInstruction)
	{
	case I65_Vcpy:
		uiData = uiW;
		break;
	case I65_Vnot:
		uiData = ~uiW;
		break;
	case I65_Vand:
	case I65_Vtst:
		uiData = uiV & uiW;
		break;
	case I65_Vor:
		uiData = uiV | uiW;
		break;
	case I65_Vxor:
		uiData = uiV ^ uiW;
		break;
	case I65_Vadd:
		uiData = uiV + uiW;
		break;
	case I65_Vsub:
	case I65_Vcmp:
		uiData = uiV - uiW;
		break;
	case I65_Vpcpy:
		uiData = uiW;
		bPre = true;
		break;
	case I65_Vpand:
		uiData = uiV & uiW;
		bPre = true;
		break;
	case I65_Vpor:
		uiData = uiV | uiW;
		bPre = true;
		break;
	default:
		uiData = uiV ^ uiW;
		bPre = true;
		break;
	}
	uiData &= gauiFast65Masks[uiBytes];
	SetNegativeAndZero(bPre ? uiV : uiData, uiBytes);

	BlockStep(uiBytes, true, psOp->bDecrement, psOp->bDecrementW, psOp->bAllowStop, psOp->uiStopStyle);
	if (psOp->uiAccess == MA65_ReadWrite)
	{
		WriteValue(uiAddressV, uiBytes, uiData);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A x B -> Y a byte of B a step, W counts B's nybbles.  C stays on the opcode until the last step.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::Multiply(void)
{
	uint8	uiNybbles;
	uint8	uiYBytes;
	uint32	uiStep;
	uint64	uiY;
	uint64	uiByte;
	uint64	uiA;
	bool	bDone;

	uiNybbles = 2 * mauiBytes[R65_B];
	uiYBytes = mauiBytes[R65_Y];
	uiStep = (uint32)mauiRegisters[R65_W];
	if (uiStep < uiNybbles)
	{
		uiY = (uiStep == 0) ? 0 : mauiRegisters[R65_Y] & mauiMasks[R65_Y];
		uiByte = (mauiRegisters[R65_B] >> (4 * uiStep)) & 0xFF;
		uiA = mauiRegisters[R65_A] & mauiMasks[R65_A];
		uiY = (uiY + ((uiA * uiByte) << (4 * uiStep))) & mauiMasks[R65_Y];
		SetRegister(R65_Y, uiY, uiYBytes);
		mauiRegisters[R65_W] = uiStep + 2;
	}

	bDone = mauiRegisters[R65_W] >= uiNybbles;
	SetFlag(STATUS65_OVERFLOW, bDone);
	SetFlag(STATUS65_ZERO, (mauiRegisters[R65_Y] & mauiMasks[R65_Y]) == 0);
	if (!bDone)
	{
		muiC -= 2;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A nybble of base 16 long division a step: Y goes from the dividend to the quotient, B is the remainder.
// C stays on the opcode until the last step.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::Divide(void)
{
	uint8	uiNybbles;
	uint8	uiYBytes;
	uint8	uiBBytes;
	uint64	uiY;
	uint64	uiA;
	uint64	uiRemainder;
	uint64	uiRemainderHigh;
	uint64	uiQuotient;
	bool	bDone;

	uiYBytes = mauiBytes[R65_Y];
	uiBBytes = mauiBytes[R65_B];
	uiNybbles = 2 * uiYBytes;
	if (mauiRegisters[R65_W] < uiNybbles)
	{
		//The remainder is 68 bits after the shift: the top 4 bits are kept apart.
		uiY = mauiRegisters[R65_Y] & mauiMasks[R65_Y];
		uiRemainder = mauiRegisters[R65_B] & mauiMasks[R65_B];
		uiRemainderHigh = uiRemainder >> 60;
		uiRemainder = (uiRemainder << 4) | ((uiY >> (8 * uiYBytes - 4)) & 0xF);
		uiA = mauiRegisters[R65_A] & mauiMasks[R65_A];

		if (uiA == 0)
		{
			uiQuotient = 15;
		}
		else
		{
			uiQuotient = 0;
			while ((uiQuotient < 15) && ((uiRemainderHigh != 0) || (uiRemainder >= uiA)))
			{
				if (uiRemainder < uiA)
				{
					uiRemainderHigh--;
				}
				uiRemainder -= uiA;
				uiQuotient++;
			}
		}

		SetRegister(R65_B, uiRemainder, uiBBytes);
		uiY = ((uiY << 4) | uiQuotient) & mauiMasks[R65_Y];
		SetRegister(R65_Y, uiY, uiYBytes);
		mauiRegisters[R65_W] = (uint32)(mauiRegisters[R65_W] + 1);
	}

	bDone = mauiRegisters[R65_W] >= uiNybbles;
	SetFlag(STATUS65_OVERFLOW, bDone);
	SetFlag(STATUS65_ZERO, (mauiRegisters[R65_Y] & mauiMasks[R65_Y]) == 0);
	SetFlag(STATUS65_CARRY, (mauiRegisters[R65_B] & mauiMasks[R65_B]) != 0);
	if (!bDone)
	{
		muiC -= 2;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes A, B, S, V, W, X then Y, each at its width.  The values are taken before the stack register moves.
// n and z come from the last one pushed.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::PushRegisters(SFast65C64Op* psOp)
{
	uint64	auiValues[7];
	uint8	uiRegister;
	uint8	uiBytes;
	uint64	uiLast;
	uint8	uiLastBytes;
	size	i;

	memcpy(auiValues, &mauiRegisters[R65_A], sizeof(auiValues));

	uiLast = 0;
	uiLastBytes = 0;
	for (i = 0; i < 7; i++)
	{
		if (psOp->uiMask & (1 << i))
		{
			uiRegister = (uint8)(R65_A + i);
			uiBytes = mauiBytes[uiRegister];
			Push(psOp->uiStack, auiValues[i], uiBytes);
			uiLast = auiValues[i];
			uiLastBytes = uiBytes;
		}
	}

	if (uiLastBytes != 0)
	{
		SetNegativeAndZero(uiLast, uiLastBytes);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pulls Y, X, W, V, S, B then A.  The stack register moves past them all before any is written.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::PullRegisters(SFast65C64Op* psOp)
{
	uint32	auiAddresses[7];
	uint8	auiRegisters[7];
	uint8	auiBytes[7];
	uint64	auiValues[7];
	size	uiCount;
	size	i;
	int		j;

	uiCount = 0;
	for (j = 6; j >= 0; j--)
	{
		if (psOp->uiMask & (1 << j))
		{
			auiRegisters[uiCount] = (uint8)(R65_A + j);
			auiBytes[uiCount] = mauiBytes[R65_A + j];
			auiAddresses[uiCount] = Pull(psOp->uiStack, auiBytes[uiCount]);
			uiCount++;
		}
	}

	for (i = 0; i < uiCount; i++)
	{
		auiValues[i] = ReadValue(auiAddresses[i], auiBytes[i]);
	}

	for (i = 0; i < uiCount; i++)
	{
		SetRegister(auiRegisters[i], auiValues[i], auiBytes[i]);
		SetNegativeAndZero(auiValues[i], auiBytes[i]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pul into an address: the address is calculated before the pull, a pointer's Post register after it.
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::PullOperand(SFast65C64Op* psOp, uint16* auiWords)
{
	uint32	uiAddress;
	uint32	uiStack;

	uiAddress = BaseAddress(psOp, auiWords);
	uiStack = Pull(psOp->uiStack, 4);
	if (psOp->bIndirect)
	{
		uiAddress = (uint32)ReadValue(uiAddress, 4) + GetRegisterAddress(psOp->uiPost);
	}
	WriteValue(uiAddress, 4, ReadValue(uiStack, 4));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::Execute(void)
{
	SFast65C64Op*	psOp;
	uint16			auiWords[MAX_65C64_OPERAND_WORDS];
	uint8			uiRegister;
	uint8			uiOldBytes;
	uint8			uiNewBytes;
	uint8			uiWidthCode;
	uint64			uiValue;
	uint64			uiBits;
	uint8			uiBytes;
	bool			bFirst;
	bool			bSecond;
	bool			bTaken;
	uint8			i;

	psOp = GetFast65C64Op(FetchWord());
	for (i = 0; i < psOp->uiOperandWords; i++)
	{
		auiWords[i] = FetchWord();
	}

	switch (psOp->uiInstruction)
	{
	case I65_Cpy:
	case I65_And:
	case I65_Or:
	case I65_Xor:
	case I65_Tst:
	case I65_Add:
	case I65_Sub:
	case I65_Cmp:
	case I65_Shl:
	case I65_Shr:
		Alu(psOp, auiWords);
		break;

	case I65_Bras:
	case I65_Brac:
	case I65_Brasa:
	case I65_Braso:
		//The second flag is one of c, z, n or v.
		bFirst = GetFlagCode(psOp->uiFlagFirst);
		bSecond = GetFlagCode(psOp->uiFlagSecond + 2);
		switch (psOp->uiCondition)
		{
		case 0:
			bTaken = !bFirst;
			break;
		case 1:
			bTaken = bFirst;
			break;
		case 2:
			bTaken = bFirst && bSecond;
			break;
		default:
			bTaken = bFirst != bSecond;
			break;
		}
		if (bTaken)
		{
			if (psOp->uiSecondKind == OK65_Immediate)
			{
				muiC += (uint32)Extend(Immediate(psOp, auiWords), psOp->uiImmediateBytes, 8, true);
			}
			else
			{
				muiC += (uint32)psOp->iBranchOffset;
			}
		}
		break;

	case I65_Bal:
		muiC += (uint32)psOp->iBranchOffset;
		break;

	case I65_Jmp:
	case I65_Jsr:
	case I65_Bra:
	case I65_Brsr:
	case I65_Psh:
		Jump(psOp, auiWords);
		break;

	case I65_Ret:
	case I65_Pul:
	case I65_Rti:
		Return(psOp);
		break;

	case I65_Sep:
		muiP |= psOp->uiMask;
		break;

	case I65_Cep:
		muiP &= ~(uint16)psOp->uiMask;
		break;

	case I65_Wth:
	case I65_Wtth:
		//Widths are in bytes.
		if (psOp->uiSecondKind == OK65_Embedded)
		{
			uiValue = psOp->uiWidthBytes;
		}
		else
		{
			uiValue = mauiRegisters[psOp->uiSecondRegister] & mauiMasks[psOp->uiSecondRegister];
		}
		uiWidthCode = BytesToWidthCode(uiValue);
		SetWidthCode(psOp->uiWidthFirst, uiWidthCode);
		if (psOp->uiInstruction == I65_Wtth)
		{
			SetWidthCode(psOp->uiWidthSecond, uiWidthCode);
		}
		SetWidths();
		break;

	case I65_Brk:
		Push(R65_S, muiC, 4);
		Push(R65_S, muiP, 2);
		muiInterruptDepth++;
		muiC = ReadVector(psOp->uiVector);
		break;

	case I65_Stp:
		//With wake bits stp waits for an interrupt, without them only reset restarts it.
		if (psOp->uiMask != 0)
		{
			mbWaiting = true;
			muiWakeMask = psOp->uiMask;
		}
		else
		{
			mbStopped = true;
		}
		break;

	case I65_Gwth:
		uiBits = 8 * mauiBytes[psOp->uiWidthFirst];
		if (psOp->bDecrement)
		{
			uiBits--;
		}
		SetRegister(psOp->uiFirstRegister, uiBits, mauiBytes[psOp->uiFirstRegister]);
		break;

	case I65_Wthx:
		uiRegister = psOp->uiWidthFirst;
		uiOldBytes = mauiBytes[uiRegister];
		uiWidthCode = BytesToWidthCode(psOp->uiWidthBytes);
		uiNewBytes = gauiFast65WidthBytes[uiWidthCode];
		if (uiNewBytes > uiOldBytes)
		{
			SetRegister(uiRegister, Extend(mauiRegisters[uiRegister], uiOldBytes, uiNewBytes, psOp->bSigned), uiNewBytes);
		}
		SetWidthCode(uiRegister, uiWidthCode);
		SetWidths();
		break;

	case I65_Nop:
		muiCycles += psOp->uiWait;
		break;

	case I65_PshRegisters:
		PushRegisters(psOp);
		break;

	case I65_PulRegisters:
		PullRegisters(psOp);
		break;

	case I65_Mul:
		Multiply();
		break;

	case I65_Div:
		Divide();
		break;

	case I65_Scp:
	case I65_Scb:
		Subcopy(psOp, auiWords);
		break;

	case I65_Neg:
		//The first register gets 0 - the second, sign extended to the first's width.
		uiBytes = mauiBytes[psOp->uiFirstRegister];
		if (uiBytes == 0)
		{
			uiBytes = 8;
		}
		uiValue = (0 - Extend(mauiRegisters[psOp->uiSecondRegister], mauiBytes[psOp->uiSecondRegister], uiBytes, true)) & gauiFast65Masks[uiBytes];
		SetRegister(psOp->uiFirstRegister, uiValue, uiBytes);
		SetNegativeAndZero(uiValue, uiBytes);
		break;

	case I65_Abs:
		//n says the second register was negative.
		uiBytes = mauiBytes[psOp->uiFirstRegister];
		if (uiBytes == 0)
		{
			uiBytes = 8;
		}
		uiValue = Extend(mauiRegisters[psOp->uiSecondRegister], mauiBytes[psOp->uiSecondRegister], uiBytes, true);
		bFirst = ((uiValue >> (8 * uiBytes - 1)) & 1) != 0;
		if (bFirst)
		{
			uiValue = (0 - uiValue) & gauiFast65Masks[uiBytes];
		}
		SetRegister(psOp->uiFirstRegister, uiValue, uiBytes);
		SetFlag(STATUS65_NEGATIVE, bFirst);
		SetFlag(STATUS65_ZERO, uiValue == 0);
		break;

	case I65_Fsbi:
	case I65_Fcbi:
	case I65_Fscr:
	case I65_Fccr:
	case I65_Fsny:
	case I65_Fcny:
	case I65_Fsby:
	case I65_Fcby:
		Find(psOp);
		break;

	case I65_Bist:
		//Bits past 63 read as 0.
		uiBits = BitIndex(psOp, auiWords);
		SetFlagCode(psOp->uiFlagFirst, (uiBits < 64) && ((mauiRegisters[psOp->uiSource] >> uiBits) & 1));
		break;

	case I65_Stab:
		StatusToBit(psOp, auiWords);
		break;

	case I65_Stat:
		SetFlagCode(psOp->uiFlagFirst, GetFlagCode(psOp->uiFlagSecond));
		break;

	case I65_PshOperand:
		//An immediate at its own width, or the 32 bit value at an address.
		if (psOp->uiSecondKind == OK65_Immediate)
		{
			Push(psOp->uiStack, Immediate(psOp, auiWords), psOp->uiImmediateBytes);
		}
		else
		{
			uiValue = ReadValue(DataAddress(psOp, auiWords), 4);
			Push(psOp->uiStack, uiValue, 4);
		}
		break;

	case I65_PulOperand:
		PullOperand(psOp, auiWords);
		break;

	case I65_Bcpy:
	case I65_Band:
	case I65_Btst:
	case I65_Bor:
	case I65_Bxor:
	case I65_Badd:
	case I65_Bsub:
	case I65_Bcmp:
	case I65_Bpcpy:
	case I65_Bpand:
	case I65_Bpor:
	case I65_Bpxor:
		Block(psOp);
		break;

	case I65_Vcpy:
	case I65_Vnot:
	case I65_Vand:
	case I65_Vtst:
	case I65_Vor:
	case I65_Vxor:
	case I65_Vadd:
	case I65_Vsub:
	case I65_Vcmp:
	case I65_Vpcpy:
	case I65_Vpand:
	case I65_Vpor:
	case I65_Vpxor:
		Vector(psOp);
		break;

	default:
		//Undefined encodings do nothing.
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CFast65C64::GetA(void) { return mauiRegisters[R65_A]; }
uint64 CFast65C64::GetB(void) { return mauiRegisters[R65_B]; }
uint64 CFast65C64::GetX(void) { return mauiRegisters[R65_X]; }
uint64 CFast65C64::GetY(void) { return mauiRegisters[R65_Y]; }
uint32 CFast65C64::GetS(void) { return (uint32)mauiRegisters[R65_S]; }
uint32 CFast65C64::GetV(void) { return (uint32)mauiRegisters[R65_V]; }
uint32 CFast65C64::GetW(void) { return (uint32)mauiRegisters[R65_W]; }
uint32 CFast65C64::GetProgramCounter(void) { return muiC; }
uint16 CFast65C64::GetP(void) { return muiP; }
uint32 CFast65C64::GetInterruptDepth(void) { return muiInterruptDepth; }
bool CFast65C64::IsFlagSet(uint16 uiFlag) { return (muiP & uiFlag) != 0; }
bool CFast65C64::IsStopped(void) { return mbStopped; }
bool CFast65C64::IsWaiting(void) { return mbWaiting; }
uint8 CFast65C64::GetPendingEdges(void) { return muiPendingEdges; }
uint64 CFast65C64::NumCycles(void) { return muiCycles; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::TestSetRegister(E65Register eRegister, uint64 uiValue)
{
	SetRegister((uint8)eRegister, uiValue, 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::TestSetProgramCounter(uint32 uiC)
{
	muiC = uiC;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFast65C64::TestSetP(uint16 uiP)
{
	muiP = uiP;
	SetWidths();
}

