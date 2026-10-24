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
#include "BaseLib/Logger.h"
#include "65Instruction.h"
#include "65BusCycle.h"
#include "65DataOperation.h"
#include "65C64InstructionFactory.h"
#include "65C64.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 Mask65(uint8 uiBytes)
{
	if (uiBytes >= 8)
	{
		return 0xFFFFFFFFFFFFFFFFULL;
	}
	return (1ULL << (8 * uiBytes)) - 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 Extend65(uint64 uiValue, uint8 uiFromBytes, uint8 uiToBytes, bool bSigned)
{
	if (uiFromBytes == 0)
	{
		return 0;
	}

	uiValue &= Mask65(uiFromBytes);
	if (bSigned && (uiFromBytes < uiToBytes) && ((uiValue >> (8 * uiFromBytes - 1)) & 1))
	{
		uiValue |= ~Mask65(uiFromBytes);
	}
	return uiValue & Mask65(uiToBytes);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 WidthCodeToBytes65(uint8 uiWidthCode)
{
	static uint8 auiBytes[4] = { 1, 2, 4, 8 };

	return auiBytes[uiWidthCode & 3];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 BytesToWidthCode65(uint64 uiBytes)
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
// The number of aligned words the bytes uiAddress ... uiAddress+uiBytes-1 touch.
//
//////////////////////////////////////////////////////////////////////////
static uint8 CountWords(uint32 uiAddress, uint8 uiBytes)
{
	return (uint8)(((uiAddress & 1) + uiBytes + 1) / 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint32 WordAddress(uint32 uiAddress, uint8 uiWord)
{
	return (uiAddress & 0xFFFFFFFE) + 2 * uiWord;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint8 LanesAt(uint32 uiWordAddress, uint32 uiAddress, uint8 uiBytes)
{
	uint8	uiLanes;

	uiLanes = 0;
	if ((uint32)(uiWordAddress - uiAddress) < uiBytes)
	{
		uiLanes |= BYTE_LANE_LOWER;
	}
	if ((uint32)(uiWordAddress + 1 - uiAddress) < uiBytes)
	{
		uiLanes |= BYTE_LANE_UPPER;
	}
	return uiLanes;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void MergeAt(uint64* puiValue, uint32 uiWordAddress, uint32 uiAddress, uint8 uiBytes, uint16 uiData)
{
	uint32	uiLane;
	uint32	uiOffset;

	for (uiLane = 0; uiLane < 2; uiLane++)
	{
		uiOffset = uiWordAddress + uiLane - uiAddress;
		if (uiOffset < uiBytes)
		{
			*puiValue &= ~(0xFFULL << (8 * uiOffset));
			*puiValue |= (uint64)((uiData >> (8 * uiLane)) & 0xFF) << (8 * uiOffset);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint16 ExtractAt(uint64 uiValue, uint32 uiWordAddress, uint32 uiAddress, uint8 uiBytes)
{
	uint32	uiLane;
	uint32	uiOffset;
	uint16	uiData;

	uiData = 0;
	for (uiLane = 0; uiLane < 2; uiLane++)
	{
		uiOffset = uiWordAddress + uiLane - uiAddress;
		if (uiOffset < uiBytes)
		{
			uiData |= (uint16)(((uiValue >> (8 * uiOffset)) & 0xFF) << (8 * uiLane));
		}
	}
	return uiData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::Init(C65C64Pins* pcPins)
{
	mpcPins = pcPins;

	muiA = 0;
	muiB = 0;
	muiX = 0;
	muiY = 0;
	muiS = RESET_65C64_STACK_POINTER;
	muiV = 0;
	muiW = 0;
	muiC = 0;
	muiP = STATUS65_EDGE_DISABLE | STATUS65_LEVEL_DISABLE;
	muiInterruptDepth = 1;

	muiOpcode = 0;
	memset(&msDecoded, 0, sizeof(S65C64Decoded));
	mpcInstruction = NULL;
	miCycle = -1;
	miNextCycle = 0;
	mbNextInstruction = false;
	mbStopped = false;

	mbWaiting = false;
	muiWakeMask = 0;
	muiPendingEdges = 0;
	muiEdgeLow = 0;
	mbLevelInterrupt = false;

	muiIOData = 0;
	mbPreviousClockLow = false;
	mbPreviousClockHigh = false;
	mbReset = false;

	ResetInstructionState();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::Kill(void)
{
	mpcInstruction = NULL;
	mpcPins = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::ResetInstructionState(void)
{
	muiDI = 0;
	muiOperationBytes = 0;
	memset(mauiOperandWords, 0, sizeof(mauiOperandWords));
	muiPointerAddress = 0;
	muiPointer = 0;
	muiNumPointerWords = 0;
	muiEffectiveAddress = 0;
	muiVectorAddress = 0;
	muiVector = 0;
	muiNumTransfers = 0;
	muiNumReadWords = 0;
	muiNumWriteWords = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::InputTransition(CTimeline* pcTimeline)
{
	STraceValue		sClock;
	bool			bRisingEdge;
	bool			bFallingEdge;

	sClock = mpcPins->ReadPhi2(pcTimeline);
	if (mpcPins->ReadRES(pcTimeline).IsLow())
	{
		ResetPulled();
		mbPreviousClockLow = sClock.IsLow();
		mbPreviousClockHigh = sClock.IsHigh();
		return;
	}

	SampleInterrupts(pcTimeline);

	if (sClock.IsHigh() || sClock.IsLow())
	{
		bRisingEdge = sClock.IsHigh() && mbPreviousClockLow;
		bFallingEdge = sClock.IsLow() && mbPreviousClockHigh;

		mbPreviousClockLow = sClock.IsLow();
		mbPreviousClockHigh = sClock.IsHigh();

		if (!mbStopped && !mbWaiting)
		{
			if (bFallingEdge)
			{
				ExecutePhi2Falling(pcTimeline);
			}

			if (bRisingEdge)
			{
				ExecutePhi2Rising(pcTimeline);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::ExecutePhi2Falling(CTimeline* pcTimeline)
{
	C65BusCycle*		pcBusCycle;
	C65DataOperation*	pcDataOperation;

	pcDataOperation = GetDataOperation();
	if (pcDataOperation)
	{
		if (pcDataOperation->IsRead())
		{
			//Data on the data pins is read when PHI2 falls.
			muiIOData = mpcPins->ReadData(pcTimeline);
		}
		else
		{
			//Written data is only driven while PHI2 is high.
			mpcPins->ReleaseData(pcTimeline);
		}

		GetBusCycle()->ExecuteTrailingSide(this, false);
	}

	Cycle();
	if (mbStopped)
	{
		return;
	}

	pcBusCycle = GetBusCycle();
	pcBusCycle->ExecuteInitialSide(this);
	WriteSignalsPhi2Falling(pcTimeline, pcBusCycle->GetDataOperation(), pcBusCycle);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::WriteSignalsPhi2Falling(CTimeline* pcTimeline, C65DataOperation* pcDataOperation, C65BusCycle* pcBusCycle)
{
	uint8	uiLanes;
	bool	bInternal;

	bInternal = !pcDataOperation->IsProgramAddress() && !pcDataOperation->IsDataAddress() && pcDataOperation->IsNotVectorPull();
	uiLanes = bInternal ? 0 : pcDataOperation->GetByteLanes(this);

	mpcPins->WriteRWB(pcTimeline, pcDataOperation->IsRead());
	mpcPins->WriteOPV(pcTimeline, pcDataOperation->IsOpcode());
	mpcPins->WriteVPA(pcTimeline, pcDataOperation->IsProgramAddress());
	mpcPins->WriteVDA(pcTimeline, pcDataOperation->IsDataAddress());
	mpcPins->WriteVPB(pcTimeline, pcDataOperation->IsNotVectorPull());
	mpcPins->WriteByteLanes(pcTimeline, (uiLanes & BYTE_LANE_LOWER) != 0, (uiLanes & BYTE_LANE_UPPER) != 0);
	mpcPins->WriteAddress(pcTimeline, pcBusCycle->GetAddress(this));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::ExecutePhi2Rising(CTimeline* pcTimeline)
{
	C65DataOperation*	pcDataOperation;

	pcDataOperation = GetDataOperation();
	if (pcDataOperation)
	{
		if (!pcDataOperation->IsRead())
		{
			mpcPins->WriteData(pcTimeline, muiIOData);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::Cycle(void)
{
	C65BusCycle*	pcBusCycle;

	if (mbStopped)
	{
		return;
	}

	for (;;)
	{
		if (mbNextInstruction)
		{
			mbNextInstruction = false;
			NextInstruction();
		}

		miCycle = miNextCycle;
		miNextCycle++;

		//An instruction is done when none of its remaining bus cycles must execute.
		pcBusCycle = GetBusCycle();
		if (pcBusCycle == NULL)
		{
			mbNextInstruction = true;
		}
		else if (pcBusCycle->MustExecute(this))
		{
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::NextInstruction(void)
{
	miNextCycle = 0;
	if (EnterInterrupt())
	{
		return;
	}

	if (mbWaiting)
	{
		mpcInstruction = C65C64InstructionFactory::GetInstance()->GetWait();
	}
	else
	{
		mpcInstruction = C65C64InstructionFactory::GetInstance()->GetFetchNext();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Edge interrupts are latched when their input falls.  They are taken before the
// level interrupt, edge interrupt 0 first.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SampleInterrupts(CTimeline* pcTimeline)
{
	size	i;
	uint8	uiBit;
	bool	bLow;

	for (i = 0; i < NUM_65C64_EDGE_INTERRUPTS; i++)
	{
		uiBit = (uint8)(1 << i);
		bLow = mpcPins->ReadEIRQ(pcTimeline, i).IsLow();
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
	mbLevelInterrupt = mpcPins->ReadLIRQ(pcTimeline).IsLow();

	//An interrupt stp allows wakes the processor even if the interrupt is disabled.
	if (mbWaiting)
	{
		if (((muiPendingEdges << 1) & muiWakeMask) || (mbLevelInterrupt && (muiWakeMask & WAKE65_LEVEL)))
		{
			mbWaiting = false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65C64::EnterInterrupt(void)
{
	size	i;
	uint8	uiBit;

	if (!IsFlagSet(STATUS65_EDGE_DISABLE) && (muiPendingEdges != 0))
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

	if (!IsFlagSet(STATUS65_LEVEL_DISABLE) && mbLevelInterrupt)
	{
		StartInterrupt(VECTOR_65C64_LEVEL, STATUS65_LEVEL_DISABLE);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// C (the next instruction) and P are pushed on S, then the vector is read.  An edge
// interrupt disables edge and level interrupts, a level interrupt disables level interrupts.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::StartInterrupt(uint16 uiVector, uint16 uiDisable)
{
	ResetInstructionState();
	memset(&msDecoded, 0, sizeof(S65C64Decoded));
	msDecoded.eInstruction = I65_Interrupt;
	msDecoded.eStack = R65_S;
	msDecoded.uiVector = uiVector;

	PushTransfer(muiC, 4);
	PushTransfer(muiP, 2);
	muiP |= uiDisable;
	muiVectorAddress = VECTOR_65C64_BASE + 4 * uiVector;
	muiVector = 0;
	muiInterruptDepth++;

	mbWaiting = false;
	mpcInstruction = C65C64InstructionFactory::GetInstance()->GetInterrupt();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::DoneInstruction(void)
{
	mbNextInstruction = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::ResetPulled(void)
{
	mpcInstruction = C65C64InstructionFactory::GetInstance()->GetReset();
	miCycle = -1;
	miNextCycle = 0;
	mbNextInstruction = false;
	mbStopped = false;
	mbWaiting = false;
	muiWakeMask = 0;
	muiPendingEdges = 0;
	ResetInstructionState();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65Instruction* C65C64::GetInstruction(void)
{
	return mpcInstruction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65BusCycle* C65C64::GetBusCycle(void)
{
	if ((mpcInstruction != NULL) && (miCycle >= 0))
	{
		return mpcInstruction->GetBusCycle(miCycle);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65DataOperation* C65C64::GetDataOperation(void)
{
	C65BusCycle*	pcBusCycle;

	pcBusCycle = GetBusCycle();
	if (pcBusCycle)
	{
		return pcBusCycle->GetDataOperation();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
S65C64Decoded* C65C64::GetDecoded(void)
{
	return &msDecoded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int16 C65C64::GetCycle(void)
{
	return miCycle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65C64::IsStopped(void)
{
	return mbStopped;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65C64::IsWaiting(void)
{
	return mbWaiting;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65C64::GetPendingEdges(void)
{
	return muiPendingEdges;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::ReadOpcode(void)
{
	C65BusCycle*	pcBusCycle;

	muiOpcode = muiIOData;
	Decode65C64(muiOpcode, &msDecoded);
	mpcInstruction = C65C64InstructionFactory::GetInstance()->GetInstruction(muiOpcode);
	ResetInstructionState();

	//The opcode fetch is the instruction's first cycle.  An instruction executes in the
	//cycle its last word is read, so a register instruction is done in this cycle.
	miCycle = 0;
	miNextCycle = 1;
	pcBusCycle = GetBusCycle();
	pcBusCycle->ExecuteTrailingSide(this, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::IncrementProgramCounter(void)
{
	muiC += 2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetOperandWord(uint8 uiWord, uint16 uiData)
{
	if (uiWord < MAX_65C64_OPERAND_WORDS)
	{
		mauiOperandWords[uiWord] = uiData;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 C65C64::GetPointerAddress(void)
{
	return muiPointerAddress;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65C64::GetNumPointerWords(void)
{
	return muiNumPointerWords;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65C64::GetPointerWordLanes(uint8 uiWord)
{
	return LanesAt(WordAddress(muiPointerAddress, uiWord), muiPointerAddress, 4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetPointerWord(uint8 uiWord, uint16 uiData)
{
	uint64	uiPointer;

	uiPointer = muiPointer;
	MergeAt(&uiPointer, WordAddress(muiPointerAddress, uiWord), muiPointerAddress, 4, uiData);
	muiPointer = (uint32)uiPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::AddWords(S65C64TransferWord* pasWords, size* puiNumWords, uint32 uiAddress, uint8 uiBytes)
{
	S65C64TransferWord*	psWord;
	uint8				uiNumWords;
	uint8				i;

	uiNumWords = CountWords(uiAddress, uiBytes);
	if (*puiNumWords + uiNumWords > MAX_65C64_TRANSFER_WORDS)
	{
		LOG_ERROR("Too many transfer words in one instruction.");
		return;
	}

	for (i = 0; i < uiNumWords; i++)
	{
		psWord = &pasWords[*puiNumWords];
		psWord->uiAddress = WordAddress(uiAddress, i);
		psWord->uiTransfer = (uint8)muiNumTransfers;
		psWord->uiLanes = LanesAt(psWord->uiAddress, uiAddress, uiBytes);
		(*puiNumWords)++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::AddTransfer(uint32 uiAddress, uint8 uiBytes, uint64 uiValue, bool bRead, bool bWrite)
{
	S65C64Transfer*		psTransfer;

	//Words that are read come before words that are written.
	if (muiNumTransfers >= MAX_65C64_TRANSFERS)
	{
		LOG_ERROR("Too many transfers in one instruction.");
		return;
	}

	psTransfer = &masTransfers[muiNumTransfers];
	psTransfer->uiAddress = uiAddress;
	psTransfer->uiBytes = uiBytes;
	psTransfer->uiValue = uiValue;

	if (bRead)
	{
		AddWords(masReadWords, &muiNumReadWords, uiAddress, uiBytes);
	}
	if (bWrite)
	{
		AddWords(masWriteWords, &muiNumWriteWords, uiAddress, uiBytes);
	}
	muiNumTransfers++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size C65C64::GetNumReadWords(void)
{
	return muiNumReadWords;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 C65C64::GetReadWordAddress(uint8 uiWord)
{
	if (uiWord < muiNumReadWords)
	{
		return masReadWords[uiWord].uiAddress;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65C64::GetReadWordLanes(uint8 uiWord)
{
	if (uiWord < muiNumReadWords)
	{
		return masReadWords[uiWord].uiLanes;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetReadWord(uint8 uiWord, uint16 uiData)
{
	S65C64TransferWord*	psWord;
	S65C64Transfer*		psTransfer;

	if (uiWord < muiNumReadWords)
	{
		psWord = &masReadWords[uiWord];
		psTransfer = &masTransfers[psWord->uiTransfer];
		MergeAt(&psTransfer->uiValue, psWord->uiAddress, psTransfer->uiAddress, psTransfer->uiBytes, uiData);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size C65C64::GetNumWriteWords(void)
{
	return muiNumWriteWords;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 C65C64::GetWriteWordAddress(uint8 uiWord)
{
	if (uiWord < muiNumWriteWords)
	{
		return masWriteWords[uiWord].uiAddress;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65C64::GetWriteWordLanes(uint8 uiWord)
{
	if (uiWord < muiNumWriteWords)
	{
		return masWriteWords[uiWord].uiLanes;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 C65C64::GetWriteWord(uint8 uiWord)
{
	S65C64TransferWord*	psWord;
	S65C64Transfer*		psTransfer;

	if (uiWord < muiNumWriteWords)
	{
		psWord = &masWriteWords[uiWord];
		psTransfer = &masTransfers[psWord->uiTransfer];
		return ExtractAt(psTransfer->uiValue, psWord->uiAddress, psTransfer->uiAddress, psTransfer->uiBytes);
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 C65C64::GetVectorAddress(void)
{
	return muiVectorAddress;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetVectorWord(uint8 uiWord, uint16 uiData)
{
	muiVector |= (uint32)uiData << (16 * uiWord);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65C64::GetNumWaitCycles(void)
{
	//nop's wait, or stab's extra cycle.
	return msDecoded.uiWait;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 C65C64::GetIOData(void)
{
	return muiIOData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetIOData(uint16 uiData)
{
	muiIOData = uiData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::PushTransfer(uint64 uiValue, uint8 uiBytes)
{
	uint32	uiStack;

	//The value's top byte is at S.
	uiStack = GetRegisterAddress(msDecoded.eStack);
	AddTransfer(uiStack - uiBytes + 1, uiBytes, uiValue, false, true);
	SetRegister(msDecoded.eStack, uiStack - uiBytes, GetRegisterBytes(msDecoded.eStack));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::PullTransfer(uint8 uiBytes)
{
	uint32	uiStack;

	uiStack = GetRegisterAddress(msDecoded.eStack) + uiBytes;
	AddTransfer(uiStack - uiBytes + 1, uiBytes, 0, true, false);
	SetRegister(msDecoded.eStack, uiStack, GetRegisterBytes(msDecoded.eStack));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65C64::CalculateOperationBytes(void)
{
	//An operation without a register (memory with an embedded value or the zero register) is 32 bits.
	switch (msDecoded.eInstruction)
	{
	case I65_Cpy:
	case I65_And:
	case I65_Or:
	case I65_Xor:
	case I65_Tst:
	case I65_Add:
	case I65_Sub:
	case I65_Cmp:
		if ((msDecoded.sFirst.eKind == OK65_Register) && (msDecoded.sFirst.eRegister != R65_Zero))
		{
			return GetRegisterBytes(msDecoded.sFirst.eRegister);
		}
		if ((msDecoded.sSecond.eKind == OK65_Register) && (msDecoded.sSecond.eRegister != R65_Zero))
		{
			return GetRegisterBytes(msDecoded.sSecond.eRegister);
		}
		return 4;

	case I65_Shl:
	case I65_Shr:
		if ((msDecoded.sFirst.eKind == OK65_Register) && (msDecoded.sFirst.eRegister != R65_Zero))
		{
			return GetRegisterBytes(msDecoded.sFirst.eRegister);
		}
		return 4;

	case I65_PshOperand:
	case I65_PulOperand:
		return 4;

	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::CalculateOperand(void)
{
	S65C64Decoded*	psDecoded;
	uint32			uiAddress;
	uint8			i;

	psDecoded = &msDecoded;
	if (psDecoded->sSecond.eKind == OK65_Immediate)
	{
		muiDI = 0;
		for (i = 0; i < psDecoded->uiOperandWords; i++)
		{
			muiDI |= (uint64)mauiOperandWords[i] << (16 * i);
		}
		muiDI &= Mask65(psDecoded->sSecond.uiImmediateBytes);
	}

	muiOperationBytes = CalculateOperationBytes();

	if ((psDecoded->sFirst.eKind == OK65_Memory) || (psDecoded->sSecond.eKind == OK65_Memory))
	{
		if (psDecoded->uiAddressWords == 2)
		{
			uiAddress = (uint32)mauiOperandWords[0] | ((uint32)mauiOperandWords[1] << 16);
		}
		else if (psDecoded->uiAddressWords == 1)
		{
			uiAddress = mauiOperandWords[0];
		}
		else
		{
			uiAddress = 0;
		}
		uiAddress += GetRegisterAddress(psDecoded->ePre1);
		uiAddress += GetRegisterAddress(psDecoded->ePre2);

		if (psDecoded->bIndirect)
		{
			muiPointerAddress = uiAddress;
			muiNumPointerWords = CountWords(uiAddress, 4);
			muiPointer = 0;
		}
		else
		{
			muiEffectiveAddress = uiAddress;
			DataAddressKnown();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::CalculateIndirectAddress(void)
{
	muiEffectiveAddress = muiPointer + GetRegisterAddress(msDecoded.ePost);
	DataAddressKnown();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::DataAddressKnown(void)
{
	if (msDecoded.bMemoryData)
	{
		AddTransfer(muiEffectiveAddress, muiOperationBytes, 0, (msDecoded.eDataAccess == MA65_Read) || (msDecoded.eDataAccess == MA65_ReadWrite), (msDecoded.eDataAccess == MA65_Write) || (msDecoded.eDataAccess == MA65_ReadWrite));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::LoadVector(void)
{
	muiC = muiVector;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::Reset(void)
{
	muiA = 0;
	muiB = 0;
	muiX = 0;
	muiY = 0;
	muiV = 0;
	muiW = 0;
	muiS = RESET_65C64_STACK_POINTER;
	muiP = STATUS65_EDGE_DISABLE | STATUS65_LEVEL_DISABLE;
	muiInterruptDepth = 1;

	muiVectorAddress = VECTOR_65C64_BASE + 4 * VECTOR_65C64_RESET;
	muiVector = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 C65C64::GetOperandValue(S65C64Operand* psOperand, uint8 uiBytes, bool bSigned)
{
	uint8	uiRegisterBytes;

	switch (psOperand->eKind)
	{
	case OK65_Register:
		uiRegisterBytes = GetRegisterBytes(psOperand->eRegister);
		return Extend65(GetRegister(psOperand->eRegister), uiRegisterBytes, uiBytes, bSigned);
	case OK65_Immediate:
		return Extend65(muiDI, psOperand->uiImmediateBytes, uiBytes, bSigned);
	case OK65_Embedded:
		return (uint64)psOperand->iEmbedded & Mask65(uiBytes);
	case OK65_Memory:
		return masTransfers[0].uiValue & Mask65(uiBytes);
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetFirstValue(uint64 uiValue, uint8 uiBytes)
{
	if (msDecoded.sFirst.eKind == OK65_Memory)
	{
		masTransfers[0].uiValue = uiValue & Mask65(uiBytes);
	}
	else if (msDecoded.sFirst.eKind == OK65_Register)
	{
		SetRegister(msDecoded.sFirst.eRegister, uiValue, uiBytes);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetNegativeAndZero(uint64 uiValue, uint8 uiBytes)
{
	SetFlag(STATUS65_NEGATIVE, ((uiValue >> (8 * uiBytes - 1)) & 1) != 0);
	SetFlag(STATUS65_ZERO, (uiValue & Mask65(uiBytes)) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::ExecuteAlu(E65Instruction eInstruction)
{
	uint8	uiBytes;
	uint32	uiBits;
	uint64	uiMask;
	uint64	uiFirst;
	uint64	uiSecond;
	uint64	uiResult;
	bool	bCarry;

	uiBytes = muiOperationBytes;
	uiBits = 8 * uiBytes;
	uiMask = Mask65(uiBytes);
	uiFirst = GetOperandValue(&msDecoded.sFirst, uiBytes, false);
	uiSecond = GetOperandValue(&msDecoded.sSecond, uiBytes, msDecoded.bSigned);

	switch (eInstruction)
	{
	case I65_Cpy:
		uiResult = uiSecond;
		SetFirstValue(uiResult, uiBytes);
		SetNegativeAndZero(uiResult, uiBytes);
		break;

	case I65_And:
		uiResult = uiFirst & uiSecond;
		SetFirstValue(uiResult, uiBytes);
		SetNegativeAndZero(uiResult, uiBytes);
		break;

	case I65_Or:
		uiResult = uiFirst | uiSecond;
		SetFirstValue(uiResult, uiBytes);
		SetNegativeAndZero(uiResult, uiBytes);
		break;

	case I65_Xor:
		uiResult = uiFirst ^ uiSecond;
		SetFirstValue(uiResult, uiBytes);
		SetNegativeAndZero(uiResult, uiBytes);
		break;

	case I65_Tst:
		SetNegativeAndZero(uiFirst & uiSecond, uiBytes);
		break;

	case I65_Add:
		uiResult = (uiFirst + uiSecond) & uiMask;
		if (uiBytes >= 8)
		{
			bCarry = uiResult < uiFirst;
		}
		else
		{
			bCarry = (((uiFirst + uiSecond) >> uiBits) & 1) != 0;
		}
		SetFirstValue(uiResult, uiBytes);
		SetNegativeAndZero(uiResult, uiBytes);
		SetFlag(STATUS65_CARRY, bCarry);
		SetFlag(STATUS65_OVERFLOW, ((((uiFirst ^ uiResult) & (uiSecond ^ uiResult)) >> (uiBits - 1)) & 1) != 0);
		break;

	case I65_Sub:
	case I65_Cmp:
		//cmp sets v like sub, so signed comparisons can branch on n != v (the user, 2026-10-23).
		uiResult = (uiFirst - uiSecond) & uiMask;
		if (eInstruction == I65_Sub)
		{
			SetFirstValue(uiResult, uiBytes);
		}
		SetFlag(STATUS65_OVERFLOW, ((((uiFirst ^ uiSecond) & (uiFirst ^ uiResult)) >> (uiBits - 1)) & 1) != 0);
		SetNegativeAndZero(uiResult, uiBytes);
		SetFlag(STATUS65_CARRY, uiFirst >= uiSecond);
		break;

	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::ILL(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::CPY(void)
{
	ExecuteAlu(I65_Cpy);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::AND(void)
{
	ExecuteAlu(I65_And);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::OR(void)
{
	ExecuteAlu(I65_Or);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::XOR(void)
{
	ExecuteAlu(I65_Xor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::TST(void)
{
	ExecuteAlu(I65_Tst);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::ADD(void)
{
	ExecuteAlu(I65_Add);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SUB(void)
{
	ExecuteAlu(I65_Sub);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::CMP(void)
{
	ExecuteAlu(I65_Cmp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 C65C64::GetTarget(void)
{
	S65C64Operand*	psOperand;
	uint8			uiBytes;

	psOperand = &msDecoded.sSecond;
	switch (psOperand->eKind)
	{
	case OK65_Register:
		uiBytes = GetRegisterBytes(psOperand->eRegister);
		return (uint32)Extend65(GetRegister(psOperand->eRegister), uiBytes, 4, msDecoded.bRelative);
	case OK65_Immediate:
		return (uint32)Extend65(muiDI, psOperand->uiImmediateBytes, 4, msDecoded.bRelative);
	case OK65_Memory:
		return muiEffectiveAddress;
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::JMP(void)
{
	bool	bJump;
	uint32	uiTarget;

	//C is already the address of the next instruction.
	bJump = msDecoded.sSecond.eKind != OK65_None;
	uiTarget = GetTarget();

	if (msDecoded.bPushC)
	{
		PushTransfer(muiC, 4);
	}
	if (msDecoded.bPushP)
	{
		PushTransfer(muiP, 2);
	}

	if (bJump)
	{
		if (msDecoded.bRelative)
		{
			muiC += uiTarget;
		}
		else
		{
			muiC = uiTarget;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::ReturnSetup(void)
{
	if (msDecoded.bPullP)
	{
		PullTransfer(2);
	}
	if (msDecoded.bPullC)
	{
		PullTransfer(4);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::RET(void)
{
	size	uiTransfer;

	uiTransfer = 0;
	if (msDecoded.bPullP)
	{
		muiP = (uint16)masTransfers[uiTransfer].uiValue;
		uiTransfer++;
	}
	if (msDecoded.bPullC)
	{
		muiC = (uint32)masTransfers[uiTransfer].uiValue;
	}

	if ((msDecoded.eInstruction == I65_Rti) && (muiInterruptDepth > 0))
	{
		muiInterruptDepth--;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SEP(void)
{
	muiP |= msDecoded.uiMask;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::CEP(void)
{
	muiP &= ~(uint16)msDecoded.uiMask;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::WTH(void)
{
	uint64	uiBytes;
	uint8	uiWidthCode;

	if (msDecoded.sSecond.eKind == OK65_Embedded)
	{
		uiBytes = msDecoded.uiWidthBytes;
	}
	else
	{
		uiBytes = GetOperandValue(&msDecoded.sSecond, 8, false);
	}

	uiWidthCode = BytesToWidthCode65(uiBytes);
	SetWidthCode(msDecoded.eWidthFirst, uiWidthCode);
	if (msDecoded.eInstruction == I65_Wtth)
	{
		SetWidthCode(msDecoded.eWidthSecond, uiWidthCode);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::BRK(void)
{
	PushTransfer(muiC, 4);
	PushTransfer(muiP, 2);
	muiVectorAddress = VECTOR_65C64_BASE + 4 * msDecoded.uiVector;
	muiVector = 0;
	muiInterruptDepth++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::STP(void)
{
	//With wake bits stp waits for an interrupt (WAI), without them only reset restarts it.
	if (msDecoded.uiMask != 0)
	{
		mbWaiting = true;
		muiWakeMask = msDecoded.uiMask;
	}
	else
	{
		mbStopped = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::GWTH(void)
{
	uint64	uiBits;

	uiBits = 8 * GetRegisterBytes(msDecoded.eWidthFirst);
	if (msDecoded.bDecrement)
	{
		uiBits--;
	}
	SetRegister(msDecoded.sFirst.eRegister, uiBits, GetRegisterBytes(msDecoded.sFirst.eRegister));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::WTHX(void)
{
	E65Register	eRegister;
	uint8		uiOldBytes;
	uint8		uiNewBytes;
	uint8		uiWidthCode;

	eRegister = msDecoded.eWidthFirst;
	uiOldBytes = GetRegisterBytes(eRegister);
	uiWidthCode = BytesToWidthCode65(msDecoded.uiWidthBytes);
	uiNewBytes = WidthCodeToBytes65(uiWidthCode);
	if (uiNewBytes > uiOldBytes)
	{
		SetRegister(eRegister, Extend65(GetRegister(eRegister), uiOldBytes, uiNewBytes, msDecoded.bSigned), uiNewBytes);
	}
	SetWidthCode(eRegister, uiWidthCode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65C64::GetBranchFlag(uint8 uiFlag)
{
	switch (uiFlag)
	{
	case 0:
		return false;
	case 1:
		return true;
	case 2:
		return IsFlagSet(STATUS65_CARRY);
	case 3:
		return IsFlagSet(STATUS65_ZERO);
	case 4:
		return IsFlagSet(STATUS65_NEGATIVE);
	case 5:
		return IsFlagSet(STATUS65_OVERFLOW);
	case 6:
		return IsFlagSet(STATUS65_FLAG_F);
	default:
		return IsFlagSet(STATUS65_FLAG_G);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 C65C64::GetBranchOffset(void)
{
	if (msDecoded.sSecond.eKind == OK65_Immediate)
	{
		return (int64)Extend65(muiDI, msDecoded.sSecond.uiImmediateBytes, 8, true);
	}
	return msDecoded.iBranchOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::BRANCH(void)
{
	bool	bFirst;
	bool	bSecond;
	bool	bTaken;

	//The second flag is one of c, z, n or v.
	bFirst = GetBranchFlag(msDecoded.uiFlagFirst);
	bSecond = GetBranchFlag(msDecoded.uiFlagSecond + 2);
	switch (msDecoded.uiCondition)
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
		muiC += (uint32)GetBranchOffset();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::BAL(void)
{
	muiC += (uint32)GetBranchOffset();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::NOP(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SHL(void)
{
	uint8	uiBytes;
	uint64	uiBits;
	uint64	uiValue;
	uint64	uiShift;
	uint64	uiResult;

	//c is the last bit shifted out, v the bit shifted out before it.
	uiBytes = muiOperationBytes;
	uiBits = 8 * uiBytes;
	uiValue = GetOperandValue(&msDecoded.sFirst, uiBytes, false);
	uiShift = GetOperandValue(&msDecoded.sSecond, 8, false);

	uiResult = (uiShift >= uiBits) ? 0 : (uiValue << uiShift) & Mask65(uiBytes);
	SetFirstValue(uiResult, uiBytes);
	SetFlag(STATUS65_ZERO, uiResult == 0);
	SetFlag(STATUS65_CARRY, (uiShift >= 1) && (uiShift <= uiBits) && ((uiValue >> (uiBits - uiShift)) & 1));
	SetFlag(STATUS65_OVERFLOW, (uiShift >= 2) && (uiShift <= uiBits + 1) && ((uiValue >> (uiBits + 1 - uiShift)) & 1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SHR(void)
{
	uint8	uiBytes;
	uint64	uiBits;
	uint64	uiValue;
	uint64	uiShift;
	uint64	uiResult;

	//c is the last bit shifted out, v the bit shifted out before it.  n is the sign before shifting.
	uiBytes = muiOperationBytes;
	uiBits = 8 * uiBytes;
	uiValue = GetOperandValue(&msDecoded.sFirst, uiBytes, false);
	uiShift = GetOperandValue(&msDecoded.sSecond, 8, false);

	uiResult = (uiShift >= uiBits) ? 0 : uiValue >> uiShift;
	SetFirstValue(uiResult, uiBytes);
	SetFlag(STATUS65_NEGATIVE, ((uiValue >> (uiBits - 1)) & 1) != 0);
	SetFlag(STATUS65_ZERO, uiResult == 0);
	SetFlag(STATUS65_CARRY, (uiShift >= 1) && (uiShift <= uiBits) && ((uiValue >> (uiShift - 1)) & 1));
	SetFlag(STATUS65_OVERFLOW, (uiShift >= 2) && (uiShift <= uiBits + 1) && ((uiValue >> (uiShift - 2)) & 1));
}


//////////////////////////////////////////////////////////////////////////
//
// A subcopy index field: a style bit, then a register (3 bits) or an embedded index
// (3 bits for bytes, 6 bits for bits).  A register index is read at its set width.
//
//////////////////////////////////////////////////////////////////////////
uint64 C65C64::GetSubcopyIndex(uint16 uiField, bool bBits)
{
	E65Register			eRegister;

	if (uiField & 0x80)
	{
		return bBits ? ((uiField >> 1) & 0x3F) : ((uiField >> 4) & 0x7);
	}
	eRegister = gae65Registers[(uiField >> 4) & 7];
	return GetRegister(eRegister) & Mask65(GetRegisterBytes(eRegister));
}


//////////////////////////////////////////////////////////////////////////
//
// scp and scb: copy count bytes (bits) from an index in the source register to an index
// in the destination register, ignoring the registers' widths.  The source is read
// before the destination is written.  Source bits past bit 63 read as 0, destination
// bits past bit 63 are dropped.  n and z are set from the destination at its width.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SCP(void)
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

	bBits = msDecoded.eInstruction == I65_Scb;
	uiWord = mauiOperandWords[0];
	uiUnit = bBits ? 1 : 8;
	uiUnitMask = bBits ? 1 : 0xFF;
	uiUnits = 64 / uiUnit;

	if (msDecoded.uiCount != 0)
	{
		uiCount = msDecoded.uiCount;
		if (bBits)
		{
			uiCount += ((uiWord >> 8) & 1) * 16 + (uiWord & 1) * 32;
		}
	}
	else
	{
		uiCount = GetRegister(msDecoded.eCount) & Mask65(GetRegisterBytes(msDecoded.eCount));
	}
	uiSourceIndex = GetSubcopyIndex((uint16)(uiWord >> 8), bBits);
	uiDestinationIndex = GetSubcopyIndex((uint16)(uiWord & 0xFF), bBits);

	uiSource = GetRegister(msDecoded.eSource);
	uiDestination = GetRegister(msDecoded.eDestination);
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
	SetRegister(msDecoded.eDestination, uiDestination, 8);

	uiBytes = GetRegisterBytes(msDecoded.eDestination);
	if (uiBytes != 0)
	{
		SetNegativeAndZero(GetRegister(msDecoded.eDestination), uiBytes);
	}
	else
	{
		SetFlag(STATUS65_NEGATIVE, false);
		SetFlag(STATUS65_ZERO, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The destination (first) gets 0 - source (second).  The source is sign extended to the
// destination's width.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::NEG(void)
{
	uint8	uiBytes;
	uint64	uiValue;

	uiBytes = GetRegisterBytes(msDecoded.sFirst.eRegister);
	if (uiBytes == 0)
	{
		uiBytes = 8;
	}
	uiValue = (0 - GetOperandValue(&msDecoded.sSecond, uiBytes, true)) & Mask65(uiBytes);
	SetFirstValue(uiValue, uiBytes);
	SetNegativeAndZero(uiValue, uiBytes);
}


//////////////////////////////////////////////////////////////////////////
//
// The destination (first) gets abs(source (second)).  n says the source was negative.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::ABS(void)
{
	uint8	uiBytes;
	uint64	uiValue;
	bool	bNegative;

	uiBytes = GetRegisterBytes(msDecoded.sFirst.eRegister);
	if (uiBytes == 0)
	{
		uiBytes = 8;
	}
	uiValue = GetOperandValue(&msDecoded.sSecond, uiBytes, true);
	bNegative = ((uiValue >> (8 * uiBytes - 1)) & 1) != 0;
	if (bNegative)
	{
		uiValue = (0 - uiValue) & Mask65(uiBytes);
	}
	SetFirstValue(uiValue, uiBytes);
	SetFlag(STATUS65_NEGATIVE, bNegative);
	SetFlag(STATUS65_ZERO, uiValue == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Scans the first register (within its width) from the unit at the index register,
// in the encoded direction, for a unit with any bit set (find set) or any bit clear
// (find clear).  If c is set the index moves on one first.  v is set if no unit was
// found, the index is then one past the end (the width in units, or all ones when
// decrementing).  c is always set.  The zero register as the index scans from 0 with
// an 8 bit index.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::FIND(void)
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

	uiValue = GetRegister(msDecoded.eSource) & Mask65(GetRegisterBytes(msDecoded.eSource));
	uiUnits = (8 * GetRegisterBytes(msDecoded.eSource)) / msDecoded.uiUnitBits;
	uiUnitMask = (1ULL << msDecoded.uiUnitBits) - 1;
	//The zero register is index 0 scanned with an 8 bit index (the result is dropped), or the scan never ends.
	uiIndexBytes = GetRegisterBytes(msDecoded.eCount);
	uiIndexMask = Mask65((uiIndexBytes != 0) ? uiIndexBytes : 1);
	uiIndex = GetRegister(msDecoded.eCount) & uiIndexMask;
	uiStep = msDecoded.bDecrement ? uiIndexMask : 1;

	if (IsFlagSet(STATUS65_CARRY))
	{
		uiIndex = (uiIndex + uiStep) & uiIndexMask;
	}

	bFound = false;
	while (uiIndex < uiUnits)
	{
		uiUnit = (uiValue >> (uiIndex * msDecoded.uiUnitBits)) & uiUnitMask;
		if (msDecoded.bNotZero ? (uiUnit != 0) : (uiUnit != uiUnitMask))
		{
			bFound = true;
			break;
		}
		uiIndex = (uiIndex + uiStep) & uiIndexMask;
	}

	SetRegister(msDecoded.eCount, uiIndex, uiIndexBytes);
	SetFlag(STATUS65_OVERFLOW, !bFound);
	SetFlag(STATUS65_CARRY, true);
}


//////////////////////////////////////////////////////////////////////////
//
// Status flags are numbered as in the branches: 2 c, 3 z, 4 n, 5 v, 6 f, 7 g.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetFlagCode(uint8 uiFlag, bool bValue)
{
	static uint16	auiFlags[8] = { 0, 0, STATUS65_CARRY, STATUS65_ZERO, STATUS65_NEGATIVE, STATUS65_OVERFLOW, STATUS65_FLAG_F, STATUS65_FLAG_G };

	if ((uiFlag >= 2) && (uiFlag < 8))
	{
		SetFlag(auiFlags[uiFlag], bValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The bit index of bist and stab: index style 7 is a register (operand word bits 15-13),
// 6 is immediate (operand word bits 15-10), 0 to 5 is the index itself.
//
//////////////////////////////////////////////////////////////////////////
uint64 C65C64::GetBitIndex(void)
{
	E65Register			eRegister;

	if (msDecoded.uiIndexStyle == 7)
	{
		eRegister = gae65Registers[(mauiOperandWords[0] >> 13) & 7];
		return GetRegister(eRegister) & Mask65(GetRegisterBytes(eRegister));
	}
	else if (msDecoded.uiIndexStyle == 6)
	{
		return (mauiOperandWords[0] >> 10) & 0x3F;
	}
	return msDecoded.uiIndexStyle;
}


//////////////////////////////////////////////////////////////////////////
//
// A bit of a register to a status flag.  Bits past 63 read as 0.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::BIST(void)
{
	uint64	uiIndex;

	uiIndex = GetBitIndex();
	SetFlagCode(msDecoded.uiFlagFirst, (uiIndex < 64) && ((GetRegister(msDecoded.eSource) >> uiIndex) & 1));
}


//////////////////////////////////////////////////////////////////////////
//
// A status flag (or 0 or 1) to a bit of a register, ignoring its width.  With an operand
// word the flag is written to a run of bits: operand word bits 9-8 are the repeat style
// (0 register incrementing, 1 immediate incrementing, 2 register decrementing,
// 3 immediate decrementing) and bits 7-5 the register or 7-2 the immediate count.
// Bits outside 0 to 63 are dropped.  32 or more repeated bits take a cycle more.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::STAB(void)
{
	E65Register			eRegister;
	uint16				uiWord;
	uint16				uiRepeatStyle;
	uint64				uiRepeat;
	uint64				uiIndex;
	uint64				uiValue;
	uint64				uiBit;
	uint64				i;
	bool				bDecrement;
	bool				bFlag;

	uiIndex = GetBitIndex();
	bFlag = GetBranchFlag(msDecoded.uiFlagFirst);
	uiRepeat = 1;
	bDecrement = false;
	if (msDecoded.uiOperandWords != 0)
	{
		uiWord = mauiOperandWords[0];
		uiRepeatStyle = (uiWord >> 8) & 3;
		bDecrement = uiRepeatStyle >= 2;
		if ((uiRepeatStyle & 1) == 0)
		{
			eRegister = gae65Registers[(uiWord >> 5) & 7];
			uiRepeat = GetRegister(eRegister) & Mask65(GetRegisterBytes(eRegister));
		}
		else
		{
			uiRepeat = (uiWord >> 2) & 0x3F;
		}
	}

	uiValue = GetRegister(msDecoded.eDestination);
	for (i = 0; (i < uiRepeat) && (i < 128); i++)
	{
		uiBit = bDecrement ? uiIndex - i : uiIndex + i;
		if (uiBit < 64)
		{
			uiValue &= ~(1ULL << uiBit);
			uiValue |= (bFlag ? 1ULL : 0ULL) << uiBit;
		}
	}
	SetRegister(msDecoded.eDestination, uiValue, 8);

	msDecoded.uiWait = (uiRepeat >= 32) ? 1 : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A status flag (or 0 or 1) to a status flag.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::STAT(void)
{
	SetFlagCode(msDecoded.uiFlagFirst, GetBranchFlag(msDecoded.uiFlagSecond));
}


//////////////////////////////////////////////////////////////////////////
//
// A block operation step works on the element at V, R's width wide.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::BlockSetup(void)
{
	bool	bRead;
	bool	bWrite;

	//Copy only writes, test and compare only read; the others read [V] then write it.
	bRead = msDecoded.eAccess != MA65_Write;
	bWrite = msDecoded.eAccess != MA65_Read;
	muiOperationBytes = GetRegisterBytes(msDecoded.eSource);
	muiEffectiveAddress = muiV;
	AddTransfer(muiEffectiveAddress, muiOperationBytes, 0, bRead, bWrite);
}


//////////////////////////////////////////////////////////////////////////
//
// The end of a block or vector operation step.  D (uiData) is stored to [V] (transfer
// uiTransfer) if bWrite, z and n are set from D if bSetNegativeAndZero.  V (and W for a
// vector operation) moves on by the element width, A is decremented and v is set when A
// is negative.  C stays on the opcode (the instruction repeats) until v is set or, if
// stopping is allowed, the stop condition holds: z set, z clear, n set or n clear.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::BlockStep(uint64 uiData, size uiTransfer, bool bWrite, bool bSetNegativeAndZero)
{
	uint8	uiBytes;
	uint8	uiABytes;
	uint64	uiA;
	bool	bNegative;
	bool	bStop;

	uiBytes = muiOperationBytes;
	uiData &= Mask65(uiBytes);
	if (bWrite)
	{
		masTransfers[uiTransfer].uiValue = uiData;
	}
	if (bSetNegativeAndZero)
	{
		SetNegativeAndZero(uiData, uiBytes);
	}

	if (msDecoded.bDecrement)
	{
		muiV -= uiBytes;
	}
	else
	{
		muiV += uiBytes;
	}
	if (msDecoded.bVector)
	{
		if (msDecoded.bDecrementW)
		{
			muiW -= uiBytes;
		}
		else
		{
			muiW += uiBytes;
		}
	}

	uiABytes = GetRegisterBytes(R65_A);
	uiA = (GetRegister(R65_A) - 1) & Mask65(uiABytes);
	SetRegister(R65_A, uiA, uiABytes);
	bNegative = ((uiA >> (8 * uiABytes - 1)) & 1) != 0;
	SetFlag(STATUS65_OVERFLOW, bNegative);

	bStop = false;
	if (msDecoded.bAllowStop)
	{
		switch (msDecoded.uiStopStyle)
		{
		case 0:
			bStop = IsFlagSet(STATUS65_ZERO);
			break;
		case 1:
			bStop = !IsFlagSet(STATUS65_ZERO);
			break;
		case 2:
			bStop = IsFlagSet(STATUS65_NEGATIVE);
			break;
		default:
			bStop = !IsFlagSet(STATUS65_NEGATIVE);
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
// Block and, test, or, xor, add, subtract and compare: D = [V] op R, n and z are set from
// D.  Test and compare do not write D back to [V].
//
//////////////////////////////////////////////////////////////////////////
void C65C64::BLOCK(void)
{
	uint64	uiMemory;
	uint64	uiRegister;
	uint64	uiData;

	uiMemory = masTransfers[0].uiValue;
	uiRegister = GetRegister(msDecoded.eSource);
	switch (msDecoded.eInstruction)
	{
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
	default:
		uiData = uiMemory - uiRegister;
		break;
	}
	BlockStep(uiData, 0, msDecoded.eAccess == MA65_ReadWrite, true);
}


//////////////////////////////////////////////////////////////////////////
//
// Block copy: R -> [V].  Nothing is read, only v is set.  It runs until A is negative: the
// stop bits are ignored.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::BCPY(void)
{
	BlockStep(GetRegister(msDecoded.eSource), 0, true, false);
}


//////////////////////////////////////////////////////////////////////////
//
// The "pre" (encoding sheet "Read") block operations: n and z are set from the value read
// at [V] (so a stop condition can end the operation early), then bpcpy writes R,
// bpand [V] and R, bpor [V] or R, bpxor [V] xor R.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::BPRE(void)
{
	uint64	uiRead;
	uint64	uiRegister;
	uint64	uiData;

	uiRead = masTransfers[0].uiValue;
	uiRegister = GetRegister(msDecoded.eSource);
	SetNegativeAndZero(uiRead, muiOperationBytes);
	switch (msDecoded.eInstruction)
	{
	case I65_Bpand:
		uiData = uiRead & uiRegister;
		break;
	case I65_Bpor:
		uiData = uiRead | uiRegister;
		break;
	case I65_Bpxor:
		uiData = uiRead ^ uiRegister;
		break;
	default:
		uiData = uiRegister;
		break;
	}
	BlockStep(uiData, 0, true, false);
}


//////////////////////////////////////////////////////////////////////////
//
// A vector operation step works on the elements at W and V.  [W] is read first, then [V]
// (not for copy and invert), then [V] is written (not for test and compare).
//
//////////////////////////////////////////////////////////////////////////
void C65C64::VectorSetup(void)
{
	bool	bReadV;
	bool	bWriteV;

	bReadV = (msDecoded.eInstruction != I65_Vcpy) && (msDecoded.eInstruction != I65_Vnot);
	bWriteV = msDecoded.eAccess == MA65_ReadWrite;
	muiOperationBytes = msDecoded.uiWidthBytes;
	AddTransfer(muiW, muiOperationBytes, 0, true, false);
	AddTransfer(muiV, muiOperationBytes, 0, bReadV, bWriteV);
}


//////////////////////////////////////////////////////////////////////////
//
// D is [V] op [W] (or [W] for copy, ![W] for invert); n and z are set from D.  The "pre"
// operations set n and z from the original [V] instead, so a stop condition can end them
// at a terminator in V.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::VECTOR(void)
{
	uint64	uiW;
	uint64	uiV;
	uint64	uiData;
	bool	bWrite;
	bool	bPre;

	uiW = masTransfers[0].uiValue;
	uiV = masTransfers[1].uiValue;
	bWrite = msDecoded.eAccess == MA65_ReadWrite;
	bPre = false;
	switch (msDecoded.eInstruction)
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

	if (bPre)
	{
		SetNegativeAndZero(uiV, muiOperationBytes);
	}
	BlockStep(uiData, 1, bWrite, !bPre);
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes an immediate (its own width) or the 32 bit value at an address (read first).
//
//////////////////////////////////////////////////////////////////////////
void C65C64::PushOperand(void)
{
	if (msDecoded.sSecond.eKind == OK65_Immediate)
	{
		PushTransfer(muiDI, msDecoded.sSecond.uiImmediateBytes);
	}
	else
	{
		PushTransfer(masTransfers[0].uiValue, 4);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::PullOperandSetup(void)
{
	CalculateOperand();
	PullTransfer(4);
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the pulled 32 bit value to the address.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::PullOperand(void)
{
	AddTransfer(muiEffectiveAddress, 4, masTransfers[0].uiValue, false, true);
}


//////////////////////////////////////////////////////////////////////////
//
// One step multiplies A by a byte of B and adds it into Y at that byte's position.
// W counts B's nybbles (2 a step).  The first step (W = 0) clears Y.  C stays on the
// opcode until the last step, so the instruction repeats and can be interrupted between
// steps.  Unsigned, the product is truncated to Y's width.  v and z are set every step.
// Every step does the whole calculation, zeros are not short cut.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::MUL(void)
{
	uint8	uiNybbles;
	uint8	uiYBytes;
	uint32	uiStep;
	uint64	uiY;
	uint64	uiByte;
	uint64	uiA;
	bool	bDone;

	uiNybbles = 2 * GetRegisterBytes(R65_B);
	uiYBytes = GetRegisterBytes(R65_Y);
	uiStep = muiW;
	if (uiStep < uiNybbles)
	{
		uiY = (uiStep == 0) ? 0 : GetRegister(R65_Y) & Mask65(uiYBytes);
		uiByte = (GetRegister(R65_B) >> (4 * uiStep)) & 0xFF;
		uiA = GetRegister(R65_A) & Mask65(GetRegisterBytes(R65_A));
		uiY = (uiY + ((uiA * uiByte) << (4 * uiStep))) & Mask65(uiYBytes);
		SetRegister(R65_Y, uiY, uiYBytes);
		muiW = uiStep + 2;
	}

	bDone = muiW >= uiNybbles;
	SetFlag(STATUS65_OVERFLOW, bDone);
	SetFlag(STATUS65_ZERO, (GetRegister(R65_Y) & Mask65(uiYBytes)) == 0);
	if (!bDone)
	{
		muiC -= 2;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// One step of base 16 long division.  Y's top nybble moves into the remainder B, the
// largest multiple of A that fits (at most 15) is subtracted and the quotient nybble
// moves into the bottom of Y, so Y goes from the dividend to the quotient.  W counts
// the steps, one per nybble of Y.  W must be 0 at the start; B is the top of the dividend
// (usually 0, and below A so the quotient fits).  The remainder is
// truncated to B's width.  Dividing by 0 gives quotient nybbles of 15.  Unsigned.
// v, z (Y is 0) and c (B is not 0) are set every step.  C stays on the opcode until
// the last step.  Every step does the whole calculation, zeros are not short cut.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::DIV(void)
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

	uiYBytes = GetRegisterBytes(R65_Y);
	uiBBytes = GetRegisterBytes(R65_B);
	uiNybbles = 2 * uiYBytes;
	if (muiW < uiNybbles)
	{
		//The remainder is 68 bits after the shift: the top 4 bits are kept apart.
		uiY = GetRegister(R65_Y) & Mask65(uiYBytes);
		uiRemainder = GetRegister(R65_B) & Mask65(uiBBytes);
		uiRemainderHigh = uiRemainder >> 60;
		uiRemainder = (uiRemainder << 4) | ((uiY >> (8 * uiYBytes - 4)) & 0xF);
		uiA = GetRegister(R65_A) & Mask65(GetRegisterBytes(R65_A));

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
		uiY = ((uiY << 4) | uiQuotient) & Mask65(uiYBytes);
		SetRegister(R65_Y, uiY, uiYBytes);
		muiW++;
	}

	bDone = muiW >= uiNybbles;
	SetFlag(STATUS65_OVERFLOW, bDone);
	SetFlag(STATUS65_ZERO, (GetRegister(R65_Y) & Mask65(uiYBytes)) == 0);
	SetFlag(STATUS65_CARRY, (GetRegister(R65_B) & Mask65(uiBBytes)) != 0);
	if (!bDone)
	{
		muiC -= 2;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::PushRegisters(void)
{
	uint64		auiValues[7];
	E65Register	eRegister;
	size		i;
	uint8		uiBytes;
	uint64		uiLast;
	uint8		uiLastBytes;

	//Push A, B, S, V, W, X then Y.  The values are taken before the stack register moves.
	for (i = 0; i < 7; i++)
	{
		auiValues[i] = GetRegister((E65Register)(R65_A + i));
	}

	uiLast = 0;
	uiLastBytes = 0;
	for (i = 0; i < 7; i++)
	{
		if (msDecoded.uiMask & (1 << i))
		{
			eRegister = (E65Register)(R65_A + i);
			uiBytes = GetRegisterBytes(eRegister);
			PushTransfer(auiValues[i], uiBytes);
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
//
//////////////////////////////////////////////////////////////////////////
void C65C64::PullRegistersSetup(void)
{
	E65Register	eRegister;
	int			i;

	//Pull Y, X, W, V, S, B then A.
	for (i = 6; i >= 0; i--)
	{
		if (msDecoded.uiMask & (1 << i))
		{
			eRegister = (E65Register)(R65_A + i);
			maePulled[muiNumTransfers] = eRegister;
			PullTransfer(GetRegisterBytes(eRegister));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::PullRegisters(void)
{
	size			i;
	S65C64Transfer*	psTransfer;

	for (i = 0; i < muiNumTransfers; i++)
	{
		psTransfer = &masTransfers[i];
		SetRegister(maePulled[i], psTransfer->uiValue, psTransfer->uiBytes);
		SetNegativeAndZero(psTransfer->uiValue, psTransfer->uiBytes);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 C65C64::GetRegister(E65Register eRegister)
{
	switch (eRegister)
	{
	case R65_A:
		return muiA;
	case R65_B:
		return muiB;
	case R65_S:
		return muiS;
	case R65_V:
		return muiV;
	case R65_W:
		return muiW;
	case R65_X:
		return muiX;
	case R65_Y:
		return muiY;
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetRegister(E65Register eRegister, uint64 uiValue, uint8 uiBytes)
{
	uint64	uiMask;

	//Only the bytes in the width change.
	uiMask = Mask65(uiBytes);
	switch (eRegister)
	{
	case R65_A:
		muiA = (muiA & ~uiMask) | (uiValue & uiMask);
		break;
	case R65_B:
		muiB = (muiB & ~uiMask) | (uiValue & uiMask);
		break;
	case R65_S:
		muiS = (uint32)((muiS & ~uiMask) | (uiValue & uiMask));
		break;
	case R65_V:
		muiV = (uint32)((muiV & ~uiMask) | (uiValue & uiMask));
		break;
	case R65_W:
		muiW = (uint32)((muiW & ~uiMask) | (uiValue & uiMask));
		break;
	case R65_X:
		muiX = (muiX & ~uiMask) | (uiValue & uiMask);
		break;
	case R65_Y:
		muiY = (muiY & ~uiMask) | (uiValue & uiMask);
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65C64::GetRegisterBytes(E65Register eRegister)
{
	switch (eRegister)
	{
	case R65_A:
	case R65_B:
	case R65_X:
	case R65_Y:
		return WidthCodeToBytes65(GetWidthCode(eRegister));
	case R65_S:
	case R65_V:
	case R65_W:
		return 4;
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 C65C64::GetRegisterAddress(E65Register eRegister)
{
	return (uint32)(GetRegister(eRegister) & Mask65(GetRegisterBytes(eRegister)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65C64::GetWidthCode(E65Register eRegister)
{
	switch (eRegister)
	{
	case R65_A:
		return (muiP >> STATUS65_WIDTH_A_SHIFT) & 3;
	case R65_B:
		return (muiP >> STATUS65_WIDTH_B_SHIFT) & 3;
	case R65_X:
		return (muiP >> STATUS65_WIDTH_X_SHIFT) & 3;
	case R65_Y:
		return (muiP >> STATUS65_WIDTH_Y_SHIFT) & 3;
	default:
		return WIDTH65_32;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetWidthCode(E65Register eRegister, uint8 uiWidthCode)
{
	int	iShift;

	switch (eRegister)
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
//
//////////////////////////////////////////////////////////////////////////
uint64 C65C64::GetA(void) { return muiA; }
uint64 C65C64::GetB(void) { return muiB; }
uint64 C65C64::GetX(void) { return muiX; }
uint64 C65C64::GetY(void) { return muiY; }
uint32 C65C64::GetS(void) { return muiS; }
uint32 C65C64::GetV(void) { return muiV; }
uint32 C65C64::GetW(void) { return muiW; }
uint32 C65C64::GetProgramCounter(void) { return muiC; }
uint16 C65C64::GetP(void) { return muiP; }
uint32 C65C64::GetInterruptDepth(void) { return muiInterruptDepth; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65C64::IsFlagSet(uint16 uiFlag)
{
	return (muiP & uiFlag) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetFlag(uint16 uiFlag, bool bValue)
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
//
//////////////////////////////////////////////////////////////////////////
void C65C64::GetMnemonicString(CChars* psz)
{
	if (mpcInstruction)
	{
		psz->Append(mpcInstruction->GetName());
	}
	else
	{
		psz->Append("---");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::GetCycleOperationString(CChars* psz)
{
	C65BusCycle*	pcBusCycle;

	pcBusCycle = GetBusCycle();
	if (pcBusCycle)
	{
		pcBusCycle->ToOperationString(psz);
	}
	else
	{
		psz->Append("---");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::GetStatusString(CChars* psz)
{
	psz->Append("a");
	psz->Append((uint32)(8 * GetRegisterBytes(R65_A)));
	psz->Append(" b");
	psz->Append((uint32)(8 * GetRegisterBytes(R65_B)));
	psz->Append(" x");
	psz->Append((uint32)(8 * GetRegisterBytes(R65_X)));
	psz->Append(" y");
	psz->Append((uint32)(8 * GetRegisterBytes(R65_Y)));
	psz->Append(IsFlagSet(STATUS65_EDGE_DISABLE) ? " e1" : " e0");
	psz->Append(IsFlagSet(STATUS65_LEVEL_DISABLE) ? " l1" : " l0");
	psz->Append(IsFlagSet(STATUS65_FLAG_F) ? " f1" : " f0");
	psz->Append(IsFlagSet(STATUS65_FLAG_G) ? " g1" : " g0");
	psz->Append(IsFlagSet(STATUS65_NEGATIVE) ? " n1" : " n0");
	psz->Append(IsFlagSet(STATUS65_OVERFLOW) ? " v1" : " v0");
	psz->Append(IsFlagSet(STATUS65_ZERO) ? " z1" : " z0");
	psz->Append(IsFlagSet(STATUS65_CARRY) ? " c1" : " c0");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::TestSetRegister(E65Register eRegister, uint64 uiValue)
{
	SetRegister(eRegister, uiValue, 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::TestSetProgramCounter(uint32 uiC)
{
	muiC = uiC;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::TestSetP(uint16 uiP)
{
	muiP = uiP;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::TestStartInstruction(void)
{
	//The next falling edge of PHI2 starts the fetch of the opcode at C.  Reset is skipped.
	mpcInstruction = NULL;
	miCycle = -1;
	miNextCycle = 0;
	mbStopped = false;
	mbWaiting = false;
	mbNextInstruction = true;
	ResetInstructionState();
}

