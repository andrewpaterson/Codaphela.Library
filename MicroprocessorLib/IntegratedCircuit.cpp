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
#include "BaseLib/Logger.h"
#include "BaseLib/PointerFunctions.h"
#include "Voltage.h"
#include "Timeline.h"
#include "PowerInPort.h"
#include "LogicPort.h"
#include "Circuit.h"
#include "SubcircuitSimulation.h"
#include "IntegratedCircuitEvent.h"
#include "IntegratedCircuit.h"


//////////////////////////////////////////////////////////////////////////
//
// Takes ownership of pcPins and adds the chip to its circuit, which owns
// it.  The state is created by Reset.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CPins* pcPins)
{
	CCircuitComponent::Init(pcContainingSubcircuitSimulation, szName);
	mpcPins = pcPins;
	mpcPins->SetIntegratedCircuit(this);
	mapcEvents.Init();
	GetCircuit()->Add(this);
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the chip's and its ports' events off the timeline and frees its
// pins and state, which disconnects its ports without telling anyone.  To
// take a chip out of a running simulation call DisconnectPorts first.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::Kill(void)
{
	CTimeline*	pcTimeline;
	size		i;

	pcTimeline = GetTimeline();
	Disconnect(pcTimeline);
	for (i = 0; i < mpcPins->NumPorts(); i++)
	{
		if (mpcPins->GetPort(i)->IsLogicPort())
		{
			((CLogicPort*)mpcPins->GetPort(i))->RemoveEvents(pcTimeline);
		}
	}
	mapcEvents.Kill();

	SafeKill(mpcPins);
	CCircuitComponent::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Called by a CTickEvent.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::ExecuteTick(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::TraceConnected(CTimeline* pcTimeline, CPort* pcPort)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the chip's own events off the timeline, as Logicim's
// IntegratedCircuit.disconnect.  The ports are disconnected separately.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::Disconnect(CTimeline* pcTimeline)
{
	CArrayEventPtr	apcEvents;
	size			i;

	apcEvents.Init();
	for (i = 0; i < mapcEvents.NumElements(); i++)
	{
		apcEvents.Add(mapcEvents.GetPtr(i));
	}
	pcTimeline->RemoveAll(&apcEvents);
	apcEvents.Kill();
	mapcEvents.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Starts the chip again with new state, and drives its outputs from it.
// Its events must already be off the timeline, as when the timeline is
// new.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::Reset(CTimeline* pcTimeline)
{
	ResetEventsAndPorts();
	ResetState();
	SimulationStarted(pcTimeline);
	WriteOutputs(pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets the chip's events and resets its ports, before it starts again.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::ResetEventsAndPorts(void)
{
	size	i;

	mapcEvents.ReInit();
	for (i = 0; i < mpcPins->NumPorts(); i++)
	{
		mpcPins->GetPort(i)->Reset();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Sets what the chip remembers back to how it starts.  Chips with nothing
// to remember don't override it.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::ResetState(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Drives the outputs from what the chip remembers.  Chips whose outputs
// don't come from what they remember don't override it.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::WriteOutputs(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CIntegratedCircuit::GetVCC(int64 iTime)
{
	return mpcPins->GetVoltageCommon()->GetVoltageIn(iTime);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CIntegratedCircuit::GetGND(int64 iTime)
{
	return mpcPins->GetVoltageGround()->GetVoltageIn(iTime);
}


//////////////////////////////////////////////////////////////////////////
//
// VCC is above zero and GND is zero.
//
//////////////////////////////////////////////////////////////////////////
bool CIntegratedCircuit::IsPowered(int64 iTime)
{
	float32	fVcc;
	float32	fGnd;

	fVcc = GetVCC(iTime);
	fGnd = GetGND(iTime);
	return (IsVoltage(fVcc) && (fVcc > 0.0f)) && (IsVoltage(fGnd) && (fGnd == 0.0f));
}


//////////////////////////////////////////////////////////////////////////
//
// Called by CIntegratedCircuitEvent::Init.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::Add(CIntegratedCircuitEvent* pcEvent)
{
	mapcEvents.Add(pcEvent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIntegratedCircuit::Remove(CIntegratedCircuitEvent* pcEvent)
{
	size	i;
	CChars	sz;

	for (i = 0; i < mapcEvents.NumElements(); i++)
	{
		if (mapcEvents.GetPtr(i) == pcEvent)
		{
			mapcEvents.RemoveAt(i, true);
			return true;
		}
	}

	sz.Init(" Cannot remove event on ");
	AppendDescription(&sz);
	sz.Append(".");
	gcLogger.Error2(__METHOD__, sz.Text(), NULL);
	sz.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPins* CIntegratedCircuit::GetPins(void) { return mpcPins; }
CPort* CIntegratedCircuit::GetPort(const char* szName) { return mpcPins->GetPort(szName); }
CPort* CIntegratedCircuit::GetPort(size uiIndex) { return mpcPins->GetPort(uiIndex); }
size CIntegratedCircuit::NumPorts(void) { return mpcPins->NumPorts(); }
size CIntegratedCircuit::NumEvents(void) { return mapcEvents.NumElements(); }
CEvent* CIntegratedCircuit::GetEvent(size uiIndex) { return mapcEvents.SafeGetPtr(uiIndex); }

