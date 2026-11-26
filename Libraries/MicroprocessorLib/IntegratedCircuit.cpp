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
// Keeps pPins and adds the chip to its circuit, which keeps it.  The
// state is created by Reset.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CPins> pPins)
{
	PreInit();

	CCircuitComponent::Init(pcContainingSubcircuitSimulation, szName);
	mpPins = pPins;
	mpPins->SetIntegratedCircuit(this);
	mapcEvents.Init();
	mbPowered = false;
	mfPoweredVcc = NoVoltage();
	GetCircuit()->Add(this);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::Class(void)
{
	CCircuitComponent::Class();

	M_Pointer(mpPins);
	U_Unknown(CArrayEventPtr, mapcEvents);
	U_Bool(mbPowered);
	U_Float32(mfPoweredVcc);
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the chip's own events off the timeline.  Its pins, which may be
// freed before or after it, take their ports' events off and disconnect
// the ports without telling anyone.  To take a chip out of a running
// simulation call DisconnectPorts first.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::Free(void)
{
	Disconnect(GetTimeline());
	mapcEvents.Kill();

	CCircuitComponent::Free();
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
// A power port connected, cut or left undriven may have changed the
// chip's power.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::TraceConnected(CTimeline* pcTimeline, CPort* pcPort)
{
	if (pcPort->IsPowerIn())
	{
		PowerChanged(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Starts the chip if its power has come, stops it if its power has gone,
// and if only its VCC has moved has its ports write again at the new VCC
// and read their traces at its thresholds.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::PowerChanged(CTimeline* pcTimeline)
{
	int64	iTime;
	bool	bPowered;
	float32	fVcc;
	float32	fOldVcc;
	size	i;
	CPort*	pcPort;

	iTime = pcTimeline->GetTime();
	bPowered = IsPowered(iTime);
	fVcc = GetVCC(iTime);
	fOldVcc = mfPoweredVcc;
	if (bPowered && !mbPowered)
	{
		Start(pcTimeline);
	}
	else if (!bPowered && mbPowered)
	{
		Stop(pcTimeline);
	}
	else if (bPowered && (fVcc != fOldVcc))
	{
		for (i = 0; i < mpPins->NumPorts(); i++)
		{
			pcPort = mpPins->GetPort(i);
			if (pcPort->IsLogicPort())
			{
				((CLogicPort*)pcPort)->VccChanged(pcTimeline, fOldVcc);
			}
		}
	}
	mbPowered = bPowered;
	mfPoweredVcc = fVcc;
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
// Starts the chip again with new state, and if it has power drives its
// outputs from it; without power it starts when its power comes, see
// PowerChanged.  Its events must already be off the timeline, as when
// the timeline is new.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::Reset(CTimeline* pcTimeline)
{
	int64	iTime;

	ResetEventsAndPorts();
	ResetState();
	iTime = pcTimeline->GetTime();
	mbPowered = IsPowered(iTime);
	mfPoweredVcc = GetVCC(iTime);
	if (mbPowered)
	{
		SimulationStarted(pcTimeline);
		WriteOutputs(pcTimeline);
	}
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
	for (i = 0; i < mpPins->NumPorts(); i++)
	{
		mpPins->GetPort(i)->Reset();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the chip's events and its ports' events off the timeline.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::RemoveAllEvents(CTimeline* pcTimeline)
{
	size	i;
	CPort*	pcPort;

	Disconnect(pcTimeline);
	for (i = 0; i < mpPins->NumPorts(); i++)
	{
		pcPort = mpPins->GetPort(i);
		if (pcPort->IsLogicPort())
		{
			((CLogicPort*)pcPort)->RemoveEvents(pcTimeline);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The chip's power has come while the simulation runs: it starts as from
// power up, and reads the inputs that are already at a level, which no
// transition will tell it about.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::Start(CTimeline* pcTimeline)
{
	size	i;
	CPort*	pcPort;

	RemoveAllEvents(pcTimeline);
	Reset(pcTimeline);
	for (i = 0; i < mpPins->NumPorts(); i++)
	{
		pcPort = mpPins->GetPort(i);
		if (pcPort->IsLogicPort() && (pcPort->GetPortType() != PT_Output))
		{
			((CLogicPort*)pcPort)->ReadTraceLevel(pcTimeline);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The chip's power has gone: it forgets what it was doing and its
// outputs stop driving, which the ports on their traces are told.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::Stop(CTimeline* pcTimeline)
{
	size		i;
	CPort*		pcPort;
	CLogicPort*	pcLogicPort;

	RemoveAllEvents(pcTimeline);
	for (i = 0; i < mpPins->NumPorts(); i++)
	{
		pcPort = mpPins->GetPort(i);
		if (pcPort->IsLogicPort())
		{
			pcLogicPort = (CLogicPort*)pcPort;
			if (!pcLogicPort->GetOutput()->IsNone())
			{
				pcLogicPort->WriteImpedance(pcTimeline);
			}
		}
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
// The element what the chip remembers is saved in, or NULL if it remembers
// nothing beyond its ports and events.
//
//////////////////////////////////////////////////////////////////////////
const char* CIntegratedCircuit::GetStateXMLName(void) const
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes what the chip remembers into its GetStateXMLName element, which
// the caller has begun.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuit::SaveXML(CXMLWriter* pcWriter)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Sets what the chip remembers from its GetStateXMLName element.  False
// (and an error logged) if the element is bad.
//
//////////////////////////////////////////////////////////////////////////
bool CIntegratedCircuit::LoadXML(CMarkupTag* pcTag)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CIntegratedCircuit::GetVCC(int64 iTime)
{
	return mpPins->GetVoltageCommon()->GetVoltageIn(iTime);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CIntegratedCircuit::GetGND(int64 iTime)
{
	return mpPins->GetVoltageGround()->GetVoltageIn(iTime);
}


//////////////////////////////////////////////////////////////////////////
//
// GND is zero and VCC is one the chip's family works at, see
// CVoltageConfigurationSource::IsPowered; any VCC above zero for a chip
// with no family.  Always for a chip that needs no power.
//
//////////////////////////////////////////////////////////////////////////
bool CIntegratedCircuit::IsPowered(int64 iTime)
{
	float32							fVcc;
	float32							fGnd;
	CVoltageConfigurationSource*	pcSource;

	if (!NeedsPower())
	{
		return true;
	}
	fGnd = GetGND(iTime);
	if (!IsVoltage(fGnd) || (fGnd != 0.0f))
	{
		return false;
	}
	fVcc = GetVCC(iTime);
	pcSource = GetVoltageConfigurationSource();
	if (pcSource != NULL)
	{
		return pcSource->IsPowered(fVcc);
	}
	return IsVoltage(fVcc) && (fVcc > 0.0f);
}


//////////////////////////////////////////////////////////////////////////
//
// An instrument, as an oscilloscope, runs whether or not the circuit has
// power.
//
//////////////////////////////////////////////////////////////////////////
bool CIntegratedCircuit::NeedsPower(void) const
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The family of the chip's first logic port that has one, or NULL.
//
//////////////////////////////////////////////////////////////////////////
CVoltageConfigurationSource* CIntegratedCircuit::GetVoltageConfigurationSource(void)
{
	size							i;
	CPort*							pcPort;
	CVoltageConfigurationSource*	pcSource;

	for (i = 0; i < mpPins->NumPorts(); i++)
	{
		pcPort = mpPins->GetPort(i);
		if (pcPort->IsLogicPort())
		{
			pcSource = ((CLogicPort*)pcPort)->GetVoltageConfigurationSource();
			if (pcSource != NULL)
			{
				return pcSource;
			}
		}
	}
	return NULL;
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
CPins* CIntegratedCircuit::GetPins(void) { return &mpPins; }
CPort* CIntegratedCircuit::GetPort(const char* szName) { return mpPins->GetPort(szName); }
CPort* CIntegratedCircuit::GetPort(size uiIndex) { return mpPins->GetPort(uiIndex); }
size CIntegratedCircuit::NumPorts(void) { return mpPins->NumPorts(); }
size CIntegratedCircuit::NumEvents(void) const { return mapcEvents.NumElements(); }
CEvent* CIntegratedCircuit::GetEvent(size uiIndex) { return mapcEvents.SafeGetPtr(uiIndex); }

