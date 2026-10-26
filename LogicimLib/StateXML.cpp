/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "MicroprocessorLib/ClockOscillator.h"
#include "MicroprocessorLib/Constant.h"
#include "MicroprocessorLib/Counter.h"
#include "MicroprocessorLib/DTypeFlipFlop.h"
#include "MicroprocessorLib/Oscilloscope.h"
#include "MicroprocessorLib/W65C816Chip.h"
#include "MicroprocessorLib/65C64Chip.h"
#include "BaseLib/LogString.h"
#include "StateXML.h"


CClockOscillatorStateXML	gcClockOscillatorStateXML;
CConstantStateXML			gcConstantStateXML;
CCounterStateXML			gcCounterStateXML;
CDTypeFlipFlopStateXML		gcDTypeFlipFlopStateXML;
COscilloscopeStateXML		gcOscilloscopeStateXML;
CW65C816ChipStateXML		gcW65C816ChipStateXML;
C65C64ChipStateXML			gc65C64ChipStateXML;

CStateXML* gapcStateXMLs[] =
{
	&gcClockOscillatorStateXML,
	&gcConstantStateXML,
	&gcCounterStateXML,
	&gcDTypeFlipFlopStateXML,
	&gcOscilloscopeStateXML,
	&gcW65C816ChipStateXML,
	&gc65C64ChipStateXML,
};


//////////////////////////////////////////////////////////////////////////
//
// NULL for an IC that remembers nothing (gates, buffers).
//
//////////////////////////////////////////////////////////////////////////
CStateXML* FindStateXMLForIntegratedCircuit(CIntegratedCircuit* pcIntegratedCircuit)
{
	size	i;

	for (i = 0; i < sizeof(gapcStateXMLs) / sizeof(CStateXML*); i++)
	{
		if (StringCompare(gapcStateXMLs[i]->GetIntegratedCircuitClassName(), pcIntegratedCircuit->ClassName()) == 0)
		{
			return gapcStateXMLs[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStateXML* FindStateXMLForElement(const char* szXMLName)
{
	size	i;

	for (i = 0; i < sizeof(gapcStateXMLs) / sizeof(CStateXML*); i++)
	{
		if (StringCompare(gapcStateXMLs[i]->GetXMLName(), szXMLName) == 0)
		{
			return gapcStateXMLs[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CClockOscillatorStateXML::GetIntegratedCircuitClassName(void) { return "CClockOscillator"; }
const char* CClockOscillatorStateXML::GetXMLName(void) { return "ClockOscillatorState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClockOscillatorStateXML::Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit)
{
	CClockOscillator*	pcClock;

	pcClock = (CClockOscillator*)pcIntegratedCircuit;
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("high", pcClock->IsHigh());
	pcWriter->Attribute("fullTicks", pcClock->GetFullTicks());
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CClockOscillatorStateXML::Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit)
{
	CClockOscillator*	pcClock;
	bool				bHigh;
	int64				iFullTicks;

	if (!pcTag->GetBoolAttribute("high", &bHigh) || !pcTag->GetInt64Attribute("fullTicks", &iFullTicks))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a true or false high and a whole number fullTicks.", NULL);
	}
	pcClock = (CClockOscillator*)pcIntegratedCircuit;
	pcClock->SetHigh(bHigh);
	pcClock->SetFullTicks(iFullTicks);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CConstantStateXML::GetIntegratedCircuitClassName(void) { return "CConstant"; }
const char* CConstantStateXML::GetXMLName(void) { return "ConstantState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConstantStateXML::Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit)
{
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("value", ((CConstant*)pcIntegratedCircuit)->GetConstantValue());
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CConstantStateXML::Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit)
{
	int64	iValue;

	if (!pcTag->GetInt64Attribute("value", &iValue))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a whole number value.", NULL);
	}
	((CConstant*)pcIntegratedCircuit)->SetConstantValue(iValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCounterStateXML::GetIntegratedCircuitClassName(void) { return "CCounter"; }
const char* CCounterStateXML::GetXMLName(void) { return "CounterState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCounterStateXML::Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit)
{
	CCounter*	pcCounter;

	pcCounter = (CCounter*)pcIntegratedCircuit;
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("count", (int64)pcCounter->GetCount());
	pcWriter->Attribute("terminal", pcCounter->IsTerminal());
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCounterStateXML::Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit)
{
	CCounter*	pcCounter;
	int64		iCount;
	bool		bTerminal;

	if (!pcTag->GetInt64Attribute("count", &iCount) || (iCount < 0) || (iCount > 0xffffffffLL) || !pcTag->GetBoolAttribute("terminal", &bTerminal))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a count from 0 to 4294967295 and a true or false terminal.", NULL);
	}
	pcCounter = (CCounter*)pcIntegratedCircuit;
	pcCounter->SetCount((uint32)iCount);
	pcCounter->SetTerminal(bTerminal);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CDTypeFlipFlopStateXML::GetIntegratedCircuitClassName(void) { return "CDTypeFlipFlop"; }
const char* CDTypeFlipFlopStateXML::GetXMLName(void) { return "DTypeFlipFlopState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDTypeFlipFlopStateXML::Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit)
{
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("state", ((CDTypeFlipFlop*)pcIntegratedCircuit)->GetState());
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDTypeFlipFlopStateXML::Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit)
{
	bool	bState;

	if (!pcTag->GetBoolAttribute("state", &bState))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a true or false state.", NULL);
	}
	((CDTypeFlipFlop*)pcIntegratedCircuit)->SetState(bState);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* COscilloscopeStateXML::GetIntegratedCircuitClassName(void) { return "COscilloscope"; }
const char* COscilloscopeStateXML::GetXMLName(void) { return "OscilloscopeState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COscilloscopeStateXML::Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit)
{
	COscilloscope*	pcScope;
	size			uiInput;
	size			uiSample;
	CChars			szMinimum;
	CChars			szMaximum;
	char			sz[48];

	pcScope = (COscilloscope*)pcIntegratedCircuit;
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("inputCount", (int64)pcScope->GetInputCount());
	pcWriter->Attribute("sampleCount", (int64)pcScope->GetSampleCount());
	pcWriter->Attribute("tickPosition", (int64)pcScope->GetTickPosition());
	for (uiInput = 0; uiInput < pcScope->GetInputCount(); uiInput++)
	{
		szMinimum.Init();
		szMaximum.Init();
		for (uiSample = 0; uiSample < pcScope->GetSampleCount(); uiSample++)
		{
			if (uiSample != 0)
			{
				szMinimum.Append(' ');
				szMaximum.Append(' ');
			}
			snprintf(sz, sizeof(sz), "%.9g", (double)pcScope->GetMinVoltage(uiInput, uiSample));
			szMinimum.Append(sz);
			snprintf(sz, sizeof(sz), "%.9g", (double)pcScope->GetMaxVoltage(uiInput, uiSample));
			szMaximum.Append(sz);
		}
		pcWriter->Begin("Samples");
		pcWriter->Attribute("input", (int64)uiInput);
		pcWriter->Attribute("min", szMinimum.Text());
		pcWriter->Attribute("max", szMaximum.Text());
		pcWriter->End();
		szMaximum.Kill();
		szMinimum.Kill();
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// Reads exactly uiCount space separated floats.
//
//////////////////////////////////////////////////////////////////////////
static bool ReadOscilloscopeVoltages(char* szText, float32* afVoltages, size uiCount)
{
	char*	szEnd;
	size	i;

	if (szText == NULL)
	{
		return false;
	}
	for (i = 0; i < uiCount; i++)
	{
		afVoltages[i] = (float32)strtod(szText, &szEnd);
		if (szEnd == szText)
		{
			return false;
		}
		szText = szEnd;
	}
	while (*szText == ' ')
	{
		szText++;
	}
	return *szText == '\0';
}


//////////////////////////////////////////////////////////////////////////
//
// Inputs without a <Samples> keep the samples they have.
//
//////////////////////////////////////////////////////////////////////////
bool COscilloscopeStateXML::Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit)
{
	COscilloscope*	pcScope;
	int64			iInputCount;
	int64			iSampleCount;
	int64			iTickPosition;
	int64			iInput;
	STagIterator	sIter;
	CMarkupTag*		pcSamples;
	float32*		afMinimum;
	float32*		afMaximum;
	size			uiSample;
	bool			bValid;

	pcScope = (COscilloscope*)pcIntegratedCircuit;
	if (!pcTag->GetInt64Attribute("inputCount", &iInputCount) || !pcTag->GetInt64Attribute("sampleCount", &iSampleCount) || !pcTag->GetInt64Attribute("tickPosition", &iTickPosition) ||
		(iInputCount != (int64)pcScope->GetInputCount()) || (iSampleCount != (int64)pcScope->GetSampleCount()) || (iTickPosition < 0) || (iTickPosition >= iSampleCount))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs the oscilloscope's inputCount and sampleCount and a tickPosition inside the samples.", NULL);
	}
	pcScope->SetTickPosition((size)iTickPosition);

	afMinimum = (float32*)malloc(sizeof(float32) * (size)iSampleCount);
	afMaximum = (float32*)malloc(sizeof(float32) * (size)iSampleCount);
	bValid = true;
	pcSamples = pcTag->GetTag("Samples", &sIter);
	while ((pcSamples != NULL) && bValid)
	{
		bValid = pcSamples->GetInt64Attribute("input", &iInput) && (iInput >= 0) && (iInput < iInputCount) &&
				 ReadOscilloscopeVoltages(pcSamples->GetStringAttribute("min"), afMinimum, (size)iSampleCount) &&
				 ReadOscilloscopeVoltages(pcSamples->GetStringAttribute("max"), afMaximum, (size)iSampleCount);
		if (bValid)
		{
			for (uiSample = 0; uiSample < (size)iSampleCount; uiSample++)
			{
				pcScope->SetVoltages((size)iInput, uiSample, afMinimum[uiSample], afMaximum[uiSample]);
			}
		}
		pcSamples = pcTag->GetNextTag(&sIter);
	}
	free(afMaximum);
	free(afMinimum);

	if (!bValid)
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] has a Samples element without an input in range and sampleCount min and max voltages.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool LoadStateBool(CMarkupTag* pcTag, const char* szName, bool* pbValue)
{
	if (!pcTag->GetBoolAttribute(szName, pbValue))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] ", szName, " must be true or false.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool LoadStateInt(CMarkupTag* pcTag, const char* szName, int32 iMinimum, int32 iMaximum, int32* piValue)
{
	if (!pcTag->GetInt32Attribute(szName, piValue) || (*piValue < iMinimum) || (*piValue > iMaximum))
	{
		*piValue = 0;
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] ", szName, " must be a whole number from ", IntToString(iMinimum), " to ", IntToString(iMaximum), ".", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CW65C816ChipStateXML::GetIntegratedCircuitClassName(void) { return "CW65C816Chip"; }
const char* CW65C816ChipStateXML::GetXMLName(void) { return "W65C816State"; }


//////////////////////////////////////////////////////////////////////////
//
// Every register, flag and latch of the core as an attribute; addresses
// are a bank and an offset.
//
//////////////////////////////////////////////////////////////////////////
void CW65C816ChipStateXML::Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit)
{
	SW65CCoreState	sState;

	((CW65C816Chip*)pcIntegratedCircuit)->GetMPU()->GetState(&sState);
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("emulation", sState.bEmulationFlag);
	pcWriter->Attribute("zero", sState.bZeroFlag);
	pcWriter->Attribute("negative", sState.bNegativeFlag);
	pcWriter->Attribute("decimal", sState.bDecimalFlag);
	pcWriter->Attribute("interruptDisable", sState.bInterruptDisableFlag);
	pcWriter->Attribute("accumulatorWidth", sState.bAccumulatorWidthFlag);
	pcWriter->Attribute("indexWidth", sState.bIndexWidthFlag);
	pcWriter->Attribute("carry", sState.bCarryFlag);
	pcWriter->Attribute("overflow", sState.bOverflowFlag);
	pcWriter->Attribute("break", sState.bBreakFlag);
	pcWriter->Attribute("accumulator", (int32)sState.uiAccumulator);
	pcWriter->Attribute("xIndex", (int32)sState.uiXIndex);
	pcWriter->Attribute("yIndex", (int32)sState.uiYIndex);
	pcWriter->Attribute("dataBank", (int32)sState.uiDataBank);
	pcWriter->Attribute("directPage", (int32)sState.uiDirectPage);
	pcWriter->Attribute("programBank", (int32)sState.cProgramCounter.GetBank());
	pcWriter->Attribute("programCounter", (int32)sState.cProgramCounter.GetOffset());
	pcWriter->Attribute("stackPointer", (int32)sState.uiStackPointer);
	pcWriter->Attribute("cycle", (int32)sState.iCycle);
	pcWriter->Attribute("nextCycle", (int32)sState.iNextCycle);
	pcWriter->Attribute("opcode", (int32)sState.uiOpcodeIndex);
	pcWriter->Attribute("stopped", sState.bStopped);
	pcWriter->Attribute("waiting", sState.bWaiting);
	pcWriter->Attribute("emulationStatusBits", (int32)sState.uiEmulationStatusBits);
	pcWriter->Attribute("abortProcessRegister", (int32)sState.uiAbortProcessRegister);
	pcWriter->Attribute("abortAccumulator", (int32)sState.uiAbortAccumulator);
	pcWriter->Attribute("abortXIndex", (int32)sState.uiAbortXIndex);
	pcWriter->Attribute("abortYIndex", (int32)sState.uiAbortYIndex);
	pcWriter->Attribute("abortDataBank", (int32)sState.uiAbortDataBank);
	pcWriter->Attribute("abortDirectPage", (int32)sState.uiAbortDirectPage);
	pcWriter->Attribute("abortProgramBank", (int32)sState.cAbortProgramCounter.GetBank());
	pcWriter->Attribute("abortProgramCounter", (int32)sState.cAbortProgramCounter.GetOffset());
	pcWriter->Attribute("abortStackPointer", (int32)sState.uiAbortStackPointer);
	pcWriter->Attribute("addressBank", (int32)sState.cAddress.GetBank());
	pcWriter->Attribute("address", (int32)sState.cAddress.GetOffset());
	pcWriter->Attribute("data", (int32)sState.uiData);
	pcWriter->Attribute("ioData", (int32)sState.uiIOData);
	pcWriter->Attribute("directOffset", (int32)sState.uiDirectOffset);
	pcWriter->Attribute("newProgramBank", (int32)sState.cNewProgramCounter.GetBank());
	pcWriter->Attribute("newProgramCounter", (int32)sState.cNewProgramCounter.GetOffset());
	pcWriter->Attribute("busEnable", sState.bBusEnable);
	pcWriter->Attribute("irq", sState.bIrq);
	pcWriter->Attribute("abort", sState.bAbort);
	pcWriter->Attribute("nextInstruction", sState.bNextInstruction);
	pcWriter->Attribute("nmi", sState.bNmi);
	pcWriter->Attribute("nmiLow", sState.bNmiLow);
	pcWriter->Attribute("previousClockLow", sState.bPreviousClockLow);
	pcWriter->Attribute("previousClockHigh", sState.bPreviousClockHigh);
	pcWriter->Attribute("resetCount", (int32)sState.uiResetCount);
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// Every attribute must be there; nothing is changed if one isn't.
//
//////////////////////////////////////////////////////////////////////////
bool CW65C816ChipStateXML::Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit)
{
	SW65CCoreState	sState;
	int32			iValue;
	int32			iBank;
	bool			bResult;

	bResult = true;
	iValue = 0;
	iBank = 0;
	bResult &= LoadStateBool(pcTag, "emulation", &sState.bEmulationFlag);
	bResult &= LoadStateBool(pcTag, "zero", &sState.bZeroFlag);
	bResult &= LoadStateBool(pcTag, "negative", &sState.bNegativeFlag);
	bResult &= LoadStateBool(pcTag, "decimal", &sState.bDecimalFlag);
	bResult &= LoadStateBool(pcTag, "interruptDisable", &sState.bInterruptDisableFlag);
	bResult &= LoadStateBool(pcTag, "accumulatorWidth", &sState.bAccumulatorWidthFlag);
	bResult &= LoadStateBool(pcTag, "indexWidth", &sState.bIndexWidthFlag);
	bResult &= LoadStateBool(pcTag, "carry", &sState.bCarryFlag);
	bResult &= LoadStateBool(pcTag, "overflow", &sState.bOverflowFlag);
	bResult &= LoadStateBool(pcTag, "break", &sState.bBreakFlag);
	bResult &= LoadStateInt(pcTag, "accumulator", 0, 0xFFFF, &iValue);
	sState.uiAccumulator = (uint16)iValue;
	bResult &= LoadStateInt(pcTag, "xIndex", 0, 0xFFFF, &iValue);
	sState.uiXIndex = (uint16)iValue;
	bResult &= LoadStateInt(pcTag, "yIndex", 0, 0xFFFF, &iValue);
	sState.uiYIndex = (uint16)iValue;
	bResult &= LoadStateInt(pcTag, "dataBank", 0, 0xFF, &iValue);
	sState.uiDataBank = (uint8)iValue;
	bResult &= LoadStateInt(pcTag, "directPage", 0, 0xFFFF, &iValue);
	sState.uiDirectPage = (uint16)iValue;
	bResult &= LoadStateInt(pcTag, "programBank", 0, 0xFF, &iBank);
	bResult &= LoadStateInt(pcTag, "programCounter", 0, 0xFFFF, &iValue);
	sState.cProgramCounter.Init((uint8)iBank, (uint16)iValue);
	bResult &= LoadStateInt(pcTag, "stackPointer", 0, 0xFFFF, &iValue);
	sState.uiStackPointer = (uint16)iValue;
	bResult &= LoadStateInt(pcTag, "cycle", -0x8000, 0x7FFF, &iValue);
	sState.iCycle = (int16)iValue;
	bResult &= LoadStateInt(pcTag, "nextCycle", -0x8000, 0x7FFF, &iValue);
	sState.iNextCycle = (int16)iValue;
	bResult &= LoadStateInt(pcTag, "opcode", 0, 0xFFFF, &iValue);
	sState.uiOpcodeIndex = (uint16)iValue;
	bResult &= LoadStateBool(pcTag, "stopped", &sState.bStopped);
	bResult &= LoadStateBool(pcTag, "waiting", &sState.bWaiting);
	bResult &= LoadStateInt(pcTag, "emulationStatusBits", 0, 0xFF, &iValue);
	sState.uiEmulationStatusBits = (uint8)iValue;
	bResult &= LoadStateInt(pcTag, "abortProcessRegister", 0, 0xFF, &iValue);
	sState.uiAbortProcessRegister = (uint8)iValue;
	bResult &= LoadStateInt(pcTag, "abortAccumulator", 0, 0xFFFF, &iValue);
	sState.uiAbortAccumulator = (uint16)iValue;
	bResult &= LoadStateInt(pcTag, "abortXIndex", 0, 0xFFFF, &iValue);
	sState.uiAbortXIndex = (uint16)iValue;
	bResult &= LoadStateInt(pcTag, "abortYIndex", 0, 0xFFFF, &iValue);
	sState.uiAbortYIndex = (uint16)iValue;
	bResult &= LoadStateInt(pcTag, "abortDataBank", 0, 0xFF, &iValue);
	sState.uiAbortDataBank = (uint8)iValue;
	bResult &= LoadStateInt(pcTag, "abortDirectPage", 0, 0xFFFF, &iValue);
	sState.uiAbortDirectPage = (uint16)iValue;
	bResult &= LoadStateInt(pcTag, "abortProgramBank", 0, 0xFF, &iBank);
	bResult &= LoadStateInt(pcTag, "abortProgramCounter", 0, 0xFFFF, &iValue);
	sState.cAbortProgramCounter.Init((uint8)iBank, (uint16)iValue);
	bResult &= LoadStateInt(pcTag, "abortStackPointer", 0, 0xFFFF, &iValue);
	sState.uiAbortStackPointer = (uint16)iValue;
	bResult &= LoadStateInt(pcTag, "addressBank", 0, 0xFF, &iBank);
	bResult &= LoadStateInt(pcTag, "address", 0, 0xFFFF, &iValue);
	sState.cAddress.Init((uint8)iBank, (uint16)iValue);
	bResult &= LoadStateInt(pcTag, "data", 0, 0xFFFF, &iValue);
	sState.uiData = (uint16)iValue;
	bResult &= LoadStateInt(pcTag, "ioData", 0, 0xFF, &iValue);
	sState.uiIOData = (uint8)iValue;
	bResult &= LoadStateInt(pcTag, "directOffset", 0, 0xFFFF, &iValue);
	sState.uiDirectOffset = (uint16)iValue;
	bResult &= LoadStateInt(pcTag, "newProgramBank", 0, 0xFF, &iBank);
	bResult &= LoadStateInt(pcTag, "newProgramCounter", 0, 0xFFFF, &iValue);
	sState.cNewProgramCounter.Init((uint8)iBank, (uint16)iValue);
	bResult &= LoadStateBool(pcTag, "busEnable", &sState.bBusEnable);
	bResult &= LoadStateBool(pcTag, "irq", &sState.bIrq);
	bResult &= LoadStateBool(pcTag, "abort", &sState.bAbort);
	bResult &= LoadStateBool(pcTag, "nextInstruction", &sState.bNextInstruction);
	bResult &= LoadStateBool(pcTag, "nmi", &sState.bNmi);
	bResult &= LoadStateBool(pcTag, "nmiLow", &sState.bNmiLow);
	bResult &= LoadStateBool(pcTag, "previousClockLow", &sState.bPreviousClockLow);
	bResult &= LoadStateBool(pcTag, "previousClockHigh", &sState.bPreviousClockHigh);
	bResult &= LoadStateInt(pcTag, "resetCount", 0, 0xFFFF, &iValue);
	sState.uiResetCount = (uint16)iValue;
	if (!bResult)
	{
		return false;
	}
	((CW65C816Chip*)pcIntegratedCircuit)->GetMPU()->SetState(&sState);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static const char* gasz65C64Decoded[] = { "none", "opcode", "interrupt" };
static const char* gasz65C64Instructions[] = { "none", "opcode", "reset", "fetchNext", "interrupt", "wait" };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool LoadStateInt64(CMarkupTag* pcTag, const char* szName, int64 iMinimum, int64 iMaximum, int64* piValue)
{
	if (!pcTag->GetInt64Attribute(szName, piValue) || (*piValue < iMinimum) || (*piValue > iMaximum))
	{
		*piValue = 0;
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] ", szName, " must be a whole number in range.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The index of szValue in aszNames, or -1 (and an error logged).
//
//////////////////////////////////////////////////////////////////////////
static int LoadStateName(CMarkupTag* pcTag, const char* szName, const char** aszNames, int iNumNames)
{
	char*	szValue;
	int		i;

	szValue = pcTag->GetStringAttribute(szName);
	if (szValue != NULL)
	{
		for (i = 0; i < iNumNames; i++)
		{
			if (StringCompare(szValue, aszNames[i]) == 0)
			{
				return i;
			}
		}
	}
	gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] has no ", szName, " or an unknown one [", (szValue != NULL) ? szValue : "", "].", NULL);
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* C65C64ChipStateXML::GetIntegratedCircuitClassName(void) { return "C65C64Chip"; }
const char* C65C64ChipStateXML::GetXMLName(void) { return "State65C64"; }


//////////////////////////////////////////////////////////////////////////
//
// The registers, latches and flags as attributes (64 bit values as their
// signed bits), the operand words as a list, and each transfer and read
// and write word in progress as an element.
//
//////////////////////////////////////////////////////////////////////////
void C65C64ChipStateXML::Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit)
{
	S65C64CoreState		sState;
	CChars				szWords;
	size				i;

	((C65C64Chip*)pcIntegratedCircuit)->GetMPU()->GetState(&sState);
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("a", (int64)sState.uiA);
	pcWriter->Attribute("b", (int64)sState.uiB);
	pcWriter->Attribute("x", (int64)sState.uiX);
	pcWriter->Attribute("y", (int64)sState.uiY);
	pcWriter->Attribute("s", (int64)sState.uiS);
	pcWriter->Attribute("v", (int64)sState.uiV);
	pcWriter->Attribute("w", (int64)sState.uiW);
	pcWriter->Attribute("c", (int64)sState.uiC);
	pcWriter->Attribute("p", (int64)sState.uiP);
	pcWriter->Attribute("interruptDepth", (int64)sState.uiInterruptDepth);
	pcWriter->Attribute("di", (int64)sState.uiDI);
	pcWriter->Attribute("operationBytes", (int64)sState.uiOperationBytes);
	pcWriter->Attribute("pointerAddress", (int64)sState.uiPointerAddress);
	pcWriter->Attribute("pointer", (int64)sState.uiPointer);
	pcWriter->Attribute("numPointerWords", (int64)sState.uiNumPointerWords);
	pcWriter->Attribute("effectiveAddress", (int64)sState.uiEffectiveAddress);
	pcWriter->Attribute("vectorAddress", (int64)sState.uiVectorAddress);
	pcWriter->Attribute("vector", (int64)sState.uiVector);
	pcWriter->Attribute("opcode", (int64)sState.uiOpcode);
	pcWriter->Attribute("decodedVector", (int64)sState.uiDecodedVector);
	pcWriter->Attribute("cycle", (int64)sState.iCycle);
	pcWriter->Attribute("nextCycle", (int64)sState.iNextCycle);
	pcWriter->Attribute("nextInstruction", sState.bNextInstruction);
	pcWriter->Attribute("stopped", sState.bStopped);
	pcWriter->Attribute("waiting", sState.bWaiting);
	pcWriter->Attribute("wakeMask", (int64)sState.uiWakeMask);
	pcWriter->Attribute("pendingEdges", (int64)sState.uiPendingEdges);
	pcWriter->Attribute("edgeLow", (int64)sState.uiEdgeLow);
	pcWriter->Attribute("levelInterrupt", sState.bLevelInterrupt);
	pcWriter->Attribute("ioData", (int64)sState.uiIOData);
	pcWriter->Attribute("previousClockLow", sState.bPreviousClockLow);
	pcWriter->Attribute("previousClockHigh", sState.bPreviousClockHigh);
	pcWriter->Attribute("reset", sState.bReset);
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
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// Every attribute must be there; nothing is changed if one isn't.  The
// 65C64 instruction factory must be initialised.
//
//////////////////////////////////////////////////////////////////////////
bool C65C64ChipStateXML::Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit)
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
	int64				iAddress;
	int64				iBytes;
	int64				iPulled;

	memset(&sState, 0, sizeof(S65C64CoreState));
	bResult = true;
	iValue = 0;
	bResult &= LoadStateInt64(pcTag, "a", INT64_MIN, INT64_MAX, &iValue);
	sState.uiA = (uint64)iValue;
	bResult &= LoadStateInt64(pcTag, "b", INT64_MIN, INT64_MAX, &iValue);
	sState.uiB = (uint64)iValue;
	bResult &= LoadStateInt64(pcTag, "x", INT64_MIN, INT64_MAX, &iValue);
	sState.uiX = (uint64)iValue;
	bResult &= LoadStateInt64(pcTag, "y", INT64_MIN, INT64_MAX, &iValue);
	sState.uiY = (uint64)iValue;
	bResult &= LoadStateInt64(pcTag, "s", 0, 0xFFFFFFFFLL, &iValue);
	sState.uiS = (uint32)iValue;
	bResult &= LoadStateInt64(pcTag, "v", 0, 0xFFFFFFFFLL, &iValue);
	sState.uiV = (uint32)iValue;
	bResult &= LoadStateInt64(pcTag, "w", 0, 0xFFFFFFFFLL, &iValue);
	sState.uiW = (uint32)iValue;
	bResult &= LoadStateInt64(pcTag, "c", 0, 0xFFFFFFFFLL, &iValue);
	sState.uiC = (uint32)iValue;
	bResult &= LoadStateInt64(pcTag, "p", 0, 0xFFFF, &iValue);
	sState.uiP = (uint16)iValue;
	bResult &= LoadStateInt64(pcTag, "interruptDepth", 0, 0xFFFFFFFFLL, &iValue);
	sState.uiInterruptDepth = (uint32)iValue;
	bResult &= LoadStateInt64(pcTag, "di", INT64_MIN, INT64_MAX, &iValue);
	sState.uiDI = (uint64)iValue;
	bResult &= LoadStateInt64(pcTag, "operationBytes", 0, 0xFF, &iValue);
	sState.uiOperationBytes = (uint8)iValue;
	bResult &= LoadStateInt64(pcTag, "pointerAddress", 0, 0xFFFFFFFFLL, &iValue);
	sState.uiPointerAddress = (uint32)iValue;
	bResult &= LoadStateInt64(pcTag, "pointer", 0, 0xFFFFFFFFLL, &iValue);
	sState.uiPointer = (uint32)iValue;
	bResult &= LoadStateInt64(pcTag, "numPointerWords", 0, 0xFF, &iValue);
	sState.uiNumPointerWords = (uint8)iValue;
	bResult &= LoadStateInt64(pcTag, "effectiveAddress", 0, 0xFFFFFFFFLL, &iValue);
	sState.uiEffectiveAddress = (uint32)iValue;
	bResult &= LoadStateInt64(pcTag, "vectorAddress", 0, 0xFFFFFFFFLL, &iValue);
	sState.uiVectorAddress = (uint32)iValue;
	bResult &= LoadStateInt64(pcTag, "vector", 0, 0xFFFFFFFFLL, &iValue);
	sState.uiVector = (uint32)iValue;
	bResult &= LoadStateInt64(pcTag, "opcode", 0, 0xFFFF, &iValue);
	sState.uiOpcode = (uint16)iValue;
	bResult &= LoadStateInt64(pcTag, "decodedVector", 0, 0xFFFF, &iValue);
	sState.uiDecodedVector = (uint16)iValue;
	bResult &= LoadStateInt64(pcTag, "cycle", -0x8000, 0x7FFF, &iValue);
	sState.iCycle = (int16)iValue;
	bResult &= LoadStateInt64(pcTag, "nextCycle", -0x8000, 0x7FFF, &iValue);
	sState.iNextCycle = (int16)iValue;
	bResult &= LoadStateBool(pcTag, "nextInstruction", &sState.bNextInstruction);
	bResult &= LoadStateBool(pcTag, "stopped", &sState.bStopped);
	bResult &= LoadStateBool(pcTag, "waiting", &sState.bWaiting);
	bResult &= LoadStateInt64(pcTag, "wakeMask", 0, 0xFF, &iValue);
	sState.uiWakeMask = (uint8)iValue;
	bResult &= LoadStateInt64(pcTag, "pendingEdges", 0, 0xFF, &iValue);
	sState.uiPendingEdges = (uint8)iValue;
	bResult &= LoadStateInt64(pcTag, "edgeLow", 0, 0xFF, &iValue);
	sState.uiEdgeLow = (uint8)iValue;
	bResult &= LoadStateBool(pcTag, "levelInterrupt", &sState.bLevelInterrupt);
	bResult &= LoadStateInt64(pcTag, "ioData", 0, 0xFFFF, &iValue);
	sState.uiIOData = (uint16)iValue;
	bResult &= LoadStateBool(pcTag, "previousClockLow", &sState.bPreviousClockLow);
	bResult &= LoadStateBool(pcTag, "previousClockHigh", &sState.bPreviousClockHigh);
	bResult &= LoadStateBool(pcTag, "reset", &sState.bReset);

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
		if ((sState.uiNumTransfers >= MAX_65C64_TRANSFERS) ||
			!LoadStateInt64(pcTransfer, "address", 0, 0xFFFFFFFFLL, &iAddress) ||
			!LoadStateInt64(pcTransfer, "bytes", 0, 8, &iBytes) ||
			!LoadStateBool(pcTransfer, "read", &sState.asTransfers[sState.uiNumTransfers].bRead) ||
			!LoadStateBool(pcTransfer, "write", &sState.asTransfers[sState.uiNumTransfers].bWrite) ||
			!LoadStateInt64(pcTransfer, "value", INT64_MIN, INT64_MAX, &iValue) ||
			!LoadStateInt64(pcTransfer, "pulled", R65_Zero, R65_Y, &iPulled))
		{
			bResult = gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] has a bad or one too many Transfer.", NULL);
			break;
		}
		sState.asTransfers[sState.uiNumTransfers].uiAddress = (uint32)iAddress;
		sState.asTransfers[sState.uiNumTransfers].uiBytes = (uint8)iBytes;
		sState.asTransfers[sState.uiNumTransfers].uiValue = (uint64)iValue;
		sState.aePulled[sState.uiNumTransfers] = (E65Register)iPulled;
		sState.uiNumTransfers++;
		pcTransfer = pcTag->GetNextTag(&sIter);
	}

	if (!bResult)
	{
		return false;
	}
	((C65C64Chip*)pcIntegratedCircuit)->GetMPU()->SetState(&sState);
	return true;
}
