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
// A width in bits (wth's register operand) rounded up to 8, 16, 32 or 64.
//
//////////////////////////////////////////////////////////////////////////
uint8 BitsToWidthCode65(uint64 uiBits)
{
	if (uiBits <= 8)
	{
		return WIDTH65_8;
	}
	else if (uiBits <= 16)
	{
		return WIDTH65_16;
	}
	else if (uiBits <= 32)
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

	msState.uiA = 0;
	msState.uiB = 0;
	msState.uiX = 0;
	msState.uiY = 0;
	msState.uiS = RESET_65C64_STACK_POINTER;
	msState.uiV = 0;
	msState.uiW = 0;
	msState.uiC = 0;
	msState.uiP = STATUS65_EDGE_DISABLE | STATUS65_LEVEL_DISABLE;
	msState.uiInterruptDepth = 1;

	msState.uiOpcode = 0;
	memset(&msDecoded, 0, sizeof(S65C64Decoded));
	mpcInstruction = NULL;
	msState.iCycle = -1;
	msState.iNextCycle = 0;
	msState.bNextInstruction = false;
	msState.bStopped = false;

	msState.bWaiting = false;
	msState.uiWakeMask = 0;
	msState.uiPendingEdges = 0;
	msState.uiEdgeLow = 0;
	msState.bLevelInterrupt = false;

	msState.uiIOData = 0;
	msState.bPreviousClockLow = false;
	msState.bPreviousClockHigh = false;
	msState.bReset = false;

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
void C65C64::GetState(S65C64CoreState* psState)
{
	C65C64InstructionFactory*	pcFactory;
	S65C64Decoded				sDecoded;

	*psState = msState;
	psState->uiDecodedVector = 0;
	Decode65C64(msState.uiOpcode, &sDecoded);
	if (msDecoded.eInstruction == I65_Interrupt)
	{
		psState->eDecoded = DK65_Interrupt;
		psState->uiDecodedVector = msDecoded.uiVector;
	}
	else if (memcmp(&msDecoded, &sDecoded, sizeof(S65C64Decoded)) == 0)
	{
		psState->eDecoded = DK65_Opcode;
	}
	else
	{
		psState->eDecoded = DK65_None;
	}

	pcFactory = C65C64InstructionFactory::GetInstance();
	if (mpcInstruction == NULL)
	{
		psState->eInstruction = IK65_None;
	}
	else if (mpcInstruction == pcFactory->GetReset())
	{
		psState->eInstruction = IK65_Reset;
	}
	else if (mpcInstruction == pcFactory->GetFetchNext())
	{
		psState->eInstruction = IK65_FetchNext;
	}
	else if (mpcInstruction == pcFactory->GetInterrupt())
	{
		psState->eInstruction = IK65_Interrupt;
	}
	else if (mpcInstruction == pcFactory->GetWait())
	{
		psState->eInstruction = IK65_Wait;
	}
	else
	{
		psState->eInstruction = IK65_Opcode;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The core carries on from the state as if it had never stopped.  The
// instruction factory must be initialised.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetState(S65C64CoreState* psState)
{
	C65C64InstructionFactory*	pcFactory;

	msState = *psState;
	if (psState->eDecoded == DK65_Interrupt)
	{
		DecodeInterrupt(psState->uiDecodedVector);
	}
	else if (psState->eDecoded == DK65_Opcode)
	{
		Decode65C64(msState.uiOpcode, &msDecoded);
	}
	else
	{
		memset(&msDecoded, 0, sizeof(S65C64Decoded));
	}

	pcFactory = C65C64InstructionFactory::GetInstance();
	switch (psState->eInstruction)
	{
	case IK65_Opcode:
		mpcInstruction = pcFactory->GetInstruction(msState.uiOpcode);
		break;
	case IK65_Reset:
		mpcInstruction = pcFactory->GetReset();
		break;
	case IK65_FetchNext:
		mpcInstruction = pcFactory->GetFetchNext();
		break;
	case IK65_Interrupt:
		mpcInstruction = pcFactory->GetInterrupt();
		break;
	case IK65_Wait:
		mpcInstruction = pcFactory->GetWait();
		break;
	default:
		mpcInstruction = NULL;
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An interrupt is decoded as an instruction of its own that pushes on S.
//
//////////////////////////////////////////////////////////////////////////
void C65C64::DecodeInterrupt(uint16 uiVector)
{
	memset(&msDecoded, 0, sizeof(S65C64Decoded));
	msDecoded.eInstruction = I65_Interrupt;
	msDecoded.eStack = R65_S;
	msDecoded.uiVector = uiVector;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::ResetInstructionState(void)
{
	msState.uiDI = 0;
	msState.uiOperationBytes = 0;
	memset(msState.auiOperandWords, 0, sizeof(msState.auiOperandWords));
	msState.uiPointerAddress = 0;
	msState.uiPointer = 0;
	msState.uiNumPointerWords = 0;
	msState.uiEffectiveAddress = 0;
	msState.uiVectorAddress = 0;
	msState.uiVector = 0;
	msState.uiNumTransfers = 0;
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
		msState.bPreviousClockLow = sClock.IsLow();
		msState.bPreviousClockHigh = sClock.IsHigh();
		return;
	}

	SampleInterrupts(pcTimeline);

	if (sClock.IsHigh() || sClock.IsLow())
	{
		bRisingEdge = sClock.IsHigh() && msState.bPreviousClockLow;
		bFallingEdge = sClock.IsLow() && msState.bPreviousClockHigh;

		msState.bPreviousClockLow = sClock.IsLow();
		msState.bPreviousClockHigh = sClock.IsHigh();

		if (!msState.bStopped && !msState.bWaiting)
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
			msState.uiIOData = mpcPins->ReadData(pcTimeline);
		}
		else
		{
			//Written data is only driven while PHI2 is high.
			mpcPins->ReleaseData(pcTimeline);
		}

		GetBusCycle()->ExecuteTrailingSide(this, false);
	}

	Cycle();
	if (msState.bStopped)
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
			mpcPins->WriteData(pcTimeline, msState.uiIOData);
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

	if (msState.bStopped)
	{
		return;
	}

	for (;;)
	{
		if (msState.bNextInstruction)
		{
			msState.bNextInstruction = false;
			NextInstruction();
		}

		msState.iCycle = msState.iNextCycle;
		msState.iNextCycle++;

		//An instruction is done when none of its remaining bus cycles must execute.
		pcBusCycle = GetBusCycle();
		if (pcBusCycle == NULL)
		{
			msState.bNextInstruction = true;
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
	msState.iNextCycle = 0;
	if (EnterInterrupt())
	{
		return;
	}

	if (msState.bWaiting)
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
		if (bLow && !(msState.uiEdgeLow & uiBit))
		{
			msState.uiPendingEdges |= uiBit;
		}

		if (bLow)
		{
			msState.uiEdgeLow |= uiBit;
		}
		else
		{
			msState.uiEdgeLow &= ~uiBit;
		}
	}
	msState.bLevelInterrupt = mpcPins->ReadLIRQ(pcTimeline).IsLow();

	//An interrupt stp allows wakes the processor even if the interrupt is disabled.
	if (msState.bWaiting)
	{
		if (((msState.uiPendingEdges << 1) & msState.uiWakeMask) || (msState.bLevelInterrupt && (msState.uiWakeMask & WAKE65_LEVEL)))
		{
			msState.bWaiting = false;
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

	if (!IsFlagSet(STATUS65_EDGE_DISABLE) && (msState.uiPendingEdges != 0))
	{
		for (i = 0; i < NUM_65C64_EDGE_INTERRUPTS; i++)
		{
			uiBit = (uint8)(1 << i);
			if (msState.uiPendingEdges & uiBit)
			{
				msState.uiPendingEdges &= ~uiBit;
				StartInterrupt((uint16)(VECTOR_65C64_EDGE_0 - i), STATUS65_EDGE_DISABLE | STATUS65_LEVEL_DISABLE);
				return true;
			}
		}
	}

	if (!IsFlagSet(STATUS65_LEVEL_DISABLE) && msState.bLevelInterrupt)
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
	DecodeInterrupt(uiVector);

	PushTransfer(msState.uiC, 4);
	PushTransfer(msState.uiP, 2);
	msState.uiP |= uiDisable;
	msState.uiVectorAddress = VECTOR_65C64_BASE + 4 * uiVector;
	msState.uiVector = 0;
	msState.uiInterruptDepth++;

	msState.bWaiting = false;
	mpcInstruction = C65C64InstructionFactory::GetInstance()->GetInterrupt();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::DoneInstruction(void)
{
	msState.bNextInstruction = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::ResetPulled(void)
{
	mpcInstruction = C65C64InstructionFactory::GetInstance()->GetReset();
	msState.iCycle = -1;
	msState.iNextCycle = 0;
	msState.bNextInstruction = false;
	msState.bStopped = false;
	msState.bWaiting = false;
	msState.uiWakeMask = 0;
	msState.uiPendingEdges = 0;
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
	if ((mpcInstruction != NULL) && (msState.iCycle >= 0))
	{
		return mpcInstruction->GetBusCycle(msState.iCycle);
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
	return msState.iCycle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 C65C64::GetOpcode(void)
{
	return msState.uiOpcode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65C64::IsStopped(void)
{
	return msState.bStopped;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65C64::IsWaiting(void)
{
	return msState.bWaiting;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65C64::GetPendingEdges(void)
{
	return msState.uiPendingEdges;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::ReadOpcode(void)
{
	C65BusCycle*	pcBusCycle;

	msState.uiOpcode = msState.uiIOData;
	Decode65C64(msState.uiOpcode, &msDecoded);
	mpcInstruction = C65C64InstructionFactory::GetInstance()->GetInstruction(msState.uiOpcode);
	ResetInstructionState();

	//The opcode fetch is the instruction's first cycle.  An instruction executes in the
	//cycle its last word is read, so a register instruction is done in this cycle.
	msState.iCycle = 0;
	msState.iNextCycle = 1;
	pcBusCycle = GetBusCycle();
	pcBusCycle->ExecuteTrailingSide(this, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::IncrementProgramCounter(void)
{
	msState.uiC += 2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetOperandWord(uint8 uiWord, uint16 uiData)
{
	if (uiWord < MAX_65C64_OPERAND_WORDS)
	{
		msState.auiOperandWords[uiWord] = uiData;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 C65C64::GetPointerAddress(void)
{
	return msState.uiPointerAddress;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65C64::GetNumPointerWords(void)
{
	return msState.uiNumPointerWords;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65C64::GetPointerWordLanes(uint8 uiWord)
{
	return LanesAt(WordAddress(msState.uiPointerAddress, uiWord), msState.uiPointerAddress, 4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetPointerWord(uint8 uiWord, uint16 uiData)
{
	uint64	uiPointer;

	uiPointer = msState.uiPointer;
	MergeAt(&uiPointer, WordAddress(msState.uiPointerAddress, uiWord), msState.uiPointerAddress, 4, uiData);
	msState.uiPointer = (uint32)uiPointer;
}


//////////////////////////////////////////////////////////////////////////
//
// The number of words of the transfers that are written (bWrite) or read.
//
//////////////////////////////////////////////////////////////////////////
size C65C64::CountTransferWords(bool bWrite)
{
	S65C64Transfer*	psTransfer;
	size			uiNumWords;
	size			i;

	uiNumWords = 0;
	for (i = 0; i < msState.uiNumTransfers; i++)
	{
		psTransfer = &msState.asTransfers[i];
		if (bWrite ? psTransfer->bWrite : psTransfer->bRead)
		{
			uiNumWords += CountWords(psTransfer->uiAddress, psTransfer->uiBytes);
		}
	}
	return uiNumWords;
}


//////////////////////////////////////////////////////////////////////////
//
// The transfer uiWord of the written (bWrite) or read words is in, and the
// word's address, or NULL if there are not that many words.
//
//////////////////////////////////////////////////////////////////////////
S65C64Transfer* C65C64::FindTransferWord(uint8 uiWord, bool bWrite, uint32* puiWordAddress)
{
	S65C64Transfer*	psTransfer;
	size			uiNumWords;
	size			i;

	for (i = 0; i < msState.uiNumTransfers; i++)
	{
		psTransfer = &msState.asTransfers[i];
		if (bWrite ? psTransfer->bWrite : psTransfer->bRead)
		{
			uiNumWords = CountWords(psTransfer->uiAddress, psTransfer->uiBytes);
			if (uiWord < uiNumWords)
			{
				*puiWordAddress = WordAddress(psTransfer->uiAddress, uiWord);
				return psTransfer;
			}
			uiWord -= (uint8)uiNumWords;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::AddTransfer(uint32 uiAddress, uint8 uiBytes, uint64 uiValue, bool bRead, bool bWrite)
{
	S65C64Transfer*		psTransfer;

	if (msState.uiNumTransfers >= MAX_65C64_TRANSFERS)
	{
		LOG_ERROR("Too many transfers in one instruction.");
		return;
	}

	psTransfer = &msState.asTransfers[msState.uiNumTransfers];
	psTransfer->uiAddress = uiAddress;
	psTransfer->uiBytes = uiBytes;
	psTransfer->bRead = bRead;
	psTransfer->bWrite = bWrite;
	psTransfer->uiValue = uiValue;
	msState.uiNumTransfers++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size C65C64::GetNumReadWords(void)
{
	return CountTransferWords(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 C65C64::GetReadWordAddress(uint8 uiWord)
{
	uint32	uiWordAddress;

	if (FindTransferWord(uiWord, false, &uiWordAddress))
	{
		return uiWordAddress;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65C64::GetReadWordLanes(uint8 uiWord)
{
	S65C64Transfer*	psTransfer;
	uint32			uiWordAddress;

	psTransfer = FindTransferWord(uiWord, false, &uiWordAddress);
	if (psTransfer)
	{
		return LanesAt(uiWordAddress, psTransfer->uiAddress, psTransfer->uiBytes);
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetReadWord(uint8 uiWord, uint16 uiData)
{
	S65C64Transfer*	psTransfer;
	uint32			uiWordAddress;

	psTransfer = FindTransferWord(uiWord, false, &uiWordAddress);
	if (psTransfer)
	{
		MergeAt(&psTransfer->uiValue, uiWordAddress, psTransfer->uiAddress, psTransfer->uiBytes, uiData);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size C65C64::GetNumWriteWords(void)
{
	return CountTransferWords(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 C65C64::GetWriteWordAddress(uint8 uiWord)
{
	uint32	uiWordAddress;

	if (FindTransferWord(uiWord, true, &uiWordAddress))
	{
		return uiWordAddress;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65C64::GetWriteWordLanes(uint8 uiWord)
{
	S65C64Transfer*	psTransfer;
	uint32			uiWordAddress;

	psTransfer = FindTransferWord(uiWord, true, &uiWordAddress);
	if (psTransfer)
	{
		return LanesAt(uiWordAddress, psTransfer->uiAddress, psTransfer->uiBytes);
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 C65C64::GetWriteWord(uint8 uiWord)
{
	S65C64Transfer*	psTransfer;
	uint32			uiWordAddress;

	psTransfer = FindTransferWord(uiWord, true, &uiWordAddress);
	if (psTransfer)
	{
		return ExtractAt(psTransfer->uiValue, uiWordAddress, psTransfer->uiAddress, psTransfer->uiBytes);
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 C65C64::GetVectorAddress(void)
{
	return msState.uiVectorAddress;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetVectorWord(uint8 uiWord, uint16 uiData)
{
	msState.uiVector |= (uint32)uiData << (16 * uiWord);
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
	return msState.uiIOData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetIOData(uint16 uiData)
{
	msState.uiIOData = uiData;
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
		msState.uiDI = 0;
		for (i = 0; i < psDecoded->uiOperandWords; i++)
		{
			msState.uiDI |= (uint64)msState.auiOperandWords[i] << (16 * i);
		}
		msState.uiDI &= Mask65(psDecoded->sSecond.uiImmediateBytes);
	}

	msState.uiOperationBytes = CalculateOperationBytes();

	if ((psDecoded->sFirst.eKind == OK65_Memory) || (psDecoded->sSecond.eKind == OK65_Memory))
	{
		if (psDecoded->uiAddressWords == 2)
		{
			uiAddress = (uint32)msState.auiOperandWords[0] | ((uint32)msState.auiOperandWords[1] << 16);
		}
		else if (psDecoded->uiAddressWords == 1)
		{
			uiAddress = msState.auiOperandWords[0];
		}
		else
		{
			uiAddress = 0;
		}
		uiAddress += GetRegisterAddress(psDecoded->ePre1);
		uiAddress += GetRegisterAddress(psDecoded->ePre2);

		if (psDecoded->bIndirect)
		{
			msState.uiPointerAddress = uiAddress;
			msState.uiNumPointerWords = CountWords(uiAddress, 4);
			msState.uiPointer = 0;
		}
		else
		{
			msState.uiEffectiveAddress = uiAddress;
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
	msState.uiEffectiveAddress = msState.uiPointer + GetRegisterAddress(msDecoded.ePost);
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
		AddTransfer(msState.uiEffectiveAddress, msState.uiOperationBytes, 0, (msDecoded.eDataAccess == MA65_Read) || (msDecoded.eDataAccess == MA65_ReadWrite), (msDecoded.eDataAccess == MA65_Write) || (msDecoded.eDataAccess == MA65_ReadWrite));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::LoadVector(void)
{
	msState.uiC = msState.uiVector;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::Reset(void)
{
	msState.uiA = 0;
	msState.uiB = 0;
	msState.uiX = 0;
	msState.uiY = 0;
	msState.uiV = 0;
	msState.uiW = 0;
	msState.uiS = RESET_65C64_STACK_POINTER;
	msState.uiP = STATUS65_EDGE_DISABLE | STATUS65_LEVEL_DISABLE;
	msState.uiInterruptDepth = 1;

	msState.uiVectorAddress = VECTOR_65C64_BASE + 4 * VECTOR_65C64_RESET;
	msState.uiVector = 0;
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
		return Extend65(msState.uiDI, psOperand->uiImmediateBytes, uiBytes, bSigned);
	case OK65_Embedded:
		return (uint64)psOperand->iEmbedded & Mask65(uiBytes);
	case OK65_Memory:
		return msState.asTransfers[0].uiValue & Mask65(uiBytes);
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
		msState.asTransfers[0].uiValue = uiValue & Mask65(uiBytes);
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

	uiBytes = msState.uiOperationBytes;
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
		return (uint32)Extend65(msState.uiDI, psOperand->uiImmediateBytes, 4, msDecoded.bRelative);
	case OK65_Memory:
		return msState.uiEffectiveAddress;
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
		PushTransfer(msState.uiC, 4);
	}
	if (msDecoded.bPushP)
	{
		PushTransfer(msState.uiP, 2);
	}

	if (bJump)
	{
		if (msDecoded.bRelative)
		{
			msState.uiC += uiTarget;
		}
		else
		{
			msState.uiC = uiTarget;
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
		msState.uiP = (uint16)msState.asTransfers[uiTransfer].uiValue;
		uiTransfer++;
	}
	if (msDecoded.bPullC)
	{
		msState.uiC = (uint32)msState.asTransfers[uiTransfer].uiValue;
	}

	if ((msDecoded.eInstruction == I65_Rti) && (msState.uiInterruptDepth > 0))
	{
		msState.uiInterruptDepth--;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SEP(void)
{
	msState.uiP |= msDecoded.uiMask;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::CEP(void)
{
	msState.uiP &= ~(uint16)msDecoded.uiMask;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::WTH(void)
{
	uint8	uiWidthCode;

	//The embedded width is encoded in bytes, a register holds the width in bits.
	if (msDecoded.sSecond.eKind == OK65_Embedded)
	{
		uiWidthCode = BytesToWidthCode65(msDecoded.uiWidthBytes);
	}
	else
	{
		uiWidthCode = BitsToWidthCode65(GetOperandValue(&msDecoded.sSecond, 8, false));
	}

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
	PushTransfer(msState.uiC, 4);
	PushTransfer(msState.uiP, 2);
	msState.uiVectorAddress = VECTOR_65C64_BASE + 4 * msDecoded.uiVector;
	msState.uiVector = 0;
	msState.uiInterruptDepth++;
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
		msState.bWaiting = true;
		msState.uiWakeMask = msDecoded.uiMask;
	}
	else
	{
		msState.bStopped = true;
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
		return (int64)Extend65(msState.uiDI, msDecoded.sSecond.uiImmediateBytes, 8, true);
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
		msState.uiC += (uint32)GetBranchOffset();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::BAL(void)
{
	msState.uiC += (uint32)GetBranchOffset();
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
	uiBytes = msState.uiOperationBytes;
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
	uiBytes = msState.uiOperationBytes;
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
	uiWord = msState.auiOperandWords[0];
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
		eRegister = gae65Registers[(msState.auiOperandWords[0] >> 13) & 7];
		return GetRegister(eRegister) & Mask65(GetRegisterBytes(eRegister));
	}
	else if (msDecoded.uiIndexStyle == 6)
	{
		return (msState.auiOperandWords[0] >> 10) & 0x3F;
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
		uiWord = msState.auiOperandWords[0];
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
	msState.uiOperationBytes = GetRegisterBytes(msDecoded.eSource);
	msState.uiEffectiveAddress = msState.uiV;
	AddTransfer(msState.uiEffectiveAddress, msState.uiOperationBytes, 0, bRead, bWrite);
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

	uiBytes = msState.uiOperationBytes;
	uiData &= Mask65(uiBytes);
	if (bWrite)
	{
		msState.asTransfers[uiTransfer].uiValue = uiData;
	}
	if (bSetNegativeAndZero)
	{
		SetNegativeAndZero(uiData, uiBytes);
	}

	if (msDecoded.bDecrement)
	{
		msState.uiV -= uiBytes;
	}
	else
	{
		msState.uiV += uiBytes;
	}
	if (msDecoded.bVector)
	{
		if (msDecoded.bDecrementW)
		{
			msState.uiW -= uiBytes;
		}
		else
		{
			msState.uiW += uiBytes;
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
		msState.uiC -= 2;
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

	uiMemory = msState.asTransfers[0].uiValue;
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

	uiRead = msState.asTransfers[0].uiValue;
	uiRegister = GetRegister(msDecoded.eSource);
	SetNegativeAndZero(uiRead, msState.uiOperationBytes);
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
	msState.uiOperationBytes = msDecoded.uiWidthBytes;
	AddTransfer(msState.uiW, msState.uiOperationBytes, 0, true, false);
	AddTransfer(msState.uiV, msState.uiOperationBytes, 0, bReadV, bWriteV);
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

	uiW = msState.asTransfers[0].uiValue;
	uiV = msState.asTransfers[1].uiValue;
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
		SetNegativeAndZero(uiV, msState.uiOperationBytes);
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
		PushTransfer(msState.uiDI, msDecoded.sSecond.uiImmediateBytes);
	}
	else
	{
		PushTransfer(msState.asTransfers[0].uiValue, 4);
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
	AddTransfer(msState.uiEffectiveAddress, 4, msState.asTransfers[0].uiValue, false, true);
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
	uiStep = msState.uiW;
	if (uiStep < uiNybbles)
	{
		uiY = (uiStep == 0) ? 0 : GetRegister(R65_Y) & Mask65(uiYBytes);
		uiByte = (GetRegister(R65_B) >> (4 * uiStep)) & 0xFF;
		uiA = GetRegister(R65_A) & Mask65(GetRegisterBytes(R65_A));
		uiY = (uiY + ((uiA * uiByte) << (4 * uiStep))) & Mask65(uiYBytes);
		SetRegister(R65_Y, uiY, uiYBytes);
		msState.uiW = uiStep + 2;
	}

	bDone = msState.uiW >= uiNybbles;
	SetFlag(STATUS65_OVERFLOW, bDone);
	SetFlag(STATUS65_ZERO, (GetRegister(R65_Y) & Mask65(uiYBytes)) == 0);
	if (!bDone)
	{
		msState.uiC -= 2;
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
	if (msState.uiW < uiNybbles)
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
		msState.uiW++;
	}

	bDone = msState.uiW >= uiNybbles;
	SetFlag(STATUS65_OVERFLOW, bDone);
	SetFlag(STATUS65_ZERO, (GetRegister(R65_Y) & Mask65(uiYBytes)) == 0);
	SetFlag(STATUS65_CARRY, (GetRegister(R65_B) & Mask65(uiBBytes)) != 0);
	if (!bDone)
	{
		msState.uiC -= 2;
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
			msState.aePulled[msState.uiNumTransfers] = eRegister;
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

	for (i = 0; i < msState.uiNumTransfers; i++)
	{
		psTransfer = &msState.asTransfers[i];
		SetRegister(msState.aePulled[i], psTransfer->uiValue, psTransfer->uiBytes);
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
		return msState.uiA;
	case R65_B:
		return msState.uiB;
	case R65_S:
		return msState.uiS;
	case R65_V:
		return msState.uiV;
	case R65_W:
		return msState.uiW;
	case R65_X:
		return msState.uiX;
	case R65_Y:
		return msState.uiY;
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
		msState.uiA = (msState.uiA & ~uiMask) | (uiValue & uiMask);
		break;
	case R65_B:
		msState.uiB = (msState.uiB & ~uiMask) | (uiValue & uiMask);
		break;
	case R65_S:
		msState.uiS = (uint32)((msState.uiS & ~uiMask) | (uiValue & uiMask));
		break;
	case R65_V:
		msState.uiV = (uint32)((msState.uiV & ~uiMask) | (uiValue & uiMask));
		break;
	case R65_W:
		msState.uiW = (uint32)((msState.uiW & ~uiMask) | (uiValue & uiMask));
		break;
	case R65_X:
		msState.uiX = (msState.uiX & ~uiMask) | (uiValue & uiMask);
		break;
	case R65_Y:
		msState.uiY = (msState.uiY & ~uiMask) | (uiValue & uiMask);
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
		return (msState.uiP >> STATUS65_WIDTH_A_SHIFT) & 3;
	case R65_B:
		return (msState.uiP >> STATUS65_WIDTH_B_SHIFT) & 3;
	case R65_X:
		return (msState.uiP >> STATUS65_WIDTH_X_SHIFT) & 3;
	case R65_Y:
		return (msState.uiP >> STATUS65_WIDTH_Y_SHIFT) & 3;
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
	msState.uiP = (uint16)((msState.uiP & ~(3 << iShift)) | ((uiWidthCode & 3) << iShift));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 C65C64::GetA(void) { return msState.uiA; }
uint64 C65C64::GetB(void) { return msState.uiB; }
uint64 C65C64::GetX(void) { return msState.uiX; }
uint64 C65C64::GetY(void) { return msState.uiY; }
uint32 C65C64::GetS(void) { return msState.uiS; }
uint32 C65C64::GetV(void) { return msState.uiV; }
uint32 C65C64::GetW(void) { return msState.uiW; }
uint32 C65C64::GetProgramCounter(void) { return msState.uiC; }
uint16 C65C64::GetP(void) { return msState.uiP; }
uint32 C65C64::GetInterruptDepth(void) { return msState.uiInterruptDepth; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65C64::IsFlagSet(uint16 uiFlag)
{
	return (msState.uiP & uiFlag) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::SetFlag(uint16 uiFlag, bool bValue)
{
	if (bValue)
	{
		msState.uiP |= uiFlag;
	}
	else
	{
		msState.uiP &= ~uiFlag;
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
	msState.uiC = uiC;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::TestSetP(uint16 uiP)
{
	msState.uiP = uiP;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64::TestStartInstruction(void)
{
	//The next falling edge of PHI2 starts the fetch of the opcode at C.  Reset is skipped.
	mpcInstruction = NULL;
	msState.iCycle = -1;
	msState.iNextCycle = 0;
	msState.bStopped = false;
	msState.bWaiting = false;
	msState.bNextInstruction = true;
	ResetInstructionState();
}

