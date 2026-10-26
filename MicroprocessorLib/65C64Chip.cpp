/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Logger.h"
#include "65C64InstructionFactory.h"
#include "StateXMLFields.h"
#include "65C64Chip.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins and wires the processor's signals to them.
// The instruction factory is made ready for the chip (see
// C65C64InstructionFactory) until it's killed.
//
//////////////////////////////////////////////////////////////////////////
void C65C64Chip::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<C65C64ChipPins> pPins)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);

	C65C64InstructionFactory::GetInstance()->Init();
	mcSignals.Init(pPins->GetAddress(), pPins->GetData(), pPins->GetPHI2(), pPins->GetRESB(), pPins->GetRWB());
	mcSignals.SetByteTraces(pPins->GetLowerByte(), pPins->GetUpperByte());
	mcSignals.SetValidTraces(pPins->GetOPV(), pPins->GetVPA(), pPins->GetVDA(), pPins->GetVPB());
	mcSignals.SetInterruptTraces(pPins->GetEIRQB(0), pPins->GetEIRQB(1), pPins->GetEIRQB(2), pPins->GetEIRQB(3), pPins->GetLIRQB());
	mcMPU.Init(&mcSignals);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64Chip::Class(void)
{
	CIntegratedCircuit::Class();

	U_Data(C65C64, mcMPU);
	U_Data(C65C64Pins, mcSignals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64Chip::Free(void)
{
	mcMPU.Kill();
	mcSignals.Kill();
	C65C64InstructionFactory::GetInstance()->Kill();
	CIntegratedCircuit::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// The processor starts out being reset.
//
//////////////////////////////////////////////////////////////////////////
void C65C64Chip::SimulationStarted(CTimeline* pcTimeline)
{
	mcMPU.ResetPulled();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64Chip::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	mcMPU.InputTransition(pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* C65C64Chip::GetType(void) { return C65C64_CHIP_TYPE; }
C65C64* C65C64Chip::GetMPU(void) { return &mcMPU; }
C65C64ChipPins* C65C64Chip::GetChipPins(void) { return (C65C64ChipPins*)&mpPins; }


//////////////////////////////////////////////////////////////////////////
//
// The core's registers, latches and flags: the one definition saving and
// loading both use.  The operand words, what was decoded, the instruction
// and the transfers are saved after them.
//
//////////////////////////////////////////////////////////////////////////
static const SStateXMLField gas65C64CoreStateFields[] =
{
	STATE_XML_FIELD(S65C64CoreState, uiA, "a", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiB, "b", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiX, "x", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiY, "y", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiS, "s", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiV, "v", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiW, "w", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiC, "c", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiP, "p", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiInterruptDepth, "interruptDepth", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiDI, "di", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiOperationBytes, "operationBytes", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiPointerAddress, "pointerAddress", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiPointer, "pointer", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiNumPointerWords, "numPointerWords", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiEffectiveAddress, "effectiveAddress", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiVectorAddress, "vectorAddress", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiVector, "vector", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiOpcode, "opcode", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiDecodedVector, "decodedVector", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, iCycle, "cycle", SXFT_Signed),
	STATE_XML_FIELD(S65C64CoreState, iNextCycle, "nextCycle", SXFT_Signed),
	STATE_XML_FIELD(S65C64CoreState, bNextInstruction, "nextInstruction", SXFT_Bool),
	STATE_XML_FIELD(S65C64CoreState, bStopped, "stopped", SXFT_Bool),
	STATE_XML_FIELD(S65C64CoreState, bWaiting, "waiting", SXFT_Bool),
	STATE_XML_FIELD(S65C64CoreState, uiWakeMask, "wakeMask", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiPendingEdges, "pendingEdges", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, uiEdgeLow, "edgeLow", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, bLevelInterrupt, "levelInterrupt", SXFT_Bool),
	STATE_XML_FIELD(S65C64CoreState, uiIOData, "ioData", SXFT_Unsigned),
	STATE_XML_FIELD(S65C64CoreState, bPreviousClockLow, "previousClockLow", SXFT_Bool),
	STATE_XML_FIELD(S65C64CoreState, bPreviousClockHigh, "previousClockHigh", SXFT_Bool),
	STATE_XML_FIELD(S65C64CoreState, bReset, "reset", SXFT_Bool),
};


//The names of E65C64DecodedKind and E65C64InstructionKind, in order.
static const char* gasz65C64Decoded[] = { "none", "opcode", "interrupt" };
static const char* gasz65C64Instructions[] = { "none", "opcode", "reset", "fetchNext", "interrupt", "wait" };


//////////////////////////////////////////////////////////////////////////
//
// (XML names can't start with a digit, so the element is State65C64.)
//
//////////////////////////////////////////////////////////////////////////
const char* C65C64Chip::GetStateXMLName(void) { return "State65C64"; }


//////////////////////////////////////////////////////////////////////////
//
// The registers, latches and flags as attributes (64 bit values as their
// signed bits), the operand words as a list, and each transfer in progress
// as an element (its words follow from its address and bytes).
//
//////////////////////////////////////////////////////////////////////////
void C65C64Chip::SaveXML(CXMLWriter* pcWriter)
{
	S65C64CoreState		sState;
	CChars				szWords;
	size				i;

	GetMPU()->GetState(&sState);
	SaveStateXMLFields(pcWriter, gas65C64CoreStateFields, sizeof(gas65C64CoreStateFields) / sizeof(SStateXMLField), &sState);
	szWords.Init();
	for (i = 0; i < MAX_65C64_OPERAND_WORDS; i++)
	{
		if (i != 0)
		{
			szWords.Append(' ');
		}
		szWords.Append((int32)sState.auiOperandWords[i]);
	}
	pcWriter->Attribute("operandWords", szWords.Text());
	szWords.Kill();
	pcWriter->Attribute("decoded", gasz65C64Decoded[sState.eDecoded]);
	pcWriter->Attribute("instruction", gasz65C64Instructions[sState.eInstruction]);

	for (i = 0; i < sState.uiNumTransfers; i++)
	{
		pcWriter->Begin("Transfer");
		pcWriter->Attribute("address", (int64)sState.asTransfers[i].uiAddress);
		pcWriter->Attribute("bytes", (int32)sState.asTransfers[i].uiBytes);
		pcWriter->Attribute("read", sState.asTransfers[i].bRead);
		pcWriter->Attribute("write", sState.asTransfers[i].bWrite);
		pcWriter->Attribute("value", (int64)sState.asTransfers[i].uiValue);
		pcWriter->Attribute("pulled", (int32)sState.aePulled[i]);
		pcWriter->End();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every attribute must be there; nothing is changed if one isn't.  The
// 65C64 instruction factory must be initialised.
//
//////////////////////////////////////////////////////////////////////////
bool C65C64Chip::LoadXML(CMarkupTag* pcTag)
{
	S65C64CoreState		sState;
	int64				iValue;
	bool				bResult;
	char*				szWords;
	char*				szEnd;
	size				i;
	int					iKind;
	STagIterator		sIter;
	CMarkupTag*			pcTransfer;
	S65C64Transfer*		psTransfer;
	int64				iAddress;
	int64				iBytes;
	int64				iPulled;

	memset(&sState, 0, sizeof(S65C64CoreState));
	bResult = LoadStateXMLFields(pcTag, gas65C64CoreStateFields, sizeof(gas65C64CoreStateFields) / sizeof(SStateXMLField), &sState);

	szWords = pcTag->GetStringAttribute("operandWords");
	szEnd = NULL;
	for (i = 0; (i < MAX_65C64_OPERAND_WORDS) && bResult; i++)
	{
		iValue = (szWords != NULL) ? strtoll(szWords, &szEnd, 10) : -1;
		if ((szWords == NULL) || (szEnd == szWords) || (iValue < 0) || (iValue > 0xFFFF))
		{
			bResult = gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs four operandWords from 0 to 65535.", NULL);
			break;
		}
		sState.auiOperandWords[i] = (uint16)iValue;
		szWords = szEnd;
	}

	iKind = LoadStateName(pcTag, "decoded", gasz65C64Decoded, sizeof(gasz65C64Decoded) / sizeof(const char*));
	bResult &= iKind != -1;
	sState.eDecoded = (E65C64DecodedKind)((iKind != -1) ? iKind : 0);
	iKind = LoadStateName(pcTag, "instruction", gasz65C64Instructions, sizeof(gasz65C64Instructions) / sizeof(const char*));
	bResult &= iKind != -1;
	sState.eInstruction = (E65C64InstructionKind)((iKind != -1) ? iKind : 0);

	pcTransfer = pcTag->GetTag("Transfer", &sIter);
	while ((pcTransfer != NULL) && bResult)
	{
		psTransfer = &sState.asTransfers[sState.uiNumTransfers];
		if ((sState.uiNumTransfers >= MAX_65C64_TRANSFERS) ||
			!LoadStateInt64(pcTransfer, "address", 0, 0xFFFFFFFFLL, &iAddress) ||
			!LoadStateInt64(pcTransfer, "bytes", 0, 8, &iBytes) ||
			!LoadStateBool(pcTransfer, "read", &psTransfer->bRead) ||
			!LoadStateBool(pcTransfer, "write", &psTransfer->bWrite) ||
			!LoadStateInt64(pcTransfer, "value", INT64_MIN, INT64_MAX, &iValue) ||
			!LoadStateInt64(pcTransfer, "pulled", R65_Zero, R65_Y, &iPulled))
		{
			bResult = gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] has a bad or one too many Transfer.", NULL);
			break;
		}
		psTransfer->uiAddress = (uint32)iAddress;
		psTransfer->uiBytes = (uint8)iBytes;
		psTransfer->uiValue = (uint64)iValue;
		sState.aePulled[sState.uiNumTransfers] = (E65Register)iPulled;
		sState.uiNumTransfers++;
		pcTransfer = pcTag->GetNextTag(&sIter);
	}

	if (!bResult)
	{
		return false;
	}
	GetMPU()->SetState(&sState);
	return true;
}

