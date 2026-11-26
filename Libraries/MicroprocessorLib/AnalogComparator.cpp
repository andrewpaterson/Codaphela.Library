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
#include "BaseLib/ConstructorCall.h"
#include "Voltage.h"
#include "Timeline.h"
#include "AnalogComparator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnalogComparator::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CAnalogComparatorPins> pPins)
{
	PreInit();
	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Its output is set when its inputs are first read.
//
//////////////////////////////////////////////////////////////////////////
void CAnalogComparator::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
// An input has crossed the other: the output is IN+ above IN-, unless
// they are within half the offset of each other (the input's transition
// is at the whole offset, so rounding doesn't lose it) or either has no
// voltage.
//
//////////////////////////////////////////////////////////////////////////
void CAnalogComparator::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	CAnalogComparatorPins*	pcPins;
	int64					iTime;
	float32					fPlus;
	float32					fMinus;

	if (!IsPowered(pcTimeline->GetTime()))
	{
		return;
	}
	pcPins = GetAnalogComparatorPins();
	iTime = pcTimeline->GetTime();
	fPlus = pcPins->GetPlus()->GetVoltage(iTime);
	fMinus = pcPins->GetMinus()->GetVoltage(iTime);
	if (!IsVoltage(fPlus) || !IsVoltage(fMinus))
	{
		return;
	}
	if (fPlus >= fMinus + ANALOG_COMPARATOR_OFFSET / 2)
	{
		pcPins->GetOut()->WriteBool(pcTimeline, true);
	}
	else if (fPlus <= fMinus - ANALOG_COMPARATOR_OFFSET / 2)
	{
		pcPins->GetOut()->WriteBool(pcTimeline, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CAnalogComparator::GetType(void) { return ANALOG_COMPARATOR_TYPE; }
CAnalogComparatorPins* CAnalogComparator::GetAnalogComparatorPins(void) { return (CAnalogComparatorPins*)&mpPins; }

