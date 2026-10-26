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
#include <string.h>
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Logger.h"
#include "W65C816InstructionFactory.h"
#include "StateXMLFields.h"
#include "W65C816Chip.h"


//////////////////////////////////////////////////////////////////////////
//
// Takes ownership of pcPins and wires the processor's signals to them.
// The instruction factory is made ready for the chip (see
// CW65C816InstructionFactory) until it's killed.
//
//////////////////////////////////////////////////////////////////////////
void CW65C816Chip::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CW65C816ChipPins* pcPins)
{
	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pcPins);

	CW65C816InstructionFactory::GetInstance()->Init();
	mcSignals.Init(pcPins->GetAddress(), pcPins->GetData(), pcPins->GetPHI2(), pcPins->GetRESB(), pcPins->GetRWB());
	mcSignals.SetImportantTraces(pcPins->GetVPA(), pcPins->GetVDA(), pcPins->GetVPB());
	mcSignals.SetOtherTraces(pcPins->GetMLB(), pcPins->GetMX(), pcPins->GetBE(), pcPins->GetE(), pcPins->GetRDY());
	mcSignals.SetInterruptTraces(pcPins->GetABORTB(), pcPins->GetIRQB(), pcPins->GetNMIB());
	mcMPU.Init(&mcSignals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816Chip::Kill(void)
{
	mcMPU.Kill();
	mcSignals.Kill();
	CW65C816InstructionFactory::GetInstance()->Kill();
	CIntegratedCircuit::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's reset, the processor starts out being reset.
//
//////////////////////////////////////////////////////////////////////////
void CW65C816Chip::SimulationStarted(CTimeline* pcTimeline)
{
	mcMPU.ResetPulled();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816Chip::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	mcMPU.InputTransition(pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CW65C816Chip::GetType(void) { return W65C816_CHIP_TYPE; }
CW65C816* CW65C816Chip::GetMPU(void) { return &mcMPU; }
CW65C816ChipPins* CW65C816Chip::GetChipPins(void) { return (CW65C816ChipPins*)mpcPins; }


//////////////////////////////////////////////////////////////////////////
//
// Every register, flag and latch of the core but its addresses: the one
// definition saving and loading both use.
//
//////////////////////////////////////////////////////////////////////////
static const SStateXMLField gasW65CCoreStateFields[] =
{
	STATE_XML_FIELD(SW65CCoreState, bEmulationFlag, "emulation", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bZeroFlag, "zero", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bNegativeFlag, "negative", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bDecimalFlag, "decimal", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bInterruptDisableFlag, "interruptDisable", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bAccumulatorWidthFlag, "accumulatorWidth", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bIndexWidthFlag, "indexWidth", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bCarryFlag, "carry", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bOverflowFlag, "overflow", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bBreakFlag, "break", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, uiAccumulator, "accumulator", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, uiXIndex, "xIndex", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, uiYIndex, "yIndex", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, uiDataBank, "dataBank", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, uiDirectPage, "directPage", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, uiStackPointer, "stackPointer", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, iCycle, "cycle", SXFT_Signed),
	STATE_XML_FIELD(SW65CCoreState, iNextCycle, "nextCycle", SXFT_Signed),
	STATE_XML_FIELD(SW65CCoreState, uiOpcodeIndex, "opcode", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, bStopped, "stopped", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bWaiting, "waiting", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, uiEmulationStatusBits, "emulationStatusBits", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, uiAbortProcessRegister, "abortProcessRegister", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, uiAbortAccumulator, "abortAccumulator", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, uiAbortXIndex, "abortXIndex", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, uiAbortYIndex, "abortYIndex", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, uiAbortDataBank, "abortDataBank", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, uiAbortDirectPage, "abortDirectPage", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, uiAbortStackPointer, "abortStackPointer", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, uiData, "data", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, uiIOData, "ioData", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, uiDirectOffset, "directOffset", SXFT_Unsigned),
	STATE_XML_FIELD(SW65CCoreState, bBusEnable, "busEnable", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bIrq, "irq", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bAbort, "abort", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bNextInstruction, "nextInstruction", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bNmi, "nmi", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bNmiLow, "nmiLow", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bPreviousClockLow, "previousClockLow", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, bPreviousClockHigh, "previousClockHigh", SXFT_Bool),
	STATE_XML_FIELD(SW65CCoreState, uiResetCount, "resetCount", SXFT_Unsigned),
};


//The core's addresses, each saved as its 24 bits: the bank above the offset.
struct SW65CCoreStateAddress
{
	const char*		szName;
	CW65Address		SW65CCoreState::*pcAddress;
};


static const SW65CCoreStateAddress gasW65CCoreStateAddresses[] =
{
	{ "programCounter", &SW65CCoreState::cProgramCounter },
	{ "abortProgramCounter", &SW65CCoreState::cAbortProgramCounter },
	{ "address", &SW65CCoreState::cAddress },
	{ "newProgramCounter", &SW65CCoreState::cNewProgramCounter },
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CW65C816Chip::GetStateXMLName(void) { return "W65C816State"; }


//////////////////////////////////////////////////////////////////////////
//
// Every register, flag and latch of the core as an attribute.
//
//////////////////////////////////////////////////////////////////////////
void CW65C816Chip::SaveXML(CXMLWriter* pcWriter)
{
	SW65CCoreState	sState;
	CW65Address*	pcAddress;
	size			i;

	GetMPU()->GetState(&sState);
	SaveStateXMLFields(pcWriter, gasW65CCoreStateFields, sizeof(gasW65CCoreStateFields) / sizeof(SStateXMLField), &sState);
	for (i = 0; i < sizeof(gasW65CCoreStateAddresses) / sizeof(SW65CCoreStateAddress); i++)
	{
		pcAddress = &(sState.*gasW65CCoreStateAddresses[i].pcAddress);
		pcWriter->Attribute(gasW65CCoreStateAddresses[i].szName, (int64)(((uint32)pcAddress->GetBank() << 16) | pcAddress->GetOffset()));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every attribute must be there; nothing is changed if one isn't.
//
//////////////////////////////////////////////////////////////////////////
bool CW65C816Chip::LoadXML(CMarkupTag* pcTag)
{
	SW65CCoreState	sState;
	int64			iValue;
	size			i;
	bool			bResult;

	memset(&sState, 0, sizeof(SW65CCoreState));
	bResult = LoadStateXMLFields(pcTag, gasW65CCoreStateFields, sizeof(gasW65CCoreStateFields) / sizeof(SStateXMLField), &sState);
	for (i = 0; i < sizeof(gasW65CCoreStateAddresses) / sizeof(SW65CCoreStateAddress); i++)
	{
		bResult &= LoadStateInt64(pcTag, gasW65CCoreStateAddresses[i].szName, 0, 0xFFFFFF, &iValue);
		(sState.*gasW65CCoreStateAddresses[i].pcAddress).Init((uint8)(iValue >> 16), (uint16)iValue);
	}
	if (!bResult)
	{
		return false;
	}
	GetMPU()->SetState(&sState);
	return true;
}

