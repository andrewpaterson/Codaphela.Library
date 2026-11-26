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
#include "Trace.h"
#include "Pull.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPull::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, float32 fResistance)
{
	PreInit();
	CPassive::Init(pcContainingSubcircuitSimulation, szName);
	mpcSupplyPort = NewMalloc<CPowerInPort>();
	mpcSupplyPort->Init(PT_PowerIn, PULL_PIN_SUPPLY, this);
	mpcSignalPort = NewMalloc<CPullPort>();
	mpcSignalPort->Init(PULL_PIN_SIGNAL, this, mpcSupplyPort, fResistance);
	mfSupplyVoltage = NoVoltage();
	mfCapacitance = 0.0f;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPull::Class(void)
{
	CPassive::Class();

	U_Pointer(mpcSupplyPort);
	U_Pointer(mpcSignalPort);
	U_Float32(mfSupplyVoltage);
	U_Float32(mfCapacitance);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPull::SimulationStarted(CTimeline* pcTimeline)
{
	mfSupplyVoltage = mpcSupplyPort->GetVoltageIn(pcTimeline->GetTime());
	SetCapacitance(mfCapacitance);
}


//////////////////////////////////////////////////////////////////////////
//
// The supply has moved, or a port was connected or cut: if only pulls
// hold the signal its ports are told.  The signal connected, its trace
// takes the capacitance given.
//
//////////////////////////////////////////////////////////////////////////
void CPull::TraceConnected(CTimeline* pcTimeline, CPort* pcPort)
{
	int64		iTime;
	float32		fSupplyVoltage;
	CTrace*		pcSignal;

	if (pcPort == mpcSignalPort)
	{
		if ((mfCapacitance > 0.0f) && (mpcSignalPort->GetTrace() != NULL))
		{
			mpcSignalPort->GetTrace()->SetCapacitance(mfCapacitance);
		}
		return;
	}
	if (pcPort != mpcSupplyPort)
	{
		return;
	}
	iTime = pcTimeline->GetTime();
	fSupplyVoltage = mpcSupplyPort->GetVoltageIn(iTime);
	pcSignal = mpcSignalPort->GetTrace();
	if ((pcSignal != NULL) && (fSupplyVoltage != mfSupplyVoltage) && !pcSignal->HasStrongDriver(iTime))
	{
		pcSignal->NotifyVoltageChanged(pcTimeline, mfSupplyVoltage, mpcSignalPort);
	}
	mfSupplyVoltage = fSupplyVoltage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CPull::GetType(void) { return "Pull"; }
CPowerInPort* CPull::GetSupplyPort(void) const { return mpcSupplyPort; }
CPullPort* CPull::GetSignalPort(void) const { return mpcSignalPort; }
float32 CPull::GetCapacitance(void) const { return mfCapacitance; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPull::SetCapacitance(float32 fCapacitance)
{
	mfCapacitance = fCapacitance;
	if ((mfCapacitance > 0.0f) && (mpcSignalPort->GetTrace() != NULL))
	{
		mpcSignalPort->GetTrace()->SetCapacitance(mfCapacitance);
	}
}

