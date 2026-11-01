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
#include "Voltage.h"
#include "Trace.h"


//The id the next trace is given.
int64	giNextTraceID = 1;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrace::Init(void)
{
	Init(giNextTraceID);
}


//////////////////////////////////////////////////////////////////////////
//
// Later traces are given ids after iID.
//
//////////////////////////////////////////////////////////////////////////
void CTrace::Init(int64 iID)
{
	mapcConnectedPorts.Init();
	miID = iID;
	if (iID >= giNextTraceID)
	{
		giNextTraceID = iID + 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Ports still on the trace forget it, without telling anyone, as when
// a circuit is freed before its components.
//
//////////////////////////////////////////////////////////////////////////
void CTrace::Kill(void)
{
	size	i;

	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		mapcConnectedPorts.GetPtr(i)->TraceKilled();
	}
	mapcConnectedPorts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The average of the driven voltages, NaN when nothing drives the trace.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrace::GetVoltage(int64 iTime)
{
	size	i;
	float32	fVoltage;
	float32	fDrivenVoltage;
	size	uiDrivers;

	fDrivenVoltage = 0;
	uiDrivers = 0;
	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		fVoltage = mapcConnectedPorts.GetPtr(i)->GetVoltageOut(iTime);
		if (IsVoltage(fVoltage))
		{
			fDrivenVoltage += fVoltage;
			uiDrivers++;
		}
	}

	if (uiDrivers == 1)
	{
		return fDrivenVoltage;
	}
	if (uiDrivers == 0)
	{
		return NoVoltage();
	}
	return fDrivenVoltage / uiDrivers;
}


//////////////////////////////////////////////////////////////////////////
//
// The difference between the highest and lowest driven voltages, 0 when
// nothing drives the trace.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrace::GetShortVoltage(int64 iTime)
{
	float32	fMinimum;
	float32	fMaximum;

	fMinimum = GetMinimumVoltage(iTime);
	if (!IsVoltage(fMinimum))
	{
		return 0;
	}
	fMaximum = GetMaximumVoltage(iTime);
	return fMaximum - fMinimum;
}


//////////////////////////////////////////////////////////////////////////
//
// NaN when nothing drives the trace.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrace::GetMinimumVoltage(int64 iTime)
{
	size	i;
	float32	fVoltage;
	float32	fMinimum;

	fMinimum = NoVoltage();
	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		fVoltage = mapcConnectedPorts.GetPtr(i)->GetVoltageOut(iTime);
		if (IsVoltage(fVoltage))
		{
			if (!IsVoltage(fMinimum) || (fVoltage < fMinimum))
			{
				fMinimum = fVoltage;
			}
		}
	}
	return fMinimum;
}


//////////////////////////////////////////////////////////////////////////
//
// NaN when nothing drives the trace.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrace::GetMaximumVoltage(int64 iTime)
{
	size	i;
	float32	fVoltage;
	float32	fMaximum;

	fMaximum = NoVoltage();
	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		fVoltage = mapcConnectedPorts.GetPtr(i)->GetVoltageOut(iTime);
		if (IsVoltage(fVoltage))
		{
			if (!IsVoltage(fMaximum) || (fVoltage > fMaximum))
			{
				fMaximum = fVoltage;
			}
		}
	}
	return fMaximum;
}


//////////////////////////////////////////////////////////////////////////
//
// Like "3.3V", or "----" when nothing drives the trace.
//
//////////////////////////////////////////////////////////////////////////
void CTrace::AppendVoltage(CChars* psz, int64 iTime)
{
	::AppendVoltage(psz, GetVoltage(iTime));
}


//////////////////////////////////////////////////////////////////////////
//
// A port is only connected once.
//
//////////////////////////////////////////////////////////////////////////
void CTrace::Connect(CPort* pcPort)
{
	size	i;

	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		if (mapcConnectedPorts.GetPtr(i) == pcPort)
		{
			return;
		}
	}
	mapcConnectedPorts.Add(pcPort);
}


//////////////////////////////////////////////////////////////////////////
//
// Tells the ports, except pcExcept, that the trace has stopped being
// driven, see CPort::TraceUndriven.  Logicim doesn't; its ports never
// learn their trace went undriven.
//
//////////////////////////////////////////////////////////////////////////
void CTrace::NotifyUndriven(CTimeline* pcTimeline, CPort* pcExcept)
{
	size	i;
	CPort*	pcPort;

	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		pcPort = mapcConnectedPorts.GetPtr(i);
		if (pcPort != pcExcept)
		{
			pcPort->TraceUndriven(pcTimeline);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps the other ports in the order they were connected.
//
//////////////////////////////////////////////////////////////////////////
void CTrace::Disconnect(CPort* pcPort)
{
	size	i;

	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		if (mapcConnectedPorts.GetPtr(i) == pcPort)
		{
			mapcConnectedPorts.RemoveAt(i, true);
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// 0 for NULL.
//
//////////////////////////////////////////////////////////////////////////
int64 CTrace::GetID(CTrace* pcTrace)
{
	if (pcTrace != NULL)
	{
		return pcTrace->GetID();
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrace::ResetNextID(void)
{
	giNextTraceID = 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTrace::NumConnectedPorts(void) { return mapcConnectedPorts.NumElements(); }
CPort* CTrace::GetConnectedPort(size uiIndex) { return mapcConnectedPorts.SafeGetPtr(uiIndex); }
int64 CTrace::GetID(void) { return miID; }

